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
    paddedName = (name.size() > 15) ? name.substr(0, 15) + "Midi" : name + "Midi";
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
    paddedName = name;
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << std::endl << "] " << paddedName.c_str() << "Midi" << "=" << std::endl;
}

// CPP output
void outputCPPheader(std::ofstream& outfile, const std::string& name, int& charCount)
{
    paddedName = name;
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << "uint8_t " << paddedName.c_str() << "Midi[] =" << std::endl;
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
    paddedName = name;
    std::replace(paddedName.begin(), paddedName.end(), '-', '_');
    outfile << paddedName.c_str() << "Midi = bytearray([" << std::endl;
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


void main(int argc, char* argv[])
{
    if(argc != 6)
    {
        fprintf(stderr, "Usage:   gigamidi <input filename> <output filename> <format 0, 1, 2 or 3> <address> <line length>\n");
        fprintf(stderr, "Example: gigamidi game_over.bin game_over.i 0 0x8000 100\n");
        fprintf(stderr, "Input:   miditones binary file produced with miditones, e.g. miditones -t4 -b <filename>.bin\n");
        fprintf(stderr, "Format:  0 = vCPU ASM, 1 = GCL, 2 = C/C++, 3 = Python\n");
        exit(0);
    }

    std::string inFileName = std::string(argv[1]);
    std::string outFileName = std::string(argv[2]);

    int format = std::stoi(std::string(argv[3]));
    if(format < Format::vCPU  || format >= Format::NumFormats)
    {
        fprintf(stderr, "Format must be 0, 1, 2 or 3\n");
        exit(0);
    }

    // Handles hex numbers
    uint16_t address;   
    std::stringstream ss;
    ss << std::hex << argv[4];
    ss >> address;

    int lineLength = std::stoi(std::string(argv[5]));

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

    size_t i = inFileName.rfind("/");
    if(i == std::string::npos) i = inFileName.rfind("\\");
    if(i == std::string::npos) i = 0;
    size_t j = inFileName.rfind(".");
    if(j == std::string::npos) j = inFileName.length();
    std::string midiName = inFileName.substr(i+1, j - (i+1));

    size_t midiSize = infile.gcount();
    uint8_t* _midiPtr = midiBuffer;

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
        uint8_t command = *_midiPtr++; midiSize--;
        if(command & 0x80)
        {
            // Start note
            if((command & 0xF0) == 0x90)
            {
                uint8_t note = *_midiPtr++; midiSize--;
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
                switch(format)
                {
                    case Format::vCPU: outputvCPUcommand(outfile, command, charCount); break;
                    case Format::GCL:  outputGCLcommand(outfile, command, charCount);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command, charCount);  break;
                    case Format::PY:   outputPYcommand(outfile, command, charCount);   break;
                }
            }
            // Stop midi
            else if((command & 0xF0) == 0xF0)
            {
                switch(format)
                {
                    case Format::vCPU: outputvCPUcommand(outfile, command, charCount); break;
                    case Format::GCL:  outputGCLcommand(outfile, command, charCount);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command, charCount);  break;
                    case Format::PY:   outputPYcommand(outfile, command, charCount);   break;
                }
            }
            // Restart midi
            else if((command & 0xF0) == 0xE0)
            {
                switch(format)
                {
                    case Format::vCPU: outputvCPUcommand(outfile, command, charCount); break;
                    case Format::GCL:  outputGCLcommand(outfile, command, charCount);  break;
                    case Format::CPP:  outputCPPcommand(outfile, command, charCount);  break;
                    case Format::PY:   outputPYcommand(outfile, command, charCount);   break;
                }
            }
        }
        // Delay n milliseconds where n = 16bit value, converted to 8bit, (0x00 <-> 0x7F)
        else
        {
            uint16_t delay16 = ((command<<8) | *_midiPtr++); midiSize--;
            uint8_t delay8 = uint8_t(floor(double(delay16) / 16.6666666667 + 0.5));  // maximum delay is 0x7F * 16.6666666667, (2116ms)
            if(delay16 > 2116) 
            {
                delay8 = 0x7F;
                fprintf(stderr, "Warning, 16 bit delay is larger that 2116ms, this conversion could have serious timing issues! : wanted %dms : received %dms\n", delay16, int(delay8*16.6666666667));
            }

            totalTime16 += double(delay16);
            totalTime8 += double(delay8) * 16.6666666667;

            switch(format)
            {
                case Format::vCPU: outputvCPUcommand(outfile, delay8, charCount); break;
                case Format::GCL:  outputGCLcommand(outfile, delay8, charCount);  break;
                case Format::CPP:  outputCPPcommand(outfile, delay8, charCount);  break;
                case Format::PY:   outputPYcommand(outfile, delay8, charCount);   break;
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

    fprintf(stderr, "Total time of MIDI score : wanted %.1lfms : received %.1lfms : MIDI playback will be off by %.1lfms\n", totalTime16, totalTime8, totalTime8 - totalTime16);
}