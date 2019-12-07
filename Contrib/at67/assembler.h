#ifndef ASSEMBLER_H
#define ASSEMBLER_H


#include <stdint.h>
#include <string>
#include <map>


#define OPCODE_LD  0x00
#define OPCODE_AND 0x20
#define OPCODE_OR  0x40
#define OPCODE_XOR 0x60
#define OPCODE_ADD 0x80
#define OPCODE_SUB 0xA0
#define OPCODE_ST  0xC0
#define OPCODE_J   0xE0

#define EA_0D_AC    0x00
#define EA_0X_AC    0x04
#define EA_YD_AC    0x08
#define EA_YX_AC    0x0C
#define EA_0D_X     0x10
#define EA_0D_Y     0x14
#define EA_0D_OUT   0x18
#define EA_YX_OUTIX 0x1C

#define BUS_D   0x00
#define BUS_RAM 0x01
#define BUS_AC  0x02
#define BUS_IN  0x03

#define BRA_CC_FAR    0x00
#define BRA_CC_GT     0x04
#define BRA_CC_LT     0x08
#define BRA_CC_NE     0x0C
#define BRA_CC_EQ     0x10
#define BRA_CC_GE     0x14
#define BRA_CC_LE     0x18
#define BRA_CC_ALWAYS 0x1C

#define DEFAULT_START_ADDRESS  0x0200
#define DEFAULT_CALL_TABLE     0x0000

#define USER_ROM_ADDRESS  0x0B00 // pictures in ROM v1

#define VCPU_BRANCH_OPCODE 0x35


namespace Assembler
{
    enum ByteSize {BadSize=-1, OneByte=1, TwoBytes=2, ThreeBytes=3};
    enum OpcodeType {ReservedDB=0, ReservedDW, ReservedDBR, ReservedDWR, vCpu, Native};

    struct ByteCode
    {
        bool _isRomAddress;
        bool _isCustomAddress;
        uint8_t _data;
        uint16_t _address;
    };

    struct DasmCode
    {
        uint8_t _instruction;
        uint8_t _byteSize;
        uint8_t _data0;
        uint8_t _data1;
        uint16_t _address;
        std::string _text;
    };

    struct InstructionType
    {
        uint8_t _opcode;
        uint8_t _branch;
        ByteSize _byteSize;
        OpcodeType _opcodeType;
    };

    struct InstructionDasm
    {
        uint8_t _opcode;
        uint8_t _branch;
        ByteSize _byteSize;
        OpcodeType _opcodeType;
        std::string _mnemonic;
    };

    struct LineToken
    {
        bool _fromInclude = false;
        int _includeLineNumber;
        std::string _text;
        std::string _includeName;
    };


    bool getUseOpcodeCALLI(void);
    const std::string& getIncludePath(void);
    uint16_t getStartAddress(void);
    int getCurrDasmByteCount(void);
    int getPrevDasmByteCount(void);
    int getPrevDasmPageByteCount(void);
    int getCurrDasmPageByteCount(void);
    int getDisassembledCodeSize(void);
    DasmCode* getDisassembledCode(int index);

    void setUseOpcodeCALLI(bool useOpcodeCALLI);
    void setIncludePath(const std::string& includePath);

    int getAsmOpcodeSize(const std::string& opcodeStr);
    int getAsmOpcodeSizeText(const std::string& textStr);
    int getAsmOpcodeSizeFile(const std::string& filename);

    void initialise(void);
    void clearAssembler(void);
    bool getNextAssembledByte(ByteCode& byteCode, bool debug=false);

    bool assemble(const std::string& filename, uint16_t startAddress=DEFAULT_START_ADDRESS);
    int disassemble(uint16_t address);

#ifndef STAND_ALONE
    void printGprintfStrings(void);
#endif
}

#endif