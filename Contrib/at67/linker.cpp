#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
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

    std::vector<Compiler::InternalSub> _internalSubs =
    {
        {0x0000, 0x0000, "realTimeProc"     , "", true,  false},
        {0x0000, 0x0000, "convertEqOp"      , "", true,  false},
        {0x0000, 0x0000, "convertNeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertLeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertGeOp"      , "", true,  false},
        {0x0000, 0x0000, "convertLtOp"      , "", true,  false},
        {0x0000, 0x0000, "convertGtOp"      , "", true,  false},
        {0x0000, 0x0000, "power16bit"       , "", false, false}, 
        {0x0000, 0x0000, "power16bitExt"    , "", false, false}, 
        {0x0000, 0x0000, "multiply16bit"    , "", false, false}, 
        {0x0000, 0x0000, "divide16bit"      , "", false, false},
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
        {0x0000, 0x0000, "gotoNumericLabel" , "", false, false},
        {0x0000, 0x0000, "gosubNumericLabel", "", false, false},
        {0x0000, 0x0000, "scanlineMode"     , "", false, false},
        {0x0000, 0x0000, "waitVBlank"       , "", false, false},
        {0x0000, 0x0000, "resetVideoTable"  , "", false, false},
        {0x0000, 0x0000, "initClearFuncs"   , "", false, false},
        {0x0000, 0x0000, "clearScreen"      , "", false, false},
        {0x0000, 0x0000, "clearVertBlinds"  , "", false, false},
        {0x0000, 0x0000, "clearRVertBlinds" , "", false, false},
        {0x0000, 0x0000, "clearCursorRow"   , "", false, false},
        {0x0000, 0x0000, "drawHLine"        , "", false, false},
        {0x0000, 0x0000, "drawVLine"        , "", false, false},
        {0x0000, 0x0000, "drawLine"         , "", false, false},
        {0x0000, 0x0000, "drawLineExt"      , "", false, false},
        {0x0000, 0x0000, "drawLineLoop"     , "", false, false},
        {0x0000, 0x0000, "drawLineLoadXY"   , "", false, false},
        {0x0000, 0x0000, "drawVTLine"       , "", false, false},
        {0x0000, 0x0000, "drawVTLineExt"    , "", false, false},
        {0x0000, 0x0000, "drawVTLineLoop"   , "", false, false},
        {0x0000, 0x0000, "drawVTLineLoadXY" , "", false, false},
        {0x0000, 0x0000, "atLineCursor"     , "", false, false},
        {0x0000, 0x0000, "resetAudio"       , "", false, false},
        {0x0000, 0x0000, "playMidi"         , "", false, false},
        {0x0000, 0x0000, "midiStartNote"    , "", false, false},
        {0x0000, 0x0000, "printInit"        , "", false, false},
        {0x0000, 0x0000, "printText"        , "", false, false},
        {0x0000, 0x0000, "printLeft"        , "", false, false},
        {0x0000, 0x0000, "printRight"       , "", false, false},
        {0x0000, 0x0000, "printMid"         , "", false, false},
        {0x0000, 0x0000, "printDigit"       , "", false, false},
        {0x0000, 0x0000, "printInt16"       , "", false, false},
        {0x0000, 0x0000, "printChr"         , "", false, false},
        {0x0000, 0x0000, "printChar"        , "", false, false},
        {0x0000, 0x0000, "printHexByte"     , "", false, false},
        {0x0000, 0x0000, "printHexWord"     , "", false, false},
        {0x0000, 0x0000, "atTextCursor"     , "", false, false},
        {0x0000, 0x0000, "newLineScroll"    , "", false, false},
        {0x0000, 0x0000, "stringChr"        , "", false, false},
        {0x0000, 0x0000, "stringHex"        , "", false, false},
        {0x0000, 0x0000, "stringHexw"       , "", false, false},
        {0x0000, 0x0000, "createHex"        , "", false, false},
        {0x0000, 0x0000, "stringCopy"       , "", false, false},
        {0x0000, 0x0000, "stringAdd"        , "", false, false},
        {0x0000, 0x0000, "stringMid"        , "", false, false},
        {0x0000, 0x0000, "stringLeft"       , "", false, false},
        {0x0000, 0x0000, "stringRight"      , "", false, false},
    };
    const std::vector<std::string> _subIncludes = 
    {
        "math.i"        ,
        "memory.i"      ,
        "flow_control.i",
        "clear_screen.i",
        "conv_conds.i"  ,
        "graphics.i"    ,
        "audio.i"       ,
        "print_text.i"  ,
        "string.i"      ,
    };
    const std::vector<std::string> _subIncludesCALLI = 
    {
        "math_CALLI.i"        ,
        "memory_CALLI.i"      ,
        "flow_control_CALLI.i",
        "clear_screen_CALLI.i",
        "conv_conds_CALLI.i"  ,
        "graphics_CALLI.i"    ,
        "audio_CALLI.i"       ,
        "print_text_CALLI.i"  ,
        "string_CALLI.i"      ,
    };


    bool initialise(void)
    {
        return true;
    }


    bool findSub(const std::vector<std::string>& tokens, const std::string& subName)
    {
        for(int i=0; i<tokens.size(); i++)
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
        for(int i=0; i<_subIncludeFiles[includeName].size(); i++)
        {
            std::vector<std::string> tokens = Expression::tokeniseLine(_subIncludeFiles[includeName][i]);
            for(int j=0; j<tokens.size(); j++) Expression::stripWhitespace(tokens[j]);
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
                for(int j=0; j<tokens.size(); j++)
                {
                    std::string token = tokens[j];
                    if(tokens[j].find_first_of(";#") != std::string::npos) break;
                    int size = Assembler::getAsmOpcodeSize(token);
                    vasmSize += (size == 0) ? Compiler::getMacroSize(token) : size;
                }
            }

            numLines++;
        }

        //if(vasmSize) fprintf(stderr, "%s : %s : opcode size : %d\n", filename.c_str(), subName.c_str(), vasmSize);

        return vasmSize;
    }

    bool getIncludeSubSize(const std::string& includeName, int subIndex)
    {
        uint16_t size = getAsmOpcodeSizeOfIncludeSub(includeName, _internalSubs[subIndex]._name);
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
        for(int i=0; i<includeVarsDone.size(); i++)
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
        for(int i=0; i<_subIncludeFiles[includeName].size(); i++)
        {
            std::string line = _subIncludeFiles[includeName][i];
            std::vector<std::string> tokens = Expression::tokenise(line, ' ');
            for(int j=0; j<tokens.size(); j++) Expression::stripWhitespace(tokens[j]);

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
                for(int j=0; j<_internalSubs.size(); j++)
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

    bool loadInternalSub(int subIndex, bool overwrite=false)
    {
        if(!overwrite  &&  _internalSubs[subIndex]._address) return false;

        uint16_t address;
        if(Memory::giveFreeRAM(Memory::FitAscending, _internalSubs[subIndex]._size, 0x0200, Compiler::getRuntimeStart(), address))
        {
            // Save end of runtime/strings
            if(address < Compiler::getRuntimeEnd()) Compiler::setRuntimeEnd(address);

            fprintf(stderr, "%-18s  :  0x%04x  :  %2d bytes\n", _internalSubs[subIndex]._name.c_str(), address, _internalSubs[subIndex]._size);

            _internalSubs[subIndex]._address = address;
            _internalSubs[subIndex]._inUse = true;
            return true;
        }
        else
        {
            fprintf(stderr, "Compiler::loadInternalSub() : Not enough RAM for %s of size %d\n", _internalSubs[subIndex]._name.c_str(), _internalSubs[subIndex]._size);
            return false;
        }

        return false;
    }

    bool loadInclude(const std::string& filename)
    {
        // Include file already loaded
        if(_subIncludeFiles.find(filename) != _subIncludeFiles.end()) return true;

        std::string path = Assembler::getIncludePath() + "/include/";
        std::ifstream infile(path + filename);
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
        for(int i=0; i<_subIncludes.size(); i++)
        {
            if(!Assembler::getUseOpcodeCALLI())
            {
                if(!loadInclude(_subIncludes[i])) return false;
            }
            else
            {
                if(!loadInclude(_subIncludesCALLI[i])) return false;
            }
        }

        // Parse loaded includes
        for(int i=0; i<_internalSubs.size(); i++)
        {
            if(!Assembler::getUseOpcodeCALLI())
            {
                for(int j=0; j<_subIncludes.size(); j++)
                {
                    if(getIncludeSubSize(_subIncludes[j], i)) break;
                }
            }
            else
            {
                for(int j=0; j<_subIncludesCALLI.size(); j++)
                {
                    if(getIncludeSubSize(_subIncludesCALLI[j], i)) break;
                }
            }
        }

        return true;
    }

    bool linkInternalSubs(void)
    {
        for(int i=0; i<Compiler::getCodeLines().size(); i++)
        {
            // Valid BASIC code
            if(Compiler::getCodeLines()[i]._code.size() >= 2  &&  Compiler::getCodeLines()[i]._vasm.size())
            {
                // Vasm code
                for(int j=0; j<Compiler::getCodeLines()[i]._vasm.size(); j++)
                {
                    std::vector<std::string> tokens = Expression::tokenise(Compiler::getCodeLines()[i]._vasm[j]._code, ' ');
                    for(int k=0; k<tokens.size(); k++) Expression::stripWhitespace(tokens[k]);

                    for(int k=0; k<_internalSubs.size(); k++)
                    {
                        // Check for internal subs in code
                        if(findSub(tokens, _internalSubs[k]._name)) loadInternalSub(k);

                        // Check for internal subs in macros, (even nested)
                        std::string opcode = Compiler::getCodeLines()[i]._vasm[j]._opcode;
                        if(opcode.size()  &&  opcode[0] == '%')
                        {
                            opcode.erase(0, 1);
                            if(Compiler::findMacroText(opcode, _internalSubs[k]._name)) loadInternalSub(k);
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
        for(int i=0; i<_internalSubs.size(); i++)
        {
            if(_internalSubs[i]._inUse  &&  !_internalSubs[i]._loaded)
            {
                Compiler::getRuntime().push_back("\n");
                std::vector<std::string> code;

                if(getInternalSubCode(_internalSubs[i]._includeName, includeVarsDone, code, i)) goto RESTART_COLLECTION; // this is a BASIC compiler, it can't possibly work without at least one GOTO

                includeVarsDone.push_back(_internalSubs[i]._includeName);

                for(int j=0; j<code.size(); j++)
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
        for(int i=0; i<_internalSubs.size(); i++)
        {
            // Check for internal sub directly
            if(_internalSubs[i]._inUse  &&  _internalSubs[i]._loaded  &&  _internalSubs[i]._address == 0x0000) loadInternalSub(i);

            // Runtime size
            if(_internalSubs[i]._inUse  &&  _internalSubs[i]._loaded  &&  _internalSubs[i]._address) runtimeSize += _internalSubs[i]._size;
        }

        fprintf(stderr, "\nCompiler::relinkInternalSubs() : runtime START 0x%04x : runtime END 0x%04x : runtime SIZE %d bytes\n", Compiler::getRuntimeStart() & Memory::getSizeRAM() - 1, Compiler::getRuntimeEnd(), runtimeSize);
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

        for(int i=0; i<_internalSubs.size(); i++)
        {
            if(_internalSubs[i]._inUse)
            {
                Compiler::getOutput().push_back(_internalSubs[i]._name + std::string(LABEL_TRUNC_SIZE - _internalSubs[i]._name.size(), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(_internalSubs[i]._address) + "\n");
            }
        }

        // Zero page call table is not needed when using CALLI
        if(!Assembler::getUseOpcodeCALLI())
        {
            if(_internalSubs[0]._inUse) {Compiler::getOutput().push_back("realTimeProcAddr" + std::string(LABEL_TRUNC_SIZE - strlen("realTimeProcAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(REAL_TIME_PROC) + "\n");}

            uint16_t convertCcOpsAddr = CONVERT_CC_OPS;
            if(_internalSubs[1]._inUse) {Compiler::getOutput().push_back("convertEqOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertEqOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[2]._inUse) {Compiler::getOutput().push_back("convertNeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertNeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[3]._inUse) {Compiler::getOutput().push_back("convertLeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[4]._inUse) {Compiler::getOutput().push_back("convertGeOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGeOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[5]._inUse) {Compiler::getOutput().push_back("convertLtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertLtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
            if(_internalSubs[6]._inUse) {Compiler::getOutput().push_back("convertGtOpAddr" + std::string(LABEL_TRUNC_SIZE - strlen("convertGtOpAddr"), ' ') + "EQU" + std::string(9 - sizeof("EQU"), ' ') + Expression::wordToHexString(convertCcOpsAddr) + "\n"); convertCcOpsAddr += 2;}
        }

        Compiler::getOutput().push_back("\n");

        for(int i=0; i<Compiler::getRuntime().size(); i++) Compiler::getOutput().push_back(Compiler::getRuntime()[i]);
    }


    void resetIncludeFiles(void)
    {
        _subIncludeFiles.clear();
    }

    void resetInternalSubs(void)
    {
        for(int i=0; i<_internalSubs.size(); i++)
        {
            _internalSubs[i]._size = 0;
            _internalSubs[i]._address = 0;
            _internalSubs[i]._includeName = "";
            _internalSubs[i]._loaded = false;

            // RealTimeProc macro and relational init macros are always loaded
            (i>=0  && i<7) ? _internalSubs[i]._inUse = true : _internalSubs[i]._inUse = false;
        }
    }
}