#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <string>
#include <vector>


#define GTRGB_IDENTIFIER "GTRGB"


namespace Image
{
    // Possible future formats, (GT_RGB_222 is the only one currently supported)
    enum GtRgbFormats {GT_RGB_222=0, GT_RGBA_2222, GT_Index_8, GT_RGB_565, GT_RGBA_5551, GT_RGBA_4444, GT_RGB_888, GT_RGBA_8888};
    enum DiffusionType {LumError0=0, LumError1, LumError2, LumError3, RgbError, NumDiffusionTypes};

    
    // Byte align and pack the structs so that we can read/write them directly, (should be compatible for Windows, Linux and hopefully MacOS)
    #pragma pack(push, 1)
    struct GtRgbHeader
    {
        char _name[6] = GTRGB_IDENTIFIER;
        uint16_t _format;
        uint16_t _width;
        uint16_t _height;
    };
    struct GtRgbFile
    {
        GtRgbHeader _header;
        std::vector<uint8_t> _data;
        std::vector<uint16_t> _optional;
    };

    struct TgaHeader
    {
        uint8_t  _idLength;
        uint8_t  _colourMapType;
        uint8_t  _imageType;
        uint16_t _colourMapOrigin;
        uint16_t _colourMapLength;
        uint8_t  _colourMapDepth;
        uint16_t _originX;
        uint16_t _originY;
        uint16_t _width;
        uint16_t _height;
        uint8_t  _bitsPerPixel;
        uint8_t  _imageDescriptor;
    };
    struct TgaFile
    {
        TgaHeader _header;
        std::vector<uint8_t> _data;
        uint8_t _imageOrigin = 0x00;
    };
    #pragma pack(pop)


    double getGammaInput(void);
    double getGammaOutput(void);
    double getDiffusionScale(void);
    DiffusionType getDiffusionType(void);

    void setGammaInput(double gammaInput);
    void setGammaOutput(double gammaOutput);
    void setDiffusionScale(double diffusionScale);
    void setDiffusionType(int diffusionType);


    void initialise(void);

    bool loadGtRgbFile(const std::string& filename, GtRgbFile& gtRgbFile);
    bool saveGtRgbFile(const std::string& filename, GtRgbFile& gtRgbFile);
    bool loadTgaFile(const std::string& filename, TgaFile& tgaFile);

    bool convertRGB8toRGB2(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin);
    bool convertRGBA8toRGB2(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin);
    bool ditherRGB8toRGB2(std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin);
    bool ditherRGBA8toRGB2(std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin);

    void process(void);
}

#endif
