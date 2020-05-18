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
    
    using PragamFuncPtr = bool (*)(const std::string& input, int codeLineIndex, size_t foundPos);
    struct Pragma
    {
        std::string _name;
        PragamFuncPtr _func;
    };

    using KeywordFuncPtr = bool (*)(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    struct Keyword
    {
        std::string _name;
        KeywordFuncPtr _func;
        Compiler::StatementResult _result;
    };


    std::vector<std::string>& getOperators(void);
    std::map<std::string, Pragma>& getPragmas(void);
    std::map<std::string, Keyword>& getKeywords(void);
    std::map<std::string, std::string>& getFunctions(void);
    std::map<std::string, std::string>& getStringKeywords(void);
    std::map<std::string, std::string>& getEqualsKeywords(void);

    bool initialise(void);

    bool findPragma(std::string code, const std::string& pragma, size_t& foundPos);
    KeywordResult handlePragmas(std::string& input, int codeLineIndex);

    bool findKeyword(std::string code, const std::string& keyword, size_t& foundPos);
    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, int tokenIndex, KeywordFuncResult& result);

    Expression::Numeric functionARR(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionPEEK(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionDEEK(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionUSR(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionRND(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionLEN(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionGET(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionABS(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionSGN(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionASC(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionCMP(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionVAL(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionLUP(Expression::Numeric& numeric,    int codeLineIndex);
    Expression::Numeric functionADDR(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionPOINT(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric functionCHR$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionSTR$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionHEX$(Expression::Numeric& numeric,   int codeLineIndex);
    Expression::Numeric functionHEXW$(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric functionLEFT$(Expression::Numeric& numeric,  int codeLineIndex);
    Expression::Numeric functionRIGHT$(Expression::Numeric& numeric, int codeLineIndex);
    Expression::Numeric functionMID$(Expression::Numeric& numeric,   int codeLineIndex);

    bool pragmaCODEROMTYPE(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool pragmaRUNTIMEPATH(const std::string& input,        int codeLineIndex, size_t foundPos);
    bool pragmaRUNTIMESTART(const std::string& input,       int codeLineIndex, size_t foundPos);
    bool pragmaSTRINGWORKAREA(const std::string& input,     int codeLineIndex, size_t foundPos);
    bool pragmaCODEOPTIMISETYPE(const std::string& input,   int codeLineIndex, size_t foundPos);
    bool pragmaARRAYINDICIESONE(const std::string& input,   int codeLineIndex, size_t foundPos);
    bool pragmaSPRITESTRIPECHUNKS(const std::string& input, int codeLineIndex, size_t foundPos);

    // Keywords
    bool keywordEND(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordINC(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDEC(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordON(Compiler::CodeLine& codeLine,       int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOTO(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordGOSUB(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRETURN(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCLS(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPRINT(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordINPUT(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFOR(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordNEXT(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordIF(Compiler::CodeLine& codeLine,       int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSEIF(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordELSE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordENDIF(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWHILE(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWEND(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordREPEAT(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordUNTIL(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCONST(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDIM(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFUNC(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDEF(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordALLOC(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordFREE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordAT(Compiler::CodeLine& codeLine,       int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPUT(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordMODE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordWAIT(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPSET(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLINE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordHLINE(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordVLINE(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCIRCLE(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordCIRCLEF(Compiler::CodeLine& codeLine,  int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRECT(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordRECTF(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPOLY(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSCROLL(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPOKE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordDOKE(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordPLAY(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordLOAD(Compiler::CodeLine& codeLine,     int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSPRITE(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSOUND(Compiler::CodeLine& codeLine,    int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordSET(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordASM(Compiler::CodeLine& codeLine,      int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool keywordENDASM(Compiler::CodeLine& codeLine,   int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
}

#endif