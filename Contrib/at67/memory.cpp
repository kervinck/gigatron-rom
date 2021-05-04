#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>

#include "memory.h"


namespace Memory
{
    int _sizeRAM = RAM_SIZE_LO;
    int _baseFreeRAM = _sizeRAM - RAM_USED_DEFAULT;
    int _sizeFreeRAM = _baseFreeRAM;

    std::vector<RamEntry> _freeRam;
    std::vector<RamEntry> _videoRam;


    int getSizeRAM(void) {return _sizeRAM;}
    int getBaseFreeRAM(void) {return _baseFreeRAM;}
    int getSizeFreeRAM(void) {return _sizeFreeRAM;}
    int getFreeGtbRAM(int numLines)
    {
        int free = ((0x80 - HI_BYTE(GTB_LINE0_ADDRESS))*NUM_GTB_LINES_PER_ROW - numLines)*MAX_GTB_LINE_SIZE - MAX_GTB_LINE_SIZE;
        if(_sizeRAM == RAM_SIZE_HI) free += RAM_UPPER_SIZE;
        return free;
    }

    void setSizeRAM(int sizeRAM) {_sizeRAM = sizeRAM; initialise();}
    void setSizeFreeRAM(int freeRAM) {_sizeFreeRAM = (freeRAM >= 0) ? freeRAM : 0;}


    void initialise(void)
    {
        _freeRam.clear();
        _videoRam.clear();

        // 0x0200 <-> 0x0400
        _freeRam.push_back({RAM_PAGE_START_0, RAM_PAGE_SIZE_0});
        _freeRam.push_back({RAM_PAGE_START_1, RAM_PAGE_SIZE_1});
        _freeRam.push_back({RAM_PAGE_START_2, RAM_PAGE_SIZE_2});

        // 0x0500 <-> 0x0600
        _freeRam.push_back({RAM_PAGE_START_3, RAM_PAGE_SIZE_3*2});

        // 0x08A0 <-> 0x7FA0
        for(uint16_t i=RAM_SEGMENTS_START; i<=RAM_SEGMENTS_END; i+=RAM_SEGMENTS_OFS) _freeRam.push_back({i, RAM_SEGMENTS_SIZE});

        // 0x8000 <-> 0xFF00
        if(_sizeRAM == RAM_SIZE_HI) _freeRam.push_back({RAM_UPPER_START, RAM_UPPER_SIZE});

        // VRAM 0x0800 <-> 0x7F00, 160x120 pixels, offscreen areas start at 0xXXA0 and end at 0xXXFF, (can be used for horizontal scrolling or code/data storage)
        for(uint16_t i=RAM_VIDEO_START; i<=RAM_VIDEO_END; i+=RAM_VIDEO_OFS) _videoRam.push_back({i, RAM_SCANLINE_SIZE});

        _baseFreeRAM = _sizeRAM - RAM_USED_DEFAULT;
        _sizeFreeRAM = _baseFreeRAM;
    }

    void invertFreeRAM(void)
    {
        _freeRam.clear();
        _freeRam = _videoRam;
    }

    void updateFreeRAM(void)
    {
        _sizeFreeRAM = 0;

        // Check to see if any chunks can be merged
        for(auto it=_freeRam.begin(); it!=_freeRam.end()-1;)
        {
            uint16_t addr0 = it->_address;
            uint16_t size0 = uint16_t(it->_size);

            uint16_t addr1 = (it + 1)->_address;

            // Merge
            if(addr0 + size0 == addr1)
            {
                (it + 1)->_size += size0;
                (it + 1)->_address = addr0;
                it = _freeRam.erase(it);
            }
            else
            {
                it++;
            }
        }

        // Sort by address in ascending order
        std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
        {
            uint16_t addressA = ramEntryA._address;
            uint16_t addressB = ramEntryB._address;
            return (addressA < addressB);
        });

        for(int i=0; i<int(_freeRam.size()); i++)
        {
            _sizeFreeRAM += _freeRam[i]._size;
        }
    }

    bool updateFreeRamList(int index, uint16_t address, int size)
    {
        if(index >= 0  &&  size >= 0)
        {
            _freeRam.erase(_freeRam.begin() + index);
            if(size) _freeRam.push_back({uint16_t(address), size});
            updateFreeRAM();

            //fprintf(stderr, "1 RAM segment: %3d  :  %04x %3d  :  %04x %3d\n", _sizeFreeRAM, address, size, uint16_t(address + size), newSize);

            return true;
        }

        return false;
    }

    bool updateFreeRamList(int index, uint16_t address0, int size0, uint16_t address1, int size1)
    {
        if(index >= 0)
        {
            _freeRam.erase(_freeRam.begin() + index);
            if(size0) _freeRam.push_back({uint16_t(address0), size0});
            if(size1) _freeRam.push_back({uint16_t(address1), size1});
            updateFreeRAM();

            //fprintf(stderr, "2 RAM segments: %3d  :  %04x %3d  :  %04x %3d\n", _sizeFreeRAM, address0, size0, address1, size1);

            return true;
        }

        return false;
    }

    bool isFreeRAM(uint16_t address, int size)
    {
        if(address > _sizeRAM - 1)
        {
            fprintf(stderr, "Memory::isFreeRAM() : Memory at 0x%04x does not exist on this %d byte system : your request : 0x%04x %d\n", address, _sizeRAM, address, size);
            return false;
        }

        for(int i=0; i<int(_freeRam.size()); i++)
        {
            // Free RAM segment
            if(address == _freeRam[i]._address  &&  size <= _freeRam[i]._size)
            {
                return true;
            }
            // Free RAM within segment
            else if(address > _freeRam[i]._address  &&  (address + size <= _freeRam[i]._address + _freeRam[i]._size))
            {
                return true;
            }
        }

        return false;
    }

    bool isVideoRAM(uint16_t address)
    {
        for(int i=0; i<int(_videoRam.size()); i++)
        {
            // Address within Video RAM segment
            if(address >= _videoRam[i]._address  &&  (address < _videoRam[i]._address + _videoRam[i]._size))
            {
                return true;
            }
        }

        return false;
    }

    bool getNextCodeAddress(FitType fitType, uint16_t start, int size, uint16_t& address)
    {
        switch(fitType)
        {
            case FitAscending:
            {
                for(int j=0; j<int(_freeRam.size()); j++)
                {
                    for(int i=0; i<_freeRam[j]._size; i++)
                    {
                        uint16_t left = uint16_t(_freeRam[j]._size - i);
                        uint16_t addr = uint16_t(_freeRam[j]._address + i);
                        if(addr >= start  &&  size <= left  &&  HI_BYTE(addr) == HI_BYTE(addr + size))
                        {
                            address = addr;
                            return true;
                        }
                    }
                }
            }
            break;

            case FitDescending:
            {
                for(int j=int(_freeRam.size())-1; j>=0; j--)
                {
                    for(int i=_freeRam[j]._size-1; i>=0; i--)
                    {
                        uint16_t left = uint16_t(_freeRam[j]._size - i);
                        uint16_t addr = uint16_t(_freeRam[j]._address + i);
                        if(addr < start  &&  size <= left  &&  HI_BYTE(addr) == HI_BYTE(addr + size))
                        {
                            address = addr;
                            return true;
                        }
                    }
                }
            }
            break;

            default: break;
        }

        fprintf(stderr, "Memory::getNextCodeAddress() : Couldn't find free code space in RAM of size %d bytes\n", size);
        return false;
    }

    bool giveFreeRAM(uint16_t address, int size)
    {
        // Check to see if new chunk can be merged with any other free chunks
        for(int i=0; i<int(_freeRam.size()); i++)
        {
            uint16_t siz = uint16_t(_freeRam[i]._size);
            uint16_t addr = _freeRam[i]._address;

            // RAM is already free
            if(address >= addr  &&  address + size <= addr + siz) return false;

            // Insert
            if(address + size == addr)
            {
                _freeRam[i]._size += size;
                _freeRam[i]._address = address;
                updateFreeRAM();
                return true;
            }

            // Append
            if(address == addr + siz)
            {
                _freeRam[i]._size += size;
                updateFreeRAM();
                return true;
            }
        }

        // Add new chunk
        _freeRam.push_back({address, size});
        updateFreeRAM();
        return true;
    }

    bool takeFreeRAM(uint16_t address, int size, bool printError)
    {
        if(address > _sizeRAM - 1)
        {
            if(printError) fprintf(stderr, "Memory::takeFreeRAM() : Memory at 0x%04x does not exist on this %d byte system : your request : 0x%04x %d\n", address, _sizeRAM, address, size);
            return false;
        }

        for(int i=0; i<int(_freeRam.size()); i++)
        {
            // RAM segment becomes smaller
            if(address == _freeRam[i]._address  &&  size <= _freeRam[i]._size)
            {
                if(!updateFreeRamList(i, address + uint16_t(size), _freeRam[i]._size - size)) break;
                return true;
            }
            // RAM segment gets split into 2 smaller segments
            else if(address > _freeRam[i]._address  &&  (address + size <= _freeRam[i]._address + _freeRam[i]._size))
            {
                uint16_t address0 = _freeRam[i]._address;
                uint16_t address1 = uint16_t(address + size);
                int size0 = address - _freeRam[i]._address;
                int size1 = (_freeRam[i]._address + _freeRam[i]._size) - (address + size);
                if(!updateFreeRamList(i, address0, size0, address1, size1)) break;
                return true;
            }
        }

        if(printError) fprintf(stderr, "Memory::takeFreeRAM() : Memory at 0x%04x already in use : your request : 0x%04x %d\n", address, address, size);
        return false;
    }

    bool getFreeRAMLargest(uint16_t& address, int& size)
    {
        // Sort entries from highest size to lowest size
        std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
        {
            int sizeA = ramEntryA._size;
            int sizeB = ramEntryB._size;
            return (sizeA > sizeB);
        });

        if(_freeRam.size() == 0) return false;

        address = _freeRam[0]._address;
        size = _freeRam[0]._size;

        updateFreeRAM();

        return true;
    }

    // Attempts to returns RAM request of a given size : withinPage specifiec no page boundary crossings : oddeven specifies type of address returned, 0=don't care, 1=even, 2=odd
    bool getFreeRAM(FitType fitType, int size, uint16_t min, uint16_t max, uint16_t& address, bool withinPage, ParityType oddEven)
    {
        switch(fitType)
        {
            case FitAscending:
            {
                for(int j=0; j<int(_freeRam.size()); j++)
                {
                    for(int i=0; i<_freeRam[j]._size; i++)
                    {
                        uint16_t left = uint16_t(_freeRam[j]._size - i);
                        uint16_t addr = uint16_t(_freeRam[j]._address + i);

                        if(addr >= min  &&  addr + size-1 <= max  &&  size <= left)
                        {
                            // Skip if request must be within a page and it isn't
                            if(withinPage  &&  HI_BYTE(addr) != HI_BYTE(addr + size-1)) continue;

                            // Skip if address doesn't meet odd/even requirements
                            if(oddEven)
                            {
                                if((addr & 1) != oddEven - ParityEven) continue;
                            }

                            address = addr;
                            return takeFreeRAM(addr, size);
                        }
                    }
                }
            }
            break;

            case FitDescending:
            {
                for(int j=int(_freeRam.size())-1; j>=0; j--)
                {
                    for(int i=_freeRam[j]._size-1; i>=0; i--)
                    {
                        uint16_t left = uint16_t(_freeRam[j]._size - i);
                        uint16_t addr = uint16_t(_freeRam[j]._address + i);

                        if(addr >= min  &&  addr + size-1 <= max  &&  size <= left)
                        {
                            // Skip if request must be within a page and it isn't
                            if(withinPage  &&  HI_BYTE(addr) != HI_BYTE(addr + size-1)) continue;

                            // Skip if address doesn't meet odd/even requirements
                            if(oddEven)
                            {
                                if((addr & 1) != oddEven - ParityEven) continue;
                            }

                            address = addr;
                            return takeFreeRAM(addr, size);
                        }
                    }
                }
            }
            break;

            default: break;
        }
        
        fprintf(stderr, "Memory::getFreeRAM() : No free RAM found of size %d bytes\n", size);
        return false;
    }

    // Return free RAM chunks in order of request : withinPage specifiec no page boundary crossings
    bool getFreeRAM(FitType fitType, uint16_t addrMin, uint16_t addrMax, uint16_t sizeMin, uint16_t& address, uint16_t inSize, uint16_t& outSize, bool withinPage)
    {
        switch(fitType)
        {
            case FitAscending:
            {
                for(int i=0; i<int(_freeRam.size()); i++)
                {
                    outSize = uint16_t(_freeRam[i]._size);
                    address = uint16_t(_freeRam[i]._address);

                    // Skip if size is too small or request crosses a page boundary
                    if((outSize < sizeMin)  ||  (withinPage  &&  HI_BYTE(address) != HI_BYTE(address + outSize-1))) continue;

                    if(address >= addrMin  &&  address + outSize-1 <= addrMax)
                    {
                        if(inSize < outSize) outSize = inSize;
                        return takeFreeRAM(address, outSize);
                    }
                }
            }
            break;

            case FitDescending:
            {
                for(int i=int(_freeRam.size())-1; i>=0; i--)
                {
                    outSize = uint16_t(_freeRam[i]._size);
                    address = uint16_t(_freeRam[i]._address);

                    // Skip if size is too small or request crosses a page boundary
                    if((outSize < sizeMin)  ||  (withinPage  &&  HI_BYTE(address) != HI_BYTE(address + outSize-1))) continue;

                    if(address >= addrMin  &&  address + outSize-1 <= addrMax)
                    {
                        if(inSize < outSize) outSize = inSize;
                        return takeFreeRAM(address, outSize);
                    }
                }
            }
            break;

            default: break;
        }
        
        fprintf(stderr, "Memory::getFreeRAM() : No free RAM found within 0x%04x and 0x%04x\n", addrMin, addrMax);
        return false;
    }

    void printFreeRamList(SortType sortType)
    {
        // Make a local copy so that we don't change the sort on the real free RAM list
        std::vector<RamEntry> freeRam = _freeRam;

        switch(sortType)
        {
            // Sort entries from lowest address to highest address
            case AddressAscending:
            {
                std::sort(freeRam.begin(), freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    uint16_t addressA = ramEntryA._address;
                    uint16_t addressB = ramEntryB._address;
                    return (addressA < addressB);
                });
            }
            break;

            // Sort entries from highest address to lowest address
            case AddressDescending:
            {
                std::sort(freeRam.begin(), freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    uint16_t addressA = ramEntryA._address;
                    uint16_t addressB = ramEntryB._address;
                    return (addressA > addressB);
                });
            }
            break;

            // Sort entries from lowest size to highest size
            case SizeAscending:
            {
                std::sort(freeRam.begin(), freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    int sizeA = ramEntryA._size;
                    int sizeB = ramEntryB._size;
                    return (sizeA < sizeB);
                });
            }
            break;

            // Sort entries from highest size to lowest size
            case SizeDescending:
            {
                std::sort(freeRam.begin(), freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    int sizeA = ramEntryA._size;
                    int sizeB = ramEntryB._size;
                    return (sizeA > sizeB);
                });
            }
            break;

            default: break;
        }

        int totalFree = 0;
        fprintf(stderr, "\n");
        for(int i=0; i<int(freeRam.size()); i++)
        {
            totalFree += freeRam[i]._size;
            fprintf(stderr, "Memory::printFreeRamList() : %3d : 0x%04x %3d\n", i, freeRam[i]._address, freeRam[i]._size);
        }
        fprintf(stderr, "Memory::printFreeRamList() : Expected %5d : Found %5d\n", _sizeFreeRAM, totalFree);
    }
}
