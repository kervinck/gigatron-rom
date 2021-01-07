#include <fstream>

#include "cpu.h"
#include "memory.h"
#include "timing.h"
#include "audio.h"
#include "midi.h"
#include "gtmidi.h"

#include <SDL.h>


namespace Midi
{
    bool _hasVolume = false;

    uint8_t* _midiBase = nullptr;
    uint8_t* _midiStream = nullptr;

    uint8_t _midiDelay = 0;
    uint16_t _midiStreamSize = 0;


    uint8_t* getStream(void) {return _midiStream;}


    bool setStream(uint8_t* midiBuffer, uint16_t midiBufferSize)
    {
        Audio::initialiseChannels();

        GtMidiHdr* gtMidiHdr = (GtMidiHdr*)midiBuffer;
        if(gtMidiHdr == nullptr  ||  strncmp((char *)gtMidiHdr->_tag, GTMIDI_TAG_STRING, GTMIDI_TAG_SIZE) != 0)
        {
            _midiBase = nullptr;
            _midiStream = nullptr;
            return false;
        }

        _hasVolume = bool(gtMidiHdr->_hasVolume);
        _midiStreamSize = (gtMidiHdr->_hiSize <<8) + gtMidiHdr->_loSize;
        if(_midiStreamSize == 0  ||  _midiStreamSize != midiBufferSize - GTMIDI_STREAM_OFFSET)
        {
            _midiStream = nullptr;
            return false;
        }

        _midiBase = midiBuffer + GTMIDI_STREAM_OFFSET;
        _midiStream = midiBuffer + GTMIDI_STREAM_OFFSET;

        return true;
    }

    void stop(void)
    {
        _hasVolume = false;
        _midiBase = nullptr;
        _midiStream = nullptr;
        _midiDelay = 0;
        _midiStreamSize = 0;

        Audio::initialiseChannels();
    }

    void play(void)
    {
        if(_midiStream == nullptr) return;

        // Tick audio
        Cpu::setRAM(GIGA_SOUND_TIMER, 2);

        if(_midiDelay) _midiDelay--; 

        while(_midiDelay == 0)
        {
            if(_midiStream - _midiBase >= _midiStreamSize) _midiStream = _midiBase;

            uint8_t command = *_midiStream++;
            if(command & 0x80)
            {
                // Start note
                if((command & 0xF0) == 0x90)
                {
                    // Midi note converted to Giga note
                    uint8_t channel = command & GIGA_CHANNELS_MASK;  // spec supports up to 16 channels, Gigatron supports 4
                    uint16_t note = *_midiStream++;
                    note = (note - 11) * 2;
                    note = Cpu::getROM16(note + 0x0900, 1);
                    Cpu::setRAM16(GIGA_CH0_KEY_L + channel*GIGA_CHANNEL_OFFSET, note);

                    // Midi volume
                    if(_hasVolume)
                    {
                        uint8_t volume = *_midiStream++;
                        Cpu::setRAM(GIGA_CH0_WAV_A + channel*GIGA_CHANNEL_OFFSET, volume);
                    }
                }
                // Stop note
                else if((command & 0xF0) == 0x80)
                {
                    uint8_t channel = command & GIGA_CHANNELS_MASK;  // spec supports up to 16 channels, Gigatron supports 4
                    Cpu::setRAM16(GIGA_CH0_KEY_L + channel*GIGA_CHANNEL_OFFSET, 0x0000);
                }
                // Segment address
                else if((command & 0xF0) == 0xD0)
                {
                    uint16_t segment = *_midiStream++;
                    segment |= HI_MASK((*_midiStream++) <<8);
                    _midiStream = (uint8_t*)(uintptr_t)segment;
                }
            }
            // Delay n*16.66666667 milliseconds where n = 8bit value
            else
            {
                _midiDelay = command;
            }
        }
    }
}
