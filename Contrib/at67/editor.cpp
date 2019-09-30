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
#include "memory.h"
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
    struct KeyCodeMod
    {
        int scanCode;
        SDL_Keymod modifier;
    };

    struct FileEntry
    {
        FileType _fileType;
        std::string _name;
    };


    int _cursorX = 0;
    int _cursorY = 0;

    bool _hexEdit = false;
    bool _startMusic = false;
    bool _ps2KeyboardDown = false;

    SDL_Keycode _sdlKeyScanCode = 0;
    uint16_t _sdlKeyModifier = 0;

    bool _singleStep = false;
    bool _singleStepMode = false;
    uint32_t _singleStepTicks = 0;
    uint8_t _singleStepWatch = 0x00;

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
    
    int _fileEntriesSize = 0;
    int _fileEntriesIndex = 0;
    std::vector<FileEntry> _fileEntries;

    INIReader _iniReader;
    std::map<std::string, int> _sdlKeys;
    std::map<std::string, KeyCodeMod> _inputKeys;


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
    FileType getFileEntryType(int index) {return _fileEntries[index % _fileEntries.size()]._fileType;}
    FileType getCurrentFileEntryType(void) {return _fileEntries[(_cursorY + _fileEntriesIndex) % _fileEntries.size()]._fileType;}
    std::string* getFileEntryName(int index) {return &_fileEntries[index % _fileEntries.size()]._name;}
    std::string* getCurrentFileEntryName(void) {return &_fileEntries[(_cursorY + _fileEntriesIndex) % _fileEntries.size()]._name;}
    std::string getBrowserPath(bool removeSlash)
    {
        std::string str = _filePath;
        if(removeSlash  &&  str.length()) str.erase(str.length()-1);
        return str;
    }

    void setCursorX(int x) {_cursorX = x;}
    void setCursorY(int y) {_cursorY = y;}
    void setStartMusic(bool startMusic) {_startMusic = startMusic;}
    void setSingleStep(bool singleStep) {_singleStep = singleStep;}
    void setSingleStepMode(bool singleStepMode) {_singleStepMode = singleStepMode;}
    void setLoadBaseAddress(uint16_t address) {_loadBaseAddress = address;}
    void setSingleStepWatchAddress(uint16_t address) {_singleStepWatchAddress = address;}
    void setCpuUsageAddressA(uint16_t address) {_cpuUsageAddressA = address;}
    void setCpuUsageAddressB(uint16_t address) {_cpuUsageAddressB = address;}


    bool scanCodeFromIniKey(const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, KeyCodeMod& keyCodeMod)
    {
        keyCodeMod.modifier = KMOD_NONE;

        std::string mod;
        std::string key = _iniReader.Get(sectionString, iniKey, defaultKey);
        key = Expression::strToUpper(key);

        // Parse CTRL or ALT
        size_t keyPos = key.find("+");
        if(keyPos != std::string::npos  &&  keyPos != key.length()-1)
        {
            mod = key.substr(0, keyPos);
            key = key.substr(keyPos + 1);
            if(mod == "ALT")  keyCodeMod.modifier = KMOD_LALT;
            if(mod == "CTRL") keyCodeMod.modifier = KMOD_LCTRL;
        }

        if(_sdlKeys.find(key) == _sdlKeys.end())
        {
            fprintf(stderr, "Editor::scanCodeFromIniKey() : key %s not recognised in INI file '%s' : reverting to default key '%s'\n", key.c_str(), INPUT_CONFIG_INI, defaultKey.c_str());
            keyCodeMod.scanCode = _sdlKeys[defaultKey];
            return false;
        }

        keyCodeMod.scanCode = _sdlKeys[key];
        //fprintf(stderr, "Editor::scanCodeFromIniKey() : %s : %s : %s : %s : key=%d : mod=%d\n", sectionString.c_str(), iniKey.c_str(), key.c_str(), mod.c_str(), keyCodeMod.scanCode, keyCodeMod.modifier);
        return true;
    }

    void initialise(void)
    {
        SDL_StartTextInput();

        // Current working directory
        char cwdPath[FILENAME_MAX];
        getcwd(cwdPath, FILENAME_MAX);
        _cwdPath = std::string(cwdPath);
        _filePath = _cwdPath + "/";

        // Keyboard to SDL key mapping
        _sdlKeys["ENTER"]       = SDLK_RETURN;
        _sdlKeys["CR"]          = SDLK_RETURN;
        _sdlKeys["A"]           = SDLK_a;
        _sdlKeys["B"]           = SDLK_b;
        _sdlKeys["C"]           = SDLK_c;
        _sdlKeys["D"]           = SDLK_d;
        _sdlKeys["E"]           = SDLK_e;
        _sdlKeys["F"]           = SDLK_f;
        _sdlKeys["G"]           = SDLK_g;
        _sdlKeys["H"]           = SDLK_h;
        _sdlKeys["I"]           = SDLK_i;
        _sdlKeys["J"]           = SDLK_j;
        _sdlKeys["K"]           = SDLK_k;
        _sdlKeys["L"]           = SDLK_l;
        _sdlKeys["M"]           = SDLK_m;
        _sdlKeys["N"]           = SDLK_n;
        _sdlKeys["O"]           = SDLK_o;
        _sdlKeys["P"]           = SDLK_p;
        _sdlKeys["Q"]           = SDLK_q;
        _sdlKeys["R"]           = SDLK_r;
        _sdlKeys["S"]           = SDLK_s;
        _sdlKeys["T"]           = SDLK_t;
        _sdlKeys["U"]           = SDLK_u;
        _sdlKeys["V"]           = SDLK_v;
        _sdlKeys["W"]           = SDLK_w;
        _sdlKeys["X"]           = SDLK_x;
        _sdlKeys["Y"]           = SDLK_y;
        _sdlKeys["Z"]           = SDLK_z;
        _sdlKeys["1"]           = SDLK_1;
        _sdlKeys["!"]           = SDLK_1;
        _sdlKeys["2"]           = SDLK_2;
        _sdlKeys["@"]           = SDLK_2;
        _sdlKeys["3"]           = SDLK_3;
        _sdlKeys["#"]           = SDLK_3;
        _sdlKeys["4"]           = SDLK_4;
        _sdlKeys["$"]           = SDLK_4;
        _sdlKeys["5"]           = SDLK_5;
        _sdlKeys["%"]           = SDLK_5;
        _sdlKeys["6"]           = SDLK_6;
        _sdlKeys["^"]           = SDLK_6;
        _sdlKeys["7"]           = SDLK_7;
        _sdlKeys["&"]           = SDLK_7;
        _sdlKeys["8"]           = SDLK_8;
        _sdlKeys["*"]           = SDLK_8;
        _sdlKeys["9"]           = SDLK_9;
        _sdlKeys["("]           = SDLK_9;
        _sdlKeys["0"]           = SDLK_0;
        _sdlKeys[")"]           = SDLK_0;
        _sdlKeys["F1"]          = SDLK_F1;
        _sdlKeys["F2"]          = SDLK_F2;
        _sdlKeys["F3"]          = SDLK_F3;
        _sdlKeys["F4"]          = SDLK_F4;
        _sdlKeys["F5"]          = SDLK_F5;
        _sdlKeys["F6"]          = SDLK_F6;
        _sdlKeys["F7"]          = SDLK_F7;
        _sdlKeys["F8"]          = SDLK_F8;
        _sdlKeys["F9"]          = SDLK_F9;
        _sdlKeys["F10"]         = SDLK_F10;
        _sdlKeys["F11"]         = SDLK_F11;
        _sdlKeys["F12"]         = SDLK_F12;
        _sdlKeys["SPACE"]       = SDLK_SPACE;
        _sdlKeys["BACKSPACE"]   = SDLK_BACKSPACE;
        _sdlKeys["TAB"]         = SDLK_TAB;
        _sdlKeys["_"]           = SDLK_MINUS;
        _sdlKeys["-"]           = SDLK_MINUS;
        _sdlKeys["+"]           = SDLK_EQUALS;
        _sdlKeys["="]           = SDLK_EQUALS;
        _sdlKeys["`"]           = SDLK_BACKQUOTE;
        _sdlKeys["~"]           = SDLK_BACKQUOTE;
        _sdlKeys["<"]           = SDLK_COMMA;
        _sdlKeys[","]           = SDLK_COMMA;
        _sdlKeys[">"]           = SDLK_PERIOD;
        _sdlKeys["."]           = SDLK_PERIOD;
        _sdlKeys["["]           = SDLK_LEFTBRACKET;
        _sdlKeys["{"]           = SDLK_LEFTBRACKET;
        _sdlKeys["]"]           = SDLK_RIGHTBRACKET;
        _sdlKeys["}"]           = SDLK_RIGHTBRACKET;
        _sdlKeys[";"]           = SDLK_SEMICOLON;
        _sdlKeys[":"]           = SDLK_SEMICOLON;
        _sdlKeys["'"]           = SDLK_QUOTE;
        _sdlKeys["\""]          = SDLK_QUOTE;
        _sdlKeys["\\"]          = SDLK_BACKSLASH;
        _sdlKeys["|"]           = SDLK_BACKSLASH;
        _sdlKeys["/"]           = SDLK_SLASH;
        _sdlKeys["?"]           = SDLK_SLASH;
        _sdlKeys["LEFT"]        = SDLK_LEFT;
        _sdlKeys["RIGHT"]       = SDLK_RIGHT;
        _sdlKeys["UP"]          = SDLK_UP;
        _sdlKeys["DOWN"]        = SDLK_DOWN;
        _sdlKeys["PAGEUP"]      = SDLK_PAGEUP;
        _sdlKeys["PAGEDOWN"]    = SDLK_PAGEDOWN;
        _sdlKeys["CAPSLOCK"]    = SDLK_CAPSLOCK;
        _sdlKeys["PRINTSCREEN"] = SDLK_PRINTSCREEN;
        _sdlKeys["SCROLLLOCK"]  = SDLK_SCROLLLOCK;
        _sdlKeys["ESC"]         = SDLK_ESCAPE;
        _sdlKeys["PAUSE"]       = SDLK_PAUSE;
        _sdlKeys["INSERT"]      = SDLK_INSERT;
        _sdlKeys["HOME"]        = SDLK_HOME;
        _sdlKeys["DELETE"]      = SDLK_DELETE;
        _sdlKeys["END"]         = SDLK_END;
        _sdlKeys["NUMLOCK"]     = SDLK_NUMLOCKCLEAR;
        _sdlKeys["KP_DIVIDE"]   = SDLK_KP_DIVIDE;
        _sdlKeys["KP_MULTIPLY"] = SDLK_KP_MULTIPLY;
        _sdlKeys["KP_MINUS"]    = SDLK_KP_MINUS; 
        _sdlKeys["KP_PLUS"]     = SDLK_KP_PLUS;
        _sdlKeys["KP_ENTER"]    = SDLK_KP_ENTER;
        _sdlKeys["KP_1"]        = SDLK_KP_1;
        _sdlKeys["KP_2"]        = SDLK_KP_2;
        _sdlKeys["KP_3"]        = SDLK_KP_3;
        _sdlKeys["KP_4"]        = SDLK_KP_4;
        _sdlKeys["KP_5"]        = SDLK_KP_5;
        _sdlKeys["KP_6"]        = SDLK_KP_6;
        _sdlKeys["KP_7"]        = SDLK_KP_7;
        _sdlKeys["KP_8"]        = SDLK_KP_8;
        _sdlKeys["KP_9"]        = SDLK_KP_9;
        _sdlKeys["KP_0"]        = SDLK_KP_0;
        _sdlKeys["KP_PERIOD"]   = SDLK_KP_PERIOD;
        _sdlKeys["LCTRL"]       = SDLK_LCTRL;
        _sdlKeys["LSHIFT"]      = SDLK_LSHIFT;
        _sdlKeys["LALT"]        = SDLK_LALT;
        _sdlKeys["LGUI"]        = SDLK_LGUI;
        _sdlKeys["RCTRL"]       = SDLK_RCTRL;
        _sdlKeys["RSHIFT"]      = SDLK_RSHIFT;
        _sdlKeys["RALT"]        = SDLK_RALT;
        _sdlKeys["RGUI"]        = SDLK_RGUI;

        // Monitor INI key to SDL key mapping
        _inputKeys["Edit"]     = {SDLK_RETURN, KMOD_NONE};
        _inputKeys["RAM_Mode"] = {SDLK_r, KMOD_NONE};
        _inputKeys["Execute"]  = {SDLK_F5, KMOD_LCTRL};
        _inputKeys["Left"]     = {SDLK_LEFT, KMOD_NONE};
        _inputKeys["Right"]    = {SDLK_RIGHT, KMOD_NONE};
        _inputKeys["Up"]       = {SDLK_UP, KMOD_NONE};
        _inputKeys["Down"]     = {SDLK_DOWN, KMOD_NONE};
        _inputKeys["PageUp"]   = {SDLK_PAGEUP, KMOD_NONE};
        _inputKeys["PageDown"] = {SDLK_PAGEDOWN, KMOD_NONE};
        _inputKeys["Hex_Mode"] = {SDLK_F9, KMOD_LCTRL};

        // Browser INI key to SDL key mapping
        _inputKeys["Load"] = {SDLK_l, KMOD_NONE};

        // Emulator INI key to SDL key mapping
        _inputKeys["Help"]         = {SDLK_h, KMOD_LCTRL};
        _inputKeys["Quit"]         = {SDLK_q, KMOD_LCTRL};
        _inputKeys["ROM_Type"]     = {SDLK_r, KMOD_LCTRL};
        _inputKeys["Reset"]        = {SDLK_F1, KMOD_LCTRL};
        _inputKeys["ScanlineMode"] = {SDLK_F3, KMOD_LCTRL};
        _inputKeys["Speed+"]       = {SDLK_EQUALS, KMOD_NONE};
        _inputKeys["Speed-"]       = {SDLK_MINUS, KMOD_NONE};
        _inputKeys["PS2_KB"]       = {SDLK_F10, KMOD_LCTRL};

        // Gigatron INI key to SDL key mapping
        _inputKeys["Giga_Left"]   = {SDLK_a, KMOD_NONE};
        _inputKeys["Giga_Right"]  = {SDLK_d, KMOD_NONE};
        _inputKeys["Giga_Up"]     = {SDLK_w, KMOD_NONE};
        _inputKeys["Giga_Down"]   = {SDLK_s, KMOD_NONE};
        _inputKeys["Giga_Start"]  = {SDLK_SPACE, KMOD_NONE};
        _inputKeys["Giga_Select"] = {SDLK_z, KMOD_NONE};
        _inputKeys["Giga_A"]      = {SDLK_GREATER, KMOD_NONE};
        _inputKeys["Giga_B"]      = {SDLK_SLASH, KMOD_NONE};

        // Hardware INI key to SDL key mapping
        _inputKeys["Giga"]         = {SDLK_F11, KMOD_LCTRL};
        _inputKeys["Giga_PS2"]     = {SDLK_F12, KMOD_LCTRL};
        _inputKeys["Giga_Reset"]   = {SDLK_F1, KMOD_LALT};
        _inputKeys["Giga_Execute"] = {SDLK_F5, KMOD_LALT};

        // Debugger INI key to SDL key mapping
        _inputKeys["Debug"] = {SDLK_F6, KMOD_LCTRL};
        _inputKeys["Step"]  = {SDLK_F7, KMOD_LCTRL};


        // Input configuration
        INIReader iniReader(INPUT_CONFIG_INI);
        _iniReader = iniReader;
        if(_iniReader.ParseError() < 0)
        {
            fprintf(stderr, "Editor::initialise() : couldn't load INI file '%s' : reverting to default keys.\n", INPUT_CONFIG_INI);
            return;
        }

        // Parse input keys INI file
        enum Section {Monitor, Browser, Emulator, Gigatron, Hardware, Debugger};
        std::map<std::string, Section> section;
        section["Monitor"]  = Monitor;
        section["Browser"]  = Browser;
        section["Emulator"] = Emulator;
        section["Gigatron"] = Gigatron;
        section["Hardware"] = Hardware;
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
                    scanCodeFromIniKey(sectionString, "Execute",  "CTRL+F5",  _inputKeys["Execute"]);
                    scanCodeFromIniKey(sectionString, "Left",     "LEFT",     _inputKeys["Left"]);
                    scanCodeFromIniKey(sectionString, "Right",    "RIGHT",    _inputKeys["Right"]);
                    scanCodeFromIniKey(sectionString, "Up",       "UP",       _inputKeys["Up"]);
                    scanCodeFromIniKey(sectionString, "Down",     "DOWN",     _inputKeys["Down"]);
                    scanCodeFromIniKey(sectionString, "PageUp",   "PAGEUP",   _inputKeys["PageUp"]);
                    scanCodeFromIniKey(sectionString, "PageDown", "PAGEDOWN", _inputKeys["PageDown"]);
                    scanCodeFromIniKey(sectionString, "Hex_Mode", "CTRL+F9",  _inputKeys["Hex_Mode"]);
                }
                break;

                case Browser:
                {
                    scanCodeFromIniKey(sectionString, "Load", "L", _inputKeys["Load"]);
                }
                break;

                case Emulator:
                {
                    scanCodeFromIniKey(sectionString, "Help",         "CTRL+H",   _inputKeys["Help"]);
                    scanCodeFromIniKey(sectionString, "Quit",         "CTRL+Q",   _inputKeys["Quit"]);
                    scanCodeFromIniKey(sectionString, "ROM_Type",     "CTRL+R",   _inputKeys["ROM_Type"]);
                    scanCodeFromIniKey(sectionString, "Reset",        "CTRL+F1",  _inputKeys["Reset"]);
                    scanCodeFromIniKey(sectionString, "ScanlineMode", "CTRL+F3",  _inputKeys["ScanlineMode"]);
                    scanCodeFromIniKey(sectionString, "Speed+",       "+",        _inputKeys["Speed+"]);
                    scanCodeFromIniKey(sectionString, "Speed-",       "-",        _inputKeys["Speed-"]);
                    scanCodeFromIniKey(sectionString, "PS2_KB",       "CTRL+F11", _inputKeys["PS2_KB"]);
                }
                break;

                case Gigatron:
                {
                    scanCodeFromIniKey(sectionString, "Giga_Left",   "A",        _inputKeys["Giga_Left"]);
                    scanCodeFromIniKey(sectionString, "Giga_Right",  "D",        _inputKeys["Giga_Right"]);
                    scanCodeFromIniKey(sectionString, "Giga_Up",     "W",        _inputKeys["Giga_Up"]);
                    scanCodeFromIniKey(sectionString, "Giga_Down",   "S",        _inputKeys["Giga_Down"]);
                    scanCodeFromIniKey(sectionString, "Giga_Start",  "SPACE",    _inputKeys["Giga_Start"]);
                    scanCodeFromIniKey(sectionString, "Giga_Select", "Z",        _inputKeys["Giga_Select"]);
                    scanCodeFromIniKey(sectionString, "Giga_A",      ".",        _inputKeys["Giga_A"]);
                    scanCodeFromIniKey(sectionString, "Giga_B",      "/",        _inputKeys["Giga_B"]);
                }
                break;

                case Hardware:
                {
                    scanCodeFromIniKey(sectionString, "Giga",         "CTRL+F10", _inputKeys["Giga"]);
                    scanCodeFromIniKey(sectionString, "Giga_PS2",     "CTRL+F12", _inputKeys["Giga_PS2"]);
                    scanCodeFromIniKey(sectionString, "Giga_Reset",   "ALT+F1",   _inputKeys["Giga_Reset"]);
                    scanCodeFromIniKey(sectionString, "Giga_Execute", "ALT+F5",   _inputKeys["Giga_Execute"]);
                }
                break;

                case Debugger:
                {
                    scanCodeFromIniKey(sectionString, "Debug", "CTRL+F6", _inputKeys["Debug"]);
                    scanCodeFromIniKey(sectionString, "Step",  "CTRL+F7", _inputKeys["Step"]);
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

    void updateEditor(void)
    {
        int range = 0;
        if(_sdlKeyScanCode >= SDLK_0  &&  _sdlKeyScanCode <= SDLK_9) range = 1;
        if(_sdlKeyScanCode >= SDLK_a  &&  _sdlKeyScanCode <= SDLK_f) range = 2;
        if(range == 1  ||  range == 2)
        {
            uint16_t value = 0;    
            switch(range)
            {
                case 1: value = _sdlKeyScanCode - SDLK_0;      break;
                case 2: value = _sdlKeyScanCode - SDLK_a + 10; break;
            }

            // Edit cpu usage addresses
            if(_cursorY == -2  &&  _hexEdit)
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
            else if(_cursorY == -1  &&  _hexEdit)
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
                else if(ent->d_type == DT_REG  &&  (name.find(".gbas") != std::string::npos  ||  name.find(".gtb") != std::string::npos  ||  name.find(".gcl") != std::string::npos  ||
                                                    name.find(".gasm") != std::string::npos  ||  name.find(".vasm") != std::string::npos  ||  name.find(".gt1") != std::string::npos))
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

        // Only reset cursor and file index if file list size has changed
        if(int(_fileEntriesSize != _fileEntries.size()))
        {
            _cursorX = 0;
            _cursorY = 0;
            _fileEntriesIndex = 0;
            _fileEntriesSize = int(_fileEntries.size());
        }
    }

    void changeBrowseDirectory(void)
    {
        std::string entry = *getCurrentFileEntryName();

        if (entry != "..")
            _filePath += entry + "/";
        else
            backOneDirectory();

        browseDirectory();
    }

    // PS2 Keyboard emulation mode
    bool handlePs2KeyDown(void)
    {
        if(_editorMode == PS2KB  ||  _editorMode == GigaPS2)
        {
            switch(_sdlKeyScanCode)
            {
                case SDLK_LEFT:     (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_LEFT,   true) : Cpu::setIN(Cpu::getIN() & ~INPUT_LEFT  ); return true;
                case SDLK_RIGHT:    (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_RIGHT,  true) : Cpu::setIN(Cpu::getIN() & ~INPUT_RIGHT ); return true;
                case SDLK_UP:       (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_UP,     true) : Cpu::setIN(Cpu::getIN() & ~INPUT_UP    ); return true;
                case SDLK_DOWN:     (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_DOWN,   true) : Cpu::setIN(Cpu::getIN() & ~INPUT_DOWN  ); return true;
                case SDLK_PAGEUP:   (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_START,  true) : Cpu::setIN(Cpu::getIN() & ~INPUT_START ); return true;
                case SDLK_PAGEDOWN: (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_SELECT, true) : Cpu::setIN(Cpu::getIN() & ~INPUT_SELECT); return true;
            }

            if((_sdlKeyScanCode >= 0  &&  _sdlKeyScanCode <= 31) ||  _sdlKeyScanCode == 127)
            {
                switch(_sdlKeyScanCode)
                {
                    case SDLK_TAB:       (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_INPUT_A, true) : Cpu::setIN(Cpu::getIN() & ~INPUT_A); return true;
                    case SDLK_ESCAPE:    (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_INPUT_B, true) : Cpu::setIN(Cpu::getIN() & ~INPUT_B); return true;
                    case SDLK_RETURN:    (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_CR,      true) : Cpu::setIN('\n');                    return true;
                    case SDLK_DELETE:    (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(GIGA_PS2_DEL,     true) : Cpu::setIN(127);                     return true;
                }
            }

            if(_sdlKeyScanCode >= 32  &&  _sdlKeyScanCode <= 126)
            {
                _ps2KeyboardDown = true;
                return true;
            }
        }

        return false;
    }

    // Gigatron Keyboard emulation mode
    bool handleGigaKeyDown(void)
    {
        if(_sdlKeyScanCode == _inputKeys["Giga_Left"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Left"].modifier)          {if(_editorMode == Giga) {Loader::sendCommandToGiga('A', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_LEFT);  }
        else if(_sdlKeyScanCode == _inputKeys["Giga_Right"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Right"].modifier)   {if(_editorMode == Giga) {Loader::sendCommandToGiga('D', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_RIGHT); }
        else if(_sdlKeyScanCode == _inputKeys["Giga_Up"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Up"].modifier)         {if(_editorMode == Giga) {Loader::sendCommandToGiga('W', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_UP);    }
        else if(_sdlKeyScanCode == _inputKeys["Giga_Down"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Down"].modifier)     {if(_editorMode == Giga) {Loader::sendCommandToGiga('S', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_DOWN);  }
        else if(_sdlKeyScanCode == _inputKeys["Giga_Start"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Start"].modifier)   {if(_editorMode == Giga) {Loader::sendCommandToGiga('E', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_START); }
        else if(_sdlKeyScanCode == _inputKeys["Giga_Select"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Select"].modifier) {if(_editorMode == Giga) {Loader::sendCommandToGiga('Q', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_SELECT);}
        else if(_sdlKeyScanCode == _inputKeys["Giga_A"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_A"].modifier)           {if(_editorMode == Giga) {Loader::sendCommandToGiga('Z', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_A);     }
        else if(_sdlKeyScanCode == _inputKeys["Giga_B"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_B"].modifier)           {if(_editorMode == Giga) {Loader::sendCommandToGiga('X', true); return true;} Cpu::setIN(Cpu::getIN() & ~INPUT_B);     }

        if(_editorMode == Giga  &&  _sdlKeyScanCode >= 0  &&  _sdlKeyScanCode <= 127) return true;

        return false;
    }

    void handleKeyDown(void)
    {
        if(_sdlKeyScanCode == _inputKeys["Quit"].scanCode  &&  _sdlKeyModifier == _inputKeys["Quit"].modifier)
        {
            SDL_Quit();
            exit(0);
        }

        // ROMS after v1 have their own inbuilt scanline handlers
        else if(_sdlKeyScanCode == _inputKeys["ScanlineMode"].scanCode  &&  _sdlKeyModifier == _inputKeys["ScanlineMode"].modifier)
        {
            // ROMS after v1 have their own inbuilt scanline handlers
            if(Cpu::getRomType() != Cpu::ROMv1) Cpu::setIN(Cpu::getIN() | INPUT_SELECT);
        }

        // Emulator reset
        else if(!_singleStepMode  &&  _sdlKeyScanCode == _inputKeys["Reset"].scanCode  &&  _sdlKeyModifier == _inputKeys["Reset"].modifier) {Cpu::reset(); return;}

        // Hardware reset
        else if(!_singleStepMode  &&  _sdlKeyScanCode == _inputKeys["Giga_Reset"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Reset"].modifier) {Loader::sendCommandToGiga('R', false); return;}

        // PS2 Keyboard emulation mode
        if(handlePs2KeyDown()) return;

        // Gigatron Keyboard emulation mode
        if(handleGigaKeyDown()) return;

        //fprintf(stderr, "Editor::handleKeyDown() : key=%d : mod=%04x\n", _sdlKeyScanCode, _sdlKeyModifier);

        int limitY = (_editorMode != Load) ? HEX_CHARS_Y : std::min(int(_fileEntries.size()), HEX_CHARS_Y);
        if(_sdlKeyScanCode == _inputKeys["Left"].scanCode  &&  _sdlKeyModifier == _inputKeys["Left"].modifier)        {_cursorX = (--_cursorX < 0) ? HEX_CHARS_X-1 : _cursorX;  _memoryDigit = 0; _addressDigit = 0;}
        else if(_sdlKeyScanCode == _inputKeys["Right"].scanCode  &&  _sdlKeyModifier == _inputKeys["Right"].modifier) {_cursorX = (++_cursorX >= HEX_CHARS_X) ? 0  : _cursorX;  _memoryDigit = 0; _addressDigit = 0;}
        else if(_sdlKeyScanCode == _inputKeys["Up"].scanCode  &&  _sdlKeyModifier == _inputKeys["Up"].modifier)       {_cursorY = (--_cursorY < -2) ? limitY-1     : _cursorY;  _memoryDigit = 0; _addressDigit = 0;}
        else if(_sdlKeyScanCode == _inputKeys["Down"].scanCode  &&  _sdlKeyModifier == _inputKeys["Down"].modifier)   {_cursorY = (++_cursorY >= limitY) ? 0       : _cursorY;  _memoryDigit = 0; _addressDigit = 0;}

        else if(_sdlKeyScanCode == _inputKeys["PageUp"].scanCode  &&  _sdlKeyModifier == _inputKeys["PageUp"].modifier)
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

        else if(_sdlKeyScanCode == _inputKeys["PageDown"].scanCode  &&  _sdlKeyModifier == _inputKeys["PageDown"].modifier)
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

        else if(_sdlKeyScanCode == _inputKeys["Speed+"].scanCode  &&  _sdlKeyModifier == _inputKeys["Speed+"].modifier)
        {
            double timingHack = Timing::getTimingHack() - VSYNC_TIMING_60*0.05;
            if(timingHack >= 0.0) Timing::setTimingHack(timingHack);
        }

        else if(_sdlKeyScanCode == _inputKeys["Speed-"].scanCode  &&  _sdlKeyModifier == _inputKeys["Speed-"].modifier)
        {
            double timingHack = Timing::getTimingHack() + VSYNC_TIMING_60*0.05;
            if(timingHack <= VSYNC_TIMING_60) Timing::setTimingHack(timingHack);
        }
    }

    // PS2 Keyboard emulation mode
    bool handlePs2KeyUp(void)
    {
        if(_editorMode == PS2KB)
        {
            switch(_sdlKeyScanCode)
            {
                case SDLK_LEFT:     Cpu::setIN(0xFF); return true;
                case SDLK_RIGHT:    Cpu::setIN(0xFF); return true;
                case SDLK_UP:       Cpu::setIN(0xFF); return true;
                case SDLK_DOWN:     Cpu::setIN(0xFF); return true;
                case SDLK_PAGEUP:   Cpu::setIN(0xFF); return true;
                case SDLK_PAGEDOWN: Cpu::setIN(0xFF); return true;
            }

            if((_sdlKeyScanCode >= 0  &&  _sdlKeyScanCode <= 31) ||  _sdlKeyScanCode == 127)
            {
                switch(_sdlKeyScanCode)
                {
                    case SDLK_TAB:       Cpu::setIN(0xFF); return true;
                    case SDLK_ESCAPE:    Cpu::setIN(0xFF); return true;
                    case SDLK_RETURN:    Cpu::setIN(0xFF); return true;
                    case SDLK_DELETE:    Cpu::setIN(0xFF); return true;
                }
            }

            if(_sdlKeyScanCode >= 32  &&  _sdlKeyScanCode <= 126)
            {
                _ps2KeyboardDown = false;
                Cpu::setIN(0xFF);
                return true;
            }
        }
        else if(_editorMode == GigaPS2)
        {
            if(_sdlKeyScanCode >= 32  &&  _sdlKeyScanCode <= 126) _ps2KeyboardDown = false;
            if(_sdlKeyScanCode >= 0  &&  _sdlKeyScanCode <= 127) return true;
        }

        return false;
    }

    // Gigatron Keyboard emulation mode
    bool handleGigaKeyUp(void)
    {
        if(_editorMode == Giga  &&  _sdlKeyScanCode >= 0  &&  _sdlKeyScanCode <= 127) return true;

        if(_sdlKeyScanCode == _inputKeys["Giga_Left"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Left"].modifier)          Cpu::setIN(Cpu::getIN() | INPUT_LEFT);
        else if(_sdlKeyScanCode == _inputKeys["Giga_Right"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Right"].modifier)   Cpu::setIN(Cpu::getIN() | INPUT_RIGHT);
        else if(_sdlKeyScanCode == _inputKeys["Giga_Up"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Up"].modifier)         Cpu::setIN(Cpu::getIN() | INPUT_UP);
        else if(_sdlKeyScanCode == _inputKeys["Giga_Down"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Down"].modifier)     Cpu::setIN(Cpu::getIN() | INPUT_DOWN);
        else if(_sdlKeyScanCode == _inputKeys["Giga_Start"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Start"].modifier)   Cpu::setIN(Cpu::getIN() | INPUT_START);
        else if(_sdlKeyScanCode == _inputKeys["Giga_Select"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Select"].modifier) Cpu::setIN(Cpu::getIN() | INPUT_SELECT);
        else if(_sdlKeyScanCode == _inputKeys["Giga_A"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_A"].modifier)           Cpu::setIN(Cpu::getIN() | INPUT_A);
        else if(_sdlKeyScanCode == _inputKeys["Giga_B"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_B"].modifier)           Cpu::setIN(Cpu::getIN() | INPUT_B);

        return false;
    }

    void handleKeyUp(void)
    {
        // Toggle help screen
        if(_sdlKeyScanCode == _inputKeys["Help"].scanCode  &&  _sdlKeyModifier == _inputKeys["Help"].modifier)
        {
            static bool helpScreen = false;
            helpScreen = !helpScreen;
            Graphics::setDisplayHelpScreen(helpScreen);
        }

        else if(_sdlKeyScanCode == _inputKeys["ROM_Type"].scanCode  &&  _sdlKeyModifier == _inputKeys["ROM_Type"].modifier)
        {
            Cpu::swapRom();
        }

        // ROMS after v1 have their own inbuilt scanline handlers
        else if(_sdlKeyScanCode == _inputKeys["ScanlineMode"].scanCode  &&  _sdlKeyModifier == _inputKeys["ScanlineMode"].modifier)
        {
            (Cpu::getRomType() != Cpu::ROMv1) ? Cpu::setIN(Cpu::getIN() & ~INPUT_SELECT) : Cpu::swapScanlineMode();
        }

        // PS2 Keyboard emulation mode
        if(handlePs2KeyUp()) return;

        // Gigatron Keyboard emulation mode
        if(handleGigaKeyUp()) return;

        // Browse vCPU directory
        if(_sdlKeyScanCode == _inputKeys["Load"].scanCode  &&  _sdlKeyModifier == _inputKeys["Load"].modifier)
        {
            if(!_singleStepMode)
            {
                _editorMode = _editorMode == Load ? Hex : Load;
                if(_editorMode == Load) browseDirectory();
            }
        }

        // Enter debug mode
        else if(_sdlKeyScanCode == _inputKeys["Debug"].scanCode  &&  _sdlKeyModifier == _inputKeys["Debug"].modifier)
        {
            _prevEditorMode = _editorMode;
            _editorMode = Debug;
            _singleStepMode = true;
            _hexEdit = false;
        }

        // Enter PS2 Keyboard mode
        else if(_sdlKeyScanCode == _inputKeys["PS2_KB"].scanCode  &&  _sdlKeyModifier == _inputKeys["PS2_KB"].modifier)
        {
            if(_editorMode != PS2KB)
            {
                _prevEditorMode = _editorMode;
                _editorMode = PS2KB;
            }
            else
            {
                _editorMode = (_prevEditorMode != PS2KB) ? _prevEditorMode : Hex;
            }
        }

        // Enter Gigatron mode
        else if(_sdlKeyScanCode == _inputKeys["Giga"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga"].modifier)
        {
            if(_editorMode != Giga)
            {
                _prevEditorMode = _editorMode;
                _editorMode = Giga;
            }
            else
            {
                _editorMode = (_prevEditorMode != Giga) ? _prevEditorMode : Hex;
            }
        }

        // Enter Gigatron PS2 Keyboard mode
        else if(_sdlKeyScanCode == _inputKeys["Giga_PS2"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_PS2"].modifier)
        {
            if(_editorMode != GigaPS2)
            {
                _prevEditorMode = _editorMode;
                _editorMode = GigaPS2;
            }
            else
            {
                _editorMode = (_prevEditorMode != GigaPS2) ? _prevEditorMode : Hex;
            }
        }

        // RAM/ROM mode
        else if(_sdlKeyScanCode ==  _inputKeys["RAM_Mode"].scanCode  &&  _sdlKeyModifier == _inputKeys["RAM_Mode"].modifier)
        {
            static int memoryMode = RAM;
            memoryMode = (++memoryMode) % NumMemoryModes;
            _memoryMode = (MemoryMode)memoryMode;
        }

        // Hex mode
        else if(_sdlKeyScanCode ==  _inputKeys["Hex_Mode"].scanCode  &&  _sdlKeyModifier == _inputKeys["Hex_Mode"].modifier)
        {
            _prevEditorMode = _editorMode;
            _editorMode = Hex;
            _hexEdit = false;
        }

        // Toggle hex edit or start an upload
        else if(_sdlKeyScanCode == _inputKeys["Edit"].scanCode  &&  _sdlKeyModifier == _inputKeys["Edit"].modifier)
        {
            if(_editorMode != Load  ||  _cursorY < 0)
            {
                _hexEdit = !_hexEdit;
            }
            else
            {
                FileType fileType = getCurrentFileEntryType();
                switch(fileType)
                {
                    case File: Loader::setUploadTarget(Loader::Emulator); break;
                    case Dir: changeBrowseDirectory(); break;
                }
            }
        }

        // Hardware upload and execute
        else if(_sdlKeyScanCode == _inputKeys["Giga_Execute"].scanCode  &&  _sdlKeyModifier == _inputKeys["Giga_Execute"].modifier)
        {
            Loader::setUploadTarget(Loader::Hardware);
        }

        else if(_sdlKeyScanCode == SDLK_F8)
        {
            //_startMusic = !_startMusic;
        }

        else if(_sdlKeyScanCode == SDLK_F9)
        {
            //Audio::nextScore();
        }

        static EditorMode editorMode = Hex;
        if(_editorMode != editorMode)
        {
            editorMode = _editorMode;

            if(_editorMode == GigaPS2) Loader::sendCommandToGiga(GIGA_PS2_ENABLE,  true);  // enable Arduino emulated PS2 keyboard
            if(_editorMode != GigaPS2) Loader::sendCommandToGiga(GIGA_PS2_DISABLE, true);  // disable Arduino emulated PS2 keyboard
        }

        updateEditor();
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
                _sdlKeyScanCode = event.key.keysym.sym;
                _sdlKeyModifier = event.key.keysym.mod & (KMOD_LCTRL | KMOD_LALT);

                switch(event.type)
                {
                    case SDL_MOUSEWHEEL: handleMouseWheel(event); break;

                    case SDL_KEYUP:
                    {
                        // Leave debug mode
                        if((_sdlKeyScanCode == _inputKeys["Debug"].scanCode  &&  _sdlKeyModifier == _inputKeys["Debug"].modifier)  ||  
                           (_sdlKeyScanCode == _inputKeys["Hex_Mode"].scanCode  &&  _sdlKeyModifier == _inputKeys["Hex_Mode"].modifier))
                        {
                            _singleStep = false;
                            _singleStepMode = false;
                            _editorMode = (_prevEditorMode != Debug) ? _prevEditorMode : Hex;
                            if(_sdlKeyScanCode == _inputKeys["Hex_Mode"].scanCode  &&  _sdlKeyModifier == _inputKeys["Hex_Mode"].modifier) _hexEdit = false;
                        }
                        else
                        {
                            handleKeyUp();
                        }
                    }
                    break;

                    case SDL_KEYDOWN:
                    {
                        // Single step
                        if(_sdlKeyScanCode == _inputKeys["Step"].scanCode  &&  _sdlKeyModifier == _inputKeys["Step"].modifier)
                        {
                            _singleStep = true;
                            _singleStepMode = false;
                            _singleStepTicks = SDL_GetTicks();
                            _singleStepWatch = Cpu::getRAM(_singleStepWatchAddress);
                        }
                        else
                        {
                            handleKeyDown();
                        }
                    }
                    break;
                }
            }
        }

        return _singleStep;
    }

    void handlePS2key(SDL_Event& event)
    {
        if(_ps2KeyboardDown) (_editorMode == GigaPS2) ? Loader::sendCommandToGiga(event.text.text[0], true) : Cpu::setIN(event.text.text[0]);
    }

    void handleInput(void)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            _sdlKeyScanCode = event.key.keysym.sym;
            _sdlKeyModifier = event.key.keysym.mod & (KMOD_LCTRL | KMOD_LALT);

            switch(event.type)
            {
                case SDL_MOUSEWHEEL: handleMouseWheel(event); break;
                case SDL_TEXTINPUT:  handlePS2key(event);     break;
                case SDL_KEYDOWN:    handleKeyDown();         break;
                case SDL_KEYUP:      handleKeyUp();           break;
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
