#include <algorithm>

#include "graphics.h"
#include "timing.h"
#include "editor.h"


namespace Graphics
{
    uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t _colours[COLOUR_PALETTE];
    uint32_t _hlineTiming[GIGA_HEIGHT];

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
        if(SDL_CreateWindowAndRenderer(DM.w+1, DM.h, 0, &_window, &_renderer) < 0)
        //if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &_window, &_renderer) < 0) //SDL_WINDOW_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL window\n");
            exit(EXIT_FAILURE);
        }

        _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
        if(_texture == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() :  failed to create SDL texture\n");
            exit(EXIT_FAILURE);
        }

        _surface = SDL_GetWindowSurface(_window);
        if(_surface == NULL)
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
            fprintf(stderr, "Graphics::initialise() : failed to create SDL font _surface\n");
            exit(EXIT_FAILURE);
        }
        _fontSurface = SDL_ConvertSurfaceFormat(fontSurface, _surface->format->format, NULL);
        SDL_FreeSurface(fontSurface);
        if(_fontSurface == NULL)
        {
            SDL_Quit();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL converted font _surface\n");
            exit(EXIT_FAILURE);
        }
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
                int state = (Cpu::getXOUT() & mask) != 0;
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
    bool drawText(const std::string& text, int x, int y, uint32_t colour, bool invert, int invertSize)
    {
        x += MENU_START_X;
        y += MENU_START_Y;
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return false;

        uint32_t* fontPixels = (uint32_t*)_fontSurface->pixels;
        for(int i=0; i<text.size(); i++)
        {
            uint8_t chr = text.c_str()[i] - 32;
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

            drawText(std::string("LEDS:"), 0, 0, 0xFFFFFFFF, false, 0);
            drawText(std::string("FPS:"), 0, FONT_CELL_Y, 0xFFFFFFFF, false, 0);
            drawText(std::string(str), 62, FONT_CELL_Y, 0xFFFFFF00, false, 0);
            drawText(std::string(VERSION_STR), 46, 472, 0xFFFFFF00, false, 0);
            sprintf(str, "XOUT: %02X  IN: %02X", Cpu::getXOUT(), Cpu::getIN());
            drawText(std::string(str), 0, FONT_CELL_Y*2, 0xFFFFFFFF, false, 0);
            drawText("Mode:       ", 0, 472 - FONT_CELL_Y, 0xFFFFFFFF, false, 0);
            sprintf(str, "Hex  ");
            if(Editor::getHexEdit()) sprintf(str, "Edit ");
            else if(Editor::getEditorMode() == Editor::Load) sprintf(str, "Load ");
            else if(Editor::getEditorMode() == Editor::Debug) sprintf(str, "Debug");
            drawText(std::string(str), 68, 472 - FONT_CELL_Y, 0xFF00FF00, false, 0);
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
                drawText("Load:       Vars:", 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0);
                uint16_t loadAddress = Editor::getLoadBaseAddress();
                uint16_t varsAddress = Editor::getVarsBaseAddress();
                bool onCursor0 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 0;
                bool onCursor1 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 1;
                sprintf(str, "%04X", loadAddress);
                drawText(std::string(str), 36, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor0) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor0, 4);
                sprintf(str, "%04X", varsAddress);
                drawText(std::string(str), 108, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor1) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor1, 4);
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    drawText("                       ", 8, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, false, 0);
                }
                for(int i=0; i<HEX_CHARS_Y; i++)
                {
                    int index = Editor::getFileNamesIndex() + i;
                    if(index >= int(Editor::getFileNamesSize())) break;
                    drawText(*Editor::getFileName(Editor::getFileNamesIndex() + i), 8, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, i == Editor::getCursorY(), 18);
                }

                // 8 * 2 hex display of vCPU program variables
                for(int j=0; j<2; j++)
                {
                    for(int i=0; i<HEX_CHARS_X; i++)
                    {
                        sprintf(str, "%02X ", Cpu::getRAM(varsAddress++));
                        drawText(std::string(str), 8 + i*HEX_CHAR_WIDE, int(FONT_CELL_Y*4.25) + FONT_CELL_Y*HEX_CHARS_Y + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFF00FFFF, false, 0);
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
                    case Editor::RAM:  drawText("RAM:        Vars:", 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                    case Editor::ROM0: drawText("ROM0:       Vars:", 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                    case Editor::ROM1: drawText("ROM1:       Vars:", 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
                }

                // 8 * 32 hex display of memory
                uint16_t hexddress = Editor::getHexBaseAddress();
                uint16_t varsAddress = Editor::getVarsBaseAddress();
                bool onCursor = Editor::getCursorY() == -1;
                bool onCursor0 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 0;
                bool onCursor1 = Editor::getCursorY() == -1  &&  (Editor::getCursorX() & 0x01) == 1;
                sprintf(str, "%04X", hexddress);
                drawText(std::string(str), 36, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor0) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor0, 4);
                sprintf(str, "%04X", varsAddress);
                drawText(std::string(str), 108, FONT_CELL_Y*3, (Editor::getHexEdit() && onCursor1) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor1, 4);
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
                        drawText(std::string(str), 8 + i*HEX_CHAR_WIDE, FONT_CELL_Y*4 + j*(FONT_HEIGHT+FONT_GAP_Y), (Editor::getHexEdit() && Editor::getMemoryMode() == Editor::RAM && onCursor) ? 0xFF00FF00 : 0xFFFFFFFF, onCursor, 2);
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
                        drawText(std::string(str), 0 + i*15, FONT_CELL_Y*4 + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFFFFFFFF, false, 0);
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
                        drawText(std::string(str), 8 + i*HEX_CHAR_WIDE, int(FONT_CELL_Y*4.25) + FONT_CELL_Y*HEX_CHARS_Y + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFF00FFFF, false, 0);
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

    void render(bool synchronise)
    {
        drawLeds();
        renderText();
        renderTextWindow();

        SDL_UpdateTexture(_texture, NULL, _pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
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
}