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


    int getSizeRAM(void) {return _sizeRAM;}
    int getBaseFreeRAM(void) {return _baseFreeRAM;}
    int getSizeFreeRAM(void) {return _sizeFreeRAM;}
    int getFreeGtbRAM(int numLines)
    {
        int free = ((0x80 - HI_BYTE(GTB_LINE0_ADDRESS))*NUM_GTB_LINES_PER_ROW - numLines)*MAX_GTB_LINE_SIZE - MAX_GTB_LINE_SIZE;
        if(_sizeRAM == RAM_SIZE_HI) free += RAM_EXPANSION_SIZE;
        return free;
    }

    void setSizeRAM(int sizeRAM) {_sizeRAM = sizeRAM; initialise();}
    void setSizeFreeRAM(int freeRAM) {_sizeFreeRAM = (freeRAM >= 0) ? freeRAM : 0;}


    void initialise(void)
    {
        _freeRam.clear();

        // 0x0200 <-> 0x0400
        _freeRam.push_back({RAM_PAGE_START_0, RAM_PAGE_SIZE_0});
        _freeRam.push_back({RAM_PAGE_START_1, RAM_PAGE_SIZE_1});
        _freeRam.push_back({RAM_PAGE_START_2, RAM_PAGE_SIZE_2});

        // 0x0500 <-> 0x0600
        _freeRam.push_back({RAM_PAGE_START_3, RAM_PAGE_SIZE_3});
        _freeRam.push_back({RAM_PAGE_START_4, RAM_PAGE_SIZE_4});

        // 0x08A0 <-> 0c7FA0
        for(uint16_t i=RAM_SEGMENTS_START; i<=RAM_SEGMENTS_END; i+=RAM_SEGMENTS_OFS) _freeRam.push_back({i, RAM_SEGMENTS_SIZE});

        // 0x8000 <-> 0xFF00
        if(_sizeRAM == RAM_SIZE_HI)
        {
            for(uint32_t a=RAM_EXPANSION_START; a<RAM_EXPANSION_START + RAM_EXPANSION_SIZE; a+=RAM_EXPANSION_SEG)
            {
                _freeRam.push_back({uint16_t(a), RAM_EXPANSION_SEG});
            }
        }

        _baseFreeRAM = _sizeRAM - RAM_USED_DEFAULT;
        _sizeFreeRAM = _baseFreeRAM;
    }

    void updateSizeFreeRAM(void)
    {
        _sizeFreeRAM = 0;

        // Sort by address
        std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
        {
            uint16_t addressA = ramEntryA._address;
            uint16_t addressB = ramEntryB._address;
            return (addressA < addressB);
        });

        for(int i=0; i<_freeRam.size(); i++)
        {
            _sizeFreeRAM += _freeRam[i]._size;
        }
    }

    bool updateFreeRamList(int index, uint16_t address, int size, int newSize)
    {
        if(index >= 0  &&  newSize >= 0)
        {
            _freeRam.erase(_freeRam.begin() + index);
            if(newSize) _freeRam.push_back({uint16_t(address + size), newSize});
            updateSizeFreeRAM();

            //fprintf(stderr, "1 RAM chunk: %3d  :  %04x %3d  :  %04x %3d\n", _sizeFreeRAM, address, size, uint16_t(address + size), newSize);

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
            updateSizeFreeRAM();

            //fprintf(stderr, "2 RAM chunks: %3d  :  %04x %3d  :  %04x %3d\n", _sizeFreeRAM, address0, size0, address1, size1);

            return true;
        }

        return false;
    }

    bool takeFreeRAM(uint16_t address, int size)
    {
        if(address > _sizeRAM - 1)
        {
            fprintf(stderr, "Memory::takeFreeRAM() : Memory at 0x%04x does not exist on this %d byte system : your request : 0x%04x %d\n", address, _sizeRAM, address, size);
            return false;
        }

        for(int i=0; i<_freeRam.size(); i++)
        {
            // RAM chunk becomes smaller
            if(address == _freeRam[i]._address  &&  size <= _freeRam[i]._size)
            {
                return updateFreeRamList(i, address, size, _freeRam[i]._size - size);
            }
            // RAM chunk gets split into 2 smaller chunks
            else if(address > _freeRam[i]._address  &&  (address + size <= _freeRam[i]._address + _freeRam[i]._size))
            {
                uint16_t address0 = _freeRam[i]._address;
                uint16_t address1 = address + size;
                int size0 = address - _freeRam[i]._address;
                int size1 = (_freeRam[i]._address + _freeRam[i]._size) - (address + size);
                return updateFreeRamList(i, address0, size0, address1, size1);
            }
        }

        fprintf(stderr, "Memory::takeFreeRAM() : Memory at 0x%04x already in use : your request : 0x%04x %d\n", address, address, size);
        return false;
    }

    bool giveFreeRAM(FitType fitType, int size, uint16_t min, uint16_t max, uint16_t& address)
    {
        int index = -1;
        int newSize = 0;

        switch(fitType)
        {
            case FitSmallest:
            {
                int smallest = 0xFFFF;
                for(int i=0; i<_freeRam.size(); i++)
                {
                    if(_freeRam[i]._size >= size  &&  _freeRam[i]._size < smallest)
                    {
                        index = i;
                        newSize = _freeRam[i]._size - size;
                        smallest = _freeRam[i]._size;
                        address =  _freeRam[i]._address;
                    }
                }

                return updateFreeRamList(index, address, size, newSize);
            }
            break;

            case FitLargest:
            {
                int largest = 0;
                for(int i=0; i<_freeRam.size(); i++)
                {
                    if(_freeRam[i]._size >= size  &&  _freeRam[i]._size > largest)
                    {
                        index = i;
                        newSize = _freeRam[i]._size - size;
                        largest = _freeRam[i]._size;
                        address =  _freeRam[i]._address;
                    }
                }

                return updateFreeRamList(index, address, size, newSize);
            }
            break;

            case FitAscending:
            {
                uint16_t ascending = min;
                for(int i=0; i<_freeRam.size(); i++)
                {
                    if(_freeRam[i]._size >= size  &&  _freeRam[i]._address >= ascending  &&  _freeRam[i]._address <= max)
                    {
                        index = i;
                        newSize = _freeRam[i]._size - size;
                        address =  _freeRam[i]._address;
                        ascending = _freeRam[i]._address;
                    }
                }

                return updateFreeRamList(index, address, size, newSize);
            }
            break;

            case FitDescending:
            {
                uint16_t descending = max;
                for(int i=0; i<_freeRam.size(); i++)
                {
                    if(_freeRam[i]._size >= size  &&  _freeRam[i]._address <= descending  &&  _freeRam[i]._address >= min)
                    {
                        index = i;
                        newSize = _freeRam[i]._size - size;
                        address =  _freeRam[i]._address;
                        descending = _freeRam[i]._address;
                    }
                }

                return updateFreeRamList(index, address, size, newSize);
            }
            break;
        }
        
        return false;
    }

    void printFreeRamList(SortType sortType)
    {
        switch(sortType)
        {
            // Sort entries from lowest address to highest address
            case AddressAscending:
            {
                std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
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
                std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    uint16_t addressA = ramEntryA._address;
                    uint16_t addressB = ramEntryB._address;
                    return (addressA > addressB);
                });
            }
            break;

            // Sort entries from lowest address to highest address
            case SizeAscending:
            {
                std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    int sizeA = ramEntryA._size;
                    int sizeB = ramEntryB._size;
                    return (sizeA < sizeB);
                });
            }
            break;

            // Sort entries from highest address to lowest address
            case SizeDescending:
            {
                std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& ramEntryA, const RamEntry& ramEntryB)
                {
                    int sizeA = ramEntryA._size;
                    int sizeB = ramEntryB._size;
                    return (sizeA > sizeB);
                });
            }
            break;
        }

        int totalFree = 0;
        for(int i=0; i<_freeRam.size(); i++)
        {
            totalFree += _freeRam[i]._size;
            fprintf(stderr, "Memory::printFreeRamList() : %3d : 0x%04x %3d\n", i, _freeRam[i]._address, _freeRam[i]._size);
        }
        fprintf(stderr, "Memory::printFreeRamList() : Expected %5d : Found %5d\n", _sizeFreeRAM, totalFree);
    }
}
