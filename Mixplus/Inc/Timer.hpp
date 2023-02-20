
#include "main.h"
#include "tim.h"

typedef void(*OnTickListener)();

class Timer
{
private:
    TIM_HandleTypeDef* tim;
    OnTickListener tick;

    friend void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
public:
    explicit Timer(TIM_HandleTypeDef* t);
    explicit Timer(TIM_HandleTypeDef* t,int arr);

    static void delay(int t);
    void ontick(OnTickListener CallBack);
    void start();
    void stop();
    unsigned int getcounter();
    void reset();
    void setcounter(unsigned int c);

    void freq(int clockfreq,int f);
    void circle(int c);
};

TIM_HandleTypeDef *tim_handle_list[16] = {0};
Timer * obj_list[16] = {0};
int Point_Of_Timer = 0;

void Timer::delay(int t) {
    HAL_Delay(t);
}

Timer::Timer(TIM_HandleTypeDef *t) {
    this->tim=t;
    this->tick = nullptr;

    tim_handle_list[Point_Of_Timer] = t;
    obj_list[Point_Of_Timer] = this;

    Point_Of_Timer++;
}

Timer::Timer(TIM_HandleTypeDef *t, int arr) {
    this->tim=t;
    this->tick = nullptr;
    this->tim->Instance->ARR = arr;

    tim_handle_list[Point_Of_Timer] = t;
    obj_list[Point_Of_Timer] = this;

    Point_Of_Timer++;
}

void Timer::ontick(OnTickListener CallBack) {
    tick = CallBack;
}

void Timer::start() {
    HAL_TIM_Base_Start_IT(tim);
}

void Timer::stop() {
    HAL_TIM_Base_Stop_IT(tim);
}

unsigned int Timer::getcounter()
{
    return __HAL_TIM_GET_COUNTER(tim);
}

void Timer::setcounter(unsigned int c)
{
    __HAL_TIM_SET_COUNTER(tim, c);
}

void Timer::reset()
{
    this->setcounter(0);
}

void Timer::freq(int clockfreq, int f)
{
    int p = (clockfreq * 1000000) / f;
    if(p>65535)return;

    tim->Instance->PSC = p - 1;
    this->setcounter(tim->Instance->ARR);
}

void Timer::circle(int c)
{
    tim->Instance->ARR = c;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    for (int i = 0; i < Point_Of_Timer; i++) {
        if (tim_handle_list[i] == htim) {
            if (obj_list[i]->tick != nullptr)
                obj_list[i]->tick();
            break;
        }
    }
}