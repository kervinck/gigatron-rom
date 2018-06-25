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

#define GIGA_PS2_LEFT    1
#define GIGA_PS2_RIGHT   2
#define GIGA_PS2_UP      3
#define GIGA_PS2_DOWN    4
#define GIGA_PS2_START   7
#define GIGA_PS2_SELECT  8
#define GIGA_PS2_INPUT_A 9
#define GIGA_PS2_INPUT_B 27
#define GIGA_PS2_CR      13
#define GIGA_PS2_DEL     127
#define GIGA_PS2_ENABLE  5
#define GIGA_PS2_DISABLE 6

#define HEX_BASE_ADDRESS   0x0200
#define LOAD_BASE_ADDRESS  0x0200
#define VARS_BASE_ADDRESS  0x0030
#define VIDEO_Y_ADDRESS    0x0009

#define INPUT_CONFIG_INI  "input_config.ini"


namespace Editor
{
    enum MemoryMode {RAM=0, ROM0, ROM1, NumMemoryModes};
    enum EditorMode {Hex=0, Load, Giga, PS2KB, GigaPS2, Debug, NumEditorModes};
    enum FileType {File=0, Dir, Fifo, Link, NumFileTypes};


    int getCursorX(void);
    int getCursorY(void);
    bool getHexEdit(void);
    bool getStartMusic(void);
    bool getSingleStepMode(void);
    MemoryMode getMemoryMode(void);
    EditorMode getEditorMode(void);
    uint8_t getMemoryDigit(void);
    uint8_t getAddressDigit(void);
    uint16_t getHexBaseAddress(void);
    uint16_t getLoadBaseAddress(void);
    uint16_t getVarsBaseAddress(void);
    uint16_t getSingleStepWatchAddress(void);
    uint16_t getCpuUsageAddressA(void);
    uint16_t getCpuUsageAddressB(void);
    int getFileEntriesIndex(void);
    int getFileEntriesSize(void);
    FileType getFileEntryType(int index);
    FileType getCurrentFileEntryType(void);
    std::string* getFileEntryName(int index);
    std::string* getCurrentFileEntryName(void);
    std::string getBrowserPath(bool removeSlash=false);

    void setCursorX(int x);
    void setCursorY(int y);
    void setStartMusic(bool startMusic);
    void setSingleStep(bool singleStep);
    void setSingleStepMode(bool singleStepMode);
    void setLoadBaseAddress(uint16_t address);
    void setSingleStepWatchAddress(uint16_t address);
    void setCpuUsageAddressA(uint16_t address);
    void setCpuUsageAddressB(uint16_t address);

    void initialise(void);
    void browseDirectory(void);
    bool singleStepDebug(void);
    void handleInput(void);
}

#endif