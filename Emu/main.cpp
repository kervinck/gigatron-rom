/**********************************************************************************************/
/* gtemuSDL ver 0.3.2                                                                         */
/*                                                                                            */ 
/* gtemuSDL is an emulator for the Gigatron TTL microcomputer, written in C++ using SDL2.     */
/* This project provides Microsoft Windows support and should be compatible with Linux, MacOS */
/* and possibly even Android. As of this version it has only been tested on Windows 10 x64.   */
/**********************************************************************************************/


#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "graphics.h"
#include "expression.h"
#include "assembler.h"


int main(int argc, char* argv[])
{
    Cpu::State S;

    Cpu::initialise(S);
    Audio::initialise();
    Graphics::initialise();
    Expression::initialise();
    Assembler::initialise();

    bool debugging = false;

    int vgaX = 0, vgaY = 0;
    int HSync = 0, VSync = 0;

    for(;;)
    {
        int64_t clock = Cpu::getClock();

        // MCP100 Power-On Reset
        if(clock < 0) S._PC = 0; 

        // Update CPU
        Cpu::State T = Cpu::cycle(S);

        HSync = (T._OUT & 0x40) - (S._OUT & 0x40);
        VSync = (T._OUT & 0x80) - (S._OUT & 0x80);
        
        // Falling vSync edge
        if(VSync < 0)
        {
            vgaY = VSYNC_START;

            // Input and graphics
            if(!debugging)
            {
                Editor::handleInput();
                Graphics::render();
            }
        }

        // Pixel
        if(vgaX++ < HLINE_END)
        {
            if(vgaY >= 0  &&  vgaY < SCREEN_HEIGHT  &&  vgaX >=HPIXELS_START  &&  vgaX < HPIXELS_END)
            {
                if(!debugging) Graphics::refreshPixel(S, vgaX-HPIXELS_START, vgaY);
            }
        }

        // Watchdog
        static int64_t clock_prev = clock;
        if(!debugging  &&  clock > STARTUP_DELAY_CLOCKS  &&  clock - clock_prev > CPU_STALL_CLOCKS)
        {
            clock_prev = CLOCK_RESET;
            Cpu::setClock(CLOCK_RESET);
            vgaX = 0, vgaY = 0;
            HSync = 0, VSync = 0;
            fprintf(stderr, "main(): CPU stall for %lld clocks : rebooting...\n", clock - clock_prev);
        }

        // Rising hSync edge
        if(HSync > 0)
        {
            clock_prev = clock;

            Cpu::setXOUT(T._AC);
            
            // Audio
            Audio::playSample();

            // Loader
            Loader::upload(vgaY);

            vgaX = 0;
            vgaY++;

            // Change this once in a while
            T._undef = rand() & 0xff;
        }

        // Debugger
        debugging = Editor::singleStepDebug();

        // Master clock
        clock = Cpu::getClock();
        Cpu::setClock(++clock);

        S=T;
    }

    return 0;
}