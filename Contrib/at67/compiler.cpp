#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stack>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "compiler.h"
#include "assembler.h"


#define LABEL_TRUNC_SIZE  20      // The smaller you make this, the more your BASIC label names will be truncated in the resultant .vasm code
#define OPCODE_TRUNC_SIZE 24      // The smaller you make this, the more your VASM opcode/macro names will be truncated in the resultant .vasm code
#define USER_STR_SIZE     95

// 18 bytes, (0x00EE <-> 0x00FF), reserved for vCPU stack, allows for 9 nested calls. The amount of GOSUBS you can use is dependant on how
// much of the stack is being used by nested system calls. *NOTE* there is NO call table for user code for this compiler
#define USER_VAR_START    0x0030  // 80 bytes, (0x0030 <-> 0x007F), reserved for BASIC user variables
#define VAC_SAVE_START    0x0082  // 2 bytes,  (0x0082 <-> 0x0083), reserved for saving vAC
#define INT_VAR_START     0x0084  // 44 bytes, (0x0084 <-> 0x00AF), internal register variables, used by the BASIC runtime
#define LOOP_VAR_START    0x00B0  // 16 bytes, (0x00B0 <-> 0x00BF), reserved for loops, maximum of 4 nested loops
#define TEMP_VAR_START    0x00C0  // 16 bytes, (0x00C0 <-> 0x00CF), reserved for temporary expression variables
#define CONVERT_CC_OPS    0x00D0  // 12 bytes, (0x00D0 <-> 0x00DB), critical relational operator routines that can't straddle page boundaries
#define REAL_TIME_PROC    0x00DC  // 2 bytes,  (0x00DC <-> 0x00DD), critical time sliced routine that usually handles MIDI/SPRITES, etc
#define USER_CODE_START   0x0200
#define USER_VAR_END      0x007F

//#define SMALL_CODE_SIZE
//#define ARRAY_INDICES_ONE


namespace Compiler
{
    enum VarType {VarInt8=0, VarInt16, VarInt32, VarFloat16, VarFloat32, VarArray};
    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated, VarExists};
    enum IntSize {Int8=1, Int16=2, Int32=4};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};
    enum KeywordResult {KeywordNotFound, KeywordError, KeywordFound};
    enum ConstantStrType {StrChar, StrHex, StrHexw};
    enum StatementResult {StatementError, StatementExpression, SingleStatementParsed, MultiStatementParsed};

    struct Label
    {
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        bool _isNumeric = false;
        bool _pageJump = false;
        bool _gosub = false;
    };

    struct InternalLabel
    {
        uint16_t _address;
        std::string _name;
    };

    struct VasmLine
    {
        uint16_t _address;
        std::string _opcode;
        std::string _code;
        std::string _internalLabel;
        bool _pageJump = false;
        int _vasmSize = 0;
    };

    struct OnGotoGosubLut
    {
        uint16_t _address;
        std::string _name;
        std::vector<int> _lut;
    };

    struct CodeLine
    {
        std::string _text;
        std::string _code;
        std::vector<std::string> _tokens;
        std::vector<VasmLine> _vasm;
        std::string _expression;
        OnGotoGosubLut _onGotoGosubLut;
        int _vasmSize = 0;
        int _labelIndex = -1;
        int  _varIndex = -1;
        bool _assignOperator = false;
        bool _containsVars = false;
        bool _pushEmitted = false;

        bool initialise(void)
        {
            _code.clear();
            _tokens.clear();
            _vasm.clear();
            _expression.clear();
            _vasmSize = 0;
            _labelIndex = -1;
            _varIndex = -1;
            _assignOperator = false;
            _containsVars = false;
        }
    };

    struct IntegerVar
    {
        int16_t _data;
        int16_t _init;
        uint16_t _address;
        uint16_t _array;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        VarType _varType = VarInt16;
        int _intSize = Int16;
        int _arrSize = 0;
    };

    struct StringVar
    {
        uint8_t _size;
        uint16_t _address;
        std::string _data;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
    };

    struct KeywordFuncResult
    {
        int16_t _data = 0;
        std::string _name;
    };

    using KeywordFuncPtr = bool (*)(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    struct Keyword
    {
        int _params;
        std::string _name;
        KeywordFuncPtr _func;
    };

    struct ForNextData
    {
        int _varIndex;
        std::string _labelName;
        int16_t _loopEnd;
        int16_t _loopStep;
        uint16_t _varEnd;
        uint16_t _varStep;
        bool _optimise = false;
    };

    struct MacroNameEntry
    {
        std::string _name;
        int _indexEnd;
        int _byteSize;
    };

    struct MacroIndexEntry
    {
        int _indexStart;
        int _indexEnd;
        int _byteSize;
    };

    struct InternalSub
    {
        uint16_t _address;
        uint16_t _size;
        std::string _name;
        std::string _includeName;
        bool _inUse = false;
        bool _loaded = false;
    };

    struct DefDataByte
    {
        uint16_t _address;
        std::vector<uint8_t> _data;
    };

    struct DefDataWord
    {
        uint16_t _address;
        std::vector<uint16_t> _data;
    };


    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart   = USER_VAR_START;
    uint16_t _runtimeEnd     = 0xFFFF;

    bool _nextTempVar = true;
    bool _createNumericLabelLut = false;

    int _currentLabelIndex = -1;
    int _currentCodeLineIndex = 0;

    std::string _tempVarStartStr;
    std::string _nextInternalLabel;

    std::map<std::string, Keyword> _keywords;
    std::vector<std::string> _keywordsWithEquals;

    std::vector<std::string> _input;
    std::vector<std::string> _output;
    std::vector<std::string> _runtime;

    std::vector<Label>         _labels;
    std::vector<std::string>   _gosubLabels;
    std::vector<std::string>   _equateLabels;
    std::vector<InternalLabel> _internalLabels;
    std::vector<InternalLabel> _discardedLabels;

    std::vector<CodeLine>   _codeLines;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar>  _stringVars;

    std::stack<ForNextData> _forNextDataStack;

    std::vector<std::string> _macroLines;
    std::map<int, MacroNameEntry> _macroNameEntries;
    std::map<std::string, MacroIndexEntry> _macroIndexEntries;


    std::vector<InternalSub> _internalSubs =
    {
        {0x0000, 0x0000, "realTimeProc"     , "", true,  false},
        {0x0000, 0x0000, "convertEqOp"      , "", true,  false},
        {0x0000, 0x0000, "convertNeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertLeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertGeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertLtOp"      , "", true,  false},
        {0x0000, 0x0000, "convertGtOp"      , "", true,  false},
        {0x0000, 0x0000, "multiply16bit"    , "", false, false}, 
        {0x0000, 0x0000, "divide16bit"      , "", false, false},
        {0x0000, 0x0000, "rand16bit"        , "", false, false},
        {0x0000, 0x0000, "randMod16bit"     , "", false, false},
        {0x0000, 0x0000, "shiftLeft4bit"    , "", false, false},
        {0x0000, 0x0000, "shiftLeft8bit"    , "", false, false},
        {0x0000, 0x0000, "shiftRight1bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight2bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight3bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight4bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight5bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight6bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight7bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight8bit"   , "", false, false},
        {0x0000, 0x0000, "getArrayByte"     , "", false, false},
        {0x0000, 0x0000, "setArrayByte"     , "", false, false},
        {0x0000, 0x0000, "getArrayInt16"    , "", false, false},
        {0x0000, 0x0000, "setArrayInt16"    , "", false, false},
        {0x0000, 0x0000, "gotoNumericLabel" , "", false, false},
        {0x0000, 0x0000, "gosubNumericLabel", "", false, false},
        {0x0000, 0x0000, "scanlineMode"     , "", false, false},
        {0x0000, 0x0000, "waitVBlank"       , "", false, false},
        {0x0000, 0x0000, "resetVideoTable"  , "", false, false},
        {0x0000, 0x0000, "initClearFuncs"   , "", false, false},
        {0x0000, 0x0000, "clearScreen"      , "", false, false},
        {0x0000, 0x0000, "clearVertBlinds"  , "", false, false},
        {0x0000, 0x0000, "clearRVertBlinds" , "", false, false},
        {0x0000, 0x0000, "clearCursorRow"   , "", false, false},
        {0x0000, 0x0000, "drawHLine"        , "", false, false},
        {0x0000, 0x0000, "drawVLine"        , "", false, false},
        {0x0000, 0x0000, "drawLine"         , "", false, false},
        {0x0000, 0x0000, "drawLineExt"      , "", false, false},
        {0x0000, 0x0000, "drawLineDelta1"   , "", false, false},
        {0x0000, 0x0000, "atLineCursor"     , "", false, false},
        {0x0000, 0x0000, "resetAudio"       , "", false, false},
        {0x0000, 0x0000, "playMidi"         , "", false, false},
        {0x0000, 0x0000, "midiStartNote"    , "", false, false},
        {0x0000, 0x0000, "printInit"        , "", false, false},
        {0x0000, 0x0000, "printText"        , "", false, false},
        {0x0000, 0x0000, "printDigit"       , "", false, false},
        {0x0000, 0x0000, "printInt16"       , "", false, false},
        {0x0000, 0x0000, "printChar"        , "", false, false},
        {0x0000, 0x0000, "printHexByte"     , "", false, false},
        {0x0000, 0x0000, "printHexWord"     , "", false, false},
        {0x0000, 0x0000, "atTextCursor"     , "", false, false},
        {0x0000, 0x0000, "newLineScroll"    , "", false, false},
    };
    const std::vector<std::string> _subIncludes = 
    {
        "include/math.i"        ,
        "include/memory.i"      ,
        "include/flow_control.i",
        "include/clear_screen.i",
        "include/conv_conds.i"  ,
        "include/graphics.i"    ,
        "include/print_text.i"  ,
        "include/audio.i"       ,
    };
    const std::vector<std::string> _subIncludesCALLI = 
    {
        "include/math_CALLI.i"        ,
        "include/memory_CALLI.i"      ,
        "include/flow_control_CALLI.i",
        "include/clear_screen_CALLI.i",
        "include/conv_conds_CALLI.i"  ,
        "include/graphics_CALLI.i"    ,
        "include/print_text_CALLI.i"  ,
        "include/audio_CALLI.i"       ,
    };

    std::vector<DefDataByte> _defDataBytes;
    std::vector<DefDataWord> _defDataWords;


    bool keywordREM(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLET(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordEND(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordON(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOTO(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOSUB(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCLS(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPRINT(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFOR(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordNEXT(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordIF(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordENDIF(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSEIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDIM(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDEF(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordINPUT(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordREAD(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDATA(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDO(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLOOP(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWHILE(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordUNTIL(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordAT(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPUT(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordMODE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWAIT(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLINE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordHLINE(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordVLINE(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSCROLL(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPOKE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDOKE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPLAY(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);

    bool initialise(void)
    {
        _keywords["END"   ] = {0, "END",    keywordEND   };
        _keywords["ON"    ] = {4, "ON",     keywordON    };
        _keywords["GOTO"  ] = {1, "GOTO",   keywordGOTO  };
        _keywords["GOSUB" ] = {1, "GOSUB",  keywordGOSUB };
        _keywords["RETURN"] = {0, "RETURN", keywordRETURN};
        _keywords["CLS"   ] = {0, "CLS",    keywordCLS   };
        _keywords["PRINT" ] = {0, "PRINT",  keywordPRINT };
        _keywords["FOR"   ] = {0, "FOR",    keywordFOR   };
        _keywords["NEXT"  ] = {0, "NEXT",   keywordNEXT  };
        _keywords["IF"    ] = {0, "IF",     keywordIF    };
        _keywords["ENDIF" ] = {0, "ENDIF",  nullptr      };
        _keywords["ELSE"  ] = {0, "ELSE",   nullptr      };
        _keywords["ELSEIF"] = {0, "ELSEIF", nullptr      };
        _keywords["DIM"   ] = {1, "DIM",    keywordDIM   };
        _keywords["DEF"   ] = {0, "DEF",    keywordDEF   };
        _keywords["INPUT" ] = {0, "INPUT",  nullptr      };
        _keywords["READ"  ] = {0, "READ",   nullptr      };
        _keywords["DATA"  ] = {0, "DATA",   nullptr      };
        _keywords["PEEK"  ] = {1, "PEEK",   nullptr      };
        _keywords["DEEK"  ] = {1, "DEEK",   nullptr      };
        _keywords["POKE"  ] = {1, "POKE",   keywordPOKE  };
        _keywords["DOKE"  ] = {1, "DOKE",   keywordDOKE  };
        _keywords["PLAY"  ] = {1, "PLAY",   keywordPLAY  };
        _keywords["USR"   ] = {1, "USR",    nullptr      };
        _keywords["DO"    ] = {0, "DO",     nullptr      };
        _keywords["LOOP"  ] = {0, "LOOP",   nullptr      };
        _keywords["WHILE" ] = {0, "WHILE",  nullptr      };
        _keywords["UNTIL" ] = {0, "UNTIL",  nullptr      };
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
        _keywords["AT"    ] = {1, "AT",     keywordAT    };
        _keywords["PUT"   ] = {1, "PUT",    keywordPUT   };
        _keywords["MODE"  ] = {1, "MODE",   keywordMODE  };
        _keywords["WAIT"  ] = {1, "WAIT",   keywordWAIT  };
        _keywords["LINE"  ] = {1, "LINE",   keywordLINE  };
        _keywords["HLINE" ] = {1, "HLINE",  keywordHLINE };
        _keywords["VLINE" ] = {1, "VLINE",  keywordVLINE };
        _keywords["SCROLL"] = {1, "SCROLL", keywordSCROLL};
        _keywords["CHR$"  ] = {1, "CHR$",   nullptr      };
        _keywords["HEX$"  ] = {1, "HEX$",   nullptr      };
        _keywords["HEXW$" ] = {1, "HEXW$",  nullptr      };
        _keywords["MID$"  ] = {1, "MID$",   nullptr      };
        _keywords["LEFT$" ] = {1, "LEFT$",  nullptr      };
        _keywords["RIGHT$"] = {1, "RIGHT$", nullptr      };
        _keywords["SPC$"  ] = {1, "SPC$",   nullptr      };
        _keywords["STR$"  ] = {1, "STR$",   nullptr      };
        _keywords["TIME$" ] = {1, "TIME$",  nullptr      };

        _keywordsWithEquals.push_back("DIM");
        _keywordsWithEquals.push_back("DEF");
        _keywordsWithEquals.push_back("FOR");
        _keywordsWithEquals.push_back("IF");

        return true;
    }


    bool readInputFile(std::ifstream& infile, const std::string& filename, int& numLines)
    {
        std::string line;

        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::readInputFile() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        // Read input .gbas file
        while(!infile.eof())
        {
            std::getline(infile, line);
            _input.push_back(line);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Compiler::readInputFile() : Bad line : '%s' : in '%s' : on line %d\n", line.c_str(), filename.c_str(), numLines+1);
                return false;
            }

            numLines++;
        }

        return true;
    }

    bool writeOutputFile(std::ofstream& outfile, const std::string& filename)
    {
        if(!outfile.is_open())
        {
            fprintf(stderr, "Compiler::writeOutputFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        // Write output .vasm file
        for(int i=0; i<_output.size(); i++)
        {
            outfile.write((char *)_output[i].c_str(), _output[i].size());
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Compiler::writeOutputFile() : write error in '%s'\n", filename.c_str());
                return false;
            }
        }

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

    int findLabel(const std::string& labelName)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_labels[i]._name == labelName) return i;
        }

        return -1;
    }
    
    int findLabel(uint16_t address)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_labels[i]._address == address) return i;
        }

        return -1;
    }

    int findInternalLabel(const std::string& labelName)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_internalLabels[i]._name == labelName) return i;
        }

        return -1;
    }
    
    int findInternalLabel(uint16_t address)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_internalLabels[i]._address == address) return i;
        }

        return -1;
    }

    int findVar(std::string& varName)
    {
        varName = Expression::getSubAlpha(varName);
        for(int i=0; i<_integerVars.size(); i++)
        {
            if(_integerVars[i]._name == varName) return i;
        }

        return -1;
    }


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label, bool isNumeric=false, bool addUnderscore=true, bool pageJump=false, bool gosub=false)
    {
        std::string n = name;
        Expression::stripWhitespace(n);
        std::string o = (addUnderscore) ? "_" + n : n;
        Expression::addString(o, LABEL_TRUNC_SIZE - int(o.size()));
        size_t space = o.find_first_of(" ");
        if(space == std::string::npos  ||  space >= LABEL_TRUNC_SIZE - 1)
        {
            o = o.substr(0, LABEL_TRUNC_SIZE);
            o[LABEL_TRUNC_SIZE - 1] = ' ';
        }

        label = {address, n, o, codeLineIndex, isNumeric, pageJump, gosub};
        Expression::stripWhitespace(label._name);
        _labels.push_back(label);
        _currentLabelIndex = int(_labels.size() - 1);
    }

    void createIntVar(const std::string& varName, int16_t data, int16_t init, CodeLine& codeLine, int codeLineIndex, bool containsVars,  int& varIndex, VarType varType=VarInt16, uint16_t arrayStart=0x0000, int intSize=Int16, int arrSize=0)
    {
        // Create var
        varIndex = int(_integerVars.size());
        codeLine._assignOperator = true;
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;

        uint16_t varStart = (varType == VarArray) ? 0x0000 : _userVarStart;
        IntegerVar integerVar = {data, init, varStart, arrayStart, varName, varName, codeLineIndex, varType, intSize, arrSize};
        _integerVars.push_back(integerVar);

        // Create var output
        std::string line = "_" + _integerVars[varIndex]._name;
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        size_t space = line.find_first_of(" ");
        if(space == std::string::npos  ||  space >= LABEL_TRUNC_SIZE - 1)
        {
            line = line.substr(0, LABEL_TRUNC_SIZE);
            line[LABEL_TRUNC_SIZE - 1] = ' ';
        }
        _integerVars[varIndex]._output = line;

        if(varType == VarInt16)
        {
            _userVarStart += Int16;
            if(_userVarStart >= USER_VAR_END) _userVarStart = USER_VAR_START;
        }
    }

    VarResult createCodeVar(CodeLine& codeLine, int codeLineIndex, int& varIndex)
    {
        size_t equals = Expression::findNonStringEquals(codeLine._code) - codeLine._code.begin();
        if(codeLine._code.size() > 2  &&  equals < codeLine._code.size())
        {
            // Check all tokens individually, don't just do a find as a var may exist with a reserved keyword embedded within it
            for(int i=0; i<codeLine._tokens.size(); i++)
            {
                std::string token = codeLine._tokens[i];
                Expression::stripWhitespace(token);
                Expression::strToUpper(token);

                // No need to check after the equals
                if(token == "=") break;

                // Check tokens that are reserved keywords using equals
                for(int j=0; j<_keywordsWithEquals.size(); j++)
                {
                    size_t keyword = token.find(_keywordsWithEquals[j]);
                    if(keyword != std::string::npos) return VarNotFound;
                }
            }

            // Name and input
            std::string varName = codeLine._code.substr(0, equals);
            Expression::stripWhitespace(varName);

            // Var already exists?
            varIndex = findVar(varName);
            if(varIndex != -1)
            {
                codeLine._assignOperator = true;
                codeLine._containsVars = false;
                codeLine._varIndex = varIndex;

                return VarExists;
            }

            createIntVar(varName, 0, 0, codeLine, codeLineIndex, false, varIndex);

            return VarCreated;
        }

        return VarNotFound;
    }

    uint32_t isExpression(std::string& input, int& varIndex, int& params)
    {
        uint32_t expressionType = 0x0000;

        // Check for strings
        if(input.find("$") != std::string::npos) expressionType |= Expression::HasStrings;
        if(input.find("\"") != std::string::npos) expressionType |= Expression::HasStrings;

        std::string stripped = Expression::stripStrings(input);
        std::vector<std::string> tokens = Expression::tokenise(stripped, "-+/*%<>=();, ", false);

        // Check for keywords
        for(int i=0; i<tokens.size(); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(_keywords.find(token) != _keywords.end())
            {
                params = _keywords[token]._params;
                expressionType |= Expression::HasKeywords;
                break;
            }
        }

        // Check for vars
        for(int i=0; i<tokens.size(); i++)
        {
            varIndex = findVar(tokens[i]);
            if(varIndex != -1)
            {
                // Array variables are treated as a function call
                if(_integerVars[varIndex]._varType == VarArray) params = 1;

                expressionType |= Expression::HasVars;
                break;
            }
        }

        // Check for operators
        if(stripped.find_first_of("-+/*%<>=") != std::string::npos) expressionType |= Expression::HasOperators;
        std::string mod = stripped;
        Expression::strToUpper(mod);
        if(mod.find("AND") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("XOR") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("OR")  != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("NOT") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("MOD") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("LSL") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("LSR") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("<<")  != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(">>")  != std::string::npos) expressionType |= Expression::HasOperators;

        return expressionType;
    }

    void updateVar(int16_t data, CodeLine& codeLine, int varIndex, bool containsVars)
    {
        codeLine._assignOperator = true;
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        _integerVars[varIndex]._data = data;
    }

    bool createCodeLine(const std::string& code, int codeLineOffset, int labelIndex, int varIndex, bool assign, bool vars, CodeLine& codeLine)
    {
        // Handle variables
        std::string expression = code;
        size_t equal = code.find_first_of("=");
        if(equal != std::string::npos)
        {
            expression = code.substr(equal + 1);
        }
        else
        {
            size_t lbra = code.find_first_of("(");
            size_t rbra = code.find_last_of(")");
            if(lbra != std::string::npos  &&  rbra != std::string::npos)
            {
                expression = code.substr(lbra + 1, rbra - (lbra + 1));
            }
        }
        Expression::trimWhitespace(expression);

        std::vector<int> lut;
        OnGotoGosubLut onGotoGosubLut = {0x0000, "", lut};

        std::vector<VasmLine> vasm;
        std::string text = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        Expression::trimWhitespace(text);
        std::string codeText = Expression::collapseWhitespaceNotStrings(text);
        std::vector<std::string> tokens = Expression::tokenise(codeText, ' ', false);
        codeLine = {text, codeText, tokens, vasm, expression, onGotoGosubLut, 0, labelIndex, varIndex, assign, vars, false};
        Expression::operatorReduction(codeLine._expression);

        if(codeLine._code.size() < 3) return false; // anything too small is ignored

        return true;
    }


    void getNextTempVar(void)
    {
        static int prevCodeLineIndex = -1;
        if(_currentCodeLineIndex != prevCodeLineIndex)
        {
            prevCodeLineIndex = _currentCodeLineIndex;
            _tempVarStart = TEMP_VAR_START;
        }
        else
        {
            _tempVarStart += 2;
            if(_tempVarStart >= TEMP_VAR_START + 0x10) _tempVarStart = TEMP_VAR_START; // 16 bytes of temporary expression variables allows for 8 expression depths
        }

        _tempVarStartStr = Expression::wordToHexString(_tempVarStart);
    }

    // Find text in a macro
    bool findMacroText(const std::string& macroName, const std::string& text)
    {
        if(_macroIndexEntries.find(macroName) == _macroIndexEntries.end()) return false;

        int indexStart = _macroIndexEntries[macroName]._indexStart;
        int indexEnd = _macroIndexEntries[macroName]._indexEnd;
        for(int i=indexStart+1; i<indexEnd; i++)
        {
            size_t commentStart = _macroLines[i].find_first_of(";#");
            std::string macroLine = (commentStart != std::string::npos) ? _macroLines[i].substr(0, commentStart) : _macroLines[i];
            std::vector<std::string> tokens = Expression::tokeniseLine(macroLine);

            for(int j=0; j<tokens.size(); j++)
            {
                if(tokens[j].find(text) != std::string::npos) return true;
            }

            // Check for nested macros
            for(int j=0; j<tokens.size(); j++)
            {
                if(findMacroText(tokens[j], text)) return true;
            }
        }

        return false;
    }

    // Find macro and work out it's vASM byte size
    int getMacroSize(const std::string& macroName)
    {
        if(_macroIndexEntries.find(macroName) == _macroIndexEntries.end()) return 0;

        int opcodesSize = 0;
        int indexStart = _macroIndexEntries[macroName]._indexStart;
        int indexEnd = _macroIndexEntries[macroName]._indexEnd;
        for(int i=indexStart+1; i<indexEnd; i++)
        {
            size_t commentStart = _macroLines[i].find_first_of(";#");
            std::string macroLine = (commentStart != std::string::npos) ? _macroLines[i].substr(0, commentStart) : _macroLines[i];
            std::vector<std::string> tokens = Expression::tokeniseLine(macroLine);

            int opcodeSize = 0;
            for(int j=0; j<tokens.size(); j++)
            {
                opcodeSize = Assembler::getAsmOpcodeSize(tokens[j]);
                if(opcodeSize)
                {
                    opcodesSize += opcodeSize;
                    break;
                }
            }

            // Check for nested macros
            if(opcodeSize == 0)
            {
                for(int j=0; j<tokens.size(); j++)
                {
                    opcodeSize = getMacroSize(tokens[j]);
                    if(opcodeSize)
                    {
                        opcodesSize += opcodeSize;
                        break;
                    }
                }
            }            
        }

        return opcodesSize;
    }

    bool initialiseMacros(void)
    {
        std::string filename = (!Assembler::getUseOpcodeCALLI()) ? "gbas/include/macros.i" : "gbas/include/macros_CALLI.i";
        std::ifstream infile(filename);

        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::initialiseMacros() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        std::string lineToken;
        while(!infile.eof())
        {
            std::getline(infile, lineToken);
            _macroLines.push_back(lineToken);
        }

        // Macro names
        int macroIndex;
        std::string macroName;
        bool foundMacro = false;
        for(int i=0; i<_macroLines.size(); i++)
        {
            std::vector<std::string> tokens = Expression::tokeniseLine(_macroLines[i]);
            if(!foundMacro  &&  tokens.size() >= 2  &&  tokens[0] == "%MACRO")
            {
                macroIndex = i;
                macroName = tokens[1];

                MacroNameEntry macroNameEntry = {macroName, 0, 0};
                _macroNameEntries[macroIndex] = macroNameEntry;

                MacroIndexEntry macroIndexEntry = {macroIndex, 0, 0};
                _macroIndexEntries[macroName] = macroIndexEntry;

                foundMacro = true;
            }
            else if(foundMacro  &&  tokens.size() >= 1  &&  tokens[0] == "%ENDM")
            {
                _macroNameEntries[macroIndex]._indexEnd = i;
                _macroIndexEntries[macroName]._indexEnd = i;
                foundMacro = false;
            }
        }

        // %MACRO is missing a %ENDM
        if(foundMacro)
        {
            //_macroNameEntries.erase(macroIndex);
            //_macroIndexEntries.erase(macroName);

            fprintf(stderr, "Compiler::initialiseMacros() : %%MACRO %s on line %d: is missing a %%ENDM\n", macroName.c_str(), macroIndex);
            return false;
        }

        // Calculate macros vASM byte sizes
        for(auto it=_macroNameEntries.begin(); it!=_macroNameEntries.end(); ++it)
        {
            int macroIndex = _macroIndexEntries[it->second._name]._indexStart;
            it->second._byteSize = getMacroSize(it->second._name);
            _macroIndexEntries[it->second._name]._byteSize = it->second._byteSize;
            //fprintf(stderr, "%s  %d %d  %d %d bytes\n", it->second._name.c_str(), macroIndex, it->second._indexEnd, it->second._byteSize, _macroIndexEntries[it->second._name]._byteSize);
        }

        return true;
    }


    int createVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, std::string& line)
    {
        int vasmSize = 0;
        std::string opcode = std::string(opcodeStr);

        // Get macro size
        if(opcode.size()  &&  opcode[0] == '%')
        {
            opcode.erase(0, 1);
            if(_macroIndexEntries.find(opcode) != _macroIndexEntries.end())
            {
                vasmSize = _macroIndexEntries[opcode]._byteSize;
            }
        }
        // Get opcode size
        else
        {
            vasmSize = Assembler::getAsmOpcodeSize(opcode);
        }

        _vasmPC += vasmSize;

        //fprintf(stderr, "%s  %d %04x\n", opcode.c_str(), vasmSize, _vasmPC);

        std::string operand = std::string(operandStr);
        line = opcode + std::string(OPCODE_TRUNC_SIZE - opcode.size(), ' ') + operand;

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLineIndex, const std::string& internalLabel="", bool pageJump=false)
    {
        std::string line;
        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);

        // NEXT and THEN don't know where the next vasm instruction is, so they use _nextInternalLabel, (which has priority over internalLabel)
        std::string label = (_nextInternalLabel.size()) ? _nextInternalLabel : internalLabel;

        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, line, label, pageJump, vasmSize});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        if(nextTempVar) getNextTempVar();
        _nextInternalLabel = "";
    }

    // Generic LDW expression parser
    uint32_t parseArrayVarExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, int16_t& value)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, value);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasVars)
        {
            emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
        }

        return expressionType;
    }

    bool writeArrayVar(CodeLine& codeLine, int codeLineIndex, int varIndex)
    {
        // Array var?
        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, 0, lbra, rbra)) return false;
        size_t equals = codeLine._code.find("=");
        if(equals == std::string::npos  ||  equals < rbra) return false;

        // Previous expression result
        emitVcpuAsm("STW", "register0", false, codeLineIndex); // register0 = memValue, but can't use memValue here as include file is not guaranteed to be loaded

        // Array index from expression
        int16_t arrIndex;
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        uint32_t expressionType = parseArrayVarExpression(codeLine, codeLineIndex, arrText, arrIndex);

        int intSize = _integerVars[varIndex]._intSize;
        uint16_t arrayPtr = _integerVars[varIndex]._array;

        // Constant index
        if(!(expressionType & Expression::HasVars))
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + arrIndex*intSize), false, codeLineIndex);
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDW",  "register0", false, codeLineIndex);
            emitVcpuAsm("DOKE", "register1", false, codeLineIndex);
        }
        else
        {
            if(Assembler::getUseOpcodeCALLI())
            {
#ifdef SMALL_CODE_SIZE
                // Saves 7 bytes per array access but costs an extra 2 instructions in performance
                emitVcpuAsm("STW", "memIndex", false, codeLineIndex);
                emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
                emitVcpuAsm("CALLI", "setArrayInt16", false, codeLineIndex);
#endif
            }
            else
            {
#ifdef SMALL_CODE_SIZE
                // Saves 3 bytes per array access, but an extra 5 instructions in performance
                emitVcpuAsm("STW",  "memIndex", false, codeLineIndex);
                emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
                emitVcpuAsm("STW",  "memAddr", false, codeLineIndex);
                emitVcpuAsm("LDWI", "setArrayInt16", false, codeLineIndex);
                emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
#endif
            }

#ifndef SMALL_CODE_SIZE
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
            emitVcpuAsm("ADDW", "register1", false, codeLineIndex);
            emitVcpuAsm("ADDW", "register1", false, codeLineIndex);
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDW",  "register0", false, codeLineIndex);
            emitVcpuAsm("DOKE", "register1", false, codeLineIndex);
#endif
        }

        return true;
    }

    bool emitVcpuAsmUserVar(const std::string& opcodeStr, const char* varNamePtr, bool nextTempVar)
    {
        std::string opcode = std::string(opcodeStr);
        std::string varName = std::string(varNamePtr);
        int varIndex = findVar(varName);
        if(varIndex == -1)
        {
            fprintf(stderr, "Compiler::emitVcpuAsmUserVar() : couldn't find variable name '%s'\n", varName.c_str());
            return false;
        }

        emitVcpuAsm(opcode, "_" + _integerVars[varIndex]._name, nextTempVar);

        return true;
    }

    // Generic expression parser
    enum OperandType {OperandVar, OperandTemp, OperandConst};
    OperandType parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, std::string& operand)
    {
        int16_t value;
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, value);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords))
        {
            operand = Expression::byteToHexString(uint8_t(_tempVarStart));
            return OperandTemp;
        }
        else if(expressionType & Expression::HasVars)
        {
            operand = _integerVars[varIndex]._name;
            return OperandVar;
        }

        operand = std::to_string(value);
        return OperandConst;
    }

    // Generic LDW expression parser
    uint32_t parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, int16_t& value)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, value);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasVars)
        {
            emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
        }
        else
        {
            (value >= 0  &&  value <= 255) ? emitVcpuAsm("LDI", std::to_string(value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(value), false, codeLineIndex);
        }

        return expressionType;
    }

    // Loop specific parser
    uint32_t parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, int16_t& value, int16_t replace)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, value);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasVars)
        {
            emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
        }
        else
        {
            if(value == 0  &&  replace != 0) value = replace;
            (value >= 0  &&  value <= 255) ? emitVcpuAsm("LDI", std::to_string(value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(value), false, codeLineIndex);
        }

        return expressionType;
    }

    bool isGosubLabel(const std::string& label)
    {
        std::string lab = label;

        for(int i=0; i<_gosubLabels.size(); i++)
        {
            if(_gosubLabels[i] == Expression::strToUpper(lab)) return true;
        }

        return false;
    }

    bool checkForGosubLabel(const std::string& code, int lineNumber)
    {
        std::vector<std::string> tokens = Expression::tokenise(code, ' ', true, true);
        for(int i=0; i<tokens.size(); i++)
        {
            if(tokens[i] == "GOSUB")
            {
                if(i+1 >= tokens.size())
                {
                    fprintf(stderr, "Compiler::checkForGosubLabel() : missing label after GOSUB in : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
                    return false;
                }
                _gosubLabels.push_back(tokens[i+1]);
            }
        }

        return true;
    }

    LabelResult checkForLabel(std::string& code, int lineNumber)
    {
        Label label;
        CodeLine codeLine;
        std::string gosubOpcode = "";

        // Numeric labels
        if(code.size() > 1  &&  isdigit(code[0]))
        {
            size_t space = code.find_first_of(" \n\r\f\t\v,");
            if(space == std::string::npos) space = code.size() - 1;

            // Force space between line numbers and line
            for(int i=1; i<space; i++)
            {
                if(!isdigit(code[i]))
                {
                    space = i;
                    code.insert(i, " ");
                    break;
                }
            }

            if(code.size() - (space + 1) <= 2)
            {
                fprintf(stderr, "Compiler::checkForLabel() : line number cannot exist on its own : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
                return LabelError;
            }

            // Create label
            std::string labelName = code.substr(0, space);
            bool foundGosub = isGosubLabel(labelName);
            createLabel(_vasmPC, labelName, labelName, int(_codeLines.size()), label, true, true, false, foundGosub);
            if(createCodeLine(code, int(space + 1), _currentLabelIndex, -1, false, false, codeLine)) _codeLines.push_back(codeLine);

            return LabelFound;
        }

        // Text label
        size_t colon1 = code.find_first_of(":");
        if(colon1 != std::string::npos)
        {
            std::string labelName = code.substr(0, colon1);
            bool validLabel = true;
            if(labelName.size())
            {
                for(int i=0; i<labelName.size(); i++)
                {
                    if(!(labelName[i] == '_')  &&  !isalnum(labelName[i]))
                    {
                        validLabel = false;
                        break;
                    }
                }
            }
            if(validLabel)
            {
                // Create label
                bool foundGosub = isGosubLabel(labelName);
                createLabel(_vasmPC, labelName, labelName, int(_codeLines.size()), label, false, true, false, foundGosub);
                if(createCodeLine(code, int(colon1  + 1), _currentLabelIndex, -1, false, false, codeLine)) _codeLines.push_back(codeLine);
                return LabelFound;
            }
        }

        // Non label code
        if(createCodeLine(code, 0, -1, -1, false, false, codeLine)) _codeLines.push_back(codeLine);

        return LabelNotFound;
    }

    bool parseLabels(int numLines)
    {
        // By default do not support CALLI
        Assembler::setUseOpcodeCALLI(false);
        for(auto it=_input.begin(); it!=_input.end(); ++it)
        {
            if(it->find("_useOpcodeCALLI_") != std::string::npos)
            {
                Assembler::setUseOpcodeCALLI(true);
                break;
            }
        }

        // Relies on _useOpcodeCALLI_
        initialiseMacros();

        // Entry point initialisation
        Label label;
        CodeLine codeLine;
        createLabel(_vasmPC, "_entryPoint_", "_entryPoint_\t", 0, label, false, false, false, false);
        if(createCodeLine("INIT", 0, 0, -1, false, false, codeLine)) _codeLines.push_back(codeLine);
        emitVcpuAsm("%Initialise", "", false, 0);
        if(!Assembler::getUseOpcodeCALLI())
        {
            // Handles time sliced, (real time), code such as MIDI and SPRITES
            emitVcpuAsm("%InitRealTimeProc", "", false, 0);

            // Handle relational operators
            emitVcpuAsm("%InitEqOp", "", false, 0);
            emitVcpuAsm("%InitNeOp", "", false, 0);
            emitVcpuAsm("%InitLeOp", "", false, 0);
            emitVcpuAsm("%InitGeOp", "", false, 0);
            emitVcpuAsm("%InitLtOp", "", false, 0);
            emitVcpuAsm("%InitGtOp", "", false, 0);
        }

        // GOSUB labels
        for(int i=0; i<numLines; i++)
        {
            if(!checkForGosubLabel(_input[i], i)) return false;
        }

        // All labels
        for(int i=0; i<numLines; i++)
        {
            switch(checkForLabel(_input[i], i))
            {
                case LabelFound:    break;
                case LabelNotFound: break;
                case LabelError:    return false;
            }
        }

        return true;
    }

    // Adjust label addresses for any labels with addresses higher than optimised vasm instruction address
    void adjustLabelAddresses(uint16_t optimisedAddress, int offset)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_labels[i]._address >= optimisedAddress)
            {
                _labels[i]._address += offset;
            }
        }
    }

    // Adjust vasm code addresses
    void adjustVasmAddresses(int codeLineIndex, int vasmLineIndex, int offset)
    {
        for(int i=codeLineIndex; i<_codeLines.size(); i++)
        {
            int start = (i == codeLineIndex) ? vasmLineIndex : 0;
            for(int j=start; j<_codeLines[i]._vasm.size(); j++)
            {
                _codeLines[i]._vasm[j]._address += offset;
            }
        }
    }

    // Create string and advance string pointer
    bool createString(CodeLine& codeLine, int codeLineIndex, const std::string& str, std::string& name, uint16_t& address)
    {
        if(str.size() > USER_STR_SIZE)
        {
            fprintf(stderr, "Compiler::createString() : user string is %d characters too long in '%s' on line %d\n", int(str.size() - USER_STR_SIZE), codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Reuse string if possible
        int index = -1;
        for(int j=0; j<_stringVars.size(); j++)
        {
            if(_stringVars[j]._data == str) 
            {
                index = j;
                break;
            }
        }
        if(index != -1)
        {
            name  = _stringVars[index]._name;
            address = _stringVars[index]._address;
        }
        else
        {
            if(!Memory::giveFreeRAM(Memory::FitAscending, int(str.size()) + 1, 0x0200, 0x7FFF, address))
            {
                fprintf(stderr, "Compiler::createString() : Not enough RAM for string %s of size %d\n", str.c_str(), int(str.size()));
                return false;
            }

            // Save end of runtime/strings
            if(address < _runtimeEnd) _runtimeEnd = address;

            name = "str_" + Expression::wordToHexString(address);
            StringVar stringVar = {uint8_t(str.size()), address, str, name, name + std::string(LABEL_TRUNC_SIZE - name.size(), ' '), -1};
            _stringVars.push_back(stringVar);
        }

        return true;
    }

    // Create constant string    
    void createConstantString(ConstantStrType constantStrType, int16_t& value)
    {
        char str[16];
        switch(constantStrType)
        {
            case StrChar: sprintf(str, "%c",   uint8_t(value) & 0x007F); break;
            case StrHex:  sprintf(str, "%02x", uint8_t(value));          break;
            case StrHexw: sprintf(str, "%04x", uint16_t(value));         break;

            default: break;
        }

        std::string name;
        uint16_t address;
        createString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, std::string(str), name, address);
        emitVcpuAsm("LDWI", name, false);
        value = address;
    }


    // ********************************************************************************************
    // Functions
    // ********************************************************************************************
    bool handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric)
    {
        if(numeric._isAddress)
        {
            // Temporary variable address
            if(isdigit(numeric._varName[0]))
            {
                emitVcpuAsm(opcodeStr, Expression::byteToHexString(uint8_t(numeric._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar(opcodeStr, numeric._varName.c_str(), false)) return false;
            }
        }

        numeric._value = uint8_t(_tempVarStart);
        numeric._isAddress = true;
        numeric._varName = _tempVarStartStr;

        return true;
    }

    Expression::Numeric functionCHR$(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            // Print constant, (without wasting memory)
            if(Expression::getEnablePrint())
            {
                emitVcpuAsm("LDI", std::to_string(numeric._value), false);
                emitVcpuAsm("%PrintAcChar", "", false);
                return numeric;
            }

            // Create constant string
            createConstantString(StrChar, numeric._value);
            return numeric;
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcChar", "", false);

        return numeric;
    }

    Expression::Numeric functionHEX$(Expression::Numeric& numeric)
    {
        // Print constant, (without wasting memory)
        if(!numeric._isAddress)
        {
            if(Expression::getEnablePrint())
            {
                emitVcpuAsm("LDI", std::to_string(numeric._value), false);
                emitVcpuAsm("%PrintAcHexByte", "", false);
                return numeric;
            }

            // Create constant string
            createConstantString(StrHex, numeric._value);
            return numeric;
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcHexByte", "", false);

        return numeric;
    }

    Expression::Numeric functionHEXW$(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            // Print constant, (without wasting memory)
            if(Expression::getEnablePrint())
            {
                emitVcpuAsm("LDWI", std::to_string(numeric._value), false);
                emitVcpuAsm("%PrintAcHexWord", "", false);
                return numeric;
            }

            // Create constant string
            createConstantString(StrHexw, numeric._value);
            return numeric;
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcHexWord", "", false);

        return numeric;
    }

    Expression::Numeric functionPEEK(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            (numeric._value >= 0  && numeric._value <= 255) ? emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        emitVcpuAsm("PEEK", "", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }

    Expression::Numeric functionDEEK(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            (numeric._value >= 0  && numeric._value <= 255) ? emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        emitVcpuAsm("DEEK", "", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }

    Expression::Numeric functionUSR(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            (numeric._value >= 0  && numeric._value <= 255) ? emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        emitVcpuAsm("CALL", "giga_vAC", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }

    Expression::Numeric functionRND(Expression::Numeric& numeric)
    {
        bool useMod = true;
        if(!numeric._isAddress)
        {
            // RND(0) skips the MOD call and allows you to filter the output manually
            if(numeric._value == 0)
            {
                useMod = false;
            }
            else
            {
                (numeric._value > 0  && numeric._value <= 255) ? emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false) : emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
            }
        }

        getNextTempVar();
        if(useMod)
        {
            handleSingleOp("LDW", numeric);
            emitVcpuAsm("%RandMod", "", false);
        }
        else
        {
            numeric._value = uint8_t(_tempVarStart);
            numeric._isAddress = true;
            numeric._varName = _tempVarStartStr;

            emitVcpuAsm("%Rand", "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }

    Expression::Numeric functionARR(Expression::Numeric& numeric)
    {
        getNextTempVar();

        int intSize = _integerVars[numeric._index]._intSize;
        uint16_t arrayPtr = _integerVars[numeric._index]._array;

        if(!numeric._isAddress)
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + numeric._value*intSize), false);
            emitVcpuAsm("DEEK", "", false);

            numeric._value = uint8_t(_tempVarStart);
            numeric._isAddress = true;
            numeric._varName = _tempVarStartStr;
        }
        else
        {
            handleSingleOp("LDW", numeric);

#ifdef SMALL_CODE_SIZE
            // Saves 2 bytes per array access but costs 2 extra instructions in performance
            if(Assembler::getUseOpcodeCALLI())
            {
                emitVcpuAsm("STW", "memIndex", false);
                emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                emitVcpuAsm("CALLI", "getArrayInt16", false);
            }
            else
#endif
            {
                emitVcpuAsm("STW", "register2", false);
                emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                emitVcpuAsm("ADDW", "register2", false);
                emitVcpuAsm("ADDW", "register2", false);
                emitVcpuAsm("DEEK", "",          false);
            }
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }


    // ********************************************************************************************
    // Unary Operators
    // ********************************************************************************************
    Expression::Numeric operatorNEG(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            numeric._value = -numeric._value;
            return numeric;
        }

        getNextTempVar();
        emitVcpuAsm("LDI", std::to_string(0), false);
        handleSingleOp("SUBW", numeric);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);
        
        return numeric;
    }

    Expression::Numeric operatorNOT(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            numeric._value = ~numeric._value;
            return numeric;
        }

        getNextTempVar();
        emitVcpuAsm("LDWI", std::to_string(-1), false);
        handleSingleOp("SUBW", numeric);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return numeric;
    }


    // ********************************************************************************************
    // Binary Operators
    // ********************************************************************************************
    bool handleDualOp(const std::string& opcodeStr, Expression::Numeric& lhs, Expression::Numeric& rhs, bool outputHex)
    {
        std::string opcode = std::string(opcodeStr);

        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(!rhs._isAddress  &&  uint16_t(rhs._value) > 255)
        {
            std::swap(lhs, rhs);
            if(opcode == "SUB")
            {
                opcode = "ADD";
                if(lhs._value > 0) lhs._value = -lhs._value;
            }
        }

        // LHS
        if(lhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(lhs._varName[0]))
            {
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("LDW", lhs._varName.c_str(), true)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                (outputHex) ? emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(lhs._value)), false) : emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                (outputHex) ? emitVcpuAsm("LDWI", Expression::wordToHexString(lhs._value), false) : emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
            }

            _nextTempVar = true;
        }

        // RHS
        if(rhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(rhs._varName[0]))
            {
                emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(rhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar(opcode + "W", rhs._varName.c_str(), _nextTempVar)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            emitVcpuAsm(opcode + "I", std::to_string(rhs._value), false);
        }

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varName = _tempVarStartStr;

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return true;
    }

    Expression::Numeric operatorADD(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value += right._value;
            return left;
        }

        left._isValid = handleDualOp("ADD", left, right, false);
        return left;
    }

    Expression::Numeric operatorSUB(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value -= right._value;
            return left;
        }

        left._isValid = handleDualOp("SUB", left, right, false);
        return left;
    }

    Expression::Numeric operatorAND(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value &= right._value;
            return left;
        }

        left._isValid = handleDualOp("AND", left, right, true);
        return left;
    }

    Expression::Numeric operatorXOR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value ^= right._value;
            return left;
        }

        left._isValid = handleDualOp("XOR", left, right, true);
        return left;
    }

    Expression::Numeric operatorOR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value |= right._value;
            return left;
        }

        left._isValid = handleDualOp("OR", left, right, true);
        return left;
    }

    bool handleLogicalOp(const std::string& opcode, Expression::Numeric& lhs, Expression::Numeric& rhs)
    {
        // SYS shift function need this preamble, LSLW doesn't
        // Temporary variable address
        if(isdigit(lhs._varName[0]))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
        }
        // User variable address
        else
        {
            if(!emitVcpuAsmUserVar("LDW", lhs._varName.c_str(), true)) return false;
        }

        if(opcode != "LSLW"  &&  opcode != "<<") emitVcpuAsm("STW", "mathShift", false);

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varName = _tempVarStartStr;

        return true;
    }

    Expression::Numeric operatorLSL(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value <<= right._value;
            return left;
        }

        getNextTempVar();

        if(left._isAddress  &&  !right._isAddress)
        {
            std::string opcode;
            switch(right._value)
            {
                case 1: opcode = "LSLW"; break;
                case 4: opcode = "%ShiftLeft4bit"; break;
                case 8: opcode = "%ShiftLeft8bit"; break;
            }

            handleLogicalOp(opcode, left, right);

            emitVcpuAsm(opcode, "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorLSR(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value >>= right._value;
            return left;
        }

        getNextTempVar();

        if(left._isAddress  &&  !right._isAddress)
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
                case 8: opcode = "%ShiftRight8bit"; break;
            }

            handleLogicalOp(opcode, left, right);

            emitVcpuAsm(opcode, "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }


    // ********************************************************************************************
    // Conditional Operators
    // ********************************************************************************************
    bool handleCondOp(Expression::Numeric& lhs, Expression::Numeric& rhs)
    {
        std::string opcode = "SUB";

        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(!rhs._isAddress  &&  uint16_t(rhs._value) > 255)
        {
            std::swap(lhs, rhs);
            opcode = "ADD";
            if(lhs._value > 0) lhs._value = -lhs._value;
        }

        // LHS
        if(lhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(lhs._varName[0]))
            {
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("LDW", lhs._varName.c_str(), true)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
            }

            _nextTempVar = true;
        }

        // RHS
        if(rhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(rhs._varName[0]))
            {
                emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(rhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar(opcode + "W", rhs._varName.c_str(), _nextTempVar)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            emitVcpuAsm(opcode + "I", std::to_string(rhs._value), false);
        }

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varName = _tempVarStartStr;

        return true;
    }

    Expression::Numeric operatorEQ(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value == right._value);
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert equals into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertEqOp", false) : emitVcpuAsm("CALL", "convertEqOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorNE(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value != right._value;
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert equals into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertNeOp", false) : emitVcpuAsm("CALL", "convertNeOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorLE(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value <= right._value;
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert less than or equals into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertLeOp", false) : emitVcpuAsm("CALL", "convertLeOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorGE(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value >= right._value;
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert greater than or equals into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertGeOp", false) : emitVcpuAsm("CALL", "convertGeOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorLT(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value < right._value);
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert less than into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertLtOp", false) : emitVcpuAsm("CALL", "convertLtOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorGT(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value > right._value);
            return left;
        }

        left._isValid = handleCondOp(left, right);

        // Convert greater than into a logical 1, (boolean conversion)
        Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convertGtOp", false) : emitVcpuAsm("CALL", "convertGtOpAddr", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }


    // ********************************************************************************************
    // Math Operators
    // ********************************************************************************************
    bool handleMathOp(const std::string& opcode, const std::string& operand, Expression::Numeric& lhs, Expression::Numeric& rhs, bool isMod=false)
    {
        // LHS
        if(lhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(lhs._varName[0]))
            {
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("LDW", lhs._varName.c_str(), true)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            // 8bit positive constants
            if(lhs._value >=0  &&  lhs._value <= 255)
            {
                emitVcpuAsm("LDI", std::to_string(lhs._value), false);
            }
            // 16bit constants
            else
            {
                emitVcpuAsm("LDWI", std::to_string(lhs._value), false);
            }

            _nextTempVar = true;
        }

        emitVcpuAsm("STW", "mathX", false);

        // RHS
        if(rhs._isAddress)
        {
            // Temporary variable address
            if(isdigit(rhs._varName[0]))
            {
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(rhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("LDW", rhs._varName.c_str(), _nextTempVar)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            if(rhs._value >=0  &&  rhs._value <= 255)
            {
                emitVcpuAsm("LDI", std::to_string(rhs._value), false);
            }
            else
            {
                emitVcpuAsm("LDWI", std::to_string(rhs._value), false);
            }
        }

        emitVcpuAsm("STW", "mathY", false);

        if(Assembler::getUseOpcodeCALLI())
        {
            emitVcpuAsm(opcode, operand, false);
        }
        else
        {
            emitVcpuAsm("LDWI", operand, false);
            emitVcpuAsm(opcode, "giga_vAC", false);
        }

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varName = _tempVarStartStr;
        
        if(isMod) emitVcpuAsm("LDW", "mathRem", false);
        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return true;
    }

    Expression::Numeric operatorMUL(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value *= right._value;
            return left;
        }

        // Optimise multiply with 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, std::string(""));

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "multiply16bit", left, right) : handleMathOp("CALL", "multiply16bit", left, right);

        return left;
    }

    Expression::Numeric operatorDIV(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (right._value == 0) ? 0 : left._value / right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, std::string(""));

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "divide16bit", left, right) : handleMathOp("CALL", "divide16bit", left, right);

        return left;
    }

    Expression::Numeric operatorMOD(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (right._value == 0) ? 0 : left._value % right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, std::string(""));

        left._isValid = (Assembler::getUseOpcodeCALLI()) ? handleMathOp("CALLI", "divide16bit", left, right, true) : handleMathOp("CALL", "divide16bit", left, right, true);

        return left;
    }


    // ********************************************************************************************
    // Recursive Descent Parser helpers
    // ********************************************************************************************
    char peek(bool dontSkipSpaces)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(!dontSkipSpaces  &&  Expression::peek() == ' ')
        {
            if(!Expression::advance(1)) return 0;
        }

        return Expression::peek();
    }

    char get(bool dontSkipSpaces)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(!dontSkipSpaces  &&  Expression::peek() == ' ')
        {
            if(!Expression::advance(1)) return 0;
        }

        return Expression::get();
    }

    bool number(int16_t& value)
    {
        char uchr;

        std::string valueStr;
        uchr = toupper(peek(false));
        valueStr.push_back(uchr); get(false);
        uchr = toupper(peek(false));
        if((uchr >= '0'  &&  uchr <= '9')  ||  uchr == 'X'  ||  uchr == 'H'  ||  uchr == 'B'  ||  uchr == 'O'  ||  uchr == 'Q')
        {
            valueStr.push_back(uchr); get(false);
            uchr = toupper(peek(false));
            while((uchr >= '0'  &&  uchr <= '9')  ||  (uchr >= 'A'  &&  uchr <= 'F'))
            {
                // Don't skip spaces here, as hex numbers can become attached to variables, keywords, etc
                valueStr.push_back(get(true));
                uchr = toupper(peek(true));
            }
        }

        return Expression::stringToI16(valueStr, value);
    }

    Expression::Numeric factor(int16_t defaultValue)
    {
        int16_t value = 0;
        Expression::Numeric numeric;

        if(peek(false) == '(')
        {
            get(false);
            numeric = expression();
            if(peek(false) != ')')
            {
                fprintf(stderr, "Compiler::factor() : Found '%c' : expecting ')' in '%s' on line %d\n", peek(false), Expression::getExpressionToParse(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, -1, false, false, std::string(""));
            }
            get(false);
        }
        else if((peek(false) >= '0'  &&  peek(false) <= '9')  ||  peek(false) == '&')
        {
            if(!number(value))
            {
                fprintf(stderr, "Compiler::factor() : Bad numeric data in '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, -1, false, false, std::string(""));
            }
            else
            {
                numeric = Expression::Numeric(value, -1, true, false, std::string(""));
            }
        }
        // Functions
        else if(Expression::find("CHR$"))
        {
            numeric = factor(0); numeric = functionCHR$(numeric);
        }
        else if(Expression::find("HEX$"))
        {
            numeric = factor(0); numeric = functionHEX$(numeric);
        }
        else if(Expression::find("HEXW$"))
        {
            numeric = factor(0); numeric = functionHEXW$(numeric);
        }
        else if(Expression::find("PEEK"))
        {
            numeric = factor(0); numeric = functionPEEK(numeric);
        }
        else if(Expression::find("DEEK"))
        {
            numeric = factor(0); numeric = functionDEEK(numeric);
        }
        else if(Expression::find("USR"))
        {
            numeric = factor(0); numeric = functionUSR(numeric);
        }
        else if(Expression::find("RND"))
        {
            numeric = factor(0); numeric = functionRND(numeric);
        }
        // Unary operators
        else if(Expression::find("NOT"))
        {
            numeric = factor(0); numeric = operatorNOT(numeric);
        }
        else
        {
            switch(peek(false))
            {
                // Unary operators
                case '+': get(false); numeric = factor(0);                                 break;
                case '-': get(false); numeric = factor(0); numeric = operatorNEG(numeric); break;

                // Reached end of expression
                case 0: numeric = Expression::Numeric(defaultValue, -1, false, false, std::string("")); break;

                default:
                {
                    // Variables
                    std::string varName = Expression::getExpression();
                    int varIndex = findVar(varName);
                    if(varIndex != -1)
                    {
                        Expression::advance(varName.size());
                        
                        // Arrays
                        if(_integerVars[varIndex]._varType == VarArray)
                        {
                            numeric = factor(0); numeric._index = varIndex; numeric = functionARR(numeric);
                        }
                        // Vars
                        else
                        {
                            numeric = Expression::Numeric(defaultValue, -1, true, true, varName);
                        }
                    }
                    // Unknown symbol
                    else
                    {
                        numeric = Expression::Numeric(defaultValue, -1, false, false, std::string(""));
                        if(varName.size()) fprintf(stderr, "Compiler::factor() : Found an unknown symbol '%s' : in '%s' on line %d\n", varName.c_str(), _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber() + 1);
                    }
                }
                break;
            }
        }

        return numeric;
    }

    Expression::Numeric term(void)
    {
        Expression::Numeric f, result = factor(0);

        for(;;)
        {
            if(peek(false) == '*')           {get(false);f = factor(0);result = operatorMUL(result, f);}
            else if(peek(false) == '/')      {get(false);f = factor(0);result = operatorDIV(result, f);}
            else if(peek(false) == '%')      {get(false);f = factor(0);result = operatorMOD(result, f);}
            else if(Expression::find("MOD")) {           f = factor(0);result = operatorMOD(result, f);}
            else if(Expression::find("AND")) {           f = factor(0);result = operatorAND(result, f);}
            else return result;
        }
    }

    Expression::Numeric expression(void)
    {
        Expression::Numeric t, result = term();

        for(;;)
        {
            if(peek(false) == '+')           {get(false);t = term();result = operatorADD(result, t);}
            else if(peek(false) == '-')      {get(false);t = term();result = operatorSUB(result, t);}
            else if(Expression::find("XOR")) {           t = term();result = operatorXOR(result, t);}
            else if(Expression::find("OR"))  {           t = term();result = operatorOR(result,  t);}
            else if(Expression::find("LSL")) {           t = term();result = operatorLSL(result, t);}
            else if(Expression::find("LSR")) {           t = term();result = operatorLSR(result, t);}
            else if(Expression::find("<<"))  {           t = term();result = operatorLSL(result, t);}
            else if(Expression::find(">>"))  {           t = term();result = operatorLSR(result, t);}
            else if(Expression::find("="))   {           t = term();result = operatorEQ(result,  t);}
            else if(Expression::find("<>"))  {           t = term();result = operatorNE(result,  t);}
            else if(Expression::find("<="))  {           t = term();result = operatorLE(result,  t);}
            else if(Expression::find(">="))  {           t = term();result = operatorGE(result,  t);}
            else if(peek(false) == '<')      {get(false);t = term();result = operatorLT(result,  t);}
            else if(peek(false) == '>')      {get(false);t = term();result = operatorGT(result,  t);}
            else return result;
        }
    }


    KeywordResult handleKeywords(CodeLine& codeLine, const std::string& keyword, int codeLineIndex, KeywordFuncResult& result)
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

    StatementResult createVasmCode(CodeLine& codeLine, int codeLineIndex)
    {
        // Check for subroutine start, make sure PUSH is emitted only once, even for multi-statement lines, (codeLine is a local copy of each statement within a multi-statement codeLine)
        if(!_codeLines[_currentCodeLineIndex]._pushEmitted  &&  codeLine._labelIndex >= 0  &&  _labels[codeLine._labelIndex]._gosub)
        {
            _codeLines[_currentCodeLineIndex]._pushEmitted = true;
            emitVcpuAsm("PUSH", "", false, codeLineIndex);
        }

        // Specific parsing requirments for most keywords, (*NOT* functions), some keywords like IF will also parse multi-statements
        for(int i=0; i<codeLine._tokens.size(); i++)
        {
            KeywordFuncResult result;
            KeywordResult keywordResult = handleKeywords(codeLine, codeLine._tokens[i], codeLineIndex, result);
            std::string token = codeLine._tokens[i];
            if(Expression::strToUpper(token)      == "REM"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "END"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "CLS"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "PUT"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "PRINT" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "ON"    ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "FOR"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "NEXT"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "GOTO"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "GOSUB" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "RETURN") return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "AT"    ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "MODE"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "WAIT"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "SCROLL") return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "LINE"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "HLINE" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "VLINE" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "DIM"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "DEF"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "POKE"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "DOKE"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "PLAY"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "IF"    ) return MultiStatementParsed;
        }

        bool containsVars = false;
        int varIndexRhs = -1, params = 0;
        uint32_t expressionType = isExpression(codeLine._expression, varIndexRhs, params);
        if(expressionType & Expression::HasVars) containsVars = true;

        int16_t value = 0;
        Expression::parse(codeLine._expression, codeLineIndex, value);
        if(codeLine._varIndex != -1)
        {
            updateVar(value, codeLine, codeLine._varIndex, containsVars);
        }

        // Check for matching brackets
        if(std::count(codeLine._expression.begin(), codeLine._expression.end(), '(') != std::count(codeLine._expression.begin(), codeLine._expression.end(), ')'))
        {
            fprintf(stderr, "Compiler::createVasmCode() : Brackets are not matched in '%s' on line %d\n", codeLine._expression.c_str(), codeLineIndex);
            return StatementError;
        }

        // TODO: only works with Int16, fix for all var types
        // Variable assignment
        if(codeLine._assignOperator)
        {
            // Assignment with a var expression
            if(codeLine._containsVars)
            {
                // Try and optimise LDW away if possible
                if(varIndexRhs >= 0  &&  _integerVars[varIndexRhs]._varType != VarArray  &&  !(expressionType & Expression::HasOperators))
                {
                    emitVcpuAsm("LDW", "_" + _integerVars[varIndexRhs]._name, false, codeLineIndex);
                }

                if(_integerVars[codeLine._varIndex]._varType == VarArray)
                {
                    writeArrayVar(codeLine, codeLineIndex, codeLine._varIndex);
                }
                else
                {
                    emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, codeLineIndex);
                }
            }
            // Standard assignment
            else
            {
                // Skip for functions
                if(params != 1)
                {
                    // 8bit constants
                    if(_integerVars[codeLine._varIndex]._data >=0  &&  _integerVars[codeLine._varIndex]._data <= 255)
                    {
                        emitVcpuAsm("LDI", std::to_string(_integerVars[codeLine._varIndex]._data), false, codeLineIndex);
                    }
                    // 16bit constants
                    else
                    {
                        emitVcpuAsm("LDWI", std::to_string(_integerVars[codeLine._varIndex]._data), false, codeLineIndex);
                    }
                }

                if(_integerVars[codeLine._varIndex]._varType == VarArray)
                {
                    writeArrayVar(codeLine, codeLineIndex, codeLine._varIndex);
                }
                else
                {
                    emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, codeLineIndex);
                }
            }
        }

        return StatementExpression;
    }

    StatementResult parseMultiStatements(const std::string& code, CodeLine& codeLine, int codeLineIndex, int& varIndex)
    {
        // Make a local copy, otherwise original tokens are destroyed
        CodeLine codeline = codeLine;

        // Tokenise and parse multi-statement lines
        StatementResult statementResult;
        std::vector<std::string> tokens = Expression::tokenise(code, ':', false);
        for(int j=0; j<tokens.size(); j++)
        {
            createCodeLine(tokens[j], 0, _codeLines[codeLineIndex]._labelIndex, -1, false, false, codeline);
            createCodeVar(codeline, codeLineIndex, varIndex);
            statementResult = createVasmCode(codeline, codeLineIndex);
            if(statementResult == StatementError) return StatementError;

            // Some commands, (such as IF), process multi-statements themselves
            if(statementResult == MultiStatementParsed) break;
        }

        return statementResult;
    }

    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool keywordREM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove REM and everything after it in code
        codeLine._code.erase(foundPos, codeLine._code.size() - foundPos);

        // Remove REM and everything after it in expression
        size_t rem;
        std::string expr = codeLine._expression;
        Expression::strToUpper(expr);
        if((rem = expr.find("REM")) != std::string::npos)
        {
            codeLine._expression.erase(rem, codeLine._expression.size() - rem);
        }

        // Remove REM and everything after it in tokens
        for(int i=0; i<codeLine._tokens.size(); i++)
        {
            std::string str = codeLine._tokens[i];
            Expression::strToUpper(str);
            if(str.find("REM") != std::string::npos)
            {
                codeLine._tokens.erase(codeLine._tokens.begin() + i, codeLine._tokens.end());
                break;
            }
        }

        return true;
    }

    bool keywordLET(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove LET from code
        codeLine._code.erase(foundPos, foundPos + 3);

        return true;
    }

    bool keywordEND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Label label;
        std::string endName = "_end_" + Expression::wordToHexString(_vasmPC);
        createLabel(_vasmPC, endName, "END\t", codeLineIndex, label, false, false, false, false);
        _codeLines[codeLineIndex]._labelIndex = _currentLabelIndex;
        emitVcpuAsm("BRA", endName, false, codeLineIndex);

        return true;
    }

    bool keywordON(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        int16_t onValue = 0;
        std::string onToken = codeLine._code.substr(foundPos, gOffset - (foundPos + 1));
        Expression::stripWhitespace(onToken);
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, onToken, onValue);
        emitVcpuAsm("STW", "register0", false, codeLineIndex);

        // Parse labels
        std::vector<size_t> gOffsets;
        std::vector<std::string> gotoTokens = Expression::tokenise(codeLine._code.substr(gOffset + gSize), ',', gOffsets, false);
        if(gotoTokens.size() < 1)
        {
            fprintf(stderr, "Compiler::keywordON() : Syntax error, must have at least one label after GOTO, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Create label LUT
        _codeLines[codeLineIndex]._onGotoGosubLut._lut.clear();
        for(int i=0; i<gotoTokens.size(); i++)
        {
            std::string gotoLabel = gotoTokens[i];
            Expression::stripWhitespace(gotoLabel);
            int labelIndex = findLabel(gotoLabel);
            if(labelIndex == -1)
            {
                fprintf(stderr, "Compiler::keywordON() : invalid label %s in slot %d in '%s' on line %d\n", gotoLabel.c_str(), i, codeLine._text.c_str(), codeLineIndex + 1);
                _codeLines[codeLineIndex]._onGotoGosubLut._lut.clear();
                return false;
            }
                
            // Only ON GOSUB needs a PUSH, (emitted in createVasmCode())
            if(gosubOffset != std::string::npos) _labels[labelIndex]._gosub = true;

            // Create lookup table out of label addresses
            _codeLines[codeLineIndex]._onGotoGosubLut._lut.push_back(labelIndex);
        }

        // Allocate giga memory for LUT
        int size = int(gotoTokens.size()) * 2;
        uint16_t address;
        if(!Memory::giveFreeRAM(Memory::FitAscending, size, 0x0200, 0x7FFF, address))
        {
            fprintf(stderr, "Compiler::keywordON() : Not enough RAM for onGotoGosub LUT of size %d\n", size);
            return false;
        }
        _codeLines[codeLineIndex]._onGotoGosubLut._address = address;
        _codeLines[codeLineIndex]._onGotoGosubLut._name = "lut_" + Expression::wordToHexString(address);

        emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
        emitVcpuAsm("STW",  "register0", false, codeLineIndex);
        emitVcpuAsm("LDWI", _codeLines[codeLineIndex]._onGotoGosubLut._name, false, codeLineIndex);
        emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
#ifdef ARRAY_INDICES_ONE
        emitVcpuAsm("SUBI", "2",         false, codeLineIndex);  // enable this to start at 1 instead of 0
#endif
        emitVcpuAsm("DEEK", "",          false, codeLineIndex);
        emitVcpuAsm("CALL", "giga_vAC",  false, codeLineIndex);

        return true;
    }

    bool keywordGOTO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        int16_t gotoValue = 0;
        std::string gotoToken = gotoTokens[0];
        Expression::stripWhitespace(gotoToken);
        int labelIndex = findLabel(gotoToken);
        if(labelIndex == -1)
        {
            _createNumericLabelLut = true;

            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, gotoToken, gotoValue);
            emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gotoTokens.size() == 2)
            {
                std::string defaultToken = gotoTokens[1];
                Expression::stripWhitespace(defaultToken);
                int labelIndex = findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Compiler::keywordGOTO() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                emitVcpuAsm("LDWI", "_" + _labels[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gotoNumericLabel
            if(Assembler::getUseOpcodeCALLI())
            {
                emitVcpuAsm("CALLI", "gotoNumericLabel", false, codeLineIndex);
            }
            else
            {
                emitVcpuAsm("LDWI", "gotoNumericLabel", false, codeLineIndex);
                emitVcpuAsm("CALL", "giga_vAC",         false, codeLineIndex);
            }

            return true;
        }

        // Within same page
        // TODO: Optimiser messes this strategy up, FIX IT
        if(0) //HI_MASK(_vasmPC) == HI_MASK(_labels[labelIndex]._address))
        {
            emitVcpuAsm("BRA", "_" + gotoToken, false, codeLineIndex);
        }
        // Long jump
        else
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                emitVcpuAsm("CALLI", "_" + gotoToken, false, codeLineIndex);
            }
            else
            {
                emitVcpuAsm("LDWI", "_" + gotoToken, false, codeLineIndex);
                emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
            }
        }

        return true;
    }

    void checkLinesForReturn(void)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            // Only check numbered lines
            if(isdigit(_labels[i]._name[0]))
            {
                int codeLineIndex = _labels[i]._codeLineIndex;
                std::string code = _codeLines[codeLineIndex]._code;
                Expression::strToUpper(code);
                if(code.find("RETURN") != std::string::npos) _labels[i]._gosub = true;
            }
        }
    }
    bool keywordGOSUB(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        int16_t gosubValue = 0;
        std::string gosubToken = gosubTokens[0];
        Expression::stripWhitespace(gosubToken);
        int labelIndex = findLabel(gosubToken);
        if(labelIndex == -1)
        {
            _createNumericLabelLut = true;

            // Check numbered label lines for a return
            checkLinesForReturn();

            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, gosubToken, gosubValue);
            emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gosubTokens.size() == 2)
            {
                std::string defaultToken = gosubTokens[1];
                Expression::stripWhitespace(defaultToken);
                int labelIndex = findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Compiler::keywordGOSUB() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
                    return false;
                }

                _labels[labelIndex]._gosub = true;
                emitVcpuAsm("LDWI", "_" + _labels[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gosubNumericLabel
            if(Assembler::getUseOpcodeCALLI())
            {
                emitVcpuAsm("CALLI", "gosubNumericLabel", false, codeLineIndex);
            }
            else
            {
                emitVcpuAsm("LDWI", "gosubNumericLabel", false, codeLineIndex);
                emitVcpuAsm("CALL", "giga_vAC",         false, codeLineIndex);
            }

            return true;
        }

        // CALL label
        _labels[labelIndex]._gosub = true;

        if(Assembler::getUseOpcodeCALLI())
        {
            emitVcpuAsm("CALLI", "_" + gosubToken, false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", "_" + gosubToken, false, codeLineIndex);
            emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
        }

        return true;
    }

    bool keywordRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        emitVcpuAsm("POP", "", false, codeLineIndex);
        emitVcpuAsm("RET", "", false, codeLineIndex);

        return true;
    }

    bool keywordCLS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(Assembler::getUseOpcodeCALLI())
        {
            emitVcpuAsm("CALLI", "clearVertBlinds", false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", "clearVertBlinds", false, codeLineIndex);
            emitVcpuAsm("CALL", "giga_vAC",        false, codeLineIndex);
        }

        return true;
    }

    bool keywordPOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordPOKE() : Syntax error, 'POKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<OperandType> operandTypes {OperandConst, OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i]);
        }

        if((operandTypes[0] == OperandVar  ||  operandTypes[0] == OperandTemp)  &&  (operandTypes[1] == OperandVar  ||  operandTypes[1] == OperandTemp))
        {
            (operandTypes[1] == OperandVar) ? emitVcpuAsm("LD", "_" + operands[1], false, codeLineIndex) : emitVcpuAsm("LD", "" + operands[1], false, codeLineIndex);
            (operandTypes[0] == OperandVar) ? emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : emitVcpuAsm("POKE", "" + operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == OperandVar  ||  operandTypes[0] == OperandTemp)  &&  operandTypes[1] == OperandConst)
        {
            emitVcpuAsm("LDI", operands[1], false, codeLineIndex);
            (operandTypes[0] == OperandVar) ? emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : emitVcpuAsm("POKE", "" + operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == OperandConst  &&  (operandTypes[1] == OperandVar  ||  operandTypes[1] == OperandTemp))
        {
            emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            emitVcpuAsm("STW", "register0", false, codeLineIndex);
            (operandTypes[1] == OperandVar) ? emitVcpuAsm("LD", "_" + operands[1], false, codeLineIndex) : emitVcpuAsm("LD", "" + operands[1], false, codeLineIndex);
            emitVcpuAsm("POKE", "register0", false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            emitVcpuAsm("STW",  "register0", false, codeLineIndex);
            emitVcpuAsm("LDI",  operands[1], false, codeLineIndex);
            emitVcpuAsm("POKE", "register0", false, codeLineIndex);
        }

        return true;
    }

    bool keywordDOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordDOKE() : syntax error, 'DOKE A,X', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<OperandType> operandTypes {OperandConst, OperandConst};

        for(int i=0; i<tokens.size(); i++)
        {
            operandTypes[i] = parseExpression(codeLine, codeLineIndex, tokens[i], operands[i]);
        }

        if((operandTypes[0] == OperandVar  ||  operandTypes[0] == OperandTemp)  &&  (operandTypes[1] == OperandVar  ||  operandTypes[1] == OperandTemp))
        {
            (operandTypes[1] == OperandVar) ? emitVcpuAsm("LDW", "_" + operands[1], false, codeLineIndex) : emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
            (operandTypes[0] == OperandVar) ? emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == OperandVar  ||  operandTypes[0] == OperandTemp)  &&  operandTypes[1] == OperandConst)
        {
            emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
            (operandTypes[0] == OperandVar) ? emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == OperandConst  &&  (operandTypes[1] == OperandVar  ||  operandTypes[1] == OperandTemp))
        {
            emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            emitVcpuAsm("STW", "register0", false, codeLineIndex);
            (operandTypes[1] == OperandVar) ? emitVcpuAsm("LDW", "_" + operands[1], false, codeLineIndex) : emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
            emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
            emitVcpuAsm("STW",  "register0", false, codeLineIndex);
            emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
            emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
        }

        return true;
    }

    bool keywordAT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() < 1  &&  tokens.size() > 2)
        {
            fprintf(stderr, "Compiler::keywordAT() : Syntax error, 'AT X' or 'AT X,Y', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<int16_t> params = {0, 0};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: emitVcpuAsm("ST", "cursorXY",     false, codeLineIndex); break;
                case 1: emitVcpuAsm("ST", "cursorXY + 1", false, codeLineIndex); break;
            }
        }

        if(Assembler::getUseOpcodeCALLI())
        {
            emitVcpuAsm("CALLI", "atTextCursor", false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", "atTextCursor", false, codeLineIndex);
            emitVcpuAsm("CALL", "giga_vAC",     false, codeLineIndex);
        }

        return true;
    }

    bool keywordPUT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordPUT() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        int16_t param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        emitVcpuAsm("%PrintAcChar", "", false, codeLineIndex);

        return true;
    }

    bool keywordMODE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordMODE() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        int16_t param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        emitVcpuAsm("STW", "graphicsMode", false, codeLineIndex);
        emitVcpuAsm("%ScanlineMode", "",   false, codeLineIndex);

        return true;
    }

    bool keywordWAIT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::string expression = codeLine._code.substr(foundPos);
        if(expression.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordWAIT() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        int16_t param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, expression, param);
        emitVcpuAsm("STW", "waitVBlankNum", false, codeLineIndex);
        emitVcpuAsm("%WaitVBlank", "",      false, codeLineIndex);

        return true;
    }

    bool keywordLINE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
            std::vector<int16_t> params = {0, 0};
            for(int i=0; i<tokens.size(); i++)
            {
                uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 1: emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;
                }
            }

            emitVcpuAsm("%AtLineCursor", "", false, codeLineIndex);
        }
        else
        {
            std::vector<int16_t> params = {0, 0, 0, 0};
            for(int i=0; i<tokens.size(); i++)
            {
                uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: emitVcpuAsm("STW", "drawLine_x1", false, codeLineIndex); break;
                    case 1: emitVcpuAsm("STW", "drawLine_y1", false, codeLineIndex); break;
                    case 2: emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 3: emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;
                }
            }
        }

        emitVcpuAsm("%DrawLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordHLINE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() !=3)
        {
            fprintf(stderr, "Compiler::keywordHLINE() : Syntax error, 'HLINE X1,Y,X2', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<int16_t> params = {0, 0, 0};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: emitVcpuAsm("STW", "drawHLine_x1", false, codeLineIndex); break;
                case 1: emitVcpuAsm("STW", "drawHLine_y1", false, codeLineIndex); break;
                case 2: emitVcpuAsm("STW", "drawHLine_x2", false, codeLineIndex); break;
            }
        }

        emitVcpuAsm("%DrawHLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordVLINE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', offsets, false);
        if(tokens.size() !=3)
        {
            fprintf(stderr, "Compiler::keywordVLINE() : Syntax error, 'VLINE X1,Y,X2', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::vector<int16_t> params = {0, 0, 0};
        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: emitVcpuAsm("STW", "drawVLine_x1", false, codeLineIndex); break;
                case 1: emitVcpuAsm("STW", "drawVLine_y1", false, codeLineIndex); break;
                case 2: emitVcpuAsm("STW", "drawVLine_y2", false, codeLineIndex); break;
            }
        }

        emitVcpuAsm("%DrawVLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordPLAY(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', offsets, false);
        if(tokens.size() != 2)
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

        std::string addressToken = tokens[1];
        Expression::stripWhitespace(addressToken);
        int16_t param;
        uint32_t expressionType = parseExpression(codeLine, codeLineIndex, addressToken, param);

        emitVcpuAsm("%PlayMidi", "", false, codeLineIndex);

        return true;
    }

    bool keywordSCROLL(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
            emitVcpuAsm("LDWI", "0x0001", false, codeLineIndex);
            emitVcpuAsm("ORW", "miscFlags", false, codeLineIndex);
        }
        else
        {
            emitVcpuAsm("LDWI", "0xFFFE", false, codeLineIndex);
            emitVcpuAsm("ANDW", "miscFlags", false, codeLineIndex);
        }
        emitVcpuAsm("STW", "miscFlags", false, codeLineIndex);

        return true;
    }

    bool keywordPRINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse print tokens
        std::vector<std::string> tokens = Expression::tokeniseLine(codeLine._code.substr(foundPos), ";");

        int16_t value;
        int varIndex, params;

        for(int i=0; i<tokens.size(); i++)
        {
            uint32_t expressionType = isExpression(tokens[i], varIndex, params);

            if(expressionType & Expression::HasKeywords)
            {
                Expression::setEnablePrint(true);
                Expression::parse(tokens[i], codeLineIndex, value);
                Expression::setEnablePrint(false);
            }
            else if((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                emitVcpuAsm("%PrintVarInt16", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
            }
            else if((expressionType & Expression::HasVars)  &&  !(expressionType & Expression::HasStrings))
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                if(varIndex >= 0)
                {
                    if(_integerVars[varIndex]._varType == VarArray)
                    {
                        emitVcpuAsm("%PrintVarInt16", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
                    }
                    else
                    {
                        emitVcpuAsm("%PrintVarInt16", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
                    }
                }
                else
                {
                    emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasOperators)
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value), false, codeLineIndex);
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
                    if(!createString(codeLine, codeLineIndex, str, name, address)) return false;

                    // Print string
                    emitVcpuAsm("%PrintString", name, false, codeLineIndex);
                }
                else
                {
                    if(varIndex >= 0  &&  _integerVars[varIndex]._name.find("$") != std::string::npos)
                    {
                        emitVcpuAsm("%PrintVarString", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
                    }
                }
            }
            else if(expressionType == Expression::HasNumbers)
            {
                // If valid expression
                if(Expression::parse(tokens[i], codeLineIndex, value))
                {
                    emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value), false, codeLineIndex);
                }
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                emitVcpuAsm("CALLI", "newLineScroll", false, codeLineIndex);
            }
            else
            {
                emitVcpuAsm("LDWI", "newLineScroll", false, codeLineIndex);
                emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordFOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        int varIndex = findVar(var);
        (varIndex < 0) ? createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varIndex) : updateVar(loopStart, codeLine, varIndex, false);

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
        if(_forNextDataStack.size() == 4)
        {
            fprintf(stderr, "Compiler::keywordFOR() : Syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Nested loops temporary variables
        int offset = int(_forNextDataStack.size()) * 4;
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
                emitVcpuAsm("LDI", std::to_string(loopStart), false, codeLineIndex);
                emitVcpuAsm("STW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
            }
        }

        // General purpose supports 16bit, expressions and variables
        if(!optimise)
        {
            // Parse start field
            uint32_t expressionType = parseExpression(codeLine, codeLineIndex, startToken, loopStart, 0);
            emitVcpuAsm("STW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);

            // Parse end field
            expressionType = parseExpression(codeLine, codeLineIndex, endToken, loopEnd, 0);
            emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);

            // Parse step field
            int16_t replace = 1; // if step is 0, replace it with 1
            expressionType = parseExpression(codeLine, codeLineIndex, stepToken, loopStep, replace);
            emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
        }

        _nextInternalLabel = "_next_" + Expression::wordToHexString(_vasmPC);
        _forNextDataStack.push({varIndex, _nextInternalLabel, loopEnd, loopStep, varEnd, varStep, optimise});

        return true;
    }

    bool keywordNEXT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : Syntax error, (bad var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Pop stack for this nested loop
        if(_forNextDataStack.empty()) return false;
        ForNextData forNextData = _forNextDataStack.top();
        _forNextDataStack.pop();

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
                emitVcpuAsm("%ForNextLoopUp", "_" + _integerVars[varIndex]._name + " " + forNextData._labelName + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex);
            }
            // Negative step
            else
            {
                emitVcpuAsm("%ForNextLoopDown", "_" + _integerVars[varIndex]._name + " " + forNextData._labelName + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex);
            }
        }
        else
        {
            // Positive step
            if(forNextData._loopStep > 0)
            {
                emitVcpuAsm("%ForNextLoopStepUp", "_" + _integerVars[varIndex]._name + " " + forNextData._labelName + " " + Expression::byteToHexString(uint8_t(forNextData._varEnd)) + " " + Expression::byteToHexString(uint8_t(forNextData._varStep)), false, codeLineIndex);
            }
            else
            {
                emitVcpuAsm("%ForNextLoopStepDown", "_" + _integerVars[varIndex]._name + " " + forNextData._labelName + " " + Expression::byteToHexString(uint8_t(forNextData._varEnd)) + " " + Expression::byteToHexString(uint8_t(forNextData._varStep)), false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // IF
        std::string code = _codeLines[codeLineIndex]._code;
        Expression::strToUpper(code);
        size_t offsetIF = code.find("IF");

        // THEN
        code = codeLine._code;
        Expression::strToUpper(code);
        size_t offsetTHEN = code.find("THEN");
        if(offsetTHEN == std::string::npos)
        {
            fprintf(stderr, "Compiler::keywordIF() : Syntax error, THEN missing, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Condition
        int16_t condition = 0;
        std::string conditionToken = codeLine._code.substr(foundPos, offsetTHEN - foundPos);
        parseExpression(codeLine, codeLineIndex, conditionToken, condition);
        emitVcpuAsm("%PageJumpBEQ", "", false, codeLineIndex);
        int indexBEQ = int(_codeLines[codeLineIndex]._vasm.size()) - 1;

        // Action
        std::string actionToken = _codeLines[codeLineIndex]._code.substr(offsetIF + offsetTHEN + 4);
        if(actionToken.size() == 0)
        {
            fprintf(stderr, "Compiler::keywordIF() : Syntax error, IF THEN <action>, (missing action), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        Expression::trimWhitespace(actionToken);
        std::string actionText = Expression::collapseWhitespaceNotStrings(actionToken);

        // Multi-statements
        int varIndex;
        if(parseMultiStatements(actionText, codeLine, codeLineIndex, varIndex) == StatementError) return false;

        // Update branch's label, (check to see if a label already exists)
        _nextInternalLabel = "_if_" + Expression::wordToHexString(_vasmPC);
        _codeLines[codeLineIndex]._vasm[indexBEQ]._code = "PageJumpBEQ" + std::string(OPCODE_TRUNC_SIZE - (sizeof("PageJumpBEQ")-1), ' ') + _nextInternalLabel;

        return true;
    }

    bool keywordENDIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordELSE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordELSEIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordDIM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
        int varIndex = findVar(varName);
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

        createIntVar(varName, 0, varInit, codeLine, codeLineIndex, false, varIndex, VarArray, arrayStart, VarInt16, arraySize);

        return true;
    }

    bool keywordDEF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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
                _defDataBytes.push_back({address, dataBytes});
                if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
            }
            // Update current address data
            else
            {
                // Append data
                address = _defDataBytes.back()._address + uint16_t(_defDataBytes.back()._data.size());
                for(int i=0; i<dataBytes.size(); i++)
                {
                    _defDataBytes.back()._data.push_back(dataBytes[i]);
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
                _defDataWords.push_back({address, dataWords});
                if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
            }
            // Update current address data
            else
            {
                // Append data
                address = _defDataWords.back()._address + uint16_t(_defDataWords.back()._data.size()) * 2;
                for(int i=0; i<dataWords.size(); i++)
                {
                    _defDataWords.back()._data.push_back(dataWords[i]);
                }

                // Mark new RAM chunk as used
                if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
            }
        }

        return true;
    }

    bool keywordINPUT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordREAD(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordDATA(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordDO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordLOOP(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordWHILE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordUNTIL(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }


    bool parseCode(void)
    {
        size_t foundPos;
        CodeLine codeLine;

        // REM and LET modify code
        for(int i=0; i<_codeLines.size(); i++)
        {
            KeywordFuncResult result;

            // ' is a shortcut for REM
            if((foundPos = _codeLines[i]._code.find_first_of('\'')) != std::string::npos  ||  findKeyword(_codeLines[i]._code, "REM", foundPos))
            {
                keywordREM(_codeLines[i], 0, foundPos, result);
            }
            else if(findKeyword(_codeLines[i]._code, "LET", foundPos))
            {
                keywordLET(_codeLines[i], 0, foundPos - 3, result);
            }
        }

        // Add END to code
        if(_codeLines.size()  &&  _codeLines[_codeLines.size() - 1]._code.find("END") == std::string::npos)
        {
            if(createCodeLine("END", 0, -1, -1, false, false, codeLine)) _codeLines.push_back(codeLine);
        }

        // Parse code creating vars and vasm code, (BASIC code lines were created in ParseLabels())
        int varIndex;
        for(int i=0; i<_codeLines.size(); i++)
        {
            _currentCodeLineIndex = i;

            // First line of BASIC code is always a dummy INIT line, ignore it
            if(i > 0  &&  _codeLines[i]._code.size() > 2)
            {
                // Adjust label address
                if(_codeLines[i]._labelIndex >= 0) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;

                // Multi-statements
                StatementResult statementResult = parseMultiStatements(_codeLines[i]._code, codeLine, i, varIndex);
                if(statementResult == StatementError) return false;
            }
        }

        return true;
    }

    // Migrate internal label for an instruction that has been deleted, (use it before the instuction is deleted)
    bool migrateInternalLabel(int index, int oldLine, int newLine)
    {
        // If a label exists, move it to next available vasm line
        if(_codeLines[index]._vasm[oldLine]._internalLabel.size())
        {
            // Next available vasm line is part of a new BASIC line, so can't optimise
            if(_codeLines[index]._vasm.size() <= newLine) return false;
            _codeLines[index]._vasm[newLine]._internalLabel = _codeLines[index]._vasm[oldLine]._internalLabel;
        }
    
        return true;
    }

    bool optimiseCode(void)
    {
        enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwPair, ExtraStw, ExtraLdw, StwLdwAddw, StwLdwAddwVar, StwLdwAndw, StwLdwAndwVar, StwLdwXorw, StwLdwXorwVar, StwLdwOrw, StwLdwOrwVar, AddiZero, SubiZero, NumOptimiseTypes};
        struct MatchSequence
        {
            int firstIndex;
            int secondIndex;
            std::vector<std::string> _sequence;
        };

        std::vector<MatchSequence> matchSequences = 
        {
            {0, 1, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"                                                         }},
            {0, 1, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LD"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0x"                                                         }},
            {0, 1, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"                                                         }},
            {0, 1, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"                                                          }},
            {0, 1, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"                                                          }},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "ORW"  + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  "ORW"  + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},
            {0, 0, {"ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ')       , ""                                                                                                             }},
            {0, 0, {"SUBI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ')       , ""                                                                                                             }},
        };

        for(int i=0; i<_codeLines.size(); i++)
        {
            for(int j=0; j<matchSequences.size(); j++)
            {
                for(auto itVasm=_codeLines[i]._vasm.begin(); itVasm!=_codeLines[i]._vasm.end();)
                {
                    bool linesDeleted = false;
                    int vasmIndex = int(itVasm - _codeLines[i]._vasm.begin());

                    // Can only optimise within a BASIC code line
                    int vasmIndexMax = vasmIndex + int(matchSequences[j]._sequence.size()) - 1;
                    if(vasmIndexMax >= _codeLines[i]._vasm.size())
                    {
                        ++itVasm;
                        continue;
                    }

                    // Find opcode match
                    bool foundOpcodeMatch = true;
                    for(int k=vasmIndex; k<=vasmIndexMax; k++)
                    {
                        if(_codeLines[i]._vasm[k]._code.find(matchSequences[j]._sequence[k - vasmIndex]) ==  std::string::npos)
                        {
                            foundOpcodeMatch = false;
                            break;
                        }
                    }

                    linesDeleted = false;
                    if(foundOpcodeMatch)
                    {
                        // First operand
                        int firstIndex = matchSequences[j].firstIndex;
                        int firstLine = vasmIndex + firstIndex;
                        size_t firstSpace = _codeLines[i]._vasm[firstLine]._code.find_first_of("  \n\r\f\t\v");
                        std::string firstOperand = _codeLines[i]._vasm[firstLine]._code.substr(firstSpace);
                        Expression::stripWhitespace(firstOperand);

                        // Second operand
                        int secondIndex = matchSequences[j].secondIndex;
                        int secondLine = vasmIndex + secondIndex;
                        size_t secondSpace = _codeLines[i]._vasm[secondLine]._code.find_first_of("  \n\r\f\t\v");
                        std::string secondOperand = _codeLines[i]._vasm[secondLine]._code.substr(secondSpace);
                        Expression::stripWhitespace(secondOperand);

                        // Find operand match
                        if(firstOperand == secondOperand)
                        {
                            // Match STW LDW, delete STW LDW
                            if(j == StwLdwPair  ||  j == StwLdPair)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            
                            // Match STW LDW, delete LDW
                            else if(j == ExtraLdw)
                            {
                                // Migrate internal label to next available instruction
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // Delete LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine + 1]._address, -2);
                                adjustVasmAddresses(i, firstLine + 1, -2);
                            }

                            // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW
                            else if(j == StwLdwAddw)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ADDW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwAddwVar)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ADDW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW
                            else if(j == StwLdwAndw)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ANDW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwAndwVar)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ANDW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW
                            else if(j == StwLdwXorw)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // XORW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwXorwVar)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // XORW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW
                            else if(j == StwLdwOrw)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ORW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwOrwVar)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ORW's operand becomes the LDW's operand
                                size_t ldwSpace = _codeLines[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = _codeLines[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                _codeLines[i]._vasm[firstLine + 2]._code = "ORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                        }

                        // Extra STW, (doesn't require an operand match)
                        if(j == StwPair  ||  j == ExtraStw)
                        {
                            // Migrate internal label to next available instruction
                            if(!migrateInternalLabel(i, firstLine, firstLine + 1)) break;

                            // Delete first STW
                            linesDeleted = true;
                            itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                            adjustLabelAddresses(_codeLines[i]._vasm[firstLine]._address, -2);
                            adjustVasmAddresses(i, firstLine, -2);
                        }
                    }

                    // Arithmetic with zero, (doesn't require an opcode match)
                    if(j == AddiZero  ||  j == SubiZero)
                    {
                        std::string operand;
                        size_t pos = itVasm->_code.find(matchSequences[j]._sequence[0]);
                        if(pos != std::string::npos)
                        {
                            operand = itVasm->_code.substr(pos + matchSequences[j]._sequence[0].size());
                            if(operand == "0" || operand == "0x00")
                            {
                                // Migrate internal label to next available instruction
                                if(!migrateInternalLabel(i, vasmIndex, vasmIndex + 1)) break;

                                // Delete ADD/SUB
                                linesDeleted = true;
                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + vasmIndex);
                                adjustLabelAddresses(_codeLines[i]._vasm[vasmIndex]._address, -2);
                                adjustVasmAddresses(i, vasmIndex, -2);
                            }
                        }
                    }

                    if(!linesDeleted) ++itVasm;
                }
            }
        }

        return true;
    }

    void adjustExclusionLabelAddresses(uint16_t address, int offset)
    {
        // Adjust addresses for any non page jump labels with addresses higher than start label, (labels can be stored out of order)
        for(int i=0; i<_labels.size(); i++)
        {
            if(!_labels[i]._pageJump  &&  _labels[i]._address >= address)
            {
                _labels[i]._address += offset;
            }
        }
    }

    void adjustExclusionVasmAddresses(int codeLineIndex, uint16_t address, uint16_t page, int offset)
    {
        for(int i=codeLineIndex; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                // Don't adjust page jump's
                if(!_codeLines[i]._vasm[j]._pageJump  &&  _codeLines[i]._vasm[j]._address >= address)
                {
                    _codeLines[i]._vasm[j]._address += offset;
                }
            }
        }
    }

    auto insertPageJumpInstruction(std::vector<Compiler::CodeLine>::iterator& itCode, std::vector<Compiler::VasmLine>::iterator& itVasm,
                                   const std::string& opcode, const std::string& code, uint16_t address, int vasmSize)
    {
        if(itVasm >= itCode->_vasm.end())
        {
            fprintf(stderr, "Compiler::insertPageJumpInstruction() : Trying to insert a PAGE JUMP into lala land, in '%s'", itCode->_code.c_str());
            _EXIT_(EXIT_FAILURE);
        }

        return itCode->_vasm.insert(itVasm, {address, opcode, code, "", true, vasmSize});
    }

    bool checkExclusionZone(const std::string& opcode, uint16_t vPC, uint16_t& nextPC)
    {
        uint8_t hPC = HI_BYTE(vPC);
        uint16_t audioExcl = (hPC <<8) + 0x00F8;
        uint16_t pageExcl  = (hPC <<8) + 0x00FE;

        nextPC = (hPC + 1) <<8;

        // Code page restrictions
        uint16_t codeEnd0 = 0x0300;
        uint16_t codePage1 = 0x0800;
        uint16_t codeOffs1 = 0x00A0;
        uint16_t codePage2 = 0x8000; // only available with 64K RAM
        if(nextPC == codeEnd0)
        {
            nextPC = codePage1 + codeOffs1;
        }
        else
        {
            if(nextPC >= codePage1  &&  nextPC < codePage2) nextPC += codeOffs1;
        }
        // Allow some padding for late linking of the runtime
        if(nextPC >= (_runtimeEnd - 0x1000)  &&  nextPC < codePage2) nextPC = codePage2;

        // 3 bytes for CALLI PAGE JUMP
        if(Assembler::getUseOpcodeCALLI())
        {
            audioExcl -= 3;
            pageExcl  -= 3;
        }
        // 7 bytes for non CALLI PAGE JUMP, (STW, LDWI, CALL)
        else
        {
            audioExcl -= 7;
            pageExcl  -= 7;
        }

        // Adjust for MACRO size
        std::string macro = opcode;
        if(macro.size()  &&  macro[0] == '%')
        {
            macro.erase(0, 1);

            if(_macroIndexEntries.find(macro) != _macroIndexEntries.end())
            {
                int macroSize = _macroIndexEntries[macro]._byteSize;
                audioExcl -= macroSize;
                pageExcl -= macroSize;
            }
        }

        return (vPC >= pageExcl  ||  ((hPC == 0x02 || hPC == 0x03 || hPC == 0x04)  &&  vPC >= audioExcl));
    }

    bool checkExclusionZones(void)
    {
        std::string line;

        bool resetCheck = true;

        // Each time any excluded area code is fixed, restart check
        while(resetCheck)
        {
            for(auto itCode=_codeLines.begin(); itCode!=_codeLines.end();)
            {
                if(itCode->_vasm.size() == 0)
                {
                    itCode++;
                    continue;
                }

                int codeLineIndex = int(itCode - _codeLines.begin());

                for(auto itVasm=itCode->_vasm.begin(); itVasm!=itCode->_vasm.end();)
                {
                    resetCheck = false;
                    int vasmLineIndex = int(itVasm - itCode->_vasm.begin());

                    uint16_t nextPC;
                    bool excluded = checkExclusionZone(itVasm->_opcode, itVasm->_address, nextPC);

                    if(!itVasm->_pageJump  &&  excluded)
                    {
                        std::vector<std::string> tokens;
                        uint16_t currPC = (vasmLineIndex > 0) ? itCode->_vasm[vasmLineIndex-1]._address : itVasm->_address;

                        // Insert PAGE JUMP
                        int restoreOffset = 0;
                        auto it = (vasmLineIndex > 0) ? itVasm-1 : itVasm;
                        int index = (vasmLineIndex > 0) ? vasmLineIndex-1 : vasmLineIndex;
                        std::string nextPClabel = "_page_" + Expression::wordToHexString(nextPC);
                        if(Assembler::getUseOpcodeCALLI())
                        {
                            // CALLI PAGE JUMP
                            std::string codeCALLI;
                            int sizeCALLI = createVcpuAsm("CALLI", nextPClabel, codeLineIndex, codeCALLI);
                            itVasm = itCode->_vasm.insert((vasmLineIndex > 0) ? itVasm-1 : itVasm, {currPC, "CALLI", codeCALLI, "", true, sizeCALLI});
                        }
                        else
                        {
                            // ROMS that don't have CALLI, (save and restore vAC)
                            std::string codeSTW, codeLDWI, codeCALL, codeLDW;
                            int sizeSTW  = createVcpuAsm("STW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeSTW);
                            int sizeLDWI = createVcpuAsm("LDWI", nextPClabel, codeLineIndex, codeLDWI);
                            int sizeCALL = createVcpuAsm("CALL", "giga_vAC", codeLineIndex, codeCALL);
                            int sizeLDW  = createVcpuAsm("LDW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeLDW);
                            itVasm = itCode->_vasm.insert((vasmLineIndex > 0) ? itVasm-1 : itVasm, {currPC, "STW", codeSTW, "", true, sizeSTW});
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDWI", codeLDWI, uint16_t(currPC + sizeSTW), sizeLDWI);
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "CALL", codeCALL, uint16_t(currPC + sizeSTW + sizeLDWI), sizeCALL);
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDW", codeLDW, uint16_t(nextPC), sizeLDW);

                            // New page address is offset by size of vAC restore
                            restoreOffset = sizeLDW;
                        }

                        // Fix labels and addresses
                        int offset = nextPC + restoreOffset - currPC;
                        adjustExclusionLabelAddresses(currPC, offset);
                        adjustExclusionVasmAddresses(codeLineIndex, currPC, nextPC + restoreOffset, offset);

                        // Check for existing label, (after label adjustments)
                        int labelIndex = -1;
                        std::string labelName;
                        VasmLine* vasm0 = &itCode->_vasm[itVasm - itCode->_vasm.begin()]; // points to CALLI and LDW
                        VasmLine* vasm1 = &itCode->_vasm[itVasm + 1 - itCode->_vasm.begin()]; // points to instruction after CALLI and after LDW
                        if(findLabel(nextPC) >= 0)
                        {
                            labelIndex = findLabel(nextPC);
                            labelName = _labels[labelIndex]._name;
                        }
                        if(labelIndex == -1)
                        {
                            // Create CALLI page jump label, (created later in outputCode())
                            if(Assembler::getUseOpcodeCALLI())
                            {
                                // Code referencing these labels must be fixed later in outputLabels
                                if(vasm1->_internalLabel.size()) _discardedLabels.push_back({vasm1->_address, vasm1->_internalLabel});
            
                                vasm1->_internalLabel = nextPClabel;
                            }
                            // Create pre-CALLI page jump label, (created later in outputCode())
                            else
                            {
                                // Code referencing these labels must be fixed later in outputLabels
                                if(vasm0->_internalLabel.size()) _discardedLabels.push_back({vasm0->_address, vasm0->_internalLabel});

                                vasm0->_internalLabel = nextPClabel;
                            }
                        }
                        // Existing label at the PAGE JUMP address, so use it
                        else
                        {
                            // Update CALLI page jump label
                            if(Assembler::getUseOpcodeCALLI())
                            {
                                // Macro labels are underscored by default
                                vasm0->_code = (labelName[0] == '_') ? "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                            }
                            // Update pre-CALLI page jump label
                            else
                            {
                                // Macro labels are underscored by default
                                VasmLine* vasm = &itCode->_vasm[itVasm - 2 - itCode->_vasm.begin()]; // points to LDWI
                                vasm->_code = (labelName[0] == '_') ? "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                            }
                        }

                        // Restart checking procedure
                        resetCheck = true;
                        break;
                    }

                    itVasm++;
                }
            
                // Restart checking procedure
                if(resetCheck) break;

                itCode++;
            }
        }

        return true;
    }

    bool checkBranchLabels(void)
    {
        for(int i=0; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                uint16_t vasmAddress = _codeLines[i]._vasm[j]._address;
                std::string vasmCode = _codeLines[i]._vasm[j]._code;
                std::string vasmLabel = _codeLines[i]._vasm[j]._internalLabel;

                if(vasmCode == "BRA")
                {
                    size_t space = vasmCode.find_first_of("  \n\r\f\t\v");
                    std::string operand = vasmCode.substr(space);

                    // Is operand a label?
                    int labelIndex = findLabel(operand);
                    if(labelIndex >= 0)
                    {
                        if(HI_MASK(vasmAddress) != HI_MASK(_labels[labelIndex]._address))
                        {
                            fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", _labels[labelIndex]._address, vasmAddress, vasmCode.c_str(), i);
                            return false;
                        }
                    }
                    else if(vasmLabel.size())
                    // Check internal label
                    {
                        int labelIndex = findInternalLabel(operand);
                        if(labelIndex >= 0)
                        {
                            if(HI_MASK(vasmAddress) != HI_MASK(_internalLabels[labelIndex]._address))
                            {
                                fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", _internalLabels[labelIndex]._address, vasmAddress, vasmCode.c_str(), i);
                                return false;
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    void outputReservedWords(void)
    {
        std::string line = "_startAddress_ ";
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        line += "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(USER_CODE_START) + "\n";
        _output.push_back(line);
    }

    void outputLabels(void)
    {
        std::string line;

        _output.push_back("; Labels\n");

        // BASIC labels
        for(int i=0; i<_labels.size(); i++)
        {
            std::string address = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
            _equateLabels.push_back(_labels[i]._name);
        }

        // Internal labels
        for(int i=0; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                std::string internalLabel = _codeLines[i]._vasm[j]._internalLabel;
                if(internalLabel.size())
                {
                    std::string address = Expression::wordToHexString(_codeLines[i]._vasm[j]._address);
                    _output.push_back(internalLabel + std::string(LABEL_TRUNC_SIZE - internalLabel.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
                    _internalLabels.push_back({_codeLines[i]._vasm[j]._address, internalLabel});
                    _equateLabels.push_back(internalLabel);
                }
            }
        }

        // Check for label conflicts
        for(int i=0; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                // BASIC label conflict
                for(int k=0; k<_internalLabels.size(); k++)
                {
                    int labelIndex = findLabel(_internalLabels[k]._address);
                    if(labelIndex >= 0)
                    {   
                        std::string basicLabel = _labels[labelIndex]._output;
                        Expression::stripWhitespace(basicLabel);
                        Expression::replaceText(_codeLines[i]._vasm[j]._code, _internalLabels[k]._name, basicLabel);
                    }

                    // Discarded internal label
                    for(int l=0; l<_discardedLabels.size(); l++)
                    {
                        if(_internalLabels[k]._address == _discardedLabels[l]._address)
                        {
                            Expression::replaceText(_codeLines[i]._vasm[j]._code, _discardedLabels[l]._name, _internalLabels[k]._name);
                        }
                    }
                }
            }
        }

        _output.push_back("\n");
    }

    void outputVars(void)
    {
        _output.push_back("; Variables\n");

        for(int i=0; i<_integerVars.size(); i++)
        {
            // Int var
            if(_integerVars[i]._varType == VarInt16)
            {
                std::string address = Expression::wordToHexString(_integerVars[i]._address);
                _output.push_back(_integerVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
            }
            // Array var
            else if(_integerVars[i]._varType == VarArray)
            {
                std::string arrName = "_" + _integerVars[i]._name + "_array";
                _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[i]._array) + "\n");
            
                std::string dbString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<_integerVars[i]._arrSize/2; j++)
                {
                    dbString += Expression::wordToHexString(_integerVars[i]._init) + " ";
                }
                _output.push_back(dbString + "\n");
            }
        }

        _output.push_back("\n");
    }

    void outputStrs(void)
    {
        _output.push_back("; Strings\n");

        for(int i=0; i<_stringVars.size(); i++)
        {
            _output.push_back(_stringVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._address) + "\n");
            _output.push_back(_stringVars[i]._output + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(_stringVars[i]._size) + " '" + _stringVars[i]._data + "'\n");
        }

        _output.push_back("\n");
    }

    bool outputDefs(void)
    {
        _output.push_back("; Define Bytes\n");

        // Create def byte data
        for(int i=0; i<_defDataBytes.size(); i++)
        {
            std::string defName = "def_bytes_" + Expression::wordToHexString(_defDataBytes[i]._address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_defDataBytes[i]._address) + "\n");
            
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<_defDataBytes[i]._data.size(); j++)
            {
                dbString += std::to_string(_defDataBytes[i]._data[j]) + " ";
            }
            _output.push_back(dbString + "\n");
        }

        // Create def word data
        for(int i=0; i<_defDataWords.size(); i++)
        {
            std::string defName = "def_words_" + Expression::wordToHexString(_defDataWords[i]._address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_defDataWords[i]._address) + "\n");
            
            std::string dwString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<_defDataWords[i]._data.size(); j++)
            {
                dwString += std::to_string(_defDataWords[i]._data[j]) + " ";
            }
            _output.push_back(dwString + "\n");
        }

        _output.push_back("\n");

        return true;
    }

    bool outputLuts(void)
    {
        _output.push_back("; Lookup Tables\n");

        // GOTO GOSUB with vars
        if(_createNumericLabelLut)
        {
            std::vector<uint16_t> numericLabels;
            std::vector<uint16_t> numericAddresses;
            for(int i=0; i<_labels.size(); i++)
            {
                if(_labels[i]._isNumeric)
                {
                    uint16_t numericLabel;
                    if(!Expression::stringToU16(_labels[i]._name, numericLabel))
                    {
                        fprintf(stderr, "Compiler::outputLuts() : bad numeric label %s : %04x\n", _labels[i]._name.c_str(), numericLabel);
                        return false;
                    }

                    numericLabels.push_back(numericLabel);
                    numericAddresses.push_back(_labels[i]._address);
                }
            }

            // Create numeric labels lut
            if(numericLabels.size())
            {
                // Numeric labels lut, (delimited by -1)
                int lutSize = int(numericLabels.size()) * 2;
                uint16_t lutAddress;
                if(!Memory::giveFreeRAM(Memory::FitAscending, lutSize + 2, 0x0200, 0x7FFF, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : Not enough RAM for numeric labels LUT of size %d\n", lutSize + 2);
                    return false;
                }

                std::string lutName = "lut_numericLabs";
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dbString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<numericLabels.size(); i++)
                {
                    dbString += std::to_string(numericLabels[i]) + " ";
                }
                _output.push_back(dbString + "-1\n");

                // Numeric label addresses lut
                if(!Memory::giveFreeRAM(Memory::FitAscending, lutSize, 0x0200, 0x7FFF, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : Not enough RAM for numeric addresses LUT of size %d\n", lutSize);
                    return false;
                }

                lutName = "lut_numericAddrs";
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<numericAddresses.size(); i++)
                {
                    dwString += Expression::wordToHexString(numericAddresses[i]) + " ";
                }
                _output.push_back(dwString + "\n");
            }
        }

        // ON GOTO GOSUB labels
        for(int i=0; i<_codeLines.size(); i++)
        {
            // Output LUT if it exists
            int lutSize = int(_codeLines[i]._onGotoGosubLut._lut.size());
            uint16_t lutAddress = _codeLines[i]._onGotoGosubLut._address;
            std::string lutName = _codeLines[i]._onGotoGosubLut._name;
            if(lutSize)
            {
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dbString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<lutSize; j++)
                {
                    int index = _codeLines[i]._onGotoGosubLut._lut[j];
                    if(index == -1) fprintf(stderr, "Compiler::outputLuts() : Warning, label index is invalid for LUT entry %d\n", j);

                    uint16_t labelAddress = _labels[index]._address;
                    dbString += Expression::wordToHexString(labelAddress) + " ";
                }
                _output.push_back(dbString + "\n");
            }
        }

        _output.push_back("\n");

        return true;
    }


    int getAsmOpcodeSizeSubInFile(const std::string& filename, const std::string& subname)
    {
        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::getAsmOpcodeSizeSubInFile() : Failed to open file : '%s'\n", filename.c_str());
            return -1;
        }

        // Get file
        int numLines = 0;
        int vasmSize = 0;
        bool buildingSub = false;
        Assembler::LineToken lineToken;

        while(!infile.eof())
        {
            std::getline(infile, lineToken._text);
            std::vector<std::string> tokens = Expression::tokeniseLine(lineToken._text);
            if(!buildingSub  &&  tokens.size() >= 2  &&  tokens[0] == "%SUB"  &&  tokens[1] == subname)
            {
                buildingSub = true;
            }
            else if(buildingSub  &&  tokens.size() >= 1  &&  tokens[0] == "%ENDS")
            {
                buildingSub = false;
                break;
            }
            else if(buildingSub)
            {
                for(int j=0; j<tokens.size(); j++)
                {
                    std::string token = tokens[j];
                    if(tokens[j].find_first_of(";#") != std::string::npos) break;
                    int size = Assembler::getAsmOpcodeSize(token);
                    vasmSize += (size == 0) ? getMacroSize(token) : size;
                }
            }

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Assembler::getAsmOpcodeSizeSubInFile() : Bad lineToken : '%s' : in '%s' : on line %d\n", lineToken._text.c_str(), filename.c_str(), numLines+1);
                return -1;
            }

            numLines++;
        }

        //if(vasmSize) fprintf(stderr, "%s : %s : opcode size : %d\n", filename.c_str(), subname.c_str(), vasmSize);

        return vasmSize;
    }

    void getInternalSubSize(const std::string& includeName, int subIndex)
    {
        uint16_t size = getAsmOpcodeSizeSubInFile("gbas/" + includeName, _internalSubs[subIndex]._name);
        if(size)
        {
            _internalSubs[subIndex]._size = size;
            _internalSubs[subIndex]._includeName = includeName;
        }
    }

    bool getInternalSubCode(const std::string& includeName, const std::vector<std::string>& includeVarsDone, std::vector<std::string>& code, int subIndex)
    {
        std::string filename = "gbas/" + includeName;
        std::string subname = _internalSubs[subIndex]._name;

        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Assembler::getInternalSubCode() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        // Get file
        int numLines = 0;
        bool varsDone = false;
        bool buildingSub = false;

        // Check if include vars already done
        for(int i=0; i<includeVarsDone.size(); i++)
        {
            if(includeVarsDone[i] == includeName)
            {
                varsDone = true;
                break;
            }
        }

        while(!infile.eof())
        {
            std::string line;
            std::getline(infile, line);
            bool foundSub = (line.find("%SUB") != std::string::npos);
            bool foundEnd = (line.find("%ENDS") != std::string::npos);

            if(!buildingSub  &&  foundSub)
            {
                varsDone = true;
                if(line.find(subname) != std::string::npos) buildingSub = true;
            }
            else if(buildingSub  &&  foundEnd)
            {
                _internalSubs[subIndex]._loaded = true;
                buildingSub = false;
                return false;
            }
            else if(buildingSub)
            {
                code.push_back(line);
                for(int i=0; i<_internalSubs.size(); i++)
                {
                    if(!_internalSubs[i]._inUse  &&  line.find(_internalSubs[i]._name) != std::string::npos)
                    {
                        _internalSubs[i]._inUse = true;
                        return true;
                    }
                }
            }
            // Save all text up until the first %SUB
            else if(!buildingSub  &&  !varsDone  &&  !foundSub  &&  !foundEnd)
            {
                code.push_back(line);
            }
            else if(!buildingSub  &&  !varsDone  &&  line.find_first_not_of("  \n\r\f\t\v") == std::string::npos)
            {
                code.push_back("\n");
            }

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Assembler::getInternalSubCode() : Bad lineToken : '%s' : in '%s' : on line %d\n", line.c_str(), filename.c_str(), numLines+1);
                return false;
            }

            numLines++;
        }

        return false;
    }

    bool loadInternalSub(int subIndex, bool overwrite=false)
    {
        if(!overwrite  &&  _internalSubs[subIndex]._address) return false;

        uint16_t address;
        if(Memory::giveFreeRAM(Memory::FitAscending, _internalSubs[subIndex]._size, 0x60A0, 0x7FFF, address))
        {
            // Save end of runtime/strings
            if(address < _runtimeEnd) _runtimeEnd = address;

            fprintf(stderr, "%-18s  :  0x%04x  :  %2d bytes\n", _internalSubs[subIndex]._name.c_str(), address, _internalSubs[subIndex]._size);

            _internalSubs[subIndex]._address = address;
            _internalSubs[subIndex]._inUse = true;
            return true;
        }
        else
        {
            fprintf(stderr, "Compiler::loadInternalSub() : Not enough RAM for %s of size %d\n", _internalSubs[subIndex]._name.c_str(), _internalSubs[subIndex]._size);
            return false;
        }

        return false;
    }

    bool parseIncludes(void)
    {
        for(int i=0; i<_internalSubs.size(); i++)
        {
            if(!Assembler::getUseOpcodeCALLI())
            {
                for(int j=0; j<_subIncludes.size(); j++) getInternalSubSize(_subIncludes[j], i);
            }
            else
            {
                for(int j=0; j<_subIncludesCALLI.size(); j++) getInternalSubSize(_subIncludesCALLI[j], i);
            }
        }

        return true;
    }

    bool linkInternalSubs(void)
    {
        for(int i=0; i<_codeLines.size(); i++)
        {
            // Valid BASIC code
            if(_codeLines[i]._code.size() > 2  &&  _codeLines[i]._vasm.size())
            {
                // Vasm code
                for(int j=0; j<_codeLines[i]._vasm.size(); j++)
                {
                    for(int k=0; k<_internalSubs.size(); k++)
                    {
                        // Check for internal subs in code
                        if(_codeLines[i]._vasm[j]._code.find(_internalSubs[k]._name) != std::string::npos) loadInternalSub(k);

                        // Check for internal subs in macros, (even nested)
                        std::string opcode = _codeLines[i]._vasm[j]._opcode;
                        if(opcode.size()  &&  opcode[0] == '%')
                        {
                            opcode.erase(0, 1);
                            if(findMacroText(opcode, _internalSubs[k]._name)) loadInternalSub(k);
                        }
                    }
                }
            }
        }

        return true;
    }

    void relinkInternalSubs(void)
    {
        for(int i=0; i<_internalSubs.size(); i++)
        {
            // Check for internal sub directly
            if(_internalSubs[i]._inUse  &&  _internalSubs[i]._loaded  &&  _internalSubs[i]._address == 0x0000) loadInternalSub(i);
        }

        fprintf(stderr, "Compiler::linkInternalSubs() : runtime END %04x\n", _runtimeEnd);
    }

    void outputInternalSubs(void)
    {
        _output.push_back("\n");
        _output.push_back(";****************************************************************************************************************************************\n");
        _output.push_back(";****************************************************************************************************************************************\n");
        _output.push_back(";* Internal runtime, DO NOT MODIFY HERE, modifications must be made in the original include files                                       *\n");  
        _output.push_back(";****************************************************************************************************************************************\n");
        _output.push_back(";****************************************************************************************************************************************\n");
        _output.push_back("\n");

        for(int i=0; i<_internalSubs.size()-1; i++)
        {
            if(_internalSubs[i]._inUse)
            {
                _output.push_back(_internalSubs[i]._name + std::string(LABEL_TRUNC_SIZE - _internalSubs[i]._name.size(), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(_internalSubs[i]._address) + "\n");
            }
        }

        // Zero page call table is not needed when using CALLI
        if(!Assembler::getUseOpcodeCALLI())
        {
            if(_internalSubs[0]._inUse) {_output.push_back("realTimeProcAddr" + std::string(LABEL_TRUNC_SIZE - strlen("realTimeProcAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(REAL_TIME_PROC) + "\n");}

            uint16_t convertCcOpsAddr = CONVERT_CC_OPS;
            if(_internalSubs[1]._inUse) {_output.push_back("convertEqOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertEqOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[2]._inUse) {_output.push_back("convertNeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertNeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[3]._inUse) {_output.push_back("convertLeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[4]._inUse) {_output.push_back("convertGeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[5]._inUse) {_output.push_back("convertLtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[6]._inUse) {_output.push_back("convertGtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
        }

        _output.push_back("\n");

        for(int i=0; i<_runtime.size(); i++) _output.push_back(_runtime[i]);
    }

    void outputInternalVars(void)
    {
        _output.push_back("\n");
        _output.push_back("; Internal variables\n");
        _output.push_back("register0"      + std::string(LABEL_TRUNC_SIZE - strlen("register0"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(INT_VAR_START) + "\n");
        _output.push_back("register1"      + std::string(LABEL_TRUNC_SIZE - strlen("register1"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x02\n");
        _output.push_back("register2"      + std::string(LABEL_TRUNC_SIZE - strlen("register2"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x04\n");
        _output.push_back("register3"      + std::string(LABEL_TRUNC_SIZE - strlen("register3"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x06\n");
        _output.push_back("register4"      + std::string(LABEL_TRUNC_SIZE - strlen("register4"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x08\n");
        _output.push_back("register5"      + std::string(LABEL_TRUNC_SIZE - strlen("register5"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0A\n");
        _output.push_back("register6"      + std::string(LABEL_TRUNC_SIZE - strlen("register6"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0C\n");
        _output.push_back("register7"      + std::string(LABEL_TRUNC_SIZE - strlen("register7"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0E\n");
        _output.push_back("register8"      + std::string(LABEL_TRUNC_SIZE - strlen("register8"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x10\n");
        _output.push_back("register9"      + std::string(LABEL_TRUNC_SIZE - strlen("register9"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x12\n");
        _output.push_back("register10"     + std::string(LABEL_TRUNC_SIZE - strlen("register10"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x0024\n");
        _output.push_back("register11"     + std::string(LABEL_TRUNC_SIZE - strlen("register11"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x0026\n");
        _output.push_back("register12"     + std::string(LABEL_TRUNC_SIZE - strlen("register12"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x0028\n");
        _output.push_back("register13"     + std::string(LABEL_TRUNC_SIZE - strlen("register13"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x002A\n");
        _output.push_back("fgbgColour"     + std::string(LABEL_TRUNC_SIZE - strlen("fgbgColour"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x14\n");
        _output.push_back("cursorXY"       + std::string(LABEL_TRUNC_SIZE - strlen("cursorXY"), ' ')       + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x16\n");
        _output.push_back("midiStream"     + std::string(LABEL_TRUNC_SIZE - strlen("midiStream"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x18\n");
        _output.push_back("midiDelay"      + std::string(LABEL_TRUNC_SIZE - strlen("midiDelay"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1A\n");
        _output.push_back("frameCountPrev" + std::string(LABEL_TRUNC_SIZE - strlen("frameCountPrev"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1C\n");
        _output.push_back("miscFlags"      + std::string(LABEL_TRUNC_SIZE - strlen("miscFlags"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1E\n");

        _output.push_back("\n");
    }

    void outputIncludes(void)
    {
        _output.push_back("; Includes\n");
        _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "include/gigatron.i\n");

        if(!Assembler::getUseOpcodeCALLI())
        {
            _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "include/macros.i\n");
        }
        else
        {
            _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "include/macros_CALLI.i\n");
        }

        _output.push_back("\n");
    }

    void outputCode(void)
    {
        std::string line;

        _output.push_back("; Code\n");

        for(int i=0; i<_codeLines.size(); i++)
        {
            int labelIndex = _codeLines[i]._labelIndex;

            // Valid BASIC code
            if(_codeLines[i]._code.size() > 2  &&  _codeLines[i]._vasm.size())
            {
                // BASIC Label, (may not be owned by vasm line 0 as PAGE JUMPS may move labels)
                std::string basicLabel = (labelIndex >= 0) ? _labels[labelIndex]._output : "";

                // Vasm code
                for(int j=0; j<_codeLines[i]._vasm.size(); j++)
                {
                    int vasmSize = _codeLines[i]._vasm[j]._vasmSize;
                    uint16_t vasmAddress = _codeLines[i]._vasm[j]._address;
                    std::string vasmCode = _codeLines[i]._vasm[j]._code;
                    std::string vasmLabel = _codeLines[i]._vasm[j]._internalLabel;

                    // BASIC label has priority over internal label
                    bool useBasicLabel = (labelIndex >=0  &&  _labels[labelIndex]._address == vasmAddress);
                    std::string label = (useBasicLabel) ? basicLabel : vasmLabel + std::string(LABEL_TRUNC_SIZE - vasmLabel.size(), ' ');

                    if(j == 0)
                    {
                        line = (label.size()) ? label + vasmCode : std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                    }
                    else
                    {
                        line += (label.size()) ?  "\n" + label + std::string(LABEL_TRUNC_SIZE - label.size(), ' ') + vasmCode : "\n" + std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                    }

                    if(vasmSize) Memory::takeFreeRAM(vasmAddress, vasmSize);
                }

                // Commented BASIC code, (assumes any tabs are 4 spaces)
#define TAB_SPACE_LENGTH 4
#define COMMENT_PADDING  (TAB_SPACE_LENGTH*19)
                int lineLength = Expression::tabbedStringLength(line, TAB_SPACE_LENGTH);
                line.append(COMMENT_PADDING - (lineLength % COMMENT_PADDING), ' ');
                //fprintf(stderr, "%d\n", lineLength + COMMENT_PADDING - (lineLength % COMMENT_PADDING));
                line += "; " + _codeLines[i]._text + "\n\n";
                _output.push_back(line);
            }
        }
        
        _output.push_back("\n");
    }


    void discardUnusedLabels(void)
    {
        for(int k=0; k<_equateLabels.size(); k++)
        {
            bool foundLabel = false;

            for(int i=0; i<_codeLines.size(); i++)
            {
                // Valid BASIC code
                if(_codeLines[i]._code.size() > 2  &&  _codeLines[i]._vasm.size())
                {
                    // Vasm code
                    for(int j=0; j<_codeLines[i]._vasm.size(); j++)
                    {
                        if(_codeLines[i]._vasm[j]._code.find(_equateLabels[k]) != std::string::npos)
                        {
                            foundLabel = true;
                            break;
                        }
                    }
                }
            }

            // Found a potential unused label to discard
            if(!foundLabel)
            {
                // Can only discard internal labels
                bool foundInternal = (_equateLabels[k].find("_if_") != std::string::npos  ||  _equateLabels[k].find("_next_") != std::string::npos  ||  _equateLabels[k].find("_page_") != std::string::npos);

                for(int l=0; l<_output.size(); l++)
                {
                    // Find unused internal label in output and discard it
                    if(foundInternal  &&  _output[l].find(_equateLabels[k]) != std::string::npos)
                    {
                        _output.erase(_output.begin() + l);
                        break;
                    }
                }
            }
        }
    }


    void collectInternalRuntime(void)
    {
        std::vector<std::string> includeVarsDone;

RESTART:
        for(int i=0; i<_internalSubs.size(); i++)
        {
            if(_internalSubs[i]._inUse  &&  !_internalSubs[i]._loaded)
            {
                _runtime.push_back("\n");
                std::vector<std::string> code;

                if(getInternalSubCode(_internalSubs[i]._includeName, includeVarsDone, code, i)) goto RESTART;

                includeVarsDone.push_back(_internalSubs[i]._includeName);

                for(int j=0; j<code.size(); j++)
                {
                    _runtime.push_back(code[j] + "\n");
                }
                _runtime.push_back("\n");
            }
        }
    }


    void resetInternalSubs(void)
    {
        for(int i=0; i<_internalSubs.size(); i++)
        {
            _internalSubs[i]._size = 0;
            _internalSubs[i]._address = 0;
            _internalSubs[i]._includeName = "";
            _internalSubs[i]._loaded = false;

            // Relational init macros are always loaded
            (i>=0  && i<6) ? _internalSubs[i]._inUse = true : _internalSubs[i]._inUse = false;
        }
    }

    void clearCompiler(void)
    {
        _vasmPC         = USER_CODE_START;
        _tempVarStart   = TEMP_VAR_START;
        _userVarStart   = USER_VAR_START;
        _runtimeEnd     = 0xFFFF;

        _nextTempVar = true;
        _createNumericLabelLut = false;

        _currentLabelIndex = -1;
        _currentCodeLineIndex = 0;

        _tempVarStartStr = "";

        _input.clear();
        _output.clear();
        _runtime.clear();

        _labels.clear();
        _gosubLabels.clear();
        _equateLabels.clear();
        _internalLabels.clear();
        _discardedLabels.clear();

        _codeLines.clear();
        _integerVars.clear();
        _stringVars.clear();
        _defDataBytes.clear();
        _defDataWords.clear();

        resetInternalSubs();

        Memory::initialise();

        Expression::setExprFunc(expression);

        while(!_forNextDataStack.empty()) _forNextDataStack.pop();
    }

    bool compile(const std::string& inputFilename, const std::string& outputFilename)
    {
        Assembler::clearAssembler();
        clearCompiler();

        // Read .gbas file
        int numLines = 0;
        std::ifstream infile(inputFilename);
        if(!readInputFile(infile, inputFilename, numLines)) return false;

        fprintf(stderr, "\n\n****************************************************************************************************\n");
        fprintf(stderr, "* Compiling file '%s'\n", inputFilename.c_str());
        fprintf(stderr, "****************************************************************************************************\n");

        // Labels
        if(!parseLabels(numLines)) return false;

        // Includes
        if(!parseIncludes()) return false;

        // Code
        if(!parseCode()) return false;

        // Optimise
        if(!optimiseCode()) return false;

        // Check code exclusion zones
        if(!checkExclusionZones()) return false;

        // Check branch labels
        if(!checkBranchLabels()) return false;

        // Only link runtime subroutines that are referenced
        if(!linkInternalSubs()) return false;

        // Output
        outputReservedWords();
        outputInternalVars();
        outputIncludes();
        outputLabels();
        outputVars();
        outputStrs();
        outputDefs();
        outputLuts();
        outputCode();

        // Discard
        discardUnusedLabels();

        // Re-linking is needed here as outputInternalRuntime can find new subs that need to be linked
        collectInternalRuntime();
        relinkInternalSubs();
        outputInternalSubs();

        //Memory::printFreeRamList(Memory::SizeAscending);

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        return true;
    }
}