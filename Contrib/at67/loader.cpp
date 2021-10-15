#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include "dirent/dirent.h"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#else
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#endif

#ifndef STAND_ALONE
#include "editor.h"
#include "timing.h"
#include "graphics.h"
#include "inih/INIReader.h"
#include "rs232/rs232.h"
#include "spi.h"
#endif

#include "memory.h"
#include "cpu.h"
#include "audio.h"
#include "loader.h"
#include "expression.h"
#include "assembler.h"
#include "compiler.h"
#include "keywords.h"
#include "validater.h"


#define DEFAULT_COM_BAUD_RATE 115200
#define DEFAULT_COM_PORT      0
#define DEFAULT_GIGA_TIMEOUT  5.0
#define MAX_GT1_SIZE          (1<<16)


namespace Loader
{
    bool _hostIsBigEndian = false;

    std::string _exePath = ".";
    std::string _cwdPath = ".";
    std::string _filePath = ".";


    const std::string& getExePath(void) {return _exePath;}
    const std::string& getCwdPath(void) {return _cwdPath;}
    const std::string& getFilePath(void) {return _filePath;}
    void setFilePath(const std::string& filePath) {_filePath = filePath;}

#ifdef _WIN32
    char* getcwd(char* dst, int size)
    {
        return _getcwd(dst, size);
    }
    int chdir(const char* path)
    {
        return _chdir(path);
    }
    std::string getExeDir(void)
    {
        char dir[MAX_PATH] = {0};
        GetModuleFileName(NULL, dir, MAX_PATH);
        std::string path = dir;
        size_t slash = path.find_last_of("\\/");
        path = (slash != std::string::npos) ? path.substr(0, slash) : ".";
        Expression::replaceText(path, "\\", "/");
        return path;
    }
#else
    std::string getExeDir(void)
    {
        char dir[PATH_MAX];
        ssize_t result = readlink("/proc/self/exe", dir, PATH_MAX);
        std::string path = (result > 0) ? dir : ".";
        size_t slash = path.find_last_of("\\/");
        path = (slash != std::string::npos) ? path.substr(0, slash) : ".";
        Expression::replaceText(path, "\\", "/");
        return path;
    }
#endif

    bool loadGt1File(const std::string& filename, Gt1File& gt1File)
    {
        std::ifstream infile(filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadGt1File() : failed to open '%s'\n", filename.c_str());
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
                fprintf(stderr, "Loader::loadGt1File() : bad header in segment %d of '%s'\n", segmentCount, filename.c_str());
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
            int segmentSize = (segment._segmentSize == 0) ? SEGMENT_SIZE : segment._segmentSize;
            segment._dataBytes.resize(segmentSize);
            infile.read((char *)&segment._dataBytes[0], segmentSize);
            if(infile.eof() || infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadGt1File() : bad segment %d in '%s'\n", segmentCount, filename.c_str());
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

        size_t dot = filepath.rfind('.');
        filename = (dot != std::string::npos) ? filepath.substr(0, dot) + ".gt1" : filepath + ".gt1";

        std::ofstream outfile(filename, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Loader::saveGt1File() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        // Sort segments from lowest address to highest address
        std::sort(gt1File._segments.begin(), gt1File._segments.end(), [](const Gt1Segment& segmentA, const Gt1Segment& segmentB)
        {
            uint16_t addressA = (segmentA._hiAddress <<8) | segmentA._loAddress;
            uint16_t addressB = (segmentB._hiAddress <<8) | segmentB._loAddress;
            return (addressA < addressB);
        });

        // Merge page 0 segments together
        Gt1Segment page0;
        int page0Segments = 0;
        for(int i=0; i<int(gt1File._segments.size()); i++) if(gt1File._segments[i]._hiAddress == 0x00) page0Segments++;
        if(page0Segments > 1)
        {
            uint8_t start = gt1File._segments[0]._loAddress;
            uint8_t end = gt1File._segments[page0Segments-1]._loAddress + uint8_t(gt1File._segments[page0Segments-1]._dataBytes.size()) - 1;

            // Reserve space taking into account ONE_CONST_ADDRESS
            page0._loAddress = start;
            page0._segmentSize = end - start + 1;
            page0._dataBytes.resize(end - start + 1, 0x00);
            if(start <= ONE_CONST_ADDRESS && end >= ONE_CONST_ADDRESS) page0._dataBytes[ONE_CONST_ADDRESS-start] = 0x01;

            // Copy page 0 segments
            fprintf(stderr, "\n* Merging %d page 0 segments\n", page0Segments);
            for(int i=0; i<page0Segments; i++)
            {
                int j = 0;
                int seg = gt1File._segments[i]._loAddress - start;
                if(page0._dataBytes.size() < seg + gt1File._segments[i]._dataBytes.size())
                {
                    fprintf(stderr, "* Can't Merge: start: 0x%0x  end: 0x%02x  size: 0x%02x\n", gt1File._segments[0]._loAddress, 
                                                                                                gt1File._segments[0]._loAddress + uint8_t(gt1File._segments[0]._dataBytes.size()) - 1, 
                                                                                                uint8_t(gt1File._segments[0]._dataBytes.size()));
                    return false;
                }

                for(int k=seg; k<seg+int(gt1File._segments[i]._dataBytes.size()); k++)
                {
                    page0._dataBytes[k] = gt1File._segments[i]._dataBytes[j++];
                }
                fprintf(stderr, "* Segment: %03d  start: 0x%0x  end: 0x%02x  size: 0x%02x\n", i, gt1File._segments[i]._loAddress, 
                                                                                                 gt1File._segments[i]._loAddress + uint8_t(gt1File._segments[i]._dataBytes.size()) - 1, 
                                                                                                 uint8_t(gt1File._segments[i]._dataBytes.size()));
            }

            // Erase old page 0 segments
            for(int i=0; i<page0Segments; i++) gt1File._segments.erase(gt1File._segments.begin());

            // Insert merged page 0 segment
            gt1File._segments.insert(gt1File._segments.begin(), page0);
            fprintf(stderr, "* Merged:       start: 0x%0x  end: 0x%02x  size: 0x%02x\n", gt1File._segments[0]._loAddress, 
                                                                                         gt1File._segments[0]._loAddress + uint8_t(gt1File._segments[0]._dataBytes.size()) - 1, 
                                                                                         uint8_t(gt1File._segments[0]._dataBytes.size()));
        }

#if 1
        // Merge non page 0 segments together
        std::vector<Gt1Segment> segmentsOut;
        int lastPage = int((Memory::getSizeRAM() - 1) >>8);
        for(int j=1; j<=lastPage; j++)
        {
            uint8_t hiAddr = uint8_t(j);
            std::vector<Gt1Segment> segmentsIn;
            for(int i=0; i<int(gt1File._segments.size()); i++)
            {
                if(gt1File._segments[i]._hiAddress == hiAddr) segmentsIn.push_back(gt1File._segments[i]);
            }

            if(segmentsIn.size())
            {
                // Sort segmentsIn from lowest low byte address to highest low byte address
                std::sort(segmentsIn.begin(), segmentsIn.end(), [](const Gt1Segment& segmentA, const Gt1Segment& segmentB)
                {
                    return (segmentA._loAddress < segmentB._loAddress);
                });

                bool lastSegFractured = false;
                Gt1Segment segment = {false, hiAddr, segmentsIn[0]._loAddress, uint8_t(segmentsIn[0]._dataBytes.size()), segmentsIn[0]._dataBytes};
                for(int i=0; i<int(segmentsIn.size()); i++)
                {
                    if(segment._dataBytes.size() == 0)
                    {
                        lastSegFractured = false;
                        segment._loAddress = segmentsIn[i]._loAddress;
                        segment._dataBytes = segmentsIn[i]._dataBytes;
                        segment._segmentSize = uint8_t(segmentsIn[i]._dataBytes.size());
                    }

                    if(i<int(segmentsIn.size()-1)  &&  segmentsIn[i]._loAddress + segmentsIn[i]._dataBytes.size()  ==  segmentsIn[i + 1]._loAddress)
                    {
                        segment._dataBytes.insert(segment._dataBytes.end(), segmentsIn[i + 1]._dataBytes.begin(), segmentsIn[i + 1]._dataBytes.end());

                        uint16_t segmentSize = segment._segmentSize + segmentsIn[i + 1]._segmentSize;
                        if(segmentSize > SEGMENT_SIZE)
                        {
                            fprintf(stderr, "Loader::saveGt1File() : total segment size:%d > %d in segment %d\n", segment._segmentSize, SEGMENT_SIZE, i);
                            return false;
                        }

                        segment._segmentSize += segmentsIn[i + 1]._segmentSize;
                        if(segmentSize == SEGMENT_SIZE) segment._segmentSize = 0;
                    }
                    else
                    {
                        lastSegFractured = true;
                        segmentsOut.push_back(segment);
                        segment._dataBytes.clear();
                    }
                }
                if(!lastSegFractured) segmentsOut.push_back(segment);
            }
        }

        // Erase pages 1 to N and insert new pages, (page 0 exists and has already been merged into first segment)
        if(gt1File._segments[0]._hiAddress == 0x00)
        {
            gt1File._segments.erase(gt1File._segments.begin() + 1, gt1File._segments.end());
        }
        // Erase all pages and insert new pages
        else
        {
            gt1File._segments.clear();
        }
        gt1File._segments.insert(gt1File._segments.end(), segmentsOut.begin(), segmentsOut.end());
#endif

        // Output
        for(int i=0; i<int(gt1File._segments.size()); i++)
        {
            // Write header
            outfile.write((char *)&gt1File._segments[i]._hiAddress, SEGMENT_HEADER_SIZE);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveGt1File() : write error in header of segment %d\n", i);
                return false;
            }

            // Write segment
            int segmentSize = (gt1File._segments[i]._segmentSize == 0) ? SEGMENT_SIZE : gt1File._segments[i]._segmentSize;
            outfile.write((char *)&gt1File._segments[i]._dataBytes[0], segmentSize);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveGt1File() : bad segment %d in '%s'\n", i, filename.c_str());
                return false;
            }
        }

        // Write trailer
        outfile.write((char *)&gt1File._terminator, GT1FILE_TRAILER_SIZE);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveGt1File() : write error in trailer of '%s'\n", filename.c_str());
            return false;
        }

        return true;
    }

    uint16_t printGt1Stats(const std::string& filename, const Gt1File& gt1File, bool isGbasFile)
    {
        size_t nameSuffix = filename.find_last_of(".");
        std::string output = filename.substr(0, nameSuffix) + ".gt1";

        // Header
        uint16_t totalSize = 0;
        for(int i=0; i<int(gt1File._segments.size()); i++)
        {
            // Don't count page 0 RAM usage or segments outside of current RAM size
            if(gt1File._segments[i]._hiAddress)
            {
                uint16_t address = gt1File._segments[i]._loAddress + (gt1File._segments[i]._hiAddress <<8);
                uint16_t segmentSize = (gt1File._segments[i]._segmentSize == 0) ? SEGMENT_SIZE : gt1File._segments[i]._segmentSize;
                //if((address + segmentSize - 1) < Memory::getSizeRAM()  &&  !Memory::isVideoRAM(address)) totalSize += segmentSize;
                if((address + segmentSize - 1) < Memory::getSizeRAM()) totalSize += segmentSize;
                if((address & SEGMENT_MASK) + segmentSize > SEGMENT_SIZE)
                {
                    fprintf(stderr, "\nLoader::printGt1Stats() : Page overflow : segment %d : address 0x%04x : segmentSize %3d\n", i, address, segmentSize);
                    return 0;
                }
            }
        }
        uint16_t startAddress = gt1File._loStart + (gt1File._hiStart <<8);
        fprintf(stderr, "\n*******************************************************\n");
        fprintf(stderr, "*                     Creating .gt1                    \n");
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "* %-20s : 0x%04x  : %5d bytes                          \n", output.c_str(), startAddress, totalSize);
#if 1
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "*   Segment   :  Type  : Address :    Size             \n");
        fprintf(stderr, "*******************************************************\n");

        // Segments
        int contiguousSegments = 0;
        int startContiguousSegment = 0;
        uint16_t startContiguousAddress = 0x0000;
        for(int i=0; i<int(gt1File._segments.size()); i++)
        {
            uint16_t address = gt1File._segments[i]._loAddress + (gt1File._segments[i]._hiAddress <<8);
            uint16_t segmentSize = (gt1File._segments[i]._segmentSize == 0) ? SEGMENT_SIZE : gt1File._segments[i]._segmentSize;
            std::string memory = "RAM";
            if(gt1File._segments[i]._isRomAddress)
            {
                memory = "ROM";
                if(gt1File._segments.size() == 1)
                {
                    fprintf(stderr, "*    %4d     :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, totalSize);
                    fprintf(stderr, "*******************************************************\n");
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
            if(segmentSize == SEGMENT_SIZE)
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
                    fprintf(stderr, "*    %4d     :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, segmentSize);
                }
                // Contiguous segment < 256 bytes
                else
                {
                    fprintf(stderr, "*    %4d     :  %s   : 0x%04x  : %5d bytes (%dx%d)\n", startContiguousSegment, memory.c_str(), startContiguousAddress, contiguousSegments*SEGMENT_SIZE, contiguousSegments, SEGMENT_SIZE);
                    fprintf(stderr, "*    %4d     :  %s   : 0x%04x  : %5d bytes\n", i, memory.c_str(), address, segmentSize);
                    contiguousSegments = 0;
                }
            }
        }
#endif
        if(!isGbasFile) Memory::setSizeFreeRAM(Memory::getBaseFreeRAM() - totalSize); 

        int memSize = (Memory::getSizeRAM() == RAM_SIZE_LO) ? 32 : 64;
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "* Free RAM on %dK Gigatron       : %5d bytes           \n", memSize, Memory::getSizeFreeRAM());
        fprintf(stderr, "*******************************************************\n");

        return totalSize;
    }


#ifndef STAND_ALONE
    enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
    enum FrameState {Resync=0, Frame, Execute, NumFrameStates};


    bool _disableUploads = false;

    int _gt1UploadSize = 0;
    char _gt1Buffer[MAX_GT1_SIZE];

    int _numComPorts = 0;
    int _currentComPort = -1;
    bool _enableComPort = false;

    std::string _configComPort;
    int _configBaudRate = DEFAULT_COM_BAUD_RATE;
    double _configTimeOut = DEFAULT_GIGA_TIMEOUT;
    
    std::string _configGclBuild = ".";
    bool _configGclBuildFound = false;

    bool _autoSet64k = true;

    std::vector<ConfigRom> _configRoms;

    std::string _currentGame = "";

    INIReader _configIniReader;
    INIReader _highScoresIniReader;
    std::map<std::string, SaveData> _saveData;

    SDL_Thread* _uploadThread = nullptr;


    void shutdown(void)
    {
        if(_uploadThread) SDL_WaitThread(_uploadThread, nullptr);
        closeComPort();
    }

    bool getKeyAsString(INIReader& iniReader, const std::string& sectionString, const std::string& iniKey, const std::string& defaultKey, std::string& result, bool upperCase=true)
    {
        result = iniReader.Get(sectionString, iniKey, defaultKey);
        if(result == defaultKey) return false;
        if(upperCase) Expression::strToUpper(result);
        return true;
    }
#endif

    void initialise(void)
    {
        // Current working directory
        char cwdPath[FILENAME_MAX];
        if(!getcwd(cwdPath, FILENAME_MAX)) strcpy(cwdPath, ".");
        _cwdPath = std::string(cwdPath);
        Expression::replaceText(_cwdPath, "\\", "/");
        _exePath = getExeDir();

        if(Cpu::getHostEndianness() == Cpu::BigEndian) _hostIsBigEndian = true;

#ifndef STAND_ALONE
        _enableComPort = false;
        _currentComPort = -1;

        _numComPorts = comEnumerate();
        if(_numComPorts == 0) fprintf(stderr, "Loader::initialise() : no COM ports found.\n");

        // Loader config
        std::string configPath = _exePath + "/" + LOADER_CONFIG_INI;
        //fprintf(stderr, "%s\n", configPath.c_str());
        INIReader iniReader(configPath);
        _configIniReader = iniReader;
        if(_configIniReader.ParseError() == 0)
        {
            // Parse Loader Keys
            enum Section {Comms, ROMS, RAM, Unk = -1 };
            std::map<std::string, Section> section;
            section["Comms"] = Comms;
            section["ROMS"]  = ROMS;
            section["RAM"]   = RAM;

            for(auto sectionString : _configIniReader.Sections())
            {
                std::string result;
                Section sec = Unk;

                if(section.find(sectionString) != section.end())
                    sec = section[sectionString];

                switch(sec)
                {
                    case Comms:
                    {
                        // Enable comms
                        getKeyAsString(_configIniReader, sectionString, "Enable", "0", result, false);
                        _enableComPort = strtol(result.c_str(), nullptr, 10);

                        // Baud rate
                         getKeyAsString(_configIniReader, sectionString, "BaudRate", "115200", result);   
                        _configBaudRate = strtol(result.c_str(), nullptr, 10);
 
                        // Com port
                        char *endPtr;
                        getKeyAsString(_configIniReader, sectionString, "ComPort", "0", result);
                        _currentComPort = strtol(result.c_str(), &endPtr, 10);
                        if((endPtr - &result[0]) != int(result.size()))
                        {
                            _currentComPort = comFindPort(result.c_str());
                            if(_currentComPort < 0)
                            {
                                fprintf(stderr, "Loader::initialise() : Couldn't find COM Port '%s' in INI file '%s'\n", result.c_str(), LOADER_CONFIG_INI);
                            }
                        }
                        _configComPort = result;

                        // Time out
                        getKeyAsString(_configIniReader, sectionString, "TimeOut", "5.0", result);
                        _configTimeOut = strtod(result.c_str(), nullptr);

                        // GCL tools build path
                        _configGclBuildFound = getKeyAsString(_configIniReader, sectionString, "GclBuild", ".", result, false);
                        _configGclBuild = result;
                    }
                    break;

                    case ROMS:
                    {
                        for(int index=0; ; index++)
                        {
                            ConfigRom configRom;

                            // ROM name
                            std::string romName = "RomName" + std::to_string(index);
                            if(!getKeyAsString(_configIniReader, sectionString, romName, "", result, false)) break;
                            configRom._name = result;

                            // ROM type
                            std::string romVer = "RomType" + std::to_string(index);
                            if(!getKeyAsString(_configIniReader, sectionString, romVer, "", result)) break;
                            configRom._type = uint8_t(std::stoul(result, nullptr, 16));

                            _configRoms.push_back(configRom);
                        }
                    }
                    break;

                    case RAM:
                    {
                        // Enable auto 64K
                        getKeyAsString(_configIniReader, sectionString, "AutoSet64k", "1", result, false);
                        _autoSet64k = strtol(result.c_str(), nullptr, 10);
                    }
                    break;

                    default:
                    {
                        // Delegate to other ini handlers.
                        if (! Spi::config(_configIniReader, sectionString))
                            fprintf(stderr, "Loader::initialise() : INI file '%s' has bad Sections : reverting to default values.\n", LOADER_CONFIG_INI);
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
        INIReader highScoresIniReader(_exePath + "/" + HIGH_SCORES_INI);
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
            fprintf(stderr, "Loader::initialise() : couldn't load high scores INI file '%s' : high scores are disabled.\n", HIGH_SCORES_INI);
        }
#endif
    }


#ifndef STAND_ALONE
    const std::string& getConfigComPort(void) {return _configComPort;}

    const std::string& getCurrentGame(void) {return _currentGame;}
    void setCurrentGame(const std::string& currentGame) {_currentGame = currentGame;}

    int getConfigRomsSize(void) {return int(_configRoms.size());}
    ConfigRom* getConfigRom(int index)
    {
        if(_configRoms.size() == 0  ||  index >= int(_configRoms.size())) return nullptr;

        return &_configRoms[index];
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

    bool openComPort(void)
    {
        if(!_enableComPort)
        {
            _currentComPort = -1;
            return false;
        }

        if(_numComPorts == 0)
        {
            _numComPorts = comEnumerate();
            if(_numComPorts == 0)
            {
                fprintf(stderr, "Loader::openComPort() : no COM ports found\n");
                return false;
            }
        }

        if(_currentComPort < 0)
        {
            _numComPorts = 0;
            fprintf(stderr, "Loader::openComPort() : couldn't open COM port '%s'\n", _configComPort.c_str());
            return false;
        } 
        else if(comOpen(_currentComPort, _configBaudRate) == 0)
        {
            _numComPorts = 0;
            fprintf(stderr, "Loader::openComPort() : couldn't open COM port '%s'\n", comGetPortName(_currentComPort));
            return false;
        } 

        return true;
    }

    void closeComPort(void)
    {
        comClose(_currentComPort);
        _currentComPort = -1;
    }

    bool checkComPort(void)
    {
        if(!_enableComPort)
        {
            fprintf(stderr, "Loader::checkComPort() : Comms in '%s' disabled\n", LOADER_CONFIG_INI);
            return false;
        }

        if(_currentComPort < 0)
        {
            fprintf(stderr, "Loader::checkComPort() : Invalid COM port '%s'\n", _configComPort.c_str());
            return false;
        }

        return true;
    }


    bool readCharGiga(char* chr)
    {
        if(!checkComPort()) return false;

        return (comRead(_currentComPort, chr, 1) == 1);
    }

    bool sendCharGiga(char chr)
    {
        if(!checkComPort()) return false;

        return (comWrite(_currentComPort, &chr, 1) == 1);
    }

    bool readLineGiga(std::string& line)
    {
        if(!checkComPort()) return false;

        line.clear();
        char buffer = 0;
        uint64_t prevFrameCounter = SDL_GetPerformanceCounter();

        while(buffer != '\n')
        {
            if(comRead(_currentComPort, &buffer, 1))
            {
                if((buffer >= 32  &&  buffer <= 126)  ||  buffer == '\n') line.push_back(buffer);
            }
            double frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
            if(frameTime > _configTimeOut) return false;
        }

        // Replace '\n'
        line.back() = 0;

        //fprintf(stderr, "%s\n", line.c_str());

        return true;
    }

    bool readLineGiga(std::vector<std::string>& text)
    {
        if(!checkComPort()) return false;

        std::string line;
        if(!readLineGiga(line))
        {
            fprintf(stderr, "Loader::readLineGiga() : timed out on COM port : %s\n", comGetPortName(_currentComPort));
            return false;
        }

        text.push_back(line);

        return true;
    }

    bool readUntilPromptGiga(std::vector<std::string>& text)
    {
        if(!checkComPort()) return false;

        std::string line;
        do
        {
            if(!readLineGiga(line))
            {
                fprintf(stderr, "Loader::readUntilPromptGiga() : timed out on COM port : %s\n", comGetPortName(_currentComPort));
                return false;
            }

            if(size_t e = line.find('!') != std::string::npos)
            {
                fprintf(stderr, "Loader::readUntilPromptGiga() : Arduino Error : %s\n", &line[e]);
                return false;
            }

            text.push_back(line);
        }
        while(line.find("?") == std::string::npos);

        return true;
    }

    bool waitForPromptGiga(std::string& line)
    {
        if(!checkComPort()) return false;

        do
        {
            if(!readLineGiga(line))
            {
                fprintf(stderr, "Loader::waitForPromptGiga() : timed out on COM port : %s\n", comGetPortName(_currentComPort));
                return false;
            }

            //fprintf(stderr, "Loader::waitForPromptGiga() : %s\n", line.c_str());

            if(size_t e = line.find('!') != std::string::npos)
            {
                fprintf(stderr, "Loader::waitForPromptGiga() : Arduino Error : %s\n", &line[e]);
                return false;
            }
        }
        while(line.find("?") == std::string::npos);

        return true;
    }

    bool sendCommandToGiga(char cmd, std::string& line, bool wait)
    {
        if(!checkComPort()) return false;

        char command[2] = {cmd, '\n'};
        comWrite(_currentComPort, command, 2);

        // Wait for ready prompt
        if(wait)
        {
            if(!waitForPromptGiga(line)) return false;
        }

        return true;
    }

    bool sendCommandToGiga(char cmd, bool wait)
    {
        if(!checkComPort()) return false;

        std::string line;
        return sendCommandToGiga(cmd, line, wait);
    }

    bool sendCommandToGiga(const std::string& cmd, std::vector<std::string>& text)
    {
        if(!checkComPort()) return false;

        comWrite(_currentComPort, cmd.c_str(), cmd.size());
        return readUntilPromptGiga(text);
    }


    int uploadToGigaThread(void* userData)
    {
        if(!checkComPort())
        {
            _uploadThread = nullptr;
            return -1;
        }

        Graphics::enableUploadBar(true);

        std::string line;
        if(!sendCommandToGiga('R', line, true)) return -1;
        if(!sendCommandToGiga('L', line, true)) return -1;
        if(!sendCommandToGiga('U', line, true)) return -1;

        int gt1Size = *((int*)userData);

        int index = 0;
        while(std::isdigit((unsigned char)line[0]))
        {
            int n = strtol(line.c_str(), nullptr, 10);
            comWrite(_currentComPort, &_gt1Buffer[index], n);
            index += n;

            if(!waitForPromptGiga(line))
            {
                Graphics::enableUploadBar(false);
                //fprintf(stderr, "\n");
                return -1;
            }

            float upload = float(index) / float(gt1Size);
            Graphics::updateUploadBar(upload);
            //fprintf(stderr, "Loader::uploadToGiga() : Uploading...%3d%%\r", int(upload * 100.0f));
        }

        Graphics::enableUploadBar(false);
        //fprintf(stderr, "\n");

        _uploadThread = nullptr;

        return 0;
    }

    void uploadToGiga(const std::string& filepath, const std::string& filename)
    {
        if(!checkComPort()) return;

        // An upload is already in progress
        if(Graphics::getUploadBarEnabled()) return;

        std::ifstream gt1file(filepath, std::ios::binary | std::ios::in);
        if(!gt1file.is_open())
        {
            fprintf(stderr, "Loader::uploadToGiga() : failed to open '%s'\n", filepath.c_str());
            return;
        }

        gt1file.read(_gt1Buffer, MAX_GT1_SIZE);
        if(gt1file.bad())
        {
            fprintf(stderr, "Loader::uploadToGiga() : failed to read GT1 file '%s'\n", filepath.c_str());
            return;
        }

        Graphics::setUploadFilename(filename);

        _gt1UploadSize = int(gt1file.gcount());
        _uploadThread = SDL_CreateThread(uploadToGigaThread, "gtemuAT67::Loader::uploadToGiga()", (void*)&_gt1UploadSize);
    }

    void disableUploads(bool disable)
    {
        _disableUploads = disable;
    }

    bool loadDataFile(SaveData& saveData)
    {
        SaveData sdata = saveData;
        std::string filename = sdata._filename + ".dat";
        std::ifstream infile(_exePath + "/" + filename, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadDataFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        // Load counts
        uint16_t numCounts = 0;
        infile.read((char *)&numCounts, 2);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadDataFile() : read error in number of counts in '%s'\n", filename.c_str());
            return false;
        }
        if(_hostIsBigEndian) Cpu::swapEndianness(numCounts);
        for(int i=0; i<numCounts; i++)
        {
            uint16_t count;
            infile.read((char *)&count, 2);
            if(infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadDataFile() : read error in counts of '%s'\n", filename.c_str());
                return false;
            }
            if(_hostIsBigEndian) Cpu::swapEndianness(count);
            sdata._counts[i] = count;
        }         

        // Load addresses
        uint16_t numAddresses = 0;
        infile.read((char *)&numAddresses, 2);
        if(infile.eof() || infile.bad() || infile.fail())
        {
            fprintf(stderr, "Loader::loadDataFile() : read error in number of addresses in '%s'\n", filename.c_str());
            return false;
        }
        if(_hostIsBigEndian) Cpu::swapEndianness(numAddresses);
        for(int i=0; i<numAddresses; i++)
        {
            uint16_t address;
            infile.read((char *)&address, 2);
            if(infile.bad() || infile.fail())
            {
                fprintf(stderr, "Loader::loadDataFile() : read error in addresses of '%s'\n", filename.c_str());
                return false;
            }
            if(_hostIsBigEndian) Cpu::swapEndianness(address);
            sdata._addresses[i] = address;
        }         

        if(sdata._counts.size() == 0  ||  sdata._counts.size() != sdata._addresses.size())
        {
            fprintf(stderr, "Loader::loadDataFile() : save data is corrupt : saveData._counts.size() = %d : saveData._addresses.size() = %d\n", int(sdata._counts.size()), int(sdata._addresses.size()));
            return false;
        }

        // load data
        for(int j=0; j<int(sdata._addresses.size()); j++)
        {
            //sdata._data.push_back(std::vector<uint8_t>(sdata._counts[j], 0x00));
            for(uint16_t i=0; i<sdata._counts[j]; i++)
            {
                uint8_t data;
                infile.read((char *)&data, 1);
                if(infile.bad() || infile.fail())
                {
                    fprintf(stderr, "Loader::loadDataFile() : read error in data of '%s'\n", filename.c_str());
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
    bool saveDataFile(SaveData& saveData)
    {
        std::string filename = saveData._filename + ".dat";
        std::ofstream outfile(_exePath + "/" + filename, std::ios::binary | std::ios::out);
        if(!outfile.is_open())
        {
            fprintf(stderr, "Loader::saveDataFile() : failed to open '%s'\n", filename.c_str());
            return false;
        }

        if(saveData._counts.size() == 0  ||  saveData._counts.size() != saveData._addresses.size())
        {
            fprintf(stderr, "Loader::saveDataFile() : save data is corrupt : saveData._counts.size() = %d : saveData._addresses.size() = %d\n", int(saveData._counts.size()), int(saveData._addresses.size()));
            return false;
        }

        // Save counts
        uint16_t numCounts = uint16_t(saveData._counts.size());
        if(_hostIsBigEndian) Cpu::swapEndianness(numCounts);
        outfile.write((char *)&numCounts, 2);
        if(_hostIsBigEndian) Cpu::swapEndianness(numCounts);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveDataFile() : write error in number of counts of '%s'\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numCounts; i++)
        {
            if(_hostIsBigEndian) Cpu::swapEndianness(saveData._counts[i]);
            outfile.write((char *)&saveData._counts[i], 2);
            if(_hostIsBigEndian) Cpu::swapEndianness(saveData._counts[i]);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveDataFile() : write error in counts of '%s'\n", filename.c_str());
                return false;
            }
        }         

        // Save addresses
        uint16_t numAddresses = uint16_t(saveData._addresses.size());
        if(_hostIsBigEndian) Cpu::swapEndianness(numAddresses);
        outfile.write((char *)&numAddresses, 2);
        if(_hostIsBigEndian) Cpu::swapEndianness(numAddresses);
        if(outfile.bad() || outfile.fail())
        {
            fprintf(stderr, "Loader::saveDataFile() : write error in number of addresses of '%s'\n", filename.c_str());
            return false;
        }
        for(int i=0; i<numAddresses; i++)
        {
            if(_hostIsBigEndian) Cpu::swapEndianness(saveData._addresses[i]);
            outfile.write((char *)&saveData._addresses[i], 2);
            if(_hostIsBigEndian) Cpu::swapEndianness(saveData._addresses[i]);
            if(outfile.bad() || outfile.fail())
            {
                fprintf(stderr, "Loader::saveDataFile() : write error in addresses of '%s'\n", filename.c_str());
                return false;
            }
        }         

        // Check data has been initialised
        for(int j=0; j<int(saveData._addresses.size()); j++)
        {
            if(saveData._data.size() != saveData._addresses.size())
            {
                fprintf(stderr, "Loader::saveDataFile() : data has not been initialised or loaded, nothing to save for '%s'\n", filename.c_str());
                return false;
            }
            for(int i=0; i<saveData._counts[j]; i++)
            {
                if(saveData._data[j].size() != saveData._counts[j]) 
                {
                    fprintf(stderr, "Loader::saveDataFile() : data has not been initialised or loaded, nothing to save for '%s'\n", filename.c_str());
                    return false;
                }
            }
        }

        // Save data
        for(int j=0; j<int(saveData._addresses.size()); j++)
        {
            for(int i=0; i<saveData._counts[j]; i++)
            {
                uint8_t data = saveData._data[j][i];
                outfile.write((char *)&data, 1);
                if(outfile.bad() || outfile.fail())
                {
                    fprintf(stderr, "Loader::saveDataFile() : write error in data of '%s'\n", filename.c_str());
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
            //fprintf(stderr, "Loader::loadHighScore() : warning, no game entry defined in '%s' for '%s'\n", HIGH_SCORES_INI, _currentGame.c_str());
            return;
        }

        if(Loader::loadDataFile(_saveData[_currentGame]))
        {
            fprintf(stderr, "Loader::loadHighScore() : loaded high score data successfully for '%s'\n", _currentGame.c_str());
        }
    }

    // Saves high score for current game to a simple <game>.dat file
    bool saveHighScore(void)
    {
        if(_saveData.find(_currentGame) == _saveData.end())
        {
            fprintf(stderr, "Loader::saveHighScore() : error, no game entry defined in '%s' for '%s'\n", HIGH_SCORES_INI, _currentGame.c_str());
            return false;
        }

        if(Loader::saveDataFile(_saveData[_currentGame]))
        {
            //fprintf(stderr, "Loader::saveHighScore() : saved high score data successfully for '%s'\n", _currentGame.c_str());
            return true;
        }

        fprintf(stderr, "Loader::saveHighScore() : saving high score data failed, for '%s'\n", _currentGame.c_str());

        return false;
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
        for(int j=0; j<int(_saveData[_currentGame]._addresses.size()); j++)
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
                uint8_t data = Cpu::getRAM(uint16_t(_saveData[_currentGame]._addresses[j] + i));

                // TODO: create a list of INI rules to make this test more flexible
                if(data < _saveData[_currentGame]._data[j][i]) return;
                if(_saveData[_currentGame]._data[j][i] == 0  ||  data > _saveData[_currentGame]._data[j][i])
                {
                    for(int k=i; k!=end; k+=step)
                    {
                        _saveData[_currentGame]._data[j][k] = Cpu::getRAM(uint16_t(_saveData[_currentGame]._addresses[j] + k));
                    }
                    saveHighScore();
                    return;
                }

                start += step;
            }
        }
    }

    bool loadGtbFile(const std::string& filepath)
    {
        // open .gtb file
        std::ifstream infile(filepath, std::ios::binary | std::ios::in);
        if(!infile.is_open())
        {
            fprintf(stderr, "Loader::loadGtbFile() : failed to open '%s'\n", filepath.c_str());
            return false;
        }

        // Read .gtb file
        std::string line;
        std::vector<std::string> lines;
        while(!infile.eof())
        {
            std::getline(infile, line);
            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Loader::loadGtbFile() : '%s:%d' : bad line '%s'\n", filepath.c_str(), int(lines.size()+1), line.c_str());
                return false;
            }

            if(line.size()) lines.push_back(line);
        }
        
        // Delete non numbered lines, (comments etc)
        for(auto i=lines.begin(); i!=lines.end();)
        {
            long lineNumber = strtol(i->c_str(), nullptr, 10);
            if(lineNumber < 1  ||  lineNumber > 32767)
            {
                i = lines.erase(i);
            }
            else
            {
                ++i;
            }
        }

#if 0
        // Remove trailing comments
        for(int i=0; i<lines.size(); i++)
        {
            size_t pos = lines[i].find('\'');
            if(pos != string::npos  &&  pos > 2) lines[i] = lines[i].substr(0, pos-1);
            fprintf(stderr, "Loader::loadGtbFile() : %s\n", lines[i].c_str());
        }
#endif

        // Load .gtb file into memory
        uint16_t startAddress = GTB_LINE0_ADDRESS + MAX_GTB_LINE_SIZE;
        uint16_t endAddress = startAddress;
        char *endPtr;
        for(int i=0; i<int(lines.size()); i++)
        {
            uint16_t lineNumber = (uint16_t)strtol(lines[i].c_str(), &endPtr, 10);
            Cpu::setRAM(endAddress + 0, LO_BYTE(lineNumber));
            Cpu::setRAM(endAddress + 1, HI_BYTE(lineNumber));
            uint8_t lineStart = uint8_t(endPtr - &lines[i][0]);

            // First 2 bytes are int16 line number
            for(uint8_t j=lineStart; j<(MAX_GTB_LINE_SIZE-2 + lineStart); j++)
            {
                uint8_t offset = 2 + j - lineStart;
                bool validData = offset < MAX_GTB_LINE_SIZE-1  &&  j < lines[i].size()  &&  lines[i][j] >= ' ';
                uint8_t data = validData ? lines[i][j] : 0;
                Cpu::setRAM(endAddress + offset, data);
            }
            endAddress += MAX_GTB_LINE_SIZE;
            if(LO_BYTE(endAddress) < LO_BYTE(GTB_LINE0_ADDRESS)) endAddress = HI_MASK(endAddress) | LO_BYTE(GTB_LINE0_ADDRESS);
        }

        uint16_t freeMemory = uint16_t(Memory::getFreeGtbRAM(int(lines.size())));
        fprintf(stderr, "Loader::loadGtbFile() : start %04x : end %04x : free %d : '%s'\n", startAddress, endAddress, freeMemory, filepath.c_str());

        Cpu::setRAM(GTB_LINE0_ADDRESS + 0, LO_BYTE(endAddress));
        Cpu::setRAM(GTB_LINE0_ADDRESS + 1, HI_BYTE(endAddress));
        std::string list = "RUN";
        for(int i=0; i<int(list.size()); i++) Cpu::setRAM(uint16_t(endAddress + 2 + i), list[i]);
        Cpu::setRAM(uint16_t(endAddress + 2 + uint16_t(list.size())), 0);

        return true;
    }

    void setMemoryModel64k(void)
    {
        if (Memory::getSizeRAM() < RAM_SIZE_HI)
        {
            Memory::setSizeRAM(RAM_SIZE_HI);
            Cpu::setSizeRAM(Memory::getSizeRAM());
            Cpu::setRAM(0x0001, 0x00); // inform system that RAM is 64k
        }
    }

    void uploadToEmulatorRAM(const Gt1File& gt1File)
    {
        // Set 64k memory model based on any segment address being >= 0x8000
        for(int i=0; i<int(gt1File._segments.size()); i++)
        {
            if(gt1File._segments[i]._loAddress + (gt1File._segments[i]._hiAddress <<8) >= RAM_UPPER_START)
            {
                setMemoryModel64k();
                break;
            }
        }

        for(int j=0; j<int(gt1File._segments.size()); j++)
        {
            // Ignore if address will not fit in current RAM
            uint16_t address = gt1File._segments[j]._loAddress + (gt1File._segments[j]._hiAddress <<8);
            if((address + int(gt1File._segments[j]._dataBytes.size()) - 1) < Memory::getSizeRAM())
            {
                for(int i=0; i<int(gt1File._segments[j]._dataBytes.size()); i++)
                {
                    Cpu::setRAM(uint16_t(address + i), gt1File._segments[j]._dataBytes[i]);
                }
            }
        }
    }

    // Uploads directly into the emulator's RAM/ROM or to real Gigatron hardware if available
    void uploadDirect(UploadTarget uploadTarget)
    {
        Gt1File gt1File;

        bool gt1FileBuilt = false;
        bool isGtbFile = false;
        bool isGt1File = false;
        bool isGbasFile = false;
        bool hasRomCode = false;
        bool hasRamCode = false;

        uint16_t execAddress = USER_CODE_START;
        std::string gtbFilepath;

        if(Editor::getCurrentFileEntryName())
        {
            _filePath = Editor::getBrowserPath() + *Editor::getCurrentFileEntryName();
        }

        std::string filepath = _filePath;
        size_t slash = filepath.find_last_of("\\/");
        std::string filename = filepath.substr(slash + 1);

        Expression::replaceText(filepath, "\\", "/");

        size_t nameSuffix = filename.find_last_of(".");
        size_t pathSuffix = filepath.find_last_of(".");
        if(nameSuffix == std::string::npos  ||  pathSuffix == std::string::npos)
        {
            fprintf(stderr, "\nLoader::uploadDirect() : invalid filepath '%s' or filename '%s'\n", filepath.c_str(), filename.c_str());
            return;
        }

        // Set 64k memory model based on .INI file or filename
        if(_autoSet64k  &&  Expression::strUpper(filename).find("64K") != std::string::npos)
        {
            setMemoryModel64k();
        }

        if(uploadTarget == Emulator)
        {
            Audio::restoreWaveTables();
            Audio::initialiseChannels();
        }

        // Compile gbas to gasm
        if(filename.find(".gbas") != filename.npos)
        {
            Cpu::enable6BitSound(Cpu::ROMv5a, false);

            std::string output = filepath.substr(0, pathSuffix) + ".gasm";
            if(!Compiler::compile(filepath, output))
            {
                //Memory::printFreeRamList(Memory::SizeDescending);
                return;
            }

            // Create gasm name and path
            filename = filename.substr(0, nameSuffix) + ".gasm";
            filepath = filepath.substr(0, pathSuffix) + ".gasm";
            isGbasFile = true;
        }
        // Load to gtb and launch TinyBasic
        else if(_configGclBuildFound  &&  filename.find(".gtb") != filename.npos)
        {
            gtbFilepath = filepath;
            filename = "TinyBASIC_v3.gt1";
            filepath = _configGclBuild + "/Apps/TinyBASIC/" + filename;
            isGtbFile = true;
        }
        // Compile gcl to gt1
        else if(_configGclBuildFound  &&  filename.find(".gcl") != filename.npos)
        {
            // Create compile gcl string
            if(chdir(_configGclBuild.c_str()))
            {
                fprintf(stderr, "\nLoader::uploadDirect() : failed to change directory to '%s' : can't build %s\n", _configGclBuild.c_str(), filename.c_str());
                return;
            }

            // Prepend CWD to relative paths
            if(filepath.find(":") == std::string::npos  &&  filepath[0] != '/')
            {
                filepath = _cwdPath + "/" + filepath;
                pathSuffix = filepath.find_last_of(".");
            }

            slash = filepath.find_last_of("\\/");
            std::string gclPath = (slash != std::string::npos) ? filepath.substr(0, slash) : "./";
            std::string command = "python3 -B Core/compilegcl.py -s interface.json \"" + filepath + "\" \"" + gclPath + "\"";

            //fprintf(stderr, "\nLoader::uploadDirect() : %s : %s : %s : %s\n", filepath.c_str(), command.c_str(), _cwdPath.c_str(), _exePath.c_str());

            // Create gt1 name and path
            filename = filename.substr(0, nameSuffix) + ".gt1";
            filepath = filepath.substr(0, pathSuffix) + ".gt1";

            // Build gcl
            int gt1FileDeleted = remove(filepath.c_str());
            fprintf(stderr, "\n");
            (void)!system(command.c_str());

            // Check for gt1
            std::ifstream infile(filepath, std::ios::binary | std::ios::in);
            if(!infile.is_open())
            {
                fprintf(stderr, "\nLoader::uploadDirect() : failed to compile '%s'\n", filepath.c_str());
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
            Assembler::clearAssembler(true);

            if(!loadGt1File(filepath, gt1File)) return;
            execAddress = gt1File._loStart + (gt1File._hiStart <<8);
            Editor::setLoadBaseAddress(execAddress);

            // Changes memory model if needed then uploads to emulator
            if(uploadTarget == Emulator)
            {
                uploadToEmulatorRAM(gt1File);
            }

            isGt1File = true;
            hasRamCode = true;
            hasRomCode = false;
            _disableUploads = false;
        }
        // Upload vCPU assembly code
        else if(filename.find(".gasm") != filename.npos  ||  filename.find(".vasm") != filename.npos  ||  filename.find(".s") != filename.npos  ||  filename.find(".asm") != filename.npos)
        {
            uint16_t address = (isGbasFile) ? Compiler::getUserCodeStart() : DEFAULT_EXEC_ADDRESS;
            if(!Assembler::assemble(filepath, address, isGbasFile)) return;
            if(isGbasFile  &&  !Validater::checkRuntimeVersion()) return;

            // Found a breakpoint in source code
            if(Editor::getVpcBreakPointsSize())
            {
                Editor::startDebugger();
                Editor::runToBreakpoint();
                Editor::setEditorMode(Editor::Dasm);
            }

            execAddress = (isGbasFile) ? Compiler::getUserCodeStart() : Assembler::getStartAddress();
            uint16_t customAddress = execAddress;
            Editor::setLoadBaseAddress(execAddress);
            address = execAddress;

            // Save to gt1 format
            gt1File._loStart = LO_BYTE(address);
            gt1File._hiStart = HI_BYTE(address);
            Gt1Segment gt1Segment;
            gt1Segment._loAddress = LO_BYTE(address);
            gt1Segment._hiAddress = HI_BYTE(address);

            // Generate gt1File
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
                    gt1Segment._loAddress = LO_BYTE(address);
                    gt1Segment._hiAddress = HI_BYTE(address);
                }

                // Upload any ROM code to emulator ROM
                if(byteCode._isRomAddress  &&  !_disableUploads  &&  uploadTarget == Emulator)
                {
                    Cpu::setROM(customAddress, address++, byteCode._data);
                }

                gt1Segment._dataBytes.push_back(byteCode._data);
            }

            // Last segment
            if(gt1Segment._dataBytes.size())
            {
                gt1Segment._segmentSize = uint8_t(gt1Segment._dataBytes.size());
                gt1File._segments.push_back(gt1Segment);
            }

            // Changes memory model if needed then uploads to emulator RAM
            if(uploadTarget == Emulator)
            {
                uploadToEmulatorRAM(gt1File);
            }

            // Don't save gt1 file for any asm files that contain native rom code
            std::string gt1FileName;
            if(!hasRomCode)
            {
                if(!saveGt1File(filepath, gt1File, gt1FileName))
                {
                    Cpu::reset();
                    return;
                }
            }

            gt1FileBuilt = true;
        }
        // Invalid file
        else
        {
            fprintf(stderr, "Loader::upload() : invalid file or file does not exist '%s'\n", filename.c_str());
            return;
        }

        if(uploadTarget == Emulator) fprintf(stderr, "\nTarget : Emulator\n");
        else if(uploadTarget == Hardware) fprintf(stderr, "\nTarget : Gigatron\n");

        // BASIC calculates the correct value of free RAM as part of the compilation
        printGt1Stats(filename, gt1File, isGbasFile);

        if(uploadTarget == Emulator)
        {
            size_t i = filename.find('.');
            _currentGame = (i != std::string::npos) ? filename.substr(0, i) : filename;
            loadHighScore();

            // Load .gtb file into memory and launch TinyBasic, (TinyBasic has already been loaded)
            if(isGtbFile  &&  gtbFilepath.size())
            {
                loadGtbFile(gtbFilepath);
            }

            // Reset single step watch address to video line counter
            Editor::setSingleStepAddress(FRAME_COUNT_ADDRESS);

            // Execute code
            if(!_disableUploads  &&  hasRamCode)
            {
                // vPC
                Cpu::setRAM(0x0016, LO_BYTE(execAddress-2));
                Cpu::setRAM(0x0017, HI_BYTE(execAddress));

                // vLR
                Cpu::setRAM(0x001a, LO_BYTE(execAddress-2));
                Cpu::setRAM(0x001b, HI_BYTE(execAddress));

                // Reset stack and constants
                Cpu::setRAM(STACK_POINTER, 0x00);
                Cpu::setRAM(ZERO_CONST_ADDRESS, 0x00);
                Cpu::setRAM(ONE_CONST_ADDRESS, 0x01);

                // Reset VBlank and video top
                Cpu::setRAM16(VBLANK_PROC, 0x0000);
                Cpu::setRAM(VIDEO_TOP, 0x00);

                // Reset video table and reset single step watch address to video line counter
                Graphics::resetVTable();
            }
        }
        else if(uploadTarget == Hardware)
        {
            if(!isGt1File)
            {
                size_t i = filepath.rfind('.');
                std::string filepathGt1 = (i != std::string::npos) ? filepath.substr(0, i) + ".gt1" : filepath + ".gt1";
                uploadToGiga(filepathGt1, filename);
            }
            else
            {
                uploadToGiga(filepath, filename);
            }
        }

        // Updates browser in case a new gt1 file was created from a gcl file or a gasm file
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
                    sendByte(LO_BYTE(address), checksum);
                    loaderState = LoaderState::HighAddress;
                }
            }
            break;

            case LoaderState::HighAddress: // 8 bits
            {
                if(vgaY == VSYNC_START+30)
                {
                    sendByte(HI_BYTE(address), checksum);
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

            default: break;
        }

        return sending;
    }

    // TODO: fix the Gigatron version of upload so that it can send more than 60 total bytes, (i.e. break up the payload into multiple packets of 60, 1 packet per frame)
    void upload(int vgaY)
    {
        static bool frameUploading = false;
        static uint8_t payload[RAM_SIZE_HI];
        static uint8_t payloadSize = 0;

        if(frameUploading)
        {
            uint16_t execAddress = Editor::getLoadBaseAddress();

            frameUploading = true;            
            static uint8_t checksum = 0;
            static FrameState frameState = FrameState::Resync;
            switch(frameState)
            {
                case FrameState::Resync:
                {
                    if(!sendFrame(vgaY, 0xFF, payload, payloadSize, execAddress, checksum))
                    {
                        checksum = 'g'; // loader resets checksum
                        frameState = FrameState::Frame;
                    }
                }
                break;

                case FrameState::Frame:
                {
                    if(!sendFrame(vgaY,'L', payload, payloadSize, execAddress, checksum))
                    {
                        frameState = FrameState::Execute;
                    }
                }
                break;

                case FrameState::Execute:
                {
                    if(!sendFrame(vgaY, 'L', payload, 0, execAddress, checksum))
                    {
                        checksum = 0;
                        frameState = FrameState::Resync;
                        frameUploading = false;
                    }
                }
                break;

                default: break;
            }
        }
    }
#endif
}
