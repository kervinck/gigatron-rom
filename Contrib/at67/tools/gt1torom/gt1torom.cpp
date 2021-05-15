#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include "../../memory.h"


#define GT1TOROM_MAJOR_VERSION "0.2"
#define GT1TOROM_MINOR_VERSION "0"
#define GT1TOROM_VERSION_STR "gt1torom v" GT1TOROM_MAJOR_VERSION "." GT1TOROM_MINOR_VERSION

#define GT1_MAX_SIZE (1<<16)

#define TRAMPOLINE_START 0x00FB


uint8_t _gt1[GT1_MAX_SIZE];


bool writeRomDataWithTrampoline(const std::string& outputFilename0, const std::string& outputFilename1, std::ofstream& outfile0, std::ofstream& outfile1, uint16_t& startAddress, uint16_t size, bool _default)
{
    uint16_t trampolineOffset = 0x0000;
    for(uint16_t i=0; i<size; i++)
    {
        uint16_t address = startAddress + i + trampolineOffset;

        // Write ROM trampoline
        if(LO_BYTE(address) == TRAMPOLINE_START)
        {
            static uint8_t trampolineOpcode[]  = {0xFE, 0xFC, 0x14, 0xE0, 0xC2};
            static uint8_t trampolineOperand[] = {0x00, 0xFD, 0x04, 0x65, 0x18};

            for(int j=0; j<int(sizeof(trampolineOpcode)); j++)
            {
                outfile0.write((char *)&trampolineOpcode[j], 1);
                if(outfile0.bad() || outfile0.fail())
                {
                    fprintf(stderr, "gt1torom : write error at address %04x in file '%s'\n", address + j, outputFilename0.c_str());
                    return false;
                }

                outfile1.write((char *)&trampolineOperand[j], 1);
                if(outfile1.bad() || outfile1.fail())
                {
                    fprintf(stderr, "gt1torom : write error at address %04x in file '%s'\n", address + j, outputFilename1.c_str());
                    return false;
                }
            }

            trampolineOffset += sizeof(trampolineOpcode);
        }
        
        // Don't write default data after last trampoline
        if(LO_BYTE(address) != TRAMPOLINE_START  ||  !_default)
        {
            static char nativeLoad[2] = {0x00, 0x00};
            outfile0.write(&nativeLoad[0], 1);
            if(outfile0.bad() || outfile0.fail())
            {
                fprintf(stderr, "gt1torom : write error at address %04x in file '%s'\n", startAddress + i, outputFilename0.c_str());
                return false;
            }

            if (_default)
                outfile1.write(&nativeLoad[1], 1);
            else
                outfile1.write((char *)&_gt1[i], 1);
            if(outfile1.bad() || outfile1.fail())
            {
                fprintf(stderr, "gt1torom : write error at address %04x in file '%s'\n", startAddress + i, outputFilename1.c_str());
                return false;
            }
        }
    }

    startAddress += size + trampolineOffset;

    return true;
}


int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        fprintf(stderr, "%s\n", GT1TOROM_VERSION_STR);
        fprintf(stderr, "Usage:   gt1torom <input filename> <output filename> <uint16_t start address in hex>\n");
        return 1;
    }

    std::string inputFilename = std::string(argv[1]);
    if(inputFilename.find(".gt1") == inputFilename.npos  &&  inputFilename.find(".GT1") == inputFilename.npos)
    {
        fprintf(stderr, "Wrong file extension in %s : must be '.gt1'\n", inputFilename.c_str());
        return 1;
    }

    // Check for gt1 file
    std::ifstream gt1file(inputFilename, std::ios::binary | std::ios::in);
    if(!gt1file.is_open())
    {
        fprintf(stderr, "gt1torom : couldn't open %s GT1 file.\n", inputFilename.c_str());
        return 1;
    }

    // Load gt1 file
    gt1file.read((char *)_gt1, GT1_MAX_SIZE);
    if(gt1file.bad())
    {
        fprintf(stderr, "gt1torom : failed to read %s GT1 file.\n", inputFilename.c_str());
        return 1;
    }
    uint16_t gt1Size = uint16_t(gt1file.gcount());

    std::string outputFilename0 = std::string(argv[2]) + "_ti";
    std::ofstream outfile0(outputFilename0, std::ios::binary | std::ios::out);
    if(!outfile0.is_open())
    {
        fprintf(stderr, "gt1torom : failed to open '%s'\n", outputFilename0.c_str());
        return 1;
    }

    std::string outputFilename1 = std::string(argv[2]) + "_td";
    std::ofstream outfile1(outputFilename1, std::ios::binary | std::ios::out);
    if(!outfile1.is_open())
    {
        fprintf(stderr, "gt1torom : failed to open '%s'\n", outputFilename1.c_str());
        return 1;
    }

    // Handles hex numbers
    uint16_t startAddress = 0x0000;
    std::stringstream ss;
    ss << std::hex << argv[3];
    ss >> startAddress;

    if(!writeRomDataWithTrampoline(outputFilename0, outputFilename1, outfile0, outfile1, startAddress, gt1Size, false)) return 1;
    if(!writeRomDataWithTrampoline(outputFilename0, outputFilename1, outfile0, outfile1, startAddress, TRAMPOLINE_START + 1 - LO_BYTE(startAddress), true)) return 1;

    fprintf(stderr, "%s success : next available address : 0x%04X\n", GT1TOROM_VERSION_STR, startAddress - 1);

    return 0;
}
