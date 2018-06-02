#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "..\loader.h"
#include "..\assembler.h"
#include "..\expression.h"


#define MAJOR_VERSION "0.1"
#define MINOR_VERSION "1"
#define VERSION_STR "gtasm v" MAJOR_VERSION "." MINOR_VERSION


void main(int argc, char* argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s\n", VERSION_STR);
        fprintf(stderr, "Usage:   gtasm <input filename> <uint16_t start address in hex>\n");
        exit(0);
    }

    std::string filename = std::string(argv[1]);

    uint16_t address, customAddress, executeAddress = DEFAULT_START_ADDRESS;

    // Handles hex numbers
    std::stringstream ss;
    ss << std::hex << argv[2];
    ss >> address;
    if(address < DEFAULT_START_ADDRESS) address = DEFAULT_START_ADDRESS;

    executeAddress = address;
    customAddress = executeAddress;

    // Upload vCPU assembly code
    if(filename.find(".vasm") == filename.npos  &&  filename.find(".s") == filename.npos  &&  filename.find(".asm") == filename.npos)
    {
        fprintf(stderr, "Wrong file extension in %s : must be one of : '.vasm' or '.s' or '.asm'\n", filename.c_str());
        exit(0);
    }

    Assembler::initialise();
    Expression::initialise();

    if(!Assembler::assemble(filename, executeAddress)) exit(0);

    // Save to gt1 format
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
            customAddress = address;
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
    if(!hasRomCode  &&  !saveGt1File(filename, gt1File)) exit(0);

    Loader::printGt1Stats(filename, gt1File);
}