#ifndef GBEXPR_H
#define GBEXPR_H


namespace GBexpr
    enum ExpressionType {Invalid=-1, None, Valid};
    enum NumericType {BadBase=-1, Decimal, HexaDecimal, Octal, Binary};


    void initialise(void);

    ExpressionType isExpression(const std::string& input);

    std::string& strToUpper(std::string& s);

    NumericType getBase(const std::string& input, long& result);
    bool stringToU8(const std::string& token, uint8_t& result);
    bool stringToU16(const std::string& token, uint16_t& result);

    bool parse(const std::string& input, std::string& output);
}

#endif
