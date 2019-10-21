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


#define LABEL_TRUNC_SIZE 16     // The smaller you make this, the more your BASIC label names will be truncated in the resultant .vasm code
#define USER_STR_SIZE    95

// 32 bytes, (0x00E0 <-> 0x00FF), reserved for vCPU stack, allows for 16 nested calls. The amount of GOSUBS you can use is dependant on how
// much of the stack is being used by nested system calls, (approximately 8 nested GOSUBS worst case, 16 best case).
// *NOTE* there is NO call table for this compiler
#define USER_VAR_START_0 0x0030  // 80 bytes, (0x0030 <-> 0x007F), reserved for BASIC user variables
#define USER_VAR_START_1 0x0082  // 30 bytes, (0x0082 <-> 0x009F), reserved for BASIC user variables
#define INT_VAR_START    0x00A0  // 32 bytes, (0x00A0 <-> 0x00BF), internal register variables, used by the BASIC runtime
#define LOOP_VAR_START   0x00C0  // 16 bytes, (0x00C0 <-> 0x00CF), reserved for loops, maximum of 4 nested loops
#define TEMP_VAR_START   0x00D0  // 16 bytes, (0x00D0 <-> 0x00DF), reserved for temporary expression variables
#define VAC_SAVE_START   0x00E0  // 2 bytes,  (0x00E0 <-> 0x00E1), reserved for saving vAC
#define USER_CODE_START  0x0200
#define USER_STACK_START 0x06FF
#define USER_STR_START   0x6FA0
#define INT_FUNC_START   0x7FA0
#define USER_VAR_END_0   0x007F
#define USER_VAR_END_1   0x009F


namespace Compiler
{
    enum VarType {VarInt8=0, VarInt16, VarInt32, VarFloat16, VarFloat32};
    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated, VarExists};
    enum IntSize {Int8=1, Int16=2, Int32=4};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};
    enum KeywordResult {KeywordNotFound, KeywordError, KeywordFound};

    struct Label
    {
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        bool _pageJump = false;
        bool _gosub = false;
    };

    struct VasmLine
    {
        uint16_t _address;
        std::string _opcode;
        std::string _code;
        std::string _labelInternal;
        int _gotoLabelIndex = -1;
        bool _pageJump = false;
    };

    struct CodeLine
    {
        std::string _text;
        std::string _code;
        std::vector<std::string> _tokens;
        std::vector<VasmLine> _vasm;
        std::string _expression;
        int _vasmSize = 0;
        int _labelIndex = -1;
        int  _varIndex = -1;
        VarType _varType = VarInt16;
        bool _assignOperator = false;
        bool _containsVars = false;
        bool _ownsLabel = false;

        bool initialise(void)
        {
            _code.clear();
            _tokens.clear();
            _vasm.clear();
            _expression.clear();
            _vasmSize = 0;
            _labelIndex = -1;
            _varIndex = -1;
            _varType = VarInt16;
            _assignOperator = false;
            _containsVars = false;
            _ownsLabel = false;
        }
    };

    struct IntegerVar
    {
        int16_t _data;
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        IntSize _intSize = Int16;
    };

    struct FloatVar
    {
        int16_t _data;
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        FloatSize _floatSize = Float16;
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

    struct ArrayVar
    {
        uint16_t _size;
        uint16_t _address;
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
        int _labelIndex;
        int _codeLineIndex;
        int16_t _loopEnd;
        int16_t _loopStep;
        uint16_t _varEnd;
        uint16_t _varStep;
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


    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart0  = USER_VAR_START_0;
    uint16_t _userVarStart1  = USER_VAR_START_1;
    uint16_t _userStrStart   = USER_STR_START;
    uint16_t _userStackStart = USER_STACK_START;

    bool _nextTempVar = true;

    int _currentLabelIndex = -1;
    int _currentCodeLineIndex = 0;

    std::string _tempVarStartStr;

    std::map<std::string, Keyword> _keywords;
    std::vector<std::string> _keywordsWithEquals;

    std::vector<std::string> _input;
    std::vector<std::string> _output;
    std::vector<std::string> _vasmCode;

    std::vector<std::string> _gosubLabels;

    std::vector<Label>      _labels;
    std::vector<CodeLine>   _codeLines;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar>  _stringVars;
    std::vector<ArrayVar>   _arrayVars;

    std::stack<ForNextData> _forNextDataStack;

    std::vector<std::string> _macroLines;
    std::map<int, MacroNameEntry> _macroNameEntries;
    std::map<std::string, MacroIndexEntry> _macroIndexEntries;


    bool keywordREM(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLET(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordEND(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOTO(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCLS(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPRINT(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFOR(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordNEXT(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordIF(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordTHEN(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSE(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSEIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordENDIF(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDIM(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDEF(CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordINPUT(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordREAD(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDATA(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordON(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOSUB(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDO(CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLOOP(CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWHILE(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordUNTIL(CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    

    bool initialise(void)
    {
        _keywords["END"   ] = {0, "END",    keywordEND   };
        _keywords["GOTO"  ] = {1, "GOTO",   keywordGOTO  };
        _keywords["CLS"   ] = {0, "CLS",    keywordCLS   };
        _keywords["PRINT" ] = {0, "PRINT",  keywordPRINT };
        _keywords["FOR"   ] = {0, "FOR",    keywordFOR   };
        _keywords["NEXT"  ] = {0, "NEXT",   keywordNEXT  };
        _keywords["IF"    ] = {0, "IF",     nullptr      };
        _keywords["THEN"  ] = {0, "THEN",   nullptr      };
        _keywords["ELSE"  ] = {0, "ELSE",   nullptr      };
        _keywords["ELSEIF"] = {0, "ELSEIF", nullptr      };
        _keywords["ENDIF" ] = {0, "ENDIF",  nullptr      };
        _keywords["DIM"   ] = {0, "DIM",    nullptr      };
        _keywords["DEF"   ] = {0, "DEF",    nullptr      };
        _keywords["INPUT" ] = {0, "INPUT",  nullptr      };
        _keywords["READ"  ] = {0, "READ",   nullptr      };
        _keywords["DATA"  ] = {0, "DATA",   nullptr      };
        _keywords["PEEK"  ] = {1, "PEEK",   nullptr      };
        _keywords["POKE"  ] = {1, "POKE",   nullptr      };
        _keywords["DEEK"  ] = {1, "DEEK",   nullptr      };
        _keywords["DOKE"  ] = {1, "DOKE",   nullptr      };
        _keywords["ON"    ] = {0, "ON",     nullptr      };
        _keywords["GOSUB" ] = {1, "GOSUB",  keywordGOSUB };
        _keywords["RETURN"] = {0, "RETURN", keywordRETURN};
        _keywords["DO"    ] = {0, "DO",     nullptr      };
        _keywords["LOOP"  ] = {0, "LOOP",   nullptr      };
        _keywords["WHILE" ] = {0, "WHILE",  nullptr      };
        _keywords["UNTIL" ] = {0, "UNTIL",  nullptr      };
        _keywords["AND"   ] = {2, "AND",    nullptr      };
        _keywords["OR"    ] = {2, "OR",     nullptr      };
        _keywords["XOR"   ] = {2, "XOR",    nullptr      };
        _keywords["NOT"   ] = {1, "NOT",    nullptr      };
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
        _keywords["CHR$"  ] = {1, "CHR$",   nullptr      };
        _keywords["HEX$"  ] = {1, "HEX$",   nullptr      };
        _keywords["HEXW$" ] = {1, "HEXW$",  nullptr      };
        _keywords["MID$"  ] = {1, "MID$",   nullptr      };
        _keywords["LEFT$" ] = {1, "LEFT$",  nullptr      };
        _keywords["RIGHT$"] = {1, "RIGHT$", nullptr      };
        _keywords["SPC$"  ] = {1, "SPC$",   nullptr      };
        _keywords["STR$"  ] = {1, "STR$",   nullptr      };
        _keywords["TIME$" ] = {1, "TIME$",  nullptr      };

        _keywordsWithEquals.push_back("FOR");
        _keywordsWithEquals.push_back("IF");

        return intialiseMacros();
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

    int findVar(std::string& varName)
    {
        varName = Expression::getSubAlpha(varName);
        for(int i=0; i<_integerVars.size(); i++)
        {
            if(_integerVars[i]._name == varName) return i;
        }

        return -1;
    }


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label, bool addUnderscore=true, bool pageJump=false, bool gosub=false)
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

        label = {address, n, o, codeLineIndex, pageJump, gosub};
        Expression::stripWhitespace(label._name);
        _labels.push_back(label);
        _currentLabelIndex = int(_labels.size() - 1);
    }

    void createVar(const std::string& varName, int16_t data, int codeLineIndex, bool containsVars, int& varIndex)
    {
        uint16_t userVarStart = (_userVarStart0 < USER_VAR_END_0) ? _userVarStart0 : _userVarStart1;
        if(userVarStart >= USER_VAR_END_1)
        {
            fprintf(stderr, "Compiler::createVar() : You have exceeded the maximum number of Page 0 variables allowed : on line %d\n", codeLineIndex);
        }

        // Create var
        varIndex = int(_integerVars.size());
        _codeLines[codeLineIndex]._assignOperator = true;
        _codeLines[codeLineIndex]._containsVars = containsVars;
        _codeLines[codeLineIndex]._varType = VarInt16;
        _codeLines[codeLineIndex]._varIndex = varIndex;
        IntegerVar integerVar = {data, userVarStart, varName, varName, codeLineIndex, Int16};
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

        if(_userVarStart0 < USER_VAR_END_0)
        {
            _userVarStart0 += Int16;
        }
        else
        {
            _userVarStart1 += Int16;
        }
    }

    VarResult createCodeVar(const std::string& code, int codeLineIndex, int& varIndex)
    {
        size_t equals1 = Expression::findNonStringEquals(code) - code.begin();
        if(code.size() > 2  &&  equals1 < code.size())
        {
            std::string codeUC = code;
            Expression::strToUpper(codeUC);
            for(int i=0; i<_keywordsWithEquals.size(); i++)
            {
                size_t keyword = codeUC.find(_keywordsWithEquals[i]);
                if(keyword < equals1) return VarNotFound;
            }

            std::string equalsText = code.substr(equals1+1);
            auto equals2 = Expression::findNonStringEquals(equalsText);
            if(equals2 != equalsText.end())
            {
                fprintf(stderr, "Compiler::createCodeVar() : too many '=' in '%s'\n", code.c_str());
                return VarError;
            }

            // Name and input
            std::string varName = code.substr(0, equals1);
            Expression::stripWhitespace(varName);

            // Var already exists?
            varIndex = findVar(varName);
            if(varIndex != -1)
            {
                _codeLines[codeLineIndex]._assignOperator = true;
                _codeLines[codeLineIndex]._containsVars = false;
                _codeLines[codeLineIndex]._varType = VarInt16;
                _codeLines[codeLineIndex]._varIndex = varIndex;

                return VarExists;
            }

            createVar(varName, 0, codeLineIndex, false, varIndex);

            return VarCreated;
        }

        return VarNotFound;
    }

    uint32_t isExpression(const std::string& input, int& varIndex, int& params)
    {
        uint32_t expressionType = 0x0000;

        // Check for vars
        std::vector<std::string> tokens = Expression::tokenise(input, "-+/*();, ", false);
        for(int i=0; i<tokens.size(); i++)
        {
            varIndex = findVar(tokens[i]);
            if(varIndex != -1) expressionType |= Expression::HasVars;
        }

        for(int i=0; i<tokens.size(); i++)
        {
            Expression::strToUpper(tokens[i]);
            if(_keywords.find(tokens[i]) != _keywords.end())
            {
                params = _keywords[tokens[i]]._params;
                expressionType |= Expression::HasKeywords;
            }
        }

        // Check for strings
        if(input.find("$") != std::string::npos) expressionType |= Expression::IsString;
        if(input.find("\"") != std::string::npos) expressionType |= Expression::IsString;

        // Check for operators
        if(input.find_first_of("-+/*()") != std::string::npos) expressionType |= Expression::Valid;

        return expressionType;
    }

    void updateVar(int16_t data, int codeLineIndex, int varIndex, bool containsVars)
    {
        _codeLines[codeLineIndex]._assignOperator = true;
        _codeLines[codeLineIndex]._containsVars = containsVars;
        _codeLines[codeLineIndex]._varType = VarInt16;
        _codeLines[codeLineIndex]._varIndex = varIndex;
        _integerVars[varIndex]._data = data;
    }

    bool createCodeLine(const std::string& code, int codeLineOffset, int labelIndex, int varIndex, VarType varType, bool assign, bool vars, bool ownsLabel, CodeLine& codeLine)
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

        std::vector<VasmLine> vasm;
        std::string codeText = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        Expression::trimWhitespace(codeText);
        std::string codeExpr = Expression::collapseWhitespace(codeText);
        std::vector<std::string> tokens = Expression::tokenise(codeExpr, ' ', true);
        codeLine = {codeText, codeExpr, tokens, vasm, expression, 0, labelIndex, varIndex, varType, assign, vars, ownsLabel};
        //Expression::stripNonStringWhitespace(codeLine._code);
        //Expression::stripWhitespace(codeLine._expression);
        Expression::operatorReduction(codeLine._expression);
        if(codeLine._code.size() < 3) return true;
        _codeLines.push_back(codeLine);

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

    bool intialiseMacros(void)
    {
        static std::string filename = "gbas/include/macros.i";
        static std::ifstream infile(filename);

        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::intialiseMacros() : Failed to open file : '%s'\n", filename.c_str());
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

            fprintf(stderr, "Compiler::intialiseMacros() : %%MACRO %s on line %d: is missing a %%ENDM\n", macroName.c_str(), macroIndex);
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
        std::string tabs = (opcode.size() > 3) ? "\t" : "\t\t";
        line = opcode + tabs + operand;

        return vasmSize;
    }

    int insertVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, int vasmLineIdx, int gotoLabelIndex=-1, bool pageJump=false)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.insert(_codeLines[codeLineIdx]._vasm.begin() + vasmLineIdx, {uint16_t(_vasmPC - vasmSize), opcodeStr, line, "", gotoLabelIndex, pageJump});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLineIndex, const std::string& labelInternal="", int gotoLabelIndex=-1, bool pageJump=false)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, line, labelInternal, gotoLabelIndex, pageJump});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        if(nextTempVar) getNextTempVar();
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

    LabelResult checkForLabel(const std::string& code, int lineNumber)
    {
        Label label;
        CodeLine codeLine;
        std::string gosubOpcode = "";

        // Numeric labels
        if(code.size() > 1  &&  isdigit(code[0]))
        {
            size_t space = code.find_first_of(" \n\r\f\t\v");
            if(space == std::string::npos)
            {
                fprintf(stderr, "Compiler::checkForLabel() : white space expected after line mumber in : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
                return LabelError;
            }
            for(int i=1; i<space; i++)
            {
                if(!isdigit(code[i]))
                {
                    fprintf(stderr, "Compiler::checkForLabel() : non digits found in line number in : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
                    return LabelError;
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
            createLabel(_vasmPC, labelName, labelName, int(_codeLines.size()), label, true, false, foundGosub);
            if(!createCodeLine(code, int(space + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine)) return LabelError;

            return LabelFound;
        }

        // Text labels
        size_t colon1 = code.find_first_of(":");
        size_t colon2 = code.find_first_of(":", colon1+1);
        if(colon1 == std::string::npos)
        {
            // Non label code
            if(!createCodeLine(code, 0, _currentLabelIndex, -1, VarInt16, false, false, false, codeLine)) return LabelError;
            return LabelNotFound;
        }
        if(colon2 != std::string::npos)
        {
            fprintf(stderr, "Compiler::checkForLabel() : only one label per line is allowed in : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
            return LabelError;
        }

        if(code.size() - (colon1 + 1) <= 2)
        {
            fprintf(stderr, "Compiler::checkForLabel() : label cannot exist on its own : '%s' : on line %d\n", code.c_str(), lineNumber + 1);
            return LabelError;
        }

        // Create label
        std::string labelName = code.substr(0, colon1);
        bool foundGosub = isGosubLabel(labelName);
        createLabel(_vasmPC, labelName, labelName, int(_codeLines.size()), label, true, false, foundGosub);
        if(!createCodeLine(code, int(colon1  + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine)) return LabelError;

        return LabelFound;
    }

    bool parseLabels(int numLines)
    {
        // Entry point initialisation
        Label label;
        CodeLine codeLine;
        createLabel(_vasmPC, "_entryPoint_", "_entryPoint_\t", 0, label, false, false, false);
        if(!createCodeLine("INIT", 0, 0, -1, VarInt16, false, false, true, codeLine)) return false;
        emitVcpuAsm("%Initialise", "", false, 0);

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
    void adjustLabelAddresses(int labelIndex, uint16_t optimisedAddress, int offset)
    {
        for(int i=0; i<_labels.size(); i++)
        {
            if(_labels[i]._address > _labels[labelIndex]._address  &&  _labels[i]._address >= optimisedAddress)
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


    bool parseVars(void)
    {
        int varIndex;
        VarResult varResult;

        // Create vars
        for(int i=0; i<_codeLines.size(); i++)
        {
            // REM and LET modify code
            size_t foundPos;
            KeywordFuncResult result;
            if(findKeyword(_codeLines[i]._code, "REM", foundPos))
            {
                keywordREM(_codeLines[i], 0, foundPos, result);
            }
            else if(findKeyword(_codeLines[i]._code, "LET", foundPos))
            {
                keywordLET(_codeLines[i], 0, foundPos, result);
            }

            varResult = createCodeVar(_codeLines[i]._code, i, varIndex);
            if(varResult == VarError) return false;
        }

        return true;
    }

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

    // ********************************************************************************************
    // Functions
    // ********************************************************************************************
    Expression::Numeric functionCHR$(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            emitVcpuAsm("LDWI", std::to_string(numeric._value), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        //emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcChar", "", false);

        return numeric;
    }

    Expression::Numeric functionHEX$(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(numeric._value)), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        //emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcHexByte", "", false);

        return numeric;
    }

    Expression::Numeric functionHEXW$(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        getNextTempVar();
        handleSingleOp("LDW", numeric);
        //emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);
        if(Expression::getEnablePrint()) emitVcpuAsm("%PrintAcHexWord", "", false);

        return numeric;
    }

    Expression::Numeric functionPEEK(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(numeric._value), false);
        }

        handleSingleOp("LDW", numeric);
        emitVcpuAsm("PEEK", "", false);
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

    Expression::Numeric operatorMUL(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value *= right._value;
            return left;
        }

        // Optimise multiply with 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, true, false, std::string(""));

        return left;
    }

    Expression::Numeric operatorDIV(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value /= right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, true, false, std::string(""));

        return left;
    }


    // ********************************************************************************************
    // Recursive Descent Parser helpers
    // ********************************************************************************************
    char peek(bool skipSpace)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(!skipSpace  &&  Expression::peek() == ' ')
        {
            if(!Expression::advance(1)) return 0;
        }

        return Expression::peek();
    }

    char get(bool skipSpace)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(!skipSpace  &&  Expression::peek() == ' ')
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
                numeric = Expression::Numeric(0, false, false, std::string(""));
            }
            get(false);
        }
        else if((peek(false) >= '0'  &&  peek(false) <= '9')  ||  peek(false) == '&')
        {
            if(!number(value))
            {
                fprintf(stderr, "Compiler::factor() : Bad numeric data in '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, false, false, std::string(""));
            }
            else
            {
                numeric = Expression::Numeric(value, true, false, std::string(""));
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
        // Unary operators
        else if(Expression::find("NOT"))
        {
            numeric = factor(0); numeric = operatorNOT(numeric);
        }
        else
        {
            // Unary operators
            switch(peek(false))
            {
                case '+': get(false); numeric = factor(0);                                 break;
                case '-': get(false); numeric = factor(0); numeric = operatorNEG(numeric); break;

                default:
                {
                    std::string varName = Expression::getExpression();
                    if(findVar(varName) != -1)
                    {
                        numeric = Expression::Numeric(defaultValue, true, true, varName);
                        Expression::advance(varName.size());
                    }
                    else
                    {
                        numeric = Expression::Numeric(defaultValue, false, false, std::string("")); 
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
            if(peek(false) == '*')      {get(false); f = factor(0); result = operatorMUL(result, f);                                           }
            else if(peek(false) == '/') {get(false); f = factor(0); result = (f._value != 0) ? operatorDIV(result, f) : operatorMUL(result, f);}
            else return result;
        }
    }

    Expression::Numeric expression(void)
    {
        Expression::Numeric t, result = term();

        for(;;)
        {
            if(peek(false) == '+')           {get(false); t = term(); result = operatorADD(result, t);}
            else if(peek(false) == '-')      {get(false); t = term(); result = operatorSUB(result, t);}
            else if(Expression::find("AND")) {t = term(); result = operatorAND(result, t);            }
            else if(Expression::find("XOR")) {t = term(); result = operatorXOR(result, t);            }
            else if(Expression::find("OR"))  {t = term(); result = operatorOR(result, t);             }
            else return result;
        }
    }

    bool varExpressionParse(CodeLine& codeLine, int codeLineIndex)
    {
        int16_t value;
        return Expression::parse(codeLine._expression, codeLineIndex, value);
    }

    int varAssignmentParse(CodeLine& codeLine, int codeLineIndex)
    {
        //if(codeLine._expression.find_first_of("-+/*()&|^") != std::string::npos) return -1;
        int varIndex = findVar(codeLine._expression);
        return varIndex;
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


    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool keywordREM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove REM and everything after it in code
        codeLine._code.erase(foundPos - 3, codeLine._code.size() - (foundPos - 3));

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
        codeLine._code.erase(foundPos - 3, foundPos);

        return true;
    }

    bool keywordEND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Label label;
        std::string endName = "_end_" + std::to_string(codeLineIndex);
        createLabel(_vasmPC, endName, "END\t", codeLineIndex, label, false, false, false);
        _codeLines[codeLineIndex]._ownsLabel = true;
        _codeLines[codeLineIndex]._labelIndex = _currentLabelIndex;
        emitVcpuAsm("BRA", endName, false, codeLineIndex);

        return true;
    }

    bool keywordGOTO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != _keywords["GOTO"]._params + 1)
        {
            fprintf(stderr, "Compiler::keywordGOTO() : missing or invalid label in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string gotoLabel = codeLine._tokens[1];
        int labelIndex = findLabel(gotoLabel);
        if(labelIndex == -1)
        {
            fprintf(stderr, "Compiler::keywordGOTO() : invalid label in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Within same page
        if(HI_MASK(_vasmPC) == HI_MASK(_labels[labelIndex]._address))
        {
            emitVcpuAsm("BRA", "_" + gotoLabel, false, codeLineIndex, "", labelIndex);
        }
        // Long jump
        else
        {
            emitVcpuAsm("LDWI", "_" + gotoLabel, false, codeLineIndex, "", -1);
            emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex, "", -1);
        }

        return true;
    }

    bool keywordCLS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        emitVcpuAsm("%Initialise", "", false, codeLineIndex);

        return true;
    }

    bool keywordPRINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse print tokens
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', offsets, false);

        // First offset is always missing and last one is always incorrect
        offsets.insert(offsets.begin(), 0);
        offsets.pop_back();

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
            else if(expressionType & Expression::HasVars)
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                if(varIndex >= 0)
                {
                    emitVcpuAsm("%PrintVarInt16", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
                }
                else
                {
                    emitVcpuAsm("%PrintVarInt16", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::Valid)
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value), false, codeLineIndex);
            }
            else if(expressionType & Expression::IsString)
            {
                size_t lquote = tokens[i].find_first_of("\"");
                size_t rquote = tokens[i].find_first_of("\"", lquote + 1);
                if(lquote != std::string::npos  &&  rquote != std::string::npos)
                {
                    if(rquote == lquote + 1) continue; // skip empty strings
                    std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));
                    if(str.size() > USER_STR_SIZE)
                    {
                        fprintf(stderr, "Compiler::keywordPRINT() : user string is %d characters too long in '%s' on line %d\n", int(str.size() - USER_STR_SIZE), codeLine._text.c_str(), codeLineIndex + 1);
                        return false;
                    }

                    // Reuse string
                    bool foundString = false;
                    for(int j=0; j<_stringVars.size(); j++)
                    {
                        if(_stringVars[j]._data == str) 
                        {
                            emitVcpuAsm("%PrintString", _stringVars[j]._name, false, codeLineIndex);
                            foundString = true;
                            break;
                        }
                    }
                    if(foundString) continue;

                    // Create string            
                    if((_userStrStart + HI_MASK(str.size())) != HI_MASK(_userStrStart))
                    {
                        _userStrStart -= 0x0100;
                        if(_userStrStart < 0x08A0)
                        {
                            fprintf(stderr, "Compiler::keywordPRINT() : out of string memory, user string address %04x in '%s' on line %d\n", _userStrStart, codeLine._text.c_str(), codeLineIndex + 1);
                            return false;
                        }
                    }

                    std::string usrStrName = "usrStr_" + Expression::wordToHexString(_userStrStart);
                    StringVar usrStrVar = {uint8_t(str.size()), _userStrStart, str, usrStrName, usrStrName + "\t\t", -1};
                    _stringVars.push_back(usrStrVar);
                    _userStrStart += uint16_t(str.size() + 1);
                    emitVcpuAsm("%PrintString", _stringVars[_stringVars.size() - 1]._name, false, codeLineIndex);
                }
            }
            else
            {
                Expression::parse(tokens[i], codeLineIndex, value);
                emitVcpuAsm("%PrintInt16", Expression::wordToHexString(value), false, codeLineIndex);
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            emitVcpuAsm("LDWI", "newLineScroll", false, codeLineIndex);
            emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
        }

        return true;

#if 0
        // Parse print tokens
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', offsets, false);

        // First offset is always missing and last one is always bogus
        offsets.insert(offsets.begin(), 0);
        offsets.pop_back();

        for(int i=0; i<tokens.size(); i++)
        {
            Expression::ExpressionType expressionType;// = isExpression(tokens[i]);
            switch(expressionType)
            {
                case Expression::IsString:
                {
                    KeywordResult keywordResult;// = handleStringwords(codeLine, foundPos + offsets[i], tokens[i], codeLineIndex, result, true);
                    if(keywordResult == KeywordFound) continue;

                    size_t lquote = tokens[i].find_first_of("\"");
                    size_t rquote = tokens[i].find_first_of("\"", lquote + 1);
                    if(lquote != std::string::npos  &&  rquote != std::string::npos)
                    {
                        if(rquote == lquote + 1) continue; // skip empty strings
                        std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));
                        if(str.size() > USER_STR_SIZE)
                        {
                            fprintf(stderr, "Compiler::keywordPRINT() : user string is %d characters too long in '%s' on line %d\n", int(str.size() - USER_STR_SIZE), codeLine._text.c_str(), codeLineIndex + 1);
                            return false;
                        }

                        // Reuse string
                        bool foundString = false;
                        for(int j=0; j<_stringVars.size(); j++)
                        {
                            if(_stringVars[j]._data == str) 
                            {
                                emitVcpuAsm("%PrintString", _stringVars[j]._name, false, codeLineIndex);
                                foundString = true;
                                break;
                            }
                        }
                        if(foundString) continue;

                        // Create string            
                        if((_userStrStart + HI_MASK(str.size())) != HI_MASK(_userStrStart))
                        {
                            _userStrStart -= 0x0100;
                            if(_userStrStart < 0x08A0)
                            {
                                fprintf(stderr, "Compiler::keywordPRINT() : out of string memory, user string address %04x in '%s' on line %d\n", _userStrStart, codeLine._text.c_str(), codeLineIndex + 1);
                                return false;
                            }
                        }

                        std::string usrStrName = "usrStr_" + Expression::wordToHexString(_userStrStart);
                        StringVar usrStrVar = {uint8_t(str.size()), _userStrStart, str, usrStrName, usrStrName + "\t\t", -1};
                        _stringVars.push_back(usrStrVar);
                        _userStrStart += uint16_t(str.size() + 1);
                        emitVcpuAsm("%PrintString", _stringVars[_stringVars.size() - 1]._name, false, codeLineIndex);
                    }
                }
                break;

                case Expression::None:
                case Expression::Valid:
                {
                    int16_t result;
                    if(!Expression::parse(tokens[i], codeLineIndex, result)) return false;
                    emitVcpuAsm("%PrintInt16", Expression::wordToHexString(result), false, codeLineIndex);
                }
                break;

                case Expression::HasVars:
                {
                    // Keywords like PEEK, DEEK, etc
                    KeywordResult keywordResult;// = handleKeywords(codeLine, foundPos + offsets[i], tokens[i], codeLineIndex, result, true);
                    if(keywordResult == KeywordFound) continue;

                    // Search math words and string words
                    //keywordResult = handleMathwords(codeLine, foundPos, codeLineIndex, result, true);
                    //if(keywordResult == KeywordNotFound) keywordResult = handleStringwords(codeLine, foundPos, codeLineIndex, result, true);
                    if(keywordResult == KeywordNotFound)
                    {
                        CodeLine cl = codeLine;
                        cl._code = cl._expression = tokens[i];
                        if(!varExpressionParse(cl, codeLineIndex)) return false;
                        int varIndex = varAssignmentParse(cl, codeLineIndex);
                        if(varIndex >= 0)
                        {
                            emitVcpuAsm("%PrintVarInt16", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
                        }
                        else
                        {
                            emitVcpuAsm("%PrintVarInt16", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
                        }
                    }
                }
                break;

                default:
                {
                    fprintf(stderr, "Compiler::keywordPRINT() : invalid input in '%s' on line %d\n", tokens[i].c_str(), codeLineIndex);
                    return false;
                }
                break;
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            emitVcpuAsm("LDWI", "newLineScroll", false, codeLineIndex);
            emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
        }

        return true;
#endif
    }

    bool keywordFOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() < 6  ||  codeLine._tokens.size() > 8)
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        if(codeLine._tokens[2] != "=")
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (missing '='), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        if(Expression::strToUpper(codeLine._tokens[4]) != "TO")
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (missing 'TO'), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }
        
        // Optional step
        if(codeLine._tokens.size() == 8  &&  Expression::strToUpper(codeLine._tokens[6]) == "STEP")
        {
        }

        // Loop start
        int16_t loopStart;
        if(!Expression::stringToI16(codeLine._tokens[3], loopStart))
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (bad FOR start), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        // Var counter, (create or update if being reused)
        int varIndex = findVar(codeLine._tokens[1]);
        (varIndex < 0) ? createVar(codeLine._tokens[1], loopStart, codeLineIndex, false, varIndex) : updateVar(loopStart, codeLineIndex, varIndex, false);

        // Loop end
        int16_t loopEnd;
        if(!Expression::stringToI16(codeLine._tokens[5], loopEnd))
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (bad FOR end), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

#if 0
        // Maximum of 4 nested loops
        if(_forNextDataStack.size() == 4)
        {
            fprintf(stderr, "Compiler::keywordFOR() : syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        int offset = int(_forNextDataStack.size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        emitVcpuAsm("%ForNextInitVsVe", "_" + _integerVars[varIndex]._name + " " + std::to_string(loopStart) + " " + std::to_string(loopEnd) + " 1" + " " + Expression::wordToHexString(varEnd) + " " + Expression::wordToHexString(varStep), false, codeLineIndex);
#endif

        // Variable is intialised automatically in createVasmCode()
        //emitVcpuAsm("LDWI", std::to_string(loopStart), false, codeLineIndex);

        // Find first valid line
        int lineAfterLoopInit = -1;
        for(int i=_currentCodeLineIndex + 1; i<_codeLines.size(); i++)
        {
            if(_codeLines[i]._code.size())
            {
                lineAfterLoopInit = i;
                break;
            }
        }
        if(lineAfterLoopInit == -1)
        {
            lineAfterLoopInit = _currentCodeLineIndex + 1;
            fprintf(stderr, "Compiler::keywordFOR() : no valid line of FOR loop code, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
        }

        // Create FOR loop label, (label is attached to line after FOR loop initialisation)
        Label label;
        std::string name = "_next" + std::to_string(lineAfterLoopInit); // + "_" + _integerVars[varIndex]._name;
        createLabel(_vasmPC, name, name + "\t", lineAfterLoopInit, label, false, false, false);
        _codeLines[lineAfterLoopInit]._ownsLabel = true;
        _codeLines[lineAfterLoopInit]._labelIndex = _currentLabelIndex;

        // Update all lines belonging to this label
        for(int i=lineAfterLoopInit; i<_codeLines.size(); i++)
        {
            if(!_codeLines[i]._ownsLabel) _codeLines[i]._labelIndex = _currentLabelIndex;
        }

        // FOR loops that have inputs as variables use a stack
        _forNextDataStack.push({varIndex, _codeLines[lineAfterLoopInit]._labelIndex, lineAfterLoopInit, loopEnd, 1, 0x00, 0x00});

        return true;
    }

    bool keywordNEXT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : syntax error, (bad var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        ForNextData forNextData = _forNextDataStack.top();
        _forNextDataStack.pop();
        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Compiler::keywordNEXT() : syntax error, (wrong var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        emitVcpuAsm("%ForNextLoopP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex, "", forNextData._labelIndex);

#if 0
        emitVcpuAsm("%ForNextLoopVsVeP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + Expression::wordToHexString(forNextData._varEnd) + " " + Expression::wordToHexString(forNextData._varStep), false, codeLineIndex, "", forNextData._labelIndex);
#endif

        return true;
    }

    bool keywordIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordTHEN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

    bool keywordENDIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordDIM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordDEF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
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

    bool keywordON(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool keywordGOSUB(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != _keywords["GOSUB"]._params + 1)
        {
            fprintf(stderr, "Compiler::keywordGOSUB() : missing or invalid label in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        std::string gosubLabel = codeLine._tokens[1];
        int labelIndex = findLabel(gosubLabel);
        if(labelIndex == -1)
        {
            fprintf(stderr, "Compiler::keywordGOSUB() : invalid label in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex + 1);
            return false;
        }

        _labels[labelIndex]._gosub = true;

        emitVcpuAsm("LDWI", "_" + gosubLabel, false, codeLineIndex);
        emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);

        return true;
    }

    bool keywordRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        emitVcpuAsm("POP", "", false, codeLineIndex);
        emitVcpuAsm("RET", "", false, codeLineIndex);

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


    bool createVasmCode(CodeLine& codeLine, int codeLineIndex)
    {
        // Check for subroutine start
        if(codeLine._ownsLabel  &&  _labels[codeLine._labelIndex]._gosub) emitVcpuAsm("PUSH", "", false, codeLineIndex);

        for(int i=0; i<codeLine._tokens.size(); i++)
        {
            KeywordFuncResult result;
            KeywordResult keywordResult = handleKeywords(codeLine, codeLine._tokens[i], codeLineIndex, result);
            //if(keywordResult == KeywordFound) return true;
            std::string token = codeLine._tokens[i];
            if(Expression::strToUpper(token) == "PRINT") return true;
        }

        int varIndex, params;
        int16_t value = 0;
        bool containsVars = false;
        uint32_t expressionType = isExpression(codeLine._expression, varIndex, params);
        if(expressionType & Expression::HasVars) containsVars = true;
        Expression::parse(codeLine._expression, codeLineIndex, value);
        if(codeLine._varIndex != -1)
        {
            updateVar(value, codeLineIndex, codeLine._varIndex, containsVars);
        }

        // Check for matching brackets
        if(std::count(codeLine._expression.begin(), codeLine._expression.end(), '(') != std::count(codeLine._expression.begin(), codeLine._expression.end(), ')'))
        {
            fprintf(stderr, "Compiler::createVasmCode() : Brackets are not matched in '%s' on line %d\n", codeLine._expression.c_str(), codeLineIndex);
            return false;
        }

        // TODO: only works with Int16, fix for all var types
        // Variable assignment
        if(codeLine._assignOperator)
        {
            // Optimisation, (removes uneeded LDW's)
            static int prevVarIndex = -1;

            // Assignment with a var expression
            if(codeLine._containsVars)
            {
                emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, codeLineIndex);
                prevVarIndex = codeLine._varIndex;
            }
            // Standard assignment
            else
            {
                // Skip for functions
                //if(params != 1  &&  (expressiontype != Expression::HasKeywords  ||  varIndex == -1))
                if(params != 1  &&  !(expressionType & Expression::HasKeywords)  &&  varIndex == -1)
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
                emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, codeLineIndex);
                prevVarIndex = codeLine._varIndex;
            }
        }

        return true;
    }

    bool parseCode(void)
    {
        std::string line;

        for(int i=0; i<_codeLines.size(); i++)
        {
            _currentCodeLineIndex = i;

            // First line of BASIC code is always a dummy INIT line, ignore it
            if(i > 0  &&  _codeLines[i]._code.size() > 2)
            {
                // Adjust label address
                if(_codeLines[i]._ownsLabel)
                {
                    _labels[_codeLines[i]._labelIndex]._address = _vasmPC;
                }

                // Create .vasm code
                if(!createVasmCode(_codeLines[i], i)) return false;
            }
        }

        return true;
    }

    enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwPair, ExtraStw, AddiZero, SubiZero, NumOptimiseTypes};
    bool optimiseCode(void)
    {
        const std::string firstMatch[NumOptimiseTypes]  = {"STW\t\t", "STW\t\t", "STW\t\t", "STW\t\t0x", "ADDI\t", "SUBI\t"};
        const std::string secondMatch[NumOptimiseTypes] = {"LDW\t\t", "LD\t\t",  "STW\t\t", "STW\t\t_",  "",       ""      };

        for(int i=0; i<_codeLines.size(); i++)
        {
            int firstLine = 0;
            std::string firstVar;
            bool firstFound = false;

            for(int j=StwLdwPair; j<NumOptimiseTypes; j++)
            {
                for(auto itVasm=_codeLines[i]._vasm.begin(); itVasm!=_codeLines[i]._vasm.end();)
                {
                    bool linesDeleted = false;

                    switch(j)
                    {
                        // Matched pairs
                        case StwLdwPair:
                        case StwLdPair:
                        case StwPair:
                        case ExtraStw:
                        {
                            // First match
                            if(!firstFound)
                            {
                                size_t first = itVasm->_code.find(firstMatch[j]);
                                if(first != std::string::npos)
                                {
                                    firstFound = true;
                                    firstLine = int(itVasm - _codeLines[i]._vasm.begin());
                                    size_t whiteSpace = itVasm->_code.find_first_of("  \n\r\f\t\v", first + firstMatch[j].size());
                                    firstVar = itVasm->_code.substr(first + firstMatch[j].size(), whiteSpace);
                                }
                            }
                            else
                            {
                                // Second match must be on next vasm line, next vasm line can be a new BASIC code line
                                if(int(itVasm - _codeLines[i]._vasm.begin())  ==  firstLine + 1) // ||  itVasm == _codeLines[i]._vasm.begin())
                                {
                                    size_t second = itVasm->_code.find(secondMatch[j]);
                                    if(second != std::string::npos)
                                    {
                                        switch(j)
                                        {
                                            // Remove superfluous STW/LDW pairs
                                            case StwLdwPair:
                                            case StwLdPair:
                                            {
                                                // If operand of STW/LDW pair matches
                                                size_t whiteSpace = itVasm->_code.find_first_of("  \n\r\f\t\v", second + secondMatch[j].size());
                                                std::string secondVar = itVasm->_code.substr(second + secondMatch[j].size(), whiteSpace);
                                                //fprintf(stderr, "StwLdwPair: %s %s\n", firstVar.c_str(), secondVar.c_str());
                                                if(firstVar == secondVar)
                                                {
                                                    linesDeleted = true;
                                                    itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                                    itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                                    adjustLabelAddresses(_codeLines[i]._labelIndex, _codeLines[i]._vasm[firstLine]._address, -4);
                                                    adjustVasmAddresses(i, firstLine, -4);
                                                }
                                            }
                                            break;

                                            // Remove superfluous STW
                                            case StwPair:
                                            case ExtraStw:
                                            {
                                                //fprintf(stderr, "StwPair: %s\n", _codeLines[i]._vasm[firstLine]._code.c_str());
                                                linesDeleted = true;
                                                itVasm = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                                adjustLabelAddresses(_codeLines[i]._labelIndex, _codeLines[i]._vasm[firstLine]._address, -2);
                                                adjustVasmAddresses(i, firstLine, -2);
                                            }
                                            break;
                                        }
                                    }
                                }

                                firstLine = 0;
                                firstVar = "";
                                firstFound = false;
                            }
                        }
                        break;

                        // Matched singles
                        case AddiZero:
                        case SubiZero:
                        {
                            // Arithmetic with zero
                            std::string operand;
                            size_t pos = itVasm->_code.find(firstMatch[j]);
                            if(pos != std::string::npos)
                            {
                                operand = itVasm->_code.substr(pos + firstMatch[j].size());
                                if(operand == "0" || operand == "0x00")
                                {
                                    linesDeleted = true;
                                    itVasm = _codeLines[i]._vasm.erase(itVasm);
                                    adjustLabelAddresses(_codeLines[i]._labelIndex, _codeLines[i]._vasm[firstLine]._address, -2);
                                    adjustVasmAddresses(i, int(itVasm - _codeLines[i]._vasm.begin()), -2);
                                }
                            }
                        }
                        break;
                    }

                    if(!linesDeleted) itVasm++;
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
                                   const std::string& opcode, const std::string& operand, uint16_t address)
    {
        if(itVasm >= itCode->_vasm.end())
        {
            //itCode->_vasm.push_back({address, opcode, operand, "", -1, true});
            //return itCode->_vasm.end() - 1;

            fprintf(stderr, "Compiler::insertPageJumpInstruction() : Trying to insert a PAGE JUMP into lala land, in '%s'", itCode->_code.c_str());
            _EXIT_(EXIT_FAILURE);
        }

        return itCode->_vasm.insert(itVasm, {address, opcode, operand, "", -1, true});
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
                uint16_t vasmStartPC = itCode->_vasm[0]._address;

#if 0
                // Each line of basic code must be smaller than 243 bytes for memory map 0x0200 to 0x04FF
                if(vasmStartPC < 0x0500)
                {
                    if(itCode->_vasmSize >= 243)
                    {
                        fprintf(stderr, "Compiler::checkExclusionZones() : BASIC line at %04x is bigger than 243 bytes, (%d bytes) in '%s' on line %d\n", vasmStartPC, itCode->_vasmSize, itCode->_code.c_str(), codeLineIndex);
                        return false;
                    }
                }
                // Each line of basic code must be smaller than 249 bytes for memory map 0x0500 to 0x05FF, (user stack 0x0600 to 0x06FF)
                else if(itCode->_vasmSize < 0x0600)
                {
                    if(itCode->_vasmSize >= 249)
                    {
                        fprintf(stderr, "Compiler::checkExclusionZones() : BASIC line at %04x is bigger than 249 bytes, (%d bytes) in '%s' on line %d\n", vasmStartPC, itCode->_vasmSize, itCode->_code.c_str(), codeLineIndex);
                        return false;
                    }
                }
                // Each line of basic code must be smaller than 90 bytes for memory map 0x8A00 to 0x8AFF <--> 6FA0 to 6FFF, (internal routines occupy 0x70A0 to 0x7FFF)
                else
                {
                    if(itCode->_vasmSize >= 90)
                    {
                        fprintf(stderr, "Compiler::checkExclusionZones() : BASIC line at %04x is bigger than 90 bytes, (%d bytes) in '%s' on line %d\n", vasmStartPC, itCode->_vasmSize, itCode->_code.c_str(), codeLineIndex);
                        return false;
                    }
                }
#endif

                uint8_t hPC = HI_BYTE(itCode->_vasm[0]._address);
                uint16_t nextPC = (hPC + 1) <<8;
                for(auto itVasm=itCode->_vasm.begin(); itVasm!=itCode->_vasm.end();)
                {
                    resetCheck = false;
                    int vasmLineIndex = int(itVasm - itCode->_vasm.begin());

                    uint8_t lPC = LO_BYTE(itVasm->_address);
                    uint8_t audioExcl = 0xF2;
                    uint8_t pageExcl = 0xF8;

                    // Check MACRO opcodes
                    std::string macroOpcode = itVasm->_opcode;
                    if(macroOpcode.size()  &&  macroOpcode[0] == '%')
                    {
                        macroOpcode.erase(0, 1);

                        if(_macroIndexEntries.find(macroOpcode) != _macroIndexEntries.end())
                        {
                            int macroSize = _macroIndexEntries[macroOpcode]._byteSize;
                            audioExcl -= macroSize;
                            pageExcl -= macroSize;
                        }
                    }

                    if(itVasm->_pageJump == false  &&  (lPC > pageExcl  ||  ((hPC == 0x02 || hPC == 0x03 || hPC == 0x04)  &&  lPC > audioExcl)))
                    {
                        std::string operandSTW, operandLDWI, operandCALL, operandLDW;
                        std::vector<std::string> tokens;
                        uint16_t currPC = (vasmLineIndex > 0) ? itCode->_vasm[vasmLineIndex-1]._address : itVasm->_address;

                        // Insert page jump, (save and restore vAC)
                        int sizeSTW  = createVcpuAsm("STW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, operandSTW);
                        int sizeLDWI = createVcpuAsm("LDWI", Expression::wordToHexString(nextPC), codeLineIndex, operandLDWI);
                        int sizeCALL = createVcpuAsm("CALL", "giga_vAC", codeLineIndex, operandCALL);
                        int sizeLDW  = createVcpuAsm("LDW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, operandLDW);
                        itVasm = itCode->_vasm.insert((vasmLineIndex > 0) ? itVasm-1 : itVasm, {currPC, "STW", operandSTW, "", -1, true});
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDWI", operandLDWI, uint16_t(currPC + sizeSTW));
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "CALL", operandCALL, uint16_t(currPC + sizeSTW + sizeLDWI));
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDW", operandLDW, uint16_t(nextPC));

                        // Create page jump label, (created later in outputCode())
                        itCode->_vasm[itVasm - itCode->_vasm.begin()]._labelInternal = Expression::wordToHexString(nextPC);

                        // New page address is offset by size of vAC restore
                        nextPC += sizeLDW;
                        int offset = nextPC - currPC;

                        // Fix labels and addresses
                        adjustExclusionLabelAddresses(currPC, offset);
                        adjustExclusionVasmAddresses(codeLineIndex, currPC, nextPC, offset);

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
                int gotoLabelIndex = _codeLines[i]._vasm[j]._gotoLabelIndex;
                if(gotoLabelIndex >= 0)
                {
                    //if(HI_MASK(_codeLines[i]._vasm[j]._address) != HI_MASK(_labels[gotoLabelIndex]._address))
                    //{
                    //    fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", _labels[gotoLabelIndex]._address,
                    //                                                                                                                   _codeLines[i]._vasm[j]._address, 
                    //                                                                                                                   _codeLines[i]._code.c_str(), i);
                    //    return false;
                    //}
                }
            }
        }

        return true;
    }


    void outputReservedWords(void)
    {
        std::string line = "_startAddress_ ";
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        line += "EQU\t\t" + Expression::wordToHexString(USER_CODE_START) + "\n";
        _output.push_back(line);

        // No call table for compiler, save page 0 for user variables
        //line = "_callTable_ ";
        //Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        //line += "EQU\t\t" + Expression::wordToHexString(CALL_TABLE_START) + "\n\n";
        //_output.push_back(line);
    }

    void outputLabels(void)
    {
        std::string line;

        _output.push_back("; Labels\n");

        // BASIC labels
        for(int i=0; i<_labels.size(); i++)
        {
            std::string address = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU\t\t" + address + "\n");
        }

        // Internal labels, (used by page jumps)
        for(int i=0; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                std::string labelInternal = _codeLines[i]._vasm[j]._labelInternal;
                if(labelInternal.size())
                {
                    std::string address = Expression::wordToHexString(_codeLines[i]._vasm[j]._address);
                    _output.push_back(labelInternal + std::string(LABEL_TRUNC_SIZE - labelInternal.size(), ' ') + "EQU\t\t" + address + "\n");
                }
            }
        }

        _output.push_back("\n");
    }

    void outputInternalSubs(void)
    {
        _output.push_back("clearRegion     EQU     " + Expression::wordToHexString(INT_FUNC_START) + "\n");
        _output.push_back("resetVideoTable EQU     clearRegion - 0x0100\n");
        _output.push_back("clearCursorRow  EQU     clearRegion - 0x0200\n");
        _output.push_back("printText       EQU     clearRegion - 0x0300\n");
        _output.push_back("printDigit      EQU     clearRegion - 0x0400\n");
        _output.push_back("printVarInt16   EQU     clearRegion - 0x0500\n");
        _output.push_back("printChar       EQU     clearRegion - 0x0600\n");
        _output.push_back("printHexByte    EQU     clearRegion - 0x0700\n");
        _output.push_back("newLineScroll   EQU     clearRegion - 0x0800\n");
        _output.push_back("resetAudio      EQU     clearRegion - 0x0900\n");
        _output.push_back("playMidi        EQU     clearRegion - 0x0A00\n");
        _output.push_back("midiStartNote   EQU     clearRegion - 0x0B00\n");
        _output.push_back("\n");
    }

    void outputInternalVars(void)
    {
        _output.push_back("; Internal variables\n");
        _output.push_back("register0       EQU     " + Expression::wordToHexString(INT_VAR_START) + "\n");
        _output.push_back("register1       EQU     register0 + 0x02\n");
        _output.push_back("register2       EQU     register0 + 0x04\n");
        _output.push_back("register3       EQU     register0 + 0x06\n");
        _output.push_back("register4       EQU     register0 + 0x08\n");
        _output.push_back("register5       EQU     register0 + 0x0A\n");
        _output.push_back("register6       EQU     register0 + 0x0C\n");
        _output.push_back("register7       EQU     register0 + 0x0E\n");
        _output.push_back("register8       EQU     register0 + 0x10\n");
        _output.push_back("register9       EQU     register0 + 0x12\n");
        _output.push_back("textColour      EQU     register0 + 0x14\n");
        _output.push_back("cursorXY        EQU     register0 + 0x16\n");
        _output.push_back("midiStreamPtr   EQU     register0 + 0x18\n");
        _output.push_back("midiDelay       EQU     register0 + 0x1A\n");
        _output.push_back("frameCountPrev  EQU     register0 + 0x1C\n");
        _output.push_back("\n");
    }

    void outputIncludes(void)
    {
        _output.push_back("; Includes\n");
        _output.push_back("%include include/gigatron.i\n");
        _output.push_back("%include include/audio.i\n");
        _output.push_back("%include include/clear_screen.i\n");
        _output.push_back("%include include/print_text.i\n");
        _output.push_back("%include include/macros.i\n");
        _output.push_back("\n");
    }

    void outputVars(void)
    {
        _output.push_back("; Variables\n");

        for(int i=0; i<_integerVars.size(); i++)
        {
            std::string address = Expression::wordToHexString(_integerVars[i]._address);
            _output.push_back(_integerVars[i]._output + "EQU\t\t" + address + "\n");
        }

        _output.push_back("\n");
    }

    void outputStrs(void)
    {
        _output.push_back("; Strings\n");

        for(int i=0; i<_stringVars.size(); i++)
        {
            _output.push_back(_stringVars[i]._output + "EQU\t\t" + Expression::wordToHexString(_stringVars[i]._address) + "\n");
            _output.push_back(_stringVars[i]._output + "DB\t\t" + std::to_string(_stringVars[i]._size) + " '" + _stringVars[i]._data + "'\n");
        }

        _output.push_back("\n");
    }

    void outputCode(void)
    {
        std::string line;

        _output.push_back("; Code\n");

        for(int i=0; i<_codeLines.size(); i++)
        {
            // Valid BASIC code
            if(_codeLines[i]._code.size() > 2  &&  _codeLines[i]._vasm.size())
            {
                // BASIC Label
                std::string vasmCode = _codeLines[i]._vasm[0]._code;
                std::string basicLabel = _labels[_codeLines[i]._labelIndex]._output;
                line = (_codeLines[i]._ownsLabel) ? basicLabel + vasmCode : std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                _vasmCode.push_back(vasmCode);

                // Vasm code
                for(int j=1; j<_codeLines[i]._vasm.size(); j++)
                {
                    // Internal label
                    std::string vasmCode = _codeLines[i]._vasm[j]._code;
                    std::string vasmLabel = _codeLines[i]._vasm[j]._labelInternal;
                    line += (vasmLabel.size() > 0) ?  "\n\n" + vasmLabel + std::string(LABEL_TRUNC_SIZE - vasmLabel.size(), ' ') + vasmCode : "\n" + std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                    _vasmCode.push_back(vasmCode);
                }

#define TAB_SPACE_LENGTH 4
#define COMMENT_PADDING  (TAB_SPACE_LENGTH*13)
                // Commented BASIC code, (assumes tabs are 4 spaces)
                int lineLength = Expression::tabbedStringLength(line, TAB_SPACE_LENGTH);
                line.append(COMMENT_PADDING - (lineLength % COMMENT_PADDING), ' ');
                //fprintf(stderr, "%d\n", lineLength + COMMENT_PADDING - (lineLength % COMMENT_PADDING));
                line += "\t\t; " + _codeLines[i]._text + "\n\n";
                _output.push_back(line);
            }
        }
        
        _output.push_back("\n");
    }


    void clearCompiler(void)
    {
        _vasmPC         = USER_CODE_START;
        _tempVarStart   = TEMP_VAR_START;
        _userVarStart0  = USER_VAR_START_0;
        _userVarStart1  = USER_VAR_START_1;
        _userStrStart   = USER_STR_START;
        _userStackStart = USER_STACK_START;

        _nextTempVar = true;

        _currentLabelIndex = 0;
        _currentCodeLineIndex = 0;

        _tempVarStartStr = "";

        _input.clear();
        _output.clear();

        _labels.clear();
        _codeLines.clear();
        _integerVars.clear();
        _stringVars.clear();
        _arrayVars.clear();

        while(!_forNextDataStack.empty()) _forNextDataStack.pop();

        Memory::intitialise();

        Expression::setExprFunc(expression);
    }

    bool compile(const std::string& inputFilename, const std::string& outputFilename)
    {
        clearCompiler();

        uint16_t address;
        Memory::getRAM(Memory::FitLargest, Memory::RamVasm, 96, address);

        // Read .gbas file
        int numLines = 0;
        std::ifstream infile(inputFilename);
        if(!readInputFile(infile, inputFilename, numLines)) return false;

        fprintf(stderr, "\nCompiling file '%s'\n", inputFilename.c_str());

        // Labels
        if(!parseLabels(numLines)) return false;

        // Vars
        if(!parseVars()) return false;

        // Code
        if(!parseCode()) return false;

        // Optimise
        if(!optimiseCode()) return false;

        // Check code exclusion zones
        if(!checkExclusionZones()) return false;

        // Check branch labels
        if(!checkBranchLabels()) return false;

        // Output
        outputReservedWords();
        outputInternalSubs();
        outputInternalVars();
        outputIncludes();
        outputLabels();
        outputVars();
        outputStrs();
        outputCode();

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        return true;
    }
}