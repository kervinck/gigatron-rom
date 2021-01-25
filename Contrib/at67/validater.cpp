#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <algorithm>

#include "memory.h"
#include "cpu.h"
#include "assembler.h"
#include "compiler.h"
#include "validater.h"


namespace Validater
{
    bool initialise(void)
    {
        return true;
    }

    void adjustLabelAddresses(uint16_t address, int offset)
    {
        // Adjust addresses for any non page jump labels with addresses higher than start label, (labels can be stored out of order)
        for(int i=0; i<int(Compiler::getLabels().size()); i++)
        {
            if(!Compiler::getLabels()[i]._pageJump  &&  Compiler::getLabels()[i]._address >= address)
            {
                Compiler::getLabels()[i]._address += int16_t(offset);
            }
        }

        for(int i=0; i<int(Compiler::getInternalLabels().size()); i++)
        {
            if(Compiler::getInternalLabels()[i]._address >= address)
            {
                Compiler::getInternalLabels()[i]._address += int16_t(offset);
            }
        }
    }

    void adjustVasmAddresses(int codeLineIndex, uint16_t address, int offset)
    {
        for(int i=codeLineIndex; i<int(Compiler::getCodeLines().size()); i++)
        {
            for(int j=0; j<int(Compiler::getCodeLines()[i]._vasm.size()); j++)
            {
                // Don't adjust page jump's
                if(!Compiler::getCodeLines()[i]._vasm[j]._pageJump  &&  Compiler::getCodeLines()[i]._vasm[j]._address >= address)
                {
                    Compiler::getCodeLines()[i]._vasm[j]._address += int16_t(offset);
                }
            }
        }
    }

    auto insertPageJumpInstruction(const std::vector<Compiler::CodeLine>::iterator& itCode, const std::vector<Compiler::VasmLine>::iterator& itVasm,
                                   const std::string& opcode, const std::string& operand, const std::string& code, uint16_t address, int vasmSize)
    {
        if(itVasm >= itCode->_vasm.end())
        {
            fprintf(stderr, "Validater::insertPageJumpInstruction() : Trying to insert a PAGE JUMP into lala land, in '%s'", itCode->_code.c_str());
            _EXIT_(EXIT_FAILURE);
        }

        Memory::takeFreeRAM(address, vasmSize, true);

        return itCode->_vasm.insert(itVasm, {address, opcode, operand, code, "", true, vasmSize});
    }

    // TODO: make this more flexible, (e.g. sound channels off etc)
    bool checkForRelocation(const std::string& opcode, uint16_t vPC, uint16_t& nextPC, uint16_t& numThunks, uint16_t& totalThunkSize, bool& print)
    {
#define CALL_PAGE_JUMP_SIZE    7
#define CALLI_PAGE_JUMP_SIZE   3
#define CALL_PAGE_JUMP_OFFSET  2
#define CALLI_PAGE_JUMP_OFFSET 0

        int opcodeSize = 0;
        if(opcode.size())
        {
            // Macro
            if(opcode[0] == '%')
            {
                std::string macro = opcode;
                macro.erase(0, 1);

                if(Compiler::getMacroIndexEntries().find(macro) != Compiler::getMacroIndexEntries().end())
                {
                    opcodeSize = Compiler::getMacroIndexEntries()[macro]._byteSize;
                }
            }
            // VASM
            else
            {
                opcodeSize = Assembler::getAsmOpcodeSize(opcode);
            }

            if(opcodeSize)
            {
                // Increase opcodeSize by size of page jump prologue
                int opSize = (Compiler::getCodeRomType() >= Cpu::ROMv5a) ? opcodeSize + CALLI_PAGE_JUMP_SIZE : opcodeSize + CALL_PAGE_JUMP_SIZE;

                // Code can't straddle page boundaries
                if(HI_BYTE(vPC) == HI_BYTE(vPC + opSize)  &&  Memory::isFreeRAM(vPC, opSize))
                {
                    // Code relocation is not required if requested RAM address is free
                    Memory::takeFreeRAM(vPC, opcodeSize, true);
                    return false;
                }

                // Get next free code address after page jump prologue and relocate code, (return true)
                if(!Memory::getNextCodeAddress(Memory::FitAscending, vPC, opSize, nextPC))
                {
                    fprintf(stderr, "Validater::checkForRelocation(): Memory alloc at 0x%0x4 of size %d failed\n", vPC, opSize);
                    return false;
                }

                if(!print)
                {
                    print = true;
                    fprintf(stderr, "\n*******************************************************\n");
                    fprintf(stderr, "*                      Relocating                      \n");
                    fprintf(stderr, "*******************************************************\n");
                    fprintf(stderr, "*       Opcode         : Address :    Size     :  New  \n");
                    fprintf(stderr, "*******************************************************\n");
                }

                numThunks++;

                uint16_t newPC = nextPC;
                if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                {
                    newPC += CALLI_PAGE_JUMP_OFFSET;
                    totalThunkSize += CALLI_PAGE_JUMP_SIZE + CALLI_PAGE_JUMP_OFFSET;
                }
                else
                {
                    newPC += CALL_PAGE_JUMP_OFFSET;
                    totalThunkSize += CALL_PAGE_JUMP_SIZE + CALL_PAGE_JUMP_OFFSET;
                }
                fprintf(stderr, "* %-20s : 0x%04x  :    %2d bytes : 0x%04x\n", opcode.c_str(), vPC, opcodeSize, newPC);
                return true;
            }
        }

        return false;
    }

    bool checkForRelocations(void)
    {
        std::string line;
        bool print = false;
        uint16_t numThunks = 0, totalThunkSize = 0;
        for(auto itCode=Compiler::getCodeLines().begin(); itCode!=Compiler::getCodeLines().end();)
        {
            if(itCode->_vasm.size() == 0)
            {
                itCode++;
                continue;
            }

            int codeLineIndex = int(itCode - Compiler::getCodeLines().begin());

            for(auto itVasm=itCode->_vasm.begin(); itVasm!=itCode->_vasm.end();)
            {
                uint16_t nextPC;
                bool excluded = checkForRelocation(itVasm->_opcode, itVasm->_address, nextPC, numThunks, totalThunkSize, print);

                if(!itVasm->_pageJump  &&  excluded)
                {
                    std::vector<std::string> tokens;
                    uint16_t currPC = itVasm->_address;

                    // Insert PAGE JUMP
                    int restoreOffset = 0;
                    std::string nextPClabel = "_page_" + Expression::wordToHexString(nextPC);
                    if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                    {
                        // CALLI PAGE JUMP
                        std::string codeCALLI;
                        int sizeCALLI = Compiler::createVcpuAsm("CALLI", nextPClabel, codeLineIndex, codeCALLI);
                        itVasm = insertPageJumpInstruction(itCode, itVasm, "CALLI", nextPClabel, codeCALLI,  uint16_t(currPC), sizeCALLI);
                    }
                    else
                    {
                        // ROMS that don't have CALLI save and restore vAC
                        std::string codeSTW, codeLDWI, codeCALL, codeLDW;
#define VAC_SAVE_STACK
#ifdef VAC_SAVE_STACK
                        int sizeSTW  = Compiler::createVcpuAsm("STLW", "0xFE", codeLineIndex, codeSTW);
                        int sizeLDWI = Compiler::createVcpuAsm("LDWI", nextPClabel, codeLineIndex, codeLDWI);
                        int sizeCALL = Compiler::createVcpuAsm("CALL", "giga_vAC", codeLineIndex, codeCALL);
                        int sizeLDW  = Compiler::createVcpuAsm("LDLW", "0xFE", codeLineIndex, codeLDW);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 0, "STLW", "0xFE", codeSTW,  uint16_t(currPC), sizeSTW);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDWI", nextPClabel, codeLDWI, uint16_t(currPC + sizeSTW), sizeLDWI);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "CALL", "giga_vAC", codeCALL, uint16_t(currPC + sizeSTW + sizeLDWI), sizeCALL);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDLW", "0xFE", codeLDW,  uint16_t(nextPC), sizeLDW);
#else
#define VAC_SAVE_START  0x00D6
                        int sizeSTW  = Compiler::createVcpuAsm("STW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeSTW);
                        int sizeLDWI = Compiler::createVcpuAsm("LDWI", nextPClabel, codeLineIndex, codeLDWI);
                        int sizeCALL = Compiler::createVcpuAsm("CALL", "giga_vAC", codeLineIndex, codeCALL);
                        int sizeLDW  = Compiler::createVcpuAsm("LDW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeLDW);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 0, "STW",  Expression::byteToHexString(VAC_SAVE_START), codeSTW,  uint16_t(currPC), sizeSTW);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDWI", nextPClabel, codeLDWI, uint16_t(currPC + sizeSTW), sizeLDWI);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "CALL", "giga_vAC", codeCALL, uint16_t(currPC + sizeSTW + sizeLDWI), sizeCALL);
                        itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDW",  Expression::byteToHexString(VAC_SAVE_START), codeLDW,  uint16_t(nextPC), sizeLDW);
#endif
                        // New page address is offset by size of vAC restore
                        restoreOffset = sizeLDW;
                    }

                    // Fix labels and addresses
                    int offset = nextPC + restoreOffset - currPC;
                    adjustLabelAddresses(currPC, offset);
                    adjustVasmAddresses(codeLineIndex, currPC, offset);

                    // Check for existing label, (after label adjustments)
                    int labelIndex = -1;
                    std::string labelName;
                    Compiler::VasmLine* vasmCurr = &itCode->_vasm[itVasm - itCode->_vasm.begin()]; // points to CALLI and LDW
                    Compiler::VasmLine* vasmNext = &itCode->_vasm[itVasm + 1 - itCode->_vasm.begin()]; // points to instruction after CALLI and after LDW
                    if(Compiler::findLabel(nextPC) >= 0)
                    {
                        labelIndex = Compiler::findLabel(nextPC);
                        labelName = Compiler::getLabels()[labelIndex]._name;
                    }
                    if(labelIndex == -1)
                    {
                        // Create CALLI page jump label, (created later in outputCode())
                        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                        {
                            // Code referencing these labels must be fixed later in outputLabels, (discarded label addresses must be updated if they match page jump address)
                            if(vasmNext->_internalLabel.size())
                            {
                                Compiler::getDiscardedLabels().push_back({vasmNext->_address, vasmNext->_internalLabel});
                                Compiler::adjustDiscardedLabels(vasmNext->_internalLabel, vasmNext->_address);
                            }
            
                            vasmNext->_internalLabel = nextPClabel;
                        }
                        // Create pre-CALLI page jump label, (created later in outputCode())
                        else
                        {
                            // Code referencing these labels must be fixed later in outputLabels, (discarded label addresses must be updated if they match page jump address)
                            if(vasmCurr->_internalLabel.size())
                            {
                                Compiler::getDiscardedLabels().push_back({vasmCurr->_address, vasmCurr->_internalLabel});
                                Compiler::adjustDiscardedLabels(vasmCurr->_internalLabel, vasmCurr->_address);
                            }

                            vasmCurr->_internalLabel = nextPClabel;
                        }
                    }
                    // Existing label at the PAGE JUMP address, so use it
                    else
                    {
                        // Update CALLI page jump label
                        if(Compiler::getCodeRomType() >= Cpu::ROMv5a)
                        {
                            // Macro labels are underscored by default
                            vasmCurr->_code = (labelName[0] == '_') ? "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                        }
                        // Update pre-CALLI page jump label
                        else
                        {
                            // Macro labels are underscored by default
                            Compiler::VasmLine* vasm = &itCode->_vasm[itVasm - 2 - itCode->_vasm.begin()]; // points to LDWI
                            vasm->_code = (labelName[0] == '_') ? "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                        }
                    }
                }

                itVasm++;
            }

            itCode++;
        }

        if(print)
        {
            fprintf(stderr, "*******************************************************\n");
            fprintf(stderr, "* Number of page jumps = %4d, RAM used = %5d bytes *\n", numThunks, totalThunkSize);
            fprintf(stderr, "*******************************************************\n");
        }

        return true;
    }

    bool opcodeHasBranch(const std::string& opcode)
    {
        if(opcode == "BRA")                return true;
        if(opcode == "BEQ")                return true;
        if(opcode == "BNE")                return true;
        if(opcode == "BGE")                return true;
        if(opcode == "BLE")                return true;
        if(opcode == "BGT")                return true;
        if(opcode == "BLT")                return true;
        if(opcode == "%ForNextInc")        return true;
        if(opcode == "%ForNextDec")        return true;
        if(opcode == "%ForNextDecZero")    return true;
        if(opcode == "%ForNextAdd")        return true;
        if(opcode == "%ForNextSub")        return true;
        if(opcode == "%ForNextVarAdd")     return true;
        if(opcode == "%ForNextVarSub")     return true;

        return false;
    }

    bool checkBranchLabels(void)
    {
        for(int i=0; i<int(Compiler::getCodeLines().size()); i++)
        {
            // Line number taking into account modules
            int codeLineStart = Compiler::getCodeLineStart(i);

            for(int j=0; j<int(Compiler::getCodeLines()[i]._vasm.size()); j++)
            {
                uint16_t opcAddr = Compiler::getCodeLines()[i]._vasm[j]._address;
                std::string opcode = Compiler::getCodeLines()[i]._vasm[j]._opcode;
                const std::string& code = Compiler::getCodeLines()[i]._vasm[j]._code;
                const std::string& basic = Compiler::getCodeLines()[i]._text;

                Expression::stripWhitespace(opcode);
                if(opcodeHasBranch(opcode))
                {
                    std::vector<std::string> tokens = Expression::tokenise(code, ' ', false);
                    if(tokens.size() < 2) continue;

                    // Normal branch
                    std::string operand;
                    if(tokens.size() == 2)
                    {
                        Expression::stripWhitespace(tokens[1]);
                        operand = tokens[1];
                    }
                    // Branch embedded in a FOR NEXT macro
                    else if(tokens.size() > 2)
                    {
                        Expression::stripWhitespace(tokens[2]);
                        operand = tokens[2];
                    }

                    // Remove underscores from BASIC labels for matching
                    if(operand.size() > 1  &&  operand[0] == '_') operand.erase(0, 1);

                    // Is operand a label?
                    int labelIndex = Compiler::findLabel(operand);
                    if(labelIndex >= 0)
                    {
                        uint16_t labAddr = Compiler::getLabels()[labelIndex]._address;
                        if(HI_MASK(opcAddr) != HI_MASK(labAddr))
                        {
                            fprintf(stderr, "\nValidater::checkBranchLabels() : *** Error ***, %s is branching from 0x%04x to 0x%04x, for '%s' on line %d\n\n", opcode.c_str(), opcAddr, labAddr, basic.c_str(), codeLineStart);
                            return false;
                        }
                    }
                    // Check internal label
                    else
                    {
                        // Internal labels always have underscores, so put it back
                        operand.insert(0, 1, '_');

                        labelIndex = Compiler::findInternalLabel(operand);
                        if(labelIndex >= 0)
                        {
                            uint16_t labAddr = Compiler::getInternalLabels()[labelIndex]._address;
                            if(HI_MASK(opcAddr) != HI_MASK(labAddr))
                            {
                                fprintf(stderr, "\nValidater::checkBranchLabels() : *** Error ***, %s is branching from 0x%04x to 0x%04x, for '%s' on line %d\n\n", opcode.c_str(), opcAddr, labAddr, basic.c_str(), codeLineStart);
                                return false;
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    bool checkStatementBlocks(void)
    {
        bool success = true;

        // Check FOR NEXT blocks
        while(!Compiler::getForNextDataStack().empty())
        {
            success = false;
            Compiler::ForNextData forNextData = Compiler::getForNextDataStack().top();
            int codeLineIndex = forNextData._codeLineIndex;
            const std::string& code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Validater::checkStatementBlocks() : Syntax error, missing NEXT statement, for '%s' on line %d\n", code.c_str(), codeLineIndex);
            Compiler::getForNextDataStack().pop();
        }

        // Check ELSE ELSEIF blocks
        while(!Compiler::getElseIfDataStack().empty())
        {
            success = false;
            Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
            int codeLineIndex = elseIfData._codeLineIndex;
            const std::string& code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Validater::checkStatementBlocks() : Syntax error, missing ELSE or ELSEIF statement, for '%s' on line %d\n", code.c_str(), codeLineIndex);
            Compiler::getElseIfDataStack().pop();
        }

        // Check WHILE WEND blocks
        while(!Compiler::getWhileWendDataStack().empty())
        {
            success = false;
            Compiler::WhileWendData whileWendData = Compiler::getWhileWendDataStack().top();
            int codeLineIndex = whileWendData._codeLineIndex;
            const std::string& code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Validater::checkStatementBlocks() : Syntax error, missing WEND statement, for '%s' on line %d\n", code.c_str(), codeLineIndex);
            Compiler::getWhileWendDataStack().pop();
        }

        // Check DO UNTIL blocks
        while(!Compiler::getRepeatUntilDataStack().empty())
        {
            success = false;
            Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
            int codeLineIndex = repeatUntilData._codeLineIndex;
            const std::string& code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Validater::checkStatementBlocks() : Syntax error, missing UNTIL statement, for '%s' on line %d\n", code.c_str(), codeLineIndex);
            Compiler::getRepeatUntilDataStack().pop();
        }

        return success;
    }

    bool checkCallProcFuncData(void)
    {
        bool success = true;

        // Check PROC's corresponding CALL's
        for(auto it=Compiler::getCallDataMap().begin(); it!=Compiler::getCallDataMap().end(); ++it)
        {
            int numParams = it->second._numParams;
            int codeLineIndex = it->second._codeLineIndex;
            const std::string& procName = it->second._name;
            const std::string& code = Compiler::getCodeLines()[codeLineIndex]._code;

            if(Compiler::getProcDataMap().find(procName) == Compiler::getProcDataMap().end())
            {
                fprintf(stderr, "Validator::checkCallProcFuncData() : Syntax error, 'CALL <NAME>' cannot find a corresponding 'PROC <NAME>', in '%s' on line %d\n", code.c_str(), codeLineIndex);
                success = false;
                continue;
            }

            if(Compiler::getProcDataMap()[procName]._numParams != numParams)
            {
                fprintf(stderr, "Validator::checkCallProcFuncData() : Syntax error, 'CALL <NAME>' has incorrect number of parameters compared to 'PROC <NAME>', in '%s' on line %d\n", code.c_str(), codeLineIndex);
                success = false;
                continue;
            }
        }

        return success;
    }

    bool checkRuntimeVersion(void)
    {
        int16_t runtimeVersion = Assembler::getRuntimeVersion();
        if(runtimeVersion != RUNTIME_VERSION)
        {
            fprintf(stderr, "\n*************************************************************************************************\n");
            fprintf(stderr, "* Expected runtime version %04d : Found runtime version %04d\n", RUNTIME_VERSION, runtimeVersion);
            fprintf(stderr, "*************************************************************************************************\n\n");

            return false;
        }

        return true;
    }
}