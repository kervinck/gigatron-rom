#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "memory.h"
#include "cpu.h"

#ifndef STAND_ALONE
#include <SDL.h>
#include "loader.h"
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "gigatron_0x1c.h"
#include "gigatron_0x20.h"
#include "gigatron_0x28.h"
#include "gigatron_0x38.h"
#endif

#ifdef _WIN32
#include <Windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif


namespace Cpu
{
    int _numRoms = 0;
    int _romIndex = 0;
    int _vCpuInstPerFrame = 0;
    int _vCpuInstPerFrameMax = 0;
    float _vCpuUtilisation = 0.0;

    bool _checkRomType = true;

    int64_t _clock = -2;
    uint8_t _IN = 0xFF, _XOUT = 0x00;
    uint8_t _ROM[ROM_SIZE][2], _RAM[RAM_SIZE];
    std::vector<uint8_t*> _romFiles;
    uint16_t _vPC = 0x0200;
    RomType _romType = ROMERR;

    std::vector<uint8_t> _scanlinesRom0;
    std::vector<uint8_t> _scanlinesRom1;
    int _scanlineMode = ScanlineMode::Normal;

    std::vector<InternalGt1> _internalGt1s;


    int getNumRoms(void) {return _numRoms;}
    uint8_t* getPtrToROM(int& romSize) {romSize = sizeof(_ROM); return (uint8_t*)_ROM;}
    RomType getRomType(void) {return _romType;}

    void initialiseInternalGt1s(void)
    {
        InternalGt1 internalGt1Snake = {0xE39C, 0xFDB1, 0xFC89, 5};
        _internalGt1s.push_back(internalGt1Snake);

        InternalGt1 internalGt1Racer = {0xEA2C, 0xFDBB, 0xFC90, 5};
        _internalGt1s.push_back(internalGt1Racer);

        InternalGt1 internalGt1Mandelbrot = {0xF16E, 0xFDC5, 0xFC97, 10};
        _internalGt1s.push_back(internalGt1Mandelbrot);

        InternalGt1 internalGt1Pictures = {0xF655, 0xFDCF, 0xFCA3, 8};
        _internalGt1s.push_back(internalGt1Pictures);

        InternalGt1 internalGt1Credits = {0xF731, 0xFDD9, 0xFCAD, 7};
        _internalGt1s.push_back(internalGt1Credits);

        InternalGt1 internalGt1Loader = {0xF997, 0xFDE3, 0xFCB6, 6};
        _internalGt1s.push_back(internalGt1Loader);
    }

    void patchSYS_Exec_88(void)
    {
        _ROM[0x00AD][ROM_INST] = 0x00;
        _ROM[0x00AD][ROM_DATA] = 0x00;

        _ROM[0x00AF][ROM_INST] = 0x00;
        _ROM[0x00AF][ROM_DATA] = 0x67;

        _ROM[0x00B5][ROM_INST] = 0xDC;
        _ROM[0x00B5][ROM_DATA] = 0xCF;

        _ROM[0x00B6][ROM_INST] = 0x80;
        _ROM[0x00B6][ROM_DATA] = 0x23;

        _ROM[0x00BB][ROM_INST] = 0x80;
        _ROM[0x00BB][ROM_DATA] = 0x00;
    }

    void patchScanlineModeVideoB(void)
    {
        _ROM[0x01C2][ROM_INST] = 0x14;
        _ROM[0x01C2][ROM_DATA] = 0x01;

        _ROM[0x01C9][ROM_INST] = 0x01;
        _ROM[0x01C9][ROM_DATA] = 0x09;

        _ROM[0x01CA][ROM_INST] = 0x90;
        _ROM[0x01CA][ROM_DATA] = 0x01;

        _ROM[0x01CB][ROM_INST] = 0x01;
        _ROM[0x01CB][ROM_DATA] = 0x0A;

        _ROM[0x01CC][ROM_INST] = 0x8D;
        _ROM[0x01CC][ROM_DATA] = 0x00;

        _ROM[0x01CD][ROM_INST] = 0xC2;
        _ROM[0x01CD][ROM_DATA] = 0x0A;

        _ROM[0x01CE][ROM_INST] = 0x00;
        _ROM[0x01CE][ROM_DATA] = 0xD4;

        _ROM[0x01CF][ROM_INST] = 0xFC;
        _ROM[0x01CF][ROM_DATA] = 0xFD;

        _ROM[0x01D0][ROM_INST] = 0xC2;
        _ROM[0x01D0][ROM_DATA] = 0x0C;

        _ROM[0x01D1][ROM_INST] = 0x02;
        _ROM[0x01D1][ROM_DATA] = 0x00;

        _ROM[0x01D2][ROM_INST] = 0x02;
        _ROM[0x01D2][ROM_DATA] = 0x00;

        _ROM[0x01D3][ROM_INST] = 0x02;
        _ROM[0x01D3][ROM_DATA] = 0x00;
    }

    void patchScanlineModeVideoC(void)
    {
        _ROM[0x01DA][ROM_INST] = 0xFC;
        _ROM[0x01DA][ROM_DATA] = 0xFD;

        _ROM[0x01DB][ROM_INST] = 0xC2;
        _ROM[0x01DB][ROM_DATA] = 0x0C;

        _ROM[0x01DC][ROM_INST] = 0x02;
        _ROM[0x01DC][ROM_DATA] = 0x00;

        _ROM[0x01DD][ROM_INST] = 0x02;
        _ROM[0x01DD][ROM_DATA] = 0x00;

        _ROM[0x01DE][ROM_INST] = 0x02;
        _ROM[0x01DE][ROM_DATA] = 0x00;
    }

    void patchTitleIntoRom(const std::string& title)
    {
        int minLength = std::min(int(title.size()), MAX_TITLE_CHARS);
        for(int i=0; i<minLength; i++) _ROM[ROM_TITLE_ADDRESS + i][ROM_DATA] = title[i];
        for(int i=minLength; i<MAX_TITLE_CHARS; i++) _ROM[ROM_TITLE_ADDRESS + i][ROM_DATA] = ' ';
    }

    void patchSplitGt1IntoRom(const std::string& splitGt1path, const std::string& splitGt1name, uint16_t startAddress, InternalGt1Id gt1Id)
    {
        size_t filelength = 0;
        char filebuffer[RAM_SIZE];

        std::ifstream romfile_ti(splitGt1path + "_ti", std::ios::binary | std::ios::in);
        if(!romfile_ti.is_open()) fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to open %s ROM file.\n", std::string(splitGt1path + "_ti").c_str());
        romfile_ti.seekg (0, romfile_ti.end); filelength = romfile_ti.tellg(); romfile_ti.seekg (0, romfile_ti.beg);
        romfile_ti.read(filebuffer, filelength);
        if(romfile_ti.eof() || romfile_ti.bad() || romfile_ti.fail()) fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to read %s ROM file.\n", std::string(splitGt1path + "_ti").c_str());
        for(int i=0; i<filelength; i++) _ROM[startAddress + i][ROM_INST] = filebuffer[i];

        std::ifstream romfile_td(splitGt1path + "_td", std::ios::binary | std::ios::in);
        if(!romfile_td.is_open()) fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to open %s ROM file.\n", std::string(splitGt1path + "_td").c_str());
        romfile_td.seekg (0, romfile_td.end); filelength = romfile_td.tellg(); romfile_td.seekg (0, romfile_td.beg);
        romfile_td.read(filebuffer, filelength);
        if(romfile_td.eof() || romfile_td.bad() || romfile_td.fail()) fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to read %s ROM file.\n", std::string(splitGt1path + "_td").c_str());
        for(int i=0; i<filelength; i++) _ROM[startAddress + i][ROM_DATA] = filebuffer[i];

        // Replace internal gt1 menu option with split gt1
        _ROM[_internalGt1s[gt1Id]._patch + 0][ROM_DATA] = LO_BYTE(startAddress);
        _ROM[_internalGt1s[gt1Id]._patch + 1][ROM_DATA] = HI_BYTE(startAddress);

        // Replace internal gt1 menu option name with split gt1 name
        int minLength = std::min(uint8_t(splitGt1name.size()), _internalGt1s[gt1Id]._length);
        for(int i=0; i<minLength; i++) _ROM[_internalGt1s[gt1Id]._string + i][ROM_DATA] = splitGt1name[i];
        for(int i=minLength; i<_internalGt1s[gt1Id]._length; i++) _ROM[_internalGt1s[gt1Id]._string + i][ROM_DATA] = ' ';
    }


#ifndef STAND_ALONE
    int64_t getClock(void) {return _clock;}
    uint8_t getIN(void) {return _IN;}
    uint8_t getXOUT(void) {return _XOUT;}
    uint16_t getVPC(void) {return _vPC;}
    uint8_t getRAM(uint16_t address) {return _RAM[address & (RAM_SIZE-1)];}
    uint8_t getROM(uint16_t address, int page) {return _ROM[address & (ROM_SIZE-1)][page & 0x01];}
    uint16_t getRAM16(uint16_t address) {return _RAM[address & (RAM_SIZE-1)] | (_RAM[(address+1) & (RAM_SIZE-1)]<<8);}
    uint16_t getROM16(uint16_t address, int page) {return _ROM[address & (ROM_SIZE-1)][page & 0x01] | (_ROM[(address+1) & (ROM_SIZE-1)][page & 0x01]<<8);}
    float getvCpuUtilisation(void) {return _vCpuUtilisation;}


    void setClock(int64_t clock) {_clock = clock;}
    void setIN(uint8_t in) {_IN = in;}
    void setXOUT(uint8_t xout) {_XOUT = xout;}

    void setRAM(uint16_t address, uint8_t data)
    {
        // Constant "0" and "1" are stored here
        if(address == 0x0000) return;
        if(address == 0x0080) return;

        _RAM[address & (RAM_SIZE-1)] = data;
    }

    void setROM(uint16_t base, uint16_t address, uint8_t data)
    {
        uint16_t offset = (address - base) / 2;
        _ROM[base + offset][address & 0x01] = data;
    }

    void setRAM16(uint16_t address, uint16_t data)
    {
        // Constant "0" and "1" are stored here
        if(address == 0x0000) return;
        if(address == 0x0080) return;

        _RAM[address & (RAM_SIZE-1)] = uint8_t(LO_BYTE(data));
        _RAM[(address+1) & (RAM_SIZE-1)] = uint8_t(HI_BYTE(data));
    }

    void setROM16(uint16_t base, uint16_t address, uint16_t data)
    {
        uint16_t offset = (address - base) / 2;
        _ROM[base + offset][address & 0x01] = uint8_t(LO_BYTE(data));
        _ROM[base + offset][(address+1) & 0x01] = uint8_t(HI_BYTE(data));
    }

    void setRomType(void)
    {
        if(!_checkRomType) return;
        _checkRomType = false;

        uint8_t romType = getRAM(ROM_TYPE) & ROM_TYPE_MASK;
        switch((RomType)romType)
        {
            case ROMv1:
            {
                _romType = (RomType)romType;

                // Patches SYS_Exec_88 loader to accept page0 segments as the first segment and works with 64KB SRAM hardware
                patchSYS_Exec_88();

                saveScanlineModes();
                setRAM(VIDEO_MODE_D, 0xF3);
                _scanlineMode = ScanlineMode::Normal;
            }
            break;

            case ROMv2:  
            case ROMv3:  
            case ROMv4:  
            case DEVROM: 
            {
                _romType = (RomType)romType;
                setRAM(VIDEO_MODE_D, 0xEC);
                setRAM(VIDEO_MODE_B, 0x0A);
                setRAM(VIDEO_MODE_C, 0x0A);
            }
            break;

            default:
            {
                Cpu::shutdown();
                fprintf(stderr, "Cpu::setRomType() : Unknown EPROM Type = 0x%02x : exiting...\n", romType);
                _EXIT_(EXIT_FAILURE);
            }
            break;
        }
        //fprintf(stderr, "Cpu::setRomType() : ROM Type = 0x%02x\n", _romType);
    }

    void saveScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            _scanlinesRom0.push_back(_ROM[i][ROM_INST]);
            _scanlinesRom1.push_back(_ROM[i][ROM_DATA]);
        }
    }

    void restoreScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            _ROM[i][ROM_INST] = _scanlinesRom0[i - 0x01C2];
            _ROM[i][ROM_DATA] = _scanlinesRom1[i - 0x01C2];
        }
    }

    void swapScanlineMode(void)
    {
        if(++_scanlineMode == ScanlineMode::NumScanlineModes-1) _scanlineMode = ScanlineMode::Normal;

        switch(_scanlineMode)
        {
            case Normal:  restoreScanlineModes();                               break;
            case VideoB:  patchScanlineModeVideoB();                            break;
            case VideoC:  patchScanlineModeVideoC();                            break;
            case VideoBC: patchScanlineModeVideoB(); patchScanlineModeVideoC(); break;
        }
    }

    void garble(uint8_t* mem, int len)
    {
        for(int i=0; i<len; i++) mem[i] = rand();
    }

    void createRomHeader(const uint8_t* rom, const std::string& filename, const std::string& name, int length)
    {
        std::ofstream outfile(filename);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Graphics::createRomHeader() : failed to create '%s'\n", filename.c_str());
            return;
        }

        outfile << "uint8_t " << name + "[] = \n";
        outfile << "{\n";
        outfile << "    ";

        int colCount = 0;
        uint8_t* data = (uint8_t*)rom;
        for(int i=0; i<length; i++)
        {
            outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << ", ";
            if(++colCount == 12)
            {
                colCount = 0;
                if(i < length-1) outfile << "\n    ";
            }
        }
        
        outfile << "\n};" << std::endl;
    }

    void loadRom(int index)
    {
        _romIndex = index % _numRoms;
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));
        reset(true);
    }

    void swapRom(void)
    {
        _romIndex = (_romIndex + 1) % _numRoms;
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));
        reset(true);
    }


    void shutdown(void)
    {
        for(int i=NUM_INT_ROMS; i<_romFiles.size(); i++)
        {
            if(_romFiles[i])
            {
                delete [] _romFiles[i];
                _romFiles[i] = nullptr;
            }
        }

        SDL_Quit();
    }

    void initialise(State& S)
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if(!AllocConsole()) return;

        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            csbi.dwSize.Y = 1000;
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);
        }

        if(!freopen("CONIN$", "w", stdin)) return;
        if(!freopen("CONOUT$", "w", stderr)) return;
        if (!freopen("CONOUT$", "w", stdout)) return;
        setbuf(stdout, NULL);
#endif    

        // Memory
        srand((unsigned int)time(NULL)); // Initialize with randomized data
        garble((uint8_t*)_ROM, sizeof _ROM);
        garble(_RAM, sizeof _RAM);
        garble((uint8_t*)&S, sizeof S);

        // Internal ROMS
        _romFiles.push_back(_gigatron_0x1c_rom);
        _romFiles.push_back(_gigatron_0x20_rom);
        _romFiles.push_back(_gigatron_0x28_rom);
        _romFiles.push_back(_gigatron_0x38_rom);
        uint8_t types[NUM_INT_ROMS] = {0x1c, 0x20, 0x28, 0x38};
        std::string names[NUM_INT_ROMS] = {"ROMv1.rom", "ROMv2.rom", "ROMv3.rom", "ROMv4.rom"};
        for(int i=0; i<NUM_INT_ROMS; i++) Editor::addRomEntry(types[i], names[i]);

        // External ROMS
        for(int i=0; i<Loader::getConfigRomsSize(); i++)
        {
            uint8_t type = Loader::getConfigRom(i)->_type;
            std::string name = Loader::getConfigRom(i)->_name;

            std::ifstream file(name, std::ios::binary | std::ios::in);
            if(!file.is_open())
            {
                fprintf(stderr, "Cpu::initialise() : failed to open ROM file : %s\n", name.c_str());
            }
            else
            {
                // Load ROM file
                uint8_t* rom = new uint8_t[sizeof(_ROM)];
                if(!rom)
                {
                    shutdown();
                    fprintf(stderr, "Cpu::initialise() : out of memory!\n");
                    _EXIT_(EXIT_FAILURE);
                }

                file.read((char *)rom, sizeof(_ROM));
                if(file.bad() || file.fail())
                {
                    fprintf(stderr, "Cpu::initialise() : failed to read ROM file : %s\n", name.c_str());
                }
                else
                {
                    _romFiles.push_back(rom);
                    Editor::addRomEntry(type, name);
                }
            }
        }

        // Switchable ROMS
        _numRoms = int(_romFiles.size());
        _romIndex = _numRoms - 1;
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));

//#define CREATE_ROM_HEADER
#ifdef CREATE_ROM_HEADER
        // Create a header file representation of a ROM, (match the ROM type number with the ROM file before enabling and running this code)
        createRomHeader((uint8_t *)_ROM, "gigatron_xxxx.h", "_gigatron_xxxx_rom", sizeof(_ROM));
#endif

//#define CUSTOM_ROM
#ifdef CUSTOM_ROM
        initialiseInternalGt1s();
        patchSYS_Exec_88();

#define CUSTOM_ROMV0
#ifdef CUSTOM_ROMV0
        patchTitleIntoRom(" TTL microcomputer ROM v0");
        patchSplitGt1IntoRom("./roms/starfield.rom", "Starfield", 0x0b00, MandelbrotGt1);
        patchSplitGt1IntoRom("./roms/life.rom", "Life", 0x0f00, LoaderGt1);
        patchSplitGt1IntoRom("./roms/lines.rom", "Lines", 0x1100, SnakeGt1);
        patchSplitGt1IntoRom("./roms/gigatris.rom", "Gigatris", 0x1300, PicturesGt1);
        patchSplitGt1IntoRom("./roms/tetris.rom", "Tetris", 0x3000, CreditsGt1);
        patchSplitGt1IntoRom("./roms/miditest.rom", "Midi", 0x5800, RacerGt1);
#else
        patchTitleIntoRom(" TTL microcomputer ROM v0");
        patchSplitGt1IntoRom("./roms/midi64.rom", "Midi64", 0x0b00, PicturesGt1);
#endif
#endif

        // SDL initialisation
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
        {
            fprintf(stderr, "Cpu::initialise() : failed to initialise SDL.\n");
            _EXIT_(EXIT_FAILURE);
        }
    }

    State cycle(const State& S)
    {
        // New state is old state unless something changes
        State T = S;
    
        // Instruction Fetch
        T._IR = _ROM[S._PC][ROM_INST]; 
        T._D  = _ROM[S._PC][ROM_DATA];

        // Adapted from https://github.com/kervinck/gigatron-rom/blob/master/Contrib/dhkolf/libgtemu/gtemu.c
        // Optimise for the statistically most common instructions
        switch(S._IR)
        {
            case 0x5D: // ora [Y,X++],OUT
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._X);
                T._OUT = _RAM[addr] | S._AC;
                T._X++;
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0xC2: // st [D]
            {
                _RAM[S._D] = S._AC;
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x01: // ld [D]
            {
                T._AC = _RAM[S._D];
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x00: // ld D
            {
                T._AC = S._D;
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x80: // adda D
            {
                T._AC += S._D;
                T._PC = S._PC + 1;
                return T;
            }
            break;

#if 0
            case 0xFC: // bra D
            {
                T._PC = (S._PC & 0xFF00) | S._D;
                return T;
            }
            break;

            case 0x0D: // ld [Y,X]
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._X);
                T._AC = _RAM[addr];
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0xA0: // suba D
            {
                T._AC -= S._D;
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0xE8: // blt PC,D
            {
                T._PC = (S._AC & 0x80) ? (S._PC & 0xFF00) | S._D : S._PC + 1;
                return T;
            }
            break;

            case 0x81: // adda [D]
            {
                T._AC += _RAM[S._D];
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x89: // adda [Y,D]
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._D);
                T._AC += _RAM[addr];
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x12: // ld AC,X
            {
                T._X = S._AC;
                T._PC = S._PC + 1;
                return T;
            }
            break;

            case 0x18: // ld D,OUT
            {
                T._OUT = S._D;
                T._PC = S._PC + 1;
                return T;
            }
            break;
#endif
        }

        int ins = S._IR >> 5;       // Instruction
        int mod = (S._IR >> 2) & 7; // Addressing mode (or condition)
        int bus = S._IR & 3;        // Busmode
        int W = (ins == 6);         // Write instruction?
        int J = (ins == 7);         // Jump instruction?
    
        uint8_t lo=S._D, hi=0, *to=NULL; // Mode Decoder
        int incX=0;
        if(!J)
        {
            switch(mod)
            {
                #define E(p) (W ? 0 : p) // Disable _AC and _OUT loading during _RAM write
                case 0: to = E(&T._AC);                            break;
                case 1: to = E(&T._AC);  lo=S._X;                  break;
                case 2: to = E(&T._AC);           hi=S._Y;         break;
                case 3: to = E(&T._AC);  lo=S._X; hi=S._Y;         break;
                case 4: to =   &T._X;                              break;
                case 5: to =   &T._Y;                              break;
                case 6: to = E(&T._OUT);                           break;
                case 7: to = E(&T._OUT); lo=S._X; hi=S._Y; incX=1; break;
            }
        }

        uint16_t addr = (hi << 8) | lo;
        int B = S._undef; // Data Bus
        switch(bus)
        {
            case 0: B=S._D;                              break;
            case 1: if (!W) B = _RAM[addr&(RAM_SIZE-1)]; break;
            case 2: B=S._AC;                             break;
            case 3: B=_IN;                               break;
        }

        if(W) _RAM[addr&(RAM_SIZE-1)] = B; // Random Access Memory

        uint8_t ALU; // Arithmetic and Logic Unit
        switch(ins)
        {
            case 0: ALU =         B; break; // LD
            case 1: ALU = S._AC & B; break; // ANDA
            case 2: ALU = S._AC | B; break; // ORA
            case 3: ALU = S._AC ^ B; break; // XORA
            case 4: ALU = S._AC + B; break; // ADDA
            case 5: ALU = S._AC - B; break; // SUBA
            case 6: ALU = S._AC;     break; // ST
            case 7: ALU = -S._AC;    break; // Bcc/JMP
        }

        if(to) *to = ALU; // Load value into register
        if(incX) T._X = S._X + 1; // Increment _X

        T._PC = S._PC + 1; // Next instruction
        if(J)
        {
            if(mod != 0) // Conditional branch within page
            {
                int cond = (S._AC>>7) + 2*(S._AC==0);
                if(mod & (1 << cond)) // 74153
                {
                    T._PC = (S._PC & 0xff00) | B;
                }
            }
            else
            {
                T._PC = (S._Y << 8) | B; // Unconditional far jump
            }
        }

        return T;
    }

    void reset(bool coldBoot)
    {
        _checkRomType = true;

        // Cold boot
        if(coldBoot)
        {
            //setRAM(BOOT_COUNT, 0x00);
            //setRAM(BOOT_CHECK, 0xA6);
        }

        Graphics::resetVTable();
        Editor::setSingleStepAddress(VIDEO_Y_ADDRESS);
        setClock(CLOCK_RESET);
    }

    // Counts maximum and used vCPU instruction slots available per frame
    void vCpuUsage(State& S)
    {
        // All ROM's so far v1 through v4 use the same vCPU dispatch address!
        if(S._PC == ROM_VCPU_DISPATCH)
        {
            _vPC = (getRAM(0x0017) <<8) | getRAM(0x0016);
            if(_vPC < Editor::getCpuUsageAddressA()  ||  _vPC > Editor::getCpuUsageAddressB()) _vCpuInstPerFrame++;
            _vCpuInstPerFrameMax++;

            static uint64_t prevFrameCounter = 0;
            double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
            if(frameTime > VSYNC_TIMING_60)
            {
                // TODO: this is a bit of a hack, but it's emulation only so...
                // Check for magic cookie that defines a CpuUsageAddressA and CpuUsageAddressB sequence
                uint16_t magicWord0 = (getRAM(0x7F99) <<8) | getRAM(0x7F98);
                uint16_t magicWord1 = (getRAM(0x7F9B) <<8) | getRAM(0x7F9A);
                uint16_t cpuUsageAddressA = (getRAM(0x7F9D) <<8) | getRAM(0x7F9C);
                uint16_t cpuUsageAddressB = (getRAM(0x7F9F) <<8) | getRAM(0x7F9E);
                if(magicWord0 == 0xDEAD  &&  magicWord1 == 0xBEEF)
                {
                    Editor::setCpuUsageAddressA(cpuUsageAddressA);
                    Editor::setCpuUsageAddressB(cpuUsageAddressB);
                }

                prevFrameCounter = SDL_GetPerformanceCounter();
                _vCpuUtilisation = (_vCpuInstPerFrameMax) ? float(_vCpuInstPerFrame) / float(_vCpuInstPerFrameMax) : 0.0f;
                _vCpuInstPerFrame = 0;
                _vCpuInstPerFrameMax = 0;
            }
        }
    }
#endif
}
