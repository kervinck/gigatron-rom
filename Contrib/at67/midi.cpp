#include <fstream>
#include <cstring>

#include "midi.h"

#ifndef STAND_ALONE
#include "cpu.h"
#include "memory.h"
#include "timing.h"
#include "audio.h"

#include <SDL.h>
#endif


namespace Midi
{
    bool loadFile(const std::string& filepath, uint8_t* midiBuffer, int& midiBufferSize, GtMidiHdr* gtMidiHdr)
    {
        // Read
        std::ifstream infile(filepath, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Midi::loadFile() : failed to open file '%s' for reading\n", filepath.c_str());
            return false;
        }

        // Process header if required
        uint16_t streamSize = 0;
        if(gtMidiHdr)
        {
            infile.read((char *)gtMidiHdr, GTMIDI_HDR_SIZE);
            if(infile.bad())
            {
                fprintf(stderr, "Midi::loadFile() : read error of header in file '%s'\n", filepath.c_str());
                return false;
            }
            if(strncmp((char *)gtMidiHdr->_tag, GTMIDI_TAG_STRING, GTMIDI_TAG_SIZE) != 0)
            {
                fprintf(stderr, "Midi::loadFile() : bad header tag in file '%s'\n", filepath.c_str());
                return false;
            }
            streamSize = (gtMidiHdr->_hiSize <<8) + gtMidiHdr->_loSize;
        }

        infile.read((char *)midiBuffer, MIDI_MAX_BUFFER_SIZE - 1);
        if(infile.bad())
        {
            fprintf(stderr, "Midi::loadFile() : read error of stream in file '%s'\n", filepath.c_str());
            return false;
        }

        midiBufferSize = int(infile.gcount());
        if(gtMidiHdr)
        {
            if(midiBufferSize != streamSize)
            {
                fprintf(stderr, "Midi::loadFile() : buffer stream size %d does not match header stream size %d in file '%s'\n", midiBufferSize, streamSize, filepath.c_str());
                return false;
            }
        }
        else
        {
            if(midiBufferSize < int(GTMIDI_STREAM_OFFSET))
            {
                fprintf(stderr, "Midi::loadFile() : malformed header in file '%s'\n", filepath.c_str());
                return false;
            }
        }
        if(midiBufferSize >= MIDI_MAX_BUFFER_SIZE)
        {
            fprintf(stderr, "Midi::loadFile() : midi data too large in file '%s'\n", filepath.c_str());
            return false;
        }

        return true;
    }

#ifndef STAND_ALONE
    bool _hasVolume = false;

    uint8_t* _midiBase = nullptr;
    uint8_t* _midiStream = nullptr;
    uint8_t* _midiPaused = nullptr;

    uint8_t _midiDelay = 0;
    uint16_t _midiStreamSize = 0;


    uint8_t* getStream(void) {return _midiStream;}

    bool setStream(const std::string* filenamePtr, uint8_t* midiBuffer, uint16_t midiBufferSize)
    {
        if(midiBuffer == nullptr) return false;

        Audio::initialiseChannels();

        GtMidiHdr* gtMidiHdr = (GtMidiHdr*)midiBuffer;
        if(strncmp((char *)gtMidiHdr->_tag, GTMIDI_TAG_STRING, GTMIDI_TAG_SIZE) != 0)
        {
            _midiBase = nullptr;
            _midiStream = nullptr;
            _midiPaused = nullptr;
            if(filenamePtr) fprintf(stderr, "Midi::setStream() : malformed midi header in file '%s'\n", filenamePtr->c_str());
            return false;
        }

        _hasVolume = bool(gtMidiHdr->_hasVolume);
        _midiStreamSize = (gtMidiHdr->_hiSize <<8) + gtMidiHdr->_loSize;
        if(_midiStreamSize == 0  ||  _midiStreamSize != midiBufferSize - GTMIDI_STREAM_OFFSET)
        {
            _midiBase = nullptr;
            _midiStream = nullptr;
            _midiPaused = nullptr;
            if(filenamePtr) fprintf(stderr, "Midi::setStream() : malformed midi header in file '%s'\n", filenamePtr->c_str());
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

    void pause(bool enable)
    {
        if(enable)
        {
            _midiPaused = _midiStream;
            _midiStream = nullptr;
        }
        else
        {
            _midiStream = _midiPaused;
            _midiPaused = nullptr;
        }
    }

    void play(void)
    {
        if(_midiStream == nullptr) return;

        // Tick audio
        Cpu::setRAM(GIGA_SOUND_TIMER, 2);

        if(_midiDelay) _midiDelay--; 

        while(_midiDelay == 0)
        {
            if(_midiStream - _midiBase >= _midiStreamSize)
            {
                _midiStream = nullptr; //_midiBase;
                break;
            }

            uint8_t command = *_midiStream++;
            if(command & 0x80)
            {
                // Start note
                if((command & 0xF0) == MIDI_CMD_START_NOTE)
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
                else if((command & 0xF0) == MIDI_CMD_STOP_NOTE)
                {
                    uint8_t channel = command & GIGA_CHANNELS_MASK;  // spec supports up to 16 channels, Gigatron supports 4
                    Cpu::setRAM16(GIGA_CH0_KEY_L + channel*GIGA_CHANNEL_OFFSET, 0x0000);
                }
                // New segment address
                else if((command & 0xF0) == MIDI_CMD_JMP_SEG)
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
#endif
}
