#ifndef ASSEMBLER_H
#define ASSEMBLER_H


#include <stdint.h>
#include <string>
#include <map>


#define OPCODE_N_LD  0x00
#define OPCODE_N_AND 0x20
#define OPCODE_N_OR  0x40
#define OPCODE_N_XOR 0x60
#define OPCODE_N_ADD 0x80
#define OPCODE_N_SUB 0xA0
#define OPCODE_N_ST  0xC0
#define OPCODE_N_J   0xE0

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

#define DEFAULT_EXEC_ADDRESS 0x0200
#define DEFAULT_CALL_TABLE   0x0000

#define USER_ROMv1_ADDRESS 0x0B00 // pictures in ROM v1

#define OPCODE_V_BRA   0x90
#define OPCODE_V_BCC   0x35
#define OPCODE_V_CALL  0xCF
#define OPCODE_V_HALT  0xB4
#define OPERAND_V_HALT 0x80

#define GIGA_V_AC 0x18


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
        uint8_t _opcode0;
        uint8_t _opcode1;
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

    struct Gprintf
    {
        enum Format {Chr, Int, Bin, Oct, Hex, Str};
        struct Var
        {
            int _indirection = 0;
            Format _format;
            int _width;
            uint16_t _data;
            std::string _text;
        };

        uint16_t _address;
        int _lineNumber;
        std::string _lineToken;
        std::string _format;
        std::vector<Var> _vars;
        std::vector<std::string> _subs;
    };

    struct Define
    {
        bool _enabled = false;
        bool _toggle = false;
        int16_t _value = 0;
        std::string _name;
    };


    uint8_t getvSpMin(void);
    uint16_t getStartAddress(void);
    int getCurrDasmByteCount(void);
    int getPrevDasmByteCount(void);
    int getPrevDasmPageByteCount(void);
    int getCurrDasmPageByteCount(void);
    int getDisassembledCodeSize(void);
    DasmCode* getDisassembledCode(int index);
    const std::string& getIncludePath(void);

    void setvSpMin(uint8_t vSpMin);
    void setIncludePath(const std::string& includePath);

    int getAsmOpcodeSize(const std::string& opcodeStr);
    int getAsmOpcodeSizeText(const std::string& textStr);
    int getAsmOpcodeSizeFile(const std::string& filename);

    void clearDefines(void);
    bool createDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex);
    bool handleIfDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex);
    bool handleEndIfDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex);
    bool handleElseDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex);
    bool isCurrentDefineDisabled(void);
    int16_t getRuntimeVersion(void);

    void initialise(void);
    void clearAssembler(bool dontClearGprintfs=false);
    bool getNextAssembledByte(ByteCode& byteCode, bool debug=false);

    bool assemble(const std::string& filename, uint16_t startAddress=DEFAULT_EXEC_ADDRESS, bool dontClearGprintfs=false);
    int disassemble(uint16_t address);

#ifndef STAND_ALONE
    bool addGprintf(const Gprintf& gprintf, uint16_t address);
    bool parseGprintfFormat(const std::string& fmtstr, const std::vector<std::string>& variables, std::vector<Gprintf::Var>& vars, std::vector<std::string>& subs);
    bool getGprintfString(const Gprintf& gprintf, std::string& gstring);
    void handleGprintfs(void);
#endif
}

#endif