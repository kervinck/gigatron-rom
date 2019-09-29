#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "../../cpu.h"


#define GTSPLITROM_MAJOR_VERSION "0.2"
#define GTSPLITROM_MINOR_VERSION "2"
#define GTSPLITROM_VERSION_STR "gtsplitrom v" GTSPLITROM_MAJOR_VERSION "." GTSPLITROM_MINOR_VERSION


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s\n", GTSPLITROM_VERSION_STR);
        fprintf(stderr, "Usage:   gtsplitrom <input filename> <output filename>\n");
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
        fprintf(stderr, "gtsplitrom : couldn't open %s ROM file.\n", inputFilename.c_str());
        return 1;
    }

    // Load ROM file
    int romSize = 0;
    uint8_t* _ROM = Cpu::getPtrToROM(romSize);
    romfile.read((char *)_ROM, romSize);
    if(romfile.bad() || romfile.fail())
    {
        fprintf(stderr, "gtsplitrom : failed to read %s ROM file.\n", inputFilename.c_str());
        return 1;
    }

    // ROM file is not big enough
    if(romfile.gcount() < romSize)
    {
        fprintf(stderr, "gtsplitrom : ROM file %s is not large enough : size is %d : required size is %d\n", inputFilename.c_str(), int(romfile.gcount()), romSize);
        return 1;
    }

//#define ROM_VER_0x1c
#if defined(ROM_VER_0x1c)
    // Initialises internal GT1 vector
    Cpu::initialiseInternalGt1s();

    // Patches SYS_Exec_88 loader to accept page0 segments as the first segment and works with 64KB SRAM hardware
    Cpu::patchSYS_Exec_88();

    // Modifies ROM to disable scanline videoB
    Cpu::patchScanlineModeVideoB();
#endif

    std::string outputFilename0 = std::string(argv[2]) + "_i";
    std::ofstream outfile0(outputFilename0, std::ios::binary | std::ios::out);
    if(!outfile0.is_open())
    {
        fprintf(stderr, "gtsplitrom : failed to open '%s'\n", outputFilename0.c_str());
        return 1;
    }

    std::string outputFilename1 = std::string(argv[2]) + "_d";
    std::ofstream outfile1(outputFilename1, std::ios::binary | std::ios::out);
    if(!outfile1.is_open())
    {
        fprintf(stderr, "gtsplitrom : failed to open '%s'\n", outputFilename1.c_str());
        return 1;
    }

    for(int i=0; i<romSize; i+=2)
    {
        outfile0.write((char *)&_ROM[i + 0], 1);
        if(outfile0.bad() || outfile0.fail())
        {
            fprintf(stderr, "gtsplitrom : write error at address %04x in file '%s'\n", i, outputFilename0.c_str());
            return 1;
        }

        outfile1.write((char *)&_ROM[i + 1], 1);
        if(outfile1.bad() || outfile1.fail())
        {
            fprintf(stderr, "gtsplitrom : write error at address %04x in file '%s'\n", i, outputFilename1.c_str());
            return 1;
        }
    }

    fprintf(stderr, "%s success.\n", GTSPLITROM_VERSION_STR);

    return 0;
}
