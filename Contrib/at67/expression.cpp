#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "expression.h"

#define UNREFERENCED_PARAM(P) ((void)P)


namespace Expression
{
    std::string _expressionToParse;
    char* _expression;

    int _lineNumber = 0;

    bool _advanceError = false;
    bool _enableOptimisedPrint = false;

    bool _binaryChars[256]      = {false};
    bool _octalChars[256]       = {false};
    bool _decimalChars[256]     = {false};
    bool _hexaDecimalChars[256] = {false};

    uintptr_t _advancePtr;
    exprFuncPtr _exprFunc;

    Numeric _output;


    // Forward declarations
    Numeric expression(bool);

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
    Numeric& operatorCEIL(Numeric& numeric)
    {
        numeric._value = ceil(numeric._value);
        return numeric;
    }
    Numeric& operatorFLOOR(Numeric& numeric)
    {
        numeric._value = floor(numeric._value);
        return numeric;
    }
    Numeric& operatorPOWF(Numeric& numeric)
    {
        if(numeric._params.size() > 0) numeric._value = pow(numeric._value, numeric._params[0]._value);
        numeric._params.clear();
        return numeric;
    }
    Numeric& operatorSQRT(Numeric& numeric)
    {
        if(numeric._value > 0.0) numeric._value = sqrt(numeric._value);
        return numeric;
    }
    Numeric& operatorEXP(Numeric& numeric)
    {
        numeric._value = exp(numeric._value);
        return numeric;
    }
    Numeric& operatorEXP2(Numeric& numeric)
    {
        numeric._value = exp2(numeric._value);
        return numeric;
    }
    Numeric& operatorLOG(Numeric& numeric)
    {
        if(numeric._value > 0.0) numeric._value = log(numeric._value);
        return numeric;
    }
    Numeric& operatorLOG2(Numeric& numeric)
    {
        if(numeric._value > 0.0) numeric._value = log2(numeric._value);
        return numeric;
    }
    Numeric& operatorLOG10(Numeric& numeric)
    {
        if(numeric._value > 0.0) numeric._value = log10(numeric._value);
        return numeric;
    }
    Numeric& operatorSIN(Numeric& numeric)
    {
        numeric._value = sin(numeric._value*MATH_PI/180.0);
        return numeric;
    }
    Numeric& operatorCOS(Numeric& numeric)
    {
        numeric._value = cos(numeric._value*MATH_PI/180.0);
        return numeric;
    }
    Numeric& operatorTAN(Numeric& numeric)
    {
        numeric._value = tan(numeric._value*MATH_PI/180.0);
        return numeric;
    }
    Numeric& operatorASIN(Numeric& numeric)
    {
        numeric._value = asin(numeric._value)/MATH_PI*180.0;
        return numeric;
    }
    Numeric& operatorACOS(Numeric& numeric)
    {
        numeric._value = acos(numeric._value)/MATH_PI*180.0;
        return numeric;
    }
    Numeric& operatorATAN(Numeric& numeric)
    {
        numeric._value = atan(numeric._value)/MATH_PI*180.0;
        return numeric;
    }
    Numeric& operatorATAN2(Numeric& numeric)
    {
        if(numeric._params.size() > 0  &&  (numeric._value != 0.0  ||  numeric._params[0]._value != 0.0)) numeric._value = atan2(numeric._value, numeric._params[0]._value)/MATH_PI*180.0;
        numeric._params.clear();
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

    bool isNumber(const std::string& input)
    {
        return !input.empty()  &&  std::find_if(input.begin(), input.end(), [](unsigned char c) { return !isdigit(c); }) == input.end();
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

    // First char must be alpha, all chars except first can be numerics, all chars except first and last can be underscores, can contain valid brackets as an array reference
    TokType isVarNameValid(const std::string& varName)
    {
        TokType tokType = Invalid;

        if(varName.size() == 0) return Invalid;
        if(!isalpha((unsigned char)varName[0])) return Invalid;

        size_t lbra, rbra;
        if(!findMatchingBrackets(varName, 0, lbra, rbra))
        {
            tokType = Variable;
            lbra = varName.size();
        }
        else
        {
            tokType = Array;
        }

        if(!isalnum((unsigned char)varName[lbra - 1])) return Invalid;

        for(int i=1; i<int(lbra-1); i++)
        {
            if(!isalnum((unsigned char)varName[i])  &&  varName[i] != '_') return Invalid;
        }

        return tokType;
    }

    // First char must be alpha, all chars except first can be numerics, all chars except first and last can be underscores, last char can be a dollar sign, can contain valid brackets as an array reference
    TokType isStrNameValid(const std::string& strName)
    {
        TokType tokType = Invalid;

        if(strName.size() == 0) return Invalid;
        if(!isalpha((unsigned char)strName[0])) return Invalid;

        size_t lbra, rbra;
        if(!findMatchingBrackets(strName, 0, lbra, rbra))
        {
            tokType = Variable;
            lbra = strName.size();
        }
        else
        {
            tokType = Array;
        }

        if(strName[lbra - 1] != '$') return Invalid;

        for(int i=1; i<int(lbra-1); i++)
        {
            if(!isalnum((unsigned char)strName[i])  &&  strName[i] != '_') return Invalid;
        }

        return tokType;
    }

    // First char must be alpha, all chars except first can be numerics, all chars except first and last can be underscores
    bool isLabNameValid(const std::string& labName)
    {
        if(labName.size() == 0) return false;
        if(!isalpha((unsigned char)labName[0])) return false;
        if(!isalnum((unsigned char)labName[labName.size() - 1])) return false;

        for(int i=1; i<int(labName.size())-1; i++)
        {
            if(!isalnum((unsigned char)labName[i])  &&  labName[i] != '_') return false;
        }

        return true;
    }

    bool isStringValid(const std::string& input)
    {
        std::string str = input;
        stripNonStringWhitespace(str);
        if(str.size() < 2) return false;
        if(str[0] == '"'  &&  str.back() == '"')
        {
            for(int i=1; i<int(str.size())-1; i++)
            {
                if(str[i] == '"'  &&  str[i-1] != '\\') return false;
            }

            return true;
        }

        return false;
    }

    int _chrBra = -1;
    int _chrPrev = -1;
    bool _containsQuotes = false;
    void initHasHelpers(int bra=-1)
    {
        _chrBra = bra;
        _chrPrev = -1;
        _containsQuotes = false;
    }
    bool hasNonStringWhiteSpace(int chr)
    {
        bool result = true;

        if(chr == '"'  &&  _chrPrev != '\\') _containsQuotes = !_containsQuotes;
        if(!isspace((unsigned char)chr) || _containsQuotes) result = false;

        _chrPrev = chr;
        return result;
    }
    bool hasNonStringEquals(int chr)
    {
        bool result = true;

        if(chr == '"'  &&  _chrPrev != '\\') _containsQuotes = !_containsQuotes;
        if(chr != '='  ||  _containsQuotes) result = false;

        _chrPrev = chr;
        return result;
    }
    bool hasNonStringColon(int chr)
    {
        bool result = true;

        if(chr == '"'  &&  _chrPrev != '\\') _containsQuotes = !_containsQuotes;
        if(chr != ':'  ||  _containsQuotes) result = false;

        _chrPrev = chr;
        return result;
    }
    bool hasNonStringBracket(int chr)
    {
        bool result = true;

        if(chr == '"'  &&  _chrPrev != '\\') _containsQuotes = !_containsQuotes;
        if(chr != _chrBra  ||  _containsQuotes) result = false;

        _chrPrev = chr;
        return result;
    }

    bool hasOnlyWhiteSpace(const std::string& input)
    {
        if(std::find_if_not(input.begin(), input.end(), isspace) != input.end()) return false;

        return true;
    }

    void isInComment(const std::string& input, int index, bool& output)
    {
        // Once a comment is found this function must instantly return when called again, otherwise the logic breaks
        if(output) return;

        if(index >= int(input.size())) return;
    
        // In comment if trailing a single quote or if NOT a forward char literal or if trailing a 'REM' sequence
        if((input[index] == '\''  &&  index > int(input.size())-3)  ||  (index <= int(input.size())-3  &&  input[index] == '\''  &&  input[index+2] != '\'')  ||
           (index <= int(input.size()) - 3  &&  toupper((unsigned char)input[index]) == 'R'  &&  toupper((unsigned char)input[index+1]) == 'E'  &&  toupper((unsigned char)input[index+2]) == 'M'))
        {
            output = true;

            // Not in comment if a reverse char literal
            if(index >= 2  &&  input[index] == '\''  &&  input[index-2] == '\'')
            {
                output = false;
            }
        }
    }

    std::string::const_iterator findNonStringEquals(const std::string& input)
    {
        initHasHelpers();
        return std::find_if(input.begin(), input.end(), hasNonStringEquals);
    }

    std::string::const_iterator findNonStringColon(const std::string& input)
    {
        initHasHelpers();
        return std::find_if(input.begin(), input.end(), hasNonStringColon);
    }

    std::string::const_iterator findNonStringBracket(const std::string& input, int bra)
    {
        initHasHelpers(bra);
        return std::find_if(input.begin(), input.end(), hasNonStringBracket);
    }

    void stripNonStringWhitespace(std::string& input)
    {
        initHasHelpers();
        input.erase(remove_if(input.begin(), input.end(), hasNonStringWhiteSpace), input.end());
    }

    void stripWhitespace(std::string& input)
    {
        input.erase(remove_if(input.begin(), input.end(), isSpace), input.end());
    }

    bool stripChars(std::string& input, const std::string& chars)
    {
        std::string output = input;
        for(int i=0; i<int(chars.size()); i++)
        {
            output.erase(std::remove(output.begin(), output.end(), chars[i]), output.end()) == output.end();
        }

        input = output;

        return true;
    }

    std::string stripStrings(const std::string& input)
    {
        size_t start = 0, end = std::string::npos;
        std::string output = input;

        for(;;)
        {
            start = output.find_first_of('"', end + 1);
            end = output.find_first_of('"', start + 1);
            if(start == std::string::npos  ||  end == std::string::npos) break;

            output.erase(start, end - start + 1);
            start = 0;
            end = std::string::npos;
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
        if(start == std::string::npos)
        {
            stripWhitespace(input);
            return;
        }

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
            if((i==0  &&  input[i] == '\"')  ||  (i > 0  &&  input[i] == '\"'  &&  input[i-1] != '\\')) inString = !inString;

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
            if(!inComment)
            {
                if((i==0  &&  input[i] == '"')  ||  (i > 0  &&  input[i] == '"'  &&  input[i-1] != '\\')) inString = !inString;
            }

            if(!inString)
            {
                // Check for comment, ' and REM
                isInComment(input, i, inComment);

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

    bool findMatchingBrackets(const std::string& input, size_t start, size_t& lbra, size_t& rbra, char obra)
    {
        char cbra = 0;
        switch(obra)
        {
            case '(': cbra = ')'; break;
            case '{': cbra = '}'; break;
            case '[': cbra = ']'; break;

            default: return false;
        }

        lbra = std::string::npos;
        rbra = std::string::npos;

        int matched = 0;
        bool startMatching = false;

        for(size_t i=start; i<input.size(); i++)
        {
            if(input[i] == obra)
            {
                if(!startMatching)
                {
                    lbra = i;
                    startMatching = true;
                }
                matched++;
            }

            if(input[i] == cbra)
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

    bool findMatchingBrackets(const std::string& input, size_t start, size_t& lbra, size_t& rbra, char obra, std::string& name, int& paramNum)
    {
        bool result = findMatchingBrackets(input, start, lbra, rbra, obra);
        std::vector<std::string> params = Expression::tokenise(input.substr(lbra + 1, rbra - (lbra + 1)), ',', true);
        name = (lbra) ? input.substr(0, lbra): input;
        paramNum = (result) ? int(params.size()) : 0;
        return result;
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

    std::string strLower(const std::string& s)
    {
        std::string str = s;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return char(tolower(c));} );
        return str;
    }

    std::string strUpper(const std::string& s)
    {
        std::string str = s;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return char(toupper(c));} );
        return str;
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
        // Valid chars are alpha, 'address of' and 'size of'
        return (isalpha((unsigned char)i) || (i=='@') || (i=='#'));
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
    std::vector<std::string> tokeniseMulti(const std::string& text, const std::string& delimiterStr, bool toUpper)
    {
        size_t offset0 = 0;
        size_t offset1 = SIZE_MAX;
        bool firstToken = true;
        std::vector<std::string> result;

        for(;;)
        {
            offset0 = text.find_first_of(delimiterStr, offset1 + 1);
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

            offset1 = text.find_first_of(delimiterStr, offset0 + 1);
            if(offset1 == std::string::npos)
            {
                // Last token
                tokeniseHelper(result, text, offset0 + 1, text.size() - offset0 + 1, toUpper);

                return result;
            }
            
            tokeniseHelper(result, text, offset0 + 1, offset1 - (offset0 + 1), toUpper);
        }
    }

    // Tokenise using any char as a delimiter, returns tokens, preserve strings, preserves bracketed expressions
    std::vector<std::string> tokenise(const std::string& text, char c, bool skipSpaces, bool toUpper)
    {
        std::vector<std::string> result;
        const char* str = text.c_str();

        do
        {
            const char *begin = str;

            int numQuotes = 0;
            int numBrackets = 0;
            while(*str  &&  (*str != c  ||  (numQuotes & 1)  ||  numBrackets))
            {
                if(*str == '"'  &&  (str == begin  ||  (str > begin  &&  *(str-1) != '\\'))) numQuotes++;
                if(*str == '(') numBrackets++;
                if(*str == ')') numBrackets--;
                str++;
            }

            std::string s = std::string(begin, str);
            if(str > begin  &&  !(skipSpaces  &&  std::all_of(s.begin(), s.end(), isSpace)))
            {
                if(toUpper) strToUpper(s);
                result.push_back(s);
            }
        }
        while(*str++ != 0);

        return result;
    }

    // Tokenise using any char as a delimiter, returns tokens and their offsets in original text, preserve strings
    std::vector<std::string> tokeniseOffsets(const std::string& text, char c, std::vector<size_t>& offsets, bool skipSpaces, bool toUpper)
    {
        std::vector<std::string> result;
        const char* str = text.c_str();

        do
        {
            const char *begin = str;

            int numQuotes = 0;
            while(*str  &&  (*str != c  ||  (numQuotes & 1)))
            {
                if(*str == '"'  &&  (str == begin  ||  (str > begin  &&  *(str-1) != '\\'))) numQuotes++;
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

    // Tokenise using delimiters and single quotes, preserves strings
    std::vector<std::string> tokeniseLine(const std::string& line, const std::string& delimiterStr)
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
                if(strchr(delimiterStr.c_str(), line[i]))
                {
                    if(delimiterState != Quotes)
                    {
                        delimiterState = WhiteSpace;
                        delimiterStart = false;
                    }
                }
                // Single quote string delimiters, skip escaped quotes
                else if((i == 0  ||  (i > 0  &&  line[i-1] != '\\'))  &&  line[i] == '\'')
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
                        if(!strchr(delimiterStr.c_str(), line[i])) token += line[i];
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

    // Tokenise using delimiters and double quotes, preserves strings, outputs offsets to tokens
    std::vector<std::string> tokeniseLineOffsets(const std::string& line, const std::string& delimiterStr, std::vector<size_t>& offsets)
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
                if(strchr(delimiterStr.c_str(), line[i]))
                {
                    if(delimiterState != Quotes)
                    {
                        delimiterState = WhiteSpace;
                        delimiterStart = false;
                    }
                }
                // String delimiters, skip escaped quotes
                else if((i==0  &&  line[i] == '\"')  ||  (i > 0  &&  line[i] == '\"'  &&  line[i-1] != '\\'))
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
                        if(!strchr(delimiterStr.c_str(), line[i])) token += line[i];
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

    void replaceText(std::string& input, const std::string& text, const std::string& replace, size_t offset)
    {
        for(size_t foundPos=offset; ; foundPos+=replace.size())
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

    void setExpression(const std::string& expression, intptr_t n)
    {
        _advanceError = false;
        _expressionToParse = expression;
        _expression = (char*)_expressionToParse.c_str() + n;
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

    bool advance(intptr_t n)
    {
        if(size_t(_expression + n - _expressionToParse.c_str()) >= _expressionToParse.size())
        {
            _expression = (char*)_expressionToParse.c_str() + _expressionToParse.size();
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

    // Searches for a function name in expression and then expects a left bracket before any other non whitespace char
    bool findFunc(const std::string& text)
    {
        size_t pos = size_t(_expression - _expressionToParse.c_str());
        std::string expr = _expressionToParse.substr(pos, text.size());
        std::string expression = _expressionToParse.substr(pos);
        stripNonStringWhitespace(expression);
        if(strToUpper(expr) == text  &&  expression.size() > text.size())
        {
            char lbra = expression[text.size()];
            if(lbra == '(')
            {
                advance(text.size());
                return true;
            }
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
                fprintf(stderr, "Expression::factor() : '%s:%d' : missing ')'\n", _expressionToParse.c_str(), _lineNumber + 1);
                numeric = Numeric();
            }
            get();
        }
        else if((peek() >= '0'  &&  peek() <= '9')  ||  peek() == '$'  ||  peek() == '&')
        {
            if(!number(value))
            {
                fprintf(stderr, "Expression::factor() : '%s:%d' : bad numeric data\n", _expressionToParse.c_str(), _lineNumber + 1);
                numeric = Numeric();
            }
            else
            {
                numeric = Numeric(value, -1, true, false, false, Number, BooleanCC, Int16Both, std::string(""), std::string(""));
            }
        }
        // Functions
        else if(find("POW"))
        {
            numeric = factor(0); numeric = operatorPOWF(numeric);
        }
        else if(find("SQRT"))
        {
            numeric = factor(0); numeric = operatorSQRT(numeric);
        }
        else if(find("EXP2"))
        {
            numeric = factor(0); numeric = operatorEXP2(numeric);
        }
        else if(find("EXP"))
        {
            numeric = factor(0); numeric = operatorEXP(numeric);
        }
        else if(find("LOG10"))
        {
            numeric = factor(0); numeric = operatorLOG10(numeric);
        }
        else if(find("LOG2"))
        {
            numeric = factor(0); numeric = operatorLOG2(numeric);
        }
        else if(find("LOG"))
        {
            numeric = factor(0); numeric = operatorLOG(numeric);
        }
        else if(find("SIN"))
        {
            numeric = factor(0); numeric = operatorSIN(numeric);
        }
        else if(find("COS"))
        {
            numeric = factor(0); numeric = operatorCOS(numeric);
        }
        else if(find("TAN"))
        {
            numeric = factor(0); numeric = operatorTAN(numeric);
        }
        else if(find("ASIN"))
        {
            numeric = factor(0); numeric = operatorASIN(numeric);
        }
        else if(find("ACOS"))
        {
            numeric = factor(0); numeric = operatorACOS(numeric);
        }
        else if(find("ATAN2"))
        {
            numeric = factor(0); numeric = operatorATAN2(numeric);
        }
        else if(find("ATAN"))
        {
            numeric = factor(0); numeric = operatorATAN(numeric);
        }
        else if(find("RAND"))
        {
            numeric = factor(0); numeric = operatorRAND(numeric);
        }
        else if(find("REV16"))
        {
            numeric = factor(0); numeric = operatorREV16(numeric);
        }
        else if(find("REV8"))
        {
            numeric = factor(0); numeric = operatorREV8(numeric);
        }
        else if(find("REV4"))
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
                default: numeric = Numeric(defaultValue, -1, true, false, false, Number, BooleanCC, Int16Both, std::string(_expression), std::string("")); break;
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

    Numeric expression(bool returnAddress)
    {
        UNREFERENCED_PARAM(returnAddress);

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

        numeric = _exprFunc(numeric._returnAddress);

        // If there are trailing chars left over and they are not whitespace, then syntax error
        char* chr = _expression;
        while(*chr)
        {
            if(!isspace(*chr++))
            {
                numeric._isValid = false;
                break;
            }
        }

        return numeric._isValid;
    }
}