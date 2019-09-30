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


#define LABEL_TRUNC_SIZE 16     // The smaller you make this, the more your BASIC label names will be truncated in the resultant .vasm code
#define USER_STR_SIZE    95

#define USER_VAR_START   0x0030  // 80 bytes, (0x0030 <-> 0x007F), reserved for BASIC user variables
#define TEMP_VAR_START   0x0082  // 16 bytes, (0x0082 <-> 0x0091), reserved for temporary expression variables
#define LOOP_VAR_START   0x0092  // 16 bytes, (0x0092 <-> 0x00A1), reserved for loops, maximum of 4 nested loops
#define INT_VAR_START    0x00A2  // internal register variables, used by the BASIC runtime
#define CALL_TABLE_START 0x00EE  // 16 bytes, (0x00F0 <-> 0x00FF), reserved for vCPU stack, allows for 8 nested calls
#define USER_CODE_START  0x0200
#define USER_STACK_START 0x06FF
#define USER_STR_START   0x6FA0
#define INT_FUNC_START   0x7FA0


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
    };

    struct VasmLine
    {
        uint16_t _address;
        std::string _opcode;
        std::string _code;
        std::string _label;
        int gotoLabelIndex = -1;
    };

    struct CodeLine
    {
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

        void initialise(void)
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

    using KeywordFuncPtr = std::function<bool (CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)>;
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

    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart   = USER_VAR_START;
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

    std::vector<Label>      _labels;
    std::vector<CodeLine>   _codeLines;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar>  _stringVars;
    std::vector<ArrayVar>   _arrayVars;

    std::stack<ForNextData> _forNextDataStack;


    bool handleREM(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleLET(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleGOTO(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handlePRINT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleFOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleNEXT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleTHEN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleELSE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleELSEIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleENDIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDIM(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDEF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleINPUT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleREAD(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDATA(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handlePEEK(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handlePOKE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDEEK(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDOKE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleON(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleGOSUB(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleRETURN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleDO(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleLOOP(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleWHILE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleUNTIL(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleEXIT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleAND(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleXOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleNOT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);

    bool handleABS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleACS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleASC(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleASN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleATN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleCOS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleEXP(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleINT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleLOG(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleRND(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleSIN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleSQR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleTAN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);

    bool handleCHR$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleMID$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleLEFT$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleRIGHT$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleSPC$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleSTR$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);
    bool handleTIME$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result);

    
    void initialise(void)
    {
        _keywords.push_back({"REM",    handleREM});
        _keywords.push_back({"LET",    handleLET});
        _keywords.push_back({"GOTO",   handleGOTO});
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

        _keywordsWithEquals.push_back("REM");
        _keywordsWithEquals.push_back("FOR");
        _keywordsWithEquals.push_back("IF");
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


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label, bool addUnderscore=true, bool longJump=false)
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

        label = {address, n, o, codeLineIndex, longJump};
        Expression::stripWhitespace(label._name);
        _labels.push_back(label);
        _currentLabelIndex = int(_labels.size() - 1);
    }

    void createVar(const std::string& varName, int16_t data, int lineNumber, bool containsVars, int& varIndex)
    {
        // Create var
        varIndex = int(_integerVars.size());
        _codeLines[lineNumber]._assignOperator = true;
        _codeLines[lineNumber]._containsVars = containsVars;
        _codeLines[lineNumber]._varType = VarInt16;
        _codeLines[lineNumber]._varIndex = varIndex;
        IntegerVar integerVar = {data, data, _userVarStart, varName, varName, lineNumber, Int16};
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

        _userVarStart += Int16;
    }
    void updateVar(int16_t data, int lineNumber, int varIndex, bool containsVars)
    {
        _codeLines[lineNumber]._assignOperator = true;
        _codeLines[lineNumber]._containsVars = containsVars;
        _codeLines[lineNumber]._varType = VarInt16;
        _codeLines[lineNumber]._varIndex = varIndex;
        _integerVars[varIndex]._data = data;
    }

    bool createCodeLine(uint16_t address, const std::string& code, const std::string& vasmCode, int codeLineOffset, int labelIndex, int varIndex, VarType varType, bool assign, bool vars, bool ownsLabel, CodeLine& codeLine)
    {
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
        if(vasmCode.size()) vasm.push_back({address, "", vasmCode, "", -1});
        std::string codeSub = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        std::vector<std::string> tokens = Expression::tokenise(codeSub, ' ', true);
        codeLine = {codeSub, tokens, vasm, expression, 0, labelIndex, varIndex, varType, assign, vars, ownsLabel};
        Expression::stripNonStringWhitespace(codeLine._code);
        Expression::stripWhitespace(codeLine._expression);
        Expression::operatorReduction(codeLine._expression);
        if(codeLine._code.size() < 3) return true;
        _codeLines.push_back(codeLine);

        // REM modifies code
        size_t foundPos;
        if(findKeyword(codeLine._code, "REM", foundPos))
        {
            KeywordFuncResult result;
            handleREM(codeLine, int(_codeLines.size()), foundPos, result);
        }

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

    int getMacroSize(const std::string& macroStr);
    int getOpcodeSize(const std::string& opcodeStr)
    {
        // Recursively check macros
        if(opcodeStr.size() > 4)
        {
            return getMacroSize(opcodeStr);
        }

        std::string opcode = std::string(opcodeStr);
        if(opcode == ";"  ||  opcode == "gprintf") return 0;
        if(opcode == "LSLW"  ||  opcode == "PEEK"  ||  opcode == "DEEK"  ||  opcode == "RET"  ||  opcode == "PUSH"  ||  opcode == "POP") return 1;
        if(opcode == "LDWI"  ||  opcode == "BEQ"  ||  opcode == "BNE"  ||  opcode == "BLT"  ||  opcode == "BGT"  ||  opcode == "BLE"  ||  opcode == "BGE") return 3;
        return 2;
    }

    int getMacroSize(const std::string& macroStr)
    {
        static std::string filename = "gbas/include/macros.i";
        static std::ifstream infile(filename);
        static std::vector<std::string> lineTokens;

        // Read file
        if(lineTokens.size() == 0)
        {
            if(!infile.is_open())
            {
                fprintf(stderr, "Compiler::getMacroSize() : Failed to open file : '%s'\n", filename.c_str());
                return 0;
            }

            std::string lineToken;
            while(!infile.eof())
            {
                std::getline(infile, lineToken);
                lineTokens.push_back(lineToken);
            }
        }

        // Search for macro
        std::string macro = std::string(macroStr);
        for(int i=0; i<lineTokens.size(); i++)
        {
            if(lineTokens[i].find("%MACRO  " + macro) != std::string::npos)
            {
                int opcodesSize = 0;
                for(int j=i+1; j<lineTokens.size(); j++)
                {
                    if(lineTokens[j].find("%ENDM") != std::string::npos) return opcodesSize;
                    size_t opcodeStart = lineTokens[j].find_first_not_of("  \n\r\f\t\v");
                    size_t opcodeEnd = lineTokens[j].find_first_of("(  \n\r\f\t\v", opcodeStart);
                    if(opcodeStart == std::string::npos  ||  opcodeEnd == std::string::npos) continue;
                    opcodesSize += getOpcodeSize(lineTokens[j].substr(opcodeStart, opcodeEnd - opcodeStart));
                }
            }
        }

        return 0;
    }

    int createVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, std::string& line)
    {
        int vasmSize = 0;
        std::string opcode = std::string(opcodeStr);

        // Check for macro, remove % and get it's total opcode size
        if(opcode.size()  &&  opcode[0] == '%')
        {
            opcode.erase(0, 1);
            vasmSize = getMacroSize(opcode);
        }

        // Get opcode size
        if(vasmSize == 0) vasmSize = getOpcodeSize(opcode);
        _vasmPC += vasmSize;

        std::string operand = std::string(operandStr);
        std::string tabs = (opcode.size() > 3) ? "\t" : "\t\t";
        line = opcode + tabs + operand;

        return vasmSize;
    }

    int insertVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, int vasmLineIdx, int gotoLabelIndex=-1)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.insert(_codeLines[codeLineIdx]._vasm.begin() + vasmLineIdx, {uint16_t(_vasmPC - vasmSize), opcodeStr, line, "", gotoLabelIndex});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLineIndex, const std::string& label="", int gotoLabelIndex=-1)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, line, label, gotoLabelIndex});
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


    LabelResult checkForLabel(const std::string& code, int lineNumber)
    {
        Label label;
        CodeLine codeLine;

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

            createLabel(_vasmPC, code.substr(0, space), code.substr(0, space), int(_codeLines.size()), label);
            if(!createCodeLine(_vasmPC, code, "", int(space + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine)) return LabelError;

            return LabelFound;
        }

        // Text labels
        size_t colon1 = code.find_first_of(":");
        size_t colon2 = code.find_first_of(":", colon1+1);
        if(colon1 == std::string::npos)
        {
            if(!createCodeLine(_vasmPC, code, "", 0, _currentLabelIndex, -1, VarInt16, false, false, false, codeLine)) return LabelError;
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

        createLabel(_vasmPC, code.substr(0, colon1), code.substr(0, colon1), int(_codeLines.size()), label);
        if(!createCodeLine(_vasmPC, code, "", int(colon1 + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine)) return LabelError;

        return LabelFound;
    }

    bool parseLabels(int numLines)
    {
        // Entry point initialisation
        Label label;
        CodeLine codeLine;
        createLabel(_vasmPC, "_entryPoint_", "_entryPoint_\t", 0, label, false);
        if(!createCodeLine(_vasmPC, "INIT", "", 0, 0, -1, VarInt16, false, false, true, codeLine)) return false;
        emitVcpuAsm("%Initialise", "", false, 0);

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

    void adjustLabelAddresses(int labelIndex, int offset)
    {
        // Adjust label addresses for any labels with addresses higher than start label, (labels can be stored out of order)
        for(int i=0; i<_labels.size(); i++)
        {
            if(_labels[i]._address > _labels[labelIndex]._address)
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

    VarResult checkForVars(const std::string& code, int& varIndex, int lineNumber)
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
                    if(!Expression::parse((char*)data.c_str(), lineNumber, result)) return VarError;
                }
                break;

                case Expression::Invalid:
                {
                    fprintf(stderr, "Compiler::checkForVars() : invalid expression '%s' on line %d\n", data.c_str(), lineNumber);
                    return VarError;
                }
                break;
            }

            // TODO: only works with Int16, fix for all var types
            // Save result to var
            if(varIndex >= 0)
            {
                updateVar(result, lineNumber, varIndex, containsVars);
                return VarUpdated;
            }
            // Initialise var
            else
            {
                createVar(varName, result, lineNumber, containsVars, varIndex);
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

    bool varExpressionParse(CodeLine& codeLine, int lineNumber)
    {
        int16_t value;
        Expression::setExprFunc(expression);
        return Expression::parse((char*)codeLine._expression.c_str(), lineNumber, value);
    }

    int varAssignmentParse(CodeLine& codeLine, int lineNumber)
    {
        if(codeLine._expression.find_first_of("-+/*()") != std::string::npos) return -1;
        int varIndex = findVar(codeLine._expression);
        return varIndex;
    }


    KeywordResult handleKeywords(CodeLine& codeLine, size_t offset, int lineNumber, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search keywords
        for(int i=0; i<_keywords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _keywords[i]._name, foundPos))
            {
                bool error = _keywords[i]._func(codeLine, lineNumber, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }
        
        return KeywordNotFound;
    }

    KeywordResult handleMathwords(CodeLine& codeLine, size_t offset, int lineNumber, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search mathwords
        for(int i=0; i<_mathwords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _mathwords[i]._name, foundPos))
            {
                bool error = _mathwords[i]._func(codeLine, lineNumber, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }

    KeywordResult handleStringwords(CodeLine& codeLine, size_t offset, int lineNumber, KeywordFuncResult& result)
    {
        size_t foundPos;

        // Search stringwords
        for(int i=0; i<_stringwords.size(); i++)
        {
            if(findKeyword(codeLine._code.substr(offset), _stringwords[i]._name, foundPos))
            {
                bool error = _stringwords[i]._func(codeLine, lineNumber, foundPos, result);
                return (!error) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }

    bool handleREM(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
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

    bool handleLET(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleGOTO(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        std::string gotoLabel = codeLine._code.substr(4);
        int labelIndex = findLabel(gotoLabel);
        if(labelIndex == -1)
        {
            fprintf(stderr, "Compiler::createVasmCode() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        // Within same page
        if(HI_MASK(_vasmPC) == HI_MASK(_labels[labelIndex]._address))
        {
            emitVcpuAsm("BRA", "_" + gotoLabel, false, lineNumber, "", labelIndex);
        }
        // Long jump
        else
        {
#if 1
            emitVcpuAsm("LDWI", "_" + gotoLabel, false, lineNumber);
            emitVcpuAsm("CALL", "vAC", false, lineNumber);
#else            
            // This uses the call table and wastes precious zero page memory
            emitVcpuAsm("CALL", "_" + gotoLabel, false, lineNumber);
#endif
        }

        return true;
    }

    bool handlePRINT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        // Parse print tokens
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', false);
        for(int i=0; i<tokens.size(); i++)
        {
            Expression::ExpressionType expressionType = isExpression(tokens[i]);
            switch(expressionType)
            {
                case Expression::IsString:
                {
                    KeywordResult keywordResult = keywordResult = handleStringwords(codeLine, foundPos, lineNumber, result);
                    if(keywordResult == KeywordFound)
                    {
                        if(result._name == "CHR$")
                        {
                            emitVcpuAsm("%PrintAcChar", "", false, lineNumber);
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
                            fprintf(stderr, "Compiler::createVasmCode() : user string is %d characters too long in '%s' on line %d\n", int(str.size() - USER_STR_SIZE), codeLine._code.c_str(), lineNumber);
                            return false;
                        }

                        // Reuse string
                        bool foundString = false;
                        for(int j=0; j<_stringVars.size(); j++)
                        {
                            if(_stringVars[j]._data == str) 
                            {
                                emitVcpuAsm("%PrintString", _stringVars[j]._name, false, lineNumber);
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
                                fprintf(stderr, "Compiler::createVasmCode() : out of string memory, user string address %04x in '%s' on line %d\n", _userStrStart, codeLine._code.c_str(), lineNumber);
                                return false;
                            }
                        }

                        std::string usrStrName = "usrStr_" + Expression::wordToHexString(_userStrStart);
                        StringVar usrStrVar = {uint8_t(str.size()), _userStrStart, str, usrStrName, usrStrName + "\t\t", -1};
                        _stringVars.push_back(usrStrVar);
                        _userStrStart += uint16_t(str.size() + 1);
                        emitVcpuAsm("%PrintString", _stringVars[_stringVars.size() - 1]._name, false, lineNumber);
                    }
                }
                break;

                case Expression::None:
                case Expression::Valid:
                {
                    int16_t result;
                    Expression::setExprFunc(Expression::expression);
                    if(!Expression::parse((char*)tokens[i].c_str(), lineNumber, result)) return false;
                    emitVcpuAsm("%PrintInt16", Expression::wordToHexString(result), false, lineNumber);
                }
                break;

                case Expression::HasAlpha:
                {
                    // Search math words and string words
                    KeywordResult keywordResult = handleMathwords(codeLine, foundPos, lineNumber, result);
                    if(keywordResult == KeywordNotFound) keywordResult = handleStringwords(codeLine, foundPos, lineNumber, result);
                    if(keywordResult == KeywordNotFound)
                    {
                        CodeLine cl = codeLine;
                        cl._code = cl._expression = tokens[i];
                        if(!varExpressionParse(cl, lineNumber)) return false;
                        int varIndex = varAssignmentParse(cl, lineNumber);
                        if(varIndex >= 0)
                        {
                            emitVcpuAsm("%PrintVarInt16", "_" + _integerVars[varIndex]._name, false, lineNumber);
                        }
                        else
                        {
                            emitVcpuAsm("%PrintVarInt16", Expression::byteToHexString(uint8_t(_tempVarStart)), false, lineNumber);
                        }
                    }
                }
                break;

                default:
                {
                    fprintf(stderr, "Compiler::handlePRINT() : invalid input in '%s' on line %d\n", tokens[i].c_str(), lineNumber);
                    return false;
                }
                break;
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            emitVcpuAsm("CALL", "newLineScroll", false, lineNumber);
        }

        return true;
    }

    bool handleFOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() < 6  ||  codeLine._tokens.size() > 8)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }
        if(codeLine._tokens[2] != "=")
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (missing '='), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }
        if(Expression::strToUpper(codeLine._tokens[4]) != "TO")
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (missing 'TO'), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
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
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (bad FOR start), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        // Var counter, (create or update if being reused)
        int varIndex = findVar(codeLine._tokens[1]);
        (varIndex < 0) ? createVar(codeLine._tokens[1], loopStart, lineNumber, false, varIndex) : updateVar(loopStart, lineNumber, varIndex, false);

        // Loop end
        int16_t loopEnd;
        if(!Expression::stringToI16(codeLine._tokens[5], loopEnd))
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (bad FOR end), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

#if 0
        // Maximum of 4 nested loops
        if(_forNextDataStack.size() == 4)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        int offset = int(_forNextDataStack.size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        emitVcpuAsm("%ForNextInitVsVe", "_" + _integerVars[varIndex]._name + " " + std::to_string(loopStart) + " " + std::to_string(loopEnd) + " 1" + " " + Expression::wordToHexString(varEnd) + " " + Expression::wordToHexString(varStep), false, lineNumber);
#endif

        emitVcpuAsm("LDWI", std::to_string(loopStart), false, lineNumber);

        // Create FOR loop label, (label is attached to line after for loop initialisation)
        Label label;
        int lineAfterLoopInit = _currentCodeLineIndex + 1;
        std::string name = "next" + std::to_string(_currentCodeLineIndex); // + "_" + _integerVars[varIndex]._name;
        createLabel(_vasmPC, name, name + "\t", lineAfterLoopInit, label, false);
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

    bool handleNEXT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Compiler::handleNEXT() : syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Compiler::handleNEXT() : syntax error, (bad var), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        ForNextData forNextData = _forNextDataStack.top();
        _forNextDataStack.pop();
        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (wrong var), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        emitVcpuAsm("%ForNextLoopP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + std::to_string(forNextData._loopEnd), false, lineNumber, "", forNextData._labelIndex);

#if 0
        emitVcpuAsm("%ForNextLoopVsVeP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + Expression::wordToHexString(forNextData._varEnd) + " " + Expression::wordToHexString(forNextData._varStep), false, lineNumber, "", forNextData._labelIndex);
#endif

        return true;
    }

    bool handleIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTHEN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleELSE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleELSEIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleENDIF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDIM(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDEF(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleINPUT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleREAD(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDATA(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handlePEEK(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handlePOKE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDEEK(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDOKE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleON(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleGOSUB(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }
    bool handleRETURN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleDO(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLOOP(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleWHILE(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleUNTIL(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleEXIT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleAND(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleXOR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleNOT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleABS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleACS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleASC(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleASN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleATN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleCOS(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleEXP(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleINT(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLOG(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleRND(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSIN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSQR(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTAN(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleCHR$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
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
                    if(!Expression::parse((char*)expr.c_str(), lineNumber, result._data)) return false;
                    emitVcpuAsm("LDI", std::to_string(result._data), false, lineNumber);
                }
                break;

                case Expression::HasAlpha:
                {
                    CodeLine cl = codeLine;
                    cl._code = cl._expression = expr;
                    if(!varExpressionParse(cl, lineNumber)) return false;
                    int varIndex = varAssignmentParse(cl, lineNumber);
                    (varIndex >= 0) ? emitVcpuAsm("LD", "_" + _integerVars[varIndex]._name, false, lineNumber) : emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(_tempVarStart)), false, lineNumber);
                }
                break;

                default:
                {
                    fprintf(stderr, "Compiler::handleCHR$() : invalid input in '%s' on line %d\n", expr.c_str(), lineNumber);
                    return false;
                }
                break;
            }
        }
        else
        {
            fprintf(stderr, "Compiler::handleCHR$() : expecting () in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        result._name = "CHR$";

        return true;
    }

    bool handleMID$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleLEFT$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleRIGHT$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSPC$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleSTR$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool handleTIME$(CodeLine& codeLine, int lineNumber, size_t foundPos, KeywordFuncResult& result)
    {
        return true;
    }

    bool createVasmCode(CodeLine& codeLine, int lineNumber)
    {
#if 0
        // Temporary var expression, i.e. within ()
        if(codeLine._containsVars)
        {
            if(!varExpressionParse(codeLine, lineNumber)) return false;
            emitVcpuAsm("STW", Expression::wordToHexString(TEMP_VAR_START), false, lineNumber);
        }
#endif

        KeywordFuncResult result;
        KeywordResult keywordResult = handleKeywords(codeLine, 0, lineNumber, result);
        if(keywordResult == KeywordNotFound) keywordResult = handleMathwords(codeLine, 0, lineNumber, result);
        if(keywordResult == KeywordNotFound) keywordResult = handleStringwords(codeLine, 0, lineNumber, result);
        else if(keywordResult == KeywordError) return false;

        // Check for matching brackets
        if(std::count(codeLine._expression.begin(), codeLine._expression.end(), '(') != std::count(codeLine._expression.begin(), codeLine._expression.end(), ')'))
        {
            fprintf(stderr, "Compiler::createVasmCode() : Brackets are not matched in '%s' on line %d\n", codeLine._expression.c_str(), lineNumber);
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
                if(!varExpressionParse(codeLine, lineNumber)) return false;
                int varIndex = varAssignmentParse(codeLine, lineNumber);

                // Optimise LDW away if possible
                if(varIndex >= 0  &&  varIndex != prevVarIndex  &&  keywordResult != KeywordFound)
                {
                    emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, lineNumber);
                }
                emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, lineNumber);
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
                        emitVcpuAsm("LDI", std::to_string(_integerVars[codeLine._varIndex]._init), false, lineNumber);
                    }
                    // 16bit constants
                    else
                    {
                        emitVcpuAsm("LDWI", std::to_string(_integerVars[codeLine._varIndex]._init), false, lineNumber);
                    }
                }
                emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, lineNumber);
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
                                                    adjustLabelAddresses(_codeLines[i]._labelIndex, -4);
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
                                                adjustLabelAddresses(_codeLines[i]._labelIndex, -2);
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
                                    adjustLabelAddresses(_codeLines[i]._labelIndex, -2);
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
                uint16_t vasmPC = (hPC + 1) <<8;
                for(auto itVasm=itCode->_vasm.begin(); itVasm!=itCode->_vasm.end();)
                {
                    resetCheck = false;

                    uint8_t lPC = LO_BYTE(itVasm->_address);
                    if((lPC >= 0xF3  &&  (hPC == 0x02 || hPC == 0x03 || hPC == 0x04))  ||  lPC >= 0xF9)
                    {
                        // Copy old vasm code
                        int vasmSize = itCode->_vasmSize;
                        std::vector<VasmLine> vasm = itCode->_vasm;

                        // Insert long jump
                        _vasmPC = itCode->_vasm[0]._address;
                        itCode->_vasm.clear(); // itVasm invalidated
                        insertVcpuAsm("LDWI", Expression::wordToHexString(vasmPC), codeLineIndex, 0);
                        //insertVcpuAsm("STW", "register0", codeLineIndex, 1);
                        //insertVcpuAsm("CALL", "register0", codeLineIndex, 2);
                        //itCode->_vasmSize = 7;
                        insertVcpuAsm("CALL", "vAC", codeLineIndex, 1);
                        itCode->_vasmSize = 5;

                        // Create long jump label and new code
                        Label label;
                        std::string name = Expression::wordToHexString(vasmPC);
                        createLabel(vasmPC, name, name + "\t", codeLineIndex + 1, label, false, true);
                        CodeLine codeLine = {itCode->_code, itCode->_tokens, vasm, "", vasmSize, _currentLabelIndex, -1, VarInt16, false, false, true};
                        itCode = _codeLines.insert(itCode + 1, codeLine);

                        // Fix labels and addresses
                        int offset = vasmPC - itCode->_vasm[0]._address;
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
                int gotoLabelIndex = _codeLines[i]._vasm[j].gotoLabelIndex;
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

        line = "_callTable_ ";
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        line += "EQU\t\t" + Expression::wordToHexString(CALL_TABLE_START) + "\n\n";
        _output.push_back(line);
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
        _output.push_back("printText       EQU     clearRegion - 0x0100\n");
        _output.push_back("printDigit      EQU     clearRegion - 0x0200\n");
        _output.push_back("printChar       EQU     clearRegion - 0x0300\n");
        _output.push_back("newLineScroll   EQU     clearRegion - 0x0400\n");
        _output.push_back("resetAudio      EQU     clearRegion - 0x0500\n");
        _output.push_back("playMidi        EQU     clearRegion - 0x0600\n");
        _output.push_back("midiStartNote   EQU     clearRegion - 0x0700\n");
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
        _output.push_back("textColour      EQU     register0 + 0x10\n");
        _output.push_back("cursorXY        EQU     register0 + 0x12\n");
        _output.push_back("midiStreamPtr   EQU     register0 + 0x14\n");
        _output.push_back("midiDelay       EQU     register0 + 0x16\n");
        _output.push_back("frameCountPrev  EQU     register0 + 0x18\n");
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
            std::string basicCode = _codeLines[i]._code;
            if(basicCode.size() > 2  &&  _codeLines[i]._vasm.size())
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

                // Commented BASIC code
                line +=  "\t\t; " + basicCode + "\n";

                _output.push_back(line);
            }
        }
        
        _output.push_back("\n");
    }


    void clearCompiler(void)
    {
        _vasmPC         = USER_CODE_START;
        _tempVarStart   = TEMP_VAR_START;
        _userVarStart   = USER_VAR_START;
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
        Memory::getRam(Memory::FitLargest, Memory::RamVasm, 96, address);

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