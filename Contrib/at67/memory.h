#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <vector>


#define ROM_SIZE (1<<16)
#define RAM_SIZE_LO (1<<15)
#define RAM_SIZE_HI (1<<16)

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

#define GTB_LINE0_ADDRESS     0x1BA0
#define MAX_GTB_LINE_SIZE     32
#define NUM_GTB_LINES_PER_ROW 3

#define LO_BYTE(a) (a & 0x00FF)
#define HI_BYTE(a) ((a >>8) & 0x00FF)
#define HI_MASK(a) (a & 0xFF00)
#define MAKE_ADDR(a, b) ((LO_BYTE(a) <<8) | LO_BYTE(b))

 
namespace Memory
{
    enum FitType {FitSmallest, FitLargest, FitAscending, NumFitTypes};
    enum RamType {RamVasm=0, RamVar, RamStr, RamArray, RamStack, NumRamTypes};

    struct RamEntry
    {
        uint16_t _address;
        int _size;
    };


    int getSizeRAM(void);
    int getBaseFreeRAM(void);
    int getFreeRAM(void);
    int getFreeGtbRAM(int numLines);

    void setFreeRAM(int freeRAM);

    void intitialise(void);

    bool getRAM(FitType fitType, RamType ramType, int size, uint16_t& address);
}

#endif