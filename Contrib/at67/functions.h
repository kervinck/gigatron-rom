#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <map>

#include "expression.h"


namespace Functions
{
    std::map<std::string, std::string>& getFunctions(void);
    std::map<std::string, std::string>& getStringFunctions(void);


    void restart(void);
    bool initialise(void);

    // int16_t functions
    Expression::Numeric IARR(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric SARR(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric PEEK(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric DEEK(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric USR(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric RND(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric URND(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LEN(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric GET(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ABS(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric SGN(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ASC(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric STRCMP(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric BCDCMP(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric VAL(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LUP(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ADDR(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric POINT(Expression::Numeric& numeric,  const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric MIN(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric MAX(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric CLAMP(Expression::Numeric& numeric,  const std::string& moduleName, const std::string& codeLineText, int codeLineStart);

    // string functions
    Expression::Numeric CHR$(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric SPC$(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric STR$(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric STRING$(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric TIME$(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric HEX$(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric LEFT$(Expression::Numeric& numeric,   const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric RIGHT$(Expression::Numeric& numeric,  const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric MID$(Expression::Numeric& numeric,    const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric LOWER$(Expression::Numeric& numeric,  const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric UPPER$(Expression::Numeric& numeric,  const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
    Expression::Numeric STRCAT$(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineIndex);
}

#endif