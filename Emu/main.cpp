/*
Ver 0.2.2
- YouTube https://youtu.be/fH30TR6jeQM

Features:
- Robust variable timing from a minimum of 60FPS up to whatever your PC can handle.
- Robust synchronisation of audio with video at any FPS at or above 60FPS.
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.
- Supports fullscreen optimised rendering.
- Supports Gigatron TTL input buttons.
- Supports Gigatron TTL LED display.
- Supports Gigatron TTL audio channels.
- Supports Gigatron TTL loading of external vCPU code with a file browser.
- Displays Gigatron TTL IN and XOUT registers.
- Displays memory monitor of RAM, (RAM is editable), ROM0 and ROM1 at any start address.
- Displays read only contents of vCPU variable space.
- Can execute hand crafted code within the Hex Editor.
- Three seperate editable start addresses are provided; memory display address,
vCPU vars display address and load start address.

Adapted from:
- gigatron-rom https://github.com/kervinck/gigatron-rom
- SDL(2) graphics https://github.com/kervinck/gigatron-rom/pull/1
- http://talkchess.com/forum/viewtopic.php?topic_view=threads&p=742122&t=65944 and
http://talkchess.com/forum/viewtopic.php?t=65944&start=11

Building:
- CMake 3.7 or higher is required for building, has only been tested on Windows so far.
- A C++17 compatible compiler will be needed to build this code, if one is not available,
then the std::filesystem section of code may be commented out.
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.

Installation:
- After building, copy the executable, SDL2 shared library/DLL, "theloop2.rom" and
"EmuFont-96x48.bmp" to an appropriate directory; run the executable from there.
- Create a vCPU directory within the above directory and add any user generated vCPU files
that you want to upload to the emulator.

Controls:
- <ESC>     Quits the application.
- <l>       Loads external vCPU files within the vCPU directory, this code is uploaded to
            an editable load address. Loading user vCPU code to system critical addresses
            can cause the emulator to hang, 0x0200 is guaranteed to be safe.
- <r>       Switches Hex Editor between RAM, ROM(0) and ROM(1).
- <F5>      Executes whatever code is present at the load address.
- <CR>      Directly loads external vCPU code if editor is in file browse mode, otherwise
            switches to edit mode.
- <-/+>     Decrease/increase the speed of the emulation, from a minimum of 60FPS to a
            maximum determined by your PC's CPU.

- <Left>    Navigate the Hex editor one byte at a time or the file browser one file at a time.
- <Right>
- <Up/Down>

- <PgUp>    Scroll the hex editor and file browser one page at a time.
- <PgDn>

- <Wheel>   Scroll the hex editor and file browser one line/file at a time.

- <d>       Right.
- <a>       Left.
- <w>       Up.
- <s>       Down.
- <SPACE>   Start.
- <z>       Select.
- </>       B.
- <.>       A.

Limitations:
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail.
- Controls and VSync are modifiable in the code, VSync currently seems to have little value
as an option as the main loop is synchronised using the performance counters.

TODO:
- Test under Linux, MAC and Android.
- Allow for uploading of vCPU payloads larger than 60 bytes in size.
- Add single step debugging of vCPU code within the Hex Editor.
/*

/*
BSD 2-Clause License

Copyright (c) 2017, Marcel van Kervinck
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "graphics.h"


int main(int argc, char* argv[])
{
    Cpu::State S;

    Cpu::initialise(S);
    Audio::initialise();
    Graphics::initialise();

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
            Editor::handleInput();
            Graphics::render();
        }
        if(vgaX++ < HLINE_END)
        {
            if(HSync) { }              // Visual indicator of hSync
            else if(vgaX == HLINE_END) { }   // Too many pixels
            else if(~S._OUT & 0x80) { } // Visualize vBlank pulse
            else if(vgaX >=HPIXELS_START  &&  vgaX < HPIXELS_END  &&  vgaY >= 0  &&  vgaY < SCREEN_HEIGHT)
            {
                Graphics::drawPixel(S, vgaX-HPIXELS_START, vgaY);
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

        S=T;
    }

    return 0;
}