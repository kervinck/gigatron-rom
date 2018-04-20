#include <algorithm>

#include "graphics.h"
#include "timing.h"
#include "editor.h"


namespace Graphics
{
    uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t _colours[COLOUR_PALETTE];

    SDL_Window* _window = NULL;
    SDL_Renderer* _renderer = NULL;
    SDL_Texture* _texture = NULL;
    SDL_Surface* _surface = NULL;
    SDL_Surface* _fontSurface = NULL;


    uint32_t* getPixels(void) {return _pixels;}
    uint32_t* getColours(void) {return _colours;}

    SDL_Window* getWindow(void) {return _window;}
    SDL_Renderer* getRenderer(void) {return _renderer;}
    SDL_Texture* getTexture(void) {return _texture;}
    SDL_Surface* getSurface(void) {return _surface;}
    SDL_Surface* getFontSurface(void) {return _fontSurface;}


    void initialise(void)
    {
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
        if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &_window, &_renderer) < 0)
        {
            SDL_Quit();
            fprintf(stderr, "Error: failed to create SDL _window\n");
            exit(EXIT_FAILURE);
        }

        _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
        if(_texture == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Error: failed to create SDL _texture\n");
            exit(EXIT_FAILURE);
        }

        _surface = SDL_GetWindowSurface(_window);
        if(_surface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Error: failed to create SDL _surface\n");
            exit(EXIT_FAILURE);
        }

        // Text
        SDL_Surface* fontSurface = SDL_LoadBMP("EmuFont-256x128.bmp"); 
        if(fontSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Error: failed to create SDL font _surface\n");
            exit(EXIT_FAILURE);
        }
        _fontSurface = SDL_ConvertSurfaceFormat(fontSurface, _surface->format->format, NULL);
        SDL_FreeSurface(fontSurface);
        if(_fontSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Error: failed to create SDL converted font _surface\n");
            exit(EXIT_FAILURE);
        }
    }

    void drawPixel(const Cpu::State& S, int vgaX, int vgaY)
    {
        uint32_t colour = _colours[S._OUT & (COLOUR_PALETTE-1)];
        uint32_t address = vgaX*3 + vgaY*SCREEN_WIDTH - 0;
        _pixels[address + 0] = colour;
        _pixels[address + 1] = colour;
        _pixels[address + 2] = colour;
    }

    void drawLeds(void)
    {
        // Update 60 times per second no matter what the FPS is
        if(Timing::getFrameTime()  &&  Timing::getFrameUpdate())
        {
            for(int i=0; i<NUM_LEDS; i++)
            {
                int mask = 1 << (NUM_LEDS-1 - i);
                int state = (Cpu::getXOUT() & mask) != 0;
                uint32_t colour = state ? 0xFF00FF00 : 0xFF770000;

                int address = int(float(SCREEN_WIDTH) * 0.875f) + i*NUM_LEDS + 6*SCREEN_WIDTH;
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

    // Simple text routine, font is a non proportional 8*16 font loaded from a 256*128 BMP file
    bool drawText(const std::string& text, int x, int y, uint32_t colour, bool invert, int invertSize)
    {
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return false;

        uint32_t* fontPixels = (uint32_t*)_fontSurface->pixels;
        for(int i=0; i<text.size(); i++)
        {
            uint8_t chr = text.c_str()[i];
            uint8_t row = chr % CHARS_PER_ROW;
            uint8_t col = chr / CHARS_PER_ROW;

            int srcx = row*FONT_WIDTH, srcy = col*FONT_HEIGHT;
            if(srcx+FONT_WIDTH-1>=FONT_BMP_WIDTH || srcy+FONT_HEIGHT-1>=FONT_BMP_HEIGHT) return false;

            int dstx = x + i*FONT_WIDTH, dsty = y;
            if(dstx+FONT_WIDTH-1>=SCREEN_WIDTH || dsty+FONT_HEIGHT-1>=SCREEN_HEIGHT) return false;

            for(int j=0; j<FONT_WIDTH; j++)
            {
                for(int k=0; k<FONT_HEIGHT; k++)
                {
                    int fontAddress = (srcx + j)  +  (srcy + k)*FONT_BMP_WIDTH;
                    int pixelAddress = (dstx + j)  +  (dsty + k)*SCREEN_WIDTH;
                    if((invert  &&  i<invertSize) ? !fontPixels[fontAddress] : fontPixels[fontAddress])
                    {
                        _pixels[pixelAddress] = 0xFF000000 | colour;
                    }
                    else
                    {
                        _pixels[pixelAddress] = 0xFF000000;
                    }
                }
            }
        }

        return true;
    }

    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour)
    {
        uint32_t pixelAddress = x + digit*FONT_WIDTH + y*SCREEN_WIDTH;

        pixelAddress += (FONT_HEIGHT-4)*SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        pixelAddress += SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        pixelAddress += SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
        pixelAddress += SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;

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

            drawText(std::string("LEDS:"), 485, 0, 0xFFFFFFFF, false, 0);
            drawText(std::string("FPS:"), 485, 20, 0xFFFFFFFF, false, 0);
            drawText(std::string(str), 548, 20, 0xFFFFFF00, false, 0);
            drawText(std::string(VERSION_STR), 525, 464, 0xFFFFFF00, false, 0);
            sprintf(str, "XOUT: %02X  IN: %02X", Cpu::getXOUT(), Cpu::getIN());
            drawText(std::string(str), 485, 40, 0xFFFFFFFF, false, 0);
            drawText("Mode:       ", 485, 448, 0xFFFFFFFF, false, 0);
            sprintf(str, "Hex ");
            if(Editor::getHexEdit() == true) sprintf(str, "Edit");
            if(Editor::getLoadFile() == true) sprintf(str, "Load");
            drawText(std::string(str), 541, 448, 0xFF00FF00, false, 0);
        }
    }

    void renderTextWindow(void)
    {
        // Update 60 times per second no matter what the FPS is
        if(Timing::getFrameTime()  &&  Timing::getFrameUpdate())
        {
            char str[32] = "";

            // File load
            if(Editor::getLoadFile() == true)
            {
                if(Editor::getCursorY() == -1) Editor::setCursorY(HEX_CHARS_Y-1);
                if(Editor::getCursorY() >= Editor::getDirectoryNamesSize()) Editor::setCursorY(0);
                if(Editor::getCursorY() < 0) Editor::setCursorY(std::min(int(Editor::getDirectoryNamesSize()), HEX_CHARS_Y) - 1);
                drawText("vCPU Files:", 485, 60, 0xFFFFFFFF, false, 0);
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    drawText("                  ", 493, 80 + i*FONT_HEIGHT, 0xFFFFFFFF, false, 0);
                }
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    int index = Editor::getDirectoryNamesIndex() + i;
                    if(index >= int(Editor::getDirectoryNamesSize())) break;
                    drawText(*Editor::getDirectoryName(Editor::getDirectoryNamesIndex() + i), 493, 80 + i*FONT_HEIGHT, 0xFFFFFFFF, i == Editor::getCursorY(), 18);
                }
            }
            // Hex monitor
            else
            {
                switch(Editor::getHexRomMode())
                {
                    case 0: drawText("RAM:   ", 485, 60, 0xFFFFFFFF, false, 0); break;
                    case 1: drawText("ROM 0: ", 485, 60, 0xFFFFFFFF, false, 0); break;
                    case 2: drawText("ROM 1: ", 485, 60, 0xFFFFFFFF, false, 0); break;
                }

                uint16_t address = Editor::getHexBaseAddress();
                bool onCursor = Editor::getCursorY() == -1;
                sprintf(str, "%04X  ", address);
                drawText(std::string(str), 541, 60, (Editor::getHexEdit() && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 4);
                for(int j=0; j<HEX_CHARS_Y; j++)
                {
                    for(int i=0; i<HEX_CHARS_X; i++)
                    {
                        uint8_t value = 0;
                        switch(Editor::getHexRomMode())
                        {
                            case 0: value = Cpu::getRAM(address++);    break;
                            case 1: value = Cpu::getROM(address++, 0); break;
                            case 2: value = Cpu::getROM(address++, 1); break;
                        }
                        sprintf(str, "%02X ", value);
                        bool onCursor = (i == Editor::getCursorX()  &&  j == Editor::getCursorY());
                        drawText(std::string(str), 493 + i*HEX_CHAR_WIDE, 80 + j*FONT_HEIGHT, (Editor::getHexEdit() && Editor::getHexRomMode() ==0 && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 2);
                    }
                }
                if(Editor::getHexEdit() == true)
                {
                    // Draw address digit selection box
                    if(Editor::getCursorY() == -1) drawDigitBox(Editor::getAddressDigit(), 541, 60, 0xFFFF00FF);

                    // Draw memory digit selection box                
                    if(Editor::getCursorY() >= 0  &&  Editor::getHexRomMode() == 0) drawDigitBox(Editor::getMemoryDigit(), 493 + Editor::getCursorX()*HEX_CHAR_WIDE, 80 + Editor::getCursorY()*FONT_HEIGHT, 0xFFFF00FF);
                }
            }
        }
    }

    void render(void)
    {
        drawLeds();
        renderText();
        renderTextWindow();

        SDL_UpdateTexture(_texture, NULL, _pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
        SDL_RenderPresent(_renderer);

        Timing::synchronise();
    }
}