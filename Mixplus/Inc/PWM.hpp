
#include "main.h"
#include "tim.h"

class PWM
{
private:
    TIM_HandleTypeDef* tim;
    const unsigned int chs[4] = {TIM_CHANNEL_1,TIM_CHANNEL_2,TIM_CHANNEL_3,TIM_CHANNEL_4};
    bool status[4] = {false,false,false,false};
public:

    explicit PWM(TIM_HandleTypeDef* t);
    explicit PWM(TIM_HandleTypeDef* t,int arr);

    void begin(int index);
    void pulse(int index, double p);

    void freq(int clockfreq,int f);
};

PWM::PWM(TIM_HandleTypeDef *t) {
    this->tim=t;
}

PWM::PWM(TIM_HandleTypeDef *t, int arr) {
    this->tim=t;

    this->tim->Instance->ARR = arr;
}

void PWM::begin(int index) {
    if(!status[index])
    {
        HAL_TIM_PWM_Start(tim,chs[index]);
        status[index]=true;
    }
}

void PWM::pulse(int index, double p) {
    this->begin(index);
    int n_arr = this->tim->Instance->ARR;

    __HAL_TIM_SET_COMPARE(tim,chs[index],n_arr*p);
}

void PWM::freq(int clockfreq, int f)
{
    int p = (clockfreq * 1000000) / f;
    if(p>65535)return;

    tim->Instance->PSC = p - 1;
    __HAL_TIM_SET_COUNTER(tim,tim->Instance->ARR);
}