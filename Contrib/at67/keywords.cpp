#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "keywords.h"
#include "operators.h"


namespace Keywords
{
    enum EmitStringResult {SyntaxError, InvalidStringVar, ValidStringVar};

    std::vector<std::string> _operators;
    std::map<std::string, Keyword> _keywords;
    std::map<std::string, std::string> _functions;
    std::map<std::string, std::string> _stringKeywords;
    std::map<std::string, std::string> _equalsKeywords;


    std::vector<std::string>& getOperators(void)                {return _operators;     }
    std::map<std::string, Keyword>& getKeywords(void)           {return _keywords;      }
    std::map<std::string, std::string>& getFunctions(void)      {return _functions;     }
    std::map<std::string, std::string>& getStringKeywords(void) {return _stringKeywords;}
    std::map<std::string, std::string>& getEqualsKeywords(void) {return _equalsKeywords;}


    bool initialise(void)
    {
        _operators.push_back(" AND ");
        _operators.push_back(" XOR ");
        _operators.push_back(" OR " );
        _operators.push_back(" NOT ");
        _operators.push_back(" MOD ");
        _operators.push_back(" LSL ");
        _operators.push_back(" LSR ");
        _operators.push_back(" ASR ");
        _operators.push_back("<<"   );
        _operators.push_back(">>"   );

        _functions["PEEK"] = {"PEEK"};
        _functions["DEEK"] = {"DEEK"};
        _functions["USR" ] = {"USR" };
        _functions["RND" ] = {"RND" };
        _functions["LEN" ] = {"LEN" };
        _functions["ABS" ] = {"ABS" };
        _functions["ACS" ] = {"ACS" };
        _functions["ASC" ] = {"ASC" };
        _functions["ASN" ] = {"ASN" };
        _functions["ATN" ] = {"ATN" };
        _functions["COS" ] = {"COS" };
        _functions["EXP" ] = {"EXP" };
        _functions["INT" ] = {"INT" };
        _functions["LOG" ] = {"LOG" };
        _functions["SIN" ] = {"SIN" };
        _functions["SQR" ] = {"SQR" };
        _functions["TAN" ] = {"TAN" };
        _functions["FRE" ] = {"FRE" };
        _functions["TIME"] = {"TIME"};

        _keywords["REM"   ] = {"REM",    keywordREM,    Compiler::SingleStatementParsed};
        _keywords["LET"   ] = {"LET",    keywordLET,    Compiler::SingleStatementParsed};
        _keywords["END"   ] = {"END",    keywordEND,    Compiler::SingleStatementParsed};
        _keywords["INC"   ] = {"INC",    keywordINC,    Compiler::SingleStatementParsed};
        _keywords["ON"    ] = {"ON",     keywordON,     Compiler::SingleStatementParsed};
        _keywords["GOTO"  ] = {"GOTO",   keywordGOTO,   Compiler::SingleStatementParsed};
        _keywords["GOSUB" ] = {"GOSUB",  keywordGOSUB,  Compiler::SingleStatementParsed};
        _keywords["RETURN"] = {"RETURN", keywordRETURN, Compiler::SingleStatementParsed};
        _keywords["CLS"   ] = {"CLS",    keywordCLS,    Compiler::SingleStatementParsed};
        _keywords["?"     ] = {"?",      keywordPRINT,  Compiler::SingleStatementParsed};
        _keywords["PRINT" ] = {"PRINT",  keywordPRINT,  Compiler::SingleStatementParsed};
        _keywords["INPUT" ] = {"INPUT",  keywordINPUT,  Compiler::SingleStatementParsed};
        _keywords["FOR"   ] = {"FOR",    keywordFOR,    Compiler::SingleStatementParsed};
        _keywords["NEXT"  ] = {"NEXT",   keywordNEXT,   Compiler::SingleStatementParsed};
        _keywords["IF"    ] = {"IF",     keywordIF,     Compiler::MultiStatementParsed };
        _keywords["ELSEIF"] = {"ELSEIF", keywordELSEIF, Compiler::SingleStatementParsed};
        _keywords["ELSE"  ] = {"ELSE",   keywordELSE,   Compiler::SingleStatementParsed};
        _keywords["ENDIF" ] = {"ENDIF",  keywordENDIF,  Compiler::SingleStatementParsed};
        _keywords["WHILE" ] = {"WHILE",  keywordWHILE,  Compiler::SingleStatementParsed};
        _keywords["WEND"  ] = {"WEND",   keywordWEND,   Compiler::SingleStatementParsed};
        _keywords["REPEAT"] = {"REPEAT", keywordREPEAT, Compiler::SingleStatementParsed};
        _keywords["UNTIL" ] = {"UNTIL",  keywordUNTIL,  Compiler::SingleStatementParsed};
        _keywords["CONST" ] = {"CONST",  keywordCONST,  Compiler::SingleStatementParsed};
        _keywords["DIM"   ] = {"DIM",    keywordDIM,    Compiler::SingleStatementParsed};
        _keywords["DEF"   ] = {"DEF",    keywordDEF,    Compiler::SingleStatementParsed};
        _keywords["AT"    ] = {"AT",     keywordAT,     Compiler::SingleStatementParsed};
        _keywords["PUT"   ] = {"PUT",    keywordPUT,    Compiler::SingleStatementParsed};
        _keywords["MODE"  ] = {"MODE",   keywordMODE,   Compiler::SingleStatementParsed};
        _keywords["WAIT"  ] = {"WAIT",   keywordWAIT,   Compiler::SingleStatementParsed};
        _keywords["LINE"  ] = {"LINE",   keywordLINE,   Compiler::SingleStatementParsed};
        _keywords["HLINE" ] = {"HLINE",  keywordHLINE,  Compiler::SingleStatementParsed};
        _keywords["VLINE" ] = {"VLINE",  keywordVLINE,  Compiler::SingleStatementParsed};
        _keywords["SCROLL"] = {"SCROLL", keywordSCROLL, Compiler::SingleStatementParsed};
        _keywords["POKE"  ] = {"POKE",   keywordPOKE,   Compiler::SingleStatementParsed};
        _keywords["DOKE"  ] = {"DOKE",   keywordDOKE,   Compiler::SingleStatementParsed};
        _keywords["PLAY"  ] = {"PLAY",   keywordPLAY,   Compiler::SingleStatementParsed};

        _stringKeywords["CHR$"  ] = {"CHR$"  };
        _stringKeywords["HEX$"  ] = {"HEX$"  };
        _stringKeywords["HEXW$" ] = {"HEXW$" };
        _stringKeywords["MID$"  ] = {"MID$"  };
        _stringKeywords["LEFT$" ] = {"LEFT$" };
        _stringKeywords["RIGHT$"] = {"RIGHT$"};
        _stringKeywords["SPC$"  ] = {"SPC$"  };
        _stringKeywords["STR$"  ] = {"STR$"  };
        _stringKeywords["TIME$" ] = {"TIME$" };

        _equalsKeywords["CONST" ] = {"CONST" };
        _equalsKeywords["DIM"   ] = {"DIM"   };
        _equalsKeywords["DEF"   ] = {"DEF"   };
        _equalsKeywords["FOR"   ] = {"FOR"   };
        _equalsKeywords["IF"    ] = {"IF"    };
        _equalsKeywords["ELSEIF"] = {"ELSEIF"};
        _equalsKeywords["WHILE" ] = {"WHILE" };
        _equalsKeywords["UNTIL" ] = {"UNTIL" };

        return true;
    }


    bool findKeyword(std::string code, const std::string& keyword, size_t& foundPos)
    {
        Expression::strToUpper(code);
        foundPos = code.find(keyword);
        if(foundPos != std::string::npos)
        {
            foundPos += keyword.size();
            return true;
        }
        return false;
    }

    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, int tokenIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        std::string key = keyword;
        Expression::strToUpper(key);
        if(_keywords.find(key) == _keywords.end()) return KeywordNotFound;

        // Handle keyword in code line
        if(findKeyword(key, _keywords[key]._name, foundPos)  &&  _keywords[key]._func)
        {
            bool success = _keywords[key]._func(codeLine, codeLineIndex, tokenIndex, foundPos, result);
            return (!success) ? KeywordError : KeywordFound;
        }

        return KeywordFound;
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

    void getOrCreateString(Expression::Numeric& numeric, std::string& name, uint16_t& addr, int& index)
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
        }
    }

    void handleConstantString(Expression::Numeric& numeric, Compiler::ConstStrType constStrType, std::string& name, int& index)
    {
        switch(constStrType)
        {
            case Compiler::StrLeft:
            case Compiler::StrRight:
            {
                uint8_t length = uint8_t(numeric._parameters[0]._value);
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, 0, index);
            }
            break;

            case Compiler::StrMid:
            {
                uint8_t offset = uint8_t(numeric._parameters[0]._value);
                uint8_t length = uint8_t(numeric._parameters[1]._value);
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, offset, index);
            }
            break;
        }

        name = Compiler::getStringVars()[index]._name;
        uint16_t srcAddr = Compiler::getStringVars()[index]._address;

        if(Expression::getEnablePrint())
        {
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("%PrintAcString", "", false);
        }
        else
        {
            int index = Expression::getOutputNumeric()._index;
            uint16_t dstAddr = Compiler::getStringVars()[index]._address;
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
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
                Compiler::emitVcpuAsm("LDI", std::to_string(param._value), false);
            }
            // 16bit
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(param._value), false);
            }

            return;
        }

        Operators::handleSingleOp("LDW", param);
    }


    // ********************************************************************************************
    // Functions
    // ********************************************************************************************
    Expression::Numeric functionPEEK(Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : 
                                                              Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("PEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionDEEK(Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : 
                                                              Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("DEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionUSR(Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("CALLI", Expression::byteToHexString(uint8_t(numeric._value)), false) : 
                                                                  Compiler::emitVcpuAsm("CALLI", Expression::wordToHexString(numeric._value), false);
            }
            else
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : 
                                                                  Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
            }
        }

        Compiler::getNextTempVar();

        if(Assembler::getUseOpcodeCALLI())
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

    Expression::Numeric functionRND(Expression::Numeric& numeric)
    {
        bool useMod = true;
        if(numeric._varType == Expression::Number)
        {
            // RND(0) skips the MOD call and allows you to filter the output manually
            if(numeric._value == 0)
            {
                useMod = false;
            }
            else
            {
                (numeric._value > 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : 
                                                                 Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
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
            numeric._value = uint8_t(Compiler::getTempVarStart());
            numeric._varType = Expression::TmpVar;
            numeric._name = Compiler::getTempVarStartStr();

            Compiler::emitVcpuAsm("%Rand", "", false);
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionARR(Expression::Numeric& numeric)
    {
        Compiler::getNextTempVar();

        int intSize = Compiler::getIntegerVars()[numeric._index]._intSize;
        uint16_t arrayPtr = Compiler::getIntegerVars()[numeric._index]._array;

        // Literal array index
        if(numeric._parameters[0]._varType == Expression::Number)
        {
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + numeric._parameters[0]._value*intSize), false);
            Compiler::emitVcpuAsm("DEEK", "", false);

            numeric._value = uint8_t(Compiler::getTempVarStart());
            numeric._varType = Expression::TmpVar;
            numeric._name = Compiler::getTempVarStartStr();
        }
        // Variable array index
        else
        {
            // Can't call Operators::handleSingleOp() here, so special case it
            switch(numeric._parameters[0]._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(numeric._parameters[0]._value)), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    Compiler::emitVcpuAsmUserVar("LDW", numeric._parameters[0], false);
                }
                break;
            }

            numeric._value = uint8_t(Compiler::getTempVarStart());
            numeric._varType = Expression::TmpVar;
            numeric._name = Compiler::getTempVarStartStr();

#ifdef SMALL_CODE_SIZE
            // Saves 2 bytes per array access but costs an extra 2 instructions in performance
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("STW", "memIndex", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                Compiler::emitVcpuAsm("CALLI", "getArrayInt16", false);
            }
            else
#endif
            {
                Compiler::emitVcpuAsm("STW", "register2", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                Compiler::emitVcpuAsm("ADDW", "register2", false);
                Compiler::emitVcpuAsm("ADDW", "register2", false);
                Compiler::emitVcpuAsm("DEEK", "",          false);
            }
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionLEN(Expression::Numeric& numeric)
    {
        if(numeric._varType != Expression::Number)
        {
            Compiler::getNextTempVar();

            if(numeric._index == -1  &&  numeric._varType != Expression::TmpStrVar)
            {
                fprintf(stderr, "Compiler::functionLEN() : couldn't find variable name '%s'\n", numeric._name.c_str());
                return numeric;
            }

            int length;
            switch(numeric._varType)
            {
                case Expression::IntVar:   length = Compiler::getIntegerVars()[numeric._index]._intSize; break;
                case Expression::ArrVar:   length = Compiler::getIntegerVars()[numeric._index]._arrSize; break;
                case Expression::StrVar:   length = Compiler::getStringVars()[numeric._index]._maxSize;  break;
                case Expression::Constant: length = Compiler::getConstants()[numeric._index]._size;      break;
            }

            // Variable lengths
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
            // Constants lengths
            else
            {
                (length <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(length), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(length), false);
            }

            numeric._value = uint8_t(Compiler::getTempVarStart());
            numeric._varType = Expression::TmpVar;
            numeric._name = Compiler::getTempVarStartStr();

            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            
        }

        return numeric;
    }

    Expression::Numeric functionCHR$(Expression::Numeric& numeric)
    {
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
            if(Expression::getEnablePrint())
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(numeric._value), false);
                Compiler::emitVcpuAsm("%PrintAcChar", "", false);
                return numeric;
            }

            // Create CHR string
            Compiler::emitVcpuAsm("LDI", std::to_string(numeric._value), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringChr", "", false);

            return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint())
        {
            Compiler::emitVcpuAsm("%PrintAcChar", "", false);
            return numeric;
        }

        // Create CHR string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringChr", "", false);

        return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionHEX$(Expression::Numeric& numeric)
    {
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
            if(Expression::getEnablePrint())
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false);
                Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
                return numeric;
            }

            // Create HEX string
            Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringHex", "", false);

            return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
            return numeric;
        }

        // Create HEX string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringHex", "", false);

        return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionHEXW$(Expression::Numeric& numeric)
    {
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
            if(Expression::getEnablePrint())
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
                Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
                return numeric;
            }

            // Create HEXW string
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
            Compiler::emitVcpuAsm("STW", "strHex", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringHexw", "", false);

            return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
            return numeric;
        }

        // Create HEXW string
        Compiler::emitVcpuAsm("STW", "strHex", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringHexw", "", false);

        return Expression::Numeric(dstAddr, index, true, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionLEFT$(Expression::Numeric& numeric)
    {
        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrLeft, name, index);

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnablePrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcLeft", "", false);
            }
            else
            {
                int index = Expression::getOutputNumeric()._index;
                uint16_t dstAddr = Compiler::getStringVars()[index]._address;

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringLeft", "", false);
            }

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric functionRIGHT$(Expression::Numeric& numeric)
    {
        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrRight, name, index);

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnablePrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcRight", "", false);
            }
            else
            {
                int index = Expression::getOutputNumeric()._index;
                uint16_t dstAddr = Compiler::getStringVars()[index]._address;

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringRight", "", false);
            }

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric functionMID$(Expression::Numeric& numeric)
    {
        // Literal string and parameters, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 2  &&  numeric._parameters[0]._varType == Expression::Number  &&  
           numeric._parameters[1]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrMid, name, index);

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 2)
        {
            std::string name;
            uint16_t srcAddr;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnablePrint())
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
                int index = Expression::getOutputNumeric()._index;
                uint16_t dstAddr = Compiler::getStringVars()[index]._address;

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strOffset", false);
                handleStringParameter(numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringMid", "", false);
            }

            return Expression::Numeric(0, index, true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }


    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool keywordREM(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove REM and everything after it in code
        codeLine._code.erase(foundPos, codeLine._code.size() - foundPos);

        // Remove REM and everything after it in expression
        size_t rem;
        std::string expr = codeLine._expression;
        Expression::strToUpper(expr);
        if((rem = expr.find("REM")) != std::string::npos  ||  (rem = expr.find("'")) != std::string::npos)
        {
            codeLine._expression.erase(rem, codeLine._expression.size() - rem);
        }

        // Remove REM and everything after it in tokens
        for(int i=0; i<codeLine._tokens.size(); i++)
        {
            std::string str = codeLine._tokens[i];
            Expression::strToUpper(str);
            if(str.find("REM") != std::string::npos  ||  str.find("'") != std::string::npos)
            {
                codeLine._tokens.erase(codeLine._tokens.begin() + i, codeLine._tokens.end());
                break;
            }
        }

        return true;
    }

    bool keywordLET(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove LET from code
        codeLine._code.erase(foundPos, 3);

        size_t let;
        std::string expr = codeLine._expression;
        Expression::strToUpper(expr);
        if((let = expr.find("LET")) != std::string::npos)
        {
            codeLine._expression.erase(let, 3);
        }

        for(int i=0; i<codeLine._tokens.size(); i++)
        {
            std::string str = codeLine._tokens[i];
            Expression::strToUpper(str);
            if((let = expr.find("LET")) != std::string::npos)
            {
                codeLine._tokens[i].erase(let, 3);
                break;
            }
        }

        return true;
    }

    bool keywordEND(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string labelName = "_end_" + Expression::wordToHexString(Compiler::getVasmPC());
        Compiler::emitVcpuAsm("BRA", labelName, false, codeLineIndex, labelName);

        return true;
    }

    bool keywordINC(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Operand must be an integer var
        std::string varToken = codeLine._code.substr(foundPos);
        Expression::stripWhitespace(varToken);
        int varIndex = Compiler::findVar(varToken, false);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::keywordINC() : Syntax error, integer variable '%s' not found, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Compiler::emitVcpuAsm("INC", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);

        return true;
    }

    bool keywordON(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        int onIndex = -1;
        int gotoIndex = -1;
        int numLabels = 0;

        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t gotoOffset = code.find("GOTO");
        size_t gosubOffset = code.find("GOSUB");
        if(gotoOffset == std::string::npos  &&  gosubOffset == std::string::npos)
        {
            fprintf(stderr, "Compiler::keywordON() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        size_t gSize = (gotoOffset != std::string::npos) ? 4 : 5;
        size_t gOffset = (gotoOffset != std::string::npos) ? gotoOffset : gosubOffset;

        // Parse ON field
        Expression::Numeric onValue;
        std::string onToken = codeLine._code.substr(foundPos, gOffset - (foundPos + 1));
        Expression::stripWhitespace(onToken);
        uint32_t expressionType = Compiler::parseExpression(codeLine, codeLineIndex, onToken, onValue);
        Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);

        // Parse labels
        std::vector<size_t> gOffsets;
        std::vector<std::string> gotoTokens = Expression::tokenise(codeLine._code.substr(gOffset + gSize), ',', gOffsets, false);
        if(gotoTokens.size() < 1)
        {
            fprintf(stderr, "Compiler::keywordON() : Syntax error, must have at least one label after GOTO, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Create label LUT
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
        for(int i=0; i<gotoTokens.size(); i++)
        {
            std::string gotoLabel = gotoTokens[i];
            Expression::stripWhitespace(gotoLabel);
            int labelIndex = Compiler::findLabel(gotoLabel);
            if(labelIndex == -1)
            {
                fprintf(stderr, "Compiler::keywordON() : invalid label %s in slot %d in '%s' on line %d\n", gotoLabel.c_str(), i, codeLine._text.c_str(), codeLineIndex + 1);
                Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
                return false;
            }
                
            // Only ON GOSUB needs a PUSH, (emitted in createVasmCode())
            if(gosubOffset != std::string::npos) Compiler::getLabels()[labelIndex]._gosub = true;

            // Create lookup table out of label addresses
            Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.push_back(labelIndex);
        }

        // Allocate giga memory for LUT
        int size = int(gotoTokens.size()) * 2;
        uint16_t address;
        if(!Memory::giveFreeRAM(Memory::FitAscending, size, 0x0200, Compiler::getRuntimeStart(), address))
        {
            fprintf(stderr, "Compiler::keywordON() : Not enough RAM for onGotoGosub LUT of size %d\n", size);
            return false;
        }
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._address = address;
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name = "lut_" + Expression::wordToHexString(address);

        Compiler::emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
        Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
        Compiler::emitVcpuAsm("LDWI", Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name, false, codeLineIndex);
        Compiler::emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
#ifdef ARRAY_INDICES_ONE
        Compiler::emitVcpuAsm("SUBI", "2",         false, codeLineIndex);  // enable this to start at 1 instead of 0
#endif
        Compiler::emitVcpuAsm("DEEK", "",          false, codeLineIndex);
        Compiler::emitVcpuAsm("CALL", "giga_vAC",  false, codeLineIndex);

        return true;
    }

    bool keywordGOTO(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse labels
        std::vector<size_t> gotoOffsets;
        std::vector<std::string> gotoTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gotoOffsets, false);
        if(gotoTokens.size() < 1  ||  gotoTokens.size() > 2)
        {
            fprintf(stderr, "Compiler::keywordGOTO() : Syntax error, must have one or two parameters, e.g. 'GOTO 200' or 'GOTO k+1,default' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Parse GOTO field
        Expression::Numeric gotoValue;
        std::string gotoToken = gotoTokens[0];
        Expression::stripWhitespace(gotoToken);

        bool useBRA = false;
        if(gotoToken[0] == '&')
        {
            useBRA = true;
            gotoToken.erase(0, 1);
        }

        int labelIndex = Compiler::findLabel(gotoToken);
        if(labelIndex == -1)
        {
            Compiler::setCreateNumericLabelLut(true);

            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, gotoToken, gotoValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gotoTokens.size() == 2)
            {
                std::string defaultToken = gotoTokens[1];
                Expression::stripWhitespace(defaultToken);
                int labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Compiler::keywordGOTO() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gotoNumericLabel
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "gotoNumericLabel", false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "gotoNumericLabel", false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",         false, codeLineIndex);
            }

            return true;
        }

        // Within same page, (validation check on same page branch may fail after outputCode(), user will be warned)
        if(useBRA)
        {
            Compiler::emitVcpuAsm("BRA", "_" + gotoToken, false, codeLineIndex);
        }
        // Long jump
        else
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "_" + gotoToken, false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "_" + gotoToken, false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordGOSUB(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse labels
        std::vector<size_t> gosubOffsets;
        std::vector<std::string> gosubTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gosubOffsets, false);
        if(gosubTokens.size() < 1  ||  gosubTokens.size() > 2)
        {
            fprintf(stderr, "Compiler::keywordGOSUB() : Syntax error, must have one or two parameters, e.g. 'GOSUB 200' or 'GOSUB k+1,default' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Parse GOSUB field
        Expression::Numeric gosubValue;
        std::string gosubToken = gosubTokens[0];
        Expression::stripWhitespace(gosubToken);
        int labelIndex = Compiler::findLabel(gosubToken);
        if(labelIndex == -1)
        {
            Compiler::setCreateNumericLabelLut(true);

            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, gosubToken, gosubValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gosubTokens.size() == 2)
            {
                std::string defaultToken = gosubTokens[1];
                Expression::stripWhitespace(defaultToken);
                int labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Compiler::keywordGOSUB() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                Compiler::getLabels()[labelIndex]._gosub = true;
                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gosubNumericLabel
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "gosubNumericLabel", false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "gosubNumericLabel", false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",         false, codeLineIndex);
            }

            return true;
        }

        // CALL label
        Compiler::getLabels()[labelIndex]._gosub = true;

        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "_" + gosubToken, false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "_" + gosubToken, false, codeLineIndex);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
        }

        return true;
    }

    bool keywordRETURN(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::emitVcpuAsm("POP", "", false, codeLineIndex);
        Compiler::emitVcpuAsm("RET", "", false, codeLineIndex);

        return true;
    }

    bool keywordCLS(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() > 2)
        {
            fprintf(stderr, "Compiler::keywordCLS() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        if(codeLine._tokens.size() == 2)
        {
            Expression::Numeric param;
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, codeLine._tokens[1], param);
            Compiler::emitVcpuAsm("STW", "clsAddress", false, codeLineIndex);
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "clearScreen", false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "clearScreen", false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",        false, codeLineIndex);
            }
        }
        else
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "clearVertBlinds", false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "clearVertBlinds", false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",        false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordPRINT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse print tokens
        //std::vector<std::string> tokens = Expression::tokeniseLine(codeLine._code.substr(foundPos), ";");
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', false, false);

        for(int i=0; i<tokens.size(); i++)
        {
            Expression::Numeric numeric;
            int varIndex = -1, constIndex = -1, strIndex = -1;
            uint32_t expressionType = Compiler::isExpression(tokens[i], varIndex, constIndex, strIndex);

            if((expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
            {
                // Gigatron prints text on the fly without creating strings
                Expression::setEnablePrint(true);
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Expression::setEnablePrint(false);
            }
            else if((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(numeric._value), false, codeLineIndex);
                }
                else
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasIntVars)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                if(varIndex >= 0)
                {
                    if(Compiler::getIntegerVars()[varIndex]._varType == Compiler::VarArray)
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                    }
                    else
                    {
                        switch(numeric._int16Byte)
                        {
                            case Expression::Int16Low:  Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name,          false, codeLineIndex); break;
                            case Expression::Int16High: Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name + " + 1", false, codeLineIndex); break;
                            case Expression::Int16Both: Compiler::emitVcpuAsm("LDW", "_" + Compiler::getIntegerVars()[varIndex]._name,          false, codeLineIndex); break;
                        }

                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                    }
                }
                else
                {
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasStrVars)
            {
                if(strIndex >= 0)
                {
                    std::string strName = Compiler::getStringVars()[strIndex]._name;
                    Compiler::emitVcpuAsm("%PrintString", "_" + strName, false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasKeywords)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
            }
            else if(expressionType & Expression::HasOperators)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(numeric._value), false, codeLineIndex);
            }
            else if(expressionType & Expression::HasStrings)
            {
                size_t lquote = tokens[i].find_first_of("\"");
                size_t rquote = tokens[i].find_first_of("\"", lquote + 1);
                if(lquote != std::string::npos  &&  rquote != std::string::npos)
                {
                    if(rquote == lquote + 1) continue; // skip empty strings
                    std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));

                    // Create string
                    std::string name;
                    uint16_t address;
                    if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", "_" + name, false, codeLineIndex);
                }
            }
            else if(expressionType == Expression::HasStrConsts)
            {
                // Print constant string
                std::string internalName = Compiler::getConstants()[constIndex]._internalName;
                Compiler::emitVcpuAsm("%PrintString", "_" + internalName, false, codeLineIndex);
            }
            else if(expressionType == Expression::HasIntConsts)
            {
                // Print constant int
                int16_t data = Compiler::getConstants()[constIndex]._data;
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(data), false, codeLineIndex);
            }
            else if(expressionType == Expression::HasNumbers)
            {
                // If valid expression
                if(Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(numeric._value), false, codeLineIndex);
                }
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "newLineScroll", false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "newLineScroll", false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordINPUT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() < 2  ||  codeLine._tokens.size() > 3)
        {
            fprintf(stderr, "Compiler::keywordINPUT() : Syntax error in INPUT statement, must be 'INPUT <optional string>, <int/str var>', in : '%s' : on line %d\n", codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string varToken;
        if(codeLine._tokens.size() == 3)
        {
            if(Expression::isValidString(codeLine._tokens[1]))
            {
                Expression::stripNonStringWhitespace(codeLine._tokens[1]);
                size_t lquote = codeLine._tokens[1].find_first_of("\"");
                size_t rquote = codeLine._tokens[1].find_first_of("\"", lquote + 1);
                if(lquote != std::string::npos  &&  rquote != std::string::npos)
                {
                    // Skip empty strings
                    if(rquote > lquote + 1)
                    {
                        std::string str = codeLine._tokens[1].substr(lquote + 1, rquote - (lquote + 1));

                        // Create string
                        std::string name;
                        uint16_t address;
                        if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                        // Print string
                        Compiler::emitVcpuAsm("%PrintString", "_" + name, false, codeLineIndex);
                    }
                }

                varToken = codeLine._tokens[2];
            }
            else
            {
                fprintf(stderr, "Compiler::keywordINPUT() : Syntax error in INPUT statement, first parameter must be a string, in : '%s' : on line %d\n", codeLine._code.c_str(), codeLineIndex + 1);
                return false;
            }
        }
        else
        {
            varToken = codeLine._tokens[1];
        }
        
        Expression::stripWhitespace(varToken);
        int intVar = Compiler::findVar(varToken);
        if(intVar >= 0)
        {
            Compiler::emitVcpuAsm("LDW", Expression::wordToHexString(Compiler::getIntegerVars()[intVar]._address), false, codeLineIndex);
            return true;
        }
        else
        {
            if(varToken.back() == '$'  &&  Expression::isVarNameValid(varToken))
            {
                int strIndex = Compiler::findStr(varToken);
                if(strIndex >= 0)
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[strIndex]._address), false, codeLineIndex);
                    return true;
                }
            }
        }

        fprintf(stderr, "Compiler::keywordINPUT() : variable '%s' does not exist, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineIndex + 1);
        return false;
    }

    bool keywordFOR(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        bool optimise = true;
        int varIndex, constIndex, strIndex;
        uint32_t expressionType;

        // Parse first line of FOR loop
        bool foundStep = false;
        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t equals, to, step;
        if((equals = code.find("=")) == std::string::npos)
        {
            fprintf(stderr, "Compiler::keywordFOR() : Syntax error, (missing '='), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        if((to = code.find("TO")) == std::string::npos)
        {
            fprintf(stderr, "Compiler::keywordFOR() : Syntax error, (missing '='), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        step = code.find("STEP");

        // Maximum of 4 nested loops
        if(Compiler::getForNextDataStack().size() == 4)
        {
            fprintf(stderr, "Compiler::keywordFOR() : Syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Nested loops temporary variables
        int offset = int(Compiler::getForNextDataStack().size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        // Loop start
        int16_t loopStart = 0;
        std::string startToken = codeLine._code.substr(equals + 1, to - (equals + 1));
        Expression::stripWhitespace(startToken);
        expressionType = Compiler::isExpression(startToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;

        // Var counter, (create or update if being reused)
        std::string var = codeLine._code.substr(foundPos, equals - foundPos);
        Expression::stripWhitespace(var);
        int varCounter = Compiler::findVar(var);
        (varCounter < 0) ? Compiler::createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varCounter) : Compiler::updateVar(loopStart, codeLine, varCounter, false);

        // Loop end
        int16_t loopEnd = 0;
        size_t end = (step == std::string::npos) ? codeLine._code.size() : step;
        std::string endToken = codeLine._code.substr(to + 2, end - (to + 2));
        Expression::stripWhitespace(endToken);
        expressionType = Compiler::isExpression(endToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;

        // Loop step
        int16_t loopStep = 1;
        std::string stepToken;
        if(step != std::string::npos)
        {
            end = codeLine._code.size();
            stepToken = codeLine._code.substr(step + 4, end - (step + 4));
            Expression::stripWhitespace(stepToken);
            expressionType = Compiler::isExpression(stepToken, varIndex, constIndex, strIndex);
            if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;
        }

        Expression::Numeric startNumeric, endNumeric, stepNumeric;
        if(optimise)
        {
            // Parse start
            Expression::parse(startToken, codeLineIndex, startNumeric);
            loopStart = startNumeric._value;

            // Parse end
            Expression::parse(endToken, codeLineIndex, endNumeric);
            loopEnd = endNumeric._value;

            // Parse step
            if(stepToken.size())
            {
                Expression::parse(stepToken, codeLineIndex, stepNumeric);
                loopStep = stepNumeric._value;
            }
            else
            {
                // Auto step based on start and end
                loopStep = (loopEnd >= loopStart) ? 1 : -1;
            }

            // Optimised case for 8bit constants
            if(optimise  &&  startNumeric._isValid  &&  loopStart >= 0  &&  loopStart <= 255  &&  endNumeric._isValid  &&  loopEnd >= 0  &&  loopEnd <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStart), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);
            }
            // 16bit constants
            else
            {
                optimise = false;

                Compiler::emitVcpuAsm("LDWI", std::to_string(loopStart), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);
                Compiler::emitVcpuAsm("LDWI", std::to_string(loopEnd), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);
                Compiler::emitVcpuAsm("LDWI", std::to_string(loopStep), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
            }
        }
        else
        {
            // Parse start
            parseExpression(codeLine, codeLineIndex, startToken, startNumeric);
            loopStart = startNumeric._value;
            Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);

            // Parse end
            parseExpression(codeLine, codeLineIndex, endToken, endNumeric);
            loopEnd = endNumeric._value;
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);

            // Parse step
            if(stepToken.size())
            {
                parseExpression(codeLine, codeLineIndex, stepToken, stepNumeric);
                loopStep = stepNumeric._value;
            }
            else
            {
                loopStep = 1;
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStep), false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
        }

        // Label and stack
        Compiler::setNextInternalLabel("_next_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getForNextDataStack().push({varCounter, Compiler::getNextInternalLabel(), loopEnd, loopStep, varEnd, varStep, optimise, codeLineIndex});

        return true;
    }

    bool keywordNEXT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = Compiler::findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, (bad var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Pop stack for this nested loop
        if(Compiler::getForNextDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, missing FOR statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        Compiler::ForNextData forNextData = Compiler::getForNextDataStack().top();
        Compiler::getForNextDataStack().pop();

        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, (wrong var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string varName = Compiler::getIntegerVars()[varIndex]._name;
        std::string labName = forNextData._labelName;
        int16_t loopEnd = forNextData._loopEnd;
        uint16_t varEnd = forNextData._varEnd;
        uint16_t varStep = forNextData._varStep;

        if(forNextData._optimise)
        {
            // Positive step
            if(forNextData._loopStep > 0)
            {
                Compiler::emitVcpuAsm("%ForNextLoopUp", "_" + varName + " " + labName + " " + std::to_string(loopEnd), false, codeLineIndex);
            }
            // Negative step
            else
            {
                Compiler::emitVcpuAsm("%ForNextLoopDown", "_" + varName + " " + labName + " " + std::to_string(loopEnd), false, codeLineIndex);
            }
        }
        else
        {
            // Positive step
            if(forNextData._loopStep > 0)
            {
                Compiler::emitVcpuAsm("%ForNextLoopStepUp", "_" + varName + " " + labName + " " + Expression::byteToHexString(uint8_t(varEnd)) + " " + Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("%ForNextLoopStepDown", "_" + varName + " " + labName + " " + Expression::byteToHexString(uint8_t(varEnd)) + " " + Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        bool ifElseEndif = false;

        // IF
        std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
        Expression::strToUpper(code);
        size_t offsetIF = code.find("IF");

        // THEN
        code = codeLine._code;
        Expression::strToUpper(code);
        size_t offsetTHEN = code.find("THEN");
        if(offsetTHEN == std::string::npos) ifElseEndif = true;

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos, offsetTHEN - foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        int jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        // Bail early as we assume this is an IF ELSE ENDIF block
        if(ifElseEndif)
        {
            Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::IfBlock, condition._ccType});
            return true;
        }

        // Action
        std::string actionToken = Compiler::getCodeLines()[codeLineIndex]._code.substr(offsetIF + offsetTHEN + 4);
        if(actionToken.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordIF() : Syntax error, IF THEN <action>, (missing action), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::trimWhitespace(actionToken);
        std::string actionText = Expression::collapseWhitespaceNotStrings(actionToken);

        // Multi-statements
        int varIndex, strIndex;
        if(Compiler::parseMultiStatements(actionText, codeLine, codeLineIndex, varIndex, strIndex) == Compiler::StatementError) return false;

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLineIndex]._vasm[jmpIndex];
        switch(condition._ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, nextInternalLabel);                            break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, Compiler::getNextInternalLabel());             break;
        }

        return true;
    }

    bool keywordELSEIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordELSEIF() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        if(elseIfData._ifElseEndType != Compiler::IfBlock  &&  elseIfData._ifElseEndType != Compiler::ElseIfBlock)
        {
            fprintf(stderr, "Compiler::keywordELSEIF() : Syntax error, ELSEIF follows IF or ELSEIF, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Jump to endif for previous BASIC line
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1});
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_elseif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;
        }

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::ElseIfBlock, condition._ccType});

        return true;
    }

    bool keywordELSE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Compiler::keywordELSE() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordELSE() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        // Jump to endif for previous BASIC line
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1});
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's or elseif's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;
        }

        Compiler::getElseIfDataStack().push({jmpIndex, nextInternalLabel, codeIndex, Compiler::ElseBlock, ccType});

        return true;
    }

    bool keywordENDIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Compiler::keywordENDIF() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordENDIF() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Compiler::IfElseEndType ifElseEndType = elseIfData._ifElseEndType;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_endif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's/elseif's jump to this new label
        if(ifElseEndType == Compiler::IfBlock  ||  ifElseEndType == Compiler::ElseIfBlock)
        {
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
                case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
                case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;
            }
        }

        // Update elseif's and/or else's jump to endif label
        while(!Compiler::getEndIfDataStack().empty())
        {
            int codeLine = Compiler::getEndIfDataStack().top()._codeLineIndex;
            int jmpIndex = Compiler::getEndIfDataStack().top()._jmpIndex;
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLine]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: 
                {
                    if(Assembler::getUseOpcodeCALLI())
                    {
                        vasm->_code = "CALLI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("CALLI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                    else
                    {
                        vasm->_code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                }
                break;

                case Expression::NormalCC:  addLabelToJump(Compiler::getCodeLines()[codeLine]._vasm, Compiler::getNextInternalLabel());                         break;
                case Expression::FastCC:    addLabelToJump(Compiler::getCodeLines()[codeLine]._vasm, Compiler::getNextInternalLabel());                         break;
            }

            Compiler::getEndIfDataStack().pop();
        }

        return true;
    }

    bool keywordWHILE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::setNextInternalLabel("_while_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getWhileWendDataStack().push({0, Compiler::getNextInternalLabel(), codeLineIndex, Expression::BooleanCC});

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        Compiler::getWhileWendDataStack().top()._jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;
        Compiler::getWhileWendDataStack().top()._ccType = condition._ccType;

        return true;
    }

    bool keywordWEND(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Pop stack for this WHILE loop
        if(Compiler::getWhileWendDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordWEND() : Syntax error, missing WHILE statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        Compiler::WhileWendData whileWendData = Compiler::getWhileWendDataStack().top();
        Compiler::getWhileWendDataStack().pop();

        // Branch to WHILE and check condition again
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", whileWendData._labelName, false, codeLineIndex);
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(whileWendData._ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", whileWendData._labelName, false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", whileWendData._labelName, false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
            }
        }

        // Branch if condition false to instruction after WEND
        Compiler::setNextInternalLabel("_wend_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm[whileWendData._jmpIndex];
        switch(whileWendData._ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId()); break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId()));             break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel());                                                                      break;
        }

        return true;
    }

    bool keywordREPEAT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::setNextInternalLabel("_repeat_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getRepeatUntilDataStack().push({Compiler::getNextInternalLabel(), codeLineIndex});

        return true;
    }

    bool keywordUNTIL(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Pop stack for this REPEAT loop
        if(Compiler::getRepeatUntilDataStack().empty())
        {
            fprintf(stderr, "Compiler::keywordUNTIL() : Syntax error, missing REPEAT statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
        Compiler::getRepeatUntilDataStack().pop();

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);

        // Branch if condition false to instruction after REPEAT
        switch(condition._ccType)
        {
            case Expression::BooleanCC: Compiler::emitVcpuAsm("%JumpFalse", repeatUntilData._labelName + " " + std::to_string(Compiler::incJumpFalseUniqueId()), false, codeLineIndex);       break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName + " " + std::to_string(Compiler::incJumpFalseUniqueId())); break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName);                                                          break;
        }

        return true;
    }

    bool keywordCONST(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), '=', true);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordCONST() : Syntax error, require a variable and an int or str constant, e.g. CONST a=50 or CONST a$=\"doggy\", in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::stripWhitespace(tokens[0]);
        if(!Expression::isVarNameValid(tokens[0]))
        {
            fprintf(stderr, "Compiler::keywordCONST() : Syntax error, name must contain only alphanumerics and '$', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // String
        if(tokens[0].back() == '$')
        {
            // Strip whitespace
            Expression::stripNonStringWhitespace(tokens[1]);
            if(Expression::isValidString(tokens[1]))
            {
                uint16_t address;
                std::string internalName;

                // Strip quotes
                tokens[1].erase(0, 1);
                tokens[1].erase(tokens[1].size()-1, 1);

                Compiler::getOrCreateString(codeLine, codeLineIndex, tokens[1], internalName, address);
                Compiler::getConstants().push_back({uint8_t(tokens[1].size()), 0, address, tokens[1], tokens[0], internalName, Compiler::ConstStr});
            }
            // String keyword
            else
            {
                size_t lbra, rbra;
                if(!Expression::findMatchingBrackets(tokens[1], 0, lbra, rbra))
                {
                    fprintf(stderr, "Compiler::keywordCONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                std::string keywordToken = tokens[1].substr(0, lbra);
                std::string paramToken = tokens[1].substr(lbra + 1, rbra - (lbra + 1));
                Expression::strToUpper(keywordToken);
                if(_stringKeywords.find(keywordToken) == _stringKeywords.end())
                {
                    fprintf(stderr, "Compiler::keywordCONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                int16_t param;
                if(!Expression::stringToI16(paramToken, param))
                {
                    fprintf(stderr, "Compiler::keywordCONST() : Syntax error, keyword param must be a constant number, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                // Create constant string
                int index;
                uint8_t length = 0;
                uint16_t address = 0x0000;
                if(keywordToken == "CHR$")       {length = 1; address = Compiler::getOrCreateConstString(Compiler::StrChar, param, index);}
                else if(keywordToken == "HEX$")  {length = 2; address = Compiler::getOrCreateConstString(Compiler::StrHex,  param, index);}
                else if(keywordToken == "HEXW$") {length = 4; address = Compiler::getOrCreateConstString(Compiler::StrHexw, param, index);}

                // Create constant
                if(address)
                {
                    std::string internalName = Compiler::getStringVars().back()._name;
                    Compiler::getConstants().push_back({length, 0, address, Compiler::getStringVars().back()._text, tokens[0], internalName, Compiler::ConstStr});
                }
            }
        }
        // Integer
        else
        {
            Expression::stripWhitespace(tokens[1]);

            Expression::Numeric numeric;
            Expression::parse(tokens[1], codeLineIndex, numeric);
            if(tokens[1].size() == 0  ||  !numeric._isValid  ||  numeric._varType == Expression::TmpVar  ||  numeric._varType == Expression::IntVar)
            {
                fprintf(stderr, "Compiler::keywordCONST() : Syntax error, invalid constant expression, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                return false;
            }

            Compiler::getConstants().push_back({2, numeric._value, 0x0000, "", tokens[0], "_" + tokens[0], Compiler::ConstInt16});
        }

        return true;
    }

    bool keywordDIM(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string dimText = codeLine._code.substr(foundPos);

        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            fprintf(stderr, "Compiler::keywordDIM() : Syntax error in DIM statement, must be DIM var(x), in : '%s' : on line %d\n", codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        // Positive constant size
        uint16_t arraySize;
        std::string sizeText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        int varIndex = -1, constIndex = -1, strIndex = -1;
        uint32_t expressionType = Compiler::isExpression(sizeText, varIndex, constIndex, strIndex);
        if(expressionType == Expression::HasIntConsts)
        {
            // Print constant int
            arraySize = Compiler::getConstants()[constIndex]._data;
        }
        else if(!Expression::stringToU16(sizeText, arraySize)  ||  arraySize <= 0)
        {
            fprintf(stderr, "Compiler::keywordDIM() : Array size must be a positive constant, found %s in : '%s' : on line %d\n", sizeText.c_str(), codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        // Most BASIC's declared 0 to n elements, hence size = n + 1
#ifndef ARRAY_INDICES_ONE
        arraySize++;
#endif

        std::string varName = codeLine._code.substr(foundPos, lbra - foundPos);
        Expression::stripWhitespace(varName);

        // Var already exists?
        varIndex = Compiler::findVar(varName);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Compiler::keywordDIM() : Var %s already exists in : '%s' : on line %d\n", varName.c_str(), codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        // Optional array var init values
        uint16_t varInit;
        size_t varPos = codeLine._code.find("=");
        if(varPos != std::string::npos)
        {
            std::string varText = codeLine._code.substr(varPos + 1);
            if(!Expression::stringToU16(varText, varInit))
            {
                fprintf(stderr, "Compiler::keywordDIM() : Initial value must be a constant, found %s in : '%s' : on line %d\n", varText.c_str(), codeLine._code.c_str(), codeLineIndex + 1);
                return false;
            }
        }

        arraySize *= 2;
        uint16_t arrayStart = 0x0000;
        if(!Memory::giveFreeRAM(Memory::FitAscending, arraySize, 0x0200, Compiler::getRuntimeStart(), arrayStart))
        {
            fprintf(stderr, "Compiler::keywordDIM() : Not enough RAM for int array of size %d in : '%s' : on line %d\n", arraySize, codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        createIntVar(varName, 0, varInit, codeLine, codeLineIndex, false, varIndex, Compiler::VarArray, arrayStart, Compiler::Int16, arraySize);

        return true;
    }

    bool keywordDEF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string defText = codeLine._code.substr(foundPos);

        // Equals
        size_t equalsPos = codeLine._code.find("=");
        if(equalsPos == std::string::npos)
        {
            fprintf(stderr, "Compiler::keywordDEF() : Syntax error, missing equals sign, in : '%s' : on line %d\n", codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        // Address field
        size_t typePos, lbra, rbra;
        uint16_t address = 0;
        bool foundAddress = false;
        if(Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            std::string addrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            if(!Expression::stringToU16(addrText, address)  ||  address < DEFAULT_START_ADDRESS)
            {
                fprintf(stderr, "Compiler::keywordDEF() : Address field must be a constant or above %04x, found %s in : '%s' : on line %d\n", DEFAULT_START_ADDRESS, addrText.c_str(), codeLine._code.c_str(), codeLineIndex + 1);
                return false;
            }

            foundAddress = true;
            typePos = lbra;
        }
        else
        {
            typePos = equalsPos;
        }

        // Address
        std::string typeText = codeLine._code.substr(foundPos, typePos - foundPos);
        Expression::stripWhitespace(typeText);
        Expression::strToUpper(typeText);
        if(typeText != "BYTE"  &&  typeText != "WORD")
        {
            fprintf(stderr, "Compiler::keywordDEF() : Type field must be either BYTE or WORD, found %s in : '%s' : on line %d\n", typeText.c_str(), codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        // Data fields
        std::vector<std::string> dataTokens = Expression::tokenise(codeLine._code.substr(equalsPos + 1), ',', true);
        if(dataTokens.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordDEF() : Syntax error, require at least one data field, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // BYTE or WORD
        if(typeText == "BYTE")
        {
            std::vector<uint8_t> dataBytes;
            for(int i=0; i<dataTokens.size(); i++)
            {
                uint8_t data;
                Expression::stripWhitespace(dataTokens[i]);
                if(!Expression::stringToU8(dataTokens[i], data))
                {
                    fprintf(stderr, "Compiler::keywordDEF() : Numeric error '%s', in '%s' on line %d\n", dataTokens[i].c_str(), codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }
                dataBytes.push_back(data);
            }

            // New address entry
            if(foundAddress)
            {
                Compiler::getDefDataBytes().push_back({address, dataBytes});
                if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
            }
            // Update current address data
            else
            {
                // Append data
                address = Compiler::getDefDataBytes().back()._address + uint16_t(Compiler::getDefDataBytes().back()._data.size());
                for(int i=0; i<dataBytes.size(); i++)
                {
                    Compiler::getDefDataBytes().back()._data.push_back(dataBytes[i]);
                }

                // Mark new RAM chunk as used
                if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
            }
        }
        else if(typeText == "WORD")
        {
            std::vector<uint16_t> dataWords;
            for(int i=0; i<dataTokens.size(); i++)
            {
                uint16_t data;
                Expression::stripWhitespace(dataTokens[i]);
                if(!Expression::stringToU16(dataTokens[i], data))
                {
                    fprintf(stderr, "Compiler::keywordDEF() : Numeric error '%s', in '%s' on line %d\n", dataTokens[i].c_str(), codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }
                dataWords.push_back(data);
            }

            // New address entry
            if(foundAddress)
            {
                Compiler::getDefDataWords().push_back({address, dataWords});
                if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
            }
            // Update current address data
            else
            {
                // Append data
                address = Compiler::getDefDataWords().back()._address + uint16_t(Compiler::getDefDataWords().back()._data.size()) * 2;
                for(int i=0; i<dataWords.size(); i++)
                {
                    Compiler::getDefDataWords().back()._data.push_back(dataWords[i]);
                }

                // Mark new RAM chunk as used
                if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
            }
        }

        return true;
    }

    bool keywordAT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  &&  tokens.size() > 2)
        {
            fprintf(stderr, "Compiler::keywordAT() : Syntax error, 'AT X' or 'AT X,Y', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("ST", "cursorXY",     false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("ST", "cursorXY + 1", false, codeLineIndex); break;
            }
        }

        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "atTextCursor", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "atTextCursor", false, codeLineIndex);
            Compiler::emitVcpuAsm("CALL", "giga_vAC",     false, codeLineIndex);
        }

        return true;
    }

    bool keywordPUT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordPUT() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::Numeric param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        Compiler::emitVcpuAsm("%PrintAcChar", "", false, codeLineIndex);

        return true;
    }

    bool keywordMODE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordMODE() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::Numeric param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        Compiler::emitVcpuAsm("STW", "graphicsMode", false, codeLineIndex);
        Compiler::emitVcpuAsm("%ScanlineMode", "",   false, codeLineIndex);

        return true;
    }

    bool keywordWAIT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordWAIT() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::Numeric param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        Compiler::emitVcpuAsm("STW", "waitVBlankNum", false, codeLineIndex);
        Compiler::emitVcpuAsm("%WaitVBlank", "",      false, codeLineIndex);

        return true;
    }

    bool keywordLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() !=2  &&  tokens.size() != 4)
        {
            fprintf(stderr, "Compiler::keywordLINE() : Syntax error, 'LINE X,Y' or 'LINE X1,Y1,X2,Y2', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        if(tokens.size() == 2)
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<tokens.size(); i++)
            {
                uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;
                }
            }

            Compiler::emitVcpuAsm("%AtLineCursor", "", false, codeLineIndex);
            Compiler::emitVcpuAsm("%DrawVTLine", "", false, codeLineIndex);
        }
        else
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<tokens.size(); i++)
            {
                uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x1", false, codeLineIndex); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y1", false, codeLineIndex); break;
                    case 2: Compiler::emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 3: Compiler::emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;
                }
            }

            Compiler::emitVcpuAsm("%DrawLine", "", false, codeLineIndex);
        }

        return true;
    }

    bool keywordHLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() !=3)
        {
            fprintf(stderr, "Compiler::keywordHLINE() : Syntax error, 'HLINE X1,Y,X2', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawHLine_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawHLine_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawHLine_x2", false, codeLineIndex); break;
            }
        }

        Compiler::emitVcpuAsm("%DrawHLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordVLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() !=3)
        {
            fprintf(stderr, "Compiler::keywordVLINE() : Syntax error, 'VLINE X1,Y,X2', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawVLine_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawVLine_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawVLine_y2", false, codeLineIndex); break;
            }
        }

        Compiler::emitVcpuAsm("%DrawVLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordSCROLL(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Compiler::keywordSCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string scrollToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(scrollToken);
        if(scrollToken != "ON"  &&  scrollToken != "OFF")
        {
            fprintf(stderr, "Compiler::keywordSCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        if(scrollToken == "ON")
        {
            Compiler::emitVcpuAsm("LDWI", "0x0001", false, codeLineIndex);
            Compiler::emitVcpuAsm("ORW", "miscFlags", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "0xFFFE", false, codeLineIndex);
            Compiler::emitVcpuAsm("ANDW", "miscFlags", false, codeLineIndex);
        }
        Compiler::emitVcpuAsm("STW", "miscFlags", false, codeLineIndex);

        return true;
    }

    bool keywordPOKE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordPOKE() : Syntax error, 'POKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i], numerics[i]);
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDI", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false, codeLineIndex);
            Compiler::emitVcpuAsm("POKE", "register0", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
            Compiler::emitVcpuAsm("LDI",  operands[1], false, codeLineIndex);
            Compiler::emitVcpuAsm("POKE", "register0", false, codeLineIndex);
        }

        return true;
    }

    bool keywordDOKE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordDOKE() : syntax error, 'DOKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i], numerics[i]);
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
            Compiler::emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
            Compiler::emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
            Compiler::emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
        }

        return true;
    }

    bool keywordPLAY(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), " ,", false);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Compiler::keywordPLAY() : Syntax error, use 'PLAY MIDI <address>, <waveType>', where <address> and <waveType> are optional; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string midiToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(midiToken);
        if(midiToken != "MIDI")
        {
            fprintf(stderr, "Compiler::keywordPLAY() : Syntax error, use 'PLAY MIDI <address>, <waveType>', where <address> and <waveType> are optional; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Tick Midi
        if(tokens.size() == 1)
        {
            Compiler::emitVcpuAsm("%TickMidi", "", false, codeLineIndex);
            return true;
        }

        // Default wave type
        if(tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDI", "2",       false, codeLineIndex);
            Compiler::emitVcpuAsm("ST", "waveType", false, codeLineIndex);
        }
        // Midi wave type, (optional)
        else if(tokens.size() == 3)
        {
            std::string waveTypeToken = tokens[2];
            Expression::stripWhitespace(waveTypeToken);
            Expression::Numeric param;
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, waveTypeToken, param);
            Compiler::emitVcpuAsm("ST", "waveType", false, codeLineIndex);
        }

        // Midi stream address
        std::string addressToken = tokens[1];
        Expression::stripWhitespace(addressToken);
        Expression::Numeric param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, addressToken, param);
        Compiler::emitVcpuAsm("%PlayMidi", "", false, codeLineIndex);

        return true;
    }
}