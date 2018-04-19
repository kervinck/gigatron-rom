/*
Ver 0.2
- YouTube https://youtu.be/fH30TR6jeQM

Features:
- Robust variable timing from a minimum of 60FPS up to whatever your PC can handle.
- Robust synchronisation of audio with video at any FPS at or above 60FPS.
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.
- Supports fullscreen optimised rendering.
- Supports Gigatron TTL input buttons.
- Supports Gigatron TTL LED display.
- Supports Gigatron TTL audio channels.
- Supports Gigatron TTL loading of external vCPU code with a file browser.
- Displays Gigatron TTL IN and XOUT registers.
- Displays memory monitor of RAM, ROM0 and ROM1 at any start address.
- Can execute hand crafted code within the Hex Editor.

Adapted from:
- gigatron-rom https://github.com/kervinck/gigatron-rom
- SDL(2) graphics https://github.com/kervinck/gigatron-rom/pull/1
- http://talkchess.com/forum/viewtopic.php?topic_view=threads&p=742122&t=65944 and http://talkchess.com/forum/viewtopic.php?t=65944&start=11

Building:
- No project or makefiles are supplied, they should be trivial to generate.
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.

Installation:
- After building, copy the executable, SDL2 shared library/DLL, "theloop2.rom" and "EmuFont-256x128.bmp" to an appropriate directory; run the executable from there.

Controls:
- <ESC>     Quits the application.
- <l>       Loads external vCPU files within the vCPU directory.
- <r>       Switches Hex Editor between RAM, ROM0 and ROM1.
- <F5>      Executes whatever code is present at the Hex Editor's address.
- <CR>      Directly loads external vCPU code if editor is in file browese mode, otherwise switches to edit mode.
- <-/+>     Decrease/increase the speed of the emulation, from a minimum of 60FPS to a maximum determined by your PC's CPU.

- <Left>    Navigate the Hex editor one hex digit at a time or the file browser one file at a time.
- <Right>
- <Up/Down>

- <PgUp>    Scroll the hex editor and file browser one page at a time.
- <PgDn>

- <Wheel>   Scroll the hex editor and file browser one line/file at a time.

- <d>       Right.
- <a>       Left.
- <w>       Up.
- <s>       Down.
- <SPACE>   Start.
- <z>       Select.
- </>       B.
- <.>       A.

Limitations:
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail or is ignored.
- Controls and VSync are modifiable in the code, VSync currently seems to have little value as an option as the main loop is synchronised using the performance counters.

TODO:
- Add single step debugging of vCPU code within Hex Editor.
- Test under linux, MAC and Android.
- Refactor code into modules and provide makefiles or a cmake setup.
/*

/*
BSD 2-Clause License

Copyright (c) 2017, Marcel van Kervinck
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#if defined(_WIN32)
#include <Windows.h>
#undef min
#undef max
#undef IN
#undef OUT
#endif

#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <iostream>

#if defined(_WIN32)
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#define VERSION_STR "Ver: 0.2"

#define SCREEN_WIDTH     640
#define SCREEN_HEIGHT    480
#define FONT_BMP_WIDTH   256
#define FONT_BMP_HEIGHT  128
#define FONT_WIDTH       8
#define FONT_HEIGHT      16
#define CHARS_PER_ROW    32
#define HEX_CHARS_X      6
#define HEX_CHARS_Y      23
#define HEX_CHAR_WIDE    24
#define VSYNC_START     -36
#define VSYNC_RATE       60
#define SCAN_LINES       521
#define HLINE_START      0
#define HLINE_END        200
#define HPIXELS_START    13
#define HPIXELS_END      173
#define COLOUR_PALETTE   64
#define NUM_LEDS         4
#define TIMING_HACK      0.016666

#define INPUT_RIGHT   0x01
#define INPUT_LEFT    0x02
#define INPUT_DOWN    0x04
#define INPUT_UP      0x08
#define INPUT_START   0x10
#define INPUT_SELECT  0x20
#define INPUT_B       0x40
#define INPUT_A       0x80

#define ROM_SIZE (1<<16)
#define RAM_SIZE (1<<16) // Can be 32k or 64k


enum EditorModes {Hex=0, Load, GigaLoad, NumEditorModes};

// Globals
uint8_t IN = 0xFF, XOUT = 0x00;
uint8_t ROM[ROM_SIZE][2], RAM[RAM_SIZE];
uint32_t Pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t Colours[COLOUR_PALETTE];
SDL_AudioDeviceID AudioDevice = 1;
SDL_Surface* FontSurface = NULL;
int HSync = 0, VSync = 0;
int CursorX = 0, CursorY = 0;
bool StartUploading = false;
bool LoadFile = false;
bool HexEdit = false;
int EditorMode = EditorModes::Hex;
int HexRomMode = 0;
double TimingHack = TIMING_HACK;
double FrameTime = 0.0;
uint64_t FrameCount = 0;
bool FrameUpdate = false;
std::vector<std::string> DirectoryNames;
int DirectoryNamesIndex = 0;
uint16_t HexBaseAddress = 0x0200;
uint8_t MemoryDigit = 0;
uint8_t AddressDigit = 0;


// TTL state that the CPU controls
typedef struct
{
    uint16_t PC;
    uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;
// CPU emulator
CpuState cpuCycle(const CpuState S)
{
    CpuState T = S; // New state is old state unless something changes
    
    T.IR = ROM[S.PC][0]; // Instruction Fetch
    T.D  = ROM[S.PC][1];
    
    int ins = S.IR >> 5;       // Instruction
    int mod = (S.IR >> 2) & 7; // Addressing mode (or condition)
    int bus = S.IR&3;          // Busmode
    int W = (ins == 6);        // Write instruction?
    int J = (ins == 7);        // Jump instruction?
    
    uint8_t lo=S.D, hi=0, *to=NULL; // Mode Decoder
    int incX=0;
    if(!J)
    {
        switch (mod)
        {
            #define E(p) (W?0:p) // Disable AC and OUT loading during RAM write
            case 0: to=E(&T.AC);                          break;
            case 1: to=E(&T.AC); lo=S.X;                  break;
            case 2: to=E(&T.AC);         hi=S.Y;          break;
            case 3: to=E(&T.AC); lo=S.X; hi=S.Y;          break;
            case 4: to=  &T.X;                            break;
            case 5: to=  &T.Y;                            break;
            case 6: to=E(&T.OUT);                         break;
            case 7: to=E(&T.OUT); lo=S.X; hi=S.Y; incX=1; break;
        }
    }

    uint16_t addr = (hi << 8) | lo;
    int B = S.undef; // Data Bus
    switch(bus)
    {
        case 0: B=S.D;                              break;
        case 1: if (!W) B = RAM[addr&(RAM_SIZE-1)]; break;
        case 2: B=S.AC;                             break;
        case 3: B=IN;                               break;
    }

    if(W) RAM[addr&(RAM_SIZE-1)] = B; // Random Access Memory

    uint8_t ALU; // Arithmetic and Logic Unit
    switch(ins)
    {
        case 0: ALU =        B; break; // LD
        case 1: ALU = S.AC & B; break; // ANDA
        case 2: ALU = S.AC | B; break; // ORA
        case 3: ALU = S.AC ^ B; break; // XORA
        case 4: ALU = S.AC + B; break; // ADDA
        case 5: ALU = S.AC - B; break; // SUBA
        case 6: ALU = S.AC;     break; // ST
        case 7: ALU = -S.AC;    break; // Bcc/JMP
    }

    if(to) *to = ALU; // Load value into register
    if(incX) T.X = S.X + 1; // Increment X

    T.PC = S.PC + 1; // Next instruction
    if(J)
    {
        if(mod != 0) // Conditional branch within page
        {
            int cond = (S.AC>>7) + 2*(S.AC==0);
            if (mod & (1 << cond)) // 74153
            T.PC = (S.PC & 0xff00) | B;
        }
        else
        {
            T.PC = (S.Y << 8) | B; // Unconditional far jump
        }
    }

    return T;
}

void garble(uint8_t mem[], int len)
{
    for(int i=0; i<len; i++) mem[i] = rand();
}


// Loader
void sendByte(uint8_t value, uint8_t& checksum)
{
    IN = value;
    checksum += value;
}
enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
bool sendFrame(int vgaY, uint8_t firstByte, uint8_t* message, uint8_t len, uint16_t address, uint8_t& checksum)
{
    static LoaderState loaderState = LoaderState::FirstByte;
    static uint8_t payload[60];

    bool sending = true;

    switch(loaderState)
    {
        case LoaderState::FirstByte: // 8 bits
        {
            if(vgaY == VSYNC_START+8)
            {
                memcpy(payload, message, len);
                sendByte(firstByte, checksum);
                checksum += firstByte << 6;
                loaderState = LoaderState::MsgLength;
            }
        }
        break;

        case LoaderState::MsgLength: // 6 bits
        {
            if(vgaY == VSYNC_START+14)
            {
                sendByte(len, checksum);
                loaderState = LoaderState::LowAddress;
            }
        }
        break;

        case LoaderState::LowAddress: // 8 bits
        {
            if(vgaY == VSYNC_START+22)
            {
                sendByte(address & 0x00FF, checksum);
                loaderState = LoaderState::HighAddress;
            }
        }
        break;

        case LoaderState::HighAddress: // 8 bits
        {
            if(vgaY == VSYNC_START+30)
            {
                sendByte(address >> 8, checksum);
                loaderState = LoaderState::Message;
            }
        }
        break;

        case LoaderState::Message: // 8*60 bits
        {
            static int msgIdx = 0;
            if(vgaY == VSYNC_START+38+msgIdx*8)
            {
                sendByte(payload[msgIdx], checksum);
                if(++msgIdx == 60)
                {
                    msgIdx = 0;
                    loaderState = LoaderState::LastByte;
                }
            }
        }
        break;

        case LoaderState::LastByte: // 8 bits
        {
            if(vgaY == VSYNC_START+38+60*8)
            {
                uint8_t lastByte = -checksum;
                sendByte(lastByte, checksum);
                checksum = lastByte;
                loaderState = LoaderState::ResetIN;
            }
        }
        break;

        case LoaderState::ResetIN:
        {
            if(vgaY == VSYNC_START+39+60*8)
            {
                IN = 0xFF;
                loaderState = LoaderState::FirstByte;
                sending = false;
            }
        }
        break;
    }

    return sending;
}
enum FrameState {Resync=0, Frame, Execute, NumFrameStates};
void upload(int vgaY)
{
    static bool frameUploading = false;
    static FILE* fileToUpload = NULL;
    static uint8_t payload[10000];
    static uint8_t payloadSize = 0;

    if(StartUploading == true  ||  frameUploading == true)
    {
        if(StartUploading == true)
        {
            StartUploading = false;
            frameUploading = true;
            std::string filename = std::string(".//vCPU//" + DirectoryNames[CursorY % DirectoryNames.size()]);
            fileToUpload = fopen(filename.c_str(), "rb");
            if(fileToUpload == NULL)
            {
                frameUploading = false;
                return;
            }

            payloadSize = uint8_t(fread(payload, 1, 60, fileToUpload));
            fclose(fileToUpload);

            for(int i=0; i<payloadSize; i++) RAM[HexBaseAddress+i] = payload[i];

            RAM[0x0016] = HexBaseAddress-2 & 0x00FF;
            RAM[0x0017] = (HexBaseAddress & 0xFF00) >>8;
            RAM[0x001a] = HexBaseAddress-2 & 0x00FF;
            RAM[0x001b] = (HexBaseAddress & 0xFF00) >>8;
            frameUploading = false;
            return;
        }
            
        static uint8_t checksum = 0;
        static FrameState frameState = FrameState::Resync;

        switch(frameState)
        {
            case FrameState::Resync:
            {
                if(sendFrame(vgaY, -1, payload, payloadSize, HexBaseAddress, checksum) == false)
                {
                    checksum = 'g'; // loader resets checksum
                    frameState = FrameState::Frame;
                }
            }
            break;

            case FrameState::Frame:
            {
                if(sendFrame(vgaY,'L', payload, payloadSize, HexBaseAddress, checksum) == false)
                {
                    frameState = FrameState::Execute;
                }
            }
            break;

            case FrameState::Execute:
            {
                if(sendFrame(vgaY, 'L', payload, 0, HexBaseAddress, checksum) == false)
                {
                    checksum = 0;
                    frameState = FrameState::Resync;
                    frameUploading = false;
                }
            }
            break;
        }
    }
}

void initialise(CpuState& S, SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, SDL_Surface** surface)
{
    // Memory
    srand(unsigned int(time(NULL))); // Initialize with randomized data
    garble((uint8_t*)ROM, sizeof ROM);
    garble(RAM, sizeof RAM);
    garble((uint8_t*)&S, sizeof S);

    // ROM file
    FILE *fp = fopen("theloop.2.rom", "rb");
    if(!fp)
    {
        fprintf(stderr, "Error: failed to open ROM file\n");
        exit(EXIT_FAILURE);
    }
    fread(ROM, 1, sizeof ROM, fp);
    fclose(fp);

    // Colour palette    
    for(int i=0; i<COLOUR_PALETTE; i++)
    {
        int r = (i>>0) & 3;
        int g = (i>>2) & 3;
        int b = (i>>4) & 3;

        r = r | (r << 2) | (r << 4) | (r << 6);
        g = g | (g << 2) | (g << 4) | (g << 6);
        b = b | (b << 2) | (b << 4) | (b << 6);

        uint32_t p = 0xFF000000;
        p |= r << 16;
        p |= g << 8;
        p |= b << 0;
        Colours[i] = p;
    }

    // SDL initialisation
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
    {
        fprintf(stderr, "Error: failed to initialise SDL\n");
        exit(EXIT_FAILURE);
    }
    //SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer) < 0)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to create SDL window\n");
        exit(EXIT_FAILURE);
    }
    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
    if(*texture == NULL)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to create SDL texture\n");
        exit(EXIT_FAILURE);
    }
    *surface = SDL_GetWindowSurface(*window);
    if(*surface == NULL)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to create SDL surface\n");
        exit(EXIT_FAILURE);
    }

    // SDL audio
    SDL_AudioSpec wanted;
    SDL_zero(wanted);
    wanted.freq = SCAN_LINES*VSYNC_RATE;
    //wanted.format = AUDIO_U16;
    wanted.format = AUDIO_U8;
    wanted.channels = 1;
    //wanted.samples = 1; //521;
    //AudioDevice = SDL_OpenAudioDevice(NULL, 0, &wanted, NULL, 0);
    //if(AudioDevice == 0)
    if(SDL_OpenAudio(&wanted, NULL) < 0)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to initialise SDL audio\n");
        exit(EXIT_FAILURE);
    }
    SDL_PauseAudio(0);

    // Text
    SDL_Surface* fontSurface = SDL_LoadBMP("EmuFont-256x128.bmp"); 
    if(fontSurface == NULL)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to create SDL font surface\n");
        exit(EXIT_FAILURE);
    }
    FontSurface = SDL_ConvertSurfaceFormat(fontSurface, (*surface)->format->format, NULL);
    SDL_FreeSurface(fontSurface);
    if(FontSurface == NULL)
    {
        SDL_Quit();
        fprintf(stderr, "Error: failed to create SDL converted font surface\n");
        exit(EXIT_FAILURE);
    }
}

// Simple text routine, font is a non proportional 8*16 font loaded from a 256*128 BMP file
bool drawText(const std::string& text, int x, int y, uint32_t colour, bool invert, int invertSize)
{
    if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return false;

    uint32_t* fontPixels = (uint32_t*)FontSurface->pixels;
    for(int i=0; i<text.size(); i++)
    {
        uint8_t chr = text.c_str()[i];
        uint8_t row = chr % CHARS_PER_ROW;
        uint8_t col = chr / CHARS_PER_ROW;

        int srcx = row*FONT_WIDTH, srcy = col*FONT_HEIGHT;
        if(srcx+FONT_WIDTH-1>=FONT_BMP_WIDTH || srcy+FONT_HEIGHT-1>=FONT_BMP_HEIGHT) return false;

        int dstx = x + i*FONT_WIDTH, dsty = y;
        if(dstx+FONT_WIDTH-1>=SCREEN_WIDTH || dsty+FONT_HEIGHT-1>=SCREEN_HEIGHT) return false;

        for(int j=0; j<FONT_WIDTH; j++)
        {
            for(int k=0; k<FONT_HEIGHT; k++)
            {
                int fontAddress = (srcx + j)  +  (srcy + k)*FONT_BMP_WIDTH;
                int pixelAddress = (dstx + j)  +  (dsty + k)*SCREEN_WIDTH;
                if((invert  &&  i<invertSize) ? !fontPixels[fontAddress] : fontPixels[fontAddress])
                {
                    Pixels[pixelAddress] = 0xFF000000 | colour;
                }
                else
                {
                    Pixels[pixelAddress] = 0xFF000000;
                }
            }
        }
    }

    return true;
}

void input(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEWHEEL:
            {
                if(event.wheel.y > 0)
                {
                    if(LoadFile == true)
                    {
                        DirectoryNamesIndex--;
                        if(DirectoryNamesIndex < 0) DirectoryNamesIndex = 0;
                    }
                    else
                    {
                        HexBaseAddress = (HexBaseAddress - HEX_CHARS_X) & (RAM_SIZE-1);
                    }
                }
                if(event.wheel.y < 0)
                {
                    if(LoadFile == true)
                    {
                        if(DirectoryNames.size() > HEX_CHARS_Y)
                        {
                            DirectoryNamesIndex++;
                            if(DirectoryNames.size() - DirectoryNamesIndex < HEX_CHARS_Y) DirectoryNamesIndex--;
                        }
                    }
                    else
                    {
                        HexBaseAddress = (HexBaseAddress + HEX_CHARS_X) & (RAM_SIZE-1);
                    }
                }
            }
            break;

            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_d:      IN &= ~INPUT_RIGHT;   break;
                    case SDLK_a:      IN &= ~INPUT_LEFT;    break;
                    case SDLK_s:      IN &= ~INPUT_DOWN;    break;
                    case SDLK_w:      IN &= ~INPUT_UP;      break;
                    case SDLK_SPACE:  IN &= ~INPUT_START;   break;
                    case SDLK_z:      IN &= ~INPUT_SELECT;  break;
                    case SDLK_SLASH:  IN &= ~INPUT_B;       break;
                    case SDLK_PERIOD: IN &= ~INPUT_A;       break;

                    case SDLK_RIGHT: CursorX = (++CursorX >= HEX_CHARS_X) ? 0  : CursorX;   MemoryDigit = 0; AddressDigit = 0; break;
                    case SDLK_LEFT:  CursorX = (--CursorX < 0) ? HEX_CHARS_X-1 : CursorX;   MemoryDigit = 0; AddressDigit = 0; break;
                    case SDLK_DOWN:  CursorY = (++CursorY >= HEX_CHARS_Y) ? 0  : CursorY;   MemoryDigit = 0; AddressDigit = 0; break;
                    case SDLK_UP:    CursorY = (--CursorY < -1) ? HEX_CHARS_Y-1 : CursorY;  MemoryDigit = 0; AddressDigit = 0; break;

                    case SDLK_PAGEUP:
                    {
                        if(LoadFile == true)
                        {
                            DirectoryNamesIndex--;
                            if(DirectoryNamesIndex < 0) DirectoryNamesIndex = 0;
                        }
                        else
                        {
                            HexBaseAddress = (HexBaseAddress - HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
                        }
                    }
                    break;

                    case SDLK_PAGEDOWN:
                    {
                        if(LoadFile == true)
                        {
                            if(DirectoryNames.size() > HEX_CHARS_Y)
                            {
                                DirectoryNamesIndex++;
                                if(DirectoryNames.size() - DirectoryNamesIndex < HEX_CHARS_Y) DirectoryNamesIndex--;
                            }
                        }
                        else
                        {
                            HexBaseAddress = (HexBaseAddress + HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
                        }
                    }
                    break;

                    case SDLK_EQUALS:
                    {
                        TimingHack -= TIMING_HACK*0.05;
                        if(TimingHack < 0.0) TimingHack = 0.0;
                    }
                    break;
                    case SDLK_MINUS:
                    {
                        TimingHack += TIMING_HACK*0.05;
                        if(TimingHack > TIMING_HACK) TimingHack = TIMING_HACK;
                    }
                    break;

                    case SDLK_ESCAPE:
                    {
                        SDL_Quit();
                        exit(0);
                    }
                }
            }
            break;

            case SDL_KEYUP:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_d:      IN |= INPUT_RIGHT;    break;
                    case SDLK_a:      IN |= INPUT_LEFT;     break;
                    case SDLK_s:      IN |= INPUT_DOWN;     break;
                    case SDLK_w:      IN |= INPUT_UP;       break;
                    case SDLK_SPACE:  IN |= INPUT_START;    break;
                    case SDLK_z:      IN |= INPUT_SELECT;   break;
                    case SDLK_SLASH:  IN |= INPUT_B;        break;
                    case SDLK_PERIOD: IN |= INPUT_A;        break;
                   
                    case SDLK_l:
                    {
                        CursorX = 0; CursorY = 0;
                        LoadFile = !LoadFile;
                        if(LoadFile == true)
                        {
                            std::string path = "./vCPU";
                            DirectoryNames.clear();
#if defined(_WIN32)
                            for(std::experimental::filesystem::directory_iterator next(path), end; next!=end; ++next)
                            {
                                DirectoryNames.push_back(next->path().filename().string());
                            }
                            std::sort(DirectoryNames.begin(), DirectoryNames.end());
#else
                            for(std::filesystem::directory_iterator next(path), end; next!=end; ++next)
                            {
                                DirectoryNames.push_back(next->path().filename().string());
                            }
                            std::sort(DirectoryNames.begin(), DirectoryNames.end());
#endif
                        }
                    }
                    break;

                    case SDLK_F5:
                    {
                        RAM[0x0016] = HexBaseAddress-2 & 0x00FF;
                        RAM[0x0017] = (HexBaseAddress & 0xFF00) >>8;
                        RAM[0x001a] = HexBaseAddress-2 & 0x00FF;
                        RAM[0x001b] = (HexBaseAddress & 0xFF00) >>8;
                    }
                    break;

                    case SDLK_r:
                    {
                        HexRomMode = (++HexRomMode) % 3;
                    }
                    break;
        
                    case SDLK_RETURN:
                    {
                        // Hex edit
                        if(LoadFile == false)
                        {
                            HexEdit = !HexEdit;
                        }
                        // File load
                        else
                        {
                            StartUploading = true;
                        }
                    }
                    break;
                }

                if(HexEdit == true)
                {
                    int range = 0;
                    if(event.key.keysym.sym >= SDLK_0  &&  event.key.keysym.sym <= SDLK_9) range = 1;
                    if(event.key.keysym.sym >= SDLK_a  &&  event.key.keysym.sym <= SDLK_f) range = 2;
                    if(range == 1  ||  range == 2)
                    {
                        uint16_t value = 0;    
                        switch(range)
                        {
                            case 1: value = event.key.keysym.sym - SDLK_0;      break;
                            case 2: value = event.key.keysym.sym - SDLK_a + 10; break;
                        }

                        // Edit address
                        if(CursorY == -1)
                        {
                            switch(AddressDigit)
                            {
                                case 0: value = (value << 12) & 0xF000; HexBaseAddress = HexBaseAddress & 0x0FFF | value; break;
                                case 1: value = (value << 8)  & 0x0F00; HexBaseAddress = HexBaseAddress & 0xF0FF | value; break;
                                case 2: value = (value << 4)  & 0x00F0; HexBaseAddress = HexBaseAddress & 0xFF0F | value; break;
                                case 3: value = (value << 0)  & 0x000F; HexBaseAddress = HexBaseAddress & 0xFFF0 | value; break;
                            }
                            AddressDigit = (++AddressDigit) & 0x03;
                        }
                        // Edit memory
                        else if(HexRomMode ==0)
                        {
                            uint16_t address = HexBaseAddress + CursorX + CursorY*HEX_CHARS_X;
                            switch(MemoryDigit)
                            {
                                case 0: value = (value << 4) & 0x00F0; RAM[address] = RAM[address] & 0x000F | value; break;
                                case 1: value = (value << 0) & 0x000F; RAM[address] = RAM[address] & 0x00F0 | value; break;
                            }
                            MemoryDigit = (++MemoryDigit) & 0x01;
                        }
                    }
                }
            }
            break;

            case SDL_QUIT: 
            {
                SDL_Quit();
                exit(0);
            }
        }
    }
}


void updatePixel(const CpuState& S, int vgaX, int vgaY)
{
    uint32_t colour = Colours[S.OUT & (COLOUR_PALETTE-1)];
    uint32_t address = vgaX*3 + vgaY*SCREEN_WIDTH - 0;
    Pixels[address + 0] = colour;
    Pixels[address + 1] = colour;
    Pixels[address + 2] = colour;
}

void renderLeds(void)
{
    // Update 60 times per second no matter what the FPS is
    if(FrameTime  &&  FrameUpdate)
    {
        for(int i=0; i<NUM_LEDS; i++)
        {
            int mask = 1 << (NUM_LEDS-1 - i);
            int state = (XOUT & mask) != 0;
            uint32_t colour = state ? 0xFF00FF00 : 0xFF770000;

            int address = int(float(SCREEN_WIDTH) * 0.875f) + i*NUM_LEDS + 6*SCREEN_WIDTH;
            Pixels[address + 0] = colour;
            Pixels[address + 1] = colour;
            Pixels[address + 2] = colour;
            address += SCREEN_WIDTH;
            Pixels[address + 0] = colour;
            Pixels[address + 1] = colour;
            Pixels[address + 2] = colour;
        }
    }
}

void renderText(void)
{
    // Update 60 times per second no matter what the FPS is
    if(FrameTime  &&  FrameUpdate)
    {
        char str[32];
        sprintf(str, "%3.2f    ", 1.0f / FrameTime);

        drawText(std::string("LEDS:"), 485, 0, 0xFFFFFFFF, false, 0);
        drawText(std::string("FPS:"), 485, 20, 0xFFFFFFFF, false, 0);
        drawText(std::string(str), 548, 20, 0xFFFFFF00, false, 0);
        drawText(std::string(VERSION_STR), 530, 464, 0xFFFFFF00, false, 0);
        sprintf(str, "XOUT: %02X  IN: %02X", XOUT, IN);
        drawText(std::string(str), 485, 40, 0xFFFFFFFF, false, 0);
        drawText("Mode:       ", 485, 448, 0xFFFFFFFF, false, 0);
        sprintf(str, "Hex ");
        if(HexEdit == true) sprintf(str, "Edit");
        if(LoadFile == true) sprintf(str, "Load");
        drawText(std::string(str), 541, 448, 0xFF00FF00, false, 0);
    }
}

void renderDigitBox(uint8_t digit, int x, int y, uint32_t colour)
{
    uint32_t pixelAddress = x + digit*FONT_WIDTH + y*SCREEN_WIDTH;

    pixelAddress += (FONT_HEIGHT-4)*SCREEN_WIDTH;
    for(int i=0; i<FONT_WIDTH; i++) Pixels[pixelAddress+i] = colour;
    pixelAddress += SCREEN_WIDTH;
    for(int i=0; i<FONT_WIDTH; i++) Pixels[pixelAddress+i] = colour;
    pixelAddress += SCREEN_WIDTH;
    for(int i=0; i<FONT_WIDTH; i++) Pixels[pixelAddress+i] = colour;
    pixelAddress += SCREEN_WIDTH;
    for(int i=0; i<FONT_WIDTH; i++) Pixels[pixelAddress+i] = colour;

    //for(int i=0; i<FONT_WIDTH-1; i++) Pixels[pixelAddress+i] = colour;
    //pixelAddress += (FONT_HEIGHT-1)*SCREEN_WIDTH;
    //for(int i=0; i<FONT_WIDTH-1; i++) Pixels[pixelAddress+i] = colour;
    //for(int i=0; i<FONT_HEIGHT; i++) Pixels[pixelAddress-i*SCREEN_WIDTH] = colour;
    //pixelAddress += FONT_WIDTH-1;
    //for(int i=0; i<FONT_HEIGHT; i++) Pixels[pixelAddress-i*SCREEN_WIDTH] = colour;
}

void renderTextWindow(void)
{
    // Update 60 times per second no matter what the FPS is
    if(FrameTime  &&  FrameUpdate)
    {
        char str[32] = "";

        // File load
        if(LoadFile == true)
        {
            if(CursorY == -1) CursorY = HEX_CHARS_Y-1;
            if(CursorY >= DirectoryNames.size()) CursorY = 0;
            if(CursorY < 0) CursorY = std::min(int(DirectoryNames.size()), HEX_CHARS_Y) - 1;
            drawText("vCPU Files:", 485, 60, 0xFFFFFFFF, false, 0);
            for(int i=0; i<HEX_CHARS_Y; i++)
            {
                drawText("                  ", 493, 80 + i*FONT_HEIGHT, 0xFFFFFFFF, false, 0);
            }
            for(int i=0; i<HEX_CHARS_Y; i++)
            {
                int index = DirectoryNamesIndex + i;
                if(index >= int(DirectoryNames.size())) break;
                drawText(DirectoryNames[DirectoryNamesIndex + i], 493, 80 + i*FONT_HEIGHT, 0xFFFFFFFF, i == CursorY, 18);
            }
        }
        // Hex monitor
        else
        {
            switch(HexRomMode)
            {
                case 0: drawText("RAM:   ", 485, 60, 0xFFFFFFFF, false, 0); break;
                case 1: drawText("ROM 0: ", 485, 60, 0xFFFFFFFF, false, 0); break;
                case 2: drawText("ROM 1: ", 485, 60, 0xFFFFFFFF, false, 0); break;
            }

            uint16_t address = HexBaseAddress;
            bool onCursor = CursorY == -1;
            sprintf(str, "%04X  ", HexBaseAddress);
            drawText(std::string(str), 541, 60, (HexEdit && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 4);
            for(int j=0; j<HEX_CHARS_Y; j++)
            {
                for(int i=0; i<HEX_CHARS_X; i++)
                {
                    uint8_t value = 0;
                    switch(HexRomMode)
                    {
                        case 0: value = RAM[address++];    break;
                        case 1: value = ROM[address++][0]; break;
                        case 2: value = ROM[address++][1]; break;
                    }
                    sprintf(str, "%02X ", value);
                    bool onCursor = (i == CursorX  &&  j == CursorY);
                    drawText(std::string(str), 493 + i*HEX_CHAR_WIDE, 80 + j*FONT_HEIGHT, (HexEdit && HexRomMode ==0 && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 2);
                }
            }
            if(HexEdit == true)
            {
                // Draw address digit selection box
                if(CursorY == -1) renderDigitBox(AddressDigit, 541, 60, 0xFFFF00FF);

                // Draw memory digit selection box                
                if(CursorY >= 0  &&  HexRomMode == 0) renderDigitBox(MemoryDigit, 493 + CursorX*HEX_CHAR_WIDE, 80 + CursorY*FONT_HEIGHT, 0xFFFF00FF);
            }
        }
    }
}

void render(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface)
{
    renderLeds();
    renderText();
    renderTextWindow();

    SDL_UpdateTexture(texture, NULL, Pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    static uint64_t prevFrameCounter = 0;
    uint64_t frameCounter;
    do
    {
        frameCounter = SDL_GetPerformanceCounter();
        FrameTime = double(frameCounter - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
    }
    while(FrameTime < TimingHack);

    // Used for updating at a constant 60 times per second no matter what the FPS is
    FrameUpdate = ((FrameCount % int(1.0*TIMING_HACK/std::min(FrameTime, TIMING_HACK))) == 0);

    prevFrameCounter = frameCounter;

    FrameCount++;
}

void playAudioSample(void)
{
    double ratio = 1.0;
    if(FrameTime) ratio = std::max(1.0, TIMING_HACK / std::min(FrameTime, TIMING_HACK));

    static double skip = 0.0;
    uint64_t count = uint64_t(skip);
    skip += 1.0 / ratio;
    if(uint64_t(skip) > count)
    {
        uint8_t sample = XOUT & 0xF0;
        SDL_QueueAudio(AudioDevice, &sample, 1);
    }
}


int main(int argc, char* argv[])
{
    CpuState S;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    SDL_Surface* surface = NULL;

    initialise(S, &window, &renderer, &texture, &surface);                   

    int vgaX=0, vgaY=0;
    for(long long t=-2; ; t++) 
    {
        if(t < 0) S.PC = 0; // MCP100 Power-On Reset

        CpuState T = cpuCycle(S); // Update CPU

        HSync = (T.OUT & 0x40) - (S.OUT & 0x40); // "VGA monitor" (use simple stdout)
        VSync = (T.OUT & 0x80) - (S.OUT & 0x80);
        if(VSync < 0) // Falling vSync edge
        {
            vgaY = VSYNC_START;

            // Input and graphics
            input();
            render(window, renderer, texture, surface);
        }
        if(vgaX++ < HLINE_END)
        {
            if(HSync) { }              // Visual indicator of hSync
            else if(vgaX == HLINE_END) { }   // Too many pixels
            else if(~S.OUT & 0x80) { } // Visualize vBlank pulse
            else if(vgaX >=HPIXELS_START  &&  vgaX < HPIXELS_END  &&  vgaY >= 0  &&  vgaY < SCREEN_HEIGHT)
            {
                updatePixel(S, vgaX-HPIXELS_START, vgaY);
            }
        }
        if(HSync > 0) // Rising hSync edge
        {
            XOUT = T.AC;
            
            // Audio
            playAudioSample();

            // Upload user code to loader
            upload(vgaY);

            vgaX = 0;
            vgaY++;
            T.undef = rand() & 0xff; // Change this once in a while
        }

        S=T;
    }

    return 0;
}
