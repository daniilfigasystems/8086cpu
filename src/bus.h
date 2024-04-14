#ifndef _BUS_
#define _BUS_

#include "ram.h"

typedef struct BUS
{
    RAM ram;
} BUS;

void BUS_store(BUS *bus, int addr, short val, char size)
{
    switch(size)
    {
        case 8:
            RAM_store8(&bus->ram, addr, val);
        break;
        case 16:
            RAM_store16(&bus->ram, addr, val);
        break;
    }
}

short BUS_load(BUS *bus, int addr, char size)
{
    switch(size)
    {
        case 8:
            return RAM_load8(&bus->ram, addr);
        break;
        case 16:
            return RAM_load16(&bus->ram, addr);
        break;
    }
}

#endif