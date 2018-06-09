#include <stdio.h>
#include <stdlib.h>
#include <fstream>


#define GSPLITROM_MAJOR_VERSION "0.1"
#define GSPLITROM_MINOR_VERSION "2"
#define GSPLITROM_VERSION_STR "gtsplitrom v" GSPLITROM_MAJOR_VERSION "." GSPLITROM_MINOR_VERSION

#define ROM_SIZE (1<<16)


uint8_t _ROM[ROM_SIZE][2];


void setScanlineModeVideoB(void)
{
    _ROM[0x01C2][0] = 0x14;
    _ROM[0x01C2][1] = 0x01;

    _ROM[0x01C9][0] = 0x01;
    _ROM[0x01C9][1] = 0x09;

    _ROM[0x01CA][0] = 0x90;
    _ROM[0x01CA][1] = 0x01;

    _ROM[0x01CB][0] = 0x01;
    _ROM[0x01CB][1] = 0x0A;

    _ROM[0x01CC][0] = 0x8D;
    _ROM[0x01CC][1] = 0x00;

    _ROM[0x01CD][0] = 0xC2;
    _ROM[0x01CD][1] = 0x0A;

    _ROM[0x01CE][0] = 0x00;
    _ROM[0x01CE][1] = 0xD4;

    _ROM[0x01CF][0] = 0xFC;
    _ROM[0x01CF][1] = 0xFD;

    _ROM[0x01D0][0] = 0xC2;
    _ROM[0x01D0][1] = 0x0C;

    _ROM[0x01D1][0] = 0x02;
    _ROM[0x01D1][1] = 0x00;

    _ROM[0x01D2][0] = 0x02;
    _ROM[0x01D2][1] = 0x00;

    _ROM[0x01D3][0] = 0x02;
    _ROM[0x01D3][1] = 0x00;
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s\n", GSPLITROM_VERSION_STR);
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
    romfile.read((char *)_ROM, sizeof(_ROM));
    if(romfile.bad() || romfile.fail())
    {
        fprintf(stderr, "gtsplitrom : failed to read %s ROM file.\n", inputFilename.c_str());
        return 1;
    }

    // ROM file is not big enough
    if(romfile.gcount() < sizeof(_ROM))
    {
        fprintf(stderr, "gtsplitrom : ROM file %s is not large enough : size is %d : required size is %d.\n", inputFilename.c_str(), int(romfile.gcount()), int(sizeof(_ROM)));
        return 1;
    }

    // Modifies ROM to disable scanline VideoB
    setScanlineModeVideoB();

    std::string outputFilename0 = std::string(argv[2]) + "0";
    std::ofstream outfile0(outputFilename0, std::ios::binary | std::ios::out);
    if(!outfile0.is_open())
    {
        fprintf(stderr, "gtsplitrom : failed to open '%s'.\n", outputFilename0.c_str());
        return 1;
    }

    std::string outputFilename1 = std::string(argv[2]) + "1";
    std::ofstream outfile1(outputFilename1, std::ios::binary | std::ios::out);
    if(!outfile1.is_open())
    {
        fprintf(stderr, "gtsplitrom : failed to open '%s'.\n", outputFilename1.c_str());
        return 1;
    }

    for(int i=0; i<ROM_SIZE; i++)
    {
        outfile0.write((char *)&_ROM[i][0], 1);
        if(outfile0.bad() || outfile0.fail())
        {
            fprintf(stderr, "gtsplitrom : write error at address %04x in file %s.\n", i, outputFilename0.c_str());
            return 1;
        }

        outfile1.write((char *)&_ROM[i][1], 1);
        if(outfile1.bad() || outfile1.fail())
        {
            fprintf(stderr, "gtsplitrom : write error at address %04x in file %s.\n", i, outputFilename1.c_str());
            return 1;
        }
    }

    fprintf(stderr, "%s success.\n", GSPLITROM_VERSION_STR);

    return 0;
}
