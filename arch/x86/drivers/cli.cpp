#include "cli.h"
void cli::init_cli()
{
    uint32 bufferlen = 5;
    char buffer[bufferlen +1];
    for (uint16 i = 0; i < bufferlen; i++)
    {
        buffer[i] = display.getChar();
    }
    display << buffer;
}