#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <vector>
#include <map>

#include "compiler.h"


namespace Keywords
{
    enum KeywordResult {KeywordNotFound, KeywordError, KeywordFound};

    struct KeywordFuncResult
    {
        int16_t _data = 0;
        std::string _name;
    };

    using KeywordFuncPtr = bool (*)(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);

    struct Keyword
    {
        int _params;
        std::string _name;
        KeywordFuncPtr _func;
    };


    std::map<std::string, Keyword>& getKeywords(void);
    std::map<std::string, Keyword>& getStringKeywords(void);
    std::vector<std::string>& getEqualsKeywords(void);

    bool initialise(void);

    bool findKeyword(std::string code, const std::string& keyword, size_t& foundPos);
    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, KeywordFuncResult& result);

    bool keywordREM(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLET(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordEND(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordON(Compiler::CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOTO(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOSUB(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRETURN(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCLS(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordINPUT(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPRINT(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFOR(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordNEXT(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordIF(Compiler::CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSE(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSEIF(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordENDIF(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWHILE(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWEND(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordREPEAT(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordUNTIL(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDIM(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDEF(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordAT(Compiler::CodeLine& codeLine,     int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPUT(Compiler::CodeLine& codeLine,    int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordMODE(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWAIT(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLINE(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordHLINE(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordVLINE(Compiler::CodeLine& codeLine,  int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSCROLL(Compiler::CodeLine& codeLine, int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPOKE(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDOKE(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPLAY(Compiler::CodeLine& codeLine,   int codeLineIndex, size_t foundPos, KeywordFuncResult& result);
}

#endif