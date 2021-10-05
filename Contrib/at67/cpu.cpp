#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "memory.h"
#include "assembler.h"
#include "cpu.h"
#include "spi.h"

#ifndef STAND_ALONE
#include <SDL.h>
#include "audio.h"
#include "loader.h"
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "gigatron_0x1c.h"
#include "gigatron_0x20.h"
#include "gigatron_0x28.h"
#include "gigatron_0x38.h"
#include "gigatron_0x40.h"
#endif

#ifdef _WIN32
#include <Windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif


namespace Cpu
{
    class RAM_s {
        size_t   size;
        uint16_t ctrl;
        uint8_t  xin;
        uint8_t  bank;
        uint8_t  ram[4][0x8000];

    public:
        RAM_s(size_t size) {
            resize(size);}
        const uint8_t& operator[](uint16_t address) const {
            if (! address && (ctrl & 1))  return xin;
            return const_cast<RAM_s*>(this)->operator[](address); }
        uint8_t& operator[](uint16_t address) {
            if ((!(ctrl & 0x20)) && (address & 0x7f80) == 0x80)  address ^= 0x8000;
            return ram[(address & 0x8000) ? bank : 0][address &0x7fff]; }
        bool has_extension() {
            return size==0x20000;}
        void setctrl(uint16_t c) {
            if (has_extension()) {
                c &= 0x80fd; Spi::clock(ctrl, c);
                ctrl = c; bank = (c & 0xC0) >> 6; } }
        uint16_t getctrl() { return ctrl; }
        void setxin(uint8_t x) {xin=x;}
        uint8_t getxin() { return xin; }
        void resize(size_t s) {
            bank=(s>=0x10000)?1:0; ctrl=uint16_t((bank<<6)|0x3c); size=s;
            if (size!=0x8000 && size!=0x10000 && size!=0x20000) abort(); }
        uint8_t get(uint32_t addr) {return ram[(addr>>15)&3][addr&0x7fff];}
        void set(uint32_t addr,uint8_t data) {ram[(addr>>15)&3][addr&0x7fff]=data;}
    } _RAM(0x8000);

    const RAM_s &_cRAM = _RAM;

    const uint8_t _endianBytes[] = {0x00, 0x01, 0x02, 0x03};

    int _numRoms = 0;
    int _romIndex = 0;
    int _vCpuInstPerFrame = 0;
    int _vCpuInstPerFrameMax = 0;
    float _vCpuUtilisation = 0.0;

    bool _coldBoot = true;
    bool _isInReset = false;
    bool _checkRomType = true;
    bool _debugging = false;
    bool _initAudio = true;
    bool _consoleSaveFile = true;

    uint8_t _ROM[ROM_SIZE][2];
    std::vector<uint8_t*> _romFiles;
    RomType _romType = ROMERR;
    std::map<std::string, RomType> _romTypeMap = {{"ROMV1", ROMv1}, {"ROMV2", ROMv2}, {"ROMV3", ROMv3}, {"ROMV4", ROMv4}, {"ROMV5A", ROMv5a}, {"SDCARD", SDCARD}, {"DEVROM", DEVROM}};
    std::map<RomType, std::string> _romTypeStr = {{ROMv1, "ROMv1"}, {ROMv2, "ROMv2"}, {ROMv3, "ROMv3"}, {ROMv4, "ROMv4"}, {ROMv5a, "ROMv5A"}, {SDCARD, "SDCARD"}, {DEVROM, "DEVROM"}};

    std::vector<uint8_t> _scanlinesRom0;
    std::vector<uint8_t> _scanlinesRom1;
    int _scanlineMode = ScanlineMode::Normal;

    std::vector<InternalGt1> _internalGt1s;


    int getNumRoms(void) {return _numRoms;}
    int getRomIndex(void) {return _romIndex;}

    uint8_t* getPtrToROM(int& romSize) {romSize = sizeof(_ROM); return (uint8_t*)_ROM;}
    RomType getRomType(void) {return _romType;}
    std::map<std::string, RomType>& getRomTypeMap(void) {return _romTypeMap;}

    bool getRomTypeStr(RomType romType, std::string& romTypeStr)
    {
        if(_romTypeStr.find(romType) == _romTypeStr.end())
        {
            romTypeStr = "";
            return false;
        }

        romTypeStr = _romTypeStr[romType];

        return true;
    }


//#define COLLECT_INST_STATS
#if defined(COLLECT_INST_STATS)
    struct InstCount
    {
        uint8_t _inst = 0;
        uint64_t _count = 0;
    };

    uint64_t _totalCount = 0;
    float _totalPercent = 0.0f;
    std::vector<InstCount> _instCounts(256);

    void displayInstCounts(void)
    {
        std::sort(_instCounts.begin(), _instCounts.end(), [](const InstCount& a, const InstCount& b)
        {
            return (a._count > b._count);
        });

        for(int i=0; i<<int(_instCounts.size()); i++)
        {
            float percent = float(_instCounts[i]._count)/float(_totalCount)*100.0f;
            if(percent > 1.0f)
            {
                _totalPercent += percent;
                fprintf(stderr, "inst:%02x count:%012lld %.1f%%\n", _instCounts[i]._inst, _instCounts[i]._count, percent);
            }
        }

        fprintf(stderr, "Total instructions:%lld\n", _totalCount);
        fprintf(stderr, "Total percentage:%f\n", _totalPercent);
    }
#endif

#ifdef _WIN32
    void enableWin32ConsoleSaveFile(bool consoleSaveFile)
    {
        _consoleSaveFile = consoleSaveFile;
    }
#endif

    Endianness getHostEndianness(void)
    {
        return *((Endianness*)_endianBytes);
    }

    void swapEndianness(uint16_t& value)
    {
        value = (value >>8)  |  (value <<8);
    }

    void swapEndianness(uint32_t& value)
    {
        value = (value >>24)  |  ((value >>8) & 0x0000FF00)  |  ((value <<8) & 0x00FF0000)  |  (value <<24);
    }

    void swapEndianness(uint64_t& value)
    {
        value = (value >>56)  |  ((value >>40) & 0x000000000000FF00LL)  |  ((value >>24) & 0x0000000000FF0000LL)  |  ((value >>8) & 0x00000000FF000000LL)  |
        ((value <<8) & 0x000000FF00000000LL)  |  ((value <<24) & 0x0000FF0000000000LL)  |  ((value <<40) & 0x00FF000000000000LL)  |  (value <<56);
    }

    void initialiseInternalGt1s(void)
    {
        InternalGt1 internalGt1Snake = {0xE39C, 0xFDB1, 0xFC89, 5};
        _internalGt1s.push_back(internalGt1Snake);

        InternalGt1 internalGt1Racer = {0xEA2C, 0xFDBB, 0xFC90, 5};
        _internalGt1s.push_back(internalGt1Racer);

        InternalGt1 internalGt1Mandelbrot = {0xF16E, 0xFDC5, 0xFC97, 10};
        _internalGt1s.push_back(internalGt1Mandelbrot);

        InternalGt1 internalGt1Pictures = {0xF655, 0xFDCF, 0xFCA3, 8};
        _internalGt1s.push_back(internalGt1Pictures);

        InternalGt1 internalGt1Credits = {0xF731, 0xFDD9, 0xFCAD, 7};
        _internalGt1s.push_back(internalGt1Credits);

        InternalGt1 internalGt1Loader = {0xF997, 0xFDE3, 0xFCB6, 6};
        _internalGt1s.push_back(internalGt1Loader);
    }

    void patchSYS_Exec_88(void)
    {
        _ROM[0x00AD][ROM_INST] = 0x00;
        _ROM[0x00AD][ROM_DATA] = 0x00;

        _ROM[0x00AF][ROM_INST] = 0x00;
        _ROM[0x00AF][ROM_DATA] = 0x67;

        _ROM[0x00B5][ROM_INST] = 0xDC;
        _ROM[0x00B5][ROM_DATA] = 0xCF;

        _ROM[0x00B6][ROM_INST] = 0x80;
        _ROM[0x00B6][ROM_DATA] = 0x23;

        _ROM[0x00BB][ROM_INST] = 0x80;
        _ROM[0x00BB][ROM_DATA] = 0x00;
    }

    void patchScanlineModeVideoB(void)
    {
        _ROM[0x01C2][ROM_INST] = 0x14;
        _ROM[0x01C2][ROM_DATA] = 0x01;

        _ROM[0x01C9][ROM_INST] = 0x01;
        _ROM[0x01C9][ROM_DATA] = 0x09;

        _ROM[0x01CA][ROM_INST] = 0x90;
        _ROM[0x01CA][ROM_DATA] = 0x01;

        _ROM[0x01CB][ROM_INST] = 0x01;
        _ROM[0x01CB][ROM_DATA] = 0x0A;

        _ROM[0x01CC][ROM_INST] = 0x8D;
        _ROM[0x01CC][ROM_DATA] = 0x00;

        _ROM[0x01CD][ROM_INST] = 0xC2;
        _ROM[0x01CD][ROM_DATA] = 0x0A;

        _ROM[0x01CE][ROM_INST] = 0x00;
        _ROM[0x01CE][ROM_DATA] = 0xD4;

        _ROM[0x01CF][ROM_INST] = 0xFC;
        _ROM[0x01CF][ROM_DATA] = 0xFD;

        _ROM[0x01D0][ROM_INST] = 0xC2;
        _ROM[0x01D0][ROM_DATA] = 0x0C;

        _ROM[0x01D1][ROM_INST] = 0x02;
        _ROM[0x01D1][ROM_DATA] = 0x00;

        _ROM[0x01D2][ROM_INST] = 0x02;
        _ROM[0x01D2][ROM_DATA] = 0x00;

        _ROM[0x01D3][ROM_INST] = 0x02;
        _ROM[0x01D3][ROM_DATA] = 0x00;
    }

    void patchScanlineModeVideoC(void)
    {
        _ROM[0x01DA][ROM_INST] = 0xFC;
        _ROM[0x01DA][ROM_DATA] = 0xFD;

        _ROM[0x01DB][ROM_INST] = 0xC2;
        _ROM[0x01DB][ROM_DATA] = 0x0C;

        _ROM[0x01DC][ROM_INST] = 0x02;
        _ROM[0x01DC][ROM_DATA] = 0x00;

        _ROM[0x01DD][ROM_INST] = 0x02;
        _ROM[0x01DD][ROM_DATA] = 0x00;

        _ROM[0x01DE][ROM_INST] = 0x02;
        _ROM[0x01DE][ROM_DATA] = 0x00;
    }

    void patchTitleIntoRom(const std::string& title)
    {
        int minLength = std::min(int(title.size()), MAX_TITLE_CHARS);
        for(int i=0; i<minLength; i++) _ROM[ROM_TITLE_ADDRESS + i][ROM_DATA] = title[i];
        for(int i=minLength; i<MAX_TITLE_CHARS; i++) _ROM[ROM_TITLE_ADDRESS + i][ROM_DATA] = ' ';
    }

    char filebuffer[RAM_SIZE_HI];
    bool patchSplitGt1IntoRom(const std::string& splitGt1path, const std::string& splitGt1name, uint16_t startAddress, InternalGt1Id gt1Id)
    {
        std::streampos filelength = 0;

        // Instruction ROM
        std::ifstream romfile_ti(splitGt1path + "_ti", std::ios::binary | std::ios::in);
        if(!romfile_ti.is_open())
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to open %s ROM file.\n", std::string(splitGt1path + "_ti").c_str());
            return false;
        }
        romfile_ti.seekg (0, romfile_ti.end); filelength = romfile_ti.tellg(); romfile_ti.seekg (0, romfile_ti.beg);
        if(filelength > RAM_SIZE_HI)
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : ROM file %s must be less than %d in size.\n", std::string(splitGt1path + "_ti").c_str(), RAM_SIZE_HI);
            return false;
        }
        romfile_ti.read(filebuffer, filelength);
        if(romfile_ti.eof() || romfile_ti.bad() || romfile_ti.fail())
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to read %s ROM file.\n", std::string(splitGt1path + "_ti").c_str());
            return false;
        }
        for(int i=0; i<int(filelength); i++) _ROM[startAddress + i][ROM_INST] = filebuffer[i];

        // Data ROM
        std::ifstream romfile_td(splitGt1path + "_td", std::ios::binary | std::ios::in);
        if(!romfile_td.is_open())
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to open %s ROM file.\n", std::string(splitGt1path + "_td").c_str());
            return false;
        }
        romfile_td.seekg (0, romfile_td.end); filelength = romfile_td.tellg(); romfile_td.seekg (0, romfile_td.beg);
        if(filelength > RAM_SIZE_HI)
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : ROM file %s must be less than %d in size.\n", std::string(splitGt1path + "_td").c_str(), RAM_SIZE_HI);
            return false;
        }
        romfile_td.read(filebuffer, filelength);
        if(romfile_td.eof() || romfile_td.bad() || romfile_td.fail())
        {
            fprintf(stderr, "Cpu::patchSplitGt1IntoRom() : failed to read %s ROM file.\n", std::string(splitGt1path + "_td").c_str());
            return false;
        }
        for(int i=0; i<int(filelength); i++) _ROM[startAddress + i][ROM_DATA] = filebuffer[i];

        // Replace internal gt1 menu option with split gt1
        _ROM[_internalGt1s[gt1Id]._patch + 0][ROM_DATA] = LO_BYTE(startAddress);
        _ROM[_internalGt1s[gt1Id]._patch + 1][ROM_DATA] = HI_BYTE(startAddress);

        // Replace internal gt1 menu option name with split gt1 name
        int minLength = std::min(uint8_t(splitGt1name.size()), _internalGt1s[gt1Id]._length);
        for(int i=0; i<minLength; i++) _ROM[_internalGt1s[gt1Id]._string + i][ROM_DATA] = splitGt1name[i];
        for(int i=minLength; i<_internalGt1s[gt1Id]._length; i++) _ROM[_internalGt1s[gt1Id]._string + i][ROM_DATA] = ' ';

        return true;
    }


#ifndef STAND_ALONE
    int _vgaX = 0, _vgaY = 0;
    int _hSync = 0, _vSync = 0;
    int64_t _clockStall = CLOCK_RESET;
    int64_t _clock = CLOCK_RESET;
    uint8_t _IN = 0xFF, _XOUT = 0x00;
    State _stateS, _stateT;
    vCpuPc _vPC;

#ifdef _WIN32
    HWND _consoleWindowHWND;
#endif

    bool getColdBoot(void) {return _coldBoot;}
    bool getIsInReset(void) {return _isInReset;}
    State& getStateS(void) {return _stateS;}
    State& getStateT(void) {return _stateT;}
    int64_t getClock(void) {return _clock;}
    uint8_t getIN(void) {return _IN;}
    uint8_t getXOUT(void) {return _XOUT;}
    uint16_t getCTRL(void) {return _RAM.getctrl();}
    uint8_t getXIN(void) {return _RAM.getxin();}
    uint16_t getVPC(void) {return _vPC.first;}
    uint16_t getOldVPC(void) {return _vPC.second;}
    uint8_t getRAM(uint16_t address) {return _cRAM[address];}
    uint8_t getXRAM(uint32_t address) {return _RAM.get(address);}
    uint8_t getROM(uint16_t address, int page) {return _ROM[address & (ROM_SIZE-1)][page & 0x01];}
    uint16_t getRAM16(uint16_t address) {return _cRAM[address] | (_cRAM[address+1]<<8);}
    uint16_t getXRAM16(uint32_t address) {return _RAM.get(address) | (_RAM.get(address+1)<<8);}
    uint16_t getROM16(uint16_t address, int page) {return _ROM[address & (ROM_SIZE-1)][page & 0x01] | (_ROM[(address+1) & (ROM_SIZE-1)][page & 0x01]<<8);}
    float getvCpuUtilisation(void) {return _vCpuUtilisation;}

    void setOldVPC(uint16_t oldVPC) {_vPC.second = oldVPC;}
    void setColdBoot(bool coldBoot) {_coldBoot = coldBoot;}
    void setIsInReset(bool isInReset) {_isInReset = isInReset;}
    void setClock(int64_t clock) {_clock = clock;}
    void setIN(uint8_t in) {_IN = in;}
    void setXOUT(uint8_t xout) {_XOUT = xout;}
    void setCTRL(uint16_t ctrl) {_RAM.setctrl(ctrl);}
    void setXIN(uint8_t xin) {_RAM.setxin(xin);}

    void setRAM(uint16_t address, uint8_t data)
    {
        // Constant "0" and "1" are stored here
        if(address == ZERO_CONST_ADDRESS  &&  data != 0x00) {fprintf(stderr, "Cpu::setRAM() : Warning writing to address : 0x%04x : 0x%02x\n", address, data); return;}
        if(address == ONE_CONST_ADDRESS   &&  data != 0x01) {fprintf(stderr, "Cpu::setRAM() : Warning writing to address : 0x%04x : 0x%02x\n", address, data); return;}
        _RAM[address] = data;
    }

    void setXRAM(uint32_t address, uint8_t data)
    {
        if (address < 0x8000)
           setRAM((uint16_t)address, data);
        else
           _RAM.set(address, data);
    }

    void setROM(uint16_t base, uint16_t address, uint8_t data)
    {
        uint16_t offset = (address - base) / 2;
        _ROM[base + offset][address & 0x01] = data;
    }

    void setRAM16(uint16_t address, uint16_t data)
    {
        setRAM(address, uint8_t(LO_BYTE(data)));
        setRAM(address+1, uint8_t(HI_BYTE(data)));
    }

    void setXRAM16(uint32_t address, uint16_t data)
    {
        setXRAM(address, uint8_t(LO_BYTE(data)));
        setXRAM(address+1, uint8_t(HI_BYTE(data)));
    }

    void setSizeRAM(int size)
    {
        _RAM.resize(size_t(size));
    }

    void clearUserRAM(void)
    {
        // Not great for the Gigatron's RNG, will statistically bias it's results
        for(uint16_t addr=RAM_PAGE_START_0; addr<RAM_PAGE_START_0+RAM_PAGE_SIZE_0; addr++) setRAM(addr, 0x00);
        for(uint16_t addr=RAM_PAGE_START_1; addr<RAM_PAGE_START_1+RAM_PAGE_SIZE_1; addr++) setRAM(addr, 0x00);
        for(uint16_t addr=RAM_PAGE_START_2; addr<RAM_PAGE_START_2+RAM_PAGE_SIZE_2; addr++) setRAM(addr, 0x00);
        for(uint16_t addr=RAM_PAGE_START_3; addr<RAM_PAGE_START_3+RAM_PAGE_SIZE_3; addr++) setRAM(addr, 0x00);
        for(uint16_t addr=RAM_PAGE_START_4; addr<RAM_PAGE_START_4+RAM_PAGE_SIZE_4; addr++) setRAM(addr, 0x00);

        for(uint16_t addr=RAM_SEGMENTS_START; addr<=RAM_SEGMENTS_END; addr+=RAM_SEGMENTS_OFS)
        {
            for(uint16_t offs=0; offs<RAM_SEGMENTS_SIZE; offs++) setRAM(addr+offs, 0x00);
        }
    }

    void setROM16(uint16_t base, uint16_t address, uint16_t data)
    {
        uint16_t offset = (address - base) / 2;
        _ROM[base + offset][address & 0x01] = uint8_t(LO_BYTE(data));
        _ROM[base + offset][(address+1) & 0x01] = uint8_t(HI_BYTE(data));
    }

    void setRomType(void)
    {
        if(!_checkRomType) return;
        _checkRomType = false;

        uint8_t romType = ROMERR;
#if 1
        // This is the correct way to do it
        romType = getRAM(ROM_TYPE) & ROM_TYPE_MASK;
#else
        // Directly read from hard coded addresses in ROM, because RAM can be corrupted in emulation mode and resets to 
        // correct the corruption are not guaranteed, (unlike real hardware)
        if(getROM(0x009A, 1) == 0x1C)
        {
            romType = ROMv1;
        }
        else if(getROM(0x0098, 1) == 0x20)
        {
            romType = ROMv2;
        }
        else if(getROM(0x0098, 1) == 0x28)
        {
            romType = ROMv3;
        }
        else if(getROM(0x007E, 1) == 0x38)
        {
            romType = ROMv4;
        }
        else if(getROM(0x005E, 1) == 0x40)
        {
            romType = ROMv5a;
        }
        else if(getROM(0x005E, 1) == 0xF0)
        {
            romType = SDCARD;
        }
        else if(getROM(0x005E, 1) == 0xF8)
        {
            romType = DEVROM;
        }
#endif
        _romType = (RomType)romType;
        switch(_romType)
        {
            case ROMv1:
            {
                // Patches SYS_Exec_88 loader to accept page0 segments as the first segment and works with 64KB SRAM hardware
                patchSYS_Exec_88();

                saveScanlineModes();
                setRAM(VIDEO_MODE_D, 0xF3);
                _scanlineMode = ScanlineMode::Normal;
            }
            break;

            case ROMv2: 
            case ROMv3: 
            case ROMv4:
            case ROMv5a:
            case SDCARD:
            case DEVROM:
            {
                setRAM(VIDEO_MODE_D, 0xEC);
                setRAM(VIDEO_MODE_B, 0x0A);
                setRAM(VIDEO_MODE_C, 0x0A);
            }
            break;

            default:
            {
                Cpu::shutdown();
                fprintf(stderr, "Cpu::setRomType() : Unknown EPROM Type = 0x%02x : exiting...\n", romType);
                _EXIT_(EXIT_FAILURE);
            }
            break;
        }
        //fprintf(stderr, "Cpu::setRomType() : ROM Type = 0x%02x\n", _romType);
    }

    void saveScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            _scanlinesRom0.push_back(_ROM[i][ROM_INST]);
            _scanlinesRom1.push_back(_ROM[i][ROM_DATA]);
        }
    }

    void restoreScanlineModes(void)
    {
        for(int i=0x01C2; i<=0x01DE; i++)
        {
            _ROM[i][ROM_INST] = _scanlinesRom0[i - 0x01C2];
            _ROM[i][ROM_DATA] = _scanlinesRom1[i - 0x01C2];
        }
    }

    void swapScanlineMode(void)
    {
        if(++_scanlineMode == ScanlineMode::NumScanlineModes-1) _scanlineMode = ScanlineMode::Normal;

        switch(_scanlineMode)
        {
            case Normal:  restoreScanlineModes();                               break;
            case VideoB:  patchScanlineModeVideoB();                            break;
            case VideoC:  patchScanlineModeVideoC();                            break;
            case VideoBC: patchScanlineModeVideoB(); patchScanlineModeVideoC(); break;

            default: break;
        }
    }

    void garble(uint8_t* mem, int len)
    {
        for(int i=0; i<len; i++) mem[i] = uint8_t(rand());
    }

    void createRomHeader(const uint8_t* rom, const std::string& filename, const std::string& name, int length)
    {
        std::ofstream outfile(filename);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Graphics::createRomHeader() : failed to create '%s'\n", filename.c_str());
            return;
        }

        outfile << "uint8_t " << name + "[] = \n";
        outfile << "{\n";
        outfile << "    ";

        int colCount = 0;
        uint8_t* data = (uint8_t*)rom;
        for(int i=0; i<length; i++)
        {
            outfile << "0x" << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << ", ";
            if(++colCount == 12)
            {
                colCount = 0;
                if(i < length-1) outfile << "\n    ";
            }
        }
        
        outfile << "\n};" << std::endl;
    }

    void loadRom(int index)
    {
        _romIndex = index % _numRoms;
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));
        reset(true);
    }

    void swapRom(void)
    {
        _romIndex = (_romIndex + 1) % _numRoms;
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));
        reset(true);
    }

    void shutdown(void)
    {
#ifdef _WIN32
        saveWin32Console();
#endif

        for(int i=NUM_INT_ROMS; i<int(_romFiles.size()); i++)
        {
            if(_romFiles[i])
            {
                delete [] _romFiles[i];
                _romFiles[i] = nullptr;
            }
        }

        Loader::shutdown();

        SDL_Quit();
    }

#ifdef _WIN32
    BOOL WINAPI consoleCtrlHandler(DWORD fdwCtrlType)
    {
        switch(fdwCtrlType)
        {
            case CTRL_C_EVENT:        shutdown(); return FALSE;
            case CTRL_CLOSE_EVENT:    shutdown(); return FALSE;
            case CTRL_BREAK_EVENT:    shutdown(); return FALSE;
            case CTRL_LOGOFF_EVENT:   shutdown(); return FALSE;
            case CTRL_SHUTDOWN_EVENT: shutdown(); return FALSE;
    
            default: return FALSE;
        }
    }
#endif

    // Enable experimental 6bit sound
    void enable6BitSound(RomType romType, bool enable)
    {
        const std::vector<uint16_t> romv5aAddrs    = {0x0056, 0x005E, 0x012C, 0x015C, 0x01A6, 0x01A7, 0x02D6, 0x02D7};
        const std::vector<uint8_t>  romv5aOpcodes  = {0x00,   0x00,   0x00,   0x00,   0x40,   0x20,   0x40,   0x20  };
        const std::vector<uint8_t>  romv5aOperands = {0x03,   0x03,   0x03,   0xFC,   0x03,   0xFC,   0x03,   0xFC  };

        static bool firstTime = true;
        static std::map<uint16_t, uint8_t[2]> romv5aBackup;

        switch(romType)
        {
            case ROMv5a:
            case SDCARD:
            case DEVROM:
            {
                if(getRomType() < Cpu::ROMv5a)
                {
                    if(enable)
                    {
                        std::string romTypeStr;
                        getRomTypeStr(getRomType(), romTypeStr);
                        fprintf(stderr, "\nCpu::enable6BitSound() : Error, ROM version must be >= ROMv5a, current ROM is %s\n", romTypeStr.c_str());
                    }
                    return;
                }

                if(firstTime) romv5aBackup.clear();

                // LED pattern reduced to lower 2 LED's
                for(uint16_t a=0x0130; a<=0x0147; a++)
                {
                    if(firstTime)
                    {
                        romv5aBackup[a][0] = _ROM[a][ROM_INST];
                        romv5aBackup[a][1] = _ROM[a][ROM_DATA];
                    }

                    if(enable)
                    {
                        _ROM[a][ROM_INST] = 0x00;
                        _ROM[a][ROM_DATA] = a & 0x03;
                    }
                    else
                    {
                        _ROM[a][ROM_INST] = romv5aBackup[a][0];
                        _ROM[a][ROM_DATA] = romv5aBackup[a][1];
                    }
                }

                // Codes
                for(int i=0; i<int(romv5aAddrs.size()); i++)
                {
                    if(firstTime)
                    {
                        romv5aBackup[romv5aAddrs[i]][0] = _ROM[romv5aAddrs[i]][ROM_INST];
                        romv5aBackup[romv5aAddrs[i]][1] = _ROM[romv5aAddrs[i]][ROM_DATA];
                    }

                    if(enable)
                    {
                        _ROM[romv5aAddrs[i]][ROM_INST] = romv5aOpcodes[i];
                        _ROM[romv5aAddrs[i]][ROM_DATA] = (romv5aAddrs[i] == 0x005E) ? uint8_t(romType) | romv5aOperands[i] : romv5aOperands[i];
                    }
                    else
                    {
                        _ROM[romv5aAddrs[i]][ROM_INST] = romv5aBackup[romv5aAddrs[i]][0];
                        _ROM[romv5aAddrs[i]][ROM_DATA] = romv5aBackup[romv5aAddrs[i]][1];
                    }
                }

#if 0
                // LED mask
                _ROM[0x0056][ROM_INST] = 0x00;
                _ROM[0x0056][ROM_DATA] = 0x03;
        
                // ROM version and enable all 4 channels
                _ROM[0x005E][ROM_INST] = 0x00;
                _ROM[0x005E][ROM_DATA] = uint8_t(romType) | 0x03;

                // LED mask
                _ROM[0x012C][ROM_INST] = 0x00;
                _ROM[0x012C][ROM_DATA] = 0x03;
        
                // Audio mask
                _ROM[0x015C][ROM_INST] = 0x00;
                _ROM[0x015C][ROM_DATA] = 0xFC;
        
                // LED mask
                _ROM[0x01A6][ROM_INST] = 0x40;
                _ROM[0x01A6][ROM_DATA] = 0x03;

                // Audio mask
                _ROM[0x01A7][ROM_INST] = 0x20;
                _ROM[0x01A7][ROM_DATA] = 0xFC;
        
                // LED mask
                _ROM[0x02D6][ROM_INST] = 0x40;
                _ROM[0x02D6][ROM_DATA] = 0x03;

                // Audio mask
                _ROM[0x02D7][ROM_INST] = 0x20;
                _ROM[0x02D7][ROM_DATA] = 0xFC;
#endif
                firstTime = false;
            }
            break;

            default: break;
        }
    }


    void initialise(void)
    {
#ifdef _WIN32
        if(!AllocConsole()) return;

        // Increase internal buffer
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            csbi.dwSize.Y = 10000;
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);
        }

        if(!freopen("CONIN$", "w", stdin)) return;
        if(!freopen("CONOUT$", "w", stderr)) return;
        if(!freopen("CONOUT$", "w", stdout)) return;
        setbuf(stdout, NULL);

        _consoleWindowHWND = GetConsoleWindow();

        SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
#endif    

        _RAM.resize(Memory::getSizeRAM());

        // Memory, initialize with randomized data
        srand((unsigned int)time(NULL));
        garble((uint8_t*)_ROM, sizeof(_ROM));
        garble(&_RAM[0], Memory::getSizeRAM());
        garble((uint8_t*)&_stateS, sizeof(_stateS));

        // Internal ROMS
        _romFiles.push_back(_gigatron_0x1c_rom);
        _romFiles.push_back(_gigatron_0x20_rom);
        _romFiles.push_back(_gigatron_0x28_rom);
        _romFiles.push_back(_gigatron_0x38_rom);
        _romFiles.push_back(_gigatron_0x40_rom);
        uint8_t types[NUM_INT_ROMS] = {0x1c, 0x20, 0x28, 0x38, 0x40};
        std::string names[NUM_INT_ROMS] = {"ROMv1.rom", "ROMv2.rom", "ROMv3.rom", "ROMv4.rom", "ROMv5a.rom"};
        for(int i=0; i<NUM_INT_ROMS; i++) Editor::addRomEntry(types[i], names[i]);

        // Latest internal ROM is the one that is loaded at startup
        _romIndex = int(_romFiles.size()) - 1;

        // External ROMS
        for(int i=0; i<Loader::getConfigRomsSize(); i++)
        {
            uint8_t type = Loader::getConfigRom(i)->_type;
            std::string name = Loader::getConfigRom(i)->_name;

            std::ifstream file(name, std::ios::binary | std::ios::in);
            if(!file.is_open())
            {
                fprintf(stderr, "Cpu::initialise() : failed to open ROM file : %s\n", name.c_str());
            }
            else
            {
                // Load ROM file
                uint8_t* rom = new (std::nothrow) uint8_t[sizeof(_ROM)];
                if(!rom)
                {
                    // This is fairly pointless as the code does not have any exception handling for the many std:: memory allocations that occur
                    // If you're running out of memory running this application, (which requires around 20 Mbytes), then you need to leave the 80's
                    shutdown();
                    fprintf(stderr, "Cpu::initialise() : out of memory!\n");
                    _EXIT_(EXIT_FAILURE);
                }

                file.read((char *)rom, sizeof(_ROM));
                if(file.bad() || file.fail())
                {
                    fprintf(stderr, "Cpu::initialise() : failed to read ROM file : %s\n", name.c_str());
                }
                else
                {
                    _romFiles.push_back(rom);
                    Editor::addRomEntry(type, name);
                }
            }
        }

        // Switchable ROMS
        _numRoms = int(_romFiles.size());
        memcpy(_ROM, _romFiles[_romIndex], sizeof(_ROM));

//#define CREATE_ROM_HEADER
#ifdef CREATE_ROM_HEADER
        // Create a header file representation of a ROM, (match the ROM type number with the ROM file before enabling and running this code)
        createRomHeader((uint8_t *)_ROM, "gigatron_0x40.h", "_gigatron_0x40_rom", sizeof(_ROM));
#endif

//#define CUSTOM_ROM
#ifdef CUSTOM_ROM
        initialiseInternalGt1s();
        patchSYS_Exec_88();

#define CUSTOM_ROMV0
#ifdef CUSTOM_ROMV0
        patchTitleIntoRom(" TTL microcomputer ROM v0");
        patchSplitGt1IntoRom("./roms/starfield.rom", "Starfield", 0x0b00, MandelbrotGt1);
        patchSplitGt1IntoRom("./roms/life.rom", "Life", 0x0f00, LoaderGt1);
        patchSplitGt1IntoRom("./roms/lines.rom", "Lines", 0x1100, SnakeGt1);
        patchSplitGt1IntoRom("./roms/gigatris.rom", "Gigatris", 0x1300, PicturesGt1);
        patchSplitGt1IntoRom("./roms/tetris.rom", "Tetris", 0x3000, CreditsGt1);
        patchSplitGt1IntoRom("./roms/miditest.rom", "Midi", 0x5800, RacerGt1);
#else
        patchTitleIntoRom(" TTL microcomputer ROM v0");
        patchSplitGt1IntoRom("./roms/midi64.rom", "Midi64", 0x0b00, PicturesGt1);
#endif
#endif

        // SDL initialisation
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
        {
            fprintf(stderr, "Cpu::initialise() : failed to initialise SDL.\n");
            _EXIT_(EXIT_FAILURE);
        }

        // Initialise COM port here so that we can see error messages
        Loader::openComPort();
    }

    void cycle(const State& S, State& T)
    {
        // New state is old state unless something changes
        T = S;
    
        // Instruction Fetch
        T._IR = _ROM[S._PC][ROM_INST]; 
        T._D  = _ROM[S._PC][ROM_DATA];

        // Adapted from https://github.com/kervinck/gigatron-rom/blob/master/Contrib/dhkolf/libgtemu/gtemu.c
        // Optimise for the statistically most common instructions
        switch(S._IR)
        {
            case 0x5D: // ora [Y,X++],OUT
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._X);
                T._OUT = _cRAM[addr] | S._AC;
                T._X++;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0xC2: // st [D]
            {
                _RAM[S._D] = S._AC;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0x01: // ld [D]
            {
                T._AC = _cRAM[S._D];
                T._PC = S._PC + 1;
                return;
            }
            break;

#if 1
            case 0x00: // ld D
            {
                T._AC = S._D;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0x80: // adda D
            {
                T._AC += S._D;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0xFC: // bra D
            {
                T._PC = (S._PC & 0xFF00) | S._D;
                return;
            }
            break;

            case 0x0D: // ld [Y,X]
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._X);
                T._AC = _cRAM[addr];
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0xA0: // suba D
            {
                T._AC -= S._D;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0xE8: // blt PC,D
            {
                T._PC = (S._AC & 0x80) ? (S._PC & 0xFF00) | S._D : S._PC + 1;
                return;
            }
            break;

            case 0x81: // adda [D]
            {
                T._AC += _cRAM[S._D];
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0x89: // adda [Y,D]
            {
                uint16_t addr = MAKE_ADDR(S._Y, S._D);
                T._AC += _cRAM[addr];
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0x12: // ld AC,X
            {
                T._X = S._AC;
                T._PC = S._PC + 1;
                return;
            }
            break;

            case 0x18: // ld D,OUT
            {
                T._OUT = S._D;
                T._PC = S._PC + 1;
                return;
            }
            break;
#endif
            default: break;
        }

        int ins = S._IR >> 5;       // Instruction
        int mod = (S._IR >> 2) & 7; // Addressing mode (or condition)
        int bus = S._IR & 3;        // Busmode
        int W = (ins == 6);         // Write instruction?
        int J = (ins == 7);         // Jump instruction?
    
        uint8_t lo=S._D, hi=0, *to=NULL; // Mode Decoder
        int incX=0;
        if(!J)
        {
            switch(mod)
            {
                #define E(p) (W ? 0 : p) // Disable _AC and _OUT loading during _RAM write
                case 0: to = E(&T._AC);                            break;
                case 1: to = E(&T._AC);  lo=S._X;                  break;
                case 2: to = E(&T._AC);           hi=S._Y;         break;
                case 3: to = E(&T._AC);  lo=S._X; hi=S._Y;         break;
                case 4: to =   &T._X;                              break;
                case 5: to =   &T._Y;                              break;
                case 6: to = E(&T._OUT);                           break;
                case 7: to = E(&T._OUT); lo=S._X; hi=S._Y; incX=1; break;

                default: break;
            }
        }

        uint16_t addr = (hi << 8) | lo;
        uint8_t B = S._undef; // Data Bus
        switch(bus)
        {
            case 0: B=S._D;                                            break;
            case 1: if (!W) B = _cRAM[addr]; else _RAM.setctrl(addr);   break;
            case 2: B=S._AC;                                           break;
            case 3: B=_IN;                                             break;

            default: break;
        }

        if(W && (bus != 1 || !_RAM.has_extension()))
            _RAM[addr] = B; // Random Access Memory

        uint8_t ALU = 0; // Arithmetic and Logic Unit
        switch(ins)
        {
            case 0: ALU =         B; break; // LD
            case 1: ALU = S._AC & B; break; // ANDA
            case 2: ALU = S._AC | B; break; // ORA
            case 3: ALU = S._AC ^ B; break; // XORA
            case 4: ALU = S._AC + B; break; // ADDA
            case 5: ALU = S._AC - B; break; // SUBA
            case 6: ALU = S._AC;     break; // ST
            case 7: ALU = -S._AC;    break; // Bcc/JMP

            default: break;
        }

        if(to) *to = ALU; // Load value into register
        if(incX) T._X = S._X + 1; // Increment _X

        T._PC = S._PC + 1; // Next instruction
        if(J)
        {
            if(mod != 0) // Conditional branch within page
            {
                int cond = (S._AC>>7) + 2*(S._AC==0);
                if(mod & (1 << cond)) // 74153
                {
                    T._PC = (S._PC & 0xff00) | B;
                }
            }
            else
            {
                T._PC = (S._Y << 8) | B; // Unconditional far jump
            }
        }
    }

    void reset(bool coldBoot)
    {
        _coldBoot = coldBoot;
        _checkRomType = true;

        clearUserRAM();
        setRAM(ZERO_CONST_ADDRESS, 0x00);
        setRAM(ONE_CONST_ADDRESS, 0x01);
        setClock(CLOCK_RESET);

        Memory::initialise();
        Assembler::setvSpMin(0x00);
        Graphics::resetVTable();
        Editor::setSingleStepAddress(FRAME_COUNT_ADDRESS);

        Audio::restoreWaveTables();
    }

    void softReset(void)
    {
        Loader::setCurrentGame(std::string(""));
    }

    void swapMemoryModel(void)
    {
        if (Memory::getSizeRAM() == RAM_SIZE_LO) {
            Memory::setSizeRAM(RAM_SIZE_HI);
            _RAM.resize(RAM_SIZE_HI);
        } else if (! _RAM.has_extension()) {
            Memory::setSizeRAM(RAM_SIZE_HI);
            _RAM.resize(RAM_SIZE_HI * 2);
        } else {
            Memory::setSizeRAM(RAM_SIZE_LO);
            _RAM.resize(RAM_SIZE_LO);
        }
        Memory::initialise();
        reset(false);
    }

    void vCpuUsage(const State& S, const State& T)
    {
        UNREFERENCED_PARAM(T);

        // All ROM's so far v1 through v5a/DEVROM use the same vCPU dispatch address!
        if(S._PC == ROM_VCPU_DISPATCH)
        {
            _vPC.first = (getRAM(0x0017) <<8) | getRAM(0x0016);
            if(_vPC.first < Editor::getCpuUsageAddressA()  ||  _vPC.first > Editor::getCpuUsageAddressB()) _vCpuInstPerFrame++;
            _vCpuInstPerFrameMax++;

            // Soft reset
            if(_vPC.first == VCPU_SOFT_RESET) softReset();

            static uint64_t prevFrameCounter = 0;
            double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
            if(frameTime > VSYNC_TIMING_60)
            {
                // TODO: this is a bit of a hack, but it's emulation only so...
                // Check for magic cookie that defines a CpuUsageAddressA and CpuUsageAddressB sequence
                uint16_t magicWord0 = (getRAM(0x7F99) <<8) | getRAM(0x7F98);
                uint16_t magicWord1 = (getRAM(0x7F9B) <<8) | getRAM(0x7F9A);
                uint16_t cpuUsageAddressA = (getRAM(0x7F9D) <<8) | getRAM(0x7F9C);
                uint16_t cpuUsageAddressB = (getRAM(0x7F9F) <<8) | getRAM(0x7F9E);
                if(magicWord0 == 0xDEAD  &&  magicWord1 == 0xBEEF)
                {
                    Editor::setCpuUsageAddressA(cpuUsageAddressA);
                    Editor::setCpuUsageAddressB(cpuUsageAddressB);
                }

                prevFrameCounter = SDL_GetPerformanceCounter();
                _vCpuUtilisation = (_vCpuInstPerFrameMax) ? float(_vCpuInstPerFrame) / float(_vCpuInstPerFrameMax) : 0.0f;
                _vCpuInstPerFrame = 0;
                _vCpuInstPerFrameMax = 0;
            }
        }
    }

    bool process(bool disableOutput)
    {
        bool vBlank = false;

        // MCP100 Power-On Reset
        if(_clock < 0)
        {
            _stateS._PC = 0; 
            _initAudio = true;
            _isInReset = true;
            Loader::setCurrentGame(std::string(""));
        }

        // Update CPU
        cycle(_stateS, _stateT);

        // vCPU instruction slot utilisation, also updates _vPC
        vCpuUsage(_stateS, _stateT);

        _hSync = (_stateT._OUT & 0x40) - (_stateS._OUT & 0x40);
        _vSync = (_stateT._OUT & 0x80) - (_stateS._OUT & 0x80);
    
        // Falling vSync edge
        if(_vSync < 0)
        {
            vBlank = true;

            _clockStall = _clock;
            _vgaY = VSYNC_START;

            if(!_debugging)
            {
                if(disableOutput)
                {
                    Timing::synchronise();
                }
                // Input and graphics 60 times per second
                else
                {
                    Editor::handleInput();
                    Graphics::render(true);
                }
            }
        }

        // Pixel
        if(_vgaX++ < HLINE_END  &&  !disableOutput)
        {
            if(_vgaY >= 0  &&  _vgaY < SCREEN_HEIGHT)
            {
                if(_vgaX >=HPIXELS_START  &&  _vgaX < HPIXELS_END) Graphics::refreshPixel(_stateS, _vgaX-HPIXELS_START, _vgaY);

                // Show pixel reticle when debugging Native code
                //if(_debugging  &&  _vgaX >=HPIXELS_START-1  &&  _vgaX <= HPIXELS_END-1) Graphics::pixelReticle(_stateS, _vgaX-(HPIXELS_START-1), _vgaY);
            }
        }

#if defined(COLLECT_INST_STATS)
        _totalCount++;
        _instCounts[_stateT._IR]._count++;
        _instCounts[_stateT._IR]._inst = _stateT._IR;
        if(_clock > STARTUP_DELAY_CLOCKS * 500.0)
        {
            displayInstCounts();
            _EXIT_(0);
        }
#endif        

        // RomType, init audio and watchdog
        if(_clock > STARTUP_DELAY_CLOCKS)
        {
            if(_isInReset)
            {
                setRomType();
                _isInReset = false;
            }

            if(_initAudio  &&  _clock > STARTUP_DELAY_CLOCKS*10.0)
            {
                // ROM's V1 to V3 do not re-initialise RAM Audio Wave Tables on soft reboot, (we re-initialise them for ALL ROM versions)
                Audio::initialiseChannels();
                Audio::saveWaveTables();

                _coldBoot = false;
                _initAudio = false;
            }

            if(!_debugging  &&  _clock - _clockStall > CPU_STALL_CLOCKS)
            {
                fprintf(stderr, "Cpu::process(): CPU stall for %" PRId64 " clocks : rebooting.\n", _clock - _clockStall);
                _clockStall = CLOCK_RESET;
                reset(true);
                _vgaX = 0, _vgaY = 0;
                _hSync = 0, _vSync = 0;
            }
        }

        // Rising hSync edge
        if(_hSync > 0)
        {
            _XOUT = _stateT._AC;
        
            // Audio
            //Audio::playSample();
            //Audio::fillBuffer();
            Audio::fillCallbackBuffer();

#if defined(HARDWARE_LOADER)
            // TODO: don't enable until Loader::upload is fixed!
            // Emulation of hardware Loader
            if(_clock > STARTUP_DELAY_CLOCKS*10.0) Loader::upload(_vgaY);
#endif

            // Horizontal timing errors
            if(_vgaY >= 0  &&  _vgaY < SCREEN_HEIGHT)
            {
                static uint32_t colour = 0xFF220000;
                if((_vgaY % 4) == 0) colour = 0xFF220000;
                if(_vgaX != 200  &&  _vgaX != 400) // Support for 6.25Mhz and 12.5MHz
                {
                    colour = 0xFFFF0000;
                    //fprintf(stderr, "Cpu::process(): Horizontal timing error : vgaX %03d : vgaY %03d : xout %02x : time %0.3f\n", _vgaX, _vgaY, _stateT._AC, float(_clock)/float(CLOCK_FREQ));
                }
                if((_vgaY % 4) == 3) Graphics::refreshTimingPixel(_stateS, GIGA_WIDTH, _vgaY / 4, colour, _debugging);
            }

            _vgaX = 0;
            _vgaY++;

            // Change this once in a while
            _stateT._undef = rand() & 0xff;
        }

        // Debugger
        _debugging = Editor::handleDebugger();

        _stateS = _stateT;
        _clock++;

        return vBlank;
    }

#ifdef _WIN32
    void restoreWin32Console(void)
    {
        if(!_consoleSaveFile) return;

        std::string line;
        std::ifstream infile(Loader::getCwdPath() + "/" + "console.txt");
        if(infile.is_open())
        {
            getline(infile, line);

            int xpos, ypos, width, height;
            sscanf_s(line.c_str(), "%d %d %d %d", &xpos, &ypos, &width, &height);
            if(xpos < -2000  ||  xpos > 4000  ||  ypos < 120  ||  ypos > 1000  ||  width < 100  ||  width > 2000  ||  height < 100 ||  height > 1000)
            {
                xpos = -1000; ypos = 120; width = 1000; height = 1000;
            }

            MoveWindow(_consoleWindowHWND, xpos, ypos, width, height, true);
            BringWindowToTop(_consoleWindowHWND);
        }
    }

    void saveWin32Console(void)
    {
        if(!_consoleSaveFile) return;

        RECT rect;
        if(GetWindowRect(_consoleWindowHWND, &rect))
        {
            std::ofstream outfile(Loader::getCwdPath() + "/" + "console.txt");
            if(outfile.is_open())
            {
                int xpos = rect.left;
                int ypos = rect.top;
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                outfile << xpos << " " << ypos << " " << width << " " << height << std::endl;
            }
        }
    }
#endif
#endif
}
