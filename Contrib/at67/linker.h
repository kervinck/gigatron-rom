#ifndef LINKER_H
#define LINKER_H


namespace Linker
{
    typedef std::pair<int, uint16_t> SubIndexSize;


    bool getInternalSub(const std::string& name, Compiler::InternalSub& internalSub);
    bool setInternalSub(const std::string& name, const Compiler::InternalSub& internalSub);
    bool setInternalSubToLoad(const std::string& name);

    void reset(void);
    bool initialise(void);

    bool enableFontLinking(void);
    bool disableFontLinking(void);

    bool parseIncludes(void);

    bool linkInternalSubs(void);

    void collectInternalRuntime(void);
    void relinkInternalSubs(void);
    void outputInternalSubs(void);
}

#endif
