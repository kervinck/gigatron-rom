/*
Features:
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.
- Supports fullscreen optimised rendering.
- Supports Gigatron TTL input buttons.
- Supports Gigatron TTL LED display.
- Supports Gigatron TTL audio channels.

Adapted from:
- gigatron-rom https://github.com/kervinck/gigatron-rom
- SDL(2) graphics https://github.com/kervinck/gigatron-rom/pull/1
- http://talkchess.com/forum/viewtopic.php?topic_view=threads&p=742122&t=65944 and http://talkchess.com/forum/viewtopic.php?t=65944&start=11

Building:
- No project or makefiles are supplied, they should be trivial to generate.
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.

Installation:
- After building, copy the executable, SDL2 shared library/DLL and "theloop2.rom" to an appropriate directory; run the executable from there.

Controls:
- <ESC>		quits the application.
- <RIGHT>	Right.
- <LEFT>	Left.
- <UP>		Up.
- <DOWN>	Down.
- <SPACE>	Start.
- <s>		Select.
- <b>		B.
- <a>		A.

Limitations:
- The simulation is timed using VSYNC or performance counters or both; this can lead to incorrect drift over time, especially with the Audio.
- The simulation timing is controlled via a simple threshold, (TIMING_HACK), that is tweaked for minimum audio latency and minimum audio glitches, this threshold likely needs to be re-tuned for computers of significantly different processing power compared to the machine this was developed on.
- Audio simulation is extremely simple and only synchronises with the video if your computer is able to meet the timing requirements to run the simulation at 60 frames per second and you leave the performance counters timing enabled.
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail or is ignored.
- Controls and VSync are modifiable in the code, VSync currently seems to have little value as an option as the main loop is synchronised using the performance counters.

TODO:
- Test under linux, MAC and Android.
- Interface to the inbuilt loader to allow the uploading of user generated code.
- More robust timing for the simulation and the audio.
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


#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define VSYNC_START    -36
#define VSYNC_RATE      60
#define SCAN_LINES      521
#define HLINE_END       200
#define COLOUR_PALETTE  64
#define NUM_LEDS        4
#define TIMING_HACK     0.01615

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


uint8_t IN = 0xFF, XOUT = 0x00;
uint8_t ROM[ROM_SIZE][2], RAM[RAM_SIZE];
uint32_t Pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t Colours[COLOUR_PALETTE];
uint8_t AudioSamples[AUD_SAMPLES];
int32_t AudioIndex = 0;
SDL_AudioDeviceID AudioDevice = 1;


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

void initialise(CpuState& S, SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture)
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
                    case SDLK_b:     IN &= ~INPUT_B;       break;
                    case SDLK_a:     IN &= ~INPUT_A;       break;

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
                    case SDLK_RIGHT: IN |= INPUT_RIGHT;   break;    
                    case SDLK_LEFT:  IN |= INPUT_LEFT;    break;     
                    case SDLK_DOWN:  IN |= INPUT_DOWN;    break;     
                    case SDLK_UP:    IN |= INPUT_UP;      break;       
                    case SDLK_SPACE: IN |= INPUT_START;   break;    
                    case SDLK_s:     IN |= INPUT_SELECT;  break;   
                    case SDLK_b:     IN |= INPUT_B;       break;        
                    case SDLK_a:     IN |= INPUT_A;       break;        
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

		int address = int(float(SCREEN_WIDTH) * 0.9f) + i*NUM_LEDS;
        Pixels[address + 0] = colour;
        Pixels[address + 1] = colour;
        Pixels[address + 2] = colour;
	}
}

void updatePixel(const CpuState& S, int vgaX, int vgaY)
{
    Uint32 colour = Colours[S.OUT & (COLOUR_PALETTE-1)];
    Uint32 address = vgaX*3 + vgaY*SCREEN_WIDTH + 20;
    Pixels[address + 0] = colour;
    Pixels[address + 1] = colour;
    Pixels[address + 2] = colour;
}

void render(SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_UpdateTexture(texture, NULL, Pixels, SCREEN_WIDTH * sizeof(Uint32));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

    static uint64_t prevFrameCounter = 0;
    uint64_t frameCounter;
    for(;;)
    {
        frameCounter = SDL_GetPerformanceCounter();
        if(double(frameCounter - prevFrameCounter) / double(SDL_GetPerformanceFrequency()) >= TIMING_HACK) break;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    prevFrameCounter = frameCounter;
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
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    initialise(S, &window, &renderer, &texture);                   

    int vgaX=0, vgaY=0;
    for(long long t=-2; ; t++)
    {
	    if(t < 0) S.PC = 0; // MCP100 Power-On Reset

	    CpuState T = cpuCycle(S); // Update CPU

	    int hSync = (T.OUT & 0x40) - (S.OUT & 0x40); // "VGA monitor" (use simple stdout)
	    int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
        if(vSync < 0) // Falling vSync edge
        {
            vgaY = VSYNC_START;

            // Input and video
            input();
            updateLeds();
            render(renderer, texture);
        }
	    if(vgaX++ < HLINE_END)
        {
		    if(hSync) { }              // Visual indicator of hSync
		    else if(vgaX == HLINE_END) { }   // Too many pixels
		    else if(~S.OUT & 0x80) { } // Visualize vBlank pulse
		    else if(vgaX >=0  &&  vgaX < HLINE_END  &&  vgaY >= 0  &&  vgaY < SCREEN_HEIGHT)
            {
                updatePixel(S, vgaX, vgaY);
		    }
	    }
	    if(hSync > 0) // Rising hSync edge
        {
            XOUT = T.AC;
            
            // Audio
            fillAudioBuffer();
            if(vgaY == SCREEN_HEIGHT+4)
            {
                playAudioBuffer();
            }

		    vgaX = 0;
		    vgaY++;
		    T.undef = rand() & 0xff; // Change this once in a while
	    }

	    S=T;
    }

    return 0;
}
