
#include "main.h"
#include "adc.h"
#include "stdlib.h"

class Analog
{
private:
    ADC_HandleTypeDef* adc;
    int count_of_channels;

public:
    explicit Analog(ADC_HandleTypeDef* a, int cc);

    void get(uint16_t * buf);
};

Analog::Analog(ADC_HandleTypeDef* a, int cc)
{
    this->count_of_channels = cc;
    this->adc = a;
}

void Analog::get(uint16_t * buf)
{

    for (int i = 0; i < count_of_channels; i++)
    {
        HAL_ADC_Start(adc);
        HAL_ADC_PollForConversion(adc, 0xFFFF);

        if (HAL_IS_BIT_SET(HAL_ADC_GetState(adc), HAL_ADC_STATE_REG_EOC)) {
            buf[i] = HAL_ADC_GetValue(adc);
        }
    }
}

class AnalogDMA
{
private:
    ADC_HandleTypeDef* adc;
    int count_of_channels;

    uint16_t* Buffer = nullptr;
public:
    explicit AnalogDMA(ADC_HandleTypeDef* a, int cc);
    void begin();
    void get(uint16_t * buf);
};

AnalogDMA::AnalogDMA(ADC_HandleTypeDef* a, int cc)
{
    this->count_of_channels = cc;
    this->adc = a;
    Buffer = (uint16_t*)malloc(sizeof(uint16_t) * cc);
}

void AnalogDMA::begin()
{
    HAL_ADC_Start_DMA(adc, (uint32_t*)Buffer, count_of_channels);
}

void AnalogDMA::get(uint16_t * buf) {

    for (int i = 0; i < count_of_channels; i++)
    {
        buf[i] = Buffer[i];
    }
}
