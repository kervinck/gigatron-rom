#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>


#define DEFAULT_START_ADDRESS 0x0200


namespace Assembler
{
    uint16_t getStartAddress(void);

    bool getNextAssembledByte(uint8_t& data);
    bool assemble(const std::string& filename, uint16_t startAddress=DEFAULT_START_ADDRESS);
}

#endif