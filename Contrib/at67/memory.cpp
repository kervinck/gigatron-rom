#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>

#include "memory.h"


namespace Memory
{
    bool has64KRam = false;

    std::vector<RamEntry> _freeRam;
    std::vector<RamEntry> _stackRam;
    std::vector<RamEntry> _vasmRam;
    std::vector<RamEntry> _varRam;
    std::vector<RamEntry> _strRam;
    std::vector<RamEntry> _arrRam;


    void intitialise(void)
    {
        _freeRam.clear();
        _stackRam.clear();
        _vasmRam.clear();
        _varRam.clear();
        _strRam.clear();
        _arrRam.clear();

        _freeRam.push_back({RAM_PAGE_START_0, RAM_PAGE_SIZE_0});
        _freeRam.push_back({RAM_PAGE_START_1, RAM_PAGE_SIZE_1});
        _freeRam.push_back({RAM_PAGE_START_2, RAM_PAGE_SIZE_2});
        _freeRam.push_back({RAM_PAGE_START_3, RAM_PAGE_SIZE_3});

        _stackRam.push_back({RAM_STACK_START, RAM_STACK_SIZE});

        for(uint16_t i=RAM_SEGMENTS_START; i<=RAM_SEGMENTS_END; i+=RAM_SEGMENTS_OFS) _freeRam.push_back({i, RAM_SEGMENTS_SIZE});

        if(has64KRam) _freeRam.push_back({RAM_EXPANSION_START, RAM_EXPANSION_SIZE});
    }

    bool updateRamLists(RamType ramType, int index, uint16_t address, uint16_t size, uint16_t newSize)
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
                case RamArr:   _arrRam.push_back({address, size});   break;
                case RamStack: _stackRam.push_back({address, size}); break;
            }

            return true;
        }

        return false;
    }

    bool getRam(FitType fitType, RamType ramType, uint16_t size, uint16_t& address)
    {
        int index = -1;
        uint16_t newSize = 0;

        switch(fitType)
        {
            case FitSmallest:
            {
                //std::sort(_freeRam.begin(), _freeRam.end(), [](const RamEntry& a, const RamEntry& b) {return a._size < b._size; });
                uint16_t smallest = 0xFFFF;
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
                uint16_t largest = 0;
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
