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

    for(long long t=-2; ; t++) 
    {
        if(t < 0) S._PC = 0; // MCP100 Power-On Reset

        Cpu::State T = Cpu::cycle(S); // Update CPU

        HSync = (T._OUT & 0x40) - (S._OUT & 0x40); // "VGA monitor" (use simple stdout)
        VSync = (T._OUT & 0x80) - (S._OUT & 0x80);
        if(VSync < 0) // Falling vSync edge
        {
            vgaY = VSYNC_START;

            // Input and graphics
            if(!debugging)
            {
                Editor::handleInput();
                Graphics::render();
            }
        }
        if(vgaX++ < HLINE_END)
        {
            if(HSync) { }              // Visual indicator of hSync
            else if(vgaX == HLINE_END) { }   // Too many pixels
            else if(~S._OUT & 0x80) { } // Visualize vBlank pulse
            else if(vgaX >=HPIXELS_START  &&  vgaX < HPIXELS_END  &&  vgaY >= 0  &&  vgaY < SCREEN_HEIGHT)
            {
                if(!debugging) Graphics::refreshPixel(S, vgaX-HPIXELS_START, vgaY);
            }
        }
        if(HSync > 0) // Rising hSync edge
        {
            Cpu::setXOUT(T._AC);
            
            // Audio
            Audio::playSample();

            // Upload user code to loader
            Loader::upload(vgaY);

            vgaX = 0;
            vgaY++;
            T._undef = rand() & 0xff; // Change this once in a while
        }

        debugging = Editor::singleStepDebug();

        S=T;
    }

    return 0;
}