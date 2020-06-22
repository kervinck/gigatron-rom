#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <atomic>

#include "memory.h"
#include "loader.h"
#include "cpu.h"
#include "audio.h"
#include "timing.h"
#include "editor.h"
#include "graphics.h"
#include "menu.h"
#include "dialog.h"
#include "terminal.h"
#include "expression.h"
#include "inih/INIReader.h"

#include <SDL.h>


#define AUDIO_SAMPLES     (SCAN_LINES)
#define AUDIO_BUFFER_SIZE (SCAN_LINES*4)
#define AUDIO_FREQUENCY   (int(SCAN_LINES*59.98))

#define MAX_WAVE_X 64
#define MAX_WAVE_Y 64
#define BORDER_X1  15
#define BORDER_X2  144
#define BORDER_Y1  26
#define BORDER_Y2  91

#define MAX_COMMAND_CHARS 79
#define MAX_TEXT_ROWS ((SCREEN_HEIGHT - (FONT_HEIGHT+2)*1)/(FONT_HEIGHT+2))


namespace Audio
{
    bool _realTimeAudio = true;

    SDL_AudioDeviceID _audioDevice = 1;

    std::atomic<int64_t> _audioInIndex(AUDIO_SAMPLES*2);
    std::atomic<int64_t> _audioOutIndex(0);
    uint16_t _audioSamples[AUDIO_BUFFER_SIZE] = {0};

    uint8_t _waveTables[256];

    INIReader _configIniReader;


    bool getRealTimeAudio(void) {return _realTimeAudio;}

    bool getKeyAsString(const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, std::string& result)
    {
        result = _configIniReader.Get(sectionString, iniKey, defaultKey);
        if(result == defaultKey) return false;
        Expression::strToUpper(result);
        return true;
    }

    void sdl2AudioCallback(void* userData, unsigned char *stream, int length)
    {
        UNREFERENCED_PARAM(userData);

        int16_t *sdl2Stream = (int16_t *)stream;

        for(int i=0; i<length/2; i++)
        {
            if(_audioOutIndex % AUDIO_BUFFER_SIZE  ==  _audioInIndex % AUDIO_BUFFER_SIZE)
            {
                sdl2Stream[i] = _audioSamples[(_audioOutIndex-1) % AUDIO_BUFFER_SIZE];
            }
            else
            {
                sdl2Stream[i] = _audioSamples[_audioOutIndex++ % AUDIO_BUFFER_SIZE];
            }
        }
    }


    void initialise(void)
    {
        // Loader config
        INIReader iniReader(Loader::getExePath() + "/" + AUDIO_CONFIG_INI);
        _configIniReader = iniReader;
        if(_configIniReader.ParseError() == 0)
        {
            // Parse Loader Keys
            enum Section {Audio};
            std::map<std::string, Section> section;
            section["Audio"] = Audio;
            for(auto sectionString : _configIniReader.Sections())
            {
                if(section.find(sectionString) == section.end())
                {
                    fprintf(stderr, "Audio::initialise() : INI file '%s' has bad Sections : reverting to default values.\n", AUDIO_CONFIG_INI);
                    break;
                }

                std::string result;
                switch(section[sectionString])
                {
                    case Audio:
                    {
                        getKeyAsString(sectionString, "RealTimeAudio", "1", result);   
                        _realTimeAudio = strtol(result.c_str(), nullptr, 10);
                    }
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "Audio::initialise() : couldn't find audio configuration INI file '%s' : reverting to default values.\n", AUDIO_CONFIG_INI);
        }

        SDL_AudioSpec audSpec;
        SDL_zero(audSpec);
        audSpec.freq = AUDIO_FREQUENCY;
        audSpec.format = AUDIO_S16;
        audSpec.channels = 1;
        audSpec.callback = sdl2AudioCallback;
        audSpec.samples = AUDIO_SAMPLES;

        if(SDL_OpenAudio(&audSpec, NULL) < 0)
        {
            Cpu::shutdown();
            fprintf(stderr, "Audio::initialise() : failed to initialise SDL audio\n");
            _EXIT_(EXIT_FAILURE);
        }

        initialiseChannels();

        SDL_PauseAudio(0);

        initialiseEditor();
    }

    void saveGigaWaveTables(void)
    {
        for(uint16_t i=0; i<256; i++) _waveTables[i] = Cpu::getRAM(0x0700 + i);
    }

    void restoreGigaWaveTables(void)
    {
        for(uint16_t i=0; i<256; i++) Cpu::setRAM(0x0700 + i, _waveTables[i]);
    }

    void restoreGigaWaveTable(uint16_t waveTable)
    {
        waveTable = waveTable & 3;
        for(uint16_t i=waveTable; i<256; i+=4) Cpu::setRAM(0x0700 + i, _waveTables[i]);
    }

    void initialiseChannels(bool coldBoot)
    {
        // Stop audio
        Cpu::setRAM(GIGA_SOUND_TIMER, 0x00);

        // Enable all 4 audio channels by default
        Cpu::setRAM(CHANNEL_MASK, uint8_t(Cpu::getRomType()) | 0x03);

        // Reset channels
        for(uint16_t i=0; i<GIGA_SOUND_CHANNELS; i++)
        {
            Cpu::setRAM(GIGA_CH0_WAV_A + i*GIGA_CHANNEL_OFFSET, 0x00); // ADD modulation, (volume)
            Cpu::setRAM(GIGA_CH0_WAV_X + i*GIGA_CHANNEL_OFFSET, 0x03); // waveform index and XOR index modulation, (noise)
            Cpu::setRAM(GIGA_CH0_KEY_L + i*GIGA_CHANNEL_OFFSET, 0x00); // low frequency look up from ROM
            Cpu::setRAM(GIGA_CH0_KEY_H + i*GIGA_CHANNEL_OFFSET, 0x00); // high frequency look up from ROM
            Cpu::setRAM(GIGA_CH0_OSC_L + i*GIGA_CHANNEL_OFFSET, 0x00); // low internal oscillator
            Cpu::setRAM(GIGA_CH0_OSC_H + i*GIGA_CHANNEL_OFFSET, 0x00); // high internal oscillator
        }

        // ROM's V1 to V3 do not re-initialise RAM Audio Wave Tables on soft reboot, (we re-initialise them for ALL ROM versions)
        //if(romType > Cpu::ROMERR  &&  romType < Cpu::ROMv4)
        {
            // Save Audio Wave Tables on cold reboot
            if(coldBoot)
            {
                saveGigaWaveTables();
            }
            // Restore Audio Wave Tables on warm reboot
            else
            {
                restoreGigaWaveTables();
            }
        }

#if 0
        // Audio channels are byte interlaced
        for(int i=0; i<64; i++)
        {
            //float phase = powf(float(i) / 64.0f + 1.875f, 2.925f);
            float phase = powf(float(i) / 64.0f + 1.8f, 3.13f);
            //Cpu::setRAM(0x700+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
            Cpu::setRAM(0x701+uint16_t(i*4), uint8_t(sinf(phase)*31.0f + 31.0f));
            //Cpu::setRAM(0x702+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
            //Cpu::setRAM(0x703+i*4, uint8_t(sinf(float(i) / 64.0f * 2.0f * 3.141529f)*31.0f + 31.0f));
        }
#endif
    }

    void fillCallbackBuffer(void)
    {
        _audioSamples[_audioInIndex++ % AUDIO_BUFFER_SIZE] = (Cpu::getXOUT() & 0xf0) <<5;
    }

    void fillBuffer(void)
    {
        _audioSamples[_audioInIndex++] = (Cpu::getXOUT() & 0xf0) <<5;
        if(_audioInIndex == AUDIO_SAMPLES)
        {
            playBuffer();
            _audioInIndex = 0;
        }
    }

    void playBuffer(void)
    {
        SDL_QueueAudio(_audioDevice, &_audioSamples[0], uint32_t(_audioInIndex) <<1);
        _audioInIndex = 0;
    }

    void playSample(void)
    {
        uint16_t sample = (Cpu::getXOUT() & 0xf0) <<5;
        SDL_QueueAudio(_audioDevice, &sample, 2);
    }

    void clearQueue(void)
    {
        SDL_ClearQueuedAudio(_audioDevice);
    }


//**********************************************************************************************************************
//* Editor
//**********************************************************************************************************************
#ifndef STAND_ALONE
    enum MenuItem {MenuLoad=0, MenuSave, MenuErase};
    enum DialogItem {DialogFile=2, DialogNo=5, DialogYes=6};
    enum WaveIndex {WaveUser=0, Wave0, Wave1, Wave2, Wave3};

    const std::vector<std::string> _waveStrs = {"User", "Wav0", "Wav1", "Wav2", "Wav3"};
    const std::vector<std::string> _suffixes = {".gtwav"};
    const std::string _eraseLine = std::string(MAX_COMMAND_CHARS+1, 32);

    bool _firstTimeRender = true;
    bool _refreshScreen = false;

    uint8_t _waveUser[MAX_WAVE_X] = {0};
    uint8_t _waveBuffer[MAX_WAVE_X] = {0};
    int _waveIndex = WaveUser;
    int _commandCharIndex = 0;
    std::string _commandLine;

    Editor::MouseState _mouseState;


    void initialiseEditor(void)
    {
        // Menu
        std::vector<std::string> menuItems = {"AUDIO ", "Load  ", "Save  ", "Erase "};
        Menu::createMenu("Audio", menuItems, 6, 4);

        // Overwrite dialog
        std::vector<Dialog::Item> dialogItemsOw =
        {
            Dialog::Item(false, "Overwrite?"), 
            Dialog::Item(false, ""), 
            Dialog::Item(false, ""), 
            Dialog::Item(false, ""), 
            Dialog::Item(false, ""), 
            Dialog::Item(true, "No", Dialog::Item::LeftX, Dialog::Item::Bd),
            Dialog::Item(true, "Yes", Dialog::Item::RightX, Dialog::Item::CurrentY, Dialog::Item::Bd)
        };
        Dialog::createDialog("Overwrite", dialogItemsOw, 3, 5, Dialog::Dialog::DoubleWidth, 0, 1);

        // Wave
        std::vector<Dialog::Item> dialogItemsLa = {Dialog::Item(false, "User")};
        Dialog::createDialog("Wave", dialogItemsLa, 4, 1, Dialog::Dialog::Regular, 1, 0, 106, 60);
        Dialog::positionDialog("Wave", 298, 86);

        // Prev
        std::vector<Dialog::Item> dialogItemsPr = {Dialog::Item(true, "Prev", Dialog::Item::CenterX, Dialog::Item::NextY, Dialog::Item::Bd)};
        Dialog::createDialog("Prev", dialogItemsPr, 4, 1, Dialog::Dialog::Regular, 1, 0, 106, 60);
        Dialog::positionDialog("Prev", 64, 376);

        // Next
        std::vector<Dialog::Item> dialogItemsNe = {Dialog::Item(true, "Next", Dialog::Item::CenterX, Dialog::Item::NextY, Dialog::Item::Bd)};
        Dialog::createDialog("Next", dialogItemsNe, 4, 1, Dialog::Dialog::Regular, 1, 0, 106, 60);
        Dialog::positionDialog("Next", 528, 376);

        // Play
        std::vector<Dialog::Item> dialogItemsPl = {Dialog::Item(true, "Play", Dialog::Item::CenterX, Dialog::Item::NextY, Dialog::Item::Bd)};
        Dialog::createDialog("Play", dialogItemsPl, 4, 1, Dialog::Dialog::Regular, 1, 0, 106, 60);

        // Reset
        std::vector<Dialog::Item> dialogItemsRe = {Dialog::Item(true, "Reset", Dialog::Item::CenterX, Dialog::Item::NextY, Dialog::Item::Bd)};
        Dialog::createDialog("Reset", dialogItemsRe, 4, 1, Dialog::Dialog::Regular, 1, 0, 106, 60);
        Dialog::positionDialog("Reset", 324, 376);
    }

    void clearCommandLine(void)
    {
        _commandCharIndex = 0;
        _commandLine.clear();
    }

    void prevCommandLineChar(void)
    {
        if(--_commandCharIndex < 0) _commandCharIndex = 0;
    }

    void nextCommandLineChar(void)
    {
        if(++_commandCharIndex > int(_commandLine.size())) _commandCharIndex = int(_commandLine.size());
    }

    void backspaceCommandLineChar(void)
    {
        if(_commandLine.size()  &&  _commandCharIndex > 0  &&  _commandCharIndex <= int(_commandLine.size()))
        {
            _commandLine.erase(--_commandCharIndex, 1);
        }
    }

    void deleteCommandLineChar(void)
    {
        if(_commandLine.size()  &&  _commandCharIndex >= 0  &&  _commandCharIndex < int(_commandLine.size()))
        {
            _commandLine.erase(_commandCharIndex, 1);
            if(_commandCharIndex > int(_commandLine.size())) _commandCharIndex = int(_commandLine.size());
        }
    }

    void copyWaveTable(uint8_t src[], uint8_t dst[])
    {
        for(uint16_t i=0; i<MAX_WAVE_X; i++)
        {
            dst[i] = src[i];
        }
    }

    void loadGigaWaveTable(uint16_t wave)
    {
        wave = wave & 3;
        uint16_t address = 0x0700 + wave;
        for(uint16_t i=0; i<MAX_WAVE_X; i++)
        {
            _waveBuffer[i] = Cpu::getRAM(address + i*4);
        } 
    }

    void uploadGigaWaveTable(uint16_t waveTable, uint8_t waveBuffer[])
    {
        int j = 0;
        waveTable = waveTable & 3;
        for(uint16_t i=waveTable; i<256; i+=4) Cpu::setRAM(0x0700 + i, waveBuffer[j++]);
    }

    void playGigaNote(uint8_t wave, uint16_t note, int duration)
    {
        wave = wave & 3;
        Cpu::setRAM(GIGA_CH0_WAV_X, wave);

        Cpu::setRAM(GIGA_SOUND_TIMER, uint8_t(duration));

        note = (note - 11) * 2;
        uint16_t romNote = Cpu::getROM16(note + 0x0900, 1);
        Cpu::setRAM16(GIGA_CH0_KEY_L, romNote);

        while(Cpu::getRAM(GIGA_SOUND_TIMER))
        {
            Cpu::process(true);
        }
    }

    void refreshScreen(void)
    {
        _firstTimeRender = false;
        _refreshScreen = false;

        Graphics::clearScreen(0x22222222, 0x00000000);
        Graphics::drawLine(BORDER_X1,   BORDER_Y2, BORDER_X2, BORDER_Y2, 0x88888888);
        Graphics::drawLine(BORDER_X1,   BORDER_Y1, BORDER_X1, BORDER_Y2, 0x88888888);
        Graphics::drawLine(BORDER_X1-1, BORDER_Y1, BORDER_X2, BORDER_Y1, 0x88888888);
        Graphics::drawLine(BORDER_X2,   BORDER_Y1, BORDER_X2, BORDER_Y2, 0x88888888);

        Editor::browseDirectory(_suffixes);
    }

    void refreshWave(void)
    {
        for(int i=0; i<MAX_WAVE_X; i++)
        {
            Graphics::drawPixel(uint8_t((BORDER_X1 + 1) + i*2),     uint8_t((BORDER_Y2 - 1) - (_waveBuffer[i] & 63)), 0xFFFFFFFF);
            Graphics::drawPixel(uint8_t((BORDER_X1 + 1) + i*2 + 1), uint8_t((BORDER_Y2 - 1) - (_waveBuffer[i] & 63)), 0xFFFFFFFF);
        }
    }

    void refreshUi(void)
    {
        // Update wave
        Dialog::Item dialogItem;
        Dialog::getDialogItem("Wave", 0, dialogItem);
        dialogItem.setText(_waveStrs[_waveIndex]);
        Dialog::setDialogItem("Wave", 0, dialogItem);
        Dialog::renderDialog("Wave", 0, 0);

        // Interactables
        Dialog::renderDialog("Prev", _mouseState._x, _mouseState._y);
        Dialog::renderDialog("Next", _mouseState._x, _mouseState._y);
        if(!_waveIndex)
        {
            Dialog::positionDialog("Play", 298, 376);
            Dialog::renderDialog("Play", _mouseState._x, _mouseState._y);
        }
        else
        {
            Dialog::positionDialog("Play", 254, 376);
            Dialog::renderDialog("Play", _mouseState._x, _mouseState._y);
            Dialog::renderDialog("Reset", _mouseState._x, _mouseState._y);
        }

        // Command line
        std::string commandLine = _commandLine;
        if(_commandCharIndex >= int(commandLine.size()))
        {
            _commandCharIndex = int(commandLine.size());
            commandLine += char(32);
        }
        else
        {
            commandLine[_commandCharIndex] = char(32);
        }
        Graphics::drawText(_eraseLine, 0, MAX_TEXT_ROWS*(FONT_HEIGHT+2)+1, 0x00000000, true, MAX_COMMAND_CHARS+1);
        Graphics::drawText(commandLine, FONT_WIDTH, MAX_TEXT_ROWS*(FONT_HEIGHT+2)+1, 0xFFFFFFFF, true, 1, _commandCharIndex);
    }

    bool isMouseInWave(int mouseX, int mouseY, int& pixelX, int& pixelY)
    {
        Menu::Menu menu;
        if(Menu::getMenu("Audio", menu))
        {
            if(menu.getIsActive()) return false;
        }

        // Normalised mouse position
        float mx = float(mouseX) / float(Graphics::getWidth()) * 0.66666666f;
        float my = float(mouseY) / float(Graphics::getHeight());
        pixelX = int(mx * float(GIGA_WIDTH));
        pixelY = int(my * float(GIGA_HEIGHT));
        if(pixelX > BORDER_X1/2  &&  pixelX < BORDER_X2/2  &&  pixelY > BORDER_Y1  &&  pixelY < BORDER_Y2)
        {
            return true;
        }

        return false;
    }

    void handleGuiEvents(SDL_Event& event)
    {
        switch(event.type)
        {
            case SDL_WINDOWEVENT:
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        Graphics::setWidthHeight(event.window.data1, event.window.data2);
                    }
                    break;

                    default: break;
                }
            }
            break;

            case SDL_QUIT: 
            {
                Cpu::shutdown();
                exit(0);
            }

            default: break;
        }
    }

    void handleNonModalDialogs(void)
    {
        // Save user wave
        if(_waveIndex == WaveUser)
        {
            copyWaveTable(_waveBuffer, _waveUser);
        }

        if(Dialog::getDialogItemIndex("Prev", _mouseState._x, _mouseState._y) != -1)
        {
            if(--_waveIndex < WaveUser) _waveIndex = Wave3;
            if(_waveIndex) loadGigaWaveTable(uint16_t(_waveIndex - 1));
        }
        else if(Dialog::getDialogItemIndex("Next", _mouseState._x, _mouseState._y) != -1)
        {
            if(++_waveIndex > Wave3) _waveIndex = WaveUser;
            if(_waveIndex) loadGigaWaveTable(uint16_t(_waveIndex - 1));
        }
        else if(Dialog::getDialogItemIndex("Play", _mouseState._x, _mouseState._y) != -1)
        {
            // Play internal wave
            if(_waveIndex)
            {
                playGigaNote(uint8_t(_waveIndex - 1), 60, 30);
            }
            // Upload user wave to internal wave 0 play it, then restore
            else
            {
                uploadGigaWaveTable(0, _waveBuffer);
                playGigaNote(0, 60, 30);
                restoreGigaWaveTable(0);
            }
        }
        else if(Dialog::getDialogItemIndex("Reset", _mouseState._x, _mouseState._y) != -1)
        {
            if(_waveIndex)
            {
                restoreGigaWaveTable(uint16_t(_waveIndex - 1));
                loadGigaWaveTable(uint16_t(_waveIndex - 1));
            }
        }

        // Restore user wave
        if(_waveIndex == WaveUser)
        {
            copyWaveTable(_waveUser, _waveBuffer);
        }
    }

    int handleModalDialog(const std::string& name)
    {
        // Mouse button state
        _mouseState._state = SDL_GetMouseState(&_mouseState._x, &_mouseState._y);

        SDL_Event event;
        int dialogItem = -1;
        while(SDL_PollEvent(&event))
        {
            _mouseState._state = SDL_GetMouseState(&_mouseState._x, &_mouseState._y);

            handleGuiEvents(event);

            switch(event.type)
            {
                case SDL_MOUSEBUTTONUP: dialogItem = Dialog::getDialogItemIndex(name, _mouseState._x, _mouseState._y); break;

                default: break;
            }
        }
        
        return dialogItem;
    }

    bool loadWaveFile(std::string* filenamePtr)
    {
        if(filenamePtr == nullptr  &&  _commandLine.size() == 0) return false;

        // Browser get priority over command line
        std::string filename;
        if(filenamePtr != nullptr)
        {
            filename = *filenamePtr;
            _commandLine = filename;
            _commandCharIndex = (_commandLine.size()) ? int(_commandLine.size()) : 0;
        }
        else
        {
            filename = _commandLine;
        }

        // Read
        std::string filepath = Editor::getBrowserPath() + filename;
        std::ifstream infile(filepath, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Audio::loadWaveFile() : failed to open file '%s' for reading\n", filepath.c_str());
            return false;
        }

        infile.read((char *)&_waveBuffer, MAX_WAVE_X);
        if(infile.bad() || infile.fail())
        {
            fprintf(stderr, "Audio::loadWaveFile() : read error in file '%s'\n", filepath.c_str());
            return false;
        }

        if(_waveIndex) uploadGigaWaveTable(int16_t(_waveIndex - 1), _waveBuffer);

        //fprintf(stderr, "Audio::loadWaveFile() : Loaded wave file '%s'\n", filepath.c_str());
        return true;
    }

    bool saveWaveFile(void)
    {
        if(_commandLine.size() == 0) return false;

        std::string filepath = Editor::getBrowserPath() + _commandLine;

        // Detect overwrite
        std::ifstream infile(filepath, std::ios::binary | std::ios::in);
        if(infile.is_open())
        {
            _refreshScreen = true;
            refreshScreen();
            refreshWave();

            // Update dialog filename item
            Dialog::Item dialogItem;
            Dialog::getDialogItem("Overwrite", DialogFile, dialogItem);
            dialogItem.setText(_commandLine);
            Dialog::setDialogItem("Overwrite", DialogFile, dialogItem);
            Dialog::positionDialog("Overwrite", 250, 200);

            int dialogItemIndex = -1;
            do
            {
                Editor::getMouseState(_mouseState);
                dialogItemIndex = handleModalDialog("Overwrite");
                Dialog::renderDialog("Overwrite", _mouseState._x, _mouseState._y);
                Graphics::render(true);
            }
            while(dialogItemIndex == -1);
            infile.close();

            if(dialogItemIndex == DialogNo) return false;
        }

        // Write
        std::ofstream outfile(filepath, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Audio::saveWaveFile() : failed to open file '%s' for writing\n", filepath.c_str());
            return false;
        }

        outfile.write((char *)&_waveBuffer, MAX_WAVE_X);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Audio::saveWaveFile() : write error in file '%s'\n", filepath.c_str());
            return false;
        }

        return true;
    }

    void handleMenu(void)
    {
        Menu::Menu menu;
        if(!Menu::getMenu("Audio", menu)) return;

        int menuItemIndex;
        Menu::getMenuItemIndex("Audio", menuItemIndex);
        switch(menuItemIndex)
        {
            // Load waveform
            case MenuLoad:
            {
                loadWaveFile(nullptr);
            }
            break;

            // Save waveform
            case MenuSave:
            {
                saveWaveFile();
            }
            break;

            // Erase waveform
            case MenuErase:
            {
                for(int i=0; i<MAX_WAVE_X; i++) _waveBuffer[i] = 0;
            }
            break;

            default: break;
        }
    }

    void handleMouseLeftButtonDown(void)
    {
        int pixelX, pixelY;
        if(isMouseInWave(_mouseState._x, _mouseState._y, pixelX, pixelY))
        {
            _waveBuffer[pixelX - (BORDER_X1/2 + 1)] = uint8_t(64 - (pixelY - BORDER_Y1));
            if(_waveIndex) uploadGigaWaveTable(int16_t(_waveIndex - 1), _waveBuffer);
            _refreshScreen = true;
        }
    }

    void handleMouseRightButtonDown(void)
    {
        int pixelX, pixelY;
        if(isMouseInWave(_mouseState._x, _mouseState._y, pixelX, pixelY))
        {
            _waveBuffer[pixelX - (BORDER_X1/2 + 1)] = 0;
            if(_waveIndex) uploadGigaWaveTable(int16_t(_waveIndex - 1), _waveBuffer);
            _refreshScreen = true;
        }
        else
        {
            Menu::captureItem("Audio", _mouseState._x, _mouseState._y);
            Menu::renderMenu("Audio");
        }
    }

    void handleMouseButtonDown(const SDL_Event& event)
    {
        UNREFERENCED_PARAM(event);

        if(_mouseState._state == SDL_BUTTON_LEFT)
        {
            // Handle browse UI
            if(Editor::getPageUpButton())
            {
                Editor::handleBrowsePageUp(HEX_CHARS_Y);
            }
            else if(Editor::getPageDnButton())
            {
                Editor::handleBrowsePageDown(HEX_CHARS_Y);
            }
            // No loading/browsing if cursor is out of bounds
            else if(Editor::getCursorY() < 0  ||  Editor::getCursorY() >= Editor::getFileEntriesSize())
            {
                return;
            }
            else
            {
                Editor::FileType fileType = Editor::getCurrentFileEntryType();
                switch(fileType)
                {
                    case Editor::File: loadWaveFile(Editor::getCurrentFileEntryName()); break;
                    case Editor::Dir:  Editor::changeBrowseDirectory();                 break;

                    default: break;
                }
            }

            _refreshScreen = true;
        }

        if(_mouseState._state == SDL_BUTTON_X1)
        {
            Menu::captureMenu("Audio", _mouseState._x, _mouseState._y);
        }
    }

    void handleMouseButtonUp(const SDL_Event& event)
    {
        UNREFERENCED_PARAM(event);

        handleMenu();
        handleNonModalDialogs();

        _refreshScreen = true;
    }

    void handleMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0) Editor::handleBrowsePageUp(1);
        if(event.wheel.y < 0) Editor::handleBrowsePageDown(1);

        _refreshScreen = true;
    }

    void handleKey(const SDL_Event& event)
    {
        char keyCode = event.text.text[0];

        if(keyCode >= 32  &&  keyCode <= 126)
        {
            // Accept alpha for first char and alphanumeric or underscore or period for rest
            if((_commandLine.size() == 0  &&  _commandCharIndex == 0  &&  isalpha(keyCode))  ||
               (_commandLine.size() != 0  &&  (isalnum(keyCode)  ||  keyCode == '_'  ||  keyCode == '.')))
            {
                // Max of one period in a filename
                if(keyCode == '.'  &&  std::count(_commandLine.begin(), _commandLine.end(), '.') == 1) return;

                if(_commandLine.size() >= MAX_COMMAND_CHARS-1) return;

                _commandLine.insert(_commandLine.begin() + _commandCharIndex, char(keyCode));
                _commandCharIndex++;
            }
        }
    }

    void handleKeyDown(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Leave audio editor
        if(keyCode == Editor::getEmulatorScanCode("AudioEditor")  &&  keyMod == Editor::getEmulatorKeyMod("AudioEditor"))
        {
            _firstTimeRender = true;
            Editor::setEditorToPrevMode();
        }        
        // Quit
        else if(keyCode == Editor::getEmulatorScanCode("Quit")  &&  keyMod == Editor::getEmulatorKeyMod("Quit"))
        {
            Cpu::shutdown();
            exit(0);
        }
        // Image editor
        else if(keyCode == Editor::getEmulatorScanCode("ImageEditor")  &&  keyMod == Editor::getEmulatorKeyMod("ImageEditor"))
        {
            _firstTimeRender = true;
            Editor::setEditorMode(Editor::Image);
        }
        // Terminal mode
        else if(keyCode == Editor::getEmulatorScanCode("Terminal")  &&  keyMod == Editor::getEmulatorKeyMod("Terminal"))
        {
            _firstTimeRender = true;
            Terminal::switchToTerminal();
        }

        if(keyMod == 0x0000)
        {
            switch(keyCode)
            {
                case SDLK_LEFT:      prevCommandLineChar();                                                        break;
                case SDLK_RIGHT:     nextCommandLineChar();                                                        break;
                case SDLK_DELETE:    deleteCommandLineChar();                                                      break;
                case SDLK_BACKSPACE: backspaceCommandLineChar();                                                   break;
                case SDLK_HOME:      _commandCharIndex = 0;                                                        break;
                case SDLK_END:       _commandCharIndex = (_commandLine.size()) ? int(_commandLine.size()) - 1 : 0; break;

                case '\r':
                case '\n': saveWaveFile(); break;

                default : break;
            }
        }
    }

    void handleKeyUp(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Help screen
        if(keyCode == Editor::getEmulatorScanCode("Help")  &&  keyMod == Editor::getEmulatorKeyMod("Help"))
        {
            static bool helpScreen = false;
            helpScreen = !helpScreen;
            Graphics::setDisplayHelpScreen(helpScreen);
        }
        // Disassembler
        else if(keyCode == Editor::getEmulatorScanCode("Disassembler")  &&  keyMod == Editor::getEmulatorKeyMod("Disassembler"))
        {
            _firstTimeRender = true;
            Editor::setEditorMode(Editor::Dasm);
        }
        // Browser
        else if(keyCode == Editor::getEmulatorScanCode("Browse")  &&  keyMod == Editor::getEmulatorKeyMod("Browse"))
        {
            _firstTimeRender = true;
            Editor::setEditorMode(Editor::Load);
            Editor::browseDirectory();
        }
        // Hex monitor
        else if(keyCode == Editor::getEmulatorScanCode("HexMonitor")  &&  keyMod == Editor::getEmulatorKeyMod("HexMonitor"))
        {
            _firstTimeRender = true;
            Editor::setEditorMode(Editor::Hex);
        }
    }

    void handleInput(void)
    {
        // Mouse button state
        _mouseState._state = SDL_GetMouseState(&_mouseState._x, &_mouseState._y);

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            SDL_Keycode keyCode = event.key.keysym.sym;
            Uint16 keyMod = event.key.keysym.mod & (KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);

            _mouseState._state = SDL_GetMouseState(&_mouseState._x, &_mouseState._y);
            Editor::setMouseState(_mouseState);

            handleGuiEvents(event);

            switch(event.type)
            {
                case SDL_MOUSEBUTTONDOWN: handleMouseButtonDown(event);   break;
                case SDL_MOUSEBUTTONUP:   handleMouseButtonUp(event);     break;
                case SDL_MOUSEWHEEL:      handleMouseWheel(event);        break;
                case SDL_TEXTINPUT:       handleKey(event);               break;
                case SDL_KEYDOWN:         handleKeyDown(keyCode, keyMod); break;
                case SDL_KEYUP:           handleKeyUp(keyCode, keyMod);   break;

                default: break;
            }
        }

        switch(_mouseState._state)
        {
            case SDL_BUTTON_LEFT: handleMouseLeftButtonDown();  break;
            case SDL_BUTTON_X1:   handleMouseRightButtonDown(); break;

            default: break;
        }
    }

    void process(void)
    {
        if(_firstTimeRender  ||  _refreshScreen)
        {
            refreshScreen();
        }

        refreshWave();
        refreshUi();

        handleInput();
        Graphics::render(true);
    }
#endif
}