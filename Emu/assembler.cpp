#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "cpu.h"
#include "editor.h"
#include "assembler.h"


#define BRANCH_ADJUSTMENT   2
#define RESERVED_OPCODE_DB  0x00
#define RESERVED_OPCODE_DW  0x01


namespace Assembler
{
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary, NumNumericTypes};
    enum ParseType {FirstPass=0, SecondPass, NumParseTypes};
    enum ByteSize {BadSize=-1, OneByte, TwoBytes, ThreeBytes, NumByteSizes};
    enum EvaluateResult {Failed=-1, NotFound, Found, NumEvaluateResults};

    struct Label
    {
        uint16_t _address;
        std::string _name;
    };

    struct Equate
    {
        bool _customAddress;
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
        ByteSize _byteSize;
        uint8_t _opcode;
        uint8_t _operand0;
        uint8_t _operand1;
        uint16_t _address;
    };


    uint16_t _byteCount = 0;
    uint16_t _callTable = 0x0000;
    uint16_t _startAddress = DEFAULT_START_ADDRESS;

    std::vector<Label> _labels;
    std::vector<Equate> _equates;
    std::vector<Mutable> _mutables;
    std::vector<Instruction> _instructions;
    std::vector<ByteCode> _byteCode;

    bool _binaryChars[256] = {false};
    bool _octalChars[256] = {false};
    bool _decimalChars[256] = {false};
    bool _hexaDecimalChars[256] = {false};


    uint16_t getStartAddress(void) {return _startAddress;}


    void initialise(void)
    {
        bool* b = _binaryChars;      b['0']=1; b['1']=1;
        bool* o = _octalChars;       o['0']=1; o['1']=1; o['2']=1; o['3']=1; o['4']=1; o['5']=1; o['6']=1; o['7']=1;
        bool* d = _decimalChars;     d['0']=1; d['1']=1; d['2']=1; d['3']=1; d['4']=1; d['5']=1; d['6']=1; d['7']=1; d['8']=1; d['9']=1;
        bool* h = _hexaDecimalChars; h['0']=1; h['1']=1; h['2']=1; h['3']=1; h['4']=1; h['5']=1; h['6']=1; h['7']=1; h['8']=1; h['9']=1; h['A']=1; h['B']=1; h['C']=1; h['D']=1; h['E']=1; h['F']=1;
    }

    // Returns true when finished
    bool getNextAssembledByte(ByteCode& byteCode)
    {
        if(_byteCount >= _byteCode.size())
        {
            _byteCount = 0;
            return true;
        }

        byteCode = _byteCode[_byteCount++];
        static uint16_t address = byteCode._address;
        if(byteCode._address) address = byteCode._address;
        fprintf(stderr, "%04X  %02X\n", address, byteCode._data);
        address++;
        return false;
    }    

    void strToupper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return toupper(c);} );
    }

    NumericType getBase(const std::string& input, long& result)
    {
        bool success = true;
        std::string token = input;
        strToupper(token);
        
        // Hex
        if(token.size() >= 2  &&  token.c_str()[0] == '$')
        {
            for(int i=1; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success == true)
            {
                result = strtol(&token.c_str()[1], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Hex
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  token.c_str()[1] == 'X')
        {
            for(int i=2; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success == true)
            {
                result = strtol(&token.c_str()[2], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Octal
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'O' || token.c_str()[1] == 'Q'))
        {
            for(int i=2; i<token.size(); i++) success &= _octalChars[token.c_str()[i]];
            if(success == true)
            {
                result = strtol(&token.c_str()[2], NULL, 8);
                return Octal; 
            }
        }
        // Binary
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  token.c_str()[1] == 'B')
        {
            for(int i=2; i<token.size(); i++) success &= _binaryChars[token.c_str()[i]];
            if(success == true)
            {
                result = strtol(&token.c_str()[2], NULL, 2);
                return Binary; 
            }
        }
        // Decimal
        else
        {
            for(int i=0; i<token.size(); i++) success &= _decimalChars[token.c_str()[i]];
            if(success == true)
            {
                result = strtol(&token.c_str()[0], NULL, 10);
                return Decimal; 
            }
        }

        return BadBase;
    }

    bool stringToU8(const std::string& token, uint8_t& result)
    {
        if(token.size() < 1  ||  token.size() > 10) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = uint8_t(lResult);
        return true;
    }

    bool stringToU16(const std::string& token, uint16_t& result)
    {
        if(token.size() < 1  ||  token.size() > 18) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = uint16_t(lResult);
        return true;
    }

    ByteSize getOpcode(const std::string& token, uint8_t& opcode, uint8_t& branch)
    {
        branch = 0x00;

        // Gigatron vCPU instructions
        if(token == "ST")    {opcode = 0x5E; return TwoBytes;  }
        if(token == "STW")   {opcode = 0x2B; return TwoBytes;  }
        if(token == "STLW")  {opcode = 0xEC; return TwoBytes;  }
        if(token == "LD")    {opcode = 0x1A; return TwoBytes;  }
        if(token == "LDI")   {opcode = 0x59; return TwoBytes;  }
        if(token == "LDWI")  {opcode = 0x11; return ThreeBytes;}
        if(token == "LDW")   {opcode = 0x21; return TwoBytes;  }
        if(token == "LDLW")  {opcode = 0xEE; return TwoBytes;  }
        if(token == "ADDW")  {opcode = 0x99; return TwoBytes;  }
        if(token == "SUBW")  {opcode = 0xB8; return TwoBytes;  }
        if(token == "ADDI")  {opcode = 0xE3; return TwoBytes;  }
        if(token == "SUBI")  {opcode = 0xE6; return TwoBytes;  }
        if(token == "LSLW")  {opcode = 0xE9; return OneByte;   }
        if(token == "INC")   {opcode = 0x93; return TwoBytes;  }
        if(token == "ANDI")  {opcode = 0x82; return TwoBytes;  }
        if(token == "ANDW")  {opcode = 0xF8; return TwoBytes;  }
        if(token == "ORI")   {opcode = 0x88; return TwoBytes;  }
        if(token == "ORW")   {opcode = 0xFA; return TwoBytes;  }
        if(token == "XORI")  {opcode = 0x8C; return TwoBytes;  }
        if(token == "XORW")  {opcode = 0xFC; return TwoBytes;  }
        if(token == "PEEK")  {opcode = 0xAD; return OneByte;   }
        if(token == "DEEK")  {opcode = 0xF6; return OneByte;   }
        if(token == "POKE")  {opcode = 0xF0; return TwoBytes;  }
        if(token == "DOKE")  {opcode = 0xF3; return TwoBytes;  }
        if(token == "LUP")   {opcode = 0x7F; return TwoBytes;  }
        if(token == "BRA")   {opcode = 0x90; return TwoBytes;  }
        if(token == "CALL")  {opcode = 0xCF; return TwoBytes;  }
        if(token == "RET")   {opcode = 0xFF; return OneByte;   }
        if(token == "PUSH")  {opcode = 0x75; return OneByte;   }
        if(token == "POP")   {opcode = 0x63; return OneByte;   }
        if(token == "ALLOC") {opcode = 0xDF; return TwoBytes;  }
        if(token == "SYS")   {opcode = 0xB4; return TwoBytes;  }
        if(token == "DEF")   {opcode = 0xCD; return TwoBytes;  }

        // Gigatron vCPU branch instructions
        if(token == "BEQ")   {opcode = 0x35; branch = 0x3F; return ThreeBytes;}
        if(token == "BNE")   {opcode = 0x35; branch = 0x72; return ThreeBytes;}
        if(token == "BLT")   {opcode = 0x35; branch = 0x50; return ThreeBytes;}
        if(token == "BGT")   {opcode = 0x35; branch = 0x4D; return ThreeBytes;}
        if(token == "BLE")   {opcode = 0x35; branch = 0x56; return ThreeBytes;}
        if(token == "BGE")   {opcode = 0x35; branch = 0x53; return ThreeBytes;}

        // Reserved assembler opcodes
        if(token == "DB")    {opcode = RESERVED_OPCODE_DB; return TwoBytes;   }
        if(token == "DW")    {opcode = RESERVED_OPCODE_DW; return ThreeBytes; }

        return BadSize;
    }

    bool searchEquates(const std::string& token, Equate& equate)
    {
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
            if(parse == FirstPass)
            {
                uint16_t operand = 0x0000;
                if(stringToU16(tokens[2], operand) == false)
                {
                    // Search equates for a previous declaration
                    Equate equate;
                    if(searchEquates(tokens[2], equate) == false) return Failed;
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
                // Standard equates
                else
                {
                    Equate equate = {false, operand, tokens[0]};
                    _equates.push_back(equate);
                }
            }

            return Found;
        }

        return NotFound;
    }

    void EvaluateLabels(const std::vector<std::string>& tokens, int totalByteSize, int tokenIndex)
    {
        // Mutable labels
        if(tokens[0].c_str()[0] == '.')
        {
            size_t comma = tokens[0].find_first_of(",");
            if(comma != std::string::npos)
            {
                uint8_t offset = uint8_t(tokens[0][comma+1] - '0');
                if(offset > 9) offset = 1;  
                Mutable mutable_ = {offset, uint16_t(_startAddress + totalByteSize), tokens[0].substr(0, comma)};
                _mutables.push_back(mutable_);
            }
        }
        // Normal labels
        else
        {
            // Check equates for a custom start address
            uint16_t address = uint16_t(_startAddress + totalByteSize);
            for(int i=0; i<_equates.size(); i++)
            {
                if(_equates[i]._name == tokens[tokenIndex])
                {
                    _equates[i]._customAddress = true;
                    address = _equates[i]._operand;
                    break;
                }
            }

            Label label = {address, tokens[tokenIndex]}; // offset is filled in during first pass
            _labels.push_back(label);
        }
    }

    bool handleDefineByte(const std::vector<std::string>& tokens, int tokenIndex, uint8_t& operand)
    {
        bool success = false;

        for(int i=tokenIndex+1; i<tokens.size(); i++)
        {
            success = stringToU8(tokens[i], operand);
            if(success == false)
            {
                Equate equate;
                success = searchEquates(tokens[i], equate);
                if(success == true)
                {
                    operand = uint8_t(equate._operand);
                }
                else
                {
                    break;
                }
            }
            Instruction instruction = {OneByte, operand, 0x00, 0x00, 0x0000};
            _instructions.push_back(instruction);
        }

        return success;
    }

    bool handleDefineWord(const std::vector<std::string>& tokens, int tokenIndex, uint16_t& operand)
    {
        bool success = false;

        for(int i=tokenIndex+1; i<tokens.size(); i++)
        {
            success = stringToU16(tokens[i], operand);
            if(success == false)
            {
                Equate equate;
                success = searchEquates(tokens[i], equate);
                if(success == true)
                {
                    operand = equate._operand;
                }
                else
                {
                    break;
                }
            }
            Instruction instruction = {TwoBytes, uint8_t(operand & 0x00FF), uint8_t((operand & 0xFF00) >>8), 0x00, 0x0000};
            _instructions.push_back(instruction);
        }

        return success;
    }

    void packByteCodeBuffer(void)
    {
        ByteCode byteCode;
        for(int i=0; i<_instructions.size(); i++)
        {
            switch(_instructions[i]._byteSize)
            {
                case OneByte:
                {
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);
                }
                break;

                case TwoBytes:
                {
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);

                    byteCode._data = _instructions[i]._operand0;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);
                }
                break;

                case ThreeBytes:
                {
                    byteCode._data = _instructions[i]._opcode;
                    byteCode._address = _instructions[i]._address;
                    _byteCode.push_back(byteCode);

                    byteCode._data = _instructions[i]._operand0;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);

                    byteCode._data = _instructions[i]._operand1;
                    byteCode._address = 0x0000;
                    _byteCode.push_back(byteCode);
                }
                break;
            }
        }
    }

    bool assemble(const std::string& filename, uint16_t startAddress)
    {
        std::ifstream infile(filename);
        if(infile.is_open()  == false) return false;

        _startAddress = startAddress;
        _callTable = 0x0000;

        _byteCode.clear();
        _labels.clear();
        _equates.clear();
        _mutables.clear();
        _instructions.clear();

        // Parse the file twice, the first pass we evaluate all the equates and labels, the second pass is for the instructions
        for(int parse=FirstPass; parse<NumParseTypes; parse++)
        {
            int line = 0;
            int totalByteSize = 0;
            bool parseError = false;

            infile.clear(); 
            infile.seekg(0, infile.beg);
            while(infile.good() == true  &&  parseError == false)
            {
                std::string lineToken;

                while(parseError == false)
                {
                    std::getline(infile, lineToken);
                    if(infile.eof() == true) break;
                    if(infile.good() == false)
                    {
                        parseError = true;
                        fprintf(stderr, "Bad lineToken %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                        break;
                    }

                    // Lines containing only white space are skipped
                    size_t nonWhiteSpace = lineToken.find_first_not_of("  \n\r");
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
                    size_t comment = tokens[0].find_first_of(";*#/");
                    if(comment != std::string::npos)
                    {
                        line++;
                        continue;
                    }

                    // Starting address, labels and equates
                    if(nonWhiteSpace == 0)
                    {
                        if(tokens.size() >= 3)
                        {
                            EvaluateResult result = evaluateEquates(tokens, (ParseType)parse, tokenIndex);
                            if(result == Failed)
                            {
                                parseError = true;
                                fprintf(stderr, "Bad EQU %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                                break;
                            }
                            // Skip equate lines
                            else if(result == Found) 
                            {
                                line++;
                                continue;
                            }
                            
                            // Starting address or a label
                            bool success = stringToU16(tokens[tokenIndex], startAddress);
                            if(success == true  &&  startAddress >= DEFAULT_START_ADDRESS  &&  startAddress < RAM_SIZE)
                            {
                                _startAddress = startAddress;
                            }
                            else
                            {
                                // Labels
                                if(parse == FirstPass) EvaluateLabels(tokens, totalByteSize, tokenIndex);
                            }
                        }

                        // On to the next token even if we failed with this one
                        tokenIndex++;
                    }

                    // Opcode
                    uint8_t opcode = 0x00, branch = 0x00;
                    ByteSize byteSize = getOpcode(tokens[tokenIndex++], opcode, branch);
                    if(byteSize == BadSize)
                    {
                        parseError = true;
                        fprintf(stderr, "Bad Opcode %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
                        break;
                    }

                    switch(byteSize)
                    {
                        case OneByte:    totalByteSize+=1;  break;
                        case TwoBytes:   totalByteSize+=2;  break;
                        case ThreeBytes: totalByteSize+=3;  break;
                    }

                    if(parse == SecondPass)
                    {
                        Instruction instruction = {byteSize, opcode, 0x00, 0x00, 0x0000};

                        // Missing operand
                        if((byteSize == TwoBytes  ||  byteSize == ThreeBytes)  &&  tokens.size() <= tokenIndex)
                        {
                            parseError = true;
                            fprintf(stderr, "Missing operand/s: %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                            break;
                        }

                        // First instruction inherits start address
                        if(_instructions.size() == 0) instruction._address = _startAddress;

                        // Custom address
                        for(int i=0; i<_equates.size(); i++)
                        {
                            if(_equates[i]._name == tokens[0]  &&  _equates[i]._customAddress == true) instruction._address = _equates[i]._operand;
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
                                uint8_t operand;

                                // BRA
                                if(opcode == 0x90)
                                {
                                    // Search for branch label
                                    Label label;
                                    if(searchLabels(tokens[tokenIndex], label) == true)
                                    {
                                        operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                    }
                                    else
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Label missing: %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                        break;
                                    }

                                }
                                // CALL
                                else if(opcode == 0xCF  &&  _callTable)
                                {
                                    // Search for call label
                                    Label label;
                                    if(searchLabels(tokens[tokenIndex], label) == true)
                                    {
                                        // Found label, put it's address into the call table and point the call instruction to the call table
                                        operand = uint8_t(_callTable & 0x00FF);
                                        uint16_t address = uint16_t(label._address);
                                        Cpu::setRAM(_callTable, uint8_t(address & 0x00FF));
                                        Cpu::setRAM(_callTable+1, uint8_t((address & 0xFF00) >>8));
                                        _callTable -= 0x0002;
                                    }
                                    else
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Label missing: %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                        break;
                                    }
                                }
                                // All other 2 byte instructions
                                else
                                {
                                    bool success = stringToU8(tokens[tokenIndex], operand);
                                    if(success == false)
                                    {
                                        // Search equates
                                        Equate equate;
                                        success = searchEquates(tokens[tokenIndex], equate);
                                        operand = uint8_t(equate._operand);

                                        // Search mutables
                                        if(success == false)
                                        {
                                            Mutable mutable_;
                                            success = searchMutables(tokens[tokenIndex], mutable_);
                                            operand = uint8_t(mutable_._address) + mutable_._offset;
                                        }

                                        if(success == false)
                                        {
                                            parseError = true;
                                            fprintf(stderr, "Equate or Mutable missing: %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                            break;
                                        }
                                    }
                                }

                                // Reserved assembler opcode DB, (define byte)
                                if(opcode == RESERVED_OPCODE_DB)
                                {
                                    // Push first operand
                                    instruction._byteSize = OneByte;
                                    instruction._opcode   = uint8_t(operand & 0x00FF);
                                    _instructions.push_back(instruction);

                                    // Push any remaining operands using equate searches
                                    if(handleDefineByte(tokens, tokenIndex, operand) == false)
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Bad DB data: %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
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
                                    uint8_t operand;
                                    if(searchLabels(tokens[tokenIndex], label))
                                    {
                                        operand = uint8_t(label._address) - BRANCH_ADJUSTMENT;
                                    }
                                    else
                                    {
                                        parseError = true;
                                        fprintf(stderr, "Label missing: %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                        break;
                                    }

                                    instruction._operand0 = branch;
                                    instruction._operand1 = operand & 0x00FF;
                                    _instructions.push_back(instruction);
                                }
                                // All other 3 byte instructions
                                else
                                {
                                    uint16_t operand;
                                    bool success = stringToU16(tokens[tokenIndex], operand);
                                    if(success == false)
                                    {
                                        // Search equates
                                        Equate equate;
                                        success = searchEquates(tokens[tokenIndex], equate);
                                        operand = equate._operand;

                                        // Search mutables
                                        if(success == false)
                                        {
                                            Mutable mutable_;
                                            success = searchMutables(tokens[tokenIndex], mutable_);
                                            operand = mutable_._address + mutable_._offset;
                                        }

                                        if(success == false)
                                        {
                                            parseError = true;
                                            fprintf(stderr, "Equate or Mutable missing %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                            break;
                                        }
                                    }

                                    // Reserved assembler opcode DW, (define word)
                                    if(opcode == RESERVED_OPCODE_DW)
                                    {
                                        // Push first operand
                                        instruction._byteSize = TwoBytes;
                                        instruction._opcode   = uint8_t(operand & 0x00FF);
                                        instruction._operand0 = uint8_t((operand & 0xFF00) >>8);
                                        _instructions.push_back(instruction);

                                        // Push any remaining operands using equate searches
                                        if(handleDefineWord(tokens, tokenIndex, operand) == false)
                                        {
                                            parseError = true;
                                            fprintf(stderr, "Bad DW data: %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line+1);
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

                    line++;
                }              

                if(infile.eof() == true) break;

                if(parseError == true  ||  infile.bad() == true  ||  infile.fail() == true) return false;
            }
        }

        // Pack byte code buffer from instruction buffer
        packByteCodeBuffer();

        return true;
    }
}
