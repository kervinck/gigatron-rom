#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "expression.h"


namespace Expression
{
    char* _expressionToParse;
    char* _expression;

    int _lineNumber = 0;

    bool _binaryChars[256]      = {false};
    bool _octalChars[256]       = {false};
    bool _decimalChars[256]     = {false};
    bool _hexaDecimalChars[256] = {false};

    bool _containsQuotes = false;

    exprFuncPtr _exprFunc;


    // Forward declarations
    Numeric expression(void);


    // Default operators
    Numeric neg(Numeric& numeric)
    {
        numeric._value = -numeric._value;
        return numeric;
    }
    Numeric add(Numeric& left, Numeric& right)
    {
        left._value += right._value;
        return left;
    }
    Numeric sub(Numeric& left, Numeric& right)
    {
        left._value -= right._value;
        return left;
    }
    Numeric mul(Numeric& left, Numeric& right)
    {
        left._value *= right._value;
        return left;
    }
    Numeric div(Numeric& left, Numeric& right)
    {
        left._value = (right._value == 0) ? 0 : left._value / right._value;
        return left;
    }

    void setExprFunc(exprFuncPtr exprFunc) {_exprFunc = exprFunc;}


    void initialise(void)
    {
        bool* b = _binaryChars;      b['0']=1; b['1']=1;
        bool* o = _octalChars;       o['0']=1; o['1']=1; o['2']=1; o['3']=1; o['4']=1; o['5']=1; o['6']=1; o['7']=1;
        bool* d = _decimalChars;     d['0']=1; d['1']=1; d['2']=1; d['3']=1; d['4']=1; d['5']=1; d['6']=1; d['7']=1; d['8']=1; d['9']=1;
        bool* h = _hexaDecimalChars; h['0']=1; h['1']=1; h['2']=1; h['3']=1; h['4']=1; h['5']=1; h['6']=1; h['7']=1; h['8']=1; h['9']=1; h['A']=1; h['B']=1; h['C']=1; h['D']=1; h['E']=1; h['F']=1;

        setExprFunc(expression);
    }

    ExpressionType isExpression(const std::string& input)
    {
        if(input.find_first_of("[]") != std::string::npos) return Invalid;
        if(input.find("++") != std::string::npos) return Invalid;
        if(input.find("--") != std::string::npos) return Invalid;
        if(input.find_first_of("+-*/()") != std::string::npos) return Valid;
        return None;
    }

    bool hasNonStringWhiteSpace(int chr)
    {
        if(chr == '"') _containsQuotes = !_containsQuotes;
        if(!isspace(chr) || _containsQuotes) return false;
        return true;    
    }

    bool hasNonStringEquals(int chr)
    {
        if(chr == '"') _containsQuotes = !_containsQuotes;
        if(chr != '='  ||  _containsQuotes) return false;
        return true;    
    }

    std::string::const_iterator findNonStringEquals(const std::string& input)
    {
        _containsQuotes = false;
        return std::find_if(input.begin(), input.end(), hasNonStringEquals);
    }

    void stripWhitespace(std::string& input)
    {
        _containsQuotes = false;
        input.erase(remove_if(input.begin(), input.end(), hasNonStringWhiteSpace), input.end());
    }

    void padString(std::string &str, int num, char pad)
    {
        if(num > str.size()) str.insert(0, num - str.size(), pad);
    }

    void addString(std::string &str, int num, char add)
    {
        if(num > 0) str.append(num, add);
    }

    void operatorReduction(std::string& input)
    {
        size_t ss, aa, sa, as;

        do
        {
            ss = input.find("--");
            if(ss != std::string::npos)
            {
                input.erase(ss, 2);
                input.insert(ss, "+");
            }

            aa = input.find("++");
            if(aa != std::string::npos)
            {
                input.erase(aa, 2);
                input.insert(aa, "+");
            }

            sa = input.find("-+");
            if(sa != std::string::npos)
            {
                input.erase(sa, 2);
                input.insert(sa, "-");
            }

            as = input.find("+-");
            if(as != std::string::npos)
            {
                input.erase(as, 2);
                input.insert(as, "-");
            }
        }
        while(ss != std::string::npos  ||  aa != std::string::npos  ||  sa != std::string::npos  ||  as != std::string::npos);
    }

    std::string byteToHexString(uint8_t n)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)n;
        return "0x" + ss.str();
    }

    std::string wordToHexString(uint16_t n)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(4) << n;
        return "0x" + ss.str();
    }

    std::string& strToUpper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return toupper(c);} );
        return s;
    }

    NumericType getBase(const std::string& input, long& result)
    {
        bool success = true;
        std::string token = input;
        strToUpper(token);
        
        // Hex
        if(token.size() >= 2  &&  token.c_str()[0] == '$')
        {
            for(int i=1; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[1], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Hex
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  token.c_str()[1] == 'X')
        {
            for(int i=2; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Octal
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'O' || token.c_str()[1] == 'Q'))
        {
            for(int i=2; i<token.size(); i++) success &= _octalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 8);
                return Octal; 
            }
        }
        // Binary
        else if(token.size() >= 3  &&  token.c_str()[0] == '0'  &&  token.c_str()[1] == 'B')
        {
            for(int i=2; i<token.size(); i++) success &= _binaryChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 2);
                return Binary; 
            }
        }
        // Decimal
        else
        {
            for(int i=0; i<token.size(); i++) success &= _decimalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[0], NULL, 10);
                return Decimal; 
            }
        }

        return BadBase;
    }

    bool stringToI8(const std::string& token, int8_t& result)
    {
        if(token.size() < 1  ||  token.size() > 10) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = int8_t(lResult);
        return true;
    }

    bool stringToU8(const std::string& token, uint8_t& result)
    {
        if(token.size() < 1  ||  token.size() > 10) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = uint8_t(lResult);
        return true;
    }

    bool stringToI16(const std::string& token, int16_t& result)
    {
        if(token.size() < 1  ||  token.size() > 18) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = int16_t(lResult);
        return true;
    }

    bool stringToU16(const std::string& token, uint16_t& result)
    {
        if(token.size() < 1  ||  token.size() > 18) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = uint16_t(lResult);
        return true;
    }

    char peek(void)
    {
        return *_expression;
    }

    char get(void)
    {
        return *_expression++;
    }

    char* getExpression(void)
    {
        return _expression;
    }

    bool number(int16_t& value)
    {
        char uchr;

        std::string valueStr;
        uchr = toupper(peek());
        valueStr.push_back(uchr); get();
        uchr = toupper(peek());
        if((uchr >= '0'  &&  uchr <= '9')  ||  uchr == 'X'  ||  uchr == 'B'  ||  uchr == 'O'  ||  uchr == 'Q')
        {
            valueStr.push_back(uchr); get();
            while((peek() >= '0'  &&  peek() <= '9')  ||  (peek() >= 'A'  &&  peek() <= 'F'))
            {
                valueStr.push_back(get());
            }
        }

        return stringToI16(valueStr, value);
    }

    Numeric fac(int16_t defaultValue)
    {
        int16_t value = 0;
        if(peek() == '(')
        {
            get();
            Numeric numeric = expression();
            get();
            return numeric;
        }
        else if(peek() == '-')
        {
            get();
            return neg(fac(0));
        }
        else if((peek() >= '0'  &&  peek() <= '9')  ||  peek() == '$')
        {
            if(!number(value))
            {
                fprintf(stderr, "Expression::factor() : Bad numeric data in '%s' on line %d\n", _expressionToParse, _lineNumber + 1);
                value = 0;
            }
            return Numeric(value, false, nullptr);
        }

        Numeric numeric = Numeric(defaultValue, true, _expression);
        return numeric;
    }

    Numeric term(void)
    {
        Numeric result = fac(0);
        while(peek() == '*'  ||  peek() == '/')
        {
            if(get() == '*')
            {
                result = mul(result, fac(0));
            }
            else
            {
                Numeric f = fac(0);
                if(f._value == 0)
                {
                    result = mul(result, f);
                }
                else
                {
                    result = div(result, f);
                }
            }
        }

        return result;
    }

    Numeric expression(void)
    {
        Numeric result = term();
        while(peek() == '+' || peek() == '-')
        {
            if(get() == '+')
            {
                result = add(result, term());
            }
            else
            {
                result = sub(result, term());
            }
        }

        return result;
    }

    int16_t parse(char* expressionToParse, int lineNumber)
    {
        _expressionToParse = expressionToParse;
        _expression = expressionToParse;
        _lineNumber = lineNumber;

        return _exprFunc()._value;
    }
}