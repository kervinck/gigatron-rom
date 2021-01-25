#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmath>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "operators.h"
#include "linker.h"


namespace Operators
{
    bool _nextTempVar = true;

    std::vector<std::string> _operators;

    std::vector<std::string>& getOperators(void) {return _operators;}


    bool initialise(void)
    {
        _nextTempVar = true;

        // Operators
        _operators.push_back(" AND ");
        _operators.push_back(" XOR ");
        _operators.push_back(" OR " );
        _operators.push_back("NOT " );
        _operators.push_back(" MOD ");
        _operators.push_back(" LSL ");
        _operators.push_back(" LSR ");
        _operators.push_back(" ASR ");

        return true;
    }


    void changeToTmpVar(Expression::Numeric& numeric)
    {
        numeric._value = uint8_t(Compiler::getTempVarStart());
        numeric._varType = Expression::TmpVar;
        numeric._name = Compiler::getTempVarStartStr();
    }

    void createSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric)
    {
        switch(numeric._varType)
        {
            // Temporary variable address
            case Expression::TmpVar:
            {
                Compiler::emitVcpuAsm(opcodeStr, Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
            }
            break;

            // User variable name
            case Expression::IntVar16:
            {
                Compiler::emitVcpuAsmUserVar(opcodeStr, numeric, false);
            }
            break;

            default: break;
        }
    }

    void handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric)
    {
        createSingleOp(opcodeStr, numeric);
        changeToTmpVar(numeric);
    }

    void selectSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            int16_t val = int16_t(std::lround(numeric._value));
            (val >= 0  && val <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(val), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(val), false);
        }
        else
        {
            Operators::createSingleOp(opcodeStr, numeric);
        }
    }

    bool handleDualOp(const std::string& opcodeStr, Expression::Numeric& lhs, Expression::Numeric& rhs, bool outputHex)
    {
        std::string opcode = std::string(opcodeStr);

        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(rhs._varType == Expression::Number  &&  (rhs._value < 0  ||  rhs._value > 255))
        {
            std::swap(lhs, rhs);
            if(opcode == "SUB")
            {
                opcode = "ADD";
                if(lhs._value > 0) lhs._value = -lhs._value;
            }
        }

        // LHS
        if(lhs._varType == Expression::Number)
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                (outputHex) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(lhs._value))), false) : Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(lhs._value))), false);
            }
            // 16bit constants
            else
            {
                (outputHex) ? Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(lhs._value))), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(lhs._value))), false);
            }

            _nextTempVar = true;
        }
        else
        {
            switch(lhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(lhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        // RHS
        if(rhs._varType == Expression::Number)
        {
            // Skip XOR if operand is 0, (n XOR 0 = n)
            if(rhs._value  ||  opcode != "XOR") 
            {
                (outputHex) ? Compiler::emitVcpuAsm(opcode + "I", Expression::wordToHexString(int8_t(std::lround(rhs._value))), false) : Compiler::emitVcpuAsm(opcode + "I", std::to_string(int8_t(std::lround(rhs._value))), false);
            }
        }
        else
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(std::lround(rhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar(opcode + "W", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        changeToTmpVar(lhs);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return true;
    }

    bool handleLogicalOp(const std::string& opcode, Expression::Numeric& lhs)
    {
        // SYS shift function needs this preamble, LSLW doesn't
        switch(lhs._varType)
        {
            // Temporary variable address
            case Expression::TmpVar:
            {
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(lhs._value))), false);
            }
            break;

            // User variable name
            case Expression::IntVar16:
            {
                if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
            }
            break;

            default: break;
        }

        if(opcode != "LSLW") Compiler::emitVcpuAsm("STW", "mathShift", false);

        changeToTmpVar(lhs);

        return true;
    }

    void emitCcType(Expression::CCType ccType, std::string& cc)
    {
        switch(ccType)
        {
            case Expression::BooleanCC:
            {
                // Init functions are not needed for ROMv5a and higher as CALLI is able to directly CALL them
                if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                {
                    Compiler::emitVcpuAsm("CALLI", "convert" + cc + "Op", false);
                }
                else
                // Enable system internal sub intialiser and mark system internal sub to be loaded
                {
                    Compiler::emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
                    Compiler::enableSysInitFunc("Init" + cc + "Op");
                    Linker::setInternalSubToLoad("convert" + cc + "Op");
                }
            }
            break;

            case Expression::NormalCC: Compiler::emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false); break;
            case Expression::FastCC: Compiler::emitVcpuAsm("B" + Expression::strToUpper(cc), "", false); break;

            default: break;
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
    }
    bool handleConditionOp(Expression::Numeric& lhs, Expression::Numeric& rhs, Expression::CCType ccType, bool& invertedLogic, const std::string& opcode="SUB")
    {
        // Swap left and right to take advantage of LDWI for 16bit numbers
        invertedLogic = false;
        if(rhs._varType == Expression::Number  &&  (rhs._value < 0  ||  rhs._value > 255))
        {
            std::swap(lhs, rhs);
            invertedLogic = true;
        }

        // JumpCC and BCC are inverses of each other
        lhs._ccType = ccType;
        if(ccType == Expression::FastCC) invertedLogic = !invertedLogic;

        // LHS
        if(lhs._varType == Expression::Number)
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(lhs._value))), false);
            }
            // 16bit constants
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(lhs._value))), false);
            }

            _nextTempVar = true;
        }
        else
        {
            switch(lhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(lhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        // RHS
        if(rhs._varType == Expression::Number)
        {
            // Skip XOR if operand is 0, (n XOR 0 = n)
            if(rhs._value  ||  opcode != "XOR") Compiler::emitVcpuAsm(opcode + "I", std::to_string(int16_t(std::lround(rhs._value))), false);
        }
        else        
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(std::lround(rhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar(opcode + "W", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        changeToTmpVar(lhs);

        return true;
    }

    bool handleStringCcOP(Expression::Numeric& lhs, Expression::Numeric& rhs, Expression::OPType opType)
    {
        if(lhs._varType != Expression::String  &&  lhs._varType != Expression::Constant  &&  lhs._varType != Expression::StrVar  &&  lhs._varType != Expression::Str2Var  &&
           lhs._varType != Expression::TmpStrVar) return false;
        if(rhs._varType != Expression::String  &&  rhs._varType != Expression::Constant  &&  rhs._varType != Expression::StrVar  &&  rhs._varType != Expression::Str2Var  &&
           rhs._varType != Expression::TmpStrVar) return false;

        if(lhs._varType == Expression::String  &&  rhs._varType == Expression::String)
        {
            switch(opType)
            {
                case Expression::EqOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) == 0); return true;
                case Expression::NeOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) != 0); return true;
                case Expression::LeOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) <= 0); return true;
                case Expression::GeOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) >= 0); return true;
                case Expression::LtOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) <  0); return true;
                case Expression::GtOP: lhs._value = (int16_t(lhs._text.compare(rhs._text)) >  0); return true;

                default: break;
            }
        }

        // Get addresses of strings to be compared
        int lhsIndex = int(lhs._index);
        int rhsIndex = int(rhs._index);
        std::string lhsName, rhsName;
        uint16_t lhsAddr, rhsAddr;

        // String inputs can be literal, const, var and temp
        if(lhs._varType == Expression::TmpStrVar)
        {
            lhsAddr = Compiler::getStrWorkArea();
        }
        else
        {
            Compiler::getOrCreateString(lhs, lhsName, lhsAddr, lhsIndex);
        }
        if(rhs._varType == Expression::TmpStrVar)
        {
            rhsAddr = Compiler::getStrWorkArea();
        }
        else
        {
            Compiler::getOrCreateString(rhs, rhsName, rhsAddr, rhsIndex);
        }

        // By definition this must be a match
        if(lhsAddr == rhsAddr)
        {
            Compiler::emitVcpuAsm("LDI", "1", false);
        }
        // Compare strings
        else
        {
            Compiler::emitStringAddress(lhs, lhsAddr);
            Compiler::emitVcpuAsm("STW",  "strSrcAddr", false);
            Compiler::emitStringAddress(rhs, rhsAddr);
            Compiler::emitVcpuAsm("%StringCmp", "", false);

            // 0, 1, 2, (lesser, equal, greater)
            switch(opType)
            {
                case Expression::EqOP:
                {
                    // 0, 1, 2 to 0, 1, 0
                    Compiler::emitVcpuAsm("ANDI", "1", false);
                }
                break;

                case Expression::NeOP:
                {
                    // 0, 1, 2 to 1, 0, 1
                    Compiler::emitVcpuAsm("ANDI", "1", false);
                    Compiler::emitVcpuAsm("XORI", "1", false);
                }
                break;

                case Expression::LeOP:
                {
                    // 0, 1 to 1 : 2 to 0
                    Compiler::emitVcpuAsm("XORI", "2", false);
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        Compiler::emitVcpuAsm("CALLI", "convertNeOpAddr", false);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("CALL", "convertNeOpAddr", false);
                        Compiler::enableSysInitFunc("InitNeOp");
                        Linker::setInternalSubToLoad("convertNeOp");
                    }
                }
                break;

                case Expression::GeOP:
                {
                    // 1, 2 to 1 : 0 to 0
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        Compiler::emitVcpuAsm("CALLI", "convertNeOpAddr", false);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("CALL", "convertNeOpAddr", false);
                        Compiler::enableSysInitFunc("InitNeOp");
                        Linker::setInternalSubToLoad("convertNeOp");
                    }
                }
                break;

                case Expression::LtOP:
                {
                    // 0 to 1 : 1, 2 to 0
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        Compiler::emitVcpuAsm("CALLI", "convertEqOpAddr", false);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("CALL", "convertEqOpAddr", false);
                        Compiler::enableSysInitFunc("InitEqOp");
                        Linker::setInternalSubToLoad("convertEqOp");
                    }
                }
                break;

                case Expression::GtOP:
                {
                    // 0, 1 to 0 : 2 to 1
                    Compiler::emitVcpuAsm("ANDI",  "2", false);
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        Compiler::emitVcpuAsm("CALLI", "convertNeOpAddr", false);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("CALL", "convertNeOpAddr", false);
                        Compiler::enableSysInitFunc("InitNeOp");
                        Linker::setInternalSubToLoad("convertNeOp");
                    }
                }
                break;

                default: break;
            }
        }

        changeToTmpVar(lhs);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return true;
    }

    bool handleStringAdd(Expression::Numeric& lhs, Expression::Numeric& rhs)
    {
        if(lhs._varType != Expression::String  &&  lhs._varType != Expression::Constant  &&  lhs._varType != Expression::StrVar  &&  lhs._varType != Expression::Str2Var  &&
           lhs._varType != Expression::TmpStrVar) return false;
        if(rhs._varType != Expression::String  &&  rhs._varType != Expression::Constant  &&  rhs._varType != Expression::StrVar  &&  rhs._varType != Expression::Str2Var  &&
           rhs._varType != Expression::TmpStrVar) return false;

        // Get addresses of strings to be compared
        int lhsIndex = int(lhs._index);
        int rhsIndex = int(rhs._index);
        std::string lhsName, rhsName;
        uint16_t lhsAddr, rhsAddr;

        // String inputs can be literal, const, var and temp
        if(lhs._varType == Expression::TmpStrVar)
        {
            lhsAddr = Compiler::getStrWorkArea();
        }
        else
        {
            Compiler::getOrCreateString(lhs, lhsName, lhsAddr, lhsIndex);
        }
        if(rhs._varType == Expression::TmpStrVar)
        {
            // If both lhs and rhs are temporaries then get next string work area and swap lhs and rhs, (stringConcat requires dst = src0)
            if(lhs._varType == Expression::TmpStrVar) Compiler::nextStrWorkArea();
            rhsAddr = Compiler::getStrWorkArea();
            std::swap(lhs, rhs);
            std::swap(lhsAddr, rhsAddr);
        }
        else
        {
            Compiler::getOrCreateString(rhs, rhsName, rhsAddr, rhsIndex);
        }

        Compiler::emitStringAddress(lhs, lhsAddr);
        Compiler::emitVcpuAsm("STW",  "strSrcAddr", false);
        Compiler::emitStringAddress(rhs, rhsAddr);
        Compiler::emitVcpuAsm("STW",  "strSrcAddr2", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()), false);
        Compiler::emitVcpuAsm("%StringConcat", "", false);

        lhs =  Expression::Numeric(0, -1, true, false, false, Expression::TmpStrVar, Expression::BooleanCC, Expression::Int16Both, "", std::string(""));

        return true;
    }

    bool handleMathOp(const std::string& opcode, const std::string& operand, Expression::Numeric& lhs, Expression::Numeric& rhs, bool isMod=false)
    {
        // LHS
        if(lhs._varType == Expression::Number)
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(lhs._value))), false);
            }
            // 16bit constants
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(lhs._value))), false);
            }

            _nextTempVar = true;
        }
        else
        {
            switch(lhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(lhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("STW", "mathX", false);

        // RHS
        if(rhs._varType == Expression::Number)
        {
            if(rhs._value >=0  &&  rhs._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(rhs._value))), false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(rhs._value))), false);
            }
        }
        else
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(rhs._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar16:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("STW", "mathY", false);

        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm(opcode, operand, false);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", operand, false);
            Compiler::emitVcpuAsm(opcode, "giga_vAC", false);
        }

        changeToTmpVar(lhs);
        
        if(isMod) Compiler::emitVcpuAsm("LDW", "mathRem", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return true;
    }

    uint32_t handleRevOp(uint32_t input, uint32_t n)
    {
        uint32_t output = 0;
        uint32_t bits = input & uint32_t(pow(2, n) - 1);
        for(uint32_t i=0; i<=n-1; i++)
        {
            output = (output << 1) | (bits & 1);
            bits = bits >> 1;
        }

        return output;
    }


    // ********************************************************************************************
    // Unary Operators
    // ********************************************************************************************
    Expression::Numeric POS(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = +numeric._value;
            return numeric;
        }

        Compiler::getNextTempVar();
        handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        
        return numeric;
    }

    Expression::Numeric NEG(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = -numeric._value;
            return numeric;
        }

        Compiler::getNextTempVar();
        Compiler::emitVcpuAsm("LDI", std::to_string(0), false);
        handleSingleOp("SUBW", numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        
        return numeric;
    }

    Expression::Numeric NOT(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = ~int16_t(std::lround(numeric._value));
            return numeric;
        }

        Compiler::getNextTempVar();
        Compiler::emitVcpuAsm("LDWI", std::to_string(-1), false);
        handleSingleOp("XORW", numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }


    // ********************************************************************************************
    // Unary Math Operators
    // ********************************************************************************************
    Expression::Numeric CEIL(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = ceil(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric FLOOR(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = floor(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric POWF(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._params.size() > 0  &&  numeric._params[0]._varType == Expression::Number)
        {
            numeric._value = pow(numeric._value, numeric._params[0]._value);
        }

        numeric._params.clear();
        return numeric;
    }

    Expression::Numeric SQRT(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._value > 0.0)
        {
            numeric._value = sqrt(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric EXP(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = exp(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric EXP2(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = exp2(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric LOG(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._value > 0.0)
        {
            numeric._value = log(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric LOG2(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._value > 0.0)
        {
            numeric._value = log2(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric LOG10(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._value > 0.0)
        {
            numeric._value = log10(numeric._value);
        }

        return numeric;
    }

    Expression::Numeric SIN(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = sin(numeric._value*MATH_PI/180.0);
        }

        return numeric;
    }

    Expression::Numeric COS(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = cos(numeric._value*MATH_PI/180.0);
        }

        return numeric;
    }

    Expression::Numeric TAN(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = tan(numeric._value*MATH_PI/180.0);
        }

        return numeric;
    }

    Expression::Numeric ASIN(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = asin(numeric._value)/MATH_PI*180.0;
        }

        return numeric;
    }

    Expression::Numeric ACOS(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = acos(numeric._value)/MATH_PI*180.0;
        }

        return numeric;
    }

    Expression::Numeric ATAN(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = atan(numeric._value)/MATH_PI*180.0;
        }

        return numeric;
    }

    Expression::Numeric ATAN2(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number  &&  numeric._params.size() > 0  &&  numeric._params[0]._varType == Expression::Number)
        {
            if(numeric._value != 0.0  ||  numeric._params[0]._value != 0.0)
            {
                numeric._value = atan2(numeric._value, numeric._params[0]._value)/MATH_PI*180.0;
            }
        }

        numeric._params.clear();
        return numeric;
    }

    Expression::Numeric RAND(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            long value = std::lround(numeric._value);
            numeric._value = (value <= 0) ? 0 : double(rand() % value);
        }

        return numeric;
    }

    Expression::Numeric REV16(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = double(handleRevOp(uint32_t(std::lround(numeric._value)), 16));
        }

        return numeric;
    }

    Expression::Numeric REV8(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = double(handleRevOp(uint32_t(std::lround(numeric._value)), 8));
        }

        return numeric;
    }

    Expression::Numeric REV4(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(numeric._varType == Expression::Number)
        {
            numeric._value = double(handleRevOp(uint32_t(std::lround(numeric._value)), 4));
        }

        return numeric;
    }


    // ********************************************************************************************
    // Binary Operators
    // ********************************************************************************************
    Expression::Numeric ADD(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringAdd(left, right)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value += right._value;
            return left;
        }

        left._isValid = handleDualOp("ADD", left, right, false);
        return left;
    }

    Expression::Numeric SUB(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value -= right._value;
            return left;
        }

        left._isValid = handleDualOp("SUB", left, right, false);
        return left;
    }

    Expression::Numeric AND(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = int16_t(std::lround(left._value)) & int16_t(std::lround(right._value));
            return left;
        }

        left._isValid = handleDualOp("AND", left, right, true);
        return left;
    }

    Expression::Numeric XOR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = int16_t(std::lround(left._value)) ^ int16_t(std::lround(right._value));
            return left;
        }

        left._isValid = handleDualOp("XOR", left, right, true);
        return left;
    }

    Expression::Numeric OR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = int16_t(std::lround(left._value)) | int16_t(std::lround(right._value));
            return left;
        }

        left._isValid = handleDualOp("OR", left, right, true);
        return left;
    }


    // ********************************************************************************************
    // Logical Operators
    // ********************************************************************************************
    Expression::Numeric LSL(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (int16_t(std::lround(left._value)) << int16_t(std::lround(right._value))) & 0x0000FFFF;
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar16)  &&  right._varType == Expression::Number)
        {
            if(right._value == 8)
            {
                switch(left._varType)
                {
                    // Temporary variable address
                    case Expression::TmpVar:
                    {
                        Compiler::emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(std::lround(left._value))), false);
                    }
                    break;

                    // User variable name
                    case Expression::IntVar16:
                    {
                        int varIndex = Compiler::findVar(left._name);
                        if(varIndex == -1) fprintf(stderr, "Operator::LSL() : '%s:%d' : couldn't find variable name '%s' : %s\n", moduleName.c_str(), codeLineStart, left._name.c_str(), codeLineText.c_str());
                        Compiler::emitVcpuAsm("LD", "_" + left._name, false);
                    }

                    default: break;
                }

                changeToTmpVar(left);

                Compiler::emitVcpuAsm("ST", "giga_vAC + 1", false);
                Compiler::emitVcpuAsm("ORI", "0xFF", false);
                Compiler::emitVcpuAsm("XORI", "0xFF", false);
            }
            else
            {
                handleLogicalOp("LSLW", left);
                for(uint8_t s=0; s<uint8_t(std::lround(right._value)); s++)
                {
                    Compiler::emitVcpuAsm("LSLW", "", false);
                }
            }

            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return left;
    }

    Expression::Numeric LSR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = int16_t(std::lround(left._value)) >> int16_t(std::lround(right._value));
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar16)  &&  right._varType == Expression::Number)
        {
            // Optimised high byte read
            if(right._value == 8)
            {
                switch(left._varType)
                {
                    // Temporary variable address
                    case Expression::TmpVar:
                    {
                        Compiler::emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(std::lround(left._value))) + " + 1", false);
                    }
                    break;

                    // User variable name
                    case Expression::IntVar16:
                    {
                        int varIndex = Compiler::findVar(left._name);
                        if(varIndex == -1) fprintf(stderr, "Operator::LSR() : '%s:%d' : couldn't find variable name '%s' : %s\n", moduleName.c_str(), codeLineStart, left._name.c_str(), codeLineText.c_str());
                        Compiler::emitVcpuAsm("LD", "_" + left._name + " + 1", false);
                    }
                    break;

                    default: break;
                }

                changeToTmpVar(left);
            }
            else
            {
                std::string opcode;
                switch(int16_t(std::lround(right._value)))
                {
                    case 1: opcode = "%ShiftRight1bit"; break;
                    case 2: opcode = "%ShiftRight2bit"; break;
                    case 3: opcode = "%ShiftRight3bit"; break;
                    case 4: opcode = "%ShiftRight4bit"; break;
                    case 5: opcode = "%ShiftRight5bit"; break;
                    case 6: opcode = "%ShiftRight6bit"; break;
                    case 7: opcode = "%ShiftRight7bit"; break;

                    default: break;
                }

                handleLogicalOp(opcode, left);
                Compiler::emitVcpuAsm(opcode, "", false);
            }
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric ASR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value /= (1 << int16_t(std::lround(right._value))) & 0x0000FFFF;
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar16)  &&  right._varType == Expression::Number)
        {
            std::string opcode;
            switch(int16_t(std::lround(right._value)))
            {
                case 1: opcode = "%ShiftRightSgn1bit"; break;
                case 2: opcode = "%ShiftRightSgn2bit"; break;
                case 3: opcode = "%ShiftRightSgn3bit"; break;
                case 4: opcode = "%ShiftRightSgn4bit"; break;
                case 5: opcode = "%ShiftRightSgn5bit"; break;
                case 6: opcode = "%ShiftRightSgn6bit"; break;
                case 7: opcode = "%ShiftRightSgn7bit"; break;
                case 8: opcode = "%ShiftRightSgn8bit"; break;

                default: break;
            }

            handleLogicalOp(opcode, left);
            Compiler::emitVcpuAsm(opcode, "", false);
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }


    // ********************************************************************************************
    // Conditional Operators
    // ********************************************************************************************
    Expression::Numeric EQ(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::EqOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (left._value == right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic, "XOR");

        // Convert EQ into one of the condition types of branch instruction
        std::string cc = (ccType == Expression::FastCC) ? "Ne" : "Eq"; //(!invertedLogic) ? "Eq" : "Ne";
        emitCcType(ccType, cc);

        return left;
    }

    Expression::Numeric NE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::NeOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = left._value != right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic, "XOR");

        // Convert NE into one of the condition types of branch instruction
        std::string cc = (ccType == Expression::FastCC) ? "Eq" : "Ne"; //(!invertedLogic) ? "Ne" : "Eq";
        emitCcType(ccType, cc);

        return left;
    }

    Expression::Numeric LE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::LeOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = left._value <= right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert LE into one of the condition types of branch instruction
        std::string cc = (!invertedLogic) ? "Le" : "Gt";
        emitCcType(ccType, cc);

        return left;
    }

    Expression::Numeric GE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::GeOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = left._value >= right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert GE into one of the condition types of branch instruction
        std::string cc = (!invertedLogic) ? "Ge" : "Lt";
        emitCcType(ccType, cc);

        return left;
    }

    Expression::Numeric LT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::LtOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (left._value < right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert LT into one of the condition types of branch instruction
        std::string cc = (!invertedLogic) ? "Lt" : "Ge";
        emitCcType(ccType, cc);

        return left;
    }

    Expression::Numeric GT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(handleStringCcOP(left, right, Expression::GtOP)) return left;

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (left._value > right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert GT into one of the condition types of branch instruction
        std::string cc = (!invertedLogic) ? "Gt" : "Le";
        emitCcType(ccType, cc);

        return left;
    }


    // ********************************************************************************************
    // Math Operators
    // ********************************************************************************************
    Expression::Numeric POW(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = pow(double(left._value), double(right._value));
            return left;
        }

        // Optimise base = 0
        if(left._varType == Expression::Number  &&  left._value == 0)
        {
            return Expression::Numeric(0, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }
        // Optimise base = 1
        else if(left._varType == Expression::Number  &&  left._value == 1)
        {
            return Expression::Numeric(1, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }
        // Optimise exponent = 0
        else if(right._varType == Expression::Number  &&  right._value == 0)
        {
            return Expression::Numeric(1, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? handleMathOp("CALLI", "power16bit", left, right) : handleMathOp("CALL", "power16bit", left, right);

        return left;
    }

    Expression::Numeric MUL(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value *= right._value;
            return left;
        }

        // Optimise multiply with 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? handleMathOp("CALLI", "multiply16bit", left, right) : handleMathOp("CALL", "multiply16bit", left, right);

        return left;
    }

    Expression::Numeric DIV(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (right._value == 0) ? 0 : left._value / right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? handleMathOp("CALLI", "divide16bit", left, right) : handleMathOp("CALL", "divide16bit", left, right);

        return left;
    }

    Expression::Numeric MOD(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart)
    {
        UNREFERENCED_PARAM(moduleName);
        UNREFERENCED_PARAM(codeLineText);
        UNREFERENCED_PARAM(codeLineStart);

        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (int16_t(std::lround(right._value)) == 0) ? 0 : int16_t(std::lround(left._value)) % int16_t(std::lround(right._value));
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? handleMathOp("CALLI", "divide16bit", left, right, true) : handleMathOp("CALL", "divide16bit", left, right, true);

        return left;
    }
}