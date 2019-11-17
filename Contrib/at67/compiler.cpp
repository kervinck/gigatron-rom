#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "keywords.h"
#include "optimiser.h"
#include "validater.h"
#include "linker.h"


namespace Compiler
{
    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated, VarExists};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};
    enum ConstantStrType {StrChar, StrHex, StrHexw};


    uint16_t _vasmPC         = USER_CODE_START;
    uint16_t _tempVarStart   = TEMP_VAR_START;
    uint16_t _userVarStart   = USER_VAR_START;
    uint16_t _runtimeEnd     = 0x7FFF;
    uint16_t _runtimeStart   = 0x7FFF;

    bool _nextTempVar = true;
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

    std::vector<DefDataByte> _defDataBytes;
    std::vector<DefDataWord> _defDataWords;

    uint16_t getVasmPC(void) {return _vasmPC;}
    uint16_t getRuntimeEnd(void) {return _runtimeEnd;}
    uint16_t getRuntimeStart(void) {return _runtimeStart;}
    uint16_t getTempVarStart(void) {return _tempVarStart;}
    int getCurrentLabelIndex(void) {return _currentLabelIndex;}
    std::string& getNextInternalLabel(void) {return _nextInternalLabel;}

    void setRuntimeEnd(uint16_t runtimeEnd) {_runtimeEnd = runtimeEnd;}
    void setRuntimeStart(uint16_t runtimeStart) {_runtimeStart = runtimeStart;}
    void setCreateNumericLabelLut(bool createNumericLabelLut) {_createNumericLabelLut = createNumericLabelLut;}

    int incJumpFalseUniqueId(void) {return _jumpFalseUniqueId++;}

    std::vector<Label>& getLabels(void) {return _labels;}
    std::vector<CodeLine>& getCodeLines(void) {return _codeLines;}
    std::vector<IntegerVar>& getIntegerVars(void) {return _integerVars;}
    std::vector<StringVar>& getStringVars(void) {return _stringVars;}
    std::vector<InternalLabel>& getInternalLabels(void) {return _internalLabels;}
    std::vector<InternalLabel>& getDiscardedLabels(void) {return _discardedLabels;}
    std::vector<std::string>& getOutput(void) {return _output;}
    std::vector<std::string>& getRuntime(void) {return _runtime;}
    std::vector<DefDataByte>& getDefDataBytes(void) {return _defDataBytes;}
    std::vector<DefDataWord>& getDefDataWords(void) {return _defDataWords;}

    std::map<std::string, MacroIndexEntry>& getMacroIndexEntries(void) {return _macroIndexEntries;}

    std::stack<ForNextData>& getForNextDataStack(void) {return _forNextDataStack;}
    std::stack<ElseIfData>& getElseIfDataStack(void) {return _elseIfDataStack;}
    std::stack<EndIfData>& getEndIfDataStack(void) {return _endIfDataStack;}
    std::stack<WhileWendData>& getWhileWendDataStack(void) {return _whileWendDataStack;}
    std::stack<RepeatUntilData>& getRepeatUntilDataStack(void) {return _repeatUntilDataStack;}


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


    void createLabel(uint16_t address, const std::string& name, const std::string& output, int codeLineIndex, Label& label, bool numeric, bool addUnderscore, bool pageJump, bool gosub)
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
        if(codeLine._code.size() >= 2  &&  equals < codeLine._code.size())
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
                for(int j=0; j<Keywords::getEqualsKeywords().size(); j++)
                {
                    size_t keyword = token.find(Keywords::getEqualsKeywords()[j]);
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
        std::vector<std::string> tokens = Expression::tokenise(stripped, "-+/*%&<>=();, ", false);

        // Check for string keywords
        for(int i=0; i<tokens.size(); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getStringKeywords().find(token) != Keywords::getStringKeywords().end())
            {
                params = Keywords::getStringKeywords()[token]._params;
                expressionType |= Expression::HasStringKeywords;
                break;
            }
        }

        // Check for keywords
        for(int i=0; i<tokens.size(); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getKeywords().find(token) != Keywords::getKeywords().end())
            {
                params = Keywords::getKeywords()[token]._params;
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
        if(stripped.find_first_of("-+/*%&<>=") != std::string::npos) expressionType |= Expression::HasOperators;
        std::string mod = stripped;
        Expression::strToUpper(mod);
        if(mod.find(" AND ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" XOR ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" OR"  ) != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" NOT ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" MOD ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" LSL ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(" LSR ") != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find("<<"   ) != std::string::npos) expressionType |= Expression::HasOperators;
        if(mod.find(">>"   ) != std::string::npos) expressionType |= Expression::HasOperators;

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

        if(codeLine._code.size() < 2) return false; // anything too small is ignored

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

    void emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx, const std::string& internalLabel, bool pageJump)
    {
        if(codeLineIdx == -1) codeLineIdx = _currentCodeLineIndex;

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
    uint32_t parseArrayVarExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
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
        Expression::Numeric arrIndex;
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        uint32_t expressionType = parseArrayVarExpression(codeLine, codeLineIndex, arrText, arrIndex);

        int intSize = _integerVars[varIndex]._intSize;
        uint16_t arrayPtr = _integerVars[varIndex]._array;

        // Constant index
        if(!(expressionType & Expression::HasVars))
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + arrIndex._value*intSize), false, codeLineIndex);
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
                // Saves 3 bytes per array access, but costs an extra 5 instructions in performance
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
    OperandType parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, std::string& operand, Expression::Numeric& numeric)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
        {
            operand = Expression::byteToHexString(uint8_t(_tempVarStart));
            return OperandTemp;
        }
        else if(expressionType & Expression::HasVars)
        {
            operand = _integerVars[varIndex]._name;
            return OperandVar;
        }

        operand = std::to_string(numeric._value);
        return OperandConst;
    }

    // Generic LDW expression parser
    uint32_t parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasVars)
        {
            emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
        }
        else
        {
            (numeric._value >= 0  &&  numeric._value <= 255) ? emitVcpuAsm("LDI", std::to_string(numeric._value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(numeric._value), false, codeLineIndex);
        }

        return expressionType;
    }

    // Loop specific parser
    uint32_t parseExpression(CodeLine& codeLine, int codeLineIndex, std::string& expression, Expression::Numeric& numeric, int16_t replace)
    {
        int varIndex, params;
        Expression::parse(expression, codeLineIndex, numeric);
        uint32_t expressionType = isExpression(expression, varIndex, params);
        if(((expressionType & Expression::HasVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(_tempVarStart)), false, codeLineIndex);
        }
        else if(expressionType & Expression::HasVars)
        {
            emitVcpuAsm("LDW", "_" + _integerVars[varIndex]._name, false, codeLineIndex);
        }
        else
        {
            if(numeric._value == 0  &&  replace != 0) numeric._value = replace;
            (numeric._value >= 0  &&  numeric._value <= 255) ? emitVcpuAsm("LDI", std::to_string(numeric._value), false, codeLineIndex) : emitVcpuAsm("LDWI", std::to_string(numeric._value), false, codeLineIndex);
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
                if(!isdigit(code[i])  &&  code[i] != ':')
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
            bool foundGosub = false;
            bool numericGosub = false;
            std::string labelName = code.substr(0, space);
            size_t colon = labelName.find(':');
            if(colon != std::string::npos)
            {
                foundGosub = true;
                numericGosub = true;
                labelName.erase(colon, 1);
            }
            else
            {
                foundGosub = isGosubLabel(labelName);
            }
            createLabel(_vasmPC, labelName, labelName, int(_codeLines.size()), label, numericGosub, true, false, foundGosub);
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
            size_t calli;
            if((calli = it->find("_useOpcodeCALLI_")) != std::string::npos)
            {
                it->erase(calli, sizeof("_useOpcodeCALLI_"));
                Assembler::setUseOpcodeCALLI(true);
            }
            size_t runtime;
            if((runtime = it->find("_runtimeStart_")) != std::string::npos)
            {
                std::vector<size_t> offsets;
                std::vector<std::string> tokens = Expression::tokenise(*it, ' ', offsets, false, false);
                for(int i=0; i<tokens.size(); i++)
                {
                    Expression::stripWhitespace(tokens[i]);
                }
                if(tokens.size() != 2) 
                {
                    fprintf(stderr, "Compiler::parseLabels() : Syntax error, _runtimeStart_ <address>, found %s\n", it->c_str());
                    return false;
                }
                uint16_t address;
                if(!Expression::stringToU16(tokens[1], address))
                {
                    fprintf(stderr, "Compiler::parseLabels() : Syntax error, invalid address in _runtimeStart_ <address>, found %s\n", it->c_str());
                    return false;
                }
                _runtimeStart = address;
                it->erase(offsets[0] - sizeof("_runtimeStart_"), offsets[1] - (offsets[0] - sizeof("_runtimeStart_")));
                
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
            }
        }

        return true;
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
            if(!Memory::giveFreeRAM(Memory::FitAscending, int(str.size()) + 1, 0x0200, _runtimeStart, address))
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
            // Saves 2 bytes per array access but costs an extra 2 instructions in performance
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
            if(right._value == 8)
            {
                if(isdigit(left._varName[0]))
                {
                    emitVcpuAsm("LD", Expression::byteToHexString(uint8_t(left._value)) + " + 1", false);
                }
                else
                {
                    int varIndex = findVar(left._varName);
                    if(varIndex == -1) fprintf(stderr, "Compiler::operatorLSR() : couldn't find variable name '%s'\n", left._varName.c_str());
                    emitVcpuAsm("LD", "_" + left._varName + " + 1", false);
                }

                left._value = uint8_t(_tempVarStart);
                left._isAddress = true;
                left._varName = _tempVarStartStr;
            }
            else
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
                    //case 8: opcode = "%ShiftRight8bit"; break;
                }

                handleLogicalOp(opcode, left, right);
                emitVcpuAsm(opcode, "", false);
            }
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }


    // ********************************************************************************************
    // Conditional Operators
    // ********************************************************************************************
    bool handleCondOp(Expression::Numeric& lhs, Expression::Numeric& rhs, bool logical, bool& invertedLogic)
    {
        lhs._isLogical = logical;

        // Swap left and right to take advantage of LDWI for 16bit numbers
        invertedLogic = false;
        if(!rhs._isAddress  &&  uint16_t(rhs._value) > 255)
        {
            std::swap(lhs, rhs);
            invertedLogic = true;
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
                emitVcpuAsm("SUBW", Expression::byteToHexString(uint8_t(rhs._value)), false);
            }
            // User variable address
            else
            {
                if(!emitVcpuAsmUserVar("SUBW", rhs._varName.c_str(), _nextTempVar)) return false;
                _nextTempVar = false;
            }
        }
        else
        {
            emitVcpuAsm("SUBI", std::to_string(rhs._value), false);
        }

        lhs._value = uint8_t(_tempVarStart);
        lhs._isAddress = true;
        lhs._varName = _tempVarStartStr;

        return true;
    }

    Expression::Numeric operatorEQ(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value == right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert equals into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Eq" : "Ne";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorNE(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value != right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert not equals into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Ne" : "Eq";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorLE(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value <= right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert less than or equals into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Le" : "Gt";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorGE(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = left._value >= right._value;
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert greater than or equals into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Ge" : "Lt";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorLT(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value < right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert less than into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Lt" : "Ge";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

        emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(_tempVarStart)), false);

        return left;
    }

    Expression::Numeric operatorGT(Expression::Numeric& left, Expression::Numeric& right, bool logical)
    {
        if(!left._isAddress  &&  !right._isAddress)
        {
            left._value = (left._value > right._value);
            return left;
        }

        bool invertedLogic = false;
        left._isValid = handleCondOp(left, right, logical, invertedLogic);

        // Convert greater than into a logical 1, (boolean conversion)
        std::string cc = (!invertedLogic) ? "Gt" : "Le";
        if(logical)
        {
            Assembler::getUseOpcodeCALLI() ? emitVcpuAsm("CALLI", "convert" + cc + "Op", false) : emitVcpuAsm("CALL", "convert" + cc + "OpAddr", false);
        }
        else
        {
            emitVcpuAsm("%Jump" + Expression::strToUpper(cc), "", false);
        }

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
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, false, std::string(""));

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
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, false, std::string(""));

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
        if((!left._isAddress  &&  left._value == 0)  ||  (!right._isAddress  &&  right._value == 0)) return Expression::Numeric(0, -1, true, false, false, std::string(""));

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
                numeric = Expression::Numeric(0, -1, false, false, false, std::string(""));
            }
            get(false);
        }
        else if((peek(false) >= '0'  &&  peek(false) <= '9')  ||  peek(false) == '&')
        {
            if(!number(value))
            {
                fprintf(stderr, "Compiler::factor() : Bad numeric data in '%s' on line %d\n", _codeLines[_currentCodeLineIndex]._code.c_str(), Expression::getLineNumber() + 1);
                numeric = Expression::Numeric(0, -1, false, false, false, std::string(""));
            }
            else
            {
                numeric = Expression::Numeric(value, -1, true, false, false, std::string(""));
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
                case 0: numeric = Expression::Numeric(defaultValue, -1, false, false, false, std::string("")); break;

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
                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            numeric = Expression::Numeric(defaultValue, varIndex, true, true, false, varName);
                        }
                    }
                    // Unknown symbol
                    else
                    {
                        numeric = Expression::Numeric(defaultValue, -1, false, false, false, std::string(""));
                        if(varName.size()) fprintf(stderr, "Compiler::factor() : Found an unknown symbol '%s' : in '%s' on line %d\n", varName.c_str(), 
                                                                                                                                       _codeLines[_currentCodeLineIndex]._code.c_str(),
                                                                                                                                       Expression::getLineNumber() + 1);
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
            if(peek(false) == '+')           {get(false);t = term();result = operatorADD(result, t       );}
            else if(peek(false) == '-')      {get(false);t = term();result = operatorSUB(result, t       );}
            else if(Expression::find("XOR")) {           t = term();result = operatorXOR(result, t       );}
            else if(Expression::find("OR"))  {           t = term();result = operatorOR(result,  t       );}
            else if(Expression::find("LSL")) {           t = term();result = operatorLSL(result, t       );}
            else if(Expression::find("LSR")) {           t = term();result = operatorLSR(result, t       );}
            else if(Expression::find("<<"))  {           t = term();result = operatorLSL(result, t       );}
            else if(Expression::find(">>"))  {           t = term();result = operatorLSR(result, t       );}
            else if(peek(false) == '=')      {get(false);t = term();result = operatorEQ(result,  t, true );}
            else if(Expression::find("<>"))  {           t = term();result = operatorNE(result,  t, true );}
            else if(Expression::find("<="))  {           t = term();result = operatorLE(result,  t, true );}
            else if(Expression::find(">="))  {           t = term();result = operatorGE(result,  t, true );}
            else if(peek(false) == '<')      {get(false);t = term();result = operatorLT(result,  t, true );}
            else if(peek(false) == '>')      {get(false);t = term();result = operatorGT(result,  t, true );}
            else if(Expression::find("&="))  {           t = term();result = operatorEQ(result,  t, false);}
            else if(Expression::find("&<>")) {           t = term();result = operatorNE(result,  t, false);}
            else if(Expression::find("&<=")) {           t = term();result = operatorLE(result,  t, false);}
            else if(Expression::find("&>=")) {           t = term();result = operatorGE(result,  t, false);}
            else if(Expression::find("&<"))  {           t = term();result = operatorLT(result,  t, false);}
            else if(Expression::find("&>"))  {           t = term();result = operatorGT(result,  t, false);}
            else return result;
        }
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
            Keywords::KeywordFuncResult result;
            Keywords::KeywordResult keywordResult = Keywords::handleKeywords(codeLine, codeLine._tokens[i], codeLineIndex, result);
            if(keywordResult == Keywords::KeywordError) return StatementError;
            std::string token = codeLine._tokens[i];
            if(Expression::strToUpper(token)      == "REM"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "END"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "CLS"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "PUT"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "PRINT" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "ON"    ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "FOR"   ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "NEXT"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "WHILE" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "WEND"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "REPEAT") return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "UNTIL" ) return SingleStatementParsed;
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
            else if(Expression::strToUpper(token) == "ELSE"  ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "ENDIF" ) return SingleStatementParsed;
            else if(Expression::strToUpper(token) == "IF"    ) return MultiStatementParsed;
        }

        bool containsVars = false;
        int varIndexRhs = -1, params = 0;
        uint32_t expressionType = isExpression(codeLine._expression, varIndexRhs, params);
        if(expressionType & Expression::HasVars) containsVars = true;

        Expression::Numeric value;
        Expression::parse(codeLine._expression, codeLineIndex, value);
        if(codeLine._varIndex != -1)
        {
            updateVar(value._value, codeLine, codeLine._varIndex, containsVars);
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

    void addLabelToJumpCC(std::vector<VasmLine>& vasm, std::string& label)
    {
        for(int i=0; i<vasm.size(); i++)
        {
            if(vasm[i]._code.substr(0, sizeof("Jump")-1) == "Jump")
            {
                vasm[i]._code += label;
                return;
            }
        }
    }

    void addLabelToJump(std::vector<VasmLine>& vasm, std::string& label)
    {
        for(int i=0; i<vasm.size(); i++)
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
                if(vasm[i]._code.find("LDWI_JUMP") != std::string::npos)
                {
                    vasm[i]._opcode = "LDWI";
                    vasm[i]._code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + label;
                    return;
                }
            }
        }
    }

    // If _nextInternalLabel is already queued, add it to discarded labels so that it is fixed later in outputLabels
    void setNextInternalLabel(const std::string& label)
    {
        if(_nextInternalLabel.size()) _discardedLabels.push_back({_vasmPC, _nextInternalLabel});

        _nextInternalLabel = label;
    }


    bool parseCode(void)
    {
        size_t foundPos;
        CodeLine codeLine;

        // REM and LET modify code
        for(int i=0; i<_codeLines.size(); i++)
        {
            Keywords::KeywordFuncResult result;

            // ' is a shortcut for REM
            if((foundPos = _codeLines[i]._code.find_first_of('\'')) != std::string::npos  ||  Keywords::findKeyword(_codeLines[i]._code, "REM", foundPos))
            {
                Keywords::keywordREM(_codeLines[i], i, foundPos, result);
            }
            else if(Keywords::findKeyword(_codeLines[i]._code, "LET", foundPos))
            {
                Keywords::keywordLET(_codeLines[i], i, foundPos - 3, result);
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
            if(i > 0  &&  _codeLines[i]._code.size() >= 2)
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
                        // Use unique address embedded within names rather than discarded real address which can be out of date due to optimiser and relocater
                        std::string internalName = _internalLabels[k]._name.substr(_internalLabels[k]._name.size() - 4, 4);
                        std::string discardedName = _discardedLabels[l]._name.substr(_discardedLabels[l]._name.size() - 4, 4);
                        if(internalName == discardedName)
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

            // Create numeric labels lut
            if(numericLabels.size())
            {
                // Numeric labels lut, (delimited by -1)
                int lutSize = int(numericLabels.size()) * 2;
                uint16_t lutAddress;
                if(!Memory::giveFreeRAM(Memory::FitAscending, lutSize + 2, 0x0200, _runtimeStart, lutAddress))
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
                if(!Memory::giveFreeRAM(Memory::FitAscending, lutSize, 0x0200, _runtimeStart, lutAddress))
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
            if(_codeLines[i]._code.size() >= 2  &&  _codeLines[i]._vasm.size())
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
                if(_codeLines[i]._code.size() >= 2  &&  _codeLines[i]._vasm.size())
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
                bool foundInternal = (_equateLabels[k].find("_if_") != std::string::npos     ||  _equateLabels[k].find("_else_") != std::string::npos  ||  _equateLabels[k].find("_elseif_") != std::string::npos  ||
                                      _equateLabels[k].find("_endif_") != std::string::npos  ||  _equateLabels[k].find("_while_") != std::string::npos ||  _equateLabels[k].find("_wend_") != std::string::npos    ||
                                      _equateLabels[k].find("_repeat_") != std::string::npos ||  _equateLabels[k].find("_next_") != std::string::npos  ||  _equateLabels[k].find("_page_") != std::string::npos);

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


    void clearCompiler(void)
    {
        _vasmPC         = USER_CODE_START;
        _tempVarStart   = TEMP_VAR_START;
        _userVarStart   = USER_VAR_START;
        _runtimeEnd     = 0x7FFF;
        _runtimeStart   = 0x7FFF;

        _nextTempVar = true;
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
        _integerVars.clear();
        _stringVars.clear();
        _defDataBytes.clear();
        _defDataWords.clear();

        Linker::resetInternalSubs();

        Memory::initialise();

        Expression::setExprFunc(expression);

        while(!_forNextDataStack.empty())     _forNextDataStack.pop();
        while(!_elseIfDataStack.empty())      _elseIfDataStack.pop();
        while(!_endIfDataStack.empty())       _endIfDataStack.pop();
        while(!_whileWendDataStack.empty())   _whileWendDataStack.pop();
        while(!_repeatUntilDataStack.empty()) _repeatUntilDataStack.pop();
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
        if(!Linker::parseIncludes()) return false;

        // Code
        if(!parseCode()) return false;

        // Optimise
        if(!Optimiser::optimiseCode()) return false;

        // Check code exclusion zones
        if(!Validater::checkExclusionZones()) return false;

        // Check branch labels
        if(!Validater::checkBranchLabels()) return false;

        // Check keywords that form statement blocks
        if(!Validater::checkStatementBlocks()) return false;

        // Only link runtime subroutines that are referenced
        if(!Linker::linkInternalSubs()) return false;

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

        // Re-linking is needed here as collectInternalRuntime() can find new subs that need to be linked
        Linker::collectInternalRuntime();
        Linker::relinkInternalSubs();
        Linker::outputInternalSubs();

        //Memory::printFreeRamList(Memory::SizeAscending);

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        return true;
    }
}