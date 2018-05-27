#ifndef EXPRESSION_H
#define EXPRESSION_H


namespace Expression
{
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary};


    void initialise(void);
    std::string& strToUpper(std::string& s);

    NumericType getBase(const std::string& input, long& result);
    bool stringToU8(const std::string& token, uint8_t& result);
    bool stringToU16(const std::string& token, uint16_t& result);

    uint16_t expression(void);
    uint16_t parse(char* expressionToParse);
}

#endif
