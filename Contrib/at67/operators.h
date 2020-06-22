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

    void createTmpVar(Expression::Numeric& numeric);
    void createSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);
    void handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);

    // Unary logic operators
    Expression::Numeric POS(Expression::Numeric& numeric);
    Expression::Numeric NEG(Expression::Numeric& numeric);
    Expression::Numeric NOT(Expression::Numeric& numeric);

    // Unary math operators
    Expression::Numeric CEIL(Expression::Numeric& numeric);
    Expression::Numeric FLOOR(Expression::Numeric& numeric);
    Expression::Numeric POWF(Expression::Numeric& numeric);
    Expression::Numeric SQRT(Expression::Numeric& numeric);
    Expression::Numeric EXP(Expression::Numeric& numeric);
    Expression::Numeric EXP2(Expression::Numeric& numeric);
    Expression::Numeric LOG(Expression::Numeric& numeric);
    Expression::Numeric LOG2(Expression::Numeric& numeric);
    Expression::Numeric LOG10(Expression::Numeric& numeric);
    Expression::Numeric SIN(Expression::Numeric& numeric);
    Expression::Numeric COS(Expression::Numeric& numeric);
    Expression::Numeric TAN(Expression::Numeric& numeric);
    Expression::Numeric ASIN(Expression::Numeric& numeric);
    Expression::Numeric ACOS(Expression::Numeric& numeric);
    Expression::Numeric ATAN(Expression::Numeric& numeric);
    Expression::Numeric ATAN2(Expression::Numeric& numeric);
    Expression::Numeric RAND(Expression::Numeric& numeric);
    Expression::Numeric REV16(Expression::Numeric& numeric);
    Expression::Numeric REV8(Expression::Numeric& numeric);
    Expression::Numeric REV4(Expression::Numeric& numeric);

    // Binary logic operators
    Expression::Numeric AND(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric XOR(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric OR(Expression::Numeric&  left, Expression::Numeric& right);
    Expression::Numeric LSL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric LSR(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric ASR(Expression::Numeric& left, Expression::Numeric& right);

    // Binary math operators
    Expression::Numeric ADD(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric SUB(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric MUL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric DIV(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric MOD(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric POW(Expression::Numeric& left, Expression::Numeric& right);

    // Relational operators
    Expression::Numeric EQ(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric NE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric LE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric GE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric LT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric GT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
}

#endif