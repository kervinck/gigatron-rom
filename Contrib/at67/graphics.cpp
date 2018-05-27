#include <fstream>
#include <algorithm>

#include "graphics.h"
#include "timing.h"
#include "editor.h"
#include "loader.h"


namespace Graphics
{
    int _width, _height;

    bool _displayHelpScreen = false;
    uint8_t _displayHelpScreenAlpha = 0;

    uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t _colours[COLOUR_PALETTE];
    uint32_t _hlineTiming[GIGA_HEIGHT];

    SDL_Window* _window = NULL;
    SDL_Renderer* _renderer = NULL;
    SDL_Texture* _screenTexture = NULL;
    SDL_Surface* _screenSurface = NULL;
    SDL_Texture* _helpTexture = NULL;
    SDL_Surface* _helpSurface = NULL;
    SDL_Surface* _fontSurface = NULL;


    uint32_t* getPixels(void) {return _pixels;}
    uint32_t* getColours(void) {return _colours;}

    SDL_Window* getWindow(void) {return _window;}
    SDL_Renderer* getRenderer(void) {return _renderer;}
    SDL_Texture* getScreenTexture(void) {return _screenTexture;}
    SDL_Surface* getScreenSurface(void) {return _screenSurface;}
    SDL_Texture* getHelpTexture(void) {return _helpTexture;}
    SDL_Surface* getHelpSurface(void) {return _helpSurface;}
    SDL_Surface* getFontSurface(void) {return _fontSurface;}

    void setDisplayHelpScreen(bool display)
    {
        _displayHelpScreen = display;
    }

    void createHelpSurface(int width, int height)
    {
        uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        _helpSurface = SDL_CreateRGBSurface(0,width, height, 32, rmask, gmask, bmask, amask);
        if(_helpSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::createHelpSurface() :  failed to create SDL surface\n");
            exit(EXIT_FAILURE);
        }
    }

    void createHelptexture(void)
    {
        std::ifstream infile(INPUT_KEYS_INI);
        if(!infile.is_open())
        {
            fprintf(stderr, "Graphics::createHelptexture() : failed to open '%s' : no help screen will be available\n", INPUT_KEYS_INI);

            // Create an empty help screen texture
            _helpTexture = SDL_CreateTextureFromSurface(_renderer, _helpSurface);
            if(_helpTexture == NULL)
            {
                SDL_Quit();
                fprintf(stderr, "Graphics::createHelpTexture() : failed to create help screen texture\n");
                exit(EXIT_FAILURE);
            }
            return;
        }

        int lines = 1;
        std::string lineToken;
        std::vector<std::string> lineTokens;
        while(!infile.eof())
        {
            std::getline(infile, lineToken);
            lineTokens.push_back(lineToken);
            if(!infile.good()  &&  !infile.eof())
            {
                SDL_Quit();
                fprintf(stderr, "Graphics::createHelptexture() : Bad line : '%s' : in %s : on line %d\n", lineToken.c_str(), INPUT_KEYS_INI, lines);
                exit(EXIT_FAILURE);
            }

            lines++;
        }

        // Read ini file into lines buffer
        int maxLines = SCREEN_HEIGHT / FONT_HEIGHT;
        int numLines = std::min(lines-1, maxLines);
        if(lineTokens.size() < numLines)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::createHelptexture() : Bad number of lineTokens : in %s\n", INPUT_KEYS_INI);
            exit(EXIT_FAILURE);
        }

        // Print text in lines with spaces as padding to the help screen surface
        for(int i=0; i<numLines; i++)
        {
            size_t nonWhiteSpace = lineTokens[i].find_first_not_of("  \n\r\f\t\v");
            if(nonWhiteSpace == std::string::npos) lineTokens[i] = std::string(MAX_CHARS_HELP, ' ');
            if(lineTokens[i].size() < MAX_CHARS_HELP) lineTokens[i] += std::string(MAX_CHARS_HELP - lineTokens[i].size(), ' ');
            drawText(lineTokens[i], (uint32_t*)_helpSurface->pixels, 0, i*FONT_HEIGHT + (maxLines - numLines)/2 * FONT_HEIGHT, 0xFF00FF00, false, 0, true, 0xFFFFFFFF, 0xFF00FFFF);
        }

        // Create help screen texture
        _helpTexture = SDL_CreateTextureFromSurface(_renderer, _helpSurface);
        if(_helpTexture == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::createHelpTexture() :  failed to create SDL texture\n");
            exit(EXIT_FAILURE);
        }

        // Enable blending on help screen texture
        SDL_SetTextureBlendMode(_helpTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(_helpTexture, 0);
    }

    void initialise(void)
    {
        for(int i=0; i<GIGA_HEIGHT; i++) _hlineTiming[i] = 0xFF00FF00;

        for(int i=0; i<COLOUR_PALETTE; i++)
        {
            int r = (i>>0) & 3;
            int g = (i>>2) & 3;
            int b = (i>>4) & 3;

            r = r | (r << 2) | (r << 4) | (r << 6);
            g = g | (g << 2) | (g << 4) | (g << 6);
            b = b | (b << 2) | (b << 4) | (b << 6);

            uint32_t p = 0xFF000000;
            p |= r << 16;
            p |= g << 8;
            p |= b << 0;
            _colours[i] = p;
        }

        //SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        _width = DM.w+1;
        _height = DM.h;
        if(SDL_CreateWindowAndRenderer(_width, _height, 0, &_window, &_renderer) < 0)
        //if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &_window, &_renderer) < 0) //SDL_WINDOW_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL window\n");
            exit(EXIT_FAILURE);
        }

        _screenTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
        if(_screenTexture == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() :  failed to create SDL texture\n");
            exit(EXIT_FAILURE);
        }

        _screenSurface = SDL_GetWindowSurface(_window);
        if(_screenSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() :  failed to create SDL surface\n");
            exit(EXIT_FAILURE);
        }

        // Text
        SDL_Surface* fontSurface = SDL_LoadBMP("EmuFont-96x48.bmp"); 
        if(fontSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL font surface, you're probably missing 'EmuFont-96x48.bmp' in the current directory/path\n");
            exit(EXIT_FAILURE);
        }
        _fontSurface = SDL_ConvertSurfaceFormat(fontSurface, _screenSurface->format->format, NULL);
        SDL_FreeSurface(fontSurface);
        if(_fontSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL converted font surface\n");
            exit(EXIT_FAILURE);
        }

        // Help screen
        createHelpSurface(SCREEN_WIDTH, SCREEN_HEIGHT);
        createHelptexture();
    }

    void resetVTable(void)
    {
        for(int i=0; i<GIGA_HEIGHT; i++)
        {
            Cpu::setRAM(GIGA_VTABLE + i*2, (GIGA_VRAM >>8) + i);
            Cpu::setRAM(GIGA_VTABLE + 1 + i*2, 0x00);
        }
    }

    void refreshTimingPixel(const Cpu::State& S, int vgaX, int pixelY, uint32_t colour, bool debugging)
    {
        _hlineTiming[pixelY % GIGA_HEIGHT] = colour;

        if(debugging) return;

        uint32_t screen = (vgaX % SCREEN_WIDTH)*3 + (pixelY % GIGA_HEIGHT)*4*SCREEN_WIDTH;
        _pixels[screen + 0 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3*SCREEN_WIDTH] = 0x00;
    }

    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY, bool debugging)
    {
        if(debugging) return;

        uint32_t colour = _colours[S._OUT & (COLOUR_PALETTE-1)];
        uint32_t address = (vgaX % SCREEN_WIDTH)*3 + (vgaY % SCREEN_HEIGHT)*SCREEN_WIDTH;
        _pixels[address + 0] = colour;
        _pixels[address + 1] = colour;
        _pixels[address + 2] = colour;
    }

    void refreshScreen(void)
    {
        uint8_t offsetx = 0;

        for(int y=0; y<GIGA_HEIGHT; y++)
        {
            offsetx += Cpu::getRAM(GIGA_VTABLE + 1 + y*2);
    
            for(int x=0; x<=GIGA_WIDTH; x++)
            {
                uint16_t address = (Cpu::getRAM(GIGA_VTABLE + y*2) <<8) + ((offsetx + x) & 0xFF);
                uint32_t colour = (x < GIGA_WIDTH) ? _colours[Cpu::getRAM(address) & (COLOUR_PALETTE-1)] : _hlineTiming[y];
                uint32_t screen = (y*4 % SCREEN_HEIGHT)*SCREEN_WIDTH  +  (x*3 % SCREEN_WIDTH);

                _pixels[screen + 0 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0*SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1*SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2*SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3*SCREEN_WIDTH] = 0x00;
            }
        }
    }

    void drawLeds(void)
    {
        // Update 60 times per second no matter what the FPS is
        if(Timing::getFrameTime()  &&  Timing::getFrameUpdate())
        {
            for(int i=0; i<NUM_LEDS; i++)
            {
                int mask = 1 << (NUM_LEDS-1 - i);
                int state = Cpu::getXOUT() & mask;
                uint32_t colour = state ? 0xFF00FF00 : 0xFF770000;

                int address = int(float(SCREEN_WIDTH) * 0.866f) + i*NUM_LEDS + 3*SCREEN_WIDTH;
                _pixels[address + 0] = colour;
                _pixels[address + 1] = colour;
                _pixels[address + 2] = colour;
                address += SCREEN_WIDTH;
                _pixels[address + 0] = colour;
                _pixels[address + 1] = colour;
                _pixels[address + 2] = colour;
            }
        }
    }

    // Simple text routine, font is a non proportional 6*8 font loaded from a 96*48 BMP file
    bool drawText(const std::string& text, uint32_t* pixels, int x, int y, uint32_t colour, bool invert, int invertSize, bool fullscreen, uint32_t commentColour, uint32_t sectionColour)
    {
        if(!fullscreen)
        {
            x += MENU_START_X;
            y += MENU_START_Y;
        }
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return false;

        uint32_t* fontPixels = (uint32_t*)_fontSurface->pixels;
        for(int i=0; i<text.size(); i++)
        {
            if(sectionColour)
            {
                static bool useSectionColour = false;
                if(x == 0) useSectionColour = false;
                if(text.c_str()[i] == '[') useSectionColour = true;
                if(text.c_str()[i] == ']') useSectionColour = false;
                if(useSectionColour) colour = sectionColour;
            }
            if(commentColour)
            {
                static bool useCommentColour = false;
                if(x == 0) useCommentColour = false;
                if(text.c_str()[i] == ';') useCommentColour = true;
                if(useCommentColour) colour = commentColour;
            }

            uint8_t chr = text.c_str()[i] - 32;
            uint8_t row = chr % CHARS_PER_ROW;
            uint8_t col = chr / CHARS_PER_ROW;

            int srcx = row*FONT_WIDTH, srcy = col*FONT_HEIGHT;
            if(srcx+FONT_WIDTH-1>=FONT_BMP_WIDTH || srcy+FONT_HEIGHT-1>=FONT_BMP_HEIGHT) return false;

            int dstx = x + i*FONT_WIDTH, dsty = y;
            if(dstx+FONT_WIDTH-1>=SCREEN_WIDTH-FONT_WIDTH || dsty+FONT_HEIGHT-1>=SCREEN_HEIGHT) return false;

            for(int j=0; j<FONT_WIDTH; j++)
            {
                for(int k=0; k<FONT_HEIGHT; k++)
                {
                    int fontAddress = (srcx + j)  +  (srcy + k)*FONT_BMP_WIDTH;
                    int pixelAddress = (dstx + j)  +  (dsty + k)*SCREEN_WIDTH;
                    if((invert  &&  i<invertSize) ? !fontPixels[fontAddress] : fontPixels[fontAddress])
                    {
                        pixels[pixelAddress] = 0xFF000000 | colour;
                    }
                    else
                    {
                        pixels[pixelAddress] = 0xFF000000;
                    }
                }
            }
        }

        return true;
    }

    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour)
    {
        x += MENU_START_X;
        y += MENU_START_Y;
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return;

        uint32_t pixelAddress = x + digit*FONT_WIDTH + y*SCREEN_WIDTH;

        pixelAddress += (FONT_HEIGHT-1)*SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;

        //pixelAddress += (FONT_HEIGHT-4)*SCREEN_WIDTH;
        //for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        //pixelAddress += SCREEN_WIDTH;
        //for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        //pixelAddress += SCREEN_WIDTH;
        //for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        //pixelAddress += SCREEN_WIDTH;
        //for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;

        //for(int i=0; i<FONT_WIDTH-1; i++) _pixels[pixelAddress+i] = colour;
        //pixelAddress += (FONT_HEIGHT-1)*SCREEN_WIDTH;
        //for(int i=0; i<FONT_WIDTH-1; i++) _pixels[pixelAddress+i] = colour;
        //for(int i=0; i<FONT_HEIGHT; i++) _pixels[pixelAddress-i*SCREEN_WIDTH] = colour;
        //pixelAddress += FONT_WIDTH-1;
        //for(int i=0; i<FONT_HEIGHT; i++) _pixels[pixelAddress-i*SCREEN_WIDTH] = colour;
    }

    void renderText(void)
    {
        // Update 60 times per second no matter what the FPS is
        if(Timing::getFrameTime()  &&  Timing::getFrameUpdate())
        {
            char str[32];
            sprintf(str, "%3.2f    ", 1.0f / Timing::getFrameTime());

            drawText(std::string("LEDS:"), _pixels, 0, 0, 0xFFFFFFFF, false, 0);
            drawText(std::string("FPS:"), _pixels, 0, FONT_CELL_Y, 0xFFFFFFFF, false, 0);
            drawText(std::string(str), _pixels, 62, FONT_CELL_Y, 0xFFFFFF00, false, 0);
            drawText(std::string(VERSION_STR), _pixels, 46, 472, 0xFFFFFF00, false, 0);
            sprintf(str, "XOUT: %02X  IN: %02X", Cpu::getXOUT(), Cpu::getIN());
            drawText(std::string(str), _pixels, 0, FONT_CELL_Y*2, 0xFFFFFFFF, false, 0);
            drawText("Mode:       ", _pixels, 0, 472 - FONT_CELL_Y, 0xFFFFFFFF, false, 0);
            sprintf(str, "Hex  ");
            if(Editor::getHexEdit()) sprintf(str, "Edit ");
            else if(Editor::getEditorMode() == Editor::Load) sprintf(str, "Load ");
            else if(Editor::getEditorMode() == Editor::Debug) sprintf(str, "Debug");
            drawText(std::string(str), _pixels, 68, 472 - FONT_CELL_Y, 0xFF00FF00, false, 0);
        }
    }

    void renderTextWindow(void)
    {
        // Update 60 times per second no matter what the FPS is
        if(Timing::getFrameTime()  &&  Timing::getFrameUpdate())
        {
            char str[32] = "";

            // File load
            if(Editor::getEditorMode() == Editor::Load)
            {
                // File list
                drawText("Load:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0);
                uint16_t loadAddress = Editor::getLoadBaseAddress();
                uint16_t varsAddress = Editor::getVarsBaseAddress();
                bool onCursor0 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 0;
                bool onCursor1 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 1;
                sprintf(str, "%04X", loadAddress);
                drawText(std::string(str), _pixels, 36, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor0) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor0, 4);
                sprintf(str, "%04X", varsAddress);
                drawText(std::string(str), _pixels, 108, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor1) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor1, 4);
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    drawText("                       ", _pixels, 8, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, false, 0);
                }
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    int index = Editor::getFileEntriesIndex() + i;
                    if(index >= int(Editor::getFileEntriesSize())) break;
                    drawText(*Editor::getFileEntryName(index), _pixels, 8, FONT_CELL_Y*4 + i*FONT_CELL_Y, (Editor::getFileEntryType(index) == Editor::Dir) ? 0xFFA0A0A0 : 0xFFFFFFFF, i == Editor::getCursorY(), 18);
                }

                // 8 * 2 hex display of vCPU program variables
                for(int j=0; j<2; j++)
                {
                    for(int i=0; i<HEX_CHARS_X; i++)
                    {
                        sprintf(str, "%02X ", Cpu::getRAM(varsAddress++));
                        drawText(std::string(str), _pixels, 8 + i*HEX_CHAR_WIDE, int(FONT_CELL_Y*4.25) + FONT_CELL_Y*HEX_CHARS_Y + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFF00FFFF, false, 0);
                    }
                }

                // Edit digit select
                if(Editor::getHexEdit())
                {
                    // Draw address digit selection box
                    if(onCursor0) drawDigitBox(Editor::getAddressDigit(), 36, FONT_CELL_Y*3, 0xFFFF00FF);
                    if(onCursor1) drawDigitBox(Editor::getAddressDigit(), 108, FONT_CELL_Y*3, 0xFFFF00FF);
                }
            }
            // Hex monitor
            else
            {
                switch(Editor::getMemoryMode())
                {
                    case Editor::RAM:  drawText("RAM:        Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                    case Editor::ROM0: drawText("ROM0:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                    case Editor::ROM1: drawText("ROM1:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                }

                // 8 * 32 hex display of memory
                uint16_t hexddress = Editor::getHexBaseAddress();
                uint16_t varsAddress = Editor::getVarsBaseAddress();
                bool onCursor = Editor::getCursorY() == -1;
                bool onCursor0 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 0;
                bool onCursor1 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 1;
                sprintf(str, "%04X", hexddress);
                drawText(std::string(str), _pixels, 36, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor0) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor0, 4);
                sprintf(str, "%04X", varsAddress);
                drawText(std::string(str), _pixels, 108, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor1) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor1, 4);
                for(int j=0; j<HEX_CHARS_Y; j++)
                {
                    for(int i=0; i<HEX_CHARS_X; i++)
                    {
                        uint8_t value = 0;
                        switch(Editor::getMemoryMode())
                        {
                            case Editor::RAM:  value = Cpu::getRAM(hexddress++);    break;
                            case Editor::ROM0: value = Cpu::getROM(hexddress++, 0); break;
                            case Editor::ROM1: value = Cpu::getROM(hexddress++, 1); break;
                        }
                        sprintf(str, "%02X ", value);
                        bool onCursor = (i == Editor::getCursorX()  &&  j == Editor::getCursorY());
                        drawText(std::string(str), _pixels, 8 + i*HEX_CHAR_WIDE, FONT_CELL_Y*4 + j*(FONT_HEIGHT+FONT_GAP_Y), (Editor::getHexEdit() && Editor::getMemoryMode() == Editor::RAM && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 2);
                    }
                }

#if 0
                static uint16_t address = 0x08C0;
                for(int j=0; j<32; j++)
                {
                    for(int i=0; i<32; i++)
                    {
                        uint8_t value = Cpu::getRAM(address + j*256 + i);

                        sprintf(str, "%02X ", value);
                        bool onCursor = (i == Editor::getCursorX()  &&  j == Editor::getCursorY());
                        drawText(std::string(str), _pixels, 0 + i*15, FONT_CELL_Y*4 + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFFFFFFFF, false, 0);
                    }
                }
                //address ^= 0x0060;
#endif

                // 8 * 2 hex display of vCPU program variables
                for(int j=0; j<2; j++)
                {
                    for(int i=0; i<HEX_CHARS_X; i++)
                    {
                        sprintf(str, "%02X ", Cpu::getRAM(varsAddress++));
                        drawText(std::string(str), _pixels, 8 + i*HEX_CHAR_WIDE, int(FONT_CELL_Y*4.25) + FONT_CELL_Y*HEX_CHARS_Y + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFF00FFFF, false, 0);
                    }
                }

                // Edit digit select
                if(Editor::getHexEdit())
                {
                    // Draw address digit selection box
                    if(onCursor0) drawDigitBox(Editor::getAddressDigit(), 36, FONT_CELL_Y*3, 0xFFFF00FF);
                    if(onCursor1) drawDigitBox(Editor::getAddressDigit(), 108, FONT_CELL_Y*3, 0xFFFF00FF);

                    // Draw memory digit selection box                
                    if(Editor::getCursorY() >= 0  &&  Editor::getMemoryMode() == Editor::RAM) drawDigitBox(Editor::getMemoryDigit(), 8 + Editor::getCursorX()*HEX_CHAR_WIDE, FONT_CELL_Y*4 + Editor::getCursorY()*FONT_CELL_Y, 0xFFFF00FF);
                }
            }
        }
    }

    void renderHelpScreen(void)
    {
        // Only display help screen if it is enabled or has alpha > 0
        if(_displayHelpScreen  ||  _displayHelpScreenAlpha)
        {
            SDL_SetTextureAlphaMod(_helpTexture, _displayHelpScreenAlpha);
            SDL_RenderCopy(_renderer, _helpTexture, NULL, NULL);
            // Fade help screen in
            if(_displayHelpScreen  &&  _displayHelpScreenAlpha < 220)
            {
                _displayHelpScreenAlpha += 10;
                if(_displayHelpScreenAlpha > 220) _displayHelpScreenAlpha = 220;
            }
            // Fade help screen out
            if(!_displayHelpScreen  &&  _displayHelpScreenAlpha > 0)
            {
                _displayHelpScreenAlpha -= 10;
                if(_displayHelpScreenAlpha < 0) _displayHelpScreenAlpha = 0;
            }
        }
    }

    void render(bool synchronise)
    {
        drawLeds();
        renderText();
        renderTextWindow();

        SDL_UpdateTexture(_screenTexture, NULL, _pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(_renderer, _screenTexture, NULL, NULL);
        renderHelpScreen();
        SDL_RenderPresent(_renderer);
        if(synchronise) Timing::synchronise();
    }


    void drawPixel(uint8_t x, uint8_t y, uint32_t colour)
    {
        x = x % GIGA_WIDTH;
        y = y % GIGA_HEIGHT;
        uint16_t address = GIGA_VRAM + x + (y <<8);
        uint32_t screen = x*3 + y*4*SCREEN_WIDTH;

        _pixels[screen + 0 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3*SCREEN_WIDTH] = 0x00;
    }

    void drawLine(int x, int y, int x2, int y2, uint32_t colour)
    {
   	    bool yLonger = false;
	    int shortLen = y2 - y;
	    int longLen = x2 - x;
	
        if(abs(shortLen) > abs(longLen))
        {
		    uint8_t swap = shortLen;
		    shortLen = longLen;
		    longLen = swap;				
		    yLonger = true;
	    }

	    int decInc;
	    if(longLen ==0 ) decInc=0;
	    else decInc = (shortLen << 8) / longLen;

	    if(yLonger)
        {
		    if(longLen > 0)
            {
			    longLen += y;
			    for(int j=0x80+(x<<8); y<=longLen; ++y)
                {
				    drawPixel(uint8_t(j>>8), uint8_t(y), colour);	
				    j+=decInc;
			    }
			    return;
		    }
		    longLen += y;
		    for(int j=0x80+(x<<8); y>=longLen; --y)
            {
			    drawPixel(uint8_t(j>>8), uint8_t(y), colour);	
			    j-=decInc;
		    }
		    return;	
	    }

	    if(longLen>0)
        {
		    longLen += x;
		    for(int j=0x80+(y<<8); x<=longLen; ++x)
            {
			    drawPixel(uint8_t(x), uint8_t(j>>8), colour);
			    j+=decInc;
		    }
		    return;
	    }
	    longLen += x;
	    for(int j=0x80+(y<<8); x>=longLen;--x)
        {
		    drawPixel(uint8_t(x), uint8_t(j>>8), colour);
		    j-=decInc;
	    }
    }

    uint8_t getPixelGiga(uint8_t x, uint8_t y)
    {
        x = x % GIGA_WIDTH;
        y = y % GIGA_HEIGHT;
        uint16_t address = GIGA_VRAM + x + (y <<8);
        return Cpu::getRAM(address);
    }

    void drawPixelGiga(uint8_t x, uint8_t y, uint8_t colour)
    {
        x = x % GIGA_WIDTH;
        y = y % GIGA_HEIGHT;
        uint16_t address = GIGA_VRAM + x + (y <<8);
        Cpu::setRAM(address, colour);
    }

    void drawLineGiga(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour)
    {
        x1 = y1 = 0;
        x2 = y2 = 5;

        uint16_t w = x2 - x1;
        uint16_t h = y2 - y1;
        uint16_t dx1 = 1, dy1 = 1, dx2 = 0, dy2 = 0;

        dx1 = dx2 = (w > 0x7FFF) ? 0xFFFF : 1;
        dy1 = (h > 0x7FFF) ? 0xFFFF : dy1 = 1;

        uint16_t absw = (w > 0x7FFF) ? 0 - w : w;
        uint16_t absh = (h > 0x7FFF) ? 0 - h : h;

        uint16_t longest = absw;
        uint16_t shortest = absh;

        if(longest < shortest) 
        {
            longest = absh;
            shortest = absw; 
            if(h > 0x7FFF) dy2 = 0xFFFF; 
            else if (h > 0) dy2 = 1;
            dx2 = 0;            
        }

        uint16_t numerator = longest;// >> 1;
        for(uint16_t i=0; i<=longest; i++) 
        {
            drawPixelGiga(uint8_t(x1), uint8_t(y1), colour);
            numerator += shortest;
            if(numerator > longest) 
            {
                numerator -= longest;
                x1 += dx1;
                y1 += dy1;
            }
            else
            {
                x1 += dx2;
                y1 += dy2;
            }
        }     
    }

    void lifePixel(uint8_t x, uint8_t y, uint32_t colour)
    {
        uint32_t screen = x + y*SCREEN_WIDTH;
        _pixels[screen] = colour*0xFFFFFFFF;
    }

#define LIFE_WIDTH  2000
#define LIFE_HEIGHT 2000
    void life(bool initialise)
    {
        static uint8_t buffers[2][LIFE_HEIGHT][LIFE_WIDTH];
        static uint8_t lut[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};
        static int index = 0;
        static bool initialised = false;

        if(initialise)
        {
            initialised = true;

            for(int j=0; j<LIFE_HEIGHT; j++)
                for(int i=0; i<LIFE_WIDTH; i++)
                    lifePixel(i, j, 0);

            for(int k=0; k<2; k++)
                for(int j=0; j<LIFE_HEIGHT; j++)
                    for(int i=0; i<LIFE_WIDTH; i++)
                        buffers[k][j][i] = 0;
            
            // Gliders            
            for(int i=0; i<8; i+=4)
            {
                buffers[0][100][100+i] = 1; buffers[0][101][100+i] = 1; buffers[0][102][100+i] = 1; buffers[0][102][99+i] = 1; buffers[0][101][98+i] = 1;
                lifePixel(100+i, 100, 1); lifePixel(100+i, 101, 1); lifePixel(100+i, 102, 1); lifePixel(99+i, 102, 1); lifePixel(98+i, 101, 1);
            }

            index = 0;
        }

        if(initialised)
        {      
            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    lut[2] = buffers[index][j][i];
                    int count = buffers[index][j-1][i-1] + buffers[index][j-1][i] + buffers[index][j-1][i+1] + buffers[index][j][i+1] + buffers[index][j+1][i+1] + buffers[index][j+1][i] + buffers[index][j+1][i-1] + buffers[index][j][i-1];
                    buffers[index ^ 1][j][i] = lut[count];
                    if(i < 256  &&  j < 256) lifePixel(i, j, lut[count]);
                }
            }

            index ^= 1;
        }
    }

    void life1(bool initialise)
    {
        static uint8_t buffers[2][LIFE_HEIGHT][LIFE_WIDTH];
        static uint8_t lut[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};
        static bool initialised = false;

        if(initialise)
        {
            initialised = true;

            for(int j=0; j<LIFE_HEIGHT; j++)
                for(int i=0; i<LIFE_WIDTH; i++)
                    lifePixel(i, j, 0);

            for(int k=0; k<2; k++)
                for(int j=0; j<LIFE_HEIGHT; j++)
                    for(int i=0; i<LIFE_WIDTH; i++)
                        buffers[k][j][i] = 0;

            // Gliders            
            for(int i=0; i<8; i+=4)
            {
                buffers[0][100][100+i] = 1; buffers[0][101][100+i] = 1; buffers[0][102][100+i] = 1; buffers[0][102][99+i] = 1; buffers[0][101][98+i] = 1;
                lifePixel(100+i, 100, 1); lifePixel(100+i, 101, 1); lifePixel(100+i, 102, 1); lifePixel(99+i, 102, 1); lifePixel(98+i, 101, 1);
            }
        }

        if(initialised)
        {        
            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    // Increment neighbour counts
                    if(buffers[0][j][i] == 1)
                    {
                        buffers[1][j-1][i-1]++; buffers[1][j-1][i]++; buffers[1][j-1][i+1]++; buffers[1][j][i+1]++; buffers[1][j+1][i+1]++; buffers[1][j+1][i]++; buffers[1][j+1][i-1]++; buffers[1][j][i-1]++;
                    }
                }
            }

            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    lut[2] = buffers[0][j][i];
                    int cell = lut[buffers[1][j][i]];
                    buffers[1][j][i] = 0;
                    buffers[0][j][i] = cell;
                    if(i < 256  &&  j < 256) lifePixel(i, j, cell);
                }
            }
        }
    }

#define TETRIS_XPOS     60
#define TETRIS_YPOS     20
#define TETRIS_XEXT     10
#define TETRIS_YEXT     20
#define NUM_TETROMINOES 7
#define NUM_ROTATIONS   4
#define TETROMINOE_SIZE 4
#define MAX_LINES       4
#define MAX_LEVEL       8
    enum BoardState {Clear=0, Blocked, GameOver};
    struct Tetromino
    {
        uint8_t _colour;
        uint8_t _pattern[NUM_ROTATIONS][4 + TETROMINOE_SIZE*2];
    };
    Tetromino I = 
    {
        0x3C,
        4, 1, 0, 1, 0, 1, 1, 1, 2, 1, 3, 1,
        1, 4, 2, 0, 2, 0, 2, 1, 2, 2, 2, 3,
        4, 1, 0, 2, 0, 2, 1, 2, 2, 2, 3, 2,
        1, 4, 1, 0, 1, 0, 1, 1, 1, 2, 1, 3,
    };
    Tetromino J = 
    {
        0x30,
        3, 2, 0, 0, 0, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 2, 0, 1, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 2, 2,
        2, 3, 0, 0, 1, 0, 1, 1, 0, 2, 1, 2
    };
    Tetromino L = 
    {
        0x0B,
        3, 2, 0, 0, 2, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 1, 2, 2, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 0, 2,
        2, 3, 0, 0, 0, 0, 1, 0, 1, 1, 1, 2
    };
    Tetromino O = 
    {
        0x0F,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1
    };
    Tetromino S = 
    {
        0x0C,
        3, 2, 0, 0, 1, 0, 2, 0, 0, 1, 1, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 2, 1, 2, 2,
        3, 2, 0, 1, 1, 1, 2, 1, 0, 2, 1, 2,
        2, 3, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2
    };
    Tetromino T = 
    {
        0x33,
        3, 2, 0, 0, 1, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 2, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 1, 2,
        2, 3, 0, 0, 1, 0, 0, 1, 1, 1, 1, 2
    };
    Tetromino Z = 
    {
        0x03,
        3, 2, 0, 0, 0, 0, 1, 0, 1, 1, 2, 1,
        2, 3, 1, 0, 2, 0, 1, 1, 2, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 1, 2, 2, 2,
        2, 3, 0, 0, 1, 0, 0, 1, 1, 1, 0, 2
    };
    Tetromino tetrominoes[NUM_TETROMINOES] = {I, J, L, O, S, T, Z};
    int tetrisScore = 0, tetrisLevel = 0, scoreDelta = 0;
    int frameTickLevel = 60;
    int frameTick = frameTickLevel;
    int frameCount = 0;
    int x=TETRIS_XEXT/2, y = -1;
    int index = 0, rotation = 0;
    int w, h, u, v;
    int ox, oy, ov;
    int oindex, orotation;

    uint8_t getTetrisPixel(int x, int y)
    {
        x *= 4;
        y *= 4;
        return getPixelGiga(TETRIS_XPOS + x, TETRIS_YPOS + y);
    }

    void setTetrisPixel(int x, int y, uint8_t colour)
    {
        x *= 4;
        y *= 4;
        drawPixelGiga(TETRIS_XPOS + x + 0, TETRIS_YPOS + y + 0, colour);
        drawPixelGiga(TETRIS_XPOS + x + 0, TETRIS_YPOS + y + 1, colour);
        drawPixelGiga(TETRIS_XPOS + x + 0, TETRIS_YPOS + y + 2, colour);
        drawPixelGiga(TETRIS_XPOS + x + 0, TETRIS_YPOS + y + 3, colour);
        drawPixelGiga(TETRIS_XPOS + x + 1, TETRIS_YPOS + y + 0, colour);
        drawPixelGiga(TETRIS_XPOS + x + 1, TETRIS_YPOS + y + 1, colour);
        drawPixelGiga(TETRIS_XPOS + x + 1, TETRIS_YPOS + y + 2, colour);
        drawPixelGiga(TETRIS_XPOS + x + 1, TETRIS_YPOS + y + 3, colour);
        drawPixelGiga(TETRIS_XPOS + x + 2, TETRIS_YPOS + y + 0, colour);
        drawPixelGiga(TETRIS_XPOS + x + 2, TETRIS_YPOS + y + 1, colour);
        drawPixelGiga(TETRIS_XPOS + x + 2, TETRIS_YPOS + y + 2, colour);
        drawPixelGiga(TETRIS_XPOS + x + 2, TETRIS_YPOS + y + 3, colour);
        drawPixelGiga(TETRIS_XPOS + x + 3, TETRIS_YPOS + y + 0, colour);
        drawPixelGiga(TETRIS_XPOS + x + 3, TETRIS_YPOS + y + 1, colour);
        drawPixelGiga(TETRIS_XPOS + x + 3, TETRIS_YPOS + y + 2, colour);
        drawPixelGiga(TETRIS_XPOS + x + 3, TETRIS_YPOS + y + 3, colour);
    }

    void drawTetromino(int index, int rotation, int x, int y, uint8_t colour)
    {
        for(int i=0; i<TETROMINOE_SIZE; i++)
        {
            int xx = x + tetrominoes[index]._pattern[rotation][4 + i*2];
            int yy = y + tetrominoes[index]._pattern[rotation][5 + i*2];
            if(xx < 0  ||  xx >= TETRIS_XEXT) continue;
            if(yy < 0  ||  yy >= TETRIS_YEXT) continue;

            setTetrisPixel(xx, yy, colour);
        }
    }

    BoardState checkTetromino(int index, int rotation, int x, int y)
    {
        for(int i=0; i<TETROMINOE_SIZE; i++)
        {
            int xx = x + tetrominoes[index]._pattern[rotation][4 + i*2];
            int yy = y + tetrominoes[index]._pattern[rotation][5 + i*2];
            if(xx < 0  ||  xx >= TETRIS_XEXT) continue;
            if(yy < 0  ||  yy >= TETRIS_YEXT) continue;

            if(getTetrisPixel(xx, yy))
            {
                if(y == 0) return GameOver;
                return Blocked;
            }
        }

        return Clear;
    }

    void updateScore(int lines)
    {
        static int scoring[MAX_LINES] = {10, 25, 50, 100};
        static int levelling[MAX_LEVEL] = {500, 500, 1000, 2000, 4000, 8000, 16000, 32000};

        scoreDelta += scoring[(lines - 1) & (MAX_LINES - 1)] * (tetrisLevel + 1);

        // Level up
        if(scoreDelta >= levelling[tetrisLevel & (MAX_LEVEL - 1)])
        {
            tetrisScore += scoreDelta;
            tetrisLevel++;
            frameTickLevel = std::max(60 - 5*tetrisLevel, 15);
            frameTick = frameTickLevel;
            scoreDelta = 0;
        }

        fprintf(stderr, "Tetris: score : %06d  level : %d  frameTickLevel : %d  lines : %d\n", tetrisScore + scoreDelta, tetrisLevel, frameTickLevel, lines);
    }

    int checkLines(void)
    {
        int lines = 0;

        for(int ii=0; ii<MAX_LINES; ii++)
        {
            for(int ll=TETRIS_YEXT-1; ll>=0; ll--)
            {
                bool line = true;
                for(int kk=0; kk<TETRIS_XEXT; kk++) line &= getTetrisPixel(kk, ll) > 0;
                if(line)
                {
                    lines++;

                    for(int nn=ll; nn>0; nn--)
                    {
                        for(int mm=0; mm<TETRIS_XEXT; mm++) setTetrisPixel(mm, nn, getTetrisPixel(mm, nn-1));
                    }
                    for(int kk=0; kk<TETRIS_XEXT; kk++) setTetrisPixel(kk, 0, 0x00);
                }
            }
        }

        if(lines) updateScore(lines);

        return lines;
    }

    void saveTetrominoState(void)
    {
        ox = x;
        oy = y;
        ov = tetrominoes[index]._pattern[rotation][3];
        oindex = index;
        orotation = rotation;
    }

    void updateTetromino(void)
    {
        w = tetrominoes[index]._pattern[rotation][0];
        h = tetrominoes[index]._pattern[rotation][1];
        u = tetrominoes[index]._pattern[rotation][2];
        v = tetrominoes[index]._pattern[rotation][3];
    }

    int spawnTetromino(void)
    {
        index = rand() % NUM_TETROMINOES;
        rotation = rand() & (NUM_ROTATIONS-1);
        x = rand() % (TETRIS_XEXT - (w - 1)) - u;

        updateTetromino();
        saveTetrominoState();
        return checkLines();
    }
 
    void shakeScreen(int lines)
    {
        static int frameCount = 0;
        static int strength = 0;

        if(lines)
        {
            frameCount = 1;
            strength = lines;
        }

        if(frameCount)
        {
            int screenShake = rand() % 4;
            switch(screenShake)
            {
                case 0:
                {
                    Cpu::setRAM(0x0101, strength); 
                }
                break;
         
                case 1:
                {
                    Cpu::setRAM(0x0101, uint8_t(0 - strength));
                }
                break;

                case 2:
                {
                    for(int i=0x0100; i<0x01EE; i+=2) Cpu::setRAM(i, 0x08 + (i-0x0100)/2 + strength); 
                }
                break;
                
                case 3:
                {
                    for(int i=0x0100; i<0x01EE; i+=2) Cpu::setRAM(i, 0x08 + (i-0x0100)/2 + uint8_t(0 - strength)); 
                }
                break;
            }
            
            if(++frameCount >= 20) //strength * 10)
            {
                frameCount = 0;
                Cpu::setRAM(0x0101, 0x00);
                for(int i=0x0100; i<0x01EF; i+=2) Cpu::setRAM(i, 0x08 + (i-0x0101)/2); 
            }
        }
    }

    void tetris(void)
    {
        static bool firstTime = true;
        if(firstTime  &&  Cpu::getClock() > 10000000)
        {
            firstTime = false;
            for(int l=0; l<TETRIS_YEXT; l++)
                for(int k=0; k<TETRIS_XEXT; k++) setTetrisPixel(k, l, 0x00);
#if 0
            Loader::Gt1File gt1File;
            if(!loadGt1File("./vCPU/starfield.gt1", gt1File)) return;
            uint16_t executeAddress = gt1File._loStart + (gt1File._hiStart <<8);
            Editor::setLoadBaseAddress(executeAddress);

            for(int j=0; j<gt1File._segments.size(); j++)
            {
                uint16_t address = gt1File._segments[j]._loAddress + (gt1File._segments[j]._hiAddress <<8);
                for(int i=0; i<gt1File._segments[j]._segmentSize; i++)
                {
                    Cpu::setRAM(address+i, gt1File._segments[j]._dataBytes[i]);
                }
            }

            Cpu::setRAM(0x0016, executeAddress-2 & 0x00FF);
            Cpu::setRAM(0x0017, (executeAddress & 0xFF00) >>8);
            Cpu::setRAM(0x001a, executeAddress-2 & 0x00FF);
            Cpu::setRAM(0x001b, (executeAddress & 0xFF00) >>8);
#endif
        }

        bool refresh = false;

        saveTetrominoState();

        SDL_Event event;
        SDL_PollEvent(&event);

        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                if(y == -1) break;

                switch(event.key.keysym.sym)
                {
                    case SDLK_LEFT:  x--; refresh = true;                                           break;
                    case SDLK_RIGHT: x++; refresh = true;                                           break;
                    case SDLK_DOWN:  frameTick = 2;                                                 break;

                    case SDLK_UP:
                    {
                        static int rotation_old = rotation;
                        rotation = (rotation + 1) & (NUM_ROTATIONS-1);
                        updateTetromino();
                        if(y > TETRIS_YEXT - h - v)
                        {
                            rotation = rotation_old;
                            updateTetromino();
                        }
                        else
                        {
                            refresh = true;
                        }
                    }
                    break;

                    case SDLK_ESCAPE:
                    {
                        SDL_Quit();
                        exit(0);
                    }
                }
            }
            break;

            case SDL_KEYUP:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_DOWN: frameTick = frameTickLevel; break;
                }
            }
            break;
        }

        int lines = 0;
        if(refresh || ++frameCount >= frameTick)
        {
            // Erase old tetromino
            drawTetromino(oindex, orotation, ox, oy-ov, 0x00); 

            updateTetromino();

            if(x < 0 - u) x = 0 - u;
            if(x > TETRIS_XEXT - w - u) x = TETRIS_XEXT - w - u;

            // Update tetromino
            if(frameCount >= frameTick)
            {
                frameCount = 0;

                // Gravity
                if(++y > TETRIS_YEXT - h)
                {
                    // Hit ground
                    drawTetromino(index, rotation, x, y-v-1, tetrominoes[index]._colour);
                    y = -1;
                    lines = spawnTetromino();
                }
            }

            BoardState boardState = checkTetromino(index, rotation, x, y-v);
            switch(boardState)
            {
                case Clear:   drawTetromino(index, rotation, x, y-v, tetrominoes[index]._colour);     break;
                case Blocked:
                {
                    if(!refresh)
                    {
                        drawTetromino(index, rotation, x, y-v-1, tetrominoes[index]._colour);
                        y = -1;
                        lines = spawnTetromino();
                    }
                    else
                    {
                        drawTetromino(oindex, orotation, ox, oy-ov, tetrominoes[oindex]._colour);
                        x = ox, y = oy, v = ov, rotation = orotation, index = oindex;
                    }
                }
                break;
                case GameOver:
                {
                    // Game over
                    scoreDelta = 0;
                    tetrisScore = 0;
                    tetrisLevel = 0;
                    frameTickLevel = 60;
                    frameTick = frameTickLevel;
                    for(int l=0; l<TETRIS_YEXT; l++)
                        for(int k=0; k<TETRIS_XEXT; k++) setTetrisPixel(k, l, 0x00);
                    fprintf(stderr, "Tetris GAME OVER...\n");
                }
                break;
            }
        }

        shakeScreen(lines);
    }
}