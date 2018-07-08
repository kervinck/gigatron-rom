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
    enum OptimiseTypes {StwLdwPair=0, ExtraStw, NumOptimiseTypes};

    struct Label
    {
        uint16_t _address;
        std::string _name;
        std::string _output;
        int _codeLineIndex = -1;
    };

    struct VasmLine
    {
        std::string _code;
        std::string _label;
    };

    struct CodeLine
    {
        uint16_t _address;
        std::string _code;
        std::vector<std::string> _tokens;
        std::vector<VasmLine> _vasm;
        std::string _expression;
        int _labelIndex = -1;
        int  _varIndex = -1;
        VarType _varType = VarInt16;
        bool _assignOperator = false;
        bool _containsVars = false;
        bool _ownsLabel = false;
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

    using KeywordFuncPtr = std::function<bool (CodeLine& codeLine, int lineNumber, size_t foundPos)>;
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
        uint16_t _varEnd;
        uint16_t _varStep;
    };

    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart   = USER_VAR_START;
    uint16_t _userStrStart   = USER_STR_START;
    uint16_t _userStackStart = USER_STACK_START;

    bool _nextTempVar = true;

    int _currentLabelIndex = 0;
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


    bool handleREM(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleLET(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleGOTO(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handlePRINT(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleFOR(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleNEXT(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleIF(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleTHEN(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleELSE(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleELSEIF(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleENDIF(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDIM(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDEF(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleINPUT(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleREAD(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDATA(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handlePEEK(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handlePOKE(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDEEK(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDOKE(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleON(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleGOSUB(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleRETURN(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleDO(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleLOOP(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleWHILE(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleUNTIL(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleEXIT(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleAND(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleOR(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleXOR(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleNOT(CodeLine& codeLine, int lineNumber, size_t foundPos);

    bool handleABS(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleACS(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleASC(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleASN(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleATN(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleCOS(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleEXP(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleINT(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleLOG(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleRND(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleSIN(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleSQR(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleTAN(CodeLine& codeLine, int lineNumber, size_t foundPos);

    bool handleCHR$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleMID$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleLEFT$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleRIGHT$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleSPC$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleSTR$(CodeLine& codeLine, int lineNumber, size_t foundPos);
    bool handleTIME$(CodeLine& codeLine, int lineNumber, size_t foundPos);

    
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

            if(!infile.good()  &&  !infile.eof())
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


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label)
    {
        label = {address, name, output, codeLineIndex};
        Expression::stripWhitespace(label._name);
        _currentLabelIndex = int(_labels.size());
        _labels.push_back(label);
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

    void createCodeLine(uint16_t address, const std::string& code, const std::string& vasmCode, int codeLineOffset, int labelIndex, int varIndex, VarType varType, bool assign, bool vars, bool ownsLabel, CodeLine& codeLine)
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
            size_t rbra = code.find_first_of(")", lbra + 1);
            if(lbra != std::string::npos  &&  rbra != std::string::npos)
            {
                expression = code.substr(lbra + 1, rbra - (lbra + 1));
            }
        }

        std::vector<VasmLine> vasm;
        if(vasmCode.size()) vasm.push_back({vasmCode, ""});
        std::string codeSub = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        std::vector<std::string> tokens = Expression::tokenise(codeSub, ' ', true);
        codeLine = {address, codeSub, tokens, vasm, expression, labelIndex, varIndex, varType, assign, vars, ownsLabel};
        Expression::stripNonStringWhitespace(codeLine._code);
        Expression::stripWhitespace(codeLine._expression);
        Expression::operatorReduction(codeLine._expression);
        if(codeLine._code.size() < 3) return;
        _codeLines.push_back(codeLine);
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

    int getOpcodeSize(const std::string& opcodeStr)
    {
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
            if(lineTokens[i].find(macro) != std::string::npos)
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

    int insertVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, int vasmLineIdx)
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.insert(_codeLines[codeLineIdx]._vasm.begin() + vasmLineIdx, {line, ""});

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLineIndex, const std::string& label="")
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.push_back({line, label});

        if(nextTempVar) getNextTempVar();
    }

    bool emitVcpuAsmUserVar(const std::string& opcodeStr, const char* varNamePtr, bool nextTempVar)
    {
        std::string opcode = std::string(opcodeStr);
        std::string varName = std::string(varNamePtr);
        int varIndex = findVar(varName);
        if(varIndex == -1)
        {
            fprintf(stderr, "Compiler::handleAddSub() : couldn't find variable name '%s'\n", varName.c_str());
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
            createCodeLine(_vasmPC, code, "", int(space + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine);

            return LabelFound;
        }

        // Text labels
        size_t colon1 = code.find_first_of(":");
        size_t colon2 = code.find_first_of(":", colon1+1);
        if(colon1 == std::string::npos)
        {
            createCodeLine(_vasmPC, code, "", 0, _currentLabelIndex, -1, VarInt16, false, false, false, codeLine);
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
        createCodeLine(_vasmPC, code, "", int(colon1 + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine);

        return LabelFound;
    }

    bool parseLabels(int numLines)
    {
        // Entry point initialisation
        Label label;
        CodeLine codeLine;
        createLabel(_vasmPC, "_entryPoint_", "_entryPoint_\t", 0, label);
        createCodeLine(_vasmPC, "INIT", "", 0, 0, -1, VarInt16, false, false, true, codeLine);
        emitVcpuAsm("%Initialise", "", false, 0);

        std::string line;
        for(int i=0; i<numLines; i++)
        {
            line = "";
            switch(checkForLabel(_input[i], i))
            {
                case LabelFound:
                {
                    line = "_" + _labels.back()._name;
                    Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
                    size_t space = line.find_first_of(" ");
                    if(space == std::string::npos  ||  space >= LABEL_TRUNC_SIZE - 1)
                    {
                        line = line.substr(0, LABEL_TRUNC_SIZE);
                        line[LABEL_TRUNC_SIZE - 1] = ' ';
                    }

                    _labels.back()._output = line;
                }
                break;

                case LabelNotFound:
                {
                }
                break;

                case LabelError: return false;
            }
        }

        return true;
    }

    void adjustLabelAddresses(int labelStart, int offset)
    {
        // Adjust label addresses
        for(int j=_codeLines[labelStart]._labelIndex + 1; j<_labels.size(); j++)
        {
            // Don't adjust for any code that starts on a page boundary
            if((_labels[j]._address & 0x00FF) != 0x00) _labels[j]._address += offset;
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
                case Expression::HasAlpha: result = 0; containsVars = true;                             break;
                case Expression::None:     Expression::stringToI16(data, result);                       break;
                case Expression::Valid:    result = Expression::parse((char*)data.c_str(), lineNumber); break;

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

        handleAddSub("ADD", left, right);
        return left;
    }

    Expression::Numeric sub(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value -= right._value;
            return left;
        }

        handleAddSub("SUB", left, right);
        return left;
    }

    Expression::Numeric mul(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value *= right._value;
            return left;
        }

        return left;
    }

    Expression::Numeric div(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value /= right._value;
            return left;
        }

        return left;
    }

    Expression::Numeric expression(void);
    Expression::Numeric fac(int16_t defaultValue)
    {
        int16_t value = 0;
        if(Expression::peek() == '(')
        {
            Expression::get();
            Expression::Numeric numeric = expression();
            Expression::get();
            return numeric;
        }
        else if(Expression::peek() == '-')
        {
            Expression::get();
            return neg(fac(0));
        }
        else if((Expression::peek() >= '0'  &&  Expression::peek() <= '9')  ||  Expression::peek() == '$')
        {
            if(!Expression::number(value)) value = 0;
            return Expression::Numeric(value, false, nullptr);
        }

        Expression::Numeric numeric = Expression::Numeric(defaultValue, true, Expression::getExpression());
        while(isalpha(Expression::peek())) Expression::get();
        return numeric;
    }

    Expression::Numeric term(void)
    {
        Expression::Numeric result = fac(0);
        while(Expression::peek() == '*'  ||  Expression::peek() == '/')
        {
            if(Expression::get() == '*')
            {
                result = mul(result, fac(0));
            }
            else
            {
                Expression::Numeric f = fac(0);
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
        Expression::Numeric result = term();

        while(Expression::peek() == '+' || Expression::peek() == '-')
        {
            if(Expression::get() == '+')
            {
                result = add(result, term());
            }
            else
            {
                result = sub(result, term());
            }
        }

        return result;
    }

    void varExpressionParse(CodeLine& codeLine, int lineNumber)
    {
        Expression::setExprFunc(expression);
        Expression::parse((char*)codeLine._expression.c_str(), lineNumber);
    }

    int varAssignmentParse(CodeLine& codeLine, int lineNumber)
    {
        if(codeLine._expression.find_first_of("-+/*()") != std::string::npos) return -1;
        int varIndex = findVar(codeLine._expression);
        return varIndex;
    }


    bool handleREM(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        codeLine._code.clear();
        return true;
    }

    bool handleLET(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleGOTO(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        std::string gotoLabel = codeLine._code.substr(4);
        if(findLabel(gotoLabel) == -1)
        {
            fprintf(stderr, "Compiler::createVasmCode() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        emitVcpuAsm("CALL", "_" + gotoLabel, false, lineNumber);

        return true;
    }

    bool handlePRINT(CodeLine& codeLine, int lineNumber, size_t foundPos)
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
                        if((_userStrStart + str.size() & 0xFF00) != (_userStrStart & 0xFF00))
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

                case Expression::Valid:
                {
                    Expression::setExprFunc(Expression::expression);
                    uint16_t result = Expression::parse((char*)tokens[i].c_str(), lineNumber);
                    emitVcpuAsm("%PrintInt16", Expression::wordToHexString(result), false, lineNumber);
                }
                break;

                case Expression::HasAlpha:
                {
                    CodeLine cl = codeLine;
                    cl._code = cl._expression = tokens[i];
                    varExpressionParse(cl, lineNumber);
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
        if(codeLine._code[codeLine._code.size() - 1] != ';') emitVcpuAsm("CALL", "newLineScroll", false, lineNumber);

        return true;
    }

    bool handleFOR(CodeLine& codeLine, int lineNumber, size_t foundPos)
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

        // Var counter
        int varIndex = findVar(codeLine._tokens[1]);
        if(varIndex < 0) createVar(codeLine._tokens[1], loopStart, lineNumber, false, varIndex);

        // Loop end
        int16_t loopEnd;
        if(!Expression::stringToI16(codeLine._tokens[5], loopEnd))
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (bad FOR end), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        // Maximum of 4 nested loops
        if(_forNextDataStack.size() == 4)
        {
            fprintf(stderr, "Compiler::handleFOR() : syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._code.c_str(), lineNumber);
            return false;
        }

        int offset = int(_forNextDataStack.size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        emitVcpuAsm("%ForNextInit", "_" + _integerVars[varIndex]._name + " " + std::to_string(loopStart) + " " + std::to_string(loopEnd) + " 1" + " " + Expression::wordToHexString(varEnd) + " " + Expression::wordToHexString(varStep), false, lineNumber);

        Label label;
        std::string name = "forNext_" + Expression::wordToHexString(_currentCodeLineIndex);
        _codeLines[_currentCodeLineIndex + 1]._ownsLabel = true;
        _codeLines[_currentCodeLineIndex + 1]._labelIndex = int(_labels.size());
        createLabel(_vasmPC, name, name + "\t", _currentCodeLineIndex + 1, label);

        _forNextDataStack.push({varIndex, _codeLines[_currentCodeLineIndex + 1]._labelIndex, _currentCodeLineIndex + 1, varEnd, varStep});

#if 0

        

            Expression::ExpressionType expressiontype = isExpression(data);
            switch(expressiontype)
            {
                case Expression::HasAlpha: result = 0; containsVars = true;                             break;
                case Expression::None:     Expression::stringToI16(data, result);                       break;
                case Expression::Valid:    result = Expression::parse((char*)data.c_str(), lineNumber); break;

                case Expression::Invalid:
                {
                    fprintf(stderr, "Compiler::checkForVars() : invalid expression '%s' on line %d\n", data.c_str(), lineNumber);
                    return VarError;
                }
                break;
            }



        // Parse for loop
        for(int i=0; i<codeLine._tokens.size(); i++)
        {
        }
#endif

        return true;
    }

    bool handleNEXT(CodeLine& codeLine, int lineNumber, size_t foundPos)
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

        emitVcpuAsm("%ForNextStepP", "_" + _integerVars[varIndex]._name + " " + _labels[forNextData._labelIndex]._name + " " + Expression::wordToHexString(forNextData._varEnd) + " " + Expression::wordToHexString(forNextData._varStep), false, lineNumber);

        return true;
    }

    bool handleIF(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleTHEN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleELSE(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleELSEIF(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleENDIF(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDIM(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDEF(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleINPUT(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleREAD(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDATA(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handlePEEK(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handlePOKE(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDEEK(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDOKE(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleON(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleGOSUB(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }
    bool handleRETURN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleDO(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleLOOP(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleWHILE(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleUNTIL(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleEXIT(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleAND(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleOR(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleXOR(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleNOT(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleABS(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleACS(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleASC(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleASN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleATN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleCOS(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleEXP(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleINT(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleLOG(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleRND(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleSIN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleSQR(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleTAN(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleCHR$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        // Parse chr$
        size_t lbra = codeLine._code.find_first_of("(", foundPos);
        size_t rbra = codeLine._code.find_first_of(")", lbra + 1);
        if(lbra != std::string::npos  &&  rbra != std::string::npos)
        {
            std::string expr = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            Expression::ExpressionType expressionType = isExpression(expr);
            switch(expressionType)
            {
                case Expression::Valid:
                {
                    Expression::setExprFunc(Expression::expression);
                    uint16_t result = Expression::parse((char*)expr.c_str(), lineNumber);
                    emitVcpuAsm("%PrintChar", Expression::wordToHexString(result), false, lineNumber);
                }
                break;

                case Expression::HasAlpha:
                {
                    CodeLine cl = codeLine;
                    cl._code = cl._expression = expr;
                    varExpressionParse(cl, lineNumber);
                    int varIndex = varAssignmentParse(cl, lineNumber);
                    if(varIndex >= 0)
                    {
                        emitVcpuAsm("%PrintVarChar", "_" + _integerVars[varIndex]._name, false, lineNumber);
                    }
                    else
                    {
                        emitVcpuAsm("%PrintVarChar", Expression::byteToHexString(uint8_t(_tempVarStart)), false, lineNumber);
                    }
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

        return true;
    }

    bool handleMID$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleLEFT$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleRIGHT$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleSPC$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleSTR$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    bool handleTIME$(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        return true;
    }

    KeywordResult handleKeywords(CodeLine& codeLine, int lineNumber)
    {
        size_t foundPos;

        // Search keywords
        for(int i=0; i<_keywords.size(); i++)
        {
            if(findKeyword(codeLine._code, _keywords[i]._name, foundPos))
            {
                bool result = _keywords[i]._func(codeLine, lineNumber, foundPos);
                return (!result) ? KeywordError : KeywordFound;
            }
        }
        
        // Search mathwords
        for(int i=0; i<_mathwords.size(); i++)
        {
            if(findKeyword(codeLine._code, _mathwords[i]._name, foundPos))
            {
                bool result = _mathwords[i]._func(codeLine, lineNumber, foundPos);
                return (!result) ? KeywordError : KeywordFound;
            }
        }

        // Search stringwords
        for(int i=0; i<_stringwords.size(); i++)
        {
            if(findKeyword(codeLine._code, _stringwords[i]._name, foundPos))
            {
                bool result = _stringwords[i]._func(codeLine, lineNumber, foundPos);
                return (!result) ? KeywordError : KeywordFound;
            }
        }

        return KeywordNotFound;
    }

    bool createVasmCode(CodeLine& codeLine, int lineNumber)
    {
#if 0
        // Temporary var expression, i.e. within ()
        if(codeLine._containsVars)
        {
            varExpressionParse(codeLine, lineNumber);
            emitVcpuAsm("STW", Expression::wordToHexString(TEMP_VAR_START), false, lineNumber);
        }
#endif

        KeywordResult keywordResult = handleKeywords(codeLine, lineNumber);
        if(keywordResult == KeywordNotFound)
        {
            // TODO: only works with Int16, fix for all var types
            // Variable assignment
            if(codeLine._assignOperator)
            {
                // Optimisation, (removes uneeded LDW's)
                static int prevVarIndex = -1;

                // Assignment with a var expression
                if(codeLine._containsVars)
                {
                    varExpressionParse(codeLine, lineNumber);
                    int varIndex = varAssignmentParse(codeLine, lineNumber);

                    // Optimise LDW away if possible
                    if(varIndex >= 0  &&  varIndex != prevVarIndex)
                    {
                        emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, lineNumber);
                    }
                    emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, lineNumber);
                    prevVarIndex = codeLine._varIndex;
                }
                // Standard assignment
                else
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

                    emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name, false, lineNumber);
                    prevVarIndex = codeLine._varIndex;
                }
            }
        }
        else if(keywordResult == KeywordError) 
        {
            return false;
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
                uint16_t oldVasmPC = _vasmPC;
                if(!createVasmCode(_codeLines[i], i)) return false;

#if 0
                // TODO: move this to where it will work!
                // Exclusion zones and page boundary crossings
                if((_vasmPC & 0x00FF) >= 0xF1  ||  (oldVasmPC & 0xFF00) != (_vasmPC & 0xFF00))
                {
                    _codeLines[i]._vasm.clear();
                    _vasmPC = (oldVasmPC & 0xFF00) + 0x0100;
                    if(_codeLines[i]._ownsLabel) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;
                    emitVcpuAsm("CALL",  _labels[_codeLines[i]._labelIndex]._output, false, i-1);
                    if(!createVasmCode(_codeLines[i], i)) return false;
                }
#endif
            }
        }

        return true;
    }

    bool optimiseCode(void)
    {
        const std::string firstMatch[NumOptimiseTypes] = {"STW\t\t0x", "STW\t\t0x"};
        const std::string secondMatch[NumOptimiseTypes] = {"LDW\t\t0x", "STW\t\t_"};

        for(int k=StwLdwPair; k<NumOptimiseTypes; k++)
        {
            for(int i=0; i<_codeLines.size(); i++)
            {
                int firstLine = 0;
                std::string firstVar;
                bool firstFound = false;

                for(auto itLine=_codeLines[i]._vasm.begin(); itLine!=_codeLines[i]._vasm.end();)
                {
                    bool linesDeleted = false;

                    // First match
                    if(!firstFound)
                    {
                        size_t first = itLine->_code.find(firstMatch[k]);
                        if(first != std::string::npos)
                        {
                            firstFound = true;
                            firstLine = int(itLine - _codeLines[i]._vasm.begin());
                            firstVar = itLine->_code.substr(first + firstMatch[k].size(), 2);
                        }
                    }
                    else
                    {
                        // Second match must be on next line
                        if(int(itLine - _codeLines[i]._vasm.begin())  ==  firstLine + 1)
                        {
                            size_t second = itLine->_code.find(secondMatch[k]);
                            if(second != std::string::npos)
                            {
                                switch(k)
                                {
                                    // Remove superfluous STW/LDW pairs
                                    case StwLdwPair:
                                    {
                                        // If operand of STW/LDW pair matches
                                        if(firstVar == itLine->_code.substr(second + secondMatch[k].size(), 2))
                                        {
                                            linesDeleted = true;
                                            itLine = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine + 1);
                                            itLine = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                            adjustLabelAddresses(i, -4);
                                        }
                                    }
                                    break;

                                    // Remove superfluous STW
                                    case ExtraStw:
                                    {
                                        linesDeleted = true;
                                        itLine = _codeLines[i]._vasm.erase(_codeLines[i]._vasm.begin() + firstLine);
                                        adjustLabelAddresses(i, -2);
                                    }
                                    break;
                                }
                            }
                        }

                        firstLine = 0;
                        firstVar = "";
                        firstFound = false;
                    }

                    if(!linesDeleted) itLine++;
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
        _output.push_back("clearScreen     EQU     " + Expression::wordToHexString(INT_FUNC_START) + "\n");
        _output.push_back("printText       EQU     clearScreen - 0x0100\n");
        _output.push_back("printDigit      EQU     clearScreen - 0x0200\n");
        _output.push_back("printChar       EQU     clearScreen - 0x0300\n");
        _output.push_back("newLineScroll   EQU     clearScreen - 0x0400\n");
        _output.push_back("resetAudio      EQU     clearScreen - 0x0500\n");
        _output.push_back("playMidi        EQU     clearScreen - 0x0600\n");
        _output.push_back("midiStartNote   EQU     clearScreen - 0x0700\n");
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
    }

    bool compile(const std::string& inputFilename, const std::string& outputFilename)
    {
        clearCompiler();

        // Read .gbas file
        int numLines = 0;
        std::ifstream infile(inputFilename);
        if(!readInputFile(infile, inputFilename, numLines)) return false;

        fprintf(stderr, "\nCompiling file '%s'\n", inputFilename.c_str());

        // Labels
        if(!parseLabels(numLines)) return false;

        // Vars
        if(!parseVars()) return false;

        // Parse code
        if(!parseCode()) return false;

        // Optimise code
        if(!optimiseCode()) return false;

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