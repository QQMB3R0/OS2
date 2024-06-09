#include "TypeConverter.h"

IntConverter::IntConverter()
{
    resetProps();
};

void IntConverter::resetProps()
{
    for(int i = 0; i < 21; i++) buffer[i] = '\0';
    index = bufferSize - 2;
};

char *IntConverter::intToChar(int n)
{
    resetProps();

    int number = n >> 31;   // n < 0 - n = -1, else n = 0
    number = (n ^ number) - number; // Clear sign bit

    bool isNegative = n < 0;

    writeNumberToBuffer(number);
    
    if(isNegative) buffer[index] = '-';

    return &buffer[index];
};

char *IntConverter::uintToChar(const uint32 n)
{
    resetProps();
    writeNumberToBuffer(n);
    
    return &buffer[++index];
};

char *IntConverter::uintToChar(const uint64 n)
{
    resetProps(); 
    writeNumberToBuffer(n);

    return &buffer[++index];
};

void IntConverter::writeNumberToBuffer(uint64 n)
{
    do {
        buffer[index--] = '0' + (n % 10);
        n /= 10;
    }while(n != 0);
};
