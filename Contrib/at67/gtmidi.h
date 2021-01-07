#ifndef GTMIDI_H
#define GTMIDI_H

#include <cstdint>

#define GTMIDI_TAG_SIZE  6
#define GTMIDI_NAME_SIZE 32

#define GTMIDI_TAG_HEADER {'g', 't', 'M', 'I', 'D', 'I'}
#define GTMIDI_TAG_STRING "gtMIDI"

struct GtMidiHdr
{
    uint8_t _tag[GTMIDI_TAG_SIZE] = GTMIDI_TAG_HEADER;
    uint8_t _name[GTMIDI_NAME_SIZE] = {0};
    uint8_t _hasVolume = 0;
    uint8_t _hiSize = 0;
    uint8_t _loSize = 0;
};

#define GTMIDI_TAG_OFFSET     offsetof(GtMidiHdr, _tag)
#define GTMIDI_NAME_OFFSET    offsetof(GtMidiHdr, _name)
#define GTMIDI_VOLUME_OFFSET  offsetof(GtMidiHdr, _hasVolume)
#define GTMIDI_HI_SIZE_OFFSET offsetof(GtMidiHdr, _hiSize)
#define GTMIDI_LO_SIZE_OFFSET offsetof(GtMidiHdr, _loSize)
#define GTMIDI_STREAM_OFFSET  (GTMIDI_LO_SIZE_OFFSET + sizeof(GtMidiHdr::_loSize))


#endif