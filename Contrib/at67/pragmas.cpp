#include <cmath>

#include "memory.h"
#include "cpu.h"
#include "loader.h"
#include "expression.h"
#include "assembler.h"
#include "compiler.h"
#include "pragmas.h"


namespace Pragmas
{
    std::map<std::string, Pragma> _pragmas;


    std::map<std::string, Pragma>& getPragmas(void) {return _pragmas;}


    bool initialise(void)
    {
        // Pragmas
        _pragmas["_codeRomType_"]        = {"_codeRomType_",        CODEROMTYPE       };
        _pragmas["_runtimePath_"]        = {"_runtimePath_",        RUNTIMEPATH       };
        _pragmas["_runtimeStart_"]       = {"_runtimeStart_",       RUNTIMESTART      };
        _pragmas["_userCodeStart_"]      = {"_userCodeStart_",      USERCODESTART     };
        _pragmas["_arraysStart_"]        = {"_arraysStart_",        ARRAYSSTART       };
        _pragmas["_stringsStart_"]       = {"_stringsStart_",       STRINGSSTART      };
        _pragmas["_stringWorkArea_"]     = {"_stringWorkArea_",     STRINGWORKAREA    };
        _pragmas["_tempVarSize_"]        = {"_tempVarSize_",        TEMPVARSIZE       };
        _pragmas["_codeOptimiseType_"]   = {"_codeOptimiseType_",   CODEOPTIMISETYPE  };
        _pragmas["_arrayIndiciesOne_"]   = {"_arrayIndiciesOne_",   ARRAYINDICIESONE  };
        _pragmas["_maxNumSprites_"]      = {"_maxNumSprites_",      MAXNUMSPRITES     };
        _pragmas["_spriteStripeChunks_"] = {"_spriteStripeChunks_", SPRITESTRIPECHUNKS};

        return true;
    }


    bool findPragma(std::string code, const std::string& pragma, size_t& foundPos)
    {
        foundPos = code.find(pragma);
        if(foundPos != std::string::npos)
        {
            foundPos += pragma.size();
            return true;
        }
        return false;
    }

    PragmaResult handlePragmas(std::string& input, int codeLineIndex)
    {
        std::vector<std::string> tokens = Expression::tokenise(input, ' ', false);
        if(tokens.size() >= 1)
        {
            std::string token = tokens[0];
            Expression::stripNonStringWhitespace(token);

            if(_pragmas.find(token) == _pragmas.end()) return PragmaNotFound;

            // Handle pragma in input
            size_t foundPos;
            if(findPragma(token, _pragmas[token]._name, foundPos)  &&  _pragmas[token]._func)
            {
                bool success = _pragmas[token]._func(input, codeLineIndex, foundPos);
                if(success) return PragmaFound;
                
                return PragmaError;
            }
        }

        return PragmaNotFound;
    }


    // ********************************************************************************************
    // Pragmas
    // ********************************************************************************************
    bool CODEROMTYPE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        // Get rom type
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        Expression::strToUpper(pragma);
        if(Cpu::getRomTypeMap().find(pragma) == Cpu::getRomTypeMap().end())
        {
            fprintf(stderr, "Pragmas::CODEROMTYPE() : Syntax error, _codeRomType_ <\"ROM TYPE\">, in '%s' on line %d\n", input.c_str(), codeLineIndex + 1);
            return false;
        }

        Compiler::setCodeRomType(Cpu::getRomTypeMap()[pragma]);

        return true;
    }

    bool RUNTIMEPATH(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        if(pragma.size() < 3  ||  !Expression::isStringValid(pragma))
        {
            fprintf(stderr, "Pragmas::RUNTIMEPATH() : Syntax error, _runtimePath_ <\"Path to runtime\">, in '%s' on line %d\n", input.c_str(), codeLineIndex + 1);
            return false;
        }

        // Strip quotes
        std::string runtimePath = pragma;
        runtimePath.erase(0, 1);
        runtimePath.erase(runtimePath.size() - 1, 1);

        // Set build path
        Compiler::setBuildPath(runtimePath, Loader::getFilePath());

        return true;
    }

    bool RUNTIMESTART(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::RUNTIMESTART() : Syntax error, _runtimeStart_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);

        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Pragmas::RUNTIMESTART() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(),
                                                                                                                                                                          codeLineIndex);
            return false;
        }

        Compiler::setRuntimeStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_EXPANSION_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
            Memory::initialise();
        }

        // String work area needs to be updated, (return old work areas and get new ones)
        for(int i=0; i<NUM_STR_WORK_AREAS; i++)
        {
            uint16_t strWorkArea;
            Memory::giveFreeRAM(Compiler::getStrWorkArea(i), USER_STR_SIZE + 2);
            if(!Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, address, strWorkArea))
            {
                fprintf(stderr, "Pragmas::RUNTIMESTART() : Setting new String Work Area failed, in '%s' on line %d\n", input.c_str(), codeLineIndex);
                return false;
            }
            Compiler::setStrWorkArea(strWorkArea, i);
        }

        return true;
    }

    bool USERCODESTART(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::USERCODESTART() : Syntax error, _userCodeStart_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);

        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Pragmas::USERCODESTART() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(),
                                                                                                                                                                         codeLineIndex);
            return false;
        }

        Compiler::setUserCodeStart(address);

        return true;
    }

    bool ARRAYSSTART(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::ARRAYSSTART() : Syntax error, _arraysStart_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);

        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Pragmas::ARRAYSSTART() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(),
                                                                                                                                                                         codeLineIndex);
            return false;
        }

        Compiler::setArraysStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_EXPANSION_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
            Memory::initialise();
        }

        return true;
    }

    bool STRINGSSTART(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::STRINGSSTART() : Syntax error, _stringsStart_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);

        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Pragmas::STRINGSSTART() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(),
                                                                                                                                                                         codeLineIndex);
            return false;
        }

        Compiler::setStringsStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_EXPANSION_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
            Memory::initialise();
        }

        return true;
    }

    bool STRINGWORKAREA(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : Syntax error, _stringWorkArea_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        uint16_t strWorkArea = uint16_t(std::lround(addrNumeric._value));
        if(strWorkArea < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(),
                                                                                                                                                                            codeLineIndex);
            return false;
        }

        // String work area needs to be updated, (return old work areas and get new ones)
        Memory::giveFreeRAM(Compiler::getStrWorkArea(0), USER_STR_SIZE + 2);
        if(!Memory::takeFreeRAM(strWorkArea, USER_STR_SIZE + 2))
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : Setting new String Work Area failed, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }
        Compiler::setStrWorkArea(strWorkArea, 0);
        Memory::giveFreeRAM(Compiler::getStrWorkArea(1), USER_STR_SIZE + 2);
        if(!Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, strWorkArea, strWorkArea))
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : Setting new String Work Area failed, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }
        Compiler::setStrWorkArea(strWorkArea, 1);

        return true;
    }

    bool TEMPVARSIZE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::TEMPVARSIZE() : Syntax error, use '_tempVarSize_ <size>', in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric sizeNumeric;
        std::string sizeOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], sizeOperand, sizeNumeric);

        uint8_t size = uint8_t(std::lround(sizeNumeric._value));
        if(size < 2  ||  size > 16)
        {
            fprintf(stderr, "Pragmas::TEMPVARSIZE() : Size field must be in the range 2 to 16, found %s in '%s' on line %d\n", tokens[0].c_str(), input.c_str(), codeLineIndex);
            return false;
        }

        Compiler::setTempVarSize(size);

        return true;
    }

    bool CODEOPTIMISETYPE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::CODEOPTIMISETYPE() : Syntax error, _codeOptimiseType_ <size/speed>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        if(tokens[0] == "SIZE")
        {
            Compiler::setCodeOptimiseType(Compiler::CodeSize);
            return true;
        }
        else if(tokens[0] == "SPEED")
        {
            Compiler::setCodeOptimiseType(Compiler::CodeSpeed);
            return true;
        }

        fprintf(stderr, "Pragmas::CODEOPTIMISETYPE() : Syntax error, _codeOptimiseType_ <'size'/'speed'>, in '%s' on line %d\n", input.c_str(), codeLineIndex);

        return false;
    }

    bool ARRAYINDICIESONE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        UNREFERENCED_PARAM(input);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(codeLineIndex);

        Compiler::setArrayIndiciesOne(true);

        return true;
    }

    bool MAXNUMSPRITES(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false, true);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : Syntax error, _maxNumSprites_ <max num sprites>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        // Max num sprites
        Expression::Numeric maxNumNumeric;
        std::string maxNumOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], maxNumOperand, maxNumNumeric);
        uint16_t maxNumSprites = uint16_t(std::lround(maxNumNumeric._value));
        if(Compiler::getRuntimeStart() < RAM_EXPANSION_START  &&  maxNumSprites > MAX_NUM_SPRITES_LO)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : Maximum number of sprites for 32K RAM is limited to %d, found %s in '%s' on line %d\n", MAX_NUM_SPRITES_LO, tokens[0].c_str(),
                                                                                                                                                                           input.c_str(),
                                                                                                                                                                           codeLineIndex);
            return false;
        }

        if(Compiler::getRuntimeStart() >= RAM_EXPANSION_START  &&  maxNumSprites > MAX_NUM_SPRITES_HI)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : Maximum number of sprites for 64K RAM is limited to %d, found %s in '%s' on line %d\n", MAX_NUM_SPRITES_HI, tokens[0].c_str(),
                                                                                                                                                                           input.c_str(),
                                                                                                                                                                           codeLineIndex);
            return false;
        }

        // Allocate RAM for sprite's LUT now, otherwise more than 48 sprites in a 32K RAM system can fail
        uint16_t lutAddress;
        int lutSize = int(maxNumSprites) * 2;
        if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, Compiler::getRuntimeStart(), lutAddress))
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : Not enough RAM for sprites LUT of size %d\n", lutSize);
            return false;
        }
        Compiler::setSpritesAddrLutAddress(lutAddress);

        return true;
    }

    bool SPRITESTRIPECHUNKS(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false, true);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : Syntax error, _spriteStripeChunks_ <num chunks>, <optional minimum address>, <optional ascending/descending>, in '%s' on line %d\n",
                    input.c_str(), codeLineIndex);
            return false;
        }

        // Number of chunks
        Expression::Numeric chunksNumeric;
        std::string chunksOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], chunksOperand, chunksNumeric);
        uint16_t spriteStripeChunks = uint16_t(std::lround(chunksNumeric._value));
        if(spriteStripeChunks > SPRITE_STRIPE_CHUNKS_HI)
        {
            fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : Num chunks field can not be larger than %d, found %s in '%s' on line %d\n", SPRITE_STRIPE_CHUNKS_HI, tokens[0].c_str(),
                                                                                                                                                                  input.c_str(),
                                                                                                                                                                  codeLineIndex);
            return false;
        }

        Compiler::setSpriteStripeChunks(spriteStripeChunks);

        // RAM minimum address
        if(tokens.size() >= 2)
        {
            Expression::Numeric addrNumeric;
            std::string addrOperand;
            Compiler::parseExpression(codeLineIndex, tokens[1], addrOperand, addrNumeric);
            uint16_t minAddress = uint16_t(std::lround(addrNumeric._value));
            if(minAddress < DEFAULT_START_ADDRESS)
            {
                fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[1].c_str(),
                                                                                                                                                          input.c_str(),
                                                                                                                                                          codeLineIndex);
                return false;
            }

            Compiler::setSpriteStripeMinAddress(minAddress);
        }

        // RAM fit type
        if(tokens.size() == 3)
        {
            if(tokens[2] == "ASCENDING")
            {
                Compiler::setSpriteStripeFitType(Memory::FitAscending);
                return true;
            }
            else if(tokens[2] == "DESCENDING")
            {
                Compiler::setSpriteStripeFitType(Memory::FitDescending);
                return true;
            }
            else
            {
                fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : Search direction field must be 'ascending or descending', found '%s' in '%s' on line %d\n", tokens[2].c_str(),
                                                                                                                                                             input.c_str(),
                                                                                                                                                             codeLineIndex);
                return false;
            }
        }

        return true;
    }
}