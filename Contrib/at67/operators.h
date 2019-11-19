#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>
#include <vector>
#include <map>

#include "compiler.h"


namespace Operators
{
    bool initialise(void);

    bool handleSingleOp(const std::string& opcodeStr, Expression::Numeric& numeric);

    Expression::Numeric operatorNEG(Expression::Numeric& numeric);
    Expression::Numeric operatorNOT(Expression::Numeric& numeric);

    Expression::Numeric operatorADD(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorSUB(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorMUL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorDIV(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorMOD(Expression::Numeric& left, Expression::Numeric& right);

    Expression::Numeric operatorAND(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorXOR(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorOR(Expression::Numeric&  left, Expression::Numeric& right);
    Expression::Numeric operatorLSL(Expression::Numeric& left, Expression::Numeric& right);
    Expression::Numeric operatorLSR(Expression::Numeric& left, Expression::Numeric& right);

    Expression::Numeric operatorEQ(Expression::Numeric& left, Expression::Numeric& right, bool logical);
    Expression::Numeric operatorNE(Expression::Numeric& left, Expression::Numeric& right, bool logical);
    Expression::Numeric operatorLE(Expression::Numeric& left, Expression::Numeric& right, bool logical);
    Expression::Numeric operatorGE(Expression::Numeric& left, Expression::Numeric& right, bool logical);
    Expression::Numeric operatorLT(Expression::Numeric& left, Expression::Numeric& right, bool logical);
    Expression::Numeric operatorGT(Expression::Numeric& left, Expression::Numeric& right, bool logical);
}

#endif