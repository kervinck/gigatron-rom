#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>


#include <SDL.h>
#include "memory.h"
#include "cpu.h"
#include "audio.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"
#include "image.h"
#include "graphics.h"


#define MAX_TERM_COLS   80
#define MAX_TERM_ROWS   47
#define CMD_LINE_ROW    48
#define MAX_MENU_COLS   6
#define MAX_MENU_ROWS   5
#define NUM_MENU_FIELDS (MAX_MENU_ROWS-1)
//#define ONE_TOUCH_CONTROLS


namespace Terminal
{
    enum MenuField {MenuCopy, MenuAll, MenuCut, MenuDel};

    bool _terminalModeGiga = false;
    int _terminalMenuX;
    int _terminalMenuY;
    int _terminalMenuIdx = -1;
    int _terminalScrollOffset = 0;
    int _terminalScrollDelta = 0;
    int _terminalScrollIndex = 0;
    const int _terminalScreenMaxIndex = (SCREEN_HEIGHT - (FONT_HEIGHT+2)*1)/(FONT_HEIGHT+2);
    std::string _terminalCommandLine;
    std::vector<int> _terminalTextSelected;
    std::vector<std::string> _terminalText;


    void initialise(void)
    {
    }

    bool sendCommandToGiga(std::string& cmd, std::vector<std::string>& text)
    {
        for(int i=0; i<cmd.size(); i++)
        {
            Loader::sendCharGiga(cmd.c_str()[i]);
        }

        return Loader::readUntilPromptGiga(text);
    }

    void scrollToEnd(void)
    {
        _terminalScrollOffset = _terminalScrollIndex;
    }

    void exitTerminalModeGiga(void)
    {
        if(_terminalModeGiga)
        {
            _terminalModeGiga = false;
            Loader::sendCharGiga(4);
            Loader::readLineGiga(_terminalText);
            Loader::readLineGiga(_terminalText);
        }
    }

    void copyTextToClipboard(void)
    {
        int clipboardTextSize = 0;
        for(int i=0; i<_terminalText.size(); i++) clipboardTextSize += int(_terminalText[i].size());
        char* clipboardText = new (std::nothrow) char[clipboardTextSize];

        // Copy text line by line, char by char, replace trailing zero's with newlines
        int clipboardTextIndex = 0;
        for(int i=0; i<_terminalTextSelected.size(); i++)
        {
            int index = _terminalTextSelected[i];
            for(int j=0; j<_terminalText[index].size()-1; j++)
            {
                clipboardText[clipboardTextIndex++] = _terminalText[index][j];
            }

            // trailing zero's get replaced with newlines, except for last one
            (i < _terminalTextSelected.size() - 1) ? clipboardText[clipboardTextIndex++] = '\n' : clipboardText[clipboardTextIndex++] = 0;
        }

        // Save to system clipboard
        SDL_SetClipboardText(clipboardText);

        delete[] clipboardText;
    }

    void printTerminal(void)
    {
        _terminalScrollIndex = std::max(0, int(_terminalText.size()) - _terminalScreenMaxIndex);
        if(_terminalScrollOffset >= _terminalScrollIndex) _terminalScrollOffset = _terminalScrollIndex;
        if(_terminalScrollOffset < 0) _terminalScrollOffset = 0;

        Graphics::clearScreen(0x22222222, 0x00000000);

        // Terminal text
        for(int i=_terminalScrollOffset; i<_terminalText.size(); i++)
        {
            if(i - _terminalScrollOffset >= _terminalScreenMaxIndex) break;

            bool invert = false;
            for(int j=0; j<_terminalTextSelected.size(); j++)
            {
                if(i == _terminalTextSelected[j])
                {
                    invert = true;
                    break;
                }
            }
            Graphics::drawText(_terminalText[i], FONT_WIDTH, (i-_terminalScrollOffset)*(FONT_HEIGHT+2), 0xAAAAAAAA, invert, 80);
        }

        // Command line
        Graphics::drawText(_terminalCommandLine, FONT_WIDTH, _terminalScreenMaxIndex*(FONT_HEIGHT+2)+1, 0xFFFFFFFF, false, 80);
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
                }
            }
            break;

            case SDL_QUIT: 
            {
                exitTerminalModeGiga();
                Loader::closeComPort();

                Cpu::shutdown();
                exit(0);
            }
        }
    }

    void handleTerminalMouseLeftButtonDown(int mouseX, int mouseY)
    {
        // Normalised mouse position
        float mx = float(mouseX) / float(Graphics::getWidth());
        float my = float(mouseY) / float(Graphics::getHeight());
        mouseX = int(mx * float(SCREEN_WIDTH));
        mouseY = int(my * float(SCREEN_HEIGHT));
        //fprintf(stderr, "%d %d %f %f\n", mouseX, mouseY, mx, my);

        int terminalTextSelected = (mouseY + 2) / (FONT_HEIGHT+2) + _terminalScrollOffset;

        // Save selected text line as long as it doesn't already exist
        bool saveText = true;
        for(int i=0; i<_terminalTextSelected.size(); i++)
        {
            if(_terminalTextSelected[i] == terminalTextSelected)
            {
                saveText = false;
                break;
            }
        }
        if(saveText  &&  terminalTextSelected < _terminalText.size()) _terminalTextSelected.push_back(terminalTextSelected);

        // Select everything between min and max as long as it doesn't already exist
        int selectedMin = INT_MAX;
        int selectedMax = INT_MIN;
        for(int i=0; i<_terminalTextSelected.size(); i++)
        {
            if(_terminalTextSelected[i] > selectedMax) selectedMax = _terminalTextSelected[i];
            if(_terminalTextSelected[i] < selectedMin) selectedMin = _terminalTextSelected[i];
        }
        for(int i=selectedMin+1; i<selectedMax; i++)
        {
            bool saveText = true;
            for(int j=0; j<_terminalTextSelected.size(); j++)
            {
                if(_terminalTextSelected[j] == i)
                {
                    saveText = false;
                    break;
                }
            }
            if(saveText  &&  i < _terminalText.size()) _terminalTextSelected.push_back(i);
        }

        if(mouseY == 0) _terminalScrollOffset--;
    }

    void handleTerminalMouseRightButtonDown(int mouseX, int mouseY)
    {
        // Normalised mouse position
        float mx = float(mouseX) / float(Graphics::getWidth());
        float my = float(mouseY) / float(Graphics::getHeight());
        int textX = int(mx * float(SCREEN_WIDTH)) / FONT_WIDTH;
        int textY = int(my * float(SCREEN_HEIGHT)) / (FONT_HEIGHT+2);

        int menuIdx = std::min(std::max(0, (textY - _terminalMenuY)), NUM_MENU_FIELDS) - 1;
        bool onMenuX = ((textX - _terminalMenuX) >= 0)  &&  ((textX - _terminalMenuX) < MAX_MENU_COLS);
        bool onMenuY = ((textY - _terminalMenuY) >= 0)  &&  ((textY - _terminalMenuY) < MAX_MENU_ROWS);
        bool onCopy = (onMenuX && onMenuY && menuIdx==MenuCopy);
        bool onAll  = (onMenuX && onMenuY && menuIdx==MenuAll);
        bool onCut  = (onMenuX && onMenuY && menuIdx==MenuCut);
        bool onDel  = (onMenuX && onMenuY && menuIdx==MenuDel);

        _terminalMenuIdx = (onMenuX  &&  onMenuY  &&  menuIdx >= 0  &&  menuIdx < NUM_MENU_FIELDS) ? menuIdx : -1;

        Graphics::drawMenu("MENU  ", _terminalMenuX * FONT_WIDTH, (_terminalMenuY+0) * (FONT_HEIGHT+2), 0xFFFFFFFF, true,    80, 0x55555555);
        Graphics::drawMenu("Copy  ", _terminalMenuX * FONT_WIDTH, (_terminalMenuY+1) * (FONT_HEIGHT+2), 0x55555555, !onCopy, 80, 0xFFFFFFFF);
        Graphics::drawMenu("All   ", _terminalMenuX * FONT_WIDTH, (_terminalMenuY+2) * (FONT_HEIGHT+2), 0x55555555, !onAll,  80, 0xFFFFFFFF);
        Graphics::drawMenu("Cut   ", _terminalMenuX * FONT_WIDTH, (_terminalMenuY+3) * (FONT_HEIGHT+2), 0x55555555, !onCut,  80, 0xFFFFFFFF);
        Graphics::drawMenu("Delete", _terminalMenuX * FONT_WIDTH, (_terminalMenuY+4) * (FONT_HEIGHT+2), 0x55555555, !onDel,  80, 0xFFFFFFFF);
    }

    void handleTerminalMouseButtonDown(const SDL_Event& event, const Editor::MouseState& mouseState)
    {
        if(mouseState._state == SDL_BUTTON_LEFT) _terminalTextSelected.clear();

        if(mouseState._state == SDL_BUTTON_X1)
        {
            float mx = float(mouseState._x) / float(Graphics::getWidth());
            float my = float(mouseState._y) / float(Graphics::getHeight());
            _terminalMenuX = int(mx * float(SCREEN_WIDTH)) / FONT_WIDTH;
            _terminalMenuY = int(my * float(SCREEN_HEIGHT)) / (FONT_HEIGHT+2);
            if(_terminalMenuX > (MAX_TERM_COLS-MAX_MENU_COLS)) _terminalMenuX = (MAX_TERM_COLS-MAX_MENU_COLS);
            if(_terminalMenuY > (MAX_TERM_ROWS-MAX_MENU_ROWS)) _terminalMenuY = (MAX_TERM_ROWS-MAX_MENU_ROWS);
        }
    }

    void handleTerminalMouseButtonUp(const SDL_Event& event, const Editor::MouseState& mouseState)
    {
        switch(_terminalMenuIdx)
        {
            // Copy selected text
            case MenuCopy:
            {
                if(_terminalText.size()  &&  _terminalTextSelected.size())
                {
                    // Sort selected text
                    std::sort(_terminalTextSelected.begin(), _terminalTextSelected.end());

                    copyTextToClipboard();
                }
            }
            break;

            // Toggle between select all and select none
            case MenuAll:
            {
                if(_terminalTextSelected.size() == _terminalText.size())
                {
                    _terminalTextSelected.clear();
                }
                else
                {
                    _terminalTextSelected.clear();
                    for(int i=0; i<_terminalText.size(); i++)
                    {
                        _terminalTextSelected.push_back(i);
                    }
                }
            }
            break;

            case MenuCut:
            {
                if(_terminalText.size()  &&  _terminalTextSelected.size())
                {
                    // Sort selected text
                    std::sort(_terminalTextSelected.begin(), _terminalTextSelected.end());

                    copyTextToClipboard();

                    // Delete selected
                    int selectedMin = _terminalTextSelected[0];
                    int selectedMax = _terminalTextSelected.back();
                    if(_terminalText.size() > selectedMax)
                    {
                        _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                    }
                }

                _terminalTextSelected.clear();
            }
            break;

            case MenuDel:
            {
                if(_terminalText.size()  &&  _terminalTextSelected.size())
                {
                    // Sort selected text
                    std::sort(_terminalTextSelected.begin(), _terminalTextSelected.end());

                    // Delete selected
                    int selectedMin = _terminalTextSelected[0];
                    int selectedMax = _terminalTextSelected.back();
                    if(_terminalText.size() > selectedMax)
                    {
                        _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                    }
                }

                _terminalTextSelected.clear();
            }
            break;
        }

        _terminalMenuIdx = -1;
    }

    void handleTerminalMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0) _terminalScrollOffset -= 1;
        if(event.wheel.y < 0) _terminalScrollOffset += 1;
    }

    void handleTerminalKey(const SDL_Event& event)
    {
        _terminalScrollDelta = int(_terminalText.size());

        char keyCode = event.text.text[0];
        if(!_terminalModeGiga  &&  (keyCode == 't'  ||  keyCode == 'T'))
        {
            _terminalModeGiga = true;
            Loader::sendCharGiga('t');
            Loader::sendCharGiga('\n');
            Loader::readLineGiga(_terminalText);
            Loader::readLineGiga(_terminalText);
        }
        else if(_terminalModeGiga)
        {
            Loader::sendCharGiga(keyCode);
            if(keyCode >= 32  &&  keyCode <= 126) _terminalCommandLine += std::string(1, char(keyCode));
        }
        else if(keyCode >= 32  &&  keyCode <= 126)
        {
#ifdef ONE_TOUCH_CONTROLS
            if(keyCode != SDLK_w  &&  keyCode != SDLK_a  &&  keyCode != SDLK_s  &&  keyCode != SDLK_d  &&  keyCode != SDLK_z  &&  keyCode != SDLK_x  &&  keyCode != SDLK_q  &&  keyCode != SDLK_e)
#endif
            {
                _terminalCommandLine += std::string(1, char(keyCode));
            }
        }

        if(_terminalCommandLine.size() >= 79) _terminalCommandLine.clear();

        // How far PrintTerminal() needs to scroll
        _terminalScrollDelta = int(_terminalText.size()) - _terminalScrollDelta;
    }

    void handleTerminalKeyDown(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Leave terminal mode
        if(keyCode == Editor::getEmulatorScanCode("Terminal")  &&  keyMod == Editor::getEmulatorKeyMod("Terminal"))
        {
            exitTerminalModeGiga();
            Loader::closeComPort();
            Editor::setEditorToPrevMode();
        }        
        // Quit
        else if(keyCode == Editor::getEmulatorScanCode("Quit")  &&  keyMod == Editor::getEmulatorKeyMod("Quit"))
        {
            exitTerminalModeGiga();
            Loader::closeComPort();

            Cpu::shutdown();
            exit(0);
        }

        _terminalScrollDelta = int(_terminalText.size());

        std::string cmd;
        if(keyMod == 0x0000)
        {
            switch(keyCode)
            {
                case SDLK_PAGEUP:   _terminalScrollOffset -= _terminalScreenMaxIndex; break;
                case SDLK_PAGEDOWN: _terminalScrollOffset += _terminalScreenMaxIndex; break;
                case SDLK_HOME:     _terminalScrollOffset = 0;                        break;
                case SDLK_END:      _terminalScrollOffset = _terminalScrollIndex;     break;
            }

            if(!_terminalModeGiga)
            {
#ifdef ONE_TOUCH_CONTROLS
                switch(keyCode)
                {
#if 0
                    case SDLK_w: sendCommandToGiga(std::string("w\n"), _terminalText); break;
                    case SDLK_a: sendCommandToGiga(std::string("a\n"), _terminalText); break;
                    case SDLK_s: sendCommandToGiga(std::string("s\n"), _terminalText); break;
                    case SDLK_d: sendCommandToGiga(std::string("d\n"), _terminalText); break;
                    case SDLK_z: sendCommandToGiga(std::string("z\n"), _terminalText); break;
                    case SDLK_x: sendCommandToGiga(std::string("x\n"), _terminalText); break;
                    case SDLK_q: sendCommandToGiga(std::string("q\n"), _terminalText); break;
                    case SDLK_e: sendCommandToGiga(std::string("e\n"), _terminalText); break;
#else
                    case SDLK_w: Loader::sendCharGiga('w');Loader::sendCharGiga('\n'); break;
                    case SDLK_a: Loader::sendCharGiga('a');Loader::sendCharGiga('\n'); break;
                    case SDLK_s: Loader::sendCharGiga('s');Loader::sendCharGiga('\n'); break;
                    case SDLK_d: Loader::sendCharGiga('d');Loader::sendCharGiga('\n'); break;
                    case SDLK_z: Loader::sendCharGiga('z');Loader::sendCharGiga('\n'); break;
                    case SDLK_x: Loader::sendCharGiga('x');Loader::sendCharGiga('\n'); break;
                    case SDLK_q: Loader::sendCharGiga('q');Loader::sendCharGiga('\n'); break;
                    case SDLK_e: Loader::sendCharGiga('e');Loader::sendCharGiga('\n'); break;
#endif
                }
#endif

                if(keyCode == SDLK_BACKSPACE  ||  keyCode == SDLK_DELETE)
                {
                    if(_terminalCommandLine.size()) _terminalCommandLine.erase(_terminalCommandLine.end() - 1);
                }
                else if(keyCode == '\r'  ||  keyCode == '\n')
                {
                    sendCommandToGiga(_terminalCommandLine + "\n", _terminalText);
                    _terminalCommandLine.clear();
                }
            }
        }

        if(_terminalModeGiga)
        {
            // No modifier keys
            switch(keyCode)
            {
                case SDLK_UP:     Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('A');                                            break;
                case SDLK_DOWN:   Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('B');                                            break;
                case SDLK_RIGHT:  Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('C');                                            break;
                case SDLK_LEFT:   Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('D');                                            break;
                case SDLK_TAB:    Loader::sendCharGiga(keyCode);                                                                                             break;
                case SDLK_DELETE: Loader::sendCharGiga(keyCode); if(_terminalCommandLine.size()) _terminalCommandLine.erase(_terminalCommandLine.end() - 1); break;
                case '\r':
                case '\n':        Loader::sendCharGiga(keyCode); _terminalCommandLine.clear();                                                               break;
            }

            // CTRL C for BASIC, (shared with copy to clipboard)
            if(keyCode == SDLK_c  &&  keyMod & KMOD_LCTRL)
            {
                Loader::sendCharGiga(3);
            }
            // Break giga terminal mode
            else if(keyCode == SDLK_d  &&  keyMod & KMOD_LCTRL)
            {
                exitTerminalModeGiga();
            }
        }

        // How far PrintTerminal() needs to scroll
        _terminalScrollDelta = int(_terminalText.size()) - _terminalScrollDelta;
    }

    void handleTerminalKeyUp(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Help screen
        if(keyCode == Editor::getEmulatorScanCode("Help")  &&  keyMod == Editor::getEmulatorKeyMod("Help"))
        {
            static bool helpScreen = false;
            helpScreen = !helpScreen;
            Graphics::setDisplayHelpScreen(helpScreen);
        }
#if 0
        // Select all
        else if(keyCode == SDLK_a  &&  keyMod & KMOD_LCTRL)
        {
            _terminalTextSelected.clear();
            for(int i=0; i<_terminalText.size(); i++)
            {
                _terminalTextSelected.push_back(i);
            }
        }
        // Copy selection to system clipboard, (shared with break BASIC program)
        else if(keyCode == SDLK_c  &&  keyMod & KMOD_LCTRL)
        {
            // Copy selected text
            if(_terminalText.size()  &&  _terminalTextSelected.size())
            {
                // Sort selected text
                std::sort(_terminalTextSelected.begin(), _terminalTextSelected.end());

                copyTextToClipboard();
            }
        }
        // Cut selection to system clipboard
        else if(keyCode == SDLK_x  &&  keyMod & KMOD_LCTRL)
        {
            if(_terminalText.size()  &&  _terminalTextSelected.size())
            {
                // Sort selected text
                std::sort(_terminalTextSelected.begin(), _terminalTextSelected.end());

                copyTextToClipboard();

                int selectedMin = _terminalTextSelected[0];
                int selectedMax = _terminalTextSelected.back();
                if(_terminalText.size() > selectedMax)
                {
                    _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                }
            }

            _terminalTextSelected.clear();
        }
#endif
    }

    void handleTerminalInput(void)
    {
        static bool firstTime = true;
        if(firstTime)
        {
            firstTime = false;
            std::string cmd = "V\n";
            sendCommandToGiga(cmd, _terminalText);
        }

        // Mouse button state
        Editor::MouseState mouseState;
        mouseState._state = SDL_GetMouseState(&mouseState._x, &mouseState._y);

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            SDL_Keycode keyCode = event.key.keysym.sym;
            Uint16 keyMod = event.key.keysym.mod & (KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);

            mouseState._state = SDL_GetMouseState(&mouseState._x, &mouseState._y);

            handleGuiEvents(event);

            switch(event.type)
            {
                case SDL_MOUSEBUTTONDOWN: handleTerminalMouseButtonDown(event, mouseState); break;
                case SDL_MOUSEBUTTONUP:   handleTerminalMouseButtonUp(event, mouseState);   break;
                case SDL_MOUSEWHEEL:      handleTerminalMouseWheel(event);                  break;
                case SDL_TEXTINPUT:       handleTerminalKey(event);                         break;
                case SDL_KEYDOWN:         handleTerminalKeyDown(keyCode, keyMod);           break;
                case SDL_KEYUP:           handleTerminalKeyUp(keyCode, keyMod);             break;
            }
        }

        // Scroll text one line at a time until end is reached, (jump to end - delta if scroll index is not at end)
        if(_terminalScrollDelta)
        {
            _terminalScrollOffset = _terminalScrollIndex - _terminalScrollDelta;
            _terminalScrollDelta--;
            _terminalScrollOffset++;
        }

        // Read chars back from Gigatron in terminal mode
        if(_terminalModeGiga)
        {
            char chr = 0;
            static std::string line;
            if(Loader::readCharGiga(&chr))
            {
                if(chr >= 32  &&  chr <= 126) line.push_back(chr);
                if(chr == '\r'  ||  chr == '\n')
                {
                    if(line.size())
                    {
                        _terminalScrollDelta = int(_terminalText.size());
                        _terminalText.push_back(line);
                        _terminalScrollDelta = int(_terminalText.size()) - _terminalScrollDelta;
                    }
                    line.clear();
                }
            }
        }

        printTerminal();

        switch(mouseState._state)
        {
            case SDL_BUTTON_LEFT: handleTerminalMouseLeftButtonDown(mouseState._x, mouseState._y);  break;
            case SDL_BUTTON_X1:   handleTerminalMouseRightButtonDown(mouseState._x, mouseState._y); break;
        }
    }


    void process(void)
    {
        handleTerminalInput();
        Graphics::render(true);
    }
}
