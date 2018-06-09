#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <vector>

#include "cpu.h"

#ifndef STAND_ALONE
#include <SDL.h>
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "gigatron_0x1c.h"
#endif

#ifdef _WIN32
#include <Windows.h>
#endif


namespace Cpu
{
    int _vCpuInstPerFrame = 0;
    int _vCpuInstPerFrameMax = 0;
    float _vCpuUtilisation = 0.0;

    int64_t _clock = -2;
    uint8_t _IN = 0xFF, _XOUT = 0x00;
    uint8_t _ROM[ROM_SIZE][2], _RAM[RAM_SIZE];

    uint16_t _baseFreeRAM = RAM_SIZE - RAM_USED_DEFAULT;
    uint16_t _freeRAM = _baseFreeRAM;

    std::vector<uint8_t> scanlinesRom0;
    std::vector<uint8_t> scanlinesRom1;


    uint16_t getBaseFreeRAM(void) {return _baseFreeRAM;}
    uint16_t getFreeRAM(void) {return _freeRAM;}
    void setFreeRAM(uint16_t freeRAM) {_freeRAM = freeRAM;}

#ifndef STAND_ALONE
    int64_t getClock(void) {return _clock;}
    uint8_t getIN(void) {return _IN;}
    uint8_t getXOUT(void) {return _XOUT;}
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

        _RAM[address & (RAM_SIZE-1)] = uint8_t(data & 0x00FF);
        _RAM[(address+1) & (RAM_SIZE-1)] = uint8_t((data & 0xFF00)>>8);
    }

    void setROM16(uint16_t base, uint16_t address, uint16_t data)
    {
        uint16_t offset = (address - base) / 2;
        _ROM[base + offset][address & 0x01] = uint8_t(data & 0x00FF);
        _ROM[base + offset][(address+1) & 0x01] = uint8_t((data & 0xFF00)>>8);
    }

    void saveScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            scanlinesRom0.push_back(_ROM[i][0]);
            scanlinesRom1.push_back(_ROM[i][1]);
        }
    }

    void restoreScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            _ROM[i][0] = scanlinesRom0[i - 0x01C2];
            _ROM[i][1] = scanlinesRom1[i - 0x01C2];
        }
    }

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

    void setScanlineModeVideoC(void)
    {
        _ROM[0x01DA][0] = 0xFC;
        _ROM[0x01DA][1] = 0xFD;

        _ROM[0x01DB][0] = 0xC2;
        _ROM[0x01DB][1] = 0x0C;

        _ROM[0x01DC][0] = 0x02;
        _ROM[0x01DC][1] = 0x00;

        _ROM[0x01DD][0] = 0x02;
        _ROM[0x01DD][1] = 0x00;

        _ROM[0x01DE][0] = 0x02;
        _ROM[0x01DE][1] = 0x00;
    }

    void setScanlineMode(ScanlineMode scanlineMode)
    {
        switch(scanlineMode)
        {
            case Normal:  restoreScanlineModes();                           break;
            case VideoB:  setScanlineModeVideoB();                          break;
            case VideoC:  setScanlineModeVideoC();                          break;
            case VideoBC: setScanlineModeVideoB(); setScanlineModeVideoC(); break;
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
            fprintf(stderr, "Graphics::createRomHeader() : failed to create '%s'.\n", filename.c_str());
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

    void loadDefaultRom(const uint8_t* rom)
    {
        uint8_t* srcRom = (uint8_t *)rom;
        uint8_t* dstRom = (uint8_t *)_ROM;
        for(int i=0; i<sizeof(_ROM); i++)
        {
            *dstRom++ = *srcRom++;
        } 
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

        // Check for ROM file
        std::string filenameRom = "theloop.2.rom";
        std::ifstream romfile(filenameRom, std::ios::binary | std::ios::in);
        if(!romfile.is_open())
        {
            loadDefaultRom(_gigatron_0x1c_rom);
        }
        else
        {
            // Load ROM file
            romfile.read((char *)_ROM, sizeof(_ROM));
            if(romfile.bad() || romfile.fail())
            {
                fprintf(stderr, "Cpu::initialise() : failed to read %s ROM file, using default ROM.\n", filenameRom.c_str());
                loadDefaultRom(_gigatron_0x1c_rom);
            }
#ifdef CREATE_ROM_HEADER
            // Use this if you ever want to change the default ROM
            createRomHeader((uint8_t *)_ROM, "gigatron_0x1c.h", "_gigatron_0x1c_rom", sizeof(_ROM));
#endif
        }
        saveScanlineModes();

        // SDL initialisation
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
        {
            fprintf(stderr, "Cpu::initialise() : failed to initialise SDL.\n");
            _EXIT_(EXIT_FAILURE);
        }
    }

    State cycle(const State& S)
    {
        State T = S; // New state is old state unless something changes
    
        T._IR = _ROM[S._PC][0]; // Instruction Fetch
        T._D  = _ROM[S._PC][1];
    
        int ins = S._IR >> 5;       // Instruction
        int mod = (S._IR >> 2) & 7; // Addressing mode (or condition)
        int bus = S._IR&3;          // Busmode
        int W = (ins == 6);        // Write instruction?
        int J = (ins == 7);        // Jump instruction?
    
        uint8_t lo=S._D, hi=0, *to=NULL; // Mode Decoder
        int incX=0;
        if(!J)
        {
            switch (mod)
            {
                #define E(p) (W?0:p) // Disable _AC and _OUT loading during _RAM write
                case 0: to=E(&T._AC);                            break;
                case 1: to=E(&T._AC); lo=S._X;                   break;
                case 2: to=E(&T._AC);         hi=S._Y;           break;
                case 3: to=E(&T._AC); lo=S._X; hi=S._Y;          break;
                case 4: to=  &T._X;                              break;
                case 5: to=  &T._Y;                              break;
                case 6: to=E(&T._OUT);                           break;
                case 7: to=E(&T._OUT); lo=S._X; hi=S._Y; incX=1; break;
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
            case 0: ALU =        B; break; // LD
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
                if (mod & (1 << cond)) // 74153
                T._PC = (S._PC & 0xff00) | B;
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
        // Cold boot
        if(coldBoot)
        {
            setRAM(BOOT_COUNT, 0x00);
            setRAM(BOOT_CHECK, 0xA6); // TODO: don't hardcode the checksum, calculate it properly
        }

        Graphics::resetVTable();
        Editor::setSingleStepWatchAddress(VIDEO_Y_ADDRESS);
        setClock(CLOCK_RESET);
    }

    // Counts maximum and used vCPU instruction slots available per frame
    void vCpuUsage(State& S)
    {
        if(S._PC == ROM_VCPU_DISPATCH)
        {
            uint16_t vPC = (getRAM(0x0017) <<8) |getRAM(0x0016);
            if(vPC < Editor::getCpuUsageAddressA()  ||  vPC > Editor::getCpuUsageAddressB()) _vCpuInstPerFrame++;
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