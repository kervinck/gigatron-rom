#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <vector>


#define RAM_USED_DEFAULT  19986 // ignores page 0, would be 19779 otherwise

#define RAM_PAGE_START_0  0x0200
#define RAM_PAGE_START_1  0x0300
#define RAM_PAGE_START_2  0x0400
#define RAM_PAGE_START_3  0x0500
#define RAM_PAGE_SIZE_0   248
#define RAM_PAGE_SIZE_1   248
#define RAM_PAGE_SIZE_2   248
#define RAM_PAGE_SIZE_3   256

#define RAM_STACK_START  0x0600
#define RAM_STACK_SIZE   256

#define RAM_VIDEO_START  0x0800
#define RAM_VIDEO_END    0x7F00
#define RAM_VIDEO_SIZE   160

#define RAM_SEGMENTS_START  0x08A0
#define RAM_SEGMENTS_END    0x6FA0
#define RAM_SEGMENTS_OFS    0x0100
#define RAM_SEGMENTS_SIZE   96


#define RAM_EXPANSION_START  0x8000
#define RAM_EXPANSION_SIZE   0x8000


namespace Memory
{
    enum FitType {FitSmallest, FitLargest, FitAscending, NumFitTypes};
    enum RamType {RamVasm=0, RamVar, RamStr, RamArray, RamStack, NumRamTypes};

    struct RamEntry
    {
        uint16_t _address;
        uint16_t _size;
    };


    void intitialise(void);

    bool getRam(FitType fitType, RamType ramType, uint16_t size, uint16_t& address);
}

#endif