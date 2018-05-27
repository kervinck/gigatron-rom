#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

#include "expression.h"


// Part of this code was derived from https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c
namespace Expression
{
    char* _expressionToParse;

    bool _binaryChars[256] = {false};
    bool _octalChars[256] = {false};
    bool _decimalChars[256] = {false};
    bool _hexaDecimalChars[256] = {false};


    void initialise(void)
    {
        bool* b = _binaryChars;      b['0']=1; b['1']=1;
        bool* o = _octalChars;       o['0']=1; o['1']=1; o['2']=1; o['3']=1; o['4']=1; o['5']=1; o['6']=1; o['7']=1;
        bool* d = _decimalChars;     d['0']=1; d['1']=1; d['2']=1; d['3']=1; d['4']=1; d['5']=1; d['6']=1; d['7']=1; d['8']=1; d['9']=1;
        bool* h = _hexaDecimalChars; h['0']=1; h['1']=1; h['2']=1; h['3']=1; h['4']=1; h['5']=1; h['6']=1; h['7']=1; h['8']=1; h['9']=1; h['A']=1; h['B']=1; h['C']=1; h['D']=1; h['E']=1; h['F']=1;
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

    bool stringToU8(const std::string& token, uint8_t& result)
    {
        if(token.size() < 1  ||  token.size() > 10) return false;

        long lResult;
        NumericType base = getBase(token, lResult);
        if(base == BadBase) return false;

        result = uint8_t(lResult);
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
        return *_expressionToParse;
    }

    char get(void)
    {
        return *_expressionToParse++;
    }

    bool number(uint16_t& value)
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

        return stringToU16(valueStr, value);
    }

    uint16_t factor(void)
    {
        uint16_t value = 0;
        if((peek() >= '0'  &&  peek() <= '9')  ||  peek() == '$')
        {
            if(!number(value))
            {
                fprintf(stderr, "Expression::factor() : Bad numeric data in '%s'\n", _expressionToParse);
                value = 0;
            }
            return value;
        }
        else if(peek() == '(')
        {
            get(); // '('
            uint16_t result = expression();
            get(); // ')'
            return result;
        }
        else if(peek() == '-')
        {
            get();
            return -factor();
        }
        return 0; // error
    }

    uint16_t term(void)
    {
        uint16_t result = factor();
        while(peek() == '*'  ||  peek() == '/')
        {
            (get() == '*') ? result *= factor() : result /= factor();
        }

        return result;
    }

    uint16_t expression(void)
    {
        uint16_t result = term();
        while(peek() == '+' || peek() == '-')
        {
            (get() == '+') ? result += term() : result -= term();
        }

        return result;
    }

    uint16_t parse(char* expressionToParse)
    {
        _expressionToParse = expressionToParse;

        return expression();
    }
}