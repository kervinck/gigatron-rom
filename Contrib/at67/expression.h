#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <functional>


namespace Expression
{
    enum ExpressionType {Invalid=0x8000, HasNumbers=0x0000, HasStrings=0x0001, HasOperators=0x0002, HasVars=0x0004, HasKeywords=0x0008};
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary};

    struct Numeric
    {
        Numeric() {_value = 0; _isValid = false; _isAddress = false; _varName="";}
        Numeric(int16_t value, bool isValid, bool isAddress, std::string& varName) {_value = value; _isValid = isValid; _isAddress = isAddress; _varName = varName;}

        int16_t _value;
        bool _isValid = false;
        bool _isAddress = false;
        std::string _varName;
    };

    using exprFuncPtr = std::function<Numeric (void)>;

    bool getEnablePrint(void);

    void setExprFunc(exprFuncPtr exprFunc);
    void setEnablePrint(bool enablePrint);

    void initialise(void);

    ExpressionType isExpression(const std::string& input);

    bool hasNonStringWhiteSpace(int chr);
    bool hasNonStringEquals(int chr);

    std::string::const_iterator findNonStringEquals(const std::string& input);
    void stripNonStringWhitespace(std::string& input);
    void stripWhitespace(std::string& input);
    std::string stripStrings(const std::string& input);
    void trimWhitespace(std::string& input);
    std::string collapseWhitespace(std::string& input);
    std::string collapseWhitespaceNotStrings(std::string& input);
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

    std::vector<std::string> tokenise(const std::string& text, const std::string& delimiters, bool toUpper=false);
    std::vector<std::string> tokenise(const std::string& text, char c, bool skipSpaces=true, bool toUpper=false);
    std::vector<std::string> tokenise(const std::string& text, char c, std::vector<size_t>& offsets, bool skipSpaces=true, bool toUpper=false);
    std::vector<std::string> tokeniseLine(std::string& line, const std::string& delimiters=" \n\r\f\t\v");

    void replaceKeyword(std::string& expression, const std::string& keyword, const std::string& replace);

    char* getExpression(void);
    const char* getExpressionToParse(void);
    std::string& getExpressionToParseString(void);
    int getLineNumber(void);

    char peek(void);
    char get(void);
    bool advance(uintptr_t n);
    bool find(const std::string& text);

    bool number(int16_t& value);
    Numeric expression(void);
    bool parse(const std::string& expression, int lineNumber, int16_t& value);
}

#endif
