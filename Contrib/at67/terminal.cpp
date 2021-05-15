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
#include "menu.h"


#define MAX_TERM_COLS     80
#define MAX_TERM_ROWS     47
#define CMD_LINE_ROW      48
#define MAX_HISTORY_CMD   50
#define MAX_COMMAND_CHARS 79


namespace Terminal
{
    const int _screenMaxIndex = (SCREEN_HEIGHT - (FONT_HEIGHT+2)*1)/(FONT_HEIGHT+2);
    const std::string _eraseLine = std::string(MAX_COMMAND_CHARS+1, 32);

    enum MenuItem {MenuCopy=0, MenuAll, MenuCut, MenuDel};

    bool _terminalModeGiga = false;
    bool _waitForPromptGiga = false;

    int _scrollOffset = 0;
    int _scrollDelta = 0;
    int _scrollIndex = 0;
    int _commandHistoryIndex = 0;
    int _commandCharIndex = 0;

    std::string _commandLine;
    std::vector<std::string> _commandLineHistory;

    std::vector<int> _selectedText;
    std::vector<std::string> _terminalText;


    void initialise(void)
    {
        std::vector<std::string> items = {"TERM  ", "Copy  ", "All   ", "Cut   ", "Delete"};
        Menu::createMenu("Terminal", items, 6, 5);
    }

    void sendCommandToGiga(const std::string& cmd)
    {
        if(_waitForPromptGiga) return;

        for(int i=0; i<int(cmd.size()); i++)
        {
            if(!Loader::sendCharGiga(cmd.c_str()[i])) return;
        }

        _waitForPromptGiga = true;
    }

    void scrollToEnd(void)
    {
        _scrollOffset = _scrollIndex;
    }

    void clearCommandLine(void)
    {
        _commandCharIndex = 0;
        _commandLine.clear();
    }

    void clearHistoryCommandLine(void)
    {
        _commandHistoryIndex = 0;
        _commandLineHistory.clear();
    }

    void exitTerminalModeGiga(void)
    {
        clearCommandLine();
        clearHistoryCommandLine();

        if(_terminalModeGiga)
        {
            Loader::sendCharGiga(4);
            _terminalModeGiga = false;
        }
    }

    void switchToTerminal(void)
    {
        Editor::setEditorMode(Editor::Term);
        scrollToEnd();
    }

    void prevCommandLineHistory(void)
    {
        if(_commandLineHistory.size() == 0) return;
        if(--_commandHistoryIndex < 0) _commandHistoryIndex = 0;
        _commandLine = _commandLineHistory[_commandHistoryIndex];
        _commandCharIndex = int(_commandLine.size());
    }

    void nextCommandLineHistory(void)
    {
        if(_commandLineHistory.size() == 0) return;
        if(++_commandHistoryIndex >= int(_commandLineHistory.size())) _commandHistoryIndex = int(_commandLineHistory.size()) - 1;
        _commandLine = _commandLineHistory[_commandHistoryIndex];
        _commandCharIndex = int(_commandLine.size());
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

    void copyCommandLineToHistory(void)
    {
        if(_commandLineHistory.size() < MAX_HISTORY_CMD)
        {
            _commandLineHistory.push_back(_commandLine);
            _commandHistoryIndex = int(_commandLineHistory.size());
        }
        else
        {
            _commandLineHistory.push_back(_commandLine);
            _commandLineHistory.erase(_commandLineHistory.begin());
            _commandHistoryIndex = int(_commandLineHistory.size()) - 1;
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
        for(int i=0; i<int(_selectedText.size()); i++)
        {
            int index = _selectedText[i];
            for(int j=0; j<int(_terminalText[index].size())-1; j++)
            {
                clipboardText[clipboardTextIndex++] = _terminalText[index][j];
            }

            // trailing zero's get replaced with newlines, except for last one
            clipboardText[clipboardTextIndex++] = (i < int(_selectedText.size()) - 1) ? '\n' :  0;
        }

        // Save to system clipboard
        SDL_SetClipboardText(clipboardText);

        delete [] clipboardText;
    }

    void printTerminal(void)
    {
        _scrollIndex = std::max(0, int(_terminalText.size()) - _screenMaxIndex);
        if(_scrollOffset >= _scrollIndex) _scrollOffset = _scrollIndex;
        if(_scrollOffset < 0) _scrollOffset = 0;

        Graphics::clearScreen(0x22222222);
        Graphics::drawText(_eraseLine, 0, _screenMaxIndex*(FONT_HEIGHT+2)+1, 0x55555555, true, MAX_COMMAND_CHARS+1);

        // Terminal text
        for(int i=_scrollOffset; i<int(_terminalText.size()); i++)
        {
            if(i - _scrollOffset >= _screenMaxIndex) break;

            bool invert = false;
            for(int j=0; j<int(_selectedText.size()); j++)
            {
                if(i == _selectedText[j])
                {
                    invert = true;
                    break;
                }
            }
            Graphics::drawText(_terminalText[i], FONT_WIDTH, (i-_scrollOffset)*(FONT_HEIGHT+2), 0xAAAAAAAA, invert, MAX_TERM_COLS);
        }

        // Command line
        std::string commandLine = _commandLine;
        if(_commandCharIndex >= int(commandLine.size()))
        {
            _commandCharIndex = int(commandLine.size());
            commandLine += char(32);
        }

        // Flash cursor
        static uint8_t toggle = 0;
        bool invert = ((toggle++) >>4) & 1;
        Graphics::drawText(commandLine, FONT_WIDTH, _screenMaxIndex*(FONT_HEIGHT+2)+1, 0xFFFFFFFF, invert, 1, _commandCharIndex);
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

                Cpu::shutdown();
                exit(0);
            }

            default: break;
        }
    }

    void handleMouseLeftButtonDown(int mouseX, int mouseY)
    {
        // Normalised mouse position
        float mx = float(mouseX) / float(Graphics::getWidth());
        float my = float(mouseY) / float(Graphics::getHeight());
        mouseX = int(mx * float(SCREEN_WIDTH));
        mouseY = int(my * float(SCREEN_HEIGHT));
        //fprintf(stderr, "%d %d %f %f\n", mouseX, mouseY, mx, my);

        int terminalTextSelected = (mouseY + 2) / (FONT_HEIGHT+2) + _scrollOffset;

        // Save selected text line as long as it doesn't already exist
        bool saveText = true;
        for(int i=0; i<int(_selectedText.size()); i++)
        {
            if(_selectedText[i] == terminalTextSelected)
            {
                saveText = false;
                break;
            }
        }
        if(saveText  &&  terminalTextSelected < int(_terminalText.size())) _selectedText.push_back(terminalTextSelected);

        // Select everything between min and max as long as it doesn't already exist
        int selectedMin = INT_MAX;
        int selectedMax = INT_MIN;
        for(int i=0; i<int(_selectedText.size()); i++)
        {
            if(_selectedText[i] > selectedMax) selectedMax = _selectedText[i];
            if(_selectedText[i] < selectedMin) selectedMin = _selectedText[i];
        }
        for(int i=selectedMin+1; i<selectedMax; i++)
        {
            saveText = true;
            for(int j=0; j<int(_selectedText.size()); j++)
            {
                if(_selectedText[j] == i)
                {
                    saveText = false;
                    break;
                }
            }
            if(saveText  &&  i < int(_terminalText.size())) _selectedText.push_back(i);
        }

        if(mouseY == 0) _scrollOffset--;
    }

    void handleMouseRightButtonDown(int mouseX, int mouseY)
    {
        Menu::captureItem("Terminal", mouseX, mouseY);
        Menu::renderMenu("Terminal");
    }

    void handleMouseButtonDown(const SDL_Event& event, const Editor::MouseState& mouseState)
    {
        UNREFERENCED_PARAM(event);

        if(mouseState._state == SDL_BUTTON_LEFT) _selectedText.clear();

        if(mouseState._state == SDL_BUTTON_X1)
        {
            Menu::captureMenu("Terminal", mouseState._x, mouseState._y);
        }
    }

    void handleMouseButtonUp(const SDL_Event& event, const Editor::MouseState& mouseState)
    {
        UNREFERENCED_PARAM(event);
        UNREFERENCED_PARAM(mouseState);

        int menuItemIndex;
        Menu::getMenuItemIndex("Terminal", menuItemIndex);

        switch(menuItemIndex)
        {
            // Copy selected text
            case MenuCopy:
            {
                if(_terminalText.size()  &&  _selectedText.size())
                {
                    // Sort selected text
                    std::sort(_selectedText.begin(), _selectedText.end());

                    copyTextToClipboard();
                }
            }
            break;

            // Toggle between select all and select none
            case MenuAll:
            {
                if(_selectedText.size() == _terminalText.size())
                {
                    _selectedText.clear();
                }
                else
                {
                    _selectedText.clear();
                    for(int i=0; i<int(_terminalText.size()); i++)
                    {
                        _selectedText.push_back(i);
                    }
                }
            }
            break;

            case MenuCut:
            {
                if(_terminalText.size()  &&  _selectedText.size())
                {
                    // Sort selected text
                    std::sort(_selectedText.begin(), _selectedText.end());

                    copyTextToClipboard();

                    // Delete selected
                    int selectedMin = _selectedText[0];
                    int selectedMax = _selectedText.back();
                    if(int(_terminalText.size()) > selectedMax)
                    {
                        _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                    }
                }

                _selectedText.clear();
            }
            break;

            case MenuDel:
            {
                if(_terminalText.size()  &&  _selectedText.size())
                {
                    // Sort selected text
                    std::sort(_selectedText.begin(), _selectedText.end());

                    // Delete selected
                    int selectedMin = _selectedText[0];
                    int selectedMax = _selectedText.back();
                    if(int(_terminalText.size()) > selectedMax)
                    {
                        _terminalText.erase(_terminalText.begin() + selectedMin, _terminalText.begin() + selectedMax + 1);
                    }
                }

                _selectedText.clear();
            }
            break;

            default: break;
        }
    }

    void handleMouseWheel(const SDL_Event& event)
    {
        if(event.wheel.y > 0) _scrollOffset -= 1;
        if(event.wheel.y < 0) _scrollOffset += 1;
    }

    void handleKey(const SDL_Event& event)
    {
        char keyCode = event.text.text[0];

        if(_terminalModeGiga) Loader::sendCharGiga(keyCode);
        if(keyCode >= 32  &&  keyCode <= 126)
        {
            _commandLine.insert(_commandLine.begin() + _commandCharIndex, char(keyCode));
            _commandCharIndex++;
            if(_commandLine.size() >= MAX_COMMAND_CHARS) copyCommandLineToHistory();
        }
    }

    void handleKeyDown(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Leave terminal mode
        if(keyCode == Editor::getEmulatorScanCode("Terminal")  &&  keyMod == Editor::getEmulatorKeyMod("Terminal"))
        {
            exitTerminalModeGiga();
            Editor::setEditorToPrevMode();
        }        
        // Quit
        else if(keyCode == Editor::getEmulatorScanCode("Quit")  &&  keyMod == Editor::getEmulatorKeyMod("Quit"))
        {
            exitTerminalModeGiga();

            Cpu::shutdown();
            exit(0);
        }
        // Image editor
        else if(keyCode == Editor::getEmulatorScanCode("ImageEditor")  &&  keyMod == Editor::getEmulatorKeyMod("ImageEditor"))
        {
            Editor::setEditorMode(Editor::Image);
        }
        // Audio editor
        else if(keyCode == Editor::getEmulatorScanCode("AudioEditor")  &&  keyMod == Editor::getEmulatorKeyMod("AudioEditor"))
        {
            Editor::setEditorMode(Editor::Audio);
        }

        // No modifier keys
        static bool useTerminalHistory = false;
        if(keyMod == 0x0000)
        {
            // Both modes
            switch(keyCode)
            {
                case SDLK_PAGEUP:   _scrollOffset -= _screenMaxIndex; break;
                case SDLK_PAGEDOWN: _scrollOffset += _screenMaxIndex; break;
                case SDLK_HOME:     _scrollOffset = 0;                break;
                case SDLK_END:      _scrollOffset = _scrollIndex;     break;

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
                        if(!_terminalModeGiga  &&  _commandLine.size() == 1  &&  (_commandLine[0] == 't'  ||  _commandLine[0] == 'T'))
                        {
                            _terminalModeGiga = true;
                            sendCommandToGiga(_commandLine + "\n");
                            clearCommandLine();
                            clearHistoryCommandLine();
                        }
                        else
                        {
                            sendCommandToGiga(_commandLine + "\n");
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
                            for(int i=0; i<int(_commandLine.size()); i++) Loader::sendCharGiga(_commandLine[i]);
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

    void handleKeyUp(SDL_Keycode keyCode, Uint16 keyMod)
    {
        // Help screen
        if(keyCode == Editor::getEmulatorScanCode("Help")  &&  keyMod == Editor::getEmulatorKeyMod("Help"))
        {
            static bool helpScreen = false;
            helpScreen = !helpScreen;
            Graphics::setDisplayHelpScreen(helpScreen);
        }
        // TODO: conflicts with CTRL-D in pluggy terminal mode, Disassembler
        //else if(keyCode == Editor::getEmulatorScanCode("Disassembler")  &&  keyMod == Editor::getEmulatorKeyMod("Disassembler"))
        //{
        //    Editor::setEditorMode(Editor::Dasm);
        //}
        // Browser
        else if(keyCode == Editor::getEmulatorScanCode("Browse")  &&  keyMod == Editor::getEmulatorKeyMod("Browse"))
        {
            Editor::browseDirectory();
            Editor::setEditorMode(Editor::Load);
        }
        // Hex monitor
        else if(keyCode == Editor::getEmulatorScanCode("HexMonitor")  &&  keyMod == Editor::getEmulatorKeyMod("HexMonitor"))
        {
            Editor::setEditorMode(Editor::Hex);
        }
    }

    void handleInput(void)
    {
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
                case SDL_MOUSEBUTTONDOWN: handleMouseButtonDown(event, mouseState); break;
                case SDL_MOUSEBUTTONUP:   handleMouseButtonUp(event, mouseState);   break;
                case SDL_MOUSEWHEEL:      handleMouseWheel(event);                  break;
                case SDL_TEXTINPUT:       handleKey(event);                         break;
                case SDL_KEYDOWN:         handleKeyDown(keyCode, keyMod);           break;
                case SDL_KEYUP:           handleKeyUp(keyCode, keyMod);             break;

                default: break;
            }
        }

        if(_waitForPromptGiga)
        {
            std::string line;
            if(!Loader::readLineGiga(line))
            {
                _waitForPromptGiga = false;
                fprintf(stderr, "Terminal::handleInput() : timed out on COM port '%s'\n", Loader::getConfigComPort().c_str());
            }
            else
            {
                _terminalText.push_back(line);
                _scrollDelta = 1;

                if(line.find("?") != std::string::npos  ||  line.find("Ctrl-D") != std::string::npos)
                {
                    _waitForPromptGiga = false;
                    _scrollDelta = 2;
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
                        _scrollDelta = 1;
                    }
                    line.clear();
                }
            }
        }

        // Scroll text one line at a time until end is reached, (jump to end - delta if scroll index is not at end)
        if(_scrollDelta)
        {
            _scrollOffset = _scrollIndex - _scrollDelta + 2;
            _scrollDelta--;
        }

        printTerminal();

        switch(mouseState._state)
        {
            case SDL_BUTTON_LEFT: handleMouseLeftButtonDown(mouseState._x, mouseState._y);  break;
            case SDL_BUTTON_X1:   handleMouseRightButtonDown(mouseState._x, mouseState._y); break;

            default: break;
        }
    }

    void process(void)
    {
        handleInput();
        Graphics::render(true);
    }
}
