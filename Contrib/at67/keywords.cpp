#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "keywords.h"


namespace Keywords
{
    std::map<std::string, Keyword> _keywords, _stringKeywords;
    std::vector<std::string> _equalsKeywords;


    std::map<std::string, Keyword>& getKeywords(void) {return _keywords;}
    std::map<std::string, Keyword>& getStringKeywords(void) {return _stringKeywords;}
    std::vector<std::string>& getEqualsKeywords(void) {return _equalsKeywords;}


    bool initialise(void)
    {
        _keywords["END"   ] = {0, "END",    keywordEND   };
        _keywords["ON"    ] = {4, "ON",     keywordON    };
        _keywords["GOTO"  ] = {1, "GOTO",   keywordGOTO  };
        _keywords["GOSUB" ] = {1, "GOSUB",  keywordGOSUB };
        _keywords["RETURN"] = {0, "RETURN", keywordRETURN};
        _keywords["CLS"   ] = {0, "CLS",    keywordCLS   };
        _keywords["INPUT" ] = {0, "INPUT",  nullptr      };
        _keywords["PRINT" ] = {0, "PRINT",  keywordPRINT };
        _keywords["FOR"   ] = {0, "FOR",    keywordFOR   };
        _keywords["NEXT"  ] = {0, "NEXT",   keywordNEXT  };
        _keywords["IF"    ] = {0, "IF",     keywordIF    };
        _keywords["ELSE"  ] = {0, "ELSE",   keywordELSE  };
        _keywords["ELSEIF"] = {0, "ELSEIF", keywordELSEIF};
        _keywords["ENDIF" ] = {0, "ENDIF",  keywordENDIF };
        _keywords["WHILE" ] = {0, "WHILE",  keywordWHILE };
        _keywords["WEND"  ] = {0, "WEND",   keywordWEND  };
        _keywords["REPEAT"] = {0, "REPEAT", keywordREPEAT};
        _keywords["UNTIL" ] = {0, "UNTIL",  keywordUNTIL };
        _keywords["DIM"   ] = {1, "DIM",    keywordDIM   };
        _keywords["DEF"   ] = {0, "DEF",    keywordDEF   };
        _keywords["AT"    ] = {1, "AT",     keywordAT    };
        _keywords["PUT"   ] = {1, "PUT",    keywordPUT   };
        _keywords["MODE"  ] = {1, "MODE",   keywordMODE  };
        _keywords["WAIT"  ] = {1, "WAIT",   keywordWAIT  };
        _keywords["LINE"  ] = {1, "LINE",   keywordLINE  };
        _keywords["HLINE" ] = {1, "HLINE",  keywordHLINE };
        _keywords["VLINE" ] = {1, "VLINE",  keywordVLINE };
        _keywords["SCROLL"] = {1, "SCROLL", keywordSCROLL};
        _keywords["POKE"  ] = {1, "POKE",   keywordPOKE  };
        _keywords["DOKE"  ] = {1, "DOKE",   keywordDOKE  };
        _keywords["PLAY"  ] = {1, "PLAY",   keywordPLAY  };
        _keywords["PEEK"  ] = {1, "PEEK",   nullptr      };
        _keywords["DEEK"  ] = {1, "DEEK",   nullptr      };
        _keywords["USR"   ] = {1, "USR",    nullptr      };
        _keywords["ABS"   ] = {1, "ABS",    nullptr      };
        _keywords["ACS"   ] = {1, "ACS",    nullptr      };
        _keywords["ASC"   ] = {1, "ASC",    nullptr      };
        _keywords["ASN"   ] = {1, "ASN",    nullptr      };
        _keywords["ATN"   ] = {1, "ATN",    nullptr      };
        _keywords["COS"   ] = {1, "COS",    nullptr      };
        _keywords["EXP"   ] = {1, "EXP",    nullptr      };
        _keywords["INT"   ] = {1, "INT",    nullptr      };
        _keywords["LOG"   ] = {1, "LOG",    nullptr      };
        _keywords["RND"   ] = {1, "RND",    nullptr      };
        _keywords["SIN"   ] = {1, "SIN",    nullptr      };
        _keywords["SQR"   ] = {1, "SQR",    nullptr      };
        _keywords["TAN"   ] = {1, "TAN",    nullptr      };
        _keywords["FRE"   ] = {1, "FRE",    nullptr      };
        _keywords["TIME"  ] = {1, "TIME",   nullptr      };

        _stringKeywords["CHR$"  ] = {1, "CHR$",   nullptr};
        _stringKeywords["HEX$"  ] = {1, "HEX$",   nullptr};
        _stringKeywords["HEXW$" ] = {1, "HEXW$",  nullptr};
        _stringKeywords["MID$"  ] = {1, "MID$",   nullptr};
        _stringKeywords["LEFT$" ] = {1, "LEFT$",  nullptr};
        _stringKeywords["RIGHT$"] = {1, "RIGHT$", nullptr};
        _stringKeywords["SPC$"  ] = {1, "SPC$",   nullptr};
        _stringKeywords["STR$"  ] = {1, "STR$",   nullptr};
        _stringKeywords["TIME$" ] = {1, "TIME$",  nullptr};

        _equalsKeywords.push_back("DIM");
        _equalsKeywords.push_back("DEF");
        _equalsKeywords.push_back("FOR");
        _equalsKeywords.push_back("IF");
        _equalsKeywords.push_back("ELSEIF");
        _equalsKeywords.push_back("WHILE");
        _equalsKeywords.push_back("UNTIL");

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

    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        std::string key = keyword;
        Expression::strToUpper(key);
        if(_keywords.find(key) == _keywords.end()) return KeywordNotFound;

        // Handle keyword in code line
        if(findKeyword(key, _keywords[key]._name, foundPos)  &&  _keywords[key]._func)
        {
            bool success = _keywords[key]._func(codeLine, codeLineIndex, foundPos, result);
            return (!success) ? KeywordError : KeywordFound;
        }

        return KeywordFound;
    }


    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool keywordREM(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

    bool keywordLET(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

    bool keywordEND(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::Label label;
        std::string endName = "_end_" + Expression::wordToHexString(Compiler::getVasmPC());
        Compiler::createLabel(Compiler::getVasmPC(), endName, "END\t", codeLineIndex, label, false, false, false, false);
        Compiler::getCodeLines()[codeLineIndex]._labelIndex = Compiler::getCurrentLabelIndex();
        Compiler::emitVcpuAsm("BRA", endName, false, codeLineIndex);

        return true;
    }

    bool keywordON(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        if(!Memory::giveFreeRAM(Memory::FitAscending, size, 0x0200, 0x7FFF, address))
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

    bool keywordGOTO(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

        // Within same page
        // TODO: Optimiser messes this strategy up, FIX IT
        if(0) //HI_MASK(Compiler::getVasmPC()) == HI_MASK(Compiler::getLabels()[labelIndex]._address))
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

    bool keywordGOSUB(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

    bool keywordRETURN(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::emitVcpuAsm("POP", "", false, codeLineIndex);
        Compiler::emitVcpuAsm("RET", "", false, codeLineIndex);

        return true;
    }

    bool keywordCLS(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

        return true;
    }

    bool keywordPOKE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordPOKE() : Syntax error, 'POKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            Expression::Numeric numeric;
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i], numeric);
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("LD", "_" + operands[1], false, codeLineIndex) : Compiler::emitVcpuAsm("LD", "" + operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", "" + operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDI", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", "" + operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("LD", "_" + operands[1], false, codeLineIndex) : Compiler::emitVcpuAsm("LD", "" + operands[1], false, codeLineIndex);
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

    bool keywordDOKE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordDOKE() : syntax error, 'DOKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            Expression::Numeric numeric;
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i], numeric);
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("LDW", "_" + operands[1], false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
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
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("LDW", "_" + operands[1], false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
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

    bool keywordAT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
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

    bool keywordPUT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
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

    bool keywordMODE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
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

    bool keywordWAIT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
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

    bool keywordLINE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
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
        }

        Compiler::emitVcpuAsm("%DrawLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordHLINE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
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

    bool keywordVLINE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
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

    bool keywordPLAY(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), " ,", false);
        if(tokens.size() != 2  &&  tokens.size() != 3)
        {
            fprintf(stderr, "Compiler::keywordPLAY() : Syntax error, use 'PLAY MIDI <address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string midiToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(midiToken);
        if(midiToken != "MIDI")
        {
            fprintf(stderr, "Compiler::keywordPLAY() : Syntax error, use 'PLAY MIDI <address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Midi wave type, (optional)
        if(tokens.size() == 3)
        {
            std::string waveTypeToken = tokens[2];
            Expression::stripWhitespace(waveTypeToken);
            Expression::Numeric param;
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, waveTypeToken, param);
            Compiler::emitVcpuAsm("ST", "waveType", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDI", "2",       false, codeLineIndex);
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

    bool keywordSCROLL(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', offsets, false);
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

    bool keywordINPUT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordPRINT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse print tokens
        std::vector<std::string> tokens = Expression::tokeniseLine(codeLine._code.substr(foundPos), ";");

        Expression::Numeric value;
        int varIndex, params;

        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = Compiler::isExpression(tokens[i], varIndex, params);

            if(expressionType & Expression::HasStringKeywords)
            {
                Expression::setEnablePrint(true);
                Expression::parse(tokens[i], codeLineIndex, value);
                Expression::setEnablePrint(false);
            }
            else if((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
            }
            else if((expressionType & Expression::HasVars)  &&  !(expressionType & Expression::HasStrings))
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                if(varIndex >= 0)
                {
                    if(Compiler::getIntegerVars()[varIndex]._varType == Compiler::VarArray)
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("%PrintVarInt16", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);
                    }
                }
                else
                {
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasKeywords)
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
            }
            else if(expressionType & Expression::HasOperators)
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value._value), false, codeLineIndex);
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
                    if(!Compiler::createString(codeLine, codeLineIndex, str, name, address)) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", name, false, codeLineIndex);
                }
                else
                {
                    if(varIndex >= 0  &&  Compiler::getIntegerVars()[varIndex]._name.find("$") != std::string::npos)
                    {
                        Compiler::emitVcpuAsm("%PrintVarString", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);
                    }
                }
            }
            else if(expressionType == Expression::HasNumbers)
            {
                // If valid expression
                if(Expression::parse(tokens[i], codeLineIndex, value))
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value._value), false, codeLineIndex);
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

    bool keywordFOR(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
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

        // Loop start
        int16_t loopStart = 0;
        std::string startToken = codeLine._code.substr(equals + 1, to - (equals + 1));
        Expression::stripWhitespace(startToken);

        // Var counter, (create or update if being reused)
        std::string var = codeLine._code.substr(foundPos, equals - foundPos);
        Expression::stripWhitespace(var);
        int varIndex = Compiler::findVar(var);
        (varIndex < 0) ? Compiler::createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varIndex) : Compiler::updateVar(loopStart, codeLine, varIndex, false);

        // Loop end
        int16_t loopEnd = 0;
        size_t end = (step == std::string::npos) ? codeLine._code.size() : step;
        std::string endToken = codeLine._code.substr(to + 2, end - (to + 2));
        Expression::stripWhitespace(endToken);

        // Loop step
        int16_t loopStep = 1;
        std::string stepToken;
        if(step != std::string::npos)
        {
            end = codeLine._code.size();
            stepToken = codeLine._code.substr(step + 4, end - (step + 4));
            Expression::stripWhitespace(stepToken);
        }
        if(stepToken.size() != 0  &&  !Expression::stringToI16(stepToken, loopStep))
        {
            fprintf(stderr, "Compiler::keywordFOR() : Syntax error, STEP value must be a constant, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

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

        // Optimised case for 8bit constants
        bool optimise = false;
        if(Expression::stringToI16(startToken, loopStart)  &&  Expression::stringToI16(endToken, loopEnd))
        {
            if(loopStart >=0  &&  loopStart <= 255  &&  loopEnd >=0  &&  loopEnd <= 255  &&  abs(loopStep) == 1)
            {
                optimise = true;
                loopStep = (loopEnd >= loopStart) ? 1 : -1; // auto step based on start and end
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStart), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);
            }
        }

        // General purpose supports 16bit, expressions and variables
        if(!optimise)
        {
            // Parse start field
            Expression::Numeric numeric;
            numeric._value = loopStart;
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, startToken, numeric, 0);
            loopStart = numeric._value;
            Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);

            // Parse end field
            numeric._value = loopEnd;
            expressionType = parseExpression(codeLine, codeLineIndex, endToken, numeric, 0);
            loopEnd = numeric._value;
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);

            // Parse step field
            int16_t replace = 1; // if step is 0, replace it with 1
            numeric._value = loopStep;
            expressionType = parseExpression(codeLine, codeLineIndex, stepToken, numeric, replace);
            loopStep = numeric._value;
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
        }

        Compiler::setNextInternalLabel("_next_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getForNextDataStack().push({varIndex, Compiler::getNextInternalLabel(), loopEnd, loopStep, varEnd, varStep, optimise, codeLineIndex});

        return true;
    }

    bool keywordNEXT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

        if(forNextData._optimise)
        {
            // Positive step
            if(forNextData._loopStep > 0)
            {
                Compiler::emitVcpuAsm("%ForNextLoopUp", "_" + Compiler::getIntegerVars()[varIndex]._name + " " + forNextData._labelName + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex);
            }
            // Negative step
            else
            {
                Compiler::emitVcpuAsm("%ForNextLoopDown", "_" + Compiler::getIntegerVars()[varIndex]._name + " " + forNextData._labelName + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex);
            }
        }
        else
        {
            // Positive step
            if(forNextData._loopStep > 0)
            {
                Compiler::emitVcpuAsm("%ForNextLoopStepUp", "_" + Compiler::getIntegerVars()[varIndex]._name + " " + forNextData._labelName + " " + Expression::byteToHexString(uint8_t(forNextData._varEnd)) + " " + Expression::byteToHexString(uint8_t(forNextData._varStep)), false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("%ForNextLoopStepDown", "_" + Compiler::getIntegerVars()[varIndex]._name + " " + forNextData._labelName + " " + Expression::byteToHexString(uint8_t(forNextData._varEnd)) + " " + Expression::byteToHexString(uint8_t(forNextData._varStep)), false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordIF(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        if(condition._isLogical) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex);
        int jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        // Bail early as we assume this is an IF ELSE ENDIF block
        if(ifElseEndif)
        {
            Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::IfBlock, condition._isLogical});
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
        int varIndex;
        if(Compiler::parseMultiStatements(actionText, codeLine, codeLineIndex, varIndex) == Compiler::StatementError) return false;

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLineIndex]._vasm[jmpIndex];
        if(condition._isLogical)
        {
            vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel;
        }
        else
        {
            addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, nextInternalLabel);
        }

        return true;
    }

    bool keywordELSEIF(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        bool isLogical = elseIfData._isLogical;
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
            Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1});
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_elseif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        if(isLogical)
        {
            vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel;
        }
        else
        {
            addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);
        }

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        if(condition._isLogical) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex);
        jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::ElseIfBlock, condition._isLogical});

        return true;
    }

    bool keywordELSE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        bool isLogical = elseIfData._isLogical;
        Compiler::getElseIfDataStack().pop();

        // Jump to endif for previous BASIC line
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1});
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1});
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update if's or elseif's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        if(isLogical)
        {
            vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel;
        }
        else
        {
            addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);
        }

        Compiler::getElseIfDataStack().push({jmpIndex, nextInternalLabel, codeIndex, Compiler::ElseBlock, isLogical});

        return true;
    }

    bool keywordENDIF(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        bool isLogical = elseIfData._isLogical;
        Compiler::getElseIfDataStack().pop();

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_endif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());

        // Update elseif's jump to this new label
        if(ifElseEndType == Compiler::ElseIfBlock)
        {
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            if(isLogical)
            {
                vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel;
            }
            else
            {
                addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);
            }
        }

        // Update elseif's and/or else's jump to endif label
        while(!Compiler::getEndIfDataStack().empty())
        {
            int codeLine = Compiler::getEndIfDataStack().top()._codeLineIndex;
            int jmpIndex = Compiler::getEndIfDataStack().top()._jmpIndex;
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLine]._vasm[jmpIndex];
            if(isLogical)
            {
                vasm->_code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + Compiler::getNextInternalLabel();
            }
            else
            {
                addLabelToJump(Compiler::getCodeLines()[codeLine]._vasm, Compiler::getNextInternalLabel());
            }

            Compiler::getEndIfDataStack().pop();
        }

        return true;
    }

    bool keywordWHILE(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::setNextInternalLabel("_while_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getWhileWendDataStack().push({0, Compiler::getNextInternalLabel(), codeLineIndex, false});

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        if(condition._isLogical) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex);
        Compiler::getWhileWendDataStack().top()._jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;
        Compiler::getWhileWendDataStack().top()._isLogical = condition._isLogical;

        return true;
    }

    bool keywordWEND(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
            Compiler::emitVcpuAsm("LDWI", whileWendData._labelName, false, codeLineIndex);
            Compiler::emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
        }

        // Branch if condition false to instruction after WEND
        Compiler::setNextInternalLabel("_wend_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm[whileWendData._jmpIndex];
        if(whileWendData._isLogical)
        {
            vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId());
        }
        else
        {
            addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::incJumpFalseUniqueId()));
        }

        return true;
    }

    bool keywordREPEAT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Compiler::setNextInternalLabel("_repeat_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getRepeatUntilDataStack().push({Compiler::getNextInternalLabel(), codeLineIndex});

        return true;
    }

    bool keywordUNTIL(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        if(condition._isLogical)
        {
            Compiler::emitVcpuAsm("%JumpFalse", repeatUntilData._labelName + " " + std::to_string(Compiler::incJumpFalseUniqueId()), false, codeLineIndex);
        }
        else
        {
            addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName + " " + std::to_string(Compiler::incJumpFalseUniqueId()));
        }

        return true;
    }

    bool keywordDIM(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string dimText = codeLine._code.substr(foundPos);

        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            fprintf(stderr, "Compiler::keywordDIM() : Syntax error in DIM statement, must be DIM var(x), in : '%s' : on line %d\n", codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        uint16_t arraySize;
        std::string sizeText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        if(!Expression::stringToU16(sizeText, arraySize)  ||  arraySize <= 0)
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
        int varIndex = Compiler::findVar(varName);
        if(varIndex != -1)
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
        if(!Memory::giveFreeRAM(Memory::FitAscending, arraySize, 0x0200, 0x7FFF, arrayStart))
        {
            fprintf(stderr, "Compiler::keywordDIM() : Not enough RAM for int array of size %d in : '%s' : on line %d\n", arraySize, codeLine._code.c_str(), codeLineIndex + 1);
            return false;
        }

        createIntVar(varName, 0, varInit, codeLine, codeLineIndex, false, varIndex, Compiler::VarArray, arrayStart, Compiler::Int16, arraySize);

        return true;
    }

    bool keywordDEF(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        std::vector<size_t> offsets;
        std::vector<std::string> dataTokens = Expression::tokenise(codeLine._code.substr(equalsPos + 1), ',', offsets, true);
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
}