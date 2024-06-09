#ifndef TYPECONVERTER_H
#define TYPECONVERTER_H

#include "../inc/types.h"

class IntConverter
{
    private:
        const static unsigned int bufferSize = 21;
        char buffer[bufferSize];
        uint8 index;
        void resetProps();
        void writeNumberToBuffer(uint64 n);

    public:
        IntConverter();
        char *intToChar(int n);
        char *uintToChar(const uint32 n);
        char *uintToChar(const uint64 n);
};

#endif
