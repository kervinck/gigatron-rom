#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    bool _enablePrint = false;
    bool _advanceError = false;
    bool _containsQuotes = false;

    bool _binaryChars[256]      = {false};
    bool _octalChars[256]       = {false};
    bool _decimalChars[256]     = {false};
    bool _hexaDecimalChars[256] = {false};

    uintptr_t _advancePtr;
    exprFuncPtr _exprFunc;

    Numeric _output;


    // Forward declarations
    Numeric expression(void);


    // Default operators
    Numeric neg(Numeric& numeric)
    {
        numeric._value = -numeric._value;
        return numeric;
    }
    Numeric not(Numeric& numeric)
    {
        numeric._value = ~numeric._value;
        return numeric;
    }
    Numeric and(Numeric& left, Numeric& right)
    {
        left._value &= right._value;
        return left;
    }
    Numeric or(Numeric& left, Numeric& right)
    {
        left._value |= right._value;
        return left;
    }
    Numeric xor(Numeric& left, Numeric& right)
    {
        left._value ^= right._value;
        return left;
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
    Numeric pow(Numeric& left, Numeric& right)
    {
        left._value = int16_t(std::pow(double(left._value), double(right._value)));
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
    Numeric mod(Numeric& left, Numeric& right)
    {
        left._value = (right._value == 0) ? 0 : left._value % right._value;
        return left;
    }
    Numeric lsl(Numeric& left, Numeric& right)
    {
        left._value = left._value << right._value;
        return left;
    }
    Numeric lsr(Numeric& left, Numeric& right)
    {
        left._value = left._value >> right._value;
        return left;
    }
    Numeric lt(Numeric& left, Numeric& right)
    {
        left._value = left._value < right._value;
        return left;
    }
    Numeric gt(Numeric& left, Numeric& right)
    {
        left._value = left._value > right._value;
        return left;
    }
    Numeric eq(Numeric& left, Numeric& right)
    {
        left._value = left._value == right._value;
        return left;
    }
    Numeric ne(Numeric& left, Numeric& right)
    {
        left._value = left._value != right._value;
        return left;
    }
    Numeric le(Numeric& left, Numeric& right)
    {
        left._value = left._value <= right._value;
        return left;
    }
    Numeric ge(Numeric& left, Numeric& right)
    {
        left._value = left._value >= right._value;
        return left;
    }

    bool getEnablePrint(void) {return _enablePrint;}
    Numeric& getOutputNumeric(void) {return _output;}

    void setExprFunc(exprFuncPtr exprFunc) {_exprFunc = exprFunc;}
    void setEnablePrint(bool enablePrint) {_enablePrint = enablePrint;}


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
    ExpressionType isExpression(const std::string& input)
    {
        if(input.find_first_of("[]") != std::string::npos) return Invalid;
        if(input.find("++") != std::string::npos) return Invalid;
        if(input.find("--") != std::string::npos) return Invalid;
        if(input.find_first_of("-+/%*()&|^") != std::string::npos) return HasOperators;
        return HasNumbers;
    }

    bool isVarNameValid(const std::string& varName)
    {
        if(varName.size() == 0)  return false;
        if(!isalpha(varName[0])) return false;

        for(int i=1; i<varName.size()-1; i++)
        {
            if(!isalnum(varName[i])) return false;
        }

        if(!isalnum(varName[varName.size() - 1])  &&  varName[varName.size() - 1] != '$') return false;

        return true;
    }

    bool isValidString(const std::string& input)
    {
        std::string str = input;
        stripNonStringWhitespace(str);
        if(str.size() < 2) return false;
        if(str[0] == '"'  &&  str.back() == '"')
        {
            for(int i=1; i<str.size()-1; i++)
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
        if(!isspace(chr) || _containsQuotes) return false;
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
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end());
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

        std::unique_copy(input.begin(), input.end(), std::back_insert_iterator<std::string>(output), [](char a, char b) {return isspace(a) && isspace(b);});

        return output;
    }

    std::string collapseWhitespaceNotStrings(const std::string& input)
    {
        std::string output;
        int spaceCount = 0;
        bool inString = false;

        for(int i=0; i<input.size(); i++)
        {
            if(input[i] == '\"') inString = !inString;

            if(isspace(input[i]))
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

        for(int i=0; i<input.size(); i++)
        {
            // Check for string
            if(!inComment  &&  input[i] == '\"') inString = !inString;

            // Check for comment, ' and REM
            if(!inString)
            {
                if((input[i] == '\'')  ||  (i <= input.size()-3  &&  toupper(input[i]) == 'R'  &&  toupper(input[i+1]) == 'E'  &&  toupper(input[i+2]) == 'M'))
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
        if(num > input.size()) input.insert(0, num - input.size(), pad);
    }

    void addString(std::string &input, int num, char add)
    {
        if(num > 0) input.append(num, add);
    }

    int tabbedStringLength(const std::string& input, int tabSize)
    {
        int length = 0, newLine = 0;
        for(int i=0; i<input.size(); i++)
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
        lbra = -1;
        rbra = -1;

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

        lbra = -1;
        rbra = -1;

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
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return tolower(c);} );
        return s;
    }

    std::string& strToUpper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return toupper(c);} );
        return s;
    }

    bool subAlphaHelper(int i)
    {
        // Valid chars are alpha and 'address of'
        return (isalpha(i) || (i=='@'));
    }
    std::string getSubAlpha(const std::string& s)
    {
        if(s.size() > 1)
        {
            char uchr = toupper(s[1]);
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
            for(int i=1; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[1], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Hex
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  token.c_str()[1] == 'X')  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'H')))
        {
            for(int i=2; i<token.size(); i++) success &= _hexaDecimalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 16);
                return HexaDecimal; 
            }
        }
        // Octal
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  (token.c_str()[1] == 'O' || token.c_str()[1] == 'Q'))  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'O')))
        {
            for(int i=2; i<token.size(); i++) success &= _octalChars[token.c_str()[i]];
            if(success)
            {
                result = strtol(&token.c_str()[2], NULL, 8);
                return Octal; 
            }
        }
        // Binary
        else if(token.size() >= 3  &&  ((token.c_str()[0] == '0'  &&  token.c_str()[1] == 'B')  ||  (token.c_str()[0] == '&'  &&  token.c_str()[1] == 'B')))
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
        size_t offset1 = -1;
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
            if(str > begin  &&  !(skipSpaces  &&  std::all_of(s.begin(), s.end(), isspace)))
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
            if(str > begin  &&  !(skipSpaces  &&  std::all_of(s.begin(), s.end(), isspace)))
            {
                if(toUpper) strToUpper(s);
                offsets.push_back(size_t(str - text.c_str()) + 1);
                result.push_back(s);
            }
        }
        while (*str++ != 0);

        return result;
    }

    // Tokenise using whitespace and quotes, preserves strings
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiters)
    {
        std::string token = "";
        bool delimiterStart = true;
        bool stringStart = false;
        enum DelimiterState {WhiteSpace, Quotes};
        DelimiterState delimiterState = WhiteSpace;
        std::vector<std::string> tokens;

        for(int i=0; i<=line.size(); i++)
        {
            // End of line is a delimiter for white space
            if(i == line.size())
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
            }
        }

        return tokens;
    }

    // Tokenise using whitespace and quotes, preserves strings
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiters, std::vector<size_t>& offsets)
    {
        std::string token = "";
        bool delimiterStart = true;
        bool stringStart = false;
        enum DelimiterState {WhiteSpace, Quotes};
        DelimiterState delimiterState = WhiteSpace;
        std::vector<std::string> tokens;

        for(int i=0; i<=line.size(); i++)
        {
            // End of line is a delimiter for white space
            if(i == line.size())
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
        if(strToUpper(_expressionToParse.substr(pos, text.size())) == text)
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
        uchr = toupper(peek());
        valueStr.push_back(uchr); get();
        uchr = toupper(peek());
        if((uchr >= '0'  &&  uchr <= '9')  ||  uchr == 'X'  ||  uchr == 'H'  ||  uchr == 'B'  ||  uchr == 'O'  ||  uchr == 'Q')
        {
            valueStr.push_back(uchr); get();
            uchr = toupper(peek());
            while((uchr >= '0'  &&  uchr <= '9')  ||  (uchr >= 'A'  &&  uchr <= 'F'))
            {
                valueStr.push_back(get());
                uchr = toupper(peek());
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
        else
        {
            // Functions

            // Unary operators
            switch(peek())
            {
                case '+': get(); numeric = factor(0);                         break;
                case '-': get(); numeric = factor(0); numeric = neg(numeric); break;
                case '~': get(); numeric = factor(0); numeric = not(numeric); break;

                // Unknown
                default: numeric = Numeric(defaultValue, -1, true, Number, BooleanCC, Int16Both, std::string(_expression), std::string("")); break;
            }
        }

        return numeric;
    }

    Numeric term(void)
    {
        Numeric result = factor(0);

        for(;;)
        {
            if(find("**"))         {       result = pow(result, factor(0));}
            else if(peek() == '*') {get(); result = mul(result, factor(0));}
            else if(peek() == '/') {get(); result = div(result, factor(0));}
            else if(peek() == '%') {get(); result = mod(result, factor(0));}
            else return result;
        }
    }

    Numeric expr(void)
    {
        Numeric result = term();
    
        for(;;)
        {
            if(peek() == '+')      {get(); result = add(result, term());}
            else if(peek() == '-') {get(); result = sub(result, term());}
            else return result;
        }
    }

    Numeric logical(void)
    {
        Numeric result = expr();
    
        for(;;)
        {
            if(peek() == '&')      {get(); result = and(result, expr());}
            else if(peek() == '^') {get(); result = xor(result, expr());}
            else if(peek() == '|') {get(); result = or(result,  expr());}
            else if(find("<<"))    {       result = lsl(result, expr());}
            else if(find(">>"))    {       result = lsr(result, expr());}
            else return result;
        }
    }

    Numeric expression(void)
    {
        Numeric result = logical();
    
        for(;;)
        {
            if(find("=="))         {       result = eq(result, logical());}
            else if(find("!="))    {       result = ne(result, logical());}
            else if(find("<="))    {       result = le(result, logical());}
            else if(find(">="))    {       result = ge(result, logical());}
            else if(peek() == '<') {get(); result = lt(result, logical());}
            else if(peek() == '>') {get(); result = gt(result, logical());}
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
        return _exprFunc()._isValid;
    }
}