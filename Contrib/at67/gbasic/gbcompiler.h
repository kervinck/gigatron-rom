#ifndef GBCOMPILER_H
#define GBCOMPILER_H


namespace GBcompiler
{
    void initialise(void);
    bool compile(const std::string& inputFilename, const std::string& outputFilename);
}

#endif
