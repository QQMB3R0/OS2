#ifndef VGAGRAPHICS_H
#define VGAGRAPHICS_H

#include "VGADisplayInfo.h"
#include "../inc/TypeConverter.h"

typedef unsigned short dchar;

class VGADisplay
{
    private:
        uint16 xCurr;
        uint16 yCurr;
        dchar *videoMem;
        bool shift;
        char color;
        bool isBlink;
        bool isBright;

        void doShift();
        // 0 if passed char isn't escape sequence, else 1
        bool printChar(const char &c);
        void goHorizontalTab();

    public:
        VGADisplay();
        VGADisplay(uint16 x, uint16 y);
        VGADisplay& operator<<(const char &c);
        VGADisplay& operator<<(const char *c);
        VGADisplay& operator<<(const uint32 number);
        VGADisplay& operator<<(const uint64 number);
         void goNewLine();

        VGADisplay& operator++(int);
        void setChar(uint8 hexNum);
        void setBackSpace();
        void setShift(bool status);
        void setColor(char colorAttr);
        void setBlink(bool blinkAttr);
        void setBright(bool brigthAttr);
        char getChar();
};
#endif
