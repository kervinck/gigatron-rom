#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <cstdarg>

#include "memory.h"
#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "assembler.h"
#include "expression.h"

#ifndef STAND_ALONE
#include "cpu.h"
#include "editor.h"
#endif


#define BRANCH_ADJUSTMENT 2
#define MAX_DASM_LINES    30


namespace Assembler
{
    enum ParseType {PreProcessPass=0, MnemonicPass, CodePass, NumParseTypes};
    enum EvaluateResult {Failed=-1, NotFound, Reserved, Duplicate, Skipped, Success};
    enum AddressMode {D_AC=0b00000000, X_AC=0b00000100, YD_AC=0b00001000, YX_AC=0b00001100, D_X=0b00010000, D_Y=0b00010100, D_OUT=0b00011000, YXpp_OUT=0b00011100};
    enum BusMode {D=0b00000000, RAM=0b00000001, AC=0b00000010, IN=0b00000011};
    enum ReservedWords {CallTable=0, StartAddress, SingleStepWatch, DisableUpload, CpuUsageAddressA, CpuUsageAddressB, INCLUDE, MACRO, ENDM, GPRINTF, NumReservedWords};


    struct Label
    {
        uint16_t _address;
        std::string _name;
    };

    struct Equate
    {
        bool _isCustomAddress;
        uint16_t _operand;
        std::string _name;
    };

    struct Instruction
    {
        bool _isRomAddress;
        bool _isCustomAddress;
        ByteSize _byteSize;
        uint8_t _opcode;
        uint8_t _operand0;
        uint8_t _operand1;
        uint16_t _address;
        OpcodeType _opcodeType;
    };

    struct CallTableEntry
    {
        uint8_t _operand;
        uint16_t _address;
    };

    struct Macro
    {
        bool _complete = false;
        bool _fromInclude = false;
        int _fileStartLine;
        std::string _name;
        std::string _filename;
        std::vector<std::string> _params;
        std::vector<std::string> _lines;
    };


    int _lineNumber;

    uint8_t _vSpMin = 0x00;

    uint16_t _byteCount = 0;
    uint16_t _callTablePtr = 0x0000;
    uint16_t _startAddress = DEFAULT_EXEC_ADDRESS;
    uint16_t _currentAddress = _startAddress;
    uint16_t _currDasmByteCount = 1, _prevDasmByteCount = 1;
    uint16_t _currDasmPageByteCount = 0, _prevDasmPageByteCount = 0;

    std::string _includePath = ".";

    std::vector<Label> _labels;
    std::vector<Equate> _equates;
    std::vector<Instruction> _instructions;
    std::vector<ByteCode> _byteCode;
    std::vector<CallTableEntry> _callTableEntries;
    std::vector<std::string> _reservedWords;
    std::vector<DasmCode> _disassembledCode;

    std::map<std::string, InstructionType> _asmOpcodes;
    std::map<uint8_t, InstructionDasm> _vcpuOpcodes;
    std::map<uint8_t, InstructionDasm> _nativeOpcodes;
    std::map<uint16_t, Gprintf> _gprintfs;
    std::map<std::string, Define> _defines;
    std::stack<std::string> _currentDefine;


    uint8_t getvSpMin(void) {return _vSpMin;}
    uint16_t getStartAddress(void) {return _startAddress;}
    int getPrevDasmByteCount(void) {return _prevDasmByteCount;}
    int getCurrDasmByteCount(void) {return _currDasmByteCount;}
    int getPrevDasmPageByteCount(void) {return _prevDasmPageByteCount;}
    int getCurrDasmPageByteCount(void) {return _currDasmPageByteCount;}
    int getDisassembledCodeSize(void) {return int(_disassembledCode.size());}
    const std::string& getIncludePath(void) {return _includePath;}
    DasmCode* getDisassembledCode(int index) {return &_disassembledCode[index % _disassembledCode.size()];}

    void setvSpMin(uint8_t vSpMin) {_vSpMin = vSpMin;}
    void setIncludePath(const std::string& includePath) {_includePath = includePath;}


    int getAsmOpcodeSize(const std::string& opcodeStr)
    {
        if(opcodeStr[0] == ';') return 0;

        if(_asmOpcodes.find(opcodeStr) != _asmOpcodes.end())
        {
            return _asmOpcodes[opcodeStr]._byteSize;
        }

        return 0;
    }

    int getAsmOpcodeSizeText(const std::string& textStr)
    {
        for(auto it=_asmOpcodes.begin(); it!=_asmOpcodes.end(); ++it)
        {
            if(textStr.find(it->first) != std::string::npos  &&  _asmOpcodes.find(it->first) != _asmOpcodes.end())
            {
                return _asmOpcodes[it->first]._byteSize;
            }
        }

        return 0;
    }


    void initialiseOpcodes(void)
    {
        // Gigatron vCPU instructions
        _asmOpcodes["ST"   ] = {0x5E, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["STW"  ] = {0x2B, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["STLW" ] = {0xEC, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LD"   ] = {0x1A, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDI"  ] = {0x59, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDWI" ] = {0x11, 0x00, ThreeBytes, vCpu};
        _asmOpcodes["LDW"  ] = {0x21, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LDLW" ] = {0xEE, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ADDW" ] = {0x99, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SUBW" ] = {0xB8, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ADDI" ] = {0xE3, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SUBI" ] = {0xE6, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LSLW" ] = {0xE9, 0x00, OneByte,    vCpu};
        _asmOpcodes["INC"  ] = {0x93, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ANDI" ] = {0x82, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ANDW" ] = {0xF8, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ORI"  ] = {0x88, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["ORW"  ] = {0xFA, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["XORI" ] = {0x8C, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["XORW" ] = {0xFC, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["PEEK" ] = {0xAD, 0x00, OneByte,    vCpu};
        _asmOpcodes["DEEK" ] = {0xF6, 0x00, OneByte,    vCpu};
        _asmOpcodes["POKE" ] = {0xF0, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["DOKE" ] = {0xF3, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["LUP"  ] = {0x7F, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["BRA"  ] = {0x90, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["CALL" ] = {0xCF, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["RET"  ] = {0xFF, 0x00, OneByte,    vCpu};
        _asmOpcodes["PUSH" ] = {0x75, 0x00, OneByte,    vCpu};
        _asmOpcodes["POP"  ] = {0x63, 0x00, OneByte,    vCpu};
        _asmOpcodes["ALLOC"] = {0xDF, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["SYS"  ] = {0xB4, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["HALT" ] = {0xB4, 0x80, TwoBytes,   vCpu};
        _asmOpcodes["DEF"  ] = {0xCD, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["CALLI"] = {0x85, 0x00, ThreeBytes, vCpu};
        _asmOpcodes["CMPHS"] = {0x1F, 0x00, TwoBytes,   vCpu};
        _asmOpcodes["CMPHU"] = {0x97, 0x00, TwoBytes,   vCpu};

        // Gigatron vCPU branch instructions
        _asmOpcodes["BEQ"] = {0x35, 0x3F, ThreeBytes, vCpu};
        _asmOpcodes["BNE"] = {0x35, 0x72, ThreeBytes, vCpu};
        _asmOpcodes["BLT"] = {0x35, 0x50, ThreeBytes, vCpu};
        _asmOpcodes["BGT"] = {0x35, 0x4D, ThreeBytes, vCpu};
        _asmOpcodes["BLE"] = {0x35, 0x56, ThreeBytes, vCpu};
        _asmOpcodes["BGE"] = {0x35, 0x53, ThreeBytes, vCpu};

        // Reserved assembler opcodes
        _asmOpcodes["DB" ] = {0x00, 0x00, TwoBytes,   ReservedDB };
        _asmOpcodes["DW" ] = {0x00, 0x00, ThreeBytes, ReservedDW };
        _asmOpcodes["DBR"] = {0x00, 0x00, TwoBytes,   ReservedDBR};
        _asmOpcodes["DWR"] = {0x00, 0x00, ThreeBytes, ReservedDWR};
                                                                           
        // Gigatron native instructions                                    
        _asmOpcodes[".LD"  ] = {0x00, 0x00, TwoBytes, Native};
        _asmOpcodes[".NOP" ] = {0x02, 0x00, TwoBytes, Native};
        _asmOpcodes[".ANDA"] = {0x20, 0x00, TwoBytes, Native};
        _asmOpcodes[".ORA" ] = {0x40, 0x00, TwoBytes, Native};
        _asmOpcodes[".XORA"] = {0x60, 0x00, TwoBytes, Native};
        _asmOpcodes[".ADDA"] = {0x80, 0x00, TwoBytes, Native};
        _asmOpcodes[".SUBA"] = {0xA0, 0x00, TwoBytes, Native};
        _asmOpcodes[".ST"  ] = {0xC0, 0x00, TwoBytes, Native};
        _asmOpcodes[".JMP" ] = {0xE0, 0x00, TwoBytes, Native};
        _asmOpcodes[".BGT" ] = {0xE4, 0x00, TwoBytes, Native};
        _asmOpcodes[".BLT" ] = {0xE8, 0x00, TwoBytes, Native};
        _asmOpcodes[".BNE" ] = {0xEC, 0x00, TwoBytes, Native};
        _asmOpcodes[".BEQ" ] = {0xF0, 0x00, TwoBytes, Native};
        _asmOpcodes[".BGE" ] = {0xF4, 0x00, TwoBytes, Native};
        _asmOpcodes[".BLE" ] = {0xF8, 0x00, TwoBytes, Native};
        _asmOpcodes[".BRA" ] = {0xFC, 0x00, TwoBytes, Native};

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
        _vcpuOpcodes[0x85] = {0x85, 0x00, ThreeBytes, vCpu, "CALLI"};
        _vcpuOpcodes[0x1F] = {0x1F, 0x00, TwoBytes,   vCpu, "CMPHS"};
        _vcpuOpcodes[0x97] = {0x97, 0x00, TwoBytes,   vCpu, "CMPHU"};

        // Gigatron vCPU branch instructions, (this works because condition code is still unique compared to opcodes)
        _vcpuOpcodes[0x3F] = {OPCODE_V_BCC, 0x3F, ThreeBytes, vCpu, "BEQ"};
        _vcpuOpcodes[0x72] = {OPCODE_V_BCC, 0x72, ThreeBytes, vCpu, "BNE"};
        _vcpuOpcodes[0x50] = {OPCODE_V_BCC, 0x50, ThreeBytes, vCpu, "BLT"};
        _vcpuOpcodes[0x4D] = {OPCODE_V_BCC, 0x4D, ThreeBytes, vCpu, "BGT"};
        _vcpuOpcodes[0x56] = {OPCODE_V_BCC, 0x56, ThreeBytes, vCpu, "BLE"};
        _vcpuOpcodes[0x53] = {OPCODE_V_BCC, 0x53, ThreeBytes, vCpu, "BGE"};

        // Gigatron native instructions
        _nativeOpcodes[0x00] = {0x00, 0x00, TwoBytes, Native, "LD"  };
        _nativeOpcodes[0x02] = {0x02, 0x00, TwoBytes, Native, "NOP" };
        _nativeOpcodes[0x20] = {0x20, 0x00, TwoBytes, Native, "ANDA"};
        _nativeOpcodes[0x40] = {0x40, 0x00, TwoBytes, Native, "ORA" };
        _nativeOpcodes[0x60] = {0x60, 0x00, TwoBytes, Native, "XORA"};
        _nativeOpcodes[0x80] = {0x80, 0x00, TwoBytes, Native, "ADDA"};
        _nativeOpcodes[0xA0] = {0xA0, 0x00, TwoBytes, Native, "SUBA"};
        _nativeOpcodes[0xC0] = {0xC0, 0x00, TwoBytes, Native, "ST"  };
        _nativeOpcodes[0xE0] = {0xE0, 0x00, TwoBytes, Native, "JMP" };
        _nativeOpcodes[0xE4] = {0xE4, 0x00, TwoBytes, Native, "BGT" };
        _nativeOpcodes[0xE8] = {0xE8, 0x00, TwoBytes, Native, "BLT" };
        _nativeOpcodes[0xEC] = {0xEC, 0x00, TwoBytes, Native, "BNE" };
        _nativeOpcodes[0xF0] = {0xF0, 0x00, TwoBytes, Native, "BEQ" };
        _nativeOpcodes[0xF4] = {0xF4, 0x00, TwoBytes, Native, "BGE" };
        _nativeOpcodes[0xF8] = {0xF8, 0x00, TwoBytes, Native, "BLE" };
        _nativeOpcodes[0xFC] = {0xFC, 0x00, TwoBytes, Native, "BRA" };
    }

    void initialise(void)
    {
        _reservedWords.push_back("_CALLTABLE_");
        _reservedWords.push_back("_BREAKPOINT_");
        _reservedWords.push_back("_STARTADDRESS_");
        _reservedWords.push_back("_SINGLESTEPWATCH_");
        _reservedWords.push_back("_DISABLEUPLOAD_");
        _reservedWords.push_back("_CPUUSAGEADDRESSA_");
        _reservedWords.push_back("_CPUUSAGEADDRESSB_");
        _reservedWords.push_back("%INCLUDE");
        _reservedWords.push_back("%DEFINE");
        _reservedWords.push_back("%IF");
        _reservedWords.push_back("%ELSE");
        _reservedWords.push_back("%ELSEIF");
        _reservedWords.push_back("%ENDIF");
        _reservedWords.push_back("%MACRO");
        _reservedWords.push_back("%ENDM");
        _reservedWords.push_back("%SUB");
        _reservedWords.push_back("%ENDS");
        _reservedWords.push_back("GPRINTF");

        initialiseOpcodes();
    }

#ifndef STAND_ALONE
    void getDasmCurrAndPrevByteSize(uint16_t address, ByteSize byteSize)
    {
        // Save current and previous instruction lengths
        if(_disassembledCode.size() == 0)
        {
            _currDasmByteCount = uint16_t(byteSize);

            // Attempt to get bytesize of previous instruction
            for(uint16_t addr=address-1; addr>=address-3; --addr)
            {
                uint8_t size = uint8_t(address - addr); // no instruction is longer than 3 bytes
                uint8_t inst = Cpu::getRAM(addr);
                if(inst == OPCODE_V_BCC) inst = Cpu::getRAM(addr + 1);
                if(_vcpuOpcodes.find(inst) != _vcpuOpcodes.end()  &&  _vcpuOpcodes[inst]._opcodeType == vCpu  &&  _vcpuOpcodes[inst]._byteSize == size)
                {
                    _prevDasmByteCount = size;
                    break;
                }
            }
        }
    }

    void getDasmCurrAndPrevPageByteSize(int pageSize)
    {
        // Current page size
        _currDasmPageByteCount = 0;
        for(int i=0; i<pageSize; i++) _currDasmPageByteCount += _disassembledCode[i]._byteSize;

        // Previous page size
        _prevDasmPageByteCount = 0;
        uint16_t address = _disassembledCode[0]._address;
        for(int i=0; i<pageSize; i++)
        {
            // Get bytesize of previous page worth of instructions
            bool foundInstruction = false;
            for(uint16_t addr=address-1; addr>=address-3; --addr)
            {
                uint8_t size = uint8_t(address - addr); // no instruction is longer than 3 bytes
                uint8_t inst = Cpu::getRAM(addr);
                if(inst == OPCODE_V_BCC) inst = Cpu::getRAM(addr + 1);
                if(_vcpuOpcodes.find(inst) != _vcpuOpcodes.end()  &&  _vcpuOpcodes[inst]._opcodeType == vCpu  &&  _vcpuOpcodes[inst]._byteSize == size)
                {
                    foundInstruction = true;
                    _prevDasmPageByteCount += size;
                    address -= size;
                    break;
                }
            }

            if(!foundInstruction)
            {
                _prevDasmPageByteCount++;
                address--;
            }
        }
    }

    std::string removeBrackets(const char* str)
    {
        std::string string = str;
        Expression::stripChars(string, "[]");
        return string;
    }

    // Adapted from disassemble() in Core\asm.py
    bool getNativeMnemonic(uint8_t instruction, uint8_t data, char* mnemonic)
    {
        uint8_t inst, addr, bus;

        // Special case NOP
        if(instruction == 0x02  &&  data == 0x00)
        {
            strcpy(mnemonic, _nativeOpcodes[instruction]._mnemonic.c_str());
            return true;
        }

        inst = instruction & 0xE0;
        addr = instruction & 0x1C;
        bus  = instruction & 0x03;

        bool store = (inst == 0xC0);
        bool jump = (inst == 0xE0);

        if(_nativeOpcodes.find(inst) == _nativeOpcodes.end()) return false;

        // Instruction mnemonic, jump = 0xE0 + (condition codes)
        char instStr[8];
        (!jump) ? strcpy(instStr, _nativeOpcodes[inst]._mnemonic.c_str()) : strcpy(instStr, _nativeOpcodes[0xE0 + addr]._mnemonic.c_str());

        // Effective address string
        char addrStr[12];
        char regStr[4];
        if(!jump)
        {
            switch(addr)
            {
                case EA_0D_AC:    sprintf(addrStr, "[$%02x]",   data); sprintf(regStr, "AC");  break;
                case EA_0X_AC:    sprintf(addrStr, "[X]");             sprintf(regStr, "AC");  break;
                case EA_YD_AC:    sprintf(addrStr, "[Y,$%02x]", data); sprintf(regStr, "AC");  break;
                case EA_YX_AC:    sprintf(addrStr, "[Y,X]");           sprintf(regStr, "AC");  break;
                case EA_0D_X:     sprintf(addrStr, "[$%02x]",   data); sprintf(regStr, "X");   break;
                case EA_0D_Y:     sprintf(addrStr, "[$%02x]",   data); sprintf(regStr, "Y");   break;
                case EA_0D_OUT:   sprintf(addrStr, "[$%02x]",   data); sprintf(regStr, "OUT"); break;
                case EA_YX_OUTIX: sprintf(addrStr, "[Y,X++]");         sprintf(regStr, "OUT"); break;

                default: break;
            }
        }
        else
        {
            sprintf(addrStr, "[$%02x]", data);
        }

        // Bus string
        char busStr[8];
        switch(bus)
        {
            case BUS_D:   sprintf(busStr, "$%02x", data);                          break;
            case BUS_RAM: (!store) ? strcpy(busStr, addrStr) : strcpy(busStr, ""); break;
            case BUS_AC:  strcpy(busStr, "AC");                                    break;
            case BUS_IN:  strcpy(busStr, "IN");                                    break;

            default: break;
        }
        
        // Compose instruction string
        if(!jump)
        {
            if(store)
            {
                char storeStr[32];
                (bus == BUS_AC) ? sprintf(storeStr, "%-4s %s", instStr, addrStr) : sprintf(storeStr, "%-4s %s,%s", instStr, busStr, addrStr);
                if(bus == BUS_RAM) sprintf(storeStr, "CTRL %s", removeBrackets(addrStr).c_str());
                if(addr == EA_0D_X  ||  addr == EA_0D_Y) sprintf(mnemonic, "%s,%s", storeStr, regStr);
                else strcpy(mnemonic, storeStr);
            }
            else
            {
                // if reg == AC
                (addr <= EA_YX_AC) ? sprintf(mnemonic, "%-4s %s", instStr, busStr) : sprintf(mnemonic, "%-4s %s,%s", instStr, busStr, regStr);
            }
        }
        // Compose jump string
        else
        {
            char jumpStr[32];
            switch(addr)
            {
                case BRA_CC_FAR: sprintf(jumpStr, "%-4s Y,", instStr); break;
                default:         sprintf(jumpStr, "%-4s ",   instStr); break;
            }

            sprintf(mnemonic, "%-4s%s", jumpStr, busStr);
        }

        return true;
    }

    int disassemble(uint16_t address)
    {
        _disassembledCode.clear();

        _currDasmByteCount = 1;
        _prevDasmByteCount = 1;

        while(_disassembledCode.size() < MAX_DASM_LINES)
        {
            char dasmText[32];
            DasmCode dasmCode;
            ByteSize byteSize = OneByte;
            uint8_t instruction = 0, data0 = 0, data1 = 0;

            Editor::MemoryMode memoryMode = Editor::getMemoryMode();
            switch(memoryMode)
            {
                // Native instructions
                case Editor::ROM0: 
                case Editor::ROM1: 
                {
                    instruction = Cpu::getROM(address, 0);
                    data0 = Cpu::getROM(address, 1);
                    data1 = 0;

                    char mnemonic[24];
                    if(!getNativeMnemonic(instruction, data0, mnemonic))
                    {
                        sprintf(dasmText, "%04x  $%02x $%02x", address, instruction, data0);
                        dasmCode._address = address;
                        address++;
                        break;
                    }

                    sprintf(dasmText, "%04x  %s", address, mnemonic);
                    dasmCode._address = address;
                    address++;
                }
                break;

                // vCPU instructions
                case Editor::RAM:
                {
                    instruction = Cpu::getRAM(address);
                    data0 = Cpu::getRAM(address + 1);
                    data1 = Cpu::getRAM(address + 2);

                    // Invalid instruction or invalid address space
                    if((_vcpuOpcodes.find(instruction) == _vcpuOpcodes.end()  &&  instruction != OPCODE_V_BCC)  ||
                       (address >= GIGA_CH0_WAV_A  &&  address <= GIGA_CH0_OSC_H) ||  (address >= GIGA_CH1_WAV_A  &&  address <= GIGA_CH1_OSC_H) ||
                       (address >= GIGA_CH2_WAV_A  &&  address <= GIGA_CH2_OSC_H) ||  (address >= GIGA_CH3_WAV_A  &&  address <= GIGA_CH3_OSC_H))
                    {
                        sprintf(dasmText, "%04x  $%02x", address, instruction);
                        dasmCode._address = address;
                        address = (address + 1) & (Memory::getSizeRAM() - 1);
                        break;
                    }

                    // Branch instructions
                    bool foundBranch = false;
                    if(instruction == OPCODE_V_BCC)
                    {
                        instruction = data0;
                        if(_vcpuOpcodes.find(instruction) == _vcpuOpcodes.end())
                        {
                            sprintf(dasmText, "%04x  $%02x", address, instruction);
                            dasmCode._address = address;
                            address = (address + 1) & (Memory::getSizeRAM() - 1);
                            break;
                        }
                        foundBranch = true;
                    }

                    // Halt instruction
                    if(instruction == OPCODE_V_HALT  &&  data0 == OPERAND_V_HALT)
                    {
                        byteSize = TwoBytes;
                        sprintf(dasmText, "%04x  %-5s", address, "HALT");
                    }
                    // Valid instruction
                    else
                    {
                        byteSize = _vcpuOpcodes[instruction]._byteSize;
                        switch(byteSize)
                        {
                            case OneByte:  sprintf(dasmText, "%04x  %-5s", address, _vcpuOpcodes[instruction]._mnemonic.c_str());              break;
                            case TwoBytes: sprintf(dasmText, "%04x  %-5s $%02x", address, _vcpuOpcodes[instruction]._mnemonic.c_str(), data0); break;
                            case ThreeBytes: (foundBranch) ? sprintf(dasmText, "%04x  %-5s $%02x", address, _vcpuOpcodes[instruction]._mnemonic.c_str(), data1) : sprintf(dasmText, "%04x  %-5s $%02x%02x", address, _vcpuOpcodes[instruction]._mnemonic.c_str(), data1, data0); break;

                            default: break;
                        }
                    }
                    dasmCode._address = address;
                    address = uint16_t((address + byteSize) & (Memory::getSizeRAM() - 1));

                    // Save current and previous instruction sizes to allow scrolling
                    getDasmCurrAndPrevByteSize(dasmCode._address, byteSize);
                }
                break;

                default: break;
            }

            std::string dasmCodeText = std::string(dasmText);
            dasmCode._instruction = instruction;
            dasmCode._byteSize = uint8_t(byteSize);
            dasmCode._data0 = data0;
            dasmCode._data1 = data1;
            dasmCode._text = (memoryMode == Editor::RAM) ? Expression::strToUpper(dasmCodeText) : Expression::strToLower(dasmCodeText);

            _disassembledCode.push_back(dasmCode);
        }

        // Save current and previous page instruction sizes to allow page scrolling
        if(Editor::getMemoryMode() == Editor::RAM)
        {
            getDasmCurrAndPrevPageByteSize(MAX_DASM_LINES);
        }
        else
        {
            _currDasmPageByteCount = MAX_DASM_LINES;
            _prevDasmPageByteCount = MAX_DASM_LINES;
        }

        return int(_disassembledCode.size());
    }
#endif

    // Returns true when finished
    bool getNextAssembledByte(ByteCode& byteCode, bool debug)
    {
        static bool isUserCode = false;

        if(_byteCount >= _byteCode.size())
        {
            _byteCount = 0;
            if(debug  &&  isUserCode) fprintf(stderr, "\n");
            return true;
        }

        static uint16_t address = 0x0000;
        static uint16_t customAddress = 0x0000;

        // Get next byte
        if(_byteCount == 0) address = _startAddress;
        byteCode = _byteCode[_byteCount++];

        // New section
        if(byteCode._isCustomAddress)
        {
            address = byteCode._address;
            customAddress = byteCode._address;
        }

        // User code is RAM code or ROM code in user ROM space
        isUserCode = !byteCode._isRomAddress  ||  (byteCode._isRomAddress  &&  customAddress >= USER_ROMv1_ADDRESS);

        // Seperate sections
        if(debug  &&  byteCode._isCustomAddress  &&  isUserCode) fprintf(stderr, "\n");

        // 16bit for ROM, 8bit for RAM
        if(debug  &&  isUserCode)
        {
            if(byteCode._isRomAddress)
            {
                if((address & 0x0001) == 0x0000)
                {
                    fprintf(stderr, "Assembler::getNextAssembledByte() : ROM : %04X  %02X", customAddress + (LO_BYTE(address)>>1), byteCode._data);
                }
                else
                {
                    fprintf(stderr, "%02X\n", byteCode._data);
                }
            }
            else
            {
                fprintf(stderr, "Assembler::getNextAssembledByte() : RAM : %04X  %02X\n", address, byteCode._data);
            }
        }

        address++;

        return false;
    }    

    bool parseDefineOffset(const std::string& token, uint16_t& offset, size_t& lbra)
    {
        size_t rbra;
        if(Expression::findMatchingBrackets(token, 0, lbra, rbra))
        {
            Expression::Numeric value;
            if(Expression::parse(token.substr(lbra + 1, rbra - (lbra + 1)), _lineNumber, value))
            {
                offset = uint16_t(std::lround(value._value));
                return true;
            }
        }

        return false;
    }

    InstructionType getOpcode(const std::string& opcodeStr, uint16_t& offset)
    {
        offset = 0;
        std::string opcode = opcodeStr;
        Expression::strToUpper(opcode); 
        if(_asmOpcodes.find(opcode) != _asmOpcodes.end())
        {
            return _asmOpcodes[opcode];
        }

        size_t lbra;
        if(parseDefineOffset(opcode, offset, lbra))
        {
            if(_asmOpcodes.find(opcode.substr(0, lbra)) != _asmOpcodes.end())
            {
                return _asmOpcodes[opcode.substr(0, lbra)];
            }
        }

        return {0x00, 0x00, BadSize, vCpu};
    }

    void preProcessExpression(const std::vector<std::string>& tokens, int tokenIndex, std::string& input, bool stripWhiteSpace)
    {
        input.clear();

        // Pre-process
        for(int j=tokenIndex; j<int(tokens.size()); j++)
        {
            // Strip comments
            if(tokens[j].find_first_of(";#") != std::string::npos) break;

            // Concatenate
            input += tokens[j];
        }

        // Strip white space
        if(stripWhiteSpace) Expression::stripWhitespace(input);
    }

    size_t findSymbol(const std::string& input, const std::string& symbol, size_t pos=0)
    {
        const size_t len = input.length();
        if(pos >= len) return std::string::npos;

        const std::string separators = "+-*/().,!?;#'[]<> \t\n\r";
        const std::vector<std::string> operators = {"**", ">>", "<<", "==", "!=", "<=", ">="};

        for(;;)
        {
            size_t sep = input.find_first_of(separators, pos);
            bool eos = (sep == std::string::npos);
            if(eos)
            {
                for(int i=0; i<int(operators.size()); i++)
                {
                    sep = input.find(operators[i]);
                    eos = (sep == std::string::npos);
                    if(eos) break;
                }
            }

            size_t end = eos ? len : sep;
            if(input.substr(pos, end-pos) == symbol)
            {
                break;
            }
            else if(eos)
            {
                pos = std::string::npos;
                break;
            }

            pos = sep + 1;
        }

        return pos;
    }

    bool applyEquatesToExpression(std::string& expression, const std::vector<Equate>& equates)
    {
        bool modified = false;
        for(int i=0; i<int(equates.size()); i++)
        {
            for(;;)
            {
                size_t pos = findSymbol(expression, equates[i]._name);
                if(pos == std::string::npos) break;  // not found
                modified = true;
                expression.replace(pos, equates[i]._name.size(), std::to_string(equates[i]._operand));
            }
        }

        return modified;
    }

    bool applyLabelsToExpression(std::string& expression, const std::vector<Label>& labels, bool nativeCode)
    {
        bool modified = false;
        for(int i=0; i<int(labels.size()); i++)
        {
            for(;;)
            {
                size_t pos = findSymbol(expression, labels[i]._name);
                if (pos == std::string::npos) break;  // not found
                modified = true;
                uint16_t address = (nativeCode) ? labels[i]._address >>1 : labels[i]._address;
                expression.replace(pos, labels[i]._name.size(), std::to_string(address));
            }
        }

        return modified;
    }

    bool evaluateExpression(std::string input, bool nativeCode, int16_t& result)
    { 
        // Replace equates
        applyEquatesToExpression(input, _equates);

        // Replace labels
        applyLabelsToExpression(input, _labels, nativeCode);

        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

        // Parse expression and return with a result
        Expression::Numeric numeric;
        bool valid = Expression::parse(input, _lineNumber, numeric);
        result = int16_t(std::lround(numeric._value));
        return valid;
    }

    bool searchEquate(const std::string& token, Equate& equate)
    {
        bool success = false;
        for(int i=0; i<int(_equates.size()); i++)
        {
            if(_equates[i]._name == token)
            {
                equate = _equates[i];
                success = true;
                break;
            }
        }

        return success;
    }

    bool evaluateEquateOperand(const std::string& token, Equate& equate)
    {
        // Expression equates
        Expression::ExpressionType expressionType = Expression::isExpression(token);
        if(expressionType == Expression::IsInvalid) return false;
        if(expressionType == Expression::HasOperators)
        {
            int16_t value;
            if(!evaluateExpression(token, false, value)) return false;
            equate._operand = value;
            return true;
        }

        // Check for existing equate
        return searchEquate(token, equate);
    }

    bool evaluateEquateOperand(const std::vector<std::string>& tokens, int tokenIndex, Equate& equate, bool compoundInstruction)
    {
        if(tokenIndex >= int(tokens.size())) return false;

        // Expression equates
        std::string token;
        if(compoundInstruction)
        {
            token = tokens[tokenIndex];
        }
        else
        {
            preProcessExpression(tokens, tokenIndex, token, false);
        }

        return evaluateEquateOperand(token, equate);
    }

    EvaluateResult evaluateEquates(const std::vector<std::string>& tokens, ParseType parse)
    {
        std::string token0 = Expression::strUpper(tokens[0]);
        std::string token1 = Expression::strUpper(tokens[1]);
        if(token1 == "EQU")
        {
            if(parse == MnemonicPass)
            {
                Equate equate = {false, 0x0000, tokens[0]};
                if(!Expression::stringToU16(tokens[2], equate._operand))
                {
                    if(!evaluateEquateOperand(tokens, 2, equate, false)) return NotFound;
                }

                // Reserved word, (equate), _callTable_
                if(token0 == "_CALLTABLE_")
                {
                    _callTablePtr = equate._operand;
                }
                // Reserved word, (equate), _startAddress_
                else if(token0 == "_STARTADDRESS_")
                {
                    _startAddress = equate._operand;
                    _currentAddress = _startAddress;
                }
#ifndef STAND_ALONE
                // Disable upload of the current assembler module
                else if(token0 == "_DISABLEUPLOAD_")
                {
                    Loader::disableUploads(equate._operand != 0);
                }
                // Reserved word, (equate), _singleStepWatch_
                else if(token0 == "_SINGLESTEPWATCH_")
                {
                    Editor::setSingleStepAddress(equate._operand);
                }
                // Start address of vCPU exclusion zone
                else if(token0 == "_CPUUSAGEADDRESSA_")
                {
                    Editor::setCpuUsageAddressA(equate._operand);
                }
                // End address of vCPU exclusion zone
                else if(token0 == "_CPUUSAGEADDRESSB_")
                {
                    Editor::setCpuUsageAddressB(equate._operand);
                }
#endif
                // Standard equates
                else
                {
                    // Check for duplicate
                    equate._name = tokens[0];
                    if(searchEquate(tokens[0], equate)) return Duplicate;

                    _equates.push_back(equate);
                }
            }
            else if(parse == CodePass)
            {
            }

            return Success;
        }

        return Failed;
    }

    bool searchLabel(const std::string& token, Label& label)
    {
        bool success = false;
        for(int i=0; i<int(_labels.size()); i++)
        {
            if(token == _labels[i]._name)
            {
                success = true;
                label = _labels[i];
                break;
            }
        }

        return success;
    }

    bool evaluateLabelOperand(const std::string& token, Label& label)
    {
        // Expression labels
        Expression::ExpressionType expressionType = Expression::isExpression(token);
        if(expressionType == Expression::IsInvalid) return false;
        if(expressionType == Expression::HasOperators)
        {
            int16_t value;
            if(!evaluateExpression(token, false, value)) return false;
            label._address = value;
            return true;
        }

        // Check for existing label
        return searchLabel(token, label);
    }

    bool evaluateLabelOperand(const std::vector<std::string>& tokens, int tokenIndex, Label& label, bool compoundInstruction)
    {
        if(tokenIndex >= int(tokens.size())) return false;

        // Expression labels
        std::string token;
        if(compoundInstruction)
        {
            token = tokens[tokenIndex];
        }
        else
        {
            preProcessExpression(tokens, tokenIndex, token, false);
        }

        return evaluateLabelOperand(token, label);
    }

    EvaluateResult EvaluateLabels(const std::vector<std::string>& tokens, ParseType parse, int tokenIndex)
    {
        if(parse == MnemonicPass) 
        {
            // Check reserved words
            for(int i=0; i<int(_reservedWords.size()); i++)
            {
                if(tokens[tokenIndex] == _reservedWords[i]) return Reserved;
            }
            
            Label label;
            if(searchLabel(tokens[tokenIndex], label)) return Duplicate;

            // Check equates for a custom start address
            for(int i=0; i<int(_equates.size()); i++)
            {
                if(_equates[i]._name == tokens[tokenIndex])
                {
                    _equates[i]._isCustomAddress = true;
                    _currentAddress = _equates[i]._operand;
                    break;
                }
            }

            // Normal labels
            label = {_currentAddress, tokens[tokenIndex]};
            _labels.push_back(label);
        }
        else if(parse == CodePass)
        {
        }

        return Success;
    }

    bool handleDefineByte(std::vector<std::string>& tokens, int tokenIndex, const Instruction& instruction, bool createInstruction, uint16_t defineOffset, int& dbSize)
    {
        bool success = false;

        // Handle case where first operand is a string
        size_t quote1 = tokens[tokenIndex].find_first_of("'");
        size_t quote2 = tokens[tokenIndex].find_last_of("'");
        bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 1));
        if(quotes)
        {
            // Remove escape sequence
            std::string token = tokens[tokenIndex].substr(quote1+1, quote2 - (quote1+1));
            Expression::stripChars(token, "\\");
            if(createInstruction)
            {
                for(int j=1; j<int(token.size()); j++) // First instruction was created by callee
                {
                    Instruction inst = {instruction._isRomAddress, false, OneByte, uint8_t(token[j]), 0x00, 0x00, 0x0000, instruction._opcodeType};
                    _instructions.push_back(inst);
                }
            }
            dbSize += int(token.size()) - 1; // First instruction was created by callee
            success = true;
        }
       
        for(int i=tokenIndex+1; i<int(tokens.size()); i++)
        {
            // Handle all other variations of strings
            quote1 = tokens[i].find_first_of("'");
            quote2 = tokens[i].find_last_of("'");
            quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos);
            if(quotes)
            {
                // Remove escape sequence
                std::string token = tokens[i].substr(quote1+1, quote2 - (quote1+1));
                Expression::stripChars(token, "\\");
                if(createInstruction)
                {
                    for(int j=0; j<int(token.size()); j++)
                    {
                        Instruction inst = {instruction._isRomAddress, false, OneByte, uint8_t(token[j]), 0x00, 0x00, 0x0000, instruction._opcodeType};
                        _instructions.push_back(inst);
                    }
                }
                dbSize += int(token.size());
                success = true;
            }
            // Non string tokens
            else
            {
                // Strip comments
                if(tokens[i].find_first_of(";#") != std::string::npos) break;

                uint8_t operand;
                success = Expression::stringToU8(tokens[i], operand);
                if(!success)
                {
                    // Search equates
                    Equate equate;
                    Label label;
                    if((success = evaluateEquateOperand(tokens[i], equate)) == true)
                    {
                        operand = uint8_t(equate._operand);
                    }
                    // Search labels
                    else if((success = evaluateLabelOperand(tokens[i], label)) == true)
                    {
                        operand = uint8_t(label._address);
                    }
                    else
                    {
                        // Normal expression
                        if(Expression::isExpression(tokens[i]) == Expression::HasOperators)
                        {
                            Expression::Numeric value;
                            if(Expression::parse(tokens[i], _lineNumber, value))
                            {
                                operand = uint8_t(std::lround(value._value));
                                success = true;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if(createInstruction)
                {
                    bool hasDefineOffset = (defineOffset != 0);
                    uint16_t address = (hasDefineOffset) ? _currentAddress : 0x0000;
                    Instruction inst = {instruction._isRomAddress, hasDefineOffset, OneByte, operand, 0x00, 0x00, address, instruction._opcodeType};
                    _currentAddress += defineOffset;
                    _instructions.push_back(inst);
                }
                dbSize++;
            }
        }

        return success;
    }


    bool handleDefineWord(const std::vector<std::string>& tokens, int tokenIndex, const Instruction& instruction, bool createInstruction, uint16_t defineOffset, int& dwSize)
    {
        bool success = false;

        for(int i=tokenIndex+1; i<int(tokens.size()); i++)
        {
            // Strip comments
            if(tokens[i].find_first_of(";#") != std::string::npos)
            {
                success = true;
                break;
            }

            uint16_t operand;
            success = Expression::stringToU16(tokens[i], operand);
            if(!success)
            {
                // Search equates
                Equate equate;
                Label label;
                if((success = evaluateEquateOperand(tokens[i], equate)) == true)
                {
                    operand = equate._operand;
                }
                // Search labels
                else if((success = evaluateLabelOperand(tokens[i], label)) == true)
                {
                    operand = label._address;
                }
                else
                {
                    // Normal expression
                    if(Expression::isExpression(tokens[i]) == Expression::HasOperators)
                    {
                        Expression::Numeric value;
                        if(Expression::parse(tokens[i], _lineNumber, value))
                        {
                            operand = int16_t(std::lround(value._value));
                            success = true;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if(createInstruction)
            {
                bool hasDefineOffset = (defineOffset != 0);
                uint16_t address = (hasDefineOffset) ? _currentAddress : 0x0000;
                Instruction inst = {instruction._isRomAddress, hasDefineOffset, TwoBytes, uint8_t(LO_BYTE(operand)), uint8_t(HI_BYTE(operand)), 0x00, address, instruction._opcodeType};
                _currentAddress += defineOffset * 2;
                _instructions.push_back(inst);
            }
            dwSize += 2;
        }

        return success;
    }

    bool handleNativeOperand(const std::string& token, uint8_t& operand)
    {
        Expression::ExpressionType expressionType = Expression::isExpression(token);
        if(expressionType == Expression::IsInvalid) return false;
        if(expressionType == Expression::HasOperators)
        {
            // Parse expression and return with a result
            int16_t value;
            if(!evaluateExpression(token, true, value)) return false;
            operand = uint8_t(value);
            return true;
        }

        Label label;
        if(searchLabel(token, label))
        {
            operand = uint8_t(LO_BYTE(label._address >>1));
            return true;
        }

        Equate equate;
        if(searchEquate(token, equate))
        {
            operand = uint8_t(equate._operand);
            return true;
        }

        return Expression::stringToU8(token, operand);
    }

    bool handleNativeInstruction(const std::vector<std::string>& tokens, int tokenIndex, uint8_t& opcode, uint8_t& operand)
    {
        std::string input, token;

        preProcessExpression(tokens, tokenIndex, input, true);
        size_t openBracket = input.find_first_of("[");
        size_t closeBracket = input.find_first_of("]");
        bool noBrackets = (openBracket == std::string::npos  &&  closeBracket == std::string::npos);
        bool validBrackets = (openBracket != std::string::npos  &&  closeBracket != std::string::npos  &&  closeBracket > openBracket);

        size_t comma1 = input.find_first_of(",");
        size_t comma2 = input.find_first_of(",", comma1+1);
        bool noCommas = (comma1 == std::string::npos  &&  comma2 == std::string::npos);
        bool oneComma = (comma1 != std::string::npos  &&  comma2 == std::string::npos);
        bool twoCommas = (comma1 != std::string::npos  &&  comma2 != std::string::npos);

        operand = 0x00;

        // NOP
        if(opcode == 0x02) return true;

        // Accumulator
        if(input == "AC"  ||  input == "ac")
        {
            opcode |= BusMode::AC;
            return true;
        }

        // Jump
        if(opcode == 0xE0)
        {
            // y,[D]
            if(validBrackets  &&  oneComma  &&  comma1 < openBracket)
            {
                opcode |= BusMode::RAM;
                token = input.substr(openBracket+1, closeBracket - (openBracket+1));
                return handleNativeOperand(token, operand);
            }

            // y,D
            if(noBrackets  &&  oneComma)
            {
                token = input.substr(comma1+1, input.size() - (comma1+1));
                return handleNativeOperand(token, operand);
            }
        
            return false;                    
        }

        // Branch
        if(opcode >= 0xE4)
        {
            token = input;
            if(validBrackets) {opcode |= BusMode::RAM; token = input.substr(openBracket+1, closeBracket - (openBracket+1));}
            if(Expression::stringToU8(token, operand)) return true;
            return handleNativeOperand(token, operand);
        }

        // IN or IN,[D]
        if(input.find("IN") != std::string::npos  ||  input.find("in") != std::string::npos)
        {
            opcode |= BusMode::IN;

            // IN,[D]
            if(validBrackets &&  oneComma  &&  comma1 < openBracket)
            {
                token = input.substr(openBracket+1, closeBracket - (openBracket+1));
                return handleNativeOperand(token, operand);
            }
            
            // IN
            return true;
        }

        // D
        if(noBrackets && noCommas) return handleNativeOperand(input, operand);

        // Read or write
        (opcode != 0xC0) ? opcode |= BusMode::RAM : opcode |= BusMode::AC;

        // [D] or [X]
        if(validBrackets  &&  noCommas)
        {
            token = input.substr(openBracket+1, closeBracket - (openBracket+1));
            if(token == "X"  ||  token == "x") {opcode |= AddressMode::X_AC; return true;}
            return handleNativeOperand(token, operand);
        }

        // AC,X or AC,Y or AC,OUT or D,X or D,Y or D,OUT or [D],X or [D],Y or [D],OUT or D,[D] or D,[X] or D,[Y] or [Y,D] or [Y,X] or [Y,X++]
        if(oneComma)
        {
            token = input.substr(comma1+1, input.size() - (comma1+1));
            if(token == "X"    ||  token == "x")   opcode |= AddressMode::D_X;
            if(token == "Y"    ||  token == "y")   opcode |= AddressMode::D_Y;
            if(token == "OUT"  ||  token == "out") opcode |= AddressMode::D_OUT;

            token = input.substr(0, comma1);

            // AC,X or AC,Y or AC,OUT
            if(token == "AC"  ||  token == "ac") {opcode &= 0xFC; opcode |= BusMode::AC; return true;}

            // D,X or D,Y or D,OUT
            if(noBrackets)
            {
                opcode &= 0xFC; return handleNativeOperand(token, operand);
            }

            // [D],X or [D],Y or [D],OUT or D,[D] or D,[X] or D,[Y] or [Y,D] or [Y,X] or [Y,X++]
            if(validBrackets)
            {
                if(comma1 > closeBracket) token = input.substr(openBracket+1, closeBracket - (openBracket+1));
                else if(comma1 < openBracket) {opcode &= 0xFC; token = input.substr(0, comma1);}
                else if(comma1 > openBracket  &&  comma1 < closeBracket)
                {
                    token = input.substr(openBracket+1, comma1 - (openBracket+1));
                    if(token != "Y"  &&  token != "y") return false;

                    token = input.substr(comma1+1, closeBracket - (comma1+1));
                    if(token == "X"    ||  token == "x")   {opcode |= AddressMode::YX_AC;    return true;}
                    if(token == "X++"  ||  token == "x++") {opcode |= AddressMode::YXpp_OUT; return true;}

                    opcode |= AddressMode::YD_AC;                
                }
                return handleNativeOperand(token, operand);
            }

            return false;
        }

        // D,[Y,X] or D,[Y,X++]
        if(validBrackets  &&  twoCommas  &&  comma1 < openBracket  &&  comma2 > openBracket  &&  comma2 < closeBracket)
        {
            token = input.substr(0, comma1);
            if(!handleNativeOperand(token, operand)) return false;

            token = input.substr(openBracket+1, comma2 - (openBracket+1));
            if(token != "Y"  &&  token != "y") return false;
            opcode &= 0xFC; // reset bus bits to D

            token = input.substr(comma2+1, closeBracket - (comma2+1));
            if(token == "X"    ||  token == "x")   {opcode |= YX_AC;    return true;}
            if(token == "X++"  ||  token == "x++") {opcode |= YXpp_OUT; return true;}
                
            return false;
        }

        // [Y,X++],out
        if(validBrackets  &&  twoCommas  &&  comma1 > openBracket  &&  comma2 > closeBracket)
        {
            token = input.substr(openBracket+1, comma1 - (openBracket+1));
            if(token != "Y"  &&  token != "y") return false;

            token = input.substr(comma1+1, closeBracket - (comma1+1));
            if(token == "X"    ||  token == "x")   {opcode |= YX_AC;    return true;}
            if(token == "X++"  ||  token == "x++") {opcode |= YXpp_OUT; return true;}
                
            return false;
        }

        return false;
    }

    void packByteCode(Instruction& instruction, ByteCode& byteCode)
    {
        switch(instruction._byteSize)
        {
            case OneByte:
            {
                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = instruction._isCustomAddress;
                byteCode._data = instruction._opcode;
                byteCode._address = instruction._address;
                _byteCode.push_back(byteCode);
            }
            break;

            case TwoBytes:
            {
                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = instruction._isCustomAddress;
                byteCode._data = instruction._opcode;
                byteCode._address = instruction._address;
                _byteCode.push_back(byteCode);

                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = false;
                byteCode._data = instruction._operand0;
                byteCode._address = 0x0000;
                _byteCode.push_back(byteCode);
            }
            break;

            case ThreeBytes:
            {
                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = instruction._isCustomAddress;
                byteCode._data = instruction._opcode;
                byteCode._address = instruction._address;
                _byteCode.push_back(byteCode);

                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = false;
                byteCode._data = instruction._operand0;
                byteCode._address = 0x0000;
                _byteCode.push_back(byteCode);

                byteCode._isRomAddress = instruction._isRomAddress;
                byteCode._isCustomAddress = false;
                byteCode._data = instruction._operand1;
                byteCode._address = 0x0000;
                _byteCode.push_back(byteCode);
            }
            break;

            default: break;
        }
    }

    void packByteCodeBuffer(void)
    {
        // Pack instructions
        ByteCode byteCode;
        uint16_t segmentOffset = 0x0000;
        uint16_t segmentAddress = 0x0000;
        for(int i=0; i<int(_instructions.size()); i++)
        {
            // Segment RAM instructions into 256 byte pages for .gt1 file format
            if(!_instructions[i]._isRomAddress)
            {
                // Save start of segment
                if(_instructions[i]._isCustomAddress)
                {
                    segmentOffset = 0x0000;
                    segmentAddress = _instructions[i]._address;
                }

                // Force a new segment, (this could fail if an instruction straddles a page boundary, but
                // the page boundary crossing detection logic will stop the assembler before we get here)
                if(!_instructions[i]._isCustomAddress  &&  (HI_BYTE(segmentAddress + segmentOffset) != HI_BYTE(segmentAddress)))
                {
                    segmentAddress += segmentOffset;
                    segmentOffset = 0x0000;

                    _instructions[i]._isCustomAddress = true;
                    _instructions[i]._address = segmentAddress;
                }

                segmentOffset += uint16_t(_instructions[i]._byteSize);
            }

            packByteCode(_instructions[i], byteCode);
        }

        // Append call table
        if(_callTablePtr  &&  _callTableEntries.size())
        {
            // _callTable grows downwards, pointer is 2 bytes below the bottom of the table by the time we get here
            for(int i=int(_callTableEntries.size())-1; i>=0; i--)
            {
                byteCode._isRomAddress = false;
                byteCode._isCustomAddress = true;  // calltable entries can be non-sequential because of 0x80, (ONE_CONST_ADDRESS)
                byteCode._data = LO_BYTE(_callTableEntries[i]._address);
                byteCode._address = LO_BYTE(_callTableEntries[i]._operand);
                _byteCode.push_back(byteCode);

                byteCode._isRomAddress = false;
                byteCode._isCustomAddress = false;
                byteCode._data = HI_BYTE(_callTableEntries[i]._address);
                byteCode._address = LO_BYTE(_callTableEntries[i]._operand + 1);
                _byteCode.push_back(byteCode);
            }
        }
    }

    bool checkInvalidAddress(ParseType parse, uint16_t currentAddress, uint16_t instructionSize, const Instruction& instruction, const LineToken& lineToken, const std::string& filename, int lineNumber)
    {
        // Check for audio channel stomping
        if(parse == CodePass  &&  !instruction._isRomAddress)
        {
            uint16_t start = currentAddress;
            uint16_t end = currentAddress + instructionSize - 1;
            if((start >= GIGA_CH0_WAV_A  &&  start <= GIGA_CH0_OSC_H)  ||  (end >= GIGA_CH0_WAV_A  &&  end <= GIGA_CH0_OSC_H)  ||
               (start >= GIGA_CH1_WAV_A  &&  start <= GIGA_CH1_OSC_H)  ||  (end >= GIGA_CH1_WAV_A  &&  end <= GIGA_CH1_OSC_H)  ||
               (start >= GIGA_CH2_WAV_A  &&  start <= GIGA_CH2_OSC_H)  ||  (end >= GIGA_CH2_WAV_A  &&  end <= GIGA_CH2_OSC_H)  ||
               (start >= GIGA_CH3_WAV_A  &&  start <= GIGA_CH3_OSC_H)  ||  (end >= GIGA_CH3_WAV_A  &&  end <= GIGA_CH3_OSC_H))
            {
                fprintf(stderr, "Assembler::checkInvalidAddress() : Warning, audio channel boundary compromised, (if you've disabled the audio channels, then ignore this warning) ");
                fprintf(stderr, "Assembler::checkInvalidAddress() : '%s:%d' : 0x%04X <-> 0x%04X\nAssembler::checkInvalidAddress() : '%s'\nAssembler::checkInvalidAddress()\n",
                        filename.c_str(), lineNumber+1, start, end, lineToken._text.c_str());
            }
        }

        // Check for page boundary crossings
        if(parse == CodePass  &&  (instruction._opcodeType == vCpu || instruction._opcodeType == Native))
        {
            static uint16_t customAddress = 0x0000;
            if(instruction._isCustomAddress) customAddress = instruction._address;

            uint16_t oldAddress = (instruction._isRomAddress) ? customAddress + (LO_BYTE(currentAddress)>>1) : currentAddress;
            currentAddress += instructionSize - 1;
            uint16_t newAddress = (instruction._isRomAddress) ? customAddress + (LO_BYTE(currentAddress)>>1) : currentAddress;
            if((oldAddress >>8) != (newAddress >>8))
            {
                fprintf(stderr, "Assembler::checkInvalidAddress() : '%s:%d' : page boundary compromised : %04X : %04X : '%s'\n", filename.c_str(), lineNumber+1, oldAddress, newAddress, lineToken._text.c_str());
                return false;
            }
        }

        return true;
    }


    bool handleInclude(const std::vector<std::string>& tokens, const std::string& lineToken, int lineIndex, std::vector<LineToken>& includeLineTokens)
    {
        // Check include syntax
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Assembler::handleInclude() : '%s:%d' : bad %%include statement\n", lineToken.c_str(), lineIndex);
            return false;
        }

        std::string filepath = _includePath + "/" + tokens[1];
        std::replace(filepath.begin(), filepath.end(), '\\', '/');
        std::ifstream infile(filepath);
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::handleInclude() : failed to open file '%s'\n", filepath.c_str());
            return false;
        }

        // Collect lines from include file
        int lineNumber = lineIndex;
        while(!infile.eof())
        {
            LineToken includeLineToken = {true, lineNumber++ - lineIndex, "", filepath};
            std::getline(infile, includeLineToken._text);
            includeLineTokens.push_back(includeLineToken);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Assembler::handleInclude() : '%s:%d' : bad lineToken '%s'\n", filepath.c_str(), lineNumber - lineIndex, includeLineToken._text.c_str());
                return false;
            }
        }

        return true;
    }

    bool handleMacros(const std::vector<Macro>& macros, std::vector<LineToken>& lineTokens)
    {
        // Incomplete macros
        for(int i=0; i<int(macros.size()); i++)
        {
            if(!macros[i]._complete)
            {
                fprintf(stderr, "Assembler::handleMacros() : '%s:%d' : bad macro, missing 'ENDM'\n", macros[i]._filename.c_str(), macros[i]._fileStartLine);
                return false;
            }
        }

        // Delete original macros
        auto filter = [](LineToken& lineToken)
        {
            static bool foundMacro = false;
            if(lineToken._text.find("%MACRO") != std::string::npos)
            {
                foundMacro = true;
                return true;
            }
            if(foundMacro)
            {
                if(lineToken._text.find("%ENDM") != std::string::npos) foundMacro = false;
                return true;
            }

            return false;
        };
        lineTokens.erase(std::remove_if(lineTokens.begin(), lineTokens.end(), filter), lineTokens.end());

        // Find and expand macro
        int macroInstanceId = 0;
        for(int m=0; m<int(macros.size()); m++)
        {
            bool macroMissing = true;
            bool macroMissingParams = true;
            Macro macro = macros[m];

            for(auto itLine=lineTokens.begin(); itLine!=lineTokens.end();)
            {
                // Lines containing only white space are skipped
                LineToken lineToken = *itLine;
                size_t nonWhiteSpace = lineToken._text.find_first_not_of("  \n\r\f\t\v");
                if(nonWhiteSpace == std::string::npos)
                {
                    ++itLine;
                    continue;
                }

                // Tokenise current line
                std::vector<std::string> tokens = Expression::tokeniseLine(lineToken._text);

                // Find macro
                bool macroSuccess = false;
                for(int t=0; t<int(tokens.size()); t++)
                {
                    if(tokens[t] == macro._name)
                    {
                        macroMissing = false;
                        if(tokens.size() - t > macro._params.size())
                        {
                            macroMissingParams = false;
                            std::vector<std::string> labels;
                            std::vector<LineToken> macroLines;

                            // Create substitute lines
                            for(int ml=0; ml<int(macro._lines.size()); ml++)
                            {
                                // Tokenise macro line
                                std::vector<std::string> mtokens =  Expression::tokeniseLine(macro._lines[ml]);

                                // Save labels
                                nonWhiteSpace = macro._lines[ml].find_first_not_of("  \n\r\f\t\v");
                                if(nonWhiteSpace == 0) labels.push_back(mtokens[0]);

                                // Replace parameters
                                for(int mt=0; mt<int(mtokens.size()); mt++)
                                {
                                    for(int p=0; p<int(macro._params.size()); p++)
                                    {
                                        //if(mtokens[mt] == macro._params[p]) mtokens[mt] = tokens[t + 1 + p];
                                        size_t param = mtokens[mt].find(macro._params[p]);
                                        if(param != std::string::npos)
                                        {
                                            mtokens[mt].erase(param, macro._params[p].size());
                                            mtokens[mt].insert(param, tokens[t + 1 + p]);
                                        }
                                    }
                                }

                                // New macro line using any existing label
                                LineToken macroLine = {false, 0, "", ""};
                                macroLine._text = (t > 0  &&  ml == 0) ? tokens[0] : "";

                                // Append to macro line
                                for(int mt=0; mt<int(mtokens.size()); mt++)
                                {
                                    // Don't prefix macro labels with a space
                                    if(nonWhiteSpace != 0  ||  mt != 0) macroLine._text += " ";

                                    macroLine._text += mtokens[mt];
                                }

                                macroLines.push_back(macroLine);
                            }

                            // Insert substitute lines
                            for(int ml=0; ml<int(macro._lines.size()); ml++)
                            {
                                // Delete macro caller
                                if(ml == 0) itLine = lineTokens.erase(itLine);

                                // Each instance of a macro's labels are made unique
                                for(int i=0; i<int(labels.size()); i++)
                                {
                                    size_t labelFoundPos = macroLines[ml]._text.find(labels[i]);
                                    if(labelFoundPos != std::string::npos) macroLines[ml]._text.insert(labelFoundPos + labels[i].size(), std::to_string(macroInstanceId));
                                }

                                // Insert macro lines
                                itLine = lineTokens.insert(itLine, macroLines[ml]);
                                ++itLine;
                            }

                            macroInstanceId++;
                            macroSuccess = true;
                        }
                    }
                }

                if(!macroSuccess) ++itLine;
            }

            if(macroMissing)
            {
                //fprintf(stderr, "Assembler::handleMacros() : '%s:%d' : warning, macro '%s' is never called\n", macro._filename.c_str(), macro._fileStartLine, macro._name.c_str());
                continue;
            }

            if(macroMissingParams)
            {
                fprintf(stderr, "Assembler::handleMacros() : '%s:%d' : missing macro parameters for '%s'\n", macro._filename.c_str(), macro._fileStartLine, macro._name.c_str());
                return false;
            }
        }

        return true;
    }

    bool handleMacroStart(const std::string& filename, const LineToken& lineToken, const std::vector<std::string>& tokens, Macro& macro, int adjustedLineIndex)
    {
        int lineNumber = (lineToken._fromInclude) ? lineToken._includeLineNumber + 1 : adjustedLineIndex + 1;
        std::string macroFileName = (lineToken._fromInclude) ? lineToken._includeName : filename;

        // Check macro syntax
        if(tokens.size() < 2)
        {
            fprintf(stderr, "Assembler::handleMacroStart() : '%s:%d' : bad macro, missing name\n", macroFileName.c_str(), lineNumber);
            return false;
        }                    

        macro._name = tokens[1];
        macro._fromInclude = lineToken._fromInclude;
        macro._fileStartLine = lineNumber;
        macro._filename = macroFileName;

        // Save params
        for(int i=2; i<int(tokens.size()); i++) macro._params.push_back(tokens[i]);

        return true;
    }

    bool handleMacroEnd(std::vector<Macro>& macros, Macro& macro)
    {
        // Check for duplicates
        for(int i=0; i<int(macros.size()); i++)
        {
            if(macro._name == macros[i]._name)
            {
                fprintf(stderr, "Assembler::handleMacroEnd() : '%s:%d' : bad macro, duplicate name '%s'\n", macro._filename.c_str(), macro._fileStartLine, macro._name.c_str());
                return false;
            }
        }
        macro._complete = true;
        macros.push_back(macro);

        macro._name = "";
        macro._lines.clear();
        macro._params.clear();
        macro._complete = false;

        return true;
    }

    void clearDefines(void)
    {
        _defines.clear();
        while(!_currentDefine.empty()) _currentDefine.pop();
    }

    bool createDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex)
    {
        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Assembler::createDefine() : '%s:%d' : %%define requires one or two params\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        // Define name
        std::string defineName = tokens[1];
        if(_defines.find(defineName) != _defines.end())
        {
            fprintf(stderr, "Assembler::createDefine() : '%s:%d' : found duplicate define '%s'\n", filename.c_str(), adjustedLineIndex, defineName.c_str());
            return false;
        }

        // Define value
        int16_t defineValue = 0;
        if(tokens.size() == 3)
        {
            if(!evaluateExpression(tokens[2], false, defineValue))
            {
                fprintf(stderr, "Assembler::createDefine() : '%s:%d' : found invalid define value '%s'\n", filename.c_str(), adjustedLineIndex, tokens[2].c_str());
                return false;
            }
        }

        _defines[defineName] = {true, false, defineValue, defineName};

        return true;
    }

    bool handleIfDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex)
    {
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Assembler::handleIfDefine() : '%s:%d' : %%if requires one param\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        std::string define = tokens[1];
        if(_defines.find(define) == _defines.end())
        {
            _defines[define] = {false, false, 0, define};
        }

        // Push current define to stack
        _currentDefine.push(define);

        return true;
    }

    bool handleEndIfDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex)
    {
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Assembler::handleEndIfDefine() : '%s:%d' : %%endif requires no params\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        if(_currentDefine.empty())
        {
            fprintf(stderr, "Assembler::handleEndIfDefine() : '%s:%d' : syntax error, no valid define\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        // If enabled was toggled by an else then reset it
        std::string define = _currentDefine.top();
        if(_defines[define]._toggle == true)
        {
            _defines[define]._enabled = !_defines[define]._enabled;
            _defines[define]._toggle = false;
        }

        // Pop current define from stack
        _currentDefine.pop();

        return true;
    }

    bool handleElseDefine(const std::string& filename, const std::vector<std::string>& tokens, int adjustedLineIndex)
    {
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Assembler::handleElseDefine() : '%s:%d' : %%else requires no params\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        if(_currentDefine.empty())
        {
            fprintf(stderr, "Assembler::handleElseDefine() : '%s:%d' : syntax error, no valid define\n", filename.c_str(), adjustedLineIndex);
            return false;
        }

        // Toggle current define
        std::string define = _currentDefine.top();
        _defines[define]._enabled = !_defines[define]._enabled;
        _defines[define]._toggle = true;

        return true;
    }

    bool isCurrentDefineDisabled(void)
    {
        // Check top of define stack
        if(!_currentDefine.empty())
        {
            std::string define = _currentDefine.top();
            if(_defines.find(define) != _defines.end()  &&  !_defines[define]._enabled)
            {
                return true;
            }
        }

        return false;
    }

    int16_t getRuntimeVersion(void)
    {
        int16_t runtimeVersion = 0;
        if(_defines.find("RUNTIME_VERSION") != _defines.end())
        {
            runtimeVersion = _defines["RUNTIME_VERSION"]._value;
        }

        return runtimeVersion;
    }

    bool preProcess(const std::string& filename, std::vector<LineToken>& lineTokens, bool doMacros)
    {
        Macro macro;
        std::vector<Macro> macros;
        bool buildingMacro = false;

        int adjustedLineIndex = 0;
        for(auto itLine=lineTokens.begin(); itLine != lineTokens.end();)
        {
            // Lines containing only white space are skipped
            LineToken lineToken = *itLine;
            size_t nonWhiteSpace = lineToken._text.find_first_not_of("  \n\r\f\t\v");
            if(nonWhiteSpace == std::string::npos)
            {
                ++itLine;
                ++adjustedLineIndex;
                continue;
            }

            bool eraseLine = false;
            int lineIndex = int(itLine - lineTokens.begin()) + 1;

            // Strip comments
            size_t commentPos = lineToken._text.find_first_of(";#");
            if(commentPos != std::string::npos)
            {
                lineToken._text = lineToken._text.substr(0, commentPos);
            }

            // Tokenise current line
            std::vector<std::string> tokens = Expression::tokeniseLine(lineToken._text);

            // Valid pre-processor commands
            if(tokens.size() > 0)
            {
                Expression::strToUpper(tokens[0]);

                // Remove subroutine header and footer
                if(tokens[0] == "%SUB"  ||  tokens[0] == "%ENDS")
                {
                    itLine = lineTokens.erase(itLine);
                    continue;
                }

                // Include
                if(tokens[0] == "%INCLUDE")
                {
                    std::vector<LineToken> includeLineTokens;
                    if(!handleInclude(tokens, lineToken._text, lineIndex, includeLineTokens)) return false;

                    // Recursively include everything in order
                    if(!preProcess(filename, includeLineTokens, false))
                    {
                        fprintf(stderr, "Assembler::preProcess() : '%s:%d' : bad include file : '%s'\n", filename.c_str(), adjustedLineIndex, tokens[1].c_str());
                        return false;
                    }

                    // Remove original include line and replace with include text
                    itLine = lineTokens.erase(itLine);
                    itLine = lineTokens.insert(itLine, includeLineTokens.begin(), includeLineTokens.end());
                    ++adjustedLineIndex -= int(includeLineTokens.end() - includeLineTokens.begin());
                    eraseLine = true;
                }
                // Include path
                else if(tokens[0] == "%INCLUDEPATH"  &&  tokens.size() > 1)
                {
                    if(Expression::isStringValid(tokens[1]))
                    {
                        // Strip quotes
                        std::string includePath = tokens[1];
                        includePath.erase(0, 1);
                        includePath.erase(includePath.size()-1, 1);

                        // Prepend current file path to relative paths
                        if(includePath.find(":") == std::string::npos  &&  includePath[0] != '/')
                        {
                            std::string filepath = Loader::getFilePath();
                            size_t slash = filepath.find_last_of("\\/");
                            filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
                            includePath = filepath + "/" + includePath;
                        }

                        _includePath = includePath;
                        itLine = lineTokens.erase(itLine);
                        eraseLine = true;
                    }
                }
                // Define
                else if(tokens[0] == "%DEFINE")
                {
                    if(!createDefine(filename, tokens, 3)) return false;

                    itLine = lineTokens.erase(itLine);
                    eraseLine = true;
                }
                // If
                else if(tokens[0] == "%IF")
                {
                    if(!handleIfDefine(filename, tokens, adjustedLineIndex)) return false;

                    itLine = lineTokens.erase(itLine);
                    eraseLine = true;
                }
                // EndIf
                else if(tokens[0] == "%ENDIF")
                {
                    if(!handleEndIfDefine(filename, tokens, adjustedLineIndex)) return false;

                    itLine = lineTokens.erase(itLine);
                    eraseLine = true;
                }
                // Else
                else if(tokens[0] == "%ELSE")
                {
                    if(!handleElseDefine(filename, tokens, adjustedLineIndex)) return false;

                    itLine = lineTokens.erase(itLine);
                    eraseLine = true;
                }
                // Check if there is a current define and delete code if define is disabled
                else if(!_currentDefine.empty())
                {
                    std::string define = _currentDefine.top();
                    if(_defines.find(define) == _defines.end())
                    {
                        fprintf(stderr, "Assembler::preProcess() : '%s:%d' : define '%s' does not exist\n", filename.c_str(), adjustedLineIndex, define.c_str());
                        return false;
                    }

                    if(!_defines[define]._enabled)
                    {                    
                        itLine = lineTokens.erase(itLine);
                        eraseLine = true;
                    }
                }
                // Build macro
                else if(doMacros)
                {
                    if(tokens[0] == "%MACRO"  &&  !buildingMacro)
                    {
                        if(!handleMacroStart(filename, lineToken, tokens, macro, adjustedLineIndex)) return false;

                        buildingMacro = true;
                    }
                    else if(buildingMacro  &&  tokens[0] == "%ENDM")
                    {
                        if(!handleMacroEnd(macros, macro)) return false;
                        buildingMacro = false;
                    }
                    else if(buildingMacro)
                    {
                        macro._lines.push_back(lineToken._text);
                    }
                }
            }

            if(!eraseLine)
            {
                ++itLine;
                ++adjustedLineIndex;
            }
        }

        // Check matching %if %endif
        if(!_currentDefine.empty())
        {
            fprintf(stderr, "Assembler::preProcess() : '%s' : missing '%%endif'\n", filename.c_str());
            return false;
        }

        // Handle complete macros
        if(doMacros  &&  !handleMacros(macros, lineTokens)) return false;

        return true;
    }

    int getAsmOpcodeSize(const std::string& filename, std::vector<LineToken>& lineTokens)
    {
        // Pre-processor
        if(!preProcess(filename, lineTokens, true)) return -1;

        // Tokenise lines
        int vasmSize = 0;
        for(int i=0; i<int(lineTokens.size()); i++)
        {
            std::vector<std::string> tokens = Expression::tokeniseLine(lineTokens[i]._text);
            for(int j=0; j<int(tokens.size()); j++)
            {
                std::string token = Expression::strToUpper(tokens[j]);
                vasmSize += getAsmOpcodeSize(token);
            }
        }

        return vasmSize;
    }

    int getAsmOpcodeSizeFile(const std::string& filename)
    {
        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::getAsmOpcodeSizeFile() : failed to open file : '%s'\n", filename.c_str());
            return -1;
        }

        // Get file
        int numLines = 0;
        LineToken lineToken;
        std::vector<LineToken> lineTokens;
        while(!infile.eof())
        {
            std::getline(infile, lineToken._text);
            lineTokens.push_back(lineToken);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Assembler::getAsmOpcodeSizeFile() : '%s:%d' : bad lineToken '%s'\n", filename.c_str(), numLines+1, lineToken._text.c_str());
                return -1;
            }

            numLines++;
        }
        
        return getAsmOpcodeSize(filename, lineTokens);
    }

#ifndef STAND_ALONE
    bool handleBreakPoints(ParseType parse, const std::string& lineToken, int lineNumber)
    {
        UNREFERENCED_PARAM(lineNumber);

        std::string input = lineToken;
        Expression::strToUpper(input);

        if(input.find("_BREAKPOINT_") != std::string::npos)
        {
            if(parse == MnemonicPass) Editor::addVpcBreakPoint(_currentAddress);
            return true;
        }

        return false;
    }

    bool addGprintf(const Gprintf& gprintf, uint16_t address)
    {
        if(_gprintfs.find(address) != _gprintfs.end()) return false;

        _gprintfs[address] = gprintf;
        return true;
    }

    bool parseGprintfFormat(const std::string& fmtstr, const std::vector<std::string>& variables, std::vector<Gprintf::Var>& vars, std::vector<std::string>& subs)
    {
        const char* fmt = fmtstr.c_str();
        std::string sub;
        char chr;

        int width = 0, index = 0;
        bool foundToken = false;

        while((chr = *fmt++) != 0)
        {
            if(index + 1 > int(variables.size())) return false;

            if(chr == '%'  ||  foundToken)
            {
                foundToken = true;
                Gprintf::Format format = Gprintf::Int;
                sub += chr;

                switch(chr)
                {
                    case '0':
                    {
                        // Maximum field width of 16 digits
                        width = strtol(fmt, nullptr, 10) % 17;
                    }
                    break;

                    case 'c': format = Gprintf::Chr; break;
                    case 'd': format = Gprintf::Int; break;
                    case 'b': format = Gprintf::Bin; break;
                    case 'q':
                    case 'o': format = Gprintf::Oct; break;
                    case 'x': format = Gprintf::Hex; break;
                    case 's': format = Gprintf::Str; break;

                    default: break;
                }

                if(chr == 'c' || chr == 'd' || chr == 'b' || chr == 'q' || chr == 'o' || chr == 'x' || chr == 's')
                {
                    foundToken = false;
                    Gprintf::Var var = {0, format, width, 0x0000, variables[index++]};
                    vars.push_back(var);
                    subs.push_back(sub);
                    sub.clear();
                    width = 0;
                }
            }
        }

        return true;
    }

    bool handleGprintf(ParseType parse, const std::string& lineToken, int lineNumber)
    {
        std::string input = lineToken;
        Expression::strToUpper(input);

        // Handle non commented GPRINTF
        size_t gprintfPos = input.find("GPRINTF");
        if(gprintfPos != std::string::npos  &&  input.find_last_of(';', gprintfPos) == std::string::npos)
        {
            size_t lbra, rbra;
            if(Expression::findMatchingBrackets(lineToken, 0, lbra, rbra))
            {
                size_t quote1 = lineToken.find_first_of("\"", lbra+1);
                size_t quote2 = lineToken.find_last_of("\"", rbra-1);
                bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 0));
                if(quotes)
                {
                    if(parse == MnemonicPass)
                    {
                        std::string formatText = lineToken.substr(quote1+1, quote2 - (quote1+1));
                        std::string variableText = lineToken.substr(quote2+1, rbra - (quote2+1));

                        std::vector<Gprintf::Var> vars;
                        std::vector<std::string> subs;
                        std::vector<std::string> variables = Expression::tokenise(variableText, ',');
                        parseGprintfFormat(formatText, variables, vars, subs);

                        Gprintf gprintf = {_currentAddress, lineNumber, lineToken, formatText, vars, subs};
                        if(_gprintfs.find(_currentAddress) != _gprintfs.end())
                        {
                            fprintf(stderr, "Assembler::handleGprintf() : '%s:%d' : gprintf at address 0x%0x4 already exists\n", lineToken.c_str(), lineNumber, _currentAddress);
                            return false;
                        }

                        _gprintfs[_currentAddress] = gprintf;
                    }

                    return true;
                }
            }

            fprintf(stderr, "Assembler::handleGprintf() : '%s:%d' : bad gprintf format\n", lineToken.c_str(), lineNumber);
            return false;
        }

        return false;
    }

    bool parseGprintfVar(const std::string& param, uint16_t& data)
    {
        bool success = Expression::stringToU16(param, data);
        if(!success)
        {
            std::vector<std::string> tokens;
            tokens.push_back(param);

            // Search equates
            Equate equate;
            Label label;
            if((success = evaluateEquateOperand(tokens, 0, equate, false)) == true)
            {
                data = equate._operand;
            }
            // Search labels
            else if((success = evaluateLabelOperand(tokens, 0, label, false)) == true)
            {
                data = label._address;
            }
            // Normal expression
            else
            {
                if(Expression::isExpression(param) == Expression::HasOperators)
                {
                    Expression::Numeric value;
                    if(Expression::parse(param, _lineNumber, value))
                    {
                        data = int16_t(std::lround(value._value));
                        success = true;
                    }
                }
            }
        }

        return success;
    }

    bool parseGprintfs(void)
    {
        for(auto it=_gprintfs.begin(); it!=_gprintfs.end(); it++ )
        {
            Gprintf& gprintf = it->second;

            for(int j=0; j<int(gprintf._vars.size()); j++)
            {
                uint16_t addr = 0x0000;
                std::string token = gprintf._vars[j]._text;
        
                // Strip white space
                token.erase(remove_if(token.begin(), token.end(), isspace), token.end());
                gprintf._vars[j]._text = token;

                // Check for indirection
                size_t asterisk = token.find_first_of("*");
                if(asterisk != std::string::npos)
                {
                    // Don't overwrites gtBASIC vars
                    if(gprintf._vars[j]._indirection == 0) gprintf._vars[j]._indirection = 1;
                    token = token.substr(asterisk + 1);
                }

                if(!parseGprintfVar(token, addr))
                {
                    fprintf(stderr, "Assembler::parseGprintfs() : '%s:%d' : error in gprintf(), missing label or equate '%s'\n", gprintf._lineToken.c_str(), gprintf._lineNumber, token.c_str());
                    _gprintfs.erase(it);
                    return false;
                }

                gprintf._vars[j]._data = addr;
            }
        }

        return true;
    }

    bool getGprintfString(const Gprintf& gprintf, std::string& gstring)
    {
        gstring = gprintf._format;
   
        size_t subIndex = 0;
        for(int i=0; i<int(gprintf._vars.size()); i++)
        {
            char token[256];

            // Use indirection if required
            uint16_t data = (gprintf._vars[i]._indirection) ? Cpu::getRAM16(gprintf._vars[i]._data) : gprintf._vars[i]._data;

            // Maximum field width of 16 digits
            uint8_t width = gprintf._vars[i]._width % 17;
            std::string fieldWidth = "%";
            if(width) fieldWidth = std::string("%0" + std::to_string(width));

            switch(gprintf._vars[i]._format)
            {
                case Gprintf::Chr: fieldWidth += "c"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Int: fieldWidth += "d"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Oct: fieldWidth += "o"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Hex: fieldWidth += "x"; sprintf(token, fieldWidth.c_str(), data); break;

                // Strings are always indirect and assume that length is first byte
                case Gprintf::Str:
                {
                    // gtBASIC strings are dereferenced by 2 levels of indirection
                    uint16_t addr = (gprintf._vars[i]._indirection == 2) ? Cpu::getRAM16(gprintf._vars[i]._data) : gprintf._vars[i]._data;

                    // Maximum length of 254, first byte is length, last byte is '0' trailing delimiter
                    uint8_t length = Cpu::getRAM(addr) % 254;
                    for(uint16_t j=0; j<length; j++) token[j] = Cpu::getRAM(addr + j + 1);
                    token[length] = 0;
                }
                break;

                case Gprintf::Bin:
                {
                    for(int j=width-1; j>=0; j--)
                    {
                        token[width-1 - j] = '0' + ((data >> j) & 1);
                    }
                    token[width] = 0;
                }
                break;

                default: return false;
            }

            // Replace substrings
            subIndex = gstring.find(gprintf._subs[i], subIndex);
            if(subIndex != std::string::npos)
            {
                gstring.erase(subIndex, gprintf._subs[i].size());
                gstring.insert(subIndex, token);
            }
        }

        return true;
    }

    void handleGprintfs(void)
    {
        if(_gprintfs.size() == 0) return;

        if(_gprintfs.find(Cpu::getVPC()) != _gprintfs.end())
        {
            std::string gstring;
            const Gprintf& gprintf = _gprintfs[Cpu::getVPC()];

            getGprintfString(gprintf, gstring);
            fprintf(stderr, "gprintf() : 0x%04X : %s\n", gprintf._address, gstring.c_str());
        }
    }
#endif

    uint8_t sysHelper(const std::string& opcodeStr, uint16_t operand, const std::string& filename, int lineNumber)
    {
        std::string opcode = opcodeStr;
        Expression::strToUpper(opcode); 
        
        if(opcode != "SYS") return uint8_t(operand);

        if((operand & 0x0001) || operand < 28 || operand > 284)
        {
            fprintf(stderr, "Assembler::sysHelper() : '%s:%d' : SYS operand '%d' must be an even constant in [28, 284]\n", filename.c_str(), lineNumber, operand);
            return uint8_t(operand);
        }

        return uint8_t((270 - operand / 2) & 0x00FF);
    }


    void clearAssembler(bool dontClearGprintfs)
    {
        _byteCode.clear();
        _labels.clear();
        _equates.clear();
        _instructions.clear();
        _callTableEntries.clear();
        if(!dontClearGprintfs) _gprintfs.clear();

        clearDefines();        

        _vSpMin = 0x00;
        _callTablePtr = 0x0000;

        Expression::setExprFunc(Expression::expression);

#ifndef STAND_ALONE
        Editor::clearVpcBreakPoints();
#endif
    }

    bool assemble(const std::string& filename, uint16_t startAddress, bool dontClearGprintfs)
    {
        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::assemble() : failed to open file '%s'\n", filename.c_str());
            return false;
        }

        clearAssembler(dontClearGprintfs);

        _startAddress = startAddress;
        _currentAddress = _startAddress;

#ifndef STAND_ALONE
        Loader::disableUploads(false);
#endif

        // Get file
        int numLines = 0;
        LineToken lineToken;
        std::vector<LineToken> lineTokens;
        while(!infile.eof())
        {
            std::getline(infile, lineToken._text);
            lineTokens.push_back(lineToken);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Assembler::assemble() : '%s:%d' : bad lineToken '%s'\n", filename.c_str(), numLines+1, lineToken._text.c_str());
                return false;
            }

            numLines++;
        }

        // Pre-processor
        if(!preProcess(filename, lineTokens, true)) return false;

        fprintf(stderr, "\n*******************************************************\n");
        fprintf(stderr, "* Assembling file : '%s'\n", filename.c_str());
        fprintf(stderr, "*******************************************************\n");

        numLines = int(lineTokens.size());

        // The mnemonic pass we evaluate all the equates and labels, the code pass is for the opcodes and operands
        for(int parse=MnemonicPass; parse<NumParseTypes; parse++)
        {
            for(_lineNumber=0; _lineNumber<numLines; _lineNumber++)
            {
                lineToken = lineTokens[_lineNumber];

                // Lines containing only white space are skipped
                size_t nonWhiteSpace = lineToken._text.find_first_not_of("  \n\r\f\t\v");
                if(nonWhiteSpace == std::string::npos) continue;

                int tokenIndex = 0;

                // Tokenise current line
                std::vector<std::string> tokens = Expression::tokeniseLine(lineToken._text);
                if(tokens.size() == 0)
                {
                    fprintf(stderr, "Assembler::assemble() : '%s:%d' : unknown error '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                    return false;
                }

                // Comments
                if(tokens.size() > 0  &&  tokens[0].find_first_of(";#") != std::string::npos) continue;

#ifndef STAND_ALONE
                // Gprintf lines are skipped
                if(handleGprintf(ParseType(parse), lineToken._text, _lineNumber+1)) continue;

                // _breakPoint_ lines are skipped
                if(handleBreakPoints(ParseType(parse), lineToken._text, _lineNumber+1)) continue;
#endif

                // Starting address, labels and equates
                if(nonWhiteSpace == 0)
                {
                    if(tokens.size() >= 2)
                    {
                        EvaluateResult result = evaluateEquates(tokens, (ParseType)parse);
                        if(result == NotFound)
                        {
                            fprintf(stderr, "Assembler::assemble() : '%s:%d' : missing equate '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                            return false;
                        }
                        else if(result == Duplicate)
                        {
                            fprintf(stderr, "Assembler::assemble() : '%s:%d' : duplicate equate '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                            return false;
                        }
                        // Skip equate lines
                        else if(result == Success) 
                        {
                            continue;
                        }
                            
                        // Labels
                        result = EvaluateLabels(tokens, (ParseType)parse, tokenIndex);
                        if(result == Reserved)
                        {
                            fprintf(stderr, "Assembler::assemble() : '%s:%d' : can't use a reserved word in label '%s'\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                            return false;
                        }
                        else if(result == Duplicate)
                        {
                            fprintf(stderr, "Assembler::assemble() : '%s:%d' : duplicate label '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                            return false;
                        }
                    }

                    // On to the next token even if we failed with this one
                    if(tokens.size() > 1) tokenIndex++;
                }

                // Opcode
                bool operandValid = false;
                uint16_t defineOffset;
                std::string opcodeStr = tokens[tokenIndex++];
                InstructionType instructionType = getOpcode(opcodeStr, defineOffset);
                uint8_t opcode0 = instructionType._opcode0;
                uint8_t opcode1 = instructionType._opcode1;
                int outputSize = instructionType._byteSize;
                OpcodeType opcodeType = instructionType._opcodeType;
                Instruction instruction = {false, false, ByteSize(outputSize), opcode0, 0x00, 0x00, _currentAddress, opcodeType};

                if(outputSize == BadSize)
                {
                    fprintf(stderr, "Assembler::assemble() : '%s:%d' : bad Opcode '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                    return false;
                }

                // Compound instructions that require a Mnemonic pass
                bool compoundInstruction = false;
                if(opcodeType == ReservedDB  ||  opcodeType == ReservedDBR)
                {
                    compoundInstruction = true;
                    if(parse == MnemonicPass)
                    {
                        outputSize = OneByte; // first instruction has already been parsed
                        if(tokenIndex + 1 < int(tokens.size()))
                        {
                            if(!handleDefineByte(tokens, tokenIndex, instruction, false, defineOffset, outputSize))
                            {
                                fprintf(stderr, "Assembler::assemble() : '%s:%d' : bad DB data '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                                return false;
                            }
                        }
                    }
                }
                else if(opcodeType == ReservedDW  ||  opcodeType == ReservedDWR)
                {
                    compoundInstruction = true;
                    if(parse == MnemonicPass)
                    {
                        outputSize = TwoBytes; // first instruction has already been parsed
                        if(tokenIndex + 1 < int(tokens.size()))
                        {
                            if(!handleDefineWord(tokens, tokenIndex, instruction, false, defineOffset, outputSize))
                            {
                                fprintf(stderr, "Assembler::assemble() : '%s:%d' : bad DW data '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                                return false;
                            }
                        }
                    }
                }
                
                if(parse == CodePass)
                {
                    // Native NOP
                    if(opcodeType == Native  &&  opcode0 == 0x02)
                    {
                        operandValid = true;
                    }
                    // Missing operand
                    else if((outputSize == TwoBytes  ||  outputSize == ThreeBytes)  &&  int(tokens.size()) <= tokenIndex)
                    {
                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : missing operand/s '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                        return false;
                    }

                    // First instruction inherits start address
                    if(_instructions.size() == 0)
                    {
                        instruction._address = _startAddress;
                        instruction._isCustomAddress = true;
                        _currentAddress = _startAddress;
                    }

                    // Custom address
                    for(int i=0; i<int(_equates.size()); i++)
                    {
                        if(_equates[i]._name == tokens[0]  &&  _equates[i]._isCustomAddress)
                        {
                            instruction._address = _equates[i]._operand;
                            instruction._isCustomAddress = true;
                            _currentAddress = _equates[i]._operand;
                        }
                    }

                    // Operand
                    switch(outputSize)
                    {
                        case OneByte:
                        {
                            _instructions.push_back(instruction);
                            if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;
                        }
                        break;

                        case TwoBytes:
                        {
                            uint8_t operand = 0x00;

                            // HALT
                            if(opcodeType == vCpu  &&  opcode0 == OPCODE_V_HALT  &&  opcode1 == OPERAND_V_HALT)
                            {
                                operand = opcode1;
                                operandValid = true;
                            }
                            // BRA
                            else if(opcodeType == vCpu  &&  opcode0 == OPCODE_V_BRA)
                            {
                                // Search for branch label
                                Label label;
                                if(evaluateLabelOperand(tokens, tokenIndex, label, false))
                                {
                                    operandValid = true;
                                    operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                }
                                // Allow branches to raw hex values, lets hope the user knows what he is doing
                                else if(Expression::stringToU8(tokens[tokenIndex], operand))
                                {
                                    operandValid = true;
                                    operand -= BRANCH_ADJUSTMENT;
                                }
                                else
                                {
                                    fprintf(stderr, "Assembler::assemble() : '%s:%d' : label '%s' missing\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                                    return false;
                                }
                            }
                            // CALL+
                            else if(opcodeType == vCpu  &&  opcode0 == OPCODE_V_CALL)
                            {
                                // Search for call label
                                if(_callTablePtr  &&  operand != GIGA_V_AC)
                                {
                                    Label label;
                                    if(evaluateLabelOperand(tokens, tokenIndex, label, false))
                                    {
                                        // Search for address
                                        bool newLabel = true;
                                        uint16_t address = uint16_t(label._address);
                                        for(int i=0; i<int(_callTableEntries.size()); i++)
                                        {
                                            if(_callTableEntries[i]._address == address)
                                            {
                                                operandValid = true;
                                                operand = _callTableEntries[i]._operand;
                                                newLabel = false;
                                                break;
                                            }
                                        }

                                        // Found a new call address label, put it's address into the call table and point the call instruction to the call table
                                        if(newLabel)
                                        {
                                            operandValid = true;
                                            operand = uint8_t(LO_BYTE(_callTablePtr));
                                            CallTableEntry entry = {operand, address};
                                            _callTableEntries.push_back(entry);
                                            _callTablePtr -= 0x0002;

                                            // Avoid ONE_CONST_ADDRESS
                                            if(_callTablePtr == ONE_CONST_ADDRESS)
                                            {
                                                fprintf(stderr, "Assembler::assemble() : warning, (safe to ignore), Calltable : 0x%02x : collided with : 0x%02x : on line %d\n", _callTablePtr, ONE_CONST_ADDRESS, _lineNumber+1);
                                                _callTablePtr -= 0x0002;
                                            }
                                            else if(_callTablePtr+1 == ONE_CONST_ADDRESS)
                                            {
                                                fprintf(stderr, "Assembler::assemble() : warning, (safe to ignore), Calltable : 0x%02x : collided with : 0x%02x : on line %d\n", _callTablePtr+1, ONE_CONST_ADDRESS, _lineNumber+1);
                                                _callTablePtr -= 0x0001;
                                            }
                                        }
                                    }
                                }
                                // CALL that doesn't use the call table, ('CALL GIGA_V_AC', usually to save zero page memory at the expense of code size and code speed).
                                else
                                {
                                    Equate equate;
                                    if((operandValid = evaluateEquateOperand(tokens, tokenIndex, equate, false)) == true)
                                    {
                                        operand = uint8_t(equate._operand);
                                    }
                                    else 
                                    {
                                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : label '%s' missing\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                                        return false;
                                    }
                                }
                            }
                                
                            // All other non native 2 byte instructions
                            if(opcodeType != Native  &&  !operandValid)
                            {
                                Label label;
                                Equate equate;

                                // String
                                size_t quote1 = tokens[tokenIndex].find_first_of("'");
                                size_t quote2 = tokens[tokenIndex].find_last_of("'");
                                bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 1));
                                if(quotes)
                                {
                                    operand = sysHelper(opcodeStr, uint16_t(tokens[tokenIndex][quote1+1]), filename, _lineNumber+1);
                                }
                                // Search equates
                                else if((operandValid = evaluateEquateOperand(tokens, tokenIndex, equate, compoundInstruction)) == true)
                                {
                                    operand = sysHelper(opcodeStr, equate._operand, filename, _lineNumber+1);
                                }
                                // Search labels
                                else if((operandValid = evaluateLabelOperand(tokens, tokenIndex, label, compoundInstruction)) == true)
                                {
                                    operand = sysHelper(opcodeStr, label._address, filename, _lineNumber+1);
                                }
                                else if(Expression::isExpression(tokens[tokenIndex]) == Expression::HasOperators)
                                {
                                    Expression::Numeric value;
                                    std::string input;
                                    preProcessExpression(tokens, tokenIndex, input, true);
                                    if(!Expression::parse(input, _lineNumber, value)) return false;
                                    operand = sysHelper(opcodeStr, uint16_t(std::lround(value._value)), filename, _lineNumber+1);
                                    operandValid = true;
                                }
                                else
                                {
                                    uint16_t operandU16 = 0;
                                    operandValid = Expression::stringToU16(tokens[tokenIndex], operandU16);
                                    if(!operandValid)
                                    {
                                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : invalid label/Equate '%s'\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                                        return false;
                                    }

                                    operand = sysHelper(opcodeStr, operandU16, filename, _lineNumber+1);
                                }
                            }

                            // Native instructions
                            if(opcodeType == Native)
                            {
                                if(!operandValid)
                                {
                                    if(!handleNativeInstruction(tokens, tokenIndex, opcode0, operand))
                                    {
                                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : native instruction '%s' is malformed\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                                        return false;
                                    }
                                }

                                instruction._isRomAddress = true;
                                instruction._opcode = opcode0;
                                instruction._operand0 = uint8_t(LO_BYTE(operand));
                                _instructions.push_back(instruction);
                                if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;

#ifndef STAND_ALONE
                                uint16_t add = instruction._address>>1;
                                uint8_t opc = Cpu::getROM(add, 0);
                                uint8_t ope = Cpu::getROM(add, 1);
                                if(instruction._opcode != opc  ||  instruction._operand0 != ope)
                                {
                                    fprintf(stderr, "Assembler::assemble() : '%s:%d' : ROM Native instruction mismatch : 0x%04X : ASM=0x%02X%02X : ROM=0x%02X%02X\n", filename.c_str(), _lineNumber+1, add, 
                                                                                                                                                                      instruction._opcode, instruction._operand0,
                                                                                                                                                                      opc, ope);

                                    // Fix mismatched instruction?
                                    //instruction._opcode = opc;
                                    //instruction._operand0 = ope;
                                    //_instructions.back() = instruction;
                                }
#endif
                            }
                            // Reserved assembler opcode DB, (define byte)
                            else if(opcodeType == ReservedDB  ||  opcodeType == ReservedDBR)
                            {
                                // Push first operand
                                outputSize = OneByte;
                                instruction._isRomAddress = (opcodeType == ReservedDBR) ? true : false;
                                instruction._byteSize = ByteSize(outputSize);
                                instruction._opcode = uint8_t(LO_BYTE(operand));
                                if(defineOffset != 0)
                                {
                                    instruction._address = _currentAddress;
                                    instruction._isCustomAddress = true;
                                    _currentAddress += defineOffset;
                                }
                                _instructions.push_back(instruction);
    
                                // Push any remaining operands
                                if(tokenIndex + 1 < int(tokens.size()))
                                {
                                    if(!handleDefineByte(tokens, tokenIndex, instruction, true, defineOffset, outputSize))
                                    {
                                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : bad DB data '%s'\n", filename.c_str(), _lineNumber+1, lineToken._text.c_str());
                                        return false;
                                    }
                                }

                                if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;
                            }
                            // Normal instructions
                            else
                            {
                                instruction._operand0 = operand;
                                _instructions.push_back(instruction);
                                if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;
                            }
                        }
                        break;
                            
                        case ThreeBytes:
                        {
                            // BCC
                            if(opcode1)
                            {
                                // Search for branch label
                                Label label;
                                uint8_t operand = 0x00;
                                if(evaluateLabelOperand(tokens, tokenIndex, label, false))
                                {
                                    operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                }
                                else
                                {
                                    fprintf(stderr, "Assembler::assemble() : '%s:%d' : label '%s' missing\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                                    return false;
                                }

                                instruction._operand0 = opcode1;
                                instruction._operand1 = LO_BYTE(operand);
                                _instructions.push_back(instruction);
                                if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;
                            }
                            // All other 3 byte instructions
                            else
                            {
                                uint16_t operand = 0x0000;
                                Label label;
                                Equate equate;

                                // Search equates
                                if((operandValid = evaluateEquateOperand(tokens, tokenIndex, equate, compoundInstruction)) == true)
                                {
                                    operand = equate._operand;
                                }
                                // Search labels
                                else if((operandValid = evaluateLabelOperand(tokens, tokenIndex, label, compoundInstruction)) == true)
                                {
                                    operand = label._address;
                                }
                                else if(Expression::isExpression(tokens[tokenIndex]) == Expression::HasOperators)
                                {
                                    Expression::Numeric value;
                                    std::string input;
                                    preProcessExpression(tokens, tokenIndex, input, true);
                                    if(!Expression::parse((char*)input.c_str(), _lineNumber, value)) return false;
                                    operand = int16_t(std::lround(value._value));
                                    operandValid = true;
                                }
                                else
                                {
                                    operandValid = Expression::stringToU16(tokens[tokenIndex], operand);
                                    if(!operandValid)
                                    {
                                        fprintf(stderr, "Assembler::assemble() : '%s:%d' : invalid label/equate '%s'\n", filename.c_str(), _lineNumber+1, tokens[tokenIndex].c_str());
                                        return false;
                                    }
                                }
                                
                                // Reserved assembler opcode DW, (define word)
                                if(opcodeType == ReservedDW  ||  opcodeType == ReservedDWR)
                                {
                                    // Push first operand
                                    outputSize = TwoBytes;
                                    instruction._isRomAddress = (opcodeType == ReservedDWR) ? true : false;
                                    instruction._byteSize = ByteSize(outputSize);
                                    instruction._opcode   = uint8_t(LO_BYTE(operand));
                                    instruction._operand0 = uint8_t(HI_BYTE(operand));
                                    if(defineOffset != 0)
                                    {
                                        instruction._address = _currentAddress;
                                        instruction._isCustomAddress = true;
                                        _currentAddress += defineOffset * 2;
                                    }
                                    _instructions.push_back(instruction);

                                    // Push any remaining operands
                                    if(tokenIndex + 1 < int(tokens.size())) handleDefineWord(tokens, tokenIndex, instruction, true, defineOffset, outputSize);
                                    if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(outputSize), instruction, lineToken, filename, _lineNumber)) return false;
                                }
                                // Normal instructions
                                else
                                {
                                    instruction._operand0 = uint8_t(LO_BYTE(operand));
                                    instruction._operand1 = uint8_t(HI_BYTE(operand));
                                    _instructions.push_back(instruction);
                                    if(!checkInvalidAddress(ParseType(parse), _currentAddress, uint16_t(instruction._byteSize), instruction, lineToken, filename, _lineNumber)) return false;
                                }
                            }
                        }
                        break;

                        default: break;
                    }
                }

                _currentAddress += uint16_t(outputSize);
            }              
        }

        // Pack byte code buffer from instruction buffer
        packByteCodeBuffer();

#ifndef STAND_ALONE
        // Parse gprintf labels, equates and expressions
        if(!parseGprintfs()) return false;
#endif

        return true;
    }
}
