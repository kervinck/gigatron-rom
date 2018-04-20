#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <string>
#include <SDL.h>

#include "cpu.h"


#define SCREEN_WIDTH     640
#define SCREEN_HEIGHT    480
#define FONT_BMP_WIDTH   256
#define FONT_BMP_HEIGHT  128
#define FONT_WIDTH       8
#define FONT_HEIGHT      16
#define CHARS_PER_ROW    32
#define HEX_CHARS_X      6
#define HEX_CHARS_Y      23
#define HEX_CHAR_WIDE    24
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

    void drawPixel(const Cpu::State& S, int vgaX, int vgaY);
    void drawLeds(void);
    bool drawText(const std::string& text, int x, int y, uint32_t colour, bool invert, int invertSize);
    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour);

    void renderText(void);
    void renderTextWindow(void);
    void render(void);
}

#endif
