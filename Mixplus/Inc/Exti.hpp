
#include "main.h"

typedef void(*OnExtiCallBack)();
static uint16_t Pins[]={
        GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,
        GPIO_PIN_6,GPIO_PIN_7,GPIO_PIN_8,GPIO_PIN_9,GPIO_PIN_10,GPIO_PIN_11,
        GPIO_PIN_12,GPIO_PIN_13,GPIO_PIN_14,GPIO_PIN_15
};
static OnExtiCallBack CallBacks[]={
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
};
class Exti
{
public:
    static void attachInterrupt(uint16_t Pin,OnExtiCallBack proc);
    static void detachInterrupt(uint16_t Pin);
};

void Exti::attachInterrupt(uint16_t Pin, OnExtiCallBack proc)
{
    int p = 0;
    for(uint16_t i : Pins)
    {
        if(i==Pin)
        {
            CallBacks[p] = proc;
        }
        p++;
    }
}

void Exti::detachInterrupt(uint16_t Pin)
{
    int p = 0;
    for(uint16_t i : Pins)
    {
        if(i==Pin)
        {
            CallBacks[p] = nullptr;
        }
        p++;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    int p = 0;
    for(uint16_t i : Pins)
    {
        if(i==GPIO_Pin && CallBacks[p]!= nullptr)
        {
            CallBacks[p]();
        }
        p++;
    }
}

