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
#include "expression.h"
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
#define USER_CODE_START  0x0200
#define USER_STACK_START 0x06FF
#define USER_STR_START   0x6FA0
#define INT_FUNC_START   0x7FA0
#define USER_VAR_END_0   0x007F
#define USER_VAR_END_1   0x009F


namespace Compiler
{
    enum VarType {VarInt8=0, VarInt16, VarInt32, VarFloat16, VarFloat32};
    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated};
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
        bool _longJump = false;
        bool _gosub = false;
    };

    struct VasmLine
    {
        uint16_t _address;
        std::string _opcode;
        std::string _code;
        std::string _label;
        int _gotoLabelIndex = -1;
        bool _longJump = false;
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
        int16_t _init;
        int16_t _data;
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
        IntSize _intSize = Int16;
    };

    struct FloatVar
    {
        int16_t _init;
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

    using KeywordFuncPtr = std::function<bool (CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)>;
    struct Keyword
    {
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

    std::vector<Keyword> _keywords;
    std::vector<Keyword> _mathwords;
    std::vector<Keyword> _stringwords;

    std::vector<std::string> _operators;
    std::vector<std::string> _relational;
    std::vector<std::string> _delimiters;
    std::vector<std::string> _keywordsWithEquals;

    std::vector<std::string> _input;
    std::vector<std::string> _output;

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


    bool handleREM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleLET(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleEND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleGOTO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleCLS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handlePRINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleFOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleNEXT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleTHEN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleELSE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleELSEIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleENDIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDIM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDEF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleINPUT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleREAD(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDATA(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handlePEEK(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handlePOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDEEK(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleON(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleGOSUB(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleDO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleLOOP(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleWHILE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleUNTIL(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleEXIT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleAND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleXOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleNOT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);

    bool handleABS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleACS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleASC(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleASN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleATN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleCOS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleEXP(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleLOG(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleRND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleSIN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleSQR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleTAN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);

    bool handleCHR$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleHEX$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleMID$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleLEFT$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleRIGHT$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleSPC$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleSTR$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool handleTIME$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);

    
    bool initialise(void)
    {
        //_keywords.push_back({"REM",    handleREM});
        //_keywords.push_back({"LET",    handleLET});
        _keywords.push_back({"END",    handleEND});
        _keywords.push_back({"GOTO",   handleGOTO});
        _keywords.push_back({"CLS",    handleCLS});
        _keywords.push_back({"PRINT",  handlePRINT});
        _keywords.push_back({"FOR",    handleFOR});
        _keywords.push_back({"NEXT",   handleNEXT});
        _keywords.push_back({"IF",     handleIF});
        _keywords.push_back({"THEN",   handleTHEN});
        _keywords.push_back({"ELSE",   handleELSE});
        _keywords.push_back({"ELSEIF", handleELSEIF});
        _keywords.push_back({"ENDIF",  handleENDIF});
        _keywords.push_back({"DIM",    handleDIM});
        _keywords.push_back({"DEF",    handleDEF});
        _keywords.push_back({"INPUT",  handleINPUT});
        _keywords.push_back({"READ",   handleREAD});
        _keywords.push_back({"DATA",   handleDATA});
        _keywords.push_back({"PEEK",   handlePEEK});
        _keywords.push_back({"POKE",   handlePOKE});
        _keywords.push_back({"DEEK",   handleDEEK});
        _keywords.push_back({"DOKE",   handleDOKE});
        _keywords.push_back({"ON",     handleON});
        _keywords.push_back({"GOSUB",  handleGOSUB});
        _keywords.push_back({"RETURN", handleRETURN});
        _keywords.push_back({"DO",     handleDO});
        _keywords.push_back({"LOOP",   handleLOOP});
        _keywords.push_back({"WHILE",  handleWHILE});
        _keywords.push_back({"UNTIL",  handleUNTIL});
        _keywords.push_back({"EXIT",   handleEXIT});
        _keywords.push_back({"AND",    handleAND});
        _keywords.push_back({"OR",     handleOR});
        _keywords.push_back({"XOR",    handleXOR});
        _keywords.push_back({"NOT",    handleNOT});

        _mathwords.push_back({"ABS", handleABS});
        _mathwords.push_back({"ACS", handleACS});
        _mathwords.push_back({"ASC", handleASC});
        _mathwords.push_back({"ASN", handleASN});
        _mathwords.push_back({"ATN", handleATN});
        _mathwords.push_back({"COS", handleCOS});
        _mathwords.push_back({"EXP", handleEXP});
        _mathwords.push_back({"INT", handleINT});
        _mathwords.push_back({"LOG", handleLOG});
        _mathwords.push_back({"RND", handleRND});
        _mathwords.push_back({"SIN", handleSIN});
        _mathwords.push_back({"SQR", handleSQR});
        _mathwords.push_back({"TAN", handleTAN});

        _stringwords.push_back({"CHR$",   handleCHR$});
        _stringwords.push_back({"HEX$",   handleHEX$});
        _stringwords.push_back({"MID$",   handleMID$});
        _stringwords.push_back({"LEFT$",  handleLEFT$});
        _stringwords.push_back({"RIGHT$", handleRIGHT$});
        _stringwords.push_back({"SPC$",   handleSPC$});
        _stringwords.push_back({"STR$",   handleSTR$});
        _stringwords.push_back({"TI$",    handleTIME$});
        _stringwords.push_back({"TIME$",  handleTIME$});

        _operators.push_back("-");
        _operators.push_back("+");
        _operators.push_back("/");
        _operators.push_back("*");
        _operators.push_back("(");
        _operators.push_back(")");

        _relational.push_back("<");
        _relational.push_back(">");
        _relational.push_back("=");
        _relational.push_back("<>");
        _relational.push_back("<=");
        _relational.push_back(">=");

        _delimiters.push_back(" ");
        _delimiters.push_back("\"");
        _delimiters.push_back(":");
        _delimiters.push_back(";");
        _operators.push_back("(");
        _operators.push_back(")");
        _operators.push_back("[");
        _operators.push_back("]");

        //_keywordsWithEquals.push_back("REM");
        //_keywordsWithEquals.push_back("LET");
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
        varName = varName.substr(0, varName.find_first_of("-+/*()"));
        for(int i=0; i<_integerVars.size(); i++)
        {
            if(_integerVars[i]._name == varName) return i;
        }

        return -1;
    }


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label, bool addUnderscore=true, bool longJump=false, bool gosub=false)
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

        label = {address, n, o, codeLineIndex, longJump, gosub};
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
        IntegerVar integerVar = {data, data, userVarStart, varName, varName, codeLineIndex, Int16};
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
        std::string expression = "";
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

        std::vector<VasmLine> vasm;
        std::string codeSub = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        Expression::trimWhitespace(codeSub);
        std::vector<std::string> tokens = Expression::tokenise(codeSub, ' ', true);
        codeLine = {codeSub, codeSub, tokens, vasm, expression, 0, labelIndex, varIndex, varType, assign, vars, ownsLabel};
        Expression::stripNonStringWhitespace(codeLine._code);
        Expression::stripWhitespace(codeLine._expression);
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

    int insertVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, int vasmLineIdx, int gotoLabelIndex=-1, bool longJump=false)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.insert(_codeLines[codeLineIdx]._vasm.begin() + vasmLineIdx, {uint16_t(_vasmPC - vasmSize), opcodeStr, line, "", gotoLabelIndex, longJump});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLineIndex, const std::string& label="", int gotoLabelIndex=-1, bool longJump=false)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, line, label, gotoLabelIndex, longJump});
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
        std::string _label = label;

        for(int i=0; i<_gosubLabels.size(); i++)
        {
            if(_gosubLabels[i] == Expression::strToUpper(_label)) return true;
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
                    fprintf(stderr, "Compiler::checkForGosubLabel() : missing label after GOSUB in : '%s' : on line %d\n", code.c_str(), lineNumber);
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
                fprintf(stderr, "Compiler::checkForLabel() : white space expected after line mumber in : '%s' : on line %d\n", code.c_str(), lineNumber);
                return LabelError;
            }
            for(int i=1; i<space; i++)
            {
                if(!isdigit(code[i]))
                {
                    fprintf(stderr, "Compiler::checkForLabel() : non digits found in line number in : '%s' : on line %d\n", code.c_str(), lineNumber);
                    return LabelError;
                }
            }

            if(code.size() - (space + 1) <= 2)
            {
                fprintf(stderr, "Compiler::checkForLabel() : line number cannot exist on its own : '%s' : on line %d\n", code.c_str(), lineNumber);
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
            if(!createCodeLine(code, 0, _currentLabelIndex, -1, VarInt16, false, false, false, codeLine)) return LabelError;
            return LabelNotFound;
        }
        if(colon2 != std::string::npos)
        {
            fprintf(stderr, "Compiler::checkForLabel() : only one label per line is allowed in : '%s' : on line %d\n", code.c_str(), lineNumber);
            return LabelError;
        }

        if(code.size() - (colon1 + 1) <= 2)
        {
            fprintf(stderr, "Compiler::checkForLabel() : label cannot exist on its own : '%s' : on line %d\n", code.c_str(), lineNumber);
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

    Expression::ExpressionType isExpression(const std::string& input)
    {
        if(input.find("$") != std::string::npos) return Expression::IsString;
        if(input.find("\"") != std::string::npos) return Expression::IsString;
        if(find_if(input.begin(), input.end(), isalpha) != input.end()) return Expression::HasAlpha;
        return Expression::isExpression(input);
    }

    VarResult checkForVars(const std::string& code, int& varIndex, int codeLineIndex)
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
                fprintf(stderr, "Compiler::checkForVars() : too many '=' in '%s'\n", code.c_str());
                return VarError;
            }

            std::string varName = code.substr(0, equals1);
            Expression::stripWhitespace(varName);
            
            // Has var been initialised
            varIndex = findVar(varName);

            // Evaluate var
            int16_t result = 0;
            bool containsVars = false;
            std::string data = code.substr(equals1 + 1, code.size() - (equals1 + 1));
            Expression::stripNonStringWhitespace(data);
            Expression::operatorReduction(data);
            Expression::setExprFunc(Expression::expression);
            Expression::ExpressionType expressiontype = isExpression(data);
            switch(expressiontype)
            {
                case Expression::HasAlpha: result = 0; containsVars = true; break;

                case Expression::None:
                case Expression::Valid:
                {
                    if(!Expression::parse((char*)data.c_str(), codeLineIndex, result)) return VarError;
                }
                break;

                case Expression::Invalid:
                {
                    fprintf(stderr, "Compiler::checkForVars() : invalid expression '%s' on line %d\n", data.c_str(), codeLineIndex);
                    return VarError;
                }
                break;
            }

            // TODO: only works with Int16, fix for all var types
            // Save result to var
            if(varIndex >= 0)
            {
                updateVar(result, codeLineIndex, varIndex, containsVars);
                return VarUpdated;
            }
            // Initialise var
            else
            {
                createVar(varName, result, codeLineIndex, containsVars, varIndex);
                return VarCreated;
            }
        }

        return VarNotFound;
    }

    bool parseVars(void)
    {
        std::string line;

        for(int i=0; i<_codeLines.size(); i++)
        {
            // REM and LET modify code
            size_t foundPos;
            KeywordFuncResult result;
            if(findKeyword(_codeLines[i]._code, "REM", foundPos))
            {
                handleREM(_codeLines[i], 0, foundPos, result);
            }
            else if(findKeyword(_codeLines[i]._code, "LET", foundPos))
            {
                handleLET(_codeLines[i], 0, foundPos, result);
            }

            int varIndex;
            VarResult varResult = checkForVars(_codeLines[i]._code, varIndex, i);
            if(varResult == VarError) return false;
        }

        return true;
    }


    bool handleAddSub(const std::string& opcodeStr, Expression::Numeric& lhs, Expression::Numeric& rhs)
    {
        std::string opcode = std::string(opcodeStr);

        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(!rhs._isAddress  &&  abs(rhs._value) > 255)
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
            if(isdigit(*lhs._varNamePtr))
            {
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(lhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("LDW", lhs._varNamePtr, true)) return false;
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
            if(isdigit(*rhs._varNamePtr))
            {
                emitVcpuAsm(opcode + "W", Expression::byteToHexString(uint8_t(rhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar(opcode + "W", rhs._varNamePtr, _nextTempVar)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            emitVcpuAsm(opcode + "I", std::to_string(rhs._value), false);
        }

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varNamePtr = (char *)_tempVarStartStr.c_str();

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return true;
    }

    // Expression operators
    Expression::Numeric neg(Expression::Numeric& numeric)
    {
        if(!numeric._isAddress)
        {
            numeric._value = -numeric._value;
            return numeric;
        }

        std::string varName = std::string(numeric._varNamePtr);
        int varIndex = findVar(varName);
        if(varIndex == -1)
        {
            fprintf(stderr, "Compiler::neg() : couldn't find variable name '%s'\n", varName.c_str());
            return numeric;
        }
        else
        {
            getNextTempVar();
            emitVcpuAsm("LDI", std::to_string(0), false);
            emitVcpuAsm("SUBW", "_" + varName, false);
            emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);
        }

        numeric._value = uint8_t(_tempVarStart);
        numeric._isAddress = true;
        numeric._varNamePtr = (char *)_tempVarStartStr.c_str();

        return numeric;
    }

    Expression::Numeric add(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value += right._value;
            return left;
        }

        left._isValid = handleAddSub("ADD", left, right);
        return left;
    }

    Expression::Numeric sub(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value -= right._value;
            return left;
        }

        left._isValid = handleAddSub("SUB", left, right);
        return left;
    }

    Expression::Numeric mul(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value *= right._value;
            return left;
        }

        // Optimise multiply with 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, true, false, (char*)"");

        return left;
    }

    Expression::Numeric div(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value /= right._value;
            return left;
        }

        // Optimise divide with 0, term() never lets denominator = 0
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, true, false, (char*)"");

        return left;
    }

    Expression::Numeric expression(void);
    Expression::Numeric fac(int16_t defaultValue)
    {
        int16_t value = 0;
        Expression::Numeric numeric;

        if(Expression::peek() == '(')
        {
            Expression::get();
            numeric = expression();
            if(Expression::peek() != ')')
            {
                fprintf(stderr, "Compiler::factor() : Found '%c' : expecting ')' in '%s' on line %d\n", Expression::peek(), Expression::getExpressionToParse(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, false, false, nullptr);
            }
            Expression::get();
        }
        else if(Expression::peek() == '-')
        {
            Expression::get();
            numeric = fac(0);
            numeric = neg(numeric);
        }
        else if((Expression::peek() >= '0'  &&  Expression::peek() <= '9')  ||  Expression::peek() == '$')
        {
            if(!Expression::number(value))
            {
                fprintf(stderr, "Compiler::factor() : Bad numeric data in '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, false, false, nullptr);
            }
            else
            {
                numeric = Expression::Numeric(value, true, false, nullptr);
            }
        }
        else
        {
            numeric = Expression::Numeric(defaultValue, true, true, Expression::getExpression());
            while(isalpha(Expression::peek())) Expression::get();
        }

        return numeric;
    }

    Expression::Numeric term(void)
    {
        Expression::Numeric f, result = fac(0);
        while(Expression::peek() == '*'  ||  Expression::peek() == '/')
        {
            if(Expression::get() == '*')
            {
                f = fac(0);
                result = mul(result, f);
            }
            else
            {
                f = fac(0);
                if(f._value == 0)
                {
                    result = mul(result, f);
                }
                else
                {
                    result = div(result, f);
                }
            }
        }

        return result;
    }

    Expression::Numeric expression(void)
    {
        Expression::Numeric t, result = term();

        while(Expression::peek() == '+' || Expression::peek() == '-')
        {
            if(Expression::get() == '+')
            {
                t = term();
                result = add(result, t);
            }
            else
            {
                t = term();
                result = sub(result, t);
            }
        }

        return result;
    }

    bool varExpressionParse(CodeLine& codeLine, int codeLineIndex)
    {
        int16_t value;
        Expression::setExprFunc(expression);
        return Expression::parse((char*)codeLine._expression.c_str(), codeLineIndex, value);
    }

    int varAssignmentParse(CodeLine& codeLine, int codeLineIndex)
    {
        if(codeLine._expression.find_first_of("-+/*()") != std::string::npos) return -1;
        int varIndex = findVar(codeLine._expression);
        return varIndex;
    }


    KeywordResult handleKeywords(CodeLine& codeLine, size_t offset, int codeLineIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search keywords
        for(int i=0; i<_keywords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _keywords[i]._name, foundPos))
            {
                bool error = _keywords[i]._func(codeLine, codeLineIndex, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }
        
        return KeywordNotFound;
    }

    KeywordResult handleMathwords(CodeLine& codeLine, size_t offset, int codeLineIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search mathwords
        for(int i=0; i<_mathwords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _mathwords[i]._name, foundPos))
            {
                bool error = _mathwords[i]._func(codeLine, codeLineIndex, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }

    KeywordResult handleStringwords(CodeLine& codeLine, const std::string& stringword, int codeLineIndex, KeywordFuncResult& result, size_t& offset)
    {
        size_t foundPos;

        // Handle stringword
        for(int i=0; i<_stringwords.size(); i++)
        {
            if(findKeyword(stringword, _stringwords[i]._name, foundPos))
            {
                offset += foundPos;
                bool error = _stringwords[i]._func(codeLine, codeLineIndex, offset, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }

    KeywordResult handleStringwords(CodeLine& codeLine, size_t offset, int codeLineIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search stringwords
        for(int i=0; i<_stringwords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _stringwords[i]._name, foundPos))
            {
                bool error = _stringwords[i]._func(codeLine, codeLineIndex, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }


    bool handleREM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
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

    bool handleLET(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Remove LET from code
        codeLine._code.erase(foundPos - 3, foundPos);

        return true;
    }

    bool handleEND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        Label label;
        std::string endName = "_end_" + std::to_string(codeLineIndex);
        createLabel(_vasmPC, endName, "END\t", codeLineIndex, label, false, false, false);
        _codeLines[codeLineIndex]._ownsLabel = true;
        _codeLines[codeLineIndex]._labelIndex = _currentLabelIndex;
        emitVcpuAsm("BRA", endName, false, codeLineIndex);

        return true;
    }

    bool handleGOTO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string gotoLabel = codeLine._code.substr(4);
        int labelIndex = findLabel(gotoLabel);
        if(labelIndex == -1)
        {
            fprintf(stderr, "Compiler::handleGOTO() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
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
            emitVcpuAsm("LDWI", "_" + gotoLabel, false, codeLineIndex, "", -1, true);
            emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex, "", -1, true);
        }

        return true;
    }

    bool handleCLS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        emitVcpuAsm("%Initialise", "", false, codeLineIndex);

        return true;
    }

    bool handlePRINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        size_t searchPos = foundPos;

        // Parse print tokens
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', false);
        for(int i=0; i<tokens.size(); i++)
        {
            Expression::ExpressionType expressionType = isExpression(tokens[i]);
            switch(expressionType)
            {
                case Expression::IsString:
                {
                    KeywordResult keywordResult = handleStringwords(codeLine, tokens[i], codeLineIndex, result, searchPos);
                    if(keywordResult == KeywordFound)
                    {
                        if(result._name == "CHR$")
                        {
                            emitVcpuAsm("%PrintAcChar", "", false, codeLineIndex);
                            continue;
                        }
                        else if(result._name == "HEX$")
                        {
                            emitVcpuAsm("%PrintAcHex", "", false, codeLineIndex);
                            continue;
                        }
                    }

                    size_t lquote = tokens[i].find_first_of("\"");
                    size_t rquote = tokens[i].find_first_of("\"", lquote + 1);
                    if(lquote != std::string::npos  &&  rquote != std::string::npos)
                    {
                        if(rquote == lquote + 1) continue; // skip empty strings
                        std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));
                        if(str.size() > USER_STR_SIZE)
                        {
                            fprintf(stderr, "Compiler::handlePRINT() : user string is %d characters too long in '%s' on line %d\n", int(str.size() - USER_STR_SIZE), codeLine._code.c_str(), codeLineIndex);
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
                                fprintf(stderr, "Compiler::handlePRINT() : out of string memory, user string address %04x in '%s' on line %d\n", _userStrStart, codeLine._code.c_str(), codeLineIndex);
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
                    Expression::setExprFunc(Expression::expression);
                    if(!Expression::parse((char*)tokens[i].c_str(), codeLineIndex, result)) return false;
                    emitVcpuAsm("%PrintInt16", Expression::wordToHexString(result), false, codeLineIndex);
                }
                break;

                case Expression::HasAlpha:
                {
                    // Search math words and string words
                    KeywordResult keywordResult = handleMathwords(codeLine, foundPos, codeLineIndex, result);
                    if(keywordResult == KeywordNotFound) keywordResult = handleStringwords(codeLine, foundPos, codeLineIndex, result);
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
                    fprintf(stderr, "Compiler::handlePRINT() : invalid input in '%s' on line %d\n", tokens[i].c_str(), codeLineIndex);
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
    }

    bool handleFOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() < 6  ||  codeLine._tokens.size() > 8)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        if(codeLine._tokens[2] != "=")
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (missing '='), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        if(Expression::strToUpper(codeLine._tokens[4]) != "TO")
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (missing 'TO'), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
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
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (bad FOR start), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Var counter, (create or update if being reused)
        int varIndex = findVar(codeLine._tokens[1]);
        (varIndex < 0) ? createVar(codeLine._tokens[1], loopStart, codeLineIndex, false, varIndex) : updateVar(loopStart, codeLineIndex, varIndex, false);

        // Loop end
        int16_t loopEnd;
        if(!Expression::stringToI16(codeLine._tokens[5], loopEnd))
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (bad FOR end), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

#if 0
        // Maximum of 4 nested loops
        if(_forNextDataStack.size() == 4)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        int offset = int(_forNextDataStack.size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        emitVcpuAsm("%ForNextInitVsVe", "_" + _integerVars[varIndex]._name + " " + std::to_string(loopStart) + " " + std::to_string(loopEnd) + " 1" + " " + Expression::wordToHexString(varEnd) + " " + Expression::wordToHexString(varStep), false, codeLineIndex);
#endif

        emitVcpuAsm("LDWI", std::to_string(loopStart), false, codeLineIndex);

        // Create FOR loop label, (label is attached to line after for loop initialisation)
        Label label;
        int lineAfterLoopInit = _currentCodeLineIndex + 1;
        std::string name = "_next" + std::to_string(_currentCodeLineIndex); // + "_" + _integerVars[varIndex]._name;
        createLabel(_vasmPC, name, name + "\t", lineAfterLoopInit, label, false, false, false);
        _codeLines[lineAfterLoopInit]._ownsLabel = true;
        _codeLines[lineAfterLoopInit]._labelIndex = _currentLabelIndex;

        // Update all lines belonging to this label
        for(int i=_currentCodeLineIndex + 2; i<_codeLines.size(); i++)
        {
            if(!_codeLines[i]._ownsLabel) _codeLines[i]._labelIndex = _currentLabelIndex;
        }

        // FOR loops that have inputs as variables use a stack
        _forNextDataStack.push({varIndex, _codeLines[lineAfterLoopInit]._labelIndex, lineAfterLoopInit, loopEnd, 1, 0x00, 0x00});

        return true;
    }

    bool handleNEXT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::handleNEXT() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::handleNEXT() : syntax error, (bad var), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        ForNextData forNextData = _forNextDataStack.top();
        _forNextDataStack.pop();
        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Compiler::handleNEXT() : syntax error, (wrong var), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        emitVcpuAsm("%ForNextLoopP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + std::to_string(forNextData._loopEnd), false, codeLineIndex, "", forNextData._labelIndex);

#if 0
        emitVcpuAsm("%ForNextLoopVsVeP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + Expression::wordToHexString(forNextData._varEnd) + " " + Expression::wordToHexString(forNextData._varStep), false, codeLineIndex, "", forNextData._labelIndex);
#endif

        return true;
    }

    bool handleIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTHEN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleELSE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleELSEIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleENDIF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDIM(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDEF(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleINPUT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleREAD(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDATA(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handlePEEK(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handlePOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDEEK(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDOKE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleON(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleGOSUB(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        std::string gosubLabel = codeLine._code.substr(5);
        int labelIndex = findLabel(gosubLabel);
        if(labelIndex == -1)
        {
            fprintf(stderr, "Compiler::handleGOSUB() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        _labels[labelIndex]._gosub = true;

        emitVcpuAsm("LDWI", "_" + gosubLabel, false, codeLineIndex);
        emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);

        return true;
    }
    bool handleRETURN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        emitVcpuAsm("POP", "", false, codeLineIndex);
        emitVcpuAsm("RET", "", false, codeLineIndex);

        return true;
    }

    bool handleDO(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLOOP(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleWHILE(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleUNTIL(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleEXIT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleAND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleXOR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleNOT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleABS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleACS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleASC(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleASN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleATN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleCOS(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleEXP(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleINT(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLOG(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleRND(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSIN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSQR(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTAN(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleCHR$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse chr$
        //size_t lbra = codeLine._code.find_first_of("(", foundPos);
        //size_t rbra = codeLine._code.find_first_of(")", lbra + 1);
        //if(lbra != std::string::npos  &&  rbra != std::string::npos)
        size_t lbra, rbra;
        if(Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            std::string expr = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            Expression::ExpressionType expressionType = isExpression(expr);
            switch(expressionType)
            {
                case Expression::None:
                case Expression::Valid:
                {
                    Expression::setExprFunc(Expression::expression);
                    if(!Expression::parse((char*)expr.c_str(), codeLineIndex, result._data)) return false;
                    emitVcpuAsm("LDI", std::to_string(result._data), false, codeLineIndex);
                }
                break;

                case Expression::HasAlpha:
                {
                    CodeLine cl = codeLine;
                    cl._code = cl._expression = expr;
                    if(!varExpressionParse(cl, codeLineIndex)) return false;
                    int varIndex = varAssignmentParse(cl, codeLineIndex);
                    (varIndex >= 0) ? emitVcpuAsm("LD", "_" + _integerVars[varIndex]._name, false, codeLineIndex) : emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
                }
                break;

                default:
                {
                    fprintf(stderr, "Compiler::handleCHR$() : invalid input in '%s' on line %d\n", expr.c_str(), codeLineIndex);
                    return false;
                }
                break;
            }
        }
        else
        {
            fprintf(stderr, "Compiler::handleCHR$() : expecting () in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        result._name = "CHR$";

        return true;
    }

    bool handleHEX$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse hex$
        size_t lbra, rbra;
        if(Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            std::string expr = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            Expression::ExpressionType expressionType = isExpression(expr);
            switch(expressionType)
            {
                case Expression::None:
                case Expression::Valid:
                {
                    Expression::setExprFunc(Expression::expression);
                    if(!Expression::parse((char*)expr.c_str(), codeLineIndex, result._data)) return false;
                    emitVcpuAsm("LDI", std::to_string(result._data), false, codeLineIndex);
                }
                break;

                case Expression::HasAlpha:
                {
                    CodeLine cl = codeLine;
                    cl._code = cl._expression = expr;
                    if(!varExpressionParse(cl, codeLineIndex)) return false;
                    int varIndex = varAssignmentParse(cl, codeLineIndex);
                    (varIndex >= 0) ? emitVcpuAsm("LD", "_" + _integerVars[varIndex]._name, false, codeLineIndex) : emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
                }
                break;

                default:
                {
                    fprintf(stderr, "Compiler::handleHEX$() : invalid input in '%s' on line %d\n", expr.c_str(), codeLineIndex);
                    return false;
                }
                break;
            }
        }
        else
        {
            fprintf(stderr, "Compiler::handleHEX$() : expecting () in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        result._name = "HEX$";

        return true;
    }

    bool handleMID$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLEFT$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleRIGHT$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSPC$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSTR$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTIME$(CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool createVasmCode(CodeLine& codeLine, int codeLineIndex)
    {
#if 0
        // Temporary var expression, i.e. within ()
        if(codeLine._containsVars)
        {
            if(!varExpressionParse(codeLine, codeLineIndex)) return false;
            emitVcpuAsm("STW", Expression::wordToHexString(TEMP_VAR_START), false, codeLineIndex);
        }
#endif

        if(_labels[codeLine._labelIndex]._gosub) emitVcpuAsm("PUSH", "", false, codeLineIndex);

        KeywordFuncResult result;
        KeywordResult keywordResult = handleKeywords(codeLine, 0, codeLineIndex, result);
        if(keywordResult == KeywordNotFound) keywordResult = handleMathwords(codeLine, 0, codeLineIndex, result);
        if(keywordResult == KeywordNotFound) keywordResult = handleStringwords(codeLine, 0, codeLineIndex, result);
        else if(keywordResult == KeywordError) return false;

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
                if(!varExpressionParse(codeLine, codeLineIndex)) return false;
                int varIndex = varAssignmentParse(codeLine, codeLineIndex);

                // Optimise LDW away if possible
                if((varIndex >= 0  &&  varIndex != prevVarIndex  &&  keywordResult != KeywordFound)  ||  _labels[codeLine._labelIndex]._gosub == true)
                {
                    emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
                }
                emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, codeLineIndex);
                prevVarIndex = codeLine._varIndex;
            }
            // Standard assignment
            else
            {
                if(keywordResult != KeywordFound)
                {
                    // 8bit constants
                    if(_integerVars[codeLine._varIndex]._init >=0  &&  _integerVars[codeLine._varIndex]._init <= 255)
                    {
                        emitVcpuAsm("LDI", std::to_string(_integerVars[codeLine._varIndex]._init), false, codeLineIndex);
                    }
                    // 16bit constants
                    else
                    {
                        emitVcpuAsm("LDWI", std::to_string(_integerVars[codeLine._varIndex]._init), false, codeLineIndex);
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
                if(_codeLines[i]._ownsLabel) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;

                // Create .vasm code
                if(!createVasmCode(_codeLines[i], i)) return false;
            }
        }

        return true;
    }

    enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwPair, ExtraStw, AddiZero, SubiZero, NumOptimiseTypes};
    bool optimiseCode(void)
    {
        const std::string firstMatch[NumOptimiseTypes]  = {"STW\t\t0x", "STW\t\t0x", "STW\t\t0x", "STW\t\t0x", "ADDI\t", "SUBI\t"};
        const std::string secondMatch[NumOptimiseTypes] = {"LDW\t\t0x", "LD\t\t0x",  "STW\t\t0x", "STW\t\t_",  "",       ""      };

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
                                    firstVar = itVasm->_code.substr(first + firstMatch[j].size(), 2);
                                }
                            }
                            else
                            {
                                // Second match must be on next line
                                if(int(itVasm - _codeLines[i]._vasm.begin())  ==  firstLine + 1)
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
                                                if(firstVar == itVasm->_code.substr(second + secondMatch[j].size(), 2))
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
        // Adjust addresses for any non long jump labels with addresses higher than start label, (labels can be stored out of order)
        for(int i=0; i<_labels.size(); i++)
        {
            if(!_labels[i]._longJump  &&  _labels[i]._address > address)
            {
                _labels[i]._address += offset;
            }
        }
    }

    void adjustExclusionVasmAddresses(int codeLineIndex, int offset)
    {
        for(int i=codeLineIndex; i<_codeLines.size(); i++)
        {
            for(int j=0; j<_codeLines[i]._vasm.size(); j++)
            {
                _codeLines[i]._vasm[j]._address += offset;
            }
        }
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

                uint8_t hPC = HI_BYTE(itCode->_vasm[0]._address);
                uint16_t currPC = itCode->_vasm[0]._address;
                uint16_t nextPC = (hPC + 1) <<8;
                for(auto itVasm=itCode->_vasm.begin(); itVasm!=itCode->_vasm.end();)
                {
                    resetCheck = false;

                    uint8_t lPC = LO_BYTE(itVasm->_address);
                    if(itVasm->_longJump == false  &&  ((lPC > 0xF3  &&  (hPC == 0x02 || hPC == 0x03 || hPC == 0x04))  ||  lPC > 0xF9))
                    {
                        std::string line;
                        std::vector<VasmLine> vasm;
                        std::vector<std::string> tokens;
                        int vasmSize0 = createVcpuAsm("LDWI", Expression::wordToHexString(nextPC), codeLineIndex, line);
                        vasm.push_back({currPC, "LDWI", line, "", -1, true});
                        int vasmSize1 = createVcpuAsm("CALL", "giga_vAC", codeLineIndex, line);
                        vasm.push_back({uint16_t(currPC + vasmSize0), "CALL", line, "", -1, true});

                        // Create new dummy code line with old code line's label, (if it existed)
                        CodeLine codeLine = {"PAGE JUMP", "PAGEJUMP", tokens, vasm, "", vasmSize0 + vasmSize1, itCode->_labelIndex, -1, VarInt16, false, false, itCode->_ownsLabel};
                        itCode = _codeLines.insert(itCode, codeLine);

                        // Create long jump label and new code
                        Label label;
                        std::string name = Expression::wordToHexString(nextPC);
                        createLabel(nextPC, name, name + "\t", codeLineIndex + 1, label, false, true, false);
                        _codeLines[codeLineIndex + 1]._labelIndex = int(_labels.size()) - 1;
                        _codeLines[codeLineIndex + 1]._ownsLabel = true;

                        // Fix labels and addresses
                        int offset = nextPC - currPC;
                        adjustExclusionLabelAddresses(itCode->_vasm[0]._address, offset);
                        adjustExclusionVasmAddresses(codeLineIndex + 1, offset);

                        resetCheck = true;
                        break;
                    }

                    itVasm++;
                }
            
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
                    if(HI_MASK(_codeLines[i]._vasm[j]._address) != HI_MASK(_labels[gotoLabelIndex]._address))
                    {
                        fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", _labels[gotoLabelIndex]._address,
                                                                                                                                       _codeLines[i]._vasm[j]._address, 
                                                                                                                                       _codeLines[i]._code.c_str(), i);
                        return false;
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
        line += "EQU\t\t" + Expression::wordToHexString(USER_CODE_START) + "\n";
        _output.push_back(line);

        //line = "_callTable_ ";
        //Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        //line += "EQU\t\t" + Expression::wordToHexString(CALL_TABLE_START) + "\n\n";
        //_output.push_back(line);
    }

    void outputLabels(void)
    {
        std::string line;

        _output.push_back("; Labels\n");

        //std::sort(_labels.begin(), _labels.end(), [](const Label& a, const Label& b) {return a._address < b._address; });
        for(int i=0; i<_labels.size(); i++)
        {
            std::string address = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU\t\t" + address + "\n");
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
        _output.push_back("printHex        EQU     clearRegion - 0x0700\n");
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
                // New line before label, except first
                if(_codeLines[i]._labelIndex > 0) _output.push_back("\n");

                // BASIC Label
                std::string vasmCode = _codeLines[i]._vasm[0]._code;
                std::string basicLabel = _labels[_codeLines[i]._labelIndex]._output;
                line = (_codeLines[i]._ownsLabel) ? basicLabel + vasmCode : std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;

                // Vasm code
                for(int j=1; j<_codeLines[i]._vasm.size(); j++)
                {
                    // Internal label
                    std::string vasmCode = _codeLines[i]._vasm[j]._code;
                    std::string vasmLabel = _codeLines[i]._vasm[j]._label;
                    line += (vasmLabel.size() > 0) ?  "\n" + vasmLabel + std::string(LABEL_TRUNC_SIZE - vasmLabel.size(), ' ') + vasmCode : "\n" + std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                }

#define TAB_SPACE_LENGTH 4
#define COMMENT_PADDING  (TAB_SPACE_LENGTH*13)
                // Commented BASIC code, (assumes tabs are 4 spaces)
                int lineLength = Expression::tabbedStringLength(line, TAB_SPACE_LENGTH);
                line.append(COMMENT_PADDING - (lineLength % COMMENT_PADDING), ' ');
                //fprintf(stderr, "%d\n", lineLength + COMMENT_PADDING - (lineLength % COMMENT_PADDING));
                line += "\t\t; " + _codeLines[i]._text + "\n";
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