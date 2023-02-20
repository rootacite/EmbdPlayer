#include "main.h"

class GPIO
{
private:
    GPIO_TypeDef* Port;
    uint16_t Pin;

    const GPIO_PinState LOW = GPIO_PIN_RESET;
    const GPIO_PinState HIGH = GPIO_PIN_SET;

public:
    explicit GPIO(GPIO_TypeDef* Port, int16_t Pin);

    bool get();
    void set(bool s);
    void toggle();
};

GPIO::GPIO(GPIO_TypeDef* Port, int16_t Pin)
{
    this->Port = Port;
    this->Pin = Pin;
}

bool GPIO::get()
{
    return (HAL_GPIO_ReadPin(Port, Pin) == HIGH);
}

void GPIO::set(bool s)
{
    if (s)
        HAL_GPIO_WritePin(Port, Pin, HIGH);
    else
        HAL_GPIO_WritePin(Port, Pin, LOW);
}

void GPIO::toggle()
{
    this->set(!this->get());
}