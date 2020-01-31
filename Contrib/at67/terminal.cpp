#include <stdio.h>
#include <climits>
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
#define MAX_HISTORY_CMD 50


namespace Terminal
{
    const int _terminalScreenMaxIndex = (SCREEN_HEIGHT - (FONT_HEIGHT+2)*1)/(FONT_HEIGHT+2);

    enum MenuField {MenuCopy, MenuAll, MenuCut, MenuDel};

    bool _terminalModeGiga = false;
    bool _waitForPromptGiga = false;

    int _terminalMenuX;
    int _terminalMenuY;
    int _terminalMenuIdx = -1;
    int _terminalScrollOffset = 0;
    int _terminalScrollDelta = 0;
    int _terminalScrollIndex = 0;
    int _historyCommandIndex = 0;
    int _terminalCommandCharIndex = 0;

    std::string _terminalCommandLine;
    std::vector<std::string> _historyCommandLine;

    std::vector<int> _terminalTextSelected;
    std::vector<std::string> _terminalText;


    void initialise(void)
    {
    }

    void sendCommandToGiga(const std::string& cmd)
    {
        if(_waitForPromptGiga) return;

        for(int i=0; i<int(cmd.size()); i++)
        {
            Loader::sendCharGiga(cmd.c_str()[i]);
        }

        _waitForPromptGiga = true;
    }

    void scrollToEnd(void)
    {
        _terminalScrollOffset = _terminalScrollIndex;
    }

    void clearCommandLine(void)
    {
        _terminalCommandCharIndex = 0;
        _terminalCommandLine.clear();
    }

    void clearHistoryCommandLine(void)
    {
        _historyCommandIndex = 0;
        _historyCommandLine.clear();
    }

    void exitTerminalModeGiga(void)
    {
        clearCommandLine();
        clearHistoryCommandLine();

        if(_terminalModeGiga)
        {
            Loader::sendCharGiga(4);
            _terminalModeGiga = false;
            _waitForPromptGiga = true;
        }
    }

    void prevCommandLineHistory(void)
    {
        if(_historyCommandLine.size() == 0) return;
        if(--_historyCommandIndex < 0) _historyCommandIndex = 0;
        _terminalCommandLine = _historyCommandLine[_historyCommandIndex];
        _terminalCommandCharIndex = int(_terminalCommandLine.size());
    }

    void nextCommandLineHistory(void)
    {
        if(_historyCommandLine.size() == 0) return;
        if(++_historyCommandIndex >= int(_historyCommandLine.size())) _historyCommandIndex = int(_historyCommandLine.size()) - 1;
        _terminalCommandLine = _historyCommandLine[_historyCommandIndex];
        _terminalCommandCharIndex = int(_terminalCommandLine.size());
    }

    void prevCommandLineChar(void)
    {
        if(--_terminalCommandCharIndex < 0) _terminalCommandCharIndex = 0;
    }

    void nextCommandLineChar(void)
    {
        if(++_terminalCommandCharIndex > int(_terminalCommandLine.size())) _terminalCommandCharIndex = int(_terminalCommandLine.size());
    }

    void backspaceCommandLineChar(void)
    {
        if(_terminalCommandLine.size()  &&  _terminalCommandCharIndex > 0  &&  _terminalCommandCharIndex <= int(_terminalCommandLine.size()))
        {
            _terminalCommandLine.erase(--_terminalCommandCharIndex, 1);
        }
    }

    void deleteCommandLineChar(void)
    {
        if(_terminalCommandLine.size()  &&  _terminalCommandCharIndex >= 0  &&  _terminalCommandCharIndex < int(_terminalCommandLine.size()))
        {
            _terminalCommandLine.erase(_terminalCommandCharIndex, 1);
            if(_terminalCommandCharIndex > int(_terminalCommandLine.size())) _terminalCommandCharIndex = int(_terminalCommandLine.size());
        }
    }

    void copyCommandLineToHistory(void)
    {
        if(_historyCommandLine.size() < MAX_HISTORY_CMD)
        {
            _historyCommandLine.push_back(_terminalCommandLine);
            _historyCommandIndex = int(_historyCommandLine.size());
        }
        else
        {
            _historyCommandLine.push_back(_terminalCommandLine);
            _historyCommandLine.erase(_historyCommandLine.begin());
            _historyCommandIndex = int(_historyCommandLine.size()) - 1;
        }

        clearCommandLine();
    }

    void copyTextToClipboard(void)
    {
        int clipboardTextSize = 0;
        for(int i=0; i<int(_terminalText.size()); i++) clipboardTextSize += int(_terminalText[i].size());
        if(clipboardTextSize == 0) return;
        char* clipboardText = new (std::nothrow) char[clipboardTextSize];
        if(!clipboardText) return;

        // Copy text line by line, char by char, replace trailing zero's with newlines
        int clipboardTextIndex = 0;
        for(int i=0; i<int(_terminalTextSelected.size()); i++)
        {
            int index = _terminalTextSelected[i];
            for(int j=0; j<int(_terminalText[index].size())-1; j++)
            {
                clipboardText[clipboardTextIndex++] = _terminalText[index][j];
            }

            // trailing zero's get replaced with newlines, except for last one
            clipboardText[clipboardTextIndex++] = (i < int(_terminalTextSelected.size()) - 1) ? '\n' :  0;
        }

        // Save to system clipboard
        SDL_SetClipboardText(clipboardText);

        delete [] clipboardText;
    }

    void printTerminal(void)
    {
        _terminalScrollIndex = std::max(0, int(_terminalText.size()) - _terminalScreenMaxIndex);
        if(_terminalScrollOffset >= _terminalScrollIndex) _terminalScrollOffset = _terminalScrollIndex;
        if(_terminalScrollOffset < 0) _terminalScrollOffset = 0;

        Graphics::clearScreen(0x22222222, 0x00000000);

        // Terminal text
        for(int i=_terminalScrollOffset; i<int(_terminalText.size()); i++)
        {
            if(i - _terminalScrollOffset >= _terminalScreenMaxIndex) break;

            bool invert = false;
            for(int j=0; j<int(_terminalTextSelected.size()); j++)
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
        std::string commandLine = _terminalCommandLine;
        if(_terminalCommandCharIndex >= int(commandLine.size()))
        {
            _terminalCommandCharIndex = int(commandLine.size());
            commandLine += char(32);
        }
        else
        {
            commandLine[_terminalCommandCharIndex] = char(32);
        } 
        Graphics::drawText(commandLine, FONT_WIDTH, _terminalScreenMaxIndex*(FONT_HEIGHT+2)+1, 0xFFFFFFFF, true, 1, _terminalCommandCharIndex);
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
                exitTerminalModeGiga();
                Loader::closeComPort();

                Cpu::shutdown();
                exit(0);
            }

            default: break;
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
        for(int i=0; i<int(_terminalTextSelected.size()); i++)
        {
            if(_terminalTextSelected[i] == terminalTextSelected)
            {
                saveText = false;
                break;
            }
        }
        if(saveText  &&  terminalTextSelected < int(_terminalText.size())) _terminalTextSelected.push_back(terminalTextSelected);

        // Select everything between min and max as long as it doesn't already exist
        int selectedMin = INT_MAX;
        int selectedMax = INT_MIN;
        for(int i=0; i<int(_terminalTextSelected.size()); i++)
        {
            if(_terminalTextSelected[i] > selectedMax) selectedMax = _terminalTextSelected[i];
            if(_terminalTextSelected[i] < selectedMin) selectedMin = _terminalTextSelected[i];
        }
        for(int i=selectedMin+1; i<selectedMax; i++)
        {
            saveText = true;
            for(int j=0; j<int(_terminalTextSelected.size()); j++)
            {
                if(_terminalTextSelected[j] == i)
                {
                    saveText = false;
                    break;
                }
            }
            if(saveText  &&  i < int(_terminalText.size())) _terminalTextSelected.push_back(i);
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
        UNREFERENCED_PARAM(event);

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
        UNREFERENCED_PARAM(event);
        UNREFERENCED_PARAM(mouseState);

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
                    for(int i=0; i<int(_terminalText.size()); i++)
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
                    if(int(_terminalText.size()) > selectedMax)
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
                    if(int(_terminalText.size()) > selectedMax)
                    {
                        _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                    }
                }

                _terminalTextSelected.clear();
            }
            break;

            default: break;
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
        char keyCode = event.text.text[0];

        if(_terminalModeGiga) Loader::sendCharGiga(keyCode);
        if(keyCode >= 32  &&  keyCode <= 126)
        {
            _terminalCommandLine.insert(_terminalCommandLine.begin() + _terminalCommandCharIndex, char(keyCode));
            _terminalCommandCharIndex++;
            if(_terminalCommandLine.size() >= 79) copyCommandLineToHistory();
        }
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

        // No modifier keys
        static bool useTerminalHistory = false;
        if(keyMod == 0x0000)
        {
            // Both modes
            switch(keyCode)
            {
                case SDLK_PAGEUP:   _terminalScrollOffset -= _terminalScreenMaxIndex; break;
                case SDLK_PAGEDOWN: _terminalScrollOffset += _terminalScreenMaxIndex; break;
                case SDLK_HOME:     _terminalScrollOffset = 0;                        break;
                case SDLK_END:      _terminalScrollOffset = _terminalScrollIndex;     break;

                default: break;
            }

            // Normal mode
            if(!_terminalModeGiga)
            {
                switch(keyCode)
                {
                    case SDLK_UP:        prevCommandLineHistory();   break;
                    case SDLK_DOWN:      nextCommandLineHistory();   break;
                    case SDLK_LEFT:      prevCommandLineChar();      break;
                    case SDLK_RIGHT:     nextCommandLineChar();      break;
                    case SDLK_DELETE:    deleteCommandLineChar();    break;
                    case SDLK_BACKSPACE: backspaceCommandLineChar(); break;
                    
                    case '\r':
                    case '\n':
                    {
                        if(!_terminalModeGiga  &&  _terminalCommandLine.size() == 1  &&  (_terminalCommandLine[0] == 't'  ||  _terminalCommandLine[0] == 'T'))
                        {
                            _terminalModeGiga = true;
                            sendCommandToGiga(_terminalCommandLine + "\n");
                            clearCommandLine();
                            clearHistoryCommandLine();
                        }
                        else
                        {
                            sendCommandToGiga(_terminalCommandLine + "\n");
                            copyCommandLineToHistory();
                        }
                    }
                    break;

                    default: break;
                }
            }
            // Terminal mode
            else
            {
                switch(keyCode)
                {
                    case SDLK_UP:     Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('A'); break;
                    case SDLK_DOWN:   Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('B'); break;
                    case SDLK_RIGHT:  Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('C'); break;
                    case SDLK_LEFT:   Loader::sendCharGiga(27); Loader::sendCharGiga('['); Loader::sendCharGiga('D'); break;
                    case SDLK_TAB:    Loader::sendCharGiga(char(keyCode));                                            break;
                    case SDLK_DELETE: Loader::sendCharGiga(char(keyCode)); backspaceCommandLineChar();                break;
                
                    case '\r':
                    case '\n':        
                    {
                        if(useTerminalHistory)
                        {
                            useTerminalHistory = false;
                            for(int i=0; i<int(_terminalCommandLine.size()); i++) Loader::sendCharGiga(_terminalCommandLine[i]);
                            Loader::sendCharGiga('\n');
                            clearCommandLine();
                        }
                        else
                        {
                            Loader::sendCharGiga(char(keyCode));
                            copyCommandLineToHistory();
                            clearCommandLine();
                        }
                    }
                    break;

                    default: break;
                }
            }
        }
        // Terminal mode modifier keys
        else if(_terminalModeGiga)
        {
            if(keyMod & KMOD_LCTRL)
            {
                switch(keyCode)
                {
                    case SDLK_UP:   prevCommandLineHistory(); useTerminalHistory = true; break;
                    case SDLK_DOWN: nextCommandLineHistory(); useTerminalHistory = true; break;
                    case SDLK_c:    Loader::sendCharGiga(3);                             break; // CTRL C to break BASIC
                    case SDLK_d:    exitTerminalModeGiga();                              break; // CTRL D to exit terminal mode

                    default: break;
                }
            }
        }
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
    }

    void handleTerminalInput(void)
    {
        static bool firstTime = true;
        if(firstTime)
        {
            firstTime = false;
            std::string cmd = "V\n";
            sendCommandToGiga(cmd);
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

                default: break;
            }
        }

        if(_waitForPromptGiga)
        {
            std::string line;
            if(!Loader::readLineGiga(line))
            {
                _waitForPromptGiga = false;
                fprintf(stderr, "Terminal::handleTerminalInput() : timed out on serial port\n");
            }
            else
            {
                _terminalText.push_back(line);
                _terminalScrollDelta = 1;

                if(line.find("Cmd?") != std::string::npos  ||  line.find("Ctrl-D") != std::string::npos)
                {
                    _waitForPromptGiga = false;
                    _terminalScrollDelta = 2;
                }
            }
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
                        _terminalText.push_back(line);
                        _terminalScrollDelta = 1;
                    }
                    line.clear();
                }
            }
        }

        // Scroll text one line at a time until end is reached, (jump to end - delta if scroll index is not at end)
        if(_terminalScrollDelta)
        {
            _terminalScrollOffset = _terminalScrollIndex - _terminalScrollDelta + 2;
            _terminalScrollDelta--;
        }

        printTerminal();

        switch(mouseState._state)
        {
            case SDL_BUTTON_LEFT: handleTerminalMouseLeftButtonDown(mouseState._x, mouseState._y);  break;
            case SDL_BUTTON_X1:   handleTerminalMouseRightButtonDown(mouseState._x, mouseState._y); break;

            default: break;
        }
    }


    void process(void)
    {
        handleTerminalInput();
        Graphics::render(true);
    }
}
