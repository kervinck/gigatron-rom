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
    bool _singleStep = false;
    bool _singleStepMode = false;
    MemoryMode _memoryMode = RAM;
    EditorMode _editorMode = Hex, _prevEditorMode = Hex;
    uint8_t _memoryDigit = 0;
    uint8_t _addressDigit = 0;
    uint16_t _hexBaseAddress = HEX_BASE_ADDRESS;
    uint16_t _loadBaseAddress = LOAD_BASE_ADDRESS;
    uint16_t _varsBaseAddress = VARS_BASE_ADDRESS;
    uint16_t _singleStepWatchAddress = VARS_BASE_ADDRESS;
    int _fileNamesIndex = 0;
    std::vector<std::string> _fileNames;


    int getCursorX(void) {return _cursorX;}
    int getCursorY(void) {return _cursorY;}
    bool getHexEdit(void) {return _hexEdit;}
    bool getSingleStep(void) {return _singleStep;}
    bool getSingleStepMode(void) {return _singleStepMode;}
    MemoryMode getMemoryMode(void) {return _memoryMode;}
    EditorMode getEditorMode(void) {return _editorMode;}
    uint8_t getMemoryDigit(void) {return _memoryDigit;}
    uint8_t getAddressDigit(void) {return _addressDigit;}
    uint16_t getHexBaseAddress(void) {return _hexBaseAddress;}
    uint16_t getLoadBaseAddress(void) {return _loadBaseAddress;}
    uint16_t getVarsBaseAddress(void) {return _varsBaseAddress;}
    uint16_t getSingleStepWatchAddress(void) {return _singleStepWatchAddress;}
    int getFileNamesIndex(void) {return _fileNamesIndex;}
    int getFileNamesSize(void) {return int(_fileNames.size());}
    std::string* getFileName(int index) {return &_fileNames[index % _fileNames.size()];}

    void setCursorX(int x) {_cursorX = x;}
    void setCursorY(int y) {_cursorY = y;}
    void setSingleStep(bool singleStep) {_singleStep = singleStep;}
    void setSingleStepMode(bool singleStepMode) {_singleStepMode = singleStepMode;}
    void setLoadBaseAddress(uint16_t address) {_loadBaseAddress = address;}
    void setSingleStepWatchAddress(uint16_t address) {_singleStepWatchAddress = address;}


    void handleMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0)
        {
            if(_editorMode == Load)
            {
                _fileNamesIndex--;
                if(_fileNamesIndex < 0) _fileNamesIndex = 0;
            }
            else
            {
                _hexBaseAddress = (_hexBaseAddress - HEX_CHARS_X) & (RAM_SIZE-1);
            }
        }
        if(event.wheel.y < 0)
        {
            if(_editorMode == Load)
            {
                if(_fileNames.size() > HEX_CHARS_Y)
                {
                    _fileNamesIndex++;
                    if(_fileNames.size() - _fileNamesIndex < HEX_CHARS_Y) _fileNamesIndex--;
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
        int limitY = (_editorMode != Load) ? HEX_CHARS_Y : int(_fileNames.size());

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
                if(_editorMode == Load)
                {
                    _fileNamesIndex--;
                    if(_fileNamesIndex < 0) _fileNamesIndex = 0;
                }
                else
                {
                    _hexBaseAddress = (_hexBaseAddress - HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
                }
            }
            break;

            case SDLK_PAGEDOWN:
            {
                if(_editorMode == Load)
                {
                    if(_fileNames.size() > HEX_CHARS_Y)
                    {
                        _fileNamesIndex++;
                        if(_fileNames.size() - _fileNamesIndex < HEX_CHARS_Y) _fileNamesIndex--;
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

            // Testing
            case SDLK_F1:
            {
                uint8_t x1 = rand() % GIGA_WIDTH;
                uint8_t x2 = rand() % GIGA_WIDTH;
                uint8_t y1 = rand() % GIGA_HEIGHT;
                uint8_t y2 = rand() % GIGA_HEIGHT;
                uint8_t colour = rand() % 256;
                //Graphics::drawLine(x1, y1, x2, y2, colour);
                Graphics::drawLineGiga(x1, y1, x2, y2, colour);
            }
            break;
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
            if(_cursorY == -1  &&   _hexEdit == true)
            {
                // Hex address or load address
                if((_cursorX & 0x01) == 0)
                {
                    // Hex address
                    if(_editorMode != Load)
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

                        if(_loadBaseAddress < LOAD_BASE_ADDRESS) _loadBaseAddress = LOAD_BASE_ADDRESS;
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
            else if(_memoryMode == RAM  &&  _hexEdit == true)
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
        _fileNames.clear();

#if defined(_WIN32)
        for(std::experimental::filesystem::directory_iterator next(path), end; next!=end; ++next)
        {
            _fileNames.push_back(next->path().filename().string());
        }
        std::sort(_fileNames.begin(), _fileNames.end());
#else
        for(std::filesystem::directory_iterator next(path), end; next!=end; ++next)
        {
            _fileNames.push_back(next->path().filename().string());
        }
        std::sort(_fileNames.begin(), _fileNames.end());
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
                _editorMode = _editorMode == Load ? Hex : Load;
                if(_editorMode == Load) browseDirectory();
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

            // Enter debug mode
            case SDLK_F6:
            {
                _prevEditorMode = _editorMode;
                _editorMode = Debug;
                _singleStepMode = true;
            }
            break;

            // RAM/ROM mode
            case SDLK_r:
            {
                static int memoryMode = RAM;
                memoryMode = (++memoryMode) % NumMemoryModes;
                _memoryMode = (MemoryMode)memoryMode;
            }
            break;

            // Toggle hex edit or start an upload
            case SDLK_RETURN: (_editorMode != Load  ||  _cursorY == -1) ? _hexEdit = !_hexEdit : Loader::setStartUploading(true); break;
        }

        updateEditor(keyCode);
    }

    // Debug mode, handles it's own input and rendering
    bool singleStepDebug(void)
    {
        static uint8_t oldWatch = Cpu::getRAM(_singleStepWatchAddress);

        // Single step
        if(_singleStep == true)
        {
            // Watch variable 
            if(Cpu::getRAM(_singleStepWatchAddress) != oldWatch) 
            {
                _singleStep = false;
                _singleStepMode = true;
            }
            // Update graphics but only once every 16.66667ms
            else
            {
                static uint64_t prevFrameCounter = 0;
                double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());

                Timing::setFrameUpdate(false);
                if(frameTime > TIMING_HACK)
                {
                    Timing::setFrameUpdate(true);
                    Graphics::refreshScreen();
                    Graphics::render(false);
                    prevFrameCounter = SDL_GetPerformanceCounter();
                }
            }
        }

        // Pause simulation and handle debugging keys
        while(_singleStepMode == true)
        {
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_KEYUP:
                    {
                        // Leave debug mode
                        switch(event.key.keysym.sym)
                        {
                            case SDLK_F6:
                            {
                                _editorMode = _prevEditorMode;
                                _singleStepMode = false;
                            }
                            break; 
                        }
                    }
                    break;

                    case SDL_KEYDOWN:
                    {
                        switch(event.key.keysym.sym)
                        {
                            // Single step
                            case SDLK_F10:
                            {
                                _singleStep = true;
                                _singleStepMode = false;
                                oldWatch = Cpu::getRAM(_singleStepWatchAddress);
                            }
                            break;

                            // Quit
                            case SDLK_ESCAPE:
                            {
                                SDL_Quit();
                                exit(0);
                            }
                        }
                    }
                    break;
                }
            }
        }

        return _singleStep;
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