#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmath>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <numeric>

#include "memory.h"
#include "cpu.h"
#include "loader.h"
#include "assembler.h"
#include "compiler.h"
#include "pragmas.h"
#include "keywords.h"
#include "functions.h"
#include "operators.h"
#include "optimiser.h"
#include "validater.h"
#include "linker.h"
#include "midi.h"


size_t _heapTotalUsage = 0;
size_t _heapAllocations = 0;
void* operator new(size_t size)
{
    _heapTotalUsage += size;
    _heapAllocations++;

    return malloc(size);
}


namespace Compiler
{
    const std::vector<std::string> _sysInitNames = {"InitEqOp", "InitNeOp", "InitLeOp", "InitGeOp", "InitLtOp", "InitGtOp", "Init8Array2d", "Init8Array3d", "Init16Array2d", "Init16Array3d", "InitRealTimeStub"};


    enum VarResult {VarError=-1, VarNotFound, VarCreated, VarUpdated, VarExists, VarExistsAsConst};
    enum StrResult {StrError=-1, StrNotFound, StrCreated};
    enum FloatSize {Float16=2, Float32=4};
    enum LabelResult {LabelError=-1, LabelNotFound, LabelFound};


    uint16_t _vasmPC          = USER_CODE_START;
    uint16_t _userCodeStart   = USER_CODE_START;
    uint16_t _tempVarSize     = TEMP_VAR_SIZE;
    uint16_t _tempVarStart    = TEMP_VAR_START;
    uint16_t _userVarStart    = USER_VAR_START;
    uint16_t _userVarsAddr    = USER_VAR_START;
    uint16_t _runtimeEnd      = RUN_TIME_START;
    uint16_t _runtimeStart    = RUN_TIME_START;
    uint16_t _arraysStart     = RUN_TIME_START;
    uint16_t _stringsStart    = RUN_TIME_START;
    uint16_t _regWorkArea     = 0x0000;
    uint16_t _gprintfVarsAddr = 0x0000;
    uint16_t _strWorkAreaIdx  = 0;

    uint16_t _strWorkArea[NUM_STR_WORK_AREAS]  = {0x0000, 0x0000};

    uint16_t _spritesAddrLutAddress  = 0x0000;
    uint16_t _spriteStripeChunks     = 15;
    uint16_t _spriteStripeMinAddress = USER_CODE_START;
    
    Memory::FitType _spriteStripeFitType = Memory::FitDescending;
    CodeOptimiseType _codeOptimiseType = CodeSpeed;
    Cpu::RomType _codeRomType = Cpu::ROMv3;

    std::map<std::string, int> _branchTypes = {{"BRA", 0}, {"BEQ", 1}, {"BNE", 2}, {"BLE", 3}, {"BGE", 4}, {"BLT", 5}, {"BGT", 6}};


    bool _codeIsAsm = false;
    bool _arrayIndiciesOne = false;
    bool _createNumericLabelLut = false;
    bool _createTimeData = false;

    int _currentLabelIndex = -1;
    int _currentCodeLineIndex = 0;
    int _nonNumericLabelIndex = -1;
    int _jumpFalseUniqueId = 0;

    int _codeLineStart = 0;
    std::string _codeLineText;
    std::string _codeLineModule;

    std::string _runtimePath = "./runtime";
    std::string _tempVarStartStr;
    std::string _nextInternalLabel;

    std::vector<Input> _input;

    std::vector<std::string> _output;
    std::vector<std::string> _runtime;

    std::vector<Label> _labels;
    std::vector<std::string> _gosubLabels;
    std::vector<std::string> _equateLabels;
    std::vector<InternalLabel> _internalLabels;
    std::vector<InternalLabel> _discardedLabels;

    std::vector<CodeLine> _codeLines;
    std::vector<ModuleLine> _moduleLines;
    std::vector<Constant> _constants;
    std::vector<IntegerVar> _integerVars;
    std::vector<StringVar> _stringVars;
    std::map<std::string, TypeData> _typeDatas;

    std::stack<ForNextData> _forNextDataStack;
    std::stack<ElseIfData> _elseIfDataStack;
    std::stack<WhileWendData> _whileWendDataStack;
    std::stack<RepeatUntilData> _repeatUntilDataStack;

    std::map<std::string, CallData> _callDataMap;

    std::stack<ProcData> _procDataStack;
    std::map<std::string, ProcData> _procDataMap;

    std::vector<std::string> _macroLines;
    std::map<int, MacroNameEntry> _macroNameEntries;
    std::map<std::string, MacroIndexEntry> _macroIndexEntries;

    std::vector<DefDataByte> _defDataBytes;
    std::vector<DefDataWord> _defDataWords;
    std::vector<DefDataImage> _defDataImages;
    std::vector<DefDataLoaderImageChunk> _defDataLoaderImageChunks;

    std::map<int, DefDataMidi> _defDataMidis;
    std::map<int, DefDataOpen> _defDataOpens;

    std::map<int, DefDataSprite> _defDataSprites;
    SpritesAddrLut _spritesAddrLut;

    std::map<int, DefDataFont> _defDataFonts;
    FontsAddrLut _fontsAddrLut;

    std::map<std::string, DefFunction> _defFunctions;

    std::vector<std::unique_ptr<DataObject>> _dataObjects;

    uint16_t getVasmPC(void) {return _vasmPC;}
    uint16_t getUserCodeStart(void) {return _userCodeStart;}
    uint16_t getRuntimeEnd(void) {return _runtimeEnd;}
    uint16_t getRuntimeStart(void) {return _runtimeStart;}
    uint16_t getArraysStart(void) {return _arraysStart;}
    uint16_t getStringsStart(void) {return _stringsStart;}
    uint16_t getTempVarStart(void) {return _tempVarStart;}
    uint16_t getTempVarSize(void) {return _tempVarSize;}
    uint16_t getRegWorkArea(void) {return _regWorkArea;}
    uint16_t getGprintfVarsAddr(void) {return _gprintfVarsAddr;}
    uint16_t getStrWorkArea(void) {return _strWorkArea[_strWorkAreaIdx & 1];}
    uint16_t getStrWorkArea(int index) {return _strWorkArea[index & 1];}
    uint16_t getSpritesAddrLutAddress(void) {return _spritesAddrLutAddress;}
    uint16_t getSpriteStripeChunks(void) {return _spriteStripeChunks;}
    uint16_t getSpriteStripeMinAddress(void) {return _spriteStripeMinAddress;}
    Memory::FitType getSpriteStripeFitType(void) {return _spriteStripeFitType;}
    CodeOptimiseType getCodeOptimiseType(void) {return _codeOptimiseType;}
    Cpu::RomType getCodeRomType(void) {return _codeRomType;}
    const std::map<std::string, int>& getBranchTypes(void) {return _branchTypes;}
    bool getArrayIndiciesOne(void) {return _arrayIndiciesOne;}
    bool getCreateTimeData(void) {return _createTimeData;}
    int getCurrentLabelIndex(void) {return _currentLabelIndex;}
    int getCurrentCodeLineIndex(void) {return _currentCodeLineIndex;}
    const std::string& getRuntimePath(void) {return _runtimePath;}
    const std::string& getTempVarStartStr(void) {return _tempVarStartStr;}
    const std::string& getNextInternalLabel(void) {return _nextInternalLabel;}
    int getCodeLineStart(int index) {return (_moduleLines.size()  &&  (index < int(_moduleLines.size()))) ? _moduleLines[index]._index : index;}

    void setCodeIsAsm(bool codeIsAsm) {_codeIsAsm = codeIsAsm;}
    void setUserCodeStart(uint16_t userCodeStart) {_userCodeStart = userCodeStart;}
    void setRuntimeEnd(uint16_t runtimeEnd) {_runtimeEnd = runtimeEnd;}
    void setRuntimePath(const std::string& runtimePath) {_runtimePath = runtimePath;}
    void setRuntimeStart(uint16_t runtimeStart) {_runtimeStart = runtimeStart;}
    void setArraysStart(uint16_t arraysStart) {_arraysStart = arraysStart;}
    void setStringsStart(uint16_t stringsStart) {_stringsStart = stringsStart;}
    void setTempVarStart(uint16_t tempVarStart) {_tempVarStart = tempVarStart;}
    void setTempVarSize(uint16_t tempVarSize) {_tempVarSize = tempVarSize;}
    void setRegWorkArea(uint16_t regWorkArea) {_regWorkArea = regWorkArea;}
    void setGprintfVarsAddr(uint16_t gprintfVarsAddr) {_gprintfVarsAddr = gprintfVarsAddr;}
    void setStrWorkArea(uint16_t strWorkArea, int index) {_strWorkArea[index & 1] = strWorkArea;}
    void setSpritesAddrLutAddress(uint16_t spritesAddrLutAddress) {_spritesAddrLutAddress = spritesAddrLutAddress;}
    void setSpriteStripeChunks(uint16_t spriteStripeChunks) {_spriteStripeChunks = spriteStripeChunks;}
    void setSpriteStripeMinAddress(uint16_t spriteStripeMinAddress) {_spriteStripeMinAddress = spriteStripeMinAddress;}
    void setSpriteStripeFitType(Memory::FitType spriteStripeFitType) {_spriteStripeFitType = spriteStripeFitType;}
    void setCodeOptimiseType(CodeOptimiseType codeOptimiseType) {_codeOptimiseType = codeOptimiseType;}
    void setCodeRomType(Cpu::RomType codeRomType) {_codeRomType = codeRomType;}
    void setCreateNumericLabelLut(bool createNumericLabelLut) {_createNumericLabelLut = createNumericLabelLut;}
    void setCreateTimeData(bool createTimeData) {_createTimeData = createTimeData;}
    void setArrayIndiciesOne(bool arrayIndiciesOne) {_arrayIndiciesOne = arrayIndiciesOne;}

    void nextStrWorkArea(void) {_strWorkAreaIdx = (_strWorkAreaIdx + 1) & 1;}

    int getNextJumpFalseUniqueId(void) {return _jumpFalseUniqueId++;}

    std::vector<Label>& getLabels(void) {return _labels;}
    std::vector<Constant>& getConstants(void) {return _constants;}
    std::vector<CodeLine>& getCodeLines(void) {return _codeLines;}
    std::vector<ModuleLine>& getModuleLines(void) {return _moduleLines;}
    std::vector<IntegerVar>& getIntegerVars(void) {return _integerVars;}
    std::vector<StringVar>& getStringVars(void) {return _stringVars;}
    std::map<std::string, TypeData>& getTypeDatas(void) {return _typeDatas;}
    std::vector<InternalLabel>& getInternalLabels(void) {return _internalLabels;}
    std::vector<InternalLabel>& getDiscardedLabels(void) {return _discardedLabels;}
    std::vector<std::string>& getOutput(void) {return _output;}
    std::vector<std::string>& getRuntime(void) {return _runtime;}
    std::vector<DefDataByte>& getDefDataBytes(void) {return _defDataBytes;}
    std::vector<DefDataWord>& getDefDataWords(void) {return _defDataWords;}
    std::vector<DefDataImage>& getDefDataImages(void) {return _defDataImages;}
    std::vector<DefDataLoaderImageChunk>& getDefDataLoaderImageChunks(void) {return _defDataLoaderImageChunks;}

    std::map<int, DefDataMidi>& getDefDataMidis(void) {return _defDataMidis;}
    std::map<int, DefDataOpen>& getDefDataOpens(void) {return _defDataOpens;}

    std::map<int, DefDataSprite>& getDefDataSprites(void) {return _defDataSprites;}
    SpritesAddrLut& getSpritesAddrLut(void) {return _spritesAddrLut;}

    std::map<int, DefDataFont>& getDefDataFonts(void) {return _defDataFonts;}
    FontsAddrLut& getFontsAddrLut(void) {return _fontsAddrLut;}

    std::map<std::string, DefFunction>& getDefFunctions(void) {return _defFunctions;}
    std::vector<std::unique_ptr<DataObject>>& getDataObjects(void) {return _dataObjects;}

    std::map<std::string, MacroIndexEntry>& getMacroIndexEntries(void) {return _macroIndexEntries;}

    std::stack<ForNextData>& getForNextDataStack(void) {return _forNextDataStack;}
    std::stack<ElseIfData>& getElseIfDataStack(void) {return _elseIfDataStack;}
    std::stack<WhileWendData>& getWhileWendDataStack(void) {return _whileWendDataStack;}
    std::stack<RepeatUntilData>& getRepeatUntilDataStack(void) {return _repeatUntilDataStack;}

    std::map<std::string, CallData>& getCallDataMap(void) {return _callDataMap;}

    std::stack<ProcData>& getProcDataStack(void) {return _procDataStack;}
    std::map<std::string, ProcData>& getProcDataMap(void) {return _procDataMap;}

    int getNumNumericLabels(void)
    {
        int numLabels = 0;
        for(int i=0; i<int(_labels.size()); i++)
        {
            if(_labels[i]._numeric) numLabels++;
        }

        return numLabels;
    }

    // Vertical blank interrupt uses 0x30-0x35 for context save/restore, (vPC,vAC,vCpuSelect)
    bool moveVblankVars(void)
    {
        if(_codeRomType < Cpu::ROMv5a) return false;

        if(_userVarsAddr < USER_VAR_START + Int16*3)
        {
            _userVarStart = USER_VAR_START + Int16*3;
            _userVarsAddr = _userVarStart;
        }

        for(int i=0; i<int(_integerVars.size()); i++)
        {
            if(_integerVars[i]._address == USER_VAR_START  ||
               _integerVars[i]._address == USER_VAR_START + Int16 ||
              _integerVars[i]._address == USER_VAR_START + 2*Int16 )
            {
                //emitVcpuAsm("LDW", Expression::wordToHexString(_integerVars[i]._address), false);
                //emitVcpuAsm("STW", Expression::wordToHexString(_userVarsAddr), false);
                _integerVars[i]._address = _userVarsAddr;
                _userVarsAddr += Int16;
            }
        }

        return true;
    }

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

    bool setBuildPath(const std::string& buildpath, const std::string& filepath)
    {
        if(buildpath.size() == 0) return false;

        // Prepend current file path to relative paths
        if(buildpath.find(":") == std::string::npos  &&  buildpath[0] != '/')
        {
            std::string path = filepath;
            size_t slash = path.find_last_of("\\/");
            path = (slash != std::string::npos) ? path.substr(0, slash) : ".";
            std::string includePath = path + "/" + buildpath;
            Assembler::setIncludePath(includePath);
        }
        else
        {
            Assembler::setIncludePath(buildpath);
        }

        setRuntimePath(buildpath);

        return true;
    }

    bool initialise(void)
    {
        return true;
    }


    bool readInputFile(std::ifstream& infile, const std::string& filename, std::vector<Input>& input, int& numLines)
    {
        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::readInputFile() : Failed to open file '%s'\n", filename.c_str());
            return false;
        }

        // Read input .gbas file
        std::string lineToken;
        while(!infile.eof())
        {
            std::getline(infile, lineToken);
            Input inp = {true, lineToken};

            // Compound statement must span multiple lines
            size_t lbra, rbra;
            lineToken = Expression::removeCommentsNotInStrings(lineToken);
            if(Expression::findMatchingBrackets(lineToken, 0, lbra, rbra, '{'))
            {
                fprintf(stderr, "Compiler::readInputFile() : '%s:%d' : compound statement must span multiple lines\n", filename.c_str(), numLines+1);
                return false;
            }

            // Append lines into a compound statement
            int compoundLength = 0;
            if(Expression::findNonStringBracket(lineToken, '{') != lineToken.end())
            {
                do
                {
                    std::getline(infile, lineToken);
                    if(infile.eof())
                    {
                        fprintf(stderr, "Compiler::readInputFile() : '%s:%d' : missing '}'\n", filename.c_str(), numLines+1);
                        return false;
                    }

                    inp._text += lineToken;
                    lineToken = Expression::removeCommentsNotInStrings(lineToken);
                    compoundLength++;
                }
                while(Expression::findNonStringBracket(lineToken, '}') == lineToken.end());

                // Remove compound statement delimiters
                Expression::stripChars(inp._text, "{}");
            }

            // Add empty lines for compound statement to correct line numbers
            for(int i=0; i<compoundLength; i++)
            {
                input.push_back({true, ""});
                numLines++;
            }

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Compiler::readInputFile() : '%s:%d' : bad line '%s'\n", filename.c_str(), numLines+1, lineToken.c_str());
                return false;
            }

            input.push_back(inp);
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
        // Valid chars are alpha, '@' and '#'
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

        // Within a PROC/ENDPROC pair
        if(getProcDataStack().size() == 1)
        {
            // Top of stack for current PROC
            ProcData procData = getProcDataStack().top();

            // Translate to local var names
            if(procData._localVarNameMap.find(varName) != procData._localVarNameMap.end())
            {
                varName = procData._localVarNameMap[varName];
            }
        }

        // Search for var name
        for(int i=0; i<int(_integerVars.size()); i++)
        {
            if(_integerVars[i]._name == varName) return i;
        }

        return -1;
    }

    int findVar(std::string& varName, std::string& oldName, bool subAlpha)
    {
        // Valid chars are alpha and 'address of'
        if(subAlpha) varName = Expression::getSubAlpha(varName);
        oldName = varName;

        // Within a PROC/ENDPROC pair
        if(getProcDataStack().size() == 1)
        {
            // Top of stack for current PROC
            ProcData procData = getProcDataStack().top();

            // Translate to local var names
            if(procData._localVarNameMap.find(varName) != procData._localVarNameMap.end())
            {
                varName = procData._localVarNameMap[varName];
            }
        }

        // Search for var name
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

    // Global int vars
    void createIntVar(const std::string& varName, int16_t data, int16_t init, CodeLine& codeLine, int codeLineIndex, bool containsVars, int& varIndex)
    {
        // Create var
        varIndex = int(_integerVars.size());
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;

        std::vector<uint16_t> arrSizes;
        std::vector<int16_t> arrInits;
        std::vector<std::vector<uint16_t>> arrAddrs;
        std::vector<uint16_t> arrLut;
        IntegerVar integerVar = {data, init, _userVarsAddr, varName, varName, codeLineIndex, VarInt16, Int16, arrSizes, arrInits, arrAddrs, arrLut};
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
        _userVarsAddr += Int16;
        if(_userVarsAddr >= USER_VAR_END)
        {
            _userVarsAddr = _userVarStart;
            fprintf(stderr, "Compiler::createIntVar() : '%s:%d' : warning, you have used the maximum number of page zero global variables : %s\n", codeLine._moduleName.c_str(), codeLineIndex, codeLine._text.c_str()); 
        }
    }

    // Proc/Func local and param int vars
    void createProcIntVar(const std::string& varName, int16_t data, int16_t init, CodeLine& codeLine, int codeLineIndex, bool containsVars, uint16_t address, int& varIndex)
    {
        // Create var
        varIndex = int(_integerVars.size());
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;

        std::vector<uint16_t> arrSizes;
        std::vector<int16_t> arrInits;
        std::vector<std::vector<uint16_t>> arrAddrs;
        std::vector<uint16_t> arrLut;
        IntegerVar integerVar = {data, init, address, varName, varName, codeLineIndex, VarInt16, Int16, arrSizes, arrInits, arrAddrs, arrLut};
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
    }

    // Global int array vars
    void createArrIntVar(const std::string& varName, int16_t data, int16_t init, CodeLine& codeLine, int codeLineIndex, bool containsVars, bool isInit, int& varIndex, VarType varType, int intSize,
                         uint16_t address, std::vector<uint16_t>& arrSizes, const std::vector<int16_t>& arrInits, std::vector<std::vector<uint16_t>>& arrAddrs, std::vector<uint16_t>& arrLut)
    {
        // Create var
        varIndex = int(_integerVars.size());
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;

        uint16_t varAddr = (varType == VarInt16) ? _userVarsAddr : address;
        IntegerVar integerVar = {data, init, varAddr, varName, varName, codeLineIndex, varType, intSize, arrSizes, arrInits, arrAddrs, arrLut, isInit};
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
        std::vector<std::string> tokens = Expression::tokenise(varName, ' ');
        if(tokens.size() > 1) return VarError;
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
        if(varName.find("$") != std::string::npos) return VarError;

        std::string constName = varName;
        if(Compiler::findConst(constName) >= 0)
        {
            fprintf(stderr, "Keywords::createCodeVar() : '%s:%d' : const '%s' already exists : %s\n", codeLine._moduleName.c_str(), codeLineIndex, constName.c_str(), codeLine._text.c_str());
            return VarExistsAsConst;
        }

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
        Expression::TokType tokType = Expression::isStrNameValid(strName);
        if(tokType == Expression::Invalid) return VarError;
        strIndex = findStr(strName);
        if(strIndex != -1)
        {
            codeLine._varIndex = strIndex;
            switch(tokType)
            {
                case Expression::Variable: codeLine._varType = VarStr;  break;
                case Expression::Array:    codeLine._varType = VarStr2; break;

                default: return VarError;
            }

            return VarExists;
        }

        // String data validation
        std::string strData = codeLine._tokens[1];
        Expression::stripNonStringWhitespace(strData);
        std::string strKeyword = strData;

        // Check for string keywords
        if(Functions::getStringFunctions().find(Expression::strToUpper(strKeyword)) != Functions::getStringFunctions().end())
        {
            strData = "";
        }
        // Check for constant string
        else
        {
            if(Expression::isStringValid(strData))
            {
                // Strip quotes
                strData.erase(0, 1);
                strData.erase(strData.size()-1, 1);
            }
            // Check for assignment of another variable
            else
            {
                if(strData.find("$") == std::string::npos) return VarError;
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
        std::vector<std::string> tokens = Expression::tokeniseMulti(stripped, "-+/*%&<>=();, ", false);

        // Check for pragmas
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if(Pragmas::getPragmas().find(token) != Pragmas::getPragmas().end())
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

        // Check for inbuilt functions and user functions
        for(int i=0; i<int(tokens.size()); i++)
        {
            std::string token = tokens[i];
            Expression::strToUpper(token);
            if((Functions::getFunctions().find(token) != Functions::getFunctions().end())  ||  (getDefFunctions().find(token) != getDefFunctions().end()))
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
            if(Functions::getStringFunctions().find(token) != Functions::getStringFunctions().end())
            {
                // If first token is a string keyword then use optimised prints, (don't create strings)
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
                if(_constants[constIndex]._varType == ConstInt16)
                {
                    expressionType |= Expression::HasIntConsts;
                    break;
                }
                if(_constants[constIndex]._varType == ConstStr)
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
                if(_integerVars[varIndex]._varType == Var1Arr8   ||  _integerVars[varIndex]._varType == Var2Arr8   ||  _integerVars[varIndex]._varType == Var3Arr8  ||
                   _integerVars[varIndex]._varType == Var1Arr16  ||  _integerVars[varIndex]._varType == Var2Arr16  ||  _integerVars[varIndex]._varType == Var3Arr16)
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
            if(strIndex != -1  &&  !_stringVars[strIndex]._constant  &&  tokens[i][0] != '@') // 'address of' operator returns numbers)
            {
                expressionType |= Expression::HasStrVars;
                break;
            }
        }

        // Check for operators for non string expressions
        bool isStrExpression = ((expressionType >= Expression::HasStrings)  &&  (expressionType <= Expression::IsStringExpression));
        if(!isStrExpression)
        {
            if(stripped.find_first_of("-+/*%<>=") != std::string::npos) expressionType |= Expression::HasOperators;
            std::string mod = stripped;
            Expression::strToUpper(mod);
            for(int i=0; i<int(Operators::getOperators().size()); i++)
            {
                if(mod.find(Operators::getOperators()[i]) != std::string::npos) expressionType |= Expression::HasOperators;
            }
        }

        // Check for operators for string expressions
        if(isStrExpression)
        {
            if(stripped.find_first_of("+<>=") != std::string::npos) expressionType |= Expression::HasOperators;
        }

        return expressionType;
    }

    void updateIntVar(int16_t data, CodeLine& codeLine, int varIndex, bool containsVars)
    {
        codeLine._containsVars = containsVars;
        codeLine._varIndex = varIndex;
        codeLine._varType = VarInt16;
        _integerVars[varIndex]._data = data;
    }

    bool createAsmLine(const std::string& code)
    {
        std::string line, vasm = code;
        CodeLine codeLine = _codeLines[_currentCodeLineIndex];

        Expression::trimWhitespace(vasm);
        std::vector<std::string> tokens = Expression::tokenise(vasm, ' ', false);
        if(tokens.size() < 1)
        {
            fprintf(stderr, "Compiler::createAsmLine() : '%s:%d' : vASM syntax error, missing opcode : %s\n", codeLine._moduleName.c_str(), _currentCodeLineIndex, codeLine._text.c_str());
            return false;
        }

        std::string opcodeStr = tokens[0];
        std::string operandStr = "";
        for(int i=1; i<int(tokens.size()); i++) operandStr += tokens[i];

        Expression::stripWhitespace(opcodeStr);
        Expression::strToUpper(opcodeStr);
        Expression::stripWhitespace(operandStr);

        int vasmSize = createVcpuAsm(opcodeStr, operandStr, int(_codeLines.size()), line);
        if(vasmSize == 0  &&  opcodeStr != "_BREAKPOINT_")
        {
            // This is a bit of a hack, but unfortunately needed as createAsmLine is called before the keyword ENDASM can be processed
            if(opcodeStr == "ENDASM") return true;

            fprintf(stderr, "Compiler::createAsmLine() : '%s:%d' : vASM syntax error, undefined opcode : %s\n", codeLine._moduleName.c_str(), _currentCodeLineIndex, codeLine._text.c_str());
            return false;
        }

        _codeLines[_currentCodeLineIndex]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, operandStr, line, _nextInternalLabel, false, vasmSize});
        _codeLines[_currentCodeLineIndex]._vasmSize += vasmSize;
        _codeLines[_currentCodeLineIndex]._dontParse = true;
        _nextInternalLabel = "";

        return true;
    }

    bool createCodeLine(const std::string& code, int codeLineStart, int labelIndex, int varIndex, Expression::Int16Byte int16Byte, bool vars, CodeLine& codeLine, const std::string& moduleName)
    {
        // Create expression string
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
        std::string text = code.substr(codeLineStart, code.size() - (codeLineStart));
        Expression::trimWhitespace(text);
        std::string codeText = Expression::collapseWhitespaceNotStrings(text);
        codeText = Expression::removeCommentsNotInStrings(codeText);
        std::vector<size_t> offsets;
        std::vector<std::string> tokens = Expression::tokeniseLineOffsets(codeText, " (),=", offsets);
        codeLine = {text, codeText, tokens, offsets, vasm, expression, onGotoGosubLut, strConcatLut, inputLut, 0, labelIndex, varIndex, VarInt16, int16Byte, vars, false, false, moduleName};
        Expression::operatorReduction(codeLine._expression);

        // ASM keyword
        if(_codeIsAsm)
        {
            return createAsmLine(code);
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
             // _tempVarSize bytes of temporary expression variables, defaults to 8 bytes, (4 temporary expression variables) 
            _tempVarStart += 2;
            if(_tempVarStart >= TEMP_VAR_START + _tempVarSize) _tempVarStart = TEMP_VAR_START;
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
        std::string filename = (_codeRomType < Cpu::ROMv5a) ? "/macros.i" : "/macros_ROMv5a.i";
        filename = Assembler::getIncludePath() + filename;
        std::ifstream infile(filename);

        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::initialiseMacros() : failed to open file '%s'\n", filename.c_str());
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

            fprintf(stderr, "Compiler::initialiseMacros() : '%s:%d' : %%MACRO '%s' is missing a %%ENDM\n", filename.c_str(), macroIndex, macroName.c_str());
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

    bool initialiseCode(void)
    {
        // Entry point initialisation
        Label label;
        createLabel(_vasmPC, "_entryPoint_", 0, label, false, false, false, false);

        // BASIC INIT
        CodeLine codeLine;
        if(createCodeLine("INIT", 0, 0, -1, Expression::Int16Both, false, codeLine)) _codeLines.push_back(codeLine);

        // Rom check, (always used for versions greater than ROMv1)
        if(_codeRomType > Cpu::ROMv1)
        {
            emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(_codeRomType)), false, 0);
            emitVcpuAsm("STW", "romType", false, 0);
            emitVcpuAsm("%RomCheck", "", false, 0);
        }

        // Initialise
        emitVcpuAsm("%Initialise", "", false, 0);

        // Realtime proc, relational operators and array helpers are CALLS from zero page for efficiency
        if(_codeRomType < Cpu::ROMv5a)
        {
            // Linked only when actually used, the semi-colon is removed by enableSysInitFunc()
            for(int i=0; i<int(_sysInitNames.size()); i++)
            {
                emitVcpuAsm(";%" + _sysInitNames[i], "", false, 0);
            }
        }

        return true;
    }

    void finaliseCode(void)
    {
        CodeLine codeLine;

        // Add END to code
        if(_codeLines.size())
        {
            bool foundEnd = false;
            for(int i=0; i<int(_codeLines[_codeLines.size() - 1]._tokens.size()); i++)
            {
                std::string token = _codeLines[_codeLines.size() - 1]._tokens[i];
                Expression::strToUpper(token);
                if(token == "END")
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

    std::pair<int, int> emitVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, bool nextTempVar, int codeLineIdx, const std::string& internalLabel, bool pageJump)
    {
        if(codeLineIdx == -1) codeLineIdx = _currentCodeLineIndex;

        std::string line;
        int vasmSize = createVcpuAsm(opcodeStr, operandStr, codeLineIdx, line);

        // NEXT and THEN don't know where the next vasm instruction is, so they use _nextInternalLabel, (which has priority over internalLabel)
        std::string intLabel = (_nextInternalLabel.size()) ? _nextInternalLabel : internalLabel;

        // Discarded labels are replaced correctly later in outputLabels()
        if(_nextInternalLabel.size()  &&  internalLabel.size()) _discardedLabels.push_back({_vasmPC, internalLabel});

        _codeLines[codeLineIdx]._vasm.push_back({uint16_t(_vasmPC - vasmSize), opcodeStr, operandStr, line, intLabel, pageJump, vasmSize});
        _codeLines[codeLineIdx]._vasmSize += vasmSize;

        if(nextTempVar) getNextTempVar();
        _nextInternalLabel = "";

        // Return current vasm instruction index
        return std::make_pair(codeLineIdx, int(_codeLines[codeLineIdx]._vasm.size()) - 1);
    }

    void emitVcpuPreProcessingCmd(const std::string& cmdStr)
    {
        _codeLines[0]._vasm.push_back({_userCodeStart, cmdStr, "", cmdStr, "", false, 0});
    }

    void createVcpuAsmLabel(int codeLineIdxBra, int vcpuAsmBra, int codeLineIdxDst, int vcpuAsmDst, const std::string& label)
    {
        std::string opcode = _codeLines[codeLineIdxBra]._vasm[vcpuAsmBra]._opcode;
        _codeLines[codeLineIdxBra]._vasm[vcpuAsmBra]._code = opcode + std::string(OPCODE_TRUNC_SIZE - opcode.size(), ' ') + label;
        _codeLines[codeLineIdxBra]._vasm[vcpuAsmBra]._operand = label;
        _codeLines[codeLineIdxDst]._vasm[vcpuAsmDst]._internalLabel = label;
    }

    // Array1d LDW expression parser
    uint32_t parseArray1dVarExpression(int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        if(!Expression::parse(expression, codeLineIndex, numeric)) return Expression::IsInvalid;

        int varIndex, constIndex, strIndex;
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);
        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasFunctions)  ||
           (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
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
    bool writeArray1d(CodeLine& codeLine, int codeLineIndex, size_t lbra, size_t rbra, int intSize, uint16_t arrayPtr)
    {
        // Array index from expression
        Expression::Numeric arrIndex;
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        uint32_t expressionType = parseArray1dVarExpression(codeLineIndex, arrText, arrIndex);
        if(expressionType == Expression::IsInvalid) return false;

        // Constant index
        if(!(expressionType & Expression::HasIntVars))
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + int16_t(std::lround(arrIndex._value))*uint16_t(intSize)), false, codeLineIndex);
            emitVcpuAsm("STW",  "memAddr", false, codeLineIndex);
            emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);
            switch(intSize)
            {
                case Int8:
                {
                    emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);
                }
                break;

                case Int16:
                {
                    switch(codeLine._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);                                                       break;
                        case Expression::Int16High: emitVcpuAsm("INC",  "memAddr", false, codeLineIndex); emitVcpuAsm("POKE", "memAddr", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("DOKE", "memAddr", false, codeLineIndex);                                                       break;

                        default: break;
                    }
                }
                break;

                default: break;
            }
        }
        else
        {
            emitVcpuAsm("STW",  "memIndex0", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);

            switch(intSize)
            {
                case Int8:
                {
                    emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
                    emitVcpuAsm("STW",  "memAddr", false, codeLineIndex);
                    emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);
                    emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);
                }
                break;

                case Int16:
                {
                    emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
                    emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
                    emitVcpuAsm("STW",  "memAddr", false, codeLineIndex);
                    emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);

                    switch(codeLine._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);                                                       break;
                        case Expression::Int16High: emitVcpuAsm("INC",  "memAddr", false, codeLineIndex); emitVcpuAsm("POKE", "memAddr", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("DOKE", "memAddr", false, codeLineIndex);                                                       break;

                        default: break;
                    }
                }
                break;

                default: break;
            }
        }

        return true;
    }

    // ArrayXd LDW expression parser
    uint32_t parseArrayXdVarExpression(int codeLineIndex, std::string& expression, Expression::Numeric& numeric)
    {
        if(!Expression::parse(expression, codeLineIndex, numeric)) return Expression::IsInvalid;

        int varIndex, constIndex, strIndex;
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);
        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))  ||  (expressionType & Expression::HasFunctions)  ||
           (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords))
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
            emitVcpuAsm("LDI", std::to_string(uint8_t(std::lround(numeric._value))), false, codeLineIndex);
        }

        return expressionType;
    }
    bool writeArray2d(CodeLine& codeLine, int codeLineIndex, size_t lbra, size_t rbra, int intSize, uint16_t arrayPtr)
    {
        // Array index from expression
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        std::vector<std::string> indexTokens = Expression::tokenise(arrText, ',', true);
        if(indexTokens.size() != 2)
        {
            fprintf(stderr, "Compiler::writeArray2d() : '%s:%d' : number of dimensions must be equal to 2, found %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, int(indexTokens.size()), codeLine._text.c_str());
            return false;
        }

        for(int i=0; i<int(indexTokens.size()); i++)
        {
            Expression::Numeric arrIndex;
            std::string indexToken = indexTokens[i];
            Expression::stripWhitespace(indexToken);
            if(parseArrayXdVarExpression(codeLineIndex, indexToken, arrIndex) == Expression::IsInvalid) return false;
            emitVcpuAsm("STW", "memIndex" + std::to_string(i), false, codeLineIndex);
        }

        emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
        switch(intSize)
        {
            case Int8:
            {
                (getCodeRomType() >= Cpu::ROMv5a) ? emitVcpuAsm("CALLI", "convert8Arr2d", false, codeLineIndex) : emitVcpuAsm("CALL", "convert8Arr2dAddr", false, codeLineIndex);
                emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);
                emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);
            }
            break;

            case Int16:
            {
                (getCodeRomType() >= Cpu::ROMv5a) ? emitVcpuAsm("CALLI", "convert16Arr2d", false, codeLineIndex) : emitVcpuAsm("CALL", "convert16Arr2dAddr", false, codeLineIndex);
                emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);

                switch(codeLine._int16Byte)
                {
                    case Expression::Int16Low:  emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);                                                       break;
                    case Expression::Int16High: emitVcpuAsm("INC",  "memAddr", false, codeLineIndex); emitVcpuAsm("POKE", "memAddr", false, codeLineIndex); break;
                    case Expression::Int16Both: emitVcpuAsm("DOKE", "memAddr", false, codeLineIndex);                                                       break;

                    default: break;
                }
            }
            break;

            default: break;
        }

        return true;
    }

    bool writeArray3d(CodeLine& codeLine, int codeLineIndex, size_t lbra, size_t rbra, int intSize, uint16_t arrayPtr)
    {
        // Array index from expression
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        std::vector<std::string> indexTokens = Expression::tokenise(arrText, ',', true);
        if(indexTokens.size() != 3)
        {
            fprintf(stderr, "Compiler::writeArray3d() : '%s:%d' : number of dimensions must be equal to 3, found %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, int(indexTokens.size()), codeLine._text.c_str());
            return false;
        }

        for(int i=0; i<int(indexTokens.size()); i++)
        {
            Expression::Numeric arrIndex;
            std::string indexToken = indexTokens[i];
            Expression::stripWhitespace(indexToken);
            if(parseArrayXdVarExpression(codeLineIndex, indexToken, arrIndex) == Expression::IsInvalid) return false;
            emitVcpuAsm("STW",  "memIndex" + std::to_string(i), false, codeLineIndex);
        }

        emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
        switch(intSize)
        {
            case Int8:
            {
                (getCodeRomType() >= Cpu::ROMv5a) ? emitVcpuAsm("CALLI", "convert8Arr3d", false, codeLineIndex) : emitVcpuAsm("CALL", "convert8Arr3dAddr", false, codeLineIndex);
                emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);
                emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);
            }
            break;

            case Int16:
            {
                (getCodeRomType() >= Cpu::ROMv5a) ? emitVcpuAsm("CALLI", "convert16Arr3d", false, codeLineIndex) : emitVcpuAsm("CALL", "convert16Arr3dAddr", false, codeLineIndex);
                emitVcpuAsm("LDW",  "memValue", false, codeLineIndex);

                switch(codeLine._int16Byte)
                {
                    case Expression::Int16Low:  emitVcpuAsm("POKE", "memAddr", false, codeLineIndex);                                                       break;
                    case Expression::Int16High: emitVcpuAsm("INC",  "memAddr", false, codeLineIndex); emitVcpuAsm("POKE", "memAddr", false, codeLineIndex); break;
                    case Expression::Int16Both: emitVcpuAsm("DOKE", "memAddr", false, codeLineIndex);                                                       break;

                    default: break;
                }
            }
            break;

            default: break;
        }

        return true;
    }

    bool writeArrayVar(CodeLine& codeLine, int codeLineIndex, int varIndex)
    {
        // Array var?
        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, 0, lbra, rbra)) return false;
        size_t equals = codeLine._code.find("=");
        if(equals == std::string::npos  ||  equals < rbra) return false;

        // Previous expression result
        emitVcpuAsm("STW", "memValue", false, codeLineIndex);

        int intSize = _integerVars[varIndex]._intSize;
        uint16_t arrayPtr = _integerVars[varIndex]._address;
        Compiler::VarType varType = _integerVars[varIndex]._varType;

            
        switch(varType)
        {
            case VarType::Var1Arr8:
            case VarType::Var1Arr16: writeArray1d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            case VarType::Var2Arr8:
            case VarType::Var2Arr16: writeArray2d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            case VarType::Var3Arr8:
            case VarType::Var3Arr16: writeArray3d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            default: break;
        }

        return true;
    }

    void writeArrayVarNoAssign(CodeLine& codeLine, int codeLineIndex, int varIndex)
    {
        // Array var? (expects lbra and rbra to have previously been validated!)
        size_t lbra, rbra;
        Expression::findMatchingBrackets(codeLine._code, 0, lbra, rbra);

        // Previous expression result
        emitVcpuAsm("STW", "memValue", false, codeLineIndex);

        int intSize = _integerVars[varIndex]._intSize;
        uint16_t arrayPtr = _integerVars[varIndex]._address;
        VarType varType = _integerVars[varIndex]._varType;

        switch(varType)
        {
            case VarType::Var1Arr8:
            case VarType::Var1Arr16: writeArray1d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            case VarType::Var2Arr8:
            case VarType::Var2Arr16: writeArray2d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            case VarType::Var3Arr8:
            case VarType::Var3Arr16: writeArray3d(codeLine, codeLineIndex, lbra, rbra, intSize, arrayPtr); break;

            default: break;
        }
    }

    bool writeArrayStr(CodeLine& codeLine, int codeLineIndex, Expression::Numeric& numeric, int strIndex, uint16_t srcAddr)
    {
        // Array var?
        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, 0, lbra, rbra)) return false;
        size_t equals = codeLine._code.find("=");
        if(equals == std::string::npos  ||  equals < rbra) return false;

        uint16_t arrayPtr = _stringVars[strIndex]._address;
        std::string arrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));

        // Array index from expression
        Expression::Numeric arrIndex;
        uint32_t expressionType = parseArray1dVarExpression(codeLineIndex, arrText, arrIndex);
        if(expressionType == Expression::IsInvalid) return false;

        // Constant index
        if(!(expressionType & Expression::HasIntVars))
        {
            emitStringAddress(numeric, srcAddr);
            emitVcpuAsm("STW", "strSrcAddr", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + int16_t(std::lround(arrIndex._value))*2), false, codeLineIndex);
        }
        // Variable index generated by parseArray1dVarExpression()
        else
        {
            emitVcpuAsm("STW",  "memIndex0", false, codeLineIndex);
            emitStringAddress(numeric, srcAddr);
            emitVcpuAsm("STW", "strSrcAddr", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
            emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
            emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
        }

        emitVcpuAsm("DEEK", "", false, codeLineIndex);
        emitVcpuAsm("%StringCopy", "", false, codeLineIndex);

        return true;
    }

    bool writeArrayStrNoAssign(std::string& arrText, int codeLineIndex, int strIndex)
    {
        uint16_t arrayPtr = _stringVars[strIndex]._address;

        // Array index from expression
        Expression::Numeric arrIndex;
        uint32_t expressionType = parseArray1dVarExpression(codeLineIndex, arrText, arrIndex);
        if(expressionType == Expression::IsInvalid) return false;

        // Constant index
        if(!(expressionType & Expression::HasIntVars))
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr + int16_t(std::lround(arrIndex._value))*2), false, codeLineIndex);
        }
        // Variable index generated by parseArray1dVarExpression()
        else
        {
            emitVcpuAsm("STW",  "memIndex0", false, codeLineIndex);
            emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false, codeLineIndex);
            emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
            emitVcpuAsm("ADDW", "memIndex0", false, codeLineIndex);
        }

        emitVcpuAsm("DEEK", "", false, codeLineIndex);

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
                if(opcode == "SUBW")
                {
                    emitVcpuAsm("STW", "register14", false);
                    emitVcpuAsm("LD", operand, false);
                    emitVcpuAsm("STW", "register15", false);
                    emitVcpuAsm("LDW", "register14", false);
                    emitVcpuAsm("SUBW", "register15", nextTempVar);
                    return;
                }
                else
                {
                    emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(getTempVarStart())), false);
                    emitVcpuAsm("LD", operand, false);
                    emitVcpuAsm(opcode, Expression::byteToHexString(uint8_t(getTempVarStart())), nextTempVar);
                    return;
                }
            }
            break;

            case Expression::Int16High:
            {
                if(opcode == "SUBW")
                {
                    emitVcpuAsm("STW", "register14", false);
                    emitVcpuAsm("LD", operand + " + 1", false);
                    emitVcpuAsm("STW", "register15", false);
                    emitVcpuAsm("LDW", "register14", false);
                    emitVcpuAsm("SUBW", "register15", nextTempVar);
                    return;
                }
                else
                {
                    emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(getTempVarStart())), false);
                    emitVcpuAsm("LD", operand + " + 1", false);
                    emitVcpuAsm(opcode, Expression::byteToHexString(uint8_t(getTempVarStart())), nextTempVar);
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
        else if(opcode == "STW")
        {
            switch(numeric._int16Byte)
            {
                case Expression::Int16Both: opcode = "STW";                    break;
                case Expression::Int16Low:  opcode = "ST";                     break;
                case Expression::Int16High: opcode = "ST";  operand += " + 1"; break;

                default: break;
            }

            emitVcpuAsm(opcode, operand, nextTempVar);
            return true;
        }

        // ADDW, SUBW, ANDW, ORW, XORW
        handleInt16Byte(opcode, operand, numeric, nextTempVar);
        return true;
    }

    // Static expression parser
    OperandType parseStaticExpression(int codeLineIndex, std::string& expression, std::string& operand, Expression::Numeric& numeric)
    {
        if(!Expression::parse(expression, codeLineIndex, numeric)) return OperandInvalid;

        int varIndex, constIndex, strIndex;
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);
        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))       ||  
            ((expressionType & Expression::HasStrVars)  &&  (expressionType & Expression::HasOperators))      ||
            (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
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
        if(!Expression::parse(expression, codeLineIndex, numeric)) return Expression::IsInvalid;

        int varIndex, constIndex, strIndex;
        uint32_t expressionType = isExpression(expression, varIndex, constIndex, strIndex);
        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))       ||  
            ((expressionType & Expression::HasStrVars)  &&  (expressionType & Expression::HasOperators))      ||
            (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
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

        if(((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))       ||  
            ((expressionType & Expression::HasStrVars)  &&  (expressionType & Expression::HasOperators))      ||
            (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasStringKeywords)  ||  (expressionType & Expression::HasFunctions))
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
        else if(expressionType & Expression::HasStrVars)
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(_stringVars[strIndex]._address), false, codeLineIndex);
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

    bool checkForGosubLabel(const std::string& moduleName, const std::string& code, int lineNumber)
    {
        std::vector<std::string> tokens = Expression::tokeniseLine(code, " :=");
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(Expression::strToUpper(tokens[i]) == "GOSUB")
            {
                if(i+1 >= int(tokens.size()))
                {
                    fprintf(stderr, "Compiler::checkForGosubLabel() : '%s:%d' : missing label after GOSUB in '%s'\n", moduleName.c_str(), lineNumber + 1, code.c_str());
                    return false;
                }
                _gosubLabels.push_back(tokens[i+1]);
            }
        }

        return true;
    }

    LabelResult checkForLabel(const std::string& moduleName, std::string& code, int lineNumber)
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
                fprintf(stderr, "Compiler::checkForLabel() : '%s:%d' : line number cannot exist on its own in '%s'\n", moduleName.c_str(), lineNumber + 1, code.c_str());
                return LabelError;
            }

            if(code[0] == '0')
            {
                fprintf(stderr, "Compiler::checkForLabel() : '%s:%d' : line number cannot be zero or start with zero in '%s'\n", moduleName.c_str(), lineNumber + 1, code.c_str());
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
            if(createCodeLine(code, int(space + 1), _currentLabelIndex, -1, Expression::Int16Both, false, codeLine, moduleName)) _codeLines.push_back(codeLine);

            return LabelFound;
        }

        // Text label
        size_t colon = code.find_first_of(":");
        if(colon != std::string::npos)
        {
            std::string labelName = code.substr(0, colon);
            if(Expression::isLabNameValid(labelName))
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
                    if(labelCode.size() > 2  &&  createCodeLine(code, int(colon  + 1), _currentLabelIndex, -1, Expression::Int16Both, false, codeLine, moduleName))
                    {
                        validCode = true;
                    }
                }

                // Check for label without code, (create dummy codeLine for labels without code, so that codeLine count remains consistant)
                if(!validCode)
                {
                    _nonNumericLabelIndex = _currentLabelIndex;
                    createCodeLine("'" + labelName + ":", 0, -1, -1, Expression::Int16Both, false, codeLine, moduleName);
                }

                _codeLines.push_back(codeLine);

                return LabelFound;
            }
        }

        // Non label code, (except if previous line had a non numeric label without code)
        if(createCodeLine(code, 0, _nonNumericLabelIndex, -1, Expression::Int16Both, false, codeLine, moduleName))
        {
            // Don't reset _nonNumericLabelIndex until it has been assigned to a valid line
            if(codeLine._code.size() >= 2) _nonNumericLabelIndex = -1;
            _codeLines.push_back(codeLine);
        }

        return LabelNotFound;
    }

    bool parsePragmas(std::vector<Input>& input, int numLines)
    {
        // Parse each line of input for pragmas, (pragmas are case sensitive)
        for(int j=0; j<numLines; j++)
        {
            std::string inputText = input[j]._text;
            inputText = Expression::removeCommentsNotInStrings(inputText);
            Pragmas::PragmaResult pragmaResult = Pragmas::handlePragmas(inputText, j);
            switch(pragmaResult)
            {
                case Pragmas::PragmaFound: input[j]._parse = false; break;
                case Pragmas::PragmaError: return false;            break;

                default: break;
            }
        }

        _vasmPC = _userCodeStart;

        return true;
    }

    bool loadModule(const std::string& moduleName, int codeLineIndex, std::vector<Input>& input, int& numLines)
    {
        std::string moduleFile = moduleName;

        // Strip non string whitespace and quotes
        Expression::stripNonStringWhitespace(moduleFile);
        if(!Expression::isStringValid(moduleFile))
        {
            fprintf(stderr, "Compiler::loadModule() : %s is not a valid module string, use 'MODULE \"<module name>\"'\n", moduleName.c_str());
            return false;
        }

        // Remove quotes
        moduleFile.erase(0, 1);
        moduleFile.erase(moduleFile.size()-1, 1);

        // Prepend source code path
        std::string path = Loader::getFilePath();
        size_t slash = path.find_last_of("\\/");
        path = (slash != std::string::npos) ? path.substr(0, slash) : ".";
        std::string modulePath = path + "/" + moduleFile;

        // Open module
        std::ifstream infile(modulePath);
        if(!infile.is_open())
        {
            fprintf(stderr, "Compiler::loadModule() : failed to open module %s\n", moduleName.c_str());
            return false;
        }

        //fprintf(stderr, "Keywords::MODULE() : module path = %s\n", modulePath.c_str());

        // Extract module name from module filename
        slash = moduleFile.find_last_of("\\/");
        if(slash != std::string::npos) moduleFile = moduleFile.substr(slash + 1);
        size_t period = moduleFile.find_last_of(".");
        if(period != std::string::npos) moduleFile = moduleFile.substr(0, period);

        //fprintf(stderr, "Keywords::MODULE() : module name = %s\n : %d", moduleFile.c_str(), int(Compiler::getCodeLines().size()));

        for(int i=0; i<int(_moduleLines.size()); i++)
        {
            if(moduleFile == _moduleLines[i]._name)
            {
                fprintf(stderr, "Compiler::loadModule() : module %s already loaded\n", moduleName.c_str());
                return false;
            }
        }

        // Read module
        int inpLines = 0;
        std::vector<Compiler::Input> inp;
        if(!Compiler::readInputFile(infile, modulePath, inp, inpLines)) return false;

        // Erase module line name and insert module names
        auto itModule = _moduleLines.erase(_moduleLines.begin() + codeLineIndex);
        std::vector<ModuleLine> moduleLines(inp.size());
        for(int i=0; i<int(moduleLines.size()); i++)
        {
            moduleLines[i] = {i, moduleFile};
        }
        _moduleLines.insert(itModule, moduleLines.begin(), moduleLines.end());

        // Erase module line and insert module
        auto itInput = input.erase(input.begin() + codeLineIndex);
        input.insert(itInput, inp.begin(), inp.end());

        numLines = int(input.size());

        return true;
    }

    bool parseModules(std::vector<Input>& input, int& numLines)
    {
        // Parse each line of input for modules
        for(int j=0; j<numLines; j++)
        {
            std::string inputText = input[j]._text;
            inputText = Expression::removeCommentsNotInStrings(inputText);
            std::vector<std::string> tokens = Expression::tokenise(inputText, ' ');
            if(tokens.size() == 0) continue;

            Expression::strToUpper(tokens[0]);
            Expression::stripWhitespace(tokens[0]);
            if(tokens[0] == "MODULE")
            {
                if(tokens.size() < 2  ||  tokens.size() > 2)
                {
                    fprintf(stderr, "Compiler::parseModules() : '%s:%d' : syntax error, use 'MODULE \"<module name>\"'\n", input[j]._text.c_str(), j);
                    return false;
                }

                // TODO: clean this up with iterators
                if(!loadModule(tokens[1], j, input, numLines)) return false;
                j--;
            }
        }

        return true;
    }

    // Uncomments sys init funcs as they are used, (effectively inserting them into the code)
    // vASM and label addresses need to be fixed, this is done at the end of parseCode()
    void enableSysInitFunc(const std::string& sysInitFunc)
    {
        for(int i=0; i<int(_codeLines[0]._vasm.size()); i++)
        {
            if(_codeLines[0]._vasm[i]._opcode == std::string(";%" + sysInitFunc))
            {
                Expression::replaceText(_codeLines[0]._vasm[i]._opcode, ";%", "");
                Expression::replaceText(_codeLines[0]._vasm[i]._code, ";%", "");
                break;
            }
        }
    }

    // Parse labels and generate codeLines 
    bool parseLabels(std::vector<Input>& input, int numLines)
    {
        // GOSUB labels
        for(int i=0; i<numLines; i++)
        {
            if(!input[i]._parse) continue;

            if(!checkForGosubLabel(_moduleLines[i]._name, input[i]._text, i)) return false;
        }

        // All labels and code lines
        for(int i=0; i<numLines; i++)
        {
            if(!input[i]._parse)
            {
                // Create dummy codeLine for pragmas, so that codeLine count remains consistant
                CodeLine codeLine;
                if(createCodeLine("", 0, -1, -1, Expression::Int16Both, false, codeLine, MODULE_MAIN)) _codeLines.push_back(codeLine);
                continue;
            }

            switch(checkForLabel(_moduleLines[i]._name, input[i]._text, i))
            {
                case LabelFound:    break;
                case LabelNotFound: break;
                case LabelError:    return false;

                default: break;
            }
        }

        // Add END statement
        finaliseCode();

        return true;
    }

    // Get or create string
    int getOrCreateString(CodeLine& codeLine, int codeLineIndex, const std::string& str, std::string& name, uint16_t& address, uint8_t maxSize, bool constString, VarType varType)
    {
        int index = -1, strLength = int(str.size());

        // Don't count escape char '\'
        int escCount = 0;
        for(int i=0; i<strLength; i++)
        {
            if(str[i] == '\\') escCount++;
        }
        strLength -= escCount;

        // Reuse const string if possible
        if(constString)
        {
            for(int j=0; j<int(_stringVars.size()); j++)
            {
                if(_stringVars[j]._constant  &&  _stringVars[j]._text == str) 
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
                // Allocate RAM for string + length and delimiter bytes
                if(!Memory::getFreeRAM(Memory::FitDescending, strLength + 2, USER_CODE_START, _stringsStart, address))
                {
                    fprintf(stderr, "Compiler::getOrCreateString() : '%s:%d' : not enough RAM for string %s='%s' of size %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, name.c_str(), str.c_str(), strLength + 2, codeLine._text.c_str());
                    return -1;
                }

                std::vector<uint16_t> arrAddrs;
                std::vector<std::string> arrInits;
                name = "str_" + Expression::wordToHexString(address);
                StringVar stringVar = {uint8_t(strLength), uint8_t(strLength), address, str, name, "_" + name + std::string(LABEL_TRUNC_SIZE - name.size() - 1, ' '), varType, -1, true, arrInits, arrAddrs};
                _stringVars.push_back(stringVar);
                index = int(_stringVars.size()) - 1;
            }
        }
        // Variable strings
        else
        {
            // Allocate RAM for string + length and delimiter bytes
            if(!Memory::getFreeRAM(Memory::FitDescending, maxSize + 2, USER_CODE_START, _stringsStart, address))
            {
                fprintf(stderr, "Compiler::getOrCreateString() : '%s:%d' : not enough RAM for string %s='%s' of size %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, name.c_str(), str.c_str(), maxSize + 2, codeLine._text.c_str());
                return -1;
            }

            std::vector<uint16_t> arrAddrs;
            std::vector<std::string> arrInits;
            StringVar stringVar = {uint8_t(strLength), maxSize, address, str, name, "_" + name + std::string(LABEL_TRUNC_SIZE - name.size() - 1, ' '), varType, -1, false, arrInits, arrAddrs};
            _stringVars.push_back(stringVar);
            index = int(_stringVars.size()) - 1;
        }

        return index;
    }

    // Get or create constant string
    uint16_t getOrCreateConstString(const std::string& input, int& index)
    {
        std::string output = input;

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, output, name, address);
        return address;
    }

    // Get or create constant string from int
    uint16_t getOrCreateConstString(ConstStrType constStrType, int16_t input, int& index)
    {
        char output[16] = "";
        switch(constStrType)
        {
            case StrChar:  sprintf(output, "%c",   uint8_t(input) & 0x7F); break;
            case StrHex:   sprintf(output, "%04X", uint16_t(input));       break;

            default: break;
        }

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, std::string(output), name, address);
        return address;
    }

    // Get or create constant sub-string
    uint16_t getOrCreateConstString(ConstStrType constStrType, const std::string& input, int8_t length, uint8_t offset, int& index)
    {
        std::string output;
        switch(constStrType)
        {
            case StrLeft:  output = input.substr(0, length);             break;
            case StrRight: output = input.substr(input.size() - length); break;
            case StrMid:   output = input.substr(offset, length);        break;
            case StrLower: output = Expression::strLower(input);         break;
            case StrUpper: output = Expression::strUpper(input);         break;

            default: break;
        }

        std::string name;
        uint16_t address;
        index = getOrCreateString(_codeLines[_currentCodeLineIndex], _currentCodeLineIndex, output, name, address);
        return address;
    }

    // Create an array of strings
    int createStringArray(CodeLine& codeLine, int codeLineIndex, const std::string& name, uint8_t size, bool isInit, std::vector<std::string>& arrInits, std::vector<uint16_t>& arrAddrs)
    {
        int index = -1;

        if(size > USER_STR_SIZE)
        {
            fprintf(stderr, "Compiler::createStringArray() : '%s:%d' : length %d of string is larger than maximum of %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, size, USER_STR_SIZE, codeLine._text.c_str());
            return -1;
        }

        bool constArray = (size == 0);
        if(constArray  &&  arrInits.size() != arrAddrs.size())
        {
            fprintf(stderr, "Compiler::createStringArray() : '%s:%d' : CONST array initialisers missing, found %d, expecting %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, int(arrInits.size()), int(arrAddrs.size()), codeLine._text.c_str());
            return -1;
        }

        // Allocate RAM for array of strings
        for(int i=0; i<int(arrAddrs.size()); i++)
        {
            uint8_t strSize = uint8_t(((constArray) ? arrInits[i].size() : size));
            if(!Memory::getFreeRAM(Memory::FitDescending, strSize + 2, USER_CODE_START, _arraysStart, arrAddrs[i]))
            {
                fprintf(stderr, "Compiler::createStringArray() : '%s:%d' : not enough RAM for string %s of size %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, name.c_str(), strSize + 2, codeLine._text.c_str());
                return -1;
            }
        }

        // Array of pointers to strings
        uint16_t address = 0x0000;
        int arraySize = int(arrAddrs.size())*2;
        if(!Memory::getFreeRAM(Memory::FitDescending, arraySize, USER_CODE_START, _arraysStart, address, false)) // arrays do not need to be contained within pages
        {
            fprintf(stderr, "Keywords::createStringArray() : '%s:%d' : not enough RAM for int array of size %d : %s\n", codeLine._moduleName.c_str(), codeLineIndex, arraySize, codeLine._text.c_str());
            return -1;
        }

        // TODO: Fix this, so that _constant can be used in string arrays properly
        // Max string size = 0 so compiler can differentiate between const and non const string arrays, (can't set _constant to true!)
        const std::string text(size, ' ');
        uint8_t maxSize = (constArray) ? 0 : USER_STR_SIZE;
        StringVar stringVar = {size, maxSize, address, text, name, "_" + name + std::string(LABEL_TRUNC_SIZE - name.size() - 1, ' '), VarStr2, -1, false, arrInits, arrAddrs, isInit};
        _stringVars.push_back(stringVar);
        index = int(_stringVars.size()) - 1;

        return index;
    }

    void getOrCreateString(const Expression::Numeric& numeric, std::string& name, uint16_t& addr, int& index)
    {
        switch(numeric._varType)
        {
            case Expression::String:
            {
                getOrCreateConstString(numeric._text, index);
                name = getStringVars()[index]._name;
                addr = getStringVars()[index]._address;
            }
            break;

            case Expression::StrVar:
            {
                name = getStringVars()[index]._name;
                addr = getStringVars()[index]._address;
            }
            break;

            case Expression::Constant:
            {
                name = getConstants()[index]._name;
                addr = getConstants()[index]._address;
            }
            break;

            default: break;
        }
    }

    void emitStringAddress(const Expression::Numeric& numeric, uint16_t address)
    {
        if(numeric._varType == Expression::Str2Var  ||  numeric._varType == Expression::TmpStrAddr)
        {
            emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(getTempVarStart())), false);
        }
        else
        {
            emitVcpuAsm("LDWI", Expression::wordToHexString(address), false);
        }
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

    bool find(char chr)
    {
        if(peek(true) == chr)
        {
            get(true);
            return true;
        }

        return false;
    }

    void setCurrentCodeLine(void)
    {
        // Module line, Pragma parsing happens before any code has been parsed, so _codeLines[] may be empty
        _codeLineStart = getCodeLineStart(_currentCodeLineIndex);
        _codeLineText = (int(_codeLines.size()) > _currentCodeLineIndex) ? _codeLines[_currentCodeLineIndex]._code : "PRAGMA";
        _codeLineModule = (int(_moduleLines.size()) > _currentCodeLineIndex) ? _moduleLines[_currentCodeLineIndex]._name : "MAIN";
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
                    if(isDouble) return false;
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

    Expression::Numeric getString(void)
    {
        // First quote
        get(true);

        // Don't skip spaces within string, skip escaped quotes
        char prev = 0;
        std::string text;
        while(peek(false)  &&  ((prev == '\\'  &&  peek(false) == '"')  ||  peek(false) != '"'))
        {
            prev = peek(false);
            text += get(false);
        }

        if(!peek(true)  ||  peek(true) != '"')
        {
            fprintf(stderr, "Compiler::getString() : '%s:%d' : syntax error in string '%s' : '%s'\n", _codeLineModule.c_str(), _codeLineStart, text.c_str(), _codeLineText.c_str());
            return Expression::Numeric();
        }

        // Last quote
        get(true);

        return Expression::Numeric(0, -1, true, false, false, Expression::String, Expression::BooleanCC, Expression::Int16Both, std::string(""), text);
    }

    Expression::Numeric addressOf(void)
    {
        size_t lbra, rbra;
        std::string params;
        int indices[MAX_ARRAY_DIMS] = {0};

        std::string varName = Expression::getExpression();
        if(varName.size()  &&  !isalpha(varName[0]))
        {
            fprintf(stderr, "Compiler::sizeOf() : '%s:%d' : syntax error : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return Expression::Numeric();
        }

        // Parse index params if they exist for arrays, (they must evaluate to literals)
        std::vector<std::string> indexTokens;
        size_t varEnd = varName.find_first_of("-+/*%&<>=();,."); // TODO: this needs to be done in as betterer way, repeated multiple times throughout the code
        if(Expression::findMatchingBrackets(varName, 0, lbra, rbra)  &&  lbra == varEnd)
        {
            params = varName.substr(lbra + 1, rbra - (lbra + 1));
            indexTokens = Expression::tokenise(params, ',', true);
            if(indexTokens.size() < 1  ||  indexTokens.size() > MAX_ARRAY_DIMS)
            {
                fprintf(stderr, "Compiler::addressOf() : '%s:%d' : wrong number of indicies : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
                return Expression::Numeric();
            }

            // Save expression string, (parseExpression() destroys current expression string)
            std::string expression = Expression::getExpression();

            // Save output's nested count, (parseExpression() can modify output's nested count)
            int nestedCount = Expression::getOutputNumeric()._nestedCount;

            // Only literal params are valid, use ADDR() for variable params
            for(int i=0; i<int(indexTokens.size()); i++)
            {
                std::string token = indexTokens[i];
                Expression::stripWhitespace(token);

                std::string operand;
                Expression::Numeric numeric; // = expression(); // TODO: Find out why expression() can't be called instead of parseExpression()
                if(parseStaticExpression(_currentCodeLineIndex, token, operand, numeric) == OperandInvalid) return Expression::Numeric();
                if(numeric._varType != Expression::Number)
                {
                    fprintf(stderr, "Compiler::addressOf() : '%s:%d' : indicies must be literal expressions : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
                    return Expression::Numeric();
                }

                // Order of indices is [0]:k [1]:j [2]:i : func(k, j, i)
                indices[i + MAX_ARRAY_DIMS - indexTokens.size()] = int16_t(std::lround(numeric._value));
                if(indices[i + MAX_ARRAY_DIMS - indexTokens.size()] < 0)
                {
                    fprintf(stderr, "Compiler::addressOf() : '%s:%d' : indicies must be >= 0\n : %s", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
                    return Expression::Numeric();
                }
            }

            // Restore output's nested count
            Expression::getOutputNumeric()._nestedCount = nestedCount;

            // Restore expression string
            Expression::setExpression(expression);
        }

        int varIndex = -1;
        std::string oldName;
        if(indexTokens.size())
        {
            oldName = varName.substr(0, rbra+1);
            varIndex = findVar(varName);
        }
        else
        {
            if(varName.back() == ')') varName.erase(varName.size()-1);
            varIndex = findVar(varName, oldName);
        }

        int strIndex   = findStr(varName);
        int labIndex   = findLabel(varName);
        int constIndex = findConst(varName);

        // Use oldName instead of varName for advancing, as varName gets name mangled by findVar()
        Expression::advance(oldName.size());

        // Int vars and int arrays
        uint16_t address = 0x0000;
        if(varIndex != -1)
        {
            int numIndices = int(_integerVars[varIndex]._arrSizes.size());
            for(int i=0; i<numIndices; i++)
            {
                int dim = _integerVars[varIndex]._arrSizes[i];
                if(indices[i] > dim - 1)
                {
                    fprintf(stderr, "Compiler::addressOf() : '%s:%d' : index %d:%d greater than array dimension %d:%d : %s\n", _codeLineModule.c_str(), _codeLineStart, i, indices[i], i, dim, _codeLineText.c_str());
                    return Expression::Numeric();
                }
            }

            switch(_integerVars[varIndex]._varType)
            {
                case Var1Arr8:
                case Var1Arr16: address = _integerVars[varIndex]._address  +  uint16_t(indices[2] * _integerVars[varIndex]._intSize); break;

                // Returns address of first element in first dimension of array, but array dimensions are NOT guaranteed to be sequential in memory
                case Var2Arr8:
                case Var2Arr16: address = _integerVars[varIndex]._arrAddrs[0][indices[1]]  +  uint16_t(indices[2] * _integerVars[varIndex]._intSize); break;

                // Returns address of first element in first dimension of array, but array dimensions are NOT guaranteed to be sequential in memory
                case Var3Arr8:
                case Var3Arr16: address = _integerVars[varIndex]._arrAddrs[indices[0]][indices[1]]  +  uint16_t(indices[2] * _integerVars[varIndex]._intSize); break;

                default: address = _integerVars[varIndex]._address; break;
            }
        }
        // Strings and string arrays
        else if(strIndex != -1)
        {
            switch(_stringVars[strIndex]._varType)
            {
                // Returns address of a string or an address of a char in a string, (including length and delimiter bytes) 
                case VarStr:
                {
                    // Order of indices is [0]:k [1]:j [2]:i : func(k, j, i)
                    switch(indexTokens.size())
                    {
                        case 0: address = _stringVars[strIndex]._address;                        break;
                        case 1: address = _stringVars[strIndex]._address + uint16_t(indices[2]); break;

                        default:
                        {
                            fprintf(stderr, "Compiler::addressOf() : '%s:%d' : too many indices for string array '%s(%s) : %s'\n", _codeLineModule.c_str(), _codeLineStart, _stringVars[strIndex]._name.c_str(), params.c_str(),
                                                                                                                                   _codeLineText.c_str());
                            return Expression::Numeric();
                        }
                        break;
                    }
                }
                break;

                // Returns an address of a string in an array or an address of a char in a string in an array, (including length and delimiter bytes)
                case VarStr2:
                {
                    // Order of indices is [0]:k [1]:j [2]:i : func(k, j, i)
                    switch(indexTokens.size())
                    {
                        case 0: address = _stringVars[strIndex]._arrAddrs[0];                                 break;
                        case 1: address = _stringVars[strIndex]._arrAddrs[indices[2]];                        break;
                        case 2: address = _stringVars[strIndex]._arrAddrs[indices[1]] + uint16_t(indices[2]); break;

                        default:
                        {
                            fprintf(stderr, "Compiler::addressOf() : '%s:%d' : too many indices for string array '%s(%s) : %s'\n", _codeLineModule.c_str(), _codeLineStart, _stringVars[strIndex]._name.c_str(), params.c_str(),
                                                                                                                                   _codeLineText.c_str());
                            return Expression::Numeric();
                        }
                        break;
                    }
                }
                break;

                default: break;
            }
        }
        // labels
        else if(labIndex != -1)
        {
            address = _labels[labIndex]._address;
        }
        // Constants
        else if(constIndex != -1)
        {
            address = _constants[constIndex]._address;
        }
        else
        {
            fprintf(stderr, "Compiler::addressOf() : '%s:%d' : syntax error : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return Expression::Numeric();
        }

        bool relocatable = (labIndex > 0) ? true : false;
        return Expression::Numeric(address, -1, true, false, relocatable, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric sizeOf(void)
    {
        std::string varName = Expression::getExpression();
        if(varName.size()  &&  !isalpha(varName[0]))
        {
            fprintf(stderr, "Compiler::sizeOf() : '%s:%d' : syntax error : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return Expression::Numeric();
        }

        if(varName.back() == ')') varName.erase(varName.size()-1);

        std::string oldName;
        int varIndex = findVar(varName, oldName);
        int strIndex = findStr(varName);
        int constIndex = findConst(varName);

        // Use oldName instead of varName for advancing, as varName gets name mangled by findVar() for local variables
        Expression::advance(oldName.size());

        // Int and int arrays
        uint16_t size = 0;
        if(varIndex != -1)
        {
            size = uint16_t(_integerVars[varIndex]._intSize);
            switch(_integerVars[varIndex]._varType)
            {
                case Var1Arr8:
                case Var1Arr16: size *= _integerVars[varIndex]._arrSizes[2]; break;

                case Var2Arr8:
                case Var2Arr16: size *= _integerVars[varIndex]._arrSizes[1] * _integerVars[varIndex]._arrSizes[2]; break;

                case Var3Arr8:
                case Var3Arr16: size *= _integerVars[varIndex]._arrSizes[0] * _integerVars[varIndex]._arrSizes[1] * _integerVars[varIndex]._arrSizes[2]; break;

                default: break;
            }
        }
        // String and string arrays
        else if(strIndex != -1)
        {
            switch(_stringVars[strIndex]._varType)
            {
                // Returns allocated size of string, including length and delimiter bytes
                case VarStr: size = uint16_t(_stringVars[strIndex]._maxSize + 2); break;

                // Returns total allocated size of all strings in array, including length and delimiter bytes
                case VarStr2: 
                {
                    for(int i=0; i<int(_stringVars[strIndex]._arrAddrs.size()); i++)
                    {
                        // Const string array
                        if(_stringVars[strIndex]._maxSize == 0)
                        {
                            size += uint16_t(_stringVars[strIndex]._arrInits[i].size() + 2);
                        }
                        // String array
                        else
                        {
                            size += _stringVars[strIndex]._maxSize + 2;
                        }
                    }
                }
                break;

                default: break;
            }
        }
        // Constants
        else if(constIndex != -1)
        {
            size = _constants[constIndex]._size;
        }
        else
        {
            fprintf(stderr, "Compiler::sizeOf() : '%s:%d' : syntax error : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return Expression::Numeric();
        }

        return Expression::Numeric(size, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    bool userFunc(const std::string& name)
    {
        size_t lbra, rbra;
        std::string funcText = Expression::getExpression();
        if(!Expression::findMatchingBrackets(funcText, 0, lbra, rbra))
        {
            fprintf(stderr, "Compiler::userFunc() : '%s:%d' : parenthesis error : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return false;
        }

        funcText = funcText.substr(lbra + 1, rbra - (lbra + 1));
        std::vector<std::string> params = Expression::tokenise(funcText, ',', true);
        if(params.size() == 0)
        {
            fprintf(stderr, "Compiler::userFunc() : '%s:%d' : syntax error, need at least one parameter : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
            return false;
        }
        int paramsSize = int(getDefFunctions()[name]._params.size());
        if(paramsSize != int(params.size()))
        {
            fprintf(stderr, "Compiler::userFunc() : '%s:%d' : syntax error, wrong number of parameters, expecting %d : %s\n", _codeLineModule.c_str(), _codeLineStart, paramsSize, _codeLineText.c_str());
            return false;
        }
        std::string func = getDefFunctions()[name]._function;
        for(int i=0; i<int(params.size()); i++)
        {
            Expression::stripWhitespace(params[i]);
            Expression::replaceText(func, getDefFunctions()[name]._params[i], params[i]);
        }

        // Substitute function and re-create expression, (factor() then parses the new expression string)
        intptr_t offset = Expression::getExpression() - (char *)Expression::getExpressionToParseString().c_str();
        Expression::replaceText(Expression::getExpressionToParseString(), funcText, func, offset);
        Expression::setExpression(Expression::getExpressionToParseString(), offset);

        return true;
    }

    Expression::Numeric factor(int16_t defaultValue, bool returnAddress=false)
    {
        double value = 0;
        Expression::Numeric numeric;

        setCurrentCodeLine();

        // Fast boolean conditions, (condition must be enclosed within paranthesis. e.g. '&(a), &(a XOR b), etc')
        if(Expression::find(" &("))
        {
            Expression::getOutputNumeric()._nestedCount++;

            numeric = expression(returnAddress); if(!numeric._isValid) return numeric;
            numeric._ccType = Expression::FastCC;

            // Parameters
            while(peek(true)  &&  peek(true) != ')')
            {
                if(get(true) == ',') numeric._params.push_back(expression());
            }

            Expression::getOutputNumeric()._nestedCount--;

            if(peek(true) != ')')
            {
                fprintf(stderr, "Compiler::factor() : '%s:%d' : found '%c' expecting ')'\n", Expression::getExpressionToParse(), _codeLineStart, peek(true));
                numeric = Expression::Numeric();
            }
            get(true);

            // Label gets filled in later by addLabelToJumpCC(), (optimiser removes extra LDW/STW pairs)
            Operators::createSingleOp("LDW", numeric);
            emitVcpuAsm("BEQ", "", false);
            Operators::createSingleOp("STW", numeric);
        }
        else if(peek(true) == '(')
        {
            Expression::getOutputNumeric()._nestedCount++;

            get(true);
            numeric = expression(returnAddress); if(!numeric._isValid) return numeric;

            // Parameters
            while(peek(true)  &&  peek(true) != ')')
            {
                if(get(true) == ',') numeric._params.push_back(expression());
            }

            Expression::getOutputNumeric()._nestedCount--;

            if(peek(true) != ')')
            {
                fprintf(stderr, "Compiler::factor() : '%s:%d' : found '%c' expecting ')'\n", Expression::getExpressionToParse(), _codeLineStart, peek(true));
                numeric = Expression::Numeric();
            }
            get(true);
        }
        // Numeric literals
        else if((peek(true) >= '0'  &&  peek(true) <= '9')  ||  peek(true) == '&')
        {
            if(number(value))
            {
                numeric = Expression::Numeric(value, -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
            }
            else
            {
                fprintf(stderr, "Compiler::factor() : '%s:%d' : syntax error in number literal : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
                numeric = Expression::Numeric();
            }
        }
        // Char literals
        else if(peek(true) == '\'')
        {
            get(true);

            char chr = 0;
            bool validChar = false;
            if((chr = peek(false)) != 0) // don't skip spaces as char literal can be a space
            {
                if(chr >= 32)
                {
                    get(false); // don't skip spaces as char literal can be a space
                    if(peek(false) == '\'') // don't skip spaces as char literal must be a single char
                    {
                        validChar = true;
                        numeric = Expression::Numeric(double(chr), -1, true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
                        get(false); // don't skip spaces as char literal must be a single char
                    }
                }
            }

            if(!validChar)
            {
                fprintf(stderr, "Compiler::factor() : '%s:%d' : syntax error in character literal : %s\n", _codeLineModule.c_str(), _codeLineStart, _codeLineText.c_str());
                numeric = Expression::Numeric();
            }
        }
        // Strings
        else if(peek(true) == '"')
        {
            // Handles quotes internally
            numeric = getString();
        }
        // 'Address of' operator
        else if(peek(true) == '@')
        {
            get(true); numeric = addressOf();
        }
        // 'Size of' operator
        else if(peek(true) == '#')
        {
            get(true); numeric = sizeOf();
        }
        // Unary operators
        else if(peek(true) == '+')
        {
            get(true); numeric = factor(0); numeric = Operators::POS(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(peek(true) == '-')
        {
            get(true); numeric = factor(0); numeric = Operators::NEG(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::find("NOT "))
        {
            numeric = factor(0); numeric = Operators::NOT(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        // Functions with no parameters
        else if(Expression::find("TIME$"))
        {
            numeric = Functions::TIME$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        // Functions
        else if(Expression::findFunc("PEEK"))
        {
            numeric = factor(0); numeric = Functions::PEEK(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("DEEK"))
        {
            numeric = factor(0); numeric = Functions::DEEK(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("USR"))
        {
            numeric = factor(0); numeric = Functions::USR(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("RND"))
        {
            numeric = factor(0); numeric = Functions::RND(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("URND"))
        {
            numeric = factor(0); numeric = Functions::URND(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LEN"))
        {
            // Functions::IARR() needs to return an address rather than a value for LEN()
            numeric = factor(0, true); numeric = Functions::LEN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("GET"))
        {
            numeric = factor(0); numeric = Functions::GET(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ABS"))
        {
            numeric = factor(0); numeric = Functions::ABS(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("SGN"))
        {
            numeric = factor(0); numeric = Functions::SGN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ASC"))
        {
            numeric = factor(0); numeric = Functions::ASC(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("STRCMP"))
        {
            numeric = factor(0); numeric = Functions::STRCMP(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("BCDCMP"))
        {
            numeric = factor(0); numeric = Functions::BCDCMP(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("VAL"))
        {
            numeric = factor(0); numeric = Functions::VAL(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LUP"))
        {
            numeric = factor(0); numeric = Functions::LUP(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ADDR"))
        {
            // Functions::IARR() needs to return an address rather than a value for ADDR()
            numeric = factor(0, true); numeric = Functions::ADDR(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("POINT"))
        {
            numeric = factor(0); numeric = Functions::POINT(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("MIN"))
        {
            numeric = factor(0); numeric = Functions::MIN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("MAX"))
        {
            numeric = factor(0); numeric = Functions::MAX(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("CLAMP"))
        {
            numeric = factor(0); numeric = Functions::CLAMP(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("CHR$"))
        {
            numeric = factor(0); numeric = Functions::CHR$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("SPC$"))
        {
            numeric = factor(0); numeric = Functions::SPC$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("STR$"))
        {
            numeric = factor(0); numeric = Functions::STR$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("STRING$"))
        {
            numeric = factor(0); numeric = Functions::STRING$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("HEX$"))
        {
            numeric = factor(0); numeric = Functions::HEX$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LEFT$"))
        {
            numeric = factor(0); numeric = Functions::LEFT$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("RIGHT$"))
        {
            numeric = factor(0); numeric = Functions::RIGHT$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("MID$"))
        {
            numeric = factor(0); numeric = Functions::MID$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LOWER$"))
        {
            numeric = factor(0); numeric = Functions::LOWER$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("UPPER$"))
        {
            numeric = factor(0); numeric = Functions::UPPER$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("STRCAT$"))
        {
            numeric = factor(0); numeric = Functions::STRCAT$(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("CEIL"))
        {
            numeric = factor(0); numeric = Operators::CEIL(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("FLOOR"))
        {
            numeric = factor(0); numeric = Operators::FLOOR(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("POW"))
        {
            numeric = factor(0); numeric = Operators::POWF(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("SQRT"))
        {
            numeric = factor(0); numeric = Operators::SQRT(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("EXP2"))
        {
            numeric = factor(0); numeric = Operators::EXP2(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("EXP"))
        {
            numeric = factor(0); numeric = Operators::EXP(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LOG10"))
        {
            numeric = factor(0); numeric = Operators::LOG10(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LOG2"))
        {
            numeric = factor(0); numeric = Operators::LOG2(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("LOG"))
        {
            numeric = factor(0); numeric = Operators::LOG(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("SIN"))
        {
            numeric = factor(0); numeric = Operators::SIN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("COS"))
        {
            numeric = factor(0); numeric = Operators::COS(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("TAN"))
        {
            numeric = factor(0); numeric = Operators::TAN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ASIN"))
        {
            numeric = factor(0); numeric = Operators::ASIN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ACOS"))
        {
            numeric = factor(0); numeric = Operators::ACOS(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ATAN2"))
        {
            numeric = factor(0); numeric = Operators::ATAN2(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("ATAN"))
        {
            numeric = factor(0); numeric = Operators::ATAN(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("RAND"))
        {
            numeric = factor(0); numeric = Operators::RAND(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("REV16"))
        {
            numeric = factor(0); numeric = Operators::REV16(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("REV8"))
        {
            numeric = factor(0); numeric = Operators::REV8(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else if(Expression::findFunc("REV4"))
        {
            numeric = factor(0); numeric = Operators::REV4(numeric, _codeLineModule, _codeLineText, _codeLineStart);
        }
        else
        {
            // User functions, (DEF FN), names are NOT case sensitive, (like inbuilt functions)
            for(auto it=getDefFunctions().begin(); it!=getDefFunctions().end(); ++it)
            {
                std::string name = it->first;
                if(Expression::findFunc(name))
                {
                    if(!userFunc(name)) return numeric;

                    return factor(0);
                }
            }

            switch(peek(true))
            {
                // Reached end of expression
                case 0: numeric = Expression::Numeric(defaultValue, -1, false, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string("")); break;

                default:
                {
                    // Variables
                    bool foundParams = false;
                    std::string varName = Expression::getExpression();
                    if(varName.back() == ')') varName.erase(varName.size()-1);
                    if(varName.find('(') != std::string::npos) foundParams = true;

                    std::string oldName;
                    int varIndex = findVar(varName, oldName);
                    int strIndex = findStr(varName);
                    int constIndex = findConst(varName);
                    if(varIndex != -1)
                    {
                        Expression::Int16Byte int16Byte = Expression::Int16Both;

                        // Use oldName instead of varName for advancing, as varName gets name mangled by findVar() for local variables
                        Expression::advance(oldName.size());

                        // Arrays
                        if(_integerVars[varIndex]._varType == Var1Arr8   ||  _integerVars[varIndex]._varType == Var2Arr8   ||  _integerVars[varIndex]._varType == Var3Arr8  ||
                           _integerVars[varIndex]._varType == Var1Arr16  ||  _integerVars[varIndex]._varType == Var2Arr16  ||  _integerVars[varIndex]._varType == Var3Arr16)
                        {
                            Expression::VarType varType = Expression::Arr1Var16;
                            switch(_integerVars[varIndex]._varType)
                            {
                                case Var1Arr8:  varType = Expression::Arr1Var8;  break;
                                case Var2Arr8:  varType = Expression::Arr2Var8;  break;
                                case Var3Arr8:  varType = Expression::Arr3Var8;  break;

                                case Var1Arr16: varType = Expression::Arr1Var16; break;
                                case Var2Arr16: varType = Expression::Arr2Var16; break;
                                case Var3Arr16: varType = Expression::Arr3Var16; break;

                                default: break;
                            }

                            // Array numeric
                            numeric = Expression::Numeric(defaultValue, int16_t(varIndex), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, varName, std::string(""));
                            numeric._returnAddress = returnAddress;

                            // Array index parameters, (commands like LEN expect no array indices)
                            if(foundParams)
                            {
                                Expression::Numeric param = factor(0); 
                                numeric._params.push_back(param);
                                for(int i=0; i<int(param._params.size()); i++)
                                {
                                    numeric._params.push_back(param._params[i]);
                                }

                                // Read both, low or high bytes, (DEEK <X>, PEEK <X>, PEEK <X+1>)
                                if(Expression::find(".LO")) int16Byte = Expression::Int16Low;
                                if(Expression::find(".HI")) int16Byte = Expression::Int16High;
                                numeric._int16Byte = int16Byte;

                                numeric = Functions::IARR(numeric, _codeLineModule, _codeLineText, _codeLineStart);
                            }
                        }
                        // Vars
                        else
                        {
                            // Read both, low or high bytes, (LDW <X>, LD <X>, LD <X+1>)
                            if(Expression::find(".LO")) int16Byte = Expression::Int16Low;
                            if(Expression::find(".HI")) int16Byte = Expression::Int16High;

                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            numeric = Expression::Numeric(defaultValue, int16_t(varIndex), true, false, false, Expression::IntVar16, Expression::BooleanCC, int16Byte, varName, std::string(""));
                        }
                    }
                    // Strings
                    else if(strIndex != -1)
                    {
                        Expression::advance(varName.size());

                        // Arrays
                        if(_stringVars[strIndex]._varType == VarStr2)
                        {
                            // String array numeric
                            numeric = Expression::Numeric(defaultValue, int16_t(strIndex), true, false, false, Expression::Str2Var, Expression::BooleanCC, Expression::Int16Both, varName, _stringVars[strIndex]._text);

                            // Array index parameters, (commands like LEN expect no array indices)
                            if(foundParams)
                            {
                                Expression::Numeric param = factor(0);
                                numeric._params.push_back(param);
                                for(int i=0; i<int(param._params.size()); i++)
                                {
                                    numeric._params.push_back(param._params[i]);
                                }

                                numeric = Functions::SARR(numeric, _codeLineModule, _codeLineText, _codeLineStart);
                            }
                        }
                        // Vars
                        else
                        {
                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            numeric = Expression::Numeric(defaultValue, int16_t(strIndex), true, false, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, varName, _stringVars[strIndex]._text);
                        }
                    }
                    // Constants
                    else if(constIndex != -1)
                    {
                        Expression::advance(varName.size());
                        
                        switch(_constants[constIndex]._varType)
                        {
                            // Numeric is now passed back to compiler, (rather than just numeric._value), so make sure all fields are valid
                            case ConstInt16:
                            {
                                numeric = Expression::Numeric(_constants[constIndex]._data, int16_t(constIndex), true, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, varName, std::string(""));
                            }
                            break;

                            case ConstStr:
                            {
                                numeric = Expression::Numeric(defaultValue, int16_t(constIndex), true, false, false, Expression::Constant, Expression::BooleanCC, Expression::Int16Both, varName, _constants[constIndex]._text);
                            }
                            break;

                            default: break;
                        }
                    }
                    // Unknown symbol
                    else
                    {
                        numeric = Expression::Numeric(defaultValue, -1, false, false, false, Expression::Number, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));

                        if(varName.size())
                        {
                            fprintf(stderr, "\nCompiler::factor() : '%s:%d' : found an unknown symbol '%s' : %s\n", _codeLineModule.c_str(), _codeLineStart, varName.c_str(), _codeLineText.c_str());
                        }
                        else
                        {
                            Expression::advance(-1);
                            fprintf(stderr, "\nCompiler::factor() : '%s:%d' : found an unknown symbol '%s' : %s\n", _codeLineModule.c_str(), _codeLineStart, Expression::getExpression(), _codeLineText.c_str());
                            Expression::advance(1);
                        }
                    }
                }
                break;
            }
        }

        return numeric;
    }

    Expression::Numeric term(bool returnAddress=false)
    {
        Expression::Numeric numeric, result = factor(0, returnAddress);
        if(!result._isValid) return result;
    
        for(;;)
        {
            if(Expression::find("**"))       {numeric = factor(0, returnAddress); if(!numeric._isValid) return numeric; result = Operators::POW(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('*'))               {numeric = factor(0, returnAddress); if(!numeric._isValid) return numeric; result = Operators::MUL(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('/'))               {numeric = factor(0, returnAddress); if(!numeric._isValid) return numeric; result = Operators::DIV(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('%'))               {numeric = factor(0, returnAddress); if(!numeric._isValid) return numeric; result = Operators::MOD(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("MOD")) {numeric = factor(0, returnAddress); if(!numeric._isValid) return numeric; result = Operators::MOD(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else return result;
        }
    }

    Expression::Numeric expr(bool returnAddress=false)
    {
        Expression::Numeric numeric, result = term(returnAddress);
        if(!result._isValid) return result;

        for(;;)
        {
            // Toggle string work area between lhs and rhs for '+', (no need to check that parameters are strings)
            if(find('+'))      {numeric = term(returnAddress); if(!numeric._isValid) return numeric; result = Operators::ADD(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('-')) {numeric = term(returnAddress); if(!numeric._isValid) return numeric; result = Operators::SUB(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}

            else return result;
        }
    }

    Expression::Numeric logical(bool returnAddress=false)
    {
        Expression::Numeric numeric, result = expr(returnAddress);
        if(!result._isValid) return result;

        for(;;)
        {
            // Boolean conditionals
            if(Expression::find("=="))      {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('='))              {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("<>")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::NE(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("<=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LE(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find(">=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GE(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('<'))              {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LT(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(find('>'))              {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GT(result, numeric, Expression::BooleanCC, _codeLineModule, _codeLineText, _codeLineStart);}

            // Normal conditionals
            else if(Expression::find("&==")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&="))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&<>")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::NE(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&<=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LE(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&>=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GE(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&<"))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LT(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&>"))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GT(result, numeric, Expression::NormalCC, _codeLineModule, _codeLineText, _codeLineStart);}

            // Fast conditionals
            else if(Expression::find("&&==")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&="))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::EQ(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&<>")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::NE(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&<=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LE(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&>=")) {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GE(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&<"))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LT(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("&&>"))  {numeric = expr(returnAddress); if(!numeric._isValid) return numeric; result = Operators::GT(result, numeric, Expression::FastCC, _codeLineModule, _codeLineText, _codeLineStart);}

            else return result;
        }
    }

    Expression::Numeric expression(bool returnAddress)
    {
        Expression::Numeric numeric, result = logical(returnAddress);
        if(!result._isValid) return result;

        for(;;)
        {
            if(Expression::find("AND"))      {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::AND(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("XOR")) {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::XOR(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("OR"))  {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::OR(result,  numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("LSL")) {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LSL(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("LSR")) {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::LSR(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}
            else if(Expression::find("ASR")) {numeric = logical(returnAddress); if(!numeric._isValid) return numeric; result = Operators::ASR(result, numeric, _codeLineModule, _codeLineText, _codeLineStart);}

            else return result;
        }
    }

    uint16_t getStringAddress(Expression::Numeric& numeric)
    {
        uint16_t srcAddr = 0x0000;

        if(numeric._varType == Expression::StrAddr)
        {
            srcAddr = uint16_t(std::lround(numeric._value));
        }
        // String assignment from temporary
        else if(numeric._varType == Expression::TmpStrVar)
        {
            srcAddr = Compiler::getStrWorkArea();
        }
        // String assignment from literal
        else if(numeric._index == -1  &&  numeric._varType == Expression::String)
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

    uint16_t getStringAddress(std::string& strText)
    {
        uint16_t srcAddr = 0x0000;

        if(Expression::isStrNameValid(strText))
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
        else if(Expression::isStringValid(strText))
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

    StrResult assignString(CodeLine& codeLine, int codeLineIndex, int codeLineStart, Expression::Numeric& numeric, uint32_t expressionType)
    {
        if(codeLine._text.size() < 2) return StrNotFound;

        int dstIndex = codeLine._varIndex;
        if(dstIndex == -1)
        {
            fprintf(stderr, "Compiler::assignString() : '%s:%d' : syntax error : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
            return StrError;
        }

        bool isStrExpression = ((expressionType >= Expression::HasStrings)  &&  (expressionType <= Expression::IsStringExpression));
        if(isStrExpression  ||  numeric._varType == Expression::StrAddr  ||  numeric._varType == Expression::TmpStrAddr  ||  numeric._varType == Expression::TmpStrVar  ||  numeric._varType == Expression::Str2Var)
        {
            // String assignment, from var or const or literal
            uint16_t srcAddr = getStringAddress(numeric);
            if(srcAddr == 0x0000  &&  numeric._varType != Expression::Str2Var  &&  numeric._varType != Expression::TmpStrAddr)
            {
                fprintf(stderr, "Compiler::assignString() : '%s:%d' : Syntax error : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                return StrError;
            }

            // String assignment
            if(Expression::getOutputNumeric()._varType == Expression::StrVar)
            {
                // No need to copy if src and dst are equal
                uint16_t dstAddr = _stringVars[dstIndex]._address;
                if(srcAddr != dstAddr)
                {
                    emitStringAddress(numeric, srcAddr);
                    emitVcpuAsm("STW", "strSrcAddr", false, codeLineIndex);
                    emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false, codeLineIndex);
                    emitVcpuAsm("%StringCopy", "", false, codeLineIndex);
                }
            }
            // String array assignment
            else if(Expression::getOutputNumeric()._varType == Expression::Str2Var)
            {
                if(!writeArrayStr(codeLine, codeLineIndex, numeric, dstIndex, srcAddr))
                {
                    fprintf(stderr, "Compiler::assignString() : '%s:%d' : syntax error : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                    return StrError;
                }
            }

            return StrCreated;
        }

        return StrNotFound;
    }

    bool assignInt(CodeLine& codeLine, int codeLineIndex, Expression::Numeric& numeric, uint32_t expressionType, int varIndexRhs)
    {
        // Update result variable
        if(codeLine._varIndex != -1)
        {
            // Expression that contains one or more vars
            bool containsVars = (expressionType & Expression::HasIntVars) || (expressionType & Expression::HasStrVars) || (expressionType & Expression::HasStrConsts);
            updateIntVar(int16_t(std::lround(numeric._value)), codeLine, codeLine._varIndex, containsVars);
        }

        // TODO: only works with Int16, fix for all var types
        // Variable assignment
        if(codeLine._varIndex != -1)
        {
            // Assignment with a var expression
            if(codeLine._containsVars)
            {
                // Try and optimise LDW away if possible
                if(varIndexRhs >= 0  &&  _integerVars[varIndexRhs]._varType != Var1Arr16  &&  !(expressionType & Expression::HasOperators)  &&  !(expressionType & Expression::HasFunctions))
                {
                    switch(numeric._int16Byte)
                    {
                        case Expression::Int16Low:  emitVcpuAsm("LD",  "_" + _integerVars[varIndexRhs]._name,          false, codeLineIndex); break;
                        case Expression::Int16High: emitVcpuAsm("LD",  "_" + _integerVars[varIndexRhs]._name + " + 1", false, codeLineIndex); break;
                        case Expression::Int16Both: emitVcpuAsm("LDW", "_" + _integerVars[varIndexRhs]._name,          false, codeLineIndex); break;

                        default: break;
                    }
                }

                if(_integerVars[codeLine._varIndex]._varType == Var1Arr8   ||  _integerVars[codeLine._varIndex]._varType == Var2Arr8   ||  _integerVars[codeLine._varIndex]._varType == Var3Arr8  ||
                   _integerVars[codeLine._varIndex]._varType == Var1Arr16  ||  _integerVars[codeLine._varIndex]._varType == Var2Arr16  ||  _integerVars[codeLine._varIndex]._varType == Var3Arr16)
                {
                    if(!writeArrayVar(codeLine, codeLineIndex, codeLine._varIndex)) return false;
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

                if(_integerVars[codeLine._varIndex]._varType == Var1Arr8   ||  _integerVars[codeLine._varIndex]._varType == Var2Arr8   ||  _integerVars[codeLine._varIndex]._varType == Var3Arr8  ||
                   _integerVars[codeLine._varIndex]._varType == Var1Arr16  ||  _integerVars[codeLine._varIndex]._varType == Var2Arr16  ||  _integerVars[codeLine._varIndex]._varType == Var3Arr16)
                {
                    if(!writeArrayVar(codeLine, codeLineIndex, codeLine._varIndex)) return false;
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

        return true;
    }

    StatementResult createVasmCode(CodeLine& codeLine, int codeLineIndex, int codeLineStart)
    {
        // Check for subroutine start, make sure PUSH is emitted only once, even for multi-statement lines, (codeLine is a local copy of each statement within a multi-statement codeLine)
        if(!_codeLines[_currentCodeLineIndex]._pushEmitted  &&  codeLine._labelIndex >= 0  &&  _labels[codeLine._labelIndex]._gosub)
        {
            _codeLines[_currentCodeLineIndex]._pushEmitted = true;
            emitVcpuAsm("PUSH", "", false, codeLineIndex);
        }

        // Specific parsing requirements for most keywords, (*NOT* functions), some keywords like IF will also parse multi-statements; token[0] has to always be a valid keyword
        bool isTokenZeroValid = false;
        for(int i=0; i<int(codeLine._tokens.size()); i++)
        {
            Keywords::KeywordFuncResult result;
            Keywords::KeywordResult keywordResult = Keywords::handleKeywords(codeLine, codeLine._tokens[i], codeLineIndex, i, result);
            if(keywordResult == Keywords::KeywordError) return StatementError;

            // Search for keyword, if found return it's statement type result
            std::string token = codeLine._tokens[i];
            Expression::strToUpper(token);
            if(Keywords::getKeywords().find(token) != Keywords::getKeywords().end())
            {
                if(i == 0) isTokenZeroValid = true;
                if(i > 0  &&  !isTokenZeroValid)
                {
                    fprintf(stderr, "Compiler::createVasmCode() : '%s:%d' : syntax error in '%s' : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._tokens[0].c_str(), codeLine._text.c_str());
                    return StatementError;
                }
                return Keywords::getKeywords()[token]._result;
            }
        }

        int varIndexRhs = -1, constIndexRhs = -1, strIndexRhs = -1;
        uint32_t expressionType = isExpression(codeLine._expression, varIndexRhs, constIndexRhs, strIndexRhs);
        if(expressionType == Expression::IsInvalid) return StatementError;

        // Parse expression, handles ints, strings, arrays, operators and functions
        bool isStringVar = false;
        Expression::Numeric numeric;
        if(codeLine._varIndex != -1)
        {
            std::string name;
            Expression::VarType varType = Expression::Number;
            switch(codeLine._varType)
            {
                case VarInt16:  varType = Expression::IntVar16;  name = _integerVars[codeLine._varIndex]._name; break;
                case Var1Arr16: varType = Expression::Arr1Var16; name = _integerVars[codeLine._varIndex]._name; break;
                case Var2Arr16: varType = Expression::Arr2Var16; name = _integerVars[codeLine._varIndex]._name; break;
                case Var3Arr16: varType = Expression::Arr3Var16; name = _integerVars[codeLine._varIndex]._name; break;
                case Var1Arr8:  varType = Expression::Arr1Var8;  name = _integerVars[codeLine._varIndex]._name; break;
                case Var2Arr8:  varType = Expression::Arr2Var8;  name = _integerVars[codeLine._varIndex]._name; break;
                case Var3Arr8:  varType = Expression::Arr3Var8;  name = _integerVars[codeLine._varIndex]._name; break;
                
                case VarStr:  varType = Expression::StrVar;  name = _stringVars[codeLine._varIndex]._name;  isStringVar = true; break;
                case VarStr2: varType = Expression::Str2Var; name = _stringVars[codeLine._varIndex]._name;  isStringVar = true; break;

                default: break;
            }
            
            // Output variable, (functions can access this variable within parse())
            numeric = Expression::Numeric(0, int16_t(codeLine._varIndex), true, false, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }
        if(!Expression::parse(codeLine._expression, codeLineIndex, numeric))
        {
            //fprintf(stderr, "Compiler::createVasmCode() : '%s:%d' : syntax error in '%s'\n", codeLine._moduleName.c_str(), codeLineStart, Expression::getExpression());
            return StatementError;
        }

        // String assignment
        StrResult stringResult = assignString(codeLine, codeLineIndex, codeLineStart, numeric, expressionType);
        if(stringResult == StrCreated  ||  isStringVar) return StringStatementParsed;
        if(stringResult == StrError) return StatementError;

        // Int assignment
        if(!assignInt(codeLine, codeLineIndex, numeric, expressionType, varIndexRhs)) return StatementError;

        return StatementExpression;
    }

    // Very simple check, if the programmer goes out of his way to write gnarly code, e.g. ()()()()()()(((var((()()))))), then this will be accepted
    bool checkMatchingBrackets(const std::string& statement)
    {
        int bracketCount = 0;

        for(int i=0; i<int(statement.size()); i++)
        {
            switch(statement[i])
            {
                case '(': bracketCount++; break;
                case ')': bracketCount--; break;

                default: break;
            }

            // Too many closing brackets
            if(bracketCount < 0) return false;
        }

        // Brackets don't match
        if(bracketCount != 0) return false;

        return true;
    }

    StatementResult parseMultiStatements(const std::string& code, int codeLineIndex, int codeLineStart, int& varIndex, int& strIndex)
    {
        // Make a local copy, otherwise original tokens are destroyed
        CodeLine codeLine = _codeLines[codeLineIndex];

        // Tokenise and parse multi-statement lines
        StatementResult statementResult = StatementSuccess;
        std::vector<std::string> tokens = Expression::tokenise(code, ':', false);
        for(int j=0; j<int(tokens.size()); j++)
        {
REDO_STATEMENT:
            Functions::restart();
            Keywords::restart();

            createCodeLine(tokens[j], 0, codeLine._labelIndex, -1, Expression::Int16Both, false, codeLine, codeLine._moduleName);

            // Check statement matching brackets
            if(!checkMatchingBrackets(tokens[j]))
            {
                fprintf(stderr, "Compiler::parseMultiStatements() : '%s:%d' : syntax error, brackets do not match : %s\n", codeLine._moduleName.c_str(), codeLineStart, codeLine._text.c_str());
                statementResult = StatementError;
                break;
            }

            if(_codeLines[codeLineIndex]._dontParse) return StatementSuccess;

            // Skip empty lines
            if(codeLine._tokens.size() == 0) continue;

            // Create vars
            if(createCodeVar(codeLine, codeLineIndex, varIndex) == VarExistsAsConst) return StatementError;
            createCodeStr(codeLine, codeLineIndex, strIndex);

            // Create vasm
            statementResult = createVasmCode(codeLine, codeLineIndex, codeLineStart);
            if(statementResult == StatementError) break;

            // Some commands, (such as FN), modify the BASIC source and cause a re-evaluation
            if(statementResult == RedoStatementParse) goto REDO_STATEMENT;

            // Some commands, (such as IF), process multi-statements themselves
            if(statementResult == MultiStatementParsed) break;
        }

        return statementResult;
    }

    void addLabelToJumpCC(std::vector<VasmLine>& vasm, const std::string& label)
    {
        for(int i=0; i<int(vasm.size()); i++)
        {
            if(vasm[i]._code.substr(0, sizeof("Jump")-1) == "Jump"  ||  _branchTypes.find(vasm[i]._code.substr(0, 3)) != _branchTypes.end())
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
            if(_codeRomType >= Cpu::ROMv5a)
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
        // Parse code creating vars and vasm code, (BASIC code lines were created in ParseLabels())
        int varIndex, strIndex;
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            _currentCodeLineIndex = i;
            int codeLineStart = getCodeLineStart(_currentCodeLineIndex);

            // First line of BASIC code is always a dummy INIT line, ignore it
            if(i > 0  &&  _codeLines[i]._code.size() >= 2)
            {
                // Adjust label address
                if(_codeLines[i]._labelIndex >= 0) _labels[_codeLines[i]._labelIndex]._address = _vasmPC;

                // Multi-statements
                StatementResult statementResult = parseMultiStatements(_codeLines[i]._code, i, codeLineStart, varIndex, strIndex);
                if(statementResult == StatementError) return false;
            }
        }

        // Check for inserted sys init funcs and adjust vASM and label addresses
        for(int i=0; i<int(_codeLines[0]._vasm.size()); i++)
        {
            for(int j=0; j<int(_sysInitNames.size()); j++)
            {
                if(_codeLines[0]._vasm[i]._opcode == _sysInitNames[j])
                {
                    uint16_t address = _codeLines[0]._vasm[i]._address;
                    Validater::adjustLabelAddresses(address, SYS_INIT_FUNC_LEN);
                    Validater::adjustVasmAddresses(0, address, SYS_INIT_FUNC_LEN);
                    _codeLines[0]._vasm[i]._address = address;
                    break;
                }
            }
        }

        return true;
    }


    void outputReservedWords(void)
    {
        std::string line = "_startAddress_ ";
        Expression::addString(line, LABEL_TRUNC_SIZE - int(line.size()));
        line += "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_userCodeStart) + "\n";
        _output.push_back(line);
    }

    void outputLabels(void)
    {
        std::string line;

        _output.push_back("; Labels\n");

        // BASIC labels
        for(int i=0; i<int(_labels.size()); i++)
        {
            std::string addrStr = Expression::wordToHexString(_labels[i]._address);
            _output.push_back(_labels[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + addrStr + "\n");
            _equateLabels.push_back(_labels[i]._name);
        }

        // Internal labels
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
            {
                uint16_t address = _codeLines[i]._vasm[j]._address;
                std::string internalLabel = _codeLines[i]._vasm[j]._internalLabel;
                if(internalLabel.size())
                {
                    // Warn about duplicate internal labels, (should not happen)
                    if(findInternalLabel(internalLabel) > -1)
                    {
                        fprintf(stderr, "\nCompiler::outputLabels() : warning duplicate internal label '%s' at '0x%04x' on line '%d'\n\n", internalLabel.c_str(), address, i);
                        continue;
                    }

                    // Skip duplicate internal labels, (can happen when mixing ASM code with BASIC code)
                    bool foundDuplicate = false;
                    for(int k=0; k<int(_labels.size()); k++)
                    {
                        if(internalLabel.substr(1) == _labels[k]._name) //  &&  address == _labels[k]._address)
                        {
                            fprintf(stderr, "\nCompiler::outputLabels() : warning duplicate internal label, (if mixing ASM with BASIC then safe to ignore), '%s' at '0x%04x' on line '%d'\n\n",
                                            internalLabel.c_str(), address, i);
                            foundDuplicate = true;
                            break;
                        }
                    }
                    if(foundDuplicate) continue;

                    std::string addrStr = Expression::wordToHexString(_codeLines[i]._vasm[j]._address);
                    _output.push_back(internalLabel + std::string(LABEL_TRUNC_SIZE - internalLabel.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + addrStr + "\n");
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

                            // Check local var names for matches with discarded labels from procs and update if necessary
                            for(int m=0; m<int(_integerVars.size()); m++)
                            {
                                std::string replacedText = _discardedLabels[l]._name.substr(1); // ignore leading '_'
                                std::string replaceText = _internalLabels[k]._name.substr(1);   // ignore leading '_'
                                if(_integerVars[m]._name.find(replacedText) != std::string::npos)
                                {
                                    // Regenerate local var name and local var output
                                    Expression::replaceText(_integerVars[m]._name, replacedText, replaceText);
                                    _integerVars[m]._output = "_" + _integerVars[m]._name;
                                    Expression::addString(_integerVars[m]._output, LABEL_TRUNC_SIZE - int(_integerVars[m]._output.size()));
                                }
                            }
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
            VarType constType = _constants[i]._varType;

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
        _output.push_back("; Global Variables\n");
        for(int varIndex=0; varIndex<int(_integerVars.size()); varIndex++)
        {
            if(_integerVars[varIndex]._address >= INT_VAR_START) continue;

            switch(_integerVars[varIndex]._varType)
            {
                case VarInt16:
                {
                    std::string address = Expression::wordToHexString(_integerVars[varIndex]._address);
                    _output.push_back(_integerVars[varIndex]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
                }
                break;

                default: break;
            }
        }
        _output.push_back("\n");

        _output.push_back("; Local Variables\n");
        for(int varIndex=0; varIndex<int(_integerVars.size()); varIndex++)
        {
            if(_integerVars[varIndex]._address < INT_VAR_START) continue;

            switch(_integerVars[varIndex]._varType)
            {
                case VarInt16:
                {
                    std::string address = Expression::wordToHexString(_integerVars[varIndex]._address);
                    _output.push_back(_integerVars[varIndex]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + address + "\n");
                }
                break;

                default: break;
            }
        }
        _output.push_back("\n");
    }

    void outputArrs(void)
    {
        _output.push_back("; Arrays\n");

        for(int varIndex=0; varIndex<int(_integerVars.size()); varIndex++)
        {
            switch(_integerVars[varIndex]._varType)
            {
                case Var1Arr8:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][0]) + "\n");

                    // Don't output DB statements for an uninitialised array
                    if(_integerVars[varIndex]._arrInit)
                    {
                        std::string dbString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                        // I array values
                        for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                        {
                            // Single initialisation value
                            if(_integerVars[varIndex]._arrInits.size() == 0)
                            {
                                dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                            }
                            // Multiple initialisation values
                            else
                            {
                                // Number of initialisation values may be smaller than array size
                                if(i < int(_integerVars[varIndex]._arrInits.size()))
                                {
                                    dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._arrInits[i])) + " ";
                                }
                                // Use default initialisation value for the rest of the array
                                else
                                {
                                    dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                                }
                            }
                        }
                        _output.push_back(dbString + "\n");
                    }
                }
                break;

                case Var1Arr16:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][0]) + "\n");

                    // Don't output DW statements for an uninitialised array
                    if(_integerVars[varIndex]._arrInit)
                    {
                        std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                        // I array values
                        for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                        {
                            // Single initialisation value
                            if(_integerVars[varIndex]._arrInits.size() == 0)
                            {
                                dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                            }
                            // Multiple initialisation values
                            else
                            {
                                // Number of initialisation values may be smaller than array size
                                if(i < int(_integerVars[varIndex]._arrInits.size()))
                                {
                                    dwString += Expression::wordToHexString(_integerVars[varIndex]._arrInits[i]) + " ";
                                }
                                // Use default initialisation value for the rest of the array
                                else
                                {
                                    dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                                }
                            }
                        }
                        _output.push_back(dwString + "\n");
                    }
                }
                break;

                case Var2Arr8:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._address) + "\n");
                    std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                    // J array pointers
                    for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                    {
                        dwString += Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]) + " ";
                    }
                    _output.push_back(dwString + "\n");

                    // J arrays
                    int initIndex = 0;
                    for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                    {
                        arrName = "_" + _integerVars[varIndex]._name + "_" + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]);
                        _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]) + "\n");

                        // Don't output DB statements for an uninitialised array
                        if(_integerVars[varIndex]._arrInit)
                        {
                            std::string dbString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                            // I array values
                            for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                            {
                                // Single initialisation value
                                if(_integerVars[varIndex]._arrInits.size() == 0)
                                {
                                    dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                                }
                                // Multiple initialisation values
                                else
                                {
                                    // Number of initialisation values may be smaller than array size
                                    if(initIndex < int(_integerVars[varIndex]._arrInits.size()))
                                    {
                                        dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._arrInits[initIndex++])) + " ";
                                    }
                                    // Use default initialisation value for the rest of the array
                                    else
                                    {
                                        dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                                    }
                                }
                            }
                            _output.push_back(dbString + "\n");
                        }
                    }
                }
                break;

                case Var2Arr16:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._address) + "\n");
                    std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                    // J array pointers
                    for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                    {
                        dwString += Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]) + " ";
                    }
                    _output.push_back(dwString + "\n");

                    // J arrays
                    int initIndex = 0;
                    for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                    {
                        arrName = "_" + _integerVars[varIndex]._name + "_" + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]);
                        _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[0][j]) + "\n");

                        // Don't output DW statements for an uninitialised array
                        if(_integerVars[varIndex]._arrInit)
                        {
                            dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                            // I array values
                            for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                            {
                                // Single initialisation value
                                if(_integerVars[varIndex]._arrInits.size() == 0)
                                {
                                    dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                                }
                                // Multiple initialisation values
                                else
                                {
                                    // Number of initialisation values may be smaller than array size
                                    if(initIndex < int(_integerVars[varIndex]._arrInits.size()))
                                    {
                                        dwString += Expression::wordToHexString(_integerVars[varIndex]._arrInits[initIndex++]) + " ";
                                    }
                                    // Use default initialisation value for the rest of the array
                                    else
                                    {
                                        dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                                    }
                                }
                            }
                            _output.push_back(dwString + "\n");
                        }
                    }
                }
                break;

                case Var3Arr8:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._address) + "\n");
                    std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                    // K array pointers
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        dwString += Expression::wordToHexString(_integerVars[varIndex]._arrLut[k]) + " ";
                    }
                    _output.push_back(dwString + "\n");

                    // K * J array pointers
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        arrName = "_" + _integerVars[varIndex]._name + "_lut_" + std::to_string(k);
                        _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrLut[k]) + "\n");
                        dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                        // J array pointers
                        for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                        {
                            dwString += Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]) + " ";
                        }
                        _output.push_back(dwString + "\n");
                    }

                    // K * J arrays
                    int initIndex = 0;
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        // J arrays
                        for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                        {
                            arrName = "_" + _integerVars[varIndex]._name + "_" + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]);
                            _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]) + "\n");

                            // Don't output DB statements for an uninitialised array
                            if(_integerVars[varIndex]._arrInit)
                            {
                                std::string dbString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                                // I array values
                                for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                                {
                                    // Single initialisation value
                                    if(_integerVars[varIndex]._arrInits.size() == 0)
                                    {
                                        dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                                    }
                                    // Multiple initialisation values
                                    else
                                    {
                                        // Number of initialisation values may be smaller than array size
                                        if(initIndex < int(_integerVars[varIndex]._arrInits.size()))
                                        {
                                            dbString += Expression::byteToHexString(uint8_t(_integerVars[varIndex]._arrInits[initIndex++])) + " ";
                                        }
                                        // Use default initialisation value for the rest of the array
                                        else
                                        {
                                            dbString += Expression::wordToHexString(uint8_t(_integerVars[varIndex]._init)) + " ";
                                        }
                                    }
                                }
                                _output.push_back(dbString + "\n");
                            }
                        }
                    }
                }
                break;

                case Var3Arr16:
                {
                    std::string arrName = "_" + _integerVars[varIndex]._name + "_array";
                    _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._address) + "\n");
                    std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                    // K array pointers
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        dwString += Expression::wordToHexString(_integerVars[varIndex]._arrLut[k]) + " ";
                    }
                    _output.push_back(dwString + "\n");

                    // K * J array pointers
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        arrName = "_" + _integerVars[varIndex]._name + "_lut_" + std::to_string(k);
                        _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrLut[k]) + "\n");
                        dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                        // J array pointers
                        for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                        {
                            dwString += Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]) + " ";
                        }
                        _output.push_back(dwString + "\n");
                    }

                    // K * J arrays
                    int initIndex = 0;
                    for(int k=0; k<_integerVars[varIndex]._arrSizes[0]; k++)
                    {
                        // J arrays
                        for(int j=0; j<_integerVars[varIndex]._arrSizes[1]; j++)
                        {
                            arrName = "_" + _integerVars[varIndex]._name + "_" + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]);
                            _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_integerVars[varIndex]._arrAddrs[k][j]) + "\n");

                            // Don't output DW statements for an uninitialised array
                            if(_integerVars[varIndex]._arrInit)
                            {
                                dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                                // I array values
                                for(int i=0; i<_integerVars[varIndex]._arrSizes[2]; i++)
                                {
                                    // Single initialisation value
                                    if(_integerVars[varIndex]._arrInits.size() == 0)
                                    {
                                        dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                                    }
                                    // Multiple initialisation values
                                    else
                                    {
                                        // Number of initialisation values may be smaller than array size
                                        if(initIndex < int(_integerVars[varIndex]._arrInits.size()))
                                        {
                                            dwString += Expression::wordToHexString(_integerVars[varIndex]._arrInits[initIndex++]) + " ";
                                        }
                                        // Use default initialisation value for the rest of the array
                                        else
                                        {
                                            dwString += Expression::wordToHexString(_integerVars[varIndex]._init) + " ";
                                        }
                                    }
                                }
                                _output.push_back(dwString + "\n");
                            }
                        }
                    }
                }
                break;

                default: break;
            }
        }

        _output.push_back("\n");
    }

    bool sanitiseString(const std::string& input, std::string& output, int& length)
    {
        output.clear();

        int numQuotes = 0;
        for(int i=0; i<int(input.size()); i++)
        {
            // Valid ASCII and not escape sequence
            if(input[i] != '\\') output.push_back(input[i]);

            // Escape single quotes
            if(input[i] == '\'')
            {
                numQuotes++;
                output.insert(output.end() - 1, '\\');
            }
        }

        if(int(output.size()) > USER_STR_SIZE + numQuotes)
        {
            fprintf(stderr, "Expression::sanitiseString() : string '%s' of size '%d' is larger than '%d' chars\n", output.c_str(), int(output.size()), USER_STR_SIZE);
            return false;
        }

        // Reduce real length of string by number of inserted escapes, (assembler removes escapes)
        length = int(output.size()) - numQuotes;

        return true;
    }
    bool outputStrs(void)
    {
        _output.push_back("; Strings\n");

        // User strings
        for(int i=0; i<int(_stringVars.size()); i++)
        {
            // Normal strings
            if(_stringVars[i]._varType == VarStr  &&  !_stringVars[i]._constant)
            {
                int len = 0;
                std::string str;
                if(!sanitiseString(_stringVars[i]._text, str, len)) return false;

                _output.push_back(_stringVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._address) + "\n");
                _output.push_back(_stringVars[i]._output + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(len) + " '" + str + "' 0\n");
            }
            // Array of strings
            else if(_stringVars[i]._varType == VarStr2)
            {
                std::string arrName = "_" + _stringVars[i]._name;
                _output.push_back(arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._address) + "\n");
                std::string dwString = arrName + std::string(LABEL_TRUNC_SIZE - arrName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                // J array pointers
                for(int j=0; j<int(_stringVars[i]._arrAddrs.size()); j++)
                {
                    dwString += Expression::wordToHexString(_stringVars[i]._arrAddrs[j]) + " ";
                }
                _output.push_back(dwString + "\n");

                // J strings
                std::string defaultStr = (_stringVars[i]._arrInits.size()) ? _stringVars[i]._arrInits.back() : _stringVars[i]._text;
                for(int j=0; j<int(_stringVars[i]._arrAddrs.size()); j++)
                {
                    std::string strName = "_" + _stringVars[i]._name + "_" + Expression::wordToHexString(_stringVars[i]._arrAddrs[j]);
                    _output.push_back(strName + std::string(LABEL_TRUNC_SIZE - strName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._arrAddrs[j]) + "\n");

                    // Don't output DW statements for an uninitialised array
                    if(_stringVars[i]._arrInit)
                    {
                        int len = 0;
                        std::string str;
                        std::string initStr = (j < int(_stringVars[i]._arrInits.size())) ? _stringVars[i]._arrInits[j] : defaultStr;
                        if(!sanitiseString(initStr, str, len)) return false;
                        _output.push_back(strName + std::string(LABEL_TRUNC_SIZE - strName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(len) + " '" + str + "' 0\n");
                    }
                }
            }
        }
        _output.push_back("\n");

        _output.push_back("; Constant Strings\n");

        // Constant strings
        for(int i=0; i<int(_stringVars.size()); i++)
        {
            // Normal strings
            if(_stringVars[i]._varType == VarStr  &&  _stringVars[i]._constant)
            {
                int len = 0;
                std::string str;
                if(!sanitiseString(_stringVars[i]._text, str, len)) return false;

                _output.push_back(_stringVars[i]._output + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_stringVars[i]._address) + "\n");
                _output.push_back(_stringVars[i]._output + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(len) + " '" + str + "' 0\n");
            }
        }
        _output.push_back("\n");

        return true;
    }

    bool outputDATA(void)
    {
        _output.push_back("; Data\n");

        // Output DATA fields
        for(int i=0; i<int(_dataObjects.size()); i++)
        {
            DataObject* pObject = _dataObjects[i].get();
            switch(pObject->_dataType)
            {
                case DataInteger:
                {
                    DataInt* pData = (DataInt*)pObject;
                    int16_t var = pData->_data;
                    int size = 2;
                    if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, _runtimeStart, pData->_address, true))
                    {
                        fprintf(stderr, "Compiler::outputDATA() : not enough RAM for data of size %d\n", size);
                        return false;
                    }
                    std::string defName = "_data_" + Expression::wordToHexString(pData->_address);
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(pData->_address) + "\n");
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(var) + "\n");
                }
                break;

                case DataString:
                {
                    DataStr* pData = (DataStr*)pObject;
                    std::string str = pData->_data;
                    int size = int(str.size()) + 2;
                    if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, _runtimeStart, pData->_address, true))
                    {
                        fprintf(stderr, "Compiler::outputDATA() : not enough RAM for data of size %d\n", size);
                        return false;
                    }
                    std::string defName = "_data_" + Expression::wordToHexString(pData->_address);
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(pData->_address) + "\n");
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + std::to_string(size-2) + " '" + str + "' 0\n");
                }
                break;

                default: break;
            }
        }

        // Output DATA address LUT
        if(_dataObjects.size())
        {
            uint16_t address = 0x0000;
            int size = int(_dataObjects.size()) * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, _runtimeStart, address, false))
            {
                fprintf(stderr, "Compiler::outputDATA() : not enough RAM for data LUT of size %d\n", size);
                return false;
            }
            std::string defName = "_data_";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
            std::string dwString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int i=0; i<int(_dataObjects.size()); i++)
            {
                DataObject* pObject = _dataObjects[i].get();
                dwString += Expression::wordToHexString(pObject->_address) + " ";
            }
            _output.push_back(dwString + "\n");

            // Output DATA index
            address = 0x0000;
            size = 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, _runtimeStart, address, false))
            {
                fprintf(stderr, "Compiler::outputDATA() : not enough RAM for data LUT of size %d\n", size);
                return false;
            }
            defName = "_dataIndex_";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0\n");
        }

        _output.push_back("\n");

        return true;
    }

    bool outputTIME(void)
    {
        _output.push_back("; Time\n");

        // Time array and string
        if(_createTimeData)
        {
            int timeArraySize = 3;
            uint16_t timeArrayAddress;
            if(!Memory::getFreeRAM(Memory::FitDescending, timeArraySize, USER_CODE_START, _runtimeStart, timeArrayAddress))
            {
                fprintf(stderr, "Compiler::outputTIME() : not enough RAM for time array of size %d\n", timeArraySize);
                return false;
            }
            std::string defName = "_timeArray_";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(timeArrayAddress) + "\n");
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            _output.push_back(dbString + "00 00 00\n");

            int timeStringSize = 10;
            uint16_t timeStringAddress;
            if(!Memory::getFreeRAM(Memory::FitDescending, timeStringSize, USER_CODE_START, _runtimeStart, timeStringAddress))
            {
                fprintf(stderr, "Compiler::outputTIME() : not enough RAM for time string of size %d\n", timeStringSize);
                return false;
            }
            defName = "_timeString_";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(timeStringAddress) + "\n");
            dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            _output.push_back(dbString + std::to_string(timeStringSize - 2) + " '00:00:00' 0\n");
        }
        _output.push_back("\n");

        return true;
    }

    bool outputMidiDef(int midiId, uint16_t startAddr, uint16_t jmpAddr, int size, int& dataIndex)
    {
        std::string defName = "def_midis_" + Expression::wordToHexString(startAddr);
        _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(startAddr) + "\n");
        std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

        // Output a midi segment
        for(int i=0; i<size; i++)
        {
            //dbString += std::to_string(_defDataMidis[midiId]._data[dataIndex++]) + " ";
            dbString += Expression::byteToHexString(_defDataMidis[midiId]._data[dataIndex++]) + " ";
        }
        //dbString += std::to_string(MIDI_CMD_JMP_SEG) + " " + std::to_string(jmpAddr & 0x00FF) + " " + std::to_string((jmpAddr >>8) & 0x00FF);
        dbString += Expression::byteToHexString(MIDI_CMD_JMP_SEG) + " " + Expression::byteToHexString(jmpAddr & 0x00FF) + " " + Expression::byteToHexString((jmpAddr >>8) & 0x00FF);
        _output.push_back(dbString + "\n");

        return true;
    }
    bool outputSpriteDef(int spriteId, int numStripeChunks, uint16_t address, int& dataIndex)
    {
        std::string defName = "def_sprites_" + Expression::wordToHexString(address);
        _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
        std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

        // Output a stripe of chunks
        for(int i=0; i<numStripeChunks; i++)
        {
            for(int j=0; j<SPRITE_CHUNK_SIZE; j++)
            {
                dbString += std::to_string(_defDataSprites[spriteId]._data[dataIndex++]) + " ";
            }
        }

        // Output stripe delimiter
        dbString += std::to_string(_defDataSprites[spriteId]._data[dataIndex++]) + " ";
        _output.push_back(dbString + "\n");

        return true;
    }
    bool outputDefs(void)
    {
        // Create def byte data
        _output.push_back("; Define Bytes\n");
        for(int i=0; i<int(_defDataBytes.size()); i++)
        {
            std::string defName = "def_bytes_" + Expression::wordToHexString(_defDataBytes[i]._address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_defDataBytes[i]._address) + "\n");
            
            std::string db = (_defDataBytes[i]._offset != 0) ? "DB(" + std::to_string(_defDataBytes[i]._offset) + ")" : "DB";
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + db + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
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
            
            std::string dw = (_defDataWords[i]._offset != 0) ? "DW(" + std::to_string(_defDataWords[i]._offset) + ")" : "DW";
            std::string dwString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + dw + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
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
            uint16_t stride = _defDataImages[i]._stride;
            uint16_t address = _defDataImages[i]._address;
            uint16_t width = _defDataImages[i]._width;

            // For each scanline of image data
            for(int j=0; j<_defDataImages[i]._height; j++)
            {
                // 'Loader.gcl' is resident at these addresses when loading *.gt1 files, this onscreen memory can only be overwritten AFTER the loading process has finished
                // DefDataLoaderImageChunk offscreen memory chunks are generated within Keywords::Load() for each of these scanlines
                if((address >= LOADER_SCANLINE0_START  &&  address<= LOADER_SCANLINE0_END)  ||  (address >= LOADER_SCANLINE1_START  &&  address<= LOADER_SCANLINE1_END)  ||  
                   (address >= LOADER_SCANLINE2_START  &&  address<= LOADER_SCANLINE2_END))
                {
                }
                else
                {
                    std::string defName = "def_images_" + Expression::wordToHexString(address);
                    _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
            
                    std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                    for(int k=0; k<width; k++)
                    {
                        dbString += std::to_string(_defDataImages[i]._data[j * width  +  k]) + " ";
                    }
                    _output.push_back(dbString + "\n");
                }

                // Next destination row
                address += stride; 
            }
        }
        _output.push_back("\n");

        // Create def loader image chunk data
        _output.push_back("; Define Loader Image Chunks\n");
        for(int i=0; i<int(_defDataLoaderImageChunks.size()); i++)
        {
            uint16_t srcAddr = _defDataLoaderImageChunks[i]._lutEntry._srcAddr;
            uint8_t length = _defDataLoaderImageChunks[i]._lutEntry._length;

            std::string defName = "def_loader_image_chunks_" + Expression::wordToHexString(srcAddr);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(srcAddr) + "\n");

            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<length; j++)
            {
                dbString += std::to_string(_defDataLoaderImageChunks[i]._data[j]) + " ";
            }
            _output.push_back(dbString + "\n");
        }
        _output.push_back("\n");

        // Create def midi data
        _output.push_back("; Define Midis\n");
        for(auto it=_defDataMidis.begin(); it!=_defDataMidis.end(); ++it)
        {
            int dataIndex = 0;
            int midiId = it->first;
            //bool volume = it->second._volume;
            int numSegments = int(it->second._segmentAddrs.size());
            for(int i=0; i<numSegments; i++)
            {
                uint16_t segSize = it->second._segmentSizes[i];
                uint16_t segAddr = it->second._segmentAddrs[i];
                uint16_t jmpAddr = (i == numSegments-1) ? it->second._segmentAddrs[0] : it->second._segmentAddrs[i+1];
                outputMidiDef(midiId, segAddr, jmpAddr, segSize, dataIndex);
            }
        }
        _output.push_back("\n");

        // Create def sprite data
        _output.push_back("; Define Sprites\n");
        for(auto it=_defDataSprites.begin(); it!=_defDataSprites.end(); ++it)
        {
            int spriteId = it->first;

            // Skip invalid sprite
            if(it->second._stripeAddrs.size() == 0)
            {
                fprintf(stderr, "Compiler::outputDefs() : Wwrning sprite %d is missing stripe addresses\n", spriteId);
                continue;
            }

            int dataIndex = 0;
            uint16_t numColumns = it->second._numColumns;
            uint16_t numStripesPerCol = it->second._numStripesPerCol;
            uint16_t numStripeChunks = it->second._numStripeChunks;
            uint16_t remStripeChunks = it->second._remStripeChunks;
            uint16_t isInstanced = it->second._isInstanced;

            if(!isInstanced)
            {
                // For each column of sprite data
                for(int j=0; j<numColumns; j++)
                {
                    // Multiple stripes per column
                    if(numStripesPerCol > 1)
                    {
                        for(int k=0; k<numStripesPerCol-1; k++)
                        {
                            uint16_t address = it->second._stripeAddrs[j*numStripesPerCol*2 + k*2];
                            outputSpriteDef(spriteId, numStripeChunks, address, dataIndex);
                        }
                        uint16_t address = it->second._stripeAddrs[j*numStripesPerCol*2 + (numStripesPerCol-1)*2];
                        outputSpriteDef(spriteId, remStripeChunks, address, dataIndex);
                    }
                    // Single stripe per column
                    else
                    {
                        uint16_t address = it->second._stripeAddrs[j*2];
                        outputSpriteDef(spriteId, numStripeChunks, address, dataIndex);
                    }
                }
            }
        }
        _output.push_back("\n");

        // Create def font data
        _output.push_back("; Define Fonts\n");
        if(_defDataFonts.size() > 0)
        {
            int fontIdSize = 1;
            uint16_t fontIdAddress;
            if(!Memory::getFreeRAM(Memory::FitDescending, fontIdSize, USER_CODE_START, _runtimeStart, fontIdAddress))
            {
                fprintf(stderr, "Compiler::outputDefs() : not enough RAM for font id var of size %d\n", fontIdSize);
                return false;
            }
            std::string defName = "_fontId_";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(fontIdAddress) + "\n");
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            _output.push_back(dbString + "0\n");
        }
        for(auto it=_defDataFonts.begin(); it!=_defDataFonts.end(); ++it)
        {
            int fontId = it->first;

            // Skip invalid font
            if(it->second._mapping.size() == 0)
            {
                fprintf(stderr, "Compiler::outputDefs() : warning font %d is missing mapping table\n", fontId);
                continue;
            }

            // Font mapping table, (null means 32 to 127 of the ASCII set is represented, so a mapping table is not required)
            uint16_t address = it->second._mapAddr;
            if(address)
            {
                int mapSize = int(it->second._mapping.size());
                std::string defName = "def_map_" + Expression::wordToHexString(address);
                _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
                std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<mapSize; i++)
                {
                    uint8_t mapData = it->second._mapping[i];
                    dbString += std::to_string(mapData) + " ";
                }
                _output.push_back(dbString + "\n");
            }

            // For each char of font data
            int numChars = int(it->second._data.size());
            for(int i=0; i<numChars; i++)
            {
                address = it->second._charAddrs[i];

                std::string defName = "def_char_" + Expression::wordToHexString(address);
                _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
                std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');

                // Output each char
                std::vector<uint8_t>& charData = it->second._data[i];
                for(int j=0; j<int(charData.size()); j++)
                {
                    dbString += std::to_string(charData[j]) + " ";
                }
                _output.push_back(dbString + "\n");
            }

            // Baseline for each char, (shared by all chars in one font)
            address = it->second._baseAddr;
            uint16_t fgbgColour = it->second._fgbgColour;
            std::string defName = "def_baseline_" + Expression::wordToHexString(address);
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(address) + "\n");
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int i=0; i<6; i++) dbString += Expression::byteToHexString(fgbgColour & 0x00FF) + " ";
            dbString += "255";
            _output.push_back(dbString + "\n");
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

            if(numericLabels.size() == 0)
            {
                fprintf(stderr, "Compiler::outputLuts() : There is a mismatch in Numeric labels for one or more GOTO <var> and GOSUB <var>\n");
                fprintf(stderr, "                         Correct your line numbers and add appropriate trailers, ':' for GOTO <var> and '!' for GOSUB <var>\n");
                return false;
            }
            else
            {
                // Create numeric labels LUT, (delimited by 0)
                int lutSize = int(numericLabels.size())*2;
                uint16_t lutAddress;
                if(!Memory::getFreeRAM(Memory::FitDescending, lutSize + 2, USER_CODE_START, _runtimeStart, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : not enough RAM for numeric labels LUT of size %d\n", lutSize + 2);
                    return false;
                }

                std::string lutName = "_lut_numericLabs";
                _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
                std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
                for(int i=0; i<int(numericLabels.size()); i++)
                {
                    dwString += std::to_string(numericLabels[i]) + " ";
                }
                _output.push_back(dwString + "0x0000\n");

                // Create numeric addresses LUT, (same size as above, but no delimiter)
                if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : not enough RAM for numeric addresses LUT of size %d\n", lutSize);
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
                    if(index == -1) fprintf(stderr, "Compiler::outputLuts() : warning label index is invalid for LUT entry %d\n", j);

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

        // Create loader image chunk LUT
        if(_defDataLoaderImageChunks.size())
        {
            // Allocate RAM for loader images LUT and delimiter
            uint16_t lutAddress;
            uint16_t lutSize = uint16_t(_defDataLoaderImageChunks.size() * DefDataLoaderImageChunkLutEntrySize) + 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
            {
                fprintf(stderr, "Compiler::outputDefs() : not enough RAM for loader image LUT of size %d\n", lutSize);
                return false;
            }

            // Loader images LUT delimited by 0
            std::string defName = "_loader_image_chunksLut";
            _output.push_back(defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            std::string dbString = defName + std::string(LABEL_TRUNC_SIZE - defName.size(), ' ') + "DB" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int i=0; i<int(_defDataLoaderImageChunks.size()); i++)
            {
                uint16_t srcAddr = _defDataLoaderImageChunks[i]._lutEntry._srcAddr;
                uint16_t dstAddr = _defDataLoaderImageChunks[i]._lutEntry._dstAddr;
                uint8_t length = _defDataLoaderImageChunks[i]._lutEntry._length;

                dbString += std::to_string(srcAddr & 0x00FF) + " ";
                dbString += std::to_string((srcAddr & 0xFF00) >>8) + " ";
                dbString += std::to_string(dstAddr & 0x00FF) + " ";
                dbString += std::to_string((dstAddr & 0xFF00) >>8) + " ";
                dbString += std::to_string(length) + " ";
            }
            _output.push_back(dbString + "0 0\n");
        }

        // MIDIS LUT
        if(_defDataMidis.size())
        {
            // Allocate RAM for midis LUT
            uint16_t lutAddress;
            int lutSize = int(_defDataMidis.size()) * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
            {
                fprintf(stderr, "Compiler::outputLuts() : not enough RAM for midis LUT of size %d\n", lutSize);
                return false;
            }

            std::string lutName = "_midisLut_";
            _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
            std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(auto it=_defDataMidis.begin(); it!=_defDataMidis.end(); ++it)
            {
                uint16_t address = it->second._segmentAddrs[0];
                dwString += Expression::wordToHexString(address) + " ";
            }
            _output.push_back(dwString + "\n");
        }

        // SPRITE ADDRESS LUTs
        for(auto it=_defDataSprites.begin(); it!=_defDataSprites.end(); ++it)
        {
            int spriteId = it->first;

            // Skip invalid sprite
            if(it->second._stripeAddrs.size() == 0)
            {
                fprintf(stderr, "Compiler::outputLuts() : warning sprite %d is missing stripe addresses\n", spriteId);
                continue;
            }

            // Allocate RAM for sprite addresses/offsets and delimiter
            uint16_t lutAddress;
            int lutSize = int(it->second._stripeAddrs.size())*2 + 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
            {
                fprintf(stderr, "Compiler::outputLuts() : not enough RAM for sprite %d address LUT of size %d\n", spriteId, lutSize);
                return false;
            }
            _spritesAddrLut._spriteAddrs.push_back(lutAddress);

            std::string lutName = "_spriteLut_" + Expression::wordToHexString(lutAddress);
            _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
            std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int j=0; j<int(it->second._stripeAddrs.size()); j++)
            {
                uint16_t address = it->second._stripeAddrs[j];
                dwString += Expression::wordToHexString(address) + " ";
            }
            _output.push_back(dwString + "0x0000\n");
        }

        // SPRITES LUT
        if(_spritesAddrLut._spriteAddrs.size())
        {
            // Allocate RAM for sprites LUT
            if(_spritesAddrLutAddress == 0x0000)
            {
                int lutSize = int(_spritesAddrLut._spriteAddrs.size()) * 2;
                if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, _spritesAddrLutAddress))
                {
                    fprintf(stderr, "Compiler::outputLuts() : not enough RAM for sprites LUT of size %d\n", lutSize);
                    return false;
                }
            }
            _spritesAddrLut._address = _spritesAddrLutAddress;

            std::string lutName = "_spritesLut_";
            _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(_spritesAddrLutAddress) + "\n");
            
            std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int i=0; i<int(_spritesAddrLut._spriteAddrs.size()); i++)
            {
                uint16_t address = _spritesAddrLut._spriteAddrs[i];
                dwString += Expression::wordToHexString(address) + " ";
            }
            _output.push_back(dwString + "\n");
        }

        // FONT ADDRESS LUTs
        for(auto it=_defDataFonts.begin(); it!=_defDataFonts.end(); ++it)
        {
            int fontId = it->first;

            // Skip invalid font
            if(it->second._charAddrs.size() == 0)
            {
                fprintf(stderr, "Compiler::outputLuts() : warning font %d is missing char addresses\n", fontId);
                continue;
            }

            // Allocate memory for font chars + map address + baseline address
            uint16_t lutAddress;
            int lutSize = int(it->second._charAddrs.size())*2 + 4;
            if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
            {
                fprintf(stderr, "Compiler::outputLuts() : not enough RAM for font %d address LUT of size %d\n", fontId, lutSize);
                return false;
            }
            _fontsAddrLut._fontAddrs.push_back(lutAddress);

            std::string lutName = "_fontLut_" + Expression::wordToHexString(lutAddress);
            _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");

            // Mapping table, (mapping address is null when full ASCII set, 32 to 127, is represented)
            std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            uint16_t address = it->second._mapAddr;
            dwString += Expression::wordToHexString(address) + " ";

            // Font baseline address, baseline pixels shared by every char in a font
            address = it->second._baseAddr;
            dwString += Expression::wordToHexString(address) + " ";

            // Characters
            for(int j=0; j<int(it->second._charAddrs.size()); j++)
            {
                address = it->second._charAddrs[j];
                dwString += Expression::wordToHexString(address) + " ";
            }
            _output.push_back(dwString + "\n");
        }

        // FONTS LUT
        if(_fontsAddrLut._fontAddrs.size())
        {
            // Allocate RAM for fonts addresses
            uint16_t lutAddress;
            int lutSize = int(_fontsAddrLut._fontAddrs.size()) * 2;
            if(!Memory::getFreeRAM(Memory::FitDescending, lutSize, USER_CODE_START, _runtimeStart, lutAddress))
            {
                fprintf(stderr, "Compiler::outputLuts() : not enough RAM for fonts LUT of size %d\n", lutSize);
                return false;
            }
            _fontsAddrLut._address = lutAddress;

            std::string lutName = "_fontsLut_";
            _output.push_back(lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(lutAddress) + "\n");
            
            std::string dwString = lutName + std::string(LABEL_TRUNC_SIZE - lutName.size(), ' ') + "DW" + std::string(OPCODE_TRUNC_SIZE - 2, ' ');
            for(int i=0; i<int(_fontsAddrLut._fontAddrs.size()); i++)
            {
                uint16_t address = _fontsAddrLut._fontAddrs[i];
                dwString += Expression::wordToHexString(address) + " ";
            }
            _output.push_back(dwString + "\n");
        }

        _output.push_back("\n");

        return true;
    }


    void outputInternalEquates(void)
    {
        _output.push_back("\n");
        _output.push_back("; Internal Variables\n");
        _output.push_back("serialRawPrev" + std::string(LABEL_TRUNC_SIZE - strlen("serialRawPrev"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(SER_RAW_PREV)  + "\n");
        _output.push_back("register0"     + std::string(LABEL_TRUNC_SIZE - strlen("register0"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(INT_VAR_START) + "\n");
        _output.push_back("register1"     + std::string(LABEL_TRUNC_SIZE - strlen("register1"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x02\n");
        _output.push_back("register2"     + std::string(LABEL_TRUNC_SIZE - strlen("register2"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x04\n");
        _output.push_back("register3"     + std::string(LABEL_TRUNC_SIZE - strlen("register3"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x06\n");
        _output.push_back("register4"     + std::string(LABEL_TRUNC_SIZE - strlen("register4"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x08\n");
        _output.push_back("register5"     + std::string(LABEL_TRUNC_SIZE - strlen("register5"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0A\n");
        _output.push_back("register6"     + std::string(LABEL_TRUNC_SIZE - strlen("register6"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0C\n");
        _output.push_back("register7"     + std::string(LABEL_TRUNC_SIZE - strlen("register7"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x0E\n");
        _output.push_back("register8"     + std::string(LABEL_TRUNC_SIZE - strlen("register8"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x10\n");
        _output.push_back("register9"     + std::string(LABEL_TRUNC_SIZE - strlen("register9"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x12\n");
        _output.push_back("register10"    + std::string(LABEL_TRUNC_SIZE - strlen("register10"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x14\n");
        _output.push_back("register11"    + std::string(LABEL_TRUNC_SIZE - strlen("register11"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x16\n");
        _output.push_back("register12"    + std::string(LABEL_TRUNC_SIZE - strlen("register12"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x18\n");
        _output.push_back("register13"    + std::string(LABEL_TRUNC_SIZE - strlen("register13"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1A\n");
        _output.push_back("register14"    + std::string(LABEL_TRUNC_SIZE - strlen("register14"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1C\n");
        _output.push_back("register15"    + std::string(LABEL_TRUNC_SIZE - strlen("register15"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x1E\n");
        _output.push_back("fgbgColour"    + std::string(LABEL_TRUNC_SIZE - strlen("fgbgColour"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x20\n");
        _output.push_back("cursorXY"      + std::string(LABEL_TRUNC_SIZE - strlen("cursorXY"), ' ')      + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x22\n");
        _output.push_back("midiStream"    + std::string(LABEL_TRUNC_SIZE - strlen("midiStream"), ' ')    + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x24\n");
        _output.push_back("midiDelay"     + std::string(LABEL_TRUNC_SIZE - strlen("midiDelay"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x26\n");
        _output.push_back("miscFlags"     + std::string(LABEL_TRUNC_SIZE - strlen("miscFlags"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x28\n");
        _output.push_back("timerTick"     + std::string(LABEL_TRUNC_SIZE - strlen("timerTick"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x2A\n");
        _output.push_back("timerPrev"     + std::string(LABEL_TRUNC_SIZE - strlen("timerPrev"), ' ')     + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "register0 + 0x2C\n");
        _output.push_back("\n");

        _output.push_back("; Internal Constants\n");
        _output.push_back("ENABLE_SCROLL_BIT" + std::string(LABEL_TRUNC_SIZE - strlen("ENABLE_SCROLL_BIT"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(ENABLE_SCROLL_BIT) + "\n");
        _output.push_back("ON_BOTTOM_ROW_BIT" + std::string(LABEL_TRUNC_SIZE - strlen("ON_BOTTOM_ROW_BIT"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(ON_BOTTOM_ROW_BIT) + "\n");
        _output.push_back("DISABLE_CLIP_BIT"  + std::string(LABEL_TRUNC_SIZE - strlen("DISABLE_CLIP_BIT"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(DISABLE_CLIP_BIT)  + "\n");
        _output.push_back("ENABLE_SCROLL_MSK" + std::string(LABEL_TRUNC_SIZE - strlen("ENABLE_SCROLL_MSK"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(ENABLE_SCROLL_MSK) + "\n");
        _output.push_back("ON_BOTTOM_ROW_MSK" + std::string(LABEL_TRUNC_SIZE - strlen("ON_BOTTOM_ROW_MSK"), ' ') + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(ON_BOTTOM_ROW_MSK) + "\n");
        _output.push_back("DISABLE_CLIP_MSK"  + std::string(LABEL_TRUNC_SIZE - strlen("DISABLE_CLIP_MSK"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(DISABLE_CLIP_MSK)  + "\n");
        _output.push_back("\n");

        _output.push_back("; Internal Buffers\n");
        _output.push_back("regsWorkArea" + std::string(LABEL_TRUNC_SIZE - strlen("regsWorkArea"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(getRegWorkArea()) + "\n");
        _output.push_back("textWorkArea" + std::string(LABEL_TRUNC_SIZE - strlen("textWorkArea"), ' ')  + "EQU" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + Expression::wordToHexString(getStrWorkArea()) + "\n");
        _output.push_back("\n");
    }

    void outputIncludes(void)
    {
        _output.push_back("; Includes\n");
        _output.push_back("%includePath" + std::string(LABEL_TRUNC_SIZE - strlen("%includePath"), ' ') + "\"" + getRuntimePath() + "\"\n");
        _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "util.i\n");
        _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "gigatron.i\n");

        if(_codeRomType < Cpu::ROMv5a)
        {
            _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "macros.i\n");
        }
        else
        {
            _output.push_back("%include" + std::string(LABEL_TRUNC_SIZE - strlen("%include"), ' ') + "macros_ROMv5a.i\n");
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
                    // Skip sys init funcs that are commented out, (when not used)
                    if(_codeLines[i]._vasm[j]._opcode[0] == ';') continue;

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
                        // Assembler preprocessing commands
                        if(vasmCode[0] == '%')
                        {
                            line += "\n" + vasmCode;
                        }
                        // vASM labels and code
                        else
                        {
                            line += (label.size()) ?  "\n" + label + std::string(LABEL_TRUNC_SIZE - label.size(), ' ') + vasmCode : "\n" + std::string(LABEL_TRUNC_SIZE, ' ') + vasmCode;
                        }
                    }
                }

#define TAB_SPACE_LENGTH  4
#define COMMENT_PADDING  (TAB_SPACE_LENGTH*24)
                // Commented BASIC code, (assumes any tabs are 4 spaces)
                int lineLength = Expression::tabbedStringLength(line, TAB_SPACE_LENGTH);
                line.append(COMMENT_PADDING - (lineLength % COMMENT_PADDING), ' ');
                //fprintf(stderr, "%d\n", lineLength + COMMENT_PADDING - (lineLength % COMMENT_PADDING));

                // Line spacing for parsed code and non parsed code is different
                bool dontParse = (i+1 < int(_codeLines.size())) ? _codeLines[i+1]._dontParse : false;
                std::string newLine = (_codeLines[i]._dontParse  &&  dontParse) ? "\n" : "\n\n";
                line += "; " + _codeLines[i]._text + newLine;
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
        _heapTotalUsage = 0;
        _heapAllocations = 0;

        _vasmPC          = USER_CODE_START;
        _userCodeStart   = USER_CODE_START;
        _tempVarSize     = TEMP_VAR_SIZE;
        _tempVarStart    = TEMP_VAR_START;
        _userVarStart    = USER_VAR_START;
        _userVarsAddr    = USER_VAR_START;
        _runtimeEnd      = RUN_TIME_START;
        _runtimeStart    = RUN_TIME_START;
        _arraysStart     = RUN_TIME_START;
        _stringsStart    = RUN_TIME_START;
        _regWorkArea     = 0x0000;
        _gprintfVarsAddr = 0x0000;
        _strWorkArea[0]  = 0x0000;
        _strWorkArea[1]  = 0x0000;
        _strWorkAreaIdx  = 0;

        _spritesAddrLutAddress  = 0x0000;
        _spriteStripeChunks     = 15;
        _spriteStripeMinAddress = USER_CODE_START;

        _codeOptimiseType = CodeSpeed;
        _codeRomType = Cpu::ROMv3;

        _codeIsAsm = false;
        _arrayIndiciesOne = false;
        _createNumericLabelLut = false;
        _createTimeData = false;

        _currentLabelIndex = -1;
        _currentCodeLineIndex = 0;
        _nonNumericLabelIndex = -1;
        _jumpFalseUniqueId = 0;

        _runtimePath = ".";
        _tempVarStartStr = "";
        _nextInternalLabel = "";

        _input.clear();
        _output.clear();
        _runtime.clear();

        _labels.clear();
        _gosubLabels.clear();
        _equateLabels.clear();
        _internalLabels.clear();
        _discardedLabels.clear();

        _codeLines.clear();
        _moduleLines.clear();
        _constants.clear();
        _integerVars.clear();
        _stringVars.clear();
        _typeDatas.clear();

        _macroLines.clear();
        _macroNameEntries.clear();
        _macroIndexEntries.clear();

        _defDataBytes.clear();
        _defDataWords.clear();
        _defDataImages.clear();
        _defDataLoaderImageChunks.clear();
        _defDataMidis.clear();
        _defDataOpens.clear();
        _defDataSprites.clear();
        _defDataFonts.clear();
        _defFunctions.clear();
        _dataObjects.clear();

        _spritesAddrLut._address = 0x0000;
        _spritesAddrLut._spriteAddrs.clear();

        _fontsAddrLut._address = 0x0000;
        _fontsAddrLut._fontAddrs.clear();

        Linker::reset();
        Linker::disableFontLinking();

        Memory::initialise();
        Operators::initialise();

        Keywords::reset();

        Expression::setExprFunc(expression);

        while(!_forNextDataStack.empty())     _forNextDataStack.pop();
        while(!_elseIfDataStack.empty())      _elseIfDataStack.pop();
        while(!_whileWendDataStack.empty())   _whileWendDataStack.pop();
        while(!_repeatUntilDataStack.empty()) _repeatUntilDataStack.pop();

        _callDataMap.clear();
        _procDataMap.clear();
        while(!_procDataStack.empty()) _procDataStack.pop();

        // Allocate default string work area, (for string functions like LEFT$, MID$, etc), the +2 is for the length and delimiter bytes
        Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, _stringsStart, _strWorkArea[0]);
        Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, _stringsStart, _strWorkArea[1]);
    }

    bool compile(const std::string& inputFilename, const std::string& outputFilename)
    {
        Assembler::clearAssembler();
        clearCompiler();

        // Read .gbas file
        int numLines = 0;
        std::ifstream infile(inputFilename);
        if(!readInputFile(infile, inputFilename, _input, numLines)) return false;
        for(int i=0; i<int(_input.size()); i++) _moduleLines.push_back({i, MODULE_MAIN});


        fprintf(stderr, "\n\n*******************************************************\n");
        fprintf(stderr, "* Compiling file '%s'\n", inputFilename.c_str());
        fprintf(stderr, "*******************************************************\n");

        // Pragmas
        if(!parsePragmas(_input, numLines)) return false;

        // Relies on _codeRomType_, so make sure _codeRomType_ is already initialised
        if(!initialiseMacros()) return false;

        // Initialise
        if(!initialiseCode()) return false;

        // Modules
        if(!parseModules(_input, numLines)) return false;

//#define DEBUG_DATA
#ifdef DEBUG_DATA
        std::ofstream ofile0("input.txt", std::ios::out);
        for(int i=0; i<int(_input.size()); i++)
        {
            ofile0 << _input[i]._parse << " : " << _input[i]._text << std::endl;
        }
        std::ofstream ofile1("module.txt", std::ios::out);
        for(int i=0; i<int(_moduleLines.size()); i++)
        {
            ofile1 << _moduleLines[i]._index << " : " << _moduleLines[i]._name << std::endl;
        }
#endif

        // Labels
        if(!parseLabels(_input, numLines)) return false;

        // Includes
        if(!Linker::parseIncludes()) return false;

        // Code
        if(!parseCode()) return false;

#ifdef DEBUG_DATA
        std::ofstream ofile2("code.txt", std::ios::out);
        for(int i=0; i<int(_codeLines.size()); i++)
        {
            ofile2 << _codeLines[i]._module << " : " << _codeLines[i]._code << std::endl;
            for(int j=0; j<int(_codeLines[i]._vasm.size()); j++)
            {
                ofile2 << std::hex << _codeLines[i]._vasm[j]._address << " : " << _codeLines[i]._vasm[j]._opcode << "    " << _codeLines[i]._vasm[j]._operand << std::endl;
            }
        }
#endif

        // Optimise
        if(!Optimiser::optimiseCode()) return false;

        // Check for code relocations
        if(!Validater::checkForRelocations()) return false;

        // Check keywords that form statement blocks
        if(!Validater::checkStatementBlocks()) return false;

        // Check PROC's, FUNC's and CALL's
        if(!Validater::checkCallProcFuncData()) return false;

        // Only link runtime subroutines that are referenced
        if(!Linker::linkInternalSubs()) return false;

#ifndef STAND_ALONE
        // GBAS gprintf's need to be converted to vCPU gprintf's after all relocation/linking has finished
        if(!Keywords::convertGprintGbasToGprintfAsm()) return false;
#endif

        // Output
        outputReservedWords();
        outputInternalEquates();
        outputIncludes();
        outputLabels();
        outputConsts();
        outputVars();
        outputArrs();

        if(!outputStrs()) return false;
        if(!outputDATA()) return false;
        if(!outputTIME()) return false;
        if(!outputDefs()) return false;
        if(!outputLuts()) return false;

        outputCode();

        // Discard
        discardUnusedLabels();

        // Re-linking is needed here as collectInternalRuntime() can find new subs that need to be linked
        Linker::collectInternalRuntime();
        Linker::relinkInternalSubs();
        Linker::outputInternalSubs();

        if(!Validater::checkBranchLabels()) return false;

        //Memory::printFreeRamList(Memory::SizeDescending);

        // Write .vasm file
        std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
        if(!writeOutputFile(outfile, outputFilename)) return false;

        //fprintf(stderr, "\nHeap allocations %llu, total heap usage %llu\n", _heapAllocations, _heapTotalUsage);

        return true;
    }
}
