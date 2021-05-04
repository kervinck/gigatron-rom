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
        _pragmas["_enable6BitAudioEmu_"] = {"_enable6BitAudioEmu_", ENABLE6BITAUDIOEMU};

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
            fprintf(stderr, "Pragmas::CODEROMTYPE() : 'Main:%d' : syntax error, use _codeRomType_ <\"ROM TYPE\"> : %s\n", codeLineIndex + 1, input.c_str());
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
            fprintf(stderr, "Pragmas::RUNTIMEPATH() : 'Main:%d' : syntax error, use _runtimePath_ <\"Path to runtime\"> : %s\n", codeLineIndex + 1, input.c_str());
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
            fprintf(stderr, "Pragmas::RUNTIMESTART() : 'Main:%d' : syntax error, use _runtimeStart_ <address> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::RUNTIMESTART() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Pragmas::RUNTIMESTART() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), input.c_str());
            return false;
        }

        Compiler::setRuntimeStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_UPPER_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
        }

        // String work area needs to be updated, (return old work areas and get new ones)
        for(int i=0; i<NUM_STR_WORK_AREAS; i++)
        {
            uint16_t strWorkArea;
            Memory::giveFreeRAM(Compiler::getStrWorkArea(i), USER_STR_SIZE + 2);
            if(!Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, address, strWorkArea))
            {
                fprintf(stderr, "Pragmas::RUNTIMESTART() : 'Main:%d' : setting new String Work Area failed : %s\n", codeLineIndex + 1, input.c_str());
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
            fprintf(stderr, "Pragmas::USERCODESTART() : 'Main:%d' : syntax error, use _userCodeStart_ <address> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::USERCODESTART() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Pragmas::USERCODESTART() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), input.c_str());
            return false;
        }

        // Programmer wants to use video memory as code space
        if(address >= 0x0800  &&  address < 0x7FA0  &&  (address & 0x00FF) < 0x00A0) Memory::invertFreeRAM();

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
            fprintf(stderr, "Pragmas::ARRAYSSTART() : 'Main:%d' : syntax error, use _arraysStart_ <address> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::ARRAYSSTART() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Pragmas::ARRAYSSTART() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), input.c_str());
            return false;
        }

        Compiler::setArraysStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_UPPER_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
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
            fprintf(stderr, "Pragmas::STRINGSSTART() : 'Main:%d' : syntax error, use _stringsStart_ <address> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::STRINGSSTART() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Pragmas::STRINGSSTART() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), input.c_str());
            return false;
        }

        Compiler::setStringsStart(address);

        // Re-initialise memory manager for 64K
        if(address >= RAM_UPPER_START  &&  Memory::getSizeRAM() != RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
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
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : 'Main:%d' : syntax error, use _stringWorkArea_ <address> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t strWorkArea = uint16_t(std::lround(addrNumeric._value));
        if(strWorkArea < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), input.c_str());
            return false;
        }

        // String work area needs to be updated, (return old work areas and get new ones)
        Memory::giveFreeRAM(Compiler::getStrWorkArea(0), USER_STR_SIZE + 2);
        if(!Memory::takeFreeRAM(strWorkArea, USER_STR_SIZE + 2))
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : 'Main:%d' : setting new string work area failed : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }
        Compiler::setStrWorkArea(strWorkArea, 0);
        Memory::giveFreeRAM(Compiler::getStrWorkArea(1), USER_STR_SIZE + 2);
        if(!Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, strWorkArea, strWorkArea))
        {
            fprintf(stderr, "Pragmas::STRINGWORKAREA() : 'Main:%d' : setting new string work area failed : %s\n", codeLineIndex + 1, input.c_str());
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
            fprintf(stderr, "Pragmas::TEMPVARSIZE() : 'Main:%d' : syntax error, use '_tempVarSize_ <size>' : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Expression::Numeric sizeNumeric;
        std::string sizeOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], sizeOperand, sizeNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::TEMPVARSIZE() : 'Main:%d' : syntax error in size field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint8_t size = uint8_t(std::lround(sizeNumeric._value));
        if(size < 2  ||  size > 16)
        {
            fprintf(stderr, "Pragmas::TEMPVARSIZE() : 'Main:%d' : size field must be in the range 2 to 16, found %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
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
            fprintf(stderr, "Pragmas::CODEOPTIMISETYPE() : 'Main:%d' : syntax error, use _codeOptimiseType_ <size/speed> : %s\n", codeLineIndex + 1, input.c_str());
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

        fprintf(stderr, "Pragmas::CODEOPTIMISETYPE() : 'Main:%d' : syntax error, _use codeOptimiseType_ <'size'/'speed'> : %s\n", codeLineIndex + 1, input.c_str());

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
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : 'Main:%d' : syntax error, use _maxNumSprites_ <max num sprites> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        // Max num sprites
        Expression::Numeric maxNumNumeric;
        std::string maxNumOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], maxNumOperand, maxNumNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : 'Main:%d' : syntax error in max num field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t maxNumSprites = uint16_t(std::lround(maxNumNumeric._value));
        if(Compiler::getRuntimeStart() < RAM_UPPER_START  &&  maxNumSprites > MAX_NUM_SPRITES_LO)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : 'Main:%d' : maximum number of sprites for 32K RAM is limited to %d, found %s : %s\n", codeLineIndex + 1, MAX_NUM_SPRITES_LO, tokens[0].c_str(), input.c_str());
            return false;
        }

        if(Compiler::getRuntimeStart() >= RAM_UPPER_START  &&  maxNumSprites > MAX_NUM_SPRITES_HI)
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : 'Main:%d' : maximum number of sprites for 64K RAM is limited to %d, found %s : %s\n", codeLineIndex + 1, MAX_NUM_SPRITES_HI, tokens[0].c_str(), input.c_str());
            return false;
        }

        // Allocate RAM for sprite's LUT now, otherwise more than 48 sprites in a 32K RAM system can fail
        uint16_t lutAddress;
        int lutSize = int(maxNumSprites) * 2;
        if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, Compiler::getRuntimeStart(), lutAddress))
        {
            fprintf(stderr, "Pragmas::MAXNUMSPRITES() : 'Main:%d' : not enough RAM for sprites LUT of size %d : %s\n", codeLineIndex + 1, lutSize, input.c_str());
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
            fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : syntax error, use _spriteStripeChunks_ <num chunks>, <optional minimum address>, <optional ascending/descending> : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        // Number of chunks
        Expression::Numeric chunksNumeric;
        std::string chunksOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], chunksOperand, chunksNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : syntax error in num chunks field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
            return false;
        }
        uint16_t spriteStripeChunks = uint16_t(std::lround(chunksNumeric._value));
        if(spriteStripeChunks > SPRITE_STRIPE_CHUNKS_HI)
        {
            fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : num chunks field can not be larger than %d, found %s : %s\n", codeLineIndex + 1, SPRITE_STRIPE_CHUNKS_HI, tokens[0].c_str(), input.c_str());
            return false;
        }

        Compiler::setSpriteStripeChunks(spriteStripeChunks);

        // RAM minimum address
        if(tokens.size() >= 2)
        {
            Expression::Numeric addrNumeric;
            std::string addrOperand;
            if(Compiler::parseStaticExpression(codeLineIndex, tokens[1], addrOperand, addrNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : syntax error in address field %s : %s\n", codeLineIndex + 1, tokens[0].c_str(), input.c_str());
                return false;
            }
            uint16_t minAddress = uint16_t(std::lround(addrNumeric._value));
            if(minAddress < DEFAULT_EXEC_ADDRESS)
            {
                fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : address field must be above &h%04x, found %s : %s\n", codeLineIndex + 1, DEFAULT_EXEC_ADDRESS, tokens[1].c_str(), input.c_str());
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
                fprintf(stderr, "Pragmas::SPRITESTRIPECHUNKS() : 'Main:%d' : search direction field must be 'ascending or descending', found '%s' : %s\n", codeLineIndex + 1, tokens[2].c_str(), input.c_str());
                return false;
            }
        }

        return true;
    }

    bool ENABLE6BITAUDIOEMU(const std::string& input, int codeLineIndex, size_t foundPos)
    {
#ifdef STAND_ALONE
        UNREFERENCED_PARAM(foundPos);
#endif

        if(Compiler::getCodeRomType() < Cpu::ROMv5a)
        {
            fprintf(stderr, "Pragmas::ENABLE6BITAUDIOEMU() : 'Main:%d' : version error, '_enable6BitAudioEmu_ <ON/OFF>' only works with ROMv5a or greater; use '_codeRomType_ ROMv5a' : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

#ifndef STAND_ALONE
        std::string pragma = input.substr(foundPos);
        Expression::stripNonStringWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma, ',', false, true);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Pragmas::ENABLE6BITAUDIOEMU() : 'Main:%d' : syntax error, use '_enable6BitAudioEmu_ <ON/OFF>' : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        // ON/OFF
        bool enable = false;
        Expression::strToUpper(tokens[0]);
        if(tokens[0] == "ON")       enable = true;
        else if(tokens[0] == "OFF") enable = false;
        else
        {
            fprintf(stderr, "Pragmas::ENABLE6BITAUDIOEMU() : 'Main:%d' : syntax error, use '_enable6BitAudioEmu_ <ON/OFF>' : %s\n", codeLineIndex + 1, input.c_str());
            return false;
        }

        Cpu::enable6BitSound(Cpu::ROMv5a, enable);
        return true;
#else
        fprintf(stderr, "Pragmas::ENABLE6BITAUDIOEMU() : 'Main:%d' : syntax error, '_enable6BitAudioEmu_ <ON/OFF>', only works in emulation mode : %s\n", codeLineIndex + 1, input.c_str());
        return false;
#endif
    }
}