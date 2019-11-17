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

    void adjustExclusionLabelAddresses(uint16_t address, int offset)
    {
        // Adjust addresses for any non page jump labels with addresses higher than start label, (labels can be stored out of order)
        for(int i=0; i<Compiler::getLabels().size(); i++)
        {
            if(!Compiler::getLabels()[i]._pageJump  &&  Compiler::getLabels()[i]._address >= address)
            {
                Compiler::getLabels()[i]._address += offset;
            }
        }
    }

    void adjustExclusionVasmAddresses(int codeLineIndex, uint16_t address, uint16_t page, int offset)
    {
        for(int i=codeLineIndex; i<Compiler::getCodeLines().size(); i++)
        {
            for(int j=0; j<Compiler::getCodeLines()[i]._vasm.size(); j++)
            {
                // Don't adjust page jump's
                if(!Compiler::getCodeLines()[i]._vasm[j]._pageJump  &&  Compiler::getCodeLines()[i]._vasm[j]._address >= address)
                {
                    Compiler::getCodeLines()[i]._vasm[j]._address += offset;
                }
            }
        }
    }

    auto insertPageJumpInstruction(std::vector<Compiler::CodeLine>::iterator& itCode, std::vector<Compiler::VasmLine>::iterator& itVasm,
                                   const std::string& opcode, const std::string& code, uint16_t address, int vasmSize)
    {
        if(itVasm >= itCode->_vasm.end())
        {
            fprintf(stderr, "Compiler::insertPageJumpInstruction() : Trying to insert a PAGE JUMP into lala land, in '%s'", itCode->_code.c_str());
            _EXIT_(EXIT_FAILURE);
        }

        return itCode->_vasm.insert(itVasm, {address, opcode, code, "", true, vasmSize});
    }

    bool checkExclusionZone(const std::string& opcode, uint16_t vPC, uint16_t& nextPC)
    {
        uint8_t hPC = HI_BYTE(vPC);
        uint16_t audioExcl = (hPC <<8) + 0x00F8;
        uint16_t pageExcl  = (hPC <<8) + 0x00FE;

        nextPC = (hPC + 1) <<8;

        // Code page restrictions
        uint16_t codePage0 = 0x0200;
        uint16_t codeEnd0 = 0x0500;

        uint16_t codePage1 = 0x0500;
        uint16_t codeEnd1 = 0x0700;

        uint16_t codePage2 = 0x0800;
        uint16_t codeEnd2 = 0x8000;

        uint16_t codePage3 = 0x8000; // only available with 64K RAM
        uint16_t codeEnd3 = 0x0000;

        uint16_t codeSize = 250;
        if(nextPC == codeEnd0)
        {
            codeSize = 256;
        }
        else if(nextPC == codeEnd1)
        {
            codeSize = 96;
            nextPC = codePage2;
        }
        else if(nextPC >= codePage2  &&  nextPC < codeEnd2)
        {
            codeSize = 96;
            nextPC += 0x00A0;
        }
        else if(nextPC >= codePage3)
        {
            codeSize = 256;
        }

        // Allow some padding for late linking of the runtime
        if(nextPC >= (Compiler::getRuntimeEnd() - 0x0800)  &&  nextPC < codePage3)
        {
            nextPC = codePage3;
            codeSize = 256;
        }

        if(nextPC == codePage3)
        {
            static bool firstTime = true;
            if(firstTime)
            {
                firstTime = false;
                fprintf(stderr, "Compiler::checkExclusionZone() : Warning, you have exceeded 32K of RAM\n");
            }
        }
        if(nextPC == codeEnd3)
        {
            fprintf(stderr, "Compiler::checkExclusionZone() : Error, you have exceeded 64K of RAM\n");
            _EXIT_(EXIT_FAILURE);
        }

        // 3 bytes for CALLI PAGE JUMP
        if(Assembler::getUseOpcodeCALLI())
        {
            audioExcl -= 3;
            pageExcl  -= 3;
        }
        // 7 bytes for non CALLI PAGE JUMP, (STW, LDWI, CALL)
        else
        {
            audioExcl -= 7;
            pageExcl  -= 7;
        }

        // Adjust for MACRO size
        std::string macro = opcode;
        if(macro.size()  &&  macro[0] == '%')
        {
            macro.erase(0, 1);

            if(Compiler::getMacroIndexEntries().find(macro) != Compiler::getMacroIndexEntries().end())
            {
                int macroSize = Compiler::getMacroIndexEntries()[macro]._byteSize;
                audioExcl -= macroSize;
                pageExcl -= macroSize;
            }
        }

        return (vPC >= pageExcl  ||  ((hPC == 0x02 || hPC == 0x03 || hPC == 0x04)  &&  vPC >= audioExcl));
    }

    bool checkExclusionZones(void)
    {
        std::string line;

        bool resetCheck = true;

        // Each time any excluded area code is fixed, restart check
        while(resetCheck)
        {
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
                    resetCheck = false;
                    int vasmLineIndex = int(itVasm - itCode->_vasm.begin());

                    uint16_t nextPC;
                    bool excluded = checkExclusionZone(itVasm->_opcode, itVasm->_address, nextPC);

                    if(!itVasm->_pageJump  &&  excluded)
                    {
                        std::vector<std::string> tokens;
                        uint16_t currPC = (vasmLineIndex > 0) ? itCode->_vasm[vasmLineIndex-1]._address : itVasm->_address;

                        // Insert PAGE JUMP
                        int restoreOffset = 0;
                        auto it = (vasmLineIndex > 0) ? itVasm-1 : itVasm;
                        int index = (vasmLineIndex > 0) ? vasmLineIndex-1 : vasmLineIndex;
                        std::string nextPClabel = "_page_" + Expression::wordToHexString(nextPC);
                        if(Assembler::getUseOpcodeCALLI())
                        {
                            // CALLI PAGE JUMP
                            std::string codeCALLI;
                            int sizeCALLI = Compiler::createVcpuAsm("CALLI", nextPClabel, codeLineIndex, codeCALLI);
                            itVasm = itCode->_vasm.insert((vasmLineIndex > 0) ? itVasm-1 : itVasm, {currPC, "CALLI", codeCALLI, "", true, sizeCALLI});
                        }
                        else
                        {
                            // ROMS that don't have CALLI, (save and restore vAC)
                            std::string codeSTW, codeLDWI, codeCALL, codeLDW;
                            int sizeSTW  = Compiler::createVcpuAsm("STW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeSTW);
                            int sizeLDWI = Compiler::createVcpuAsm("LDWI", nextPClabel, codeLineIndex, codeLDWI);
                            int sizeCALL = Compiler::createVcpuAsm("CALL", "giga_vAC", codeLineIndex, codeCALL);
                            int sizeLDW  = Compiler::createVcpuAsm("LDW", Expression::byteToHexString(VAC_SAVE_START), codeLineIndex, codeLDW);
                            itVasm = itCode->_vasm.insert((vasmLineIndex > 0) ? itVasm-1 : itVasm, {currPC, "STW", codeSTW, "", true, sizeSTW});
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDWI", codeLDWI, uint16_t(currPC + sizeSTW), sizeLDWI);
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "CALL", codeCALL, uint16_t(currPC + sizeSTW + sizeLDWI), sizeCALL);
                            itVasm = insertPageJumpInstruction(itCode, itVasm + 1, "LDW", codeLDW, uint16_t(nextPC), sizeLDW);

                            // New page address is offset by size of vAC restore
                            restoreOffset = sizeLDW;
                        }

                        // Fix labels and addresses
                        int offset = nextPC + restoreOffset - currPC;
                        adjustExclusionLabelAddresses(currPC, offset);
                        adjustExclusionVasmAddresses(codeLineIndex, currPC, nextPC + restoreOffset, offset);

                        // Check for existing label, (after label adjustments)
                        int labelIndex = -1;
                        std::string labelName;
                        Compiler::VasmLine* vasm0 = &itCode->_vasm[itVasm - itCode->_vasm.begin()]; // points to CALLI and LDW
                        Compiler::VasmLine* vasm1 = &itCode->_vasm[itVasm + 1 - itCode->_vasm.begin()]; // points to instruction after CALLI and after LDW
                        if(Compiler::findLabel(nextPC) >= 0)
                        {
                            labelIndex = Compiler::findLabel(nextPC);
                            labelName = Compiler::getLabels()[labelIndex]._name;
                        }
                        if(labelIndex == -1)
                        {
                            // Create CALLI page jump label, (created later in outputCode())
                            if(Assembler::getUseOpcodeCALLI())
                            {
                                // Code referencing these labels must be fixed later in outputLabels
                                if(vasm1->_internalLabel.size()) Compiler::getDiscardedLabels().push_back({vasm1->_address, vasm1->_internalLabel});
            
                                vasm1->_internalLabel = nextPClabel;
                            }
                            // Create pre-CALLI page jump label, (created later in outputCode())
                            else
                            {
                                // Code referencing these labels must be fixed later in outputLabels
                                if(vasm0->_internalLabel.size()) Compiler::getDiscardedLabels().push_back({vasm0->_address, vasm0->_internalLabel});

                                vasm0->_internalLabel = nextPClabel;
                            }
                        }
                        // Existing label at the PAGE JUMP address, so use it
                        else
                        {
                            // Update CALLI page jump label
                            if(Assembler::getUseOpcodeCALLI())
                            {
                                // Macro labels are underscored by default
                                vasm0->_code = (labelName[0] == '_') ? "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "CALLI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                            }
                            // Update pre-CALLI page jump label
                            else
                            {
                                // Macro labels are underscored by default
                                Compiler::VasmLine* vasm = &itCode->_vasm[itVasm - 2 - itCode->_vasm.begin()]; // points to LDWI
                                vasm->_code = (labelName[0] == '_') ? "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + labelName : "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "_" + labelName;
                            }
                        }

                        // Restart checking procedure
                        resetCheck = true;
                        break;
                    }

                    itVasm++;
                }
            
                // Restart checking procedure
                if(resetCheck) break;

                itCode++;
            }
        }

        return true;
    }

    bool checkBranchLabels(void)
    {
        for(int i=0; i<Compiler::getCodeLines().size(); i++)
        {
            for(int j=0; j<Compiler::getCodeLines()[i]._vasm.size(); j++)
            {
                uint16_t vasmAddress = Compiler::getCodeLines()[i]._vasm[j]._address;
                std::string vasmCode = Compiler::getCodeLines()[i]._vasm[j]._code;
                std::string vasmLabel = Compiler::getCodeLines()[i]._vasm[j]._internalLabel;

                if(vasmCode == "BRA")
                {
                    size_t space = vasmCode.find_first_of("  \n\r\f\t\v");
                    std::string operand = vasmCode.substr(space);

                    // Is operand a label?
                    int labelIndex = Compiler::findLabel(operand);
                    if(labelIndex >= 0)
                    {
                        if(HI_MASK(vasmAddress) != HI_MASK(Compiler::getLabels()[labelIndex]._address))
                        {
                            fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", Compiler::getLabels()[labelIndex]._address, vasmAddress, vasmCode.c_str(), i);
                            return false;
                        }
                    }
                    else if(vasmLabel.size())
                    // Check internal label
                    {
                        int labelIndex = Compiler::findInternalLabel(operand);
                        if(labelIndex >= 0)
                        {
                            if(HI_MASK(vasmAddress) != HI_MASK(Compiler::getInternalLabels()[labelIndex]._address))
                            {
                                fprintf(stderr, "Compiler::checkBranchLabels() : trying to branch to : %04x : from %04x in '%s' on line %d\n", Compiler::getInternalLabels()[labelIndex]._address, vasmAddress, vasmCode.c_str(), i);
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
            std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Compiler::checkStatementBlocks() : Syntax error, missing NEXT statement, for '%s' on line %d\n", code.c_str(), codeLineIndex + 1);
            Compiler::getForNextDataStack().pop();
        }

        // Check ELSE ELSEIF blocks
        while(!Compiler::getElseIfDataStack().empty())
        {
            success = false;
            Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
            int codeLineIndex = elseIfData._codeLineIndex;
            std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Compiler::checkStatementBlocks() : Syntax error, missing ELSE or ELSEIF statement, for '%s' on line %d\n", code.c_str(), codeLineIndex + 1);
            Compiler::getElseIfDataStack().pop();
        }

        // Check ENDIF blocks
        while(!Compiler::getEndIfDataStack().empty())
        {
            success = false;
            Compiler::EndIfData endIfData = Compiler::getEndIfDataStack().top();
            int codeLineIndex = endIfData._codeLineIndex;
            std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Compiler::checkStatementBlocks() : Syntax error, missing ENDIF statement, for '%s' on line %d\n", code.c_str(), codeLineIndex + 1);
            Compiler::getEndIfDataStack().pop();
        }
        
        // Check WHILE WEND blocks
        while(!Compiler::getWhileWendDataStack().empty())
        {
            success = false;
            Compiler::WhileWendData whileWendData = Compiler::getWhileWendDataStack().top();
            int codeLineIndex = whileWendData._codeLineIndex;
            std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Compiler::checkStatementBlocks() : Syntax error, missing WEND statement, for '%s' on line %d\n", code.c_str(), codeLineIndex + 1);
            Compiler::getWhileWendDataStack().pop();
        }

        // Check DO UNTIL blocks
        while(!Compiler::getRepeatUntilDataStack().empty())
        {
            success = false;
            Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
            int codeLineIndex = repeatUntilData._codeLineIndex;
            std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
            fprintf(stderr, "Compiler::checkStatementBlocks() : Syntax error, missing UNTIL statement, for '%s' on line %d\n", code.c_str(), codeLineIndex + 1);
            Compiler::getRepeatUntilDataStack().pop();
        }

        return success;
    }
}