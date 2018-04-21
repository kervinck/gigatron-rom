#ifndef EDITOR_H
#define EDITOR_H

#include <stdint.h>
#include <string>


#define INPUT_RIGHT   0x01
#define INPUT_LEFT    0x02
#define INPUT_DOWN    0x04
#define INPUT_UP      0x08
#define INPUT_START   0x10
#define INPUT_SELECT  0x20
#define INPUT_B       0x40
#define INPUT_A       0x80


namespace Editor
{
    enum EditorModes {Hex=0, Load, GigaLoad, NumEditorModes};


    int getCursorX(void);
    int getCursorY(void);
    bool getHexEdit(void);
    bool getLoadFile(void);
    int getEditorMode(void);
    int getHexRomMode(void);
    uint8_t getMemoryDigit(void);
    uint8_t getAddressDigit(void);
    uint16_t getHexBaseAddress(void);
    uint16_t getLoadBaseAddress(void);
    uint16_t getVarsBaseAddress(void);
    int getDirectoryNamesIndex(void);
    int getDirectoryNamesSize(void);
    std::string* getDirectoryName(int index);

    void setCursorX(int x);
    void setCursorY(int y);

    void handleInput(void);
}

#endif