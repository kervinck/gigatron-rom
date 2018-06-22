#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include "../../cpu.h"


#define GTMAKEROM_MAJOR_VERSION "0.2"
#define GTMAKEROM_MINOR_VERSION "1"
#define GTMAKEROM_VERSION_STR "gtmakerom v" GTMAKEROM_MAJOR_VERSION "." GTMAKEROM_MINOR_VERSION


int main(int argc, char* argv[])
{
    if(argc != 8)
    {
        fprintf(stderr, "%s\n", GTMAKEROM_VERSION_STR);
        fprintf(stderr, "Usage:   gtmakerom <input ROM filename> <input GT1 ROM filename> <output ROM filename> <title> <menu_name>\n         <int menu 0 to 5> <uint16_t start_address>\n");
        fprintf(stderr, "         <input GT1 ROM filename> must be created with gt1torom.\n");
        fprintf(stderr, "         <menu> Snake = 0, Racer = 1, Mandelbrot = 2, Pictures = 3, Credits = 4, Loader = 5\n");
        fprintf(stderr, "Example: gtmakerom ROMv1.rom tetris.rom test.rom \"TTL micromputer test v0\" Tetris 5 0x0B00\n");
        return 1;
    }

    std::string inputFilename = std::string(argv[1]);
    if(inputFilename.find(".rom") == inputFilename.npos  &&  inputFilename.find(".hex") == inputFilename.npos  &&  inputFilename.find(".bin") == inputFilename.npos  &&  
       inputFilename.find(".ROM") == inputFilename.npos  &&  inputFilename.find(".HEX") == inputFilename.npos  &&  inputFilename.find(".BIN") == inputFilename.npos)
    {
        fprintf(stderr, "Wrong file extension in %s : must be one of : '.rom' or '.hex' or '.bin'\n", inputFilename.c_str());
        return 1;
    }

    // Check for ROM file
    std::ifstream romfile(inputFilename, std::ios::binary | std::ios::in);
    if(!romfile.is_open())
    {
        fprintf(stderr, "gtmakerom : couldn't open %s ROM file.\n", inputFilename.c_str());
        return 1;
    }

    // Load ROM file
    int romSize = 0;
    uint8_t* _ROM = Cpu::getPtrToROM(romSize);
    romfile.read((char *)_ROM, romSize);
    if(romfile.bad() || romfile.fail())
    {
        fprintf(stderr, "gtmakerom : failed to read %s ROM file.\n", inputFilename.c_str());
        return 1;
    }

    // ROM file is not big enough
    if(romfile.gcount() < romSize)
    {
        fprintf(stderr, "gtmakerom : ROM file %s is not large enough : size is %d : required size is %d\n", inputFilename.c_str(), int(romfile.gcount()), romSize);
        return 1;
    }

    // Initialises internal GT1 vector
    Cpu::initialiseInternalGt1s();

    // Patches SYS_Exec_88 loader to accept page0 segments as the first segment and works with 64KB SRAM hardware
    Cpu::patchSYS_Exec_88();

    // Modifies ROM to disable scanline VideoB
    Cpu::patchScanlineModeVideoB();

    std::string gt1Filename = std::string(argv[2]);
    std::string outputFilename = std::string(argv[3]);
    std::string title = std::string(argv[4]);
    std::string menuName = std::string(argv[5]);
    Cpu::InternalGt1Id gt1Id = (Cpu::InternalGt1Id)strtol(argv[6], nullptr, 10);

    // Handles hex numbers
    uint16_t startAddress = 0x0000;
    std::stringstream ss;
    ss << std::hex << argv[7];
    ss >> startAddress;

    Cpu::patchTitleIntoRom(title);
    Cpu::patchSplitGt1IntoRom(gt1Filename, menuName, startAddress, gt1Id);

    // Merged ROM
    std::ofstream outfile(outputFilename, std::ios::binary | std::ios::out);
    if(!outfile.is_open())
    {
        fprintf(stderr, "gtmakerom : failed to open '%s'\n", outputFilename.c_str());
        return 1;
    }
    outfile.write((char *)_ROM, romSize);
    if(outfile.bad() || outfile.fail())
    {
        fprintf(stderr, "gtmakerom : write error in file '%s'\n", outputFilename.c_str());
        return 1;
    }


    // Split ROM
    std::string outputFilename0 = outputFilename + "_i";
    std::ofstream outfile0(outputFilename0, std::ios::binary | std::ios::out);
    if(!outfile0.is_open())
    {
        fprintf(stderr, "gtmakerom : failed to open '%s'\n", outputFilename0.c_str());
        return 1;
    }
    std::string outputFilename1 = outputFilename + "_d";
    std::ofstream outfile1(outputFilename1, std::ios::binary | std::ios::out);
    if(!outfile1.is_open())
    {
        fprintf(stderr, "gtmakerom : failed to open '%s'\n", outputFilename1.c_str());
        return 1;
    }
    for(int i=0; i<romSize; i+=2)
    {
        outfile0.write((char *)&_ROM[i + 0], 1);
        if(outfile0.bad() || outfile0.fail())
        {
            fprintf(stderr, "gtmakerom : write error at address %04x in file '%s'\n", i, outputFilename0.c_str());
            return 1;
        }

        outfile1.write((char *)&_ROM[i + 1], 1);
        if(outfile1.bad() || outfile1.fail())
        {
            fprintf(stderr, "gtmakerom : write error at address %04x in file '%s'\n", i, outputFilename1.c_str());
            return 1;
        }
    }

    fprintf(stderr, "%s success.\n", GTMAKEROM_VERSION_STR);

    return 0;
}
