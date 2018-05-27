#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>


#define DEFAULT_START_ADDRESS  0x0200
#define DEFAULT_CALL_TABLE     0x007E


namespace Assembler
{
    struct ByteCode
    {
        bool _isRomAddress;
        bool _isCustomAddress;
        uint8_t _data;
        uint16_t _address;
    };


    uint16_t getStartAddress(void);
    void printGprintfStrings(void);

    void initialise(void);
    bool getNextAssembledByte(ByteCode& byteCode);
    bool assemble(const std::string& filename, uint16_t startAddress=DEFAULT_START_ADDRESS);
}

#endif