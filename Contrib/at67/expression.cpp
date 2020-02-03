#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "expression.h"


namespace Expression
{
    std::string _expressionToParse;
    char* _expression;

    int _lineNumber = 0;

    bool _advanceError = false;
    bool _containsQuotes = false;
    bool _enableOptimisedPrint = false;

    bool _binaryChars[256]      = {false};
    bool _octalChars[256]       = {false};
    bool _decimalChars[256]     = {false};
    bool _hexaDecimalChars[256] = {false};

    uintptr_t _advancePtr;
    exprFuncPtr _exprFunc;

    Numeric _output;


    // Forward declarations
    Numeric expression(void);

    // Helpers
    uint32_t revHelper(uint32_t input, uint32_t n)
    {
        uint32_t output = 0;
        uint32_t bits = input & uint32_t(pow(2, n) - 1);
        for(uint32_t i=0; i<=n-1; i++)
        {
            output = (output << 1) | (bits & 1);
            bits = bits >> 1;
        }

        return output;
    }

    // Unary logic operators
    Numeric& operatorNEG(Numeric& numeric)
    {
        numeric._value = -numeric._value;
        return numeric;
    }
    Numeric& operatorNOT(Numeric& numeric)
    {
        numeric._value = ~int16_t(std::lround(numeric._value));
        return numeric;
    }

    // Unary math operators
    Numeric& operatorSIN(Numeric& numeric)
    {
        numeric._value = sin(numeric._value);
        return numeric;
    }
    Numeric& operatorCOS(Numeric& numeric)
    {
        numeric._value = cos(numeric._value);
        return numeric;
    }
    Numeric& operatorTAN(Numeric& numeric)
    {
        numeric._value = tan(numeric._value);
        return numeric;
    }
    Numeric& operatorASIN(Numeric& numeric)
    {
        numeric._value = asin(numeric._value);
        return numeric;
    }
    Numeric& operatorACOS(Numeric& numeric)
    {
        numeric._value = acos(numeric._value);
        return numeric;
    }
    Numeric& operatorATAN(Numeric& numeric)
    {
        numeric._value = atan(numeric._value);
        return numeric;
    }
    Numeric& operatorRAND(Numeric& numeric)
    {
        numeric._value = double(rand() % std::lround(numeric._value));
        return numeric;
    }
    Numeric& operatorREV16(Numeric& numeric)
    {
        numeric._value = double(revHelper(uint32_t(std::lround(numeric._value)), 16));
        return numeric;
    }
    Numeric& operatorREV8(Numeric& numeric)
    {
        numeric._value = double(revHelper(uint32_t(std::lround(numeric._value)), 8));
        return numeric;
    }
    Numeric& operatorREV4(Numeric& numeric)
    {
        numeric._value = double(revHelper(uint32_t(std::lround(numeric._value)), 4));
        return numeric;
    }

    // Binary logic operators
    Numeric& operatorAND(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) & int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorOR(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) | int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorXOR(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) ^ int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorLSL(Numeric& left, Numeric& right)
    {
        left._value = (int16_t(std::lround(left._value)) << int16_t(std::lround(right._value))) & 0x0000FFFF;
        return left;
    }
    Numeric& operatorLSR(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) >> int16_t(std::lround(right._value));
        return left;
    }

    // Binary math operators
    Numeric& operatorADD(Numeric& left, Numeric& right)
    {
        left._value += right._value;
        return left;
    }
    Numeric& operatorSUB(Numeric& left, Numeric& right)
    {
        left._value -= right._value;
        return left;
    }
    Numeric& operatorMUL(Numeric& left, Numeric& right)
    {
        left._value *= right._value;
        return left;
    }
    Numeric& operatorDIV(Numeric& left, Numeric& right)
    {
        left._value = (right._value == 0) ? 0 : left._value / right._value;
        return left;
    }
    Numeric& operatorMOD(Numeric& left, Numeric& right)
    {
        left._value = (right._value == 0) ? 0 : int16_t(std::lround(left._value)) % int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorPOW(Numeric& left, Numeric& right)
    {
        left._value = pow(left._value, right._value);
        return left;
    }

    // Relational operators
    Numeric& operatorLT(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) < int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorGT(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) > int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorEQ(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) == int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorNE(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) != int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorLE(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) <= int16_t(std::lround(right._value));
        return left;
    }
    Numeric& operatorGE(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::lround(left._value)) >= int16_t(std::lround(right._value));
        return left;
    }

    Numeric& getOutputNumeric(void) {return _output;}
    bool getEnableOptimisedPrint(void) {return _enableOptimisedPrint;}

    void setExprFunc(exprFuncPtr exprFunc) {_exprFunc = exprFunc;}
    void setEnableOptimisedPrint(bool enableOptimisedPrint) {_enableOptimisedPrint = enableOptimisedPrint;}


    void initialise(void)
    {
        bool* b = _binaryChars;      b['0']=1; b['1']=1;
        bool* o = _octalChars;       o['0']=1; o['1']=1; o['2']=1; o['3']=1; o['4']=1; o['5']=1; o['6']=1; o['7']=1;
        bool* d = _decimalChars;     d['0']=1; d['1']=1; d['2']=1; d['3']=1; d['4']=1; d['5']=1; d['6']=1; d['7']=1; d['8']=1; d['9']=1; d['-']=1;
        bool* h = _hexaDecimalChars; h['0']=1; h['1']=1; h['2']=1; h['3']=1; h['4']=1; h['5']=1; h['6']=1; h['7']=1; h['8']=1; h['9']=1; h['A']=1; h['B']=1; h['C']=1; h['D']=1; h['E']=1; h['F']=1;

        setExprFunc(expression);
    }


    // ****************************************************************************************************************
    // Strings
    // ****************************************************************************************************************
    int isSpace(int chr)
    {
        return isspace((unsigned char)chr);
    }

    ExpressionType isExpression(const std::string& input)
    {
        if(input.find_first_of("[]") != std::string::npos) return IsInvalid;
        if(input.find("++") != std::string::npos) return IsInvalid;
        if(input.find("--") != std::string::npos) return IsInvalid;
        if(input.find_first_of("~-+/%*()&|^<>") != std::string::npos) return HasOperators;
        if(input.find("**") != std::string::npos  ||  input.find(">>") != std::string::npos  ||  input.find("<<") != std::string::npos) return HasOperators;
        if(input.find("==") != std::string::npos  ||  input.find("!=") != std::string::npos  ||  input.find("<=") != std::string::npos  ||  input.find(">=") != std::string::npos) return HasOperators;
        return HasNumbers;
    }

    bool isVarNameValid(const std::string& varName)
    {
        if(varName.size() == 0)  return false;
        if(!isalpha((unsigned char)varName[0])) return false;

        for(int i=1; i<int(varName.size())-1; i++)
        {
            if(!isalnum((unsigned char)varName[i])) return false;
        }

        if(!isalnum((unsigned char)varName[varName.size() - 1])  &&  varName[varName.size() - 1] != '$') return false;

        return true;
    }

    bool isValidString(const std::string& input)
    {
        std::string str = input;
        stripNonStringWhitespace(str);
        if(str.size() < 2) return false;
        if(str[0] == '"'  &&  str.back() == '"')
        {
            for(int i=1; i<int(str.size())-1; i++)
            {
                if(str[i] == '"') return false;
            }

            return true;
        }

        return false;
    }

    bool hasNonStringWhiteSpace(int chr)
    {
        if(chr == '"') _containsQuotes = !_containsQuotes;
        if(!isspace((unsigned char)chr) || _containsQuotes) return false;
        return true;    
    }

    bool hasNonStringEquals(int chr)
    {
        if(chr == '"') _containsQuotes = !_containsQuotes;
        if(chr != '='  ||  _containsQuotes) return false;
        return true;    
    }

    bool hasNonStringColon(int chr)
    {
        if(chr == '"') _containsQuotes = !_containsQuotes;
        if(chr != ':'  ||  _containsQuotes) return false;
        return true;    
    }

    std::string::const_iterator findNonStringEquals(const std::string& input)
    {
        _containsQuotes = false;
        return std::find_if(input.begin(), input.end(), hasNonStringEquals);
    }

    std::string::const_iterator findNonStringColon(const std::string& input)
    {
        _containsQuotes = false;
        return std::find_if(input.begin(), input.end(), hasNonStringColon);
    }

    void stripNonStringWhitespace(std::string& input)
    {
        _containsQuotes = false;
        input.erase(remove_if(input.begin(), input.end(), hasNonStringWhiteSpace), input.end());
    }

    void stripWhitespace(std::string& input)
    {
        input.erase(remove_if(input.begin(), input.end(), isSpace), input.end());
    }

    std::string stripStrings(const std::string& input)
    {
        size_t start = 0, end = 0;

        std::string output = input;

        while(start != std::string::npos  &&  end != std::string::npos)
        {
            start = output.find_first_of('"', end + 1);
            end = output.find_first_of('"', start + 1);
            if(start == std::string::npos  ||  end == std::string::npos) break;

            output.erase(start, end - start + 1);
            start = 0, end = 0;
        }

        return output;
    }

    std::string stripStrings(const std::string& input, std::vector<std::string>& strings, bool saveExtraFields)
    {
        size_t start, end = std::string::npos;
        std::string output = input;

        do
        {
            start = output.find_first_of('"', end + 1);
            end = output.find_first_of('"', start + 1);
            if(start == std::string::npos  ||  end == std::string::npos) break;

            strings.push_back(output.substr(start, end - start + 1));
            output.erase(start, end - start + 1);

            // Save extra fields, (digits and semicolons)
            if(saveExtraFields  &&  output[start] != ',')
            {
                // Save semicolon
                if(output[start] == ';')
                {
                    strings.back() += ';';
                    output.erase(start, 1);
                }

                // Save digits
                if(isdigit((unsigned char)output[start]))
                {
                    size_t i = start;
                    while(isdigit((unsigned char)output[i]))
                    {
                        strings.back() += output[i++];
                    }
                    output.erase(start, i - start);
                }

                // Save semicolon
                if(output[start] == ';')
                {
                    strings.back() += ';';
                    output.erase(start, 1);
                }
            }
            start = 0, end = 0;
        }
        while(start != std::string::npos  &&  end != std::string::npos);

        return output;
    }

    void trimWhitespace(std::string& input)
    {
        size_t start = input.find_first_not_of(" \n\r\f\t\v");
        if(start == std::string::npos) return;

        size_t end = input.find_last_not_of(" \n\r\f\t\v");
        size_t size = end - start + 1;

        input = input.substr(start, size);
    }

    std::string collapseWhitespace(const std::string& input)
    {
        std::string output;

        std::unique_copy(input.begin(), input.end(), std::back_insert_iterator<std::string>(output), [](unsigned char a, unsigned char b) {return isspace(a) && isspace(b);});

        return output;
    }

    std::string collapseWhitespaceNotStrings(const std::string& input)
    {
        std::string output;
        int spaceCount = 0;
        bool inString = false;

        for(int i=0; i<int(input.size()); i++)
        {
            if(input[i] == '\"') inString = !inString;

            if(isspace((unsigned char)input[i]))
            {
                if(!inString)
                {
                    if(spaceCount++ == 0) output.push_back(input[i]);
                }
                else
                {
                    output.push_back(input[i]);
                }
            }
            else
            {
                spaceCount = 0;
                output.push_back(input[i]);
            }
        }

        return output;
    }

    std::string removeCommentsNotInStrings(const std::string& input)
    {
        std::string output;
        bool inString = false;
        bool inComment = false;

        for(int i=0; i<int(input.size()); i++)
        {
            // Check for string
            if(!inComment  &&  input[i] == '\"') inString = !inString;

            // Check for comment, ' and REM
            if(!inString)
            {
                if((input[i] == '\'')  ||  (i <= int(input.size()) - 3  &&  toupper((unsigned char)input[i]) == 'R'  &&  toupper((unsigned char)input[i+1]) == 'E'  &&  toupper((unsigned char)input[i+2]) == 'M'))
                {
                    inComment = true;
                }
            }

            if(!inString)
            {
                // If not in string but in comment, skip char
                if(inComment) continue;

                // Save char
                output.push_back(input[i]);
            }
            else
            {
                // Save char
                output.push_back(input[i]);
            }
        }

        return output;
    }

    void padString(std::string &input, int num, char pad)
    {
        if(num > int(input.size())) input.insert(0, num - int(input.size()), pad);
    }

    void addString(std::string &input, int num, char add)
    {
        if(num > 0) input.append(num, add);
    }

    int tabbedStringLength(const std::string& input, int tabSize)
    {
        int length = 0, newLine = 0;
        for(int i=0; i<int(input.size()); i++)
        {
            switch(input[i])
            {
                case '\n':
                case '\r': length = 0; newLine = i + 1;                   break;
                case '\t': length += tabSize - ((i - newLine) % tabSize); break;
                default:   length++;                                      break;
            }
        }

        return length;
    }

    bool findMatchingBrackets(const std::string& input, size_t start, size_t& lbra, size_t& rbra)
    {
        lbra = std::string::npos;
        rbra = std::string::npos;

        int matched = 0;
        bool startMatching = false;

        for(size_t i=start; i<input.size(); i++)
        {
            if(input[i] == '(')
            {
                if(!startMatching)
                {
                    lbra = i;
                    startMatching = true;
                }
                matched++;
            }

            if(input[i] == ')')
            {
                rbra = i;
                matched--;
            }

            if(startMatching  &&  matched == 0) return true;
        }

        lbra = std::string::npos;
        rbra = std::string::npos;

        return false;
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


    // ****************************************************************************************************************
    // String/number conversions
    // ****************************************************************************************************************
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

    std::string& strToLower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return char(tolower(c));} );
        return s;
    }

    std::string& strToUpper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return char(toupper(c));} );
        return s;
    }

    bool subAlphaHelper(int i)
    {
        // Valid chars are alpha and 'address of'
        return (isalpha((unsigned char)i) || (i=='@'));
    }
    std::string getSubAlpha(const std::string& s)
    {
        if(s.size() > 1)
        {
            char uchr = char(toupper((unsigned char)s[1]));
            if((s[0] == '&'  &&  (uchr == 'H'  ||  uchr == 'B'  ||  uchr == 'O'))  ||
               (s[0] == '0'  &&  (uchr == 'X'  ||  uchr == 'B'  ||  uchr == 'O')))
            {
                return s;
            }
        }

        auto it = std::find_if(s.begin(), s.end(), subAlphaHelper);
        if(it == s.end()) return std::string("");

        size_t start = it - s.begin();
        size_t end = s.find_first_of("-+/*%&<>=();,. ");
        if(end == std::string::npos) return s.substr(start);

        return s.substr(start, end - start);
    }

    NumericType getBase(const std::string& input, long& result)
    {
        bool success = true;
        std::string token = input;
        strToUpper(token);
        
        // Hex
        if(token.size() >= 2  &&  token.c_str()[0] == '$')
        {
            for(int i=1; i<int(token.size()); i++) success &= _hexaDecimalChars[uint8_t(token.c_str()[i])];
            if(success)
            {
                result = strtol(&token.c_str()[1], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Hex
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  token.c_str()[1] == 'X')  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'H')))
        {
            for(int i=2; i<int(token.size()); i++) success &= _hexaDecimalChars[uint8_t(token.c_str()[i])];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Octal
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'O' || token.c_str()[1] == 'Q'))  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'O')))
        {
            for(int i=2; i<int(token.size()); i++) success &= _octalChars[uint8_t(token.c_str()[i])];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 8);
                return Octal; 
            }
        }
        // Binary
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  token.c_str()[1] == 'B')  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'B')))
        {
            for(int i=2; i<int(token.size()); i++) success &= _binaryChars[uint8_t(token.c_str()[i])];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 2);
                return Binary; 
            }
        }
        // Decimal
        else
        {
            for(int i=0; i<int(token.size()); i++) success &= _decimalChars[uint8_t(token.c_str()[i])];
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

    void stringToDouble(const std::string& token, double& result)
    {
        result = std::stod(token);
    }


    // ****************************************************************************************************************
    // Tokenising
    // ****************************************************************************************************************
    void tokeniseHelper(std::vector<std::string>& result, const std::string& text, size_t offset, size_t size, bool toUpper)
    {
        std::string s = text.substr(offset, size);
        if(s.size() == 0) return;
        if(toUpper) strToUpper(s);
        result.push_back(s);
    }

    // Tokenise using a list of chars as delimiters, returns all tokens, (i.e. start, end and whole if no delimiters)
    std::vector<std::string> tokenise(const std::string& text, const std::string& delimiters, bool toUpper)
    {
        size_t offset0 = 0;
        size_t offset1 = SIZE_MAX;
        bool firstToken = true;
        std::vector<std::string> result;

        for(;;)
        {
            offset0 = text.find_first_of(delimiters, offset1 + 1);
            if(offset0 == std::string::npos)
            {
                // Entire text is a token
                if(firstToken)
                {
                    result.push_back(text);
                    return result;
                }

                // Last token
                tokeniseHelper(result, text, offset1 + 1, text.size() - offset0 + 1, toUpper);

                return result;
            }

            // First token
            if(firstToken)
            {
                firstToken = false;
                tokeniseHelper(result, text, 0, offset0, toUpper);
            }
            else
            {
                tokeniseHelper(result, text, offset1 + 1, offset0 - (offset1 + 1), toUpper);
            }

            offset1 = text.find_first_of(delimiters, offset0 + 1);
            if(offset1 == std::string::npos)
            {
                // Last token
                tokeniseHelper(result, text, offset0 + 1, text.size() - offset0 + 1, toUpper);

                return result;
            }
            
            tokeniseHelper(result, text, offset0 + 1, offset1 - (offset0 + 1), toUpper);
        }
    }

    // Tokenise using any char as a delimiter, returns tokens, preserve strings
    std::vector<std::string> tokenise(const std::string& text, char c, bool skipSpaces, bool toUpper)
    {
        std::vector<std::string> result;
        const char* str = text.c_str();

        do
        {
            const char *begin = str;

            int numQuotes = 0;
            while((*str  &&  *str != c)  ||  (numQuotes & 1))
            {
                if(*str == '"') numQuotes++;
                str++;
            }

            std::string s = std::string(begin, str);
            if(str > begin  &&  !(skipSpaces  &&  std::all_of(s.begin(), s.end(), isSpace)))
            {
                if(toUpper) strToUpper(s);
                result.push_back(s);
            }
        }
        while (*str++ != 0);

        return result;
    }

    // Tokenise using any char as a delimiter, returns tokens and their offsets in original text, preserve strings
    std::vector<std::string> tokenise(const std::string& text, char c, std::vector<size_t>& offsets, bool skipSpaces, bool toUpper)
    {
        std::vector<std::string> result;
        const char* str = text.c_str();

        do
        {
            const char *begin = str;

            int numQuotes = 0;
            while((*str  &&  *str != c)  ||  (numQuotes & 1))
            {
                if(*str == '"') numQuotes++;
                str++;
            }

            std::string s = std::string(begin, str);
            if(str > begin  &&  !(skipSpaces  &&  std::all_of(s.begin(), s.end(), isSpace)))
            {
                if(toUpper) strToUpper(s);
                offsets.push_back(size_t(str - text.c_str()) + 1);
                result.push_back(s);
            }
        }
        while (*str++ != 0);

        return result;
    }

    // Tokenise using whitespace and single or double quotes, preserves strings
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiters)
    {
        std::string token = "";
        bool delimiterStart = true;
        bool stringStart = false;
        enum DelimiterState {WhiteSpace, Quotes};
        DelimiterState delimiterState = WhiteSpace;
        std::vector<std::string> tokens;

        for(int i=0; i<=int(line.size()); i++)
        {
            // End of line is a delimiter for white space
            if(i == int(line.size()))
            {
                if(delimiterState != Quotes)
                {
                    delimiterState = WhiteSpace;
                    delimiterStart = false;
                }
                else
                {
                    break;
                }
            }
            else
            {
                // White space delimiters
                if(strchr(delimiters.c_str(), line[i]))
                {
                    if(delimiterState != Quotes)
                    {
                        delimiterState = WhiteSpace;
                        delimiterStart = false;
                    }
                }
                // String delimiters
                else if(strchr("\'\"", line[i]))
                {
                    delimiterState = Quotes;
                    stringStart = !stringStart;
                }
            }

            // Build token
            switch(delimiterState)
            {
                case WhiteSpace:
                {
                    // Don't save delimiters
                    if(delimiterStart)
                    {
                        if(!strchr(delimiters.c_str(), line[i])) token += line[i];
                    }
                    else
                    {
                        if(token.size()) tokens.push_back(token);
                        delimiterStart = true;
                        token = "";
                    }
                }
                break;

                case Quotes:
                {
                    // Save delimiters as well as chars
                    if(stringStart)
                    {
                        token += line[i];
                    }
                    else
                    {
                        token += line[i];
                        tokens.push_back(token);
                        delimiterState = WhiteSpace;
                        stringStart = false;
                        token = "";
                    }
                }
                break;

                default: break;
            }
        }

        return tokens;
    }

    // Tokenise using whitespace and double quotes, preserves strings, outputs offsets to tokens
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiters, std::vector<size_t>& offsets)
    {
        std::string token = "";
        bool delimiterStart = true;
        bool stringStart = false;
        enum DelimiterState {WhiteSpace, Quotes};
        DelimiterState delimiterState = WhiteSpace;
        std::vector<std::string> tokens;

        for(int i=0; i<=int(line.size()); i++)
        {
            // End of line is a delimiter for white space
            if(i == int(line.size()))
            {
                if(delimiterState != Quotes)
                {
                    delimiterState = WhiteSpace;
                    delimiterStart = false;
                }
                else
                {
                    break;
                }
            }
            else
            {
                // White space delimiters
                if(strchr(delimiters.c_str(), line[i]))
                {
                    if(delimiterState != Quotes)
                    {
                        delimiterState = WhiteSpace;
                        delimiterStart = false;
                    }
                }
                // String delimiters
                else if(strchr("\"", line[i]))
                {
                    delimiterState = Quotes;
                    stringStart = !stringStart;
                }
            }

            // Build token
            switch(delimiterState)
            {
                case WhiteSpace:
                {
                    // Don't save delimiters
                    if(delimiterStart)
                    {
                        if(!strchr(delimiters.c_str(), line[i])) token += line[i];
                    }
                    else
                    {
                        if(token.size())
                        {
                            tokens.push_back(token);
                            offsets.push_back(i - token.size());
                        }
                        delimiterStart = true;
                        token = "";
                    }
                }
                break;

                case Quotes:
                {
                    // Save delimiters as well as chars
                    if(stringStart)
                    {
                        token += line[i];
                    }
                    else
                    {
                        token += line[i];
                        tokens.push_back(token);
                        offsets.push_back(i - token.size());
                        delimiterState = WhiteSpace;
                        stringStart = false;
                        token = "";
                    }
                }
                break;

                default: break;
            }
        }

        return tokens;
    }

    void replaceText(std::string& input, const std::string& text, const std::string& replace)
    {
        for(size_t foundPos=0; ; foundPos+=replace.size())
        {
            foundPos = input.find(text, foundPos);
            if(foundPos == std::string::npos) break;

            input.erase(foundPos, text.size());
            input.insert(foundPos, replace);
        }
    }


    // ****************************************************************************************************************
    // Recursive decent parser
    // ****************************************************************************************************************
    char* getExpression(void) {return _expression;}
    const char* getExpressionToParse(void) {return _expressionToParse.c_str();}
    std::string& getExpressionToParseString(void) {return _expressionToParse;}
    int getLineNumber(void) {return _lineNumber;}

    void setExpression(const std::string& expression)
    {
        _advanceError = false;
        _expressionToParse = expression;
        _expression = (char*)_expressionToParse.c_str();
    }

    char peek(void)
    {
        if(_advanceError) return 0;

        return *_expression;
    }

    char get(void)
    {
        if(_advanceError) return 0;

        char chr = *_expression;
        //fprintf(stderr, "%s : %s : %c\n", _expressionToParse.c_str(), _expression, chr);
        advance(1);
        return chr;
    }

    void save(void)
    {
        _advancePtr = uintptr_t(_expression);
    }

    void restore(void)
    {
        _expression = (char *)_advancePtr;
    }

    bool advance(uintptr_t n)
    {
        if(size_t(_expression + n - _expressionToParse.c_str()) >= _expressionToParse.size())
        {
            _expression = (char*)_expressionToParse.c_str() + _expressionToParse.size() - 1;
            _advanceError = true;
            return false;
        }

        //std::string text = std::string(_expression, n);
        //fprintf(stderr, "%s : %s : %d : %d\n", _expressionToParse.c_str(), text.c_str(), int(n), int(_expression + n - _expressionToParse.c_str()));

        _advanceError = false;
        _expression += n;

        return true;
    }

    bool find(const std::string& text)
    {
        size_t pos = size_t(_expression - _expressionToParse.c_str());
        std::string expr = _expressionToParse.substr(pos, text.size());
        if(strToUpper(expr) == text)
        {
            advance(text.size());
            return true;
        }

        return false;
    }

    bool number(int16_t& value)
    {
        char uchr;

        std::string valueStr;
        uchr = char(toupper((unsigned char)peek()));
        valueStr.push_back(uchr); get();
        uchr = char(toupper((unsigned char)peek()));
        if((uchr >= '0'  &&  uchr <= '9')  ||  uchr == 'X'  ||  uchr == 'H'  ||  uchr == 'B'  ||  uchr == 'O'  ||  uchr == 'Q')
        {
            valueStr.push_back(uchr); get();
            uchr = char(toupper((unsigned char)peek()));
            while((uchr >= '0'  &&  uchr <= '9')  ||  (uchr >= 'A'  &&  uchr <= 'F'))
            {
                valueStr.push_back(get());
                uchr = char(toupper((unsigned char)peek()));
            }
        }

        return stringToI16(valueStr, value);
    }

    Numeric factor(int16_t defaultValue)
    {
        int16_t value = 0;
        Numeric numeric;

        if(peek() == '(')
        {
            get();
            numeric = expression();

            if(peek() != ')')
            {
                fprintf(stderr, "Expression::factor() : Missing ')' in '%s' on line %d\n", _expressionToParse.c_str(), _lineNumber + 1);
                numeric = Numeric();
            }
            get();
        }
        else if((peek() >= '0'  &&  peek() <= '9')  ||  peek() == '$'  ||  peek() == '&')
        {
            if(!number(value))
            {
                fprintf(stderr, "Expression::factor() : Bad numeric data in '%s' on line %d\n", _expressionToParse.c_str(), _lineNumber + 1);
                numeric = Numeric();
            }
            else
            {
                numeric = Numeric(value, -1, true, Number, BooleanCC, Int16Both, std::string(""), std::string(""));
            }
        }
        // Functions
        else if(Expression::find("SIN"))
        {
            numeric = factor(0); numeric = operatorSIN(numeric);
        }
        else if(Expression::find("COS"))
        {
            numeric = factor(0); numeric = operatorCOS(numeric);
        }
        else if(Expression::find("TAN"))
        {
            numeric = factor(0); numeric = operatorTAN(numeric);
        }
        else if(Expression::find("ASIN"))
        {
            numeric = factor(0); numeric = operatorASIN(numeric);
        }
        else if(Expression::find("ACOS"))
        {
            numeric = factor(0); numeric = operatorACOS(numeric);
        }
        else if(Expression::find("ATAN"))
        {
            numeric = factor(0); numeric = operatorATAN(numeric);
        }
        else if(Expression::find("RAND"))
        {
            numeric = factor(0); numeric = operatorRAND(numeric);
        }
        else if(Expression::find("REV16"))
        {
            numeric = factor(0); numeric = operatorREV16(numeric);
        }
        else if(Expression::find("REV8"))
        {
            numeric = factor(0); numeric = operatorREV8(numeric);
        }
        else if(Expression::find("REV4"))
        {
            numeric = factor(0); numeric = operatorREV4(numeric);
        }
        else
        {
            // Unary operators
            switch(peek())
            {
                case '+': get(); numeric = factor(0);                         break;
                case '-': get(); numeric = factor(0); numeric = operatorNEG(numeric); break;
                case '~': get(); numeric = factor(0); numeric = operatorNOT(numeric); break;

                // Unknown
                default: numeric = Numeric(defaultValue, -1, true, Number, BooleanCC, Int16Both, std::string(_expression), std::string("")); break;
            }
        }

        return numeric;
    }

    Numeric term(void)
    {
        Numeric numeric, result = factor(0);

        for(;;)
        {
            if(find("**"))         {       numeric = factor(0); result = operatorPOW(result, numeric);}
            else if(peek() == '*') {get(); numeric = factor(0); result = operatorMUL(result, numeric);}
            else if(peek() == '/') {get(); numeric = factor(0); result = operatorDIV(result, numeric);}
            else if(peek() == '%') {get(); numeric = factor(0); result = operatorMOD(result, numeric);}
            else return result;
        }
    }

    Numeric expr(void)
    {
        Numeric numeric, result = term();
    
        for(;;)
        {
            if(peek() == '+')      {get(); numeric = term(); result = operatorADD(result, numeric);}
            else if(peek() == '-') {get(); numeric = term(); result = operatorSUB(result, numeric);}
            else return result;
        }
    }

    Numeric logical(void)
    {
        Numeric numeric, result = expr();
    
        for(;;)
        {
            if(peek() == '&')      {get(); numeric = expr(); result = operatorAND(result, numeric);}
            else if(peek() == '^') {get(); numeric = expr(); result = operatorXOR(result, numeric);}
            else if(peek() == '|') {get(); numeric = expr(); result = operatorOR(result,  numeric);}
            else if(find("<<"))    {       numeric = expr(); result = operatorLSL(result, numeric);}
            else if(find(">>"))    {       numeric = expr(); result = operatorLSR(result, numeric);}
            else return result;
        }
    }

    Numeric expression(void)
    {
        Numeric numeric, result = logical();
            
        for(;;)
        {
            if(find("=="))         {       numeric = logical(); result = operatorEQ(result, numeric);}
            else if(find("!="))    {       numeric = logical(); result = operatorNE(result, numeric);}
            else if(find("<="))    {       numeric = logical(); result = operatorLE(result, numeric);}
            else if(find(">="))    {       numeric = logical(); result = operatorGE(result, numeric);}
            else if(peek() == '<') {get(); numeric = logical(); result = operatorLT(result, numeric);}
            else if(peek() == '>') {get(); numeric = logical(); result = operatorGT(result, numeric);}
            else return result;
        }
    }

    bool parse(const std::string& expression, int lineNumber, Numeric& numeric)
    {
        _output = numeric;

        _advanceError = false;
        _expressionToParse = expression;
        _lineNumber = lineNumber;

        _expression = (char*)_expressionToParse.c_str();

        numeric = _exprFunc();
        return numeric._isValid;
    }
}