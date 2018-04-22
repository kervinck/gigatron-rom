#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "cpu.h"
#include "assembler.h"


#define BRANCH_ADJUSTMENT 2


namespace Assembler
{
    enum ParseType {Labels=0, Instructions, NumParseTypes};
    enum ByteSize {BadSize=-1, OneByte, TwoBytes, ThreeBytes, NumByteSizes};

    struct Label
    {
        uint8_t _address;
        std::string _name;
    };

    struct Equate
    {
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
    };

    uint16_t _byteCount = 0;
    uint16_t _startAddress = DEFAULT_START_ADDRESS;

    std::vector<uint8_t> _byteCode;
    std::vector<Label> _labels;
    std::vector<Equate> _equates;
    std::vector<Mutable> _mutables;
    std::vector<Instruction> _instructions;


    uint16_t getStartAddress(void) {return _startAddress;}


    // Returns true when finished
    bool getNextAssembledByte(uint8_t& data)
    {
        if(_byteCount >= _byteCode.size())
        {
            _byteCount = 0;
            return true;
        }

        data = _byteCode[_byteCount++];
        //fprintf(stderr, "%04X  %02X\n", _startAddress + _byteCount-1, data);
        return false;
    }    

    bool stringToU8(std::string& token, uint8_t& result)
    {
        if(token.size() < 1  ||  token.size() > 4) return false;

        bool isHex = false;
        if(token.size() > 2) isHex = token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'X' || token.c_str()[1] == 'x');

        // Decimal
        if(bool(isdigit(token.c_str()[0])) == true  &&  isHex == false)
        {
            if(token.size() > 3) return false;
            for(int i=0; i<token.size(); i++)
            {
                if(bool(isdigit(token.c_str()[i])) == false) return false;
            }

            result = uint8_t(strtol(&token.c_str()[0], NULL, 10));
            return true;
        }

        // $ hex
        if(token.c_str()[0] == '$')
        {
            result = uint8_t(strtol(&token.c_str()[1], NULL, 16));
            return true;
        }

        // 0x hex
        if(isHex == true)
        {
            result = uint8_t(strtol(&token.c_str()[2], NULL, 16));
            return true;
        }

        return false;
    }

    bool stringToU16(std::string& token, uint16_t& result)
    {
        if(token.size() < 1  ||  token.size() > 6) return false;

        bool isHex = false;
        if(token.size() > 2) isHex = token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'X' || token.c_str()[1] == 'x');

        // Decimal
        if(bool(isdigit(token.c_str()[0])  &&  isHex == false) == true)
        {
            if(token.size() > 5) return false;
            for(int i=0; i<token.size(); i++)
            {
                if(bool(isdigit(token.c_str()[i])) == false) return false;
            }

            result = uint16_t(strtol(&token.c_str()[0], NULL, 10));
            return true;
        }

        // $ hex
        if(token.c_str()[0] == '$')
        {
            result = uint16_t(strtol(&token.c_str()[1], NULL, 16));
            return true;
        }

        // 0x hex
        if(isHex == true)
        {
            result = uint16_t(strtol(&token.c_str()[2], NULL, 16));
            return true;
        }

        return false;
    }

    ByteSize getOpcode(const std::string& token, uint8_t& opcode, uint8_t& branch)
    {
        branch = 0x00;

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

        if(token == "BEQ")   {opcode = 0x35; branch = 0x3F; return ThreeBytes;}
        if(token == "BNE")   {opcode = 0x35; branch = 0x72; return ThreeBytes;}
        if(token == "BLT")   {opcode = 0x35; branch = 0x50; return ThreeBytes;}
        if(token == "BGT")   {opcode = 0x35; branch = 0x4D; return ThreeBytes;}
        if(token == "BLE")   {opcode = 0x35; branch = 0x56; return ThreeBytes;}
        if(token == "BGE")   {opcode = 0x35; branch = 0x53; return ThreeBytes;}

        return BadSize;
    }


    bool assemble(const std::string& filename, uint16_t startAddress)
    {
        std::ifstream infile(filename);
        if(infile.is_open()  == false) return false;

        _startAddress = startAddress;
        _byteCode.clear();
        _labels.clear();
        _instructions.clear();

        // Parse the file twice, the first pass we collect all the labels and their offsets, the second pass is for the instructions
        for(int parse=Labels; parse<NumParseTypes; parse++)
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
                        fprintf(stderr, "Bad lineToken %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                        parseError = true;
                        break;
                    }

                    // Lines containing only white space are skipped
                    size_t found = lineToken.find_first_not_of("  \n\r");
                    if(found == std::string::npos)
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

                    // Starting address, labels and comments
                    if(found == 0)
                    {
                        // Comments
                        char c = tokens[tokenIndex].c_str()[0];
                        if(c == '/'  ||  c == ';'  ||  c == ','  ||  c == '*'  ||  c == '#')
                        {
                            line++;
                            continue;
                        }

                        // Starting address
                        bool success = stringToU16(tokens[tokenIndex], startAddress);
                        if(success == true  &&  startAddress >= DEFAULT_START_ADDRESS  &&  startAddress < RAM_SIZE)
                        {
                            _startAddress = startAddress;
                        }
                        // Labels/Equates/Mutables
                        else
                        {
                            // Equates
                            if(tokens.size() == 3)
                            {
                                if(tokens[1] == "EQU"  ||  tokens[1] == "equ")
                                {
                                    if(parse == Labels)
                                    {
                                        uint16_t operand = 0x0000;
                                        if(stringToU16(tokens[2], operand) == false)
                                        {
                                            fprintf(stderr, "Bad EQU %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                                            parseError = true;
                                            break;
                                        }
                                        Equate equate = {operand, tokens[0]};
                                        _equates.push_back(equate);
                                    }

                                    line++;
                                    continue;
                                }
                            }
                            
                            // Mutables, (labels for self modifying code)
                            if(parse == Labels)
                            {
                                if(tokens[0].c_str()[0] == '.')
                                {
                                    size_t found = tokens[0].find_first_of(",");
                                    if(found != std::string::npos)
                                    {
                                        uint8_t offset = uint8_t(tokens[0][found+1] - '0');
                                        if(offset > 9) offset = 1;  
                                        Mutable _mutable = {offset, uint16_t(_startAddress + totalByteSize), tokens[0].substr(0, found)};
                                        _mutables.push_back(_mutable);
                                    }
                                }
                                // Labels
                                else
                                {
                                    Label label = {uint8_t(totalByteSize), tokens[tokenIndex]}; // offset is filled in during first pass
                                    _labels.push_back(label);
                                }
                            }
                        }

                        // On to the next token even if we failed with this one
                        tokenIndex++;
                    }

                    // Opcode
                    if(tokens.size() > tokenIndex)
                    {
                        uint8_t opcode = 0x00, branch = 0x00;
                        ByteSize byteSize = getOpcode(tokens[tokenIndex++], opcode, branch);
                        if(byteSize == BadSize)
                        {
                            fprintf(stderr, "Bad Opcode %s in %s on line %d\n", lineToken.c_str(), filename.c_str(), line);
                            parseError = true;
                            break;
                        }

                        switch(byteSize)
                        {
                            case OneByte:    totalByteSize+=1;  break;
                            case TwoBytes:   totalByteSize+=2;  break;
                            case ThreeBytes: totalByteSize+=3;  break;
                        }

                        if(parse == Instructions)
                        {
                            Instruction instruction = {byteSize, opcode, 0x00, 0x00};

                            // Operand
                            if(tokens.size() > tokenIndex  &&  byteSize > OneByte)
                            {
                                switch(byteSize)
                                {
                                    case TwoBytes:   
                                    {
                                        uint8_t operand;
                                        // BRA
                                        if(opcode == 0x90)
                                        {
                                            for(int i=0; i<_labels.size(); i++)
                                            {
                                                if(_labels[i]._name == tokens[tokenIndex]) operand = _labels[i]._address - BRANCH_ADJUSTMENT;
                                            }
                                        }
                                        // All other 2 byte instructions
                                        else
                                        {
                                            bool success = stringToU8(tokens[tokenIndex], operand);
                                            if(success == false)
                                            {
                                                // Search equates
                                                bool failed = true;
                                                for(int i=0; i<_equates.size(); i++)
                                                {
                                                    if(_equates[i]._name == tokens[tokenIndex])
                                                    {
                                                        operand = uint8_t(_equates[i]._operand);
                                                        failed = false;
                                                    }
                                                }

                                                // Search mutables
                                                for(int i=0; i<_mutables.size(); i++)
                                                {
                                                    if(_mutables[i]._name == tokens[tokenIndex])
                                                    {
                                                        operand = uint8_t(_mutables[i]._address) + _mutables[i]._offset;
                                                        failed = false;
                                                    }
                                                }

                                                if(failed == true)
                                                {
                                                    fprintf(stderr, "stringToU8(): Bad token %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                                    parseError = true;
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        instruction._operand0 = operand;
                                    }
                                    break;
                            
                                    case ThreeBytes:
                                    {
                                        // BCC
                                        if(branch)
                                        {
                                            uint8_t operand = 0x00;
                                            for(int i=0; i<_labels.size(); i++)
                                            {
                                                if(_labels[i]._name == tokens[tokenIndex]) operand = _labels[i]._address - BRANCH_ADJUSTMENT;
                                            }
                                            instruction._operand0 = branch;
                                            instruction._operand1 = operand & 0x00FF;
                                        }
                                        // All other 3 byte instructions
                                        else
                                        {
                                            uint16_t operand;
                                            bool success = stringToU16(tokens[tokenIndex], operand);
                                            if(success == false)
                                            {
                                                // Search equates
                                                bool failed = true;
                                                for(int i=0; i<_equates.size(); i++)
                                                {
                                                    if(_equates[i]._name == tokens[tokenIndex])
                                                    {
                                                        operand = _equates[i]._operand;
                                                        failed = false;
                                                    }        
                                                }

                                                // Search mutables
                                                for(int i=0; i<_mutables.size(); i++)
                                                {
                                                    if(_mutables[i]._name == tokens[tokenIndex])
                                                    {
                                                        operand = _mutables[i]._address + _mutables[i]._offset;
                                                        failed = false;
                                                    }
                                                }

                                                if(failed == true)
                                                {
                                                    fprintf(stderr, "stringToU16(): Bad token %s in %s on line %d\n", tokens[tokenIndex].c_str(), filename.c_str(), line+1);
                                                    parseError = true;
                                                    break;
                                                }
                                            }

                                            instruction._operand0 = operand & 0x00FF;
                                            instruction._operand1 = (operand & 0xFF00) >>8;
                                        }
                                    }
                                    break;
                                }
                            }

                            _instructions.push_back(instruction);
                        }
                    }

                    line++;
                }              

                if(infile.eof() == true) break;

                if(parseError == true  ||  infile.bad() == true  ||  infile.fail() == true) return false;
            }
        }

        // Pack byte code buffer from instruction buffer
        for(int i=0; i<_instructions.size(); i++)
        {
            switch(_instructions[i]._byteSize)
            {
                case OneByte:    _byteCode.push_back(_instructions[i]._opcode); break;
                case TwoBytes:   _byteCode.push_back(_instructions[i]._opcode); _byteCode.push_back(_instructions[i]._operand0); break;
                case ThreeBytes: _byteCode.push_back(_instructions[i]._opcode); _byteCode.push_back(_instructions[i]._operand0); _byteCode.push_back(_instructions[i]._operand1); break;
            }
        }

        return true;
    }
}
