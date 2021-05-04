#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <atomic>

#include "memory.h"
#include "timing.h"
#include "editor.h"
#include "loader.h"
#include "graphics.h"
#include "expression.h"
#include "assembler.h"
#include "inih/INIReader.h"
#include "defaultKeys.h"

// Use this if you ever want to change the default font, but it better be 6x8 per char or otherwise you will be in a world of hurt
//#define CREATE_FONT_HEADER
#ifndef CREATE_FONT_HEADER
#include "emuFont96x48.h"
#endif


namespace Graphics
{
    int _width, _height;
    int _posX = 0, _posY = 0;

    float _aspect = 0.0f;
    float _scaleX, _scaleY;

    bool _fullScreen = false;
    bool _resizable = false;
    bool _borderless = true;
    bool _vSync = false;
    bool _fixedSize = false;

    int _filter = 0;

    bool _displayHelpScreen = false;
    uint8_t _displayHelpScreenAlpha = 0;

    std::atomic<bool> _enableUploadBar(false);
    std::atomic<int> _uploadCursorY(-1);
    std::atomic<float> _uploadPercentage(0.0f);
    std::string _uploadFilename;

    uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t _colours[COLOUR_PALETTE];
    uint32_t _hlineTiming[GIGA_HEIGHT];

    SDL_Window* _window = NULL;
    SDL_Renderer* _renderer = NULL;
    SDL_Texture* _screenTexture = NULL;
    SDL_Texture* _helpTexture = NULL;
    SDL_Surface* _helpSurface = NULL;
    SDL_Surface* _fontSurface = NULL;

    INIReader _configIniReader;

    int getWidth(void) {return _width;}
    int getHeight(void) {return _height;}

    uint32_t* getPixels(void) {return _pixels;}
    uint32_t* getColours(void) {return _colours;}

    SDL_Window* getWindow(void) {return _window;}
    SDL_Renderer* getRenderer(void) {return _renderer;}
    SDL_Texture* getScreenTexture(void) {return _screenTexture;}
    SDL_Texture* getHelpTexture(void) {return _helpTexture;}
    SDL_Surface* getHelpSurface(void) {return _helpSurface;}
    SDL_Surface* getFontSurface(void) {return _fontSurface;}

    void setDisplayHelpScreen(bool display) {_displayHelpScreen = display;}
    void setWidthHeight(int width, int height) {_width = width, _height = height;}

    bool getUploadBarEnabled(void) {return _enableUploadBar;}
    void setUploadFilename(const std::string& uploadFilename) {_uploadFilename = uploadFilename;}
    void updateUploadBar(float uploadPercentage) {_uploadPercentage = uploadPercentage;}
    void enableUploadBar(bool enableUploadBar)
    {
        _uploadPercentage = 0.0f;
        _enableUploadBar = enableUploadBar;
        if(enableUploadBar)
        {
            _uploadCursorY = 34;
        }
        else
        {
            _uploadCursorY = -1;
            _uploadFilename = "";
        }
    }


    SDL_Surface* createSurface(int width, int height)
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

        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
        if(surface == NULL)
        {
            Cpu::shutdown();
            fprintf(stderr, "Graphics::createSurface() :  failed to create SDL surface.\n");
            _EXIT_(EXIT_FAILURE);
        }

        return surface;
    }

    void writeToSurface(const SDL_Surface* surface, const uint32_t* data, int width, int height)
    {
        uint32_t* srcPixels = (uint32_t*)data;
        uint32_t* dstPixels = (uint32_t*)surface->pixels;
        for(int j=0; j<height; j++)
        {
            for(int i=0; i<width; i++)
            {
                *dstPixels++ = *srcPixels++;
            }
        }
    }

    void createFontHeader(const SDL_Surface* fontSurface, const std::string& filename, const std::string& name, int width, int height)
    {
        std::ofstream outfile(filename);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Graphics::createFontHeader() : failed to create '%s'\n", filename.c_str());
            return;
        }

        outfile << "uint32_t " << name + "[] = \n";
        outfile << "{\n";
        outfile << "    ";

        int colCount = 0;
        uint32_t* pixels = (uint32_t*)fontSurface->pixels;
        for(int j=0; j<height; j++)
        {
            for(int i=0; i<width; i++)
            {
                outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << pixels[j*width + i] << ", ";
                if(++colCount == 8)
                {
                    colCount = 0;
                    if(j < height-1  ||  i < width-1) outfile << "\n    ";
                }
            }
        }
        
        outfile << "\n};" << std::endl;
    }

    void createHelpTexture(void)
    {
        bool useDefault = false;
        std::ifstream infile(INPUT_CONFIG_INI);
        if(!infile.is_open()) useDefault = true;

        int lines = 1;
        std::string lineToken;
        std::vector<std::string> lineTokens;

        if(useDefault)
        {
            lineTokens = _defaultKeys;
            lines = int(_defaultKeys.size());
        }
        else
        {
            // Read ini file into lines buffer
            while(!infile.eof())
            {
                std::getline(infile, lineToken);
                lineTokens.push_back(lineToken);
                if(!infile.good()  &&  !infile.eof())
                {
                    Cpu::shutdown();
                    fprintf(stderr, "Graphics::createHelpTexture() : Bad line : '%s' : in %s : on line %d\n", lineToken.c_str(), INPUT_CONFIG_INI, lines);
                    _EXIT_(EXIT_FAILURE);
                }

                lines++;
            }
        }

        // Print text in lines with spaces as padding to the help screen surface
        int maxLines = SCREEN_HEIGHT / (FONT_HEIGHT + 2);
        int numLines = std::min(lines-1, maxLines);
        uint32_t* pixels = (uint32_t*)_helpSurface->pixels;

        for(int i=0; i<numLines; i++)
        {
            int y = i*(FONT_HEIGHT + 2) + (maxLines - numLines)/2 * (FONT_HEIGHT + 2);
            size_t nonWhiteSpace = lineTokens[i].find_first_not_of("  \n\r\f\t\v");

            if(nonWhiteSpace == std::string::npos) lineTokens[i] = std::string(MAX_CHARS_HELP, ' ');
            if(lineTokens[i].size() < MAX_CHARS_HELP) lineTokens[i] += std::string(MAX_CHARS_HELP - lineTokens[i].size(), ' ');
            drawText(lineTokens[i], pixels, 0, y, 0xFF00FF00, false, 0, 0, 0x00000000, false, -1, true, 0xFFFFFFFF, 0xFF00FFFF);

            // Fill in the gaps
            for(int j=FONT_HEIGHT; j<FONT_HEIGHT+2; j++)
            {
                for(int k=0; k<SCREEN_WIDTH*3/4; k++)
                {
                    pixels[(y + j)*SCREEN_WIDTH + k] = 0xFF000000;
                }
            }
        }

        // Create help screen texture
        _helpTexture = SDL_CreateTextureFromSurface(_renderer, _helpSurface);
        if(_helpTexture == NULL)
        {
            Cpu::shutdown();
            fprintf(stderr, "Graphics::createHelpTexture() :  failed to create SDL texture.\n");
            _EXIT_(EXIT_FAILURE);
        }

        // Enable blending on help screen texture
        SDL_SetTextureBlendMode(_helpTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(_helpTexture, 0);
    }

    bool getKeyAsString(const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, std::string& result)
    {
        result = _configIniReader.Get(sectionString, iniKey, defaultKey);
        if(result == defaultKey) return false;
        Expression::strToUpper(result);
        return true;
    }


    void initialise(void)
    {
        // HLINE sync error bar
        for(int i=0; i<GIGA_HEIGHT; i++) _hlineTiming[i] = 0xFF00FF00;

        // Colour palette
        for(int i=0; i<COLOUR_PALETTE; i++)
        {
            uint8_t r = uint8_t(double((i & 0x03) >>0) / 3.0 * 255.0);
            uint8_t g = uint8_t(double((i & 0x0C) >>2) / 3.0 * 255.0);
            uint8_t b = uint8_t(double((i & 0x30) >>4) / 3.0 * 255.0);

            _colours[i] = 0xFF000000 | (r <<16) | (g <<8) | b;
            //fprintf(stderr, "%08X\n", _colours[i]); // use to create a Paint.Net palette
        }

        // Safe resolution by default
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        _aspect = float(DM.w) / float(DM.h);
        _fullScreen = false;
        _resizable = true;
        _borderless = false;
        _vSync = false;
        _fixedSize = false;
        _filter = 0;
        _width = 640;
        _height = 480;
        _scaleX = 1.5f;
        _scaleY = 1.5f;
        _posX = 40;
        _posY = 40;

        // Parse graphics config file
        INIReader iniReader(Loader::getExePath() + "/" + GRAPHICS_CONFIG_INI);
        _configIniReader = iniReader;
        if(_configIniReader.ParseError() < 0)
        {
            fprintf(stderr, "Graphics::initialise() : couldn't load INI file '%s' : reverting to default graphics options.\n", GRAPHICS_CONFIG_INI);
        }
        else
        {
            // Parse Monitor Keys
            enum Section {Monitor};
            std::map<std::string, Section> section;
            section["Monitor"] = Monitor;
            for(auto sectionString : _configIniReader.Sections())
            {
                if(section.find(sectionString) == section.end())
                {
                    fprintf(stderr, "Graphics::initialise() : INI file '%s' has bad Sections : reverting to default values.\n", GRAPHICS_CONFIG_INI);
                    break;
                }

                std::string result;
                switch(section[sectionString])
                {
                    case Monitor:
                    {
                        getKeyAsString(sectionString, "Fullscreen", "0", result);   
                        _fullScreen = strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "Resizable", "1", result);   
                        _resizable = strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "Borderless", "0", result);   
                        _borderless = strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "VSync", "0", result);        
                        _vSync = strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "FixedSize", "0", result);        
                        _fixedSize = strtol(result.c_str(), nullptr, 10);

                        getKeyAsString(sectionString, "Filter", "0", result);        
                        _filter = strtol(result.c_str(), nullptr, 10);
                        _filter = (_filter<0 || _filter>2) ? 0 : _filter;

                        getKeyAsString(sectionString, "Width", "640", result);
                        _width = (result == "DESKTOP") ? DM.w : std::strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "Height", "480", result);
                        _height = (result == "DESKTOP") ? DM.h : std::strtol(result.c_str(), nullptr, 10);

                        getKeyAsString(sectionString, "ScaleX", "1.5", result);
                        _scaleX = std::stof(result.c_str());
                        getKeyAsString(sectionString, "ScaleY", "1.5", result);
                        _scaleY = std::stof(result.c_str());

                        getKeyAsString(sectionString, "PosX", "0", result);
                        _posX = strtol(result.c_str(), nullptr, 10);
                        getKeyAsString(sectionString, "PosY", "0", result);
                        _posY = strtol(result.c_str(), nullptr, 10);
                    }
                    break;

                    default: break;
                }
            }
        }

        // SDL hints, VSync and Batching
        char vsChar = char(_vSync + '0');
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, &vsChar);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, std::to_string(_filter).c_str());

        // Fullscreen
        if(_fullScreen)
        {
            if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &_window, &_renderer) < 0)
            {
                Cpu::shutdown();
                fprintf(stderr, "Graphics::initialise() : failed to create SDL fullscreen window.\n");
                _EXIT_(EXIT_FAILURE);
            }
        }
        // Windowed
        else
        {
            if(_fixedSize)
            {
                SDL_RenderSetLogicalSize(_renderer, _width, _height);

                if(SDL_CreateWindowAndRenderer(_width, _height, 0, &_window, &_renderer) < 0)
                {
                    Cpu::shutdown();
                    fprintf(stderr, "Graphics::initialise() : failed to create SDL window.\n");
                    _EXIT_(EXIT_FAILURE);
                }

                SDL_SetWindowTitle(_window, VERSION_STR);
            }
            else
            {
                // Try and keep faithful to Gigatron's real aspect ratio no matter what the window/monitor resolution is
                _width = int(_width * _scaleX * _aspect * float(_height) / float(_width));
                _height = int(_height * _scaleY);

                if(SDL_CreateWindowAndRenderer(_width, _height, 0, &_window, &_renderer) < 0)
                {
                    Cpu::shutdown();
                    fprintf(stderr, "Graphics::initialise() : failed to create SDL window.\n");
                    _EXIT_(EXIT_FAILURE);
                }

                if(!_borderless)
                {
                    SDL_SetWindowResizable(_window, (SDL_bool)_resizable);
                    SDL_SetWindowTitle(_window, VERSION_STR);
                }

                SDL_SetWindowBordered(_window, (SDL_bool)!_borderless);
                SDL_SetWindowPosition(_window, _posX, _posY);
            }
        }

        // Screen texture
        _screenTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        if(_screenTexture == NULL)
        {
            Cpu::shutdown();
            fprintf(stderr, "Graphics::initialise() :  failed to create SDL texture.\n");
            _EXIT_(EXIT_FAILURE);
        }

#ifdef CREATE_FONT_HEADER
        // Load font file
        SDL_Surface* fontSurface = SDL_LoadBMP("EmuFont-96x48.bmp");
        if(fontSurface == NULL)
        {
            Cpu::shutdown();
            fprintf(stderr, "Graphics::initialise() : failed to create SDL font surface, you're probably missing 'EmuFont-96x48.bmp' in the current directory/path.\n");
            _EXIT_(EXIT_FAILURE);
        }
        _fontSurface = SDL_ConvertSurfaceFormat(fontSurface, SDL_PIXELFORMAT_ARGB8888, NULL);
        SDL_FreeSurface(fontSurface);
        if(_fontSurface == NULL)
        {
            Cpu::shutdown();
            fprintf(stderr, "Graphics::initialise() : failed to convert SDL font surface format to screen surface format.\n");
            _EXIT_(EXIT_FAILURE);
        }
        // Use this if you want to change the default font
        createFontHeader(_fontSurface, "emuFont96x48.h", "_emuFont96x48", FONT_BMP_WIDTH, FONT_BMP_HEIGHT);
#else
        _fontSurface = createSurface(FONT_BMP_WIDTH, FONT_BMP_HEIGHT);
        writeToSurface(_fontSurface, _emuFont96x48, FONT_BMP_WIDTH, FONT_BMP_HEIGHT);
#endif

        // Help screen
        _helpSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT);
        createHelpTexture();

#ifdef _WIN32
        Cpu::restoreWin32Console();
#endif

        SDL_RaiseWindow(_window);
    }

    uint32_t savedPixels[6*5];
    void restoreReticlePixel(int x, int y)
    {
        if(x < 0  ||  x > SCREEN_WIDTH - 1) return;
        if(y < 0  ||  y > SCREEN_HEIGHT - 1) return;
        int index = (x % 5) + (y % 6)*5;
        _pixels[x + y*SCREEN_WIDTH] = savedPixels[index];
    }
    void saveReticlePixel(int x, int y)
    {
        if(x < 0  ||  x > SCREEN_WIDTH - 1) return;
        if(y < 0  ||  y > SCREEN_HEIGHT - 1) return;
        int index = (x % 5) + (y % 6)*5;
        savedPixels[index] = _pixels[x + y*SCREEN_WIDTH];
    }
    void drawReticlePixel(int x, int y)
    {
        if(x < 0  ||  x > SCREEN_WIDTH - 1) return;
        if(y < 0  ||  y > SCREEN_HEIGHT - 1) return;
        //fprintf(stderr, "%d %d\n", x, y);
        _pixels[x + y*SCREEN_WIDTH] = 0xFFFFFFFF; //(0x00FFFFFF ^ _pixels[x + y*SCREEN_WIDTH]) & 0x00FFFFFF;
    }
    void drawReticle(int vgaX, int vgaY)
    {
        for(int j=-1; j<5; j++)
        {
            for(int i=-1; i<4; i++)
            {
                int x = vgaX*3 + i;
                int y = vgaY*4 + j;
                saveReticlePixel(x, y);
                if(j==-1 || j==4 || i==-1 || i==3) drawReticlePixel(x, y);
            }
        }
    }
    void restoreReticle(int vgaX, int vgaY)
    {
        for(int j=-1; j<5; j++)
        {
            for(int i=-1; i<4; i++)
            {
                int x = vgaX*3 + i;
                int y = vgaY*4 + j;
                restoreReticlePixel(x, y);
            }
        }
    }

    void resetVTable(void)
    {
        for(int i=0; i<GIGA_HEIGHT; i++)
        {
            Cpu::setRAM(uint16_t(GIGA_VTABLE + i*2), uint8_t((GIGA_VRAM >>8) + i));
            Cpu::setRAM(uint16_t(GIGA_VTABLE + 1 + i*2), 0x00);
        }
    }

    void refreshTimingPixel(const Cpu::State& S, int vgaX, int pixelY, uint32_t colour, bool debugging)
    {
        UNREFERENCED_PARAM(S);

        _hlineTiming[pixelY % GIGA_HEIGHT] = colour;

        if(debugging) return;

        uint32_t screen = (vgaX % (GIGA_WIDTH + 1))*3 + (pixelY % GIGA_HEIGHT)*4*SCREEN_WIDTH;
        _pixels[screen + 0 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3*SCREEN_WIDTH] = 0x00;
    }

    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY)
    {
        uint32_t colour = _colours[S._OUT & (COLOUR_PALETTE - 1)];
        uint32_t address = (vgaX % GIGA_WIDTH)*3 + (vgaY % SCREEN_HEIGHT)*SCREEN_WIDTH;
        _pixels[address + 0] = colour;
        _pixels[address + 1] = colour;
        _pixels[address + 2] = colour;
    }

    void refreshScreen(void)
    {
        uint8_t offsetx = 0;

        for (int y = 0; y<GIGA_HEIGHT; y++)
        {
            offsetx += Cpu::getRAM(uint16_t(GIGA_VTABLE + 1 + y * 2));

            for (int x = 0; x <= GIGA_WIDTH; x++)
            {
                uint16_t address = (Cpu::getRAM(uint16_t(GIGA_VTABLE + y * 2)) << 8) + ((offsetx + x) & 0xFF);
                uint32_t colour = (x < GIGA_WIDTH) ? _colours[Cpu::getRAM(address) & (COLOUR_PALETTE - 1)] : _hlineTiming[y];
                uint32_t screen = (y * 4 % SCREEN_HEIGHT)*SCREEN_WIDTH + (x * 3 % SCREEN_WIDTH);

                _pixels[screen + 0 + 0 * SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0 * SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0 * SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 1 * SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1 * SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1 * SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 2 * SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2 * SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2 * SCREEN_WIDTH] = colour;
                _pixels[screen + 0 + 3 * SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3 * SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3 * SCREEN_WIDTH] = 0x00;
            }
        }
    }

    void clearScreen(uint32_t colour)
    {
        for(int y=0; y<SCREEN_HEIGHT; y++)
        {
            for(int x=0; x<SCREEN_WIDTH*3/4; x++)
            {
                _pixels[y*SCREEN_WIDTH + x] = colour;
            }
        }
    }

    void rectFill(int x0, int y0, int x1, int y1, uint32_t colour)
    {
        x0 *= 3;
        x1 *= 3;
        y0 *= 4;
        y1 *= 4;
        for(int y=y0; y<=y1; y++)
        {
            for(int x=x0; x<=x1; x++)
            {
                _pixels[y*SCREEN_WIDTH + x] = colour;
            }
        }
    }

    void pixelReticle(const Cpu::State& S, int vgaX, int vgaY)
    {
        // Draw pixel reticle, but only for active pixels
        if(S._PC >= 0x020D  &&  S._PC <= 0x02AD)
        {
            if(vgaX > 0) restoreReticle(vgaX - 1, vgaY/4);
            if(vgaX < HPIXELS_END - HPIXELS_START) drawReticle(vgaX, vgaY/4);
        }
    }

    void drawPixel(uint8_t x, uint8_t y, uint32_t colour)
    {
        x = x % GIGA_WIDTH;
        y = y % GIGA_HEIGHT;
        uint32_t screen = x*3 + y*4*SCREEN_WIDTH;

        _pixels[screen + 0 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 0*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 0*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 1*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 1*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 1 + 2*SCREEN_WIDTH] = colour; _pixels[screen + 2 + 2*SCREEN_WIDTH] = colour;
        _pixels[screen + 0 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 1 + 3*SCREEN_WIDTH] = 0x00;   _pixels[screen + 2 + 3*SCREEN_WIDTH] = 0x00;
    }

    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t colour)
    {
        int16_t sx = x2 - x1;
        int16_t sy = y2 - y1;
        int h = sy;
        int16_t dx1 = 1, dx2 = 1, dy1 = 1, dy2 = 0;

        if(sx & 0x8000)
        {
            dx1 = -1;
            dx2 = -1;
            sx = 0 - sx;
        }

        if(sy & 0x8000)
        {
            dy1 = -1;
            sy = 0 - sy;
            if(sx < sy) dy2 = -1;
        }

        if(sx < sy) 
        {
            dx2 = 0;
            std::swap(sx, sy);
            if(h > 0) dy2 = 1;
        }

        int16_t numerator = sx >> 1;
        int16_t xy1 = x1 | (y1<<8);
        int16_t xy2 = x2 | (y2<<8);
        int16_t dxy1 = dx1 + (dy1<<8);
        int16_t dxy2 = dx2 + (dy2<<8);

        for(uint16_t i=0; i<=sx/2; i++) 
        {
            drawPixel(uint8_t(xy1), uint8_t(xy1>>8), colour);
            drawPixel(uint8_t(xy2), uint8_t(xy2>>8), colour);
            numerator += sy;
            if(numerator > sx) 
            {
                numerator -= sx;
                xy1 += dxy1;
                xy2 -= dxy1;
            }
            else
            {
                xy1 += dxy2;
                xy2 -= dxy2;
            }
        }     
    }

    void drawLeds(void)
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

    // Simple text routine, font is a non proportional 6*8 font loaded from a 96*48 BMP file
    bool drawText(const std::string& text, uint32_t* pixels, int x, int y, uint32_t fgColour, bool invert, int invertSize, int invertPos, 
                  uint32_t bgColour, bool colourKey, int numChars, bool fullscreen, uint32_t commentColour, uint32_t sectionColour)
    {
        if(!fullscreen)
        {
            x += UI_START_X;
            y += UI_START_Y;
        }
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return false;

        uint32_t* fontPixels = (uint32_t*)_fontSurface->pixels;
        numChars = (numChars == -1) ? int(text.size()) : numChars;
        for(int i=0; i<numChars; i++)
        {
            if(sectionColour)
            {
                static bool useSectionColour = false;
                if(x == 0) useSectionColour = false;
                if(text.c_str()[i] == '[') useSectionColour = true;
                if(text.c_str()[i] == ']') useSectionColour = false;
                if(useSectionColour) fgColour = sectionColour;
            }
            if(commentColour)
            {
                static bool useCommentColour = false;
                if(x == 0) useCommentColour = false;
                if(text.c_str()[i] == ';') useCommentColour = true;
                if(useCommentColour) fgColour = commentColour;
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
                    uint32_t fontPixel = fontPixels[fontAddress] & 0x00FFFFFF;
                    if((invert  &&  i>=invertPos  &&  i<invertPos+invertSize) ? !fontPixel : fontPixel)
                    {
                        pixels[pixelAddress] = 0xFF000000 | fgColour;
                    }
                    else
                    {
                        if(!colourKey) pixels[pixelAddress] = 0xFF000000 | bgColour;
                    }
                }
            }
        }

        return true;
    }

    bool drawText(const std::string& text, int x, int y, uint32_t fgColour, bool invert, int invertSize, int invertPos)
    {
        return drawText(text, _pixels, x, y, fgColour, invert, invertSize, invertPos, 0x00000000, true, -1, true, 0x00000000, 0x00000000);
    }

    bool drawMenuItem(const std::string& text, int x, int y, uint32_t fgColour, bool invert, int invertSize, uint32_t bgColour)
    {
        return drawText(text, _pixels, x, y, fgColour, invert, invertSize, 0, bgColour, false, -1, true, 0x00000000, 0x00000000);
    }

    bool drawDialog(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t bgColour, uint32_t bdColour)
    {
        Graphics::drawLine(x,   y,   x+w, y,       bdColour);
        Graphics::drawLine(x,   y+h+1, x+w, y+h+1, bdColour);
        Graphics::drawLine(x,   y,   x,   y+h+1,   bdColour);
        Graphics::drawLine(x+w, y,   x+w, y+h+1,   bdColour);
        for(int16_t yy=y+1; yy<=y+h; yy++)
        {
            Graphics::drawLine(x+1, yy, x+w-1, yy, bgColour);
        }

        return true;
    }

    bool drawDialogItem(const std::string& text, int x, int y, uint32_t fgColour, uint32_t bdColour)
    {
        return Graphics::drawText(text, _pixels, x, y, fgColour, false, 0, 0, bdColour, false, -1, true, 0x00000000, 0x00000000);
    }

    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour)
    {
        x += UI_START_X;
        y += UI_START_Y;
        if(x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT) return;

        uint32_t pixelAddress = x + digit*FONT_WIDTH + y*SCREEN_WIDTH;

        pixelAddress += (FONT_HEIGHT-1)*SCREEN_WIDTH;
        for(int i=0; i<FONT_WIDTH; i++) _pixels[pixelAddress+i] = colour;
    }

    float powStepRising(float x, float a, float b, float p)
    {
        float f = std::min(std::max(x, a), b);
        f = (f - a) / (b - a);
        return powf(f, p);
    }
    float powStepFalling(float x, float a, float b, float p)
    {
        float f = std::max(std::min(x, a), b);
        f = (f - a) / (b - a);
        return powf(f, p);
    }
    void drawUsageBar(float usage, int x, int y, int w, int h)
    {
        int ww = int(float(w)*usage);

        x += UI_START_X;
        y += UI_START_Y;

        for(int j=y; j<(y + h); j++)
        {
            for(int i=x; i<(x + ww); i++)
            {
                float normalised = float(i - x) / float(w);
                uint8_t red = uint8_t(powStepRising(normalised, 0.0f, 0.5f, 1.0f) * 255.0f);
                uint8_t grn = uint8_t(powStepFalling(normalised, 1.0f, 0.5f, 1.0f) * 255.0f);
                int pixelAddress = i + j*SCREEN_WIDTH;
                _pixels[pixelAddress] = (red <<16) + (grn <<8);
            }
            for(int i=(x + ww); i<(x + w); i++)
            {
                int pixelAddress = i + j*SCREEN_WIDTH;
                _pixels[pixelAddress] = 0x00400000;
            }
        }
    }

    void drawUploadBar(float upload)
    {
        if(!_enableUploadBar  ||  _uploadCursorY == -1) return;

        char uploadPercentage[UI_TEXT_SIZE+2] = "";
        std::string uploadFilename = _uploadFilename;
        if(uploadFilename.size() < UI_TEXT_SIZE+1) uploadFilename.append(UI_TEXT_SIZE+1 - uploadFilename.size(), ' ');
        if(upload < 1.0f)
        {
            strcpy(uploadPercentage, uploadFilename.substr(0, UI_TEXT_SIZE+1).c_str());
            sprintf(&uploadPercentage[UI_TEXT_SIZE+1 - 6], " %3d%%\r", int(upload * 100.0f));
        }

        drawText(uploadPercentage, _pixels, HEX_START_X, int(FONT_CELL_Y*4.4) + _uploadCursorY*FONT_CELL_Y, 0xFFB0B0B0, true, UI_TEXT_SIZE+1, 0, 0x00000000, false, UI_TEXT_SIZE+1);
    }

    void renderText(void)
    {
        char str[32];

        sprintf(str, "CPU        A:%04X B:%04X", 0x200, 0x220);
        drawText(std::string(str), _pixels, 0, FONT_CELL_Y*2, 0xFFFFFFFF, false, 0, false);
        sprintf(str, "%05.1f%%", Cpu::getvCpuUtilisation() * 100.0);
        drawUsageBar(Cpu::getvCpuUtilisation(), FONT_WIDTH*4 - 3, FONT_CELL_Y*2 - 3, FONT_WIDTH*6 + 5, FONT_HEIGHT + 5);
        drawText(std::string(str), _pixels, FONT_WIDTH*4, FONT_CELL_Y*2, 0x80808080, false, 0, 0, 0x00000000, true);

        //drawText(std::string("LEDS:"), _pixels, 0, 0, 0xFFFFFFFF, false, 0);
        sprintf(str, "FPS %5.1f  XOUT:%02X IN:%02X", 1.0f / Timing::getFrameTime(), Cpu::getXOUT(), Cpu::getIN());
        drawText(std::string(str), _pixels, 0, FONT_CELL_Y, 0xFFFFFFFF, false, 0);
        drawText("M:              R:", _pixels, 0, 472 - FONT_CELL_Y, 0xFFFFFFFF, false, 0);

        switch(Editor::getEditorMode())
        {
            case Editor::Hex:   (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Hex   "); break;
            case Editor::Rom:   (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Rom   "); break;
            case Editor::Load:  (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Load  "); break;
            case Editor::Dasm:  (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Dasm  "); break;
            case Editor::Term:  (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Term  "); break;
            case Editor::Image: (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Image "); break;
            case Editor::Audio: (Editor::getSingleStepEnabled()) ? strcpy(str, "Debug ") : strcpy(str, "Audio "); break;

            default: strcpy(str, "     ");
        }
        drawText(std::string(str), _pixels, 12, 472 - FONT_CELL_Y, 0xFF00FF00, false, 0);

        switch(Editor::getKeyboardMode())
        {
            case Editor::Giga:   strcpy(str, "Kbd   "); break;
            case Editor::PS2:    strcpy(str, "PS2   "); break;
            case Editor::HwGiga: strcpy(str, "HwKbd "); break;
            case Editor::HwPS2:  strcpy(str, "HwPS2 "); break;

            default: strcpy(str, "     ");
        }
        drawText("K:", _pixels, 48, 472 - FONT_CELL_Y, 0xFFFFFFFF, false, 0);
        drawText(std::string(str), _pixels, 60, 472 - FONT_CELL_Y, 0xFF00FF00, false, 0);

        sprintf(str, "%-5d", Memory::getSizeFreeRAM());
        drawText(std::string(str), _pixels, RAM_START, 472 - FONT_CELL_Y, 0xFF00FF00, false, 0);
        sprintf(str, " ROM %02X", Cpu::getRomType());
        drawText(std::string(VERSION_STR) + std::string(str), _pixels, 0, 472, 0xFFFFFFFF, false, 0);
    }

    int renderHexMonitor(bool onHex)
    {
        char str[32] = "";
        int hexDigitIndex = -1;

        switch(Editor::getMemoryMode())
        {
            case Editor::RAM:  drawText("RAM:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
            case Editor::ROM0: drawText("ROM0:      Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
            case Editor::ROM1: drawText("ROM1:      Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;

            default: break;
        }

        // 8 * 32 hex display of memory
        uint16_t hexAddress = Editor::getHexBaseAddress();
        uint16_t cursorAddress = Editor::getHexBaseAddress();
        for(int j=0; j<HEX_CHARS_Y; j++)
        {
            for(int i=0; i<HEX_CHARS_X; i++)
            {
                uint8_t value = 0;
                switch(Editor::getMemoryMode())
                {
                    case Editor::RAM:  value = Cpu::getRAM(hexAddress);    break;
                    case Editor::ROM0: value = Cpu::getROM(hexAddress, 0); break;
                    case Editor::ROM1: value = Cpu::getROM(hexAddress, 1); break;

                    default: break;
                }
                sprintf(str, "%02X ", value);
                bool onCursor = (i == Editor::getCursorX()  &&  j == Editor::getCursorY());
                if(onCursor) hexDigitIndex = j*HEX_CHARS_X + i;
                uint32_t colour = (Editor::getHexEdit() && Editor::getMemoryMode() == Editor::RAM && onCursor) ? 0xFF00FF00 : 0xFFB0B0B0;
                drawText(std::string(str), _pixels, HEX_START_X + i*HEX_CHAR_WIDE, FONT_CELL_Y*4 + j*(FONT_HEIGHT+FONT_GAP_Y), colour, onCursor, 2);
                if(onCursor) cursorAddress = hexAddress;
                hexAddress = (Editor::getMemoryMode() == Editor::RAM) ? (hexAddress + 1) & (Memory::getSizeRAM() - 1) : hexAddress + 1;
            }
        }

        sprintf(str, "%04X", cursorAddress);
        uint32_t colour = (Editor::getHexEdit() && onHex) ? 0xFF00FF00 : 0xFFFFFFFF;
        drawText(std::string(str), _pixels, HEX_START, FONT_CELL_Y*3, colour, onHex, 4);

        // Edit digit select for monitor
        if(Editor::getHexEdit())
        {
            // Draw memory digit selection box                
            if(Editor::getCursorY() >= 0  &&  Editor::getCursorY() < 32  &&  Editor::getCursorX() < 8  &&  Editor::getMemoryMode() == Editor::RAM)
            {
                drawDigitBox(Editor::getMemoryDigit(), HEX_START_X + Editor::getCursorX()*HEX_CHAR_WIDE, FONT_CELL_Y*4 + Editor::getCursorY()*FONT_CELL_Y, 0xFFFF00FF);
            }
        }

        return hexDigitIndex;
    }

    void renderRomBrowser(void)
    {
        drawText("ROM:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0);

        // Clear window
        for(int i=0; i<HEX_CHARS_Y; i++) drawText("                       ", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, false, 0);

        // ROM list
        for(int i=0; i<HEX_CHARS_Y; i++)
        {
            bool onCursor = (i == Editor::getCursorY());
            int index = Editor::getRomEntriesIndex() + i;
            if(index >= int(Editor::getRomEntriesSize())) break;
            uint32_t colour = (i < NUM_INT_ROMS) ? 0xFFB0B0B0 : 0xFFFFFFFF;
            if(i == Cpu::getRomIndex()) drawText("*", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y,  0xFFD0D000, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
            drawText(*Editor::getRomEntryName(index), _pixels, HEX_START_X + 6, FONT_CELL_Y*4 + i*FONT_CELL_Y, colour, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
        }

        // ROM type
        char str[32] = "";
        (Editor::getCursorY() < 0  ||  Editor::getCursorY() >= Editor::getRomEntriesSize()) ? sprintf(str, "  ") : sprintf(str, "%02X", Editor::getRomEntryType(Editor::getCursorY()));
        drawText(std::string(str), _pixels, HEX_START-6, FONT_CELL_Y*3, 0xFFFFFFFF, false, 4);
    }

    void renderLoadBrowser(bool onHex)
    {
        uint16_t hexLoadAddress = (Editor::getEditorMode() == Editor::Load) ? Editor::getLoadBaseAddress() : Editor::getHexBaseAddress();

        drawText("Load:      Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0);

        // Clear window
        for(int i=0; i<HEX_CHARS_Y; i++) drawText("                       ", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, false, 0);

        // File list
        for(int i=0; i<HEX_CHARS_Y; i++)
        {
            bool onCursor = (i == Editor::getCursorY());
            int index = Editor::getFileEntriesIndex() + i;
            if(index >= int(Editor::getFileEntriesSize())) break;
            uint32_t colour = (Editor::getFileEntryType(index) == Editor::Dir) ? 0xFFB0B0B0 : 0xFFFFFFFF;
            drawText(*Editor::getFileEntryName(index), _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y, colour, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
        }

        // Load address
        char str[32] = "";
        sprintf(str, "%04X", hexLoadAddress);
        uint32_t colour = (Editor::getHexEdit() && onHex) ? 0xFF00FF00 : 0xFFFFFFFF;
        drawText(std::string(str), _pixels, HEX_START, FONT_CELL_Y*3, colour, onHex, 4);
    }

    void renderDisassembler(bool onHex)
    {
        char str[32] = "";

        (Editor::getMemoryMode() == Editor::RAM) ? Assembler::disassemble(Editor::getVpcBaseAddress()) : Assembler::disassemble(Editor::getNtvBaseAddress());

        //sprintf(str, "%d\n", Editor::getVpcBreakpointsSize());
        //sprintf(str, "%d\n", Editor::getNtvBreakpointsSize());
        //fprintf(stderr, str);

        // Clear window
        for(int i=0; i<HEX_CHARS_Y; i++) drawText("                       ", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y, 0xFFFFFFFF, false, 0);

        for(int i=0; i<Assembler::getDisassembledCodeSize(); i++)
        {
            bool onPC = false;
            if(Editor::getSingleStepEnabled())
            {
                if(Editor::getMemoryMode() == Editor::RAM)
                {
                    onPC = (Assembler::getDisassembledCode(i)->_address == Cpu::getVPC());
                }
                else
                {
                    onPC = (Assembler::getDisassembledCode(i)->_address == Cpu::getStateS()._PC);
                }
            }

            // Program counter icon in debug mode
            bool onCursor = (i == Editor::getCursorY());
            if(onPC) drawText(">", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y,  0xFF00FF00, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);

            // Breakpoint icons
            if(Editor::getMemoryMode() == Editor::RAM)
            {
                for(int j=0; j<Editor::getVpcBreakPointsSize(); j++)
                {
                    if(Assembler::getDisassembledCode(i)->_address == Editor::getVpcBreakPointAddress(j)  &&  Editor::getSingleStepEnabled())
                    {
                        drawText("*", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y,  0xFFC000C0, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
                        break;
                    }
                }
            }
            else
            {
                for(int j=0; j<Editor::getNtvBreakPointsSize(); j++)
                {
                    if(Assembler::getDisassembledCode(i)->_address == Editor::getNtvBreakPointAddress(j)  &&  Editor::getSingleStepEnabled())
                    {
                        drawText("*", _pixels, HEX_START_X, FONT_CELL_Y*4 + i*FONT_CELL_Y,  0xFFC0C000, onCursor, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
                        break;
                    }
                }
            }

            // Mnemonic, highlight if on vPC and show cursor in debug mode
            uint32_t colour = (onPC) ? 0xFFFFFFFF : 0xFFB0B0B0;
            bool highlight = (onCursor || onPC)  &&  (Editor::getSingleStepEnabled());
            drawText(Assembler::getDisassembledCode(i)->_text, _pixels, HEX_START_X+6, FONT_CELL_Y*4 + i*FONT_CELL_Y, colour, highlight, UI_TEXT_SIZE, 0, 0x00000000, false, UI_TEXT_SIZE);
        }

        switch(Editor::getMemoryMode())
        {
            case Editor::RAM:  drawText("RAM:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
            case Editor::ROM0: drawText("ROM:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;
            case Editor::ROM1: drawText("ROM:       Vars:", _pixels, 0, FONT_CELL_Y*3, 0xFFFFFFFF, false, 0); break;

            default: break;
        }

        (Editor::getMemoryMode() == Editor::RAM) ? sprintf(str, "%04X", Editor::getVpcBaseAddress()) : sprintf(str, "%04X", Editor::getNtvBaseAddress());
        uint32_t colour = (Editor::getHexEdit() && onHex) ? 0xFF00FF00 : 0xFFFFFFFF;
        drawText(std::string(str), _pixels, HEX_START, FONT_CELL_Y*3, colour, onHex, 4);

        if(Editor::getMemoryMode() == Editor::RAM)
        {
            // Display vCPU registers
            sprintf(str, "PC:%04X LR:%04X Fn:%04X", Cpu::getVPC(), Cpu::getRAM(0x001A) | (Cpu::getRAM(0x001B)<<8), Cpu::getRAM(0x0022) | (Cpu::getRAM(0x0023)<<8));
            drawText(std::string(str), _pixels, HEX_START_X, int(FONT_CELL_Y*2.0) + FONT_CELL_Y*HEX_CHARS_Y, 0xFF00FFFF, false, 0);
            sprintf(str, "AC:%04X SP:%04X Sr:%04X", Cpu::getRAM(0x0018) | (Cpu::getRAM(0x0019)<<8), Assembler::getvSpMin() | (Cpu::getRAM(0x001C)<<8), Cpu::getRAM(0x000F) | (Cpu::getRAM(0x0011)<<8));
            drawText(std::string(str), _pixels, HEX_START_X, int(FONT_CELL_Y*3.0) + FONT_CELL_Y*HEX_CHARS_Y, 0xFF00FFFF, false, 0);
        }
        else
        {
            // Display Native registers
            sprintf(str, "PC:%04X  IR:%02X  OUT:%02X", Cpu::getStateS()._PC, Cpu::getStateT()._IR, Cpu::getStateT()._OUT);
            drawText(std::string(str), _pixels, HEX_START_X, int(FONT_CELL_Y*2.0) + FONT_CELL_Y*HEX_CHARS_Y, 0xFF00FFFF, false, 0);
            sprintf(str, "AC:%02X  X:%02X  Y:%02X  D:%02X", Cpu::getStateT()._AC, Cpu::getStateT()._X, Cpu::getStateT()._Y, Cpu::getStateT()._D);
            drawText(std::string(str), _pixels, HEX_START_X, int(FONT_CELL_Y*3.0) + FONT_CELL_Y*HEX_CHARS_Y, 0xFF00FFFF, false, 0);
        }
    }

    void renderTextWindow(void)
    {
        char str[32] = "";

        int x, y, cy;
        Editor::getMouseUiCursor(x, y, cy);
        Editor::setCursorX(x);
        Editor::setCursorY(cy);

        // Addresses
        uint16_t cpuUsageAddressA = Editor::getCpuUsageAddressA();
        uint16_t cpuUsageAddressB = Editor::getCpuUsageAddressB();
        uint16_t varsAddress = Editor::getVarsBaseAddress();

        // Mouse cursor over vars
        Editor::OnVarType onVarType = Editor::getOnVarType();
        bool onHex   = (onVarType == Editor::OnHex);
        bool onCpuA  = (onVarType == Editor::OnCpuA);
        bool onCpuB  = (onVarType == Editor::OnCpuB);
        bool onVars  = (onVarType == Editor::OnVars);
        bool onWatch = (onVarType == Editor::OnWatch);
        uint8_t onVarMask = uint8_t(onHex) | (uint8_t(onCpuA) <<1) | (uint8_t(onCpuB) <<2) | (uint8_t(onVars) <<3) | (uint8_t(onWatch) <<4);
        static uint8_t onVarMaskPrev = onVarMask;

        // Text window
        int hexDigitIndex = -1;
        static int hexDigitIndexPrev = hexDigitIndex;
        switch(Editor::getEditorMode())
        {
            case Editor::Hex:  hexDigitIndex = renderHexMonitor(onHex); break;
            case Editor::Rom:  renderRomBrowser();                      break;
            case Editor::Audio:
            case Editor::Image:
            case Editor::Load: renderLoadBrowser(onHex);                break;
            case Editor::Dasm: renderDisassembler(onHex);               break;

            default: break;
        }

        // Disable hex/var editing when mouse moves off a currently editing field
        if(onVarMask != onVarMaskPrev  ||  hexDigitIndex != hexDigitIndexPrev) Editor::setHexEdit(false);
        onVarMaskPrev = onVarMask;
        hexDigitIndexPrev = hexDigitIndex;

        // Draw addresses
        if(Editor::getSingleStepEnabled())
        {
            drawText("Watch:", _pixels, WATCH_START, FONT_CELL_Y*2, 0xFFFFFFFF, false, 0);
            sprintf(str, "%04X   ", Editor::getSingleStepAddress());
            uint32_t colour = (Editor::getHexEdit() && onWatch) ? 0xFF00FF00 : 0xFFFFFFFF;
            drawText(str, _pixels, WATCH_START+36, FONT_CELL_Y*2, colour, onWatch, 4);
        }
        else
        {
            sprintf(str, "%04X", cpuUsageAddressA);
            uint32_t colourA = (Editor::getHexEdit() && onCpuA) ? 0xFF00FF00 : 0xFFFFFFFF;
            drawText(std::string(str), _pixels, CPUA_START, FONT_CELL_Y*2, colourA, onCpuA, 4);
            sprintf(str, "%04X", cpuUsageAddressB);
            uint32_t colourB = (Editor::getHexEdit() && onCpuB) ? 0xFF00FF00 : 0xFFFFFFFF;
            drawText(std::string(str), _pixels, CPUB_START, FONT_CELL_Y*2, colourB, onCpuB, 4);
        }
        sprintf(str, "%04X", varsAddress);
        uint32_t colour = (Editor::getHexEdit() && onVars) ? 0xFF00FF00 : 0xFFFFFFFF;
        drawText(std::string(str), _pixels, VAR_START, FONT_CELL_Y*3, colour, onVars, 4);

        // Edit digit select for addresses
        if(Editor::getHexEdit())
        {
            // Draw address digit selections
            if(onCpuA)       drawDigitBox(Editor::getAddressDigit(), CPUA_START,     FONT_CELL_Y*2, 0xFFFF00FF);
            else if(onCpuB)  drawDigitBox(Editor::getAddressDigit(), CPUB_START,     FONT_CELL_Y*2, 0xFFFF00FF);
            else if(onHex)   drawDigitBox(Editor::getAddressDigit(), HEX_START,      FONT_CELL_Y*3, 0xFFFF00FF);
            else if(onVars)  drawDigitBox(Editor::getAddressDigit(), VAR_START,      FONT_CELL_Y*3, 0xFFFF00FF);
            else if(onWatch) drawDigitBox(Editor::getAddressDigit(), WATCH_START+36, FONT_CELL_Y*2, 0xFFFF00FF);
        }

        // 8 * 2 hex display of vCPU program variables
        for(int j=0; j<2; j++)
        {
            for(int i=0; i<HEX_CHARS_X; i++)
            {
                sprintf(str, "%02X ", Cpu::getRAM(varsAddress++));
                drawText(std::string(str), _pixels, HEX_START_X + i*HEX_CHAR_WIDE, int(FONT_CELL_Y*4.25) + FONT_CELL_Y*HEX_CHARS_Y + j*(FONT_HEIGHT+FONT_GAP_Y), 0xFF00FFFF, false, 0);
            }
        }

        // Upload bar
        drawUploadBar(_uploadPercentage);

        // Page up/down icons
        strcpy(str, "^");
        drawText(std::string(str), _pixels, PAGEUP_START_X, PAGEUP_START_Y, 0xFF00FF00, Editor::getPageUpButton(), 1);
        str[0] = 127; str[1] = 0;
        drawText(std::string(str), _pixels, PAGEDN_START_X, PAGEDN_START_Y, 0xFF00FF00, Editor::getPageDnButton(), 1);

        // Delete icon, (currently only used for clearing breakpoints)
        if(Editor::getEditorMode() == Editor::Dasm  &&  (Editor::getSingleStepEnabled()))
        {
            int numBrkPoints = (Editor::getMemoryMode() == Editor::RAM) ? Editor::getVpcBreakPointsSize() : Editor::getNtvBreakPointsSize();
            sprintf(str, "%02d", numBrkPoints);
            drawText(str, _pixels, DELALL_START_X-12, DELALL_START_Y, 0xFFFFFFFF, false, 0);
            drawText("x", _pixels, DELALL_START_X, DELALL_START_Y, 0xFFFF0000, Editor::getDelAllButton(), 1);
        }
        else
        {
            drawText("   ", _pixels, DELALL_START_X-12, DELALL_START_Y, 0, false, 0);
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
                if(_displayHelpScreenAlpha > 240) _displayHelpScreenAlpha = 0;
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
}
