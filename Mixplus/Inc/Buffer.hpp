#include "stdlib.h"

template <typename T>
class Buffer
{
private:
	T* _buffer;
	unsigned int _size;
	unsigned int _point;
	unsigned int _bottom = 0;
	unsigned int _count = 0;

	void circle_raise(unsigned int* x, unsigned int limit);

public:
	explicit Buffer(int Size);
	~Buffer();

	unsigned int buffered_size();
	void push(T c);
	T pop();
	unsigned int size();

	T& operator[](int i);
};

template<typename T>
T& Buffer<T>::operator[](int i)
{
	return _buffer[i];
}

template <typename T>
unsigned int Buffer<T>::size()
{
	return _size;
}

template <typename T>
Buffer<T>::Buffer(int Size)
{
	_buffer = (T*)malloc(sizeof(T)*Size);
	_size = Size;
	_point = 0;
}

template <typename T>
Buffer<T>::~Buffer()
{
	free(_buffer);
}

template <typename T>
void Buffer<T>::circle_raise(unsigned int* x, unsigned int limit)
{
	(*x)++;
	if (*x >= limit) *x = 0;
}

template <typename T>
void Buffer<T>::push(T c)
{
	//if (_count == _size) throw 1;

	_buffer[_point] = c;
	circle_raise(&_point, _size);

	_count++;
}

template <typename T>
T Buffer<T>::pop()
{
	//if (_count == 0)throw 0;

	T c = _buffer[_bottom];
	circle_raise(&_bottom, _size);

	_count--;
	return c;
}

template <typename T>
unsigned int Buffer<T>::buffered_size()
{
	return _count;
}