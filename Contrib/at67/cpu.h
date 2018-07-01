#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <string>


#define MAJOR_VERSION "0.7"
#define MINOR_VERSION "0"
#define VERSION_STR "gtemuSDL v" MAJOR_VERSION "." MINOR_VERSION
 
#define ROM_SIZE (1<<16)
#define RAM_SIZE (1<<16) // Can be 32k or 64k

#define ROM_INST 0
#define ROM_DATA 1

#define ROM_TITLE_ADDRESS 0xFEB1
#define MAX_TITLE_CHARS   25

#define BOOT_COUNT 0x0004
#define BOOT_CHECK 0x0005

#define RAM_USED_DEFAULT  19986 // ignores page 0, would be 19779 otherwise
#define ROM_VCPU_DISPATCH 0x0309

#if defined(_WIN32)
#define _EXIT_(f)   \
    system("pause");\
    exit(f);
#else
#define _EXIT_(f)   \
    system("read"); \
    exit(f);
#endif

// At least on Windows, _X is a constant defined somewhere before here
#ifdef _X
#undef _X
#endif


namespace Cpu
{
    enum ScanlineMode {Normal=0, VideoB, VideoC, VideoBC, NumScanlineModes};
    enum InternalGt1Id {SnakeGt1=0, RacerGt1=1, MandelbrotGt1=2, PicturesGt1=3, CreditsGt1=4, LoaderGt1=5, NumInternalGt1s};

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


    uint16_t getBaseFreeRAM(void);
    uint16_t getFreeRAM(void);
    uint8_t* getPtrToROM(int& romSize);

    void setFreeRAM(uint16_t freeRAM);

    void initialiseInternalGt1s(void);

    void patchSYS_Exec_88(void);
    void patchScanlineModeVideoB(void);
    void patchScanlineModeVideoC(void);
    void patchTitleIntoRom(const std::string& title);
    void patchSplitGt1IntoRom(const std::string& splitGt1path, const std::string& splitGt1name, uint16_t startAddress, InternalGt1Id gt1Id);

#ifndef STAND_ALONE
    int64_t getClock(void);
    uint8_t getIN(void);
    uint8_t getXOUT(void);
    uint8_t getRAM(uint16_t address);
    uint8_t getROM(uint16_t address, int page);
    uint16_t getRAM16(uint16_t address);
    uint16_t getROM16(uint16_t address, int page);
    float getvCpuUtilisation(void);

    void setClock(int64_t clock);
    void setIN(uint8_t in);
    void setXOUT(uint8_t xout);
    void setRAM(uint16_t address, uint8_t data);
    void setROM(uint16_t base, uint16_t address, uint8_t data);
    void setRAM16(uint16_t address, uint16_t data);
    void setROM16(uint16_t base, uint16_t address, uint16_t data);
    void setScanlineMode(ScanlineMode scanlineMode);

    void initialise(State& S);
    State cycle(const State& S);
    void reset(bool coldBoot=false);
    void vCpuUsage(State& S);
#endif
}

#endif