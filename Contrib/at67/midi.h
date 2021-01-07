#ifndef MIDI_H
#define MIDI_H

#include <cstdint>

#define MIDI_MAX_BUFFER_SIZE     0x10000
#define MIDI_MAX_SOUND_CHANNELS  4
#define MIDI_MASK_SOUND_CHANNELS 0x03
#define MIDI_MIN_GIGA_NOTE       12
#define MIDI_MAX_GIGA_NOTE       106
#define MIDI_PERCUSSION_NOTES    128


namespace Midi
{
    uint8_t* getStream(void);
    bool setStream(uint8_t* midiBuffer, uint16_t midiBufferSize);

    void stop(void);
    void play(void);
}

#endif