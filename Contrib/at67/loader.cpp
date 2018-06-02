#include <stdint.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <algorithm>

#ifndef STAND_ALONE
#include "cpu.h"
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "inih/INIReader.h"
#endif

#include "loader.h"
#include "assembler.h"
#include "expression.h"


namespace Loader
{
#ifndef STAND_ALONE
    enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
    enum FrameState {Resync=0, Frame, Execute, NumFrameStates};


    bool _startUploading = false;
    bool _disableUploads = false;

    std::string _currentGame = "";

    INIReader _iniReader;
    std::map<std::string, SaveData> _saveData;


    bool getStartUploading(void) {return _startUploading;}
    void setStartUploading(bool start) {_startUploading = start;}


    void initialise(void)
    {
        INIReader iniReader(HIGH_SCORES_INI);
        _iniReader = iniReader;
        if(_iniReader.ParseError() < 0)
        {
            fprintf(stderr, "Loader::initialise() : couldn't load INI file '%s' : loading and saving of high scores is disabled.\n", HIGH_SCORES_INI);
            return;
        }

        // Parse high scores INI file
        for(auto game : _iniReader.Sections())
        {
            std::vector<uint16_t> counts;
            std::vector<uint16_t> addresses;
            std::vector<Endianness> endianness;
            std::vector<std::vector<uint8_t>> data;

            int updateRate = uint16_t(_iniReader.GetReal(game, "updateRate", VSYNC_RATE));

            for(int index=0; ; index++)
            {
                std::string count = "count" + std::to_string(index);
                std::string address = "address" + std::to_string(index);
                std::string endian = "endian" + std::to_string(index);
                if(_iniReader.Get(game, count, "") == "") break;
                if(_iniReader.Get(game, address, "") == "") break;
                endian = _iniReader.Get(game, endian, "little");
                counts.push_back(uint16_t(_iniReader.GetReal(game, count, -1)));
                addresses.push_back(uint16_t(_iniReader.GetReal(game, address, -1)));
                endianness.push_back((endian == "little") ? Little : Big);
                data.push_back(std::vector<uint8_t>(counts.back(), 0x00));
            }

            SaveData saveData = {true, updateRate, game, counts, addresses, endianness, data};
            _saveData[game] = saveData;
        }
    }
#endif

    bool loadGt1File(const std::string& filename, Gt1File& gt1File)
    {
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadGt1File() : failed to open '%s'.\n", filename.c_str());
            return false;
        }

        int segmentCount = 1;
        for(;;)
        {
            // Read segment header
            Gt1Segment segment;
            infile.read((char *)&segment._hiAddress, SEGMENT_HEADER_SIZE);
            if(infile.eof() || infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadGt1File() : bad header in segment %d of '%s'.\n", segmentCount, filename.c_str());
                return false;
            }

            // Finished
            if(segment._hiAddress == 0x00  &&  infile.peek() == EOF)
            {
                // Segment header aligns with Gt1File terminator, hiStart and loStart
                gt1File._hiStart = segment._loAddress;
                gt1File._loStart = segment._segmentSize;
                break;
            }

            // Expecting a valid segment
            if(segment._segmentSize == 0)
            {
                fprintf(stderr, "Loader::loadGt1File() : bad header in segment %d of '%s'.\n", segmentCount, filename.c_str());
                return false;
            }

            // Read segment
            segment._dataBytes.resize(segment._segmentSize);
            infile.read((char *)&segment._dataBytes[0], segment._segmentSize);
            if(infile.eof() || infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadGt1File() : bad segment %d in '%s'.\n", segmentCount, filename.c_str());
                return false;
            }

            gt1File._segments.push_back(segment);
            segmentCount++;
        }

        return true;
    }

    bool saveGt1File(const std::string& filepath, Gt1File& gt1File, std::string& filename)
    {
        if(gt1File._segments.size() == 0)
        {
            fprintf(stderr, "Loader::saveGt1File() : zero segments, not saving.\n");
            return false;
        }

        size_t i = filepath.rfind('.');
        filename = (i != std::string::npos) ? filepath.substr(0, i) + ".gt1" : filepath + ".gt1";

        std::ofstream outfile(filename, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Loader::saveGt1File() : failed to open '%s'.\n", filename.c_str());
            return false;
        }

        // Sort segments from lowest address to highest address
        std::sort(gt1File._segments.begin(), gt1File._segments.end(), [](const Gt1Segment& segmentA, const Gt1Segment& segmentB)
        {
            uint16_t addressA = segmentA._loAddress + (segmentA._hiAddress <<8);
            uint16_t addressB = segmentB._loAddress + (segmentB._hiAddress <<8);
            return (addressA < addressB);
        });

        for(int i=0; i<gt1File._segments.size(); i++)
        {
            // Write header
            outfile.write((char *)&gt1File._segments[i]._hiAddress, SEGMENT_HEADER_SIZE);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveGt1File() : write error in header of segment %d.\n", i);
                return false;
            }

            // Write segment
            outfile.write((char *)&gt1File._segments[i]._dataBytes[0], gt1File._segments[i]._segmentSize);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveGt1File() : bad segment %d in '%s'.\n", i, filename.c_str());
                return false;
            }
        }

        // Write trailer
        outfile.write((char *)&gt1File._terminator, GT1FILE_TRAILER_SIZE);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveGt1File() : write error in trailer of '%s'.\n", filename.c_str());
            return false;
        }

        return true;
    }

    void printGt1Stats(const std::string& filename, const Gt1File& gt1File)
    {
        // Header
        uint16_t totalSize = 0;
        for(int i=0; i<gt1File._segments.size(); i++) totalSize += int(gt1File._segments[i]._dataBytes.size());
        uint16_t startAddress = gt1File._loStart + (gt1File._hiStart <<8);
        fprintf(stderr, "\n********************************************************************************\n");
        fprintf(stderr, "%s : 0x%04x : %5d bytes : %3d segments\n", filename.c_str(), startAddress, totalSize, int(gt1File._segments.size()));
        fprintf(stderr, "********************************************************************************\n");

        // Segments
        for(int i=0; i<gt1File._segments.size(); i++)
        {
            uint16_t address = gt1File._segments[i]._loAddress + (gt1File._segments[i]._hiAddress <<8);
            std::string memory = (gt1File._segments[i]._isRomAddress) ? "ROM" : "RAM";
            fprintf(stderr, "Segment%03d : %s 0x%04x : %5d bytes\n", i, memory.c_str(),address, int(gt1File._segments[i]._dataBytes.size()));
        }
    }

#ifndef STAND_ALONE
    void disableUploads(bool disable)
    {
        _disableUploads = disable;
    }

    bool loadDataFile(SaveData& saveData)
    {
        SaveData sdata = saveData;
        std::string filename = sdata._filename + ".dat";
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadDataFile() : failed to open '%s'.\n", filename.c_str());
            return false;
        }

        // TODO: endian
        // Load counts
        uint16_t numCounts = 0;
        infile.read((char *)&numCounts, 2);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadDataFile() : read error in number of counts in '%s'.\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numCounts; i++)
        {
            uint16_t count;
            infile.read((char *)&count, 2);
            if(infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadDataFile() : read error in counts of '%s'.\n", filename.c_str());
                return false;
            }
            sdata._counts[i] = count;
        }         

        // TODO: endian
        // Load addresses
        uint16_t numAddresses = 0;
        infile.read((char *)&numAddresses, 2);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadDataFile() : read error in number of addresses in '%s'.\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numAddresses; i++)
        {
            uint16_t address;
            infile.read((char *)&address, 2);
            if(infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadDataFile() : read error in addresses of '%s'.\n", filename.c_str());
                return false;
            }
            sdata._addresses[i] = address;
        }         

        if(sdata._counts.size() == 0  ||  sdata._counts.size() != sdata._addresses.size())
        {
            fprintf(stderr, "Loader::loadDataFile() : save data is corrupt : saveData._counts.size() = %d : saveData._addresses.size() = %d.\n", int(sdata._counts.size()), int(sdata._addresses.size()));
            return false;
        }

        // load data
        for(int j=0; j<sdata._addresses.size(); j++)
        {
            sdata._data.push_back(std::vector<uint8_t>(sdata._counts[j], 0x00));
            for(int i=0; i<sdata._counts[j]; i++)
            {
                uint8_t data;
                infile.read((char *)&data, 1);
                if(infile.bad() || infile.fail())
                {
                    fprintf(stderr, "Loader::loadDataFile() : read error in data of '%s'.\n", filename.c_str());
                    return false;
                }
                sdata._data[j][i] = data;
                Cpu::setRAM(sdata._addresses[j] + i, data);
            }
        }
        sdata._initialised = true;

        saveData = sdata;

        return true;
    }

    // Only for emulation
    bool saveDataFile(const SaveData& saveData)
    {
        std::string filename = saveData._filename + ".dat";
        std::ofstream outfile(filename, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Loader::saveDataFile() : failed to open '%s'.\n", filename.c_str());
            return false;
        }

        if(saveData._counts.size() == 0  ||  saveData._counts.size() != saveData._addresses.size())
        {
            fprintf(stderr, "Loader::saveDataFile() : save data is corrupt : saveData._counts.size() = %d : saveData._addresses.size() = %d.\n", int(saveData._counts.size()), int(saveData._addresses.size()));
            return false;
        }

        // TODO: endian
        // Save counts
        uint16_t numCounts = uint16_t(saveData._counts.size());
        outfile.write((char *)&numCounts, 2);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveDataFile() : write error in number of counts of '%s'.\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numCounts; i++)
        {
            outfile.write((char *)&saveData._counts[i], 2);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveDataFile() : write error in counts of '%s'.\n", filename.c_str());
                return false;
            }
        }         

        // Save addresses
        uint16_t numAddresses = uint16_t(saveData._addresses.size());
        outfile.write((char *)&numAddresses, 2);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveDataFile() : write error in number of addresses of '%s'.\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numAddresses; i++)
        {
            outfile.write((char *)&saveData._addresses[i], 2);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveDataFile() : write error in addresses of '%s'.\n", filename.c_str());
                return false;
            }
        }         

        // Check data has been initialised
        for(int j=0; j<saveData._addresses.size(); j++)
        {
            if(saveData._data.size() != saveData._addresses.size())
            {
                fprintf(stderr, "Loader::saveDataFile() : data has not been initialised or loaded, nothing to save for '%s'.\n", filename.c_str());
                return false;
            }
            for(int i=0; i<saveData._counts[j]; i++)
            {
                if(saveData._data[j].size() != saveData._counts[j]) 
                {
                    fprintf(stderr, "Loader::saveDataFile() : data has not been initialised or loaded, nothing to save for '%s'.\n", filename.c_str());
                    return false;
                }
            }
        }

        // Save data
        for(int j=0; j<saveData._addresses.size(); j++)
        {
            for(int i=0; i<saveData._counts[j]; i++)
            {
                uint8_t data = saveData._data[j][i];
                outfile.write((char *)&data, 1);
                if(outfile.bad() || outfile.fail())
                {
                    fprintf(stderr, "Loader::saveDataFile() : write error in data of '%s'.\n", filename.c_str());
                    return false;
                }
            }
        }

        return true;
    }

    // Loads high score for current game from a simple <game>.dat file
    void loadHighScore(void)
    {
        if(_saveData.find(_currentGame) == _saveData.end())
        {
            //fprintf(stderr, "Loader::loadHighScore() : warning, no game entry defined in '%s' for '%s'.\n", HIGH_SCORES_INI, _currentGame.c_str());
            return;
        }

        if(Loader::loadDataFile(_saveData[_currentGame]))
        {
            fprintf(stderr, "Loader::loadHighScore() : loaded high score data successfully for '%s'.\n", _currentGame.c_str());
        }
    }

    // Saves high score for current game to a simple <game>.dat file
    void saveHighScore(void)
    {
        if(_saveData.find(_currentGame) == _saveData.end())
        {
            fprintf(stderr, "Loader::saveHighScore() : error, no game entry defined in '%s' for '%s'.\n", HIGH_SCORES_INI, _currentGame.c_str());
            return;
        }

        if(Loader::saveDataFile(_saveData[_currentGame]))
        {
            fprintf(stderr, "Loader::saveHighScore() : saved high score data successfully for '%s'.\n", _currentGame.c_str());
        }
    }

    // Updates a game's high score, (call this in the vertical blank)
    void updateHighScore(void)
    {
        static int frameCount = 0;

        // No entry in high score file defined for this game, so silently exit
        if(_saveData.find(_currentGame) == _saveData.end()) return;
        if(!_saveData[_currentGame]._initialised) return;

        // Update once every updateRate VBlank ticks, (defaults to VSYNC_RATE, hence once every second)
        if(frameCount++ < _saveData[_currentGame]._updaterate) return;
        frameCount = 0;

        // Update data, (checks byte by byte and saves if larger, endian order is configurable)
        bool save = false;
        for(int j=0; j<_saveData[_currentGame]._addresses.size(); j++)
        {
            // Defaults to little endian
            int start = _saveData[_currentGame]._counts[j] - 1, end = -1, step = -1;
            if(_saveData[_currentGame]._endianness[j] == Big)
            {
                start = 0;
                end = _saveData[_currentGame]._counts[j];
                step = 1;
            }

            // Loop MSB to LSB or vice versa depending on endianness            
            while(start != end)
            {
                int i = start;
                uint8_t data = Cpu::getRAM(_saveData[_currentGame]._addresses[j] + i);

                // TODO: create a list of INI rules to make this test more flexible
                if(data < _saveData[_currentGame]._data[j][i]) return;
                if(_saveData[_currentGame]._data[j][i] == 0  ||  data > _saveData[_currentGame]._data[j][i])
                {
                    for(int k=i; k!=end; k+=step)
                    {
                        _saveData[_currentGame]._data[j][k] = Cpu::getRAM(_saveData[_currentGame]._addresses[j] + k);
                    }
                    saveHighScore();
                    return;
                }

                start += step;
            }
        }
    }

    void uploadDirect(void)
    {
        bool hasRomCode = false;
        bool hasRamCode = false;

        uint16_t executeAddress = Editor::getLoadBaseAddress();
        std::string filename = *Editor::getFileEntryName(Editor::getCursorY());
        std::string filepath = std::string(Editor::getBrowserPath() + "/" + filename);

        // Reset video table and reset single step watch address to video line counter
        Graphics::resetVTable();
        Editor::setSingleStepWatchAddress(VIDEO_Y_ADDRESS);

        // Upload gt1
        Gt1File gt1File;
        if(filename.find(".gt1") != filename.npos)
        {
            Assembler::clearAssembler();

            if(!loadGt1File(filepath, gt1File)) return;
            executeAddress = gt1File._loStart + (gt1File._hiStart <<8);
            Editor::setLoadBaseAddress(executeAddress);

            for(int j=0; j<gt1File._segments.size(); j++)
            {
                uint16_t address = gt1File._segments[j]._loAddress + (gt1File._segments[j]._hiAddress <<8);
                for(int i=0; i<gt1File._segments[j]._segmentSize; i++)
                {
                    Cpu::setRAM(address+i, gt1File._segments[j]._dataBytes[i]);
                }
            }

            hasRamCode = true;
            hasRomCode = false;
            _disableUploads = false;
        }
        // Upload vCPU assembly code
        else if(filename.find(".vasm") != filename.npos  ||  filename.find(".s") != filename.npos  ||  filename.find(".asm") != filename.npos)
        {
            if(!Assembler::assemble(filepath, DEFAULT_START_ADDRESS)) return;
            executeAddress = Assembler::getStartAddress();
            Editor::setLoadBaseAddress(executeAddress);
            uint16_t address = executeAddress;
            uint16_t customAddress = executeAddress;

            // Save to gt1 format
            gt1File._loStart = address & 0x00FF;
            gt1File._hiStart = (address & 0xFF00) >>8;
            Gt1Segment gt1Segment;
            gt1Segment._loAddress = address & 0x00FF;
            gt1Segment._hiAddress = (address & 0xFF00) >>8;

            Assembler::ByteCode byteCode;
            while(!Assembler::getNextAssembledByte(byteCode))
            {
                (byteCode._isRomAddress) ? hasRomCode = true : hasRamCode = true;

                // Custom address
                if(byteCode._isCustomAddress)
                {
                    if(gt1Segment._dataBytes.size())
                    {
                        // Previous segment
                        gt1Segment._segmentSize = uint8_t(gt1Segment._dataBytes.size());
                        gt1File._segments.push_back(gt1Segment);
                        gt1Segment._dataBytes.clear();
                    }

                    address = byteCode._address;
                    customAddress = address;
                    gt1Segment._isRomAddress = byteCode._isRomAddress;
                    gt1Segment._loAddress = address & 0x00FF;
                    gt1Segment._hiAddress = (address & 0xFF00) >>8;
                }

                if(!_disableUploads)
                {
                    (byteCode._isRomAddress) ? Cpu::setROM(customAddress, address++, byteCode._data) : Cpu::setRAM(address++, byteCode._data);
                }
                gt1Segment._dataBytes.push_back(byteCode._data);
            }

            // Last segment
            if(gt1Segment._dataBytes.size())
            {
                gt1Segment._segmentSize = uint8_t(gt1Segment._dataBytes.size());
                gt1File._segments.push_back(gt1Segment);
            }

            // Don't save gt1 file for any asm files that contain native rom code
            std::string gt1FileName;
            if(!hasRomCode  &&  !saveGt1File(filepath, gt1File, gt1FileName)) return;
        }
        // Invalid file
        else
        {
            fprintf(stderr, "Loader::upload() : invalid file '%s'.\n", filename.c_str());
            return;
        }

        printGt1Stats(filename, gt1File);

        // Currently only for emulation
        size_t i = filename.find('.');
        _currentGame = (i != std::string::npos) ? filename.substr(0, i) : filename;
        loadHighScore();

        // Execute code
        if(!_disableUploads  &&  hasRamCode)
        {
            Cpu::setRAM(0x0016, executeAddress-2 & 0x00FF);
            Cpu::setRAM(0x0017, (executeAddress & 0xFF00) >>8);
            Cpu::setRAM(0x001a, executeAddress-2 & 0x00FF);
            Cpu::setRAM(0x001b, (executeAddress & 0xFF00) >>8);
            //Editor::setSingleStep(true);
        }
        return;
    }

    void sendByte(uint8_t value, uint8_t& checksum)
    {
        Cpu::setIN(value);
        checksum += value;
    }

    bool sendFrame(int vgaY, uint8_t firstByte, uint8_t* message, uint8_t len, uint16_t address, uint8_t& checksum)
    {
        static LoaderState loaderState = LoaderState::FirstByte;
        static uint8_t payload[PAYLOAD_SIZE];

        bool sending = true;

        switch(loaderState)
        {
            case LoaderState::FirstByte: // 8 bits
            {
                if(vgaY == VSYNC_START+8)
                {
                    for(int i=0; i<len; ++i) payload[i % PAYLOAD_SIZE] = message[i % PAYLOAD_SIZE];
                    sendByte(firstByte, checksum);
                    checksum += firstByte << 6;
                    loaderState = LoaderState::MsgLength;
                }
            }
            break;

            case LoaderState::MsgLength: // 6 bits
            {
                if(vgaY == VSYNC_START+14)
                {
                    sendByte(len, checksum);
                    loaderState = LoaderState::LowAddress;
                }
            }
            break;

            case LoaderState::LowAddress: // 8 bits
            {
                if(vgaY == VSYNC_START+22)
                {
                    sendByte(address & 0x00FF, checksum);
                    loaderState = LoaderState::HighAddress;
                }
            }
            break;

            case LoaderState::HighAddress: // 8 bits
            {
                if(vgaY == VSYNC_START+30)
                {
                    sendByte(address >> 8, checksum);
                    loaderState = LoaderState::Message;
                }
            }
            break;

            case LoaderState::Message: // 8*PAYLOAD_SIZE bits
            {
                static int msgIdx = 0;
                if(vgaY == VSYNC_START+38+msgIdx*8)
                {
                    sendByte(payload[msgIdx], checksum);
                    if(++msgIdx == PAYLOAD_SIZE)
                    {
                        msgIdx = 0;
                        loaderState = LoaderState::LastByte;
                    }
                }
            }
            break;

            case LoaderState::LastByte: // 8 bits
            {
                if(vgaY == VSYNC_START+38+PAYLOAD_SIZE*8)
                {
                    uint8_t lastByte = -checksum;
                    sendByte(lastByte, checksum);
                    checksum = lastByte;
                    loaderState = LoaderState::ResetIN;
                }
            }
            break;

            case LoaderState::ResetIN:
            {
                if(vgaY == VSYNC_START+39+PAYLOAD_SIZE*8)
                {
                    Cpu::setIN(0xFF);
                    loaderState = LoaderState::FirstByte;
                    sending = false;
                }
            }
            break;
        }

        return sending;
    }

    // TODO: fix the Gigatron version of upload so that it can send more than 60 total bytes, (i.e. break up the payload into multiple packets of 60, 1 packet per frame)
    void upload(int vgaY)
    {
        static bool frameUploading = false;
        static FILE* fileToUpload = NULL;
        static FILE* fileToSave = NULL;
        static uint8_t payload[RAM_SIZE];
        static uint8_t payloadSize = 0;

        if(_startUploading  ||  frameUploading)
        {
            uint16_t executeAddress = Editor::getLoadBaseAddress();
            if(_startUploading)
            {
                _startUploading = false;
                //frameUploading = true;

                uploadDirect();
                return;
            }
            
            static uint8_t checksum = 0;
            static FrameState frameState = FrameState::Resync;

            switch(frameState)
            {
                case FrameState::Resync:
                {
                    if(!sendFrame(vgaY, -1, payload, payloadSize, executeAddress, checksum))
                    {
                        checksum = 'g'; // loader resets checksum
                        frameState = FrameState::Frame;
                    }
                }
                break;

                case FrameState::Frame:
                {
                    if(!sendFrame(vgaY,'L', payload, payloadSize, executeAddress, checksum))
                    {
                        frameState = FrameState::Execute;
                    }
                }
                break;

                case FrameState::Execute:
                {
                    if(!sendFrame(vgaY, 'L', payload, 0, executeAddress, checksum))
                    {
                        checksum = 0;
                        frameState = FrameState::Resync;
                        frameUploading = false;
                    }
                }
                break;
            }
        }
    }
#endif
}