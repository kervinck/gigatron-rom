#ifndef OPCODES_H
#define OPCODES_H


#include <string>
#include <map>

#define VCPU_BRANCH_OPCODE 0x35

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


namespace Assembler
{
    enum ByteSize {BadSize=-1, OneByte=1, TwoBytes=2, ThreeBytes=3};
    enum OpcodeType {ReservedDB=0, ReservedDW, ReservedDBR, ReservedDWR, vCpu, Native};

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

    std::map<std::string, InstructionType> _asmOpcodes;
    std::map<uint8_t, InstructionDasm> _vcpuOpcodes;
    std::map<uint8_t, InstructionDasm> _nativeOpcodes;


    void initialiseOpcodes(void)
    {
        // Gigatron vCPU instructions
        _asmOpcodes["ST"]    = {0x5E, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["STW"]   = {0x2B, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["STLW"]  = {0xEC, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LD"]    = {0x1A, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDI"]   = {0x59, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDWI"]  = {0x11, 0x00, ThreeBytes, vCpu};
        _asmOpcodes["LDW"]   = {0x21, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDLW"]  = {0xEE, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ADDW"]  = {0x99, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SUBW"]  = {0xB8, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ADDI"]  = {0xE3, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SUBI"]  = {0xE6, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LSLW"]  = {0xE9, 0x00, OneByte,    vCpu};
        _asmOpcodes["INC"]   = {0x93, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ANDI"]  = {0x82, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ANDW"]  = {0xF8, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ORI"]   = {0x88, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ORW"]   = {0xFA, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["XORI"]  = {0x8C, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["XORW"]  = {0xFC, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["PEEK"]  = {0xAD, 0x00, OneByte,    vCpu};
        _asmOpcodes["DEEK"]  = {0xF6, 0x00, OneByte,    vCpu};
        _asmOpcodes["POKE"]  = {0xF0, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["DOKE"]  = {0xF3, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LUP"]   = {0x7F, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["BRA"]   = {0x90, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["CALL"]  = {0xCF, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["RET"]   = {0xFF, 0x00, OneByte,    vCpu};
        _asmOpcodes["PUSH"]  = {0x75, 0x00, OneByte,    vCpu};
        _asmOpcodes["POP"]   = {0x63, 0x00, OneByte,    vCpu};
        _asmOpcodes["ALLOC"] = {0xDF, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SYS"]   = {0xB4, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["DEF"]   = {0xCD, 0x00, TwoBytes,   vCpu};

        // Gigatron vCPU branch instructions
        _asmOpcodes["BEQ"] = {0x35, 0x3F, ThreeBytes, vCpu};
        _asmOpcodes["BNE"] = {0x35, 0x72, ThreeBytes, vCpu};
        _asmOpcodes["BLT"] = {0x35, 0x50, ThreeBytes, vCpu};
        _asmOpcodes["BGT"] = {0x35, 0x4D, ThreeBytes, vCpu};
        _asmOpcodes["BLE"] = {0x35, 0x56, ThreeBytes, vCpu};
        _asmOpcodes["BGE"] = {0x35, 0x53, ThreeBytes, vCpu};

        // Reserved assembler opcodes
        _asmOpcodes["DB"]  = {0x00, 0x00, TwoBytes,   ReservedDB };
        _asmOpcodes["DW"]  = {0x00, 0x00, ThreeBytes, ReservedDW };
        _asmOpcodes["DBR"] = {0x00, 0x00, TwoBytes,   ReservedDBR};
        _asmOpcodes["DWR"] = {0x00, 0x00, ThreeBytes, ReservedDWR};
                                                                           
        // Gigatron native instructions                                    
        _asmOpcodes[".LD"]   = {0x00, 0x00, TwoBytes, Native};
        _asmOpcodes[".NOP"]  = {0x02, 0x00, TwoBytes, Native};
        _asmOpcodes[".ANDA"] = {0x20, 0x00, TwoBytes, Native};
        _asmOpcodes[".ORA"]  = {0x40, 0x00, TwoBytes, Native};
        _asmOpcodes[".XORA"] = {0x60, 0x00, TwoBytes, Native};
        _asmOpcodes[".ADDA"] = {0x80, 0x00, TwoBytes, Native};
        _asmOpcodes[".SUBA"] = {0xA0, 0x00, TwoBytes, Native};
        _asmOpcodes[".ST"]   = {0xC0, 0x00, TwoBytes, Native};
        _asmOpcodes[".JMP"]  = {0xE0, 0x00, TwoBytes, Native};
        _asmOpcodes[".BGT"]  = {0xE4, 0x00, TwoBytes, Native};
        _asmOpcodes[".BLT"]  = {0xE8, 0x00, TwoBytes, Native};
        _asmOpcodes[".BNE"]  = {0xEC, 0x00, TwoBytes, Native};
        _asmOpcodes[".BEQ"]  = {0xF0, 0x00, TwoBytes, Native};
        _asmOpcodes[".BGE"]  = {0xF4, 0x00, TwoBytes, Native};
        _asmOpcodes[".BLE"]  = {0xF8, 0x00, TwoBytes, Native};
        _asmOpcodes[".BRA"]  = {0xFC, 0x00, TwoBytes, Native};


        // Gigatron vCPU instructions
        _vcpuOpcodes[0x5E] = {0x5E, 0x00, TwoBytes,   vCpu, "ST"   };
        _vcpuOpcodes[0x2B] = {0x2B, 0x00, TwoBytes,   vCpu, "STW"  };
        _vcpuOpcodes[0xEC] = {0xEC, 0x00, TwoBytes,   vCpu, "STLW" };
        _vcpuOpcodes[0x1A] = {0x1A, 0x00, TwoBytes,   vCpu, "LD"   };
        _vcpuOpcodes[0x59] = {0x59, 0x00, TwoBytes,   vCpu, "LDI"  };
        _vcpuOpcodes[0x11] = {0x11, 0x00, ThreeBytes, vCpu, "LDWI" };
        _vcpuOpcodes[0x21] = {0x21, 0x00, TwoBytes,   vCpu, "LDW"  };
        _vcpuOpcodes[0xEE] = {0xEE, 0x00, TwoBytes,   vCpu, "LDLW" };
        _vcpuOpcodes[0x99] = {0x99, 0x00, TwoBytes,   vCpu, "ADDW" };
        _vcpuOpcodes[0xB8] = {0xB8, 0x00, TwoBytes,   vCpu, "SUBW" };
        _vcpuOpcodes[0xE3] = {0xE3, 0x00, TwoBytes,   vCpu, "ADDI" };
        _vcpuOpcodes[0xE6] = {0xE6, 0x00, TwoBytes,   vCpu, "SUBI" };
        _vcpuOpcodes[0xE9] = {0xE9, 0x00, OneByte,    vCpu, "LSLW" };
        _vcpuOpcodes[0x93] = {0x93, 0x00, TwoBytes,   vCpu, "INC"  };
        _vcpuOpcodes[0x82] = {0x82, 0x00, TwoBytes,   vCpu, "ANDI" };
        _vcpuOpcodes[0xF8] = {0xF8, 0x00, TwoBytes,   vCpu, "ANDW" };
        _vcpuOpcodes[0x88] = {0x88, 0x00, TwoBytes,   vCpu, "ORI"  };
        _vcpuOpcodes[0xFA] = {0xFA, 0x00, TwoBytes,   vCpu, "ORW"  };
        _vcpuOpcodes[0x8C] = {0x8C, 0x00, TwoBytes,   vCpu, "XORI" };
        _vcpuOpcodes[0xFC] = {0xFC, 0x00, TwoBytes,   vCpu, "XORW" };
        _vcpuOpcodes[0xAD] = {0xAD, 0x00, OneByte,    vCpu, "PEEK" };
        _vcpuOpcodes[0xF6] = {0xF6, 0x00, OneByte,    vCpu, "DEEK" };
        _vcpuOpcodes[0xF0] = {0xF0, 0x00, TwoBytes,   vCpu, "POKE" };
        _vcpuOpcodes[0xF3] = {0xF3, 0x00, TwoBytes,   vCpu, "DOKE" };
        _vcpuOpcodes[0x7F] = {0x7F, 0x00, TwoBytes,   vCpu, "LUP"  };
        _vcpuOpcodes[0x90] = {0x90, 0x00, TwoBytes,   vCpu, "BRA"  };
        _vcpuOpcodes[0xCF] = {0xCF, 0x00, TwoBytes,   vCpu, "CALL" };
        _vcpuOpcodes[0xFF] = {0xFF, 0x00, OneByte,    vCpu, "RET"  };
        _vcpuOpcodes[0x75] = {0x75, 0x00, OneByte,    vCpu, "PUSH" };
        _vcpuOpcodes[0x63] = {0x63, 0x00, OneByte,    vCpu, "POP"  };
        _vcpuOpcodes[0xDF] = {0xDF, 0x00, TwoBytes,   vCpu, "ALLOC"};
        _vcpuOpcodes[0xB4] = {0xB4, 0x00, TwoBytes,   vCpu, "SYS"  };
        _vcpuOpcodes[0xCD] = {0xCD, 0x00, TwoBytes,   vCpu, "DEF"  };

        // Gigatron vCPU branch instructions, (this works because condition code is still unique compared to opcodes)
        _vcpuOpcodes[0x3F] = {VCPU_BRANCH_OPCODE, 0x3F, ThreeBytes, vCpu, "BEQ"};
        _vcpuOpcodes[0x72] = {VCPU_BRANCH_OPCODE, 0x72, ThreeBytes, vCpu, "BNE"};
        _vcpuOpcodes[0x50] = {VCPU_BRANCH_OPCODE, 0x50, ThreeBytes, vCpu, "BLT"};
        _vcpuOpcodes[0x4D] = {VCPU_BRANCH_OPCODE, 0x4D, ThreeBytes, vCpu, "BGT"};
        _vcpuOpcodes[0x56] = {VCPU_BRANCH_OPCODE, 0x56, ThreeBytes, vCpu, "BLE"};
        _vcpuOpcodes[0x53] = {VCPU_BRANCH_OPCODE, 0x53, ThreeBytes, vCpu, "BGE"};

        // Gigatron native instructions
        _nativeOpcodes[0x00] = {0x00, 0x00, TwoBytes, Native, ".LD"  };
        _nativeOpcodes[0x02] = {0x02, 0x00, TwoBytes, Native, ".NOP" };
        _nativeOpcodes[0x20] = {0x20, 0x00, TwoBytes, Native, ".ANDA"};
        _nativeOpcodes[0x40] = {0x40, 0x00, TwoBytes, Native, ".ORA" };
        _nativeOpcodes[0x60] = {0x60, 0x00, TwoBytes, Native, ".XORA"};
        _nativeOpcodes[0x80] = {0x80, 0x00, TwoBytes, Native, ".ADDA"};
        _nativeOpcodes[0xA0] = {0xA0, 0x00, TwoBytes, Native, ".SUBA"};
        _nativeOpcodes[0xC0] = {0xC0, 0x00, TwoBytes, Native, ".ST"  };
        _nativeOpcodes[0xE0] = {0xE0, 0x00, TwoBytes, Native, ".JMP" };
        _nativeOpcodes[0xE4] = {0xE4, 0x00, TwoBytes, Native, ".BGT" };
        _nativeOpcodes[0xE8] = {0xE8, 0x00, TwoBytes, Native, ".BLT" };
        _nativeOpcodes[0xEC] = {0xEC, 0x00, TwoBytes, Native, ".BNE" };
        _nativeOpcodes[0xF0] = {0xF0, 0x00, TwoBytes, Native, ".BEQ" };
        _nativeOpcodes[0xF4] = {0xF4, 0x00, TwoBytes, Native, ".BGE" };
        _nativeOpcodes[0xF8] = {0xF8, 0x00, TwoBytes, Native, ".BLE" };
        _nativeOpcodes[0xFC] = {0xFC, 0x00, TwoBytes, Native, ".BRA" };
    }
}

#endif