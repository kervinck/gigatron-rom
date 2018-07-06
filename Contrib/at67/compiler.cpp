#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>

#include "expression.h"
#include "compiler.h"


#define LABEL_TRUNC_SIZE 16     // The smaller you make this, the more your BASIC label names will be truncated in the resultant .vasm code
#define USER_STR_SIZE    95

#define TEMP_VAR_START   0x0030
#define USER_VAR_START   0x0040
#define USER_CODE_START  0x0200
#define USER_STR_START   0x6FA0
#define CALL_TABLE_START 0x00EE


namespace Compiler
{
    enum VarType {VarInt8=0, VarInt16, VarInt32, VarFloat16, VarFloat32};
    enum VarResult {VarError=-1, VarNotFound, VarInitialised, VarUpdated};
    enum IntSize {Int8=1, Int16=2, Int32=4};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};

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


    std::vector<std::string> _keywords;
    std::vector<std::string> _operators;
    std::vector<std::string> _relational;
    std::vector<std::string> _delimiters;

    std::vector<std::string> _input;
    std::vector<std::string> _output;

    std::vector<Label>      _labels;
    std::vector<CodeLine>   _codeLines;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar>  _stringVars;
    std::vector<ArrayVar>   _arrayVars;

    std::map<std::string, bool> _keywordsUsed;    

    uint16_t _tempVarStart     = TEMP_VAR_START;
    uint16_t _integerVarsStart = USER_VAR_START;
    uint16_t _vasmPC           = USER_CODE_START;
    uint16_t _stringVarsStart  = USER_STR_START;

    bool _nextTempVar = true;

    int _currentCodeLine = 0;
    int _currentLabelIndex = 0;

    std::string _tempVarStartStr;

    
    void initialise(void)
    {
        _keywords.push_back("LET");
        _keywords.push_back("REM");  
        _keywords.push_back("DIM");  
        _keywords.push_back("DEF");  
        _keywords.push_back("REM");  
        _keywords.push_back("PRINT");
        _keywords.push_back("INPUT");
        _keywords.push_back("READ"); 
        _keywords.push_back("DATA"); 
        _keywords.push_back("GOTO"); 
        _keywords.push_back("PEEK"); 
        _keywords.push_back("POKE"); 
        _keywords.push_back("ON"); 
        _keywords.push_back("GOSUB");
        _keywords.push_back("RETURN");
        _keywords.push_back("FOR");   
        _keywords.push_back("NEXT");  
        _keywords.push_back("STEP");  
        _keywords.push_back("IF");    
        _keywords.push_back("THEN");  
        _keywords.push_back("ELSE");
        _keywords.push_back("ELSEIF");
        _keywords.push_back("ENDIF");
        _keywords.push_back("DO");
        _keywords.push_back("LOOP");
        _keywords.push_back("WHILE");
        _keywords.push_back("UNTIL");
        _keywords.push_back("EXIT");
        _keywords.push_back("AND");
        _keywords.push_back("OR");
        _keywords.push_back("XOR");
        _keywords.push_back("NOT");

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


    void createlabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label)
    {
        label = {address, name, output, codeLineIndex};
        Expression::stripWhitespace(label._name);
        _currentLabelIndex = int(_labels.size());
        _labels.push_back(label);
    }

    void createCodeLine(uint16_t address, const std::string& lineText, const std::string& vasmCode, int codeLineOffset, int labelIndex, int varIndex, VarType varType, bool assign, bool vars, bool ownsLabel, CodeLine& codeLine)
    {
        std::string expression = "";
        size_t equal = lineText.find_first_of("=");
        if(equal != std::string::npos)
        {
            expression = lineText.substr(equal + 1);
        }
        else
        {
            size_t lbra = lineText.find_first_of("(");
            size_t rbra = lineText.find_first_of(")", lbra + 1);
            if(lbra != std::string::npos  &&  rbra != std::string::npos)
            {
                expression = lineText.substr(lbra + 1, rbra - lbra + 1);
            }
        }

        std::vector<VasmLine> vasm;
        if(vasmCode.size()) vasm.push_back(VasmLine({vasmCode, ""}));
        codeLine = {address, lineText.substr(codeLineOffset, lineText.size() - (codeLineOffset)), vasm, expression, labelIndex, varIndex, varType, assign, vars, ownsLabel};
        Expression::stripNonStringWhitespace(codeLine._code);
        Expression::stripWhitespace(codeLine._expression);
        Expression::operatorReduction(codeLine._code);
        Expression::operatorReduction(codeLine._expression);
        _codeLines.push_back(codeLine);
    }

    LabelResult checkForLabel(const std::string& lineText, int lineNumber)
    {
        Label label;
        CodeLine codeLine;

        // Numeric labels
        if(lineText.size() > 1  &&  isdigit(lineText[0]))
        {
            size_t space = lineText.find_first_of(" \n\r\f\t\v");
            if(space == std::string::npos)
            {
                fprintf(stderr, "Compiler::checkForLabel() : white space expected after line mumber in : '%s' : on line %d\n", lineText.c_str(), lineNumber + 1);
                return LabelError;
            }
            for(int i=1; i<space; i++)
            {
                if(!isdigit(lineText[i]))
                {
                    fprintf(stderr, "Compiler::checkForLabel() : non digits found in line number in : '%s' : on line %d\n", lineText.c_str(), lineNumber + 1);
                    return LabelError;
                }
            }

            if(lineText.size() - (space + 1) <= 2)
            {
                fprintf(stderr, "Compiler::checkForLabel() : line number cannot exist on its own : '%s' : on line %d\n", lineText.c_str(), lineNumber + 1);
                return LabelError;
            }

            createlabel(_vasmPC, lineText.substr(0, space), lineText.substr(0, space), int(_codeLines.size()), label);
            createCodeLine(_vasmPC, lineText, "", int(space + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine);

            return LabelFound;
        }

        // Text labels
        size_t colon1 = lineText.find_first_of(":");
        size_t colon2 = lineText.find_first_of(":", colon1+1);
        if(colon1 == std::string::npos)
        {
            createCodeLine(_vasmPC, lineText, "", 0, _currentLabelIndex, -1, VarInt16, false, false, false, codeLine);
            return LabelNotFound;
        }
        if(colon2 != std::string::npos)
        {
            fprintf(stderr, "Compiler::checkForLabel() : only one label per line is allowed in : '%s' : on line %d\n", lineText.c_str(), lineNumber + 1);
            return LabelError;
        }

        if(lineText.size() - (colon1 + 1) <= 2)
        {
            fprintf(stderr, "Compiler::checkForLabel() : label cannot exist on its own : '%s' : on line %d\n", lineText.c_str(), lineNumber + 1);
            return LabelError;
        }

        createlabel(_vasmPC, lineText.substr(0, colon1), lineText.substr(0, colon1), int(_codeLines.size()), label);
        createCodeLine(_vasmPC, lineText, "", int(colon1 + 1), _currentLabelIndex, -1, VarInt16, false, false, true, codeLine);

        return LabelFound;
    }

    bool parseLabels(int numLines)
    {
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
                    if(i == 0)
                    {
                        line = "_entryPoint";
                        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
                        Label label = {_vasmPC, "_entryPoint", line, 0};
                        _labels.push_back(label);
                    }
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
        if(input.find("\"") != std::string::npos) return Expression::IsString;
        if(find_if(input.begin(), input.end(), isalpha) != input.end()) return Expression::HasAlpha;
        return Expression::isExpression(input);
    }

    VarResult checkForVars(const std::string& lineText, int& varIndex, int lineNumber)
    {
        size_t equals1 = Expression::findNonStringEquals(lineText) - lineText.begin();
        if(lineText.size() > 2  &&  equals1 < lineText.size())
        {
            std::string equalsText = lineText.substr(equals1+1);
            auto equals2 = Expression::findNonStringEquals(equalsText);
            if(equals2 != equalsText.end())
            {
                fprintf(stderr, "Compiler::checkForVars() : too many '=' in '%s'\n", lineText.c_str());
                return VarError;
            }

            std::string varName = lineText.substr(0, equals1);
            Expression::stripWhitespace(varName);
            
            // Has var been initialised
            varIndex = findVar(varName);

            // Evaluate var
            int16_t result = 0;
            bool containsVars = false;
            std::string data = lineText.substr(equals1 + 1, lineText.size() - (equals1 + 1));
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
                    fprintf(stderr, "Compiler::checkForVars() : invalid expression '%s' on line %d\n", data.c_str(), lineNumber + 1);
                    return VarError;
                }
                break;
            }

            // TODO: only works with Int16, fix for all var types
            // Save result to var
            if(varIndex >= 0)
            {
                _codeLines[lineNumber]._assignOperator = true;
                _codeLines[lineNumber]._containsVars = containsVars;
                _codeLines[lineNumber]._varType = VarInt16;
                _codeLines[lineNumber]._varIndex = varIndex;
                _integerVars[varIndex]._data = result;
                return VarUpdated;
            }
            // Initialise var
            else
            {
                varIndex = int(_integerVars.size());
                _codeLines[lineNumber]._assignOperator = true;
                _codeLines[lineNumber]._containsVars = containsVars;
                _codeLines[lineNumber]._varType = VarInt16;
                _codeLines[lineNumber]._varIndex = varIndex;
                IntegerVar integerVar = {result, result, _integerVarsStart, varName, varName, lineNumber, Int16};
                _integerVars.push_back(integerVar);
                _integerVarsStart += Int16; 
                return VarInitialised;
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
            if(varResult == VarInitialised)
            {
                line = "_" + _integerVars[varIndex]._name;
                Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
                size_t space = line.find_first_of(" ");
                if(space == std::string::npos  ||  space >= LABEL_TRUNC_SIZE - 1)
                {
                    line = line.substr(0, LABEL_TRUNC_SIZE);
                    line[LABEL_TRUNC_SIZE - 1] = ' ';
                }

                _integerVars[varIndex]._output = line;
            }
            else if(varResult == VarError)
            {
                return false;
            }
        }

        return true;
    }


    void getNextTempVar(void)
    {
        static int prevCodeLine = -1;
        if(_currentCodeLine != prevCodeLine)
        {
            prevCodeLine = _currentCodeLine;
            _tempVarStart = TEMP_VAR_START;
        }
        else
        {
            _tempVarStart += 2;
            if(_tempVarStart >= USER_VAR_START) _tempVarStart = TEMP_VAR_START;
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
        _vasmPC += vasmSize;
        _codeLines[codeLineIdx]._vasm.insert(_codeLines[codeLineIdx]._vasm.begin() + vasmLineIdx, VasmLine({line, ""}));

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx=_currentCodeLine, const std::string& label="")
    {
        std::string line;

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);
        _codeLines[codeLineIdx]._vasm.push_back(VasmLine({line, label}));

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
        if(!_keywordsUsed["REM"]) _keywordsUsed["REM"] = true;

        emitVcpuAsm(";", "", false, lineNumber);

        return true;
    }
    bool handleGOTO(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        if(!_keywordsUsed["GOTO"]) _keywordsUsed["GOTO"] = true;

        std::string gotoLabel = codeLine._code.substr(4);
        if(findLabel(gotoLabel) == -1)
        {
            fprintf(stderr, "Compiler::createVasmCode() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), lineNumber + 1);
            return false;
        }

        emitVcpuAsm("CALL", "_" + gotoLabel, false, lineNumber);

        return true;
    }
    bool handlePRINT(CodeLine& codeLine, int lineNumber, size_t foundPos)
    {
        if(!_keywordsUsed["PRINT"]) _keywordsUsed["PRINT"] = true;

        std::string code = codeLine._code;
        std::vector<std::string> tokens = Expression::tokenise(code.substr(foundPos), ';');
        std::vector<std::string> strings = Expression::tokenise(code.substr(foundPos), '"');

        emitVcpuAsm("CALL", "updateCursorY", false, lineNumber);

        // Expressions
        std::vector<int> constExpr;
        for(int i=0; i<tokens.size(); i++)
        {
            Expression::ExpressionType expressionType = isExpression(tokens[i]);
            switch(expressionType)
            {
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
            }
        }

        // User strings
        std::vector<int> usrStrIdx;
        strings.erase(strings.begin());
        for(auto it=strings.begin(); it!=strings.end();)
        {
            it = (it->find(";") != std::string::npos) ? strings.erase(it) : ++it;
        }
        for(int i=0; i<strings.size(); i++)
        {
            if(strings[i].size() > USER_STR_SIZE)
            {
                fprintf(stderr, "Compiler::createVasmCode() : user string is %d characters too long in '%s' on line %d\n", int(strings[i].size() - USER_STR_SIZE), codeLine._code.c_str(), lineNumber + 1);
                return false;
            }

            // Reuse usr strings
            bool foundString = false;
            for(int j=0; j<_stringVars.size(); j++)
            {
                if(_stringVars[j]._data == strings[i]) 
                {
                    usrStrIdx.push_back(j);
                    foundString = true;
                    break;
                }
            }
            if(foundString) continue;
            
            if((_stringVarsStart + strings[i].size() & 0xFF00) != (_stringVarsStart & 0xFF00))
            {
                _stringVarsStart -= 0x0100;
                if(_stringVarsStart < 0x08A0)
                {
                    fprintf(stderr, "Compiler::createVasmCode() : out of string memory, user string address %04x in '%s' on line %d\n", _stringVarsStart, codeLine._code.c_str(), lineNumber + 1);
                    return false;
                }
            }

            std::string usrStrName = "usrStr_" + Expression::wordToHexString(_stringVarsStart);
            StringVar usrStrVar = {uint8_t(strings[i].size()), _stringVarsStart, strings[i], usrStrName, usrStrName + "\t\t", -1};
            usrStrIdx.push_back(int(_stringVars.size()));
            _stringVars.push_back(usrStrVar);
            _stringVarsStart += uint16_t(strings[i].size() + 1);
        }

        for(int i=0; i<usrStrIdx.size(); i++)
        {
            emitVcpuAsm("%PrintString", _stringVars[usrStrIdx[i]]._name, false, lineNumber);
        }

        return true;
    }
    bool createVasmCode(CodeLine& codeLine, int lineNumber)
    {
        // TODO: only works with Int16, fix for all var types
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
        else
        {
            // Temporary var expression, i.e. within ()
            if(codeLine._containsVars)
            {
                varExpressionParse(codeLine, lineNumber);
                emitVcpuAsm("STW", Expression::wordToHexString(TEMP_VAR_START), false, lineNumber);
            }

            // REM
            size_t foundPos;
            if(findKeyword(codeLine._code, "REM", foundPos))
            {
                if(!handleREM(codeLine, lineNumber, foundPos)) return false;
            }
            // GOTO
            else if(findKeyword(codeLine._code, "GOTO", foundPos))
            {
                if(!handleGOTO(codeLine, lineNumber, foundPos)) return false;
            }
            // PRINT
            else if(findKeyword(codeLine._code, "PRINT", foundPos))
            {
                if(!handlePRINT(codeLine, lineNumber, foundPos)) return false;
            }
            else
            {
                emitVcpuAsm("LDWI", Expression::wordToHexString(0), false, lineNumber);
            }
        }

        return true;
    }

    bool parseCode(void)
    {
        std::string line;

        for(int i=0; i<_codeLines.size(); i++)
        {
            _currentCodeLine = i;

            if(_codeLines[i]._code.size() > 2)
            {
                // Adjust label address except for first label
                if(_codeLines[i]._ownsLabel) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;

                // Create .vasm code
                uint16_t oldVasmPC = _vasmPC;
                if(!createVasmCode(_codeLines[i], i)) return false;

                // Exclusion zones and page boundary crossings
                if((_vasmPC & 0x00FF) >= 0xF1  ||  (oldVasmPC & 0xFF00) != (_vasmPC & 0xFF00))
                {
                    _codeLines[i]._vasm.clear();
                    _vasmPC = (oldVasmPC & 0xFF00) + 0x0100;
                    if(_codeLines[i]._ownsLabel) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;
                    emitVcpuAsm("CALL",  _labels[_codeLines[i]._labelIndex]._output, false, i-1);
                    if(!createVasmCode(_codeLines[i], i)) return false;
                }
            }
        }

        return true;
    }

    enum OptimiseTypes {StwLdwPair=0, ExtraStw, NumOptimiseTypes};
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

    void insertInitCode(void)
    {
        // Back to front
        int labelAdjust = 0;
        labelAdjust += insertVcpuAsm("CALL", "clearScreen\n", 0, 0);
        labelAdjust += insertVcpuAsm("LDWI", "0x2020",        0, 0);
        labelAdjust += insertVcpuAsm("STW",  "textColour",    0, 0);
        labelAdjust += insertVcpuAsm("LDWI", "0x0F20",        0, 0);
        labelAdjust += insertVcpuAsm("STW",  "textPosition",  0, 0);
        labelAdjust += insertVcpuAsm("LDWI", "0x0802",        0, 0);
        labelAdjust += insertVcpuAsm("STW",  "cursorXY",      0, 0);
        labelAdjust += insertVcpuAsm("LDWI", "0x0000",        0, 0);
        adjustLabelAddresses(0, labelAdjust);
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

        for(int i=0; i<_labels.size(); i++)
        {
            std::string address = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU\t\t" + address + "\n");
        }

        _output.push_back("\n");
    }

    void outputInternalSubs(void)
    {
        _output.push_back("clearScreen     EQU     0x7FA0\n");
        _output.push_back("printText       EQU     clearScreen - 0x0100\n");
        _output.push_back("printVarInt16   EQU     clearScreen - 0x0200\n");
        _output.push_back("drawChar        EQU     clearScreen - 0x0300\n");
        _output.push_back("locateCursor    EQU     clearScreen - 0x0400\n");
        _output.push_back("resetAudio      EQU     clearScreen - 0x0500\n");
        _output.push_back("playMidi        EQU     clearScreen - 0x0600\n");
        _output.push_back("midiStartNote   EQU     clearScreen - 0x0700\n");
        _output.push_back("\n");
    }

    void outputInternalVars(void)
    {
        _output.push_back("; Internal variables\n");
        _output.push_back("register0       EQU     0xA0\n");
        _output.push_back("register1       EQU     register0 + 0x02\n");
        _output.push_back("register2       EQU     register0 + 0x04\n");
        _output.push_back("register3       EQU     register0 + 0x06\n");
        _output.push_back("register4       EQU     register0 + 0x08\n");
        _output.push_back("register5       EQU     register0 + 0x0A\n");
        _output.push_back("register6       EQU     register0 + 0x0C\n");
        _output.push_back("register7       EQU     register0 + 0x0E\n");
        _output.push_back("textPosition    EQU     register0 + 0x10\n");
        _output.push_back("textColour      EQU     register0 + 0x12\n");
        _output.push_back("cursorXY        EQU     register0 + 0x14\n");
        _output.push_back("midiStreamPtr   EQU     register0 + 0x16\n");
        _output.push_back("midiDelay       EQU     register0 + 0x18\n");
        _output.push_back("frameCountPrev  EQU     register0 + 0x1A\n");
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
        _input.clear();
        _output.clear();
        _labels.clear();
        _codeLines.clear();
        _integerVars.clear();
        _stringVars.clear();
        _arrayVars.clear();

        _tempVarStart     = TEMP_VAR_START;
        _integerVarsStart = USER_VAR_START;
        _vasmPC           = USER_CODE_START;

        _currentCodeLine = 0;

        _tempVarStartStr = "";
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

        insertInitCode();

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