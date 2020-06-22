#include <ctime>
#include <random>

#include "memory.h"
#include "cpu.h"
#include "functions.h"
#include "operators.h"


namespace Functions
{
    enum EmitStringResult {SyntaxError, InvalidStringVar, ValidStringVar};

    std::mt19937_64 _randGenerator;

    std::map<std::string, std::string> _functions;
    std::map<std::string, std::string> _stringFunctions;


    std::map<std::string, std::string>& getFunctions(void)       {return _functions;      }
    std::map<std::string, std::string>& getStringFunctions(void) {return _stringFunctions;}


    bool initialise(void)
    {
        // Functions
        _functions["PEEK"  ] = "PEEK";
        _functions["DEEK"  ] = "DEEK";
        _functions["USR"   ] = "USR";
        _functions["RND"   ] = "RND";
        _functions["LEN"   ] = "LEN";
        _functions["GET"   ] = "GET";
        _functions["ABS"   ] = "ABS";
        _functions["SGN"   ] = "SGN";
        _functions["ASC"   ] = "ASC";
        _functions["STRCMP"] = "STRCMP";
        _functions["BCDCMP"] = "BCDCMP";
        _functions["VAL"   ] = "VAL";
        _functions["LUP"   ] = "LUP";
        _functions["ADDR"  ] = "ADDR";
        _functions["POINT" ] = "POINT";
        _functions["MIN"   ] = "MIN";
        _functions["MAX"   ] = "MAX";
        _functions["CLAMP" ] = "CLAMP";
        _functions["SWAP"  ] = "SWAP";

        // String functions
        _stringFunctions["CHR$"  ] = "CHR$";
        _stringFunctions["HEX$"  ] = "HEX$";
        _stringFunctions["HEXW$" ] = "HEXW$";
        _stringFunctions["LEFT$" ] = "LEFT$";
        _stringFunctions["RIGHT$"] = "RIGHT$";
        _stringFunctions["MID$"  ] = "MID$";
        _stringFunctions["STR$"  ] = "STR$";
        _stringFunctions["TIME$" ] = "TIME$";

        uint64_t timeSeed = time(NULL);
        std::seed_seq seedSequence{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
        _randGenerator.seed(seedSequence);

        return true;
    }


    EmitStringResult emitStringAddr(const std::string& token, const std::string& operand)
    {
        std::string strToken = token;
        Expression::stripNonStringWhitespace(strToken);
        if(strToken.back() == '$'  &&  Expression::isVarNameValid(strToken))
        {
            uint16_t srcAddr;
            int strIndexSrc = Compiler::findStr(strToken);
            if(strIndexSrc >= 0)
            {
                srcAddr = Compiler::getStringVars()[strIndexSrc]._address;
            }
            else
            {
                strIndexSrc = Compiler::findConst(strToken);
                if(strIndexSrc == -1) return SyntaxError;
                
                srcAddr = Compiler::getConstants()[strIndexSrc]._address;
            }

            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("STW", operand, false);

            return ValidStringVar;
        }

        return InvalidStringVar;
    }

    void getOrCreateString(const Expression::Numeric& numeric, std::string& name, uint16_t& addr, int& index)
    {
        switch(numeric._varType)
        {
            case Expression::String:
            {
                Compiler::getOrCreateConstString(numeric._text, index);
                name = Compiler::getStringVars()[index]._name;
                addr = Compiler::getStringVars()[index]._address;
            }
            break;

            case Expression::StrVar:
            {
                name = Compiler::getStringVars()[index]._name;
                addr = Compiler::getStringVars()[index]._address;
            }
            break;

            case Expression::Constant:
            {
                name = Compiler::getConstants()[index]._name;
                addr = Compiler::getConstants()[index]._address;
            }
            break;

            default: break;
        }
    }

    void handleConstantString(const Expression::Numeric& numeric, Compiler::ConstStrType constStrType, std::string& name, int& index)
    {
        switch(constStrType)
        {
            case Compiler::StrLeft:

            case Compiler::StrRight:
            {
                uint8_t length = uint8_t(std::lround(numeric._parameters[0]._value));
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, 0, index);
            }
            break;

            case Compiler::StrMid:
            {
                uint8_t offset = uint8_t(std::lround(numeric._parameters[0]._value));
                uint8_t length = uint8_t(std::lround(numeric._parameters[1]._value));
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, offset, index);
            }
            break;

            default: break;
        }

        name = Compiler::getStringVars()[index]._name;
        uint16_t srcAddr = Compiler::getStringVars()[index]._address;

        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("%PrintAcString", "", false);
        }
        else
        {
            uint16_t dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("%StringCopy", "", false);
        }
    }

    void handleStringParameter(Expression::Numeric& param)
    {
        // Literals
        if(param._varType == Expression::Number)
        {
            // 8bit
            if(param._value >=0  &&  param._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(param._value))), false);
            }
            // 16bit
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(param._value))), false);
            }

            return;
        }

        Operators::handleSingleOp("LDW", param);
    }


    // ********************************************************************************************
    // Functions
    // ********************************************************************************************
    void opcodeARR(Expression::Numeric& param)
    {
        // Can't call Operators::handleSingleOp() here, so special case it
        switch(param._varType)
        {
            // Temporary variable address
            case Expression::TmpVar:
            {
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(param._value))), false);
            }
            break;

            // User variable
            case Expression::IntVar:
            {
                Compiler::emitVcpuAsmUserVar("LDW", param, false);
            }
            break;

            // Literal or constant
            case Expression::Number:
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(param._value))), false);
            }
            break;

            default: break;
        }
    }
    Expression::Numeric IARR(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::IARR() : ARRAY() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        Compiler::getNextTempVar();

        int intSize = Compiler::getIntegerVars()[numeric._index]._intSize;
        uint16_t arrayPtr = Compiler::getIntegerVars()[numeric._index]._address;

        // Literal array index, (only optimise for 1d arrays)
        if(numeric._varType == Expression::Arr1Var8  &&  numeric._parameters.size()  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            std::string operand = Expression::wordToHexString(arrayPtr + uint16_t(numeric._parameters[0]._value*intSize));

            Compiler::emitVcpuAsm("LDWI", operand, false); 
            Compiler::emitVcpuAsm("PEEK", "",      false);

            Operators::createTmpVar(numeric);
        }
        else if(numeric._varType == Expression::Arr1Var16  &&  numeric._parameters.size()  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            std::string operand = Expression::wordToHexString(arrayPtr + uint16_t(numeric._parameters[0]._value*intSize));

            // Handle .LO and .HI
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  Compiler::emitVcpuAsm("LDWI", operand,          false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                case Expression::Int16High: Compiler::emitVcpuAsm("LDWI", operand + " + 1", false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                case Expression::Int16Both: Compiler::emitVcpuAsm("LDWI", operand,          false); Compiler::emitVcpuAsm("DEEK", "", false); break;

                default: break;
            }

            Operators::createTmpVar(numeric);
        }
        // Variable array index or 2d/3d array
        else
        {
            for(int i=0; i<int(numeric._parameters.size()); i++)
            {
                Expression::Numeric param = numeric._parameters[i];
                opcodeARR(param);
                Compiler::emitVcpuAsm("STW", "memIndex" + std::to_string(i), false);
            }

            // Handle 1d/2d/3d arrays
            switch(numeric._varType)
            {
                case Expression::Arr1Var8:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    Compiler::emitVcpuAsm("ADDW", "memIndex0", false);
                }
                break;

                case Expression::Arr2Var8:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert8Arr2d", false) : Compiler::emitVcpuAsm("CALL", "convert8Arr2dAddr", false);
                }
                break;

                case Expression::Arr3Var8:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert8Arr3d", false, codeLineIndex) : Compiler::emitVcpuAsm("CALL", "convert8Arr3dAddr", false);
                }
                break;

                case Expression::Arr1Var16:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    Compiler::emitVcpuAsm("ADDW", "memIndex0", false);
                    Compiler::emitVcpuAsm("ADDW", "memIndex0", false);
                }
                break;

                case Expression::Arr2Var16:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert16Arr2d", false) : Compiler::emitVcpuAsm("CALL", "convert16Arr2dAddr", false);
                }
                break;

                case Expression::Arr3Var16:
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                    (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert16Arr3d", false, codeLineIndex) : Compiler::emitVcpuAsm("CALL", "convert16Arr3dAddr", false);
                }
                break;

                default: break;
            }

            // Bytes
            if(numeric._varType >= Expression::Arr1Var8  &&  numeric._varType <= Expression::Arr3Var8)
            {
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            // Words, handle .LO and .HI
            else
            {
                switch(numeric._int16Byte)
                {
                    case Expression::Int16Low:  Compiler::emitVcpuAsm("PEEK", "",  false);                                           break;
                    case Expression::Int16High: Compiler::emitVcpuAsm("ADDI", "1", false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                    case Expression::Int16Both: Compiler::emitVcpuAsm("DEEK", "",  false);                                           break;

                    default: break;
                }
            }
        }

        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric SARR(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::SARR() : ARRAY() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }
        if(numeric._parameters.size() != 1)
        {
            fprintf(stderr, "Functions::SARR() : ARRAY() can only have 1 dimension : on line %d\n", codeLineIndex);
            return numeric;
        }

        Compiler::getNextTempVar();

        uint16_t arrayPtr = Compiler::getStringVars()[numeric._index]._address;

        // Literal array index
        if(numeric._parameters[0]._varType == Expression::Number)
        {
            std::string operand = Expression::wordToHexString(arrayPtr + uint16_t(numeric._parameters[0]._value)*2);
            Compiler::emitVcpuAsm("LDWI", operand, false);
            Compiler::emitVcpuAsm("DEEK", "",      false);
        }
        // Variable array index
        else
        {
            Expression::Numeric param = numeric._parameters[0];
            opcodeARR(param);
            Compiler::emitVcpuAsm("STW", "memIndex0", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
            Compiler::emitVcpuAsm("ADDW", "memIndex0", false);
            Compiler::emitVcpuAsm("ADDW", "memIndex0", false);
            Compiler::emitVcpuAsm("DEEK", "", false);
        }

        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        numeric._varType = Expression::Str2Var;

        return numeric;
    }

    Expression::Numeric PEEK(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::PEEK() : PEEK() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType == Expression::Number)
        {
            // Optimise for page 0
            if(numeric._value >= 0  && numeric._value <= 255)
            {
                Compiler::emitVcpuAsm("LD",  Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Operators::createTmpVar(numeric);
                return numeric;
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("PEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric DEEK(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::PEEK() : PEEK() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType == Expression::Number)
        {
            // Optimise for page 0
            if(numeric._value >= 0  && numeric._value <= 255)
            {
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Operators::createTmpVar(numeric);
                return numeric;
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("DEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric USR(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::USR() : USR() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType == Expression::Number)
        {
            if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("CALLI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("CALLI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
            else
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();

        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Operators::handleSingleOp("CALLI", numeric);
        }
        else
        {
            Operators::handleSingleOp("LDW", numeric);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false);
        }
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric RND(Expression::Numeric& numeric, int codeLineIndex)
    {
        UNREFERENCED_PARAM(codeLineIndex);

        bool useMod = true;
        if(numeric._varType == Expression::Number)
        {
            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                if(numeric._value == 0)
                {
                    std::uniform_int_distribution<uint16_t> distribution(0, 0xFFFF);
                    numeric._value = distribution(_randGenerator);
                }
                else
                {
                    std::uniform_int_distribution<uint16_t> distribution(0, uint16_t(numeric._value));
                    numeric._value = distribution(_randGenerator);
                }

                return numeric;
            }

            // RND(0) skips the MOD call and allows you to filter the output manually
            if(numeric._value == 0)
            {
                useMod = false;
            }
            else
            {
                (numeric._value > 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                 Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        if(useMod)
        {
            Operators::handleSingleOp("LDW", numeric);
            Compiler::emitVcpuAsm("%RandMod", "", false);
        }
        else
        {
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%Rand", "", false);
        }
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric LEN(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(numeric._varType != Expression::Number)
        {
            Compiler::getNextTempVar();

            // Handle non variables
            if(numeric._index == -1)
            {
                switch(numeric._varType)
                {
                    // Get or create constant string
                    case Expression::String:
                    {
                        int index;
                        Compiler::getOrCreateConstString(numeric._text, index);
                        numeric._index = int16_t(index);
                        numeric._varType = Expression::StrVar;
                    }
                    break;

                    case Expression::TmpStrVar:
                    {
                    }
                    break;

                    default:
                    {
                        fprintf(stderr, "Functions::LEN() : couldn't find variable name '%s' : on line %d\n", numeric._name.c_str(), codeLineIndex);
                        return numeric;
                    }
                }
            }

            int length = 0;
            switch(numeric._varType)
            {
                case Expression::IntVar:   length = Compiler::getIntegerVars()[numeric._index]._intSize; break;
                case Expression::StrVar:   length = Compiler::getStringVars()[numeric._index]._size;     break;
                case Expression::Constant: length = Compiler::getConstants()[numeric._index]._size;      break;

                case Expression::Arr1Var8:
                case Expression::Arr1Var16:
                {
                    length = Compiler::getIntegerVars()[numeric._index]._arrSizes[2] * Compiler::getIntegerVars()[numeric._index]._intSize;
                }
                break;

                case Expression::Arr2Var8:
                case Expression::Arr2Var16:
                {
                    length = Compiler::getIntegerVars()[numeric._index]._arrSizes[1] * Compiler::getIntegerVars()[numeric._index]._arrSizes[2] *
                             Compiler::getIntegerVars()[numeric._index]._intSize;
                }
                break;

                case Expression::Arr3Var8:
                case Expression::Arr3Var16:
                {
                    length = Compiler::getIntegerVars()[numeric._index]._arrSizes[0] * Compiler::getIntegerVars()[numeric._index]._arrSizes[1] *
                             Compiler::getIntegerVars()[numeric._index]._arrSizes[2] * Compiler::getIntegerVars()[numeric._index]._intSize;
                }
                break;

                default: break;
            }

            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._value = length;
                return numeric;
            }

            // Variables
            if(numeric._varType == Expression::StrVar  &&  !Compiler::getStringVars()[numeric._index]._constant)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[numeric._index]._address), false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            else if(numeric._varType == Expression::TmpStrVar)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()), false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            // Constants
            else
            {
                // Generate code to save result into a tmp var
                (length <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(length), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(length), false);
            }

            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            
        }

        return numeric;
    }

    Expression::Numeric GET(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::GET() : GET() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType == Expression::String)
        {
            std::string sysVarName = numeric._text;
            Expression::strToUpper(sysVarName);
            if(sysVarName == "SPRITE_LUT"  &&  numeric._parameters.size() == 1)
            {
                // Literal constant
                if(numeric._parameters[0]._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                }

                // Look up sprite lut from sprites lut using a sprite index, (handleSingleOp LDW is skipped if above was a constant literal)
                Compiler::getNextTempVar();
                Operators::handleSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "spriteId", false);
                Compiler::emitVcpuAsm("%GetSpriteLUT", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                return numeric._parameters[0];
            }
            else if(sysVarName == "MUSIC_NOTE"  &&  numeric._parameters.size() == 1)
            {
                // Literal constant
                if(numeric._parameters[0]._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                }

                // Look up a ROM note using a note index, (handleSingleOp LDW is skipped if above was a constant literal)
                Compiler::getNextTempVar();
                Operators::handleSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "musicNote", false);
                Compiler::emitVcpuAsm("%GetMusicNote", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                return numeric._parameters[0];
            }
            else if(numeric._parameters.size() == 0)
            {
                Compiler::getNextTempVar();
                Operators::createTmpVar(numeric);

                if(sysVarName == "TIME_MODE")
                {
                    Compiler::emitVcpuAsm("LDWI", "handleT_mode + 1", false);
                    Compiler::emitVcpuAsm("PEEK", "", false);
                }
                if(sysVarName == "TIME_EPOCH")
                {
                    Compiler::emitVcpuAsm("LDWI", "handleT_epoch + 1", false);
                    Compiler::emitVcpuAsm("PEEK", "", false);
                }
                else if(sysVarName == "TIME_S")
                {
                    Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 0", false);
                    Compiler::emitVcpuAsm("PEEK", "", false);
                }
                else if(sysVarName == "TIME_M")
                {
                    Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 1", false);
                    Compiler::emitVcpuAsm("PEEK", "", false);
                }
                else if(sysVarName == "TIME_H")
                {
                    Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 2", false);
                    Compiler::emitVcpuAsm("PEEK", "", false);
                }
                else if(sysVarName == "TIMER")
                {
                    Compiler::emitVcpuAsm("LDW", "timerTick", false);
                }
                else if(sysVarName == "TIMER_PREV")
                {
                    Compiler::emitVcpuAsm("LDW", "timerPrev", false);
                }
                else if(sysVarName == "VBLANK_PROC")
                {
                    if(Compiler::getCodeRomType() < Cpu::ROMv5a)
                    {
                        std::string romTypeStr;
                        getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                        fprintf(stderr, "Functions::GET() : Version error, 'SET VBLANK_PROC' requires ROMv5a or higher, you are trying to link against '%s', on line %d\n", romTypeStr.c_str(), 
                                                                                                                                                                                  codeLineIndex);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(VBLANK_PROC), false);
                        Compiler::emitVcpuAsm("DEEK", "", false);
                    }
                }
                else if(sysVarName == "VBLANK_FREQ")
                {
                    if(Compiler::getCodeRomType() < Cpu::ROMv5a)
                    {
                        std::string romTypeStr;
                        getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                        fprintf(stderr, "Functions::GET() : Version error, 'SET VBLANK_FREQ' requires ROMv5a or higher, you are trying to link against '%s', on line %d\n", romTypeStr.c_str(), 
                                                                                                                                                                                  codeLineIndex);
                    }
                    // (256 - n) = vblank interrupt frequency, where n = 1 to 255
                    else
                    {
                        Compiler::emitVcpuAsm("LDWI", "realTS_rti + 2", false);
                        Compiler::emitVcpuAsm("PEEK", "", false);
                        Compiler::emitVcpuAsm("STW", "register0", false);
                        Compiler::emitVcpuAsm("LDWI", "256", false);
                        Compiler::emitVcpuAsm("SUBW", "register0", false);
                    }
                }
                else if(sysVarName == "CURSOR_X")
                {
                    Compiler::emitVcpuAsm("LD", "cursorXY", false);
                }
                else if(sysVarName == "CURSOR_Y")
                {
                    Compiler::emitVcpuAsm("LD", "cursorXY + 1", false);
                }
                else if(sysVarName == "CURSOR_XY")
                {
                    Compiler::emitVcpuAsm("LDW", "cursorXY", false);
                }
                else if(sysVarName == "FG_COLOUR")
                {
                    Compiler::emitVcpuAsm("LD", "fgbgColour + 1", false);
                }
                else if(sysVarName == "BG_COLOUR")
                {
                    Compiler::emitVcpuAsm("LD", "fgbgColour", false);
                }
                else if(sysVarName == "FGBG_COLOUR")
                {
                    Compiler::emitVcpuAsm("LDW", "fgbgColour", false);
                }
                else if(sysVarName == "MIDI_STREAM")
                {
                    Compiler::emitVcpuAsm("LDW", "midiStream", false);
                }
                else if(sysVarName == "LED_TEMPO")
                {
                    Compiler::emitVcpuAsm("LD", "giga_ledTempo", false);
                }
                else if(sysVarName == "LED_STATE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_ledState", false);
                }
                else if(sysVarName == "SOUND_TIMER")
                {
                    Compiler::emitVcpuAsm("LD", "giga_soundTimer", false);
                }
                else if(sysVarName == "CHANNEL_MASK")
                {
                    Compiler::emitVcpuAsm("LD", "giga_channelMask", false);
                    Compiler::emitVcpuAsm("ANDI", "0x03", false);
                }
                else if(sysVarName == "ROM_TYPE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_romType", false);
                    Compiler::emitVcpuAsm("ANDI", "0xFC", false);
                }
                else if(sysVarName == "VSP")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vSP", false);
                }
                else if(sysVarName == "VLR")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vLR", false);
                }
                else if(sysVarName == "VAC")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vAC", false);
                }
                else if(sysVarName == "VPC")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vPC", false);
                }
                else if(sysVarName == "XOUT_MASK")
                {
                    Compiler::emitVcpuAsm("LD", "giga_xoutMask", false);
                }
                else if(sysVarName == "BUTTON_STATE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_buttonState", false);
                }
                else if(sysVarName == "SERIAL_RAW")
                {
                    Compiler::emitVcpuAsm("LD", "giga_serialRaw", false);
                }
                else if(sysVarName == "FRAME_COUNT")
                {
                    Compiler::emitVcpuAsm("LD", "giga_frameCount", false);
                }
                else if(sysVarName == "VIDEO_Y")
                {
                    Compiler::emitVcpuAsm("LD", "giga_videoY", false);
                }
                else if(sysVarName == "RAND2")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand2", false);
                }
                else if(sysVarName == "RAND1")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand1", false);
                }
                else if(sysVarName == "RAND0")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand0", false);
                }
                else if(sysVarName == "MEM_SIZE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_memSize", false);
                }
                else if(sysVarName == "Y_RES")
                {
                    Compiler::emitVcpuAsm("LDI", "giga_yres", false);
                }
                else if(sysVarName == "X_RES")
                {
                    Compiler::emitVcpuAsm("LDI", "giga_xres", false);
                }
                else if(sysVarName == "SND_CHN4")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan4", false);
                }
                else if(sysVarName == "SND_CHN3")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan3", false);
                }
                else if(sysVarName == "SND_CHN2")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan2", false);
                }
                else if(sysVarName == "SND_CHN1")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan1", false);
                }
                else if(sysVarName == "V_TOP")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_videoTop", false);
                }
                else if(sysVarName == "V_TABLE")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_videoTable", false);
                }
                else if(sysVarName == "V_RAM")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_vram", false);
                }
                else if(sysVarName == "ROM_NOTES")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_notesTable", false);
                }
                else if(sysVarName == "ROM_TEXT82")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_text82", false);
                }
                else if(sysVarName == "ROM_TEXT32")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_text32", false);
                }
                else
                {
                    fprintf(stderr, "*** Warning *** Functions::GET() : system variable name '%s' does not exist : on line %d\n", numeric._text.c_str(), codeLineIndex);
                    return numeric;
                }

                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
        }

        return numeric;
    }

    Expression::Numeric ABS(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::ABS() : ABS() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType != Expression::String  &&  numeric._varType != Expression::StrVar &&  numeric._varType != Expression::TmpStrVar)
        {
            Compiler::getNextTempVar();

            if(numeric._varType == Expression::Number)
            {
                numeric._value = abs(numeric._value);
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Operators::createTmpVar(numeric);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
            else
            {
                Operators::handleSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("%Absolute", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
        }

        return numeric;
    }

    Expression::Numeric SGN(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::SGN() : SGN() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._varType != Expression::String  &&  numeric._varType != Expression::StrVar &&  numeric._varType != Expression::TmpStrVar)
        {
            Compiler::getNextTempVar();

            if(numeric._varType == Expression::Number)
            {
                numeric._value = Expression::sgn(numeric._value);
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Operators::createTmpVar(numeric);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
            else
            {
                Operators::handleSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("%Sign", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
        }

        return numeric;
    }

    Expression::Numeric ASC(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(numeric._varType != Expression::Number)
        {
            Compiler::getNextTempVar();

            // Handle non variables
            if(numeric._index == -1)
            {
                switch(numeric._varType)
                {
                    // Get or create constant string
                    case Expression::String:
                    {
                        int index;
                        Compiler::getOrCreateConstString(numeric._text, index);
                        numeric._index = int16_t(index);
                        numeric._varType = Expression::StrVar;
                    }
                    break;

                    case Expression::TmpStrVar:
                    {
                    }
                    break;

                    default:
                    {
                        fprintf(stderr, "Functions::ASC() : couldn't find variable name '%s' : on line %d\n", numeric._name.c_str(), codeLineIndex);
                        return numeric;
                    }
                }
            }

            uint8_t ascii = 0;
            switch(numeric._varType)
            {
                case Expression::StrVar:   ascii = Compiler::getStringVars()[numeric._index]._text[0]; break;
                case Expression::Constant: ascii = Compiler::getConstants()[numeric._index]._text[0];  break;

                default: break;
            }

            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._value = ascii;
                return numeric;
            }

            // Variables
            if(numeric._varType == Expression::StrVar  &&  !Compiler::getStringVars()[numeric._index]._constant)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[numeric._index]._address) + " + 1", false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            else if(numeric._varType == Expression::TmpStrVar)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()) + " + 1", false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            // Constants
            else
            {
                // Generate code to save result into a tmp var
                Compiler::emitVcpuAsm("LDI", std::to_string(ascii), false);
            }

            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric STRCMP(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(numeric._parameters.size() != 1)
        {
            fprintf(stderr, "Functions::STRCMP() : STRCMP() requires two string parameters : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Literal strings, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters[0]._varType == Expression::String)
        {
            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._varType = Expression::Number;
                numeric._value = uint8_t(numeric._text == numeric._parameters[0]._text);
                return numeric;
            }
            // Generate code to save result into a tmp var
            else
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(numeric._text == numeric._parameters[0]._text)), false);
            }
        }
        else
        {
            // Get addresses of strings to be compared
            std::string name0, name1;
            uint16_t srcAddr0, srcAddr1;
            int index0 = int(numeric._index);
            int index1 = int(numeric._parameters[0]._index);
            getOrCreateString(numeric, name0, srcAddr0, index0);
            getOrCreateString(numeric._parameters[0], name1, srcAddr1, index1);

            // By definition this must be a match
            if(srcAddr0 == srcAddr1)
            {
                Compiler::emitVcpuAsm("LDI", "1", false);
            }
            // Compare strings
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr0), false);
                Compiler::emitVcpuAsm("STW",  "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr1), false);
                Compiler::emitVcpuAsm("%StringCmp", "", false);
            }
        }

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric BCDCMP(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::BCDCMP() : BCDCMP() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() != 2)
        {
            fprintf(stderr, "Functions::BCDCMP() : BCDCMP() requires three string parameters : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Get addresses and length of bcd values to be compared
        uint16_t srcAddr0 = uint16_t(numeric._value);
        uint16_t srcAddr1 = uint16_t(numeric._parameters[0]._value);
        uint16_t length = uint16_t(numeric._parameters[1]._value);

        // Compare bcd values, (addresses MUST point to msd)
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr0), false);
        Compiler::emitVcpuAsm("STW",  "bcdSrcAddr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr1), false);
        Compiler::emitVcpuAsm("STW",  "bcdDstAddr", false);
        Compiler::emitVcpuAsm("LDI",  std::to_string(length), false);
        Compiler::emitVcpuAsm("%BcdCmp", "", false);

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric VAL(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(numeric._parameters.size() != 0)
        {
            fprintf(stderr, "Functions::VAL() : VAL() requires only one string parameter : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Literal strings, (optimised case)
        if(numeric._varType == Expression::String)
        {
            int16_t val = 0;
            Expression::stringToI16(numeric._text, val);

            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._varType = Expression::Number;
                numeric._value = val;
                return numeric;
            }
            // Generate code to save result into a tmp var
            else
            {
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
            }
        }
        else
        {
            // Get addresses of src string
            std::string name;
            uint16_t srcAddr;
            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            // StringVal expects srcAddr to point past the string's length byte
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr + 1), false);
            Compiler::emitVcpuAsm("%IntegerStr", "", false);
        }

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric LUP(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::LUP() : LUP(<address>, <offset>) cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() != 1) 
        {
            fprintf(stderr, "Functions::LUP() : LUP(<address>, <offset>) missing offset : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters[0]._varType != Expression::Number)
        {
            fprintf(stderr, "Functions::LUP() : LUP(<address>, <offset>) offset is not a constant literal : on line %d\n", codeLineIndex);
            return numeric;
        }

        std::string offset = Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value)));

        if(numeric._varType == Expression::Number)
        {
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(uint16_t(std::lround(numeric._value))), false);
        }
        else
        {
            Operators::createSingleOp("LDW", numeric);
        }

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("LUP", offset, false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric ADDR(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(numeric._varType != Expression::Arr1Var8   &&  numeric._varType != Expression::Arr2Var8   &&  numeric._varType != Expression::Arr3Var8   &&
           numeric._varType != Expression::Arr1Var16  &&  numeric._varType != Expression::Arr2Var16  &&  numeric._varType != Expression::Arr3Var16  &&
           numeric._varType != Expression::Str2Var)
        {
            fprintf(stderr, "Functions::ADDR() : ADDR() can only be used on array variables : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._index == -1)
        {
            fprintf(stderr, "Functions::ADDR() : ADDR() can't find array variable : on line %d\n", codeLineIndex);
            return numeric;
        }

        if((numeric._varType == Expression::Arr1Var8   &&  numeric._parameters.size() != 1)  ||  (numeric._varType == Expression::Arr2Var8   &&  numeric._parameters.size() != 2)  ||
           (numeric._varType == Expression::Arr3Var8   &&  numeric._parameters.size() != 3)  ||  (numeric._varType == Expression::Arr1Var16  &&  numeric._parameters.size() != 1)  ||
           (numeric._varType == Expression::Arr2Var16  &&  numeric._parameters.size() != 2)  ||  (numeric._varType == Expression::Arr3Var16  &&  numeric._parameters.size() != 3)  ||
           (numeric._varType == Expression::Str2Var    &&  numeric._parameters.size() != 1))
        {
            fprintf(stderr, "Functions::ADDR() : Wrong number of parameters in ADDR() : on line %d\n", codeLineIndex);
            return numeric;
        }

        // 1d index
        if(numeric._parameters.size() == 1)
        {
            // Literal index, (optimised case)
            if(numeric._parameters[0]._varType == Expression::Number)
            {
                uint16_t address = 0x0000;
                uint16_t indexI = uint16_t(numeric._parameters[0]._value);
                switch(numeric._varType)
                {
                    case Expression::Arr1Var8:  address = Compiler::getIntegerVars()[numeric._index]._address  +  indexI * 1; break;
                    case Expression::Arr1Var16: address = Compiler::getIntegerVars()[numeric._index]._address  +  indexI * 2; break;
                    case Expression::Str2Var:   address = Compiler::getStringVars()[numeric._index]._arrAddrs[indexI];        break;

                    default: break;
                }

                // No code needed for static initialisation
                if(Expression::getOutputNumeric()._staticInit)
                {
                    numeric._varType = Expression::Number;
                    numeric._value = address;
                    return numeric;
                }
                // Array address
                else
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);
                }
            }
            // Convert index parameter into an array address
            else
            {
                switch(numeric._varType)
                {
                    case Expression::Arr1Var8:
                    {
                        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getIntegerVars()[numeric._index]._address), false);
                        Operators::createSingleOp("ADDW", numeric._parameters[0]);
                    }
                    break;

                    case Expression::Arr1Var16:
                    {
                        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getIntegerVars()[numeric._index]._address), false);
                        Operators::createSingleOp("ADDW", numeric._parameters[0]);
                        Operators::createSingleOp("ADDW", numeric._parameters[0]);
                    }
                    break;

                    case Expression::Str2Var:
                    {
                        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[numeric._index]._address), false);
                        Operators::createSingleOp("ADDW", numeric._parameters[0]);
                        Operators::createSingleOp("ADDW", numeric._parameters[0]);
                        Compiler::emitVcpuAsm("DEEK", "", false);
                    }
                    break;

                    default: break;
                }
            }
        }
        // 2d indices
        else if(numeric._parameters.size() == 2)
        {
            // All literal indices, (optimised case)
            if(numeric._parameters[0]._varType == Expression::Number  &&  numeric._parameters[1]._varType == Expression::Number)
            {
                uint16_t indexJ = uint16_t(numeric._parameters[0]._value);
                uint16_t indexI = uint16_t(numeric._parameters[1]._value);

                uint16_t address = Compiler::getIntegerVars()[numeric._index]._arrAddrs[0][indexJ];
                address += (Compiler::getIntegerVars()[numeric._index]._intSize == Compiler::Int8) ? indexI : indexI * 2;

                // No code needed for static initialisation
                if(Expression::getOutputNumeric()._staticInit)
                {
                    numeric._varType = Expression::Number;
                    numeric._value = address;
                    return numeric;
                }
                // Array address
                else
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);
                }
            }
            // Convert index parameters into an array address
            else
            {
                uint16_t address = Compiler::getIntegerVars()[numeric._index]._address;
                Operators::createSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "memIndex0", false);
                Operators::createSingleOp("LDW", numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "memIndex1", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);

                switch(Compiler::getIntegerVars()[numeric._index]._intSize)
                {
                    case Compiler::Int8:
                    {
                        (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert8Arr2d", false) : Compiler::emitVcpuAsm("CALL", "convert8Arr2dAddr", false);
                    }
                    break;
                    
                    case Compiler::Int16:
                    {
                        (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert16Arr2d", false) : Compiler::emitVcpuAsm("CALL", "convert16Arr2dAddr", false);
                    }
                    break;

                    default: break;
                }
            }
        }
        // 3d indices
        else if(numeric._parameters.size() == 3)
        {
            // All literal indices, (optimised case)
            if(numeric._parameters[0]._varType == Expression::Number  &&  numeric._parameters[1]._varType == Expression::Number  &&  numeric._parameters[2]._varType == Expression::Number)
            {
                uint16_t indexK = uint16_t(numeric._parameters[0]._value);
                uint16_t indexJ = uint16_t(numeric._parameters[1]._value);
                uint16_t indexI = uint16_t(numeric._parameters[2]._value);

                uint16_t address = Compiler::getIntegerVars()[numeric._index]._arrAddrs[indexK][indexJ];
                address += (Compiler::getIntegerVars()[numeric._index]._intSize == Compiler::Int8) ? indexI : indexI * 2;

                // No code needed for static initialisation
                if(Expression::getOutputNumeric()._staticInit)
                {
                    numeric._varType = Expression::Number;
                    numeric._value = address;
                    return numeric;
                }
                // Array address
                else
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);
                }
            }
            // Convert index parameters into an array address
            else
            {
                uint16_t address = Compiler::getIntegerVars()[numeric._index]._address;
                Operators::createSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "memIndex0", false);
                Operators::createSingleOp("LDW", numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "memIndex1", false);
                Operators::createSingleOp("LDW", numeric._parameters[2]);
                Compiler::emitVcpuAsm("STW", "memIndex2", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);

                switch(Compiler::getIntegerVars()[numeric._index]._intSize)
                {
                    case Compiler::Int8:
                    {
                        (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert8Arr3d", false) : Compiler::emitVcpuAsm("CALL", "convert8Arr3dAddr", false);
                    }
                    break;
                    
                    case Compiler::Int16:
                    {
                        (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? Compiler::emitVcpuAsm("CALLI", "convert16Arr3d", false) : Compiler::emitVcpuAsm("CALL", "convert16Arr3dAddr", false);
                    }
                    break;

                    default: break;
                }
            }
        }

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric POINT(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::POINT() : POINT() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() == 1)
        {
            if(numeric._varType == Expression::Number)
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("ST", "readPixel_xy", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("ST", "readPixel_xy", false);
            }

            if(numeric._parameters[0]._varType == Expression::Number)
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                Compiler::emitVcpuAsm("ST", "readPixel_xy + 1", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("ST", "readPixel_xy + 1", false);
            }

            Compiler::getNextTempVar();
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%ReadPixel", "", false);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric MIN(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::MIN() : MIN() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() == 1)
        {
            if(numeric._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }

            if(numeric._parameters[0]._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._parameters[0]._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[0]);
            }

            Compiler::getNextTempVar();
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%IntMin", "", false);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric MAX(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::MAX() : MAX() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() == 1)
        {
            if(numeric._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }

            if(numeric._parameters[0]._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._parameters[0]._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[0]);
            }

            Compiler::getNextTempVar();
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%IntMax", "", false);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric CLAMP(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::CLAMP() : CLAMP() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        if(numeric._parameters.size() == 2)
        {
            if(numeric._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
                Compiler::emitVcpuAsm("STW", "intSrcX", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("STW", "intSrcX", false);
            }

            if(numeric._parameters[0]._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._parameters[0]._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "intSrcA", false);
            }

            if(numeric._parameters[1]._varType == Expression::Number)
            {
                int16_t val = int16_t(std::lround(numeric._parameters[1]._value));
                (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[1]);
            }

            Compiler::getNextTempVar();
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%IntClamp", "", false);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric CHR$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::CHR$() : CHR$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print CHR string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcChar", "", false);
                return numeric;
            }

            // Create CHR string
            Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("%StringChr", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcChar", "", false);
            return numeric;
        }

        // Create CHR string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("%StringChr", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric STR$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::STR$() : STR$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print STR string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                return numeric;
            }

            // Create STR string
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strInteger", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("%StringInt", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
            return numeric;
        }

        // Create STR string
        Compiler::emitVcpuAsm("STW", "strInteger", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("%StringInt", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric TIME$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::TIME$() : TIME$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Generate new time string
        Compiler::emitVcpuAsm("%TimeString", "", false);

        // Print it directly if able
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintString", "_timeString_", false);
            return numeric;
        }

        // Otherwise copy it to var or tmpvar
        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }
        Compiler::emitVcpuAsm("LDWI", "_timeString_", false);
        Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("%StringCopy", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric HEX$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::HEX$() : HEX$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print HEX string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
                return numeric;
            }

            // Create HEX string
            Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("%StringHex", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
            return numeric;
        }

        // Create HEX string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("%StringHex", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric HEXW$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::HEXW$() : HEXW$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print HEXW string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
                return numeric;
            }

            // Create HEXW string
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strHex", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("%StringHexw", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
            return numeric;
        }

        // Create HEXW string
        Compiler::emitVcpuAsm("STW", "strHex", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("%StringHexw", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric LEFT$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::LEFT$() : LEFT$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrLeft, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcLeft", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strDstLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("%StringLeft", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric RIGHT$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::RIGHT$() : RIGHT$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrRight, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcRight", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strDstLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("%StringRight", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric MID$(Expression::Numeric& numeric, int codeLineIndex)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Functions::MID$() : MID$() cannot be used in static initialisation : on line %d\n", codeLineIndex);
            return numeric;
        }

        // Literal string and parameters, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 2  &&  numeric._parameters[0]._varType == Expression::Number  &&  
           numeric._parameters[1]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrMid, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 2)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textOfs", false);
                handleStringParameter(numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcMid", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strOffset", false);
                handleStringParameter(numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "strDstLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("%StringMid", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }
}