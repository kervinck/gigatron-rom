#ifndef COMPILER_H
#define COMPILER_H

#include <vector>
#include <stack>
#include <map>
#include <algorithm>

#include "expression.h"


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
    enum IfElseEndType {IfBlock, ElseIfBlock, ElseBlock, EndIfBlock};

    struct InternalLabel
    {
        uint16_t _address;
        std::string _name;
    };

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

    struct VasmLine
    {
        uint16_t _address;
        std::string _opcode;
        std::string _code;
        std::string _internalLabel;
        bool _pageJump = false;
        int _vasmSize = 0;
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

    struct ForNextData
    {
        int _varIndex;
        std::string _labelName;
        int16_t _loopEnd;
        int16_t _loopStep;
        uint16_t _varEnd;
        uint16_t _varStep;
        bool _optimise = false;
        int _codeLineIndex;
    };

    struct ElseIfData
    {
        int _jmpIndex;
        std::string _labelName;
        int _codeLineIndex;
        IfElseEndType _ifElseEndType;
    };

    struct EndIfData
    {
        int _jmpIndex;
        int _codeLineIndex;
    };

    struct WhileWendData
    {
        int _jmpIndex;
        std::string _labelName;
        int _codeLineIndex;
    };

    struct DoUntilData
    {
        std::string _labelName;
        int _codeLineIndex;
    };


    uint16_t getRuntimeEnd(void);
    void setRuntimeEnd(uint16_t runtimeEnd);

    std::vector<Label>& getLabels(void);
    std::vector<CodeLine>& getCodeLines(void);
    std::vector<InternalLabel>& getInternalLabels(void);
    std::vector<InternalLabel>& getDiscardedLabels(void);
    std::vector<std::string>& getOutput(void);
    std::vector<std::string>& getRuntime(void);

    std::map<std::string, MacroIndexEntry>& getMacroIndexEntries(void);

    std::stack<ForNextData>& getForNextDataStack(void);
    std::stack<ElseIfData>& getElseIfDataStack(void);
    std::stack<EndIfData>& getEndIfDataStack(void);
    std::stack<WhileWendData>& getWhileWendDataStack(void);
    std::stack<DoUntilData>& getDoUntilDataStack(void);

    bool initialise(void);
    bool initialiseMacros(void);

    Expression::Numeric expression(void);

    int findLabel(const std::string& labelName);
    int findLabel(uint16_t address);
    int findInternalLabel(const std::string& labelName);
    int findInternalLabel(uint16_t address);
    int findVar(std::string& varName);

    bool findMacroText(const std::string& macroName, const std::string& text);
    int getMacroSize(const std::string& macroName);
    int createVcpuAsm(const std::string& opcodeStr, const std::string& operandStr, int codeLineIdx, std::string& line);

    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
