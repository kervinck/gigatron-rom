#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

#include "cpu.h"
#include "audio.h"
#include "timing.h"
#include "editor.h"
#include "../../../midi/scores/music.h"

#include <SDL.h>


namespace Audio
{
    int _scoreIndex = 0;
    SDL_AudioDeviceID _audioDevice = 1;

    uint8_t* _score[] = {(uint8_t*)musicMidi00};
    uint8_t* _scorePtr = (uint8_t*)_score[_scoreIndex];


    void initialise(void)
    {
        SDL_AudioSpec wanted;
        SDL_zero(wanted);
        wanted.freq = SCAN_LINES*VSYNC_RATE;
        //wanted.format = AUDIO_U16;
        wanted.format = AUDIO_U8;
        wanted.channels = 1;
        //wanted.samples = 1; //521;
        //_audioDevice = SDL_OpenAudioDevice(NULL, 0, &wanted, NULL, 0);
        //if(_audioDevice == 0)

        if(SDL_OpenAudio(&wanted, NULL) < 0)
        {
            SDL_Quit();
            fprintf(stderr, "Audio::initialise() : failed to initialise SDL audio\n");
            _EXIT_(EXIT_FAILURE);
        }
        SDL_PauseAudio(0);
    }

    void playSample(void)
    {
        double ratio = 1.0;
        if(Timing::getFrameTime()) ratio = std::max(1.0, TIMING_HACK / std::min(Timing::getFrameTime(), TIMING_HACK));

        static double skip = 0.0;
        uint64_t count = uint64_t(skip);
        skip += 1.0 / ratio;
        if(uint64_t(skip) > count)
        {
            uint8_t sample = Cpu::getXOUT() & 0xF0;
            SDL_QueueAudio(_audioDevice, &sample, 1);
        }
    }

    void resetChannels(void)
    {
        for(int i=0x0000; i<0x0400; i+=0x0100)
        {
            Cpu::setRAM(0x01FA + i, 0x00);
            Cpu::setRAM(0x01FB + i, 0x03);
            Cpu::setRAM(0x01FC + i, 0x00);
            Cpu::setRAM(0x01FD + i, 0x00);
            Cpu::setRAM(0x01FE + i, 0x00);
            Cpu::setRAM(0x01FF + i, 0x00);
        }
    }

    void nextScore(void)
    {
        resetChannels();

        if(++_scoreIndex >= 1) _scoreIndex = 0;
        _scorePtr = (uint8_t*)_score[_scoreIndex];
    }

    void playMusic(void)
    {
        if(!Editor::getStartMusic()) return;

        static bool firstTime = true;
        if(firstTime == true)
        {
            firstTime = false;
            resetChannels();            

            // Signed -31 to +31 sine wave
            //for(int i=0; i<64; i++)
            //{
            //    Cpu::setRAM(0x700+i, int8_t(sinf(float(i) / 64.0f * 2.0f* 3.141529f)*31.0f));
            //}
        }

        static int16_t scoreDelay = 0;
        static uint64_t prevFrameCounter = 0;
        static double frameTime = 0.0;

        uint64_t frameCounter = SDL_GetPerformanceCounter();
        frameTime += double(frameCounter - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
        prevFrameCounter = frameCounter;
        
        if(scoreDelay  &&  frameTime * 1000.0 > 16.666666667)
        {
            scoreDelay--; 
            frameTime = 0.0;

            // Start audio
            Cpu::setRAM(0x002C, 0x01);
        }
        
        while(scoreDelay == 0)
        {
            uint8_t command = *_scorePtr++;
            if(command & 0x80)
            {
                // Start note
                if((command & 0xF0) == 0x90)
                {
                    uint8_t channel = command & 0x03;  // spec supports up to 16 channels, we support 4
                    uint16_t note = *_scorePtr++;
                    note = (note - 10) * 2 - 2;
                    note = Cpu::getROM16(note + 0x0900, 1);
                    Cpu::setRAM(0x01FC + channel*0x0100, uint8_t(note & 0x00FF));
                    Cpu::setRAM(0x01FD + channel*0x0100, uint8_t((note & 0xFF00)>>8));
                }
                // Stop note
                else if((command & 0xF0) == 0x80)
                {
                    uint8_t channel = command & 0x03;  // spec supports up to 16 channels, we support 4
                    Cpu::setRAM(0x01FC + channel*0x0100, 0x00);
                    Cpu::setRAM(0x01FD + channel*0x0100, 0x00);
                }
                // Stop score
                else if((command & 0xF0) == 0xD0)
                {
                    //uint16_t segment = *_scorePtr++;
                    //segment = segment | (((*_scorePtr++) <<8) & 0xFF00);
                    _scorePtr = (uint8_t*)musicMidi00;
                }
            }
            // Delay n milliseconds where n = 8bit value
            else
            {
                scoreDelay = command;
            }
        }
    }
}