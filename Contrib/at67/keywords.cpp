#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmath>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <random>

#include "memory.h"
#include "cpu.h"
#include "loader.h"
#include "image.h"
#include "assembler.h"
#include "keywords.h"
#include "functions.h"
#include "operators.h"
#include "linker.h"


#define FONT_WIDTH   6
#define FONT_HEIGHT  8
#define MAPPING_SIZE 96


namespace Keywords
{
    bool _constDimStrArray = false;
    int _numNumericGotosGosubs = 0;

    std::map<std::string, Keyword> _keywords;
    std::map<std::string, std::string> _equalsKeywords;

    struct Gprintf
    {
        int codeLineIndex = 0;
        Assembler::Gprintf _gprintfAsm;
    };
    std::vector<Gprintf> _gprintfs;


    std::map<std::string, Keyword>& getKeywords(void)           {return _keywords;      }
    std::map<std::string, std::string>& getEqualsKeywords(void) {return _equalsKeywords;}


    void reset(void)
    {
        _gprintfs.clear();
    }

    void restart(void)
    {
        _constDimStrArray = false;
        _numNumericGotosGosubs = 0;
    }

    bool initialise(void)
    {
        restart();

        // Equals keywords
        _equalsKeywords["CONST" ] = "CONST";
        _equalsKeywords["DIM"   ] = "DIM";
        _equalsKeywords["DEF"   ] = "DEF";
        _equalsKeywords["FOR"   ] = "FOR";
        _equalsKeywords["IF"    ] = "IF";
        _equalsKeywords["ELSEIF"] = "ELSEIF";
        _equalsKeywords["WHILE" ] = "WHILE";
        _equalsKeywords["UNTIL" ] = "UNTIL";

        // Keywords
        _keywords["END"     ] = {"END",      END,     Compiler::SingleStatementParsed};
        _keywords["INC"     ] = {"INC",      INC,     Compiler::SingleStatementParsed};
        _keywords["DEC"     ] = {"DEC",      DEC,     Compiler::SingleStatementParsed};
        _keywords["ON"      ] = {"ON",       ON,      Compiler::SingleStatementParsed};
        _keywords["GOTO"    ] = {"GOTO",     GOTO,    Compiler::SingleStatementParsed};
        _keywords["GOSUB"   ] = {"GOSUB",    GOSUB,   Compiler::SingleStatementParsed};
        _keywords["RETURN"  ] = {"RETURN",   RETURN,  Compiler::SingleStatementParsed};
        _keywords["RET"     ] = {"RET",      RET,     Compiler::SingleStatementParsed};
        _keywords["CLS"     ] = {"CLS",      CLS,     Compiler::SingleStatementParsed};
        _keywords["?"       ] = {"?",        PRINT,   Compiler::SingleStatementParsed};
        _keywords["PRINT"   ] = {"PRINT",    PRINT,   Compiler::SingleStatementParsed};
        _keywords["INPUT"   ] = {"INPUT",    INPUT,   Compiler::SingleStatementParsed};
        _keywords["FOR"     ] = {"FOR",      FOR,     Compiler::SingleStatementParsed};
        _keywords["NEXT"    ] = {"NEXT",     NEXT,    Compiler::SingleStatementParsed};
        _keywords["IF"      ] = {"IF",       IF,      Compiler::MultiStatementParsed };
        _keywords["ELSEIF"  ] = {"ELSEIF",   ELSEIF,  Compiler::SingleStatementParsed};
        _keywords["ELSE"    ] = {"ELSE",     ELSE,    Compiler::SingleStatementParsed};
        _keywords["ENDIF"   ] = {"ENDIF",    ENDIF,   Compiler::SingleStatementParsed};
        _keywords["WHILE"   ] = {"WHILE",    WHILE,   Compiler::SingleStatementParsed};
        _keywords["WEND"    ] = {"WEND",     WEND,    Compiler::SingleStatementParsed};
        _keywords["REPEAT"  ] = {"REPEAT",   REPEAT,  Compiler::SingleStatementParsed};
        _keywords["UNTIL"   ] = {"UNTIL",    UNTIL,   Compiler::SingleStatementParsed};
        _keywords["FOREVER" ] = {"FOREVER",  FOREVER, Compiler::SingleStatementParsed};
        _keywords["&FOREVER"] = {"&FOREVER", FOREVER, Compiler::SingleStatementParsed};
        _keywords["AS"      ] = {"AS",       AS,      Compiler::SingleStatementParsed};
        _keywords["TYPE"    ] = {"TYPE",     TYPE,    Compiler::SingleStatementParsed};
        _keywords["CALL"    ] = {"CALL",     CALL,    Compiler::SingleStatementParsed};
        _keywords["PROC"    ] = {"PROC",     PROC,    Compiler::SingleStatementParsed};
        _keywords["ENDPROC" ] = {"ENDPROC",  ENDPROC, Compiler::SingleStatementParsed};
        _keywords["LOCAL"   ] = {"LOCAL",    LOCAL,   Compiler::SingleStatementParsed};
        _keywords["CONST"   ] = {"CONST",    CONST,   Compiler::SingleStatementParsed};
        _keywords["DIM"     ] = {"DIM",      DIM,     Compiler::SingleStatementParsed};
        _keywords["DEF"     ] = {"DEF",      DEF,     Compiler::SingleStatementParsed};
        _keywords["DATA"    ] = {"DATA",     DATA,    Compiler::SingleStatementParsed};
        _keywords["READ"    ] = {"READ",     READ,    Compiler::SingleStatementParsed};
        _keywords["RESTORE" ] = {"RESTORE",  RESTORE, Compiler::SingleStatementParsed};
        _keywords["ALLOC"   ] = {"ALLOC",    ALLOC,   Compiler::SingleStatementParsed};
        _keywords["FREE"    ] = {"FREE",     FREE,    Compiler::SingleStatementParsed};
        _keywords["AT"      ] = {"AT",       AT,      Compiler::SingleStatementParsed};
        _keywords["PUT"     ] = {"PUT",      PUT,     Compiler::SingleStatementParsed};
        _keywords["MODE"    ] = {"MODE",     MODE,    Compiler::SingleStatementParsed};
        _keywords["WAIT"    ] = {"WAIT",     WAIT,    Compiler::SingleStatementParsed};
        _keywords["PSET"    ] = {"PSET",     PSET,    Compiler::SingleStatementParsed};
        _keywords["LINE"    ] = {"LINE",     LINE,    Compiler::SingleStatementParsed};
        _keywords["HLINE"   ] = {"HLINE",    HLINE,   Compiler::SingleStatementParsed};
        _keywords["VLINE"   ] = {"VLINE",    VLINE,   Compiler::SingleStatementParsed};
        _keywords["CIRCLE"  ] = {"CIRCLE",   CIRCLE,  Compiler::SingleStatementParsed};
        _keywords["CIRCLEF" ] = {"CIRCLEF",  CIRCLEF, Compiler::SingleStatementParsed};
        _keywords["RECT"    ] = {"RECT",     RECT,    Compiler::SingleStatementParsed};
        _keywords["RECTF"   ] = {"RECTF",    RECTF,   Compiler::SingleStatementParsed};
        _keywords["POLY"    ] = {"POLY",     POLY,    Compiler::SingleStatementParsed};
        _keywords["POLYR"   ] = {"POLYR",    POLYR,   Compiler::SingleStatementParsed};
        _keywords["SCROLL"  ] = {"SCROLL",   SCROLL,  Compiler::SingleStatementParsed};
        _keywords["POKE"    ] = {"POKE",     POKE,    Compiler::SingleStatementParsed};
        _keywords["DOKE"    ] = {"DOKE",     DOKE,    Compiler::SingleStatementParsed};
        _keywords["INIT"    ] = {"INIT",     INIT,    Compiler::SingleStatementParsed};
        _keywords["TICK"    ] = {"TICK",     TICK,    Compiler::SingleStatementParsed};
        _keywords["PLAY"    ] = {"PLAY",     PLAY,    Compiler::SingleStatementParsed};
        _keywords["LOAD"    ] = {"LOAD",     LOAD,    Compiler::SingleStatementParsed};
        _keywords["SPRITE"  ] = {"SPRITE",   SPRITE,  Compiler::SingleStatementParsed};
        _keywords["SOUND"   ] = {"SOUND",    SOUND,   Compiler::SingleStatementParsed};
        _keywords["SET"     ] = {"SET",      SET,     Compiler::SingleStatementParsed};
        _keywords["ASM"     ] = {"ASM",      ASM,     Compiler::SingleStatementParsed};
        _keywords["ENDASM"  ] = {"ENDASM",   ENDASM,  Compiler::SingleStatementParsed};
        _keywords["BCDADD"  ] = {"BCDADD",   BCDADD,  Compiler::SingleStatementParsed};
        _keywords["BCDSUB"  ] = {"BCDSUB",   BCDSUB,  Compiler::SingleStatementParsed};
        _keywords["BCDINT"  ] = {"BCDINT",   BCDINT,  Compiler::SingleStatementParsed};
        _keywords["BCDCPY"  ] = {"BCDCPY",   BCDCPY,  Compiler::SingleStatementParsed};
        _keywords["GPRINTF" ] = {"GPRINTF",  GPRINTF, Compiler::SingleStatementParsed};

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
            // Module line offset
            int codeLineStart = (Compiler::getModuleLines().size()  &&  (codeLineIndex < Compiler::getModuleLines().size())) ? Compiler::getModuleLines()[codeLineIndex]._index : codeLineIndex;

            // Keyword
            bool success = _keywords[key]._func(codeLine, codeLineIndex, codeLineStart, tokenIndex, foundPos, result);
            return (!success) ? KeywordError : KeywordFound;
        }

        return KeywordFound;
    }

#ifndef STAND_ALONE
    bool addGprintf(const std::string& lineToken, const std::string& formatText, const std::vector<std::string>& variables, uint16_t address, int codeLineIndex)
    {
        std::vector<Assembler::Gprintf::Var> vars;
        std::vector<std::string> subs;
        Assembler::parseGprintfFormat(formatText, variables, vars, subs);

        Gprintf gprintf = {codeLineIndex, {Compiler::getVasmPC(), codeLineIndex, lineToken, formatText, vars, subs}};
        for(int i=0; i<int(gprintf._gprintfAsm._vars.size()); i++)
        {
            gprintf._gprintfAsm._vars[i]._indirection = 2;
            gprintf._gprintfAsm._vars[i]._data = uint16_t(address  + i*2);
        }

        _gprintfs.push_back(gprintf);

        return true;
    }

    bool convertGprintGbasToGprintfAsm(void)
    {
        for(int i=0; i<int(_gprintfs.size()); i++)
        {
            int codeLineIndex = _gprintfs[i].codeLineIndex;
            const Compiler::CodeLine& codeLine = Compiler::getCodeLines()[codeLineIndex];
            const Compiler::VasmLine& vasmLine = codeLine._vasm.back(); 
            Assembler::Gprintf& gprintfAsm = _gprintfs[i]._gprintfAsm;
            uint16_t address = uint16_t(vasmLine._address);

            gprintfAsm._address = address;
            if(!Assembler::addGprintf(gprintfAsm, address))
            {
                fprintf(stderr, "Keywords::fixGprintfAddresses() : Assembler::addGprintf() at '0x%04x' already exists, in '%s' on line %d\n", address, codeLine._text.c_str(), codeLineIndex);
                return false;
            }
        }

        return true;
    }
#endif


    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool END(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        //std::string labelName = "_end_" + Expression::wordToHexString(Compiler::getVasmPC());
        //Compiler::emitVcpuAsm("BRA", labelName, false, codeLineIndex, labelName);
        Compiler::emitVcpuAsm("HALT", "", false, codeLineIndex);

        return true;
    }

    bool INC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        // Operand must be an integer var
        std::string varToken = codeLine._code.substr(foundPos);
        Expression::stripWhitespace(varToken);

        // Byte write configuration
        Expression::Int16Byte int16Byte = Expression::Int16Both;
        size_t dot = varToken.find('.');
        if(dot != std::string::npos)
        {
            std::string dotName = varToken.substr(dot);
            varToken = varToken.substr(0, dot);
            Expression::strToUpper(dotName);
            if(dotName == ".LO") int16Byte = Expression::Int16Low;
            if(dotName == ".HI") int16Byte = Expression::Int16High;
        }

        int varIndex = Compiler::findVar(varToken, false);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::INC() : Syntax error, integer variable '%s' not found, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        switch(int16Byte)
        {
            case Expression::Int16Low:  Compiler::emitVcpuAsm("INC", "_" + Compiler::getIntegerVars()[varIndex]._name,          false); break;
            case Expression::Int16High: Compiler::emitVcpuAsm("INC", "_" + Compiler::getIntegerVars()[varIndex]._name + " + 1", false); break;
            case Expression::Int16Both: Compiler::emitVcpuAsm("INC", "_" + Compiler::getIntegerVars()[varIndex]._name,          false); break;

            default: break;
        }

        return true;
    }

    bool DEC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        // Operand must be an integer var
        std::string varToken = codeLine._code.substr(foundPos);
        Expression::stripWhitespace(varToken);
        int varIndex = Compiler::findVar(varToken, false);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::DEC() : Syntax error, integer variable '%s' not found, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::emitVcpuAsm("LDW",  "_" + Compiler::getIntegerVars()[varIndex]._name, false);
        Compiler::emitVcpuAsm("SUBI", "1", false);
        Compiler::emitVcpuAsm("STW",  "_" + Compiler::getIntegerVars()[varIndex]._name, false);

        return true;
    }

    bool ON(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t gotoOffset = code.find("GOTO");
        size_t gosubOffset = code.find("GOSUB");
        if(gotoOffset == std::string::npos  &&  gosubOffset == std::string::npos)
        {
            fprintf(stderr, "Keywords::ON() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        size_t gSize = (gotoOffset != std::string::npos) ? 4 : 5;
        size_t gOffset = (gotoOffset != std::string::npos) ? gotoOffset : gosubOffset;

        // Parse ON field
        Expression::Numeric onValue;
        std::string onToken = codeLine._code.substr(foundPos, gOffset - (foundPos + 1));
        Compiler::parseExpression(codeLineIndex, onToken, onValue);
        Compiler::emitVcpuAsm("STW", "register0", false);

        // Parse labels
        std::vector<size_t> gOffsets;
        std::vector<std::string> gTokens = Expression::tokenise(codeLine._code.substr(gOffset + gSize), ',', gOffsets, false);
        if(gTokens.size() < 1)
        {
            fprintf(stderr, "Keywords::ON() : Syntax error, must have at least one label after GOTO/GOSUB, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Create on goto/gosub label LUT
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
        for(int i=0; i<int(gTokens.size()); i++)
        {
            std::string gLabel = gTokens[i];
            Expression::stripWhitespace(gLabel);

            // Optimised gosub has no PUSH, (i.e. leaf function, VBI handlers, ASM code, etc)
            bool usePush = true;
            if(gLabel[0] == '&')
            {
                usePush = false;
                gLabel.erase(0, 1);
            }

            int labelIndex = Compiler::findLabel(gLabel);
            if(labelIndex == -1)
            {
                fprintf(stderr, "Keywords::ON() : invalid label %s in slot %d in '%s' on line %d\n", gLabel.c_str(), i, codeLine._text.c_str(), codeLineStart);
                Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
                return false;
            }
                
            // Only ON GOSUB needs a PUSH, (emitted in createVasmCode())
            if(gosubOffset != std::string::npos) Compiler::getLabels()[labelIndex]._gosub = usePush;

            // Create lookup table out of label addresses
            Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.push_back(labelIndex);
        }

        // Allocate giga memory for LUT
        int size = int(gTokens.size()) * 2;
        uint16_t address;
        if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, Compiler::getRuntimeStart(), address))
        {
            fprintf(stderr, "Keywords::ON() : Not enough RAM for onGotoGosub LUT of size %d\n", size);
            return false;
        }
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._address = address;
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name = "_lut_onAddrs_" + Expression::wordToHexString(address);

        Compiler::emitVcpuAsm("ADDW", "register0", false);
        Compiler::emitVcpuAsm("STW",  "register0", false);
        Compiler::emitVcpuAsm("LDWI", Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name, false);
        Compiler::emitVcpuAsm("ADDW", "register0", false);
        if(Compiler::getArrayIndiciesOne())
        {
            Compiler::emitVcpuAsm("SUBI", "2", false);  // enable this to start at 1 instead of 0
        }
        Compiler::emitVcpuAsm("DEEK", "", false);
        Compiler::emitVcpuAsm("CALL", "giga_vAC", false);

        return true;
    }

    bool GOTO(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Parse labels
        std::vector<size_t> gotoOffsets;
        std::vector<std::string> gotoTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gotoOffsets, false);
        if(gotoTokens.size() < 1  ||  gotoTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::GOTO() : Syntax error, must have one or two parameters, e.g. 'GOTO 200' or 'GOTO k+1,default' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
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
            if(Expression::isNumber(gotoToken))
            {
                fprintf(stderr, "Keywords::GOTO() : Numeric label '%s' does not exist : in '%s' on line %d\n", gotoToken.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
            if(++_numNumericGotosGosubs > Compiler::getNumNumericLabels())
            {
                fprintf(stderr, "Keywords::GOTO() : There is a mismatch in Numeric labels for one or more GOTO <var> and GOSUB <var>. ");
                fprintf(stderr, "Correct your line numbers and add appropriate trailers, ':' for GOTO <var> and '!' for GOSUB <var>\n");
                return false;
            }

            Compiler::setCreateNumericLabelLut(true);
            Compiler::parseExpression(codeLineIndex, gotoToken, gotoValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false);

            // Default label exists
            if(gotoTokens.size() == 2)
            {
                std::string defaultToken = gotoTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::GOTO() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false);

            // Call gotoNumericLabel
            Compiler::emitVcpuAsm("%GotoNumeric", "", false);

            return true;
        }

        // Within same page, (validation check on same page branch may fail after outputCode(), user will be warned)
        if(useBRA)
        {
            Compiler::emitVcpuAsm("BRA", "_" + gotoToken, false);
        }
        // Long jump
        else
        {
            if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
            {
                Compiler::emitVcpuAsm("CALLI", "_" + gotoToken, false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "_" + gotoToken, false);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false);
            }
        }

        return true;
    }

    bool GOSUB(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Parse labels
        std::vector<size_t> gosubOffsets;
        std::vector<std::string> gosubTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gosubOffsets, false);
        if(gosubTokens.size() < 1  ||  gosubTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::GOSUB() : Syntax error, must have one or two parameters, e.g. 'GOSUB <label>' or 'GOSUB <expression>, <default label>' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        if(gosubTokens[0].size() == 0)
        {
            fprintf(stderr, "Keywords::GOSUB() : Syntax error, invalid label : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Parse GOSUB field
        Expression::Numeric gosubValue;
        std::string gosubToken = gosubTokens[0];
        Expression::stripWhitespace(gosubToken);

        // Optimised gosub has no PUSH, (i.e. leaf function, VBI handlers, ASM code, etc)
        bool usePush = true;
        if(gosubToken[0] == '&')
        {
            usePush = false;
            gosubToken.erase(0, 1);
        }

        int labelIndex = Compiler::findLabel(gosubToken);
        if(labelIndex == -1)
        {
            if(Expression::isNumber(gosubToken))
            {
                fprintf(stderr, "Keywords::GOSUB() : Numeric label '%s' does not exist : in '%s' on line %d\n", gosubToken.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
            if(++_numNumericGotosGosubs > Compiler::getNumNumericLabels())
            {
                fprintf(stderr, "Keywords::GOSUB() : There is a mismatch in Numeric labels for one or more GOTO <var> and GOSUB <var>. ");
                fprintf(stderr, "Correct your line numbers and add appropriate trailers, ':' for GOTO <var> and '!' for GOSUB <var>\n");
                return false;
            }
            if(!usePush)
            {
                fprintf(stderr, "Keywords::GOSUB() : Can't use optimised GOSUB with numeric labels : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }

            Compiler::setCreateNumericLabelLut(true);
            Compiler::parseExpression(codeLineIndex, gosubToken, gosubValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false);

            // Default label exists
            if(gosubTokens.size() == 2)
            {
                std::string defaultToken = gosubTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::GOSUB() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                Compiler::getLabels()[labelIndex]._gosub = true;
                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false);

            // Call gosubNumericLabel
            Compiler::emitVcpuAsm("%GosubNumeric", "", false);

            return true;
        }

        // CALL label
        Compiler::getLabels()[labelIndex]._gosub = usePush;

        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("CALLI", "_" + gosubToken, false);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "_" + gosubToken, false);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false);
        }

        return true;
    }

    bool RETURN(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        // Use a macro instead of separate "POP" and "RET", otherwise page jumps could be inserted in between the "POP" and "RET" causing mayhem and havoc
        Compiler::emitVcpuAsm("%Return", "", false);

        return true;
    }

    bool RET(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        Compiler::emitVcpuAsm("RET", "", false);

        return true;
    }

    bool CLS(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() > 3)
        {
            fprintf(stderr, "Keywords::CLS() : Syntax error, expected 'CLS INIT' or 'CLS <address>, <optional width>, <optional height>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric param;
        if(tokens.size() == 1  &&  tokens[0].size())
        {
            std::string token = tokens[0];
            Expression::strToUpper(token);
            Expression::stripWhitespace(token);
            if(token == "INIT")
            {
                Compiler::emitVcpuAsm("%ResetVideoTable", "", false);
            }
            else
            {
                Compiler::parseExpression(codeLineIndex, tokens[0], param);
                Compiler::emitVcpuAsm("STW", "clsAddress", false);
                Compiler::emitVcpuAsm("%ClearScreen", "",  false);
            }
        }
        else if(tokens.size() > 1)
        {
            Compiler::parseExpression(codeLineIndex, tokens[0], param);
            Compiler::emitVcpuAsm("STW", "clrAddress", false);
            Compiler::parseExpression(codeLineIndex, tokens[1], param);
            Compiler::emitVcpuAsm("STW", "clrWidth", false); // runtime uses clrWidth in arithmetic, so make sure all of it is valid

            if(tokens.size() == 2)
            {
                Compiler::emitVcpuAsm("LDI", "120", false);
            }
            else
            {
                Compiler::parseExpression(codeLineIndex, tokens[2], param);
            }

            Compiler::emitVcpuAsm("STW", "clrLines", false); // runtime uses clrLines in arithmetic, so make sure all of it is valid
            Compiler::emitVcpuAsm("%ClearRect", "",  false);
        }
        else
        {
            Compiler::emitVcpuAsm("%ClearVertBlinds", "", false);
        }

        return true;
    }

    bool PRINT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineStart);

        // Parse print tokens
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', false, false);

RESTART_PRINT:
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(tokens[i].size() == 0  ||  Expression::hasOnlyWhiteSpace(tokens[i]))
            {
                tokens.erase(tokens.begin() + i);
                goto RESTART_PRINT;
            }

            Expression::Numeric numeric;
            int varIndex = -1, constIndex = -1, strIndex = -1;
            uint32_t expressionType = Compiler::isExpression(tokens[i], varIndex, constIndex, strIndex);

            if((expressionType & Expression::HasStringKeywords)  &&  (expressionType & Expression::HasOptimisedPrint))
            {
                // Prints text on the fly without creating strings
                Expression::setEnableOptimisedPrint(true);
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    Expression::setEnableOptimisedPrint(false);
                    return false;
                }
                Expression::setEnableOptimisedPrint(false);
            }
            // Arrays are handled as functions
            else if(expressionType & Expression::HasFunctions)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                }
                else
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                }
            }
            else if((expressionType & Expression::HasStrVars)  &&  (expressionType & Expression::HasOperators))
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                }
                else if(numeric._varType == Expression::Str2Var)
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                    Compiler::emitVcpuAsm("%PrintAcString", "", false);
                }
                else if(numeric._varType == Expression::TmpStrVar)
                {
                    Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()), false);
                    Compiler::emitVcpuAsm("%PrintAcString", "", false);
                }
                else
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                }
            }
            else if((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                }
                else
                {
                    if(numeric._varType != Expression::Str2Var)
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                    }
                    // String array with variable index
                    else
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                        Compiler::emitVcpuAsm("%PrintAcString", "", false);
                    }
                }
            }
            else if(expressionType & Expression::HasIntVars)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                if(varIndex >= 0)
                {
                    if(numeric._varType != Expression::Str2Var)
                    {
                        switch(numeric._int16Byte)
                        {
                            case Expression::Int16Low:  Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name,          false); break;
                            case Expression::Int16High: Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name + " + 1", false); break;
                            case Expression::Int16Both: Compiler::emitVcpuAsm("LDW", "_" + Compiler::getIntegerVars()[varIndex]._name,          false); break;

                            default: break;
                        }

                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                    }
                    // String array with variable index
                    else
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                        Compiler::emitVcpuAsm("%PrintAcString", "", false);
                    }
                }
                else
                {
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
                }
            }
            else if(expressionType & Expression::HasStrVars)
            {
                if(strIndex >= 0)
                {
                    if(Compiler::getStringVars()[strIndex]._varType != Compiler::VarStr2)
                    {
                        std::string strName = Compiler::getStringVars()[strIndex]._name;
                        Compiler::emitVcpuAsm("%PrintString", "_" + strName, false);
                    }
                    // String array with literal index
                    else
                    {
                        if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                        Compiler::emitVcpuAsm("%PrintAcString", "", false);
                    }
                }
            }
            else if(expressionType & Expression::HasKeywords)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
            }
            else if(expressionType & Expression::HasOperators)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
            else if(expressionType & Expression::HasStrings)
            {
                size_t lquote = tokens[i].find_first_of("\"");
                size_t rquote = tokens[i].find_last_of("\"");
                if(lquote != std::string::npos  &&  rquote != std::string::npos)
                {
                    if(rquote == lquote + 1) continue; // skip empty strings
                    std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));

                    // Create string
                    std::string name;
                    uint16_t address;
                    if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", "_" + name, false);
                }
            }
            else if(expressionType == Expression::HasStrConsts  &&  constIndex > -1)
            {
                // Print constant string
                std::string internalName = Compiler::getConstants()[constIndex]._internalName;
                Compiler::emitVcpuAsm("%PrintString", "_" + internalName, false);
            }
            else if(expressionType == Expression::HasIntConsts  &&  constIndex > -1)
            {
                // Print constant int
                int16_t data = Compiler::getConstants()[constIndex]._data;
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(data), false);
            }
            else if(expressionType == Expression::HasNumbers)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric)) return false;
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';'  &&  codeLine._code[codeLine._code.size() - 1] != ',')
        {
            Compiler::emitVcpuAsm("%NewLine", "", false);
        }

        return true;
    }

    bool INPUT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Tokenise string and vars
        std::vector<std::string> strings;
        std::string text = codeLine._code.substr(foundPos);
        Expression::stripNonStringWhitespace(text);
        std::vector<std::string> tokens = Expression::tokenise(text, ',', false, false);
        std::string code = Expression::stripStrings(text, strings, true);
        std::vector<std::string> varTokens = Expression::tokenise(code, ',', false, false);

        if(varTokens.size() < 1  ||  (strings.size() > varTokens.size() + 1))
        {
            fprintf(stderr, "Keywords::INPUT() : Syntax error in INPUT statement, must be 'INPUT <heading>, <int/str var0>, <prompt0>, ... <int/str varN>, <promptN>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Print heading string
        bool foundHeadingString = false;
        if(tokens.size()  &&  Expression::isStringValid(tokens[0]))
        {
            size_t lquote = tokens[0].find_first_of("\"");
            size_t rquote = tokens[0].find_last_of("\"");
            if(lquote != std::string::npos  &&  rquote != std::string::npos)
            {
                // Skip empty strings
                if(rquote > lquote + 1)
                {
                    std::string str = tokens[0].substr(lquote + 1, rquote - (lquote + 1));

                    // Create string
                    std::string name;
                    uint16_t address;
                    if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", "_" + name, false);
                    foundHeadingString = true;
                }
            }
        }

        // INPUT vars/strs/types LUTs, (extra 0x0000 delimiter used by VASM runtime)
        std::vector<uint16_t> varsLut(varTokens.size());
        std::vector<uint16_t> strsLut(varTokens.size());
        std::vector<uint16_t> typesLut(varTokens.size() + 1, 0x0000);

        // Loop through vars
        for(int i=0; i<int(varTokens.size()); i++)
        {
            // Int var exists
            bool isStrVar = false;
            int intVar = Compiler::findVar(varTokens[i]);
            if(intVar >= 0)
            {
                varsLut[i] = Compiler::getIntegerVars()[intVar]._address;
                typesLut[i] = Compiler::VarInt16;
                continue;
            }
            // Str var exists
            else
            {
                if(Expression::isStrNameValid(varTokens[i]))
                {
                    isStrVar = true;
                    int strIndex = Compiler::findStr(varTokens[i]);
                    if(strIndex >= 0)
                    {
                        varsLut[i] = Compiler::getStringVars()[strIndex]._address;
                        typesLut[i] = Compiler::VarStr;
                        continue;
                    }
                }
            }

            // Create int var
            int varIndex = 0;
            if(!isStrVar)
            {
                Compiler::createIntVar(varTokens[i], 0, 0, codeLine, codeLineIndex, false, varIndex);
                if(varIndex == -1)
                {
                    fprintf(stderr, "Keywords::INPUT() : Couldn't create integer var '%s', in '%s' on line %d\n", varTokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
                varsLut[i] = Compiler::getIntegerVars()[varIndex]._address;
                typesLut[i] = Compiler::VarInt16;
            }
            // Create str var
            else
            {
                uint16_t address;
                varIndex = getOrCreateString(codeLine, codeLineIndex, "", varTokens[i], address, USER_STR_SIZE, false);
                if(varIndex == -1)
                {
                    fprintf(stderr, "Keywords::INPUT() : Couldn't create string var '%s', in '%s' on line %d\n", varTokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
                varsLut[i] = Compiler::getStringVars()[varIndex]._address;
                typesLut[i] = Compiler::VarStr;
            }
        }

        // Loop through strs
        for(int i=0; i<int(varTokens.size()); i++)
        {
            // Create string
            std::string name;
            uint16_t address;
            int index = (foundHeadingString) ? i + 1 : i;
            std::string str = (index < int(strings.size())) ? strings[index] : "\"?\";;";
            size_t fquote = str.find_first_of('"');
            size_t lquote = str.find_last_of('"');

            // Semicolons
            if(str.size() > lquote + 1  &&  str[lquote + 1] != ';') typesLut[i] |= 0x40;
            if(str.size() > lquote + 1  &&  str[lquote + 1] == ';') str.erase(lquote + 1, 1);
            if(str.back() != ';') typesLut[i] |= 0x80;
            if(str.back() == ';') str.erase(str.size() - 1, 1);

            // Text length field
            uint8_t length = USER_STR_SIZE;
            if(str.size() > lquote + 1  &&  isdigit((unsigned char)str[lquote + 1]))
            {
                std::string field = str.substr(lquote + 1);
                if(!Expression::stringToU8(field, length))
                {
                    fprintf(stderr, "Keywords::INPUT() : Syntax error in text size field of string '%s' of INPUT statement, in '%s' on line %d\n", str.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
                if(length > USER_STR_SIZE)
                {
                    fprintf(stderr, "Keywords::INPUT() : Text size field > %d of string '%s' of INPUT statement, in '%s' on line %d\n", USER_STR_SIZE, str.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                str.erase(lquote + 1, field.size());
            }
            typesLut[i] |= (length + 1) << 8; // increment length as INPUT VASM code counts cursor
        
            // Remove quotes, (remove last quote first)
            str.erase(lquote, 1);
            str.erase(fquote, 1);

            if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;
            strsLut[i] = address;
        }

        // Allocate memory for register work area if it hasn't been allocated already
        if(Compiler::getRegWorkArea() == 0x0000)
        {
            uint16_t regAddr;
            if(!Memory::getFreeRAM(Memory::FitDescending, REG_WORK_SIZE, USER_CODE_START, Compiler::getRuntimeStart(), regAddr))
            {
                fprintf(stderr, "Keywords::INPUT() : Not enough RAM for register work area of size %d, in '%s' on line %d\n", REG_WORK_SIZE, codeLine._text.c_str(), codeLineStart);
                return false;
            }
            Compiler::setRegWorkArea(regAddr);
        }

        // INPUT LUTs
        const int lutSize = 3;
        uint16_t lutAddr, varsAddr, strsAddr, typesAddr;
        if(!Memory::getFreeRAM(Memory::FitDescending, lutSize*2, USER_CODE_START, Compiler::getRuntimeStart(), lutAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : Not enough RAM for INPUT LUT of size %d, in '%s' on line %d\n", lutSize*2, codeLine._text.c_str(), codeLineStart);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(varsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), varsAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : Not enough RAM for INPUT Vars LUT of size %d, in '%s' on line %d\n", int(varsLut.size()*2), codeLine._text.c_str(), codeLineStart);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(strsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), strsAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : Not enough RAM for INPUT Strings LUT of size %d, in '%s' on line %d\n", int(strsLut.size()*2), codeLine._text.c_str(), codeLineStart);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(typesLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), typesAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : Not enough RAM for INPUT Var Types LUT of size %d, in '%s' on line %d\n", int(typesLut.size()*2), codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::getCodeLines()[codeLineIndex]._inputLut = {lutAddr, varsAddr, strsAddr, typesAddr, varsLut, strsLut, typesLut}; // save LUT in global codeLine not local copy
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(lutAddr), false);
        Compiler::emitVcpuAsm("%Input", "", false);

        return true;
    }

    bool FOR(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        bool optimise = true;
        bool varStart = false;
        int varIndex, constIndex, strIndex;
        uint32_t expressionType;

        // Parse first line of FOR loop
        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t equals, to, step;
        if((equals = code.find("=")) == std::string::npos)
        {
            fprintf(stderr, "Keywords::FOR() : Syntax error, (missing '='), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // TO uses INC/ADD, UPTO uses ADD, DOWNTO uses DEC/SUB; &TO/&UPTO/&DOWNTO are optimised BRA versions
        Compiler::ForNextType type = Compiler::AutoTo;
        type = (code.find("UPTO")   != std::string::npos) ? Compiler::UpTo   : type;
        type = (code.find("DOWNTO") != std::string::npos) ? Compiler::DownTo : type;
        bool farJump = (code.find("&TO") == std::string::npos)  &&  (code.find("&UPTO") == std::string::npos)  &&  (code.find("&DOWNTO") == std::string::npos);
        if((to = code.find("TO")) == std::string::npos)
        {
            fprintf(stderr, "Keywords::FOR() : Syntax error, (missing 'TO' or 'DOWNTO'), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        step = code.find("STEP");

        // Maximum of 4 nested loops
        if(Compiler::getForNextDataStack().size() == MAX_NESTED_LOOPS)
        {
            fprintf(stderr, "Keywords::FOR() : Syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Nested loops temporary variables
        uint16_t offset = uint16_t(Compiler::getForNextDataStack().size()) * LOOP_VARS_SIZE;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + sizeof(uint16_t);

        // Adjust 'to' based on length of TO keyword
        int16_t loopStart = 0;
        int toOffset = (farJump) ? 0 : 0 - sizeof('&');
        toOffset = (type == Compiler::UpTo)   ? toOffset - (sizeof("UP")-1)   : toOffset;
        toOffset = (type == Compiler::DownTo) ? toOffset - (sizeof("DOWN")-1) : toOffset;

        // Loop start
        std::string startToken = codeLine._code.substr(equals + sizeof('='), to - (equals + sizeof('=')) + toOffset);
        expressionType = Compiler::isExpression(startToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) varStart = true;

        // Var counter, (create or update if being reused)
        std::string var = codeLine._code.substr(foundPos, equals - foundPos);
        Expression::stripWhitespace(var);
        int varCounter = Compiler::findVar(var);
        (varCounter < 0) ? Compiler::createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varCounter) : Compiler::updateVar(loopStart, codeLine, varCounter, false);

        // Loop end
        int16_t loopEnd = 0;
        size_t end = (step == std::string::npos) ? codeLine._code.size() : step;
        std::string endToken = codeLine._code.substr(to + sizeof("TO")-1, end - (to + sizeof("TO")-1));
        expressionType = Compiler::isExpression(endToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;

        // Loop step
        int16_t loopStep = 1;
        std::string stepToken;
        if(step != std::string::npos)
        {
            end = codeLine._code.size();
            stepToken = codeLine._code.substr(step + sizeof("STEP")-1, end - (step + sizeof("STEP")-1));
            expressionType = Compiler::isExpression(stepToken, varIndex, constIndex, strIndex);
            if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;
        }

        Expression::Numeric startNumeric, endNumeric, stepNumeric;
        if(optimise)
        {
            // Parse start
            if(!Expression::parse(startToken, codeLineIndex, startNumeric)) return false;
            loopStart = int16_t(std::lround(startNumeric._value));

            // Parse end
            if(!Expression::parse(endToken, codeLineIndex, endNumeric)) return false;
            loopEnd = int16_t(std::lround(endNumeric._value));

            // Parse step
            if(stepToken.size())
            {
                if(!Expression::parse(stepToken, codeLineIndex, stepNumeric)) return false;
                loopStep = int16_t(std::lround(stepNumeric._value));
                if(loopStep < 1  ||  loopStep > 255) optimise = false;
            }

            // Variable start
            if(optimise  &&  varStart  &&  endNumeric._isValid  &&  loopEnd >= 0  &&  loopEnd <= 255)
            {
                Compiler::parseExpression(codeLineIndex, startToken, startNumeric);
                loopStart = int16_t(std::lround(startNumeric._value));
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false);
            }
            // 8bit constants
            else if(optimise  &&  startNumeric._isValid  &&  loopStart >= 0  &&  loopStart <= 255  &&  endNumeric._isValid  &&  loopEnd >= 0  &&  loopEnd <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStart), false);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false);
            }
            // 16bit constants require variables
            else
            {
                optimise = false;

                (loopStart >= 0  &&  loopStart <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(loopStart), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(loopStart), false);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false);
                (loopEnd >= 0  &&  loopEnd <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(loopEnd), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(loopEnd), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false);
                (loopStep >= 0  &&  loopStep <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(loopStep), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(loopStep), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false);
            }
        }
        else
        {
            // Parse start
            Compiler::parseExpression(codeLineIndex, startToken, startNumeric);
            loopStart = int16_t(std::lround(startNumeric._value));
            Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false);

            // Parse end
            Compiler::parseExpression(codeLineIndex, endToken, endNumeric);
            loopEnd = int16_t(std::lround(endNumeric._value));
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false);

            // Parse step
            if(stepToken.size())
            {
                Compiler::parseExpression(codeLineIndex, stepToken, stepNumeric);
                loopStep = int16_t(std::lround(stepNumeric._value));
            }
            else
            {
                loopStep = 1;
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStep), false);
            }
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false);
        }

        // Label and stack
        Compiler::setNextInternalLabel("_next_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getForNextDataStack().push({varCounter, Compiler::getNextInternalLabel(), loopEnd, loopStep, varEnd, varStep, type, farJump, optimise, codeLineIndex});

        return true;
    }

    bool NEXT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::NEXT() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = Compiler::findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::NEXT() : Syntax error, (bad var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Pop stack for this nested loop
        if(Compiler::getForNextDataStack().empty())
        {
            fprintf(stderr, "Keywords::NEXT() : Syntax error, missing FOR statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::ForNextData forNextData = Compiler::getForNextDataStack().top();
        Compiler::getForNextDataStack().pop();

        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Keywords::NEXT() : Syntax error, (wrong var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::string varName = Compiler::getIntegerVars()[varIndex]._name;
        std::string labName = forNextData._labelName;
        int16_t loopEnd = forNextData._loopEnd;
        int16_t loopStep = forNextData._loopStep;
        uint16_t varEnd = forNextData._varEnd;
        uint16_t varStep = forNextData._varStep;
        Compiler::ForNextType type = forNextData._type;
        bool farJump = forNextData._farJump;
        bool optimise = forNextData._optimise;

        std::string forNextCmd;
        if(optimise)
        {
            if(abs(loopStep) == 1)
            {
                switch(type)
                {
                    case Compiler::AutoTo:
                    {
                        // Can't use INC when counting upto 0 or 255
                        if(loopEnd == 0  ||  loopEnd == 255)
                        {
                            forNextCmd = (farJump) ? "%ForNextFarAdd" : "%ForNextAdd";
                            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd) + " " + std::to_string(abs(loopStep)), false);
                        }
                        else
                        {
                            forNextCmd = (farJump) ? "%ForNextFarInc" : "%ForNextInc";
                            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd), false);
                        }
                    }
                    break;

                    case Compiler::UpTo:
                    {
                        forNextCmd = (farJump) ? "%ForNextFarAdd" : "%ForNextAdd";
                        Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd) + " " + std::to_string(abs(loopStep)), false);
                    }
                    break;

                    case Compiler::DownTo:
                    {
                        // Decrement to 0, (if only we had a DJNZ instruction)
                        if(loopEnd == 0)
                        {
                            forNextCmd = (farJump) ? "%ForNextFarDecZero" : "%ForNextDecZero";
                            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName, false);
                        }
                        else
                        {
                            forNextCmd = (farJump) ? "%ForNextFarDec" : "%ForNextDec";
                            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd), false);
                        }
                    }
                    break;

                    default: break;
                }
            }
            // Additive/subtractive step
            else
            {
                switch(type)
                {
                    case Compiler::AutoTo:
                    case Compiler::UpTo:   forNextCmd = (farJump) ? "%ForNextFarAdd" : "%ForNextAdd"; break;
                    case Compiler::DownTo: forNextCmd = (farJump) ? "%ForNextFarSub" : "%ForNextSub"; break;

                    default: break;
                }
                Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd) + " " + std::to_string(abs(loopStep)), false);
            }
        }
        // Positive/negative variable step
        else
        {
            switch(type)
            {
                case Compiler::AutoTo:
                case Compiler::UpTo:   forNextCmd = (farJump) ? "%ForNextFarVarAdd" : "%ForNextVarAdd"; break;
                case Compiler::DownTo: forNextCmd = (farJump) ? "%ForNextFarVarSub" : "%ForNextVarSub"; break;

                default: break;
            }
            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + Expression::byteToHexString(uint8_t(varEnd)) + " " + Expression::byteToHexString(uint8_t(varStep)), false);
        }

        return true;
    }

    bool IF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

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
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false); // Boolean condition requires this extra check
        int jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        // Bail early as we assume this is an IF ELSE ENDIF block
        if(ifElseEndif)
        {
            std::stack<Compiler::EndIfData> endIfData; // stores endif label jumps per if/endif pair
            Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::IfBlock, condition._ccType, endIfData});
            return true;
        }

        // Action
        std::string actionToken = Compiler::getCodeLines()[codeLineIndex]._code.substr(offsetIF + offsetTHEN + 4);
        if(actionToken.size() == 0)
        {
            fprintf(stderr, "Keywords::IF() : Syntax error, IF THEN <action>, (missing action), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Expression::trimWhitespace(actionToken);
        std::string actionText = Expression::collapseWhitespaceNotStrings(actionToken);

        // Short circuit GOTO if action is a literal constant or a label
        uint16_t res = 0;
        if(Expression::stringToU16(actionText, res)  ||  Compiler::findLabel(actionText) != -1)
        {
            actionText = "GOTO " + actionText;
        }

        // Multi-statements
        int varIndex, strIndex;
        if(Compiler::parseMultiStatements(actionText, codeLineIndex, codeLineStart, varIndex, strIndex) == Compiler::StatementError) return false;

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLineIndex]._vasm[jmpIndex];
        switch(condition._ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, nextInternalLabel);                            break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, Compiler::getNextInternalLabel());             break;

            default: break;
        }

        return true;
    }

    bool ELSEIF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::ELSEIF() : Syntax error, missing IF statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        std::stack<Compiler::EndIfData> endIfData = elseIfData._endIfData; // add to the list of endif label jumps per if/endif pair
        Compiler::getElseIfDataStack().pop();

        if(elseIfData._ifElseEndType != Compiler::IfBlock  &&  elseIfData._ifElseEndType != Compiler::ElseIfBlock)
        {
            fprintf(stderr, "Keywords::ELSEIF() : Syntax error, ELSEIF follows IF or ELSEIF, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Jump to endif for previous BASIC line
        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
        }
        else
        {
            // Validator checks for invalid page jumps
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1, ccType});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_elseif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

            default: break;
        }

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false); // Boolean condition requires this extra check
        jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::ElseIfBlock, condition._ccType, endIfData});

        return true;
    }

    bool ELSE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::ELSE() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::ELSE() : Syntax error, missing IF statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        std::stack<Compiler::EndIfData> endIfData = elseIfData._endIfData; // add to the list of endif label jumps per if/endif pair
        Compiler::getElseIfDataStack().pop();

        // Jump to endif for previous BASIC line
        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
        }
        else
        {
            // Validator checks for invalid page jumps
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                endIfData.push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1, ccType});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's or elseif's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

            default: break;
        }

        Compiler::getElseIfDataStack().push({jmpIndex, nextInternalLabel, codeIndex, Compiler::ElseBlock, ccType, endIfData});

        return true;
    }

    bool ENDIF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::ENDIF() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::ENDIF() : Syntax error, missing IF statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Compiler::IfElseEndType ifElseEndType = elseIfData._ifElseEndType;
        Expression::CCType ccType = elseIfData._ccType;
        std::stack<Compiler::EndIfData>& endIfData = elseIfData._endIfData; // get the list of complete endif label jumps per if/endif pair
        Compiler::getElseIfDataStack().pop();

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_endif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's/elseif's jump to this new label
        if(ifElseEndType == Compiler::IfBlock  ||  ifElseEndType == Compiler::ElseIfBlock)
        {
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
                case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
                case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

                default: break;
            }
        }

        // Traverse entire list of endif label jumps per if/endif pair and update elseif's and/or else's jumps to endif label
        while(!endIfData.empty())
        {
            codeIndex = endIfData.top()._codeLineIndex;
            jmpIndex = endIfData.top()._jmpIndex;
            ccType = endIfData.top()._ccType;
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: 
                {
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        vasm->_code = "CALLI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("CALLI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                    else
                    {
                        vasm->_code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                }
                break;

                case Expression::NormalCC:  addLabelToJump(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel()); break;
                case Expression::FastCC:    addLabelToJump(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel()); break;
        
                default: break;
            }

            endIfData.pop();
        }

        return true;
    }

    bool WHILE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineStart);

        Compiler::setNextInternalLabel("_while_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getWhileWendDataStack().push({0, Compiler::getNextInternalLabel(), codeLineIndex, Expression::BooleanCC});

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false); // Boolean condition requires this extra check
        Compiler::getWhileWendDataStack().top()._jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;
        Compiler::getWhileWendDataStack().top()._ccType = condition._ccType;

        return true;
    }

    bool WEND(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        // Pop stack for this WHILE loop
        if(Compiler::getWhileWendDataStack().empty())
        {
            fprintf(stderr, "Keywords::WEND() : Syntax error, missing WHILE statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::WhileWendData whileWendData = Compiler::getWhileWendDataStack().top();
        Compiler::getWhileWendDataStack().pop();

        // Branch to WHILE and check condition again
        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("CALLI", whileWendData._labelName, false);
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(whileWendData._ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", whileWendData._labelName, false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", whileWendData._labelName, false);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false);
            }
        }

        // Branch if condition false to instruction after WEND
        Compiler::setNextInternalLabel("_wend_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm[whileWendData._jmpIndex];
        switch(whileWendData._ccType)
        {
            case Expression::BooleanCC:
            {
                vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + Compiler::getNextInternalLabel() + " " +
                                                                                                            std::to_string(Compiler::getNextJumpFalseUniqueId());
            }
            break;

            case Expression::NormalCC:
            {
                addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId()));
            }
            break;

            case Expression::FastCC:
            {
                addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel());
            }
            break;

            default: break;
        }

        return true;
    }

    bool REPEAT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        Compiler::setNextInternalLabel("_repeat_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getRepeatUntilDataStack().push({Compiler::getNextInternalLabel(), codeLineIndex});

        return true;
    }

    bool UNTIL(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Pop stack for this REPEAT loop
        if(Compiler::getRepeatUntilDataStack().empty())
        {
            fprintf(stderr, "Keywords::UNTIL() : Syntax error, missing REPEAT statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
        Compiler::getRepeatUntilDataStack().pop();

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);

        // Branch if condition false to instruction after REPEAT
        switch(condition._ccType)
        {
            case Expression::BooleanCC: Compiler::emitVcpuAsm("%JumpFalse", repeatUntilData._labelName + " " + std::to_string(Compiler::getNextJumpFalseUniqueId()), false);                      break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName + " " + std::to_string(Compiler::getNextJumpFalseUniqueId())); break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName);                                                              break;

            default: break;
        }

        return true;
    }

    bool FOREVER(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(codeLineIndex);

        // Pop stack for this REPEAT loop
        if(Compiler::getRepeatUntilDataStack().empty())
        {
            fprintf(stderr, "Keywords::FOREVER() : Syntax error, missing REPEAT statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
        Compiler::getRepeatUntilDataStack().pop();
        std::string gotoLabel = repeatUntilData._labelName;

        // Check for optimised branch
        bool useBRA = (codeLine._tokens[tokenIndex][0] == '&') ? true : false;

        // Within same page, (validation check on same page branch may fail after outputCode(), user will be warned)
        if(useBRA)
        {
            Compiler::emitVcpuAsm("BRA", repeatUntilData._labelName, false);
        }
        // Long jump
        else
        {
            if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
            {
                Compiler::emitVcpuAsm("CALLI", repeatUntilData._labelName, false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", repeatUntilData._labelName, false);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false);
            }
        }

        return true;
    }

    bool AS(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLine);

        return true;
    }

    bool typeHelper(const std::string& input, Compiler::CodeLine& codeLine, int codeLineStart, std::string& name, uint16_t& address, Compiler::TypeVarType& varType, int& paramNum)
    {
        name = input;
        address = 0x0000;
        Expression::stripWhitespace(name);

        std::string token = input;
        Expression::strToUpper(token);
        size_t dimPos = token.find("DIM");
        if(dimPos == std::string::npos) return true;

        size_t lbra, rbra;
        token = input.substr(dimPos + 3);
        Expression::stripWhitespace(token);
        if(Expression::findMatchingBrackets(token, 0, lbra, rbra, '(', name, paramNum))
        {
            // Byte
            if(varType == Compiler::Byte  &&  (paramNum >= 1  &&  paramNum <= 3))
            {
                varType = Compiler::TypeVarType(Compiler::ArrayB + paramNum - 1);
            }
            // Word
            if(varType == Compiler::Word  &&  (paramNum >= 1  &&  paramNum <= 3))
            {
                varType = Compiler::TypeVarType(Compiler::ArrayW + paramNum - 1);
            }
            // String
            else if(varType == Compiler::String  &&  paramNum == 1)
            {
                varType = Compiler::ArrayS;
            }
            else
            {
                fprintf(stderr, "Keywords::TYPE() : Syntax error, 'TYPE' var array does not have the correct number of dimensions', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }
        }

        return true;
    }
    bool TYPE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', true);
        if(tokens.size() < 3  ||  tokens[1] != "=")
        {
            fprintf(stderr, "Keywords::TYPE() : Syntax error, 'TYPE' requires the following format 'TYPE <NAME> = <varType1>, ... <varTypeN>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Type name
        std::string typeName = tokens[0];
        Expression::stripWhitespace(typeName);
        std::map<std::string, Compiler::TypeData>& typeDatas = Compiler::getTypeDatas();
        if(typeDatas.find(typeName) != typeDatas.end())
        {
            fprintf(stderr, "Keywords::TYPE() : Type '%s' already exists, in '%s' on line %d\n", typeName.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Variables
        size_t equals = codeLine._code.find_first_of('=');
        std::string vars = codeLine._code.substr(equals + 1);
        tokens = Expression::tokenise(vars, ',', true);
        if(tokens.size() == 0)
        {
            fprintf(stderr, "Keywords::TYPE() : Missing variable types, 'TYPE' requires the following format 'TYPE <NAME> = <varType1>, ... <varTypeN>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Type data
        Compiler::TypeData typeData;
        for(int i=0; i<int(tokens.size()); i++)
        {
            int paramNum = 0;
            std::string varName;
            uint16_t varAddr = 0x0000;
            Compiler::TypeVarType varType;

            // Byte
            if(tokens[i].find('%') != std::string::npos)
            {
                varType = Compiler::Byte;
                if(!typeHelper(tokens[i], codeLine, codeLineStart, varName, varAddr, varType, paramNum)) return false;
            }
            // String
            else if(tokens[i].find('$') != std::string::npos)
            {
                varType = Compiler::String;
                if(!typeHelper(tokens[i], codeLine, codeLineStart, varName, varAddr, varType, paramNum)) return false;
            }
            // Word
            else
            {
                varType = Compiler::Word;
                if(!typeHelper(tokens[i], codeLine, codeLineStart, varName, varAddr, varType, paramNum)) return false;
            }

            if(typeData._vars.find(varName) != typeData._vars.end())
            {
                fprintf(stderr, "Keywords::TYPE() : Var '%s' already exists, in '%s' on line %d\n", tokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            typeData._vars[varName] = {varAddr, varType};
        }

        typeDatas[typeName] = typeData;

        //// Constant string array
        //std::string token = tokens[0];
        //Expression::strToUpper(token);
        //size_t dimPos = token.find("DIM");
        //if(dimPos != std::string::npos)
        //{
        //    size_t strPos = token.find("$", dimPos + 3);
        //    if(strPos != std::string::npos)
        //    {
        //        size_t lbra, rbra;
        //        if(Expression::findMatchingBrackets(token, strPos + 1, lbra, rbra))
        //        {
        //            _constDimStrArray = true;
        //            return DIM(codeLine, codeLineIndex, codeLineStart, tokenIndex, foundPos + dimPos + 3, result);
        //        }
        //    }
        //}

        return true;
    }

    bool callHelper(Compiler::CodeLine& codeLine, int codeLineIndex, std::string& token, uint16_t localVarsAddr)
    {
        UNREFERENCED_PARAM(codeLine);

        // If valid expression
        Expression::Numeric numeric;
        if(!Expression::parse(token, codeLineIndex, numeric)) return false;

        if(numeric._varType == Expression::Number)
        {
            int16_t value = int16_t(std::lround(numeric._value));
            (value >= 0  &&  value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(value)), false) : Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(value), false);
        }
        else if(numeric._varType == Expression::IntVar16)
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[numeric._index]._name,          false); break;
                case Expression::Int16High: Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[numeric._index]._name + " + 1", false); break;
                case Expression::Int16Both: Compiler::emitVcpuAsm("LDW", "_" + Compiler::getIntegerVars()[numeric._index]._name,          false); break;

                default: break;
            }
        }
        else
        {
            Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(localVarsAddr)), false);

        return true;
    }
    bool CALL(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> callTokens = Expression::tokenise(codeLine._code.substr(foundPos + 1), ',', true);
        if(callTokens.size() == 0)
        {
            fprintf(stderr, "Keywords::CALL() : Syntax error, 'CALL' requires a 'NAME' and optional parameters, 'CALL <NAME>, <param0, param1, ... paramN>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Proc name
        Expression::stripWhitespace(callTokens[0]);
        std::string procName = callTokens[0];

        // Params
        int numParams = int(callTokens.size()) - 1;
        uint16_t localVarsAddr = LOCAL_VAR_START;
        if(callTokens.size() > 1)
        {
            for(int i=1; i<int(callTokens.size()); i++)
            {
                if(localVarsAddr >= TEMP_VAR_START)
                {
                    fprintf(stderr, "Keywords::CALL() : Error, maximum number of parameters exceeded, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                if(!callHelper(codeLine, codeLineIndex, callTokens[i], localVarsAddr))
                {
                    return false;
                }

                localVarsAddr += 2;
            }
        }

        // Save for later validation
        Compiler::CallData callData = {numParams, codeLineIndex, procName};
        Compiler::getCallDataMap()[procName] = callData;

        if(Compiler::getCodeRomType() < Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("LDWI", "_" + procName, false);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false);
        }
        else
        {
            Compiler::emitVcpuAsm("CALLI", "_" + procName, false);
        }

        return true;
    }

    bool PROC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> procTokens = Expression::tokenise(codeLine._code.substr(foundPos + 1), ',', true);
        if(procTokens.size() == 0)
        {
            fprintf(stderr, "Keywords::PROC() : Syntax error, 'PROC' requires a 'NAME' and optional parameters, 'PROC <NAME>, <param0, param1, ... paramN>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(!Compiler::getProcDataStack().empty())
        {
            fprintf(stderr, "Keywords::PROC() : Syntax error, 'PROC' can NOT be nested, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::ProcData procData;
        uint16_t localVarsAddr = LOCAL_VAR_START;

        // Proc name
        Expression::stripWhitespace(procTokens[0]);
        procData._name = procTokens[0];
        procData._codeLineIndex = codeLineIndex;
        procData._numParams = int(procTokens.size()) - 1;

        // Params
        if(procTokens.size() > 1)
        {
            for(int i=1; i<int(procTokens.size()); i++)
            {
                if(localVarsAddr >= TEMP_VAR_START)
                {
                    fprintf(stderr, "Keywords::PROC() : Error, maximum number of parameters exceeded, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                Expression::stripWhitespace(procTokens[i]);
                if(Expression::isVarNameValid(procTokens[i]) == Expression::Invalid)
                {
                    fprintf(stderr, "Keywords::PROC() : Syntax error, parameter types can only be integer, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                int localVarIndex = -1;
                std::string localVarName = procData._name + "_" + procTokens[i];
                createIntVar(localVarName, 0, 0, codeLine, codeLineIndex, false, localVarsAddr, localVarIndex);
                if(localVarIndex == -1)
                {
                    fprintf(stderr, "Keywords::PROC() : Couldn't create local integer var '%s', in '%s' on line %d\n", localVarName.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                // Accessing variables within a PROC requires a translation from local var name to source var name
                procData._localVarNameMap[procTokens[i]] = localVarName;

                localVarsAddr += 2;
            }
        }

        Compiler::getProcDataStack().push(procData);
        Compiler::getProcDataMap()[procData._name] = procData;

        Compiler::emitVcpuAsm("PUSH", "", false, codeLineIndex, "_" + procData._name);

        return true;
    }

    bool ENDPROC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        if(Compiler::getProcDataStack().empty())
        {
            fprintf(stderr, "Keywords::ENDPROC() : Syntax error, missing PROC statement, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(Compiler::getProcDataStack().size() != 1)
        {
            fprintf(stderr, "Keywords::ENDPROC() : Syntax error, 'PROC' can NOT be nested, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Pop stack for current PROC
        Compiler::getProcDataStack().pop();

        Compiler::emitVcpuAsm("%Return", "", false);

        return true;
    }

    bool LOCAL(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> localTokens = Expression::tokenise(codeLine._code.substr(foundPos + 1), ',', true);
        if(localTokens.size() < 1)
        {
            fprintf(stderr, "Keywords::LOCAL() : Syntax error, 'LOCAL' requires at least one '<VAR>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(Compiler::getProcDataStack().empty())
        {
            fprintf(stderr, "Keywords::LOCAL() : Syntax error, 'LOCAL' can only be used within a 'PROC/ENDPROC' pair, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(Compiler::getProcDataStack().size() != 1)
        {
            fprintf(stderr, "Keywords::LOCAL() : Syntax error, 'LOCAL' can NOT be used in nested 'PROC's', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::ProcData procData = Compiler::getProcDataStack().top();
        procData._numLocals = int(localTokens.size());
        uint16_t localVarsAddr = LOCAL_VAR_START + uint16_t(procData._numParams)*2;

        // Local vars
        for(int i=0; i<int(localTokens.size()); i++)
        {
            if(localVarsAddr >= TEMP_VAR_START)
            {
                fprintf(stderr, "Keywords::LOCAL() : Error, maximum number of local vars exceeded, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }

            Expression::stripWhitespace(localTokens[i]);
            if(Expression::isVarNameValid(localTokens[i]) == Expression::Invalid)
            {
                fprintf(stderr, "Keywords::LOCAL() : Syntax error, local var types can only be integer, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }

            int localVarIndex = -1;
            std::string localVarName = procData._name + "_" + localTokens[i];
            createIntVar(localVarName, 0, 0, codeLine, codeLineIndex, false, localVarsAddr, localVarIndex);
            if(localVarIndex == -1)
            {
                fprintf(stderr, "Keywords::LOCAL() : Couldn't create local integer var '%s', in '%s' on line %d\n", localVarName.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Accessing variables within a PROC requires a translation from local var name to source var name
            procData._localVarNameMap[localTokens[i]] = localVarName;

            localVarsAddr += 2;
        }

        Compiler::getProcDataStack().top() = procData;
        Compiler::getProcDataMap()[procData._name] = procData;

        return true;
    }

    bool CONST(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), '=', true);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::CONST() : Syntax error, use CONST a=50 or CONST a$=\"doggy\" or const dim arr$(2) = \"One\", \"Two\", \"Three\", in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Constant string array
        std::string token = tokens[0];
        Expression::strToUpper(token);
        size_t dimPos = token.find("DIM");
        if(dimPos != std::string::npos)
        {
            size_t strPos = token.find("$", dimPos + 3);
            if(strPos != std::string::npos)
            {
                size_t lbra, rbra;
                if(Expression::findMatchingBrackets(token, strPos + 1, lbra, rbra))
                {
                    _constDimStrArray = true;
                    return DIM(codeLine, codeLineIndex, codeLineStart, tokenIndex, foundPos + dimPos + 3, result);
                }
            }
        }

        // Variable string array
        Expression::stripWhitespace(tokens[0]);
        if(Expression::isVarNameValid(tokens[0]) == Expression::Invalid  &&  Expression::isStrNameValid(tokens[0]) == Expression::Invalid)
        {
            fprintf(stderr, "Keywords::CONST() : Syntax error, name MUST contain only alphanumerics and '$', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // String
        if(Expression::isStrNameValid(tokens[0]) == Expression::Variable)
        {
            // Strip whitespace
            Expression::stripNonStringWhitespace(tokens[1]);
            if(Expression::isStringValid(tokens[1]))
            {
                uint16_t address;
                std::string internalName;

                // Strip quotes
                tokens[1].erase(0, 1);
                tokens[1].erase(tokens[1].size()-1, 1);

                // Don't count escape char '\'
                int escCount = 0;
                int strLength = int(tokens[1].size());
                for(int i=0; i<strLength; i++)
                {
                    if(tokens[1][i] == '\\') escCount++;
                }
                strLength -= escCount;

                Compiler::getOrCreateString(codeLine, codeLineIndex, tokens[1], internalName, address);
                Compiler::getConstants().push_back({uint8_t(strLength), 0, address, tokens[1], tokens[0], internalName, Compiler::ConstStr});
            }
            // String keyword
            else
            {
                size_t lbra, rbra;
                if(!Expression::findMatchingBrackets(tokens[1], 0, lbra, rbra))
                {
                    fprintf(stderr, "Keywords::CONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                std::string funcToken = tokens[1].substr(0, lbra);
                std::string paramToken = tokens[1].substr(lbra + 1, rbra - (lbra + 1));
                Expression::strToUpper(funcToken);
                if(Functions::getStringFunctions().find(funcToken) == Functions::getStringFunctions().end())
                {
                    fprintf(stderr, "Keywords::CONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                int16_t param;
                if(!Expression::stringToI16(paramToken, param))
                {
                    fprintf(stderr, "Keywords::CONST() : Syntax error, keyword param must be a constant number, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                // Create constant string
                int index;
                uint8_t length = 0;
                uint16_t address = 0x0000;
                if(funcToken == "CHR$")      {length = 1; address = Compiler::getOrCreateConstString(Compiler::StrChar, param, index);}
                else if(funcToken == "HEX$") {length = 4; address = Compiler::getOrCreateConstString(Compiler::StrHex,  param, index);}

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
            Expression::Numeric numeric(true); // true = allow static init
            if(!Expression::parse(tokens[1], codeLineIndex, numeric)) return false;
            if(tokens[1].size() == 0  ||  !numeric._isValid  ||  numeric._varType == Expression::TmpVar  ||  numeric._varType == Expression::IntVar16)
            {
                fprintf(stderr, "Keywords::CONST() : Syntax error, invalid constant expression, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }

            Compiler::getConstants().push_back({2, int16_t(std::lround(numeric._value)), 0x0000, "", tokens[0], "_" + tokens[0], Compiler::ConstInt16});
        }

        return true;
    }

    bool initDIM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, std::string& varName, int intIndex, int arrSizeTotal, int16_t& intInit, std::vector<int16_t>& intInits)
    {
        intIndex = Compiler::findVar(varName);
        if(intIndex >= 0)
        {
            fprintf(stderr, "Keywords::initDIM() : Var '%s' already exists in '%s' on line %d\n", varName.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Optional array int init values
        size_t equalsPos = codeLine._code.find("=");
        if(equalsPos != std::string::npos)
        {
            std::string initText = codeLine._code.substr(equalsPos + 1);
            Expression::stripWhitespace(initText);
            std::vector<std::string> initTokens = Expression::tokenise(initText, ',', true);
            if(initTokens.size() == 0)
            {
                fprintf(stderr, "Keywords::initDIM() : Initial value must be a constant, found %s in '%s' on line %d\n", initText.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
            else if(initTokens.size() == 1)
            {
                std::string operand;
                Expression::Numeric numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, initTokens[0], operand, numeric);
                intInit = int16_t(std::lround(numeric._value));
            }
            else if(int(initTokens.size()) > arrSizeTotal)
            {
                fprintf(stderr, "Keywords::initDIM() : Too many initialisation values for size of array, found %d for a size of %d, in '%s' on line %d\n", int(initTokens.size()), arrSizeTotal,  codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Multiple initialisation values, (if there are less init values than array size, then array is padded with last init value)
            std::string operand;
            intInits.resize(initTokens.size());
            std::vector<Expression::Numeric> funcParams(initTokens.size(), Expression::Numeric(true)); // true = allow static init
            for(int i=0; i<int(initTokens.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, initTokens[i], operand, funcParams[i]);
                intInits[i] = int16_t(std::lround(funcParams[i]._value));
            }
            intInit = intInits.back();
        }

        return true;
    }
    bool allocDIM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, uint16_t& address, Compiler::VarType& varType, std::vector<uint16_t>& arrLut, std::vector<uint16_t>& arrSizes, std::vector<std::vector<uint16_t>>& arrAddrs)
    {
        UNREFERENCED_PARAM(codeLineIndex);

        int intSize = 0;
        switch(varType)
        {
            case Compiler::Var1Arr8:  intSize = 1; break;
            case Compiler::Var1Arr16: intSize = 2; break;

            default: fprintf(stderr, "Keywords::allocDIM() : Unknown var type in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                     break;
        }

        // Allocate memory for k * j * i of intSize byte values
        int iSizeBytes = arrSizes[2] * intSize;
        for(int k=0; k<arrSizes[0]; k++)
        {
            for(int j=0; j<arrSizes[1]; j++)
            {
                if(!Memory::getFreeRAM(Memory::FitDescending, iSizeBytes, USER_CODE_START, Compiler::getArraysStart(), arrAddrs[k][j], false)) // arrays do not need to be contained within pages
                {
                    fprintf(stderr, "Keywords::allocDIM() : Not enough RAM for int array of size %d in '%s' on line %d\n", iSizeBytes, codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
        }

        // 1D array
        if(arrSizes[0] == 1  &&  arrSizes[1] == 1)
        {
            address = arrAddrs[0][0];
            varType = (varType == Compiler::Var1Arr8) ? Compiler::Var1Arr8 : Compiler::Var1Arr16;
        }
        // 2D array
        else if(arrSizes[0] == 1)
        {
            int jSizeBytes = arrSizes[1] * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, jSizeBytes, USER_CODE_START, Compiler::getArraysStart(), address, false)) // arrays do not need to be contained within pages
            {
                fprintf(stderr, "Keywords::allocDIM() : Not enough RAM for int array of size %d in '%s' on line %d\n", jSizeBytes, codeLine._text.c_str(), codeLineStart);
                return false;
            }

            address = address;
            varType = (varType == Compiler::Var1Arr8) ? Compiler::Var2Arr8 : Compiler::Var2Arr16;

            // Enable system internal sub intialiser and mark system internal sub to be loaded, (init functions are not needed for ROMv5a and higher as CALLI is able to directly CALL them)
            if(intSize == 1)
            {
                if(Compiler::getCodeRomType() < Cpu::ROMv5a) Compiler::enableSysInitFunc("Init8Array2d");
                Linker::setInternalSubToLoad("convert8Arr2d");
            }
            else if(intSize == 2)
            {
                if(Compiler::getCodeRomType() < Cpu::ROMv5a) Compiler::enableSysInitFunc("Init16Array2d");
                Linker::setInternalSubToLoad("convert16Arr2d");
            }
        }
        // 3D array
        else
        {
            int jSizeBytes = arrSizes[1] * 2;
            for(int k=0; k<arrSizes[0]; k++)
            {
                if(!Memory::getFreeRAM(Memory::FitDescending, jSizeBytes, USER_CODE_START, Compiler::getArraysStart(), arrLut[k], false)) // arrays do not need to be contained within pages
                {
                    fprintf(stderr, "Keywords::allocDIM() : Not enough RAM for int array of size %d in '%s' on line %d\n", jSizeBytes, codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }

            int kSizeBytes = arrSizes[2] * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, kSizeBytes, USER_CODE_START, Compiler::getArraysStart(), address, false)) // arrays do not need to be contained within pages
            {
                fprintf(stderr, "Keywords::allocDIM() : Not enough RAM for int array of size %d in '%s' on line %d\n", kSizeBytes, codeLine._text.c_str(), codeLineStart);
                return false;
            }

            address = address;
            varType = (varType == Compiler::Var1Arr8) ? Compiler::Var3Arr8 : Compiler::Var3Arr16;

            // Enable system internal sub intialiser and mark system internal sub to be loaded, (init functions are not needed for ROMv5a and higher as CALLI is able to directly CALL them)
            if(intSize == 1)
            {
                if(Compiler::getCodeRomType() < Cpu::ROMv5a) Compiler::enableSysInitFunc("Init8Array3d");
                Linker::setInternalSubToLoad("convert8Arr3d");
            }
            else if(intSize == 2)
            {
                if(Compiler::getCodeRomType() < Cpu::ROMv5a) Compiler::enableSysInitFunc("Init16Array3d");
                Linker::setInternalSubToLoad("convert16Arr3d");
            }
        }

        return true;
    }
    bool DIM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            fprintf(stderr, "Keywords::DIM() : Syntax error in DIM statement, must be DIM <var>(<n1>, <optional n2>), in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Dimensions
        std::vector<uint16_t> arrSizes;
        std::vector<std::string> sizeTokens = Expression::tokenise(codeLine._code.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(sizeTokens.size() > MAX_ARRAY_DIMS)
        {
            fprintf(stderr, "Keywords::DIM() : Maximum of %d dimensions, found %d in '%s' on line %d\n", MAX_ARRAY_DIMS, int(sizeTokens.size()), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        int arrSizeTotal = 1;
        for(int i=0; i<int(sizeTokens.size()); i++)
        {
            int varIndex = -1, constIndex = -1, strIndex = -1;
            std::string sizeToken = sizeTokens[i];
            Expression::stripWhitespace(sizeToken);
            uint32_t expressionType = Compiler::isExpression(sizeToken, varIndex, constIndex, strIndex);

            // Constant dimension
            if((expressionType & Expression::HasIntConsts)  &&  constIndex > -1)
            {
                std::string operand;
                Expression::Numeric numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, sizeToken, operand, numeric);
                int16_t data = int16_t(std::lround(numeric._value));
                arrSizes.push_back(data);
            }
            // Literal dimension
            else
            {
                uint16_t arrSize = 0;
                if(!Expression::stringToU16(sizeToken, arrSize)  ||  arrSize <= 0)
                {
                    fprintf(stderr, "Keywords::DIM() : Array dimensions must be a positive constant, found %s in '%s' on line %d\n", sizeToken.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
                arrSizes.push_back(arrSize);
            }

            // Most BASIC's declared 0 to n elements, hence size = n + 1
            if(!Compiler::getArrayIndiciesOne())
            {
                arrSizes.back()++;
            }

            arrSizeTotal *= arrSizes.back();
        }

        std::string varName = codeLine._code.substr(foundPos, lbra - foundPos);
        Expression::stripWhitespace(varName);

        int intIndex = -1;
        int strIndex = -1;
        int16_t intInit = 0;
        std::vector<int16_t> intInits;
        std::vector<std::string> strInits;
        Compiler::VarType varType = Compiler::Var1Arr16;

        // Str array
        if(Expression::isStrNameValid(varName))
        {
            strIndex = Compiler::findStr(varName);
            if(strIndex >= 0)
            {
                fprintf(stderr, "Keywords::DIM() : Str %s already exists in '%s' on line %d\n", varName.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
            if(arrSizes.size() != 1)
            {
                fprintf(stderr, "Keywords::DIM() : Require 1 string dimension, found %d in '%s' on line %d\n", int(arrSizes.size()), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // String array 2nd dimension is always USER_STR_SIZE, (actual arrSizes data structure looks like this, [n][USER_STR_SIZE])
            arrSizes.push_back(USER_STR_SIZE);

            // Optional array str init values
            size_t equalsPos = codeLine._code.find("=");
            if(equalsPos != std::string::npos)
            {
                std::string initText = codeLine._code.substr(equalsPos + 1);
                Expression::stripNonStringWhitespace(initText);
                std::vector<std::string> initTokens = Expression::tokenise(initText, ',', true);
                if(initTokens.size() == 0)
                {
                    fprintf(stderr, "Keywords::DIM() : Initial value must be a string, found %s in '%s' on line %d\n", initText.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
                else if(int(initTokens.size()) > arrSizes[0])
                {
                    fprintf(stderr, "Keywords::DIM() : Too many initialisation strings for size of array, found %d for a size of %d, in '%s' on line %d\n", int(initTokens.size()), arrSizes[0], codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                // Multiple initialisation values, (if there are less strings than array size, then array is padded with last string)
                for(int i=0; i<int(initTokens.size()); i++)
                {
                    if(initTokens[i].size() - 2 > USER_STR_SIZE)
                    {
                        fprintf(stderr, "Keywords::DIM() : Initialisation string '%s' is larger than %d chars, in '%s' on line %d\n", initTokens[i].c_str(), USER_STR_SIZE, codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    if(!Expression::isStringValid(initTokens[i]))
                    {
                        fprintf(stderr, "Keywords::DIM() : Invalid string initialiser, found %s in '%s' on line %d\n", initTokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Strip quotes
                    initTokens[i].erase(0, 1);
                    initTokens[i].erase(initTokens[i].size() - 1, 1);
                    strInits.push_back(initTokens[i]);
                }
            }

            varType = Compiler::VarStr2;
        }
        // Int8 array
        else if(varName.back() == '%')
        {
            // % is only used within DIM keyword
            varName.erase(varName.size()-1, 1);
            if(!initDIM(codeLine, codeLineIndex, codeLineStart, varName, intIndex, arrSizeTotal, intInit, intInits)) return false;
            varType = Compiler::Var1Arr8;
        }
        // Int16 array
        else
        {
            if(!initDIM(codeLine, codeLineIndex, codeLineStart, varName, intIndex, arrSizeTotal, intInit, intInits)) return false;
            varType = Compiler::Var1Arr16;
        }

        // Represent 1 or 2 dimensional arrays as a 3 dimensional array with dimensions of 1 for the missing dimensions
        while(arrSizes.size() != 3)
        {
            arrSizes.insert(arrSizes.begin(), 1);
        }

        // Array lut
        std::vector<uint16_t> arrLut;
        arrLut.resize(arrSizes[0]);

        // Array addresses
        std::vector<std::vector<uint16_t>> arrAddrs;
        arrAddrs.resize(arrSizes[0]);
        for(int i=0; i<arrSizes[0]; i++)
        {
            arrAddrs[i].resize(arrSizes[1]);
        }

        uint16_t address = 0x0000;

        switch(varType)
        {
            // Str arrays
            case Compiler::VarStr2:
            {
                std::vector<uint16_t> strAddrs;
                strAddrs.resize(arrSizes[1]);
                if(_constDimStrArray)
                {
                    // Constant array of strings
                    _constDimStrArray = false;
                    if(Compiler::createStringArray(codeLine, codeLineIndex, varName, 0, strInits, strAddrs) == -1) return false;
                }
                else
                {
                    // Variable array of strings
                    if(Compiler::createStringArray(codeLine, codeLineIndex, varName, USER_STR_SIZE, strInits, strAddrs) == -1) return false;
                }
            }
            break;

            // Int8 arrays, (allocDIM returns var type based on arrSizes)
            case Compiler::Var1Arr8:
            {
                if(!allocDIM(codeLine, codeLineIndex, codeLineStart, address, varType, arrLut, arrSizes, arrAddrs)) return false;
                Compiler::createIntVar(varName, 0, intInit, codeLine, codeLineIndex, false, intIndex, varType, Compiler::Int8, address, arrSizes, intInits, arrAddrs, arrLut);
            }
            break;

            // Int16 arrays, (allocDIM returns var type based on arrSizes)
            case Compiler::Var1Arr16:
            {
                if(!allocDIM(codeLine, codeLineIndex, codeLineStart, address, varType, arrLut, arrSizes, arrAddrs)) return false;
                Compiler::createIntVar(varName, 0, intInit, codeLine, codeLineIndex, false, intIndex, varType, Compiler::Int16, address, arrSizes, intInits, arrAddrs, arrLut);
            }
            break;

            default: fprintf(stderr, "Keywords::DIM() : Unknown array type %d in '%s' on line %d\n", varType, codeLine._text.c_str(), codeLineStart);
                     return false;
        }

        return true;
    }

    // Not used, implemented as a function, (Compiler::userFunc())
    bool FUNC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        size_t lbra, rbra;
        std::string fnText = codeLine._expression;
        Expression::strToUpper(fnText);
        size_t fnPos = fnText.find("FUNC");
        std::string funcText = codeLine._expression.substr(fnPos);
        if(!Expression::findMatchingBrackets(funcText, 0, lbra, rbra))
        {
            fprintf(stderr, "Keywords::FUNC() : Syntax error, invalid parenthesis in FN, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        funcText = funcText.substr(0, rbra + 1);

        // Name
        std::string name = funcText.substr(sizeof("FUNC")-1, lbra - (sizeof("FUNC")-1));
        Expression::stripWhitespace(name);
        if(Compiler::getDefFunctions().find(name) == Compiler::getDefFunctions().end())
        {
            fprintf(stderr, "Keywords::FUNC() : Syntax error, FN %s can't be found, in '%s' on line %d\n", name.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }
        int varIndex = Compiler::findVar(name);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Keywords::FUNC() : Syntax error, name collision with var %s, in '%s' on line %d\n", name.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Params
        std::vector<std::string> params = Expression::tokenise(funcText.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(params.size() == 0)
        {
            fprintf(stderr, "Keywords::FUNC() : Syntax error, need at least one parameter, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        int paramsSize = int(Compiler::getDefFunctions()[name]._params.size());
        if(paramsSize != int(params.size()))
        {
            fprintf(stderr, "Keywords::FUNC() : Syntax error, wrong number of parameters, expecting %d, in '%s' on line %d\n", paramsSize, codeLine._text.c_str(), codeLineStart);
            return false;
        }
        std::string func = Compiler::getDefFunctions()[name]._function;
        for(int i=0; i<int(params.size()); i++)
        {
            Expression::stripWhitespace(params[i]);
            Expression::replaceText(func, Compiler::getDefFunctions()[name]._params[i], params[i]);
        }

        // Replace DEF FN with FUNC
        Expression::replaceText(codeLine._code,       funcText, func);
        Expression::replaceText(codeLine._text,       funcText, func);
        Expression::replaceText(codeLine._expression, funcText, func);
        Expression::replaceText(Compiler::getCodeLines()[codeLineIndex]._code,       funcText, func);
        Expression::replaceText(Compiler::getCodeLines()[codeLineIndex]._text,       funcText, func);
        Expression::replaceText(Compiler::getCodeLines()[codeLineIndex]._expression, funcText, func);
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokeniseLine(Compiler::getCodeLines()[codeLineIndex]._code, " (),=", offsets);
        codeLine._tokens = tokens;
        codeLine._offsets = offsets;
        Compiler::getCodeLines()[codeLineIndex]._tokens = tokens;
        Compiler::getCodeLines()[codeLineIndex]._offsets = offsets;

        return true;
    }

    bool createDEFFN(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, std::string& defFunc)
    {
        UNREFERENCED_PARAM(codeLineIndex);

        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(defFunc, 0, lbra, rbra))
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, invalid parenthesis, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Name
        if(lbra == 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, missing name, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        std::string name = defFunc.substr(0, lbra);
        Expression::stripWhitespace(name);
        Expression::strToUpper(name);
        int varIndex = Compiler::findVar(name);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, name collision with var %s, in '%s' on line %d\n", name.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Function
        size_t equalsPos = defFunc.find("=", rbra + 1);
        if(equalsPos == std::string::npos)
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, missing equals sign, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        std::string function = defFunc.substr(equalsPos + 1);
        Expression::trimWhitespace(function);

        // Params
        std::vector<std::string> params = Expression::tokenise(defFunc.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(params.size() == 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, need at least one parameter, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        for(int i=0; i<int(params.size()); i++)
        {
            Expression::stripWhitespace(params[i]);
            if(function.find(params[i]) == std::string::npos)
            {
                fprintf(stderr, "Keywords::functionDEF() : Syntax error, parameter %s missing from function %s, in '%s' on line %d\n", params[i].c_str(), function.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
        }


        Compiler::DefFunction defFunction = {name, function, params};
        if(Compiler::getDefFunctions().find(name) != Compiler::getDefFunctions().end())
        {
            fprintf(stderr, "Keywords::functionDEF() : Syntax error, DEF FN %s has been defined more than once, in '%s' on line %d\n", name.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }
        Compiler::getDefFunctions()[name] = defFunction;

        return true;
    }
    bool DEF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::string defText = codeLine._code.substr(foundPos);
        std::string fnText = defText;
        Expression::strToUpper(fnText);

        // FUNC
        size_t defPos = std::string::npos;
        if((defPos = fnText.find("FN")) != std::string::npos)
        {
            std::string defFunc = defText.substr(defPos + sizeof("FN") - 1);
            return createDEFFN(codeLine, codeLineIndex, codeLineStart, defFunc);
        }

        // Equals
        size_t equalsPos = codeLine._code.find("=");
        if(equalsPos == std::string::npos)
        {
            // Integer var definition list
            std::vector<std::string> varTokens = Expression::tokenise(defText, ',', true);
            if(varTokens.size())
            {
                std::map<std::string, int> varMap;
                for(int i=0; i<int(varTokens.size()); i++)
                {
                    // Check for valid var name
                    Expression::stripWhitespace(varTokens[i]);
                    if(Expression::isVarNameValid(varTokens[i]) == Expression::Invalid)
                    {
                        fprintf(stderr, "Keywords::DEF() : Syntax error in variable definition, found %s in '%s' on line %d\n", varTokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Check for duplicate vars
                    if(varMap.find(varTokens[i]) != varMap.end())
                    {
                        fprintf(stderr, "Keywords::DEF() : Duplicate variable definition, found multiple '%s' in '%s' on line %d\n", varTokens[i].c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Create var, (no vASM code, i.e. uninitialised)
                    varMap[varTokens[i]] = i;
                    int varIndex = -1;
                    Compiler::createIntVar(varTokens[i], 0, 0, codeLine, codeLineIndex, false, varIndex);
                }

                return true;
            }

            fprintf(stderr, "Keywords::DEF() : Syntax error, missing equals sign, in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Address field
        size_t typePos, lbra, rbra;
        uint16_t address = 0;
        bool foundAddress = false;
        bool foundLutGenerator = false;
        std::string addrText, operand;
        std::vector<std::string> addrTokens;
        Expression::Numeric addrNumeric(true);  // true = allow static init
        if(Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            // Check for LUT generator
            addrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            addrTokens = Expression::tokenise(addrText, ',', true);
            if(addrTokens.size() >= 4)
            {
                foundLutGenerator = true;
            }

            // Parse address field
            if(addrTokens.size() == 0)
            {
                fprintf(stderr, "Keywords::DEF() : Address field does not exist, found %s in '%s' on line %d\n", addrText.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
            Compiler::parseExpression(codeLineIndex, addrTokens[0], operand, addrNumeric);
            address = uint16_t(std::lround(addrNumeric._value));
            typePos = lbra;
            foundAddress = true;
        }
        else
        {
            typePos = equalsPos;
        }

        // Type field
        std::string typeText = codeLine._code.substr(foundPos, typePos - foundPos);
        Expression::stripWhitespace(typeText);
        Expression::strToUpper(typeText);
        if(typeText != "BYTE"  &&  typeText != "WORD")
        {
            fprintf(stderr, "Keywords::DEF() : Type field must be either BYTE or WORD, found %s in '%s' on line %d\n", typeText.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Address offset field
        uint16_t addrOffset = 0;
        if(addrTokens.size() == 2  &&  !foundLutGenerator)
        {
            Expression::Numeric offsetNumeric(true);  // true = allow static init
            Compiler::parseExpression(codeLineIndex, addrTokens[1], operand, offsetNumeric);
            addrOffset = uint16_t(std::lround(offsetNumeric._value));
        }
        
        // ************************************************************************************************************
        // LUT generator
        if(foundLutGenerator)
        {
            if(addrTokens.size() < 4  ||  addrTokens.size() > 6)
            {
                fprintf(stderr, "Keywords::DEF() : LUT generator must have 4 to 6 parameters, '(ADDR, <VAR>, START, STOP, SIZE, <OFFSET>)', (<VAR> and <OFFSET> are optional), found %d in '%s' on line %d\n",
                                                                                                                                                int(addrTokens.size()), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            for(int i=0; i<int(addrTokens.size()); i++) Expression::stripWhitespace(addrTokens[i]);

            std::string lutGenerator = codeLine._code.substr(equalsPos + 1);
            Expression::stripWhitespace(lutGenerator);
            if(lutGenerator.size() == 0)
            {
                fprintf(stderr, "Keywords::DEF() : LUT generator '%s' is invalid in '%s' on line %d\n", lutGenerator.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Parse LUT generator variable
            bool foundVar = false;
            std::string lutGenVar;
            size_t varPos = 0;
            std::vector<size_t> varPositions;
            if(addrTokens.size() >= 5)
            {
                lutGenVar = addrTokens[1];
                if(Expression::isVarNameValid(lutGenVar) == Expression::Variable)
                {
                    foundVar = true;
                    bool foundVarFirstTime = false;

                    for(;;)
                    {
                        varPos = lutGenerator.find(lutGenVar, varPos);
                        if(varPos == std::string::npos)
                        {
                            if(!foundVarFirstTime)
                            {
                                fprintf(stderr, "Keywords::DEF() : LUT generator variable '%s' invalid in '%s' on line %d\n", lutGenVar.c_str(), codeLine._text.c_str(), codeLineStart);
                                return false;
                            }

                            // Found all occurenced of LUT generator variable
                            break;
                        }
                        lutGenerator.erase(varPos, lutGenVar.size());
                        varPositions.push_back(varPos);
                        foundVarFirstTime = true;
                        varPos++;
                    }
                }
            }

            // Parse LUT generator parameters
            int paramsOffset = (foundVar) ? 2 : 1;
            std::vector<Expression::Numeric> lutGenParams = {Expression::Numeric(true), Expression::Numeric(true), Expression::Numeric(true)}; // true = allow static init
            for(int i=0; i<int(lutGenParams.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, addrTokens[i + paramsOffset], operand, lutGenParams[i]);
            }
            if(lutGenParams[2]._value <= 0.0)
            {
                fprintf(stderr, "Keywords::DEF() : LUT size must be greater than zero, found '%d' : '%s' : on line %d\n", int(lutGenParams[2]._value), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Parse LUT generator address offset, if it exists
            if(addrTokens.size() == 6  ||  (addrTokens.size() == 5  &&  !foundVar))
            {
                Expression::Numeric offsetNumeric;
                Compiler::parseExpression(codeLineIndex, addrTokens[3 + paramsOffset], operand, offsetNumeric);
                addrOffset = uint16_t(std::lround(offsetNumeric._value));
            }

            // Evaluate LUT generator
            double start = lutGenParams[0]._value;
            double end = lutGenParams[1]._value;
            double count = fabs(lutGenParams[2]._value);
            double step = (end - start) / count;
            std::vector<int16_t> lutGenData;
            for(double d=start; d<end; d+=step)
            {
                std::string var;
                if(foundVar)
                {
                    // Insert substitute values into var's positions
                    var = std::to_string(d);
                    for(int i=0; i<int(varPositions.size()); i++)
                    {
                        lutGenerator.insert(varPositions[i] + i*var.size(), var);
                    }
                }

                Expression::Numeric lutGenResult = Expression::Numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, lutGenerator, operand, lutGenResult);
                lutGenData.push_back(int16_t(std::lround(lutGenResult._value)));

                if(foundVar)
                {
                    // Erase every occurence of substitute values
                    for(int i=0; i<int(varPositions.size()); i++)
                    {
                        lutGenerator.erase(varPositions[i], var.size());
                    }
                }
            }

            // Allows for byte allocating and setting of interlaced memory, (audio memory is not allocated, but can still be set)
            if(typeText == "BYTE")
            {
                uint16_t addr = address;
                std::vector<uint8_t> dataBytes(int(count), 0);
                for(int i=0; i<int(dataBytes.size()); i++)
                {
                    dataBytes[i] = uint8_t(lutGenData[i]);
                    if(addrOffset != 0)
                    {
                        if(addr < RAM_AUDIO_START  ||  addr > RAM_AUDIO_END)
                        {
                            if(!Memory::takeFreeRAM(addr, 1)) return false;
                            addr += addrOffset;
                        }
                    }
                }
                if(addrOffset == 0)
                {
                    if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
                }
                Compiler::getDefDataBytes().push_back({address, addrOffset, dataBytes});
            }
            // Allows for word allocating and setting of interlaced memory, (audio memory is not allocated, but can still be set)
            else if(typeText == "WORD")
            {
                uint16_t addr = address;
                std::vector<int16_t> dataWords(int(count), 0);
                for(int i=0; i<int(dataWords.size()); i++)
                {
                    dataWords[i] = int16_t(lutGenData[i]);
                    if(addrOffset != 0)
                    {
                        if(addr < RAM_AUDIO_START  ||  addr > RAM_AUDIO_END)
                        {
                            if(!Memory::takeFreeRAM(addr, 2)) return false;
                            addr += addrOffset * 2;
                        }
                    }
                }
                if(addrOffset == 0)
                {
                    if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
                }
                Compiler::getDefDataWords().push_back({address, addrOffset, dataWords});
            }

            return true;
        }

        // ************************************************************************************************************
        // Data fields
        std::vector<std::string> dataTokens = Expression::tokenise(codeLine._code.substr(equalsPos + 1), ',', true);
        if(dataTokens.size() == 0)
        {
            fprintf(stderr, "Keywords::DEF() : Syntax error, require at least one data field in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // BYTE data
        if(typeText == "BYTE")
        {
            // Parse byte fields
            std::vector<uint8_t> dataBytes;
            for(int i=0; i<int(dataTokens.size()); i++)
            {
                Expression::Numeric numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, dataTokens[i], operand, numeric);
                dataBytes.push_back(uint8_t(std::lround(numeric._value)));
            }

            // If no new address is found, then it is recalculated
            if(!foundAddress)
            {
                addrOffset = Compiler::getDefDataBytes().back()._offset;
                uint16_t offset = (addrOffset != 0) ? uint16_t(Compiler::getDefDataBytes().back()._data.size()) * addrOffset : uint16_t(Compiler::getDefDataBytes().back()._data.size());
                address = Compiler::getDefDataBytes().back()._address + offset;
            }

            // No address offset so take memory as one chunk
            if(addrOffset == 0)
            {
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataBytes.size())))
                {
                    fprintf(stderr, "Keywords::DEF() : Memory error, byte chunk allocation at '0x%04x of size '%d' failed, in '%s' on line %d\n", address, int(dataBytes.size()), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            // Valid address offset, so take memory as individual bytes
            else
            {
                uint16_t addr = address;
                for(int i=0; i<int(dataBytes.size()); i++)
                {
                    if(addr < RAM_AUDIO_START  ||  addr > RAM_AUDIO_END)
                    {
                        if(!Memory::takeFreeRAM(addr, 1))
                        {
                            fprintf(stderr, "Keywords::DEF() : Memory error, byte allocation at '0x%04x of size '1' failed, in '%s' on line %d\n", address, codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                        addr += addrOffset;
                    }
                }
            }
            Compiler::getDefDataBytes().push_back({address, addrOffset, dataBytes});
        }
        // WORD data
        else if(typeText == "WORD")
        {
            // Parse word fields
            std::vector<int16_t> dataWords;
            for(int i=0; i<int(dataTokens.size()); i++)
            {
                Expression::Numeric numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, dataTokens[i], operand, numeric);
                dataWords.push_back(int16_t(std::lround(numeric._value)));
            }

            // If no new address is found, then it is recalculated
            if(!foundAddress)
            {
                addrOffset = Compiler::getDefDataWords().back()._offset;
                uint16_t offset = (addrOffset != 0) ? uint16_t(Compiler::getDefDataWords().back()._data.size()) * addrOffset * 2 : uint16_t(Compiler::getDefDataWords().back()._data.size()) * 2;
                address = Compiler::getDefDataWords().back()._address + offset;
            }

            // No address offset so take memory as one chunk
            if(addrOffset == 0)
            {
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataWords.size()) * 2))
                {
                    fprintf(stderr, "Keywords::DEF() : Memory error, word chunk allocation at '0x%04x of size '%d' failed, in '%s' on line %d\n", address, int(dataWords.size()) * 2, codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            // Valid address offset, so take memory as individual words
            else
            {
                uint16_t addr = address;
                for(int i=0; i<int(dataWords.size()); i++)
                {
                    if(addr < RAM_AUDIO_START  ||  addr > RAM_AUDIO_END)
                    {
                        if(!Memory::takeFreeRAM(addr, 2))
                        {
                            fprintf(stderr, "Keywords::DEF() : Memory error, word allocation at '0x%04x of size '2' failed, in '%s' on line %d\n", address, codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                        addr += addrOffset * 2;
                    }
                }
            }
            Compiler::getDefDataWords().push_back({address, addrOffset, dataWords});
        }

        return true;
    }

    bool DATA(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Data fields
        std::vector<std::string> dataTokens = Expression::tokenise(codeLine._code.substr(foundPos + 1), ',', true);
        if(dataTokens.size() == 0)
        {
            fprintf(stderr, "Keywords::DATA() : Syntax error, require at least one data field in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Parse data
        std::string operand;
        for(int i=0; i<int(dataTokens.size()); i++)
        {
            std::string dataToken = dataTokens[i];
            Expression::stripNonStringWhitespace(dataToken);
            if(Expression::isStringValid(dataToken))
            {
                // Strip quotes
                dataToken.erase(0, 1);
                dataToken.erase(dataToken.size()-1, 1);

                // Add str to list
                std::unique_ptr<Compiler::DataObject> pObject = std::make_unique<Compiler::DataStr>(dataToken);
                Compiler::getDataObjects().push_back(std::move(pObject));
            }
            else
            {
                // Parse and add int to list, (ints can be constants, complex expressions or functions that return statics, hence the parsing)
                Expression::Numeric numeric(true); // true = allow static init
                Compiler::parseExpression(codeLineIndex, dataTokens[i], operand, numeric);
                int16_t data = int16_t(std::lround(numeric._value));
                std::unique_ptr<Compiler::DataObject> pObject = std::make_unique<Compiler::DataInt>(data);
                Compiler::getDataObjects().push_back(std::move(pObject));
            }
        }

        return true;
    }

    bool READ(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1)
        {
            fprintf(stderr, "Keywords::READ() : Syntax error, 'READ <var0, var1, var2...varN>', requires at least one variable', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Search for variables
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string varToken = tokens[i];
            Expression::stripWhitespace(varToken);

            int varIndex = Compiler::findVar(varToken);
            int strIndex = Compiler::findStr(varToken);

            size_t lbra, rbra;
            std::string arrText;
            Compiler::VarType varType = Compiler::VarInt16;
            if(varToken.find("$") != std::string::npos) varType = Compiler::VarStr;
            if(Expression::findMatchingBrackets(tokens[i], 0, lbra, rbra))
            {
                arrText = tokens[i].substr(lbra + 1, rbra - (lbra + 1));

                switch(varType)
                {
                    case Compiler::VarInt16: varType = Compiler::Var1Arr16; break;
                    case Compiler::VarStr:   varType = Compiler::VarStr2;   break;

                    default:
                    {
                        fprintf(stderr, "Keywords::READ() : Only single dimensioned arrays are supported : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }
                    break;
                }
            }

            // Int array must have already been dimensioned
            if(varIndex == -1  &&  varType == Compiler::Var1Arr16)
            {
                fprintf(stderr, "Keywords::READ() : Integer array is not dimensioned : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Str array must have already been dimensioned
            if(strIndex == -1  &&  varType == Compiler::VarStr2)
            {
                fprintf(stderr, "Keywords::READ() : String array is not dimensioned : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            // Create int var
            if(varIndex == -1  &&  varType == Compiler::VarInt16)
            {
                Compiler::createIntVar(varToken, 0, 0, codeLine, codeLineIndex, false, varIndex);
                if(varIndex == -1)
                {
                    fprintf(stderr, "Keywords::READ() : Couldn't create integer var '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            // Create str var
            else if(strIndex == -1  &&  varType == Compiler::VarStr)
            {
                uint16_t address;
                strIndex = getOrCreateString(codeLine, codeLineIndex, "", varToken, address, USER_STR_SIZE, false);
                if(strIndex == -1)
                {
                    fprintf(stderr, "Keywords::READ() : Couldn't create string var '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }

            // Copy read into int var
            if(varIndex >= 0)
            {
                Compiler::emitVcpuAsm("%ReadIntVar", "", false);
                Compiler::emitVcpuAsm("DEEK", "", false);

                if(varType == Compiler::VarInt16)
                {
                    Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varIndex]._name, false);
                }
                else if(varType == Compiler::Var1Arr16)
                {
                    Compiler::writeArrayVarNoAssign(codeLine, codeLineIndex, varIndex);
                }
                else
                {
                    fprintf(stderr, "Keywords::READ() : Integer var is invalid : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            // Copy read into str var
            else if(strIndex >= 0)
            {
                if(varType == Compiler::VarStr)
                {
                    Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getStringVars()[strIndex]._name, false);
                    Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                    Compiler::emitVcpuAsm("%ReadStrVar", "", false);
                }
                else if(varType == Compiler::VarStr2)
                {
                    Compiler::writeArrayStrNoAssign(arrText, codeLineIndex, strIndex);
                    Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                    Compiler::emitVcpuAsm("%ReadStrVar", "", false);
                }
                else
                { 
                    fprintf(stderr, "Keywords::READ() : String var is invalid : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            else
            {
                fprintf(stderr, "Keywords::READ() : Variable is invalid : '%s', in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }
        }

        return true;
    }

    bool RESTORE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() > 1)
        {
            fprintf(stderr, "Keywords::RESTORE() : Syntax error, 'RESTORE <optional index>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Compiler::emitVcpuAsm("LDWI", "_dataIndex_", false);
        Compiler::emitVcpuAsm("STW", "memAddr",      false);
        
        if(tokens.size() == 1)
        {
            Expression::Numeric numeric;
            Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        }
        else
        {
            Compiler::emitVcpuAsm("LDI", "0", false);
        }

        Compiler::emitVcpuAsm("DOKE", "memAddr", false);

        return true;
    }

    bool ALLOC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  &&  tokens.size() > 4)
        {
            fprintf(stderr, "Keywords::ALLOC() : Syntax error, 'ALLOC <address>, <optional size>, <optional count>, <optional offset=0x0100>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        int count = 1;
        uint16_t address, end, size = 0x0000, offset = 0x0100;
        std::string addrOperand, sizeOperand, countOperand, offsetOperand;
        Expression::Numeric addrNumeric(true), sizeNumeric(true), countNumeric(true), offsetNumeric(true);
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        if(tokens.size() >= 2)
        {
            Compiler::parseExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric);
            size = uint16_t(std::lround(sizeNumeric._value));
        }
        if(tokens.size() >= 3)
        {
            Compiler::parseExpression(codeLineIndex, tokens[2], countOperand, countNumeric);
            count = std::lround(countNumeric._value);
        }
        if(tokens.size() >= 4)
        {
            Compiler::parseExpression(codeLineIndex, tokens[3], offsetOperand, offsetNumeric);
            offset = uint16_t(std::lround(offsetNumeric._value));
            if(count == 0  ||  offset == 0)
            {
                fprintf(stderr, "Keywords::ALLOC() : Count and offset must both be non zero, found %d and 0x%04x in '%s' on line %d\n", count, offset, codeLine._text.c_str(), codeLineStart);
                return false;
            }
        }

        address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Keywords::ALLOC() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        end = (size == 0) ? 0xFFFF : address + size;
        for(int i=0; i<count; i++)
        {
            //fprintf(stderr, "0x%04x 0x%04x %d\n", address, end, end - address);
            for(uint16_t j=address; j<end; j++)
            {
                if(!Memory::takeFreeRAM(j, 1, false))
                {
                    fprintf(stderr, "Keywords::ALLOC() : Trying to allocate already allocated memory at 0x%04x in '%s' on line %d\n", j, codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }
            address += offset;
            end += offset;
        }
    
        //Memory::printFreeRamList(Memory::NoSort);

        return true;
    }

    bool FREE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::FREE() : Syntax error, 'FREE <address>, <size> or FREE STRINGWORKAREA', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Free string work area
        if(tokens.size() == 1)
        {
            std::string token = Expression::strUpper(tokens[0]);
            Expression::stripWhitespace(token);
            if(token != "STRINGWORKAREA")
            {
                fprintf(stderr, "Keywords::FREE() : Syntax error, expecting 'STRINGWORKAREA', found '%s', in '%s' on line %d\n", token.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            for(int i=0; i<NUM_STR_WORK_AREAS; i++)
            {
                if(!Memory::giveFreeRAM(Compiler::getStrWorkArea(i), USER_STR_SIZE + 2))
                {
                    fprintf(stderr, "Keywords::FREE() : RAM at '0x%04x' is already free, in '%s' on line %d\n", Compiler::getStrWorkArea(i), codeLine._text.c_str(), codeLineStart);
                    return false;
                }
            }

            return true;
        }

        Expression::Numeric addrNumeric(true), sizeNumeric(true);  // true = allow static init
        std::string addrOperand, sizeOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        Compiler::parseExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric);
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        uint16_t size = uint16_t(std::lround(sizeNumeric._value));

        //Memory::printFreeRamList(Memory::NoSort);
        if(!Memory::giveFreeRAM(address, size))
        {
            fprintf(stderr, "Keywords::FREE() : RAM at '0x%04x' is already free, in '%s' on line %d\n", address, codeLine._text.c_str(), codeLineStart);
            return false;
        }
        //Memory::printFreeRamList(Memory::NoSort);

        return true;
    }

    bool AT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::AT() : Syntax error, 'AT <x>' or 'AT <x>, <y>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("ST", "cursorXY", false);

        if(tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, tokens[1], numeric);
            Compiler::emitVcpuAsm("ST", "cursorXY + 1", false);
        }

        Compiler::emitVcpuAsm("%AtTextCursor", "", false);
        return true;
    }

    bool PUT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::PUT() : Syntax error, 'PUT <ascii>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("%PrintAcChar", "", false);

        return true;
    }

    bool MODE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        if(Compiler::getCodeRomType() < Cpu::ROMv2)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            fprintf(stderr, "Keywords::MODE() : Version error, 'MODE' requires ROMv2 or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(),  codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::MODE() : Syntax error, 'MODE <0 - 3>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("STW", "graphicsMode", false);
        Compiler::emitVcpuAsm("%ScanlineMode", "",   false);

        return true;
    }

    bool WAIT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() > 1)
        {
            fprintf(stderr, "Keywords::WAIT() : Syntax error, 'WAIT <optional vblank count>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(tokens.size() == 0)
        {
            Compiler::emitVcpuAsm("%WaitVBlank", "", false);
            return true;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("STW", "waitVBlankNum", false);
        Compiler::emitVcpuAsm("%WaitVBlanks", "",     false);

        return true;
    }

    bool PSET(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Keywords::PSET() : Syntax error, 'PSET <x>, <y>' or 'PSET <x>, <y>, <colour>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("ST", "drawPixel_xy", false);
        Compiler::parseExpression(codeLineIndex, tokens[1], numeric);
        Compiler::emitVcpuAsm("ST", "drawPixel_xy + 1", false);

        if(tokens.size() == 3)
        {
            Compiler::parseExpression(codeLineIndex, tokens[2], numeric);
            Compiler::emitVcpuAsm("ST", "fgbgColour + 1", false);
        }

        Compiler::emitVcpuAsm("%DrawPixel", "", false);
        return true;
    }

    bool LINE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2  &&  tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::LINE() : Syntax error, 'LINE <x>, <y>' or 'LINE <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(tokens.size() == 2)
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<int(tokens.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x2", false); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y2", false); break;

                    default: break;
                }
            }

            Compiler::emitVcpuAsm("%AtLineCursor", "", false);
            Compiler::emitVcpuAsm("%DrawVTLine",   "", false);
        }
        else
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<int(tokens.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x1", false); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y1", false); break;
                    case 2: Compiler::emitVcpuAsm("STW", "drawLine_x2", false); break;
                    case 3: Compiler::emitVcpuAsm("STW", "drawLine_y2", false); break;

                    default: break;
                }
            }

            Compiler::emitVcpuAsm("%DrawLine", "", false);
        }

        return true;
    }

    bool HLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::HLINE() : Syntax error, 'HLINE <x1>, <y>, <x2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawHLine_x1", false); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawHLine_y1", false); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawHLine_x2", false); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawHLine", "", false);

        return true;
    }

    bool VLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::VLINE() : Syntax error, 'VLINE <x>, <y1>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawVLine_x1", false); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawVLine_y1", false); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawVLine_y2", false); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawVLine", "", false);

        return true;
    }

    bool CIRCLE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::CIRCLE() : Syntax error, 'CIRCLE <x>, <y>, <radius>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW",  "drawCircle_cx", false);                                           break;
                case 1: Compiler::emitVcpuAsm("ADDI", "8", false); Compiler::emitVcpuAsm("STW", "drawCircle_cy", false); break;
                case 2: Compiler::emitVcpuAsm("STW",  "drawCircle_r",  false);                                           break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawCircle", "", false);

        return true;
    }

    bool CIRCLEF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::CIRCLEF() : Syntax error, 'CIRCLEF <x>, <y>, <radius>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawCircleF_cx", false); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawCircleF_cy", false); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawCircleF_r",  false); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawCircleF", "", false);

        return true;
    }

    bool RECT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::RECT() : Syntax error, 'RECT <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawRect_x1", false); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawRect_y1", false); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawRect_x2", false); break;
                case 3: Compiler::emitVcpuAsm("STW", "drawRect_y2", false); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawRect", "", false);

        return true;
    }

    bool RECTF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::RECTF() : Syntax error, 'RECTF <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawRectF_x1", false); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawRectF_y1", false); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawRectF_x2", false); break;
                case 3: Compiler::emitVcpuAsm("STW", "drawRectF_y2", false); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawRectF", "", false);

        return true;
    }

    bool POLY(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::POLY() : Syntax error, 'POLY <coords address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        Expression::Numeric param;
        Compiler::parseExpression(codeLineIndex, tokens[0], param);
        Compiler::emitVcpuAsm("STW", "drawPoly_addr", false);
        Compiler::emitVcpuAsm("%DrawPoly", "",        false);

        return true;
    }

    bool POLYR(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::POLYR() : Syntax error, 'POLYR <coords address> <optional FLIPX/FLIPY/FLIPXY>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Flip type
        if(tokens.size() == 2)
        {
            std::string flipToken = tokens[1];
            Expression::stripWhitespace(flipToken);
            Expression::strToUpper(flipToken);
        
            if(flipToken != "FLIPX"  &&  flipToken != "FLIPY"  &&  flipToken != "FLIPXY")
            {
                fprintf(stderr, "Keywords::POLYR() : Syntax error, use one of the correct flip types, 'POLY <coords address> <optional FLIPX/FLIPY/FLIPXY>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }

             // SUBW mode
            Compiler::emitVcpuAsm("LDI", "0xB8", false);
            Compiler::emitVcpuAsm("ST", "drawPoly_mode", false);

            // FlipX
            if(flipToken == "FLIPX")
            {
                Compiler::emitVcpuAsm("%SetPolyRelFlipX", "", false);
            }
            // FlipY
            else if(flipToken == "FLIPY")
            {
                Compiler::emitVcpuAsm("%SetPolyRelFlipY", "", false);
            }
            // FlipXY
            else
            {
                Compiler::emitVcpuAsm("%SetPolyRelFlipX", "", false);
                Compiler::emitVcpuAsm("%SetPolyRelFlipY", "", false);
            }
        }

        Expression::Numeric param;
        Compiler::parseExpression(codeLineIndex, tokens[0], param);
        Compiler::emitVcpuAsm("STW", "drawPoly_addr", false);
        Compiler::emitVcpuAsm("%DrawPolyRel", "", false);

        return true;
    }

    bool SCROLL(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::SCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::string scrollToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(scrollToken);
        if(scrollToken != "ON"  &&  scrollToken != "OFF")
        {
            fprintf(stderr, "Keywords::SCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        if(scrollToken == "ON")
        {
            Compiler::emitVcpuAsm("LDWI", "0x0001",   false);
            Compiler::emitVcpuAsm("ORW", "miscFlags", false);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "0xFFFE",    false);
            Compiler::emitVcpuAsm("ANDW", "miscFlags", false);
        }
        Compiler::emitVcpuAsm("STW", "miscFlags", false);

        return true;
    }

    bool POKE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::POKE() : Syntax error, 'POKE <address>, <value>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
            if(operandTypes[i] == Compiler::OperandInvalid) return false;
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;

            default: break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||
                                                                                                         operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDI", operands[1], false);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false);
                Compiler::emitVcpuAsm("ST", operands[0], false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false);
                Compiler::emitVcpuAsm("STW", "register0", false);
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false);
                Compiler::emitVcpuAsm("POKE", "register0", false);
            }
        }
        else
        {
            // Optimise for page 0
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                Compiler::emitVcpuAsm("LDI", operands[1], false);
                Compiler::emitVcpuAsm("ST",  operands[0], false);
            }
            // All other pages
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false);
                Compiler::emitVcpuAsm("STW",  "register0", false);
                Compiler::emitVcpuAsm("LDI",  operands[1], false);
                Compiler::emitVcpuAsm("POKE", "register0", false);
            }
        }

        return true;
    }

    bool DOKE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::DOKE() : syntax error, 'DOKE <address>, <value>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
            if(operandTypes[i] == Compiler::OperandInvalid) return false;
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;

            default: break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||
                                                                                                         operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDWI", operands[1], false);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false);
                Compiler::emitVcpuAsm("STW", operands[0], false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false);
                Compiler::emitVcpuAsm("STW", "register0", false);
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false);
                Compiler::emitVcpuAsm("DOKE", "register0", false);
            }
        }
        else
        {
            // Optimise for page 0
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                Compiler::emitVcpuAsm("LDWI", operands[1], false);
                Compiler::emitVcpuAsm("STW",  operands[0], false);
            }
            // All other pages
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false);
                Compiler::emitVcpuAsm("STW",  "register0", false);
                Compiler::emitVcpuAsm("LDWI", operands[1], false);
                Compiler::emitVcpuAsm("DOKE", "register0", false);
            }
        }

        return true;
    }

    void timeInit(void)
    {
        // Init time proc
        Compiler::setCreateTimeData(true);

        // ROM's 1 to 4
        if(Compiler::getCodeRomType() < Cpu::ROMv5a)
        {
            Compiler::emitVcpuAsm("LDI",  "0",               false);
            Compiler::emitVcpuAsm("STW",  "timerTick",       false);
            Compiler::emitVcpuAsm("LDI",  "giga_frameCount", false);
            Compiler::emitVcpuAsm("STW",  "timerPrev",       false);
        }
        // ROMv5a or higher
        else
        {
            Compiler::emitVcpuAsm("LDI",  "0",               false);
            Compiler::emitVcpuAsm("STW",  "timerTick",       false);
            Compiler::emitVcpuAsm("STW",  "timerPrev",       false);
        }
    }
    void midiInit(void)
    {
    }
    void midivInit(void)
    {
    }
    void userInit(void)
    {
    }
    void timeAddr(int index)
    {
        Compiler::emitVcpuAsm("LDWI", "tickTime",                             false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void midiAddr(int index)
    {
        Compiler::emitVcpuAsm("LDWI", "playMidi",                             false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void midivAddr(int index)
    {
        Compiler::emitVcpuAsm("LDWI", "playMidiVol",                            false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void userAddr(const std::string& label, int index)
    {
        Compiler::emitVcpuAsm("LDWI", "_" + label,                            false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void usageINIT(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::INIT() : Syntax error, use 'INIT TIME, MIDI, <user proc>, NOUPDATE; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
    }
    bool INIT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        enum InitTypes {InitTime, InitMidi, InitMidiV, InitUser};
        static std::map<std::string, InitTypes> initTypesMap = {{"TIME", InitTime}, {"MIDI", InitMidi}, {"MIDIV", InitMidiV}};

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 4)
        {
            usageINIT(codeLine, codeLineStart);
            return false;
        }

        if(tokens.size() == 1  ||  Compiler::getCodeRomType() > Cpu::ROMv4)
        {
            Expression::stripWhitespace(tokens[0]);
            std::string token = tokens[0];
            Expression::strToUpper(token);
            if(token == "NOUPDATE")
            {
                fprintf(stderr, "Keywords::INIT() : Syntax error, 'NOUPDATE' must be used with 'INIT TIME, MIDI, <user proc>' and only on ROMv4 or lower; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                fprintf(stderr, "Keywords::INIT() : Syntax error, use 'INIT VARS <optional starting var address>, TIME, MIDI, <user proc>, NOUPDATE; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                return false;
            }
        }

        // Search for init types and labels
        bool noUpdate = false;
        bool foundMidi = false;
        bool resetVars = false;
        std::string varsAddr;
        std::vector<InitTypes> initTypes;
        for(int i=0; i<int(tokens.size()); i++)
        {
            Expression::stripWhitespace(tokens[i]);
            std::string token = tokens[i];
            Expression::strToUpper(token);

            // Init type not found, so search for label
            if(initTypesMap.find(token) == initTypesMap.end())
            {
                if(token == "NOUPDATE")
                {
                    noUpdate = true;
                    continue;
                }

                size_t varsPos = token.find("VARS");
                if(varsPos != std::string::npos)
                {
                    // Check for starting var address
                    size_t addrPos = varsPos + sizeof("VARS")-1;
                    if(addrPos < tokens[i].size())
                    {
                        varsAddr = tokens[i].substr(addrPos);
                    }

                    resetVars = true;
                    continue;
                }

                int labIndex = Compiler::findLabel(tokens[i]);
                if(labIndex == -1)
                {
                    usageINIT(codeLine, codeLineStart);
                    return false;
                }
                else
                {
                    initTypes.push_back(InitUser);
                }
            }
            else
            {
                if(initTypesMap[token] == InitMidi  ||  initTypesMap[token] == InitMidiV)
                {
                    if(foundMidi)
                    {
                        fprintf(stderr, "Keywords::INIT() : Syntax error, can only init one instance of MIDI or MIDIV, use 'INIT TIME, MIDI/MIDIV, <user proc>; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                        return false;
                    }
                    foundMidi = true;
                }
                initTypes.push_back(initTypesMap[token]);
            }
        }

        // Proc and addr init
        for(int i=0; i<int(initTypes.size()); i++)
        {
            switch(initTypes[i])
            {
                case InitTime:  timeInit();  timeAddr(i);            break;
                case InitMidi:  midiInit();  midiAddr(i);            break;
                case InitMidiV: midivInit(); midivAddr(i);           break;
                case InitUser:  userInit();  userAddr(tokens[i], i); break;

                default: break;
            }
        }

        if(resetVars)
        {
            if(varsAddr.size())
            {
                Expression::Numeric numeric(true); // true = allow static init
                if(!Expression::parse(varsAddr, codeLineIndex, numeric)) return false;
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(int8_t(std::lround(numeric._value))), false);
            }
            else
            {
                Compiler::emitVcpuAsm("LDI", "giga_User", false);
            }

            Compiler::emitVcpuAsm("STW", "varAddress", false);
            Compiler::emitVcpuAsm("%ResetVars", "", false);
        }

        if(initTypes.size())
        {
            // ROM's 1 to 4, (time sliced code)
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                // If 'NOUPDATE" is specified then user must call tick himself
                if(!noUpdate)
                {
                    Compiler::enableSysInitFunc("InitRealTimeStub");
                    Linker::setInternalSubToLoad("realTimeStub");
                    Compiler::emitVcpuPreProcessingCmd("%define TIME_SLICING");
                    Compiler::emitVcpuAsm("LDWI", "setRealTimeProc" + std::to_string(initTypes.size() - 1), false);
                    Compiler::emitVcpuAsm("CALL", "giga_vAC",                                               false);
                }
            }
            // ROMv5a and higher, (vertical blank interrupt)
            else
            {
                // Vertical blank interrupt uses 0x30-0x33 for vPC and vAC save/restore, so we must move any variables found there
                Compiler::moveVblankVars();

                // Build chain of vertical blank interrupt handlers, (up to 3)
                Compiler::emitVcpuAsm("CALLI", "setRealTimeProc" + std::to_string(initTypes.size() - 1), false);

                // Start vertical blank interrupt
                Compiler::emitVcpuAsm("LDI",  "0xFF"           , false);
                Compiler::emitVcpuAsm("ST",   "giga_frameCount", false);
                Compiler::emitVcpuAsm("LDWI", "giga_vblankProc", false);
                Compiler::emitVcpuAsm("STW",  "register0"      , false);
                Compiler::emitVcpuAsm("LDWI", "realTimeStub"   , false);
                Compiler::emitVcpuAsm("DOKE", "register0"      , false);
            }
        }

        return true;
    }

    void usageTICK(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::INIT() : Syntax error, use 'TICK TIME/MIDI/MIDIV/ALL'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
    }
    bool TICK(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        if(Compiler::getCodeRomType() > Cpu::ROMv4)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            fprintf(stderr, "Keywords::TICK() : Version error, 'TICK' requires ROMv4 or lower, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() > 1)
        {
            usageTICK(codeLine, codeLineStart);
            return false;
        }

        std::string tickToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(tickToken);

        // Tick time
        if(tickToken == "TIME") 
        {
            Compiler::emitVcpuAsm("%TickTime", "", false);
            return true;
        }
        // Tick midi
        else if(tickToken == "MIDI") 
        {
            Compiler::emitVcpuAsm("%TickMidi", "", false);
            return true;
        }
        // Tick midi with volume
        else if(tickToken == "MIDIV") 
        {
            Compiler::emitVcpuAsm("%TickMidiV", "", false);
            return true;
        }
        // Tick everything
        else if(tickToken == "ALL") 
        {
            Compiler::emitVcpuAsm("%Tick", "", false);
            return true;
        }

        usageTICK(codeLine, codeLineStart);
        return false;
    }

    void usagePLAY(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::PLAY() : Syntax error, use 'PLAY <TYPE>, <optional address>, <optional waveType>', where <TYPE> = 'MIDI', 'MIDIV' or 'MUSIC'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
    }
    bool PLAY(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            usagePLAY(codeLine, codeLineStart);
            return false;
        }

        std::string midiToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(midiToken);
        if(midiToken != "MIDI"  &&  midiToken != "MIDIV"  &&  midiToken != "MUSIC")
        {
            usagePLAY(codeLine, codeLineStart);
            return false;
        }

        // Default wave type
        if(tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDI", "2",       false);
            Compiler::emitVcpuAsm("ST", "waveType + 1", false);
        }
        // Midi wave type, (optional)
        else if(tokens.size() == 3)
        {
            std::string waveTypeToken = tokens[2];
            Expression::Numeric param;
            Compiler::parseExpression(codeLineIndex, waveTypeToken, param);
            Compiler::emitVcpuAsm("ST", "waveType + 1", false);
        }

        // Midi stream address
        std::string addressToken = tokens[1];
        Expression::Numeric param;
        Compiler::parseExpression(codeLineIndex, addressToken, param);
        if(midiToken == "MIDI")
        {
            Compiler::emitVcpuAsm("%PlayMidi", "", false);
        }
        else if(midiToken == "MIDIV")
        {
            Compiler::emitVcpuAsm("%PlayMidiV", "", false);
        }
        else if(midiToken == "MUSIC")
        {
            Compiler::emitVcpuAsm("%PlayMusic", "", false);
        }

        return true;
    }

    void usageLOAD(int msgType, Compiler::CodeLine& codeLine, int codeLineStart)
    {
        switch(msgType)
        {
            case 0: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD <TYPE>, <filename>, where <TYPE> = 'IMAGE', 'SPRITE', 'FONT', 'MIDI', 'WAVE', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart); break;
            case 1: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD IMAGE, <filename>, <optional address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);                                       break;
            case 2: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD SPRITE, <filename>, <id>, <optional flip>, <optional overlap>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);               break;
            case 3: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD FONT, <filename>, <id>, <optional 16 bit bg:fg colours>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);                     break;
            case 4: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD MIDI, <filename>, <optional address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);                                        break;
            case 5: fprintf(stderr, "Keywords::LOAD() : Syntax error, use 'LOAD WAVE, <filename>, <optional address>, <optional address offset>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);             break;

            default: break;
        }
    }
    bool handleLOADchunk(Compiler::CodeLine& codeLine, int codeLineStart, const std::vector<uint8_t>& data, int row, uint16_t width, uint16_t address, uint8_t chunkSize, uint16_t& chunkOffset, uint16_t& chunkAddr)
    {
        if(!Memory::getFreeRAM(Memory::FitDescending, chunkSize, 0x0200, Compiler::getRuntimeStart(), chunkAddr))
        {
            usageLOAD(1, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::LOAD() : Allocating RAM for offscreen pixel chunk on row %d failed, in '%s' on line %d\n", row, codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<uint8_t> chunkData;
        for(int j=0; j<chunkSize; j++) chunkData.push_back(data[row*width + chunkOffset + j]);

        // Output loader image chunks copy code
        if(Compiler::getDefDataLoaderImageChunks().size() == 0)
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                Compiler::emitVcpuAsm("LDWI", "copyLoaderImages", false);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false);
            }
            else
            {
                Compiler::emitVcpuAsm("CALLI", "copyLoaderImages", false);
            }
        }

        Compiler::DefDataLoaderImageChunk defDataLoaderImageChunk = {chunkAddr, uint16_t(address + chunkOffset), chunkSize, chunkData};
        Compiler::getDefDataLoaderImageChunks().push_back(defDataLoaderImageChunk);

        chunkOffset += chunkSize;

        return true;
    }
    bool LOAD(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 2  ||  tokens.size() > 5)
        {
            usageLOAD(0, codeLine, codeLineStart);
            return false;
        }

        // Type
        Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(tokens[0]);
        if(tokens[0] != "MIDI"  &&  tokens[0] != "IMAGE"  &&  tokens[0] != "SPRITE"  &&  tokens[0] != "FONT"  &&  tokens[0] != "WAVE")
        {
            usageLOAD(0, codeLine, codeLineStart);
            return false;
        }

        // Handle MIDI and WAVE
        if(tokens[0] == "MIDI"  ||  tokens[0] == "WAVE")
        {
            if(tokens[0] == "MIDI")
            {
                usageLOAD(4, codeLine, codeLineStart);
            }
            else if(tokens[0] == "WAVE")
            {
                std::string filename = tokens[1];
                Expression::stripWhitespace(filename);
                std::string ext = filename;
                Expression::strToUpper(ext);
                if(ext.find(".GTWAV") != std::string::npos)
                {
                    std::string filepath = Loader::getFilePath();
                    size_t slash = filepath.find_last_of("\\/");
                    filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
                    filename = filepath + "/" + filename;

                    // Parse optional address
                    uint16_t address = RAM_AUDIO_START;
                    if(tokens.size() >= 3)
                    {
                        std::string addrToken = tokens[2];
                        Expression::Numeric addrNumeric;
                        std::string addrOperand;
                        Compiler::parseExpression(codeLineIndex, addrToken, addrOperand, addrNumeric);
                        address = uint16_t(std::lround(addrNumeric._value));
                        if(address < DEFAULT_START_ADDRESS)
                        {
                            usageLOAD(5, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, addrToken.c_str(), codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                    }

                    // Parse optional address
                    uint16_t addrOffset = 0;
                    if(tokens.size() == 4)
                    {
                        std::string offsetToken = tokens[3];
                        Expression::Numeric offsetNumeric;
                        std::string offsetOperand;
                        Compiler::parseExpression(codeLineIndex, offsetToken, offsetOperand, offsetNumeric);
                        addrOffset = uint16_t(std::lround(offsetNumeric._value));
                    }

                    // Load wave file
                    std::vector<uint8_t> dataBytes(64);
                    std::ifstream infile(filename, std::ios::binary | std::ios::in);
                    if(!infile.is_open())
                    {
                        usageLOAD(5, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : failed to open file '%s', in '%s' on line %d\n", filename.c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }
                    infile.read((char *)&dataBytes[0], 64);
                    if(infile.eof() || infile.bad() || infile.fail())
                    {
                        usageLOAD(5, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : failed to read file '%s', in '%s' on line %d\n", filename.c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    uint16_t addr = address;
                    for(int i=0; i<int(dataBytes.size()); i++)
                    {
                        if(addrOffset != 0)
                        {
                            if(addr < RAM_AUDIO_START  ||  addr > RAM_AUDIO_END)
                            {
                                if(!Memory::takeFreeRAM(addr, 1)) return false;
                                addr += addrOffset;
                            }
                        }
                    }
                    if(addrOffset == 0)
                    {
                        if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
                    }
                    Compiler::getDefDataBytes().push_back({address, addrOffset, dataBytes});
                }
            }
        }

        // Handle Image, Sprite and Font
        if(tokens[0] == "IMAGE"  ||  tokens[0] == "SPRITE"  ||  tokens[0] == "FONT")
        {
            std::string filename = tokens[1];
            Expression::stripWhitespace(filename);
            std::string ext = filename;
            Expression::strToUpper(ext);
            if(ext.find(".TGA") != std::string::npos)
            {
                std::string filepath = Loader::getFilePath();
                size_t slash = filepath.find_last_of("\\/");
                filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
                filename = filepath + "/" + filename;
                Image::TgaFile tgaFile;

                //fprintf(stderr, "\nKeywords::LOAD() : %s\n", filename.c_str());

                if(!Image::loadTgaFile(filename, tgaFile))
                {
                    fprintf(stderr, "Keywords::LOAD() : File '%s' failed to load, in '%s' on line %d\n", filename.c_str(), codeLine._text.c_str(), codeLineStart);
                    return false;
                }

                // Load image/sprite/font
                std::vector<uint8_t> data;
                std::vector<uint16_t> optional;
                Image::GtRgbFile gtRgbFile{GTRGB_IDENTIFIER, Image::GT_RGB_222, tgaFile._header._width, tgaFile._header._height, data, optional};
                Image::convertRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);

                // Image
                if(tokens[0] == "IMAGE")
                {
                    const uint16_t stride = 256;

                    if(tokens.size() > 3)
                    {
                        usageLOAD(1, codeLine, codeLineStart);
                        return false;
                    }

                    // Parse optional address
                    uint16_t address = RAM_VIDEO_START;
                    if(tokens.size() == 3)
                    {
                        std::string addrToken = tokens[2];
                        Expression::Numeric addrNumeric;
                        std::string addrOperand;
                        Compiler::parseExpression(codeLineIndex, addrToken, addrOperand, addrNumeric);
                        address = uint16_t(std::lround(addrNumeric._value));
                        if(address < DEFAULT_START_ADDRESS)
                        {
                            usageLOAD(1, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, addrToken.c_str(), codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                    }

                    if(gtRgbFile._header._width > stride  ||  gtRgbFile._header._width + (address & 0x00FF) > stride)
                    {
                        usageLOAD(1, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Image width %d + starting address 0x%04x overflow, for %s; in '%s' on line %d\n", gtRgbFile._header._width, address, filename.c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    Compiler::DefDataImage defDataImage = {address, tgaFile._header._width, tgaFile._header._height, stride, gtRgbFile._data};
                    Compiler::getDefDataImages().push_back(defDataImage);

                    int size = gtRgbFile._header._width;
                    for(int y=0; y<gtRgbFile._header._height; y++)
                    {
                        // Take offscreen memory from compiler for images wider than visible screen resolution, or images in offscreen memory
                        if(address >= RAM_VIDEO_START  &&  address <= 0x7FFF)
                        {
                            size = gtRgbFile._header._width + (address & 0x00FF) - RAM_SCANLINE_SIZE;
                            if(size > 0)
                            {
                                if(!Memory::takeFreeRAM((address & 0xFF00) + RAM_SCANLINE_SIZE, size))
                                {
                                    usageLOAD(1, codeLine, codeLineStart);
                                    fprintf(stderr, "Keywords::LOAD() : Allocating RAM for pixel row %d failed, in '%s' on line %d\n", y, codeLine._text.c_str(), codeLineStart);
                                    return false;
                                }
                            }
                        }

                        // 'Loader.gcl' is resident at these addresses when loading *.gt1 files, therefore you can overwrite these locations only AFTER the loading process has finished
                        // Split up image scanlines into offscreen chunks and load the offscreen chunks into the correct onscreen memory locations after 'Loader.gcl' is done
                        if((address >= LOADER_SCANLINE0_START  &&  address<= LOADER_SCANLINE0_END)  ||  (address >= LOADER_SCANLINE1_START  &&  address<= LOADER_SCANLINE1_END)  ||  
                           (address >= LOADER_SCANLINE2_START  &&  address<= LOADER_SCANLINE2_END))
                        {
                            uint16_t chunkOffset = 0x0000, chunkAddr = 0x0000;
                            for(int i=0; i<gtRgbFile._header._width / RAM_SEGMENTS_SIZE; i++)
                            {
                                handleLOADchunk(codeLine, codeLineStart, gtRgbFile._data, y, gtRgbFile._header._width, address, RAM_SEGMENTS_SIZE, chunkOffset, chunkAddr);
                            }
                            handleLOADchunk(codeLine, codeLineStart, gtRgbFile._data, y, gtRgbFile._header._width, address, gtRgbFile._header._width % RAM_SEGMENTS_SIZE, chunkOffset, chunkAddr);
                        }

                        // Next destination row
                        address += stride; 
                    }
                }
                // Sprite
                else if(tokens[0] == "SPRITE")
                {
                    if(Compiler::getCodeRomType() < Cpu::ROMv3)
                    {
                        std::string romTypeStr;
                        getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                        usageLOAD(2, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Version error, 'LOAD SPRITE' requires ROMv3 or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(),  codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    if(tokens.size() > 5)
                    {
                        usageLOAD(2, codeLine, codeLineStart);
                        return false;
                    }

                    if(gtRgbFile._header._width % SPRITE_CHUNK_SIZE != 0)
                    {
                        usageLOAD(2, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Sprite width not a multiple of %d, (%d x %d), for %s; in '%s' on line %d\n", SPRITE_CHUNK_SIZE, gtRgbFile._header._width, gtRgbFile._header._height, filename.c_str(),
                                                                                                                                                            codeLine._text.c_str(),
                                                                                                                                                            codeLineStart);
                        return false;
                    }

                    if(tokens.size() < 3)
                    {
                        usageLOAD(2, codeLine, codeLineStart);
                        return false;
                    }

                    // Unique sprite ID
                    std::string idToken = tokens[2];
                    Expression::Numeric idNumeric;
                    std::string idOperand;
                    Compiler::parseExpression(codeLineIndex, idToken, idOperand, idNumeric);
                    int spriteId = int(std::lround(idNumeric._value));
                    if(Compiler::getDefDataSprites().find(spriteId) != Compiler::getDefDataSprites().end())
                    {
                        usageLOAD(2, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Sprite id not unique, %d; in '%s' on line %d\n", spriteId, codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Parse optional sprite flip
                    Compiler::SpriteFlipType flipType = Compiler::NoFlip;
                    if(tokens.size() >= 4)
                    {
                        std::string flipToken = tokens[3];
                        Expression::stripWhitespace(flipToken);
                        Expression::strToUpper(flipToken);
                        if(flipToken == "NOFLIP")      flipType = Compiler::NoFlip;
                        else if(flipToken == "FLIPX")  flipType = Compiler::FlipX;
                        else if(flipToken == "FLIPY")  flipType = Compiler::FlipY;
                        else if(flipToken == "FLIPXY") flipType = Compiler::FlipXY;
                        else
                        {
                            usageLOAD(2, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : Unknown sprite flip type, %s; in '%s' on line %d\n", flipToken.c_str(), codeLine._text.c_str(), codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : Must use one of 'NOFLIP', 'FLIPX', 'FLIPY', 'FLIPXY'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                    }

                    // Parse optional sprite overlap, (for last column)
                    uint16_t overlap = 0;
                    if(tokens.size() == 5)
                    {
                        std::string overlapToken = tokens[4];
                        Expression::Numeric overlapNumeric;
                        std::string overlapOperand;
                        Compiler::parseExpression(codeLineIndex, overlapToken, overlapOperand, overlapNumeric);
                        overlap = uint16_t(std::lround(overlapNumeric._value));
                    }

                    // Build sprite data from image data
                    uint16_t numColumns = gtRgbFile._header._width / SPRITE_CHUNK_SIZE;
                    uint16_t remStripeChunks = gtRgbFile._header._height % Compiler::getSpriteStripeChunks();
                    uint16_t numStripesPerCol = gtRgbFile._header._height / Compiler::getSpriteStripeChunks() + int(remStripeChunks > 0);
                    uint16_t numStripeChunks = (numStripesPerCol == 1) ? gtRgbFile._header._height : Compiler::getSpriteStripeChunks();
                    std::vector<uint16_t> stripeAddrs;
                    std::vector<uint8_t> spriteData;

                    if(numColumns == 1  &&  overlap)
                    {
                        usageLOAD(2, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Can't have a non zero overlap with a single column sprite; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Search sprite image for instancing
                    int parentInstance = 0;
                    bool isInstanced = false;
                    for(auto it=Compiler::getDefDataSprites().begin(); it!=Compiler::getDefDataSprites().end(); ++it)
                    {
                        if(it->second._filename == filename)
                        {
                            spriteData = it->second._data;
                            parentInstance = it->first;
                            isInstanced = true;
                            break;
                        }
                    }

                    // Allocate sprite memory
                    int addrIndex = 0;
                    uint16_t address = 0x0000;
                    for(int i=0; i<numColumns; i++)
                    {
                        // One stripe per column
                        if(numStripesPerCol == 1)
                        {
                            if(isInstanced)
                            {
                                address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                addrIndex += 2;
                            }
                            else
                            {
                                if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), 
                                                       Compiler::getRuntimeStart(), address))
                                {
                                    usageLOAD(2, codeLine, codeLineStart);
                                    fprintf(stderr, "Keywords::LOAD() : Getting Sprite memory for stripe %d failed, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineStart);
                                    return false;
                                }
                            }
                            stripeAddrs.push_back(address);

                            // Destination offsets
                            switch(flipType)
                            {
                                case Compiler::NoFlip: stripeAddrs.push_back(uint16_t(0 + i*6));
                                                       if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                break;

                                case Compiler::FlipX: stripeAddrs.push_back(uint16_t(0 + (numColumns-1-i)*6));
                                                      if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                break;

                                case Compiler::FlipY: stripeAddrs.push_back(uint16_t((numStripeChunks-1)*256 + i*6));
                                                      if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                break;

                                case Compiler::FlipXY: stripeAddrs.push_back(uint16_t((numStripeChunks-1)*256 + (numColumns-1-i)*6));
                                                       if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                break;

                                default: break;
                            }

                            // Copy sprite data and delimiter
                            for(int j=0; j<numStripeChunks; j++)
                            {
                                for(int k=0; k<SPRITE_CHUNK_SIZE; k++)
                                {
                                    spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + j*SPRITE_CHUNK_SIZE*numColumns + k]);
                                }
                            }
                            spriteData.push_back(uint8_t(-gtRgbFile._header._height));
                        }
                        // Multiple stripes per column
                        else
                        {
                            // MAX_SPRITE_CHUNKS_PER_STRIPE stripes
                            for(int j=0; j<numStripesPerCol-1; j++)
                            {
                                if(isInstanced)
                                {
                                    address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                    addrIndex += 2;
                                }
                                else
                                {
                                    if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), 
                                                           Compiler::getRuntimeStart(), address))
                                    {
                                        usageLOAD(2, codeLine, codeLineStart);
                                        fprintf(stderr, "Keywords::LOAD() : Getting Sprite memory failed for stripe %d, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineStart);
                                        return false;
                                    }
                                }
                                stripeAddrs.push_back(address);

                                // Destination offsets
                                switch(flipType)
                                {
                                    case Compiler::NoFlip: stripeAddrs.push_back(uint16_t(j*numStripeChunks*256 + i*6));
                                                           if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                    break;

                                    case Compiler::FlipX: stripeAddrs.push_back(uint16_t(j*numStripeChunks*256 + (numColumns-1-i)*6));
                                                          if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                    break;

                                    case Compiler::FlipY: stripeAddrs.push_back(uint16_t(((numStripesPerCol-1-j)*numStripeChunks+remStripeChunks-1)*256 + i*6));
                                                          if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                    break;

                                    case Compiler::FlipXY: stripeAddrs.push_back(uint16_t(((numStripesPerCol-1-j)*numStripeChunks+remStripeChunks-1)*256 + (numColumns-1-i)*6));
                                                           if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                    break;

                                    default: break;
                                }

                                // Copy sprite data and delimiter
                                for(int k=0; k<numStripeChunks; k++)
                                {
                                    for(int l=0; l<SPRITE_CHUNK_SIZE; l++)
                                    {
                                        spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + j*numStripeChunks*SPRITE_CHUNK_SIZE*numColumns + k*SPRITE_CHUNK_SIZE*numColumns + l]);
                                    }
                                }
                                spriteData.push_back(255);
                            }

                            // Remainder stripe
                            if(isInstanced)
                            {
                                address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                addrIndex += 2;
                            }
                            else
                            {
                                if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), remStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(),
                                                       Compiler::getRuntimeStart(), address))
                                {
                                    usageLOAD(2, codeLine, codeLineStart);
                                    fprintf(stderr, "Keywords::LOAD() : Getting Sprite memory failed for stripe %d, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineStart);
                                    return false;
                                }
                            }
                            stripeAddrs.push_back(address);

                            // Destination offsets
                            switch(flipType)
                            {
                                case Compiler::NoFlip: stripeAddrs.push_back(uint16_t((numStripesPerCol-1)*numStripeChunks*256 + i*6));
                                                       if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                break;

                                case Compiler::FlipX: stripeAddrs.push_back(uint16_t((numStripesPerCol-1)*numStripeChunks*256 + (numColumns-1-i)*6));
                                                      if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                break;

                                case Compiler::FlipY: stripeAddrs.push_back(uint16_t((remStripeChunks-1)*256 + i*6));
                                                      if(i == numColumns - 1) stripeAddrs.back() -= overlap;  // push last column closer to all other columns
                                break;

                                case Compiler::FlipXY: stripeAddrs.push_back(uint16_t((remStripeChunks-1)*256 + (numColumns-1-i)*6));
                                                       if(i != numColumns - 1) stripeAddrs.back() -= overlap;  // push all other columns closer to last column
                                break;

                                default: break;
                            }

                            // Copy sprite data and delimiter
                            for(int j=0; j<remStripeChunks; j++)
                            {
                                for(int k=0; k<SPRITE_CHUNK_SIZE; k++)
                                {
                                    spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + (numStripesPerCol-1)*numStripeChunks*SPRITE_CHUNK_SIZE*numColumns + j*SPRITE_CHUNK_SIZE*numColumns + k]);
                                }
                            }
                            spriteData.push_back(255);
                        }
                    }

                    Compiler::DefDataSprite defDataSprite = {spriteId, filename, tgaFile._header._width, tgaFile._header._height, numColumns, numStripesPerCol, numStripeChunks, remStripeChunks,
                                                             stripeAddrs, spriteData, flipType, isInstanced};
                    Compiler::getDefDataSprites()[spriteId] = defDataSprite;
                }
                else if(tokens[0] == "FONT")
                {
                    if(Compiler::getCodeRomType() < Cpu::ROMv3)
                    {
                        std::string romTypeStr;
                        getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Version error, 'LOAD FONT' requires ROMv3 or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    if(tokens.size() < 3  ||  tokens.size() > 4)
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        return false;
                    }

                    // Unique font ID
                    std::string idToken = tokens[2];
                    Expression::Numeric idNumeric;
                    std::string idOperand;
                    Compiler::parseExpression(codeLineIndex, idToken, idOperand, idNumeric);
                    int fontId = int(std::lround(idNumeric._value));
                    if(Compiler::getDefDataFonts().find(fontId) != Compiler::getDefDataFonts().end())
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Font id not unique, %d; in '%s' on line %d\n", fontId, codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Foreground/background colours
                    uint16_t fgbgColour = 0x0000;
                    if(tokens.size() == 4)
                    {
                        std::string fgbgToken = tokens[3];
                        Expression::Numeric fgbgNumeric;
                        std::string fgbgOperand;
                        Compiler::parseExpression(codeLineIndex, fgbgToken, fgbgOperand, fgbgNumeric);
                        fgbgColour = uint16_t(std::lround(fgbgNumeric._value));
                    }

                    // Width
                    if(gtRgbFile._header._width % FONT_WIDTH != 0)
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Font width %d is not a multiple of %d; in '%s' on line %d\n", gtRgbFile._header._width, FONT_WIDTH, codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Height
                    if(gtRgbFile._header._height % FONT_HEIGHT != 0)
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Font height %d is not a multiple of %d; in '%s' on line %d\n", gtRgbFile._header._height, FONT_HEIGHT, codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Load font mapping file
                    bool foundMapFile = true;
                    size_t nameSuffix = filename.find_last_of(".");
                    filename = filename.substr(0, nameSuffix) + ".map";
                    std::ifstream infile(filename, std::ios::in);
                    if(!infile.is_open())
                    {
                        foundMapFile = false;
                    }

                    // Parse font mapping file
                    int maxIndex = -1;
                    uint16_t mapAddr = 0x0000;
                    std::vector<uint8_t> mapping(MAPPING_SIZE);
                    if(foundMapFile)
                    {
                        int ascii, index, line = 0;
                        while(!infile.eof())
                        {
                            infile >> ascii >> index;
                            if(index > maxIndex) maxIndex = index;
                            if(!infile.good() && !infile.eof())
                            {
                                usageLOAD(3, codeLine, codeLineStart);
                                fprintf(stderr, "Keywords::LOAD() : error in Mapping file %s on line %d; in '%s' on line %d\n", filename.c_str(), line + 1, codeLine._text.c_str(), codeLineStart);
                                return false;
                            }

                            if(line >= MAPPING_SIZE) break;
                            mapping[line++] = uint8_t(index);
                        }

                        if(line != MAPPING_SIZE)
                        {
                            usageLOAD(3, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : warning, found an incorrect number of map entries %d for file %s, should be %d; in '%s' on line %d\n", line - 1, filename.c_str(), MAPPING_SIZE, codeLine._text.c_str(), codeLineStart);
                            return false;
                        }

                        if(!Memory::getFreeRAM(Memory::FitDescending, MAPPING_SIZE, 0x0200, Compiler::getRuntimeStart(), mapAddr))
                        {
                            usageLOAD(3, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::LOAD() : Getting Mapping memory for Map size of %d failed, in '%s' on line %d\n", MAPPING_SIZE, codeLine._text.c_str(), codeLineStart);
                            return false;
                        }
                    }

                    // 8th line is implemented as a separate sprite call, to save memory and allow for more efficient memory packing
                    const int kCharHeight = FONT_HEIGHT-1;

                    // Copy font data and create delimiter
                    std::vector<uint8_t> charData;
                    std::vector<uint16_t> charAddrs;
                    std::vector<std::vector<uint8_t>> fontData;
                    for(int j=0; j<tgaFile._header._height; j+=FONT_HEIGHT)
                    {
                        for(int i=0; i<tgaFile._header._width; i+=FONT_WIDTH)
                        {
                            for(int l=0; l<kCharHeight; l++)
                            {
                                for(int k=0; k<FONT_WIDTH; k++)
                                {
                                    uint8_t pixel = gtRgbFile._data[j*tgaFile._header._width + i + l*tgaFile._header._width + k];
                                    if(fgbgColour)
                                    {
                                        if(pixel == 0x00) pixel = fgbgColour & 0x00FF;
                                        if(pixel == 0x3F) pixel = fgbgColour >> 8;
                                    }
                                    charData.push_back(pixel);
                                }
                            }
                            charData.push_back(uint8_t(-(kCharHeight)));
                            fontData.push_back(charData);
                            charData.clear();

                            uint16_t address = 0x0000;
                            if(!Memory::getFreeRAM(Memory::FitDescending, (kCharHeight)*FONT_WIDTH + 1, 0x0200, Compiler::getRuntimeStart(), address))
                            {
                                usageLOAD(3, codeLine, codeLineStart);
                                fprintf(stderr, "Keywords::LOAD() : Getting font memory for char %d failed, in '%s' on line %d\n", int(fontData.size() - 1), codeLine._text.c_str(), codeLineStart);
                                return false;
                            }

                            charAddrs.push_back(address);
                        }
                    }

                    if(foundMapFile  &&  maxIndex + 1 != int(fontData.size()))
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Font mapping table does not match font data, found a mapping count of %d and a chars count of %d, in '%s' on line %d\n", maxIndex + 1, int(fontData.size()), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    // Create baseline for all chars in each font
                    uint16_t baseAddr = 0x0000;
                    if(!Memory::getFreeRAM(Memory::FitDescending, FONT_WIDTH + 1, 0x0200, Compiler::getRuntimeStart(), baseAddr))
                    {
                        usageLOAD(3, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::LOAD() : Getting font memory for char %d failed, in '%s' on line %d\n", int(fontData.size() - 1), codeLine._text.c_str(), codeLineStart);
                        return false;
                    }

                    Compiler::DefDataFont defDataFont = {fontId, filename, tgaFile._header._width, tgaFile._header._height, charAddrs, fontData, mapAddr, mapping, baseAddr, fgbgColour};
                    Compiler::getDefDataFonts()[fontId] = defDataFont;

                    Linker::enableFontLinking();
                }
            }
        }

        return true;
    }

    bool SPRITE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        if(Compiler::getCodeRomType() < Cpu::ROMv3)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            fprintf(stderr, "Keywords::SPRITE() : Version error, 'SPRITE' requires ROMv3 or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::SPRITE() : Syntax error, use 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x pos>, <y pos>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Flip type
        static std::map<std::string, Compiler::SpriteFlipType> flipType = {{"NOFLIP", Compiler::NoFlip}, {"FLIPX", Compiler::FlipX}, {"FLIPY", Compiler::FlipY}, {"FLIPXY", Compiler::FlipXY}};
        std::string flipToken = tokens[0];
        Expression::stripWhitespace(flipToken);
        Expression::strToUpper(flipToken);
        if(flipType.find(flipToken) == flipType.end())
        {
            fprintf(stderr, "Keywords::SPRITE() : Syntax error, use one of the correct flip types, 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x pos>, <y pos>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Sprite identifier
        std::string idToken = tokens[1];
        Expression::Numeric idParam;
        Compiler::parseExpression(codeLineIndex, idToken, idParam);
        Compiler::emitVcpuAsm("STW", "spriteId", false);

        // Sprite X position
        std::string xposToken = tokens[2];
        Expression::Numeric xposParam;
        Compiler::parseExpression(codeLineIndex, xposToken, xposParam);
        Compiler::emitVcpuAsm("ST", "spriteXY", false);

        // Sprite Y position
        std::string yposToken = tokens[3];
        Expression::Numeric yposParam;
        Compiler::parseExpression(codeLineIndex, yposToken, yposParam);
        Compiler::emitVcpuAsm("ADDI", "8", false);
        Compiler::emitVcpuAsm("ST", "spriteXY + 1", false);

        // Draw sprite
        switch(flipType[flipToken])
        {
            case Compiler::NoFlip: Compiler::emitVcpuAsm("%DrawSprite",   "", false); break;
            case Compiler::FlipX:  Compiler::emitVcpuAsm("%DrawSpriteX",  "", false); break;
            case Compiler::FlipY:  Compiler::emitVcpuAsm("%DrawSpriteY",  "", false); break;
            case Compiler::FlipXY: Compiler::emitVcpuAsm("%DrawSpriteXY", "", false); break;
        }
 
        return true;
    }

    void usageSOUND(int msgType, Compiler::CodeLine& codeLine, int codeLineStart)
    {
        switch(msgType)
        {
            case 0: fprintf(stderr, "Keywords::SOUND() : Syntax error, use 'SOUND <TYPE>, <params>, where <TYPE> = 'ON', 'MOD' or 'OFF'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);               break;
            case 1: fprintf(stderr, "Keywords::SOUND() : Syntax error, use 'SOUND ON, <channel>, <frequency>, <optional volume>, <optional waveform>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart); break;
            case 2: fprintf(stderr, "Keywords::SOUND() : Syntax error, use 'SOUND MOD, <channel>, <wavX>, <optional wavA>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);                            break;
            case 3: fprintf(stderr, "Keywords::SOUND() : Syntax error, use 'SOUND OFF, <optional channel>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);                                            break;

            default: break;
        }
    }
    bool SOUND(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 5)
        {
            usageSOUND(0, codeLine, codeLineStart);
            return false;
        }

        // Sound state
        std::string stateToken = tokens[0];
        Expression::stripWhitespace(stateToken);
        Expression::strToUpper(stateToken);

        // Sound channel, (has to be between 1 and 4, saves an ADD/INC, no checking done)
        if(tokens.size() >= 2)
        {
            std::string chanToken = tokens[1];
            Expression::Numeric chanParam;
            Compiler::parseExpression(codeLineIndex, chanToken, chanParam);
            Compiler::emitVcpuAsm("ST", "sndChannel + 1", false);
        }
        
        // Sound channels on
        if(stateToken == "ON")
        {
            if(tokens.size() < 3)
            {
                usageSOUND(1, codeLine, codeLineStart);
                return false;
            }

            std::string freqToken = tokens[2];
            Expression::Numeric freqParam;
            Compiler::parseExpression(codeLineIndex, freqToken, freqParam);
            Compiler::emitVcpuAsm("STW", "sndFrequency", false);

            if(tokens.size() == 3)
            {
                Compiler::emitVcpuAsm("%SoundOn", "", false);
                return true;
            }

            std::string volToken = tokens[3];
            Expression::Numeric volParam;
            Compiler::parseExpression(codeLineIndex, volToken, volParam);
            Compiler::emitVcpuAsm("STW", "sndVolume", false);

            if(tokens.size() == 4)
            {
                Compiler::emitVcpuAsm("LDI", "2",           false);
                Compiler::emitVcpuAsm("STW", "sndWaveType", false);
                Compiler::emitVcpuAsm("%SoundOnV", "",      false);
                return true;
            }

            std::string wavToken = tokens[4];
            Expression::Numeric wavParam;
            Compiler::parseExpression(codeLineIndex, wavToken, wavParam);
            Compiler::emitVcpuAsm("STW", "sndWaveType", false);
            Compiler::emitVcpuAsm("%SoundOnV", "",      false);

            return true;
        }

        // Sound channels modulation
        if(stateToken == "MOD")
        {
            if(tokens.size() < 3  ||  tokens.size() > 4)
            {
                usageSOUND(2, codeLine, codeLineStart);
                return false;
            }

            std::string waveXToken = tokens[2];
            Expression::Numeric waveXParam;
            Compiler::parseExpression(codeLineIndex, waveXToken, waveXParam);
            Compiler::emitVcpuAsm("ST", "sndWaveType + 1", false);

            if(tokens.size() == 4)
            {
                std::string waveAToken = tokens[3];
                Expression::Numeric waveAParam;
                Compiler::parseExpression(codeLineIndex, waveAToken, waveAParam);
                Compiler::emitVcpuAsm("ST", "sndWaveType", false);
            }
            // Reset waveA
            else
            {
                Compiler::emitVcpuAsm("LDI", "0",          false);
                Compiler::emitVcpuAsm("ST", "sndWaveType", false);
            }

            Compiler::emitVcpuAsm("%SoundMod", "", false);
            return true;
        }

        // Sound channels off
        if(stateToken == "OFF")
        {
            if(tokens.size() > 2)
            {
                usageSOUND(3, codeLine, codeLineStart);
                return false;
            }

            // All sound channels off
            if(tokens.size() == 1)
            {
                Compiler::emitVcpuAsm("%SoundAllOff", "", false);
                return true;
            }
            // Single channel off
            else
            {
                Compiler::emitVcpuAsm("%SoundOff", "", false);
                return true;
            }
        }

        usageSOUND(0, codeLine, codeLineStart);
        return false;
    }

    void usageSET(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::SET() : Syntax error, use 'SET <VAR NAME>, <PARAM>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
    }
    bool SET(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            usageSET(codeLine, codeLineStart);
            return false;
        }

        // System variable to set
        std::string sysVarName = tokens[0];
        Expression::stripWhitespace(sysVarName);
        Expression::strToUpper(sysVarName);

        // First parameter after system var name
        std::string token1;
        Expression::Numeric param1;
        if(tokens.size() >= 2) token1 = tokens[1];

        // Second parameter after system var name
        std::string token2;
        Expression::Numeric param2;
        if(tokens.size() >= 3) token2 = tokens[2];

        // Font id variable
        if(sysVarName == "FONT_ID"  &&  tokens.size() == 2)
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv3)
            {
                std::string romTypeStr;
                getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                fprintf(stderr, "Keywords::SET() : Version error, 'SET FONTID' requires ROMv3 or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            Compiler::emitVcpuAsm("LDWI", "_fontId_", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_MODE")
        {
            Compiler::emitVcpuAsm("LDWI", "handleT_mode + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_EPOCH")
        {
            Compiler::emitVcpuAsm("LDWI", "handleT_epoch + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_S")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 0", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_M")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_H")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 2", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIMER")
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "timerTick", false);
            return true;
        }
        else if(sysVarName == "VBLANK_PROC")
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                std::string romTypeStr;
                getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                fprintf(stderr, "Keywords::SET() : Version error, 'SET VBLANK_PROC' requires ROMv5a or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(VBLANK_PROC), false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("DOKE", "register0", false);
            return true;
        }
        else if(sysVarName == "VBLANK_FREQ")
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                std::string romTypeStr;
                getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                fprintf(stderr, "Keywords::SET() : Version error, 'SET VBLANK_FREQ' requires ROMv5a or higher, you are trying to link against '%s', in '%s' on line %d\n", romTypeStr.c_str(), codeLine._text.c_str(), codeLineStart);
                return false;
            }

#if 0
            // TODO: Fix this
            // (256 - n) = vblank interrupt frequency, where n = 1 to 255
            Compiler::emitVcpuAsm("LDWI", "realTS_rti + 2", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "register1", false);
            Compiler::emitVcpuAsm("LDWI", "256", false);
            Compiler::emitVcpuAsm("SUBW", "register1", false);
            Compiler::emitVcpuAsm("POKE", "register0", false);
#endif
            return true;
        }
        else if(sysVarName == "CURSOR_X"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "cursorXY", false);
            return true;
        }
        else if(sysVarName == "CURSOR_Y"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "cursorXY + 1", false);
            return true;
        }
        else if(sysVarName == "CURSOR_XY"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "cursorXY", false);
            return true;
        }
        else if(sysVarName == "FG_COLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "fgbgColour + 1", false);
            return true;
        }
        else if(sysVarName == "BG_COLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "fgbgColour", false);
            return true;
        }
        else if(sysVarName == "FGBG_COLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "fgbgColour", false);
            return true;
        }
        else if(sysVarName == "MIDI_STREAM"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "midiStream", false);
            return true;
        }
        else if(sysVarName == "VIDEO_TOP"  &&  tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDWI", "giga_videoTop", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "LED_TEMPO"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_ledTempo", false);
            return true;
        }
        else if(sysVarName == "LED_STATE"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_ledState", false);
            return true;
        }
        else if(sysVarName == "SOUND_TIMER"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_soundTimer", false);
            return true;
        }
        else if(sysVarName == "CHANNEL_MASK"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_channelMask", false);
            return true;
        }
        else if(sysVarName == "XOUT_MASK"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_xoutMask", false);
            return true;
        }
        else if(sysVarName == "BUTTON_STATE"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_buttonState", false);
            return true;
        }
        else if(sysVarName == "FRAME_COUNT"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_frameCount", false);
            return true;
        }

        usageSET(codeLine, codeLineStart);
        return false;
    }

    bool ASM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        // If ASM is attached to a label, propagate it to the generated vCPU code, (if sub/proc/func already has a PUSH, then label is already valid, so ignore)
        if(codeLine._labelIndex > -1  &&  !Compiler::getLabels()[codeLine._labelIndex]._gosub)
        {
            Compiler::setNextInternalLabel("_" + Compiler::getLabels()[codeLine._labelIndex]._name);
        }

        Compiler::setCodeIsAsm(true);

        return true;
    }

    bool ENDASM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);

        Compiler::setCodeIsAsm(false);

        return true;
    }

    bool BCDADD(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDADD() : Syntax error, use 'BCDADD <src bcd address>, <dst bcd address>, <length>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, srcToken, srcParam);
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, dstToken, dstParam);
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        Compiler::parseExpression(codeLineIndex, lenToken, lenParam);
        Compiler::emitVcpuAsm("%BcdAdd", "", false);

        return true;
    }

    bool BCDSUB(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDSUB() : Syntax error, use 'BCDSUB <src bcd address>, <dst bcd address>, <length>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, srcToken, srcParam);
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, dstToken, dstParam);
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        Compiler::parseExpression(codeLineIndex, lenToken, lenParam);
        Compiler::emitVcpuAsm("%BcdSub", "", false);

        return true;
    }

    bool BCDINT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::BCDINT() : Syntax error, use 'BCDINT <dst bcd address>, <int>' bcd value MUST contain at least 5 digits; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // BCD dst address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, srcToken, srcParam);
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // Integer value, must be +ve, max value 42767, (32767 + 10000 because of how vASM sub Numeric::bcdInt works)
        std::string intToken = tokens[1];
        Expression::Numeric intParam;
        Compiler::parseExpression(codeLineIndex, intToken, intParam);
        Compiler::emitVcpuAsm("%BcdInt", "", false);

        return true;
    }

    bool BCDCPY(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDCPY() : Syntax error, use 'BCDCPY <src bcd address>, <dst bcd address>, <length>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, srcToken, srcParam);
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        Compiler::parseExpression(codeLineIndex, dstToken, dstParam);
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        Compiler::parseExpression(codeLineIndex, lenToken, lenParam);
        Compiler::emitVcpuAsm("%BcdCpy", "", false);

        return true;
    }

    bool GPRINTF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Produces vCPU code and allocates Gigatron memory only when compiling on the emulator
#ifdef STAND_ALONE
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLineStart);
        UNREFERENCED_PARAM(codeLine);
#else
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Allocate memory for gprintf variable's addresses if it hasn't been allocated already
        uint16_t varsAddr = 0x0000;
        if(Compiler::getGprintfVarsAddr() == 0x0000)
        {
            if(!Memory::getFreeRAM(Memory::FitDescending, GPRINT_VAR_ADDRS*2, USER_CODE_START, Compiler::getRuntimeStart(), varsAddr))
            {
                fprintf(stderr, "Keywords::GPRINTF() : Not enough RAM for variables LUT of size %d, in '%s' on line %d\n", GPRINT_VAR_ADDRS*2, codeLine._text.c_str(), codeLineStart);
                return false;
            }
            Compiler::setGprintfVarsAddr(varsAddr);
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',');
        if(tokens.size() < 2)
        {
            fprintf(stderr, "Keywords::GPRINTF() : Syntax error, use 'GPRINTF \"<format string>\", <var1>, ... <varN>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }
        if(tokens.size() > GPRINT_VAR_ADDRS)
        {
            fprintf(stderr, "Keywords::GPRINTF() : Maximum number of vars is '%d', found '%d' vars, in '%s' on line %d\n", GPRINT_VAR_ADDRS, int(tokens.size()), codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Format string
        std::string formatStr = tokens[0];
        Expression::stripNonStringWhitespace(formatStr);
        if(formatStr[0] != '\"'  ||  formatStr.back() != '\"')
        {
            fprintf(stderr, "Keywords::GPRINTF() : Syntax error in string format, use 'GPRINTF \"<format string>\", <var1>, ... <varN>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineStart);
            return false;
        }

        // Vars
        Expression::Numeric numeric;
        std::vector<std::string> variables;
        for(int i=1; i<int(tokens.size()); i++)
        {
            uint16_t address = varsAddr + uint16_t((i-1)*2);
            if(i == 1)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);
                Compiler::emitVcpuAsm("STW", "memDstAddr", false);
            }
            else
            {
                Compiler::emitVcpuAsm("INC", "memDstAddr", false);
                Compiler::emitVcpuAsm("INC", "memDstAddr", false);
            }

            // Convert GBAS format to ASM format
            variables.push_back("*" + Expression::wordToHexString(address));

            Expression::parse(tokens[i], codeLineIndex, numeric);
            if(numeric._varType == Expression::Number  ||  numeric._varType == Expression::IntVar16  ||  numeric._varType == Expression::StrVar)
            {
                Compiler::handleExpression(codeLineIndex, tokens[i], numeric);
            }

            // An extra doke as a dummy instruction so that gprintf is called at the correct address
            Compiler::emitVcpuAsm("DOKE", "memDstAddr", false);
            Compiler::emitVcpuAsm("DOKE", "memDstAddr", false);
        }

        return addGprintf(codeLine._code.substr(foundPos), formatStr, variables, Compiler::getGprintfVarsAddr(), codeLineIndex);
#endif

#ifdef STAND_ALONE
        return true;
#endif
    }
}