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


namespace Graphics
{
    uint32_t* getPixels(void);
    uint32_t* getColours(void);

    SDL_Window* getWindow(void);
    SDL_Renderer* getRenderer(void);
    SDL_Texture* getTexture(void);
    SDL_Surface* getSurface(void);
    SDL_Surface* getFontSurface(void);


    void initialise(void);

    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY);
    void refreshScreen(void);

    void drawLeds(void);
    bool drawText(const std::string& text, int x, int y, uint32_t colour, bool invert, int invertSize);
    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour);

    void renderText(void);
    void renderTextWindow(void);
    void render(bool synchronise=true);

    void drawLine(int x, int y, int x2, int y2, uint32_t colour);
    void drawLineGiga(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint8_t colour);
    void life(bool initialise);
    void life1(bool initialise);
}

#endif
