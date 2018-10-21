#ifndef COMPILER_H
#define COMPILER_H


namespace Compiler
{
    void initialise(void);
    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
