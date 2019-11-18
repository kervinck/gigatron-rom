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
    enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwPair, StwPairReg, ExtraStw, ExtraLdw, StwLdwAddw, StwLdwAddwVar, StwLdwAndw, StwLdwAndwVar, StwLdwXorw, StwLdwXorwVar,
                        StwLdwOrw, StwLdwOrwVar, PeekArray, DeekArray, PokeArray, DokeArray, PokeVarArray, DokeVarArray, PokeTmpArray, DokeTmpArray, AddiZero, SubiZero, NumOptimiseTypes};

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

        // StwPair
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x" }},

        // StwPairReg
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg"}},

        // ExtraStw
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"  }},

        // ExtraLdw
        {0, 1, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",  
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"  }},

        // StwLdwAddw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAddwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAndw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwAndwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "ANDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwXorw
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwXorwVar
        {0, 2, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "XORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x"}},

        // StwLdwOrw
        {0, 2, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "ORW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

        // StwLdwOrwVar
        {0, 2, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "ORW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

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

        {0, 0, {"ADDI" + std::string(OPCODE_TRUNC_SIZE - 4, ' '), ""}},
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

                    linesDeleted = false;
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

                        // Find operand match
                        if(firstOperand == secondOperand)
                        {
                            // Match STW LDW, delete STW LDW
                            if(j == StwLdwPair  ||  j == StwLdPair)
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
                            
                            // Match STW LDW, delete LDW
                            else if(j == ExtraLdw)
                            {
                                // Migrate internal label to next available instruction
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // Delete LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                adjustLabelAddresses(i, firstLine + 1, -2);
                                adjustVasmAddresses(i, firstLine + 1, -2);
                            }

                            // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW
                            else if(j == StwLdwAddw)
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
                            // Match STW LDW ADDW, copy LDW operand to ADDW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwAddwVar)
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
                            // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW
                            else if(j == StwLdwAndw)
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
                            // Match STW LDW ANDW, copy LDW operand to ANDW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwAndwVar)
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
                            // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW
                            else if(j == StwLdwXorw)
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
                            // Match STW LDW XORW, copy LDW operand to XORW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwXorwVar)
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
                            // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW
                            else if(j == StwLdwOrw)
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
                            // Match STW LDW ORW, copy LDW operand to ORW operand and delete STW LDW, (LDW is a var)
                            else if(j == StwLdwOrwVar)
                            {
                                // Only one of these can have an internal label
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                if(!migrateInternalLabel(i, firstLine + 1, firstLine + 2)) break;

                                // ORW's operand becomes the LDW's operand
                                size_t ldwSpace = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);
                                Compiler::getCodeLines()[i]._vasm[firstLine + 2]._code = "ORW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;

                                // Delete STW and LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                adjustLabelAddresses(i, firstLine, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            // Match STW LDWI ADDW ADDW PEEK/DEEK
                            else if(j == PeekArray  ||  j == DeekArray)
                            {
                                // Save previous line LDW, if opcode is not LDW then can't optimise
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLDW._opcode != "LDW") break;

                                // Migrate it's label if it has one
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;

                                // Get saved LD<X>'s operand
                                size_t ldwSpace = savedLDW._code.find_first_of("  \n\r\f\t\v");
                                std::string ldwOperand = savedLDW._code.substr(ldwSpace);
                                Expression::stripWhitespace(ldwOperand);

                                // Delete previous line LD<X> and first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of both ADDW's
                                (itVasm + 1)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                (itVasm + 2)->_code = "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ldwOperand;
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            // Match STW LDWI STW LDW POKE/DOKE
                            else if(j == PokeArray  ||  j == DokeArray)
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") == std::string::npos) break;

                                // Discard it's label, (it's no longer needed), and adjust it's address
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;
                                savedLD._internalLabel = "";
                                savedLD._address += 9;

                                // Delete previous line LD<X>, first STW and LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 2);

                                // Replace LDW with saved opcode and operand
                                itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            // Match STW LDW STW LDWI ADDW ADDW STW LDW POKE/DOKE
                            else if(j == PokeVarArray  ||  j == DokeVarArray  ||  j == PokeTmpArray  ||  j == DokeTmpArray)
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") == std::string::npos) break;

                                // Discard it's label, (it's no longer needed), and adjust it's address
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;
                                savedLD._internalLabel = "";
                                savedLD._address += 17;

                                // Delete previous line LDX<X>, first STW and last LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 6);

                                // Replace LDW with saved opcode and operand
                                itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                        }

                        // Extra STW, (doesn't require an operand match)
                        if(j == StwPair  ||  j == StwPairReg  ||  j == ExtraStw)
                        {
                            // Migrate internal label to next available instruction
                            if(!migrateInternalLabel(i, firstLine, firstLine + 1)) break;

                            // Delete first STW
                            linesDeleted = true;
                            itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                            adjustLabelAddresses(i, firstLine, -2);
                            adjustVasmAddresses(i, firstLine, -2);
                        }
                    }

                    // Arithmetic with zero, (doesn't require an opcode match)
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

                    if(!linesDeleted) ++itVasm;
                }
            }
        }

        return true;
    }
}