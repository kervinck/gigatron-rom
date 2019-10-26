#ifndef COMPILER_H
#define COMPILER_H


#include "expression.h"


namespace Compiler
{
    bool initialise(void);
    bool initialiseMacros(void);

    Expression::Numeric expression(void);

    bool checkExclusionZone(const std::string& opcode, uint16_t vPC, uint16_t& nextPC);

    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
