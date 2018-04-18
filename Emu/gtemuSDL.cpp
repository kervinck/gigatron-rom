/*
Ver 0.1
- YouTube https://youtu.be/pH4st5dz7Go

Features:
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.
- Supports fullscreen optimised rendering.
- Supports Gigatron TTL input buttons.
- Supports Gigatron TTL LED display.
- Supports Gigatron TTL audio channels.
- Supports Gigatron TTL loading of external vCPU code.
- Displays Gigatron TTL IN and XOUT registers.
- Displays memory monitor of vCPU standard start address, (0x200).

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
- <ESC>		Quits the application.
- <l>       Loads the hard-coded blinky vCPU example.
- <RIGHT>	Right.
- <LEFT>	Left.
- <UP>		Up.
- <DOWN>	Down.
- <SPACE>	Start.
- <s>		Select.
- <x>		B.
- <z>		A.

Limitations:
- The simulation is timed using performance counters; this can lead to incorrect drift over time, especially with the Audio.
- The simulation timing is controlled via a simple threshold, (TIMING_HACK), that simply blocks the process until the required 60FPS is met. This implies that your PC MUST be able to meet the minimum required timings to hit the target 60 FPS. If your PC does not meet the minimum requirements, then the simulation will not hit 60FPS and the audio will stutter, crackle and pop as the audio buffers are not filled in time.
- Audio simulation is extremely simple and only synchronises with the video if your computer is able to meet the timing requirements to run the simulation at 60 FPS.
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail or is ignored.
- Controls and VSync are modifiable in the code, VSync currently seems to have little value as an option as the main loop is synchronised using the performance counters.

TODO:
- Test under linux, MAC and Android.
- More robust timing for the audio, the simulation can currently run at 550FPS on my 4GHz Haswell CPU, but this causes the audio to fall way behind in synchronisation.
- Allow a simple interface to load external file system vCPU files.
- Allow memory monitor start address and contents to be editable.
- Refactor code into modules.
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

#include <chrono>
#include <thread>
#include <atomic>


#define SCREEN_WIDTH     640
#define SCREEN_HEIGHT    480
#define FONT_BMP_WIDTH   256
#define FONT_BMP_HEIGHT  128
#define FONT_WIDTH       8
#define FONT_HEIGHT      16
#define CHARS_PER_ROW    32
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

#define AUD_SAMPLES    1024
#define AUD_FREQUENCY  (SCAN_LINES*VSYNC_RATE)


// TTL state that the CPU controls
typedef struct
{
    uint16_t PC;
    uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;


// Globals
uint8_t IN = 0xFF, XOUT = 0x00;
uint8_t ROM[ROM_SIZE][2], RAM[RAM_SIZE];
uint32_t Pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t Colours[COLOUR_PALETTE];
uint8_t AudioSamples[AUD_SAMPLES];
int32_t AudioIndex = 0;
SDL_AudioDeviceID AudioDevice = 1;
SDL_Surface* FontSurface = NULL;
int HSync = 0;
int VSync = 0;
bool StartUploading = false;


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
uint8_t blinky[] = 
{
    0x11, 0x00, 0x08, // 7f00 LDWI $4450  ; Load address of center of screen
    0x2b, 0x30,       // 7f03 STW  'p'    ; Store in variable 'p' (at $0030)
    0xf0, 0x30,       // 7f05 POKE 'p'    ; Write low byte of accumulator there
    0xe3, 0x01,       // 7f07 ADDI 1      ; Increment accumulator
    0x90, 0x03        // 7f09 BRA  $7f05  ; Loop forever
}; 
enum FrameState {Resync=0, Frame, Execute, NumFrameStates};
enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
void sendByte(uint8_t value, uint8_t& checksum)
{
    IN = value;
    checksum += value;
}
bool sendFrame(int vgaY, uint8_t firstByte, uint8_t* message, uint8_t len, uint16_t address, uint8_t& checksum)
{
    static LoaderState loaderState = FirstByte;
    static uint8_t payload[60];

    bool sending = true;

    switch(loaderState)
    {
        case FirstByte: // 8 bits
        {
            if(vgaY == VSYNC_START+8)
            {
                memcpy(payload, message, len);
                sendByte(firstByte, checksum);
                checksum += firstByte << 6;
                loaderState = MsgLength;
            }
        }
        break;

        case MsgLength: // 6 bits
        {
            if(vgaY == VSYNC_START+14)
            {
                sendByte(len, checksum);
                loaderState = LowAddress;
            }
        }
        break;

        case LowAddress: // 8 bits
        {
            if(vgaY == VSYNC_START+22)
            {
                sendByte(address & 0x00FF, checksum);
                loaderState = HighAddress;
            }
        }
        break;

        case HighAddress: // 8 bits
        {
            if(vgaY == VSYNC_START+30)
            {
                sendByte(address >> 8, checksum);
                loaderState = Message;
            }
        }
        break;

        case Message: // 8*60 bits
        {
            static int msgIdx = 0;
            if(vgaY == VSYNC_START+38+msgIdx*8)
            {
                sendByte(payload[msgIdx], checksum);
                if(++msgIdx == 60)
                {
                    msgIdx = 0;
                    loaderState = LastByte;
                }
            }
        }
        break;

        case LastByte: // 8 bits
        {
            if(vgaY == VSYNC_START+38+60*8)
            {
                uint8_t lastByte = -checksum;
                sendByte(lastByte, checksum);
                checksum = lastByte;
                loaderState = ResetIN;
            }
        }
        break;

        case ResetIN:
        {
            if(vgaY == VSYNC_START+39+60*8)
            {
                IN = 0xFF;
                loaderState = FirstByte;
                sending = false;
            }
        }
        break;
    }

    return sending;
}
void upload(int vgaY)
{
    static bool frameUploading = false;

    if(StartUploading == true  ||  frameUploading == true)
    {
        if(StartUploading == true)
        {
            StartUploading = false;
            frameUploading = true;
        }
            
        static uint8_t checksum = 0;
        static FrameState frameState = Resync;

        switch(frameState)
        {
            case Resync:
            {
                if(sendFrame(vgaY, -1, blinky, sizeof(blinky), 0x0200, checksum) == false)
                {
                    checksum = 'g'; // loader resets checksum
                    frameState = Frame;
                }
            }
            break;

            case Frame:
            {
                if(sendFrame(vgaY,'L', blinky, sizeof(blinky), 0x0200, checksum) == false)
                {
                    frameState = Execute;
                }
            }
            break;

            case Execute:
            {
                if(sendFrame(vgaY, 'L', blinky, 0, 0x0200, checksum) == false)
                {
                    checksum = 0;
                    frameState = Resync;
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
    wanted.freq = AUD_FREQUENCY;
    //wanted.format = AUDIO_U16;
    wanted.format = AUDIO_U8;
    wanted.channels = 1;
    wanted.samples = AUD_SAMPLES;
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
bool drawText(const std::string& text, int x, int y, uint32_t colour)
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
                if(fontPixels[fontAddress])
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
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RIGHT: IN &= ~INPUT_RIGHT;   break;
                    case SDLK_LEFT:  IN &= ~INPUT_LEFT;    break;
                    case SDLK_DOWN:  IN &= ~INPUT_DOWN;    break;
                    case SDLK_UP:    IN &= ~INPUT_UP;      break;
                    case SDLK_SPACE: IN &= ~INPUT_START;   break;
                    case SDLK_s:     IN &= ~INPUT_SELECT;  break;
                    case SDLK_x:     IN &= ~INPUT_B;       break;
                    case SDLK_z:     IN &= ~INPUT_A;       break;

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
                    case SDLK_RIGHT: IN |= INPUT_RIGHT;    break;    
                    case SDLK_LEFT:  IN |= INPUT_LEFT;     break;     
                    case SDLK_DOWN:  IN |= INPUT_DOWN;     break;     
                    case SDLK_UP:    IN |= INPUT_UP;       break;       
                    case SDLK_SPACE: IN |= INPUT_START;    break;    
                    case SDLK_s:     IN |= INPUT_SELECT;   break;   
                    case SDLK_x:     IN |= INPUT_B;        break;        
                    case SDLK_z:     IN |= INPUT_A;        break;
                     
                    case SDLK_l: StartUploading = true; break;
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

void updateLeds(void)
{
    for(int i=0; i<NUM_LEDS; i++)
	{
		int mask = 1 << (NUM_LEDS-1 - i);
		int state = (XOUT & mask) != 0;
		uint32_t colour = state ? 0xFF00FF00 : 0xFF770000;

		int address = int(float(SCREEN_WIDTH) * 0.875f) + i*NUM_LEDS + 7*SCREEN_WIDTH;
        Pixels[address + 0] = colour;
        Pixels[address + 1] = colour;
        Pixels[address + 2] = colour;
	}
}

void updatePixel(const CpuState& S, int vgaX, int vgaY)
{
    Uint32 colour = Colours[S.OUT & (COLOUR_PALETTE-1)];
    Uint32 address = vgaX*3 + vgaY*SCREEN_WIDTH - 0;
    Pixels[address + 0] = colour;
    Pixels[address + 1] = colour;
    Pixels[address + 2] = colour;
}

void render(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface)
{
    static uint64_t frameCount = 0;
    static double frameTime0 = 0.0, frameTime1 = frameTime0;

    char str[32] = "";
    if(frameCount % 30 == 0) frameTime1 = frameTime0;
    if(frameTime1) sprintf(str, "%3.2f    ", 1.0f / frameTime1);

    drawText(std::string("LEDS:"), 485, 0, 0xFFFFFFFF);
    drawText(std::string("FPS:"), 485, 20, 0xFFFFFFFF);
    drawText(std::string(str), 548, 20, 0xFFFFFF00);
    drawText(std::string("Ver: 0.1"), 530, 464, 0xFFFFFFFF);

    sprintf(str, "IN: %02X", IN);
    drawText(std::string(str), 485, 50, 0xFFFFFFFF);
    sprintf(str, "XOUT: %02X", XOUT);
    drawText(std::string(str), 485, 70, 0xFFFFFFFF);

    uint16_t ram = 0x0200;
    sprintf(str, "RAM: %04X", ram);
    drawText(std::string(str), 485, 100, 0xFFFFFFFF);
    for(int j=0; j<21; j++)
    {
        for(int i=0; i<6; i++)
        {
            sprintf(str, "%02X ", RAM[ram++]);
            drawText(std::string(str), 493 + i*24, 120 + j*16, 0xFFFFFFFF);
        }
    }

	SDL_UpdateTexture(texture, NULL, Pixels, SCREEN_WIDTH * sizeof(Uint32));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    static uint64_t prevFrameCounter = 0;
    while(double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency()) < TIMING_HACK)
    {
        //std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    frameTime0 = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
    prevFrameCounter = SDL_GetPerformanceCounter();

    frameCount++;
}

void fillAudioBuffer(void)
{
    uint8_t sample = XOUT & 0xF0;
    AudioSamples[AudioIndex] = sample;
    AudioIndex = (AudioIndex + 1) % AUD_SAMPLES;
}

void playAudioBuffer(void)
{
    SDL_QueueAudio(AudioDevice, &AudioSamples[0], AudioIndex);
    AudioIndex = 0;
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

            // Audio, input and video
            playAudioBuffer();
            input();
            updateLeds();
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
            
            // Create audio packets
            fillAudioBuffer();

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
