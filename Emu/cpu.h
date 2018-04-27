#ifndef CPU_H
#define CPU_H

#include <stdint.h>


#define MAJOR_VERSION "0.2"
#define MINOR_VERSION "8"
#define VERSION_STR "Ver: " MAJOR_VERSION "." MINOR_VERSION

#define ROM_SIZE (1<<16)
#define RAM_SIZE (1<<16) // Can be 32k or 64k


namespace Cpu
{
    struct State
    {
        uint16_t _PC;
        uint8_t _IR, _D, _AC, _X, _Y, _OUT, _undef;
    };

    uint8_t getIN(void);
    uint8_t getXOUT(void);
    uint8_t getROM(uint16_t address, int page);
    uint8_t getRAM(uint16_t address);

    void setIN(uint8_t in);
    void setXOUT(uint8_t xout);
    void setRAM(uint16_t address, uint8_t data);

    void initialise(State& S);
    State cycle(const State& S);
}

#endif