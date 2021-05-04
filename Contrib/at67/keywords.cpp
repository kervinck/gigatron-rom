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
#include "midi.h"


#define FONT_WIDTH   6
#define FONT_HEIGHT  8
#define MAPPING_SIZE 96


namespace Keywords
{
    enum LoadUsage {LoadType=0, LoadWave, LoadMidi, LoadImage, LoadSprite, LoadFont};
    enum InitTypes {InitTime, InitMidi, InitMidiV, InitUser};
    enum MidiTypes {MidiNone, Midi, MidiV, MidiId, MidiIdV};

    struct Gprintf
    {
        int codeLineIndex = 0;
        Assembler::Gprintf _gprintfAsm;
    };


    bool _constDimStrArray = false;
    int _numNumericGotosGosubs = 0;
    MidiTypes _midiType = MidiNone;
    std::string _userRoutine;

    std::map<std::string, Keyword> _keywords;
    std::map<std::string, std::string> _equalsKeywords;

    std::map<std::string, Keyword>& getKeywords(void)           {return _keywords;      }
    std::map<std::string, std::string>& getEqualsKeywords(void) {return _equalsKeywords;}

    std::vector<Gprintf> _gprintfs;


    void reset(void)
    {
        _midiType = MidiNone;
        _userRoutine = "";
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
        _keywords["TCLIP"   ] = {"TCLIP",    TCLIP,   Compiler::SingleStatementParsed};
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
        _keywords["EXEC"    ] = {"EXEC",     EXEC,    Compiler::SingleStatementParsed};
        _keywords["OPEN"    ] = {"OPEN",     OPEN,    Compiler::SingleStatementParsed};

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
            // Line index taking into account modules
            int codeLineStart = Compiler::getCodeLineStart(codeLineIndex);

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
                fprintf(stderr, "Keywords::fixGprintfAddresses() : '%s:%d' : Assembler::addGprintf() at '0x%04x' already exists : %s\n", codeLine._moduleName.c_str(), codeLineIndex, address, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::INC() : '%s:%d' : syntax error, integer variable '%s' not found : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::DEC() : '%s:%d' : syntax error, integer variable '%s' not found : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::ON() : '%s:%d' : syntax error : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        size_t gSize = (gotoOffset != std::string::npos) ? 4 : 5;
        size_t gOffset = (gotoOffset != std::string::npos) ? gotoOffset : gosubOffset;

        // Parse ON field
        Expression::Numeric onValue;
        std::string onToken = codeLine._code.substr(foundPos, gOffset - (foundPos + 1));
        if(Compiler::parseExpression(codeLineIndex, onToken, onValue) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::ON() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, onToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "register0", false);

        // Parse labels
        std::vector<size_t> gOffsets;
        std::vector<std::string> gTokens = Expression::tokeniseOffsets(codeLine._code.substr(gOffset + gSize), ',', gOffsets, false);
        if(gTokens.size() < 1)
        {
            fprintf(stderr, "Keywords::ON() : '%s:%d' : syntax error, must have at least one label after GOTO/GOSUB : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::ON() : '%s:%d' : invalid label %s in slot %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, gLabel.c_str(), i, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::ON() : '%s:%d' : not enough RAM for onGotoGosub LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, size, codeLine._text.c_str());
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
        std::vector<std::string> gotoTokens = Expression::tokeniseOffsets(codeLine._code.substr(foundPos), ',', gotoOffsets, false);
        if(gotoTokens.size() < 1  ||  gotoTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::GOTO() : '%s:%d' : syntax error, must have one or two parameters, e.g. 'GOTO 200' or 'GOTO k+1,default' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::GOTO() : '%s:%d' : numeric label '%s' does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, gotoToken.c_str(), codeLine._text.c_str());
                return false;
            }
            if(++_numNumericGotosGosubs > Compiler::getNumNumericLabels())
            {
                fprintf(stderr, "Keywords::GOTO() : '%s:%d' : numeric label '%s' does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, gotoToken.c_str(), codeLine._text.c_str());
                return false;
            }

            Compiler::setCreateNumericLabelLut(true);
            if(Compiler::parseExpression(codeLineIndex, gotoToken, gotoValue) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::GOTO() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, gotoToken.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "numericLabel", false);

            // Default label exists
            if(gotoTokens.size() == 2)
            {
                std::string defaultToken = gotoTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::GOTO() : '%s:%d' : default label does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        std::vector<std::string> gosubTokens = Expression::tokeniseOffsets(codeLine._code.substr(foundPos), ',', gosubOffsets, false);
        if(gosubTokens.size() < 1  ||  gosubTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : syntax error, must have one or two parameters, e.g. 'GOSUB <label>' or 'GOSUB <expression>, <default label>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        if(gosubTokens[0].size() == 0)
        {
            fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : syntax error, invalid label : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : numeric label '%s' does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, gosubToken.c_str(), codeLine._text.c_str());
                return false;
            }
            if(++_numNumericGotosGosubs > Compiler::getNumNumericLabels())
            {
                fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : numeric label '%s' does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, gosubToken.c_str(), codeLine._text.c_str());
                return false;
            }
            if(!usePush)
            {
                fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : can't use optimised GOSUB with numeric labels : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return false;
            }

            Compiler::setCreateNumericLabelLut(true);
            if(Compiler::parseExpression(codeLineIndex, gosubToken, gosubValue) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, gosubToken.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "numericLabel", false);

            // Default label exists
            if(gosubTokens.size() == 2)
            {
                std::string defaultToken = gosubTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::GOSUB() : '%s:%d' : default label does not exist : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() > 3)
        {
            fprintf(stderr, "Keywords::CLS() : '%s:%d' : syntax error, expected 'CLS INIT' or 'CLS <address>, <optional width>, <optional height>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                if(Compiler::parseExpression(codeLineIndex, tokens[0], param) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::CLS() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
                    return false;
                }
                if(param._varType == Expression::Number  &&  uint16_t(std::lround(param._value)) < DEFAULT_EXEC_ADDRESS)
                {
                    fprintf(stderr, "Keywords::CLS() : '%s:%d' : address field must be above &h%04x, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), codeLine._text.c_str());
                    return false;
                }
                Compiler::emitVcpuAsm("STW", "clsAddress", false);
                Compiler::emitVcpuAsm("%ClearScreen", "",  false);
            }
        }
        else if(tokens.size() > 1)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[0], param) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::CLS() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "clrAddress", false);
            if(Compiler::parseExpression(codeLineIndex, tokens[1], param) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::CLS() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "clrWidth", false); // runtime uses clrWidth in arithmetic, so make sure all of it is valid

            if(tokens.size() == 2)
            {
                Compiler::emitVcpuAsm("LDI", "120", false);
            }
            else
            {
                if(Compiler::parseExpression(codeLineIndex, tokens[2], param) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::CLS() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[2].c_str(), codeLine._text.c_str());
                    return false;
                }
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

#if 1
            if((expressionType & Expression::HasStringKeywords)  &&  (expressionType & Expression::HasOptimisedPrint))
            {
                // Prints text on the fly without creating strings
                Expression::setEnableOptimisedPrint(true);
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    Expression::setEnableOptimisedPrint(false);
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
                Expression::setEnableOptimisedPrint(false);
            }
#else
            // TODO: Fix this, (checks for syntax errors)
            if((expressionType & Expression::HasStringKeywords))
            {
                if(expressionType & Expression::HasOptimisedPrint)
                {
                    // Prints text on the fly without creating strings
                    Expression::setEnableOptimisedPrint(true);
                    if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                    {
                        Expression::setEnableOptimisedPrint(false);
                        fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                        return false;
                    }
                    Expression::setEnableOptimisedPrint(false);
                }
                // Leading chars before a string function
                else
                {
                    //fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                    //return false;
                }
            }
#endif
            // Arrays are handled as functions
            else if(expressionType & Expression::HasFunctions)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
                        if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                        {
                            fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                            return false;
                        }
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                        Compiler::emitVcpuAsm("%PrintAcString", "", false);
                    }
                }
            }
            else if(expressionType & Expression::HasKeywords)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false);
            }
            else if(expressionType & Expression::HasOperators)
            {
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
            else if(expressionType & Expression::HasStrings)
            {
                size_t lquote = tokens[i].find_first_of("\"");
                size_t rquote = tokens[i].find_last_of("\"");
#if 1
                // TODO: Test this thoroughly
                if(lquote > 0)
                {
                    // If there are leading chars that are not whitespace, then syntax error
                    for(size_t j=0; j<lquote; j++)
                    {
                        if(!isspace(tokens[i][j]))
                        {
                            std::string error = tokens[i].substr(0, lquote);
                            fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, error.c_str(), codeLine._text.c_str());
                            return false;
                        }
                    }
                }
                if(rquote < tokens[i].size() - 1)
                {
                    // If there are trailing chars left over and they are not whitespace, then syntax error
                    for(size_t j=rquote+1; j<tokens[i].size(); j++)
                    {
                        if(!isspace(tokens[i][j]))
                        {
                            std::string error = tokens[i].substr(rquote + 1);
                            fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, error.c_str(), codeLine._text.c_str());
                            return false;
                        }
                    }
                }
#endif
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
                if(!Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::PRINT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
            fprintf(stderr, "Keywords::INPUT() : '%s:%d' : syntax error, use 'INPUT <heading>, <int/str var0>, <prompt0>, ... <int/str varN>, <promptN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::INPUT() : '%s:%d' : couldn't create integer var '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varTokens[i].c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::INPUT() : '%s:%d' : couldn't create string var '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varTokens[i].c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::INPUT() : '%s:%d' : syntax error in text size field of string '%s' of INPUT statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, str.c_str(), codeLine._text.c_str());
                    return false;
                }
                if(length > USER_STR_SIZE)
                {
                    fprintf(stderr, "Keywords::INPUT() : '%s:%d' : text size field > %d of string '%s' of INPUT statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, USER_STR_SIZE, str.c_str(), codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::INPUT() : '%s:%d' : not enough RAM for register work area of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, REG_WORK_SIZE, codeLine._text.c_str());
                return false;
            }
            Compiler::setRegWorkArea(regAddr);
        }

        // INPUT LUTs
        const int lutSize = 3;
        uint16_t lutAddr, varsAddr, strsAddr, typesAddr;
        if(!Memory::getFreeRAM(Memory::FitDescending, lutSize*2, USER_CODE_START, Compiler::getRuntimeStart(), lutAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : '%s:%d' : not enough RAM for INPUT LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, lutSize*2, codeLine._text.c_str());
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(varsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), varsAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : '%s:%d' : not enough RAM for INPUT Vars LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(varsLut.size()*2), codeLine._text.c_str());
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(strsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), strsAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : '%s:%d' : not enough RAM for INPUT Strings LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(strsLut.size()*2), codeLine._text.c_str());
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(typesLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), typesAddr))
        {
            fprintf(stderr, "Keywords::INPUT() : '%s:%d' : not enough RAM for INPUT Var Types LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(typesLut.size()*2), codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error, missing '=' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // TO uses INC/ADD, UPTO uses ADD, DOWNTO uses DEC/SUB; &TO/&UPTO/&DOWNTO are optimised BRA versions
        Compiler::ForNextType type = Compiler::AutoTo;
        type = (code.find("UPTO")   != std::string::npos) ? Compiler::UpTo   : type;
        type = (code.find("DOWNTO") != std::string::npos) ? Compiler::DownTo : type;
        bool farJump = (code.find("&TO") == std::string::npos)  &&  (code.find("&UPTO") == std::string::npos)  &&  (code.find("&DOWNTO") == std::string::npos);
        if((to = code.find("TO")) == std::string::npos)
        {
            fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error, missing 'TO' or 'DOWNTO' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        step = code.find("STEP");

        // Maximum of 4 nested loops
        if(Compiler::getForNextDataStack().size() == MAX_NESTED_LOOPS)
        {
            fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error, maximum nested loops is 4 : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        (varCounter < 0) ? Compiler::createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varCounter) : Compiler::updateIntVar(loopStart, codeLine, varCounter, false);

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
            if(!Expression::parse(startToken, codeLineIndex, startNumeric))
            {
                fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in '%s'\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression());
                return false;
            }
            loopStart = int16_t(std::lround(startNumeric._value));

            // Parse end
            if(!Expression::parse(endToken, codeLineIndex, endNumeric))
            {
                fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in '%s'\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression());
                return false;
            }
            loopEnd = int16_t(std::lround(endNumeric._value));

            // Parse step
            if(stepToken.size())
            {
                if(!Expression::parse(stepToken, codeLineIndex, stepNumeric))
                {
                    fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in '%s'\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression());
                    return false;
                }
                loopStep = int16_t(std::lround(stepNumeric._value));
                if(loopStep < 1  ||  loopStep > 255) optimise = false;
            }

            // Variable start
            if(optimise  &&  varStart  &&  endNumeric._isValid  &&  loopEnd >= 0  &&  loopEnd <= 255)
            {
                if(Compiler::parseExpression(codeLineIndex, startToken, startNumeric) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, startToken.c_str(), codeLine._text.c_str());
                    return false;
                }
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
            if(Compiler::parseExpression(codeLineIndex, startToken, startNumeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, startToken.c_str(), codeLine._text.c_str());
                return false;
            }
            loopStart = int16_t(std::lround(startNumeric._value));
            Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false);

            // Parse end
            if(Compiler::parseExpression(codeLineIndex, endToken, endNumeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, endToken.c_str(), codeLine._text.c_str());
                return false;
            }
            loopEnd = int16_t(std::lround(endNumeric._value));
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false);

            // Parse step
            if(stepToken.size())
            {
                if(Compiler::parseExpression(codeLineIndex, stepToken, stepNumeric) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::FOR() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, stepToken.c_str(), codeLine._text.c_str());
                    return false;
                }
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
            fprintf(stderr, "Keywords::NEXT() : '%s:%d' : syntax error, wrong number of tokens : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = Compiler::findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::NEXT() : '%s:%d' : syntax error, bad var : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Pop stack for this nested loop
        if(Compiler::getForNextDataStack().empty())
        {
            fprintf(stderr, "Keywords::NEXT() : '%s:%d' : syntax error, missing FOR statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        Compiler::ForNextData forNextData = Compiler::getForNextDataStack().top();
        Compiler::getForNextDataStack().pop();

        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Keywords::NEXT() : '%s:%d' : syntax error, wrong var : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        if(Compiler::parseExpression(codeLineIndex, conditionToken, condition) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::IF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, conditionToken.c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::IF() : '%s:%d' : syntax error, missing action in 'IF THEN <action>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::ELSEIF() : '%s:%d' : syntax error, missing IF statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::ELSEIF() : '%s:%d' : syntax error, ELSEIF follows IF or ELSEIF : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        if(Compiler::parseExpression(codeLineIndex, conditionToken, condition) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::ELSEIF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, conditionToken.c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::ELSE() : '%s:%d' : syntax error, wrong number of tokens : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::ELSE() : '%s:%d' : syntax error, missing IF statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::ENDIF() : '%s:%d' : syntax error, wrong number of tokens : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::ENDIF() : '%s:%d' : syntax error, missing IF statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        if(Compiler::parseExpression(codeLineIndex, conditionToken, condition) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::WHILE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, conditionToken.c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::WEND() : '%s:%d' : syntax error, missing WHILE statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::UNTIL() : '%s:%d' : syntax error, missing REPEAT statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
        Compiler::getRepeatUntilDataStack().pop();

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        if(Compiler::parseExpression(codeLineIndex, conditionToken, condition) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::UNTIL() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, conditionToken.c_str(), codeLine._text.c_str());
            return false;
        }

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
            fprintf(stderr, "Keywords::FOREVER() : '%s:%d' : syntax error, missing REPEAT statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        size_t dimPos = token.find("DIM ");
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
                fprintf(stderr, "Keywords::TYPE() : '%s:%d' : syntax error, 'TYPE' var array does not have the correct number of dimensions : %s'\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::TYPE() : '%s:%d' : syntax error, 'TYPE' requires the following format 'TYPE <NAME> = <varType1>, ... <varTypeN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Type name
        std::string typeName = tokens[0];
        Expression::stripWhitespace(typeName);
        std::map<std::string, Compiler::TypeData>& typeDatas = Compiler::getTypeDatas();
        if(typeDatas.find(typeName) != typeDatas.end())
        {
            fprintf(stderr, "Keywords::TYPE() : '%s:%d' : type '%s' already exists : %s\n", codeLine._moduleName.c_str(), codeLineStart, typeName.c_str(), codeLine._text.c_str());
            return false;
        }

        // Variables
        size_t equals = codeLine._code.find_first_of('=');
        std::string vars = codeLine._code.substr(equals + 1);
        tokens = Expression::tokenise(vars, ',', true);
        if(tokens.size() == 0)
        {
            fprintf(stderr, "Keywords::TYPE() : '%s:%d' : missing variable types, 'TYPE' requires the following format 'TYPE <NAME> = <varType1>, ... <varTypeN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::TYPE() : '%s:%d' : var '%s' already exists : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }

            typeData._vars[varName] = {varAddr, varType};
        }

        typeDatas[typeName] = typeData;

        //// Constant string array
        //std::string token = tokens[0];
        //Expression::strToUpper(token);
        //size_t dimPos = token.find("DIM ");
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

    bool callHelper(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, std::string& token, uint16_t localVarsAddr)
    {
        UNREFERENCED_PARAM(codeLine);

        // If valid expression
        Expression::Numeric numeric;
        if(!Expression::parse(token, codeLineIndex, numeric))
        {
            fprintf(stderr, "Keywords::CALL() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
            return false;
        }

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
            fprintf(stderr, "Keywords::CALL() : '%s:%d' : syntax error, 'CALL' requires a 'NAME' and optional parameters, 'CALL <NAME>, <param0, param1, ... paramN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::CALL() : '%s:%d' : syntax error, maximum number of parameters exceeded : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return false;
                }

                if(!callHelper(codeLine, codeLineIndex, codeLineStart, callTokens[i], localVarsAddr))
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
            fprintf(stderr, "Keywords::PROC() : '%s:%d' : syntax error, 'PROC' requires a 'NAME' and optional parameters, 'PROC <NAME>, <param0, param1, ... paramN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(!Compiler::getProcDataStack().empty())
        {
            fprintf(stderr, "Keywords::PROC() : '%s:%d' : syntax error, 'PROC' can NOT be nested : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::PROC() : '%s:%d' : syntax error, maximum number of parameters exceeded : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return false;
                }

                Expression::stripWhitespace(procTokens[i]);
                if(Expression::isVarNameValid(procTokens[i]) == Expression::Invalid)
                {
                    fprintf(stderr, "Keywords::PROC() : '%s:%d' : syntax error, parameter types can only be integer : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return false;
                }

                int localVarIndex = -1;
                std::string localVarName = procData._name + "_" + procTokens[i];
                Compiler::createProcIntVar(localVarName, 0, 0, codeLine, codeLineIndex, false, localVarsAddr, localVarIndex);
                if(localVarIndex == -1)
                {
                    fprintf(stderr, "Keywords::PROC() : '%s:%d' : can't create local integer var '%s'\n", codeLine._moduleName.c_str(), codeLineStart, localVarName.c_str());
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
            fprintf(stderr, "Keywords::ENDPROC() : '%s:%d' : syntax error, missing PROC statement : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(Compiler::getProcDataStack().size() != 1)
        {
            fprintf(stderr, "Keywords::ENDPROC() : '%s:%d' : syntax error, 'PROC' can NOT be nested : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : syntax error, 'LOCAL' requires at least one '<VAR>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(Compiler::getProcDataStack().empty())
        {
            fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : syntax error, 'LOCAL' can only be used within a 'PROC/ENDPROC' pair : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(Compiler::getProcDataStack().size() != 1)
        {
            fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : syntax error, 'LOCAL' can NOT be used in nested 'PROC's' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : error, maximum number of local vars exceeded : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return false;
            }

            Expression::stripWhitespace(localTokens[i]);
            if(Expression::isVarNameValid(localTokens[i]) == Expression::Invalid)
            {
                fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : syntax error, local var types can only be integer : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return false;
            }

            int localVarIndex = -1;
            std::string localVarName = procData._name + "_" + localTokens[i];
            Compiler::createProcIntVar(localVarName, 0, 0, codeLine, codeLineIndex, false, localVarsAddr, localVarIndex);
            if(localVarIndex == -1)
            {
                fprintf(stderr, "Keywords::LOCAL() : '%s:%d' : couldn't create local integer var '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, localVarName.c_str(), codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, use CONST a=50 or CONST a$=\"doggy\" or const dim arr$(2) = \"One\", \"Two\", \"Three\" : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Constant string array, (only string arrays can be constant, byte and integer arrays are excluded)
        std::string token = tokens[0];
        Expression::strToUpper(token);
        size_t dimPos = token.find("DIM ");
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
            fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, name MUST contain only alphanumerics and '$' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, invalid string or keyword : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return false;
                }

                std::string funcToken = tokens[1].substr(0, lbra);
                std::string paramToken = tokens[1].substr(lbra + 1, rbra - (lbra + 1));
                Expression::strToUpper(funcToken);
                if(Functions::getStringFunctions().find(funcToken) == Functions::getStringFunctions().end())
                {
                    fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, invalid string or keyword : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return false;
                }

                int16_t param;
                if(!Expression::stringToI16(paramToken, param))
                {
                    fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, keyword param must be a constant number : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            if(!Expression::parse(tokens[1], codeLineIndex, numeric))
            {
                fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                return false;
            }
            if(tokens[1].size() == 0  ||  !numeric._isValid  ||  numeric._varType == Expression::TmpVar  ||  numeric._varType == Expression::IntVar16)
            {
                fprintf(stderr, "Keywords::CONST() : '%s:%d' : syntax error, invalid constant expression : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return false;
            }

            Compiler::getConstants().push_back({2, int16_t(std::lround(numeric._value)), 0x0000, "", tokens[0], "_" + tokens[0], Compiler::ConstInt16});
        }

        return true;
    }

    bool initDIM(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, std::string& varName, int arrSizeTotal, int16_t& intInit, std::vector<int16_t>& intInits, bool& isInit)
    {
        std::string constName = varName;
        if(Compiler::findConst(constName) >= 0)
        {
            fprintf(stderr, "Keywords::initDIM() : '%s:%d' : const '%s' already exists : %s\n", codeLine._moduleName.c_str(), codeLineStart, varName.c_str(), codeLine._text.c_str());
            return false;
        }
        if(Compiler::findVar(varName) >= 0)
        {
            fprintf(stderr, "Keywords::initDIM() : '%s:%d' : var '%s' already exists : %s\n", codeLine._moduleName.c_str(), codeLineStart, varName.c_str(), codeLine._text.c_str());
            return false;
        }

        // Optional array int init values
        isInit = false;
        size_t equalsPos = codeLine._code.find("=");
        if(equalsPos != std::string::npos)
        {
            std::string initText = codeLine._code.substr(equalsPos + 1);
            Expression::stripWhitespace(initText);
            std::vector<std::string> initTokens = Expression::tokenise(initText, ',', true);
            if(initTokens.size() == 0)
            {
                fprintf(stderr, "Keywords::initDIM() : '%s:%d' : initial value must be a constant, found '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, initText.c_str(), codeLine._text.c_str());
                return false;
            }
            else if(initTokens.size() == 1)
            {
                std::string operand;
                Expression::Numeric numeric(true); // true = allow static init
                if(Compiler::parseStaticExpression(codeLineIndex, initTokens[0], operand, numeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::initDIM() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, initTokens[0].c_str(), codeLine._text.c_str());
                    return false;
                }
                intInit = int16_t(std::lround(numeric._value));
            }
            else if(int(initTokens.size()) > arrSizeTotal)
            {
                fprintf(stderr, "Keywords::initDIM() : '%s:%d' : too many initialisation values for size of array, found %d for a size of %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(initTokens.size()), arrSizeTotal, codeLine._text.c_str());
                return false;
            }

            // Multiple initialisation values, (if there are less init values than array size, then array is padded with last init value)
            std::string operand;
            intInits.resize(initTokens.size());
            std::vector<Expression::Numeric> funcParams(initTokens.size(), Expression::Numeric(true)); // true = allow static init
            for(int i=0; i<int(initTokens.size()); i++)
            {
                if(Compiler::parseStaticExpression(codeLineIndex, initTokens[i], operand, funcParams[i]) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::initDIM() : '%s:%d' : bad initialiser %s at index %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), i, codeLine._text.c_str());
                    return false;
                }
                intInits[i] = int16_t(std::lround(funcParams[i]._value));
            }
            intInit = intInits.back();

            isInit = true;
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

            default: fprintf(stderr, "Keywords::allocDIM() : '%s:%d' : unknown var type : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::allocDIM() : '%s:%d' : not enough RAM for int array of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, iSizeBytes, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::allocDIM() : '%s:%d' : not enough RAM for int array of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, jSizeBytes, codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::allocDIM() : '%s:%d' : not enough RAM for int array of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, jSizeBytes, codeLine._text.c_str());
                    return false;
                }
            }

            int kSizeBytes = arrSizes[2] * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, kSizeBytes, USER_CODE_START, Compiler::getArraysStart(), address, false)) // arrays do not need to be contained within pages
            {
                fprintf(stderr, "Keywords::allocDIM() : '%s:%d' : not enough RAM for int array of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, kSizeBytes, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::DIM() : '%s:%d' : syntax error in DIM statement, must be DIM <var>(<n1>, <optional n2>) : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Dimensions
        std::vector<uint16_t> arrSizes;
        std::vector<std::string> sizeTokens = Expression::tokenise(codeLine._code.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(sizeTokens.size() > MAX_ARRAY_DIMS)
        {
            fprintf(stderr, "Keywords::DIM() : '%s:%d' : maximum of %d dimensions, found %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, MAX_ARRAY_DIMS, int(sizeTokens.size()), codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, sizeToken, operand, numeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DIM() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, sizeToken.c_str(), codeLine._text.c_str());
                    return false;
                }
                int16_t data = int16_t(std::lround(numeric._value));
                arrSizes.push_back(data);
            }
            // Literal dimension
            else
            {
                uint16_t arrSize = 0;
                if(!Expression::stringToU16(sizeToken, arrSize)  ||  arrSize <= 0)
                {
                    fprintf(stderr, "Keywords::DIM() : '%s:%d' : array dimensions must be a positive constant, found '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, sizeToken.c_str(), codeLine._text.c_str());
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

        int16_t intInit = 0;
        bool isIntInit = false;
        std::vector<int16_t> intInits;
        std::vector<std::string> strInits;
        Compiler::VarType varType = Compiler::Var1Arr16;

        // Str array
        bool isStrInit = false;
        if(Expression::isStrNameValid(varName))
        {
            if(Compiler::findStr(varName) >= 0)
            {
                fprintf(stderr, "Keywords::DIM() : '%s:%d' : str %s already exists : %s\n", codeLine._moduleName.c_str(), codeLineStart, varName.c_str(), codeLine._text.c_str());
                return false;
            }
            if(arrSizes.size() != 1)
            {
                fprintf(stderr, "Keywords::DIM() : '%s:%d' : require 1 string dimension, found %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(arrSizes.size()), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::DIM() : '%s:%d' : initial value must be a string, found '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, initText.c_str(), codeLine._text.c_str());
                    return false;
                }
                else if(int(initTokens.size()) > arrSizes[0])
                {
                    fprintf(stderr, "Keywords::DIM() : '%s:%d' : too many initialisation strings for size of array, found %d for a size of %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(initTokens.size()), arrSizes[0], codeLine._text.c_str());
                    return false;
                }

                // Multiple initialisation values, (if there are less strings than array size, then array is padded with last string)
                for(int i=0; i<int(initTokens.size()); i++)
                {
                    if(initTokens[i].size() - 2 > USER_STR_SIZE)
                    {
                        fprintf(stderr, "Keywords::DIM() : '%s:%d' : initialisation string '%s' is larger than %d chars : %s\n", codeLine._moduleName.c_str(), codeLineStart, initTokens[i].c_str(), USER_STR_SIZE, codeLine._text.c_str());
                        return false;
                    }

                    if(!Expression::isStringValid(initTokens[i]))
                    {
                        fprintf(stderr, "Keywords::DIM() : '%s:%d' : invalid string initialiser, found '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, initTokens[i].c_str(), codeLine._text.c_str());
                        return false;
                    }

                    // Strip quotes
                    initTokens[i].erase(0, 1);
                    initTokens[i].erase(initTokens[i].size() - 1, 1);
                    strInits.push_back(initTokens[i]);
                }

                isStrInit = true;
            }

            varType = Compiler::VarStr2;
        }
        // Int8 array
        else if(varName.back() == '%')
        {
            // % is only used within DIM keyword
            varName.erase(varName.size()-1, 1);
            if(!initDIM(codeLine, codeLineIndex, codeLineStart, varName, arrSizeTotal, intInit, intInits, isIntInit)) return false;
            varType = Compiler::Var1Arr8;
        }
        // Int16 array
        else
        {
            if(!initDIM(codeLine, codeLineIndex, codeLineStart, varName, arrSizeTotal, intInit, intInits, isIntInit)) return false;
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
                    if(Compiler::createStringArray(codeLine, codeLineIndex, varName, 0, isStrInit, strInits, strAddrs) == -1) return false;
                }
                else
                {
                    // Variable array of strings
                    if(Compiler::createStringArray(codeLine, codeLineIndex, varName, USER_STR_SIZE, isStrInit, strInits, strAddrs) == -1) return false;
                }
            }
            break;

            // Int8 arrays, (allocDIM returns var type based on arrSizes)
            case Compiler::Var1Arr8:
            {
                int intIndex = -1;
                if(!allocDIM(codeLine, codeLineIndex, codeLineStart, address, varType, arrLut, arrSizes, arrAddrs)) return false;
                Compiler::createArrIntVar(varName, 0, intInit, codeLine, codeLineIndex, false, isIntInit, intIndex, varType, Compiler::Int8, address, arrSizes, intInits, arrAddrs, arrLut);
            }
            break;

            // Int16 arrays, (allocDIM returns var type based on arrSizes)
            case Compiler::Var1Arr16:
            {
                int intIndex = -1;
                if(!allocDIM(codeLine, codeLineIndex, codeLineStart, address, varType, arrLut, arrSizes, arrAddrs)) return false;
                Compiler::createArrIntVar(varName, 0, intInit, codeLine, codeLineIndex, false, isIntInit, intIndex, varType, Compiler::Int16, address, arrSizes, intInits, arrAddrs, arrLut);
            }
            break;

            default: fprintf(stderr, "Keywords::DIM() : '%s:%d' : unknown array type %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, varType, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::FUNC() : '%s:%d' : syntax error, invalid parenthesis in FN : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        funcText = funcText.substr(0, rbra + 1);

        // Name
        std::string name = funcText.substr(sizeof("FUNC")-1, lbra - (sizeof("FUNC")-1));
        Expression::stripWhitespace(name);
        if(Compiler::getDefFunctions().find(name) == Compiler::getDefFunctions().end())
        {
            fprintf(stderr, "Keywords::FUNC() : '%s:%d' : syntax error, FN %s can't be found : %s\n", codeLine._moduleName.c_str(), codeLineStart, name.c_str(), codeLine._text.c_str());
            return false;
        }
        int varIndex = Compiler::findVar(name);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Keywords::FUNC() : '%s:%d' : syntax error, name collision with var %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, name.c_str(), codeLine._text.c_str());
            return false;
        }

        // Params
        std::vector<std::string> params = Expression::tokenise(funcText.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(params.size() == 0)
        {
            fprintf(stderr, "Keywords::FUNC() : '%s:%d' : syntax error, need at least one parameter : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        int paramsSize = int(Compiler::getDefFunctions()[name]._params.size());
        if(paramsSize != int(params.size()))
        {
            fprintf(stderr, "Keywords::FUNC() : '%s:%d' : syntax error, wrong number of parameters, expecting %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, paramsSize, codeLine._text.c_str());
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
        std::vector<std::string> tokens = Expression::tokeniseLineOffsets(Compiler::getCodeLines()[codeLineIndex]._code, " (),=", offsets);
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
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, invalid parenthesis : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Name
        if(lbra == 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, missing name : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        std::string name = defFunc.substr(0, lbra);
        Expression::stripWhitespace(name);
        Expression::strToUpper(name);
        int varIndex = Compiler::findVar(name);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, name collision with var %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, name.c_str(), codeLine._text.c_str());
            return false;
        }

        // Function
        size_t equalsPos = defFunc.find("=", rbra + 1);
        if(equalsPos == std::string::npos)
        {
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, missing equals sign : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        std::string function = defFunc.substr(equalsPos + 1);
        Expression::trimWhitespace(function);

        // Params
        std::vector<std::string> params = Expression::tokenise(defFunc.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        if(params.size() == 0)
        {
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, need at least one parameter : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        for(int i=0; i<int(params.size()); i++)
        {
            Expression::stripWhitespace(params[i]);
            if(function.find(params[i]) == std::string::npos)
            {
                fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, parameter %s missing from function %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, params[i].c_str(), function.c_str(), codeLine._text.c_str());
                return false;
            }
        }


        Compiler::DefFunction defFunction = {name, function, params};
        if(Compiler::getDefFunctions().find(name) != Compiler::getDefFunctions().end())
        {
            fprintf(stderr, "Keywords::functionDEF() : '%s:%d' : syntax error, DEF FN %s has been defined more than once : %s\n", codeLine._moduleName.c_str(), codeLineStart, name.c_str(), codeLine._text.c_str());
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
                        fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in variable definition '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varTokens[i].c_str(), codeLine._text.c_str());
                        return false;
                    }

                    // Check for duplicate vars
                    if(varMap.find(varTokens[i]) != varMap.end())
                    {
                        fprintf(stderr, "Keywords::DEF() : '%s:%d' : duplicate variable definition '%s : %s'\n", codeLine._moduleName.c_str(), codeLineStart, varTokens[i].c_str(), codeLine._text.c_str());
                        return false;
                    }

                    // Create var, (no vASM code, i.e. uninitialised)
                    varMap[varTokens[i]] = i;
                    int varIndex = -1;
                    Compiler::createIntVar(varTokens[i], 0, 0, codeLine, codeLineIndex, false, varIndex);
                }

                return true;
            }

            fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error, missing equals sign : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : address field does not exist, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrText.c_str(), codeLine._text.c_str());
                return false;
            }
            if(Compiler::parseStaticExpression(codeLineIndex, addrTokens[0], operand, addrNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrTokens[0].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::DEF() : '%s:%d' : type field must be either BYTE or WORD, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, typeText.c_str(), codeLine._text.c_str());
            return false;
        }

        // Address offset field
        uint16_t addrOffset = 0;
        if(addrTokens.size() == 2  &&  !foundLutGenerator)
        {
            Expression::Numeric offsetNumeric(true);  // true = allow static init
            if(Compiler::parseStaticExpression(codeLineIndex, addrTokens[1], operand, offsetNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrTokens[1].c_str(), codeLine._text.c_str());
                return false;
            }
            addrOffset = uint16_t(std::lround(offsetNumeric._value));
        }
        
        // ************************************************************************************************************
        // LUT generator
        if(foundLutGenerator)
        {
            if(addrTokens.size() < 4  ||  addrTokens.size() > 6)
            {
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : LUT generator must have 4 to 6 parameters, '(ADDR, <VAR>, START, STOP, SIZE, <OFFSET>)', (<VAR> and <OFFSET> are optional), found %d : %s\n",
                        codeLine._moduleName.c_str(), codeLineStart, int(addrTokens.size()), codeLine._text.c_str());
                return false;
            }

            for(int i=0; i<int(addrTokens.size()); i++) Expression::stripWhitespace(addrTokens[i]);

            std::string lutGenerator = codeLine._code.substr(equalsPos + 1);
            Expression::stripWhitespace(lutGenerator);
            if(lutGenerator.size() == 0)
            {
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : LUT generator '%s' is invalid : %s\n", codeLine._moduleName.c_str(), codeLineStart, lutGenerator.c_str(), codeLine._text.c_str());
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
                                fprintf(stderr, "Keywords::DEF() : '%s:%d' : LUT generator variable '%s' invalid : %s\n", codeLine._moduleName.c_str(), codeLineStart, lutGenVar.c_str(), codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, addrTokens[i + paramsOffset], operand, lutGenParams[i]) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrTokens[i + paramsOffset].c_str(), codeLine._text.c_str());
                    return false;
                }
            }
            if(lutGenParams[2]._value <= 0.0)
            {
                fprintf(stderr, "Keywords::DEF() : '%s:%d' : LUT size must be greater than zero, found '%d' : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(lutGenParams[2]._value), codeLine._text.c_str());
                return false;
            }

            // Parse LUT generator address offset, if it exists
            if(addrTokens.size() == 6  ||  (addrTokens.size() == 5  &&  !foundVar))
            {
                Expression::Numeric offsetNumeric;
                if(Compiler::parseStaticExpression(codeLineIndex, addrTokens[3 + paramsOffset], operand, offsetNumeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrTokens[3 + paramsOffset].c_str(), codeLine._text.c_str());
                    return false;
                }
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
                if(Compiler::parseStaticExpression(codeLineIndex, lutGenerator, operand, lutGenResult) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, lutGenerator.c_str(), codeLine._text.c_str());
                    return false;
                }
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
            fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error, require at least one data field : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, dataTokens[i], operand, numeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dataTokens[i].c_str(), codeLine._text.c_str());
                    return false;
                }
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
                if(address >= DEFAULT_EXEC_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataBytes.size())))
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : memory error, byte chunk allocation at '0x%04x of size '%d' failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, address, int(dataBytes.size()), codeLine._text.c_str());
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
                            fprintf(stderr, "Keywords::DEF() : '%s:%d' : memory error, byte allocation at '0x%04x of size '1' failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, address, codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, dataTokens[i], operand, numeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dataTokens[i].c_str(), codeLine._text.c_str());
                    return false;
                }
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
                if(address >= DEFAULT_EXEC_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataWords.size()) * 2))
                {
                    fprintf(stderr, "Keywords::DEF() : '%s:%d' : memory error, word chunk allocation at '0x%04x of size '%d' failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, address, int(dataWords.size()) * 2, codeLine._text.c_str());
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
                            fprintf(stderr, "Keywords::DEF() : '%s:%d' : memory error, word allocation at '0x%04x of size '2' failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, address, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::DATA() : '%s:%d' : syntax error, require at least one data field : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, dataTokens[i], operand, numeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::DATA() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dataTokens[i].c_str(), codeLine._text.c_str());
                    return false;
                }
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
            fprintf(stderr, "Keywords::READ() : '%s:%d' : syntax error, use 'READ <var0, var1, var2...varN>', requires at least one variable : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                        fprintf(stderr, "Keywords::READ() : '%s:%d' : syntax error, only single dimensional arrays are supported in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
                        return false;
                    }
                    break;
                }
            }

            // Int array must have already been dimensioned
            if(varIndex == -1  &&  varType == Compiler::Var1Arr16)
            {
                fprintf(stderr, "Keywords::READ() : '%s:%d' : integer array is not dimensioned in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
                return false;
            }

            // Str array must have already been dimensioned
            if(strIndex == -1  &&  varType == Compiler::VarStr2)
            {
                fprintf(stderr, "Keywords::READ() : '%s:%d' : string array is not dimensioned in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
                return false;
            }

            // Create int var
            if(varIndex == -1  &&  varType == Compiler::VarInt16)
            {
                Compiler::createIntVar(varToken, 0, 0, codeLine, codeLineIndex, false, varIndex);
                if(varIndex == -1)
                {
                    fprintf(stderr, "Keywords::READ() : '%s:%d' : couldn't create integer var '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::READ() : '%s:%d' : couldn't create string var '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::READ() : '%s:%d' : integer var '%s' is invalid : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::READ() : '%s:%d' : string var '%s' is invalid : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
                    return false;
                }
            }
            else
            {
                fprintf(stderr, "Keywords::READ() : '%s:%d' : var '%s' is invalid : %s\n", codeLine._moduleName.c_str(), codeLineStart, varToken.c_str(), codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::RESTORE() : '%s:%d' : syntax error, use 'RESTORE <optional index>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Compiler::emitVcpuAsm("LDWI", "_dataIndex_", false);
        Compiler::emitVcpuAsm("STW", "memAddr",      false);
        
        if(tokens.size() == 1)
        {
            Expression::Numeric numeric;
            if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::RESTORE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : syntax error, use 'ALLOC <address>, <optional size>, <optional count>, <optional offset=0x0100>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        int count = 1;
        uint16_t address, end, size = 0x0000, offset = 0x0100;
        std::string addrOperand, sizeOperand, countOperand, offsetOperand;
        Expression::Numeric addrNumeric(true), sizeNumeric(true), countNumeric(true), offsetNumeric(true);
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        if(tokens.size() >= 2)
        {
            if(Compiler::parseStaticExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
                return false;
            }
            size = uint16_t(std::lround(sizeNumeric._value));
        }
        if(tokens.size() >= 3)
        {
            if(Compiler::parseStaticExpression(codeLineIndex, tokens[2], countOperand, countNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[2].c_str(), codeLine._text.c_str());
                return false;
            }
            count = std::lround(countNumeric._value);
        }
        if(tokens.size() >= 4)
        {
            if(Compiler::parseStaticExpression(codeLineIndex, tokens[3], offsetOperand, offsetNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[3].c_str(), codeLine._text.c_str());
                return false;
            }
            offset = uint16_t(std::lround(offsetNumeric._value));
            if(count == 0  ||  offset == 0)
            {
                fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : count and offset must both be non zero, found %d and 0x%04x : %s\n", codeLine._moduleName.c_str(), codeLineStart, count, offset, codeLine._text.c_str());
                return false;
            }
        }

        address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_EXEC_ADDRESS)
        {
            fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : address field must be above &h%04x, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, DEFAULT_EXEC_ADDRESS, tokens[0].c_str(), codeLine._text.c_str());
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
                    fprintf(stderr, "Keywords::ALLOC() : '%s:%d' : trying to allocate already allocated memory at 0x%04x : %s\n", codeLine._moduleName.c_str(), codeLineStart, j, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::FREE() : '%s:%d' : syntax error, use 'FREE <address>, <size> or FREE STRINGWORKAREA' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Free string work area
        if(tokens.size() == 1)
        {
            std::string token = Expression::strUpper(tokens[0]);
            Expression::stripWhitespace(token);
            if(token != "STRINGWORKAREA")
            {
                fprintf(stderr, "Keywords::FREE() : '%s:%d' : syntax error, expecting 'STRINGWORKAREA', found '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, token.c_str(), codeLine._text.c_str());
                return false;
            }

            for(int i=0; i<NUM_STR_WORK_AREAS; i++)
            {
                if(!Memory::giveFreeRAM(Compiler::getStrWorkArea(i), USER_STR_SIZE + 2))
                {
                    fprintf(stderr, "Keywords::FREE() : '%s:%d' : RAM at '0x%04x' is already free : %s\n", codeLine._moduleName.c_str(), codeLineStart, Compiler::getStrWorkArea(i), codeLine._text.c_str());
                    return false;
                }
            }

            return true;
        }

        Expression::Numeric addrNumeric(true), sizeNumeric(true);  // true = allow static init
        std::string addrOperand, sizeOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Keywords::FREE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        if(Compiler::parseStaticExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Keywords::FREE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
            return false;
        }
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        uint16_t size = uint16_t(std::lround(sizeNumeric._value));

        //Memory::printFreeRamList(Memory::NoSort);
        if(!Memory::giveFreeRAM(address, size))
        {
            fprintf(stderr, "Keywords::FREE() : '%s:%d' : RAM at '0x%04x' is already free : %s\n", codeLine._moduleName.c_str(), codeLineStart, address, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::AT() : '%s:%d' : syntax error, use 'AT <x>' or 'AT <x>, <y>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Expression::Numeric numeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::AT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("ST", "cursorXY", false);

        if(tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[1], numeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::AT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::PUT() : '%s:%d' : syntax error, use 'PUT <ascii>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Expression::Numeric numeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::PUT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("%PrintAcChr", "", false);

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
            fprintf(stderr, "Keywords::MODE() : '%s:%d' : version error, 'MODE' requires ROMv2 or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::MODE() : '%s:%d' : syntax error, use 'MODE <0 - 3>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Expression::Numeric numeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::MODE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::WAIT() : '%s:%d' : syntax error, use 'WAIT <optional vblank count>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(tokens.size() == 0)
        {
            Compiler::emitVcpuAsm("%WaitVBlank", "", false);
            return true;
        }

        Expression::Numeric numeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::WAIT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::PSET() : '%s:%d' : syntax error, use 'PSET <x>, <y>' or 'PSET <x>, <y>, <colour>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Expression::Numeric numeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::PSET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("ST", "drawPixel_xy", false);
        if(Compiler::parseExpression(codeLineIndex, tokens[1], numeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::PSET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("ST", "drawPixel_xy + 1", false);

        if(tokens.size() == 3)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[2], numeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::PSET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[2].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::LINE() : '%s:%d' : syntax error, use 'LINE <x>, <y>' or 'LINE <x1>, <y1>, <x2>, <y2>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(tokens.size() == 2)
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<int(tokens.size()); i++)
            {
                if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::LINE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                    return false;
                }
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
                if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::LINE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                    return false;
                }
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
            fprintf(stderr, "Keywords::HLINE() : '%s:%d' : syntax error, use 'HLINE <x1>, <y>, <x2>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::HLINE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::VLINE() : '%s:%d' : syntax error, use 'VLINE <x>, <y1>, <y2>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::VLINE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::CIRCLE() : '%s:%d' : syntax error, use 'CIRCLE <x>, <y>, <radius>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::CIRCLE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::CIRCLEF() : '%s:%d' : syntax error, use 'CIRCLEF <x>, <y>, <radius>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::CIRCLEF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::RECT() : '%s:%d' : syntax error, use 'RECT <x1>, <y1>, <x2>, <y2>'' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::RECT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::RECTF() : '%s:%d' : syntax error, use 'RECTF <x1>, <y1>, <x2>, <y2>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Compiler::parseExpression(codeLineIndex, tokens[i], params[i]) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::RECTF() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::POLY() : '%s:%d' : syntax error, use 'POLY <coords address>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Expression::Numeric param;
        if(Compiler::parseExpression(codeLineIndex, tokens[0], param) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::POLY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
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
            fprintf(stderr, "Keywords::POLYR() : '%s:%d' : syntax error, use 'POLYR <coords address> <optional FLIPX/FLIPY/FLIPXY>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                fprintf(stderr, "Keywords::POLYR() : '%s:%d' : syntax error, use one of the correct flip types, 'POLY <coords address> <optional FLIPX/FLIPY/FLIPXY>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
        if(Compiler::parseExpression(codeLineIndex, tokens[0], param) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::POLYR() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[0].c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "drawPoly_addr", false);
        Compiler::emitVcpuAsm("%DrawPolyRel", "", false);

        return true;
    }

    bool TCLIP(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::TCLIP() : '%s:%d' : syntax error, use 'TCLIP ON' or 'TCLIP OFF' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::string tclipToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(tclipToken);
        if(tclipToken != "ON"  &&  tclipToken != "OFF")
        {
            fprintf(stderr, "Keywords::TCLIP() : '%s:%d' : syntax error, use 'TCLIP ON' or 'TCLIP OFF' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        if(tclipToken == "ON")
        {
            Compiler::emitVcpuAsm("LDWI", "0xFFFB",    false);
            Compiler::emitVcpuAsm("ANDW", "miscFlags", false);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "0x0004",   false);
            Compiler::emitVcpuAsm("ORW", "miscFlags", false);
        }
        Compiler::emitVcpuAsm("STW", "miscFlags", false);

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
            fprintf(stderr, "Keywords::SCROLL() : '%s:%d' : syntax error, use 'SCROLL ON' or 'SCROLL OFF' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::string scrollToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(scrollToken);
        if(scrollToken != "ON"  &&  scrollToken != "OFF")
        {
            fprintf(stderr, "Keywords::SCROLL() : '%s:%d' : syntax error, use 'SCROLL ON' or 'SCROLL OFF'' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::POKE() : '%s:%d' : syntax error, use 'POKE <address>, <value>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseStaticExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
            if(operandTypes[i] == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::POKE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
            fprintf(stderr, "Keywords::DOKE() : '%s:%d' : syntax error, use 'DOKE <address>, <value>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseStaticExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
            if(operandTypes[i] == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::DOKE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[i].c_str(), codeLine._text.c_str());
                return false;
            }
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
        _midiType = Midi;
    }
    void midivInit(void)
    {
        _midiType = MidiV;
    }
    void userInit(const std::string& label)
    {
        _userRoutine = label;
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
        Compiler::emitVcpuAsm("LDWI", "playMidiVol",                          false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void userAddr(const std::string& label, int index)
    {
        Compiler::emitVcpuAsm("LDWI", "_" + label,                            false);
        Compiler::emitVcpuAsm("STW",  "realTimeProc" + std::to_string(index), false);
    }
    void usageINIT(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::INIT() : '%s:%d' : syntax error, use 'INIT VARS, <optional var address>, TIME, MIDI/MIDIV, <user proc>, NOUPDATE : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
    }
    bool INIT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        static std::map<std::string, InitTypes> initTypesMap = {{"TIME", InitTime}, {"MIDI", InitMidi}, {"MIDIV", InitMidiV}};

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
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
                fprintf(stderr, "Keywords::INIT() : '%s:%d' : syntax error, 'NOUPDATE' must be used with 'INIT TIME, MIDI/MIDIV, <user proc>' and only on ROMv4 or lower : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return false;
            }
        }

        // Search for init types and labels
        bool noUpdate = false;
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
                    if(_midiType != MidiNone)
                    {
                        fprintf(stderr, "Keywords::INIT() : '%s:%d' : syntax error, can only init one instance of MIDI or MIDIV, use 'INIT TIME, MIDI/MIDIV, <user proc> : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                        return false;
                    }
                }
                initTypes.push_back(initTypesMap[token]);
            }
        }

        // Proc and addr init
        for(int i=0; i<int(initTypes.size()); i++)
        {
            switch(initTypes[i])
            {
                case InitTime:  timeInit();           timeAddr(i);            break;
                case InitMidi:  midiInit();           midiAddr(i);            break;
                case InitMidiV: midivInit();          midivAddr(i);           break;
                case InitUser:  userInit(tokens[i]);  userAddr(tokens[i], i); break;

                default: break;
            }
        }

        if(resetVars)
        {
            if(varsAddr.size())
            {
                Expression::Numeric numeric(true); // true = allow static init
                if(!Expression::parse(varsAddr, codeLineIndex, numeric))
                {
                    fprintf(stderr, "Keywords::INIT() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                    return false;
                }
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
        fprintf(stderr, "Keywords::INIT() : '%s:%d' : syntax error, use one or more of, 'TICK TIME, MIDI, MIDIV, USER' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            fprintf(stderr, "Keywords::TICK() : '%s:%d' : version error, 'TICK' requires ROMv4 or lower, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 4)
        {
            usageTICK(codeLine, codeLineStart);
            return false;
        }

        std::map<std::string, InitTypes> initTypesMap;
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = Expression::strToUpper(tokens[i]);
            Expression::stripWhitespace(token);
            if(initTypesMap.find(token) == initTypesMap.end())
            {
                if(token == "TIME")
                {
                    if(!Compiler::getCreateTimeData())
                    {
                        fprintf(stderr, "Keywords::TICK() : '%s:%d' : TIME not initialised using INIT : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                        return false;
                    }

                    initTypesMap[token] = InitTime;
                    Compiler::emitVcpuAsm("%TickTime", "", false);
                }
                else if(token == "MIDI")
                {
                    if(_midiType != Midi)
                    {
                        fprintf(stderr, "Keywords::TICK() : '%s:%d' : MIDI not initialised using INIT : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                        return false;
                    }

                    initTypesMap[token] = InitMidi;
                    Compiler::emitVcpuAsm("%TickMidi", "", false);
                }
                else if(token == "MIDIV")
                {
                    if(_midiType != MidiV)
                    {
                        fprintf(stderr, "Keywords::TICK() : '%s:%d' : MIDIV not initialised using INIT : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                        return false;
                    }

                    initTypesMap[token] = InitMidiV;
                    Compiler::emitVcpuAsm("%TickMidiV", "", false);
                }
                else if(token == "USER")
                {
                    if(_userRoutine == "")
                    {
                        fprintf(stderr, "Keywords::TICK() : '%s:%d' : USER routine not initialised using INIT : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                        return false;
                    }

                    initTypesMap[token] = InitUser;
                    if(Compiler::getCodeRomType() < Cpu::ROMv5a)
                    {
                        Compiler::emitVcpuAsm("LDWI", "_" + _userRoutine, false);
                        Compiler::emitVcpuAsm("CALL", "giga_vAC",         false);
                    }
                    else
                    {
                        Compiler::emitVcpuAsm("CALLI", "_" + _userRoutine, false);
                    }
                }
                else
                {
                    usageTICK(codeLine, codeLineStart);
                    return false;
                }
            }
            else
            {
                usageTICK(codeLine, codeLineStart);
                return false;
            }
        }

        return true;
    }

    void usagePLAY(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::PLAY() : '%s:%d' : syntax error, use 'PLAY <TYPE>, <id/address>, <optional waveType>', where <TYPE> = 'MIDI', 'MIDID', 'MIDIV', 'MIDIDV' or 'MUSIC' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
    }
    bool PLAY(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            usagePLAY(codeLine, codeLineStart);
            return false;
        }

        // Default wave type
        if(tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDI", "2",           false);
            Compiler::emitVcpuAsm("ST", "waveType + 1", false);
        }
        // Midi wave type, (optional)
        else if(tokens.size() == 3)
        {
            std::string waveTypeToken = tokens[2];
            Expression::Numeric waveTypeNumeric;
            if(Compiler::parseExpression(codeLineIndex, waveTypeToken, waveTypeNumeric) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::PLAY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, waveTypeToken.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "waveType + 1", false);
        }

        // Midi stream address
        std::string addressToken = tokens[1];
        Expression::Numeric addressNumeric;
        if(Compiler::parseExpression(codeLineIndex, tokens[1], addressNumeric) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::PLAY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, tokens[1].c_str(), codeLine._text.c_str());
            return false;
        }

        std::string midiToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(midiToken);
        if(midiToken == "MIDI")
        {
            Compiler::emitVcpuAsm("%PlayMidi", "", false);
            return true;
        }

        if(midiToken == "MIDID")
        {
            Compiler::emitVcpuAsm("%SetMidiStream", "", false);
            Compiler::emitVcpuAsm("%PlayMidi", "",      false);
            return true;
        }
        
        if(midiToken == "MIDIV")
        {
            Compiler::emitVcpuAsm("%PlayMidiV", "", false);
            return true;
        }

        if(midiToken == "MIDIDV")
        {
            Compiler::emitVcpuAsm("%SetMidiStream", "", false);
            Compiler::emitVcpuAsm("%PlayMidiV", "",     false);
            return true;
        }

        if(midiToken == "MUSIC")
        {
            Compiler::emitVcpuAsm("%PlayMusic", "", false);
            return true;
        }

        usagePLAY(codeLine, codeLineStart);
        return false;
    }

    void loadUsage(int msgType, Compiler::CodeLine& codeLine, int codeLineStart)
    {
        switch(msgType)
        {
            case LoadType:   fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD <TYPE>, <filename>, where <TYPE> = 'IMAGE', 'SPRITE', 'FONT', 'MIDI', 'WAVE' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str()); break;
            case LoadWave:   fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD WAVE, <filename>, <optional address>, <optional address offset>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());             break;
            case LoadMidi:   fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD MIDI, <filename>, <id>, <optional loop count 1<->255, 0=forever>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());            break;
            case LoadImage:  fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD IMAGE, <filename>, <optional address>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());                                       break;
            case LoadSprite: fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD SPRITE, <filename>, <id>, <optional flip>, <optional overlap>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());               break;
            case LoadFont:   fprintf(stderr, "Keywords::LOAD() : '%s:%d' : syntax error, use 'LOAD FONT, <filename>, <id>, <optional 16 bit bg:fg colours>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());                     break;

            default: break;
        }
    }
    /********************************************************************************************************************************/
    /* Load Wave                                                                                                                    */
    /********************************************************************************************************************************/
    bool loadWave(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, const std::vector<std::string>& tokens)
    {
        if(tokens.size() < 2  ||  tokens.size() > 4)
        {
            loadUsage(LoadWave, codeLine, codeLineStart);
            return false;
        }

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
                if(Compiler::parseStaticExpression(codeLineIndex, addrToken, addrOperand, addrNumeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::loadWave() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrToken.c_str(), codeLine._text.c_str());
                    return false;
                }
                address = uint16_t(std::lround(addrNumeric._value));
                if(address < DEFAULT_EXEC_ADDRESS)
                {
                    loadUsage(LoadWave, codeLine, codeLineStart);
                    fprintf(stderr, "Keywords::loadWave() : '%s:%d' : address field must be above &h%04x, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, DEFAULT_EXEC_ADDRESS, addrToken.c_str(), codeLine._text.c_str());
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
                if(Compiler::parseStaticExpression(codeLineIndex, offsetToken, offsetOperand, offsetNumeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::loadWave() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, offsetToken.c_str(), codeLine._text.c_str());
                    return false;
                }
                addrOffset = uint16_t(std::lround(offsetNumeric._value));
            }

            // Load wave file
            std::vector<uint8_t> dataBytes(64);
            std::ifstream infile(filename, std::ios::binary | std::ios::in);
            if(!infile.is_open())
            {
                loadUsage(LoadWave, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadWave() : '%s:%d' : failed to open file '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, filename.c_str(), codeLine._text.c_str());
                return false;
            }
            infile.read((char *)&dataBytes[0], 64);
            if(infile.eof() || infile.bad() || infile.fail())
            {
                loadUsage(LoadWave, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadWave() : '%s:%d' : failed to read file '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, filename.c_str(), codeLine._text.c_str());
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

        return true;
    }
    /********************************************************************************************************************************/
    /* Load Midi                                                                                                                    */
    /********************************************************************************************************************************/
    bool loadMidi(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, const std::vector<std::string>& tokens)
    {
        if(tokens.size() < 3  ||  tokens.size() > 4)
        {
            loadUsage(LoadMidi, codeLine, codeLineStart);
            return false;
        }

        std::string filename = tokens[1];
        Expression::stripWhitespace(filename);
        std::string ext = filename;
        Expression::strToUpper(ext);
        if(ext.find(".GTMID") != std::string::npos)
        {
            std::string filepath = Loader::getFilePath();
            size_t slash = filepath.find_last_of("\\/");
            filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
            filename = filepath + "/" + filename;

            // Unique midi ID
            std::string idToken = tokens[2];
            Expression::Numeric idNumeric;
            std::string idOperand;
            if(Compiler::parseStaticExpression(codeLineIndex, idToken, idOperand, idNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::loadMidi() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, idToken.c_str(), codeLine._text.c_str());
                return false;
            }
            int midiId = int(std::lround(idNumeric._value));
            if(Compiler::getDefDataMidis().find(midiId) != Compiler::getDefDataMidis().end())
            {
                loadUsage(LoadMidi, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadMidi() : '%s:%d' : MIDI id %d not unique : %s\n", codeLine._moduleName.c_str(), codeLineStart, midiId, codeLine._text.c_str());
                return false;
            }

            // Parse optional loop count
            uint16_t loops = 0;
            if(tokens.size() == 4)
            {
                std::string loopsToken = tokens[2];
                Expression::Numeric loopsNumeric;
                std::string loopsOperand;
                if(Compiler::parseStaticExpression(codeLineIndex, loopsToken, loopsOperand, loopsNumeric) == Compiler::OperandInvalid)
                {
                    fprintf(stderr, "Keywords::loadMidi() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, loopsToken.c_str(), codeLine._text.c_str());
                    return false;
                }
                loops = uint16_t(std::lround(loopsNumeric._value));
                if(loops > 255)
                {
                    loadUsage(LoadMidi, codeLine, codeLineStart);
                    fprintf(stderr, "Keywords::loadMidi() : '%s:%d' : loops field must be between 0 and 255, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, loopsToken.c_str(), codeLine._text.c_str());
                    return false;
                }
            }

            // Load gtMID file
            int midiSize = 0;
            GtMidiHdr gtMidiHdr;
            std::vector<uint8_t> midiBuffer(MIDI_MAX_BUFFER_SIZE);
            if(!Midi::loadFile(filename, midiBuffer.data(), midiSize, &gtMidiHdr))
            {
                loadUsage(LoadMidi, codeLine, codeLineStart);
                return false;
            }
            midiBuffer.resize(midiSize);

            // Allocate memory for midi segments
            int byteIndex = 0;
            int segmentCount = 0;
            std::vector<uint16_t> segSizes;
            std::vector<uint16_t> segAddrs;
            bool hasVolume = bool(gtMidiHdr._hasVolume);
            while(midiSize)
            {
                uint16_t size = 0;
                uint16_t address = 0;
                if(!Memory::getFreeRAM(Memory::FitDescending, USER_CODE_START, Compiler::getRuntimeStart(), MIDI_MIN_SEGMENT_SIZE, address, uint16_t(midiSize + MIDI_CMD_JMP_SEG_SIZE), size, false))
                {
                    loadUsage(LoadMidi, codeLine, codeLineStart);
                    fprintf(stderr, "Keywords::loadMidi() : '%s:%d' : getting MIDI memory for segment %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, segmentCount, codeLine._text.c_str());
                    return false;
                }

                byteIndex += size;

                // Pad for jump segment command and adjust for start notes, start note can be either 3 bytes, (0x90, nn, vv) or 2 bytes, (0x90, nn), depending on hasVolume
                if(hasVolume  &&  (midiBuffer[byteIndex - 5] & 0xF0) == MIDI_CMD_START_NOTE) {byteIndex -= 2; size -= 2; Memory::giveFreeRAM(address + size-2, 2);} // landed on volume, (vv)
                else if((midiBuffer[byteIndex - 4] & 0xF0) == MIDI_CMD_START_NOTE)           {byteIndex -= 1; size -= 1; Memory::giveFreeRAM(address + size-1, 1);} // landed on note, (nn)
                                                                                             {byteIndex -= 3; size -= 3;                                          } // jump segment, (0xD0, lo, hi)
                midiSize -= size;
                segSizes.push_back(size);
                segAddrs.push_back(address);
                segmentCount++;
            }

            Compiler::getDefDataMidis()[midiId] = {midiId, hasVolume, uint8_t(loops), midiBuffer, segSizes, segAddrs};
        }

        return true;
    }
    /********************************************************************************************************************************/
    /* Load Image                                                                                                                   */
    /********************************************************************************************************************************/
    bool loadImageChunk(Compiler::CodeLine& codeLine, int codeLineStart, const std::vector<uint8_t>& data, int row, uint16_t width, uint16_t address, uint8_t chunkSize, uint16_t& chunkOffset, uint16_t& chunkAddr)
    {
        if(!Memory::getFreeRAM(Memory::FitDescending, chunkSize, USER_CODE_START, Compiler::getRuntimeStart(), chunkAddr))
        {
            loadUsage(LoadImage, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadImageChunk() : '%s:%d' : allocating RAM for offscreen pixel chunk on row %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, row, codeLine._text.c_str());
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
    bool loadImage(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, const std::vector<std::string>& tokens, const std::string& filename, const Image::TgaFile& tgaFile, const Image::GtRgbFile& gtRgbFile)
    {
        const uint16_t stride = 256;

        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            loadUsage(LoadImage, codeLine, codeLineStart);
            return false;
        }

        // Parse optional address
        uint16_t address = RAM_VIDEO_START;
        if(tokens.size() == 3)
        {
            std::string addrToken = tokens[2];
            Expression::Numeric addrNumeric;
            std::string addrOperand;
            if(Compiler::parseStaticExpression(codeLineIndex, addrToken, addrOperand, addrNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::loadImage() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, addrToken.c_str(), codeLine._text.c_str());
                return false;
            }
            address = uint16_t(std::lround(addrNumeric._value));
            if(address < DEFAULT_EXEC_ADDRESS)
            {
                loadUsage(LoadImage, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadImage() : '%s:%d' : address field must be above &h%04x, found %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, DEFAULT_EXEC_ADDRESS, addrToken.c_str(), codeLine._text.c_str());
                return false;
            }
        }

        if(gtRgbFile._header._width > stride  ||  gtRgbFile._header._width + (address & 0x00FF) > stride)
        {
            loadUsage(LoadImage, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadImage() : '%s:%d' : image width %d + starting address 0x%04x overflow, for %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, gtRgbFile._header._width, address, filename.c_str(), codeLine._text.c_str());
            return false;
        }

        Compiler::DefDataImage defDataImage = {address, tgaFile._header._width, tgaFile._header._height, stride, gtRgbFile._data};
        Compiler::getDefDataImages().push_back(defDataImage);

        int size = gtRgbFile._header._width;
        for(int y=0; y<gtRgbFile._header._height; y++)
        {
            // Take offscreen memory from compiler for images wider than visible screen resolution, or images in offscreen memory
            if(address >= RAM_VIDEO_START  &&  address <= RUN_TIME_START)
            {
                size = gtRgbFile._header._width + (address & 0x00FF) - RAM_SCANLINE_SIZE;
                if(size > 0)
                {
                    if(!Memory::takeFreeRAM((address & 0xFF00) + RAM_SCANLINE_SIZE, size))
                    {
                        loadUsage(LoadImage, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::loadImage() : '%s:%d' : allocating RAM for pixel row %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, y, codeLine._text.c_str());
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
                    loadImageChunk(codeLine, codeLineStart, gtRgbFile._data, y, gtRgbFile._header._width, address, RAM_SEGMENTS_SIZE, chunkOffset, chunkAddr);
                }
                loadImageChunk(codeLine, codeLineStart, gtRgbFile._data, y, gtRgbFile._header._width, address, gtRgbFile._header._width % RAM_SEGMENTS_SIZE, chunkOffset, chunkAddr);
            }

            // Next destination row
            address += stride; 
        }

        return true;
    }
    /********************************************************************************************************************************/
    /* Load Sprite                                                                                                                  */
    /********************************************************************************************************************************/
    bool loadSprite(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, const std::vector<std::string>& tokens, const std::string& filename, const Image::TgaFile& tgaFile, const Image::GtRgbFile& gtRgbFile)
    {
        if(Compiler::getCodeRomType() < Cpu::ROMv3)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            loadUsage(LoadSprite, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : version error, 'LOAD SPRITE' requires ROMv3 or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
            return false;
        }

        if(tokens.size() < 3  ||  tokens.size() > 5)
        {
            loadUsage(LoadSprite, codeLine, codeLineStart);
            return false;
        }

        if(gtRgbFile._header._width % SPRITE_CHUNK_SIZE != 0)
        {
            loadUsage(LoadSprite, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : sprite width not a multiple of %d, (%d x %d), for %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, SPRITE_CHUNK_SIZE, gtRgbFile._header._width, gtRgbFile._header._height, filename.c_str(), codeLine._text.c_str());
            return false;
        }

        if(tokens.size() < 3)
        {
            loadUsage(LoadSprite, codeLine, codeLineStart);
            return false;
        }

        // Unique sprite ID
        std::string idToken = tokens[2];
        Expression::Numeric idNumeric;
        std::string idOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, idToken, idOperand, idNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, idToken.c_str(), codeLine._text.c_str());
            return false;
        }
        int spriteId = int(std::lround(idNumeric._value));
        if(Compiler::getDefDataSprites().find(spriteId) != Compiler::getDefDataSprites().end())
        {
            loadUsage(LoadSprite, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : sprite id %d not unique : %s\n", codeLine._moduleName.c_str(), codeLineStart, spriteId, codeLine._text.c_str());
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
                loadUsage(LoadSprite, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : unknown sprite flip type, %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, flipToken.c_str(), codeLine._text.c_str());
                fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : must use one of 'NOFLIP', 'FLIPX', 'FLIPY', 'FLIPXY' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
            if(Compiler::parseStaticExpression(codeLineIndex, overlapToken, overlapOperand, overlapNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, overlapToken.c_str(), codeLine._text.c_str());
                return false;
            }
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
            loadUsage(LoadSprite, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : can't have a non zero overlap with a single column sprite : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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
                    if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                    {
                        loadUsage(LoadSprite, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : getting sprite memory for stripe %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(stripeAddrs.size()/2 + 1), codeLine._text.c_str());
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
                        if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                        {
                            loadUsage(LoadSprite, codeLine, codeLineStart);
                            fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : getting sprite memory failed for stripe %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(stripeAddrs.size()/2 + 1), codeLine._text.c_str());
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
                    if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), remStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                    {
                        loadUsage(LoadSprite, codeLine, codeLineStart);
                        fprintf(stderr, "Keywords::loadSprite() : '%s:%d' : getting sprite memory failed for stripe %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(stripeAddrs.size()/2 + 1), codeLine._text.c_str());
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

        Compiler::getDefDataSprites()[spriteId] = {spriteId, filename, tgaFile._header._width, tgaFile._header._height, numColumns, numStripesPerCol, numStripeChunks, remStripeChunks, stripeAddrs, spriteData, flipType, isInstanced};

        return true;
    }
    /********************************************************************************************************************************/
    /* Load font                                                                                                                    */
    /********************************************************************************************************************************/
    bool loadFont(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, const std::vector<std::string>& tokens, const std::string& filename, const Image::TgaFile& tgaFile, const Image::GtRgbFile& gtRgbFile)
    {
        if(Compiler::getCodeRomType() < Cpu::ROMv3)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : version error, 'LOAD FONT' requires ROMv3 or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
            return false;
        }

        if(tokens.size() < 3  ||  tokens.size() > 4)
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            return false;
        }

        // Unique font ID
        std::string idToken = tokens[2];
        Expression::Numeric idNumeric;
        std::string idOperand;
        if(Compiler::parseStaticExpression(codeLineIndex, idToken, idOperand, idNumeric) == Compiler::OperandInvalid)
        {
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, idToken.c_str(), codeLine._text.c_str());
            return false;
        }
        int fontId = int(std::lround(idNumeric._value));
        if(Compiler::getDefDataFonts().find(fontId) != Compiler::getDefDataFonts().end())
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : font id %d not unique : %s\n", codeLine._moduleName.c_str(), codeLineStart, fontId, codeLine._text.c_str());
            return false;
        }

        // Foreground/background colours
        uint16_t fgbgColour = 0x0000;
        if(tokens.size() == 4)
        {
            std::string fgbgToken = tokens[3];
            Expression::Numeric fgbgNumeric;
            std::string fgbgOperand;
            if(Compiler::parseStaticExpression(codeLineIndex, fgbgToken, fgbgOperand, fgbgNumeric) == Compiler::OperandInvalid)
            {
                fprintf(stderr, "Keywords::loadFont() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, fgbgToken.c_str(), codeLine._text.c_str());
                return false;
            }
            fgbgColour = uint16_t(std::lround(fgbgNumeric._value));
        }

        // Width
        if(gtRgbFile._header._width % FONT_WIDTH != 0)
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : font width %d is not a multiple of %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, gtRgbFile._header._width, FONT_WIDTH, codeLine._text.c_str());
            return false;
        }

        // Height
        if(gtRgbFile._header._height % FONT_HEIGHT != 0)
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : font height %d is not a multiple of %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, gtRgbFile._header._height, FONT_HEIGHT, codeLine._text.c_str());
            return false;
        }

        // Load font mapping file
        bool foundMapFile = true;
        size_t nameSuffix = filename.find_last_of(".");
        std::string filenameMap = filename.substr(0, nameSuffix) + ".map";
        std::ifstream infile(filenameMap, std::ios::in);
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
                    loadUsage(LoadFont, codeLine, codeLineStart);
                    fprintf(stderr, "Keywords::loadFont() : '%s:%d' : error in mapping file %s on line %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, filenameMap.c_str(), line + 1, codeLine._text.c_str());
                    return false;
                }

                if(line >= MAPPING_SIZE) break;
                mapping[line++] = uint8_t(index);
            }

            if(line != MAPPING_SIZE)
            {
                loadUsage(LoadFont, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadFont() : '%s:%d' : warning, found an incorrect number of map entries %d for file %s, should be %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, line - 1, filenameMap.c_str(), MAPPING_SIZE, codeLine._text.c_str());
                return false;
            }

            if(!Memory::getFreeRAM(Memory::FitDescending, MAPPING_SIZE, USER_CODE_START, Compiler::getRuntimeStart(), mapAddr))
            {
                loadUsage(LoadFont, codeLine, codeLineStart);
                fprintf(stderr, "Keywords::loadFont() : '%s:%d' : getting mapping memory for map size of %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, MAPPING_SIZE, codeLine._text.c_str());
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
                if(!Memory::getFreeRAM(Memory::FitDescending, (kCharHeight)*FONT_WIDTH + 1, USER_CODE_START, Compiler::getRuntimeStart(), address))
                {
                    loadUsage(LoadFont, codeLine, codeLineStart);
                    fprintf(stderr, "Keywords::loadFont() : '%s:%d' : getting font memory for char %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(fontData.size() - 1), codeLine._text.c_str());
                    return false;
                }

                charAddrs.push_back(address);
            }
        }

        if(foundMapFile  &&  maxIndex + 1 != int(fontData.size()))
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : font mapping table does not match font data, found a mapping count of %d and a chars count of %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, maxIndex + 1, int(fontData.size()), codeLine._text.c_str());
            return false;
        }

        // Create baseline for all chars in each font
        uint16_t baseAddr = 0x0000;
        if(!Memory::getFreeRAM(Memory::FitDescending, FONT_WIDTH + 1, USER_CODE_START, Compiler::getRuntimeStart(), baseAddr))
        {
            loadUsage(LoadFont, codeLine, codeLineStart);
            fprintf(stderr, "Keywords::loadFont() : '%s:%d' : getting font memory for char %d failed : %s\n", codeLine._moduleName.c_str(), codeLineStart, int(fontData.size() - 1), codeLine._text.c_str());
            return false;
        }

        Compiler::getDefDataFonts()[fontId] = {fontId, filenameMap, tgaFile._header._width, tgaFile._header._height, charAddrs, fontData, mapAddr, mapping, baseAddr, fgbgColour};

        Linker::enableFontLinking();

        return true;
    }
    bool LOAD(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 2  ||  tokens.size() > 5)
        {
            loadUsage(LoadType, codeLine, codeLineStart);
            return false;
        }

        // Type
        Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(tokens[0]);

        // Load WAVE
        if(tokens[0] == "WAVE")
        {
            return loadWave(codeLine, codeLineIndex, codeLineStart, tokens);
        }

        // Load Midi
        if(tokens[0] == "MIDI")
        {
            return loadMidi(codeLine, codeLineIndex, codeLineStart, tokens);
        }

        // Load Image, Sprite and Font
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
                    fprintf(stderr, "Keywords::LOAD() : '%s:%d' : file '%s' failed to load : %s\n", codeLine._moduleName.c_str(), codeLineStart, filename.c_str(), codeLine._text.c_str());
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
                    return loadImage(codeLine, codeLineIndex, codeLineStart, tokens, filename, tgaFile, gtRgbFile);
                }

                // Sprite
                if(tokens[0] == "SPRITE")
                {
                    return loadSprite(codeLine, codeLineIndex, codeLineStart, tokens, filename, tgaFile, gtRgbFile);
                }

                // Font
                if(tokens[0] == "FONT")
                {
                    return loadFont(codeLine, codeLineIndex, codeLineStart, tokens, filename, tgaFile, gtRgbFile);
                }
            }
        }

        loadUsage(LoadType, codeLine, codeLineStart);
        return false;
    }

    bool SPRITE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        if(Compiler::getCodeRomType() < Cpu::ROMv3)
        {
            std::string romTypeStr;
            getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : version error, 'SPRITE' requires ROMv3 or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
            return false;
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : syntax error, use 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x pos>, <y pos>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Flip type
        static std::map<std::string, Compiler::SpriteFlipType> flipType = {{"NOFLIP", Compiler::NoFlip}, {"FLIPX", Compiler::FlipX}, {"FLIPY", Compiler::FlipY}, {"FLIPXY", Compiler::FlipXY}};
        std::string flipToken = tokens[0];
        Expression::stripWhitespace(flipToken);
        Expression::strToUpper(flipToken);
        if(flipType.find(flipToken) == flipType.end())
        {
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : syntax error, use one of the correct flip types, 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x pos>, <y pos>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Sprite identifier
        std::string idToken = tokens[1];
        Expression::Numeric idParam;
        if(Compiler::parseExpression(codeLineIndex, idToken, idParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, idToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "spriteId", false);

        // Sprite X position
        std::string xposToken = tokens[2];
        Expression::Numeric xposParam;
        if(Compiler::parseExpression(codeLineIndex, xposToken, xposParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, xposToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("ST", "spriteXY", false);

        // Sprite Y position
        std::string yposToken = tokens[3];
        Expression::Numeric yposParam;
        if(Compiler::parseExpression(codeLineIndex, yposToken, yposParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::SPRITE() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, yposToken.c_str(), codeLine._text.c_str());
            return false;
        }
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
            case 0: fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error, use 'SOUND <TYPE>, <params>, where <TYPE> = 'ON', 'MOD' or 'OFF' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());               break;
            case 1: fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error, use 'SOUND ON, <channel>, <frequency>, <optional volume>, <optional waveform>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str()); break;
            case 2: fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error, use 'SOUND MOD, <channel>, <wavX>, <optional wavA>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());                            break;
            case 3: fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error, use 'SOUND OFF, <optional channel>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());                                            break;

            default: break;
        }
    }
    bool SOUND(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
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
            if(Compiler::parseExpression(codeLineIndex, chanToken, chanParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, chanToken.c_str(), codeLine._text.c_str());
                return false;
            }
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
            if(Compiler::parseExpression(codeLineIndex, freqToken, freqParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, freqToken.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "sndFrequency", false);

            if(tokens.size() == 3)
            {
                Compiler::emitVcpuAsm("%SoundOn", "", false);
                return true;
            }

            std::string volToken = tokens[3];
            Expression::Numeric volParam;
            if(Compiler::parseExpression(codeLineIndex, volToken, volParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, volToken.c_str(), codeLine._text.c_str());
                return false;
            }
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
            if(Compiler::parseExpression(codeLineIndex, wavToken, wavParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, wavToken.c_str(), codeLine._text.c_str());
                return false;
            }
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
            if(Compiler::parseExpression(codeLineIndex, waveXToken, waveXParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, waveXToken.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "sndWaveType + 1", false);

            if(tokens.size() == 4)
            {
                std::string waveAToken = tokens[3];
                Expression::Numeric waveAParam;
                if(Compiler::parseExpression(codeLineIndex, waveAToken, waveAParam) == Expression::IsInvalid)
                {
                    fprintf(stderr, "Keywords::SOUND() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, waveAToken.c_str(), codeLine._text.c_str());
                    return false;
                }
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
        fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error, use 'SET <VAR NAME>, <PARAM>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
    }
    bool SET(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
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
                fprintf(stderr, "Keywords::SET() : '%s:%d' : version error, 'SET FONTID' requires ROMv3 or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
                return false;
            }

            Compiler::emitVcpuAsm("LDWI", "_fontId_", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_MODE")
        {
            Compiler::emitVcpuAsm("LDWI", "handleT_mode + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_EPOCH")
        {
            Compiler::emitVcpuAsm("LDWI", "handleT_epoch + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_S")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 0", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_M")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 1", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIME_H")
        {
            Compiler::emitVcpuAsm("LDWI", "_timeArray_ + 2", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "TIMER")
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "timerTick", false);
            return true;
        }
        else if(sysVarName == "VBLANK_PROC")
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                std::string romTypeStr;
                getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                fprintf(stderr, "Keywords::SET() : '%s:%d' : version error, 'SET VBLANK_PROC' requires ROMv5a or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
                return false;
            }

            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(VBLANK_PROC), false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("DOKE", "register0", false);
            return true;
        }
        else if(sysVarName == "VBLANK_FREQ")
        {
            if(Compiler::getCodeRomType() < Cpu::ROMv5a)
            {
                std::string romTypeStr;
                getRomTypeStr(Compiler::getCodeRomType(), romTypeStr);
                fprintf(stderr, "Keywords::SET() : '%s:%d' : version error, 'SET VBLANK_FREQ' requires ROMv5a or higher, you are trying to link against '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, romTypeStr.c_str(), codeLine._text.c_str());
                return false;
            }

#if 0
            // TODO: Fix this
            // (256 - n) = vblank interrupt frequency, where n = 1 to 255
            Compiler::emitVcpuAsm("LDWI", "realTS_rti + 2", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "register1", false);
            Compiler::emitVcpuAsm("LDWI", "256", false);
            Compiler::emitVcpuAsm("SUBW", "register1", false);
            Compiler::emitVcpuAsm("POKE", "register0", false);
#endif
            return true;
        }
        else if(sysVarName == "CURSOR_X"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "cursorXY", false);
            return true;
        }
        else if(sysVarName == "CURSOR_Y"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "cursorXY + 1", false);
            return true;
        }
        else if(sysVarName == "CURSOR_XY"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "cursorXY", false);
            return true;
        }
        else if(sysVarName == "FG_COLOUR"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "fgbgColour + 1", false);
            return true;
        }
        else if(sysVarName == "BG_COLOUR"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "fgbgColour", false);
            return true;
        }
        else if(sysVarName == "FGBG_COLOUR"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "fgbgColour", false);
            return true;
        }
        else if(sysVarName == "MIDI_STREAM"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("STW", "midiStream", false);
            return true;
        }
        else if(sysVarName == "VIDEO_TOP"  &&  tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDWI", "giga_videoTop", false);
            Compiler::emitVcpuAsm("STW", "register0", false);
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("POKE", "register0", false);
            return true;
        }
        else if(sysVarName == "LED_TEMPO"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_ledTempo", false);
            return true;
        }
        else if(sysVarName == "LED_STATE"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_ledState", false);
            return true;
        }
        else if(sysVarName == "SOUND_TIMER"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_soundTimer", false);
            return true;
        }
        else if(sysVarName == "CHANNEL_MASK"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_channelMask", false);
            return true;
        }
        else if(sysVarName == "XOUT_MASK"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_xoutMask", false);
            return true;
        }
        else if(sysVarName == "BUTTON_STATE"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
            Compiler::emitVcpuAsm("ST", "giga_buttonState", false);
            return true;
        }
        else if(sysVarName == "FRAME_COUNT"  &&  tokens.size() == 2)
        {
            if(Compiler::parseExpression(codeLineIndex, token1, param1) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::SET() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, token1.c_str(), codeLine._text.c_str());
                return false;
            }
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

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDADD() : '%s:%d' : syntax error, use 'BCDADD <src bcd address>, <dst bcd address>, <length>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, srcToken, srcParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDADD() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, srcToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, dstToken, dstParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDADD() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dstToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        if(Compiler::parseExpression(codeLineIndex, lenToken, lenParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDADD() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, lenToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("%BcdAdd", "", false);

        return true;
    }

    bool BCDSUB(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDSUB() : '%s:%d' : syntax error, use 'BCDSUB <src bcd address>, <dst bcd address>, <length>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, srcToken, srcParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDSUB() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, srcToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, dstToken, dstParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDSUB() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dstToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        if(Compiler::parseExpression(codeLineIndex, lenToken, lenParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDSUB() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, lenToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("%BcdSub", "", false);

        return true;
    }

    bool BCDINT(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::BCDINT() : '%s:%d' : syntax error, use 'BCDINT <dst bcd address>, <int>' bcd value MUST contain at least 5 digits : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // BCD dst address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, srcToken, srcParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDINT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, srcToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // Integer value, must be +ve, max value 42767, (32767 + 10000 because of how vASM sub Numeric::bcdInt works)
        std::string intToken = tokens[1];
        Expression::Numeric intParam;
        if(Compiler::parseExpression(codeLineIndex, intToken, intParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDINT() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, intToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("%BcdInt", "", false);

        return true;
    }

    bool BCDCPY(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::BCDCPY() : '%s:%d' : syntax error, use 'BCDCPY <src bcd address>, <dst bcd address>, <length>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // BCD src address
        std::string srcToken = tokens[0];
        Expression::Numeric srcParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, srcToken, srcParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDCPY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, srcToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdSrcAddr", false);

        // BCD dst address
        std::string dstToken = tokens[1];
        Expression::Numeric dstParam(true); // true = allow static init
        if(Compiler::parseExpression(codeLineIndex, dstToken, dstParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDCPY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, dstToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "bcdDstAddr", false);

        // BCD length
        std::string lenToken = tokens[2];
        Expression::Numeric lenParam;
        if(Compiler::parseExpression(codeLineIndex, lenToken, lenParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::BCDCPY() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, lenToken.c_str(), codeLine._text.c_str());
            return false;
        }
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
                fprintf(stderr, "Keywords::GPRINTF() : '%s:%d' : not enough RAM for variables LUT of size %d : %s\n", codeLine._moduleName.c_str(), codeLineStart, GPRINT_VAR_ADDRS*2, codeLine._text.c_str());
                return false;
            }
            Compiler::setGprintfVarsAddr(varsAddr);
        }

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',');
        if(tokens.size() < 2)
        {
            fprintf(stderr, "Keywords::GPRINTF() : '%s:%d' : syntax error, use 'GPRINTF \"<format string>\", <var1>, ... <varN>' : %s\n", codeLine._text.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        if(tokens.size() > GPRINT_VAR_ADDRS)
        {
            fprintf(stderr, "Keywords::GPRINTF() : '%s:%d' : maximum number of vars is '%d', found '%d' vars : %s\n", codeLine._moduleName.c_str(), codeLineStart, GPRINT_VAR_ADDRS, int(tokens.size()), codeLine._text.c_str());
            return false;
        }

        // Format string
        std::string formatStr = tokens[0];
        Expression::stripNonStringWhitespace(formatStr);
        if(formatStr[0] != '\"'  ||  formatStr.back() != '\"')
        {
            fprintf(stderr, "Keywords::GPRINTF() : '%s:%d' : syntax error in string format, use 'GPRINTF \"<format string>\", <var1>, ... <varN>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
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

            if(!Expression::parse(tokens[i], codeLineIndex, numeric))
            {
                fprintf(stderr, "Keywords::GPRINTF() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression(), codeLine._text.c_str());
                return false;
            }
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

    bool EXEC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::EXEC() : '%s:%d' : syntax error, expected 'EXEC <rom address>, <optional ram address>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // ROM address to load from
        std::string romToken = tokens[0];
        Expression::Numeric romParam;
        if(Compiler::parseExpression(codeLineIndex, romToken, romParam) == Expression::IsInvalid)
        {
            fprintf(stderr, "Keywords::EXEC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, romToken.c_str(), codeLine._text.c_str());
            return false;
        }
        Compiler::emitVcpuAsm("STW", "giga_sysArg0", false);

        // RAM address to execute at
        if(tokens.size() == 2)
        {
            std::string ramToken = tokens[1];
            Expression::Numeric ramParam;
            if(Compiler::parseExpression(codeLineIndex, ramToken, ramParam) == Expression::IsInvalid)
            {
                fprintf(stderr, "Keywords::EXEC() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, ramToken.c_str(), codeLine._text.c_str());
                return false;
            }
        }
        else
        {
            // Default execute address
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(DEFAULT_EXEC_ADDRESS), false);
        }

        // SYS_Exec_88
        Compiler::emitVcpuAsm("%RomExec", "",  false);

        return true;
    }

    void openUsage(Compiler::CodeLine& codeLine, int codeLineStart)
    {
        fprintf(stderr, "Keywords::OPEN() : '%s:%d' : usage, 'OPEN #<id>, <path>, <file>, <mode>', where mode is one of 'r', 'w', 'a', 'r+', 'w+', 'a+' : %s\n", codeLine._moduleName.c_str(),
                                                                                                                                                                 codeLineStart, codeLine._text.c_str());
    }
    bool OPEN(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : syntax error, wrong number of parameters : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // file id
        std::string hashToken = tokens[0];
        Expression::stripWhitespace(hashToken);
        if(hashToken[0] != '#')
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : syntax error, missing '#' in '#<id>' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }
        Expression::Numeric idNumeric;
        std::string idOperand;
        std::string idToken = hashToken.substr(1);
        if(Compiler::parseStaticExpression(codeLineIndex, idToken, idOperand, idNumeric) == Compiler::OperandInvalid)
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : syntax error in %s : %s\n", codeLine._moduleName.c_str(), codeLineStart, idToken.c_str(), codeLine._text.c_str());
            return false;
        }
        int openId = int(std::lround(idNumeric._value));
        if(Compiler::getDefDataOpens().find(openId) != Compiler::getDefDataOpens().end())
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : #%d is not unique : %s\n", codeLine._moduleName.c_str(), codeLineStart, openId, codeLine._text.c_str());
            return false;
        }

        // File path
        std::string pathToken = tokens[1];
        Expression::stripWhitespace(pathToken);
        if(pathToken == "") pathToken = "/";

        // File name
        std::string fileToken = tokens[2];
        Expression::stripWhitespace(fileToken);
        if(fileToken == "")
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : syntax error, <file> is empty : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        // Open mode
        Compiler::DefDataOpen::OpenMode openMode;
        std::string modeToken = tokens[3];
        Expression::stripWhitespace(modeToken);
        Expression::strToUpper(modeToken);
        if(modeToken == "R")       openMode = Compiler::DefDataOpen::OpenRead;
        else if(modeToken == "W")  openMode = Compiler::DefDataOpen::OpenWrite;
        else if(modeToken == "A")  openMode = Compiler::DefDataOpen::OpenAppend;
        else if(modeToken == "R+") openMode = Compiler::DefDataOpen::OpenUpdateRW;
        else if(modeToken == "W+") openMode = Compiler::DefDataOpen::OpenCreateRW;
        else if(modeToken == "A+") openMode = Compiler::DefDataOpen::OpenAppendR;
        else
        {
            openUsage(codeLine, codeLineStart);
            fprintf(stderr, "Keywords::OPEN() : '%s:%d' : syntax error, <mode> is expecting one of 'r', 'w', 'a', 'r+', 'w+', 'a+', : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return false;
        }

        Compiler::getDefDataOpens()[openId] = {openId, pathToken, fileToken, openMode};

        return true;
    }
}