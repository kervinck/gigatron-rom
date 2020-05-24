#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <functional>


#define MATH_PI 3.1415926535897932384626433832795028


namespace Expression
{
    enum ExpressionType {IsInvalid=0x8000, HasNumbers=0x0000, HasStrings=0x0001, HasStrConsts=0x0002, HasStrVars=0x0004, HasStringKeywords=0x0008, IsStringExpression=0x000F,
                         HasOperators=0x0010, HasIntConsts=0x0020, HasIntVars=0x0040, HasPragmas=0x0080, HasKeywords=0x0100, HasFunctions=0x0200, HasOptimisedPrint=0x0400};
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary};
    enum CCType {BooleanCC, NormalCC, FastCC};
    enum Int16Byte {Int16Both, Int16Low, Int16High};
    enum VarType {Number, String, Constant, TmpVar, IntVar, Arr1Var, Arr2Var, Arr3Var, StrVar, Str2Var, TmpStrVar};
    enum SysMacro {SysNone, SysInitEqOp, SysInitNeOp, SysInitLeOp, SysInitGeOp, SysInitLtOp, SysInitGtOp, SysInitArray2d, SysInitArray3d, SysInitRealTimeStub};

    struct Numeric
    {
        Numeric()
        {
            _value = 0.0; _index = -1; _isValid = false; _staticInit = false; _relocatable = false; _varType = Number; _ccType = BooleanCC; _int16Byte = Int16Both; _name = ""; _text = "";
        }

        Numeric(bool staticInit) : Numeric()
        {
            _staticInit = staticInit;
        }

        Numeric(double value, int16_t index, bool isValid, bool staticInit, bool relocatable, VarType varType, CCType ccType, Int16Byte int16Byte, const std::string& name, const std::string& text)
        {
            _value = value; _index = index; _isValid = isValid; _staticInit = staticInit; _relocatable = relocatable; _varType = varType; _ccType = ccType; _int16Byte = int16Byte; _name = name; _text = text;
        }

        double _value = 0.0;
        int16_t _index = -1;
        bool _isValid = false;
        bool _staticInit = false;
        bool _relocatable = false;
        VarType _varType = Number;
        CCType _ccType = BooleanCC;
        Int16Byte _int16Byte = Int16Both;
        std::string _name;
        std::string _text;
        std::vector<Numeric> _parameters;
    };

    using exprFuncPtr = std::function<Numeric (void)>;

    template <typename T> int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }


    bool getEnableOptimisedPrint(void);
    Numeric& getOutputNumeric(void);

    void setExprFunc(exprFuncPtr exprFunc);
    void setEnableOptimisedPrint(bool enableOptimisedPrint);

    void initialise(void);

    ExpressionType isExpression(const std::string& input);
    bool isVarNameValid(const std::string& varName);
    bool isLabNameValid(const std::string& varName);
    bool isStringValid(const std::string& input);

    bool hasNonStringWhiteSpace(int chr);
    bool hasNonStringEquals(int chr);
    bool hasNonStringColon(int chr);

    std::string::const_iterator findNonStringEquals(const std::string& input);
    std::string::const_iterator findNonStringColon(const std::string& input);
    void stripNonStringWhitespace(std::string& input);
    void stripWhitespace(std::string& input);
    std::string stripStrings(const std::string& input);
    std::string stripStrings(const std::string& input, std::vector<std::string>& strings, bool saveExtraFields=false);
    void trimWhitespace(std::string& input);
    std::string collapseWhitespace(const std::string& input);
    std::string collapseWhitespaceNotStrings(const std::string& input);
    std::string removeCommentsNotInStrings(const std::string& input);
    void padString(std::string &str, int num, char pad=' ');
    void addString(std::string &str, int num, char add=' ');
    int tabbedStringLength(const std::string& input, int tabSize);
    void operatorReduction(std::string& input);
    bool findMatchingBrackets(const std::string& input, size_t start, size_t& lbra, size_t& rbra);

    std::string byteToHexString(uint8_t n);
    std::string wordToHexString(uint16_t n);
    std::string& strToLower(std::string& s);
    std::string& strToUpper(std::string& s);
    std::string getSubAlpha(const std::string& s);

    NumericType getBase(const std::string& input, long& result);
    bool stringToI8(const std::string& token, int8_t& result);
    bool stringToU8(const std::string& token, uint8_t& result);
    bool stringToI16(const std::string& token, int16_t& result);
    bool stringToU16(const std::string& token, uint16_t& result);
    void stringToDouble(const std::string& token, double& result);

    std::vector<std::string> tokenise(const std::string& text, const std::string& delimiterStr, bool toUpper=false);
    std::vector<std::string> tokenise(const std::string& text, char c, bool skipSpaces=true, bool toUpper=false);
    std::vector<std::string> tokenise(const std::string& text, char c, std::vector<size_t>& offsets, bool skipSpaces=true, bool toUpper=false);
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiterStr=" \n\r\f\t\v");
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiterStr, std::vector<size_t>& offsets);

    void replaceText(std::string& expression, const std::string& text, const std::string& replace, size_t offset=0);

    char* getExpression(void);
    const char* getExpressionToParse(void);
    std::string& getExpressionToParseString(void);
    int getLineNumber(void);

    void setExpression(const std::string& expression, intptr_t n=0);

    char peek(void);
    char get(void);
    void save(void);
    void restore(void);
    bool advance(intptr_t n);

    bool find(const std::string& text);
    bool findFunc(const std::string& text);

    bool number(int16_t& value);
    Numeric expression(void);
    bool parse(const std::string& expression, int lineNumber, Numeric& numeric);
}

#endif
