
#include "main.h"
#include "i2c.h"

class I2CMaster
{
private:
    I2C_HandleTypeDef * i2c;
public:
    I2CMaster(I2C_HandleTypeDef * i);
    void transmit(uint16_t addr, char *c, int length);
};

I2CMaster::I2CMaster(I2C_HandleTypeDef *i)
{
    this->i2c=i;
}

void I2CMaster::transmit(uint16_t addr, char *c, int length)
{
    HAL_I2C_Master_Transmit(i2c, (uint16_t) (addr << 1), (uint8_t*)c, length,0xFFFF);
}
