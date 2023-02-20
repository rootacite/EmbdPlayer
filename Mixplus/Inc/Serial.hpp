
#include "main.h"
#include "usart.h"

#define _NO_STL

#ifdef _NO_STL
#include "Buffer.hpp"
#else
#include <string>
#include <vector>
#include <cstring>
#include <map>
#endif

#ifdef _NO_STL
class Serial : public Buffer<char>
{
private:
    UART_HandleTypeDef *uart;
    friend void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

public:
    Serial(UART_HandleTypeDef *t);

    bool available();
    void begin();

    char read();
    void read(int c,char *buf);

    void write(char c);
    void write(char *c,int length);
};

uint8_t cc_serial[1];
Serial * Launched_Dic[8] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
int _pos_of_uart=0;

Serial::Serial(UART_HandleTypeDef* t) : Buffer<char>(512)
{
    this->uart = t;
    Launched_Dic[_pos_of_uart] = this;
    _pos_of_uart++;
}

void Serial::begin()
{
    HAL_UART_Receive_IT(uart,cc_serial,1);
}


bool Serial::available()
{
    return this->buffered_size() > 0;
}

void Serial::read(int c,char *buf)
{
    int _p=0;

    while(_p < c)
    {
        if(this->available())
        {
            buf[_p]=this->read();
            _p++;
        }
    }
}

char Serial::read()
{
    while(!this->available()){}

    return pop();
}

void Serial::write(char c)
{
    uint8_t cc = c;
    HAL_UART_Transmit(this->uart,&cc,1,0xFFFF);
}

void Serial::write(char *c,int length)
{
    HAL_UART_Transmit(this->uart,(uint8_t*)c,length,0xFFFF);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    for(Serial* i : Launched_Dic)
    {
        if(i->uart ==  huart)
        {
            i->push(cc_serial[0]);
        }
    }

    HAL_UART_Receive_IT(huart,cc_serial,1);
}

#else

class Serial
{
private:
    UART_HandleTypeDef* uart;
    std::vector<char> buffer;

    friend void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);

public:
    explicit Serial(UART_HandleTypeDef* t);

    int bufferedSize();
    bool available();
    void begin();

    char read();
    std::string readline();
    std::vector<char> readbytes(int c);

    void write(char c);
    void write(std::string s);
    void write(char* c, int length);
    void writeline(std::string str);
};

uint8_t cc_serial[1];
std::map<UART_HandleTypeDef*, Serial*> Launched_Dic;

Serial::Serial(UART_HandleTypeDef* t)
{
    this->uart = t;

    Launched_Dic.insert(std::pair<UART_HandleTypeDef*, Serial*>(this->uart, this));
}

void Serial::begin()
{
    HAL_UART_Receive_IT(uart, cc_serial, 1);

}

int Serial::bufferedSize()
{
    return this->buffer.size();
}

bool Serial::available()
{
    return this->buffer.size() > 0;
}

std::string Serial::readline()
{
    std::string r = "";
    char c;
    do {
        while (!this->available()) {}
        c = this->read();

        if (c == '\n')
            break;
        else
            r += c;
    } while (1);

    return r;
}

std::vector<char> Serial::readbytes(int c)
{
    std::vector<char> r;

    while (r.size() < c)
    {
        if (this->available())
        {
            r.push_back(this->read());
        }
    }

    return r;
}

char Serial::read()
{
    while (!this->available()) {};

    char r = this->buffer[0];
    this->buffer.erase(this->buffer.begin());
    return r;
}

void Serial::write(char c)
{
    uint8_t cc = c;
    HAL_UART_Transmit(this->uart, &cc, 1, 0xFFFF);
}

void Serial::write(char* c, int length)
{
    HAL_UART_Transmit(this->uart, (uint8_t*)c, length, 0xFFFF);
}

void Serial::write(std::string str)
{
    this->write((char*)str.c_str(), str.length());
}
void Serial::writeline(std::string str) {
    uint8_t* pc = new uint8_t[str.length() + 1];
    memcpy(pc, str.c_str(), str.length());
    pc[str.length()] = '\n';

    this->write((char*)pc, str.length() + 1);

    delete[]pc;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    Launched_Dic[huart]->buffer.push_back(cc_serial[0]);

    HAL_UART_Receive_IT(huart, cc_serial, 1);
}
#endif