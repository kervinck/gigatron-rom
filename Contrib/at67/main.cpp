/*******************************************************************************/
/* gtemuAT67 ver0.8.11                                                          */
/*                                                                             */ 
/* gtemuAT67 is an emulator, controller, assembler, (vCPU and Native code),    */
/* disassembler, debugger and soon to be a BASIC compiler for the Gigatron TTL */
/* microcomputer. Written in C++ using SDL2 by Ari Tsironis, (see the LICENSE  */
/* and README.md in Contrib\at67 for comprehensive documentation), this        */
/* project provides Microsoft Windows, Linux and MacOS support.                */
/*******************************************************************************/


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
    Memory::initialise();
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