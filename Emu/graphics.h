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
#define CHARS_PER_ROW    16
#define HEX_CHARS_X      8
#define HEX_CHARS_Y      32
#define HEX_CHAR_WIDE    18
#define COLOUR_PALETTE   64
#define NUM_LEDS         4
#define MENU_START_X     488
#define MENU_START_Y     0


namespace Graphics
{
    uint32_t* getPixels(void);
    uint32_t* getColours(void);

    SDL_Window* getWindow(void);
    SDL_Renderer* getRenderer(void);
    SDL_Texture* getScreenTexture(void);
    SDL_Surface* getScreenSurface(void);
    SDL_Texture* getHelpTexture(void);
    SDL_Surface* getHelpSurface(void);
    SDL_Surface* getFontSurface(void);

    void setDisplayHelpScreen(bool display);

    void initialise(void);

    void resetVTable(void);

    void refreshTimingPixel(const Cpu::State& S, int vgaX, int pixelY, uint32_t colour, bool debugging);
    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY, bool debugging);
    void refreshScreen(void);

    void drawLeds(void);
    bool drawText(const std::string& text, uint32_t* pixels, int x, int y, uint32_t colour, bool invert, int invertSize, bool fullscreen=false, uint32_t commentColour=0x00000000, uint32_t sectionColour=0x00000000);
    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour);

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
