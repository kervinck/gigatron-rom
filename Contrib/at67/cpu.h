#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <algorithm>


#define MAJOR_VERSION "0.8"
#define MINOR_VERSION "17"
#define VERSION_STR "gtemuAT67 v" MAJOR_VERSION "." MINOR_VERSION
 
#define ROM_INST 0
#define ROM_DATA 1

#define NUM_INT_ROMS 4

#define ROM_TITLE_ADDRESS 0xFEB1
#define MAX_TITLE_CHARS   25

#define BOOT_COUNT 0x0004
#define BOOT_CHECK 0x0005

#define STACK_POINTER 0x001C

#define VIDEO_MODE_D 0x000D
#define VIDEO_MODE_B 0x001F
#define VIDEO_MODE_C 0x0020

#define ROM_TYPE          0x0021
#define ROM_TYPE_MASK     0x00FC
#define ROM_VCPU_DISPATCH 0x0309

#if defined(_WIN32)
#define _EXIT_(f)   \
    system("pause");\
    exit(f);
#else
#define _EXIT_(f)  \
    system("read");\
    exit(f);
#endif

#if defined(_WIN32)
#define _PAUSE_     \
    system("pause");
#else
#define _PAUSE_                                          \
    fprintf(stderr, "Press any key to continue . . .\n");\
    system("read");
#endif

// At least on Windows, _X is a constant defined somewhere before here
#ifdef _X
#undef _X
#endif


namespace Cpu
{
    enum RomType {ROMERR=0x00, ROMv1=0x1c, ROMv2=0x20, ROMv3=0x28, ROMv4=0x38, DEVROM=0xf8};
    enum ScanlineMode {Normal=0, VideoB, VideoC, VideoBC, NumScanlineModes};
    enum InternalGt1Id {SnakeGt1=0, RacerGt1=1, MandelbrotGt1=2, PicturesGt1=3, CreditsGt1=4, LoaderGt1=5, NumInternalGt1s};
    enum Endianess {LittleEndian = 0x03020100ul, BigEndian = 0x00010203ul};

    struct State
    {
        uint16_t _PC;
        uint8_t _IR, _D, _AC, _X, _Y, _OUT, _undef;
    };

    struct InternalGt1
    {
        uint16_t _start;
        uint16_t _patch;
        uint16_t _string;
        uint8_t _length; // string length
    };


    uint8_t* getPtrToROM(int& romSize);
    RomType getRomType(void);

#ifdef _WIN32
    void restoreWin32Console(void);
#endif
    
    void loadRom(int index);
    void swapRom(void);

    Endianess getHostEndianess(void);
    void swapEndianess(uint16_t& value);
    void swapEndianess(uint32_t& value);
    void swapEndianess(uint64_t& value);

    void initialiseInternalGt1s(void);

    void patchSYS_Exec_88(void);
    void patchScanlineModeVideoB(void);
    void patchScanlineModeVideoC(void);
    void patchTitleIntoRom(const std::string& title);
    bool patchSplitGt1IntoRom(const std::string& splitGt1path, const std::string& splitGt1name, uint16_t startAddress, InternalGt1Id gt1Id);

#ifndef STAND_ALONE
    bool getIsInReset(void);
    State& getStateS(void);
    State& getStateT(void);
    int64_t getClock(void);
    uint8_t getIN(void);
    uint8_t getXOUT(void);
    uint16_t getVPC(void);
    uint8_t getRAM(uint16_t address);
    uint8_t getROM(uint16_t address, int page);
    uint16_t getRAM16(uint16_t address);
    uint16_t getROM16(uint16_t address, int page);
    float getvCpuUtilisation(void);

    void setIsInReset(bool isInReset);
    void setClock(int64_t clock);
    void setIN(uint8_t in);
    void setXOUT(uint8_t xout);
    void setRAM(uint16_t address, uint8_t data);
    void setROM(uint16_t base, uint16_t address, uint8_t data);
    void setRAM16(uint16_t address, uint16_t data);
    void setROM16(uint16_t base, uint16_t address, uint16_t data);
    void setRomType(void);

    void saveScanlineModes(void);
    void restoreScanlineModes(void);
    void swapScanlineMode(void);

    void initialise(void);
    void shutdown(void);
    void cycle(const State& S, State& T);
    void reset(bool coldBoot=false);
    void softReset(void);
    void swapMemoryModel(void);
    void vCpuUsage(const State& S, const State& T);
    void process(void);
#endif
}

#endif