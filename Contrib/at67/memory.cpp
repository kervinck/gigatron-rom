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
    std::vector<RamEntry> _stackRam;
    std::vector<RamEntry> _vasmRam;
    std::vector<RamEntry> _varRam;
    std::vector<RamEntry> _strRam;
    std::vector<RamEntry> _arrayRam;


    int getSizeRAM(void) {return _sizeRAM;}
    int getBaseFreeRAM(void) {return _baseFreeRAM;}
    int getSizeFreeRAM(void) {return _sizeFreeRAM;}
    int getFreeGtbRAM(int numLines)
    {
        int free = ((0x80 - HI_BYTE(GTB_LINE0_ADDRESS))*NUM_GTB_LINES_PER_ROW - numLines)*MAX_GTB_LINE_SIZE - MAX_GTB_LINE_SIZE;
        if(_sizeRAM == RAM_SIZE_HI) free += _sizeRAM;
        return free;
    }

    void setSizeRAM(int sizeRAM) {_sizeRAM = sizeRAM;}
    void setSizeFreeRAM(int freeRAM) {_sizeFreeRAM = (freeRAM >= 0) ? freeRAM : 0;}


    void intitialise(void)
    {
        _freeRam.clear();
        _stackRam.clear();
        _vasmRam.clear();
        _varRam.clear();
        _strRam.clear();
        _arrayRam.clear();

        _freeRam.push_back({RAM_PAGE_START_0, RAM_PAGE_SIZE_0});
        _freeRam.push_back({RAM_PAGE_START_1, RAM_PAGE_SIZE_1});
        _freeRam.push_back({RAM_PAGE_START_2, RAM_PAGE_SIZE_2});
        _freeRam.push_back({RAM_PAGE_START_3, RAM_PAGE_SIZE_3});

        _stackRam.push_back({RAM_STACK_START, RAM_STACK_SIZE});

        for(uint16_t i=RAM_SEGMENTS_START; i<=RAM_SEGMENTS_END; i+=RAM_SEGMENTS_OFS) _freeRam.push_back({i, RAM_SEGMENTS_SIZE});

        if(_sizeRAM == RAM_SIZE_HI) _freeRam.push_back({RAM_EXPANSION_START, RAM_EXPANSION_SIZE});

        _baseFreeRAM = _sizeRAM - RAM_USED_DEFAULT;
        _sizeFreeRAM = _baseFreeRAM;
    }

    bool updateRamLists(RamType ramType, int index, uint16_t address, int size, int newSize)
    {
        if(index >= 0)
        {
            _freeRam.erase(_freeRam.begin() + index);
            if(newSize > 0)
            {
                _freeRam.push_back({uint16_t(address + size), newSize});
            }
            
            switch(ramType)
            {
                case RamVasm:  _vasmRam.push_back({address, size});  break;
                case RamVar:   _varRam.push_back({address, size});   break;
                case RamStr:   _strRam.push_back({address, size});   break;
                case RamArray: _arrayRam.push_back({address, size}); break;
                case RamStack: _stackRam.push_back({address, size}); break;
            }

            return true;
        }

        return false;
    }

    bool getRAM(FitType fitType, RamType ramType, int size, uint16_t& address)
    {
        int index = -1;
        int newSize = 0;

        switch(fitType)
        {
            case FitSmallest:
            {
                //std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& a, const RamEntry& b) {return a._size < b._size; });
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

                return updateRamLists(ramType, index, address, size, newSize);
            }
            break;

            case FitLargest:
            {
                //std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& a, const RamEntry& b) {return a._size > b._size; });
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

                return updateRamLists(ramType, index, address, size, newSize);
            }
            break;

            case FitAscending:
            {
                //std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& a, const RamEntry& b) {return a._address < b._address; });
                uint16_t ascending = 0xFFFF;
                for(int i=0; i<_freeRam.size(); i++)
                {
                    if(_freeRam[i]._size >= size  &&  _freeRam[i]._address < ascending)
                    {
                        index = i;
                        newSize = _freeRam[i]._size - size;
                        ascending = _freeRam[i]._address;
                        address =  _freeRam[i]._address;
                    }
                }

                return updateRamLists(ramType, index, address, size, newSize);
            }
            break;
        }
        
        return false;
    }
}
