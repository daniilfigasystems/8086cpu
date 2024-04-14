#ifndef _RAM_
#define _RAM_

typedef struct RAM
{
    short size;
    char mem[0x10000];
    char locked = 0;
} RAM;

void RAM_store8(RAM *ram, int addr, char val)
{
    ram->mem[addr] = val & 0xff;
}

void RAM_store16(RAM *ram, int addr, short val)
{
    ram->mem[addr] = val & 0xff;
    ram->mem[addr + 1] = val >> 8 & 0xff;
}

int RAM_load8(RAM *ram, int addr)
{
    return ram->mem[addr] & 0xff;
};

int RAM_load16(RAM *ram, int addr)
{
    return ram->mem[addr] & 0xff | 
    ram->mem[addr + 1] << 8 & 0xff;
}

#endif