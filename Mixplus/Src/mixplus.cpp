
#include "mixplus.h"
#include "Serial.hpp"
#include "I2C.hpp"
#include "fatfs.h"

#include "Exti.hpp"

#include "ff.h"
#include "spi.h"

#include "PWM.hpp"
#include "Timer.hpp"

#include <string.h>
#include <cstdio>

#define delay(x) HAL_Delay(x)

#define MCP4725_I2CADDR_DEFAULT (96) ///< Default i2c address
#define MCP4725_CMD_WRITEDAC (0x40)    ///< Writes data to the DAC
#define MCP4725_CMD_WRITEDACEEPROM   (0x60)

Serial Serial1(&huart1);
Timer Timer1(&htim3);

#define VOL(X) setVoltage(X, 0)
FATFS fs;
FIL file;
char path[4]="0:";
char error[]="error!\r\n";
char mount[]="Success to Mount File System\r\n";
char c_open[]="Success to Open main.txt\r\n";

static char str_buf[1024];

UINT dwRead=0;
uint8_t ReadBuffer1[4096];
uint8_t ReadBuffer2[4096];

uint8_t* FrontBuffer=ReadBuffer1;
uint8_t* BackBuffer=ReadBuffer2;

int Pointer=0;
bool isBackPrepaired = false;
bool BufferStatus=true;

bool on_next=false;

int cco=0;
int song_index = 0;

inline uint8_t _16bitTo8Bit(uint16_t i)
{
    return i / 255;
}

inline uint16_t _16bitTo12Bit(uint16_t i)
{
    return i / 32;
}

void setup()
{
    Serial1.begin();
    HAL_Delay (500);


    uint8_t res=f_mount(&fs,"0:",0);  //挂载文件系统
    if(res!=FR_OK)
    {
        Serial1.write(error,sizeof(error));
        Error_Handler();
    }else{
        Serial1.write(mount,sizeof(mount));
    }

    TIM2->ARR =2048;
    TIM2->PSC = 0;
    __HAL_TIM_SET_COUNTER(&htim2,TIM2->ARR);

    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);

    Timer1.freq(72,441000);
    Timer1.circle(9);
    Timer1.ontick([](){
        if(Pointer>=4096)
        {
            if(!isBackPrepaired)return;

            Pointer=0;
            BufferStatus=!BufferStatus;
            isBackPrepaired=false;
        }
        if(BufferStatus)
        {
            uint16_t ix=_16bitTo12Bit(*(uint16_t*)(FrontBuffer+Pointer));
            __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,ix);
        }
        else
        {
            uint16_t ix=_16bitTo12Bit(*(uint16_t*)(BackBuffer+Pointer));
            __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,ix);
        }
        Pointer+=2;
    });
    Timer1.start();

    Exti::attachInterrupt(GPIO_PIN_9,[](){
        on_next=true;
    });
}

void loop()
{
    char f_path[32];
    sprintf(f_path,"%d.pcm",song_index);
    if(f_open(&file,f_path,FA_OPEN_EXISTING|FA_READ)==FR_OK)
        Serial1.write(c_open,sizeof(c_open));
    else
    {
        Serial1.write(error,sizeof(error));
        song_index=0;
        return;
    }


    f_lseek(&file, 0);
    //uint32_t got_size = 0;

    while(1){
        if(BufferStatus) {
            f_read(&file, BackBuffer, 4096, &dwRead);
        }
        else
        {
            f_read(&file, FrontBuffer, 4096, &dwRead);
        }
        if(dwRead==0)
            break;
        if(on_next)
        {
            delay(500);
            on_next=false;
            break;
        }
        //got_size+=dwRead;
        isBackPrepaired=true;
        do{
            HAL_Delay(1);
        }while(isBackPrepaired);
    }
    f_close(&file);

    song_index++;
}