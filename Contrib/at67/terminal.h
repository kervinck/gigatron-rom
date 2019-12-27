#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <string>

#ifndef STAND_ALONE
#include <SDL.h>
#endif


namespace Terminal
{
    void initialise(void);

    void scrollToEnd(void);
    void handleTerminalInput(void);

    void process(void);
}

#endif