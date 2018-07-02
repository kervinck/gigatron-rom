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


// The smaller you make this, the more your BASIC label names will be truncated in the resultant .vasm code
#define LABEL_TRUNC_SIZE 16

#define TEMP_VAR_START  0x0030
#define USER_VAR_START  0x0040
#define USER_EXPR_START 0x00C0
#define USER_CODE_START 0x0200



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

    struct CodeLine
    {
        uint16_t _address;
        uint16_t _vasmSize;
        std::string _code;
        std::string _vasm;
        std::string _expression;
        int _labelIndex = -1;
        int  _varIndex = -1;
        VarType _varType = VarInt16;
        bool _assignOperator = false;
        bool _containsVars = false;
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

    uint16_t _tempVarStart     = TEMP_VAR_START;
    uint16_t _integerVarsStart = USER_VAR_START;
    uint16_t _expressionsStart = USER_EXPR_START;
    uint16_t _vasmPC           = USER_CODE_START;

    int _currentCodeLine = 0;

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


    bool findKeyword(std::string code, const std::string& keyword)
    {
        Expression::strToUpper(code);
        if(code.find(keyword) != std::string::npos) return true;
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
        _labels.push_back(label);
    }

    void createCodeLine(uint16_t address, uint16_t vasmSize, const std::string& lineText, const std::string& vasmCode, int codeLineOffset, int labelIndex, int varIndex, VarType varType, bool assign, bool vars, CodeLine& codeLine)
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

        codeLine = {address, vasmSize, lineText.substr(codeLineOffset, lineText.size() - (codeLineOffset)), vasmCode, expression, labelIndex, varIndex, varType, assign, vars};
        Expression::stripWhitespace(codeLine._code);
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
            createCodeLine(_vasmPC, 0, lineText, "", int(space + 1), int(_labels.size() - 1), -1, VarInt16, false, false, codeLine);

            return LabelFound;
        }

        // Text labels
        size_t colon1 = lineText.find_first_of(":");
        size_t colon2 = lineText.find_first_of(":", colon1+1);
        if(colon1 == std::string::npos)
        {
            createCodeLine(_vasmPC, 0, lineText, "", 0, (_labels.size() == 0) ? 0 : -1, -1, VarInt16, false, false, codeLine);
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
        createCodeLine(_vasmPC, 0, lineText, "", int(colon1 + 1), int(_labels.size() - 1), -1, VarInt16, false, false, codeLine);

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


    Expression::ExpressionType isExpression(const std::string& input)
    {
        if(find_if(input.begin(), input.end(), isalpha) != input.end()) return Expression::HasAlpha;
        return Expression::isExpression(input);
    }

    VarResult checkForVars(const std::string& lineText, int& varIndex, int lineNumber)
    {
#if 0
        // Search for vars
        size_t equals1 = lineText.find_first_of("=");
        size_t equals2 = lineText.find_first_of("=", equals1+1);
        if(equals2 != std::string::npos)
        {
            fprintf(stderr, "Compiler::checkForVars() : too many '=' in '%s'\n", lineText.c_str());
            return VarError;
        }
        if(equals1 != std::string::npos)
        {
            std::string varName = lineText.substr(0, equals1);
            Expression::stripWhitespace(varName);
#else

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
#endif
            
            // Has var been initialised
            varIndex = findVar(varName);

            // Evaluate var
            int16_t result = 0;
            bool containsVars = false;
            std::string data = lineText.substr(equals1 + 1, lineText.size() - (equals1 + 1));
            Expression::stripWhitespace(data);
            Expression::operatorReduction(data);
            Expression::setDefaultOperatorFuncs();
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
        // Increment temporary variable address
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

    bool pickOpcodeAndOperand(std::string& opcode, Expression::Numeric& left, Expression::Numeric& right, std::string& leftStr, std::string& rightStr)
    {
        // Swap left and right to take advantage of LDWI for 16bit numbers
        if(!right._isAddress  &&  abs(right._value) > 255)
        {
            std::swap(left, right);
            if(opcode == "SUB")
            {
                opcode = "ADD";
                if(left._value > 0) left._value = -left._value;
            }
        }

        // Optimisation, (removes uneeded LDW's)
        bool optimise = (left._isAddress  &&  isdigit(*left._varNamePtr)  &&  left._value == _tempVarStart);
        bool constant16bit = false;

        static int prevCodeLine = -1;
        if(_currentCodeLine != prevCodeLine)
        {
            prevCodeLine = _currentCodeLine;
        }
        else
        {
            if(!optimise)
            {
                _codeLines[_currentCodeLine]._vasm += "STW\t\t" + Expression::byteToHexString(uint8_t(_tempVarStart)) + "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
                _codeLines[_currentCodeLine]._vasmSize += 2;
            }
        }

        // Left instructions
        if(left._isAddress)
        {
            if(!optimise)
            {
                // Temporary variable address
                if(isdigit(*left._varNamePtr))
                {
                    leftStr = "LDW\t\t" + Expression::byteToHexString(uint8_t(left._value));
                    _codeLines[_currentCodeLine]._vasmSize += 2;
                }
                // User variable address
                else
                {
                    std::string varName = std::string(left._varNamePtr);
                    int varIndex = findVar(varName);
                    if(varIndex == -1)
                    {
                        fprintf(stderr, "Compiler::pickOpcodeAndOperand() : couldn't find variable name '%s'\n", varName.c_str());
                        return false;
                    }
                    else
                    {
                        leftStr = "LDW\t\t_" + _integerVars[varIndex]._name;
                        _codeLines[_currentCodeLine]._vasmSize += 2;
                        getNextTempVar(); // next temporary variable for each new user variable loaded
                    }
                }
            }
        }
        else
        {
            // 8bit positive constants
            if(left._value >=0  &&  left._value <= 255)
            {
                leftStr = "LDI\t\t" + std::to_string(left._value);
                _codeLines[_currentCodeLine]._vasmSize += 2;
            }
            // 16bit constants
            else
            {
                constant16bit = true;
                leftStr = "LDWI\t" + std::to_string(left._value);
                _codeLines[_currentCodeLine]._vasmSize += 3;
            }
        }

        // Right instructions
        if(right._isAddress)
        {
            // Temporary variable address
            if(isdigit(*right._varNamePtr))
            {
                rightStr = opcode + "W\t" + Expression::byteToHexString(uint8_t(right._value));
                _codeLines[_currentCodeLine]._vasmSize += 2;
            }
            // User variable address
            else
            {
                std::string varName = std::string(right._varNamePtr);
                int varIndex = findVar(varName);
                if(varIndex == -1)
                {
                    fprintf(stderr, "Compiler::pickOpcodeAndOperand() : couldn't find variable name '%s'\n", varName.c_str());
                    return false;
                }
                else
                {
                    rightStr = opcode + "W\t_" + _integerVars[varIndex]._name;
                    _codeLines[_currentCodeLine]._vasmSize += 2;
                    if(constant16bit) getNextTempVar();  // next temporary variable for each new user variable add/sub to/from 16bit number
                }
            }
        }
        else
        {
            rightStr = opcode + "I\t" + std::to_string(right._value); 
            _codeLines[_currentCodeLine]._vasmSize += 2;
        }

        if(!optimise)
        {
            _codeLines[_currentCodeLine]._vasm += leftStr;
            _codeLines[_currentCodeLine]._vasm += "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
        }

        _codeLines[_currentCodeLine]._vasm += rightStr;
        _codeLines[_currentCodeLine]._vasm += "\n" + std::string(LABEL_TRUNC_SIZE, ' ');

        left._value = uint8_t(_tempVarStart);
        left._isAddress = true;
        left._varNamePtr = (char *)_tempVarStartStr.c_str();

        return true;
    }

    // Expression operators
    Expression::Numeric neg(Expression::Numeric& numeric)
    {
        numeric._value = -numeric._value;
        return numeric;
    }
    Expression::Numeric add(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value += right._value;
            return left;
        }

        std::string leftStr, rightStr, opcode = "ADD";
        pickOpcodeAndOperand(opcode, left, right, leftStr, rightStr);
        return left;
    }
    Expression::Numeric sub(Expression::Numeric& left, Expression::Numeric& right)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value -= right._value;
            return left;
        }

        std::string leftStr, rightStr, opcode = "SUB";;
        pickOpcodeAndOperand(opcode, left, right, leftStr, rightStr);
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
    Expression::Numeric fac(int16_t defaultValue)
    {
        int16_t value = 0;
        if((Expression::peek() >= '0'  &&  Expression::peek() <= '9')  ||  Expression::peek() == '$')
        {
            if(!Expression::number(value)) value = 0;
            return Expression::Numeric(value, false, nullptr);
        }
        else if(Expression::peek() == '(')
        {
            Expression::get();
            Expression::Numeric numeric = Expression::expression();
            Expression::get();
            return numeric;
        }
        else if(Expression::peek() == '-')
        {
            Expression::get();
            return neg(fac(0));
        }

        Expression::Numeric numeric = Expression::Numeric(defaultValue, true, Expression::getExpression());
        while(isalpha(Expression::peek())) Expression::get();
        return numeric;
    }
    void varExpressionParse(CodeLine& codeLine, int lineNumber)
    {
        Expression::setNegFunc(neg);
        Expression::setAddFunc(add);
        Expression::setSubFunc(sub);
        Expression::setMulFunc(mul);
        Expression::setDivFunc(div);
        Expression::setFacFunc(fac);
        Expression::parse((char*)codeLine._expression.c_str(), lineNumber);
    }
    int varAssignmentParse(CodeLine& codeLine, int lineNumber)
    {
        if(codeLine._expression.find_first_of("-+/*()") != std::string::npos) return -1;
        int varIndex = findVar(codeLine._expression);
        return varIndex;
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
                if(varIndex >= 0  &&  varIndex != prevVarIndex)
                {
                    codeLine._vasm += "LDW\t\t_" + _integerVars[varIndex]._name + "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
                    codeLine._vasmSize += 2;
                }
                codeLine._vasm += "STW\t\t_" + _integerVars[codeLine._varIndex]._name;
                codeLine._vasmSize += 2;
                prevVarIndex = codeLine._varIndex;
            }
            // Standard assignment
            else
            {
                // 8bit constants
                if(_integerVars[codeLine._varIndex]._init >=0  &&  _integerVars[codeLine._varIndex]._init <= 255)
                {
                    codeLine._vasm = "LDI\t\t" + std::to_string(_integerVars[codeLine._varIndex]._init) + "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
                    codeLine._vasmSize += 2;
                }
                // 16bit constants
                else
                {
                    codeLine._vasm = "LDWI\t" + std::to_string(_integerVars[codeLine._varIndex]._init) + "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
                    codeLine._vasmSize += 3;
                }

                codeLine._vasm += "STW\t\t_" + _integerVars[codeLine._varIndex]._name;
                codeLine._vasmSize += 2;
                prevVarIndex = codeLine._varIndex;
            }
        }
        else
        {
            // Temporary var expression, i.e. within ()
            if(codeLine._containsVars)
            {
                varExpressionParse(codeLine, lineNumber);
                codeLine._vasm += "STW\t\t" + Expression::wordToHexString(TEMP_VAR_START) + "\n" + std::string(LABEL_TRUNC_SIZE, ' ');
                codeLine._vasmSize += 2;
            }

            // GOTO
            if(findKeyword(codeLine._code, "GOTO"))
            {
                std::string gotoLabel = codeLine._code.substr(4);
                if(findLabel(gotoLabel) == -1)
                {
                    fprintf(stderr, "Compiler::createVasmCode() : invalid label in '%s' on line %d\n", codeLine._code.c_str(), lineNumber + 1);
                    return false;
                }
                codeLine._vasm = "BRA\t\t_" + gotoLabel;
                codeLine._vasmSize += 2;
            }
            else
            {
                codeLine._vasm = "LDWI\t0x0000";
                codeLine._vasmSize += 3;
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
                // Create .vasm code
                if(!createVasmCode(_codeLines[i], i)) return false;

                // Adjust label address except for first label
                int labelIdx = _codeLines[i]._labelIndex;
                if(labelIdx > 0) _labels[labelIdx]._address = _vasmPC;
                _vasmPC += _codeLines[i]._vasmSize;
            }
        }

        return true;
    }


    void outputStartAddress(void)
    {
        std::string line = "_startAddress_ ";
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        line += "EQU\t\t" + Expression::wordToHexString(USER_CODE_START) + "\n\n";
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

    void outputCode(void)
    {
        std::string line;

        _output.push_back("; Code\n");

        for(int i=0; i<_codeLines.size(); i++)
        {
            int labelIndex = _codeLines[i]._labelIndex;

            // New line before label, except first
            if(labelIndex > 0) _output.push_back("\n");

            // Valid code
            if(_codeLines[i]._code.size() > 2)
            {
                // Code with a label
                if(labelIndex >= 0) 
                {
                    _output.push_back(_labels[labelIndex]._output + _codeLines[i]._vasm + "\t\t; " + _codeLines[i]._code + "\n");
                }
                // Code
                else
                {
                    std::string line = _codeLines[i]._vasm + "\t\t; " + _codeLines[i]._code;
                    Expression::padString(line, LABEL_TRUNC_SIZE + int(line.size()));
                    _output.push_back(line + "\n");
                }
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
        _expressionsStart = USER_EXPR_START;
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

        // Code
        if(!parseCode()) return false;

        // Output
        outputStartAddress();
        outputLabels();
        outputVars();
        outputCode();

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        return true;
    }
}