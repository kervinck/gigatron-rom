#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>

#include "../../cpu.h"
#include "../../memory.h"
#include "../../expression.h"


// 64K is maximum size, (good luck getting it into a Gigatron)
#define MAX_MIDI_BUFFER_SIZE  0x10000
#define MAX_SOUND_CHANNELS    4
#define MASK_SOUND_CHANNELS   0x03
#define MIN_GIGA_NOTE         12
#define MAX_GIGA_NOTE         106
#define PERCUSSION_NOTES      128
#define MAX_ELEMENTS          16 // has to be divisible into 256, otherwise free memory list falls over

#define GTMIDI_MAJOR_VERSION "0.4"
#define GTMIDI_MINOR_VERSION "1"
#define GTMIDI_VERSION_STR "gtmidi v" GTMIDI_MAJOR_VERSION "." GTMIDI_MINOR_VERSION


enum Format {vCPU=0, GBAS, GCL, CPP, PY, NumFormats};

int _elementCount = 0;
std::string _segmentName;

std::map<std::string, int> _formatName = 
{
    {"VCPU", vCPU},
    {"GBAS", GBAS},
    {"GCL" , GCL },
    {"CPP" , CPP },
    {"PY"  , PY  },
};

uint8_t _midiBuffer[MAX_MIDI_BUFFER_SIZE];


void padString(std::string &str, size_t num, char pad=' ')
{
    if(num > str.size()) str.insert(0, num - str.size(), pad);
}
void addString(std::string &str, size_t num, char add=' ')
{
    str.append(num, add);
}


// vCPU output
void outputVCPUheader(std::ofstream& outfile, const std::string& name, uint16_t address, uint16_t segmentSize, uint16_t segmentIndex)
{
    _segmentName = name;
    if(segmentSize)
    {
        std::stringstream ss;
        ss << _segmentName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
        _segmentName = ss.str();
    }
    addString(_segmentName, 16 - _segmentName.size());
    outfile << _segmentName.c_str() << "EQU     0x" << std::hex << std::setw(4) << std::setfill('0') << address << std::endl;
    outfile << _segmentName.c_str() << "DB     ";
};
void outputVCPUnewLine(std::ofstream& outfile)
{
    if(++_elementCount < MAX_ELEMENTS) return;
    _elementCount = 0;

    std::string str;
    addString(str, _segmentName.size());
    outfile << std::endl << str << "DB     ";
}
void outputVCPUcommand(std::ofstream& outfile, uint8_t command)
{
    outfile << " 0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command);
    outputVCPUnewLine(outfile);
}

// GBAS output
void outputGBASheader(std::ofstream& outfile, uint16_t address)
{
    outfile << "def byte" << "(&h" << std::hex << std::setw(4) << std::setfill('0') << address << ") = ";
};
void outputGBASnewLine(std::ofstream& outfile)
{
    if(++_elementCount < MAX_ELEMENTS) return;
    _elementCount = 0;

    std::string str;
    outfile << std::endl << str << "def byte         = ";
}
void outputGBAScommand(std::ofstream& outfile, uint8_t command)
{
    outfile << " &h" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",";
    outputGBASnewLine(outfile);
}

// GCL output
void outputGCLheader(std::ofstream& outfile, uint16_t address)
{
    outfile << "$" << std::hex << std::setw(4) << std::setfill('0') << address << ":" << std::endl;
    outfile << "[def" << std::endl << " ";
};
void outputGCLnewLine(std::ofstream& outfile)
{
    if(++_elementCount < MAX_ELEMENTS) return;
    _elementCount = 0;

    outfile << std::endl << " ";
}
void outputGCLcommand(std::ofstream& outfile, uint8_t command)
{
    outfile << " $" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << "#";
    outputGCLnewLine(outfile);
}
void outputGCLfooter(std::ofstream& outfile, const std::string& name)
{
    UNREFERENCED_PARAM(name);
    outfile << std::endl << "]" << std::endl;
}

// CPP output
void outputCPPheader(std::ofstream& outfile, const std::string& name, uint16_t segmentSize, uint16_t segmentIndex)
{
    _segmentName = name;
    if(segmentSize)
    {
        std::stringstream ss;
        ss << _segmentName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
        _segmentName = ss.str() + "[] = ";
    }
    outfile << "uint8_t " << _segmentName.c_str() << std::endl;
    outfile << "{" << std::endl;
    outfile << "    ";
};
void outputCPPnewLine(std::ofstream& outfile)
{
    if(++_elementCount < MAX_ELEMENTS) return;
    _elementCount = 0;

    outfile << std::endl << "    ";
}
void outputCPPcommand(std::ofstream& outfile, uint8_t command)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",";
    outputCPPnewLine(outfile);
}
void outputCPPfooter(std::ofstream& outfile)
{
    outfile << std::endl << "};" << std::endl;
}

// PY output
void outputPYheader(std::ofstream& outfile, const std::string& name, uint16_t segmentSize, uint16_t segmentIndex)
{
    _segmentName = name;
    if(segmentSize)
    {
        std::stringstream ss;
        ss << _segmentName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
        _segmentName = ss.str() + " = bytearray([";
    }
    outfile << _segmentName.c_str() << std::endl;
    outfile << "    ";
};
void outputPYnewLine(std::ofstream& outfile)
{
    if(++_elementCount < MAX_ELEMENTS) return;
    _elementCount = 0;

    outfile << std::endl << "    ";
}
void outputPYcommand(std::ofstream& outfile, uint8_t command)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",";
    outputPYnewLine(outfile);
}
void outputPYfooter(std::ofstream& outfile)
{
    outfile << std::endl << "])" << std::endl;
}


void outputDelay(std::ofstream& outfile, Format format, uint8_t delay8, double timingAdjust, double totalTime16, double& totalTime8)
{
    // Adjust delay8 to try and keep overall timing as accurate as possible
    if(timingAdjust)
    {
        if(totalTime16 > totalTime8 + double(delay8)*16.6666666667 + 16.6666666667*timingAdjust  &&  delay8 < 0x7F)  delay8++;
        if(totalTime16 < totalTime8 + double(delay8)*16.6666666667 - 16.6666666667*timingAdjust  &&  delay8 > 0x01)  delay8--;
    }

    totalTime8 += double(delay8) * 16.6666666667;

    switch(format)
    {
        case Format::vCPU: outputVCPUcommand(outfile, delay8); break;
        case Format::GBAS: outputGBAScommand(outfile, delay8); break;
        case Format::GCL:  outputGCLcommand(outfile, delay8);  break;
        case Format::CPP:  outputCPPcommand(outfile, delay8);  break;
        case Format::PY:   outputPYcommand(outfile, delay8);   break;

        default: break;
    }
}

int main(int argc, char* argv[])
{
    if(argc != 9)
    {
        fprintf(stderr, "%s\n", GTMIDI_VERSION_STR);
        fprintf(stderr, "Usage:   gtmidi <input filename> <output filename> <midi name> <format name> <uint16_t start_address in hex>\n         <uint16_t segment_offset in hex> <int segment_size> <float timing_adjust>\n");
        fprintf(stderr, "Example: gtmidi game_over.bin game_over.i gameOver vCPU 0x8000 0 0 0.5\n");
        fprintf(stderr, "Input:   miditones binary file produced with miditones, e.g. miditones -t4 -b -s1 -pi <filename>.bin\n");
        fprintf(stderr, "Format:  'vCPU', 'GBAS', 'GCL', 'CPP', 'Py'\n");
        return 1;
    }

    std::string inFilename = std::string(argv[1]);
    std::string outFilename = std::string(argv[2]);
    std::string midiName = std::string(argv[3]);

    std::string formatName = argv[4];
    formatName = Expression::strToUpper(formatName);
    if(_formatName.find(formatName) == _formatName.end())
    {
        fprintf(stderr, "Format must be one of 'vCPU', 'GBAS', 'GCL', 'CPP', 'Py'\n");
        return 1;
    }

    Format format = (Format)_formatName[formatName];

    // Handles hex numbers
    uint16_t startAddress, segmentOffset;
    std::stringstream ss0, ss1;
    ss0 << std::hex << argv[5];
    ss0 >> startAddress;
    ss1 << std::hex << argv[6];
    ss1 >> segmentOffset;

    uint16_t segmentSize = uint16_t(strtol(argv[7], nullptr, 10));
    double timingAdjust = strtod(argv[8], nullptr);

    std::ifstream infile(inFilename, std::ios::binary | std::ios::in);
    if(!infile.is_open())
    {
        fprintf(stderr, "Failed to open input file '%s'\n", inFilename.c_str());
        return 1;
    }

    std::ofstream outfile(outFilename, std::ios::binary | std::ios::out);
    if(!outfile.is_open())
    {
        fprintf(stderr, "Failed to open output file '%s'\n", outFilename.c_str());
        return 1;
    }

    infile.read((char *)&_midiBuffer, MAX_MIDI_BUFFER_SIZE);
    if(infile.bad())
    {
        fprintf(stderr, "Failed to read input file '%s'\n", inFilename.c_str());
        return 1;
    }

    std::streamsize midiSize = infile.gcount();
    uint8_t* midiPtr = _midiBuffer;
    uint16_t gigaSize = 0;

    double totalTime16 = 0;
    double totalTime8 = 0;

    uint16_t segmentIndex = 0;

    // Header
    switch(format)
    {
        case Format::vCPU: outputVCPUheader(outfile, midiName, startAddress, segmentSize, segmentIndex); break;
        case Format::GBAS: outputGBASheader(outfile, startAddress);                                      break;
        case Format::GCL:  outputGCLheader(outfile, startAddress);                                       break;
        case Format::CPP:  outputCPPheader(outfile, midiName, segmentSize, segmentIndex);                break;
        case Format::PY:   outputPYheader(outfile, midiName, segmentSize, segmentIndex);                 break;

        default: break;
    }
    
    // Commands
    while(midiSize)
    {
        uint8_t command = *midiPtr++; midiSize--;
        if(command & 0x80)
        {
            // Start note
            if((command & 0xF0) == 0x90)
            {
                uint8_t note = *midiPtr++; midiSize--;
                if(note >= PERCUSSION_NOTES) note -= PERCUSSION_NOTES;
                if(note < MIN_GIGA_NOTE) note = MIN_GIGA_NOTE;
                if(note > MAX_GIGA_NOTE) note = MAX_GIGA_NOTE;
                gigaSize += 2;
                switch(format)
                {
                    case Format::vCPU: outputVCPUcommand(outfile, command); outputVCPUcommand(outfile, note); break;
                    case Format::GBAS: outputGBAScommand(outfile, command); outputGBAScommand(outfile, note); break;
                    case Format::GCL:  outputGCLcommand(outfile, command);  outputGCLcommand(outfile, note);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command);  outputCPPcommand(outfile, note);  break;
                    case Format::PY:   outputPYcommand(outfile, command);   outputPYcommand(outfile, note);   break;

                    default: break;
                }
            }
            // Stop note
            else if((command & 0xF0) == 0x80)
            {
                gigaSize += 1;
                switch(format)
                {
                    case Format::vCPU: outputVCPUcommand(outfile, command); break;
                    case Format::GBAS: outputGBAScommand(outfile, command); break;
                    case Format::GCL:  outputGCLcommand(outfile, command);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command);  break;
                    case Format::PY:   outputPYcommand(outfile, command);   break;

                    default: break;
                }
            }
            // Stop midi events are ignored
            else if((command & 0xF0) == 0xF0)
            {
            }
            // Restart midi events are ignored
            else if((command & 0xF0) == 0xE0)
            {
            }
        }
        // Delay n milliseconds where n = 16bit value, converted to multiple, (if necessary), 7bit values, (0x00 <-> 0x7F)
        else
        {
            // Coalesce sequence of delays together
            int index = 0;
            uint16_t coalescedDelay = ((command<<8) | *midiPtr++); midiSize--;
            while(midiSize)
            {
                if(midiPtr[index] & 0x80) break;
                coalescedDelay += (midiPtr[index]<<8) + midiPtr[index + 1];
                index += 2;
                midiSize -= 2;
            }
            midiPtr += index;

            // Break up coalesced delay into bytes
            if(coalescedDelay)
            {
                totalTime16 += double(coalescedDelay);
                coalescedDelay = uint16_t(double(coalescedDelay)/16.6666666667 + 0.5);

                // Ignore zero delays
                if(coalescedDelay)
                {
                    uint8_t div = uint8_t(coalescedDelay / 0x7F);
                    uint8_t rem = uint8_t(coalescedDelay % 0x7F);
                    gigaSize += div + 1;

                    for(uint8_t i=0; i<div; i++)
                    {
                        outputDelay(outfile, format, 0x7f, timingAdjust, totalTime16, totalTime8);
                    }
                    outputDelay(outfile, format, rem, timingAdjust, totalTime16, totalTime8);
                }
            }
        }
        
        // Segmented stream
        if(midiSize  &&  segmentSize  &&  segmentOffset)
        {
            // If segment size is approaching count, (leave room for segment command 0xD0 and 1 extra byte for Note On command)
            if(gigaSize / (segmentSize-4) > segmentIndex)
            {
                gigaSize += 3; 
                segmentIndex++;

                static uint16_t segment = startAddress;
                segment += segmentOffset;

                // Commands
                switch(format)
                {
                    case Format::vCPU: outputVCPUcommand(outfile, 0xD0); outputVCPUcommand(outfile, LO_BYTE(segment)); outputVCPUcommand(outfile, HI_BYTE(segment)); break;
                    case Format::GBAS: outputGBAScommand(outfile, 0xD0); outputGBAScommand(outfile, LO_BYTE(segment)); outputGBAScommand(outfile, HI_BYTE(segment)); break;
                    case Format::GCL:  outputGCLcommand(outfile, 0xD0);  outputGCLcommand(outfile, LO_BYTE(segment));  outputGCLcommand(outfile, HI_BYTE(segment));  break;
                    case Format::CPP:  outputCPPcommand(outfile, 0xD0);  outputCPPcommand(outfile, LO_BYTE(segment));  outputCPPcommand(outfile, HI_BYTE(segment));  break;
                    case Format::PY:   outputPYcommand(outfile, 0xD0);   outputPYcommand(outfile, LO_BYTE(segment));   outputPYcommand(outfile, HI_BYTE(segment));   break;

                    default: break;
                }
                // Old segment footer
                switch(format)
                {
                    case Format::GCL: outputGCLfooter(outfile, midiName); break;
                    case Format::CPP: outputCPPfooter(outfile);           break;
                    case Format::PY:  outputPYfooter(outfile);            break;

                    default: break;
                }
                outfile << std::endl << std::endl;
                // New segment header
                switch(format)
                {
                    case Format::vCPU: outputVCPUheader(outfile, midiName, segment, segmentSize, segmentIndex); break;
                    case Format::GBAS: outputGBASheader(outfile, segment);                                      break;
                    case Format::GCL:  outputGCLheader(outfile, segment);                                       break;
                    case Format::CPP:  outputCPPheader(outfile, midiName, segmentSize, segmentIndex);           break;
                    case Format::PY:   outputPYheader(outfile, midiName, segmentSize, segmentIndex);            break;

                    default: break;
                }
            }
        }

        // Last segment points back to address, (can be user edited in output source file to point to a different MIDI stream)                
        if(midiSize == 0)
        {
            switch(format)
            {
                case Format::vCPU: outputVCPUcommand(outfile, 0xD0); outputVCPUcommand(outfile, LO_BYTE(startAddress)); outputVCPUcommand(outfile, HI_BYTE(startAddress)); break;
                case Format::GBAS: outputGBAScommand(outfile, 0xD0); outputGBAScommand(outfile, LO_BYTE(startAddress)); outputGBAScommand(outfile, HI_BYTE(startAddress)); break;
                case Format::GCL:  outputGCLcommand(outfile, 0xD0);  outputGCLcommand(outfile, LO_BYTE(startAddress));  outputGCLcommand(outfile, HI_BYTE(startAddress));  break;
                case Format::CPP:  outputCPPcommand(outfile, 0xD0);  outputCPPcommand(outfile, LO_BYTE(startAddress));  outputCPPcommand(outfile, HI_BYTE(startAddress));  break;
                case Format::PY:   outputPYcommand(outfile, 0xD0);   outputPYcommand(outfile, LO_BYTE(startAddress));   outputPYcommand(outfile, HI_BYTE(startAddress));   break;

                default: break;
            }
        }
    }

    // Footer
    switch(format)
    {
        case Format::GCL: outputGCLfooter(outfile, midiName); break;
        case Format::CPP: outputCPPfooter(outfile);           break;
        case Format::PY:  outputPYfooter(outfile);            break;

        default: break;
    }

    fprintf(stderr, "Original size:%d  New size:%d  Original time:%.1lfms  New time:%.1lfms  Error:%.1lfms  Start Address:0x%04x  End Address:0x%04x\n",
                    int(infile.gcount()), gigaSize, totalTime16, totalTime8, totalTime8 - totalTime16, startAddress, startAddress+gigaSize);
    return 0;
}
