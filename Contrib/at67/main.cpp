/**********************************************************************************************/
/* gtemuAT67                                                                                  */
/*                                                                                            */ 
/* gtemuAT67 is an emulator for the Gigatron TTL microcomputer, written in C++ using SDL2.    */
/* This project provides Microsoft Windows support and should be compatible with Linux, MacOS */
/* and possibly even Android. As of this version it has only been tested on Windows 10 x64.   */
/**********************************************************************************************/


#include "memory.h"
#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "graphics.h"
#include "expression.h"
#include "assembler.h"
#include "compiler.h"


int main(int argc, char* argv[])
{
    Memory::intitialise();
    Loader::initialise();
    Cpu::initialise();
    Audio::initialise();
    Graphics::initialise();
    Editor::initialise();
    Expression::initialise();
    Assembler::initialise();
    Compiler::initialise();

    //Compiler::compile("gbas/test.gbas", "gbas/test.gasm");

    while(Cpu::process());

    return 0;
}