#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <map>

#include "expression.h"


namespace Functions
{
    std::map<std::string, std::string>& getFunctions(void);
    std::map<std::string, std::string>& getStringFunctions(void);

    bool initialise(void);

    Expression::Numeric IARR(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric SARR(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric PEEK(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric DEEK(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric USR(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric RND(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric LEN(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric GET(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric ABS(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric SGN(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric ASC(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric STRCMP(Expression::Numeric& numeric, int codeLineIndex);
    Expression::Numeric BCDCMP(Expression::Numeric& numeric, int codeLineIndex);
    Expression::Numeric VAL(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric LUP(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric ADDR(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric POINT(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric MIN(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric MAX(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric CLAMP(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric SWAP(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric CHR$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric STR$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric TIME$(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric HEX$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric HEXW$(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric LEFT$(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric RIGHT$(Expression::Numeric& numeric, int codeLineIndex);
    Expression::Numeric MID$(Expression::Numeric& numeric,   int codeLineIndex);
}

#endif