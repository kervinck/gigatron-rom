#ifndef PRAGMAS_H
#define PRAGMAS_H

#include <string>
#include <map>


namespace Pragmas
{
    enum PragmaResult {PragmaNotFound, PragmaError, PragmaFound};

    using PragamFuncPtr = bool (*)(const std::string& input, int codeLineIndex, size_t foundPos);
    struct Pragma
    {
        std::string _name;
        PragamFuncPtr _func;
    };


    std::map<std::string, Pragma>& getPragmas(void);


    bool initialise(void);

    bool findPragma(std::string code, const std::string& pragma, size_t& foundPos);
    PragmaResult handlePragmas(std::string& input, int codeLineIndex);

    // Pragmas
    bool CODEROMTYPE(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool RUNTIMEPATH(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool RUNTIMESTART(const std::string& input,       int codeLineIndex, size_t foundPos);
    bool USERCODESTART(const std::string& input,      int codeLineIndex, size_t foundPos);
    bool ARRAYSSTART(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool STRINGSSTART(const std::string& input,       int codeLineIndex, size_t foundPos);
    bool STRINGWORKAREA(const std::string& input,     int codeLineIndex, size_t foundPos);
    bool TEMPVARSIZE(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool CODEOPTIMISETYPE(const std::string& input,   int codeLineIndex, size_t foundPos);
    bool ARRAYINDICIESONE(const std::string& input,   int codeLineIndex, size_t foundPos);
    bool MAXNUMSPRITES(const std::string& input,      int codeLineIndex, size_t foundPos);
    bool SPRITESTRIPECHUNKS(const std::string& input, int codeLineIndex, size_t foundPos);
    bool ENABLE6BITAUDIOEMU(const std::string& input, int codeLineIndex, size_t foundPos);
}

#endif