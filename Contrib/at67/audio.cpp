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
        if(Timing::getFrameTime()) ratio = std::max(1.0, VSYNC_TIMING_60 / std::min(Timing::getFrameTime(), VSYNC_TIMING_60));

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
        Cpu::setRAM(GIGA_CH0_WAV_A, 0x00);
        Cpu::setRAM(GIGA_CH0_WAV_X, 0x03);
        Cpu::setRAM(GIGA_CH0_KEY_L, 0x00);
        Cpu::setRAM(GIGA_CH0_KEY_H, 0x00);
        Cpu::setRAM(GIGA_CH0_OSC_L, 0x00);
        Cpu::setRAM(GIGA_CH0_OSC_H, 0x00);

        Cpu::setRAM(GIGA_CH1_WAV_A, 0x00);
        Cpu::setRAM(GIGA_CH1_WAV_X, 0x03);
        Cpu::setRAM(GIGA_CH1_KEY_L, 0x00);
        Cpu::setRAM(GIGA_CH1_KEY_H, 0x00);
        Cpu::setRAM(GIGA_CH1_OSC_L, 0x00);
        Cpu::setRAM(GIGA_CH1_OSC_H, 0x00);

        Cpu::setRAM(GIGA_CH2_WAV_A, 0x00);
        Cpu::setRAM(GIGA_CH2_WAV_X, 0x03);
        Cpu::setRAM(GIGA_CH2_KEY_L, 0x00);
        Cpu::setRAM(GIGA_CH2_KEY_H, 0x00);
        Cpu::setRAM(GIGA_CH2_OSC_L, 0x00);
        Cpu::setRAM(GIGA_CH2_OSC_H, 0x00);

        Cpu::setRAM(GIGA_CH3_WAV_A, 0x00);
        Cpu::setRAM(GIGA_CH3_WAV_X, 0x03);
        Cpu::setRAM(GIGA_CH3_KEY_L, 0x00);
        Cpu::setRAM(GIGA_CH3_KEY_H, 0x00);
        Cpu::setRAM(GIGA_CH3_OSC_L, 0x00);
        Cpu::setRAM(GIGA_CH3_OSC_H, 0x00);
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

        static int16_t midiDelay = 0;
        static uint64_t prevFrameCounter = 0;
        double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
        if(frameTime > VSYNC_TIMING_60)
        {
            prevFrameCounter = SDL_GetPerformanceCounter();        
            if(midiDelay)
            {
                midiDelay--; 
                frameTime = 0.0;

                // Start audio
                Cpu::setRAM(GIGA_SOUND_TIMER, 0x01);
            }
        }

        while(midiDelay == 0)
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
                    Cpu::setRAM(GIGA_CH0_KEY_L + channel*0x0100, uint8_t(note & 0x00FF));
                    Cpu::setRAM(GIGA_CH0_KEY_H + channel*0x0100, uint8_t((note & 0xFF00)>>8));
                }
                // Stop note
                else if((command & 0xF0) == 0x80)
                {
                    uint8_t channel = command & 0x03;  // spec supports up to 16 channels, we support 4
                    Cpu::setRAM(GIGA_CH0_KEY_L + channel*0x0100, 0x00);
                    Cpu::setRAM(GIGA_CH0_KEY_H + channel*0x0100, 0x00);
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
                midiDelay = command;
            }
        }
    }
}