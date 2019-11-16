#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "cpu.h"
#include "image.h"
#include "graphics.h"


namespace Image
{
    const int _gtRgbFormatSize[] = {1, 1, 1, 2, 2, 2, 3, 4};

    bool _hostIsBigEndian = false;

    double _gammaInput = 1.0;
    double _gammaOutput = 1.0;
    double _diffusionScale = 32.0;

    DiffusionType _diffusionType = LumError3;


    double getGammaInput(void) {return _gammaInput;}
    double getGammaOutput(void) {return _gammaOutput;}
    double getDiffusionScale(void) {return _diffusionScale;}
    DiffusionType getDiffusionType(void) {return _diffusionType;}

    void setGammaInput(double gammaInput) 
    {
        if(gammaInput < 0.5) gammaInput = 0.5;
        if(gammaInput > 5.0) gammaInput = 5.0;
        _gammaInput = gammaInput;
    }
    void setGammaOutput(double gammaOutput)
    {
        if(gammaOutput < 0.5) gammaOutput = 0.5;
        if(gammaOutput > 5.0) gammaOutput = 5.0;
        _gammaOutput = gammaOutput;
    }
    void setDiffusionScale(double diffusionScale)
    {
        if(diffusionScale > 256.0) diffusionScale = 8.0;
        if(diffusionScale < 8.0) diffusionScale = 256.0;
        _diffusionScale = diffusionScale;
    }

    void setDiffusionType(int diffusionType)
    {
        if(diffusionType < 0) diffusionType = NumDiffusionTypes - 1;
        if(diffusionType > NumDiffusionTypes - 1) diffusionType = 0;
        _diffusionType = (DiffusionType)diffusionType;
    }
    

    void initialise(void)
    {
        if(Cpu::getHostEndianess() == Cpu::BigEndian) _hostIsBigEndian = true;
    }

    bool getFileSize(const std::string& filename, std::ifstream::streampos& fileSize)
    {
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open()) return false;

        fileSize = infile.tellg();
        infile.seekg(0, std::ios::end);
        fileSize = infile.tellg() - fileSize;

        return true;
    }

    // Currently only supports RGB222 format
    bool loadGtRgbFile(const std::string& filename, GtRgbFile& gtRgbFile)
    {
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        std::ifstream::streampos fileSize = 0;
        if(!getFileSize(filename, fileSize))
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : couldn't get file size of '%s'\n", filename.c_str());
            return false;
        }

        // Read header
        GtRgbHeader header;
        infile.read((char *)&header, sizeof(GtRgbHeader));
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : bad header in '%s'\n", filename.c_str());
            return false;
        }

        std::string name = std::string(header._name, sizeof(GTRGB_IDENTIFIER));
        if(name.find(GTRGB_IDENTIFIER) == std::string::npos)
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : bad header identifier in '%s'\n", filename.c_str());
            return false;
        }

        // Big endian conversion
        if(_hostIsBigEndian)
        {
            Cpu::swapEndianess(header._format);
            Cpu::swapEndianess(header._width);
            Cpu::swapEndianess(header._height);
        }

        if(header._format > GtRgbFormats::GT_RGB_888)
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : bad header format : %04x : in '%s'\n", header._format, filename.c_str());
            return false;
        }

        int formatSize = _gtRgbFormatSize[header._format];
        int totalSize = header._width * header._height * formatSize;

        // Quick sanity check on total size, (maximum RAM is 64K)
        if(totalSize >= 0x10000)
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : image is bigger than 64K bytes : width=%d : height=%d : format=%04x : in '%s'\n", header._width, header._height, header._format, filename.c_str());
            return false;
        }

        if(header._width == 0  ||  header._height == 0)
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : width and height both have to be non zero : width=%d : height=%d : format=%04x : in '%s'\n", header._width, header._height, header._format, filename.c_str());
            return false;
        }

        gtRgbFile._header = header;
        gtRgbFile._data.resize(totalSize);

        // Read data
        infile.read((char *)&gtRgbFile._data[0], totalSize);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : bad data in '%s'\n", filename.c_str());
            return false;
        }

        // Optional data
        size_t optionalSize = infile.tellg() - fileSize;
        if(optionalSize < 0  ||  (optionalSize & 1)) // should never be -ve or odd
        {
            fprintf(stderr, "Loader::loadGtRgbFile() : bad optional size in '%s'\n", filename.c_str());
            return false;
        }
        else if(optionalSize > 0)
        {
            int numOptionalData = int(optionalSize) / 2;
            gtRgbFile._optional.resize(numOptionalData);
            infile.read((char *)&gtRgbFile._optional[0], optionalSize);

            if(_hostIsBigEndian)
            {
                for(int i=0; i<numOptionalData; i++) Cpu::swapEndianess(gtRgbFile._optional[i]);
            }
        }

        return true;
    }

    bool saveGtRgbFile(const std::string& filename, GtRgbFile& gtRgbFile)
    {
        std::ofstream outfile(filename, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        if(gtRgbFile._header._format > GtRgbFormats::GT_RGB_888)
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : bad header format : %04x : for '%s'\n", gtRgbFile._header._format, filename.c_str());
            return false;
        }

        int formatSize = _gtRgbFormatSize[gtRgbFile._header._format];
        int totalSize = gtRgbFile._header._width * gtRgbFile._header._height * formatSize;

        // Quick sanity check on total size, (maximum RAM is 64K)
        if(totalSize >= 0x10000)
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : image is bigger than 64K bytes : width=%d : height=%d : format=%04x : in '%s'\n", gtRgbFile._header._width, gtRgbFile._header._height, gtRgbFile._header._format, filename.c_str());
            return false;
        }

        if(gtRgbFile._header._width == 0  ||  gtRgbFile._header._height == 0)
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : width and height both have to be non zero : width=%d : height=%d : format=%04x : in '%s'\n", gtRgbFile._header._width, gtRgbFile._header._height, gtRgbFile._header._format, filename.c_str());
            return false;
        }

        // Wrong size
        if(totalSize != gtRgbFile._data.size())
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : image size does not match data size : image size=%d : data size=%d : in '%s'\n", totalSize, int(gtRgbFile._data.size()), filename.c_str());
            return false;
        }

        // Big endian conversion
        if(_hostIsBigEndian)
        {
            Cpu::swapEndianess(gtRgbFile._header._format);
            Cpu::swapEndianess(gtRgbFile._header._width);
            Cpu::swapEndianess(gtRgbFile._header._height);
        }

        // Write header
        outfile.write((char *)&gtRgbFile._header, sizeof(GtRgbHeader));
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : write error in header of '%s'\n", filename.c_str());
            return false;
        }

        // Big endian conversion
        if(_hostIsBigEndian)
        {
            Cpu::swapEndianess(gtRgbFile._header._format);
            Cpu::swapEndianess(gtRgbFile._header._width);
            Cpu::swapEndianess(gtRgbFile._header._height);
        }

        // Write data
        outfile.write((char *)&gtRgbFile._data[0], gtRgbFile._data.size());
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveGtRgbFile() : write error in data of '%s'\n", filename.c_str());
            return false;
        }

        // Write optional data
        size_t numOptionalData = gtRgbFile._optional.size();
        if(numOptionalData > 0)
        {
            if(_hostIsBigEndian)
            {
                for(int i=0; i<numOptionalData; i++) Cpu::swapEndianess(gtRgbFile._optional[i]);
            }

            outfile.write((char *)&gtRgbFile._optional[0], numOptionalData*2);

            if(_hostIsBigEndian)
            {
                for(int i=0; i<numOptionalData; i++) Cpu::swapEndianess(gtRgbFile._optional[i]);
            }
        }


        return true;
    }

    bool loadTgaFile(const std::string& filename, TgaFile& tgaFile)
    {
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadTgaFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        // Read header
        TgaHeader header;
        infile.read((char *)&header, sizeof(TgaHeader));
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad header in '%s'\n", filename.c_str());
            return false;
        }

        if(_hostIsBigEndian)
        {
            Cpu::swapEndianess(header._colourMapLength);
            Cpu::swapEndianess(header._colourMapOrigin);
            Cpu::swapEndianess(header._originX);
            Cpu::swapEndianess(header._originY);
            Cpu::swapEndianess(header._width);
            Cpu::swapEndianess(header._height);
        }

        if(header._colourMapType != 0)
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad colourMapType %d, the only valid colourMapType is 0 : in '%s'\n", header._colourMapType, filename.c_str());
            return false;
        }

        if(header._imageType != 2)
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad imageType %d, the only valid imageType is 2 : in '%s'\n", header._imageType, filename.c_str());
            return false;
        }

        if(header._colourMapDepth !=0  ||  header._colourMapLength != 0  ||  header._colourMapOrigin != 0)
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad colourMap entries, colour maps, (palettes), not supported : in '%s'\n", filename.c_str());
            return false;
        }

        if(header._originX !=0  ||  header._originY != 0)
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad origin, origin anything other thant (0, 0), not supported : in '%s'\n", filename.c_str());
            return false;
        }

        if(header._bitsPerPixel != 24  &&  header._bitsPerPixel != 32)
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad bitsPerPixel, only bitsPerPixel = 8 is supported : in '%s'\n", filename.c_str());
            return false;
        }

        if(header._width * header._height >= 0x10000)
        {
            fprintf(stderr, "Loader::loadTgaFile() : maximum width*height = 64Kbytes, width = %d, height =%d, size = %d : in '%s'\n", header._width, header._height, header._width * header._height, filename.c_str());
            return false;
        }

        if(header._width == 0  ||  header._height == 0)
        {
            fprintf(stderr, "Loader::loadTgaFile() : width and height both have to be non zero, width = %d, height =%d : in '%s'\n", header._width, header._height, filename.c_str());
            return false;
        }

        if(header._imageDescriptor & 0x0F)
        {
            fprintf(stderr, "Loader::loadTgaFile() : attribute bits per pixel not supported, attributes = %01x : in '%s'\n", (header._imageDescriptor & 0x0F), filename.c_str());
            return false;
        }

        tgaFile._imageOrigin = (header._imageDescriptor & 0x30) >>4;

        // Read image identifer
        std::vector<char> identifier;
        if(header._idLength > 0)
        {
            identifier.resize(header._idLength);
            infile.read((char *)&identifier[0], header._idLength);
        }
        
        int totalSize = header._width * header._height * (header._bitsPerPixel / 8);

        // Quick sanity check on total size, (maximum RAM is 64K)
        if(totalSize >= 0x10000 * (header._bitsPerPixel / 8))
        {
            fprintf(stderr, "Loader::loadTgaFile() : image is bigger than %d bytes : width=%d : height=%d : in '%s'\n", 0x10000 * (header._bitsPerPixel / 8), header._width, header._height, filename.c_str());
            return false;
        }

        tgaFile._header = header;
        tgaFile._data.resize(totalSize);

        // Read data
        infile.read((char *)&tgaFile._data[0], totalSize);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadTgaFile() : bad data in '%s'\n", filename.c_str());
            return false;
        }

        return true;
    }

    
    int getPixelAddress(int width, int x, int y)
    {
        return y*width + x;
    }

    uint8_t clamp8Bit(double x)
    {
        return uint8_t(fmax(fmin(x, 255.0), 0.0));
    }    

    uint8_t convertTo2Bits(uint8_t component)
    {
        return uint8_t(double(component) / 255.0 * 3.0 + 0.5) * 85;
    }

    // *NOTE* TGA and the Gigatron store RGB pixels in little endian order, so BGR in memory
    uint8_t convertRgb24ToRgb6(uint8_t red, uint8_t green, uint8_t blue)
    {
        red = convertTo2Bits(red);
        green = convertTo2Bits(green);
        blue = convertTo2Bits(blue);
        return uint8_t(((blue & 0xC0) >>2) | ((green & 0xC0) >>4) | ((red & 0xC0) >>6));
    }

    // 24bit to 6bit
    bool convertRGB8toRGB2(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin)
    {
        if(src.size() != width*height*3) return false;
        dst.resize(width*height);

        int startX = 0, endX = width, stepX = 1;
        int startY = 0, endY = height, stepY = 1;

        imageOrigin = 2;
        switch(imageOrigin)
        {
            case 0: startX = 0, endX = width, stepX = 1;     startY = height-1, endY = -1, stepY = -1; break;
            case 1: startX = width-1, endX = -1, stepX = -1; startY = height-1, endY = -1, stepY = -1; break;
            case 2: startX = 0, endX = width, stepX = 1;     startY = 0, endY = height, stepY = 1;     break;
            case 3: startX = width-1, endX = -1, stepX = -1; startY = 0, endY = height, stepY = 1;     break;
        }

        uint8_t* ptr = &dst[0];
        for(int y=startY; y!=endY; y+=stepY)
        {
            for(int x=startX; x!=endX; x+=stepX)
            {
                int indexRGB2 = getPixelAddress(width, x, y);
                int indexRGB8 = indexRGB2 * 3;
                *ptr++ = convertRgb24ToRgb6(src[indexRGB8 + 2], src[indexRGB8 + 1], src[indexRGB8 + 0]);
            }
        }

        return true;
    }

    // 32bit to 6bit, (ignoring alpha channel)
    bool convertRGBA8toRGB2(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin)
    {
        if(src.size() != width*height*4) return false;
        dst.resize(width*height);

        int startX = 0, endX = width, stepX = 1;
        int startY = 0, endY = height, stepY = 1;

        switch(imageOrigin)
        {
            case 0: startX = 0, endX = width, stepX = 1;     startY = height-1, endY = -1, stepY = -1; break;
            case 1: startX = width-1, endX = -1, stepX = -1; startY = height-1, endY = -1, stepY = -1; break;
            case 2: startX = 0, endX = width, stepX = 1;     startY = 0, endY = height, stepY = 1;     break;
            case 3: startX = width-1, endX = -1, stepX = -1; startY = 0, endY = height, stepY = 1;     break;
        }

        uint8_t* ptr = &dst[0];
        for(int y=startY; y!=endY; y+=stepY)
        {
            for(int x=startX; x!=endX; x+=stepX)
            {
                int indexRGB2 = getPixelAddress(width, x, y);
                int indexRGBA8 = indexRGB2 * 4;
                *ptr++ = convertRgb24ToRgb6(src[indexRGBA8 + 2], src[indexRGBA8 + 1], src[indexRGBA8 + 0]);
            }
        }

        return true;
    }

    uint8_t srgbToLin(uint8_t srgb)
    {
        return uint8_t(pow(double(srgb)/255.0, _gammaInput) * 255.0);
    }

    uint8_t linToSrgb(uint8_t lin)
    {
        return uint8_t(pow(double(lin)/255.0, 1.0/_gammaOutput) * 255.0);
    }

    // Floyd-Steinberg dithering 24bit to 6bit, (*NOTE* TGA and the Gigatron store RGB pixels in little endian order, so BGR in memory)
    bool ditherRGB8toRGB2(std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin)
    {
        if(src.size() != width*height*3) return false;
        dst.resize(width*height);

        int startX = 0, endX = width, stepX = 1;
        int startY = 0, endY = height, stepY = 1;

        imageOrigin = 2;
        switch(imageOrigin)
        {
            case 0: startX = 0, endX = width, stepX = 1;     startY = height-1, endY = -1, stepY = -1; break;
            case 1: startX = width-1, endX = -1, stepX = -1; startY = height-1, endY = -1, stepY = -1; break;
            case 2: startX = 0, endX = width, stepX = 1;     startY = 0, endY = height, stepY = 1;     break;
            case 3: startX = width-1, endX = -1, stepX = -1; startY = 0, endY = height, stepY = 1;     break;
        }

        for(int y=startY; y!=endY; y+=stepY)
        {
            for(int x=startX; x!=endX; x+=stepX)
            {
                int index = getPixelAddress(width, x, y)*3;

                src[index + 2] = srgbToLin(int(src[index + 2]));
                src[index + 1] = srgbToLin(int(src[index + 1]));
                src[index + 0] = srgbToLin(int(src[index + 0]));
            }
        }

        for(int y=startY; y!=endY; y+=stepY)
        {
            for(int x=startX; x!=endX; x+=stepX)
            {
                int index = getPixelAddress(width, x, y)*3;

                int oldRed = src[index + 2];
                int oldGrn = src[index + 1];
                int oldBlu = src[index + 0];
                double nrmRed0 = double(oldRed) / 255.0;
                double nrmGrn0 = double(oldGrn) / 255.0;
                double nrmBlu0 = double(oldBlu) / 255.0;

                int newRed = convertTo2Bits(oldRed);
                int newGrn = convertTo2Bits(oldGrn);
                int newBlu = convertTo2Bits(oldBlu);
                double nrmRed1 = double(newRed) / 255.0;
                double nrmGrn1 = double(newGrn) / 255.0;
                double nrmBlu1 = double(newBlu) / 255.0;

                double oldLum;
                double newLum;
                bool useLumError = true;
                switch(_diffusionType)
                {
                    case LumError0:
                    {
                        oldLum = (nrmRed0 + nrmGrn0 + nrmBlu0) * 0.33333333333;
                        newLum = (nrmRed1 + nrmGrn1 + nrmBlu1) * 0.33333333333;
                    }
                    break;
                
                    case LumError1:
                    {
                        oldLum = 0.299*nrmRed0 + 0.587*nrmGrn0 + 0.114*nrmBlu0;
                        newLum = 0.299*nrmRed1 + 0.587*nrmGrn1 + 0.114*nrmBlu1;
                    }
                    break;

                    case LumError2:
                    {
                        oldLum = sqrt(0.299*(nrmRed0*nrmRed0) + 0.587*(nrmGrn0*nrmGrn0) + 0.114*(nrmBlu0*nrmBlu0));
                        newLum = sqrt(0.299*(nrmRed1*nrmRed1) + 0.587*(nrmGrn1*nrmGrn1) + 0.114*(nrmBlu1*nrmBlu1));
                    }
                    break;

                    case LumError3:
                    {
                        oldLum = sqrt(0.33333333333*(nrmRed0*nrmRed0) + 0.33333333333*(nrmGrn0*nrmGrn0) + 0.33333333333*(nrmBlu0*nrmBlu0));
                        newLum = sqrt(0.33333333333*(nrmRed1*nrmRed1) + 0.33333333333*(nrmGrn1*nrmGrn1) + 0.33333333333*(nrmBlu1*nrmBlu1));
                    }
                    break;

                    case RgbError:
                    {
                        useLumError = false;
                    }
                    break;
                }

                double errorScale = (_diffusionScale > 128.0) ? 0.0 : 1.0;
                double errRed = (useLumError) ? double(oldLum - newLum) * 255.0 * errorScale : double(oldRed - newRed) * errorScale;
                double errGrn = (useLumError) ? double(oldLum - newLum) * 255.0 * errorScale : double(oldGrn - newGrn) * errorScale;
                double errBlu = (useLumError) ? double(oldLum - newLum) * 255.0 * errorScale : double(oldBlu - newBlu) * errorScale;

                src[index + 2] = linToSrgb(newRed);
                src[index + 1] = linToSrgb(newGrn);
                src[index + 0] = linToSrgb(newBlu);
               
                if(x + 1 < width)
                {
                    src[getPixelAddress(width, x+1, y)*3 + 2] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y)*3 + 2])  +  7.0*errRed/_diffusionScale));
                    src[getPixelAddress(width, x+1, y)*3 + 1] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y)*3 + 1])  +  7.0*errGrn/_diffusionScale));
                    src[getPixelAddress(width, x+1, y)*3 + 0] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y)*3 + 0])  +  7.0*errBlu/_diffusionScale));
                }

                if(x - 1 >= 0  &&  y + 1 < height)
                {
                    src[getPixelAddress(width, x-1, y+1)*3 + 2] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x-1, y+1)*3 + 2])  +  3.0*errRed/_diffusionScale));
                    src[getPixelAddress(width, x-1, y+1)*3 + 1] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x-1, y+1)*3 + 1])  +  3.0*errGrn/_diffusionScale));
                    src[getPixelAddress(width, x-1, y+1)*3 + 0] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x-1, y+1)*3 + 0])  +  3.0*errBlu/_diffusionScale));
                }

                if(y + 1 < height)
                {
                    src[getPixelAddress(width, x, y+1)*3 + 2] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x, y+1)*3 + 2])  +  5.0*errRed/_diffusionScale));
                    src[getPixelAddress(width, x, y+1)*3 + 1] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x, y+1)*3 + 1])  +  5.0*errGrn/_diffusionScale));
                    src[getPixelAddress(width, x, y+1)*3 + 0] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x, y+1)*3 + 0])  +  5.0*errBlu/_diffusionScale));
                }

                if(x + 1 < width  &&  y + 1 < height)
                {
                    src[getPixelAddress(width, x+1, y+1)*3 + 2] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y+1)*3 + 2])  +  1.0*errRed/_diffusionScale));
                    src[getPixelAddress(width, x+1, y+1)*3 + 1] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y+1)*3 + 1])  +  1.0*errGrn/_diffusionScale));
                    src[getPixelAddress(width, x+1, y+1)*3 + 0] = linToSrgb(clamp8Bit(double(src[getPixelAddress(width, x+1, y+1)*3 + 0])  +  1.0*errBlu/_diffusionScale));
               }
            }
        }

        convertRGB8toRGB2(src, dst, width, height, imageOrigin);

        return true;
    }

    // Floyd-Steinberg dithering 32bit to 6bit, (ignoring alpha channel)
    bool ditherRGBA8toRGB2(std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t imageOrigin)
    {
        if(src.size() != width*height*4) return false;
        dst.resize(width*height);

        int startX = 0, endX = width, stepX = 1;
        int startY = 0, endY = height, stepY = 1;

        switch(imageOrigin)
        {
            case 0: startX = 0, endX = width, stepX = 1;     startY = height-1, endY = -1, stepY = -1; break;
            case 1: startX = width-1, endX = -1, stepX = -1; startY = height-1, endY = -1, stepY = -1; break;
            case 2: startX = 0, endX = width, stepX = 1;     startY = 0, endY = height, stepY = 1;     break;
            case 3: startX = width-1, endX = -1, stepX = -1; startY = 0, endY = height, stepY = 1;     break;
        }

        uint8_t* ptr = &dst[0];
        for(int y=startY; y!=endY; y+=stepY)
        {
            for(int x=startX; x!=endX; x+=stepX)
            {
            }
        }

        return true;
    }


    void handleImageInput(void)
    {
    }

    void process(void)
    {
        handleImageInput();
        Graphics::render(true);
    }
}
