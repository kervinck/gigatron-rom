#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>


// 64K is maximum size, (good luck getting it into a Gigatron)
#define MAX_MIDI_BUFFER_SIZE  0x10000
#define MAX_SOUND_CHANNELS    4
#define MASK_SOUND_CHANNELS   0x03


enum Format {vCPU=0, GCL, CPP, PY, NumFormats};


int segmentIndex = 0;
std::string paddedName;
uint8_t midiBuffer[MAX_MIDI_BUFFER_SIZE];


void padString(std::string &str, size_t num, char pad=' ')
{
    if(num > str.size()) str.insert(0, num - str.size(), pad);
}
void addString(std::string &str, size_t num, char add=' ')
{
    str.append(num, add);
}

// vCPU output
void outputvCPUheader(std::ofstream& outfile, const std::string& name, uint16_t address, int& charCount)
{
    std::stringstream ss;
    paddedName = (name.size() > 13) ? name.substr(0, 13) + "Midi" : name + "Midi";
    ss << paddedName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
    paddedName = ss.str();
    addString(paddedName, 20 - paddedName.size());
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << paddedName.c_str() << "EQU     0x" << std::hex << std::setw(4) << std::setfill('0') << address << std::endl;
    outfile << paddedName.c_str() << "DB     ";
    charCount = 30;
};
void outputvCPUstartNote(std::ofstream& outfile, uint8_t command, uint8_t note, int& charCount)
{
    outfile << " 0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << " 0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(note);
    charCount += 10;
}
void outputvCPUcommand(std::ofstream& outfile, uint8_t command, int& charCount)
{
    outfile << " 0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command);
    charCount += 5;
}
void outputvCPUnewLine(std::ofstream& outfile, int& charCount)
{
    std::string str;
    addString(str, paddedName.size());
    outfile << std::endl << str << "DB     ";
    charCount = 30;
}

// GCL output
void outputGCLheader(std::ofstream& outfile, uint16_t address, int& charCount)
{
    outfile << "$" << std::hex << std::setw(4) << std::setfill('0') << address << ":" << std::endl;
    outfile << "[def" << std::endl << " ";
    charCount = 1;
};
void outputGCLstartNote(std::ofstream& outfile, uint8_t command, uint8_t note, int& charCount)
{
    outfile << " $" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << "#" << " $" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(note) << "#";
    charCount += 10;
}
void outputGCLcommand(std::ofstream& outfile, uint8_t command, int& charCount)
{
    outfile << " $" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << "#";
    charCount += 5;
}
void outputGCLnewLine(std::ofstream& outfile, int& charCount)
{
    outfile << std::endl << " ";
    charCount = 1;
}
void outputGCLfooter(std::ofstream& outfile, const std::string& name)
{
    
#if 0
    // Waste of page 0 variable space if you activate this
    std::stringstream ss;
    paddedName = name + "Midi";
    ss << paddedName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex-1);
    paddedName = ss.str();
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << std::endl << "] " << paddedName.c_str() << "=" << std::endl;
#else
    outfile << std::endl << "]" << std::endl;
#endif
}

// CPP output
void outputCPPheader(std::ofstream& outfile, const std::string& name, int& charCount)
{
    std::stringstream ss;
    paddedName = name + "Midi";
    ss << paddedName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
    paddedName = ss.str() + "[] = ";
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << "uint8_t " << paddedName.c_str() << std::endl;
    outfile << "{" << std::endl;
    outfile << "    ";
    charCount = 4;
};
void outputCPPstartNote(std::ofstream& outfile, uint8_t command, uint8_t note, int& charCount)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(note) << ",";
    charCount += 10;
}
void outputCPPcommand(std::ofstream& outfile, uint8_t command, int& charCount)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",";
    charCount += 5;
}
void outputCPPnewLine(std::ofstream& outfile, int& charCount)
{
    outfile << std::endl << "    ";
    charCount = 4;
}
void outputCPPfooter(std::ofstream& outfile)
{
    outfile << std::endl << "};" << std::endl;
}

// PY output
void outputPYheader(std::ofstream& outfile, const std::string& name, int& charCount)
{
    std::stringstream ss;
    paddedName = name + "Midi";
    ss << paddedName << std::setfill('0') << std::setw(2) << std::to_string(segmentIndex);
    paddedName = ss.str() + " = bytearray([";
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << paddedName.c_str() << std::endl;
    outfile << "    ";
    charCount = 4;
};
void outputPYstartNote(std::ofstream& outfile, uint8_t command, uint8_t note, int& charCount)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(note) << ",";
    charCount += 10;
}
void outputPYcommand(std::ofstream& outfile, uint8_t command, int& charCount)
{
    outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint16_t(command) << ",";
    charCount += 5;
}
void outputPYnewLine(std::ofstream& outfile, int& charCount)
{
    outfile << std::endl << "    ";
    charCount = 4;
}
void outputPYfooter(std::ofstream& outfile)
{
    outfile << std::endl << "])" << std::endl;
}

void outputDelay(std::ofstream& outfile, Format format, uint8_t delay8, double timingAdjust, double totalTime16, double& totalTime8, int& charCount)
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
        case Format::vCPU: outputvCPUcommand(outfile, delay8, charCount); break;
        case Format::GCL:  outputGCLcommand(outfile, delay8, charCount);  break;
        case Format::CPP:  outputCPPcommand(outfile, delay8, charCount);  break;
        case Format::PY:   outputPYcommand(outfile, delay8, charCount);   break;
    }
}

void main(int argc, char* argv[])
{
    if(argc != 9)
    {
        fprintf(stderr, "Usage:   gigamidi <input filename> <output filename> <int format 0, 1, 2 or 3> <uint16_t address in hex> <uint16_t offset in hex> <int count> <int line_length> <float timing_adjust>\n");
        fprintf(stderr, "Example: gigamidi game_over.bin game_over.i 0 0x8000 0 0 100 0.5\n");
        fprintf(stderr, "Input:   miditones binary file produced with miditones, e.g. miditones -t4 -b <filename>.bin\n");
        fprintf(stderr, "Format:  0 = vCPU ASM, 1 = GCL, 2 = C/C++, 3 = Python\n");
        exit(0);
    }

    std::string inFileName = std::string(argv[1]);
    std::string outFileName = std::string(argv[2]);

    Format format = (Format)std::stoi(std::string(argv[3]));
    if(format < Format::vCPU  ||  format >= Format::NumFormats)
    {
        fprintf(stderr, "Format must be 0, 1, 2 or 3\n");
        exit(0);
    }

    // Handles hex numbers
    uint16_t address, offset;
    std::stringstream ss0, ss1;
    ss0 << std::hex << argv[4];
    ss0 >> address;
    ss1 << std::hex << argv[5];
    ss1 >> offset;

    uint16_t count;
    count = std::stoi(argv[6]);
    int lineLength = std::stoi(std::string(argv[7]));
    double timingAdjust = std::stod(std::string(argv[8]));

    std::ifstream infile(inFileName, std::ios::binary | std::ios::in);
    if(!infile.is_open())
    {
        fprintf(stderr, "Failed to open input file '%s'\n", inFileName.c_str());
        exit(0);
    }

    std::ofstream outfile(outFileName, std::ios::binary | std::ios::out);
    if(!outfile.is_open())
    {
        fprintf(stderr, "Failed to open output file '%s'\n", outFileName.c_str());
        exit(0);
    }

    infile.read((char *)&midiBuffer, MAX_MIDI_BUFFER_SIZE);
    if(infile.bad())
    {
        fprintf(stderr, "Failed to read input file '%s'\n", inFileName.c_str());
        exit(0);
    }

    size_t i = outFileName.rfind("/") + 1;
    if(i == std::string::npos) i = outFileName.rfind("\\") + 1;
    if(i == std::string::npos) i = 0;
    size_t j = outFileName.rfind(".");
    if(j == std::string::npos) j = outFileName.length();
    std::string midiName = outFileName.substr(i, j - i);

    size_t midiSize = infile.gcount();
    uint8_t* midiPtr = midiBuffer;
    int gigaSize = 0;

    double totalTime16 = 0;
    double totalTime8 = 0;

    // Header
    int charCount = 0;
    switch(format)
    {
        case Format::vCPU: outputvCPUheader(outfile, midiName, address, charCount); break;
        case Format::GCL:  outputGCLheader(outfile, address, charCount);            break;
        case Format::CPP:  outputCPPheader(outfile, midiName, charCount);           break;
        case Format::PY:   outputPYheader(outfile, midiName, charCount);            break;
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
                if(note < 12)  note = 12;
                if(note > 106) note = 106;
                gigaSize += 2;
                switch(format)
                {
                    case Format::vCPU: outputvCPUstartNote(outfile, command, note, charCount);  break;
                    case Format::GCL:  outputGCLstartNote(outfile, command, note, charCount);   break;
                    case Format::CPP:  outputCPPstartNote(outfile, command, note, charCount);   break;
                    case Format::PY:   outputPYstartNote(outfile, command, note, charCount);    break;
                }
            }
            // Stop note
            else if((command & 0xF0) == 0x80)
            {
                gigaSize += 1;
                switch(format)
                {
                    case Format::vCPU: outputvCPUcommand(outfile, command, charCount); break;
                    case Format::GCL:  outputGCLcommand(outfile, command, charCount);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command, charCount);  break;
                    case Format::PY:   outputPYcommand(outfile, command, charCount);   break;
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
        // Delay n milliseconds where n = 16bit value, converted to 8bit, (0x00 <-> 0x7F)
        else
        {
            // Coalesc sequence of delays together
            int i = 0;
            int coalescedDelay = ((command<<8) | *midiPtr++); midiSize--;
            while(midiSize)
            {
                if(midiPtr[i] & 0x80) break;
                coalescedDelay += (midiPtr[i]<<8) + midiPtr[i+1];
                i += 2;
                midiSize -= 2;
            }
            midiPtr += i;

            // Break up coalesced delay into bytes
            if(coalescedDelay)
            {
                totalTime16 += double(coalescedDelay);
                coalescedDelay = int(double(coalescedDelay)/16.6666666667 + 0.5);

                // Ignore zero delays
                if(coalescedDelay)
                {
                    int div = coalescedDelay / 0x7F;
                    int rem = coalescedDelay % 0x7F;
                    gigaSize += div + 1;

                    for(int i=0; i<div; i++)
                    {
                        outputDelay(outfile, format, 0x7f, timingAdjust, totalTime16, totalTime8, charCount);
                    }
                    outputDelay(outfile, format, rem, timingAdjust, totalTime16, totalTime8, charCount);
                }
            }
        }
        
        // Check count and offset for a segmented stream
        if(midiSize  &&  count  &&  offset)
        {
            // If output byte stream size is approaching count, (leave room for Segment command 0xD0 and 1 extra byte for Note On command 0x90)
            if(gigaSize / (count-4) > segmentIndex)
            {
                gigaSize += 3; 
                segmentIndex++;

                static uint16_t segment = address;
                segment += offset;

                // Commands
                switch(format)
                {
                    case Format::vCPU: outputvCPUcommand(outfile, 0xD0, charCount); outputvCPUcommand(outfile, segment & 0x00FF, charCount); outputvCPUcommand(outfile, (segment & 0xFF00) >>8, charCount); break;
                    case Format::GCL:  outputGCLcommand(outfile, 0xD0, charCount);  outputGCLcommand(outfile, segment & 0x00FF, charCount);  outputGCLcommand(outfile, (segment & 0xFF00) >>8, charCount);  break;
                    case Format::CPP:  outputCPPcommand(outfile, 0xD0, charCount);  outputCPPcommand(outfile, segment & 0x00FF, charCount);  outputCPPcommand(outfile, (segment & 0xFF00) >>8, charCount);  break;
                    case Format::PY:   outputPYcommand(outfile, 0xD0, charCount);   outputPYcommand(outfile, segment & 0x00FF, charCount);   outputPYcommand(outfile, (segment & 0xFF00) >>8, charCount);   break;                
                }
                // Footer
                switch(format)
                {
                    case Format::GCL: outputGCLfooter(outfile, midiName); break;
                    case Format::CPP: outputCPPfooter(outfile);           break;
                    case Format::PY:  outputPYfooter(outfile);            break;
                }
                outfile << std::endl << std::endl;
                // Header
                switch(format)
                {
                    case Format::vCPU: outputvCPUheader(outfile, midiName, segment, charCount); break;
                    case Format::GCL:  outputGCLheader(outfile, segment, charCount);            break;
                    case Format::CPP:  outputCPPheader(outfile, midiName, charCount);           break;
                    case Format::PY:   outputPYheader(outfile, midiName, charCount);            break;
                }
            }
        }

        // Last segment points back to address, (can be user edited in output source file to point to a different MIDI stream)                
        if(midiSize == 0)
        {
            switch(format)
            {
                case Format::vCPU: outputvCPUcommand(outfile, 0xD0, charCount); outputvCPUcommand(outfile, address & 0x00FF, charCount); outputvCPUcommand(outfile, (address & 0xFF00) >>8, charCount); break;
                case Format::GCL:  outputGCLcommand(outfile, 0xD0, charCount);  outputGCLcommand(outfile, address & 0x00FF, charCount);  outputGCLcommand(outfile, (address & 0xFF00) >>8, charCount);  break;
                case Format::CPP:  outputCPPcommand(outfile, 0xD0, charCount);  outputCPPcommand(outfile, address & 0x00FF, charCount);  outputCPPcommand(outfile, (address & 0xFF00) >>8, charCount);  break;
                case Format::PY:   outputPYcommand(outfile, 0xD0, charCount);   outputPYcommand(outfile, address & 0x00FF, charCount);   outputPYcommand(outfile, (address & 0xFF00) >>8, charCount);   break;                
            }
        }

        // Newline
        if(charCount >= lineLength - 10  &&  midiSize)
        {
            switch(format)
            {
                case Format::vCPU: outputvCPUnewLine(outfile, charCount); break;
                case Format::GCL:  outputGCLnewLine(outfile, charCount);  break;
                case Format::CPP:  outputCPPnewLine(outfile, charCount);  break;
                case Format::PY:   outputPYnewLine(outfile, charCount);   break;
            }
        }
    }

    // Footer
    switch(format)
    {
        case Format::GCL: outputGCLfooter(outfile, midiName); break;
        case Format::CPP: outputCPPfooter(outfile);           break;
        case Format::PY:  outputPYfooter(outfile);            break;
    }

    fprintf(stderr, "Original size: %d  New size: %d  Original time: %.1lfms  New time: %.1lfms  Error: %.1lfms\n", int(infile.gcount()), gigaSize, totalTime16, totalTime8, totalTime8 - totalTime16);
}