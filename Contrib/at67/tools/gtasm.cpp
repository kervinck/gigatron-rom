#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "..\loader.h"
#include "..\assembler.h"
#include "..\expression.h"


#define GTASM_MAJOR_VERSION "0.1"
#define GTASM_MINOR_VERSION "2"
#define GTASM_VERSION_STR "gtasm v" GTASM_MAJOR_VERSION "." GTASM_MINOR_VERSION


void main(int argc, char* argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s\n", GTASM_VERSION_STR);
        fprintf(stderr, "Usage:   gtasm <input filename> <uint16_t start address in hex>\n");
        exit(0);
    }

    std::string filename = std::string(argv[1]);
    if(filename.find(".vasm") == filename.npos  &&  filename.find(".s") == filename.npos  &&  filename.find(".asm") == filename.npos)
    {
        fprintf(stderr, "Wrong file extension in %s : must be one of : '.vasm' or '.s' or '.asm'\n", filename.c_str());
        exit(0);
    }

    // Handles hex numbers
    uint16_t address = DEFAULT_START_ADDRESS;
    std::stringstream ss;
    ss << std::hex << argv[2];
    ss >> address;
    if(address < DEFAULT_START_ADDRESS) address = DEFAULT_START_ADDRESS;

    Assembler::initialise();
    Expression::initialise();

    size_t last_dir_sep = filename.find_last_of("/\\");
    if (last_dir_sep != std::string::npos)
        Assembler::setIncludePath(filename.substr(0, last_dir_sep+1));

    if(!Assembler::assemble(filename, address)) exit(0);

    // Create gt1 format
    Loader::Gt1File gt1File;
    gt1File._loStart = address & 0x00FF;
    gt1File._hiStart = (address & 0xFF00) >>8;
    Loader::Gt1Segment gt1Segment;
    gt1Segment._loAddress = address & 0x00FF;
    gt1Segment._hiAddress = (address & 0xFF00) >>8;

    bool hasRomCode = false;
    Assembler::ByteCode byteCode;
    while(!Assembler::getNextAssembledByte(byteCode))
    {
        if(byteCode._isRomAddress) hasRomCode = true; 

        // Custom address
        if(byteCode._isCustomAddress)
        {
            if(gt1Segment._dataBytes.size())
            {
                // Previous segment
                gt1Segment._segmentSize = uint8_t(gt1Segment._dataBytes.size());
                gt1File._segments.push_back(gt1Segment);
                gt1Segment._dataBytes.clear();
            }

            address = byteCode._address;
            gt1Segment._isRomAddress = byteCode._isRomAddress;
            gt1Segment._loAddress = address & 0x00FF;
            gt1Segment._hiAddress = (address & 0xFF00) >>8;
        }

        gt1Segment._dataBytes.push_back(byteCode._data);
    }

    // Last segment
    if(gt1Segment._dataBytes.size())
    {
        gt1Segment._segmentSize = uint8_t(gt1Segment._dataBytes.size());
        gt1File._segments.push_back(gt1Segment);
    }

    // Don't save gt1 file for any asm files that contain native rom code
    std::string gt1FileName;
    if(!hasRomCode  &&  !saveGt1File(filename, gt1File, gt1FileName)) exit(0);

    Loader::printGt1Stats(gt1FileName, gt1File);
}