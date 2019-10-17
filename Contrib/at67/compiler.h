#ifndef COMPILER_H
#define COMPILER_H


namespace Compiler
{
    bool initialise(void);
    bool intialiseMacros(void);

    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
