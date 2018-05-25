#ifndef LOADER_H
#define LOADER_H


#include <vector>


#define PAYLOAD_SIZE              60
#define SEGMENT_SIZE              255
#define SEGMENT_HEADER_SIZE       3
#define GT1FILE_TRAILER_SIZE      3
#define DEFAULT_START_ADDRESS_HI  0x02
#define DEFAULT_START_ADDRESS_LO  0x00


namespace Loader
{
    enum Endianness {Little, Big};

    struct SaveData
    {
        bool _initialised = false;
        std::string _filename;
        std::vector<uint16_t> _counts;
        std::vector<uint16_t> _addresses;
        std::vector<Endianness> _endianness;
        std::vector<std::vector<uint8_t>> _data;
    };

    struct Gt1Segment
    {
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


    bool getStartUploading(void);
    void setStartUploading(bool start);

    void initialise(void);

    // Only for emulation
    bool loadDataFile(SaveData& saveData);
    bool saveDataFile(const SaveData& saveData);

    bool loadGt1File(const std::string& filename, Gt1File& gt1File);
    bool saveGt1File(const std::string& filename, const Gt1File& gt1File);

    void loadHighScore(void);
    void saveHighScore(void);
    void updateHighScore(void);

    void upload(int vgaY);
    void disableUploads(bool disable);
}

#endif
