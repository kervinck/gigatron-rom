#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>


#if defined(_WIN32)
#include <direct.h>
#include "dirent/dirent.h"
#define getcwd _getcwd
#undef max
#undef min
#else
#include <unistd.h>
#include <dirent.h>
#endif


#include <SDL.h>
#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "graphics.h"
#include "assembler.h"
#include "expression.h"
#include "inih/INIReader.h"


namespace Editor
{
    struct FileEntry
    {
        FileType _fileType;
        std::string _name;
    };


    int _cursorX = 0;
    int _cursorY = 0;

    bool _hexEdit = false;
    bool _startMusic = false;
    bool _gigatronMode = false;
    bool _ps2KeyboardMode = false;

    bool _singleStep = false;
    bool _singleStepMode = false;
    uint32_t _singleStepTicks = 0;
    uint8_t _singleStepWatch = 0x00;

    const std::string _srcPath = "/";
    std::string _cwdPath = "";
    std::string _filePath = "";


    MemoryMode _memoryMode = RAM;
    EditorMode _editorMode = Hex, _prevEditorMode = Hex;
    uint8_t _memoryDigit = 0;
    uint8_t _addressDigit = 0;
    uint16_t _hexBaseAddress = HEX_BASE_ADDRESS;
    uint16_t _loadBaseAddress = LOAD_BASE_ADDRESS;
    uint16_t _varsBaseAddress = VARS_BASE_ADDRESS;
    uint16_t _singleStepWatchAddress = VIDEO_Y_ADDRESS;
    uint16_t _cpuUsageAddressA = HEX_BASE_ADDRESS;
    uint16_t _cpuUsageAddressB = HEX_BASE_ADDRESS + 0x0020;
    
    int _fileEntriesIndex = 0;
    std::vector<FileEntry> _fileEntries;

    INIReader _iniReader;
    uint16_t _sdlKeyModifier = 0;
    std::map<std::string, int> _sdlKeys;
    std::map<std::string, int> _inputKeys;


    int getCursorX(void) {return _cursorX;}
    int getCursorY(void) {return _cursorY;}
    bool getHexEdit(void) {return _hexEdit;}
    bool getStartMusic(void) {return _startMusic;}
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
    uint16_t getCpuUsageAddressA(void) {return _cpuUsageAddressA;}
    uint16_t getCpuUsageAddressB(void) {return _cpuUsageAddressB;}
    int getFileEntriesIndex(void) {return _fileEntriesIndex;}
    int getFileEntriesSize(void) {return int(_fileEntries.size());}
    std::string getBrowserPath(void) {return _filePath;}
    FileType getFileEntryType(int index) {return _fileEntries[index % _fileEntries.size()]._fileType;}
    std::string* getFileEntryName(int index) {return &_fileEntries[index % _fileEntries.size()]._name;}

    void setCursorX(int x) {_cursorX = x;}
    void setCursorY(int y) {_cursorY = y;}
    void setStartMusic(bool startMusic) {_startMusic = startMusic;}
    void setSingleStep(bool singleStep) {_singleStep = singleStep;}
    void setSingleStepMode(bool singleStepMode) {_singleStepMode = singleStepMode;}
    void setLoadBaseAddress(uint16_t address) {_loadBaseAddress = address;}
    void setSingleStepWatchAddress(uint16_t address) {_singleStepWatchAddress = address;}
    void setCpuUsageAddressA(uint16_t address) {_cpuUsageAddressA = address;}
    void setCpuUsageAddressB(uint16_t address) {_cpuUsageAddressB = address;}

    bool scanCodeFromIniKey(const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, int& scanCode)
    {
        std::string key = _iniReader.Get(sectionString, iniKey, defaultKey);
        key = Expression::strToUpper(key);
        if(_sdlKeys.find(key) == _sdlKeys.end())
        {
            fprintf(stderr, "Editor::initialise() : key %s not recognised in INI file '%s' : reverting to default key %s.\n", key.c_str(), INPUT_CONFIG_INI, defaultKey.c_str());
            scanCode = _sdlKeys[defaultKey];
            return false;
        }

        scanCode = _sdlKeys[key];
        return true;
    }

    void initialise(void)
    {
        // Current working directory
        char cwdPath[FILENAME_MAX];
        getcwd(cwdPath, FILENAME_MAX);
        _cwdPath = std::string(cwdPath);
        _filePath = _cwdPath + _srcPath;

        // Keyboard to SDL key mapping
        _sdlKeys["ENTER"]            = SDLK_RETURN;
        _sdlKeys["A"]                = SDLK_a;
        _sdlKeys["B"]                = SDLK_b;
        _sdlKeys["C"]                = SDLK_c;
        _sdlKeys["D"]                = SDLK_d;
        _sdlKeys["E"]                = SDLK_e;
        _sdlKeys["F"]                = SDLK_f;
        _sdlKeys["G"]                = SDLK_g;
        _sdlKeys["H"]                = SDLK_h;
        _sdlKeys["I"]                = SDLK_i;
        _sdlKeys["J"]                = SDLK_j;
        _sdlKeys["K"]                = SDLK_k;
        _sdlKeys["L"]                = SDLK_l;
        _sdlKeys["M"]                = SDLK_m;
        _sdlKeys["N"]                = SDLK_n;
        _sdlKeys["O"]                = SDLK_o;
        _sdlKeys["P"]                = SDLK_p;
        _sdlKeys["Q"]                = SDLK_q;
        _sdlKeys["R"]                = SDLK_r;
        _sdlKeys["S"]                = SDLK_s;
        _sdlKeys["T"]                = SDLK_t;
        _sdlKeys["U"]                = SDLK_u;
        _sdlKeys["V"]                = SDLK_v;
        _sdlKeys["W"]                = SDLK_w;
        _sdlKeys["X"]                = SDLK_x;
        _sdlKeys["Y"]                = SDLK_y;
        _sdlKeys["Z"]                = SDLK_z;
        _sdlKeys["1"]                = SDLK_1;
        _sdlKeys["!"]                = SDLK_1;
        _sdlKeys["2"]                = SDLK_2;
        _sdlKeys["@"]                = SDLK_2;
        _sdlKeys["3"]                = SDLK_3;
        _sdlKeys["#"]                = SDLK_3;
        _sdlKeys["4"]                = SDLK_4;
        _sdlKeys["$"]                = SDLK_4;
        _sdlKeys["5"]                = SDLK_5;
        _sdlKeys["%"]                = SDLK_5;
        _sdlKeys["6"]                = SDLK_6;
        _sdlKeys["^"]                = SDLK_6;
        _sdlKeys["7"]                = SDLK_7;
        _sdlKeys["&"]                = SDLK_7;
        _sdlKeys["8"]                = SDLK_8;
        _sdlKeys["*"]                = SDLK_8;
        _sdlKeys["9"]                = SDLK_9;
        _sdlKeys["("]                = SDLK_9;
        _sdlKeys["0"]                = SDLK_0;
        _sdlKeys[")"]                = SDLK_0;
        _sdlKeys["F1"]               = SDLK_F1;
        _sdlKeys["F2"]               = SDLK_F2;
        _sdlKeys["F3"]               = SDLK_F3;
        _sdlKeys["F4"]               = SDLK_F4;
        _sdlKeys["F5"]               = SDLK_F5;
        _sdlKeys["F6"]               = SDLK_F6;
        _sdlKeys["F7"]               = SDLK_F7;
        _sdlKeys["F8"]               = SDLK_F8;
        _sdlKeys["F9"]               = SDLK_F9;
        _sdlKeys["F10"]              = SDLK_F10;
        _sdlKeys["F11"]              = SDLK_F11;
        _sdlKeys["F12"]              = SDLK_F12;
        _sdlKeys["SPACE"]            = SDLK_SPACE;
        _sdlKeys["BACKSPACE"]        = SDLK_BACKSPACE;
        _sdlKeys["TAB"]              = SDLK_TAB;
        _sdlKeys["_"]                = SDLK_MINUS;
        _sdlKeys["-"]                = SDLK_MINUS;
        _sdlKeys["+"]                = SDLK_EQUALS;
        _sdlKeys["="]                = SDLK_EQUALS;
        _sdlKeys["`"]                = SDLK_BACKQUOTE;
        _sdlKeys["~"]                = SDLK_BACKQUOTE;
        _sdlKeys["<"]                = SDLK_COMMA;
        _sdlKeys[","]                = SDLK_COMMA;
        _sdlKeys[">"]                = SDLK_PERIOD;
        _sdlKeys["."]                = SDLK_PERIOD;
        _sdlKeys["["]                = SDLK_LEFTBRACKET;
        _sdlKeys["{"]                = SDLK_LEFTBRACKET;
        _sdlKeys["]"]                = SDLK_RIGHTBRACKET;
        _sdlKeys["}"]                = SDLK_RIGHTBRACKET;
        _sdlKeys[";"]                = SDLK_SEMICOLON;
        _sdlKeys[":"]                = SDLK_SEMICOLON;
        _sdlKeys["'"]                = SDLK_QUOTE;
        _sdlKeys["\""]               = SDLK_QUOTE;
        _sdlKeys["\\"]               = SDLK_BACKSLASH;
        _sdlKeys["|"]                = SDLK_BACKSLASH;
        _sdlKeys["/"]                = SDLK_SLASH;
        _sdlKeys["?"]                = SDLK_SLASH;
        _sdlKeys["LEFT"]             = SDLK_LEFT;
        _sdlKeys["RIGHT"]            = SDLK_RIGHT;
        _sdlKeys["UP"]               = SDLK_UP;
        _sdlKeys["DOWN"]             = SDLK_DOWN;
        _sdlKeys["PAGEUP"]           = SDLK_PAGEUP;
        _sdlKeys["PAGEDOWN"]         = SDLK_PAGEDOWN;
        _sdlKeys["CAPSLOCK"]         = SDLK_CAPSLOCK;
        _sdlKeys["PRINTSCREEN"]      = SDLK_PRINTSCREEN;
        _sdlKeys["SCROLLLOCK"]       = SDLK_SCROLLLOCK;
        _sdlKeys["ESCAPE"]           = SDLK_ESCAPE;
        _sdlKeys["PAUSE"]            = SDLK_PAUSE;
        _sdlKeys["INSERT"]           = SDLK_INSERT;
        _sdlKeys["HOME"]             = SDLK_HOME;
        _sdlKeys["DELETE"]           = SDLK_DELETE;
        _sdlKeys["END"]              = SDLK_END;
        _sdlKeys["NUMLOCK"]          = SDLK_NUMLOCKCLEAR;
        _sdlKeys["KP_DIVIDE"]        = SDLK_KP_DIVIDE;
        _sdlKeys["KP_MULTIPLY"]      = SDLK_KP_MULTIPLY;
        _sdlKeys["KP_MINUS"]         = SDLK_KP_MINUS; 
        _sdlKeys["KP_PLUS"]          = SDLK_KP_PLUS;
        _sdlKeys["KP_ENTER"]         = SDLK_KP_ENTER;
        _sdlKeys["KP_1"]             = SDLK_KP_1;
        _sdlKeys["KP_2"]             = SDLK_KP_2;
        _sdlKeys["KP_3"]             = SDLK_KP_3;
        _sdlKeys["KP_4"]             = SDLK_KP_4;
        _sdlKeys["KP_5"]             = SDLK_KP_5;
        _sdlKeys["KP_6"]             = SDLK_KP_6;
        _sdlKeys["KP_7"]             = SDLK_KP_7;
        _sdlKeys["KP_8"]             = SDLK_KP_8;
        _sdlKeys["KP_9"]             = SDLK_KP_9;
        _sdlKeys["KP_0"]             = SDLK_KP_0;
        _sdlKeys["KP_PERIOD"]        = SDLK_KP_PERIOD;
        _sdlKeys["LCTRL"]            = SDLK_LCTRL;
        _sdlKeys["LSHIFT"]           = SDLK_LSHIFT;
        _sdlKeys["LALT"]             = SDLK_LALT;
        _sdlKeys["LGUI"]             = SDLK_LGUI;
        _sdlKeys["RCTRL"]            = SDLK_RCTRL;
        _sdlKeys["RSHIFT"]           = SDLK_RSHIFT;
        _sdlKeys["RALT"]             = SDLK_RALT;
        _sdlKeys["RGUI"]             = SDLK_RGUI;

        // Default INI key to SDL key mapping
        _inputKeys["Edit"]         = SDLK_RETURN;
        _inputKeys["RAM_Mode"]     = SDLK_r;
        _inputKeys["Execute"]      = SDLK_F5;
        _inputKeys["Left"]         = SDLK_LEFT;
        _inputKeys["Right"]        = SDLK_RIGHT;
        _inputKeys["Up"]           = SDLK_UP;
        _inputKeys["Down"]         = SDLK_DOWN;
        _inputKeys["PageUp"]       = SDLK_PAGEUP;
        _inputKeys["PageDown"]     = SDLK_PAGEDOWN;
        _inputKeys["Load"]         = SDLK_l;
        _inputKeys["Help"]         = SDLK_h;
        _inputKeys["Quit"]         = SDLK_ESCAPE;
        _inputKeys["Reset"]        = SDLK_F1;
        _inputKeys["ScanlineMode"] = SDLK_F3;
        _inputKeys["Speed+"]       = SDLK_EQUALS;
        _inputKeys["Speed-"]       = SDLK_MINUS;
        _inputKeys["Giga_Left"]    = SDLK_a;
        _inputKeys["Giga_Right"]   = SDLK_d;
        _inputKeys["Giga_Up"]      = SDLK_w;
        _inputKeys["Giga_Down"]    = SDLK_s;
        _inputKeys["Giga_Start"]   = SDLK_SPACE;
        _inputKeys["Giga_Select"]  = SDLK_z;
        _inputKeys["Giga_A"]       = SDLK_GREATER;
        _inputKeys["Giga_B"]       = SDLK_SLASH;
        _inputKeys["Debug"]        = SDLK_F6;
        _inputKeys["Step"]         = SDLK_F10;
        _inputKeys["Giga"]         = SDLK_F12;
        _inputKeys["PS2_KB"]       = SDLK_F4;

        // Input configuration
        INIReader iniReader(INPUT_CONFIG_INI);
        _iniReader = iniReader;
        if(_iniReader.ParseError() < 0)
        {
            fprintf(stderr, "Editor::initialise() : couldn't load INI file '%s' : reverting to default keys.\n", INPUT_CONFIG_INI);
            return;
        }

        // Parse input keys INI file
        enum Section {Monitor, Browser, Emulator, Gigatron, Debugger};
        std::map<std::string, Section> section;
        section["Monitor"]  = Monitor;
        section["Browser"]  = Browser;
        section["Emulator"] = Emulator;
        section["Gigatron"] = Gigatron;
        section["Debugger"] = Debugger;
        for(auto sectionString : _iniReader.Sections())
        {
            if(section.find(sectionString) == section.end())
            {
                fprintf(stderr, "Editor::initialise() : INI file '%s' has bad Sections : reverting to default keys.\n", INPUT_CONFIG_INI);
                break;
            }

            switch(section[sectionString])
            {
                case Monitor:
                {
                    scanCodeFromIniKey(sectionString, "Edit",     "ENTER",    _inputKeys["Edit"]);
                    scanCodeFromIniKey(sectionString, "RAM_Mode", "R",        _inputKeys["RAM_Mode"]);
                    scanCodeFromIniKey(sectionString, "Execute",  "F5",       _inputKeys["Execute"]);
                    scanCodeFromIniKey(sectionString, "Left",     "LEFT",     _inputKeys["Left"]);
                    scanCodeFromIniKey(sectionString, "Right",    "RIGHT",    _inputKeys["Right"]);
                    scanCodeFromIniKey(sectionString, "Up",       "UP",       _inputKeys["Up"]);
                    scanCodeFromIniKey(sectionString, "Down",     "DOWN",     _inputKeys["Down"]);
                    scanCodeFromIniKey(sectionString, "PageUp",   "PAGEUP",   _inputKeys["PageUp"]);
                    scanCodeFromIniKey(sectionString, "PageDown", "PAGEDOWN", _inputKeys["PageDown"]);
                }
                break;

                case Browser:
                {
                    scanCodeFromIniKey(sectionString, "Load", "L", _inputKeys["Load"]);
                }
                break;

                case Emulator:
                {
                    scanCodeFromIniKey(sectionString, "Help",         "H",      _inputKeys["Help"]);
                    scanCodeFromIniKey(sectionString, "Quit",         "ESCAPE", _inputKeys["Quit"]);
                    scanCodeFromIniKey(sectionString, "Reset",        "F1",     _inputKeys["Reset"]);
                    scanCodeFromIniKey(sectionString, "ScanlineMode", "F3",     _inputKeys["ScanlineMode"]);
                    scanCodeFromIniKey(sectionString, "Speed+",       "+",      _inputKeys["Speed+"]);
                    scanCodeFromIniKey(sectionString, "Speed-",       "-",      _inputKeys["Speed-"]);
                    scanCodeFromIniKey(sectionString, "PS2_KB",       "F4",     _inputKeys["PS2_KB"]);
                }
                break;

                case Gigatron:
                {
                    scanCodeFromIniKey(sectionString, "Giga_Left",   "A",     _inputKeys["Giga_Left"]);
                    scanCodeFromIniKey(sectionString, "Giga_Right",  "D",     _inputKeys["Giga_Right"]);
                    scanCodeFromIniKey(sectionString, "Giga_Up",     "W",     _inputKeys["Giga_Up"]);
                    scanCodeFromIniKey(sectionString, "Giga_Down",   "S",     _inputKeys["Giga_Down"]);
                    scanCodeFromIniKey(sectionString, "Giga_Start",  "SPACE", _inputKeys["Giga_Start"]);
                    scanCodeFromIniKey(sectionString, "Giga_Select", "Z",     _inputKeys["Giga_Select"]);
                    scanCodeFromIniKey(sectionString, "Giga_A",      ".",     _inputKeys["Giga_A"]);
                    scanCodeFromIniKey(sectionString, "Giga_B",      "/",     _inputKeys["Giga_B"]);
                    scanCodeFromIniKey(sectionString, "Giga",        "F12",   _inputKeys["Giga"]);
                }
                break;

                case Debugger:
                {
                    scanCodeFromIniKey(sectionString, "Debug", "F6", _inputKeys["Debug"]);
                    scanCodeFromIniKey(sectionString, "Step", "F10", _inputKeys["Step"]);
                }
                break;
            }
        }
    }

    void handleMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0)
        {
            if(_editorMode == Load)
            {
                _fileEntriesIndex--;
                if(_fileEntriesIndex < 0) _fileEntriesIndex = 0;
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
                if(_fileEntries.size() > HEX_CHARS_Y)
                {
                    _fileEntriesIndex++;
                    if(_fileEntries.size() - _fileEntriesIndex < HEX_CHARS_Y) _fileEntriesIndex--;
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
        // PS2 Keyboard emulation mode
        if(_ps2KeyboardMode)
        {
            if(keyCode >= 32  &&  keyCode <= 127)
            {
                Cpu::setIN(keyCode);
                return;
            }
        }

        int limitY = (_editorMode != Load) ? HEX_CHARS_Y : std::min(int(_fileEntries.size()), HEX_CHARS_Y);
        if(keyCode == _inputKeys["Giga_Left"])        (_gigatronMode) ? Loader::sendCommandToGiga('A', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_LEFT);
        else if(keyCode == _inputKeys["Giga_Right"])  (_gigatronMode) ? Loader::sendCommandToGiga('D', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_RIGHT); 
        else if(keyCode == _inputKeys["Giga_Up"])     (_gigatronMode) ? Loader::sendCommandToGiga('W', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_UP);    
        else if(keyCode == _inputKeys["Giga_Down"])   (_gigatronMode) ? Loader::sendCommandToGiga('S', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_DOWN);  
        else if(keyCode == _inputKeys["Giga_Start"])  (_gigatronMode) ? Loader::sendCommandToGiga('E', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_START); 
        else if(keyCode == _inputKeys["Giga_Select"]) (_gigatronMode) ? Loader::sendCommandToGiga('Q', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_SELECT);
        else if(keyCode == _inputKeys["Giga_A"])      (_gigatronMode) ? Loader::sendCommandToGiga('Z', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_A);     
        else if(keyCode == _inputKeys["Giga_B"])      (_gigatronMode) ? Loader::sendCommandToGiga('X', true) : Cpu::setIN(Cpu::getIN() & ~INPUT_B);     

        else if(keyCode == _inputKeys["Left"])  {_cursorX = (--_cursorX < 0) ? HEX_CHARS_X-1 : _cursorX;  _memoryDigit = 0; _addressDigit = 0;}
        else if(keyCode == _inputKeys["Right"]) {_cursorX = (++_cursorX >= HEX_CHARS_X) ? 0  : _cursorX;  _memoryDigit = 0; _addressDigit = 0;}
        else if(keyCode == _inputKeys["Up"])    {_cursorY = (--_cursorY < -2) ? limitY-1     : _cursorY;  _memoryDigit = 0; _addressDigit = 0;}
        else if(keyCode == _inputKeys["Down"])  {_cursorY = (++_cursorY >= limitY) ? 0       : _cursorY;  _memoryDigit = 0; _addressDigit = 0;}

        else if(keyCode == _inputKeys["PageUp"])
        {
            if(_editorMode == Load)
            {
                _fileEntriesIndex--;
                if(_fileEntriesIndex < 0) _fileEntriesIndex = 0;
            }
            else
            {
                _hexBaseAddress = (_hexBaseAddress - HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
            }
        }

        else if(keyCode == _inputKeys["PageDown"])
        {
            if(_editorMode == Load)
            {
                if(_fileEntries.size() > HEX_CHARS_Y)
                {
                    _fileEntriesIndex++;
                    if(_fileEntries.size() - _fileEntriesIndex < HEX_CHARS_Y) _fileEntriesIndex--;
                }
            }
            else
            {
                _hexBaseAddress = (_hexBaseAddress + HEX_CHARS_X*HEX_CHARS_Y) & (RAM_SIZE-1);
            }
        }

        else if(keyCode == _inputKeys["Speed+"])
        {
            double timingHack = Timing::getTimingHack() - VSYNC_TIMING_60*0.05;
            if(timingHack >= 0.0) Timing::setTimingHack(timingHack);
        }

        else if(keyCode == _inputKeys["Speed-"])
        {
            double timingHack = Timing::getTimingHack() + VSYNC_TIMING_60*0.05;
            if(timingHack <= VSYNC_TIMING_60) Timing::setTimingHack(timingHack);
        }

        else if(keyCode == _inputKeys["Quit"])
        {
            SDL_Quit();
            exit(0);
        }

        // Fast reset
        else if(keyCode == _inputKeys["Reset"])
        {
            if(!_singleStepMode)
            {
                (_sdlKeyModifier & KMOD_CTRL) ? Loader::sendCommandToGiga('R', false) : Cpu::reset();
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

            // Edit cpu usage addresses
            if(_cursorY == -2  &&   _hexEdit)
            {
                // A address or B address
                if((_cursorX & 0x01) == 0)
                {
                    // A address
                    switch(_addressDigit)
                    {
                        case 0: value = (value << 12) & 0xF000; _cpuUsageAddressA = _cpuUsageAddressA & 0x0FFF | value; break;
                        case 1: value = (value << 8)  & 0x0F00; _cpuUsageAddressA = _cpuUsageAddressA & 0xF0FF | value; break;
                        case 2: value = (value << 4)  & 0x00F0; _cpuUsageAddressA = _cpuUsageAddressA & 0xFF0F | value; break;
                        case 3: value = (value << 0)  & 0x000F; _cpuUsageAddressA = _cpuUsageAddressA & 0xFFF0 | value; break;
                    }
                }
                else
                {
                    // B address
                    switch(_addressDigit)
                    {
                        case 0: value = (value << 12) & 0xF000; _cpuUsageAddressB = _cpuUsageAddressB & 0x0FFF | value; break;
                        case 1: value = (value << 8)  & 0x0F00; _cpuUsageAddressB = _cpuUsageAddressB & 0xF0FF | value; break;
                        case 2: value = (value << 4)  & 0x00F0; _cpuUsageAddressB = _cpuUsageAddressB & 0xFF0F | value; break;
                        case 3: value = (value << 0)  & 0x000F; _cpuUsageAddressB = _cpuUsageAddressB & 0xFFF0 | value; break;
                    }
                }

                _addressDigit = (++_addressDigit) & 0x03;
            }
            // Edit load/vars addresses
            else if(_cursorY == -1  &&   _hexEdit)
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
            else if(_memoryMode == RAM  &&  _hexEdit)
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

    void backOneDirectory(void)
    {
        size_t slash = _filePath.find_last_of("\\/", _filePath.size()-2);
        if(slash != std::string::npos)
        {
            _filePath = _filePath.substr(0, slash + 1);
        }
    }

    void browseDirectory(void)
    {
        std::string path = _filePath  + ".";
        Assembler::setIncludePath(_filePath);

        _fileEntries.clear();

        DIR *dir;
        struct dirent *ent;
        std::vector<std::string> dirnames;
        dirnames.push_back("..");
        std::vector<std::string> filenames;
        if((dir = opendir(path.c_str())) != NULL)
        {
            while((ent = readdir(dir)) != NULL)
            {
                std::string name = std::string(ent->d_name);
                size_t nonWhiteSpace = name.find_first_not_of("  \n\r\f\t\v");
                if(ent->d_type == DT_DIR  &&  name[0] != '.'  &&  name.find("$RECYCLE") == std::string::npos  &&  nonWhiteSpace != std::string::npos)
                {
                    dirnames.push_back(name);
                }
                else if(ent->d_type == DT_REG  &&  (name.find(".vasm") != std::string::npos  ||  name.find(".gt1") != std::string::npos))
                {
                    filenames.push_back(name);
                }
            }
            closedir (dir);
        }

        std::sort(dirnames.begin(), dirnames.end());
        for(int i=0; i<dirnames.size(); i++)
        {
            FileEntry fileEntry = {Dir, dirnames[i]};
            _fileEntries.push_back(fileEntry);
        }

        std::sort(filenames.begin(), filenames.end());
        for(int i=0; i<filenames.size(); i++)
        {
            FileEntry fileEntry = {File, filenames[i]};
            _fileEntries.push_back(fileEntry);
        }

        _fileEntriesIndex = 0;
    }

    void changeBrowseDirectory(void)
    {
        std::string entry = *getFileEntryName(getCursorY() + _fileEntriesIndex);
        setCursorY(0);

        if (entry != "..")
            _filePath += entry + "/";
        else
            backOneDirectory();

        browseDirectory();
    }

    void handleKeyUp(SDL_Keycode keyCode)
    {
        // PS2 Keyboard emulation mode
        if(_ps2KeyboardMode)
        {
            if(keyCode >= 32  &&  keyCode <= 127)
            {
                Cpu::setIN(0xFF);
                return;
            }
        }

        if(keyCode == _inputKeys["Giga_Left"])        {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_LEFT);  }
        else if(keyCode == _inputKeys["Giga_Right"])  {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_RIGHT); }
        else if(keyCode == _inputKeys["Giga_Up"])     {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_UP);    }
        else if(keyCode == _inputKeys["Giga_Down"])   {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_DOWN);  }
        else if(keyCode == _inputKeys["Giga_Start"])  {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_START); }
        else if(keyCode == _inputKeys["Giga_Select"]) {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_SELECT);}
        else if(keyCode == _inputKeys["Giga_A"])      {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_A);     }
        else if(keyCode == _inputKeys["Giga_B"])      {if(!_gigatronMode) Cpu::setIN(Cpu::getIN() | INPUT_B);     }
                   
        // Browse vCPU directory
        else if(keyCode == _inputKeys["Load"])
        {
            if(!_singleStepMode)
            {
                _cursorX = 0; _cursorY = 0;
                _editorMode = _editorMode == Load ? Hex : Load;
                if(_editorMode == Load) browseDirectory();
            }
        }

        // Execute vCPU code
        else if(keyCode == _inputKeys["Execute"])
        {
            if(!_singleStepMode)
            {
                Cpu::setRAM(0x0016, _hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x0017, (_hexBaseAddress & 0xFF00) >>8);
                Cpu::setRAM(0x001a, _hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x001b, (_hexBaseAddress & 0xFF00) >>8);
            }
        }

        // Enter debug mode
        else if(keyCode == _inputKeys["Debug"])
        {
            _prevEditorMode = _editorMode;
            _editorMode = Debug;
            _singleStepMode = true;
        }

        // Enter Gigatron mode
        else if(keyCode == _inputKeys["Giga"])
        {
            if(_editorMode != Giga)
            {
                _prevEditorMode = _editorMode;
                _editorMode = Giga;
                _gigatronMode = true;
                _ps2KeyboardMode = false;
            }
            else
            {
                _editorMode = (_prevEditorMode != Giga) ? _prevEditorMode : Hex;
                _gigatronMode = false;
                if(_prevEditorMode == PS2KB) _ps2KeyboardMode = true;
            }
        }

        // Enter PS2 Keyboard mode
        else if(keyCode == _inputKeys["PS2_KB"])
        {
            if(_editorMode != PS2KB)
            {
                _prevEditorMode = _editorMode;
                _editorMode = PS2KB;
                _ps2KeyboardMode = true;
                _gigatronMode = false;
            }
            else
            {
                _editorMode = (_prevEditorMode != PS2KB) ? _prevEditorMode : Hex;
                _ps2KeyboardMode = false;
                if(_prevEditorMode == Giga) _gigatronMode = true;
            }
        }

        // RAM/ROM mode
        else if(keyCode ==  _inputKeys["RAM_Mode"])
        {
            static int memoryMode = RAM;
            memoryMode = (++memoryMode) % NumMemoryModes;
            _memoryMode = (MemoryMode)memoryMode;
        }

        // Toggle hex edit or start an upload
        else if(keyCode == _inputKeys["Edit"])
        {
            if(_editorMode != Load  ||  _cursorY < 0)
            {
                _hexEdit = !_hexEdit;
            }
            else
            {
                FileType fileType = getFileEntryType(getCursorY() + _fileEntriesIndex);
                switch(fileType)
                {
                    case File: (_sdlKeyModifier & KMOD_CTRL) ? Loader::setUploadTarget(Loader::Hardware) : Loader::setUploadTarget(Loader::Emulator); break;
                    case Dir: changeBrowseDirectory(); break;
                }
            }
        }

        // Toggle help screen
        else if(keyCode == _inputKeys["Help"])
        {
            static bool helpScreen = false;
            helpScreen = !helpScreen;
            Graphics::setDisplayHelpScreen(helpScreen);
        }

        else if(keyCode == _inputKeys["ScanlineMode"])
        {
            static int scanlineMode = Cpu::ScanlineMode::Normal;
            if(++scanlineMode == Cpu::ScanlineMode::NumScanlineModes-1) scanlineMode = Cpu::ScanlineMode::Normal;
            Cpu::setScanlineMode((Cpu::ScanlineMode)scanlineMode);
        }

        else if(keyCode == SDLK_F2)
        {
            //_startMusic = !_startMusic;
        }

        else if(keyCode == SDLK_F4)
        {
            //Audio::nextScore();
        }

        updateEditor(keyCode);
    }

    // Debug mode, handles it's own input and rendering
    bool singleStepDebug(void)
    {
        // Gprintfs
        Assembler::printGprintfStrings();

        // Single step
        if(_singleStep)
        {
            // Timeout on change of variable
            if(SDL_GetTicks() - _singleStepTicks > SINGLE_STEP_STALL_TIME)
            {
                setSingleStep(false); 
                setSingleStepMode(false);
                fprintf(stderr, "Editor::singleStepDebug() : Single step stall for %d milliseconds : exiting debugger.\n", SDL_GetTicks() - _singleStepTicks);
            }
            // Watch variable 
            else if(Cpu::getRAM(_singleStepWatchAddress) != _singleStepWatch) 
            {
                _singleStep = false;
                _singleStepMode = true;
            }
        }

        // Pause simulation and handle debugging keys
        while(_singleStepMode)
        {
            // Update graphics but only once every 16.66667ms
            static uint64_t prevFrameCounter = 0;
            double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());

            Timing::setFrameUpdate(false);
            if(frameTime > VSYNC_TIMING_60)
            {
                prevFrameCounter = SDL_GetPerformanceCounter();
                Timing::setFrameUpdate(true);
                Graphics::refreshScreen();
                Graphics::render(false);
            }

            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_MOUSEWHEEL: handleMouseWheel(event); break;

                    case SDL_KEYUP:
                    {
                        // Leave debug mode
                        if(event.key.keysym.sym == _inputKeys["Debug"])
                        {
                            _singleStep = false;
                            _singleStepMode = false;
                            _editorMode = (_prevEditorMode != Debug) ? _prevEditorMode : Hex;
                        }
                        else
                        {
                            handleKeyUp(event.key.keysym.sym);
                        }
                    }
                    break;

                    case SDL_KEYDOWN:
                    {
                        // Single step
                        if(event.key.keysym.sym == _inputKeys["Step"])
                        {
                            _singleStep = true;
                            _singleStepMode = false;
                            _singleStepTicks = SDL_GetTicks();
                            _singleStepWatch = Cpu::getRAM(_singleStepWatchAddress);
                        }
                        else
                        {
                            handleKeyDown(event.key.keysym.sym);
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
            _sdlKeyModifier = event.key.keysym.mod;
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

        // Updates current game's high score once per second, (assuming handleInput is called in vertical blank)
        Loader::updateHighScore();
    }
}
