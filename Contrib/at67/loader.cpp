#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <algorithm>

#ifndef STAND_ALONE
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "inih/INIReader.h"
#include "rs232/rs232.h"

#if defined(_WIN32)
#include <direct.h>
#include "dirent/dirent.h"
#define chdir _chdir
#else
#include <dirent.h>
#endif

#endif

#include "cpu.h"
#include "loader.h"
#include "assembler.h"
#include "expression.h"


#define DEFAULT_COM_BAUD_RATE 115200
#define DEFAULT_COM_PORT      0
#define DEFAULT_GIGA_TIMEOUT  5.0
#define MAX_GT1_SIZE          (1 <<16)


namespace Loader
{
#ifndef STAND_ALONE
    enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
    enum FrameState {Resync=0, Frame, Execute, NumFrameStates};


    UploadTarget _uploadTarget = None;
    bool _disableUploads = false;

    int _numComPorts = 0;
    int _currentComPort = -1;
    char _gt1Buffer[MAX_GT1_SIZE];

    int _configBaudRate = DEFAULT_COM_BAUD_RATE;
    int _configComPort = DEFAULT_COM_PORT;
    double _configTimeout = DEFAULT_GIGA_TIMEOUT;
    std::string _configGclBuild = ".";
    bool _configGclBuildFound = false;

    std::string _currentGame = "";

    INIReader _loaderConfigIniReader;
    INIReader _highScoresIniReader;
    std::map<std::string, SaveData> _saveData;


    UploadTarget getUploadTarget(void) {return _uploadTarget;}
    void setUploadTarget(UploadTarget target) {_uploadTarget = target;}


    bool getKeyAsString(INIReader& iniReader, const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, std::string& result, bool upperCase=true)
    {
        result = iniReader.Get(sectionString, iniKey, defaultKey);
        if(result == defaultKey) return false;
        if(upperCase) result = Expression::strToUpper(result);
        return true;
    }

    void initialise(void)
    {
        _numComPorts = comEnumerate();
        if(_numComPorts == 0) fprintf(stderr, "Loader::initialise() : no COM ports found.\n");

        // Loader config
        INIReader loaderConfigIniReader(LOADER_CONFIG_INI);
        _loaderConfigIniReader = loaderConfigIniReader;
        if(_loaderConfigIniReader.ParseError() == 0)
        {
            // Parse Loader Keys
            enum Section {Comms};
            std::map<std::string, Section> section;
            section["Comms"] = Comms;
            for(auto sectionString : _loaderConfigIniReader.Sections())
            {
                if(section.find(sectionString) == section.end())
                {
                    fprintf(stderr, "Loader::initialise() : INI file '%s' has bad Sections : reverting to default values.\n", LOADER_CONFIG_INI);
                    break;
                }

                std::string result;
                switch(section[sectionString])
                {
                    case Comms:
                    {
                         getKeyAsString(_loaderConfigIniReader, sectionString, "BaudRate", "115200", result);   
                        _configBaudRate = strtol(result.c_str(), nullptr, 10);
 
                        char *endPtr;
                        getKeyAsString(_loaderConfigIniReader, sectionString, "ComPort", "0", result);   
                        _configComPort = strtol(result.c_str(), &endPtr, 10);
                        if((endPtr - &result[0]) != result.size())
                        {
                            _configComPort = comFindPort(result.c_str());
                            if(_configComPort < 0) _configComPort = DEFAULT_COM_PORT;
                        }

                        getKeyAsString(_loaderConfigIniReader, sectionString, "Timeout", "5.0", result);   
                        _configTimeout = strtod(result.c_str(), nullptr);

                        _configGclBuildFound = getKeyAsString(_loaderConfigIniReader, sectionString, "GclBuild", ".", result, false);   
                        _configGclBuild = result;
                    }
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "Loader::initialise() : couldn't find loader configuration INI file '%s' : reverting to default values.\n", LOADER_CONFIG_INI);
        }


        // High score config
        INIReader highScoresIniReader(HIGH_SCORES_INI);
        _highScoresIniReader = highScoresIniReader;
        if(_highScoresIniReader.ParseError() == 0)
        {
            // Parse high scores INI file
            for(auto game : _highScoresIniReader.Sections())
            {
                std::vector<uint16_t> counts;
                std::vector<uint16_t> addresses;
                std::vector<Endianness> endianness;
                std::vector<std::vector<uint8_t>> data;

                int updateRate = uint16_t(_highScoresIniReader.GetReal(game, "updateRate", VSYNC_RATE));

                for(int index=0; ; index++)
                {
                    std::string count = "count" + std::to_string(index);
                    std::string address = "address" + std::to_string(index);
                    std::string endian = "endian" + std::to_string(index);
                    if(_highScoresIniReader.Get(game, count, "") == "") break;
                    if(_highScoresIniReader.Get(game, address, "") == "") break;
                    endian = _highScoresIniReader.Get(game, endian, "little");
                    counts.push_back(uint16_t(_highScoresIniReader.GetReal(game, count, -1)));
                    addresses.push_back(uint16_t(_highScoresIniReader.GetReal(game, address, -1)));
                    endianness.push_back((endian == "little") ? Little : Big);
                    data.push_back(std::vector<uint8_t>(counts.back(), 0x00));
                }

                SaveData saveData = {true, updateRate, game, counts, addresses, endianness, data};
                _saveData[game] = saveData;
            }
        }
        else
        {
            fprintf(stderr, "Loader::initialise() : couldn't load high scores INI file '%s' : loading and saving of high scores is disabled.\n", HIGH_SCORES_INI);
        }
    }

    int matchFileSystemName(const std::string& path, const std::string& match, std::vector<std::string>& names)
    {
        DIR *dir;
        struct dirent *ent;

        names.clear();

        if((dir = opendir(path.c_str())) != NULL)
        {
            while((ent = readdir(dir)) != NULL)
            {
                std::string name = std::string(ent->d_name);
                if(name.find(match) != std::string::npos) names.push_back(path + name);   
            }
            closedir (dir);
        }

        return int(names.size());
    }

    bool openComPort(int comPort)
    {
        if(_numComPorts == 0)
        {
            _numComPorts = comEnumerate();
            if(_numComPorts == 0)
            {
                fprintf(stderr, "Loader::openComPort() : no COM ports found.\n");
                return false;
            }
        }

        _currentComPort = comPort;

#ifdef _WIN32
        if(_currentComPort == -1) _currentComPort = 0;
#else
        if(_currentComPort == -1)
        {
            _currentComPort = 0;
            std::vector<std::string> names;
            matchFileSystemName("/dev/", "tty.usbmodem", names);
            if(names.size() == 0) matchFileSystemName("/dev/", "ttyACM", names);
            if(names.size()) _currentComPort = comFindPort(names[0].c_str());
        }
#endif        

        if(_currentComPort < 0)
        {
            _numComPorts = 0;
            fprintf(stderr, "Loader::openComPort() : couldn't open any COM port.\n");
            return false;
        } 
        else if(comOpen(_currentComPort, _configBaudRate) == 0)
        {
            _numComPorts = 0;
            fprintf(stderr, "Loader::openComPort() : couldn't open COM port '%s'.\n", comGetPortName(_currentComPort));
            return false;
        } 

        return true;
    }

    void closeComPort(void)
    {
        comClose(_currentComPort);
    }

    bool readLineGiga(std::string& line)
    {
        line.clear();
        char buffer = 0;
        uint64_t prevFrameCounter = SDL_GetPerformanceCounter();

        while(buffer != '\n')
        {
            if(comRead(_currentComPort, &buffer, 1)) line.push_back(buffer);
            double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
            if(frameTime > _configTimeout) return false;
        }

        // Replace '\n'
        line.back() = 0;

        return true;
    }

    bool waitForPromptGiga(std::string& line)
    {
        do
        {
            if(!readLineGiga(line))
            {
                fprintf(stderr, "Loader::waitForPromptGiga() : timed out on serial port : '%s'.\n", comGetPortName(_currentComPort));
                return false;
            }

            if(size_t e = line.find('!') != std::string::npos)
            {
                fprintf(stderr, "Loader::waitForPromptGiga() : Arduino Error : '%s'.\n", &line[e]);
                return false;
            }
        }
        while(line.find('?') == std::string::npos);

        return true;
    }
    

    void sendCommandGiga(char cmd, std::string& line, bool wait)
    {
        char command[2] = {cmd, '\n'};
        comWrite(_currentComPort, command, 2);

        // Wait for ready prompt
        if(wait) waitForPromptGiga(line);
    }

    void sendCommandToGiga(char cmd, bool wait)
    {
        if(!openComPort(_configComPort)) return;

        std::string line;
        sendCommandGiga(cmd, line, false);

        closeComPort();
    }

    void uploadToGiga(const std::string& filename)
    {
        if(!openComPort(_configComPort)) return;

        std::ifstream gt1file(filename, std::ios::binary | std::ios::in);
        if(!gt1file.is_open())
        {
            fprintf(stderr, "Loader::uploadToGiga() : failed to open '%s'.\n", filename.c_str());
            return;
        }

        gt1file.read(_gt1Buffer, MAX_GT1_SIZE);
        if(gt1file.bad())
        {
            fprintf(stderr, "Loader::uploadToGiga() : failed to read %s GT1 file.\n", filename.c_str());
            return;
        }

        std::string line;
        sendCommandGiga('R', line, true);
        sendCommandGiga('L', line, true);
        sendCommandGiga('U', line, true);

        int index = 0;
        while(std::isdigit(line[0]))
        {
            int n = strtol(line.c_str(), nullptr, 10);
            comWrite(_currentComPort, &_gt1Buffer[index], n);
            index += n;

            if(!waitForPromptGiga(line))
            {
                closeComPort();
                return;
            }

            float upload = float(index) / float(gt1file.gcount());
            Graphics::drawUploadBar(upload);
            fprintf(stderr, "Loader::uploadToGiga() : Uploading...%3d%%\r", int(upload * 100.0f));
        }

        fprintf(stderr, "\n");
        closeComPort();
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

            // Read segment
            int segmentSize = (segment._segmentSize == 0) ? 256 : segment._segmentSize;
            segment._dataBytes.resize(segmentSize);
            infile.read((char *)&segment._dataBytes[0], segmentSize);
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

        // Special case: There can only be one segment in page 0 - merge all the occurences with padding if necessary.
        while (gt1File._segments.size() >= 2 && gt1File._segments[0]._hiAddress == 0 && gt1File._segments[1]._hiAddress == 0)
        {
            Gt1Segment& A = gt1File._segments[0];
            Gt1Segment& B = gt1File._segments[1];
            uint8_t addr = A._loAddress + A._segmentSize;
            while (addr < B._loAddress) {
                A._dataBytes.push_back(addr == 0x80 ? 1:0);
                A._segmentSize++;
                addr++;
            }
            A._dataBytes.insert(A._dataBytes.end(), B._dataBytes.begin(), B._dataBytes.end());
            A._segmentSize += B._segmentSize;

            gt1File._segments.erase(gt1File._segments.begin() + 1);
        }

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
            int segmentSize = (gt1File._segments[i]._segmentSize == 0) ? 256 : gt1File._segments[i]._segmentSize;
            outfile.write((char *)&gt1File._segments[i]._dataBytes[0], segmentSize);
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

    uint16_t printGt1Stats(const std::string& filename, const Gt1File& gt1File)
    {
        // Header
        uint16_t totalSize = 0;
        for(int i=0; i<gt1File._segments.size(); i++)
        {
            totalSize += int(gt1File._segments[i]._dataBytes.size());
        }
        uint16_t startAddress = gt1File._loStart + (gt1File._hiStart <<8);
        fprintf(stderr, "\n************************************************************\n");
        fprintf(stderr, "* %s : 0x%04x : %5d bytes : %3d segments\n", filename.c_str(), startAddress, totalSize, int(gt1File._segments.size()));
        fprintf(stderr, "************************************************************\n");
        fprintf(stderr, "* Segment :  Type  : Address : Memory Used                  \n");
        fprintf(stderr, "************************************************************\n");

        // Segments
        int contiguousSegments = 0;
        int startContiguousSegment = 0;
        uint16_t startContiguousAddress = 0x0000;
        for(int i=0; i<gt1File._segments.size(); i++)
        {
            uint16_t address = gt1File._segments[i]._loAddress + (gt1File._segments[i]._hiAddress <<8);
            int segmentSize = (gt1File._segments[i]._segmentSize == 0) ? 256 : gt1File._segments[i]._segmentSize;
            std::string memory = "RAM";
            if(gt1File._segments[i]._isRomAddress)
            {
                memory = "ROM";
                if(gt1File._segments.size() == 1)
                {
                    fprintf(stderr, "*  %4d   :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, totalSize);
                    fprintf(stderr, "************************************************************\n");
                    return totalSize;
                }
                totalSize -= segmentSize;
            }
            else if(segmentSize != int(gt1File._segments[i]._dataBytes.size()))
            {
                fprintf(stderr, "Segment %4d : %s 0x%04x : segmentSize %3d != dataBytes.size() %3d\n", i, memory.c_str(), address, segmentSize, int(gt1File._segments[i]._dataBytes.size()));
                return 0;
            }

            // New contiguous segment
            if(segmentSize == 256)
            {
                if(contiguousSegments == 0)
                {
                    startContiguousSegment = i;
                    startContiguousAddress = address;
                }
                contiguousSegments++;
            }
            else
            {
                // Normal segment < 256 bytes
                if(contiguousSegments == 0)
                {
                    fprintf(stderr, "*  %4d   :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, segmentSize);
                }
                // Contiguous segment < 256 bytes
                else
                {
                    fprintf(stderr, "*  %4d   :  %s   : 0x%04x  : %5d bytes (%dx256)\n", startContiguousSegment, memory.c_str(), startContiguousAddress, contiguousSegments*256, contiguousSegments);
                    fprintf(stderr, "*  %4d   :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, segmentSize);
                    contiguousSegments = 0;
                }
            }
        }
        fprintf(stderr, "************************************************************\n");
        fprintf(stderr, "* Free RAM after loading: %d\n", Cpu::getBaseFreeRAM() - totalSize);
        fprintf(stderr, "************************************************************\n");

        return totalSize;
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

    void uploadDirect(UploadTarget uploadTarget)
    {
        bool isGt1File = false;
        bool hasRomCode = false;
        bool hasRamCode = false;

        uint16_t executeAddress = Editor::getLoadBaseAddress();
        std::string filename = *Editor::getFileEntryName(Editor::getCursorY() + Editor::getFileEntriesIndex());
        std::string filepath = std::string(Editor::getBrowserPath() + "/" + filename);

        // Reset video table and reset single step watch address to video line counter
        Graphics::resetVTable();
        Editor::setSingleStepWatchAddress(VIDEO_Y_ADDRESS);

        // Compile gcl to gt1
        Gt1File gt1File;
        bool gt1FileBuilt = false;
        if(_configGclBuildFound  &&  filename.find(".gcl") != filename.npos)
        {
            // Create compile gcl string
            chdir(Editor::getBrowserPath().c_str());
            std::string command = "py \"" + _configGclBuild + "\"" + "/compilegcl.py " + "\"" + filepath + "\"" + " -s " + "\"" + _configGclBuild + "\"" + "/interface.json";

            // Create gt1 name and path
            size_t dot = filename.find_last_of(".");
            if(dot != std::string::npos)
            {
                filename = filename.substr(0, dot) + ".gt1";
                dot = filepath.find_last_of(".");
                filepath = filepath.substr(0, dot) + ".gt1";
            }

            // Build gcl
            int gt1FileDeleted = remove(filepath.c_str());
            fprintf(stderr, "\n");
            system(command.c_str());

            // Check for gt1
            std::ifstream infile(filepath, std::ios::binary | std::ios::in);
            if(!infile.is_open())
            {
                fprintf(stderr, "\nLoader::uploadDirect() : failed to compile '%s'.\n", filename.c_str());
                filename = "";
                if(gt1FileDeleted == 0) Editor::browseDirectory();
            }
            else
            {
                gt1FileBuilt = true;
            }
        }
        
        // Upload gt1
        if(filename.find(".gt1") != filename.npos)
        {
            Assembler::clearAssembler();

            if(!loadGt1File(filepath, gt1File)) return;
            executeAddress = gt1File._loStart + (gt1File._hiStart <<8);
            Editor::setLoadBaseAddress(executeAddress);

            if(uploadTarget == Emulator)
            {
                for(int j=0; j<gt1File._segments.size(); j++)
                {
                    uint16_t address = gt1File._segments[j]._loAddress + (gt1File._segments[j]._hiAddress <<8);
                    for(int i=0; i<gt1File._segments[j]._dataBytes.size(); i++)
                    {
                        Cpu::setRAM(address+i, gt1File._segments[j]._dataBytes[i]);
                    }
                }
            }

            isGt1File = true;
            hasRamCode = true;
            hasRomCode = false;
            _disableUploads = false;
        }
        // Upload vCPU assembly code
        else if(filename.find(".vasm") != filename.npos  ||  filename.find(".gasm") != filename.npos  ||  filename.find(".s") != filename.npos  ||  filename.find(".asm") != filename.npos)
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

                if(uploadTarget == Emulator  &&  !_disableUploads)
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

            gt1FileBuilt = true;
        }
        // Invalid file
        else
        {
            fprintf(stderr, "Loader::upload() : invalid file or file does not exist, '%s'.\n", filename.c_str());
            return;
        }

        uint16_t totalSize = printGt1Stats(filename, gt1File);
        Cpu::setFreeRAM(Cpu::getBaseFreeRAM() - totalSize); 

        if(uploadTarget == Emulator)
        {
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
            }
        }
        else if(uploadTarget == Hardware)
        {
            if(!isGt1File)
            {
                size_t i = filepath.rfind('.');
                filename = (i != std::string::npos) ? filepath.substr(0, i) + ".gt1" : filepath + ".gt1";
                uploadToGiga(filename);
            }
            else
            {
                uploadToGiga(filepath);
            }
        }

        // Updates browser in case a new gt1 file was created from a gcl file or a vasm file
        if(gt1FileBuilt) Editor::browseDirectory();

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

        if(_uploadTarget != None  ||  frameUploading)
        {
            uint16_t executeAddress = Editor::getLoadBaseAddress();
            if(_uploadTarget != None)
            {
                uploadDirect(_uploadTarget);
                _uploadTarget = None;
                //frameUploading = true;

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
