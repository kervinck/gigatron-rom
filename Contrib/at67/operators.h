#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>
#include <vector>
#include <map>

#include "expression.h"
#include "compiler.h"


namespace Operators
{
    std::vector<std::string>& getOperators(void);

    bool initialise(void);

    void changeToTmpVar(Expression::Numeric& numeric);
    void createSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);
    void handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);
    void selectSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);

    // Unary logic operators
    Expression::Numeric POS(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric NEG(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric NOT(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);

    // Unary math operators
    Expression::Numeric CEIL(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric FLOOR(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric POWF(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric SQRT(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric EXP(Expression::Numeric&   numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric EXP2(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LOG(Expression::Numeric&   numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LOG2(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LOG10(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric SIN(Expression::Numeric&   numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric COS(Expression::Numeric&   numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric TAN(Expression::Numeric&   numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ASIN(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ACOS(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ATAN(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ATAN2(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric RAND(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric REV16(Expression::Numeric& numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric REV8(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric REV4(Expression::Numeric&  numeric, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);

    // Binary logic operators
    Expression::Numeric AND(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric XOR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric OR(Expression::Numeric&  left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LSL(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LSR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric ASR(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);

    // Binary math operators
    Expression::Numeric ADD(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric SUB(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric MUL(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric DIV(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric MOD(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric POW(Expression::Numeric& left, Expression::Numeric& right, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);

    // Relational operators
    Expression::Numeric EQ(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric NE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric GE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric LT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
    Expression::Numeric GT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType, const std::string& moduleName, const std::string& codeLineText, int codeLineStart);
}

#endif