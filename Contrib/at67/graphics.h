#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <string>
#include <SDL.h>

#include "cpu.h"


#define SCREEN_WIDTH     640
#define SCREEN_HEIGHT    480
#define GIGA_WIDTH       160
#define GIGA_HEIGHT      120
#define GIGA_VRAM        0x0800
#define GIGA_VTABLE      0x0100
#define FONT_BMP_WIDTH   96
#define FONT_BMP_HEIGHT  48
#define FONT_WIDTH       6
#define FONT_HEIGHT      8
#define FONT_GAP_Y       4
#define FONT_CELL_Y      (FONT_HEIGHT+FONT_GAP_Y)
#define MAX_CHARS_SCREEN (SCREEN_WIDTH/FONT_WIDTH)
#define MAX_CHARS_HELP   80
#define CHARS_PER_ROW    16
#define HEX_CHARS_X      8
#define HEX_CHARS_Y      32
#define HEX_CHAR_WIDE    18
#define COLOUR_PALETTE   64
#define NUM_LEDS         4
#define MENU_START_X     488
#define MENU_START_Y     0
#define MENU_CHARS_X     8
#define MENU_CHARS_Y     (SCREEN_HEIGHT/FONT_CELL_Y)
#define HEX_START        30
#define HEX_START_X      0
#define VAR_START        96
#define RAM_START        108
#define CPUA_START       78
#define CPUB_START       120
#define WATCH_START      66
#define MENU_TEXT_SIZE   23
#define PAGEUP_START_X   140
#define PAGEUP_START_Y   44
#define PAGEDN_START_X   140
#define PAGEDN_START_Y   428
#define DELALL_START_X   140
#define DELALL_START_Y   32

#define GRAPHICS_CONFIG_INI  "graphics_config.ini"


namespace Graphics
{
    int getWidth(void);
    int getHeight(void);

    uint32_t* getPixels(void);
    uint32_t* getColours(void);

    SDL_Window* getWindow(void);
    SDL_Renderer* getRenderer(void);
    SDL_Texture* getScreenTexture(void);
    SDL_Texture* getHelpTexture(void);
    SDL_Surface* getHelpSurface(void);
    SDL_Surface* getFontSurface(void);

    void setDisplayHelpScreen(bool display);
    void setWidthHeight(int width, int height);

    void initialise(void);

    void resetVTable(void);

    void refreshTimingPixel(const Cpu::State& S, int vgaX, int pixelY, uint32_t colour, bool debugging);
    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY, bool debugging);
    void refreshScreen(void);

    void drawLeds(void);
    bool drawText(const std::string& text, uint32_t* pixels, int x, int y, uint32_t colour, bool invert, int invertSize, bool colourKey=false, int size=-1, bool fullscreen=false, uint32_t commentColour=0x00000000, uint32_t sectionColour=0x00000000);
    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour);
    void drawUploadBar(float upload);

    void renderText(void);
    void renderTextWindow(void);
    void render(bool synchronise=true);

    void drawLine(int x, int y, int x2, int y2, uint32_t colour);
    void drawLineGiga(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint8_t colour);
    void life(bool initialise);
    void life1(bool initialise);
    void tetris(void);
}

#endif
