#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cstdarg>

#include "cpu.h"
#include "editor.h"
#include "loader.h"
#include "assembler.h"
#include "expression.h"


#define BRANCH_ADJUSTMENT    2


namespace Assembler
{
    enum ParseType {PreProcessPass=0, MnemonicPass, CodePass, NumParseTypes};
    enum ByteSize {BadSize=-1, OneByte=1, TwoBytes=2, ThreeBytes=3};
    enum EvaluateResult {Failed=-1, NotFound, Reserved, Duplicate, Skipped, Success};
    enum OpcodeType {ReservedDB=0, ReservedDW, ReservedDBR, ReservedDWR, vCpu, Native};
    enum AddressMode {D_AC=0b00000000, X_AC=0b00000100, YD_AC=0b00001000, YX_AC=0b00001100, D_X=0b00010000, D_Y=0b00010100, D_OUT=0b00011000, YXpp_OUT=0b00011100};
    enum BusMode {D=0b00000000, RAM=0b00000001, AC=0b00000010, IN=0b00000011};
    enum ReservedWords {CallTable=0, StartAddress, SingleStepWatch, DisableUpload, INCLUDE, MACRO, ENDM, GPRINTF, NumReservedWords};


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
    };

    struct InstructionType
    {
        uint8_t _opcode;
        uint8_t _branch;
        ByteSize _byteSize;
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
        int _startLine = 0;
        int _endLine = 0;
        std::string _name;
        std::vector<std::string> _params;
        std::vector<std::string> _lines;
    };

    struct Gprintf
    {
        enum Type {Chr, Int, Bin, Oct, Hex, Str};
        struct Var
        {
            bool _indirect = false;
            Type _type;
            int _width;
            uint16_t _data;
            std::string _var;
        };

        bool _displayed = false;
        uint16_t _address;
        int _lineNumber;
        std::string _lineToken;
        std::string _format;
        std::vector<Var> _vars;
        std::vector<std::string> _subs;
    };


    uint16_t _byteCount = 0;
    uint16_t _callTable = DEFAULT_CALL_TABLE;
    uint16_t _startAddress = DEFAULT_START_ADDRESS;
    uint16_t _currentAddress = _startAddress;

    std::vector<Label> _labels;
    std::vector<Equate> _equates;
    std::vector<Instruction> _instructions;
    std::vector<ByteCode> _byteCode;
    std::vector<CallTableEntry> _callTableEntries;
    std::vector<std::string> _reservedWords;
    std::vector<Gprintf> _gprintfs;

    uint16_t getStartAddress(void) {return _startAddress;}


    void initialise(void)
    {
        _reservedWords.push_back("_callTable_");
        _reservedWords.push_back("_startAddress_");
        _reservedWords.push_back("_singleStepWatch_");
        _reservedWords.push_back("_disableUpload_");
        _reservedWords.push_back("%include");
        _reservedWords.push_back("%MACRO");
        _reservedWords.push_back("%ENDM");
        _reservedWords.push_back("gprintf");
    }

    // Returns true when finished
    bool getNextAssembledByte(ByteCode& byteCode)
    {
        static bool isUserCode = false;

        if(_byteCount >= _byteCode.size())
        {
            _byteCount = 0;
            if(isUserCode) fprintf(stderr, "\n");
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

        // User code is RAM code or (ROM code that lives at or above 0x2300)
        isUserCode = !byteCode._isRomAddress  ||  (byteCode._isRomAddress  &&  customAddress >= 0x2300);

        // Seperate sections
        if(byteCode._isCustomAddress  &&  isUserCode) fprintf(stderr, "\n");

        // 16bit for ROM, 8bit for RAM
        if(isUserCode)
        {
            if(byteCode._isRomAddress)
            {
                if((address & 0x0001) == 0x0000)
                {
                    fprintf(stderr, "Assembler::getNextAssembledByte() : ROM : %04X  %02X", customAddress + ((address & 0x00FF)>>1), byteCode._data);
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

    InstructionType getOpcode(const std::string& input)
    {
        InstructionType instructionType = {0x00, 0x00, BadSize, vCpu};

        std::string token = input;
        Expression::strToUpper(token);

        // Gigatron vCPU instructions
        if(token == "ST")         {instructionType._opcode = 0x5E; instructionType._byteSize = TwoBytes;  }
        else if(token == "STW")   {instructionType._opcode = 0x2B; instructionType._byteSize = TwoBytes;  }
        else if(token == "STLW")  {instructionType._opcode = 0xEC; instructionType._byteSize = TwoBytes;  }
        else if(token == "LD")    {instructionType._opcode = 0x1A; instructionType._byteSize = TwoBytes;  }
        else if(token == "LDI")   {instructionType._opcode = 0x59; instructionType._byteSize = TwoBytes;  }
        else if(token == "LDWI")  {instructionType._opcode = 0x11; instructionType._byteSize = ThreeBytes;}
        else if(token == "LDW")   {instructionType._opcode = 0x21; instructionType._byteSize = TwoBytes;  }
        else if(token == "LDLW")  {instructionType._opcode = 0xEE; instructionType._byteSize = TwoBytes;  }
        else if(token == "ADDW")  {instructionType._opcode = 0x99; instructionType._byteSize = TwoBytes;  }
        else if(token == "SUBW")  {instructionType._opcode = 0xB8; instructionType._byteSize = TwoBytes;  }
        else if(token == "ADDI")  {instructionType._opcode = 0xE3; instructionType._byteSize = TwoBytes;  }
        else if(token == "SUBI")  {instructionType._opcode = 0xE6; instructionType._byteSize = TwoBytes;  }
        else if(token == "LSLW")  {instructionType._opcode = 0xE9; instructionType._byteSize = OneByte;   }
        else if(token == "INC")   {instructionType._opcode = 0x93; instructionType._byteSize = TwoBytes;  }
        else if(token == "ANDI")  {instructionType._opcode = 0x82; instructionType._byteSize = TwoBytes;  }
        else if(token == "ANDW")  {instructionType._opcode = 0xF8; instructionType._byteSize = TwoBytes;  }
        else if(token == "ORI")   {instructionType._opcode = 0x88; instructionType._byteSize = TwoBytes;  }
        else if(token == "ORW")   {instructionType._opcode = 0xFA; instructionType._byteSize = TwoBytes;  }
        else if(token == "XORI")  {instructionType._opcode = 0x8C; instructionType._byteSize = TwoBytes;  }
        else if(token == "XORW")  {instructionType._opcode = 0xFC; instructionType._byteSize = TwoBytes;  }
        else if(token == "PEEK")  {instructionType._opcode = 0xAD; instructionType._byteSize = OneByte;   }
        else if(token == "DEEK")  {instructionType._opcode = 0xF6; instructionType._byteSize = OneByte;   }
        else if(token == "POKE")  {instructionType._opcode = 0xF0; instructionType._byteSize = TwoBytes;  }
        else if(token == "DOKE")  {instructionType._opcode = 0xF3; instructionType._byteSize = TwoBytes;  }
        else if(token == "LUP")   {instructionType._opcode = 0x7F; instructionType._byteSize = TwoBytes;  }
        else if(token == "BRA")   {instructionType._opcode = 0x90; instructionType._byteSize = TwoBytes;  }
        else if(token == "CALL")  {instructionType._opcode = 0xCF; instructionType._byteSize = TwoBytes;  }
        else if(token == "RET")   {instructionType._opcode = 0xFF; instructionType._byteSize = OneByte;   }
        else if(token == "PUSH")  {instructionType._opcode = 0x75; instructionType._byteSize = OneByte;   }
        else if(token == "POP")   {instructionType._opcode = 0x63; instructionType._byteSize = OneByte;   }
        else if(token == "ALLOC") {instructionType._opcode = 0xDF; instructionType._byteSize = TwoBytes;  }
        else if(token == "SYS")   {instructionType._opcode = 0xB4; instructionType._byteSize = TwoBytes;  }
        else if(token == "DEF")   {instructionType._opcode = 0xCD; instructionType._byteSize = TwoBytes;  }

        // Gigatron vCPU branch instructions
        else if(token == "BEQ")   {instructionType._opcode = 0x35; instructionType._branch = 0x3F; instructionType._byteSize = ThreeBytes;}
        else if(token == "BNE")   {instructionType._opcode = 0x35; instructionType._branch = 0x72; instructionType._byteSize = ThreeBytes;}
        else if(token == "BLT")   {instructionType._opcode = 0x35; instructionType._branch = 0x50; instructionType._byteSize = ThreeBytes;}
        else if(token == "BGT")   {instructionType._opcode = 0x35; instructionType._branch = 0x4D; instructionType._byteSize = ThreeBytes;}
        else if(token == "BLE")   {instructionType._opcode = 0x35; instructionType._branch = 0x56; instructionType._byteSize = ThreeBytes;}
        else if(token == "BGE")   {instructionType._opcode = 0x35; instructionType._branch = 0x53; instructionType._byteSize = ThreeBytes;}

        // Reserved assembler opcodes
        else if(token == "DB")    {instructionType._byteSize = TwoBytes;   instructionType._opcodeType = ReservedDB; }
        else if(token == "DW")    {instructionType._byteSize = ThreeBytes; instructionType._opcodeType = ReservedDW; }
        else if(token == "DBR")   {instructionType._byteSize = TwoBytes;   instructionType._opcodeType = ReservedDBR;}
        else if(token == "DWR")   {instructionType._byteSize = ThreeBytes; instructionType._opcodeType = ReservedDWR;}
                                                                           
        // Gigatron native instructions                                    
        else if(token == ".LD")   {instructionType._opcode = 0x00; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".NOP")  {instructionType._opcode = 0x02; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".ANDA") {instructionType._opcode = 0x20; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".ORA")  {instructionType._opcode = 0x40; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".XORA") {instructionType._opcode = 0x60; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".ADDA") {instructionType._opcode = 0x80; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".SUBA") {instructionType._opcode = 0xA0; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".ST")   {instructionType._opcode = 0xC0; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".JMP")  {instructionType._opcode = 0xE0; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BGT")  {instructionType._opcode = 0xE4; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BLT")  {instructionType._opcode = 0xE8; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BNE")  {instructionType._opcode = 0xEC; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BEQ")  {instructionType._opcode = 0xF0; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BGE")  {instructionType._opcode = 0xF4; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BLE")  {instructionType._opcode = 0xF8; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}
        else if(token == ".BRA")  {instructionType._opcode = 0xFC; instructionType._byteSize = TwoBytes; instructionType._opcodeType = Native;}

        return instructionType;
    }

    bool parseEquateExpression(std::string input, uint16_t& operand)
    {
        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

        // Replace equates
        bool found = false;
        for(int i=0; i<_equates.size(); i++)
        {
            size_t pos = input.find(_equates[i]._name);
            while(pos != std::string::npos)
            {
                found = true;
                input.replace(pos, _equates[i]._name.size(), std::to_string(_equates[i]._operand));
                pos = input.find(_equates[i]._name, pos + _equates[i]._name.size());
            }
        }

        // Parse expression and return with a result
        if(found) operand = Expression::parse((char*)input.c_str());

        return found;
    }

    bool searchEquate(const std::string& token, Equate& equate)
    {
        // Normal equate
        bool success = false;
        for(int i=0; i<_equates.size(); i++)
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

    bool searchEquates(const std::vector<std::string>& tokens, int tokenIndex, Equate& equate)
    {
        if(tokenIndex >= tokens.size()) return false;

        std::string input;

        // Pre-process
        for(int j=tokenIndex; j<tokens.size(); j++)
        {
            // Strip comments
            if(tokens[j].find_first_of(";#") != std::string::npos) break;

            // Concatenate
            input += tokens[j];
        }

        // Expression equates
        if(input.find_first_of("[]") != std::string::npos) return false;
        if(input.find("++") != std::string::npos) return false;
        if(input.find_first_of("+-*/()") != std::string::npos)
        {
            // Parse expression and return with a result
             return parseEquateExpression(input, equate._operand);
        }

        // Normal equate
        return searchEquate(tokens[tokenIndex], equate);
    }

    uint16_t parseLabelExpression(std::string input)
    {
        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

        // Replace labels
        for(int i=0; i<_labels.size(); i++)
        {
            size_t pos = input.find(_labels[i]._name);
            while(pos != std::string::npos)
            {
                input.replace(pos, _labels[i]._name.size(), std::to_string(_labels[i]._address));
                pos = input.find(_labels[i]._name, pos + _labels[i]._name.size());
            }
        }

        // Parse expression and return with a result
        return Expression::parse((char*)input.c_str());
    }

    bool searchLabel(const std::string& token, Label& label)
    {
        bool success = false;
        for(int i=0; i<_labels.size(); i++)
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

    bool searchLabels(const std::vector<std::string>& tokens, int tokenIndex, Label& label)
    {
        if(tokenIndex >= tokens.size()) return false;

        std::string input;

        // Pre-process
        for(int j=tokenIndex; j<tokens.size(); j++)
        {
            // Strip comments
            if(tokens[j].find_first_of(";#") != std::string::npos) break;

            // Concatenate
            input += tokens[j];
        }

        // Expression equates
        if(input.find_first_of("[]") != std::string::npos) return false;
        if(input.find("++") != std::string::npos) return false;
        if(input.find_first_of("+-*/()") != std::string::npos)
        {
            // Parse expression and return with a result
            label._address = parseLabelExpression(input);
            return true;
        }

        // Normal label
        return searchLabel(tokens[tokenIndex], label);
    }

    EvaluateResult evaluateEquates(const std::vector<std::string>& tokens, ParseType parse, int tokenIndex)
    {
        if(tokens[1] == "EQU"  ||  tokens[1] == "equ")
        {
            static bool sortEquates = false;

            if(parse == MnemonicPass)
            {
                sortEquates = true;

                uint16_t operand = 0x0000;
                if(!Expression::stringToU16(tokens[2], operand))
                {
                    // Make sure it exists
                    Equate equate;
                    if(!searchEquates(tokens, 2, equate)) return NotFound;
                    operand = equate._operand;
                }

                // Reserved word, (equate), _callTable_
                if(tokens[tokenIndex] == "_callTable_")
                {
                    _callTable = operand;
                }
                // Reserved word, (equate), _singleStepWatch_
                else if(tokens[tokenIndex] == "_singleStepWatch_")
                {
                    Editor::setSingleStepWatchAddress(operand);
                }
                // Reserved word, (equate), _startAddress_
                else if(tokens[tokenIndex] == "_startAddress_")
                {
                    _startAddress = operand;
                    _currentAddress = _startAddress;
                }
                // Disable upload of the current assembler module
                else if(tokens[tokenIndex] == "_disableUpload_")
                {
                    Loader::disableUploads(operand != 0);
                }
                // Standard equates
                else
                {
                    // Check for duplicate
                    Equate equate = {false, operand, tokens[0]};                    
                    if(searchEquate(tokens[tokenIndex], equate)) return Duplicate;

                    _equates.push_back(equate);
                }
            }
            else if(parse == CodePass)
            {
                // Sort equates from largest size to smallest size, so that equate replacer in expressions works correctly
                if(sortEquates)
                {
                    sortEquates = false;
                    std::sort(_equates.begin(), _equates.end(), [](const Equate& equateA, const Equate& equateB)
                    {
                        return (equateA._name.size() > equateB._name.size());
                    });
                }
            }

            return Success;
        }

        return Failed;
    }

    EvaluateResult EvaluateLabels(const std::vector<std::string>& tokens, ParseType parse, int tokenIndex)
    {
        static bool sortLabels = false;

        if(parse == MnemonicPass) 
        {
            sortLabels = true;

            // Check reserved words
            for(int i=0; i<_reservedWords.size(); i++)
            {
                if(tokens[tokenIndex] == _reservedWords[i]) return Reserved;
            }
            
            Label label;
            if(searchLabel(tokens[tokenIndex], label)) return Duplicate;

            // Check equates for a custom start address
            for(int i=0; i<_equates.size(); i++)
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
            // Sort labels from largest size to smallest size, so that label replacer in expressions works correctly
            if(sortLabels)
            {
                sortLabels = false;
                std::sort(_labels.begin(), _labels.end(), [](const Label& labelA, const Label& labelB)
                {
                    return (labelA._name.size() > labelB._name.size());
                });
            }
        }

        return Success;
    }

    bool handleDefineByte(const std::vector<std::string>& tokens, int tokenIndex, uint8_t& operand, bool isRom)
    {
        bool success = false;

        // Handle case where first operand is a string
        size_t quote1 = tokens[tokenIndex].find_first_of("'\"");
        size_t quote2 = tokens[tokenIndex].find_first_of("'\"", quote1+1);
        bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 1));
        if(quotes)
        {
            std::string token = tokens[tokenIndex].substr(quote1+1, quote2 - (quote1+1));
            for(int j=1; j<token.size(); j++) // First byte was pushed by callee
            {
                Instruction instruction = {isRom, false, OneByte, uint8_t(token[j]), 0x00, 0x00, 0x0000};
                _instructions.push_back(instruction);
            }
            success = true;
        }

        for(int i=tokenIndex+1; i<tokens.size(); i++)
        {
            // Handle all other variations of strings
            size_t quote1 = tokens[i].find_first_of("'\"");
            size_t quote2 = tokens[i].find_first_of("'\"", quote1+1);
            bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos);
            if(quotes)
            {
                std::string token = tokens[i].substr(quote1+1, quote2 - (quote1+1));
                for(int j=0; j<token.size(); j++)
                {
                    Instruction instruction = {isRom, false, OneByte, uint8_t(token[j]), 0x00, 0x00, 0x0000};
                    _instructions.push_back(instruction);
                }
                success = true;
                continue;
            }
            else
            {
                success = Expression::stringToU8(tokens[i], operand);
                if(!success)
                {
                    Equate equate;
                    success = searchEquates(tokens, i, equate);
                    if(success)
                    {
                        operand = uint8_t(equate._operand);
                    }
                    else
                    {
                        break;
                    }
                }
                Instruction instruction = {isRom, false, OneByte, operand, 0x00, 0x00, 0x0000};
                _instructions.push_back(instruction);
            }
        }

        return success;
    }

    bool handleDefineWord(const std::vector<std::string>& tokens, int tokenIndex, uint16_t& operand, bool isRom)
    {
        bool success = false;

        for(int i=tokenIndex+1; i<tokens.size(); i++)
        {
            success = Expression::stringToU16(tokens[i], operand);
            if(!success)
            {
                Equate equate;
                success = searchEquates(tokens, i, equate);
                if(success)
                {
                    operand = equate._operand;
                }
                else
                {
                    break;
                }
            }
            Instruction instruction = {isRom, false, TwoBytes, uint8_t(operand & 0x00FF), uint8_t((operand & 0xFF00) >>8), 0x00, 0x0000};
            _instructions.push_back(instruction);
        }

        return success;
    }

    uint16_t parseNativeExpression(std::string input)
    {
        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

        // Replace labels
        for(int i=0; i<_labels.size(); i++)
        {
            size_t pos = input.find(_labels[i]._name);
            while(pos != std::string::npos)
            {
                input.replace(pos, _labels[i]._name.size(), std::to_string(_labels[i]._address >>1));
                pos = input.find(_labels[i]._name, pos + _labels[i]._name.size());
            }
        }

        // Replace equates
        for(int i=0; i<_equates.size(); i++)
        {
            size_t pos = input.find(_equates[i]._name);
            while(pos != std::string::npos)
            {
                input.replace(pos, _equates[i]._name.size(), std::to_string(_equates[i]._operand));
                pos = input.find(_equates[i]._name, pos + _equates[i]._name.size());
            }
        }

        // Parse expression and return with a result
        return Expression::parse((char*)input.c_str());
    }

    bool handleNativeOperand(const std::string& input, uint8_t& operand)
    {
        if(input.find_first_of("[]") != std::string::npos) return false;
        if(input.find("++") != std::string::npos) return false;
        if(input.find_first_of("+-*/()") != std::string::npos)
        {
            // Parse expression and return with a result
            operand = uint8_t(parseNativeExpression(input));
            return true;
        }

        Label label;
        if(searchLabel(input, label))
        {
            operand = uint8_t((label._address >>1) & 0x00FF);
            return true;
        }

        Equate equate;
        if(searchEquate(input, equate))
        {
            operand = uint8_t(equate._operand);
            return true;
        }

        return Expression::stringToU8(input, operand);
    }

    bool handleNativeInstruction(const std::vector<std::string>& tokens, int tokenIndex, uint8_t& opcode, uint8_t& operand)
    {
        std::string input, token;

        // Pre-process
        for(int j=tokenIndex; j<tokens.size(); j++)
        {
            // Strip comments
            if(tokens[j].find_first_of(";#") != std::string::npos) break;

            // Concatenate
            input += tokens[j];
        }

        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

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

    void packByteCodeBuffer(void)
    {
        // Pack instructions
        ByteCode byteCode;
        for(int i=0; i<_instructions.size(); i++)
        {
            switch(_instructions[i]._byteSize)
            {
                case OneByte:
                {
                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = _instructions[i]._isCustomAddress;
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);
                }
                break;

                case TwoBytes:
                {
                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = _instructions[i]._isCustomAddress;
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);

                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = false;
                    byteCode._data = _instructions[i]._operand0;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);
                }
                break;

                case ThreeBytes:
                {
                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = _instructions[i]._isCustomAddress;
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);

                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = false;
                    byteCode._data = _instructions[i]._operand0;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);

                    byteCode._isRomAddress = _instructions[i]._isRomAddress;
                    byteCode._isCustomAddress = false;
                    byteCode._data = _instructions[i]._operand1;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);
                }
                break;
            }
        }

        // Append call table
        if(_callTable  &&  _callTableEntries.size())
        {
            // _callTable grows downwards, pointer is 2 bytes below the bottom of the table by the time we get here
            for(int i=int(_callTableEntries.size())-1; i>=0; i--)
            {
                int end = int(_callTableEntries.size()) - 1;
                byteCode._isRomAddress = false;
                byteCode._isCustomAddress = (i == end) ?  true : false;
                byteCode._data = _callTableEntries[i]._address & 0x00FF;
                byteCode._address = _callTable + (end-i)*2 + 2;
                _byteCode.push_back(byteCode);

                byteCode._isRomAddress = false;
                byteCode._isCustomAddress = false;
                byteCode._data = (_callTableEntries[i]._address & 0xFF00) >>8;
                byteCode._address = _callTable + (end-i)*2 + 3;
                _byteCode.push_back(byteCode);
            }
        }
    }

    std::vector<std::string> tokenise(const std::string& text, char c)
    {
        std::vector<std::string> result;
        const char* str = text.c_str();

        do
        {
            const char *begin = str;

            while(*str  &&  *str != c) str++;

            if(str > begin) result.push_back(std::string(begin, str));
        }
        while (*str++ != 0);

        return result;
    }

    std::vector<std::string> tokeniseLine(std::string& line)
    {
        std::string token = "";
        bool delimiterStart = true;
        bool stringStart = false;
        enum DelimiterState {WhiteSpace, Quotes};
        DelimiterState delimiterState = WhiteSpace;
        std::vector<std::string> tokens;

        for(int i=0; i<=line.size(); i++)
        {
            // End of line is a delimiter for white space
            if(i == line.size())
            {
                if(delimiterState != Quotes)
                {
                    delimiterState = WhiteSpace;
                    delimiterStart = false;
                }
                else
                {
                    break;
                }
            }
            else
            {
                // White space delimiters
                if(strchr(" \n\r\f\t\v", line[i]))
                {
                    if(delimiterState != Quotes)
                    {
                        delimiterState = WhiteSpace;
                        delimiterStart = false;
                    }
                }
                // String delimiters
                else if(strchr("\'\"", line[i]))
                {
                    delimiterState = Quotes;
                    stringStart = !stringStart;
                }
            }

            // Build token
            switch(delimiterState)
            {
                case WhiteSpace:
                {
                    // Don't save delimiters
                    if(delimiterStart)
                    {
                        if(!strchr(" \n\r\f\t\v", line[i])) token += line[i];
                    }
                    else
                    {
                        if(token.size()) tokens.push_back(token);
                        delimiterStart = true;
                        token = "";
                    }
                }
                break;

                case Quotes:
                {
                    // Save delimiters as well as chars
                    if(stringStart)
                    {
                        token += line[i];
                    }
                    else
                    {
                        token += line[i];
                        tokens.push_back(token);
                        delimiterState = WhiteSpace;
                        stringStart = false;
                        token = "";
                    }
                }
                break;
            }
        }

        return tokens;
    }

    bool handleInclude(const std::vector<std::string>& tokens, const std::string& lineToken, int lineIndex, std::vector<std::string>& includeLineTokens)
    {
        // Check include syntax
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Assembler::handleInclude() : Bad %%include statement : '%s' : on line %d\n", lineToken.c_str(), lineIndex);
            return false;
        }

        std::ifstream infile(std::string("./vCPU/" +  tokens[1]));            
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::handleInclude() : Failed to open file : '%s'\n", tokens[1].c_str());
            return false;
        }

        // Collect lines from include file
        int numLines = 0;
        std::string includeLineToken;
        while(!infile.eof())
        {
            std::getline(infile, includeLineToken);
            includeLineTokens.push_back(includeLineToken);

            if(!infile.good() &&  !infile.eof())
            {
                fprintf(stderr, "Assembler::handleInclude() : Bad lineToken : '%s' : in %s on line %d\n", includeLineToken.c_str(), tokens[1].c_str(), numLines+1);
                return false;
            }

            numLines++;
        }

        return true;
    }

    bool handleMacros(const std::vector<Macro>& macros, std::vector<std::string>& lineTokens)
    {
        // Incomplete macros
        for(int i=0; i<macros.size(); i++)
        {
            if(!macros[i]._complete)
            {
                fprintf(stderr, "Assembler::handleMacros() : Bad macro : missing 'ENDM' : on line %d\n", macros[i]._startLine);
                return false;
            }
        }

        // Delete original macros
        int prevMacrosSize = 0;
        for(int i=0; i<macros.size(); i++)
        {
            lineTokens.erase(lineTokens.begin() + macros[i]._startLine - prevMacrosSize, lineTokens.begin() + macros[i]._endLine + 1 - prevMacrosSize);
            prevMacrosSize += macros[i]._endLine - macros[i]._startLine + 1;
        }

        // Find and expand macro
        int macroInstanceId = 0;
        for(int m=0; m<macros.size(); m++)
        {
            bool macroMissing = true;
            bool macroMissingParams = true;
            Macro macro = macros[m];

            for(auto itLine=lineTokens.begin(); itLine!=lineTokens.end();)
            {
                // Lines containing only white space are skipped
                std::string lineToken = *itLine;
                size_t nonWhiteSpace = lineToken.find_first_not_of("  \n\r\f\t\v");
                if(nonWhiteSpace == std::string::npos)
                {
                    ++itLine;
                    continue;
                }

                // Tokenise current line
                std::vector<std::string> tokens = tokeniseLine(lineToken);

                // Find macro
                bool macroSuccess = false;
                for(int t=0; t<tokens.size(); t++)
                {
                    if(tokens[t] == macro._name)
                    {
                        macroMissing = false;
                        if(tokens.size() - t > macro._params.size())
                        {
                            macroMissingParams = false;
                            std::vector<std::string> labels;
                            std::vector<std::string> macroLines;

                            // Create substitute lines
                            for(int ml=0; ml<macro._lines.size(); ml++)
                            {
                                // Tokenise macro line
                                std::vector<std::string> mtokens =  tokeniseLine(macro._lines[ml]);

                                // Save labels
                                size_t nonWhiteSpace = macro._lines[ml].find_first_not_of("  \n\r\f\t\v");
                                if(nonWhiteSpace == 0) labels.push_back(mtokens[0]);

                                // Replace parameters
                                for(int mt=0; mt<mtokens.size(); mt++)
                                {
                                    for(int p=0; p<macro._params.size(); p++)
                                    {
                                        if(mtokens[mt] == macro._params[p]) mtokens[mt] = tokens[t + 1 + p];
                                    }
                                }

                                // New macro line using any existing label
                                std::string macroLine = (t > 0  &&  ml == 0) ? tokens[0] : "";

                                // Append to macro line
                                for(int mt=0; mt<mtokens.size(); mt++)
                                {
                                    // Don't prefix macro labels with a space
                                    if(nonWhiteSpace != 0  ||  mt != 0) macroLine += " ";

                                    macroLine += mtokens[mt];
                                }

                                macroLines.push_back(macroLine);
                            }

                            // Insert substitute lines
                            for(int ml=0; ml<macro._lines.size(); ml++)
                            {
                                // Delete macro caller
                                if(ml == 0) itLine = lineTokens.erase(itLine);

                                // Each instance of a macro's labels are made unique
                                for(int i=0; i<labels.size(); i++)
                                {
                                    size_t labelFoundPos = macroLines[ml].find(labels[i]);
                                    if(labelFoundPos != std::string::npos) macroLines[ml].insert(labelFoundPos + labels[i].size(), std::to_string(macroInstanceId));
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
                fprintf(stderr, "Assembler::handleMacros() : Missing macro call : %s : on line %d\n", macro._name.c_str(), macro._startLine);
                return false;
            }

            if(macroMissingParams)
            {
                fprintf(stderr, "Assembler::handleMacros() : Missing macro parameters : %s : on line %d\n", macro._name.c_str(), macro._startLine);
                return false;
            }
        }

        return true;
    }

    bool handleMacroStart(const std::vector<std::string>& tokens, Macro& macro, int lineIndex)
    {
        // Check macro syntax
        if(tokens.size() < 2)
        {
            fprintf(stderr, "Assembler::handleMacroStart() : Bad macro : missing name : on line %d\n", lineIndex);
            return false;
        }                    

        macro._name = tokens[1];
        macro._startLine = lineIndex - 1;

        // Save params
        for(int i=2; i<tokens.size(); i++) macro._params.push_back(tokens[i]);

        return true;
    }

    bool handleMacroEnd(std::vector<Macro>& macros, Macro& macro, int lineIndex)
    {
        // Check for duplicates
        for(int i=0; i<macros.size(); i++)
        {
            if(macro._name == macros[i]._name)
            {
                fprintf(stderr, "Assembler::handleMacroEnd() : Bad macro : duplicate name : %s : on line %d\n", macro._name.c_str(), lineIndex);
                return false;
            }
        }
        macro._endLine = lineIndex - 1;
        macro._complete = true;
        macros.push_back(macro);

        macro._name = "";
        macro._startLine = 0;
        macro._endLine = 0;
        macro._lines.clear();
        macro._params.clear();
        macro._complete = false;

        return true;
    }

    bool preProcess(std::vector<std::string>& lineTokens, bool doMacros)
    {
        Macro macro;
        std::vector<Macro> macros;
        bool buildingMacro = false;

        for(auto itLine=lineTokens.begin(); itLine != lineTokens.end();)
        {
            // Lines containing only white space are skipped
            std::string lineToken = *itLine;
            size_t nonWhiteSpace = lineToken.find_first_not_of("  \n\r\f\t\v");
            if(nonWhiteSpace == std::string::npos)
            {
                ++itLine;
                continue;
            }

            int tokenIndex = 0;
            bool includeFound = false;
            int lineIndex = int(itLine - lineTokens.begin()) + 1;

            // Tokenise current line
            std::vector<std::string> tokens = tokeniseLine(lineToken);

            // Valid pre-processor commands
            if(tokens.size() > 0)
            {
                Expression::strToUpper(tokens[0]);

                // Include
                if(tokens[0] == "%INCLUDE")
                {  
                    std::vector<std::string> includeLineTokens;
                    if(!handleInclude(tokens, lineToken, lineIndex, includeLineTokens)) return false;

                    // Recursively include everything in order
                    if(!preProcess(includeLineTokens, false))
                    {
                        fprintf(stderr, "Assembler::preProcess() : Bad include file : '%s'\n", tokens[1].c_str());
                        return false;
                    }

                    // Remove original include line and replace with include text
                    itLine = lineTokens.erase(itLine);
                    itLine = lineTokens.insert(itLine, includeLineTokens.begin(), includeLineTokens.end());
                    includeFound = true;
                }

                // Build macro
                if(doMacros)
                {
                    if(tokens[0] == "%MACRO")
                    {
                        if(!handleMacroStart(tokens, macro, lineIndex)) return false;

                        buildingMacro = true;
                    }
                    else if(buildingMacro  &&  tokens[0] == "%ENDM")
                    {
                        if(!handleMacroEnd(macros, macro, lineIndex)) return false;
                        buildingMacro = false;
                    }
                    if(buildingMacro  &&  tokens[0] != "%MACRO")
                    {
                        macro._lines.push_back(lineToken);
                    }
                }
            }

            if(!includeFound) ++itLine;
        }

        // Handle complete macros
        if(doMacros  &&  !handleMacros(macros, lineTokens)) return false;

        return true;
    }

    bool parseGprintfFormat(const std::string& format, const std::vector<std::string>& variables, std::vector<Gprintf::Var>& vars, std::vector<std::string>& subs)
    {
        const char* fmt = format.c_str();
        std::string sub;
        char chr;

        int width = 0, index = 0;
        bool foundToken = false;

        while(chr = *fmt++)
        {
            if(index + 1 > variables.size()) return false;

            if(chr == '%'  ||  foundToken)
            {
                foundToken = true;
                Gprintf::Type type = Gprintf::Int;
                sub += chr;

                switch(chr)
                {
                    case '0':
                    {
                        // Maximum field width of 16 digits
                        width = std::stoi(fmt) % 17;
                    }
                    break;

                    case 'c': type = Gprintf::Chr; break;
                    case 'd': type = Gprintf::Int; break;
                    case 'b': type = Gprintf::Bin; break;
                    case 'q':
                    case 'o': type = Gprintf::Oct; break;
                    case 'x': type = Gprintf::Hex; break;
                    case 's': type = Gprintf::Str; break;
                }

                if(chr == 'c' || chr == 'd' || chr == 'b' || chr == 'q' || chr == 'o' || chr == 'x' || chr == 's')
                {
                    foundToken = false;
                    Gprintf::Var var = {false, type, width, 0x0000, variables[index++]};
                    vars.push_back(var);
                    subs.push_back(sub);
                    sub.erase();
                    width = 0;
                }
            }
        }

        return true;
    }

    bool createGprintf(ParseType parse, const std::string& lineToken, int lineNumber)
    {
        std::string input = lineToken;
        Expression::strToUpper(input);

        if(input.find("GPRINTF") != std::string::npos)
        {
            size_t openBracket = lineToken.find_first_of("(");
            size_t closeBracket = lineToken.find_first_of(")", openBracket+1);
            bool brackets = (openBracket != std::string::npos  &&  closeBracket != std::string::npos  &&  (closeBracket - openBracket > 2));

            if(brackets)
            {
                size_t quote1 = lineToken.find_first_of("\"", openBracket+1);
                size_t quote2 = lineToken.find_first_of("\"", quote1+1);
                bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 0));

                if(quotes)
                {
                    if(parse == MnemonicPass)
                    {
                        std::string formatText = lineToken.substr(quote1+1, quote2 - (quote1+1));
                        std::string variableText = lineToken.substr(quote2+1, closeBracket - (quote2+1));

                        std::vector<Gprintf::Var> vars;
                        std::vector<std::string> subs;
                        std::vector<std::string> variables = tokenise(variableText, ',');
                        parseGprintfFormat(formatText, variables, vars, subs);

                        Gprintf gprintf = {false, _currentAddress, lineNumber, lineToken, formatText, vars, subs};
                        _gprintfs.push_back(gprintf);
                    }

                    return true;
                }
            }

            fprintf(stderr, "Assembler::createGprintf() : Bad gprintf format : '%s' : on line %d\n", lineToken.c_str(), lineNumber);
            return false;
        }

        return false;
    }

    bool getGprintfString(int index, std::string& gstring)
    {
        const Gprintf& gprintf = _gprintfs[index % _gprintfs.size()];
        gstring = gprintf._format;
   
        size_t subIndex = 0;
        for(int i=0; i<gprintf._vars.size(); i++)
        {
            char token[256];

            // Use indirection if required
            uint16_t data = (gprintf._vars[i]._indirect) ? Cpu::getRAM(gprintf._vars[i]._data) | (Cpu::getRAM(gprintf._vars[i]._data+1) <<8) : gprintf._vars[i]._data;
            
            // Maximum field width of 16 digits
            uint8_t width = gprintf._vars[i]._width % 17;
            std::string fieldWidth = "%";
            if(width) fieldWidth = std::string("%0" + std::to_string(width));

            switch(gprintf._vars[i]._type)
            {
                case Gprintf::Chr: fieldWidth += "c"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Int: fieldWidth += "d"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Oct: fieldWidth += "o"; sprintf(token, fieldWidth.c_str(), data); break;
                case Gprintf::Hex: fieldWidth += "x"; sprintf(token, fieldWidth.c_str(), data); break;

                // Strings are always indirect and assume that length is first byte
                case Gprintf::Str:
                {
                    data = gprintf._vars[i]._data;
                    uint8_t length = Cpu::getRAM(data) & 0xFF; // maximum length of 256
                    for(int j=0; j<length; j++) token[j] = Cpu::getRAM(data + j + 1);
                    token[length] = 0;
                }
                break;

                case Gprintf::Bin:
                {
                    for(int j=width-1; j>=0; j--)
                    {
                        token[width-1 - j] = '0' + ((data >> j) & 1);
                        if(j == 0) token[width-1 + 1] = 0;
                    }
                }
                break;
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

    bool parseGprintfs(void)
    {
        for(int i = 0; i<_gprintfs.size(); i++)
        {
            for(int j = 0; j<_gprintfs[i]._vars.size(); j++)
            {
                bool success = false;
                uint16_t data = 0x0000;
                std::string token = _gprintfs[i]._vars[j]._var;
        
                // Strip white space
                token.erase(remove_if(token.begin(), token.end(), isspace), token.end());
                _gprintfs[i]._vars[j]._var = token;

                // Check for indirection
                size_t asterisk = token.find_first_of("*");
                if(asterisk != std::string::npos)
                {
                    _gprintfs[i]._vars[j]._indirect = true;
                    token = token.substr(asterisk+1);
                }

                success = Expression::stringToU16(token, data);
                if(!success)
                {
                    std::vector<std::string> tokens;
                    tokens.push_back(token);

                    // Search equates
                    Equate equate;
                    Label label;
                    if(success = searchEquates(tokens, 0, equate))
                    {
                        data = equate._operand;
                    }
                    // Search labels
                    else if(success = searchLabels(tokens, 0, label))
                    {
                        data = label._address;
                    }
                }

                if(!success)
                {
                    fprintf(stderr, "Assembler::parseGprintfs() : Error in gprintf(), missing label or equate : '%s' : in %s on line %d\n", token.c_str(), _gprintfs[i]._lineToken.c_str(), _gprintfs[i]._lineNumber);
                    _gprintfs.erase(_gprintfs.begin() + i);
                    return false;
                }

                _gprintfs[i]._vars[j]._data = data;
            }
        }

        return true;
    }

    void printGprintfStrings(void)
    {
        if(_gprintfs.size())
        {
            uint16_t vPC = (Cpu::getRAM(0x0017) <<8) | Cpu::getRAM(0x0016);

            for(int i=0; i<_gprintfs.size(); i++)
            {
                if(vPC == _gprintfs[i]._address)
                {
                    // Emulator can cycle many times for one CPU cycle, so make sure gprintf is displayed only once
                    if(!_gprintfs[i]._displayed)
                    {
                        std::string gstring;
                        getGprintfString(i, gstring);
                        fprintf(stderr, "gprintf() : address $%04X : %s\n", _gprintfs[i]._address, gstring.c_str());
                        _gprintfs[i]._displayed = true;
                    }
                }
                else
                {
                    _gprintfs[i]._displayed = false;;
                }
            }
        }
    }

    bool assemble(const std::string& filename, uint16_t startAddress)
    {
        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::assemble() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        _startAddress = startAddress;
        _currentAddress = _startAddress;
        _callTable = 0x0000;

        _byteCode.clear();
        _labels.clear();
        _equates.clear();
        _instructions.clear();
        _callTableEntries.clear();
        _gprintfs.clear();

        Loader::disableUploads(false);

        // Get file
        int numLines = 0;
        std::string lineToken;
        std::vector<std::string> lineTokens;
        while(!infile.eof())
        {
            std::getline(infile, lineToken);
            lineTokens.push_back(lineToken);

            if(!infile.good() &&  !infile.eof())
            {
                fprintf(stderr, "Assembler::assemble() : Bad lineToken : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), numLines+1);
                return false;
            }

            numLines++;
        }

        // Pre-processor
        if(!preProcess(lineTokens, true)) return false;

        numLines = int(lineTokens.size());

        // The mnemonic pass we evaluate all the equates and labels, the code pass is for the opcodes and operands
        for(int parse=MnemonicPass; parse<NumParseTypes; parse++)
        {
            for(int line=0; line<numLines; line++)
            {
                lineToken = lineTokens[line];

                // Lines containing only white space are skipped
                size_t nonWhiteSpace = lineToken.find_first_not_of("  \n\r\f\t\v");
                if(nonWhiteSpace == std::string::npos) continue;

                int tokenIndex = 0;

                // Tokenise current line
                std::vector<std::string> tokens = tokeniseLine(lineToken);

                // Comments
                if(tokens.size() > 0  &&  tokens[0].find_first_of(";#") != std::string::npos) continue;

                // Gprintf lines are skipped
                if(createGprintf(ParseType(parse), lineToken, line+1)) continue;

                // Starting address, labels and equates
                if(nonWhiteSpace == 0)
                {
                    if(tokens.size() >= 2)
                    {
                        EvaluateResult result = evaluateEquates(tokens, (ParseType)parse, tokenIndex);
                        if(result == NotFound)
                        {
                            fprintf(stderr, "Assembler::assemble() : Missing equate : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                            return false;
                        }
                        else if(result == Duplicate)
                        {
                            fprintf(stderr, "Assembler::assemble() : Duplicate equate : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
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
                            fprintf(stderr, "Assembler::assemble() : Can't use a reserved word in a label : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                            return false;
                        }
                        else if(result == Duplicate)
                        {
                            fprintf(stderr, "Assembler::assemble() : Duplicate label : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                            return false;
                        }
                    }

                    // On to the next token even if we failed with this one
                    if(tokens.size() > 1) tokenIndex++;
                }

                // Opcode
                bool operandValid = false;
                InstructionType instructionType = getOpcode(tokens[tokenIndex++]);
                uint8_t opcode = instructionType._opcode;
                uint8_t branch = instructionType._branch;
                ByteSize byteSize = instructionType._byteSize;
                OpcodeType opcodeType = instructionType._opcodeType;
                Instruction instruction = {false, false, byteSize, opcode, 0x00, 0x00, _currentAddress};

                if(byteSize == BadSize)
                {
                    fprintf(stderr, "Assembler::assemble() : Bad Opcode : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                    return false;
                }

                if(parse == CodePass)
                {
                    // Native NOP
                    if(opcodeType == Native  &&  opcode == 0x02)
                    {
                        operandValid = true;
                    }
                    // Missing operand
                    else if((byteSize == TwoBytes  ||  byteSize == ThreeBytes)  &&  tokens.size() <= tokenIndex)
                    {
                        fprintf(stderr, "Assembler::assemble() : Missing operand/s : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                        return false;
                    }

                    // First instruction inherits start address
                    if(_instructions.size() == 0)
                    {
                        instruction._address = _startAddress;
                        _currentAddress = _startAddress;
                    }

                    // Custom address
                    for(int i=0; i<_equates.size(); i++)
                    {
                        if(_equates[i]._name == tokens[0]  &&  _equates[i]._isCustomAddress)
                        {
                            _currentAddress = _equates[i]._operand;
                            instruction._isCustomAddress = true;
                            instruction._address = _currentAddress;
                        }
                    }

                    // Operand
                    switch(byteSize)
                    {
                        case OneByte:
                        {
                            _instructions.push_back(instruction);
                        }
                        break;

                        case TwoBytes:
                        {
                            uint8_t operand = 0x00;

                            // BRA
                            if(opcodeType == vCpu  &&  opcode == 0x90)
                            {
                                // Search for branch label
                                Label label;
                                if(searchLabels(tokens, tokenIndex, label))
                                {
                                    operandValid = true;
                                    operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                }
                                else
                                {
                                    fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                    return false;
                                }
                            }
                            // CALL
                            else if(opcodeType == vCpu  &&  opcode == 0xCF  &&  _callTable)
                            {
                                // Search for call label
                                Label label;
                                if(searchLabels(tokens, tokenIndex, label))
                                {
                                    // Search for address
                                    bool newLabel = true;
                                    uint16_t address = uint16_t(label._address);
                                    for(int i=0; i<_callTableEntries.size(); i++)
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
                                        operand = uint8_t(_callTable & 0x00FF);
                                        CallTableEntry entry = {operand, address};
                                        _callTableEntries.push_back(entry);
                                        _callTable -= 0x0002;
                                    }
                                }
                                else
                                {
                                    fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                    return false;
                                }
                            }
                                
                            // All other non native 2 byte instructions
                            if(opcodeType != Native  &&  !operandValid)
                            {
                                operandValid = Expression::stringToU8(tokens[tokenIndex], operand);
                                if(!operandValid)
                                {
                                    Label label;
                                    Equate equate;

                                    // String
                                    size_t quote1 = tokens[tokenIndex].find_first_of("'\"");
                                    size_t quote2 = tokens[tokenIndex].find_first_of("'\"", quote1+1);
                                    bool quotes = (quote1 != std::string::npos  &&  quote2 != std::string::npos  &&  (quote2 - quote1 > 1));
                                    if(quotes)
                                    {
                                        operand = uint8_t(tokens[tokenIndex][quote1+1]);
                                    }
                                    // Search labels
                                    else if(operandValid = searchEquates(tokens, tokenIndex, equate))
                                    {
                                        operand = uint8_t(equate._operand);
                                    }
                                    // Search equates
                                    else if(operandValid = searchLabels(tokens, tokenIndex, label))
                                    {
                                        operand = uint8_t(label._address);
                                    }
                                    else if(!operandValid)
                                    {
                                        fprintf(stderr, "Assembler::assemble() : Label/Equate error : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                        return false;
                                    }
                                }
                            }

                            // Native instructions
                            if(opcodeType == Native)
                            {
                                if(!operandValid)
                                {
                                    if(!handleNativeInstruction(tokens, tokenIndex, opcode, operand))
                                    {
                                        fprintf(stderr, "Assembler::assemble() : Native instruction is malformed : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                                        return false;
                                    }
                                }

                                instruction._isRomAddress = true;
                                instruction._opcode = opcode;
                                instruction._operand0 = uint8_t(operand & 0x00FF);
                                _instructions.push_back(instruction);

                                if(instruction._address < 0x2000)
                                {   
                                    uint16_t add = instruction._address>>1;
                                    uint8_t opc = Cpu::getROM(add, 0);
                                    uint8_t ope = Cpu::getROM(add, 1);
                                    if(instruction._opcode != opc  ||  instruction._operand0 != ope)
                                    {
                                        fprintf(stderr, "Assembler::assemble() : ROM Native instruction mismatch  : 0x%04X : ASM=0x%02X%02X : ROM=0x%02X%02X : on line %d\n", add, instruction._opcode, instruction._operand0, opc, ope, line+1);

                                        // Fix mismatched instruction?
                                        //instruction._opcode = opc;
                                        //instruction._operand0 = ope;
                                        //_instructions.back() = instruction;
                                    }
                                }
                            }
                            // Reserved assembler opcode DB, (define byte)
                            else if(opcodeType == ReservedDB  ||  opcodeType == ReservedDBR)
                            {
                                // Push first operand
                                instruction._isRomAddress = (opcodeType == ReservedDBR) ? true : false;
                                instruction._byteSize = OneByte;
                                instruction._opcode = uint8_t(operand & 0x00FF);
                                _instructions.push_back(instruction);

                                // Push any remaining operands using equate searches
                                if(!handleDefineByte(tokens, tokenIndex, operand, instruction._isRomAddress))
                                {
                                    fprintf(stderr, "Assembler::assemble() : Bad DB data : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                                    return false;
                                }
                            }
                            // Normal instructions
                            else
                            {
                                instruction._operand0 = operand;
                                _instructions.push_back(instruction);
                            }
                        }
                        break;
                            
                        case ThreeBytes:
                        {
                            // BCC
                            if(branch)
                            {
                                // Search for branch label
                                Label label;
                                uint8_t operand = 0x00;
                                if(searchLabels(tokens, tokenIndex, label))
                                {
                                    operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                }
                                else
                                {
                                    fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                    return false;
                                }

                                instruction._operand0 = branch;
                                instruction._operand1 = operand & 0x00FF;
                                _instructions.push_back(instruction);
                            }
                            // All other 3 byte instructions
                            else
                            {
                                uint16_t operand = 0x0000;
                                operandValid = Expression::stringToU16(tokens[tokenIndex], operand);
                                if(!operandValid)
                                {
                                    Label label;
                                    Equate equate;

                                    // Search labels
                                    if(operandValid = searchEquates(tokens, tokenIndex, equate))
                                    {
                                        operand = equate._operand;
                                    }
                                    // Search equates
                                    else if(operandValid = searchLabels(tokens, tokenIndex, label))
                                    {
                                        operand = label._address;
                                    }
                                    else if(!operandValid)
                                    {
                                        fprintf(stderr, "Assembler::assemble() : Label/Equate error : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                        return false;
                                    }
                                }

                                // Reserved assembler opcode DW, (define word)
                                if(opcodeType == ReservedDW  ||  opcodeType == ReservedDWR)
                                {
                                    // Push first operand
                                    instruction._isRomAddress = (opcodeType == ReservedDWR) ? true : false;
                                    instruction._byteSize = TwoBytes;
                                    instruction._opcode   = uint8_t(operand & 0x00FF);
                                    instruction._operand0 = uint8_t((operand & 0xFF00) >>8);
                                    _instructions.push_back(instruction);

                                    // Push any remaining operands using equate searches
                                    if(!handleDefineWord(tokens, tokenIndex, operand, instruction._isRomAddress))
                                    {
                                        fprintf(stderr, "Assembler::assemble() : Bad DW data : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                                        return false;
                                    }
                                }
                                // Normal instructions
                                else
                                {
                                    instruction._operand0 = uint8_t(operand & 0x00FF);
                                    instruction._operand1 = uint8_t((operand & 0xFF00) >>8);
                                    _instructions.push_back(instruction);
                                }
                            }
                        }
                        break;
                    }
                }

                // Check for page boundary crossings
                if(parse == CodePass)
                {
                    static uint16_t customAddress = 0x0000;
                    if(instruction._isCustomAddress) customAddress = instruction._address;

                    uint16_t oldAddress = (instruction._isRomAddress) ? customAddress + ((_currentAddress & 0x00FF)>>1) : _currentAddress;
                    _currentAddress += byteSize;
                    uint16_t newAddress = (instruction._isRomAddress) ? customAddress + ((_currentAddress & 0x00FF)>>1) : _currentAddress;
                    if((oldAddress >>8) != (newAddress >>8))
                    {
                        fprintf(stderr, "Assembler::assemble() : Page boundary compromised : %04X : %04X : '%s' : in %s on line %d\n", oldAddress, newAddress, lineToken.c_str(), filename.c_str(), line+1);
                        return false;
                    }
                }
                else
                {
                    _currentAddress += byteSize;
                }
            }              
        }

        // Pack byte code buffer from instruction buffer
        packByteCodeBuffer();

        // Parse gprintf labels, equates and expressions
        if(!parseGprintfs()) return false;

        return true;
    }
}
