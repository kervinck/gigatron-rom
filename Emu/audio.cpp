#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "cpu.h"
#include "audio.h"
#include "timing.h"

#include <SDL.h>


namespace Audio
{
    SDL_AudioDeviceID _audioDevice = 1;


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
            fprintf(stderr, "Error: failed to initialise SDL audio\n");
            exit(EXIT_FAILURE);
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
}