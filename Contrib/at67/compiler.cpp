#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmath>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "keywords.h"
#include "operators.h"
#include "optimiser.h"
#include "validater.h"
#include "linker.h"


namespace Compiler
{
    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated, VarExists};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};


    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart   = USER_VAR_START;
    uint16_t _runtimeEnd     = 0xFFFF;
    uint16_t _runtimeStart   = 0xFFFF;
    uint16_t _strWorkArea    = 0x0000;

    CodeOptimiseType _codeOptimiseType = CodeSpeed;

    bool _compilingError = false;
    bool _arrayIndiciesOne = false;
    bool _createNumericLabelLut = false;

    int _currentLabelIndex = -1;
    int _currentCodeLineIndex = 0;
    int _jumpFalseUniqueId = 0;

    std::string _tempVarStartStr;
    std::string _nextInternalLabel;

    std::vector<std::string> _input;
    std::vector<std::string> _output;
    std::vector<std::string> _runtime;

    std::vector<Label>         _labels;
    std::vector<std::string>   _gosubLabels;
    std::vector<std::string>   _equateLabels;
    std::vector<InternalLabel> _internalLabels;
    std::vector<InternalLabel> _discardedLabels;

    std::vector<CodeLine>   _codeLines;
    std::vector<Constant>   _constants;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar>  _stringVars;

    std::stack<ForNextData>     _forNextDataStack;
    std::stack<ElseIfData>      _elseIfDataStack;
    std::stack<EndIfData>       _endIfDataStack;
    std::stack<WhileWendData>   _whileWendDataStack;
    std::stack<RepeatUntilData> _repeatUntilDataStack;

    std::vector<std::string> _macroLines;
    std::map<int, MacroNameEntry> _macroNameEntries;
    std::map<std::string, MacroIndexEntry> _macroIndexEntries;

    std::vector<DefDataByte>  _defDataBytes;
    std::vector<DefDataWord>  _defDataWords;
    std::vector<DefDataImage> _defDataImages;


    uint16_t getVasmPC(void) {return _vasmPC;}
    uint16_t getRuntimeEnd(void) {return _runtimeEnd;}
    uint16_t getRuntimeStart(void) {return _runtimeStart;}
    uint16_t getTempVarStart(void) {return _tempVarStart;}
    uint16_t getStrWorkArea(void) {return _strWorkArea;}
    CodeOptimiseType getCodeOptimiseType(void) {return _codeOptimiseType;}
    bool getCompilingError(void) {return _compilingError;}
    bool getArrayIndiciesOne(void) {return _arrayIndiciesOne;}
    std::string& getTempVarStartStr(void) {return _tempVarStartStr;}
    int getCurrentLabelIndex(void) {return _currentLabelIndex;}
    std::string& getNextInternalLabel(void) {return _nextInternalLabel;}

    void setRuntimeEnd(uint16_t runtimeEnd) {_runtimeEnd = runtimeEnd;}
    void setRuntimeStart(uint16_t runtimeStart) {_runtimeStart = runtimeStart;}
    void setTempVarStart(uint16_t tempVarStart) {_tempVarStart = tempVarStart;}
    void setStrWorkArea(uint16_t strWorkArea) {_strWorkArea = strWorkArea;}
    void setCompilingError(bool compilingError) {_compilingError = compilingError;}
    void setArrayIndiciesOne(bool arrayIndiciesOne) {_arrayIndiciesOne = arrayIndiciesOne;}
    void setCreateNumericLabelLut(bool createNumericLabelLut) {_createNumericLabelLut = createNumericLabelLut;}
    void setCodeOptimiseType(CodeOptimiseType codeOptimiseType) {_codeOptimiseType = codeOptimiseType;}

    int incJumpFalseUniqueId(void) {return _jumpFalseUniqueId++;}

    std::vector<Label>& getLabels(void) {return _labels;}
    std::vector<Constant>& getConstants(void) {return _constants;}
    std::vector<CodeLine>& getCodeLines(void) {return _codeLines;}
    std::vector<IntegerVar>& getIntegerVars(void) {return _integerVars;}
    std::vector<StringVar>& getStringVars(void) {return _stringVars;}
    std::vector<InternalLabel>& getInternalLabels(void) {return _internalLabels;}
    std::vector<InternalLabel>& getDiscardedLabels(void) {return _discardedLabels;}
    std::vector<std::string>& getOutput(void) {return _output;}
    std::vector<std::string>& getRuntime(void) {return _runtime;}
    std::vector<DefDataByte>& getDefDataBytes(void) {return _defDataBytes;}
    std::vector<DefDataWord>& getDefDataWords(void) {return _defDataWords;}
    std::vector<DefDataImage>& getDefDataImages(void) {return _defDataImages;}

    std::map<std::string, MacroIndexEntry>& getMacroIndexEntries(void) {return _macroIndexEntries;}

    std::stack<ForNextData>& getForNextDataStack(void) {return _forNextDataStack;}
    std::stack<ElseIfData>& getElseIfDataStack(void) {return _elseIfDataStack;}
    std::stack<EndIfData>& getEndIfDataStack(void) {return _endIfDataStack;}
    std::stack<WhileWendData>& getWhileWendDataStack(void) {return _whileWendDataStack;}
    std::stack<RepeatUntilData>& getRepeatUntilDataStack(void) {return _repeatUntilDataStack;}


    // If _nextInternalLabel is already queued, add it to discarded labels so that it is fixed later in outputLabels
    void setNextInternalLabel(const std::string& label)
    {
        if(_nextInternalLabel.size()) _discardedLabels.push_back({_vasmPC, _nextInternalLabel});

        _nextInternalLabel = label;
    }

    void adjustDiscardedLabels(const std::string name, uint16_t address)
    {
        uint16_t match;
        Expression::stringToU16(name.substr(name.size() - 6, 6), match);
        for(int i=0; i<int(_discardedLabels.size()); i++)
        {
            if(_discardedLabels[i]._address == match) _discardedLabels[i]._address = address;
        }
    }


    bool initialise(void)
    {
        return true;
    }


    bool readInputFile(std::ifstream& infile, const std::string& filename, int& numLines)
    {
        std::string lineToken;

        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::readInputFile() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        // Read input .gbas file
        while(!infile.eof())
        {
            std::getline(infile, lineToken);
            _input.push_back(lineToken);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Compiler::readInputFile() : Bad line : '%s' : in '%s' : on line %d\n", lineToken.c_str(), filename.c_str(), numLines+1);
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
        for(int i=0; i<int(_output.size()); i++)
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


    int findLabel(const std::string& labelName)
    {
        for(int i=0; i<int(_labels.size()); i++)
        {
            if(_labels[i]._name == labelName) return i;
        }

        return -1;
    }
    
    int findLabel(uint16_t address)
    {
        for(int i=0; i<int(_labels.size()); i++)
        {
            if(_labels[i]._address == address) return i;
        }

        return -1;
    }

    int findInternalLabel(const std::string& labelName)
    {
        for(int i=0; i<int(_internalLabels.size()); i++)
        {
            if(_internalLabels[i]._name == labelName) return i;
        }

        return -1;
    }
    
    int findInternalLabel(uint16_t address)
    {
        for(int i=0; i<int(_internalLabels.size()); i++)
        {
            if(_internalLabels[i]._address == address) return i;
        }

        return -1;
    }

    int findConst(std::string& constName)
    {
        // Valid chars are alpha and 'address of'
        constName = Expression::getSubAlpha(constName);
        for(int i=0; i<int(_constants.size()); i++)
        {
            if(_constants[i]._name == constName) return i;
        }

        return -1;
    }

    int findVar(std::string& varName, bool subAlpha)
    {
        // Valid chars are alpha and 'address of'
        if(subAlpha) varName = Expression::getSubAlpha(varName);
        for(int i=0; i<int(_integerVars.size()); i++)
        {
            if(_integerVars[i]._name == varName) return i;
        }

        return -1;
    }

    int findStr(std::string& strName)
    {
        // Valid chars are alpha and 'address of'
        strName = Expression::getSubAlpha(strName);
        for(int i=0; i<int(_stringVars.size()); i++)
        {
            if(_stringVars[i]._name == strName) return i;
        }

        return -1;
    }


    void createLabel(uint16_t address, const std::string& name, int codeLineIndex, Label& label, bool numeric, bool addUnderscore, bool pageJump, bool gosub)
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

        label = {address, n, o, codeLineIndex, numeric, pageJump, gosub};
        Expression::stripWhitespace(label._name);
        _labels.push_back(label);
        _currentLabelIndex = int(_labels.size() - 1);
    }

    void createIntVar(const std::string& varName, int16_t data, int16_t init, CodeLine& codeLine, int codeLineIndex, bool containsVars, int& varIndex, VarType varType, uint16_t arrayStart, int intSize, int arrSize)
    {
        // Create var
        varIndex = int(_integerVars.size());
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;

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
        if(codeLine._code.size() < 2  ||  equals >= codeLine._code.size()) return VarNotFound;

        // Check all tokens individually, don't just do a find as a var may exist with a reserved keyword embedded within it
        for(int i=0; i<int(codeLine._tokens.size()); i++)
        {
            std::string token = codeLine._tokens[i];
            Expression::stripWhitespace(token);
            Expression::strToUpper(token);

            // No need to check after the equals
            if(token == "=") break;

            // Check tokens that are reserved keywords using equals
            if(Keywords::getEqualsKeywords().find(token) != Keywords::getEqualsKeywords().end()) return VarNotFound;
        }

        // Name
        std::string varName = codeLine._code.substr(0, equals);
        Expression::stripWhitespace(varName);

        // Byte write configuration
        size_t dot = varName.find('.');
        if(dot != std::string::npos)
        {
            std::string dotName = varName.substr(dot);
            varName = varName.substr(0, dot);
            Expression::strToUpper(dotName);
            if(dotName == ".LO") codeLine._int16Byte = Expression::Int16Low;
            if(dotName == ".HI") codeLine._int16Byte = Expression::Int16High;
        }

        // String variables are not handled here
        if(varName.back() == '$') return VarError;

        // Var already exists?
        varIndex = findVar(varName);
        if(varIndex != -1)
        {
            codeLine._containsVars = false;
            codeLine._varIndex = varIndex;
            codeLine._varType = VarInt16;

            return VarExists;
        }

        createIntVar(varName, 0, 0, codeLine, codeLineIndex, false, varIndex);

        return VarCreated;
    }

    VarResult createCodeStr(CodeLine& codeLine, int codeLineIndex, int& strIndex)
    {
        // Code and string have minimum size requirements
        size_t equals = Expression::findNonStringEquals(codeLine._code) - codeLine._code.begin();
        if(codeLine._code.size() < 2  ||  equals >= codeLine._code.size() - 2  ||  codeLine._tokens.size() < 2) return VarError;

        // Check all tokens individually, don't just do a find as a str name may exist with a reserved keyword embedded within it
        for(int i=0; i<int(codeLine._tokens.size()); i++)
        {
            std::string token = codeLine._tokens[i];
            Expression::stripWhitespace(token);
            Expression::strToUpper(token);

            // Check tokens that are reserved keywords using equals
            if(Keywords::getEqualsKeywords().find(token) != Keywords::getEqualsKeywords().end()) return VarError;
        }

        // String name validation
        std::string strName = codeLine._code.substr(0, equals);
        Expression::stripWhitespace(strName);
        if(strName.back() != '$') return VarError;
        if(!Expression::isVarNameValid(strName)) return VarError;
        strIndex = findStr(strName);
        if(strIndex != -1)
        {
            codeLine._varIndex = strIndex;
            codeLine._varType = VarStr;

            return VarExists;
        }

        // String data validation
        std::string strData = codeLine._tokens[1];
        Expression::stripNonStringWhitespace(strData);
        std::string strKeyword = strData;

        // Check for string keywords
        if(Keywords::getStringKeywords().find(Expression::strToUpper(strKeyword)) != Keywords::getStringKeywords().end())
        {
            strData = "";
        }
        // Check for constant string
        else
        {
            if(Expression::isValidString(strData))
            {
                // Strip quotes
                strData.erase(0, 1);
                strData.erase(strData.size()-1, 1);
            }
            // Check for assignment of another variable
            else
            {
                if(strData.back() != '$') return VarError;
                strData = "";
            }
        }

        // Create string
        uint16_t address;
        if(getOrCreateString(codeLine, codeLineIndex, strData, strName, address, USER_STR_SIZE, false) == -1) return VarError;
        strIndex = int(_stringVars.size()) - 1;
        codeLine._varIndex = strIndex;
        codeLine._varType = VarStr;

        return VarCreated;
    }

    uint32_t isExpression(std::string& input, int& varIndex, int& constIndex, int& strIndex)
    {
        uint32_t expressionType = Expression::HasNumbers;

        if(input.find_first_of("[]") != std::string::npos) return Expression::IsInvalid;
        if(input.find("++") != std::string::npos) return Expression::IsInvalid;
        if(input.find("--") != std::string::npos) return Expression::IsInvalid;

        // Check for strings
        if(input.find("\"") != std::string::npos) expressionType |= Expression::HasStrings;

        std::string stripped = Expression::stripStrings(input);
        std::vector<std::string> tokens = Expression::tokenise(stripped, "-+/*%&<>=();, ", false);

        // Check for pragmas
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getPragmas().find(token) != Keywords::getPragmas().end())
            {
                expressionType |= Expression::HasPragmas;
                break;
            }
        }

        // Check for keywords
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getKeywords().find(token) != Keywords::getKeywords().end())
            {
                expressionType |= Expression::HasKeywords;
                break;
            }
        }

        // Check for functions
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getFunctions().find(token) != Keywords::getFunctions().end())
            {
                expressionType |= Expression::HasFunctions;
                break;
            }
        }

        // Check for string keywords
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getStringKeywords().find(token) != Keywords::getStringKeywords().end())
            {
                // If first token is a string keyword then don't create strings
                if(i == 0) expressionType |= Expression::HasOptimisedPrint;

                expressionType |= Expression::HasStringKeywords;
                break;
            }
        }

        // Check for consts
        for(int i=0; i<int(tokens.size()); i++)
        {
            constIndex = findConst(tokens[i]);
            if(constIndex != -1  &&  tokens[i][0] != '@') // 'address of' operator returns numbers
            {
                if(_constants[constIndex]._constType == ConstInt16)
                {
                    expressionType |= Expression::HasIntConsts;
                    break;
                }
                if(_constants[constIndex]._constType == ConstStr)
                {
                    expressionType |= Expression::HasStrConsts;
                    break;
                }
            }
        }

        // Check for vars
        for(int i=0; i<int(tokens.size()); i++)
        {
            varIndex = findVar(tokens[i]);
            if(varIndex != -1  &&  tokens[i][0] != '@') // 'address of' operator returns numbers
            {
                // Array variables are treated as a function call
                if(_integerVars[varIndex]._varType == VarArray)
                {
                    expressionType |= Expression::HasFunctions;
                    break;
                }

                expressionType |= Expression::HasIntVars;
                break;
            }
        }

        // Check for string vars
        for(int i=0; i<int(tokens.size()); i++)
        {
            strIndex = findStr(tokens[i]);
            if(strIndex != -1  &&  _stringVars[strIndex]._constant == false  &&  tokens[i][0] != '@') // 'address of' operator returns numbers)
            {
                expressionType |= Expression::HasStrVars;
                break;
            }
        }

        // Check for operators for non string expressions
        bool isStrExpression = ((expressionType >= Expression::HasStrings)  &&  (expressionType <= Expression::IsStringExpression));
        if(!isStrExpression)
        {
            if(stripped.find_first_of("-+/*%&<>=") != std::string::npos) expressionType |= Expression::HasOperators;
            std::string mod = stripped;
            Expression::strToUpper(mod);
            for(int i=0; i<int(Keywords::getOperators().size()); i++)
            {
                if(mod.find(Keywords::getOperators()[i]) != std::string::npos) expressionType |= Expression::HasOperators;
            }
        }

        return expressionType;
    }

    void updateVar(int16_t data, CodeLine& codeLine, int varIndex, bool containsVars)
    {
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;
        _integerVars[varIndex]._data = data;
    }

    bool createCodeLine(const std::string& code, int codeLineOffset, int labelIndex, int varIndex, Expression::Int16Byte int16Byte, bool vars, CodeLine& codeLine)
    {
        // Handle variables
        size_t equal = code.find_first_of("=");
        std::string expression = (equal != std::string::npos) ? code.substr(equal + 1) : code;
        Expression::trimWhitespace(expression);

        std::vector<int> onLut;
        OnGotoGosubLut onGotoGosubLut = {0x0000, "", onLut};

        std::vector<uint16_t> concatStrs;
        StrConcatLut strConcatLut = {0x0000, concatStrs};

        std::vector<uint16_t> inputVars;
        std::vector<uint16_t> inputStrs;
        std::vector<uint16_t> inputTypes;
        InputLut inputLut = {0x0000, 0x0000, 0x0000, 0x0000, inputVars, inputStrs, inputTypes};

        std::vector<VasmLine> vasm;
        std::string text = code.substr(codeLineOffset, code.size() - (codeLineOffset));
        Expression::trimWhitespace(text);
        std::string codeText = Expression::collapseWhitespaceNotStrings(text);
        codeText = Expression::removeCommentsNotInStrings(codeText);
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokeniseLine(codeText, " (),=", offsets);
        codeLine = {text, codeText, tokens, offsets, vasm, expression, onGotoGosubLut, strConcatLut, inputLut, 0, labelIndex, varIndex, VarInt16, int16Byte, vars, false};
        Expression::operatorReduction(codeLine._expression);

        //if(codeLine._code.size() < 2) return false; // anything too small is ignored

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

        std::string textU = text;
        Expression::strToUpper(textU);

        int indexStart = _macroIndexEntries[macroName]._indexStart;
        int indexEnd = _macroIndexEntries[macroName]._indexEnd;
        for(int i=indexStart+1; i<indexEnd; i++)
        {
            size_t commentStart = _macroLines[i].find_first_of(";#");
            std::string macroLine = (commentStart != std::string::npos) ? _macroLines[i].substr(0, commentStart) : _macroLines[i];
            std::vector<std::string> tokens = Expression::tokeniseLine(macroLine);

            for(int j=0; j<int(tokens.size()); j++)
            {
                Expression::stripWhitespace(tokens[j]);
                Expression::strToUpper(tokens[j]);
                if(tokens[j] == textU) return true;
            }

            // Check for nested macros
            for(int j=0; j<int(tokens.size()); j++)
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
            for(int j=0; j<int(tokens.size()); j++)
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
                for(int j=0; j<int(tokens.size()); j++)
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
        std::string filename = (!Assembler::getUseOpcodeCALLI()) ? "/include/macros.i" : "/include/macros_CALLI.i";
        filename = Assembler::getIncludePath() + filename;
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
        int macroIndex = 0;
        std::string macroName;
        bool foundMacro = false;
        for(int i=0; i<int(_macroLines.size()); i++)
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
            it->second._byteSize = getMacroSize(it->second._name);
            _macroIndexEntries[it->second._name]._byteSize = it->second._byteSize;
            //macroIndex = _macroIndexEntries[it->second._name]._indexStart;
            //fprintf(stderr, "%s  %d %d  %d %d bytes\n", it->second._name.c_str(), macroIndex, it->second._indexEnd, it->second._byteSize, _macroIndexEntries[it->second._name]._byteSize);
        }

        return true;
    }


    int createVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, std::string& line)
    {
        UNREFERENCED_PARAM(codeLineIdx);

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

        _vasmPC += uint16_t(vasmSize);

        //fprintf(stderr, "%s  %d %04x\n", opcode.c_str(), vasmSize, _vasmPC);

        std::string operand = std::string(operandStr);
        line = opcode + std::string(OPCODE_TRUNC_SIZE - opcode.size(), ' ') + operand;

        return vasmSize;
    }

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx, const std::string& internalLabel, bool pageJump)
    {
        if(codeLineIdx == -1) codeLineIdx = _currentCodeLineIndex;

        std::string line;
        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);

        // NEXT and THEN don't know where the next vasm instruction is, so they use _nextInternalLabel, (which has priority over internalLabel)
        std::string label = (_nextInternalLabel.size()) ? _nextInternalLabel : internalLabel;

        // Discarded labels are replaced correctly later in outputLabels()
        if(_nextInternalLabel.size()  &&  internalLabel.size()) _discardedLabels.push_back({_vasmPC, internalLabel});

        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, line, label, pageJump, vasmSize});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        if(nextTempVar) getNextTempVar();
        _nextInternalLabel = "";
    }

    // Generic LDW expression parser
    uint32_t parseArrayVarExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        UNREFERENCED_PARAM(codeLine);

        int varIndex, constIndex, strIndex;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);
        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasIntVars)
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;
                case Expression::Int16High: emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name + " + 1", false, codeLineIndex); break;
                case Expression::Int16Both: emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;

                default: break;
            }
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
        Expression::Numeric arrIndex;
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        uint32_t expressionType = parseArrayVarExpression(codeLine, codeLineIndex, arrText, arrIndex);

        int intSize = _integerVars[varIndex]._intSize;
        uint16_t arrayPtr = _integerVars[varIndex]._array;

        // Constant index
        if(!(expressionType & Expression::HasIntVars))
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + int16_t(std::lround(arrIndex._value))*uint16_t(intSize)), false, codeLineIndex);
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDW",  "register0", false, codeLineIndex);
            switch(codeLine._int16Byte)
            {
                case Expression::Int16Low:  emitVcpuAsm("POKE", "register1", false, codeLineIndex);                                                         break;
                case Expression::Int16High: emitVcpuAsm("INC",  "register1", false, codeLineIndex); emitVcpuAsm("POKE", "register1", false, codeLineIndex); break;
                case Expression::Int16Both: emitVcpuAsm("DOKE", "register1", false, codeLineIndex);                                                         break;

                default: break;
            }
        }
        else
        {
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
            emitVcpuAsm("ADDW", "register1", false, codeLineIndex);
            emitVcpuAsm("ADDW", "register1", false, codeLineIndex);
            emitVcpuAsm("STW",  "register1", false, codeLineIndex);
            emitVcpuAsm("LDW",  "register0", false, codeLineIndex);
            switch(codeLine._int16Byte)
            {
                case Expression::Int16Low:  emitVcpuAsm("POKE", "register1", false, codeLineIndex);                                                         break;
                case Expression::Int16High: emitVcpuAsm("INC",  "register1", false, codeLineIndex); emitVcpuAsm("POKE", "register1", false, codeLineIndex); break;
                case Expression::Int16Both: emitVcpuAsm("DOKE", "register1", false, codeLineIndex);                                                         break;

                default: break;
            }
        }

        return true;
    }

    void handleInt16Byte(const std::string& opcode, const std::string& operand, Expression::Numeric& numeric, bool nextTempVar)
    {
        switch(numeric._int16Byte)
        {
            case Expression::Int16Both: 
            {
                emitVcpuAsm(opcode, operand, nextTempVar);
                return;
            }
            break;

            case Expression::Int16Low:
            {
                getNextTempVar();
                if(opcode != "SUBW")
                {
                    emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(getTempVarStart())), false);
                    emitVcpuAsm("LD", operand, false);
                    emitVcpuAsm(opcode, Expression::byteToHexString(uint8_t(getTempVarStart())), nextTempVar);
                    return;
                }
                else
                {
                    std::string temp0 = Expression::byteToHexString(uint8_t(getTempVarStart()));
                    emitVcpuAsm("STW", temp0, false);
                    emitVcpuAsm("LD", operand, false);
                    getNextTempVar();
                    std::string temp1 = Expression::byteToHexString(uint8_t(getTempVarStart()));
                    emitVcpuAsm("STW", temp1, false);
                    emitVcpuAsm("LDW", temp0, false);
                    emitVcpuAsm("SUBW", temp1, nextTempVar);
                    return;
                }
            }
            break;

            case Expression::Int16High:
            {
                getNextTempVar();
                if(opcode != "SUBW")
                {
                    emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(getTempVarStart())), false);
                    emitVcpuAsm("LD", operand + " + 1", false);
                    emitVcpuAsm(opcode, Expression::byteToHexString(uint8_t(getTempVarStart())), nextTempVar);
                    return;
                }
                else
                {
                    std::string temp0 = Expression::byteToHexString(uint8_t(getTempVarStart()));
                    emitVcpuAsm("STW", temp0, false);
                    emitVcpuAsm("LD", operand + " + 1", false);
                    getNextTempVar();
                    std::string temp1 = Expression::byteToHexString(uint8_t(getTempVarStart()));
                    emitVcpuAsm("STW", temp1, false);
                    emitVcpuAsm("LDW", temp0, false);
                    emitVcpuAsm("SUBW", temp1, nextTempVar);
                    return;
                }
            }
            break;

            default: break;
        }
    }

    bool emitVcpuAsmUserVar(const std::string& opcodeStr, Expression::Numeric& numeric, bool nextTempVar)
    {
        std::string opcode = std::string(opcodeStr);
        std::string varName = std::string(numeric._name);
        int varIndex = findVar(varName);
        if(varIndex == -1)
        {
            fprintf(stderr, "Compiler::emitVcpuAsmUserVar() : couldn't find variable name '%s'\n", varName.c_str());
            return false;
        }

        std::string operand = "_" + _integerVars[varIndex]._name;
        if(opcode == "LDW")
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Both: opcode = "LDW";                    break;
                case Expression::Int16Low:  opcode = "LD";                     break;
                case Expression::Int16High: opcode = "LD";  operand += " + 1"; break;

                default: break;
            }

            emitVcpuAsm(opcode, operand, nextTempVar);
            return true;
        }

        // ADDW, SUBW, ANDW, ORW, XORW
        handleInt16Byte(opcode, operand, numeric, nextTempVar);
        return true;
    }

    // Generic expression parser
    OperandType parseExpression(int codeLineIndex, std::string& expression, std::string& operand, Expression::Numeric& numeric)
    {
        int varIndex, constIndex, strIndex;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);

        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  
            (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
        {
            operand = Expression::byteToHexString(uint8_t(_tempVarStart));
            return OperandTemp;
        }
        else if(expressionType & Expression::HasIntVars)
        {
            operand = _integerVars[varIndex]._name;
            return OperandVar;
        }

        operand = std::to_string(int16_t(std::lround(numeric._value)));
        return OperandConst;
    }

    // LDW expression parser
    uint32_t parseExpression(int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        int varIndex, constIndex, strIndex;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);

        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||
            (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasIntVars)
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;
                case Expression::Int16High: emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name + " + 1", false, codeLineIndex); break;
                case Expression::Int16Both: emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;

                default: break;
            }
        }
        else
        {
            int16_t value = int16_t(std::lround(numeric._value));
            (value >= 0  &&  value <= 255) ? emitVcpuAsm("LDI", std::to_string(value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(value), false, codeLineIndex);
        }

        return expressionType;
    }

    // Handle expression, (use this when expression has already been parsed)
    uint32_t handleExpression(int codeLineIndex, std::string& expression, Expression::Numeric numeric)
    {
        int varIndex, constIndex, strIndex;
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);

        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||
            (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasIntVars)
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;
                case Expression::Int16High: emitVcpuAsm("LD",  "_" + _integerVars[varIndex]._name + " + 1", false, codeLineIndex); break;
                case Expression::Int16Both: emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name,          false, codeLineIndex); break;

                default: break;
            }
        }
        else
        {
            int16_t value = int16_t(std::lround(numeric._value));
            (value >= 0  &&  value <= 255) ? emitVcpuAsm("LDI", std::to_string(value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(value), false, codeLineIndex);
        }

        return expressionType;
    }

    bool isGosubLabel(const std::string& label)
    {
        for(int i=0; i<int(_gosubLabels.size()); i++)
        {
            if(_gosubLabels[i] == label) return true;
        }

        return false;
    }

    bool checkForGosubLabel(const std::string& code, int lineNumber)
    {
        std::vector<std::string> tokens = Expression::tokeniseLine(code, " :=");
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Expression::strToUpper(tokens[i]) == "GOSUB")
            {
                if(i+1 >= int(tokens.size()))
                {
                    fprintf(stderr, "Compiler::checkForGosubLabel() : missing label after GOSUB in '%s' on line %d\n", code.c_str(), lineNumber + 1);
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
        if(code.size() > 1  &&  isdigit((unsigned char)code[0]))
        {
            size_t space = code.find_first_of(" \n\r\f\t\v,");
            if(space == std::string::npos) space = code.size() - 1;

            // Force space between line numbers and line
            for(size_t i=1; i<space; i++)
            {
                if(!isdigit((unsigned char)code[i])  &&  code[i] != ':'  &&  code[i] != '!')
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
            bool numeric = false;
            bool foundGosub = false;
            std::string labelName = code.substr(0, space);
            size_t colon = labelName.find(':');
            size_t exclamation = labelName.find('!');

            if(colon != std::string::npos)
            {
                numeric = true;
                foundGosub = true;
                labelName.erase(colon, 1);
            }
            else if(exclamation != std::string::npos)
            {
                numeric = true;
                foundGosub = false;
                labelName.erase(exclamation, 1);
            }
            else
            {
                foundGosub = isGosubLabel(labelName);
            }
            createLabel(_vasmPC, labelName, int(_codeLines.size()), label, numeric, true, false, foundGosub);
            if(createCodeLine(code, int(space + 1), _currentLabelIndex, -1, Expression::Int16Both, false, codeLine)) _codeLines.push_back(codeLine);

            return LabelFound;
        }

        // Text label
        static int nonNumericLabelIndex = -1;
        size_t colon = code.find_first_of(":");
        if(colon != std::string::npos)
        {
            std::string labelName = code.substr(0, colon);
            bool validLabel = true;
            if(labelName.size())
            {
                for(int i=0; i<int(labelName.size()); i++)
                {
                    if(!(labelName[i] == '_')  &&  !isalnum((unsigned char)labelName[i]))
                    {
                        validLabel = false;
                        break;
                    }
                }
            }
            if(validLabel)
            {
                bool validCode = false;

                // Create label
                bool foundGosub = isGosubLabel(labelName);
                createLabel(_vasmPC, labelName, int(_codeLines.size()), label, false, true, false, foundGosub);

                // Check for label with code
                if(code.size() > colon + 1)
                {
                    std::string labelCode = code.substr(colon  + 1);
                    Expression::stripWhitespace(labelCode);
                    if(labelCode.size() > 2  &&  createCodeLine(code, int(colon  + 1), _currentLabelIndex, -1, Expression::Int16Both, false, codeLine))
                    {
                        validCode = true;
                        _codeLines.push_back(codeLine);
                    }
                }

                // Check for label without code
                if(!validCode)
                {
                    nonNumericLabelIndex = _currentLabelIndex;
                }

                return LabelFound;
            }
        }

        // Non label code, (except if previous line had a non numeric label without code)
        if(createCodeLine(code, 0, nonNumericLabelIndex, -1, Expression::Int16Both, false, codeLine))
        {
            // Don't reset nonNumericLabelIndex until it has been assigned to a valid line
            if(codeLine._code.size() >= 2) nonNumericLabelIndex = -1;
            _codeLines.push_back(codeLine);
        }

        return LabelNotFound;
    }

    bool parsePragmas(int numLines)
    {
        // By default do not support CALLI
        Assembler::setUseOpcodeCALLI(false);

        // Parse each line of input for pragmas
        for(int j=0; j<numLines; j++)
        {
            std::vector<size_t> offsets;
            std::string input = _input[j];
            input = Expression::removeCommentsNotInStrings(input);
            Expression::strToUpper(input);
            Keywords::KeywordResult keywordResult = Keywords::handlePragmas(input, j);
            switch(keywordResult)
            {
                case Keywords::KeywordFound:
                {
                    _input[j].erase(0);
                }
                break;

                case Keywords::KeywordError:
                {
                    fprintf(stderr, "Compiler::parsePragmas() : Syntax error in Pragma, in %s on line %d\n", _input[j].c_str(), j + 1);
                    return false;
                }
                break;

                default: break;
            }
        }

        return true;
    }

    bool parseLabels(int numLines)
    {
        // Relies on _useOpcodeCALLI_, so make sure it is already initialised
        initialiseMacros();

        // Entry point initialisation
        Label label;
        CodeLine codeLine;
        createLabel(_vasmPC, "_entryPoint_", 0, label, false, false, false, false);
        if(createCodeLine("INIT", 0, 0, -1, Expression::Int16Both, false, codeLine)) _codeLines.push_back(codeLine);
        if(!Assembler::getUseOpcodeCALLI())
        {
            // Handles time sliced, (real time), code such as AUDIO and/or MIDI
            emitVcpuAsm("%InitRealTimeProc", "", false, 0);

            // Handle relational operators
            emitVcpuAsm("%InitEqOp", "", false, 0);
            emitVcpuAsm("%InitNeOp", "", false, 0);
            emitVcpuAsm("%InitLeOp", "", false, 0);
            emitVcpuAsm("%InitGeOp", "", false, 0);
            emitVcpuAsm("%InitLtOp", "", false, 0);
            emitVcpuAsm("%InitGtOp", "", false, 0);
        }
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

                default: break;
            }
        }

        return true;
    }

    // Create string and advance string pointer
    int getOrCreateString(CodeLine& codeLine, int codeLineIndex, const std::string& str, std::string& name, uint16_t& address, uint8_t maxSize, bool constString)
    {
        UNREFERENCED_PARAM(codeLine);
        UNREFERENCED_PARAM(codeLineIndex);

        int index = -1;

        // Reuse const string if possible
        if(constString)
        {
            for(int j=0; j<int(_stringVars.size()); j++)
            {
                if(_stringVars[j]._text == str) 
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
                if(!Memory::getFreeRAM(Memory::FitDescending, int(str.size()) + 2, 0x0200, _runtimeStart, address))
                {
                    fprintf(stderr, "Compiler::getOrCreateString() : Not enough RAM for string %s='%s' of size %d\n", name.c_str(), str.c_str(), int(str.size()));
                    return -1;
                }

                // Save end of runtime/strings
                if(address < _runtimeEnd) _runtimeEnd = address;

                name = "str_" + Expression::wordToHexString(address);
                StringVar stringVar = {uint8_t(str.size()), uint8_t(str.size()), address, str, name, "_" + name + std::string(LABEL_TRUNC_SIZE - name.size() - 1, ' '), -1, true};
                _stringVars.push_back(stringVar);
                index = int(_stringVars.size()) - 1;
            }
        }
        // Variable strings
        else
        {
            // Allocate string
            if(!Memory::getFreeRAM(Memory::FitDescending, maxSize + 2, 0x0200, _runtimeStart, address))
            {
                fprintf(stderr, "Compiler::getOrCreateString() : Not enough RAM for string %s='%s' of size %d\n", name.c_str(), str.c_str(), maxSize + 2);
                return -1;
            }

            // Save end of runtime/strings
            if(address < _runtimeEnd) _runtimeEnd = address;

            StringVar stringVar = {uint8_t(str.size()), maxSize, address, str, name, "_" + name + std::string(LABEL_TRUNC_SIZE - name.size() - 1, ' '), -1, false};
            _stringVars.push_back(stringVar);
            index = int(_stringVars.size()) - 1;
        }

        return index;
    }

    // Create constant string
    uint16_t getOrCreateConstString(const std::string& input, int& index)
    {
        std::string output = input;

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, output, name, address);
        return address;
    }

    // Create constant string from int
    uint16_t getOrCreateConstString(ConstStrType constStrType, int16_t input, int& index)
    {
        char output[16] = "";
        switch(constStrType)
        {
            case StrChar:  sprintf(output, "%c",   uint8_t(input) & 0x7F); break;
            case StrHex:   sprintf(output, "%02X", uint8_t(input));        break;
            case StrHexw:  sprintf(output, "%04X", uint16_t(input));       break;

            default: break;
        }

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, std::string(output), name, address);
        return address;
    }

    // Create constant sub-string
    uint16_t getOrCreateConstString(ConstStrType constStrType, const std::string& input, int8_t length, uint8_t offset, int& index)
    {
        std::string output;
        switch(constStrType)
        {
            case StrLeft:  output = input.substr(0, length);             break;
            case StrRight: output = input.substr(input.size() - length); break;
            case StrMid:   output = input.substr(offset, length);        break;

            default: break;
        }

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, output, name, address);
        return address;
    }


    // ********************************************************************************************
    // Recursive Descent Parser helpers
    // ********************************************************************************************
    char peek(bool skipSpaces)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(skipSpaces  &&  Expression::peek() == ' ')
        {
            if(!Expression::advance(1)) return 0;
        }

        return Expression::peek();
    }

    char get(bool skipSpaces)
    {
        // Skipping spaces can attach hex numbers to variables, keywords, etc
        while(skipSpaces  &&  Expression::peek() == ' ')
        {
            if(!Expression::advance(1)) return 0;
        }

        return Expression::get();
    }

    bool number(double& value)
    {
        char uchr;

        bool isDouble = false;
        std::string valueStr;
        uchr = char(toupper((unsigned char)peek(true)));
        valueStr.push_back(uchr); get(true);
        uchr = char(toupper((unsigned char)peek(true)));

        if((uchr >= '0'  &&  uchr <= '9')  ||  uchr == 'X'  ||  uchr == 'H'  ||  uchr == 'B'  ||  uchr == 'O'  ||  uchr == 'Q'  ||  uchr == '.')
        {
            if(uchr == '.') isDouble = true;

            valueStr.push_back(uchr); get(true);
            uchr = char(toupper((unsigned char)peek(true)));
            while(uchr  &&  ((uchr >= '0'  &&  uchr <= '9')  ||  (uchr >= 'A'  &&  uchr <= 'F')  ||  uchr == '.'))
            {
                if(uchr == '.')
                {
                    // Check for multiple periods
                    if(isDouble == true) return false;
                    isDouble = true;
                }

                // Don't skip spaces here, as hex numbers can become attached to variables, keywords, etc
                valueStr.push_back(get(false));
                uchr = char(toupper((unsigned char)peek(false)));
            }
        }

        if(!isDouble)
        {
            int16_t ivalue;
            bool success = Expression::stringToI16(valueStr, ivalue);
            value = double(ivalue);
            return success;
        }

        Expression::stringToDouble(valueStr, value);
        return true;
    }

    Expression::Numeric getString(const Expression::Numeric& numeric)
    {
        UNREFERENCED_PARAM(numeric);

        // First quote
        get(true);

        // Don't skip spaces within string
        std::string text;
        while(peek(false)  &&  peek(false) != '"')
        {
            text += get(false);
        }

        if(!peek(true))
        {
            fprintf(stderr, "Compiler::getString() : Syntax error in string '%s' in '%s' on line %d\n", text.c_str(), _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber());
            //_PAUSE_;
            _compilingError = true;
            return Expression::Numeric();
        }

        // Last quote
        get(true);

        return Expression::Numeric(0, -1, true, Expression::String, Expression::BooleanCC, Expression::Int16Both, std::string(""), text);
    }

    Expression::Numeric addressOf(void)
    {
        // @ char
        get(true);

        std::string varName = Expression::getExpression();
        if(varName.back() == ')') varName.erase(varName.size()-1);
        int varIndex = findVar(varName);
        int strIndex = findStr(varName);
        int constIndex = findConst(varName);

        // Int and array vars
        if(varIndex != -1)
        {
            Expression::advance(varName.size());
            uint16_t address = (_integerVars[varIndex]._varType == VarArray) ? _integerVars[varIndex]._array : _integerVars[varIndex]._address;
            return Expression::Numeric(address, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        // Strings
        if(strIndex != -1)
        {
            Expression::advance(varName.size());
            uint16_t address = _stringVars[strIndex]._address;
            return Expression::Numeric(address, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }
        
        // Constants
        if(constIndex != -1)
        {
            Expression::advance(varName.size());
            uint16_t address = _constants[constIndex]._address;
            return Expression::Numeric(address, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        fprintf(stderr, "Compiler::factor() : Syntax error in address of '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber());
        //_PAUSE_;
        _compilingError = true;
        return Expression::Numeric();
    }

    Expression::Numeric factor(int16_t defaultValue)
    {
        double value = 0;
        Expression::Numeric numeric;

        if(peek(true) == '(')
        {
            get(true);
            numeric = expression();
            
            // Parameters
            while(peek(true)  &&  peek(true) != ')')
            {
                if(get(true) == ',') numeric._parameters.push_back(expression());
            }

            if(peek(true) != ')')
            {
                fprintf(stderr, "Compiler::factor() : Found '%c' : expecting ')' in '%s' on line %d\n", peek(true), Expression::getExpressionToParse(), Expression::getLineNumber());
                //_PAUSE_;
                _compilingError = true;
                numeric = Expression::Numeric();
            }
            get(true);
        }
        else if((peek(true) >= '0'  &&  peek(true) <= '9')  ||  peek(true) == '&')
        {
            // Number
            if(number(value))
            {
                numeric = Expression::Numeric(value, -1, true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
            }
            else
            {
                fprintf(stderr, "Compiler::factor() : Syntax error in number '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber());
                //_PAUSE_;
                _compilingError = true;
                numeric = Expression::Numeric();
            }
        }
        // String
        else if(peek(true) == '"')
        {
            numeric = getString(numeric);
        }
        // 'Address of' operator
        else if(peek(true) == '@')
        {
            numeric = addressOf();
        }
        // Functions
        else if(Expression::find("LEN"))
        {
            numeric = factor(0); numeric = Keywords::functionLEN(numeric);
        }
        else if(Expression::find("CHR$"))
        {
            numeric = factor(0); numeric = Keywords::functionCHR$(numeric);
        }
        else if(Expression::find("HEX$"))
        {
            numeric = factor(0); numeric = Keywords::functionHEX$(numeric);
        }
        else if(Expression::find("HEXW$"))
        {
            numeric = factor(0); numeric = Keywords::functionHEXW$(numeric);
        }
        else if(Expression::find("LEFT$"))
        {
            numeric = factor(0); numeric = Keywords::functionLEFT$(numeric);
        }
        else if(Expression::find("RIGHT$"))
        {
            numeric = factor(0); numeric = Keywords::functionRIGHT$(numeric);
        }
        else if(Expression::find("MID$"))
        {
            numeric = factor(0); numeric = Keywords::functionMID$(numeric);
        }
        else if(Expression::find("PEEK"))
        {
            numeric = factor(0); numeric = Keywords::functionPEEK(numeric);
        }
        else if(Expression::find("DEEK"))
        {
            numeric = factor(0); numeric = Keywords::functionDEEK(numeric);
        }
        else if(Expression::find("USR"))
        {
            numeric = factor(0); numeric = Keywords::functionUSR(numeric);
        }
        else if(Expression::find("RND"))
        {
            numeric = factor(0); numeric = Keywords::functionRND(numeric);
        }
        else if(Expression::find("NOT "))
        {
            numeric = factor(0); numeric = Operators::operatorNOT(numeric);
        }
        else if(Expression::find("SIN"))
        {
            numeric = factor(0); numeric = Operators::operatorSIN(numeric);
        }
        else if(Expression::find("COS"))
        {
            numeric = factor(0); numeric = Operators::operatorCOS(numeric);
        }
        else if(Expression::find("TAN"))
        {
            numeric = factor(0); numeric = Operators::operatorTAN(numeric);
        }
        else if(Expression::find("ASIN"))
        {
            numeric = factor(0); numeric = Operators::operatorASIN(numeric);
        }
        else if(Expression::find("ACOS"))
        {
            numeric = factor(0); numeric = Operators::operatorACOS(numeric);
        }
        else if(Expression::find("ATAN"))
        {
            numeric = factor(0); numeric = Operators::operatorATAN(numeric);
        }
        else if(Expression::find("RAND"))
        {
            numeric = factor(0); numeric = Operators::operatorRAND(numeric);
        }
        else if(Expression::find("REV16"))
        {
            numeric = factor(0); numeric = Operators::operatorREV16(numeric);
        }
        else if(Expression::find("REV8"))
        {
            numeric = factor(0); numeric = Operators::operatorREV8(numeric);
        }
        else if(Expression::find("REV4"))
        {
            numeric = factor(0); numeric = Operators::operatorREV4(numeric);
        }
        else
        {
            switch(peek(true))
            {
                // Unary operators
                case '+': get(true); numeric = factor(0); numeric = Operators::operatorPOS(numeric); break;
                case '-': get(true); numeric = factor(0); numeric = Operators::operatorNEG(numeric); break;

                // Reached end of expression
                case 0: numeric = Expression::Numeric(defaultValue, -1, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string("")); break;

                default:
                {
                    // Variables
                    std::string varName = Expression::getExpression();
                    int varIndex = findVar(varName);
                    int strIndex = findStr(varName);
                    int constIndex = findConst(varName);
                    if(varIndex != -1)
                    {
                        Expression::Int16Byte int16Byte = Expression::Int16Both;

                        Expression::advance(varName.size());

                        // Arrays
                        if(_integerVars[varIndex]._varType == VarArray)
                        {
                            // Array numeric
                            numeric = Expression::Numeric(defaultValue, int16_t(varIndex), true, Expression::ArrVar, Expression::BooleanCC, Expression::Int16Both, varName, std::string(""));

                            // Array index numeric
                            Expression::Numeric param = factor(0); 
                            numeric._parameters.push_back(param);

                            // Read both, low or high bytes, (DEEK <X>, PEEK <X>, PEEK <X+1>)
                            if(Expression::find(".LO")) int16Byte = Expression::Int16Low;
                            if(Expression::find(".HI")) int16Byte = Expression::Int16High;
                            numeric._int16Byte = int16Byte;

                            numeric = Keywords::functionARR(numeric);
                        }
                        // Vars
                        else
                        {
                            // Read both, low or high bytes, (LDW <X>, LD <X>, LD <X+1>)
                            if(Expression::find(".LO")) int16Byte = Expression::Int16Low;
                            if(Expression::find(".HI")) int16Byte = Expression::Int16High;

                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            numeric = Expression::Numeric(defaultValue, int16_t(varIndex), true, Expression::IntVar, Expression::BooleanCC, int16Byte, varName, std::string(""));
                        }
                    }
                    // Strings
                    else if(strIndex != -1)
                    {
                        Expression::advance(varName.size());

                        // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                        numeric = Expression::Numeric(defaultValue, int16_t(strIndex), true, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, varName, _stringVars[strIndex]._text);
                    }
                    // Constants
                    else if(constIndex != -1)
                    {
                        Expression::advance(varName.size());
                        
                        switch(_constants[constIndex]._constType)
                        {
                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            case ConstInt16:
                            {
                                numeric = Expression::Numeric(_constants[constIndex]._data, int16_t(constIndex), true, Expression::Number, Expression::BooleanCC, Expression::Int16Both, varName, std::string(""));
                            }
                            break;

                            case ConstStr:
                            {
                                numeric = Expression::Numeric(defaultValue, int16_t(constIndex), true, Expression::Constant, Expression::BooleanCC, Expression::Int16Both, varName, _constants[constIndex]._text);
                            }
                            break;

                            default: break;
                        }
                    }
                    // Unknown symbol
                    else
                    {
                        numeric = Expression::Numeric(defaultValue, -1, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));

                        // Pragma parsing happens before any code has been parsed, so _codeLines[] may be empty
                        std::string codeText = (int(_codeLines.size()) > _currentCodeLineIndex) ? _codeLines[_currentCodeLineIndex]._code : "PRAGMA";
                        if(varName.size())
                        {
                            fprintf(stderr, "\nCompiler::factor() : Found an unknown symbol '%s' : in '%s' on line %d\n\n", varName.c_str(), codeText.c_str(), Expression::getLineNumber());
                        }
                        else
                        {
                            fprintf(stderr, "\nCompiler::factor() : Found an unknown symbol in '%s' on line %d\n\n", codeText.c_str(), Expression::getLineNumber());
                        }

                        //_PAUSE_;
                        _compilingError = true;
                    }
                }
                break;
            }
        }

        return numeric;
    }

    Expression::Numeric term(void)
    {
        Expression::Numeric numeric, result = factor(0);

        for(;;)
        {
            if(Expression::find("**"))       {           numeric = factor(0); result = Operators::operatorPOW(result, numeric);}
            else if(peek(true) == '*')       {get(true); numeric = factor(0); result = Operators::operatorMUL(result, numeric);}
            else if(peek(true) == '/')       {get(true); numeric = factor(0); result = Operators::operatorDIV(result, numeric);}
            else if(peek(true) == '%')       {get(true); numeric = factor(0); result = Operators::operatorMOD(result, numeric);}
            else if(Expression::find("MOD")) {           numeric = factor(0); result = Operators::operatorMOD(result, numeric);}
            else return result;
        }
    }

    Expression::Numeric expr(void)
    {
        Expression::Numeric numeric, result = term();

        for(;;)
        {
            if(peek(true) == '+')      {get(true); numeric = term(); result = Operators::operatorADD(result, numeric);}
            else if(peek(true) == '-') {get(true); numeric = term(); result = Operators::operatorSUB(result, numeric);}

            else return result;
        }
    }

    Expression::Numeric logical(void)
    {
        Expression::Numeric numeric, result = expr();

        for(;;)
        {
            if(Expression::find("AND"))      {numeric = expr(); result = Operators::operatorAND(result, numeric);}
            else if(Expression::find("XOR")) {numeric = expr(); result = Operators::operatorXOR(result, numeric);}
            else if(Expression::find("OR"))  {numeric = expr(); result = Operators::operatorOR(result,  numeric);}
            else if(Expression::find("LSL")) {numeric = expr(); result = Operators::operatorLSL(result, numeric);}
            else if(Expression::find("LSR")) {numeric = expr(); result = Operators::operatorLSR(result, numeric);}
            else if(Expression::find("ASR")) {numeric = expr(); result = Operators::operatorASR(result, numeric);}
            else if(Expression::find("<<"))  {numeric = expr(); result = Operators::operatorLSL(result, numeric);}
            else if(Expression::find(">>"))  {numeric = expr(); result = Operators::operatorLSR(result, numeric);}
            else if(Expression::find("&>>")) {numeric = expr(); result = Operators::operatorASR(result, numeric);}

            else return result;
        }
    }

    Expression::Numeric expression(void)
    {
        Expression::Numeric numeric, result = logical();

        for(;;)
        {
            // Boolean conditionals
            if(Expression::find("=="))      {           numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::BooleanCC);}
            else if(peek(true) == '=')      {get(true); numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::BooleanCC);}
            else if(Expression::find("<>")) {           numeric = logical(); result = Operators::operatorNE(result, numeric, Expression::BooleanCC);}
            else if(Expression::find("<=")) {           numeric = logical(); result = Operators::operatorLE(result, numeric, Expression::BooleanCC);}
            else if(Expression::find(">=")) {           numeric = logical(); result = Operators::operatorGE(result, numeric, Expression::BooleanCC);}
            else if(peek(true) == '<')      {get(true); numeric = logical(); result = Operators::operatorLT(result, numeric, Expression::BooleanCC);}
            else if(peek(true) == '>')      {get(true); numeric = logical(); result = Operators::operatorGT(result, numeric, Expression::BooleanCC);}

            // Normal conditionals
            else if(Expression::find("&==")) {numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&="))  {numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&<>")) {numeric = logical(); result = Operators::operatorNE(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&<=")) {numeric = logical(); result = Operators::operatorLE(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&>=")) {numeric = logical(); result = Operators::operatorGE(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&<"))  {numeric = logical(); result = Operators::operatorLT(result, numeric, Expression::NormalCC);}
            else if(Expression::find("&>"))  {numeric = logical(); result = Operators::operatorGT(result, numeric, Expression::NormalCC);}

            // Fast conditionals
            else if(Expression::find("&&==")) {numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&="))  {numeric = logical(); result = Operators::operatorEQ(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&<>")) {numeric = logical(); result = Operators::operatorNE(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&<=")) {numeric = logical(); result = Operators::operatorLE(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&>=")) {numeric = logical(); result = Operators::operatorGE(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&<"))  {numeric = logical(); result = Operators::operatorLT(result, numeric, Expression::FastCC);}
            else if(Expression::find("&&>"))  {numeric = logical(); result = Operators::operatorGT(result, numeric, Expression::FastCC);}

            else return result;
        }
    }

    uint16_t getStringSrcAddr(Expression::Numeric& numeric)
    {
        uint16_t srcAddr = 0x0000;

        // String assignment from literal
        if(numeric._index == -1  &&  numeric._varType == Expression::String)
        {
            int index;
            getOrCreateConstString(numeric._text, index);
            if(index != -1) srcAddr = getStringVars()[index]._address;
        }
        // String assignment from var or const
        else if(numeric._index != -1)
        {
            switch(numeric._varType)
            {
                case Expression::Constant: srcAddr = getConstants()[numeric._index]._address;  break;
                case Expression::StrVar:   srcAddr = getStringVars()[numeric._index]._address; break;

                default: break;
            }
        }

        return srcAddr;
    }

    uint16_t getStringSrcAddr(std::string& strText)
    {
        uint16_t srcAddr = 0x0000;

        if(strText.back() == '$'  &&  Expression::isVarNameValid(strText))
        {
            int index = findStr(strText);
            if(index != -1)
            {
                srcAddr = getStringVars()[index]._address;
            }
            else
            {
                index = findConst(strText);
                if(index != -1)
                {
                    srcAddr = getConstants()[index]._address;
                }
            }
        }
        else if(Expression::isValidString(strText))
        {
            // Strip quotes
            strText.erase(0, 1);
            strText.erase(strText.size()-1, 1);

            int index;
            getOrCreateConstString(strText, index);
            if(index != -1) srcAddr = getStringVars()[index]._address;
        }

        return srcAddr;
    }

    bool handleStrings(CodeLine& codeLine, int codeLineIndex, Expression::Numeric& numeric, uint32_t expressionType)
    {
        if(codeLine._text.size() < 2) return false;

        int dstIndex = codeLine._varIndex;
        if(dstIndex == -1)
        {
            fprintf(stderr, "Compiler::handleStrings() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            //_PAUSE_;
            _compilingError = true;
            return false;
        }

        bool isStrExpression = ((expressionType >= Expression::HasStrings)  &&  (expressionType <= Expression::IsStringExpression));
        if(isStrExpression)
        {
            std::vector<std::string> tokens = Expression::tokenise(codeLine._expression, "+", false);
            if(tokens.size() == 1)
            {
                // String assignment, from var or const or literal
                uint16_t srcAddr = getStringSrcAddr(numeric);
                if(srcAddr == 0x0000)
                {
                    fprintf(stderr, "Compiler::handleStrings() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    //_PAUSE_;
                    _compilingError = true;
                    return false;
                }

                // No need to copy if src and dst are equal
                uint16_t dstAddr = _stringVars[dstIndex]._address;
                if(srcAddr != dstAddr)
                {
                    emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false, codeLineIndex);
                    emitVcpuAsm("STW", "strSrcAddr", false, codeLineIndex);
                    emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false, codeLineIndex);
                    emitVcpuAsm("STW", "strDstAddr", false, codeLineIndex);
                    emitVcpuAsm("%StringCopy", "", false, codeLineIndex);
                }

                return true;
            }
            // String concatenation
            else
            {
                if(tokens.size() < 2)
                {
                    fprintf(stderr, "Compiler::handleStrings() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    //_PAUSE_;
                    _compilingError = true;
                    return false;
                }

                // Source string addresses, (extra 0x0000 delimiter used by VASM runtime)
                std::vector<uint16_t> strAddrs(tokens.size() + 1, 0x0000);
                for(int i=0; i<int(tokens.size()); i++)
                {
                    Expression::stripNonStringWhitespace(tokens[i]);
                    strAddrs[i] = getStringSrcAddr(tokens[i]);
                    if(strAddrs[i] == 0x0000)
                    {
                        fprintf(stderr, "Compiler::handleStrings() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                        //_PAUSE_;
                        _compilingError = true;
                        return false;
                    }
                }

                // Source string addresses LUT
                uint16_t lutAddress;
                if(!Memory::getFreeRAM(Memory::FitDescending, int(strAddrs.size()*2), 0x0200, _runtimeStart, lutAddress))
                {
                    fprintf(stderr, "Compiler::handleStrings() : Not enough RAM for string concatenation LUT of size %d\n", int(strAddrs.size()));
                    return false;
                }
                _codeLines[codeLineIndex]._strConcatLut = {lutAddress, strAddrs}; // save LUT in global codeLine not local copy

                // Destination address
                uint16_t dstAddr = _stringVars[dstIndex]._address;

                // TODO: fix this bodge, (this is used by the Optimiser to remove a spurious STW)
                emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(getTempVarStart())), false, codeLineIndex);

                // Concatenate multiple source strings to string work area
                emitVcpuAsm("LDWI", Expression::wordToHexString(lutAddress), false, codeLineIndex);
                emitVcpuAsm("STW", "strLutAddr", false, codeLineIndex);
                emitVcpuAsm("LDWI", Expression::wordToHexString(getStrWorkArea()), false, codeLineIndex);
                emitVcpuAsm("STW", "strDstAddr", false, codeLineIndex);
                emitVcpuAsm("%StringConcat", "", false, codeLineIndex);

                // Copy work area to dst string
                emitVcpuAsm("LDWI", Expression::wordToHexString(getStrWorkArea()), false, codeLineIndex);
                emitVcpuAsm("STW", "strSrcAddr", false, codeLineIndex);
                emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false, codeLineIndex);
                emitVcpuAsm("STW", "strDstAddr", false, codeLineIndex);
                emitVcpuAsm("%StringCopy", "", false, codeLineIndex);

                return true;
            }
        }

        return false;
    }

    StatementResult createVasmCode(CodeLine& codeLine, int codeLineIndex)
    {
        // Check for subroutine start, make sure PUSH is emitted only once, even for multi-statement lines, (codeLine is a local copy of each statement within a multi-statement codeLine)
        if(!_codeLines[_currentCodeLineIndex]._pushEmitted  &&  codeLine._labelIndex >= 0  &&  _labels[codeLine._labelIndex]._gosub)
        {
            _codeLines[_currentCodeLineIndex]._pushEmitted = true;
            emitVcpuAsm("PUSH", "", false, codeLineIndex);
        }

        // Specific parsing requirements for most keywords, (*NOT* functions), some keywords like IF will also parse multi-statements
        for(int i=0; i<int(codeLine._tokens.size()); i++)
        {
            Keywords::KeywordFuncResult result;
            Keywords::KeywordResult keywordResult = Keywords::handleKeywords(codeLine, codeLine._tokens[i], codeLineIndex, i, result);
            if(keywordResult == Keywords::KeywordError) return StatementError;

            // Search for keyword, if found return it's statement type result
            std::string token = codeLine._tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getKeywords().find(token) != Keywords::getKeywords().end()) return Keywords::getKeywords()[token]._result;
        }

        int varIndexRhs = -1, constIndexRhs = -1, strIndexRhs = -1;
        uint32_t expressionType = isExpression(codeLine._expression, varIndexRhs, constIndexRhs, strIndexRhs);
        if(expressionType == Expression::IsInvalid) return StatementError;

        // Parse expression, handles ints, strings, operators and functions
        bool isStringVar = false;
        Expression::Numeric numeric;
        if(codeLine._varIndex != -1)
        {
            std::string name;
            Expression::VarType varType = Expression::Number;
            switch(codeLine._varType)
            {
                case VarInt16: varType = Expression::IntVar; name = _integerVars[codeLine._varIndex]._name;                     break;
                case VarStr:   varType = Expression::StrVar; name = _stringVars[codeLine._varIndex]._name;  isStringVar = true; break;

                default: break;
            }
            
            // Output variable, (functions can access this variable within parse())
            numeric = Expression::Numeric(0, int16_t(codeLine._varIndex), true, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }
        Expression::parse(codeLine._expression, codeLineIndex, numeric);

        // String assignments and operators, (exit early for a string result)
        bool stringResult = handleStrings(codeLine, codeLineIndex, numeric, expressionType);
        if(stringResult  ||  isStringVar) return StringStatementParsed;

        // Update result variable
        if(codeLine._varIndex != -1)
        {
            // Expression that contains one or more int vars
            bool intVarAssignment = (expressionType & Expression::HasIntVars);
            updateVar(int16_t(std::lround(numeric._value)), codeLine, codeLine._varIndex, intVarAssignment);
        }

        // TODO: only works with Int16, fix for all var types
        // Variable assignment
        if(codeLine._varIndex != -1)
        {
            // Assignment with a var expression
            if(codeLine._containsVars)
            {
                // Try and optimise LDW away if possible
                if(varIndexRhs >= 0  &&  _integerVars[varIndexRhs]._varType != VarArray  &&  !(expressionType & Expression::HasOperators)  &&  !(expressionType & Expression::HasFunctions))
                {
                    switch(numeric._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("LD",  "_" + _integerVars[varIndexRhs]._name,          false, codeLineIndex); break;
                        case Expression::Int16High: emitVcpuAsm("LD",  "_" + _integerVars[varIndexRhs]._name + " + 1", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("LDW", "_" + _integerVars[varIndexRhs]._name,          false, codeLineIndex); break;

                        default: break;
                    }
                }

                if(_integerVars[codeLine._varIndex]._varType == VarArray)
                {
                    writeArrayVar(codeLine, codeLineIndex, codeLine._varIndex);
                }
                else
                {
                    switch(codeLine._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("ST",  "_" + _integerVars[codeLine._varIndex]._name,          false, codeLineIndex); break;
                        case Expression::Int16High: emitVcpuAsm("ST",  "_" + _integerVars[codeLine._varIndex]._name + " + 1", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name,          false, codeLineIndex); break;

                        default: break;
                    }
                }
            }
            // Standard assignment
            else
            {
                // Skip for functions unless function parameter was a literal
                if(!(expressionType & Expression::HasFunctions)  ||  numeric._varType == Expression::Number)
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
                    switch(codeLine._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("ST",  "_" + _integerVars[codeLine._varIndex]._name,          false, codeLineIndex); break;
                        case Expression::Int16High: emitVcpuAsm("ST",  "_" + _integerVars[codeLine._varIndex]._name + " + 1", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("STW", "_" + _integerVars[codeLine._varIndex]._name,          false, codeLineIndex); break;

                        default: break;
                    }
                }
            }
        }

        return StatementExpression;
    }

    StatementResult parseMultiStatements(const std::string& code, CodeLine& codeLine, int codeLineIndex, int& varIndex, int& strIndex)
    {
        // Make a local copy, otherwise original tokens are destroyed
        CodeLine codeline = codeLine;

        // Tokenise and parse multi-statement lines
        StatementResult statementResult = StatementSuccess;
        std::vector<std::string> tokens = Expression::tokenise(code, ':', false);
        for(int j=0; j<int(tokens.size()); j++)
        {
            createCodeLine(tokens[j], 0, _codeLines[codeLineIndex]._labelIndex, -1, Expression::Int16Both, false, codeline);
            createCodeVar(codeline, codeLineIndex, varIndex);
            createCodeStr(codeline, codeLineIndex, strIndex);
            statementResult = createVasmCode(codeline, codeLineIndex);
            if(statementResult == StatementError) break;

            // Some commands, (such as IF), process multi-statements themselves
            if(statementResult == MultiStatementParsed) break;
        }

        return statementResult;
    }

    void addLabelToJumpCC(std::vector<VasmLine>& vasm, const std::string& label)
    {
        for(int i=0; i<int(vasm.size()); i++)
        {
            if(vasm[i]._code.substr(0, sizeof("Jump")-1) == "Jump"  ||  vasm[i]._code.substr(0, sizeof("B")-1) == "B")
            {
                vasm[i]._code += label;
                return;
            }
        }
    }

    void addLabelToJump(std::vector<VasmLine>& vasm, const std::string& label)
    {
        for(int i=0; i<int(vasm.size()); i++)
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                if(vasm[i]._code.find("CALLI_JUMP") != std::string::npos)
                {
                    vasm[i]._opcode = "CALLI";
                    vasm[i]._code = "CALLI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("CALLI")-1), ' ') + label;
                    return;
                }
            }
            else
            {
                if(vasm[i]._code.find("BRA_JUMP") != std::string::npos)
                {
                    vasm[i]._opcode = "BRA";
                    vasm[i]._code = "BRA" + std::string(OPCODE_TRUNC_SIZE - (sizeof("BRA")-1), ' ') + label;
                    return;
                }
                else if(vasm[i]._code.find("LDWI_JUMP") != std::string::npos)
                {
                    vasm[i]._opcode = "LDWI";
                    vasm[i]._code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + label;
                    return;
                }
            }
        }
    }


    bool parseCode(void)
    {
        size_t foundPos;
        CodeLine codeLine;

        // LET modifies code
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            Keywords::KeywordFuncResult result;

            if(Keywords::findKeyword(_codeLines[i]._code, "LET", foundPos))
            {
                Keywords::keywordLET(_codeLines[i], i, 0, foundPos - 3, result);
            }
        }

        // Add END to code
        if(_codeLines.size())
        {
            bool foundEnd = false;
            for(int i=0; i<int(_codeLines[_codeLines.size() - 1]._tokens.size()); i++)
            {
                std::string token = _codeLines[_codeLines.size() - 1]._tokens[i];
                Expression::strToUpper(token);
                if(token =="END")
                {
                    foundEnd = true;
                    break;
                }
            }

            if(!foundEnd)
            {
                if(createCodeLine("END", 0, -1, -1, Expression::Int16Both, false, codeLine)) _codeLines.push_back(codeLine);
            }
        }

        // Parse code creating vars and vasm code, (BASIC code lines were created in ParseLabels())
        int varIndex, strIndex;
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            _currentCodeLineIndex = i;

            // First line of BASIC code is always a dummy INIT line, ignore it
            if(i > 0  &&  _codeLines[i]._code.size() >= 2)
            {
                // Adjust label address
                if(_codeLines[i]._labelIndex >= 0) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;

                // Multi-statements
                StatementResult statementResult = parseMultiStatements(_codeLines[i]._code, codeLine, i, varIndex, strIndex);
                if(statementResult == StatementError) return false;
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
        for(int i=0; i<int(_labels.size()); i++)
        {
            std::string address = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
            _equateLabels.push_back(_labels[i]._name);
        }

        // Internal labels
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
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
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
            {
                // BASIC label conflict
                for(int k=0; k<int(_internalLabels.size()); k++)
                {
                    int labelIndex = findLabel(_internalLabels[k]._address);
                    if(labelIndex >= 0)
                    {   
                        std::string basicLabel = _labels[labelIndex]._output;
                        Expression::stripWhitespace(basicLabel);
                        Expression::replaceText(_codeLines[i]._vasm[j]._code, _internalLabels[k]._name, basicLabel);

                        // BASIC labels override internal labels
                        if(_codeLines[i]._vasm[j]._address == _labels[labelIndex]._address)
                        {
                            _codeLines[i]._vasm[j]._internalLabel = basicLabel;
                        }
                    }

                    // Discarded internal label
                    for(int l=0; l<int(_discardedLabels.size()); l++)
                    {
                        // Match on unique address embedded within names or the real address
                        std::string internalName = _internalLabels[k]._name.substr(_internalLabels[k]._name.size() - 4, 4);
                        std::string discardedName = _discardedLabels[l]._name.substr(_discardedLabels[l]._name.size() - 4, 4);
                        if(internalName == discardedName  ||  _internalLabels[k]._address == _discardedLabels[l]._address)
                        {
                            Expression::replaceText(_codeLines[i]._vasm[j]._code, _discardedLabels[l]._name, _internalLabels[k]._name);
                        }
                    }
                }
            }
        }

        _output.push_back("\n");
    }

    void outputConsts(void)
    {
        _output.push_back("; Constants\n");

        for(int i=0; i<int(_constants.size()); i++)
        {
            int16_t data = _constants[i]._data;
            std::string name = _constants[i]._name;
            std::string internalName = _constants[i]._internalName;
            ConstType constType = _constants[i]._constType;

            switch(constType)
            {
                case ConstInt16:
                {
                    _output.push_back(internalName + std::string(LABEL_TRUNC_SIZE - internalName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(data) + "\n");
                }
                break;
    
                case ConstStr:
                {
                }
                break;

                default: break;
            }
        }

        _output.push_back("\n");
    }

    void outputVars(void)
    {
        _output.push_back("; Variables\n");

        for(int i=0; i<int(_integerVars.size()); i++)
        {
            switch(_integerVars[i]._varType)
            {
                case VarInt16:
                {
                    std::string address = Expression::wordToHexString(_integerVars[i]._address);
                    _output.push_back(_integerVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
                }
                break;

                case VarArray:
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
                break;

                default: break;
            }
        }

        _output.push_back("\n");
    }

    void outputStrs(void)
    {
        _output.push_back("; Strings\n");

        for(int i=0; i<int(_stringVars.size()); i++)
        {
            _output.push_back(_stringVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._address) + "\n");
            _output.push_back(_stringVars[i]._output + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(_stringVars[i]._size) + " '" + _stringVars[i]._text + "' 0\n");
        }

        _output.push_back("\n");
    }

    bool outputDefs(void)
    {
        // Create def byte data
        _output.push_back("; Define Bytes\n");
        for(int i=0; i<int(_defDataBytes.size()); i++)
        {
            std::string defName = "def_bytes_" + Expression::wordToHexString(_defDataBytes[i]._address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_defDataBytes[i]._address) + "\n");
            
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<int(_defDataBytes[i]._data.size()); j++)
            {
                dbString += std::to_string(_defDataBytes[i]._data[j]) + " ";
            }
            _output.push_back(dbString + "\n");
        }
        _output.push_back("\n");

        // Create def word data
        _output.push_back("; Define Words\n");
        for(int i=0; i<int(_defDataWords.size()); i++)
        {
            std::string defName = "def_words_" + Expression::wordToHexString(_defDataWords[i]._address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_defDataWords[i]._address) + "\n");
            
            std::string dwString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<int(_defDataWords[i]._data.size()); j++)
            {
                dwString += std::to_string(_defDataWords[i]._data[j]) + " ";
            }
            _output.push_back(dwString + "\n");
        }
        _output.push_back("\n");

        // Create def image data
        _output.push_back("; Define Images\n");
        for(int i=0; i<int(_defDataImages.size()); i++)
        {
            // For each scanline of image data
            uint16_t offset = 0x0000;
            uint16_t address = _defDataImages[i]._address;
            for(int j=0; j<_defDataImages[i]._height; j++)
            {
                // TODO: find a way around this monster hack, 'Loader.gcl' is resident at these addresses when loading *.gt1 files, you can overwrite these locations only AFTER the loading process has finished
                if(address + offset < 0x5900  ||  address + offset > 0x5BFF)
                {
                    std::string defName = "def_images_" + Expression::wordToHexString(address + offset);
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address + offset) + "\n");
            
                    std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                    for(int k=0; k<int(_defDataImages[i]._width); k++)
                    {
                        dbString += std::to_string(_defDataImages[i]._data[j * _defDataImages[i]._width  +  k]) + " ";
                    }
                    _output.push_back(dbString + "\n");
                }

                offset += _defDataImages[i]._stride;
            }
        }
        _output.push_back("\n");

        return true;
    }

    bool outputLuts(void)
    {
        _output.push_back("; Lookup Tables\n");

        // Numeric labels LUT
        if(_createNumericLabelLut)
        {
            std::vector<uint16_t> numericLabels;
            std::vector<uint16_t> numericAddresses;
            for(int i=0; i<int(_labels.size()); i++)
            {
                if(_labels[i]._numeric)
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

            if(numericLabels.size())
            {
                // Create numeric labels LUT, (delimited by -1)
                int lutSize = int(numericLabels.size()) * 2;
                uint16_t lutAddress;
                if(!Memory::getFreeRAM(Memory::FitDescending, lutSize + 2, 0x0200, _runtimeStart, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : Not enough RAM for numeric labels LUT of size %d\n", lutSize + 2);
                    return false;
                }

                std::string lutName = "_lut_numericLabs";
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<int(numericLabels.size()); i++)
                {
                    dwString += std::to_string(numericLabels[i]) + " ";
                }
                _output.push_back(dwString + "0\n");

                // Create numeric addresses LUT
                if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, 0x0200, _runtimeStart, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : Not enough RAM for numeric addresses LUT of size %d\n", lutSize);
                    return false;
                }

                lutName = "_lut_numericAddrs";
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<int(numericAddresses.size()); i++)
                {
                    dwString += Expression::wordToHexString(numericAddresses[i]) + " ";
                }
                _output.push_back(dwString + "\n");
            }
        }

        // ON GOTO GOSUB label LUTs
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            // Output LUT if it exists
            int lutSize = int(_codeLines[i]._onGotoGosubLut._lut.size());
            uint16_t lutAddress = _codeLines[i]._onGotoGosubLut._address;
            std::string lutName = _codeLines[i]._onGotoGosubLut._name;
            if(lutSize)
            {
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<lutSize; j++)
                {
                    int index = _codeLines[i]._onGotoGosubLut._lut[j];
                    if(index == -1) fprintf(stderr, "Compiler::outputLuts() : Warning, label index is invalid for LUT entry %d\n", j);

                    uint16_t labelAddress = _labels[index]._address;
                    dwString += Expression::wordToHexString(labelAddress) + " ";
                }
                _output.push_back(dwString + "\n");
            }
        }

        // String concatenation LUTs
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            // Output LUT if it exists
            int lutSize = int(_codeLines[i]._strConcatLut._lut.size());
            uint16_t lutAddress = _codeLines[i]._strConcatLut._address;
            std::string lutName = "_concat_" + Expression::wordToHexString(lutAddress);
            if(lutSize)
            {
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<lutSize; j++)
                {
                    uint16_t address = _codeLines[i]._strConcatLut._lut[j];
                    dwString += Expression::wordToHexString(address) + " ";
                }
                _output.push_back(dwString + "\n");
            }
        }

        // INPUT LUTs
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            // Output varsLUT if it exists
            int varsSize = int(_codeLines[i]._inputLut._varsLut.size());
            uint16_t varsAddr = _codeLines[i]._inputLut._varsAddr;
            std::string varsName = "_inputVars_" + Expression::wordToHexString(varsAddr);
            if(varsSize)
            {
                _output.push_back(varsName + std::string(LABEL_TRUNC_SIZE - varsName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(varsAddr) + "\n");
            
                std::string dwString = varsName + std::string(LABEL_TRUNC_SIZE - varsName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<varsSize; j++)
                {
                    uint16_t address = _codeLines[i]._inputLut._varsLut[j];
                    dwString += Expression::wordToHexString(address) + " ";
                }
                _output.push_back(dwString + "\n");
            }

            // Output strsLUT if it exists
            int strsSize = int(_codeLines[i]._inputLut._strsLut.size());
            uint16_t strsAddr = _codeLines[i]._inputLut._strsAddr;
            std::string strsName = "_inputStrs_" + Expression::wordToHexString(strsAddr);
            if(strsSize)
            {
                _output.push_back(strsName + std::string(LABEL_TRUNC_SIZE - strsName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(strsAddr) + "\n");
            
                std::string dwString = strsName + std::string(LABEL_TRUNC_SIZE - strsName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<strsSize; j++)
                {
                    uint16_t address = _codeLines[i]._inputLut._strsLut[j];
                    dwString += Expression::wordToHexString(address) + " ";
                }
                _output.push_back(dwString + "\n");
            }

            // Output typesLUT if it exists
            int typesSize = int(_codeLines[i]._inputLut._typesLut.size());
            uint16_t typesAddr = _codeLines[i]._inputLut._typesAddr;
            std::string typesName = "_inputTypes_" + Expression::wordToHexString(typesAddr);
            if(typesSize)
            {
                _output.push_back(typesName + std::string(LABEL_TRUNC_SIZE - typesName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(typesAddr) + "\n");
            
                std::string dwString = typesName + std::string(LABEL_TRUNC_SIZE - typesName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int j=0; j<typesSize; j++)
                {
                    uint16_t type = _codeLines[i]._inputLut._typesLut[j];
                    dwString += Expression::wordToHexString(type) + " ";
                }
                _output.push_back(dwString + "\n");
            }

            // Output INPUT LUT
            if(varsSize  &&  strsSize  &&  typesSize  &&  _codeLines[i]._inputLut._address)
            {
                uint16_t lutAddress = _codeLines[i]._inputLut._address;
                std::string name = "_input_" + Expression::wordToHexString(lutAddress);
                _output.push_back(name + std::string(LABEL_TRUNC_SIZE - name.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = name + std::string(LABEL_TRUNC_SIZE - name.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                dwString += Expression::wordToHexString(varsAddr) + " ";
                dwString += Expression::wordToHexString(strsAddr) + " ";
                dwString += Expression::wordToHexString(typesAddr) + " ";
                _output.push_back(dwString + "\n");
            }
        }

        _output.push_back("\n");

        return true;
    }


    void outputInternalEquates(void)
    {
        _output.push_back("\n");
        _output.push_back("; Internal variables\n");
        _output.push_back("serialRawPrev"  + std::string(LABEL_TRUNC_SIZE - strlen("serialRawPrev"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(0x0081) + "\n");
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
        _output.push_back("register10"     + std::string(LABEL_TRUNC_SIZE - strlen("register10"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x14\n");
        _output.push_back("register11"     + std::string(LABEL_TRUNC_SIZE - strlen("register11"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x16\n");
        _output.push_back("register12"     + std::string(LABEL_TRUNC_SIZE - strlen("register12"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x18\n");
        _output.push_back("register13"     + std::string(LABEL_TRUNC_SIZE - strlen("register13"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1A\n");
        _output.push_back("register14"     + std::string(LABEL_TRUNC_SIZE - strlen("register14"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1C\n");
        _output.push_back("register15"     + std::string(LABEL_TRUNC_SIZE - strlen("register15"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1E\n");
        _output.push_back("fgbgColour"     + std::string(LABEL_TRUNC_SIZE - strlen("fgbgColour"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x20\n");
        _output.push_back("cursorXY"       + std::string(LABEL_TRUNC_SIZE - strlen("cursorXY"), ' ')       + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x22\n");
        _output.push_back("midiStream"     + std::string(LABEL_TRUNC_SIZE - strlen("midiStream"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x24\n");
        _output.push_back("midiDelay"      + std::string(LABEL_TRUNC_SIZE - strlen("midiDelay"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x26\n");
        _output.push_back("frameCountPrev" + std::string(LABEL_TRUNC_SIZE - strlen("frameCountPrev"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x28\n");
        _output.push_back("miscFlags"      + std::string(LABEL_TRUNC_SIZE - strlen("miscFlags"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x2A\n");
        _output.push_back("highByteMask"   + std::string(LABEL_TRUNC_SIZE - strlen("highByteMask"), ' ')   + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x2C\n");
        _output.push_back("\n");

        _output.push_back("; Internal buffers\n");
        _output.push_back("textWorkArea" + std::string(LABEL_TRUNC_SIZE - strlen("textWorkArea"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(getStrWorkArea()) + "\n");
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

        for(int i=0; i<int(_codeLines.size()); i++)
        {
            int labelIndex = _codeLines[i]._labelIndex;

            // Valid BASIC code
            if(_codeLines[i]._code.size() >= 2  &&  _codeLines[i]._vasm.size())
            {
                // BASIC Label, (may not be owned by vasm line 0 as PAGE JUMPS may move labels)
                std::string basicLabel = (labelIndex >= 0) ? _labels[labelIndex]._output : "";

                // Vasm code
                for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
                {
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
        for(int k=0; k<int(_equateLabels.size()); k++)
        {
            bool foundLabel = false;

            for(int i=0; i<int(_codeLines.size()); i++)
            {
                // Valid BASIC code
                if(_codeLines[i]._code.size() >= 2  &&  _codeLines[i]._vasm.size())
                {
                    // Vasm code
                    for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
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
                bool foundInternal = (_equateLabels[k].find("_if_") != std::string::npos     ||  _equateLabels[k].find("_else_") != std::string::npos  ||  _equateLabels[k].find("_elseif_") != std::string::npos  ||
                                      _equateLabels[k].find("_endif_") != std::string::npos  ||  _equateLabels[k].find("_while_") != std::string::npos ||  _equateLabels[k].find("_wend_") != std::string::npos    ||
                                      _equateLabels[k].find("_repeat_") != std::string::npos ||  _equateLabels[k].find("_next_") != std::string::npos  ||  _equateLabels[k].find("_page_") != std::string::npos);

                for(int l=0; l<int(_output.size()); l++)
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


    void clearCompiler(void)
    {
        _vasmPC         = USER_CODE_START;
        _tempVarStart   = TEMP_VAR_START;
        _userVarStart   = USER_VAR_START;
        _runtimeEnd     = 0xFFFF;
        _runtimeStart   = 0xFFFF;
        _strWorkArea    = 0x0000;

        _codeOptimiseType = CodeSpeed;

        _compilingError = false;
        _arrayIndiciesOne = false;
        _createNumericLabelLut = false;

        _currentLabelIndex = -1;
        _currentCodeLineIndex = 0;
        _jumpFalseUniqueId = 0;

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
        _constants.clear();
        _integerVars.clear();
        _stringVars.clear();
        _defDataBytes.clear();
        _defDataWords.clear();
        _defDataImages.clear();

        Linker::resetIncludeFiles();
        Linker::resetInternalSubs();

        Memory::initialise();
        Operators::initialise();

        Expression::setExprFunc(expression);

        while(!_forNextDataStack.empty())     _forNextDataStack.pop();
        while(!_elseIfDataStack.empty())      _elseIfDataStack.pop();
        while(!_endIfDataStack.empty())       _endIfDataStack.pop();
        while(!_whileWendDataStack.empty())   _whileWendDataStack.pop();
        while(!_repeatUntilDataStack.empty()) _repeatUntilDataStack.pop();

        // Allocate default string work area, (for string functions like LEFT$, MID$, etc)
        Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, 0x0200, _runtimeStart, _strWorkArea);
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

        // Pragmas
        if(!parsePragmas(numLines)) return false;

        // Labels
        if(!parseLabels(numLines)) return false;

        // Includes
        if(!Linker::parseIncludes()) return false;

        // Code
        if(!parseCode()) return false;

        // Check for compiler errors
        if(_compilingError)
        {
            _PAUSE_;
            return false;
        }

        // Optimise
        if(!Optimiser::optimiseCode()) return false;

        // Check for code relocations
        if(!Validater::checkForRelocations()) return false;

        // Check keywords that form statement blocks
        if(!Validater::checkStatementBlocks()) return false;

        // Only link runtime subroutines that are referenced
        if(!Linker::linkInternalSubs()) return false;

        // Output
        outputReservedWords();
        outputInternalEquates();
        outputIncludes();
        outputLabels();
        outputVars();
        outputStrs();
        outputDefs();
        outputLuts();
        outputCode();

        // Discard
        discardUnusedLabels();

        // Re-linking is needed here as collectInternalRuntime() can find new subs that need to be linked
        Linker::collectInternalRuntime();
        Linker::relinkInternalSubs();
        Linker::outputInternalSubs();

        Validater::checkBranchLabels();

        // Check for validation errors
        if(_compilingError)
        {
            _PAUSE_;
            return false;
        }

        //Memory::printFreeRamList(Memory::NoSort); //Memory::SizeAscending);

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        return true;
    }
}