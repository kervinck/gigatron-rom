#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "cpu.h"
#include "editor.h"
#include "loader.h"
#include "assembler.h"
#include "expression.h"


#define BRANCH_ADJUSTMENT    2


namespace Assembler
{
    enum ParseType {FirstPass=0, SecondPass, NumParseTypes};
    enum ByteSize {BadSize=-1, OneByte=1, TwoBytes=2, ThreeBytes=3};
    enum EvaluateResult {Failed=-1, NotFound, Found};
    enum OpcodeType {ReservedDB=0, ReservedDW, ReservedDBR, ReservedDWR, vCpu, Native};
    enum AddressMode {D_AC=0b00000000, X_AC=0b00000100, YD_AC=0b00001000, YX_AC=0b00001100, D_X=0b00010000, D_Y=0b00010100, D_OUT=0b00011000, YXpp_OUT=0b00011100};
    enum BusMode {D=0b00000000, RAM=0b00000001, AC=0b00000010, IN=0b00000011};

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

    struct Mutable
    {
        uint8_t _offset;
        uint16_t _address;
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


    uint16_t _byteCount = 0;
    uint16_t _callTable = 0x0000;
    uint16_t _startAddress = DEFAULT_START_ADDRESS;
    uint16_t _currentAddress = _startAddress;

    std::vector<Label> _labels;
    std::vector<Equate> _equates;
    std::vector<Mutable> _mutables;
    std::vector<Instruction> _instructions;
    std::vector<ByteCode> _byteCode;
    std::vector<CallTableEntry> _callTableEntries;

    uint16_t getStartAddress(void) {return _startAddress;}


    void initialise(void)
    {
    }

    // Returns true when finished
    bool getNextAssembledByte(ByteCode& byteCode)
    {
        if(_byteCount >= _byteCode.size())
        {
            fprintf(stderr, "\n");
            _byteCount = 0;
            return true;
        }

        static uint16_t address;
        if(_byteCount == 0) address = _startAddress;
        byteCode = _byteCode[_byteCount++];
        if(byteCode._isCustomAddress)
        {
            fprintf(stderr, "\n");
            address = byteCode._address;
        }
        std::string ramRom = (byteCode._isRomAddress) ? "ROM" : "RAM";
        if(!byteCode._isRomAddress  ||  (byteCode._isRomAddress  &&  address > 0x2000)) fprintf(stderr, "Assembler::getNextAssembledByte() : %s : %04X  %02X\n", ramRom.c_str(), address, byteCode._data);
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

    uint16_t parseExpression(std::string input)
    {
        // Strip white space
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());

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
            equate._operand = parseExpression(input);
            return true;
        }

        // Normal equate
        return searchEquate(tokens[tokenIndex], equate);
    }

    bool searchLabels(const std::string& token, Label& label)
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

    bool searchMutables(const std::string& token, Mutable& mutable_)
    {
        bool success = false;
        for(int i=0; i<_mutables.size(); i++)
        {
            if(token == _mutables[i]._name)
            {
                success = true;
                mutable_ = _mutables[i];
                break;
            }
        }

        return success;
    }

    EvaluateResult evaluateEquates(const std::vector<std::string>& tokens, ParseType parse, int tokenIndex)
    {
        if(tokens[1] == "EQU"  ||  tokens[1] == "equ")
        {
            static bool sortEquates = false;

            if(parse == FirstPass)
            {
                sortEquates = true;

                uint16_t operand = 0x0000;
                if(!Expression::stringToU16(tokens[2], operand))
                {
                    // Search equates for a previous declaration
                    Equate equate;
                    if(!searchEquates(tokens, 2, equate)) return Failed;
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
                    Equate equate = {false, operand, tokens[0]};
                    _equates.push_back(equate);
                }
            }
            else if(parse == SecondPass)
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

            return Found;
        }

        return NotFound;
    }

    void EvaluateLabels(const std::vector<std::string>& tokens, int tokenIndex)
    {
        // Mutable labels
        if(tokens[0].c_str()[0] == '.')
        {
            size_t comma = tokens[0].find_first_of(",");
            if(comma != std::string::npos)
            {
                uint8_t offset = uint8_t(tokens[0][comma+1] - '0');
                if(offset > 9) offset = 1;  
                Mutable mutable_ = {offset, uint16_t(_currentAddress), tokens[0].substr(0, comma)};
                _mutables.push_back(mutable_);
            }
        }
        else
        {
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
            Label label = {_currentAddress, tokens[tokenIndex]};
            _labels.push_back(label);
        }
    }

    bool handleDefineByte(const std::vector<std::string>& tokens, int tokenIndex, uint8_t& operand, bool isRom)
    {
        bool success = false;

        for(int i=tokenIndex+1; i<tokens.size(); i++)
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

    bool handleNativeEquate(const std::string& input, uint8_t& operand)
    {
        if(input.find_first_of("[]") != std::string::npos) return false;
        if(input.find("++") != std::string::npos) return false;
        if(input.find_first_of("+-*/()") != std::string::npos)
        {
            // Parse expression and return with a result
            operand = uint8_t(parseExpression(input));
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
                return handleNativeEquate(token, operand);
            }

            // y,D
            if(noBrackets  &&  oneComma)
            {
                token = input.substr(comma1+1, input.size() - (comma1+1));
                return handleNativeEquate(token, operand);
            }
        
            return false;                    
        }

        // Branch
        if(opcode >= 0xE4)
        {
            token = input;
            if(validBrackets) {opcode |= BusMode::RAM; token = input.substr(openBracket+1, closeBracket - (openBracket+1));}
            if(Expression::stringToU8(token, operand)) return true;

            Label label;
            if(searchLabels(token, label))
            {
                operand = uint8_t((label._address >>1) & 0x00FF);
                return true;
            }

            return false;
        }

        // IN or IN,[D]
        if(input.find("IN") != std::string::npos  ||  input.find("in") != std::string::npos)
        {
            opcode |= BusMode::IN;

            // IN,[D]
            if(validBrackets &&  oneComma  &&  comma1 < openBracket)
            {
                token = input.substr(openBracket+1, closeBracket - (openBracket+1));
                return handleNativeEquate(token, operand);
            }
            
            // IN
            return true;
        }

        // Read or write
        (opcode != 0xC0) ? opcode |= BusMode::RAM : opcode |= BusMode::AC;

        // D
        if(noBrackets && noCommas) return handleNativeEquate(input, operand);

        // [D] or [X]
        if(validBrackets  &&  noCommas)
        {
            token = input.substr(openBracket+1, closeBracket - (openBracket+1));
            if(token == "X"  ||  token == "x") {opcode |= AddressMode::X_AC; return true;}
            return handleNativeEquate(token, operand);
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
            if(noBrackets) {opcode &= 0xFC; return handleNativeEquate(token, operand);}

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
                return handleNativeEquate(token, operand);
            }

            return false;
        }

        // D,[Y,X] or D,[Y,X++]
        if(validBrackets  &&  twoCommas  &&  comma1 < openBracket  &&  comma2 > openBracket  &&  comma2 < closeBracket)
        {
            token = input.substr(0, comma1);
            if(!handleNativeEquate(token, operand)) return false;

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

    bool assemble(const std::string& filename, uint16_t startAddress)
    {
        std::ifstream infile(filename);
        if(!infile.is_open()) return false;

        _startAddress = startAddress;
        _currentAddress = _startAddress;
        _callTable = 0x0000;

        _byteCode.clear();
        _labels.clear();
        _equates.clear();
        _mutables.clear();
        _instructions.clear();
        _callTableEntries.clear();

        Loader::disableUploads(false);

        // Parse the file twice, the first pass we evaluate all the equates and labels, the second pass is for the instructions
        for(int parse=FirstPass; parse<NumParseTypes; parse++)
        {
            int line = 1;
            bool parseError = false;

            infile.clear(); 
            infile.seekg(0, infile.beg);
            while(infile.good()  &&  !parseError)
            {
                std::string lineToken;

                while(!parseError)
                {
                    std::getline(infile, lineToken);
                    if(infile.eof()) break;
                    if(!infile.good())
                    {
                        parseError = true;
                        fprintf(stderr, "Assembler::assemble() : Bad lineToken : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                        break;
                    }

                    // Lines containing only white space are skipped
                    size_t nonWhiteSpace = lineToken.find_first_not_of("  \n\r\f\t\v");
                    if(nonWhiteSpace == std::string::npos)
                    {
                        line++;
                        continue;
                    }

                    // Tokenise current line
                    int tokenIndex = 0;
                    std::stringstream strStream(lineToken);
                    std::istream_iterator<std::string> it(strStream);
                    std::istream_iterator<std::string> end;
                    std::vector<std::string> tokens(it, end);

                    // Comments
                    if(tokens.size() > 0  &&  tokens[0].find_first_of(";#") != std::string::npos)
                    {
                        line++;
                        continue;
                    }

                    // Starting address, labels and equates
                    if(nonWhiteSpace == 0)
                    {
                        if(tokens.size() >= 2)
                        {
                            EvaluateResult result = evaluateEquates(tokens, (ParseType)parse, tokenIndex);
                            if(result == Failed)
                            {
                                parseError = true;
                                fprintf(stderr, "Assembler::assemble() : Bad EQU : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                                break;
                            }
                            // Skip equate lines
                            else if(result == Found) 
                            {
                                line++;
                                continue;
                            }
                            
                            // Labels
                            if(parse == FirstPass) EvaluateLabels(tokens, tokenIndex);
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

                    if(byteSize == BadSize)
                    {
                        parseError = true;
                        fprintf(stderr, "Assembler::assemble() : Bad Opcode : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                        break;
                    }

                    if(parse == SecondPass)
                    {
                        Instruction instruction = {false, false, byteSize, opcode, 0x00, 0x00, _currentAddress};

                        // Native NOP
                        if(opcodeType == Native  &&  opcode == 0x02)
                        {
                            operandValid = true;
                        }
                        // Missing operand
                        else if((byteSize == TwoBytes  ||  byteSize == ThreeBytes)  &&  tokens.size() <= tokenIndex)
                        {
                            parseError = true;
                            fprintf(stderr, "Assembler::assemble() : Missing operand/s : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                            break;
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
                                    if(searchLabels(tokens[tokenIndex], label))
                                    {
                                        operandValid = true;
                                        operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                    }
                                    else
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line);
                                        break;
                                    }
                                }
                                // CALL
                                else if(opcodeType == vCpu  &&  opcode == 0xCF  &&  _callTable)
                                {
                                    // Search for call label
                                    Label label;
                                    if(searchLabels(tokens[tokenIndex], label))
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
                                        parseError = true;
                                        fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line);
                                        break;
                                    }
                                }
                                
                                // All other 2 byte instructions
                                if(!operandValid)
                                {
                                    operandValid = Expression::stringToU8(tokens[tokenIndex], operand);
                                    if(!operandValid)
                                    {
                                        // Search equates
                                        Equate equate;
                                        operandValid = searchEquates(tokens, tokenIndex, equate);
                                        operand = uint8_t(equate._operand);

                                        // Search mutables
                                        if(!operandValid)
                                        {
                                            Mutable mutable_;
                                            operandValid = searchMutables(tokens[tokenIndex], mutable_);
                                            operand = uint8_t(mutable_._address) + mutable_._offset;
                                        }

                                        if(opcodeType != Native  &&  !operandValid)
                                        {
                                            parseError = true;
                                            fprintf(stderr, "Assembler::assemble() : Equate/Mutable missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line);
                                            break;
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
                                            parseError = true;
                                            fprintf(stderr, "Assembler::assemble() : Native instruction is malformed : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                                            break;
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
                                            fprintf(stderr, "Assembler::assemble() : ROM Native instruction mismatch  : %04X : I=%02X%02X : R=%02X%02X : on line %d\n", add, instruction._opcode, instruction._operand0, opc, ope, line);
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
                                        parseError = true;
                                        fprintf(stderr, "Assembler::assemble() : Bad DB data : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                                        break;
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
                                    if(searchLabels(tokens[tokenIndex], label))
                                    {
                                        operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                    }
                                    else
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Assembler::assemble() : Label missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line);
                                        break;
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
                                        // Search equates
                                        Equate equate;
                                        operandValid = searchEquates(tokens, tokenIndex, equate);
                                        operand = equate._operand;

                                        // Search mutables
                                        if(!operandValid)
                                        {
                                            Mutable mutable_;
                                            operandValid = searchMutables(tokens[tokenIndex], mutable_);
                                            operand = mutable_._address + mutable_._offset;
                                        }

                                        if(!operandValid)
                                        {
                                            parseError = true;
                                            fprintf(stderr, "Assembler::assemble() : Equate/Mutable missing : '%s' : in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line);
                                            break;
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
                                            parseError = true;
                                            fprintf(stderr, "Assembler::assemble() : Bad DW data : '%s' : in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                                            break;
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
                    uint16_t oldAddress = _currentAddress;
                    _currentAddress += byteSize;
                    uint16_t newAddress = _currentAddress;
                    if((oldAddress >>8) != (newAddress >>8))
                    {
                        //parseError = true;
                        //fprintf(stderr, "Assembler::assemble() : Page boundary compromised : %04X : %04X : '%s' : in %s on line %d\n", oldAddress, newAddress, lineToken.c_str(), filename.c_str(), line);
                        //break;
                    }

                    line++;
                }              

                if(infile.eof()) break;

                if(parseError  ||  infile.bad()  ||  infile.fail()) return false;
            }
        }

        // Pack byte code buffer from instruction buffer
        packByteCodeBuffer();

        return true;
    }
}
