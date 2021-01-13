#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <string>
#include <vector>
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
#define MAX_SCREEN_COLS  80
#define MAX_SCREEN_ROWS  48
#define COLOUR_PALETTE   64
#define NUM_LEDS         4
#define UI_START_X       488
#define UI_START_Y       0
#define UI_CHARS_X       8
#define UI_CHARS_Y       (SCREEN_HEIGHT/FONT_CELL_Y)
#define HEX_START        30
#define HEX_START_X      0
#define VAR_START        96
#define RAM_START        108
#define CPUA_START       78
#define CPUB_START       120
#define WATCH_START      66
#define UI_TEXT_SIZE     23
#define PAGEUP_START_X   140
#define PAGEUP_START_Y   48
#define PAGEDN_START_X   140
#define PAGEDN_START_Y   420
#define DELALL_START_X   140
#define DELALL_START_Y   36

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

    bool getUploadBarEnabled(void);
    void setUploadFilename(const std::string& uploadFilename);
    void enableUploadBar(bool enableUploadBar);
    void updateUploadBar(float uploadPercentage);

    void initialise(void);

    void resetVTable(void);

    void refreshTimingPixel(const Cpu::State& S, int vgaX, int pixelY, uint32_t colour, bool debugging);
    void refreshPixel(const Cpu::State& S, int vgaX, int vgaY);
    void refreshScreen(void);

    void clearScreen(uint32_t colour);
    void rectFill(int x0, int y0, int x1, int y1, uint32_t colour);

    void pixelReticle(const Cpu::State& S, int vgaX, int vgaY);
    
    void drawPixel(uint8_t x, uint8_t y, uint32_t colour);
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t colour);

    void drawLeds(void);
    bool drawText(const std::string& text, int x, int y, uint32_t fgColour, bool invert, int invertSize, int invertPos=0);
    bool drawMenuItem(const std::string& text, int x, int y, uint32_t fgColour, bool invert, int invertSize, uint32_t bgColour);
    bool drawDialog(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t bgColour, uint32_t bdColour);
    bool drawDialogItem(const std::string& text, int x, int y, uint32_t fgColour, uint32_t bdColour);
    bool drawText(const std::string& text, uint32_t* pixels, int x, int y, uint32_t fgColour, bool invert, int invertSize, int invertPos=0,
                  uint32_t bgColour=0x00000000, bool colourKey=false, int numChars=-1, bool fullscreen=false, uint32_t commentColour=0x00000000, uint32_t sectionColour=0x00000000);
    void drawDigitBox(uint8_t digit, int x, int y, uint32_t colour);

    void renderText(void);
    void renderTextWindow(void);
    void render(bool synchronise=true);
}

#endif
