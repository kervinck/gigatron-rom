#ifndef LOADER_H
#define LOADER_H


#include <vector>

#include "timing.h"


#define PAYLOAD_SIZE              60
#define SEGMENT_SIZE              255
#define SEGMENT_HEADER_SIZE       3
#define GT1FILE_TRAILER_SIZE      3
#define DEFAULT_START_ADDRESS_HI  0x02
#define DEFAULT_START_ADDRESS_LO  0x00

#define ZERO_CONST_ADDRESS        0x00
#define ONE_CONST_ADDRESS         0x80

#define LOADER_CONFIG_INI  "loader_config.ini"
#define HIGH_SCORES_INI    "high_scores.ini"


namespace Loader
{
    struct Gt1Segment
    {
        bool _isRomAddress = false;
        uint8_t _hiAddress;
        uint8_t _loAddress;
        uint8_t _segmentSize;
        std::vector<uint8_t> _dataBytes;
    }; 

    struct Gt1File
    {
        std::vector<Gt1Segment> _segments;
        uint8_t _terminator=0;
        uint8_t _hiStart=DEFAULT_START_ADDRESS_HI;
        uint8_t _loStart=DEFAULT_START_ADDRESS_LO;
    };


    bool loadGt1File(const std::string& filename, Gt1File& gt1File);
    bool saveGt1File(const std::string& filepath, Gt1File& gt1File, std::string& filename);
    uint16_t printGt1Stats(const std::string& filename, const Gt1File& gt1File);


#ifndef STAND_ALONE
    enum Endianness {Little, Big};
    enum UploadTarget {None, Emulator, Hardware};

    struct SaveData
    {
        bool _initialised = false;
        int _updaterate = VSYNC_RATE;
        std::string _filename;

        std::vector<uint16_t> _counts;
        std::vector<uint16_t> _addresses;
        std::vector<Endianness> _endianness;
        std::vector<std::vector<uint8_t>> _data;
    };

    struct ConfigRom
    {
        uint8_t _type;
        std::string _name;
    };


    std::string& getCurrentGame(void);
    void setCurrentGame(std::string& currentGame);

    UploadTarget getUploadTarget(void);
    void setUploadTarget(UploadTarget target);

    int getConfigRomsSize(void);
    ConfigRom* getConfigRom(int index);

    void initialise(void);

    void disableUploads(bool disable);

    void openComPort(void);
    void closeComPort(void);

    bool readCharGiga(char* chr);
    bool readLineGiga(std::string& line);
    bool readLineGiga(std::vector<std::string>& text);
    bool readUntilPromptGiga(std::vector<std::string>& text);

    bool sendCharGiga(char chr);
    void sendCommandToGiga(char cmd, std::string& line, bool wait);
    void sendCommandToGiga(char cmd, bool wait);
    bool sendCommandToGiga(std::string& cmd, std::vector<std::string>& text);

    bool loadDataFile(SaveData& saveData);
    bool saveDataFile(SaveData& saveData);
    void loadHighScore(void);
    bool saveHighScore(void);
    void updateHighScore(void);

    void upload(int vgaY);
#endif
}

#endif
