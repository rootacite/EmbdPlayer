#include "SDdriver.h"
#include "spi.h"
#include "usart.h"
#include "ff.h"

uint8_t DFF = 0xFF;
uint8_t test;
uint8_t SD_TYPE = 0x00;

MSD_CARDINFO SD0_CardInfo;


void SD_SetCSPin(uint8_t p) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}


int SD_SayCommand(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t r1;
    uint8_t retry;


    do {
        retry = SPI_ReadAndWrite(DFF);
    } while (retry != 0xFF);

    SPI_ReadAndWrite(cmd | 0x40);
    SPI_ReadAndWrite(arg >> 24);
    SPI_ReadAndWrite(arg >> 16);
    SPI_ReadAndWrite(arg >> 8);
    SPI_ReadAndWrite(arg);
    SPI_ReadAndWrite(crc);
    if (cmd == CMD12)SPI_ReadAndWrite(DFF);
    do {
        r1 = SPI_ReadAndWrite(0xFF);
    } while (r1 & 0X80);

    return r1;
}


uint8_t SD_Initialization(void) {
    uint8_t r1;
    uint8_t buff[6] = {0};
    uint16_t retry;
    uint8_t i;

    MX_SPI1_Init();
    SPI_Speed(SPI_BAUDRATEPRESCALER_256);
    for (retry = 0; retry < 10; retry++) {
        SPI_ReadAndWrite(DFF);
    }

    do {
        r1 = SD_SayCommand(CMD0, 0, 0x95);
    } while (r1 != 0x01);

    SD_TYPE = 0;
    r1 = SD_SayCommand(CMD8, 0x1AA, 0x87);
    if (r1 == 0x01) {
        for (i = 0; i < 4; i++)buff[i] = SPI_ReadAndWrite(DFF);    //Get trailing return value of R7 resp
        if (buff[2] == 0X01 && buff[3] == 0XAA)
        {
            retry = 0XFFFE;
            do {
                SD_SayCommand(CMD55, 0, 0X01);
                r1 = SD_SayCommand(CMD41, 0x40000000, 0X01);
            } while (r1 && retry--);
            if (retry && SD_SayCommand(CMD58, 0, 0X01) == 0)
            {
                for (i = 0; i < 4; i++)buff[i] = SPI_ReadAndWrite(0XFF);
                if (buff[0] & 0x40) {
                    SD_TYPE = V2HC;
                } else {
                    SD_TYPE = V2;
                }
            }
        } else {
            SD_SayCommand(CMD55, 0, 0X01);
            r1 = SD_SayCommand(CMD41, 0, 0X01);
            if (r1 <= 1) {
                SD_TYPE = V1;
                retry = 0XFFFE;
                do
                {
                    SD_SayCommand(CMD55, 0, 0X01);
                    r1 = SD_SayCommand(CMD41, 0, 0X01);
                } while (r1 && retry--);
            } else
            {
                SD_TYPE = MMC;//MMC V3
                retry = 0XFFFE;
                do
                {
                    r1 = SD_SayCommand(CMD1, 0, 0X01);
                } while (r1 && retry--);
            }
            if (retry == 0 || SD_SayCommand(CMD16, 512, 0X01) != 0)SD_TYPE = ERR;
        }
    }
    SPI_Speed(SPI_BAUDRATEPRESCALER_4);
    if (SD_TYPE)return 0;
    else return 1;
}


uint8_t SD_ReceiveData(uint8_t *data, uint16_t len) {

    uint8_t r1;
    do {
        r1 = SPI_ReadAndWrite(0xFF);
        //HAL_Delay(100);
    } while (r1 != 0xFE);
    while (len--) {
        *data = SPI_ReadAndWrite(0xFF);
        data++;

    }
    SPI_ReadAndWrite(0xFF);
    SPI_ReadAndWrite(0xFF);
    return 0;
}


uint8_t SD_SendBlock(uint8_t *buf, uint8_t cmd) {
    uint16_t t;
    uint8_t r1;
    do {
        r1 = SPI_ReadAndWrite(0xFF);
    } while (r1 != 0xFF);

    SPI_ReadAndWrite(cmd);
    if (cmd != 0XFD)
    {
        for (t = 0; t < 512; t++)SPI_ReadAndWrite(buf[t]);
        SPI_ReadAndWrite(0xFF);
        SPI_ReadAndWrite(0xFF);
        t = SPI_ReadAndWrite(0xFF);
        if ((t & 0x1F) != 0x05)return 2;
    }
    return 0;
}

uint8_t SD_GETCSD(uint8_t *csd_data) {
    uint8_t r1;
    r1 = SD_SayCommand(CMD9, 0, 0x01);//发CMD9命令，读CSD寄存器
    if (r1 == 0) {
        r1 = SD_ReceiveData(csd_data, 16);//接收16个字节的数据
    }
    if (r1)return 1;
    else return 0;
}

uint32_t SD_GetSectorCount(void) {
    uint8_t csd[16];
    uint32_t Capacity;
    uint8_t n;
    uint16_t csize;

    if (SD_GETCSD(csd) != 0) return 0;


    if ((csd[0] & 0xC0) == 0x40)
    {
        csize = csd[9] + ((uint16_t) csd[8] << 8) + 1;
        Capacity = (uint32_t) csize << 10;
    } else
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t) csd[7] << 2) + ((uint16_t) (csd[6] & 3) << 10) + 1;
        Capacity = (uint32_t) csize << (n - 9);
    }
    return Capacity;
}

int MSD0_GetCardInfo(PMSD_CARDINFO SD0_CardInfo) {
    uint8_t r1;
    uint8_t CSD_Tab[16];
    uint8_t CID_Tab[16];

    /* Send CMD9, Read CSD */
    r1 = SD_SayCommand(CMD9, 0, 0xFF);
    if (r1 != 0x00) {
        return r1;
    }

    if (SD_ReceiveData(CSD_Tab, 16)) {
        return 1;
    }

    /* Send CMD10, Read CID */
    r1 = SD_SayCommand(CMD10, 0, 0xFF);
    if (r1 != 0x00) {
        return r1;
    }

    if (SD_ReceiveData(CID_Tab, 16)) {
        return 2;
    }

    /* Byte 0 */
    SD0_CardInfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
    SD0_CardInfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
    SD0_CardInfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
    /* Byte 1 */
    SD0_CardInfo->CSD.TAAC = CSD_Tab[1];
    /* Byte 2 */
    SD0_CardInfo->CSD.NSAC = CSD_Tab[2];
    /* Byte 3 */
    SD0_CardInfo->CSD.MaxBusClkFrec = CSD_Tab[3];
    /* Byte 4 */
    SD0_CardInfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
    /* Byte 5 */
    SD0_CardInfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
    SD0_CardInfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
    /* Byte 6 */
    SD0_CardInfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
    SD0_CardInfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
    SD0_CardInfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
    SD0_CardInfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
    SD0_CardInfo->CSD.Reserved2 = 0; /* Reserved */
    SD0_CardInfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
    /* Byte 7 */
    SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
    /* Byte 8 */
    SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
    SD0_CardInfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
    SD0_CardInfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
    /* Byte 9 */
    SD0_CardInfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
    SD0_CardInfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
    SD0_CardInfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
    /* Byte 10 */
    SD0_CardInfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
    SD0_CardInfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
    SD0_CardInfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
    /* Byte 11 */
    SD0_CardInfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
    SD0_CardInfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
    /* Byte 12 */
    SD0_CardInfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
    SD0_CardInfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
    SD0_CardInfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
    SD0_CardInfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
    /* Byte 13 */
    SD0_CardInfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
    SD0_CardInfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
    SD0_CardInfo->CSD.Reserved3 = 0;
    SD0_CardInfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
    /* Byte 14 */
    SD0_CardInfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
    SD0_CardInfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
    SD0_CardInfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
    SD0_CardInfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
    SD0_CardInfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
    SD0_CardInfo->CSD.ECC = (CSD_Tab[14] & 0x03);
    /* Byte 15 */
    SD0_CardInfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
    SD0_CardInfo->CSD.Reserved4 = 1;

    if (SD0_CardInfo->CardType == V2HC) {
        /* Byte 7 */
        SD0_CardInfo->CSD.DeviceSize = (uint16_t) (CSD_Tab[8]) * 256;
        /* Byte 8 */
        SD0_CardInfo->CSD.DeviceSize += CSD_Tab[9];
    }

    SD0_CardInfo->Capacity = SD0_CardInfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
    SD0_CardInfo->BlockSize = MSD_BLOCKSIZE;

    /* Byte 0 */
    SD0_CardInfo->CID.ManufacturerID = CID_Tab[0];
    /* Byte 1 */
    SD0_CardInfo->CID.OEM_AppliID = CID_Tab[1] << 8;
    /* Byte 2 */
    SD0_CardInfo->CID.OEM_AppliID |= CID_Tab[2];
    /* Byte 3 */
    SD0_CardInfo->CID.ProdName1 = CID_Tab[3] << 24;
    /* Byte 4 */
    SD0_CardInfo->CID.ProdName1 |= CID_Tab[4] << 16;
    /* Byte 5 */
    SD0_CardInfo->CID.ProdName1 |= CID_Tab[5] << 8;
    /* Byte 6 */
    SD0_CardInfo->CID.ProdName1 |= CID_Tab[6];
    /* Byte 7 */
    SD0_CardInfo->CID.ProdName2 = CID_Tab[7];
    /* Byte 8 */
    SD0_CardInfo->CID.ProdRev = CID_Tab[8];
    /* Byte 9 */
    SD0_CardInfo->CID.ProdSN = CID_Tab[9] << 24;
    /* Byte 10 */
    SD0_CardInfo->CID.ProdSN |= CID_Tab[10] << 16;
    /* Byte 11 */
    SD0_CardInfo->CID.ProdSN |= CID_Tab[11] << 8;
    /* Byte 12 */
    SD0_CardInfo->CID.ProdSN |= CID_Tab[12];
    /* Byte 13 */
    SD0_CardInfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
    /* Byte 14 */
    SD0_CardInfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
    /* Byte 15 */
    SD0_CardInfo->CID.ManufactDate |= CID_Tab[14];
    /* Byte 16 */
    SD0_CardInfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
    SD0_CardInfo->CID.Reserved2 = 1;

    return 0;
}

uint8_t SD_WriteDisk(uint8_t *buf, uint32_t sector, uint8_t cnt) {
    uint8_t r1;
    if (SD_TYPE != V2HC)sector *= 512;
    if (cnt == 1) {
        r1 = SD_SayCommand(CMD24, sector, 0X01);
        if (r1 == 0)
        {
            r1 = SD_SendBlock(buf, 0xFE);
        }
    } else {
        if (SD_TYPE != MMC) {
            SD_SayCommand(CMD55, 0, 0X01);
            SD_SayCommand(CMD23, cnt, 0X01);
        }
        r1 = SD_SayCommand(CMD25, sector, 0X01);
        if (r1 == 0) {
            do {
                r1 = SD_SendBlock(buf, 0xFC);
                buf += 512;
            } while (--cnt && r1 == 0);
            r1 = SD_SendBlock(0, 0xFD);
        }
    }
    return r1;
}

uint8_t SD_ReadDisk(uint8_t *buf, uint32_t sector, uint8_t cnt) {
    uint8_t r1;
    if (SD_TYPE != V2HC)sector <<= 9;
    if (cnt == 1) {
        r1 = SD_SayCommand(CMD17, sector, 0X01);
        if (r1 == 0)
        {
            r1 = SD_ReceiveData(buf, 512);
        }
    } else {
        r1 = SD_SayCommand(CMD18, sector, 0X01);
        do {
            r1 = SD_ReceiveData(buf, 512);
            buf += 512;
        } while (--cnt && r1 == 0);
        SD_SayCommand(CMD12, 0, 0X01);
    }
    return r1;
}



///////////////////////////END//////////////////////////////////////

