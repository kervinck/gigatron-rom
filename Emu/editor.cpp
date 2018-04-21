#include <vector>

#if defined(_WIN32)
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#include <SDL.h>
#include "cpu.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "graphics.h"


namespace Editor
{
    int _cursorX = 0;
    int _cursorY = 0;
    bool _hexEdit = false;
    bool _loadFile = false;
    int _editorMode = EditorModes::Hex;
    int _hexRomMode = 0;
    uint8_t _memoryDigit = 0;
    uint8_t _addressDigit = 0;
    uint16_t _hexBaseAddress = 0x0200;
    uint16_t _loadBaseAddress = 0x0200;
    uint16_t _varsBaseAddress = 0x0030;
    int _directoryNamesIndex = 0;
    std::vector<std::string> _directoryNames;


    int getCursorX(void) {return _cursorX;}
    int getCursorY(void) {return _cursorY;}
    bool getHexEdit(void) {return _hexEdit;}
    bool getLoadFile(void) {return _loadFile;}
    int getEditorMode(void) {return _editorMode;}
    int getHexRomMode(void) {return _hexRomMode;}
    uint8_t getMemoryDigit(void) {return _memoryDigit;}
    uint8_t getAddressDigit(void) {return _addressDigit;}
    uint16_t getHexBaseAddress(void) {return _hexBaseAddress;}
    uint16_t getLoadBaseAddress(void) {return _loadBaseAddress;}
    uint16_t getVarsBaseAddress(void) {return _varsBaseAddress;}
    int getDirectoryNamesIndex(void) {return _directoryNamesIndex;}
    int getDirectoryNamesSize(void) {return int(_directoryNames.size());}
    std::string* getDirectoryName(int index) {return &_directoryNames[index % _directoryNames.size()];}

    void setCursorX(int x) {_cursorX = x;}
    void setCursorY(int y) {_cursorY = y;}

    void handleMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0)
        {
            if(_loadFile == true)
            {
                _directoryNamesIndex--;
                if(_directoryNamesIndex < 0) _directoryNamesIndex = 0;
            }
            else
            {
                _hexBaseAddress = (_hexBaseAddress - HEX_CHARS_X) & (RAM_SIZE-1);
            }
        }
        if(event.wheel.y < 0)
        {
            if(_loadFile == true)
            {
                if(_directoryNames.size() > HEX_CHARS_Y)
                {
                    _directoryNamesIndex++;
                    if(_directoryNames.size() - _directoryNamesIndex < HEX_CHARS_Y) _directoryNamesIndex--;
                }
            }
            else
            {
                _hexBaseAddress = (_hexBaseAddress + HEX_CHARS_X) & (RAM_SIZE-1);
            }
        }
    }

    void handleKeyDown(SDL_Keycode keyCode)
    {
        int limitY = (_loadFile == false) ? HEX_CHARS_Y : int(_directoryNames.size());

        switch(keyCode)
        {
            case SDLK_d:      Cpu::setIN(Cpu::getIN() & ~INPUT_RIGHT);   break;
            case SDLK_a:      Cpu::setIN(Cpu::getIN() & ~INPUT_LEFT);    break;
            case SDLK_s:      Cpu::setIN(Cpu::getIN() & ~INPUT_DOWN);    break;
            case SDLK_w:      Cpu::setIN(Cpu::getIN() & ~INPUT_UP);      break;
            case SDLK_SPACE:  Cpu::setIN(Cpu::getIN() & ~INPUT_START);   break;
            case SDLK_z:      Cpu::setIN(Cpu::getIN() & ~INPUT_SELECT);  break;
            case SDLK_SLASH:  Cpu::setIN(Cpu::getIN() & ~INPUT_B);       break;
            case SDLK_PERIOD: Cpu::setIN(Cpu::getIN() & ~INPUT_A);       break;

            case SDLK_RIGHT: _cursorX = (++_cursorX >= HEX_CHARS_X) ? 0  : _cursorX;  _memoryDigit = 0; _addressDigit = 0; break;
            case SDLK_LEFT:  _cursorX = (--_cursorX < 0) ? HEX_CHARS_X-1 : _cursorX;  _memoryDigit = 0; _addressDigit = 0; break;
            case SDLK_DOWN:  _cursorY = (++_cursorY >= limitY) ? 0       : _cursorY;  _memoryDigit = 0; _addressDigit = 0; break;
            case SDLK_UP:    _cursorY = (--_cursorY < -1) ? limitY-1     : _cursorY;  _memoryDigit = 0; _addressDigit = 0; break;

            case SDLK_PAGEUP:
            {
                if(_loadFile == true)
                {
                    _directoryNamesIndex--;
                    if(_directoryNamesIndex < 0) _directoryNamesIndex = 0;
                }
                else
                {
                    _hexBaseAddress = (_hexBaseAddress - HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
                }
            }
            break;

            case SDLK_PAGEDOWN:
            {
                if(_loadFile == true)
                {
                    if(_directoryNames.size() > HEX_CHARS_Y)
                    {
                        _directoryNamesIndex++;
                        if(_directoryNames.size() - _directoryNamesIndex < HEX_CHARS_Y) _directoryNamesIndex--;
                    }
                }
                else
                {
                    _hexBaseAddress = (_hexBaseAddress + HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
                }
            }
            break;

            case SDLK_EQUALS:
            {
                double timingHack = Timing::getTimingHack() - TIMING_HACK*0.05;
                if(timingHack >= 0.0) Timing::setTimingHack(timingHack);
            }
            break;
            case SDLK_MINUS:
            {
                double timingHack = Timing::getTimingHack() + TIMING_HACK*0.05;
                if(timingHack <= TIMING_HACK) Timing::setTimingHack(timingHack);
            }
            break;

            case SDLK_ESCAPE:
            {
                SDL_Quit();
                exit(0);
            }
        }
    }

    void updateEditor(SDL_Keycode keyCode)
    {
        int range = 0;
        if(keyCode >= SDLK_0  &&  keyCode <= SDLK_9) range = 1;
        if(keyCode >= SDLK_a  &&  keyCode <= SDLK_f) range = 2;
        if(range == 1  ||  range == 2)
        {
            uint16_t value = 0;    
            switch(range)
            {
                case 1: value = keyCode - SDLK_0;      break;
                case 2: value = keyCode - SDLK_a + 10; break;
            }

            // Edit address
            if(_cursorY == -1)
            {
                // Hex address or load address
                if((_cursorX & 0x01) == 0)
                {
                    // Hex address
                    if(_loadFile == false)
                    {
                        switch(_addressDigit)
                        {
                            case 0: value = (value << 12) & 0xF000; _hexBaseAddress = _hexBaseAddress & 0x0FFF | value; break;
                            case 1: value = (value << 8)  & 0x0F00; _hexBaseAddress = _hexBaseAddress & 0xF0FF | value; break;
                            case 2: value = (value << 4)  & 0x00F0; _hexBaseAddress = _hexBaseAddress & 0xFF0F | value; break;
                            case 3: value = (value << 0)  & 0x000F; _hexBaseAddress = _hexBaseAddress & 0xFFF0 | value; break;
                        }
                    }
                    // Load address
                    else
                    {
                        switch(_addressDigit)
                        {
                            case 0: value = (value << 12) & 0xF000; _loadBaseAddress = _loadBaseAddress & 0x0FFF | value; break;
                            case 1: value = (value << 8)  & 0x0F00; _loadBaseAddress = _loadBaseAddress & 0xF0FF | value; break;
                            case 2: value = (value << 4)  & 0x00F0; _loadBaseAddress = _loadBaseAddress & 0xFF0F | value; break;
                            case 3: value = (value << 0)  & 0x000F; _loadBaseAddress = _loadBaseAddress & 0xFFF0 | value; break;
                        }
                    }
                }
                // Vars address
                else
                {
                    switch(_addressDigit)
                    {
                        case 0: value = (value << 12) & 0xF000; _varsBaseAddress = _varsBaseAddress & 0x0FFF | value; break;
                        case 1: value = (value << 8)  & 0x0F00; _varsBaseAddress = _varsBaseAddress & 0xF0FF | value; break;
                        case 2: value = (value << 4)  & 0x00F0; _varsBaseAddress = _varsBaseAddress & 0xFF0F | value; break;
                        case 3: value = (value << 0)  & 0x000F; _varsBaseAddress = _varsBaseAddress & 0xFFF0 | value; break;
                    }
                }

                _addressDigit = (++_addressDigit) & 0x03;
            }
            // Edit memory
            else if(_hexRomMode == 0  &&  _hexEdit == true)
            {
                uint16_t address = _hexBaseAddress + _cursorX + _cursorY*HEX_CHARS_X;
                switch(_memoryDigit)
                {
                    case 0: value = (value << 4) & 0x00F0; Cpu::setRAM(address, Cpu::getRAM(address) & 0x000F | value); break;
                    case 1: value = (value << 0) & 0x000F; Cpu::setRAM(address, Cpu::getRAM(address) & 0x00F0 | value); break;
                }
                _memoryDigit = (++_memoryDigit) & 0x01;
            }
        }
    }

    void browseDirectory(void)
    {
        std::string path = "./vCPU";
        _directoryNames.clear();

#if defined(_WIN32)
        for(std::experimental::filesystem::directory_iterator next(path), end; next!=end; ++next)
        {
            _directoryNames.push_back(next->path().filename().string());
        }
        std::sort(_directoryNames.begin(), _directoryNames.end());
#else
        for(std::filesystem::directory_iterator next(path), end; next!=end; ++next)
        {
            _directoryNames.push_back(next->path().filename().string());
        }
        std::sort(_directoryNames.begin(), _directoryNames.end());
#endif
    }

    void handleKeyUp(SDL_Keycode keyCode)
    {
        switch(keyCode)
        {
            case SDLK_d:      Cpu::setIN(Cpu::getIN() | INPUT_RIGHT);    break;
            case SDLK_a:      Cpu::setIN(Cpu::getIN() | INPUT_LEFT);     break;
            case SDLK_s:      Cpu::setIN(Cpu::getIN() | INPUT_DOWN);     break;
            case SDLK_w:      Cpu::setIN(Cpu::getIN() | INPUT_UP);       break;
            case SDLK_SPACE:  Cpu::setIN(Cpu::getIN() | INPUT_START);    break;
            case SDLK_z:      Cpu::setIN(Cpu::getIN() | INPUT_SELECT);   break;
            case SDLK_SLASH:  Cpu::setIN(Cpu::getIN() | INPUT_B);        break;
            case SDLK_PERIOD: Cpu::setIN(Cpu::getIN() | INPUT_A);        break;
                   
            // Browse vCPU directory
            case SDLK_l:
            {
                _cursorX = 0; _cursorY = 0;
                _loadFile = !_loadFile;
                if(_loadFile == true) browseDirectory();
            }
            break;

            // Execute vCPU code
            case SDLK_F5:
            {
                Cpu::setRAM(0x0016, _hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x0017, (_hexBaseAddress & 0xFF00) >>8);
                Cpu::setRAM(0x001a, _hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x001b, (_hexBaseAddress & 0xFF00) >>8);
            }
            break;

            // RAM/ROM mode
            case SDLK_r: _hexRomMode = (++_hexRomMode) % 3; break;

            // Toggle hex edit or start an upload
            case SDLK_RETURN: (_loadFile == false  ||  _cursorY == -1) ? _hexEdit = !_hexEdit : Loader::setStartUploading(true); break;
        }

        updateEditor(keyCode);
    }

    void handleInput(void)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_MOUSEWHEEL: handleMouseWheel(event); break;
                case SDL_KEYDOWN: handleKeyDown(event.key.keysym.sym); break;
                case SDL_KEYUP: handleKeyUp(event.key.keysym.sym); break;
                case SDL_QUIT: 
                {
                    SDL_Quit();
                    exit(0);
                }
            }
        }
    }
}