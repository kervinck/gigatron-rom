#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>

#include <SDL.h>
#include "cpu.h"
#include "editor.h"
#include "timing.h"
#include "graphics.h"


#ifdef _WIN32
#include <Windows.h>
#endif


namespace Cpu
{
    int64_t _clock = -2;
    uint8_t _IN = 0xFF, _XOUT = 0x00;
    uint8_t _ROM[ROM_SIZE][2], _RAM[RAM_SIZE];


    int64_t getClock(void) {return _clock;}
    uint8_t getIN(void) {return _IN;}
    uint8_t getXOUT(void) {return _XOUT;}
    uint8_t getROM(uint16_t address, int page) {return _ROM[address & (ROM_SIZE-1)][page & 0x01];}
    uint8_t getRAM(uint16_t address) {return _RAM[address & (RAM_SIZE-1)];}

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

    void garble(uint8_t* mem, int len)
    {
        for(int i=0; i<len; i++) mem[i] = rand();
    }

    void initialise(Cpu::State& S)
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
        srand(unsigned int(time(NULL))); // Initialize with randomized data
        garble((uint8_t*)_ROM, sizeof _ROM);
        garble(_RAM, sizeof _RAM);
        garble((uint8_t*)&S, sizeof S);

        // Open ROM file
        std::ifstream romfile("theloop.2.rom", std::ios::binary | std::ios::in);
        if(!romfile.is_open())
        {
            fprintf(stderr, "Cpu::initialise() : failed to open ROM file\n");
            exit(EXIT_FAILURE);
        }

        // Read ROM file
        romfile.read((char *)_ROM, sizeof(_ROM));
        if(romfile.bad() || romfile.fail())
        {
            fprintf(stderr, "Cpu::initialise() : failed to read ROM file\n");
            exit(EXIT_FAILURE);
        }

        // SDL initialisation
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
        {
            fprintf(stderr, "Cpu::initialise() : failed to initialise SDL\n");
            exit(EXIT_FAILURE);
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
}