#ifndef _8086_EMU_
#define _8086_EMU_

#include "bus.h"

#define getaddr(seg, off) ((seg << 4) + off)
#define getflag(flag) ((flag & flag) > 0)

typedef struct CPU
{
    int ah = 0, al = 0;
    int bh = 0, bl = 0;
    int ch = 0, cl = 0;
    int dh = 0, dl = 0;

    short sp, bp, si, di;
    short cs, ds, es, ss, os;
    int ip;
    char flags;
    long long clocks = 0, cycles = 0;
    char interrupt = 0;
    int queue[6];
    int ea;
} CPU;

const int CF = 1;
const int PF = 1 << 2;
const int AF = 1 << 4;
const int ZF = 1 << 6;
const int SF = 1 << 7;
const int TF = 1 << 8;
const int IF = 1 << 9;
const int DF = 1 << 10;
const int OF = 1 << 11;

const int B = 0b0;
const int W = 0b1;
const int AX = 0b000;
const int CX = 0b001;
const int DX = 0b010;
const int BX = 0b011;

int BITS[2] = {8, 16};
int SIGN[2] = {0x80, 0x8000};

const int MASK[2] = {0xff, 0xffff};

const int PARITY[] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1,
    0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
    0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1,
    0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };

void reset(CPU *cpu)
{
    cpu->ip = 0x0000;
    cpu->flags = 0;
    cpu->cs = 0xffff;
    cpu->interrupt = 0;
}

short fetch(CPU *cpu, BUS *bus)
{
    return BUS_load(bus, cpu->ip, 16);
}

int step(CPU *cpu, BUS *bus)
{
    int instr = fetch(cpu, bus);

}

int getea(CPU *cpu, int mod, int rm)
{
    int disp = 0;
    if (mod == 0b01)
    {
        cpu->clocks += 4;
        disp = cpu->queue[2];
    }
    else if (mod == 0b10)
    {
        cpu->clocks += 4;
        disp = cpu->queue[3] << 8 | cpu->queue[2];
    }
    int ea = 0;
    switch(rm)
    {
        case 0b000:
            cpu->clocks += 7;
            ea = cpu->bh << 8 | cpu->bl + cpu->si + disp;
        break;
        case 0b001:
            cpu->clocks += 8;
            ea = cpu->bh << 8 | cpu->bl + cpu->di + disp;
        break;
        case 0b010:
            cpu->clocks += 8;
            ea = cpu->bp + cpu->si + disp;
        break;
        case 0b011:
            cpu->clocks += 7;
            ea = cpu->bp + cpu->di + disp;
        break;
        case 0b100:
            cpu->clocks += 5;
            ea = cpu->si + disp;
        break;
        case 0b101:
            cpu->clocks += 5;
            ea = cpu->di + disp;
        break;
        case 0b110:
            if (mod == 0b00)
            {
                cpu->clocks += 6;
                ea = cpu->queue[3] << 8 | cpu->queue[2];
            }
            else
            {
                cpu->clocks += 5;
                ea = cpu->bp + disp;
            }
        break;
        case 0b111:
            cpu->clocks += 5;
            ea = cpu->bh << 8 | cpu->bl + disp;
        break;
    }
    return ((cpu->os << 4) + (ea & 0xffff));
}

int getsegreg(CPU *cpu, int reg)
{
    switch(reg)
    {
        case 0b00:
            return cpu->es;
        break;
        case 0b01:
            return cpu->cs;
        break;
        case 0b10:
            return cpu->ss;
        break;
        case 0b11:
            return cpu->ds;
        break;
    }
}

void* setsegreg(CPU *cpu, int reg, int val)
{
    switch(reg)
    {
        case 0b00:
            cpu->es = val & 0xffff;
        break;
        case 0b01:
            cpu->cs = val & 0xffff;
        break;
        case 0b10:
            cpu->ss = val & 0xffff;
        break;
        case 0b11:
            cpu->ds = val & 0xffff;
        break;
    }
}

int getreg(CPU *cpu, int w, int reg)
{
    if (w == B)
    {
        switch(reg)
        {
            case 0b000:
                return cpu->al;
            break;
            case 0b001:
                return cpu->cl;
            break;
            case 0b010:
                return cpu->dl;
            break;
            case 0b011:
                return cpu->bl;
            break;
            case 0b100:
                return cpu->ah;
            break;
            case 0b101:
                return cpu->ch;
            break;
            case 0b110:
                return cpu->dh;
            break;
            case 0b111:
                return cpu->bh;
            break;
        }
    }
    else
    {
        switch(reg)
        {
            case 0b000:
                return cpu->ah << 8 | cpu->al;
            break;
            case 0b001:
                return cpu->ch << 8 | cpu->cl;
            break;
            case 0b010:
                return cpu->dh << 8 | cpu->dl;
            break;
            case 0b011:
                return cpu->bh << 8 | cpu->bl;
            break;
            case 0b100:
                return cpu->sp;
            break;
            case 0b101:
                return cpu->bp;
            break;
            case 0b110:
                return cpu->si;
            break;
            case 0b111:
                return cpu->di;
            break;
        }
    }
    return 0;
}

void *setreg(CPU *cpu, int w, int reg, int val)
{
    if (w == B)
    {
        switch(reg)
        {
            case 0b000:
                cpu->al = val & 0xff;
            break;
            case 0b001:
                cpu->cl = val & 0xff;
            break;
            case 0b010:
                cpu->dl = val & 0xff;
            break;
            case 0b011:
                cpu->bl = val & 0xff;
            break;
            case 0b100:
                cpu->ah = val & 0xff;
            break;
            case 0b101:
                cpu->ch = val & 0xff;
            break;
            case 0b110:
                cpu->dh = val & 0xff;
            break;
            case 0b111:
                cpu->bh = val & 0xff;
            break;
        }
    }
    else
    {
        switch(reg)
        {
            case 0b000:
                cpu->ah = val >> 8 & 0xff;
                cpu->al = val & 0xff;
            break;
            case 0b001:
                cpu->ch = val >> 8 & 0xff;
                cpu->cl = val & 0xff;
            break;
            case 0b010:
                cpu->dh = val >> 8 & 0xff;
                cpu->dl = val & 0xff;
            break;
            case 0b011:
                cpu->bh = val >> 8 & 0xff;
                cpu->bl = val & 0xff;
            break;
            case 0b100:
                cpu->sp = val & 0xffff;
            break;
            case 0b101:
                cpu->bp = val & 0xffff;
            break;
            case 0b110:
                cpu->si = val & 0xffff;
            break;
            case 0b111:
                cpu->di = val & 0xffff;
            break;
        }
    }
}

int getmem(CPU *cpu, BUS *bus, int w, int addr)
{
    if (addr == 0L)
        addr = getaddr(cpu->cs, cpu->ip);
    int val = bus->ram.mem[addr];
    if (w == W)
        val |= bus->ram.mem[addr + 1] << 8;
    cpu->ip = cpu->ip + 1 + w & 0xffff;
    return val;
}

void setmem(CPU *cpu, BUS *bus, int w, int addr, int val)
{
    if (addr >= 0xf6000)
        return;
    bus->ram.mem[addr] = val & 0xff;
    if (w == W)
    {
        if ((addr & 0b1) == 0b1)
            cpu->clocks += 4;
        bus->ram.mem[addr + 1] = val >> 8 & 0xff;
    }
}

int getrm(CPU *cpu, BUS *bus, int w, int mod, int rm)
{
    if (mod == 0b11)
        return getreg(cpu, w, rm);
    else
        return getmem(cpu, bus, w, cpu->ea > 0 ? cpu->ea : getea(cpu, mod, rm));
}

void *setrm(CPU *cpu, BUS *bus, int w, int mod, int rm, int val)
{
    if (mod == 0b11)
        setreg(cpu, w, rm, val);
    else
        setmem(cpu, bus, w, cpu->ea > 0 ? cpu->ea : getea(cpu, mod, rm), val);
}

#endif