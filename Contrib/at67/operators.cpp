#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "operators.h"


namespace Operators
{
    bool _nextTempVar = true;


    bool initialise(void)
    {
        _nextTempVar = true;

        return true;
    }


    void handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric)
    {
        switch(numeric._varType)
        {
            // Temporary variable address
            case Expression::TmpVar:
            {
                Compiler::emitVcpuAsm(opcodeStr, Expression::byteToHexString(uint8_t(numeric._value)), false);
            }
            break;

            // User variable name
            case Expression::IntVar:
            {
                Compiler::emitVcpuAsmUserVar(opcodeStr, numeric, false);
            }
            break;
        }

        numeric._value = uint8_t(Compiler::getTempVarStart());
        numeric._varType = Expression::TmpVar;
        numeric._name = Compiler::getTempVarStartStr();
    }

    bool handleDualOp(const std::string& opcodeStr, Expression::Numeric& lhs, Expression::Numeric& rhs, bool outputHex)
    {
        std::string opcode = std::string(opcodeStr);

        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(rhs._varType == Expression::Number  &&  uint16_t(rhs._value) > 255)
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
                (outputHex) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(lhs._value)), false) : Compiler::emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                (outputHex) ? Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(lhs._value), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
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
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        // RHS
        if(rhs._varType == Expression::Number)
        {
            Compiler::emitVcpuAsm(opcode + "I", std::to_string(rhs._value), false);
        }
        else
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(rhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar(opcode + "W", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        lhs._value = uint8_t(Compiler::getTempVarStart());
        lhs._varType = Expression::TmpVar;
        lhs._name = Compiler::getTempVarStartStr();

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return true;
    }

    bool handleLogicalOp(const std::string& opcode, Expression::Numeric& lhs, Expression::Numeric& rhs)
    {
        // SYS shift function needs this preamble, LSLW doesn't
        switch(lhs._varType)
        {
            // Temporary variable address
            case Expression::TmpVar:
            {
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
            }
            break;

            // User variable name
            case Expression::IntVar:
            {
                if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
            }
            break;
        }

        if(opcode != "LSLW"  &&  opcode != "<<") Compiler::emitVcpuAsm("STW", "mathShift", false);

        lhs._value = uint8_t(Compiler::getTempVarStart());
        lhs._varType = Expression::TmpVar;
        lhs._name = Compiler::getTempVarStartStr();

        return true;
    }

    void emitCcType(Expression::CCType ccType, std::string& cc)
    {
        switch(ccType)
        {
            case Expression::BooleanCC: (Assembler::getUseOpcodeCALLI()) ? Compiler::emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : Compiler::emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false); break;
            case Expression::NormalCC: Compiler::emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false); break;
            case Expression::FastCC: Compiler::emitVcpuAsm("B" + Expression::strToUpper(cc), "", false); break;
        }
    }
    bool handleConditionOp(Expression::Numeric& lhs, Expression::Numeric& rhs, Expression::CCType ccType, bool& invertedLogic)
    {
        // Swap left and right to take advantage of LDWI for 16bit numbers
        invertedLogic = false;
        if(rhs._varType == Expression::Number  &&  uint16_t(rhs._value) > 255)
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
                Compiler::emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
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
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        // RHS
        if(rhs._varType == Expression::Number)
        {
            Compiler::emitVcpuAsm("SUBI", std::to_string(rhs._value), false);
        }
        else        
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("SUBW", Expression::byteToHexString(uint8_t(rhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar("SUBW", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        lhs._value = uint8_t(Compiler::getTempVarStart());
        lhs._varType = Expression::TmpVar;
        lhs._name = Compiler::getTempVarStartStr();

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
                Compiler::emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
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
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", lhs, true)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        Compiler::emitVcpuAsm("STW", "mathX", false);

        // RHS
        if(rhs._varType == Expression::Number)
        {
            if(rhs._value >=0  &&  rhs._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(rhs._value), false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(rhs._value), false);
            }
        }
        else
        {
            switch(rhs._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(rhs._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    if(!Compiler::emitVcpuAsmUserVar("LDW", rhs, _nextTempVar)) return false;
                    _nextTempVar = false;
                }
                break;
            }
        }

        Compiler::emitVcpuAsm("STW", "mathY", false);

        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm(opcode, operand, false);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", operand, false);
            Compiler::emitVcpuAsm(opcode, "giga_vAC", false);
        }

        lhs._value = uint8_t(Compiler::getTempVarStart());
        lhs._varType = Expression::TmpVar;
        lhs._name = Compiler::getTempVarStartStr();
        
        if(isMod) Compiler::emitVcpuAsm("LDW", "mathRem", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return true;
    }


    // ********************************************************************************************
    // Unary Operators
    // ********************************************************************************************
    Expression::Numeric operatorPOS(Expression::Numeric& numeric)
    {
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

    Expression::Numeric operatorNEG(Expression::Numeric& numeric)
    {
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

    Expression::Numeric operatorNOT(Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            numeric._value = ~numeric._value;
            return numeric;
        }

        Compiler::getNextTempVar();
        Compiler::emitVcpuAsm("LDWI", std::to_string(-1), false);
        handleSingleOp("SUBW", numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }


    // ********************************************************************************************
    // Binary Operators
    // ********************************************************************************************
    Expression::Numeric operatorADD(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value += right._value;
            return left;
        }

        left._isValid = handleDualOp("ADD", left, right, false);
        return left;
    }

    Expression::Numeric operatorSUB(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value -= right._value;
            return left;
        }

        left._isValid = handleDualOp("SUB", left, right, false);
        return left;
    }

    Expression::Numeric operatorAND(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value &= right._value;
            return left;
        }

        left._isValid = handleDualOp("AND", left, right, true);
        return left;
    }

    Expression::Numeric operatorXOR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value ^= right._value;
            return left;
        }

        left._isValid = handleDualOp("XOR", left, right, true);
        return left;
    }

    Expression::Numeric operatorOR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value |= right._value;
            return left;
        }

        left._isValid = handleDualOp("OR", left, right, true);
        return left;
    }


    // ********************************************************************************************
    // Logical Operators
    // ********************************************************************************************
    Expression::Numeric operatorLSL(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value <<= right._value;
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar)  &&  right._varType == Expression::Number)
        {
            if(right._value == 8)
            {
                switch(left._varType)
                {
                    // Temporary variable address
                    case Expression::TmpVar:
                    {
                        Compiler::emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(left._value)), false);
                    }
                    break;

                    // User variable name
                    case Expression::IntVar:
                    {
                        int varIndex = Compiler::findVar(left._name);
                        if(varIndex == -1) fprintf(stderr, "Compiler::operatorLSL() : couldn't find variable name '%s'\n", left._name.c_str());
                        Compiler::emitVcpuAsm("LD", "_" + left._name, false);
                    }
                }

                left._value = uint8_t(Compiler::getTempVarStart());
                left._varType = Expression::TmpVar;
                left._name = Compiler::getTempVarStartStr();

                Compiler::emitVcpuAsm("ST", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())) + " + 1", false);
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Compiler::emitVcpuAsm("ANDW", "highByteMask", false);
            }
            else
            {
                std::string opcode;
                switch(right._value)
                {
                    case 1:
                    case 2:
                    case 3: opcode = "LSLW"; break;

                    case 4:
                    case 5:
                    case 6:
                    case 7: opcode = "%ShiftLeft4bit"; break;
                }

                handleLogicalOp(opcode, left, right);

                Compiler::emitVcpuAsm(opcode, "", false);

                switch(right._value)
                {
                    case 2: Compiler::emitVcpuAsm("LSLW", "", false);                                                                                     break;
                    case 3: Compiler::emitVcpuAsm("LSLW", "", false); Compiler::emitVcpuAsm("LSLW", "", false);                                           break;
                    case 5: Compiler::emitVcpuAsm("LSLW", "", false);                                                                                     break;
                    case 6: Compiler::emitVcpuAsm("LSLW", "", false); Compiler::emitVcpuAsm("LSLW", "", false);                                           break;
                    case 7: Compiler::emitVcpuAsm("LSLW", "", false); Compiler::emitVcpuAsm("LSLW", "", false); Compiler::emitVcpuAsm("LSLW", "", false); break;
                }
            }

            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return left;
    }

    Expression::Numeric operatorLSR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value >>= right._value;
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar)  &&  right._varType == Expression::Number)
        {
            // Optimised high byte read
            if(right._value == 8)
            {
                switch(left._varType)
                {
                    // Temporary variable address
                    case Expression::TmpVar:
                    {
                        Compiler::emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(left._value)) + " + 1", false);
                    }
                    break;

                    // User variable name
                    case Expression::IntVar:
                    {
                        int varIndex = Compiler::findVar(left._name);
                        if(varIndex == -1) fprintf(stderr, "Compiler::operatorLSR() : couldn't find variable name '%s'\n", left._name.c_str());
                        Compiler::emitVcpuAsm("LD", "_" + left._name + " + 1", false);
                    }
                    break;
                }

                left._value = uint8_t(Compiler::getTempVarStart());
                left._varType = Expression::TmpVar;
                left._name = Compiler::getTempVarStartStr();
            }
            else
            {
                std::string opcode;
                switch(right._value)
                {
                    case 1: opcode = "%ShiftRight1bit"; break;
                    case 2: opcode = "%ShiftRight2bit"; break;
                    case 3: opcode = "%ShiftRight3bit"; break;
                    case 4: opcode = "%ShiftRight4bit"; break;
                    case 5: opcode = "%ShiftRight5bit"; break;
                    case 6: opcode = "%ShiftRight6bit"; break;
                    case 7: opcode = "%ShiftRight7bit"; break;
                }

                handleLogicalOp(opcode, left, right);
                Compiler::emitVcpuAsm(opcode, "", false);
            }
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorASR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value /= (1 << right._value);
            return left;
        }

        Compiler::getNextTempVar();

        if((left._varType == Expression::TmpVar  ||  left._varType == Expression::IntVar)  &&  right._varType == Expression::Number)
        {
            std::string opcode;
            switch(right._value)
            {
                case 1: opcode = "%ShiftRightSgn1bit"; break;
                case 2: opcode = "%ShiftRightSgn2bit"; break;
                case 3: opcode = "%ShiftRightSgn3bit"; break;
                case 4: opcode = "%ShiftRightSgn4bit"; break;
                case 5: opcode = "%ShiftRightSgn5bit"; break;
                case 6: opcode = "%ShiftRightSgn6bit"; break;
                case 7: opcode = "%ShiftRightSgn7bit"; break;
                case 8: opcode = "%ShiftRightSgn8bit"; break;
            }

            handleLogicalOp(opcode, left, right);
            Compiler::emitVcpuAsm(opcode, "", false);
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }


    // ********************************************************************************************
    // Conditional Operators
    // ********************************************************************************************
    Expression::Numeric operatorEQ(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (left._value == right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert EQ into one of the condition types of branch instruction
        std::string cc = (ccType == Expression::FastCC) ? "Ne" : "Eq"; //(!invertedLogic) ? "Eq" : "Ne";
        emitCcType(ccType, cc);

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorNE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = left._value != right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleConditionOp(left, right, ccType, invertedLogic);

        // Convert NE into one of the condition types of branch instruction
        std::string cc = (ccType == Expression::FastCC) ? "Eq" : "Ne"; //(!invertedLogic) ? "Ne" : "Eq";
        emitCcType(ccType, cc);

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorLE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
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

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorGE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
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

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorLT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
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

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }

    Expression::Numeric operatorGT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType)
    {
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

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return left;
    }


    // ********************************************************************************************
    // Math Operators
    // ********************************************************************************************
    Expression::Numeric operatorPOW(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = int16_t(std::pow(double(left._value), double(right._value)));
            return left;
        }

        // Optimise base = 0
        if(left._varType == Expression::Number  &&  left._value == 0)
        {
            return Expression::Numeric(0, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }
        // Optimise base = 1
        else if(left._varType == Expression::Number  &&  left._value == 1)
        {
            return Expression::Numeric(1, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }
        // Optimise exponent = 0
        else if(right._varType == Expression::Number  &&  right._value == 0)
        {
            return Expression::Numeric(1, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "power16bit", left, right) : handleMathOp("CALL", "power16bit", left, right);

        return left;
    }

    Expression::Numeric operatorMUL(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value *= right._value;
            return left;
        }

        // Optimise multiply with 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "multiply16bit", left, right) : handleMathOp("CALL", "multiply16bit", left, right);

        return left;
    }

    Expression::Numeric operatorDIV(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (right._value == 0) ? 0 : left._value / right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "divide16bit", left, right) : handleMathOp("CALL", "divide16bit", left, right);

        return left;
    }

    Expression::Numeric operatorMOD(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(left._varType == Expression::Number  &&  right._varType == Expression::Number)
        {
            left._value = (right._value == 0) ? 0 : left._value % right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((left._varType == Expression::Number  &&  left._value == 0)  ||  (right._varType == Expression::Number  &&  right._value == 0))
        {
            return Expression::Numeric(0, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "divide16bit", left, right, true) : handleMathOp("CALL", "divide16bit", left, right, true);

        return left;
    }
}