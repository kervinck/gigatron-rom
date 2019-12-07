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
#include "optimiser.h"


namespace Optimiser
{
    enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwStHigh, ExtraLdw, LdwPair, StwLdiAddw, StwLdwAddw, StwLdwAddwVar, StwLdiAndw, StwLdwAndw, StwLdwAndwVar, StwLdiXorw, StwLdwXorw,
                        StwLdwXorwVar, StwLdiOrw, StwLdwOrw, StwLdwOrwVar, PokeVar, DokeVar, Lsl8Var, StwPair, StwPairReg, ExtraStw, PeekArray, DeekArray, 
                        PokeArray, DokeArray, PokeVarArray, DokeVarArray, PokeTmpArray, DokeTmpArray, AddiPair, AddiZero, SubiZero, NumOptimiseTypes};

    struct MatchSequence
    {
        int _firstIndex;
        int _secondIndex;
        std::vector<std::string> _sequence;
    };

    std::vector<MatchSequence> matchSequences = 
    {
        // StwLdwPair
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x" }},

        // StwLdPair
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LD"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0x" }},

        // StwStHigh
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ST"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_" }},

        // ExtraLdw
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"  }},

        // LdwPair
        {0, 1, {"LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"  }},

        // StwLdiAddw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDI"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAddw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAddwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdiAndw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDI"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAndw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAndwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdiXorw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDI"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwXorw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwXorwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdiOrw
        {0, 2, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDI" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "ORW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

        // StwLdwOrw
        {0, 2, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ORW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

        // StwLdwOrwVar
        {0, 2, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "ORW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

        // PokeVar
        {0, 2, {"LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "", 
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + ""}},

        // DokeVar
        {0, 2, {"LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "", 
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + ""}},

        // Lsl8Var
        {0, 1, {"ST"   + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "high",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"}},

        // StwPair
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x" }},

        // StwPairReg
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg"}},

        // ExtraStw
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"  }},

        // PeekArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg", 
                "PEEK" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // DeekArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg", 
                "DEEK" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // PokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg", 
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg", 
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // DokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // PokeVarArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // DokeVarArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // PokeTmpArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // DokeTmpArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // AddiPair
        {0, 0, {"ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "",
                "ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // AddiZero
        {0, 0, {"ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' '), ""}},

        // SubiZero
        {0, 0, {"SUBI" + std::string(OPCODE_TRUNC_SIZE - 4, ' '), ""}},
    };


    bool initialise(void)
    {
        return true;
    }


    // Migrate internal label for an instruction that has been deleted, (use this function before the instruction is deleted)
    bool migrateInternalLabel(int index, int oldLine, int newLine)
    {
        // If a label exists, move it to next available vasm line
        if(Compiler::getCodeLines()[index]._vasm[oldLine]._internalLabel.size())
        {
            // Next available vasm line is part of a new BASIC line, so can't optimise
            if(Compiler::getCodeLines()[index]._vasm.size() <= newLine) return false;
            Compiler::getCodeLines()[index]._vasm[newLine]._internalLabel = Compiler::getCodeLines()[index]._vasm[oldLine]._internalLabel;
        }
    
        return true;
    }

    // Adjust label addresses for any labels with addresses higher than optimised vasm instruction address
    void adjustLabelAddresses(int codeLineIndex, int vasmLineIndex, int offset)
    {
        // Loop through commented out code
        do
        {
            if(vasmLineIndex >= Compiler::getCodeLines()[codeLineIndex]._vasm.size())
            {
                if(++codeLineIndex >= Compiler::getCodeLines().size()) return;
                vasmLineIndex = 0;
            }
        }
        while(Compiler::getCodeLines()[codeLineIndex]._vasm.size() == 0);

        uint16_t optimisedAddress = Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._address;

        for(int i=0; i<Compiler::getLabels().size(); i++)
        {
            if(Compiler::getLabels()[i]._address >= optimisedAddress)
            {
                Compiler::getLabels()[i]._address += offset;
            }
        }
    }

    // Adjust vasm code addresses
    void adjustVasmAddresses(int codeLineIndex, int vasmLineIndex, int offset)
    {
        // Loop through commented out code
        do
        {
            if(vasmLineIndex >= Compiler::getCodeLines()[codeLineIndex]._vasm.size())
            {
                if(++codeLineIndex >= Compiler::getCodeLines().size()) return;
                vasmLineIndex = 0;
            }
        }
        while(Compiler::getCodeLines()[codeLineIndex]._vasm.size() == 0);

        for(int i=codeLineIndex; i<Compiler::getCodeLines().size(); i++)
        {
            int start = (i == codeLineIndex) ? vasmLineIndex : 0;
            for(int j=start; j<Compiler::getCodeLines()[i]._vasm.size(); j++)
            {
                Compiler::getCodeLines()[i]._vasm[j]._address += offset;
            }
        }
    }

    bool optimiseCode(void)
    {

RESTART_OPTIMISE:
        for(int i=0; i<Compiler::getCodeLines().size(); i++)
        {
            for(int j=0; j<matchSequences.size(); j++)
            {
                for(auto itVasm=Compiler::getCodeLines()[i]._vasm.begin(); itVasm!=Compiler::getCodeLines()[i]._vasm.end();)
                {
                    bool linesDeleted = false;
                    int vasmIndex = int(itVasm - Compiler::getCodeLines()[i]._vasm.begin());

                    // Can only optimise within a BASIC code line, (use multi-statements to optimise across lines)
                    int vasmIndexMax = vasmIndex + int(matchSequences[j]._sequence.size()) - 1;
                    if(vasmIndexMax >= Compiler::getCodeLines()[i]._vasm.size())
                    {
                        ++itVasm;
                        continue;
                    }

                    // Find opcode match
                    bool foundOpcodeMatch = true;
                    for(int k=vasmIndex; k<=vasmIndexMax; k++)
                    {
                        if(Compiler::getCodeLines()[i]._vasm[k]._code.find(matchSequences[j]._sequence[k - vasmIndex]) ==  std::string::npos)
                        {
                            foundOpcodeMatch = false;
                            break;
                        }
                    }

                    if(foundOpcodeMatch)
                    {
                        // First operand
                        int firstIndex = matchSequences[j]._firstIndex;
                        int firstLine = vasmIndex + firstIndex;
                        size_t firstSpace = Compiler::getCodeLines()[i]._vasm[firstLine]._code.find_first_of("  \n\r\f\t\v");
                        std::string firstOperand = Compiler::getCodeLines()[i]._vasm[firstLine]._code.substr(firstSpace);
                        Expression::stripWhitespace(firstOperand);

                        // Second operand
                        int secondIndex = matchSequences[j]._secondIndex;
                        int secondLine = vasmIndex + secondIndex;
                        size_t secondSpace = Compiler::getCodeLines()[i]._vasm[secondLine]._code.find_first_of("  \n\r\f\t\v");
                        std::string secondOperand = Compiler::getCodeLines()[i]._vasm[secondLine]._code.substr(secondSpace);
                        Expression::stripWhitespace(secondOperand);

/*************************************************************************************************************************************************************/
/* Opcode matches required, operand matches required                                                                                                         */
/*************************************************************************************************************************************************************/

                        // Find operand match, (temporary variables are a minimum of 4 chars, i.e. '0xc0')
                        if(firstOperand.substr(0, 4) == secondOperand.substr(0, 4))
                        {
                            switch(j)
                            {
                                // Match STW LDW, delete STW LDW
                                //case StwLdPair:
                                case StwLdwPair:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW ST, delete STW
                                case StwStHigh:
                                {
                                    // Assume neither of these instructions can have a label
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -2);
                                }
                                break;

                                // Match STW LDW, delete LDW
                                case ExtraLdw:
                                {
                                    // If the LDW has an internal label, then it probably can't be optimised away
                                    if(!Compiler::getCodeLines()[i]._vasm[firstLine + 1]._internalLabel.size())
                                    {
                                        // Migrate internal label to next available instruction
                                        if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                        // Delete LDW
                                        linesDeleted = true;
                                        itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                        adjustLabelAddresses(i, firstLine + 1, -2);
                                        adjustVasmAddresses(i, firstLine + 1, -2);
                                    }
                                }
                                break;

                                // Match LDW LDW, delete first LDW
                                case LdwPair:
                                {
                                    // Migrate internal label from first LDW to second LDW
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 1)) break;

                                    // Delete first LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine + 1, -2);
                                    adjustVasmAddresses(i, firstLine + 1, -2);
                                }
                                break;

                                // Match STW LDI ADDW, copy LDI operand to ADDW operand, change ADDW to ADDI, delete STW LDW
                                case StwLdiAddw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ADDW's operand becomes the LDI's operand
                                    size_t ldiSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldiSpace);
                                    Expression::stripWhitespace(ldiOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldiOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW
                                case StwLdwAddw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ADDW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW, (LDW is a var)
                                case StwLdwAddwVar:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ADDW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDI ANDW, copy LDI operand to ANDW operand, change ANDW to ANDI, delete STW LDW
                                case StwLdiAndw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ANDW's operand becomes the LDI's operand
                                    size_t ldiSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldiSpace);
                                    Expression::stripWhitespace(ldiOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ANDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldiOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW
                                case StwLdwAndw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ANDW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW, (LDW is a var)
                                case StwLdwAndwVar:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ANDW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDI XORW, copy LDI operand to XORW operand, change XORW to XORI, delete STW LDW
                                case StwLdiXorw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // XORW's operand becomes the LDI's operand
                                    size_t ldiSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldiSpace);
                                    Expression::stripWhitespace(ldiOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "XORI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldiOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW
                                case StwLdwXorw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // XORW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW, (LDW is a var)
                                case StwLdwXorwVar:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // XORW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDI ORW, copy LDI operand to ORW operand, change ORW to ORI, delete STW LDW
                                case StwLdiOrw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ORW's operand becomes the LDI's operand
                                    size_t ldiSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldiSpace);
                                    Expression::stripWhitespace(ldiOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ORI" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + ldiOperand;

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW
                                case StwLdwOrw:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ORW's operand becomes the LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = matchSequences[j]._sequence[2] + ldwOperand.substr(2); // don't need the leading "0x"

                                    // Delete STW and LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW, (LDW is a var)
                                case StwLdwOrwVar:
                                {
                                    // Only one of these can have an internal label
                                    if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                    if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                    // ORW's operand becomes LDW's operand
                                    size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                    Expression::stripWhitespace(ldwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                    // Delete STW LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match LDW POKE/DOKE LDW, delete second LDW if it matches with first LDW
                                case PokeVar:
                                case DokeVar:
                                {
                                    // Migrate second LDW's label, (if it has one)
                                    if(!migrateInternalLabel(i, firstLine + 2, firstLine + 3)) break;

                                    // Delete second LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 2);
                                    adjustLabelAddresses(i, firstLine + 2, -2);
                                    adjustVasmAddresses(i, firstLine+ 2, -2);
                                }
                                break;

                                // Match ST LDW ANDW STW, replace ST operand with STW operand, delete LDW ANDW STW
                                case Lsl8Var:
                                {
                                    // ST's operand becomes STW's operand
                                    size_t stwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 3]._code.find_first_of("  \n\r\f\t\v");
                                    std::string stwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 3]._code.substr(stwSpace);
                                    Expression::stripWhitespace(stwOperand);
                                    Compiler::getCodeLines()[i]._vasm[firstLine]._code = "ST" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + stwOperand + " + 1";

                                    // Delete LDW ANDW STW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                    adjustLabelAddresses(i, firstLine + 1, -6);
                                    adjustVasmAddresses(i, firstLine + 1, -6);
                                }
                                break;
                            }
                        }

/*************************************************************************************************************************************************************/
/* Opcode matches required, operand matches NOT required                                                                                                     */
/*************************************************************************************************************************************************************/
                        switch(j)
                        {
                            // Extra STW, (doesn't require an operand match)
                            case StwPair:
                            case StwPairReg:
                            case ExtraStw:
                            {
                                // Migrate internal label to next available instruction
                                if(!migrateInternalLabel(i, firstLine, firstLine + 1)) break;

                                // Delete first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(i, firstLine, -2);
                                adjustVasmAddresses(i, firstLine, -2);
                            }
                            break;

                            // Match STW LDWI ADDW PEEK
                            case PeekArray:
                            {
                                // Save previous line LDW, if opcode is not LDW then can't optimise
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLDW._opcode != "LDW") break;

                                // Migrate it's label if it has one
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;

                                // Get saved LDW's operand
                                size_t ldwSpace = savedLDW._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = savedLDW._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);

                                // Delete previous line LDW and first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of ADDW
                                (itVasm + 1)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            break;

                            // Match STW LDWI ADDW ADDW DEEK
                            case DeekArray:
                            {
                                // Save previous line LDW, if opcode is not LDW then can't optimise
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLDW._opcode != "LDW") break;

                                // Migrate it's label if it has one
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;

                                // Get saved LDW's operand
                                size_t ldwSpace = savedLDW._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = savedLDW._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);

                                // Delete previous line LDW and first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of both ADDW's
                                (itVasm + 1)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                (itVasm + 2)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            break;

                            // Match STW LDWI STW LDW POKE/DOKE
                            case PokeArray:
                            case DokeArray:
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") == std::string::npos) break;

                                // Discard it's label, (it's no longer needed), and adjust it's address
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;
                                savedLD._internalLabel = "";
                                savedLD._address += 9; // LD<X> is moved 9bytes

                                // Delete previous line LD<X>, first STW and LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 2);

                                // Replace LDW with saved LD<X> and operand
                                itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            break;

                            // Match STW LDW STW LDWI ADDW STW LDW POKE
                            case PokeVarArray:
                            case PokeTmpArray:
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise first phase
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") != std::string::npos)
                                {
                                    // Discard it's label, (it's no longer needed), and adjust it's address
                                    if(!migrateInternalLabel(i, firstLine - 1, firstLine + 3)) break;
                                    savedLD._internalLabel = "";
                                    savedLD._address += 15; // LD<X> is moved 15 bytes

                                    // Delete previous line LD<X>, first STW and last LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 5); //points to last LDW

                                    // Replace LDW with saved LD<X> and operand
                                    itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                    adjustLabelAddresses(i, firstLine - 1, -4);
                                    adjustVasmAddresses(i, firstLine - 1, -4);
                                    firstLine = firstLine - 1;  // points to new first LDW
                                }
                                else
                                {
                                    firstLine = firstLine + 1; // points to first LDW
                                }

                                // Now optimise the first LDW and second STW for second phase
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine];

                                // Get saved LDW's operand
                                size_t ldwSpace = savedLDW._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = savedLDW._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);

                                // Delete first LDW and second STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of ADDW
                                (itVasm + 1)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                adjustLabelAddresses(i, firstLine, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            break;

                            // Match STW LDW STW LDWI ADDW ADDW STW LDW DOKE
                            case DokeVarArray:
                            case DokeTmpArray:
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise first phase
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") != std::string::npos)
                                {
                                    // Migrate LD<X>'s label to LDWI
                                    if(!migrateInternalLabel(i, firstLine - 1, firstLine + 3)) break;
                                    savedLD._internalLabel = "";
                                    savedLD._address += 17; // LD<X> is moved 17 bytes

                                    // Delete previous line LD<X>, first STW and last LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 6); //points to last LDW

                                    // Replace LDW with saved LD<X> and operand
                                    itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                    adjustLabelAddresses(i, firstLine - 1, -4);
                                    adjustVasmAddresses(i, firstLine - 1, -4);
                                    firstLine = firstLine - 1;  // points to new first LDW
                                }
                                else
                                {
                                    firstLine = firstLine + 1; // points to first LDW
                                }

                                // Now optimise the first LDW and second STW for second phase
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine];

                                // Get saved LDW's operand
                                size_t ldwSpace = savedLDW._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = savedLDW._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);

                                // Delete first LDW and second STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of both ADDW's
                                (itVasm + 1)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                (itVasm + 2)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                adjustLabelAddresses(i, firstLine, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            break;

                            // Match ADDI ADDI
                            case AddiPair:
                            {
                                uint8_t addi0, addi1;

                                // Migrate second ADDI's label to next instruction
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // Add operands together, replace first operand, delete 2nd opcode and operand
                                Compiler::VasmLine vasm = Compiler::getCodeLines()[i]._vasm[firstLine];
                                size_t space = vasm._code.find_first_of("  \n\r\f\t\v");
                                std::string operand = vasm._code.substr(space);
                                Expression::stripWhitespace(operand);
                                Expression::stringToU8(operand, addi0);
                                vasm = Compiler::getCodeLines()[i]._vasm[firstLine + 1];
                                space = vasm._code.find_first_of("  \n\r\f\t\v");
                                operand = vasm._code.substr(space);
                                Expression::stripWhitespace(operand);
                                Expression::stringToU8(operand, addi1);
                                int result = addi0 + addi1;
                                if(result > 255) break; // result to large for an ADDI operand so exit

                                // Delete second ADDI
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);

                                // Replace first ADDI's operand
                                (itVasm - 1)->_code = "ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + std::to_string(uint8_t(result));
                                adjustLabelAddresses(i, firstLine + 1, -2);
                                adjustVasmAddresses(i, firstLine + 1, -2);
                            }
                            break;
                        }
                    }

                    // Arithmetic with zero, (does it's own opcode match)
                    if(j == AddiZero  ||  j == SubiZero)
                    {
                        std::string operand;
                        size_t pos = itVasm->_code.find(matchSequences[j]._sequence[0]);
                        if(pos != std::string::npos)
                        {
                            operand = itVasm->_code.substr(pos + matchSequences[j]._sequence[0].size());
                            if(operand == "0" || operand == "0x00")
                            {
                                // Migrate internal label to next available instruction
                                if(!migrateInternalLabel(i, vasmIndex, vasmIndex + 1)) break;

                                // Delete ADD/SUB
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + vasmIndex);
                                adjustLabelAddresses(i, vasmIndex, -2);
                                adjustVasmAddresses(i, vasmIndex, -2);
                            }
                        }
                    }

                    if(!linesDeleted)
                    {
                        // Only increment iterator if it has not been invalidated
                        ++itVasm;
                    }
                    else
                    {
                        // Optimising can cause new optimising opportunities to present, so restart
                        goto RESTART_OPTIMISE;
                    }
                }
            }
        }

        return true;
    }
}