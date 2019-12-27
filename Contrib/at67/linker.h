#ifndef LINKER_H
#define LINKER_H


namespace Linker
{
    bool initialise(void);

    bool parseIncludes(void);

    bool linkInternalSubs(void);

    void collectInternalRuntime(void);
    void relinkInternalSubs(void);
    void outputInternalSubs(void);

    void resetIncludeFiles(void);
    void resetInternalSubs(void);
}

#endif
