#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <vector>
#include <map>

#include "compiler.h"


#if defined(CONST)
#undef CONST
#endif


namespace Keywords
{
    enum KeywordResult {KeywordNotFound, KeywordError, KeywordFound};


    struct KeywordFuncResult
    {
        int16_t _data = 0;
        std::string _name;
    };
    
    using KeywordFuncPtr = bool (*)(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    struct Keyword
    {
        std::string _name;
        KeywordFuncPtr _func;
        Compiler::StatementResult _result;
    };


    std::map<std::string, Keyword>& getKeywords(void);
    std::map<std::string, std::string>& getEqualsKeywords(void);

    void reset(void);
    void restart(void);
    bool initialise(void);

    bool findKeyword(std::string code, const std::string& keyword, size_t& foundPos);
    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, int tokenIndex, KeywordFuncResult& result);

#ifndef STAND_ALONE
    bool convertGprintGbasToGprintfAsm(void);
#endif

    // Keywords
    bool END(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool INC(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool DEC(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ON(Compiler::CodeLine& codeLine,      int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool GOTO(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool GOSUB(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool RETURN(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool RET(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool CLS(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool PRINT(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool INPUT(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool FOR(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool NEXT(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool IF(Compiler::CodeLine& codeLine,      int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ELSEIF(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ELSE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ENDIF(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool WHILE(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool WEND(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool REPEAT(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool UNTIL(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool FOREVER(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool AS(Compiler::CodeLine& codeLine,      int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool TYPE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool CALL(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool PROC(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ENDPROC(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool LOCAL(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool CONST(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool DIM(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool FUNC(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool DEF(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool DATA(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool READ(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool RESTORE(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ALLOC(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool FREE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool AT(Compiler::CodeLine& codeLine,      int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool PUT(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool MODE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool WAIT(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool PSET(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool LINE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool HLINE(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool VLINE(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool CIRCLE(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool CIRCLEF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool RECT(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool RECTF(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool POLY(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool POLYR(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool TCLIP(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool SCROLL(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool POKE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool DOKE(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool INIT(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool TICK(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool PLAY(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool LOAD(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool SPRITE(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool SOUND(Compiler::CodeLine& codeLine,   int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool SET(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ASM(Compiler::CodeLine& codeLine,     int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool ENDASM(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool BCDADD(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool BCDSUB(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool BCDINT(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool BCDCPY(Compiler::CodeLine& codeLine,  int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool GPRINTF(Compiler::CodeLine& codeLine, int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool EXEC(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
    bool OPEN(Compiler::CodeLine& codeLine,    int codeLineIndex, int codeLineStart, int tokenIndex, size_t foundPos, KeywordFuncResult& result);
}

#endif