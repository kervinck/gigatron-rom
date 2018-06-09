#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>


#define GT1TOROM_MAJOR_VERSION "0.1"
#define GT1TOROM_MINOR_VERSION "1"
#define GT1TOROM_VERSION_STR "gt1torom v" GT1TOROM_MAJOR_VERSION "." GT1TOROM_MINOR_VERSION

#define GT1_SIZE (1<<16)


uint8_t _gt1[GT1_SIZE];


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

    // Check for .gt1 file
    std::ifstream gt1file(inputFilename, std::ios::binary | std::ios::in);
    if(!gt1file.is_open())
    {
        fprintf(stderr, "gt1torom : couldn't open %s .gt1 file.\n", inputFilename.c_str());
        return 1;
    }

    // Load .gt1 file
    gt1file.read((char *)_gt1, sizeof(_gt1));
    if(gt1file.bad())
    {
        fprintf(stderr, "gt1torom : failed to read %s .gt1 file.\n", inputFilename.c_str());
        return 1;
    }
    int gt1Size = int(gt1file.gcount());

    std::string outputFilename0 = std::string(argv[2]) + "0";
    std::ofstream outfile0(outputFilename0, std::ios::binary | std::ios::out);
    if(!outfile0.is_open())
    {
        fprintf(stderr, "gt1torom : failed to open '%s'.\n", outputFilename0.c_str());
        return 1;
    }

    std::string outputFilename1 = std::string(argv[2]) + "1";
    std::ofstream outfile1(outputFilename1, std::ios::binary | std::ios::out);
    if(!outfile1.is_open())
    {
        fprintf(stderr, "gt1torom : failed to open '%s'.\n", outputFilename1.c_str());
        return 1;
    }

    // Handles hex numbers
    uint16_t startAddress = 0x0000;
    std::stringstream ss;
    ss << std::hex << argv[3];
    ss >> startAddress;

    uint16_t trampolineOffset = 0x0000;
    for(int i=0; i<gt1Size; i++)
    {
        uint16_t address = (startAddress + i + trampolineOffset) & 0x00FF;
        if(address == 0x00FB)
        {
            static uint8_t trampolineOpcode[]  = {0xFE, 0xFC, 0x14, 0xE0, 0xC2};
            static uint8_t trampolineOperand[] = {0x00, 0xFD, 0x04, 0x65, 0x18};

            for(int j=0; j<sizeof(trampolineOpcode); j++)
            {
                outfile0.write((char *)&trampolineOpcode[j], 1);
                if(outfile0.bad() || outfile0.fail())
                {
                    fprintf(stderr, "gt1torom : write error at address %04x in file %s.\n", startAddress + i, outputFilename0.c_str());
                    return 1;
                }

                outfile1.write((char *)&trampolineOperand[j], 1);
                if(outfile1.bad() || outfile1.fail())
                {
                    fprintf(stderr, "gt1torom : write error at address %04x in file %s.\n", startAddress + i, outputFilename1.c_str());
                    return 1;
                }
            }

            trampolineOffset += 5;
        }
        
        uint8_t nativeLoad = 0x00;
        outfile0.write((char *)&nativeLoad, 1);
        if(outfile0.bad() || outfile0.fail())
        {
            fprintf(stderr, "gt1torom : write error at address %04x in file %s.\n", startAddress + i, outputFilename0.c_str());
            return 1;
        }

        outfile1.write((char *)&_gt1[i], 1);
        if(outfile1.bad() || outfile1.fail())
        {
            fprintf(stderr, "gt1torom : write error at address %04x in file %s.\n", startAddress + i, outputFilename1.c_str());
            return 1;
        }
    }

    fprintf(stderr, "%s success.\n", GT1TOROM_VERSION_STR);

    return 0;
}
