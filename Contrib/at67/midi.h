#ifndef MIDI_H
#define MIDI_H

#include <cstdint>

#include "gtmidi.h"


#define MIDI_MAX_BUFFER_SIZE     0x10000
#define MIDI_MIN_SEGMENT_SIZE    9 // allows for 2 midi commands + segment command
#define MIDI_CMD_JMP_SEG_SIZE    3
#define MIDI_MAX_SOUND_CHANNELS  4
#define MIDI_MASK_SOUND_CHANNELS 0x03
#define MIDI_MIN_GIGA_NOTE       12
#define MIDI_MAX_GIGA_NOTE       106
#define MIDI_PERCUSSION_NOTES    128
#define MIDI_CMD_STOP_NOTE       0x80
#define MIDI_CMD_START_NOTE      0x90
#define MIDI_CMD_JMP_SEG         0xD0


namespace Midi
{
    bool loadFile(const std::string& filepath, uint8_t* midiBuffer, int& midiBufferSize, GtMidiHdr* gtMidiHdr=nullptr);

#ifndef STAND_ALONE
    uint8_t* getStream(void);

    bool setStream(const std::string* filenamePtr, uint8_t* midiBuffer, uint16_t midiBufferSize);

    void stop(void);
    void play(void);
    void pause(bool enable);
#endif
}

#endif