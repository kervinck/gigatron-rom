#ifndef COMPILER_H
#define COMPILER_H


#include "expression.h"


namespace Compiler
{
    bool initialise(void);
    bool intialiseMacros(void);

    Expression::Numeric expression(void);

    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
