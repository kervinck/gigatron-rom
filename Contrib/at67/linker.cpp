#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "linker.h"


namespace Linker
{
    std::map<std::string, std::vector<std::string>> _subIncludeFiles;
    std::map<int, uint16_t> _internalSubMap;

    // TODO: use std::map
    std::vector<Compiler::InternalSub> _internalSubs =
    {
        {0x0000, 0x0000, "romCheck"         , "", false, false},
        {0x0000, 0x0000, "romExec"          , "", false, false},
        {0x0000, 0x0000, "romRead"          , "", false, false},
        {0x0000, 0x0000, "realTimeStub"     , "", false, false},
        {0x0000, 0x0000, "convertEqOp"      , "", false, false},
        {0x0000, 0x0000, "convertNeOp"      , "", false, false},
        {0x0000, 0x0000, "convertLeOp"      , "", false, false},
        {0x0000, 0x0000, "convertGeOp"      , "", false, false},
        {0x0000, 0x0000, "convertLtOp"      , "", false, false},
        {0x0000, 0x0000, "convertGtOp"      , "", false, false},
        {0x0000, 0x0000, "convert8Arr2d"    , "", false, false},
        {0x0000, 0x0000, "convert8Arr3d"    , "", false, false},
        {0x0000, 0x0000, "convert16Arr2d"   , "", false, false},
        {0x0000, 0x0000, "convert16Arr3d"   , "", false, false},
        {0x0000, 0x0000, "setRealTimeProc0" , "", false, false},
        {0x0000, 0x0000, "setRealTimeProc1" , "", false, false},
        {0x0000, 0x0000, "setRealTimeProc2" , "", false, false},
        {0x0000, 0x0000, "loadRegs"         , "", false, false},
        {0x0000, 0x0000, "saveRegs"         , "", false, false},
        {0x0000, 0x0000, "copyBytes"        , "", false, false},
        {0x0000, 0x0000, "copyWords"        , "", false, false},
        {0x0000, 0x0000, "copyLoaderImages" , "", false, false},
        {0x0000, 0x0000, "resetVars"        , "", false, false},
        {0x0000, 0x0000, "absolute"         , "", false, false},
        {0x0000, 0x0000, "sign"             , "", false, false},
        {0x0000, 0x0000, "integerMin"       , "", false, false},
        {0x0000, 0x0000, "integerMax"       , "", false, false},
        {0x0000, 0x0000, "integerClamp"     , "", false, false},
        {0x0000, 0x0000, "power16bit"       , "", false, false},
        {0x0000, 0x0000, "power16bitExt"    , "", false, false},
        {0x0000, 0x0000, "multiply16bit"    , "", false, false},
        {0x0000, 0x0000, "multiply16bit_1"  , "", false, false},
        {0x0000, 0x0000, "divide16bit"      , "", false, false},
        {0x0000, 0x0000, "divide16bit_1"    , "", false, false},
        {0x0000, 0x0000, "rand16bit"        , "", false, false},
        {0x0000, 0x0000, "randMod16bit"     , "", false, false},
        {0x0000, 0x0000, "shiftLeft4bit"    , "", false, false},
        {0x0000, 0x0000, "shiftLeft8bit"    , "", false, false},
        {0x0000, 0x0000, "shiftRight1bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight2bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight3bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight4bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight5bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight6bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight7bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRight8bit"   , "", false, false},
        {0x0000, 0x0000, "shiftRightSgn1bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn2bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn3bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn4bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn5bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn6bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn7bit", "", false, false},
        {0x0000, 0x0000, "shiftRightSgn8bit", "", false, false},
        {0x0000, 0x0000, "getArrayByte"     , "", false, false},
        {0x0000, 0x0000, "setArrayByte"     , "", false, false},
        {0x0000, 0x0000, "getArrayInt16"    , "", false, false},
        {0x0000, 0x0000, "setArrayInt16"    , "", false, false},
        {0x0000, 0x0000, "getArrayInt16Low" , "", false, false},
        {0x0000, 0x0000, "setArrayInt16Low" , "", false, false},
        {0x0000, 0x0000, "getArrayInt16High", "", false, false},
        {0x0000, 0x0000, "setArrayInt16High", "", false, false},
        {0x0000, 0x0000, "readIntVar"       , "", false, false},
        {0x0000, 0x0000, "readStrVar"       , "", false, false},
        {0x0000, 0x0000, "gotoNumericLabel" , "", false, false},
        {0x0000, 0x0000, "gosubNumericLabel", "", false, false},
        {0x0000, 0x0000, "scanlineMode"     , "", false, false},
        {0x0000, 0x0000, "waitVBlank"       , "", false, false},
        {0x0000, 0x0000, "waitVBlanks"      , "", false, false},
        {0x0000, 0x0000, "resetVideoFlags"  , "", false, false},
        {0x0000, 0x0000, "resetVideoTable"  , "", false, false},
        {0x0000, 0x0000, "initClearFuncs"   , "", false, false},
        {0x0000, 0x0000, "clearScreen"      , "", false, false},
        {0x0000, 0x0000, "clearRect"        , "", false, false},
        {0x0000, 0x0000, "clearLine"        , "", false, false},
        {0x0000, 0x0000, "clearVertBlinds"  , "", false, false},
        {0x0000, 0x0000, "clearCursorRow"   , "", false, false},
        {0x0000, 0x0000, "readPixel"        , "", false, false},
        {0x0000, 0x0000, "drawPixel"        , "", false, false},
        {0x0000, 0x0000, "drawHLine"        , "", false, false},
        {0x0000, 0x0000, "drawVLine"        , "", false, false},
        {0x0000, 0x0000, "drawLine"         , "", false, false},
        {0x0000, 0x0000, "drawLineExt"      , "", false, false},
        {0x0000, 0x0000, "drawLineLoop"     , "", false, false},
        {0x0000, 0x0000, "drawLineLoadXY"   , "", false, false},
        {0x0000, 0x0000, "drawLineSlow"     , "", false, false},
        {0x0000, 0x0000, "drawLineSlowExt"  , "", false, false},
        {0x0000, 0x0000, "drawLineSlowLoop" , "", false, false},
        {0x0000, 0x0000, "drawLineSlowSwap" , "", false, false},
        {0x0000, 0x0000, "drawVTLine"       , "", false, false},
        {0x0000, 0x0000, "drawVTLineExt"    , "", false, false},
        {0x0000, 0x0000, "drawVTLineLoop"   , "", false, false},
        {0x0000, 0x0000, "drawVTLineLoadXY" , "", false, false},
        {0x0000, 0x0000, "drawCircle"       , "", false, false},
        {0x0000, 0x0000, "drawCircleExt1"   , "", false, false},
        {0x0000, 0x0000, "drawCircleExt2"   , "", false, false},
        {0x0000, 0x0000, "drawCircleF"      , "", false, false},
        {0x0000, 0x0000, "drawRect"         , "", false, false},
        {0x0000, 0x0000, "drawRectF"        , "", false, false},
        {0x0000, 0x0000, "drawPoly"         , "", false, false},
        {0x0000, 0x0000, "drawPolyRel"      , "", false, false},
        {0x0000, 0x0000, "setPolyRelFlipX"  , "", false, false},
        {0x0000, 0x0000, "setPolyRelFlipY"  , "", false, false},
        {0x0000, 0x0000, "atLineCursor"     , "", false, false},
        {0x0000, 0x0000, "drawSprite_"      , "", false, false},
        {0x0000, 0x0000, "drawSprite"       , "", false, false},
        {0x0000, 0x0000, "drawSpriteX"      , "", false, false},
        {0x0000, 0x0000, "drawSpriteY"      , "", false, false},
        {0x0000, 0x0000, "drawSpriteXY"     , "", false, false},
        {0x0000, 0x0000, "getSpriteLUT"     , "", false, false},
        {0x0000, 0x0000, "setMidiStream"    , "", false, false},
        {0x0000, 0x0000, "resetMidi"        , "", false, false},
        {0x0000, 0x0000, "playMidi"         , "", false, false},
        {0x0000, 0x0000, "playMidiVol"      , "", false, false},
        {0x0000, 0x0000, "midiStartNote"    , "", false, false},
        {0x0000, 0x0000, "midiGetNote"      , "", false, false},
        {0x0000, 0x0000, "resetMusic"       , "", false, false},
        {0x0000, 0x0000, "playMusic"        , "", false, false},
        {0x0000, 0x0000, "musicGetNote"     , "", false, false},
        {0x0000, 0x0000, "musicPlayNote"    , "", false, false},
        {0x0000, 0x0000, "soundAllOff"      , "", false, false},
        {0x0000, 0x0000, "soundOff"         , "", false, false},
        {0x0000, 0x0000, "soundOn"          , "", false, false},
        {0x0000, 0x0000, "soundOnV"         , "", false, false},
        {0x0000, 0x0000, "soundMod"         , "", false, false},
        {0x0000, 0x0000, "input"            , "", false, false},
        {0x0000, 0x0000, "inputExt1"        , "", false, false},
        {0x0000, 0x0000, "inputExt2"        , "", false, false},
        {0x0000, 0x0000, "inputCursor"      , "", false, false},
        {0x0000, 0x0000, "inputKeys"        , "", false, false},
        {0x0000, 0x0000, "inputIntVar"      , "", false, false},
        {0x0000, 0x0000, "inputStrVar"      , "", false, false},
        {0x0000, 0x0000, "inputReturn"      , "", false, false},
        {0x0000, 0x0000, "inputDelete"      , "", false, false},
        {0x0000, 0x0000, "inputPrint"       , "", false, false},
        {0x0000, 0x0000, "inputNewline"     , "", false, false},
        {0x0000, 0x0000, "printInit"        , "", false, false},
        {0x0000, 0x0000, "printText"        , "", false, false},
        {0x0000, 0x0000, "printLeft"        , "", false, false},
        {0x0000, 0x0000, "printRight"       , "", false, false},
        {0x0000, 0x0000, "printMid"         , "", false, false},
        {0x0000, 0x0000, "printLower"       , "", false, false},
        {0x0000, 0x0000, "printUpper"       , "", false, false},
        {0x0000, 0x0000, "printDigit"       , "", false, false},
        {0x0000, 0x0000, "printInt16"       , "", false, false},
        {0x0000, 0x0000, "printChr"         , "", false, false},
        {0x0000, 0x0000, "printChar"        , "", false, false},
        {0x0000, 0x0000, "printClip"        , "", false, false},
        {0x0000, 0x0000, "printHex"         , "", false, false},
        {0x0000, 0x0000, "printSpc"         , "", false, false},
        {0x0000, 0x0000, "atTextCursor"     , "", false, false},
        {0x0000, 0x0000, "newLineScroll"    , "", false, false},
        {0x0000, 0x0000, "integerStr"       , "", false, false},
        {0x0000, 0x0000, "stringChr"        , "", false, false},
        {0x0000, 0x0000, "stringSpc"        , "", false, false},
        {0x0000, 0x0000, "stringHex"        , "", false, false},
        {0x0000, 0x0000, "stringCopy"       , "", false, false},
        {0x0000, 0x0000, "stringCmp"        , "", false, false},
        {0x0000, 0x0000, "stringAdd"        , "", false, false},
        {0x0000, 0x0000, "stringConcat"     , "", false, false},
        {0x0000, 0x0000, "stringConcatLut"  , "", false, false},
        {0x0000, 0x0000, "stringLeft"       , "", false, false},
        {0x0000, 0x0000, "stringRight"      , "", false, false},
        {0x0000, 0x0000, "stringMid"        , "", false, false},
        {0x0000, 0x0000, "stringLower"      , "", false, false},
        {0x0000, 0x0000, "stringUpper"      , "", false, false},
        {0x0000, 0x0000, "stringDigit"      , "", false, false},
        {0x0000, 0x0000, "stringInt"        , "", false, false},
        {0x0000, 0x0000, "tickTime"         , "", false, false},
        {0x0000, 0x0000, "handleTime"       , "", false, false},
        {0x0000, 0x0000, "timeDigits"       , "", false, false},
        {0x0000, 0x0000, "timeString"       , "", false, false},
        {0x0000, 0x0000, "bcdAdd"           , "", false, false},
        {0x0000, 0x0000, "bcdSub"           , "", false, false},
        {0x0000, 0x0000, "bcdInt"           , "", false, false},
        {0x0000, 0x0000, "bcdDigits"        , "", false, false},
        {0x0000, 0x0000, "bcdCmp"           , "", false, false},
        {0x0000, 0x0000, "bcdCmpExt"        , "", false, false},
        {0x0000, 0x0000, "bcdCpy"           , "", false, false},
    };
    std::vector<std::string> _subIncludesROMv1 =
    {
        "math.i"        ,
        "memory.i"      ,
        "flow_control.i",
        "clear_screen.i",
        "conv_conds.i"  ,
        "graphics.i"    ,
        "audio.i"       ,
        "input.i"       ,
        "print_text.i"  ,
        "string.i"      ,
        "numeric.i"     ,
        "time.i"        ,
    };
    std::vector<std::string> _subIncludesROMv2 = 
    {
        "math.i"              ,
        "memory.i"            ,
        "flow_control.i"      ,
        "clear_screen_ROMv2.i",
        "conv_conds.i"        ,
        "graphics_ROMv2.i"    ,
        "audio.i"             ,
        "input.i"             ,
        "print_text_ROMv2.i"  ,
        "string.i"            ,
        "numeric.i"           ,
        "time.i"              ,
    };
    std::vector<std::string> _subIncludesROMv3 =
    {
        "math.i"              ,
        "memory.i"            ,
        "flow_control.i"      ,
        "clear_screen_ROMv2.i",
        "conv_conds.i"        ,
        "graphics_ROMv2.i"    ,
        "sprite_ROMv3.i"      ,
        "audio.i"             ,
        "input.i"             ,
        "print_text_ROMv3.i"  ,
        "string.i"            ,
        "numeric.i"           ,
        "time.i"              ,
    };
    std::vector<std::string> _subIncludesROMv4 =
    {
        "math.i"              ,
        "memory.i"            ,
        "flow_control.i"      ,
        "clear_screen_ROMv2.i",
        "conv_conds.i"        ,
        "graphics_ROMv2.i"    ,
        "sprite_ROMv3.i"      ,
        "audio.i"             ,
        "input.i"             ,
        "print_text_ROMv3.i"  ,
        "numeric.i"           ,
        "time.i"              ,
    };
    std::vector<std::string> _subIncludesROMv5a =
    {
        "math_ROMv5a.i"        ,
        "memory_ROMv5a.i"      ,
        "flow_control_ROMv5a.i",
        "clear_screen_ROMv5a.i",
        "conv_conds_ROMv5a.i"  ,
        "graphics_ROMv5a.i"    ,
        "sprite_ROMv5a.i"      ,
        "audio_ROMv5a.i"       ,
        "input_ROMv5a.i"       ,
        "print_text_ROMv5a.i"  ,
        "string_ROMv5a.i"      ,
        "numeric_ROMv5a.i"     ,
        "time_ROMv5a.i"        ,
    };


    bool getInternalSub(const std::string& name, Compiler::InternalSub& internalSub)
    {
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(_internalSubs[i]._name == name)
            {
                internalSub = _internalSubs[i];
                return true;
            }
        }

        return false;
    }

    bool setInternalSub(const std::string& name, const Compiler::InternalSub& internalSub)
    {
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(_internalSubs[i]._name == name)
            {
                _internalSubs[i] = internalSub;
                return true;
            }
        }

        return false;
    }

    bool setInternalSubToLoad(const std::string& name)
    {
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(_internalSubs[i]._name == name)
            {
                _internalSubs[i]._inUse = true;
                return true;
            }
        }

        return false;
    }


    void resetIncludeFiles(void)
    {
        _subIncludeFiles.clear();
    }

    void reset(void)
    {
        resetIncludeFiles();

        _internalSubMap.clear();

        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            _internalSubs[i]._size = 0;
            _internalSubs[i]._address = 0;
            _internalSubs[i]._includeName = "";
            _internalSubs[i]._loaded = false;
            _internalSubs[i]._inUse = false;
        }
    }

    bool initialise(void)
    {
        return true;
    }


    bool enableFontLinking(void)
    {
        resetIncludeFiles();

        for(int i=0; i<int(_subIncludesROMv3.size()); i++)
        {
            size_t textPos = _subIncludesROMv3[i].find("text");
            if(textPos != std::string::npos)
            {
                _subIncludesROMv3[i].replace(textPos, 4, "font");
            }
        }

        for(int i=0; i<int(_subIncludesROMv5a.size()); i++)
        {
            size_t textPos = _subIncludesROMv5a[i].find("text");
            if(textPos != std::string::npos)
            {
                _subIncludesROMv5a[i].replace(textPos, 4, "font");
            }
        }

        return parseIncludes();
    }

    bool disableFontLinking(void)
    {
        for(int i=0; i<int(_subIncludesROMv3.size()); i++)
        {
            size_t textPos = _subIncludesROMv3[i].find("font");
            if(textPos != std::string::npos)
            {
                _subIncludesROMv3[i].replace(textPos, 4, "text");
            }
        }

        for(int i=0; i<int(_subIncludesROMv5a.size()); i++)
        {
            size_t textPos = _subIncludesROMv5a[i].find("font");
            if(textPos != std::string::npos)
            {
                _subIncludesROMv5a[i].replace(textPos, 4, "text");
            }
        }

        return true;
    }

    bool findSub(const std::vector<std::string>& tokens, const std::string& subName)
    {
        for(int i=0; i<int(tokens.size()); i++)
        {
            if(tokens[i] == subName) return true;
        }

        return false;
    }

    int getAsmOpcodeSizeOfIncludeSub(const std::string& includeName, const std::string& subName)
    {
        if(_subIncludeFiles.find(includeName) == _subIncludeFiles.end())
        {
            fprintf(stderr, "Linker::getAsmOpcodeSizeOfIncludeSub() : Include file was never loaded : '%s'\n", includeName.c_str());
            return -1;
        }

        // Find sub in include
        int numLines = 0;
        int vasmSize = 0;
        bool buildingSub = false;
        for(int i=0; i<int(_subIncludeFiles[includeName].size()); i++)
        {
            std::vector<std::string> tokens = Expression::tokeniseLine(_subIncludeFiles[includeName][i]);
            for(int j=0; j<int(tokens.size()); j++) Expression::stripWhitespace(tokens[j]);
            if(!buildingSub  &&  tokens.size() >= 2  &&  tokens[0] == "%SUB"  &&  tokens[1] == subName)
            {
                buildingSub = true;
            }
            else if(buildingSub  &&  tokens.size() >= 1  &&  tokens[0] == "%ENDS")
            {
                buildingSub = false;
                break;
            }
            else if(buildingSub)
            {
                for(int j=0; j<int(tokens.size()); j++)
                {
                    if(tokens[j].find_first_of(";#") != std::string::npos) break;
                    int size = Assembler::getAsmOpcodeSize(tokens[j]);
                    vasmSize += (size == 0) ? Compiler::getMacroSize(tokens[j]) : size;
                }
            }

            numLines++;
        }

        //if(vasmSize) fprintf(stderr, "%s : %s : opcode size : %d\n", filename.c_str(), subName.c_str(), vasmSize);

        return vasmSize;
    }

    bool getIncludeSubSize(const std::string& includeName, int subIndex)
    {
        uint16_t size = uint16_t(getAsmOpcodeSizeOfIncludeSub(includeName, _internalSubs[subIndex]._name));
        if(size)
        {
            _internalSubs[subIndex]._size = size;
            _internalSubs[subIndex]._includeName = includeName;
            return true;
        }

        return false;
    }

    bool getInternalSubCode(const std::string& includeName, const std::vector<std::string>& includeVarsDone, std::vector<std::string>& code, int subIndex)
    {
        if(_subIncludeFiles.find(includeName) == _subIncludeFiles.end())
        {
            fprintf(stderr, "Linker::getInternalSubCode() : Include file was never loaded : '%s'\n", includeName.c_str());
            return false;
        }

        std::string subName = _internalSubs[subIndex]._name;

        // Check if include vars already done
        bool varsDone = false;
        for(int i=0; i<int(includeVarsDone.size()); i++)
        {
            if(includeVarsDone[i] == includeName)
            {
                varsDone = true;
                break;
            }
        }

        // Find sub in include
        int numLines = 0;
        bool buildingSub = false;
        for(int i=0; i<int(_subIncludeFiles[includeName].size()); i++)
        {
            std::string line = _subIncludeFiles[includeName][i];
            std::vector<std::string> tokens = Expression::tokenise(line, ' ');
            for(int j=0; j<int(tokens.size()); j++) Expression::stripWhitespace(tokens[j]);

            bool foundSub = (line.find("%SUB") != std::string::npos);
            bool foundEnd = (line.find("%ENDS") != std::string::npos);

            if(!buildingSub  &&  foundSub)
            {
                varsDone = true;
                if(findSub(tokens, subName)) buildingSub = true;
            }
            else if(buildingSub  &&  foundEnd)
            {
                _internalSubs[subIndex]._loaded = true;
                buildingSub = false;
                return false;
            }
            else if(buildingSub)
            {
                code.push_back(line);
                for(int j=0; j<int(_internalSubs.size()); j++)
                {
                    if(!_internalSubs[j]._inUse  &&  findSub(tokens, _internalSubs[j]._name))
                    {
                        _internalSubs[j]._inUse = true;
                        return true;
                    }
                }
            }
            // Save all lines up until the first %SUB
            else if(!buildingSub  &&  !varsDone  &&  !foundSub  &&  !foundEnd)
            {
                code.push_back(line);
            }
            else if(!buildingSub  &&  !varsDone  &&  line.find_first_not_of("  \n\r\f\t\v") == std::string::npos)
            {
                code.push_back("\n");
            }

            numLines++;
        }

        return false;
    }

    // Marks internal sub to be loaded later
    bool loadInternalSub(int subIndex, bool overwrite=false)
    {
        if(!overwrite  &&  _internalSubs[subIndex]._address) return false;
        _internalSubMap[subIndex] = _internalSubs[subIndex]._size;

        _internalSubs[subIndex]._address = 0xFFFF;
        _internalSubs[subIndex]._inUse = true;
        return true;
    }

    bool loadInclude(const std::string& filename)
    {
        // Include file already loaded
        if(_subIncludeFiles.find(filename) != _subIncludeFiles.end()) return true;

        std::ifstream infile(Assembler::getIncludePath() + "/" + filename);
        if(!infile.is_open())
        {
            fprintf(stderr, "Linker::loadInclude() : Failed to open file : '%s'\n", filename.c_str());
            return false;
        }

        int numLines = 0;
        std::vector<std::string> lineTokens;
        while(!infile.eof())
        {
            std::string lineToken;
            std::getline(infile, lineToken);
            lineTokens.push_back(lineToken);

            if(!infile.good() && !infile.eof())
            {
                fprintf(stderr, "Linker::loadInclude() : Bad lineToken : '%s' : in '%s' : on line %d\n", lineToken.c_str(), filename.c_str(), numLines+1);
                return false;
            }

            numLines++;
        }

        _subIncludeFiles[filename] = lineTokens;

        return true;
    }

    bool parseIncludes(void)
    {
        // Load include files into memory
        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
        {
            for(int i=0; i<int(_subIncludesROMv5a.size()); i++)
            {
                if(!loadInclude(_subIncludesROMv5a[i])) return false;
            }
        }
        else if(Compiler::getCodeRomType() >= Cpu::ROMv4)
        {
            for(int i=0; i<int(_subIncludesROMv4.size()); i++)
            {
                if(!loadInclude(_subIncludesROMv4[i])) return false;
            }
        }
        else if(Compiler::getCodeRomType() >= Cpu::ROMv3)
        {
            for(int i=0; i<int(_subIncludesROMv3.size()); i++)
            {
                if(!loadInclude(_subIncludesROMv3[i])) return false;
            }
        }
        else if(Compiler::getCodeRomType() >= Cpu::ROMv2)
        {
            for(int i=0; i<int(_subIncludesROMv2.size()); i++)
            {
                if(!loadInclude(_subIncludesROMv2[i])) return false;
            }
        }
        else
        {
            for(int i=0; i<int(_subIncludesROMv1.size()); i++)
            {
                if(!loadInclude(_subIncludesROMv1[i])) return false;
            }
        }

        // Parse loaded includes
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
            {
                for(int j=0; j<int(_subIncludesROMv5a.size()); j++)
                {
                    if(getIncludeSubSize(_subIncludesROMv5a[j], i)) break;
                }
            }
            else if(Compiler::getCodeRomType() >= Cpu::ROMv4)
            {
                for(int j=0; j<int(_subIncludesROMv4.size()); j++)
                {
                    if(getIncludeSubSize(_subIncludesROMv4[j], i)) break;
                }
            }
            else if(Compiler::getCodeRomType() >= Cpu::ROMv3)
            {
                for(int j=0; j<int(_subIncludesROMv3.size()); j++)
                {
                    if(getIncludeSubSize(_subIncludesROMv3[j], i)) break;
                }
            }
            else if(Compiler::getCodeRomType() >= Cpu::ROMv2)
            {
                for(int j=0; j<int(_subIncludesROMv2.size()); j++)
                {
                    if(getIncludeSubSize(_subIncludesROMv2[j], i)) break;
                }
            }
            else if(Compiler::getCodeRomType() >= Cpu::ROMv1)
            {
                for(int j=0; j<int(_subIncludesROMv1.size()); j++)
                {
                    if(getIncludeSubSize(_subIncludesROMv1[j], i)) break;
                }
            }
        }

        return true;
    }

    bool linkInternalSubs(void)
    {
        fprintf(stderr, "\n*******************************************************\n");
        fprintf(stderr, "*                      Linking                         \n");
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "*        Name          : Address :    Size             \n");
        fprintf(stderr, "*******************************************************\n");

        Assembler::clearDefines();

        for(int i=0; i<int(Compiler::getCodeLines().size()); i++)
        {
            // Valid BASIC code
            if(Compiler::getCodeLines()[i]._code.size() >= 2  &&  Compiler::getCodeLines()[i]._vasm.size())
            {
                // Vasm code
                for(int j=0; j<int(Compiler::getCodeLines()[i]._vasm.size()); j++)
                {
                    std::vector<std::string> tokens = Expression::tokenise(Compiler::getCodeLines()[i]._vasm[j]._code, ' ');
                    for(int k=0; k<int(tokens.size()); k++) Expression::stripWhitespace(tokens[k]);

                    // Handle %define, %if, %else and %endif
                    if(tokens.size())
                    {
                        std::string token = Expression::strUpper(tokens[0]);
                        if(token == "%DEFINE")
                        {
                            // Create define
                            if(!Assembler::createDefine(Compiler::getCodeLines()[i]._text, tokens, i)) return false;
                            continue;
                        }
                        else if(token == "%IF")
                        {
                            if(!Assembler::handleIfDefine(Compiler::getCodeLines()[i]._text, tokens, i)) return false;
                            continue;
                        }
                        else if(token == "%ENDIF")
                        {
                            if(!Assembler::handleEndIfDefine(Compiler::getCodeLines()[i]._text, tokens, i)) return false;
                            continue;
                        }
                        else if(token == "%ELSE")
                        {
                            if(!Assembler::handleElseDefine(Compiler::getCodeLines()[i]._text, tokens, i)) return false;
                            continue;
                        }
                    }

                    // If current define exists and is disabled, then skip current line
                    if(Assembler::isCurrentDefineDisabled()) continue;

                    // Check for internal subs in code and macros
                    for(int k=0; k<int(_internalSubs.size()); k++)
                    {
                        // Code
                        if(findSub(tokens, _internalSubs[k]._name))
                        {
                            loadInternalSub(k);
                        }

                        // Macros, (even nested)
                        std::string opcode = Compiler::getCodeLines()[i]._vasm[j]._opcode;
                        if(opcode.size()  &&  opcode[0] == '%')
                        {
                            opcode.erase(0, 1);
                            if(Compiler::findMacroText(opcode, _internalSubs[k]._name))
                            {
                                loadInternalSub(k);
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    void collectInternalRuntime(void)
    {
        std::vector<std::string> includeVarsDone;

RESTART_COLLECTION:
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(_internalSubs[i]._inUse  &&  !_internalSubs[i]._loaded)
            {
                Compiler::getRuntime().push_back("\n");
                std::vector<std::string> code;

                // This is a BASIC compiler, it can't possibly work without at least one GOTO
                if(getInternalSubCode(_internalSubs[i]._includeName, includeVarsDone, code, i)) goto RESTART_COLLECTION;

                includeVarsDone.push_back(_internalSubs[i]._includeName);

                for(int j=0; j<int(code.size()); j++)
                {
                    Compiler::getRuntime().push_back(code[j] + "\n");
                }
                Compiler::getRuntime().push_back("\n");
            }
        }
    }

    void relinkInternalSubs(void)
    {
        uint16_t runtimeSize = 0;
        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            // Check for internal sub directly
            if(_internalSubs[i]._inUse  &&  _internalSubs[i]._loaded  &&  _internalSubs[i]._address == 0x0000)
            {
                loadInternalSub(i);
            }

            // Runtime size
            if(_internalSubs[i]._inUse  &&  _internalSubs[i]._loaded  &&  _internalSubs[i]._address) runtimeSize += _internalSubs[i]._size;
        }

        // Copy map to vector
        std::vector<SubIndexSize> internalSubs;
        std::copy(_internalSubMap.begin(), _internalSubMap.end(), std::back_inserter<std::vector<SubIndexSize>>(internalSubs));
        
        // Sort vector, (prioritise by sub size)
        std::sort(internalSubs.begin(), internalSubs.end(), [](const SubIndexSize& a, const SubIndexSize& b)
        {
            if(a.second != b.second) return (a.second > b.second);
            return (a.first < b.first);
        });

        // Allocate memory for runtime
        for(auto it=internalSubs.begin(); it!=internalSubs.end(); ++it)
        {
            uint16_t address;
            if(Memory::getNextCodeAddress(Memory::FitDescending, Compiler::getRuntimeStart(),  _internalSubs[it->first]._size, address))
            {
                Memory::takeFreeRAM(address, _internalSubs[it->first]._size, true);

                // Save end of runtime/strings
                if(address < Compiler::getRuntimeEnd()) Compiler::setRuntimeEnd(address);

                fprintf(stderr, "* %-20s : 0x%04x  :    %2d bytes\n", _internalSubs[it->first]._name.c_str(), address, _internalSubs[it->first]._size);

                _internalSubs[it->first]._address = address;
            }
        }

        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "*                     Re-Linking                       \n");
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "*      Start           :  End    :    Size             \n");
        fprintf(stderr, "*******************************************************\n");
        fprintf(stderr, "*      0x%04x          : 0x%04x  : %5d bytes           \n", Compiler::getRuntimeStart() & (Memory::getSizeRAM() - 1), Compiler::getRuntimeEnd(), runtimeSize);
        fprintf(stderr, "*******************************************************\n");
    }

    void outputInternalSubs(void)
    {
        Compiler::getOutput().push_back("\n");
        Compiler::getOutput().push_back(";****************************************************************************************************************************************\n");
        Compiler::getOutput().push_back(";****************************************************************************************************************************************\n");
        Compiler::getOutput().push_back(";* Internal runtime, DO NOT MODIFY PAST THIS POINT, modifications must be made in the original include files                            *\n");  
        Compiler::getOutput().push_back(";****************************************************************************************************************************************\n");
        Compiler::getOutput().push_back(";****************************************************************************************************************************************\n");
        Compiler::getOutput().push_back("\n");

        for(int i=0; i<int(_internalSubs.size()); i++)
        {
            if(_internalSubs[i]._inUse)
            {
                Compiler::getOutput().push_back(_internalSubs[i]._name + std::string(LABEL_TRUNC_SIZE - _internalSubs[i]._name.size(), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(_internalSubs[i]._address) + "\n");
            }
        }

        // Zero page call table is not needed when using ROMv5a and higher
        if(Compiler::getCodeRomType() < Cpu::ROMv5a)
        {
            // Convert relational operators
            {Compiler::getOutput().push_back("convertEqOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertEqOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 0)  + "\n");}
            {Compiler::getOutput().push_back("convertNeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertNeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 2)  + "\n");}
            {Compiler::getOutput().push_back("convertLeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 4)  + "\n");}
            {Compiler::getOutput().push_back("convertGeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 6)  + "\n");}
            {Compiler::getOutput().push_back("convertLtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 8)  + "\n");}
            {Compiler::getOutput().push_back("convertGtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_CC_OPS + 10) + "\n");}

            // Get array address
            {Compiler::getOutput().push_back("convert8Arr2dAddr"  + std::string(LABEL_TRUNC_SIZE - strlen("convert8Arr2dAddr"), ' ')  + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_ARRAY + 0) + "\n");}
            {Compiler::getOutput().push_back("convert8Arr3dAddr"  + std::string(LABEL_TRUNC_SIZE - strlen("convert8Arr3dAddr"), ' ')  + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_ARRAY + 2) + "\n");}
            {Compiler::getOutput().push_back("convert16Arr2dAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convert16Arr2dAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_ARRAY + 4) + "\n");}
            {Compiler::getOutput().push_back("convert16Arr3dAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convert16Arr3dAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(CONVERT_ARRAY + 6) + "\n");}

            // Real time proc
            {Compiler::getOutput().push_back("realTimeStubAddr" + std::string(LABEL_TRUNC_SIZE - strlen("realTimeStubAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(REAL_TIME_PROC) + "\n");}
        }

        Compiler::getOutput().push_back("\n");

        for(int i=0; i<int(Compiler::getRuntime().size()); i++) Compiler::getOutput().push_back(Compiler::getRuntime()[i]);
    }
}