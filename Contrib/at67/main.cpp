/*******************************************************************************/
/* gtemuAT67 Ver 1.0.5R, (Rlease)                                              */
/*                                                                             */ 
/* gtemuAT67 is an emulator, controller, assembler, (vCPU and Native code),    */
/* disassembler, debugger and  BASIC compiler for the Gigatron TTL             */
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
#include "image.h"
#include "graphics.h"
#include "menu.h"
#include "terminal.h"
#include "expression.h"
#include "assembler.h"
#include "compiler.h"
#include "operators.h"
#include "keywords.h"
#include "pragmas.h"
#include "functions.h"
#include "optimiser.h"
#include "validater.h"
#include "linker.h"


int main(int argc, char* argv[])
{
    if(argc != 1  &&  argc != 2)
    {
        fprintf(stderr, "%s\n", VERSION_STR);
        fprintf(stderr, "Usage:   gtemuAT67 <optional input filename>\n");
        return 1;
    }

    // TODO: the order of some of these is important
    Memory::initialise();
    Loader::initialise();
    Cpu::initialise();
    Image::initialise();
    Editor::initialise();
    Audio::initialise();
    Graphics::initialise();
    Menu::initialise();
    Terminal::initialise();
    Expression::initialise();
    Assembler::initialise();
    Compiler::initialise();
    Operators::initialise();
    Functions::initialise();
    Keywords::initialise();
    Pragmas::initialise();
    Optimiser::initialise();
    Validater::initialise();
    Linker::initialise();

    // Load file
    if(argc == 2)
    {
        std::string name = std::string(argv[1]);
        size_t slash = name.find_last_of("\\/");
        std::string path = (slash != std::string::npos) ? name.substr(0, slash) : ".";
        Expression::replaceText(path, "\\", "/");
        name = (slash != std::string::npos) ? name.substr(slash + 1) : name;

#ifdef _WIN32
        Cpu::enableWin32ConsoleSaveFile(false);
#endif

        Assembler::setIncludePath(path);
        Loader::setFilePath(path + "/" + name);
        Loader::uploadDirect(Loader::Emulator);

        // Choose memory model
        if(name.find("64k") != std::string::npos  ||  name.find("64K") != std::string::npos)
        {
            if(Memory::getSizeRAM() == RAM_SIZE_LO)
            {
                Memory::setSizeRAM(RAM_SIZE_HI);
                Cpu::setSizeRAM(Memory::getSizeRAM());
            }
        }
    }

#if 0
    Image::TgaFile tgaFile;
    std::string imageName = "Clouds";
    Image::loadTgaFile(imageName + ".tga", tgaFile);

    Image::GtRgbFile gtRgbFile{GTRGB_IDENTIFIER, Image::GT_RGB_222, tgaFile._header._width, tgaFile._header._height};
    //Image::convertRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);
    Image::ditherRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);
    Image::saveGtRgbFile(imageName + ".gtrgb", gtRgbFile);
    Image::loadGtRgbFile(imageName + ".gtrgb", gtRgbFile);
    uint16_t vram = 0x0800;
    for(int y=0; y<gtRgbFile._header._height; y++)
    {
        for(int x=0; x<gtRgbFile._header._width; x++)
        {
            uint8_t data = gtRgbFile._data[y*gtRgbFile._header._width + x];
            Cpu::setRAM(vram++, data);
            if((vram & 0x00FF) == 0x00) vram += 0x0100;
        }
    }

#if 0
    Image::loadTgaFile(imageName + ".tga", tgaFile);
    gtRgbFile._header = {GTRGB_IDENTIFIER, Image::GT_RGB_222, tgaFile._header._width, tgaFile._header._height};
    Image::convertRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);
    //Image::ditherRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);
    Image::saveGtRgbFile(imageName + ".gtrgb", gtRgbFile);
    Image::loadGtRgbFile(imageName + ".gtrgb", gtRgbFile);
    vram = 0x3FA0;
    for(int y=0; y<gtRgbFile._header._height; y++)
    {
        for(int x=0; x<gtRgbFile._header._width; x++)
        {
            uint8_t data = gtRgbFile._data[y*gtRgbFile._header._width + x];
            Cpu::setRAM(vram++, data);
            if((vram & 0x00FF) == 0x00) vram += 0x00A0;
        }
    }
#endif
#endif

    while(1)
    {
        switch(Editor::getEditorMode())
        {
            case Editor::Term:  Terminal::process(); break;
            case Editor::Image: Image::process();    break;
            case Editor::Audio: Audio::process();    break;

            default: Cpu::process(); break;
        }
    }

    return 0;
}