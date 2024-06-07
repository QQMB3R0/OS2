#include "VGADisplay.h"

VGADisplay::VGADisplay()
{
    isBright = isBlink = shift = 0;
    xCurr = yCurr = 0;
    color = (char)VGADisplayInfo::colorAttr::gray;
    videoMem = (dchar *)VGADisplayInfo::videoMemAddr;
};

VGADisplay::VGADisplay(uint16 x, uint16 y)
{
    isBright = isBlink = shift = 0;
    xCurr = yCurr = 0;
    color = (char)VGADisplayInfo::colorAttr::gray;
    videoMem = (dchar *)VGADisplayInfo::videoMemAddr;
};

VGADisplay& VGADisplay::operator<<(const char &c)
{
    printChar(c);

    return *this;
};

VGADisplay& VGADisplay::operator++(int)
{
    if(++xCurr == VGADisplayInfo::width)
    {
        yCurr++; 
        xCurr = 0; 
    }
    if(yCurr == VGADisplayInfo::height)
    {
        if(shift)
        { 
            this->doShift(); 
            xCurr = 0; 
            yCurr--;
        }
        else xCurr = 0, yCurr = 0;
    }
    videoMem = (dchar *)(VGADisplayInfo::videoMemAddr) 
                + yCurr * VGADisplayInfo::width + xCurr;

    return *this;
};

VGADisplay& VGADisplay::operator<<(const char *c)
{
    for(int i = 0; c[i] != '\0'; i++)
    {
        (*this) << c[i];
    }

    return *this;
};

void VGADisplay::doShift()
{
    for(uint16 i = 0; i < VGADisplayInfo::height; i++)
    {
        for(uint16 j = 0; j < VGADisplayInfo::width; j++)
        {
            *((dchar *)(VGADisplayInfo::videoMemAddr) + (i - 1) * VGADisplayInfo::width + j) = *((dchar *)(VGADisplayInfo::videoMemAddr) + i * VGADisplayInfo::width + j);
            *((dchar *)(VGADisplayInfo::videoMemAddr) + i * VGADisplayInfo::width + j) = (*((dchar *)(VGADisplayInfo::videoMemAddr) + i * VGADisplayInfo::width + j) & 0x00) | ' ';
        }
    }
};

void VGADisplay::setShift(bool status)
{
    shift = status;
};

void VGADisplay::setColor(char colorAttr)
{
    color = colorAttr;
}; 

void VGADisplay::setBlink(bool blinkAttr)
{
    isBlink = blinkAttr;
};

void VGADisplay::setBright(bool brigthAttr)
{
    isBright = brigthAttr;
}
dchar VGADisplay::getChar()
{
    return dchar();
};

void VGADisplay::goNewLine()
{
    uint16 yCurrOld = yCurr;

    while(yCurrOld == yCurr) (*this)++;
};

void VGADisplay::goHorizontalTab()
{
    while(xCurr % 8 != 0) (*this)++;
};

bool VGADisplay::printChar(const char &c)
{
    switch(c)
    {
        case '\0': return true;
        case '\n': this->goNewLine(); return true;
        case '\t': this->goHorizontalTab(); return true;
        default:
                   {
                        char attr = isBlink << 8 | isBright << 4
                                                 | color;
                        *videoMem = c | attr << 8; 
                        (*this)++;
                        return false;
                   }
    }
};
