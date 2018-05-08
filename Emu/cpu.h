#ifndef CPU_H
#define CPU_H

#include <stdint.h>


#define MAJOR_VERSION "0.3"
#define MINOR_VERSION "10"
#define VERSION_STR "Ver: " MAJOR_VERSION "." MINOR_VERSION

#define ROM_SIZE (1<<16)
#define RAM_SIZE (1<<16) // Can be 32k or 64k

#define BOOT_COUNT 0x0004
#define BOOT_CHECK 0x0005


namespace Cpu
{
    struct State
    {
        uint16_t _PC;
        uint8_t _IR, _D, _AC, _X, _Y, _OUT, _undef;
    };

    int64_t getClock(void);
    uint8_t getIN(void);
    uint8_t getXOUT(void);
    uint8_t getRAM(uint16_t address);
    uint8_t getROM(uint16_t address, int page);

    void setClock(int64_t clock);
    void setIN(uint8_t in);
    void setXOUT(uint8_t xout);
    void setRAM(uint16_t address, uint8_t data);
    void setROM(uint16_t base, uint16_t address, uint8_t data);

    void initialise(State& S);
    State cycle(const State& S);
    void reset(bool coldBoot=false);
}

#endif