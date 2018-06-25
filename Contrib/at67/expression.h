#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <functional>


namespace Expression
{
    enum ExpressionType {Invalid=-1, None, HasAlpha, Valid};
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary};

    struct Numeric
    {
        Numeric() {_value = 0; _isAddress = false; _varNamePtr = nullptr;}
        Numeric(uint16_t value, bool isAddress, char* varNamePtr) {_value = value; _isAddress = isAddress; _varNamePtr = varNamePtr;}

        uint16_t _value;
        bool _isAddress = false;
        char* _varNamePtr = nullptr;
    };

    using unaryOpFuncPtr = std::function<Numeric(const Numeric& value)>;
    using binaryOpFuncPtr = std::function<Numeric(Numeric& result, const Numeric& value)>;


    void setNegFunc(unaryOpFuncPtr negFunc);
    void setAddFunc(binaryOpFuncPtr addFunc);
    void setSubFunc(binaryOpFuncPtr subFunc);
    void setMulFunc(binaryOpFuncPtr mulFunc);
    void setDivFunc(binaryOpFuncPtr divFunc);
    void setDefaultOperatorFuncs(void);

    void initialise(void);

    ExpressionType isExpression(const std::string& input);

    void padString(std::string &str, int num, char pad=' ');
    void addString(std::string &str, int num, char add=' ');
    void stripWhitespace(std::string& input);
    std::string byteToHexString(uint8_t n);
    std::string wordToHexString(uint16_t n);
    std::string& strToUpper(std::string& s);

    NumericType getBase(const std::string& input, long& result);
    bool stringToU8(const std::string& token, uint8_t& result);
    bool stringToU16(const std::string& token, uint16_t& result);

    uint16_t parse(char* expressionToParse, int lineNumber);
}

#endif
