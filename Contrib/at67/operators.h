#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>
#include <vector>
#include <map>

#include "expression.h"
#include "compiler.h"


#define MATH_PI 3.1415926535897932384626433832795028

namespace Operators
{
    bool initialise(void);

    void handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);

    // Unary logic operators
    Expression::Numeric operatorPOS(Expression::Numeric& numeric);
    Expression::Numeric operatorNEG(Expression::Numeric& numeric);
    Expression::Numeric operatorNOT(Expression::Numeric& numeric);

    // Unary math operators
    Expression::Numeric operatorSIN(Expression::Numeric& numeric);
    Expression::Numeric operatorCOS(Expression::Numeric& numeric);
    Expression::Numeric operatorTAN(Expression::Numeric& numeric);
    Expression::Numeric operatorASIN(Expression::Numeric& numeric);
    Expression::Numeric operatorACOS(Expression::Numeric& numeric);
    Expression::Numeric operatorATAN(Expression::Numeric& numeric);
    Expression::Numeric operatorRAND(Expression::Numeric& numeric);
    Expression::Numeric operatorREV16(Expression::Numeric& numeric);
    Expression::Numeric operatorREV8(Expression::Numeric& numeric);
    Expression::Numeric operatorREV4(Expression::Numeric& numeric);

    // Binary logic operators
    Expression::Numeric operatorAND(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorXOR(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorOR(Expression::Numeric&  left, Expression::Numeric& right);
    Expression::Numeric operatorLSL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorLSR(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorASR(Expression::Numeric& left, Expression::Numeric& right);

    // Binary math operators
    Expression::Numeric operatorADD(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorSUB(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorMUL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorDIV(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorMOD(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorPOW(Expression::Numeric& left, Expression::Numeric& right);

    // Relational operators
    Expression::Numeric operatorEQ(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric operatorNE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric operatorLE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric operatorGE(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric operatorLT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
    Expression::Numeric operatorGT(Expression::Numeric& left, Expression::Numeric& right, Expression::CCType ccType);
}

#endif