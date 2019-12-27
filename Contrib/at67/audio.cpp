#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "audio.h"
#include "timing.h"
#include "editor.h"
#include "midi/music.h"
#include "expression.h"
#include "inih/INIReader.h"

#include <SDL.h>


#define AUDIO_SAMPLES    (SCAN_LINES + 1)
#define AUDIO_FREQUENCY  (SCAN_LINES*VSYNC_RATE)


namespace Audio
{
    bool _realTimeAudio = true;

    SDL_AudioDeviceID _audioDevice = 1;

    int32_t _audioIndex = 0;
    uint16_t _audioSamples[AUDIO_SAMPLES] = {0};

    int _scoreIndex = 0;
    uint8_t* _score[] = {(uint8_t*)musicMidi00};
    uint8_t* _scorePtr = (uint8_t*)_score[_scoreIndex];

    INIReader _configIniReader;


    bool getRealTimeAudio(void) {return _realTimeAudio;}

    bool getKeyAsString(const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, std::string& result)
    {
        result = _configIniReader.Get(sectionString, iniKey, defaultKey);
        if(result == defaultKey) return false;
        Expression::strToUpper(result);
        return true;
    }

    void initialise(void)
    {
        // Loader config
        INIReader iniReader(AUDIO_CONFIG_INI);
        _configIniReader = iniReader;
        if(_configIniReader.ParseError() == 0)
        {
            // Parse Loader Keys
            enum Section {Audio};
            std::map<std::string, Section> section;
            section["Audio"] = Audio;
            for(auto sectionString : _configIniReader.Sections())
            {
                if(section.find(sectionString) == section.end())
                {
                    fprintf(stderr, "Loader::initialise() : INI file '%s' has bad Sections : reverting to default values.\n", AUDIO_CONFIG_INI);
                    break;
                }

                std::string result;
                switch(section[sectionString])
                {
                    case Audio:
                    {
                        getKeyAsString(sectionString, "RealTimeAudio", "1", result);   
                        _realTimeAudio = strtol(result.c_str(), nullptr, 10);
                    }
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "Loader::initialise() : couldn't find loader configuration INI file '%s' : reverting to default values.\n", AUDIO_CONFIG_INI);
        }

        SDL_AudioSpec audSpec;
        SDL_zero(audSpec);
        audSpec.freq = AUDIO_FREQUENCY;
        audSpec.format = AUDIO_S16;
        audSpec.channels = 1;

        if(SDL_OpenAudio(&audSpec, NULL) < 0)
        {
            Cpu::shutdown();
            fprintf(stderr, "Audio::initialise() : failed to initialise SDL audio\n");
            _EXIT_(EXIT_FAILURE);
        }
        SDL_PauseAudio(0);

        initialiseChannels();
    }

    void initialiseChannels(void)
    {
        for(int i=0; i<GIGA_SOUND_CHANNELS; i++)
        {
            Cpu::setRAM(GIGA_CH0_WAV_A + i*GIGA_CHANNEL_OFFSET, 0x00); // sample index modification for advanced noise generation
            Cpu::setRAM(GIGA_CH0_WAV_X + i*GIGA_CHANNEL_OFFSET, 0x03); // waveform index
            Cpu::setRAM(GIGA_CH0_KEY_L + i*GIGA_CHANNEL_OFFSET, 0x00); // low frequency look up from ROM
            Cpu::setRAM(GIGA_CH0_KEY_H + i*GIGA_CHANNEL_OFFSET, 0x00); // high frequency look up from ROM
            Cpu::setRAM(GIGA_CH0_OSC_L + i*GIGA_CHANNEL_OFFSET, 0x00); // low internal oscillator
            Cpu::setRAM(GIGA_CH0_OSC_H + i*GIGA_CHANNEL_OFFSET, 0x00); // high internal oscillator
        }

#if 0
        // Audio channels are byte interlaced
        for(int i=0; i<64; i++)
        {
            Cpu::setRAM(0x700+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
            Cpu::setRAM(0x701+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
            Cpu::setRAM(0x702+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
            Cpu::setRAM(0x703+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
        }
#endif
    }

    void fillAudioBuffer(void)
    {
        _audioSamples[_audioIndex] = (Cpu::getXOUT() & 0xf0) <<5;
        _audioIndex = (_audioIndex + 1) % AUDIO_SAMPLES;
    }

    void playAudioBuffer(void)
    {
        SDL_QueueAudio(_audioDevice, &_audioSamples[0], _audioIndex <<1);
        _audioIndex = 0;
    }

    void playSample(void)
    {
        uint16_t sample = (Cpu::getXOUT() & 0xf0) <<5;
        SDL_QueueAudio(_audioDevice, &sample, 2);
    }

    void clearQueue(void)
    {
        SDL_ClearQueuedAudio(_audioDevice);
    }


    void nextScore(void)
    {
        initialiseChannels();

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
            initialiseChannels();            
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
                    uint8_t channel = command & (GIGA_SOUND_CHANNELS - 1);  // spec supports up to 16 channels, Gigatron supports 4
                    uint16_t note = *_scorePtr++;
                    note = (note - 10) * 2 - 2;
                    note = Cpu::getROM16(note + 0x0900, 1);
                    Cpu::setRAM(GIGA_CH0_KEY_L + channel*GIGA_CHANNEL_OFFSET, uint8_t(LO_BYTE(note)));
                    Cpu::setRAM(GIGA_CH0_KEY_H + channel*GIGA_CHANNEL_OFFSET, uint8_t(HI_BYTE(note)));
                }
                // Stop note
                else if((command & 0xF0) == 0x80)
                {
                    uint8_t channel = command & (GIGA_SOUND_CHANNELS - 1);  // spec supports up to 16 channels, Gigatron supports 4
                    Cpu::setRAM(GIGA_CH0_KEY_L + channel*GIGA_CHANNEL_OFFSET, 0x00);
                    Cpu::setRAM(GIGA_CH0_KEY_H + channel*GIGA_CHANNEL_OFFSET, 0x00);
                }
                // Segment command
                else if((command & 0xF0) == 0xD0)
                {
                    uint16_t segment = *_scorePtr++;
                    segment |= HI_MASK((*_scorePtr++) <<8);
                    _scorePtr = (uint8_t*)(uintptr_t)segment;
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