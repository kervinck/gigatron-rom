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
    enum OptimiseTypes {StwLdwPair=0, StwLdPair, StwStHigh, ExtraLdw, ExtraLd, LdwPair, StwLdiAddw, StwLdwAddw, StwLdwAddwVar, StwLdiAndw, StwLdwAndw, StwLdwAndwVar, StwLdiXorw, 
                        StwLdwXorw, StwLdwXorwVar, StwLdiOrw, StwLdwOrw, StwLdwOrwVar, LdwBeqStwTmp, LdwBeqStwVar, LdBeqStTmp, LdBeqStVar, PokeVar, DokeVar, StwPair, StwPairReg,
                        ExtraStw, PeekArrayB, PeekArray, DeekArray, PokeArray, DokeArray, PokeVarArrayB, PokeVarArray, DokeVarArray, PokeTmpArrayB, PokeTmpArray, DokeTmpArray, 
                        StwPokeArray, StwDokeArray, LdSubLoHi, LdiSubLoHi, LdwSubLoHi, AddiPair, AddiZero, SubiZero, NumOptimiseTypes};

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

        // ExtraLd
        {0, 1, {"ST" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_",
                "LD" + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_"  }},

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

        // LdwBeqStwTmp
        {0, 2, {"LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "BEQ" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x"}},

        // LdwBeqStwVar
        {0, 2, {"LDW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_", 
                "BEQ" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"}},

        // LdBeqStTmp
        {0, 2, {"LD"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0x", 
                "BEQ" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "ST"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "0x"}},

        // LdBeqStVar
        {0, 2, {"LD"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_", 
                "BEQ" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "ST"  + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_"}},

        // PokeVar
        {0, 2, {"LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "", 
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + ""}},

        // DokeVar
        {0, 2, {"LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "", 
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + ""}},

        // StwPair
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x" }},

        // StwPairReg
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem"}},

        // ExtraStw
        {0, 0, {"STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x", 
                "STW" + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_"  }},

        // PeekArrayB
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem", 
                "PEEK" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // PeekArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem", 
                "PEEK" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // DeekArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem", 
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem", 
                "DEEK" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + ""}},

        // PokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x", 
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem", 
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // DokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // PokeVarArrayB
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // PokeVarArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // DokeVarArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // PokeTmpArrayB
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // PokeTmpArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // DokeTmpArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDWI" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "0x",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "ADDW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "mem",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // StwPokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "POKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // StwDokeArray
        {0, 0, {"STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "0x",
                "DOKE" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "mem"}},

        // LdSubLoHi
        {0, 0, {"LD"   + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LD"   + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "SUBW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // LdiSubLoHi
        {0, 0, {"LDI"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LD"   + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "SUBW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

        // LdwSubLoHi
        {0, 0, {"LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LD"   + std::string(OPCODE_TRUNC_SIZE - 2, ' ') + "_",
                "STW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "LDW"  + std::string(OPCODE_TRUNC_SIZE - 3, ' ') + "reg",
                "SUBW" + std::string(OPCODE_TRUNC_SIZE - 4, ' ') + "reg"}},

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
            if(int(Compiler::getCodeLines()[index]._vasm.size()) <= newLine) return false;
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
            if(vasmLineIndex >= int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()))
            {
                if(++codeLineIndex >= int(Compiler::getCodeLines().size())) return;
                vasmLineIndex = 0;
            }
        }
        while(Compiler::getCodeLines()[codeLineIndex]._vasm.size() == 0);

        uint16_t optimisedAddress = Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._address;

        for(int i=0; i<int(Compiler::getLabels().size()); i++)
        {
            if(Compiler::getLabels()[i]._address >= optimisedAddress)
            {
                Compiler::getLabels()[i]._address += int16_t(offset);
            }
        }
    }

    // Adjust vasm code addresses
    void adjustVasmAddresses(int codeLineIndex, int vasmLineIndex, int offset)
    {
        // Loop through commented out code
        do
        {
            if(vasmLineIndex >= int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()))
            {
                if(++codeLineIndex >= int(Compiler::getCodeLines().size())) return;
                vasmLineIndex = 0;
            }
        }
        while(Compiler::getCodeLines()[codeLineIndex]._vasm.size() == 0);

        for(int i=codeLineIndex; i<int(Compiler::getCodeLines().size()); i++)
        {
            int start = (i == codeLineIndex) ? vasmLineIndex : 0;
            for(int j=start; j<int(Compiler::getCodeLines()[i]._vasm.size()); j++)
            {
                Compiler::getCodeLines()[i]._vasm[j]._address += int16_t(offset);
            }
        }
    }

    void updateVasm(int codeLineIndex, int vasmLineIndex, const std::string& opcode, const std::string& operand)
    {
        Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._opcode = opcode;
        Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._operand = operand;
        Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._code = opcode + std::string(OPCODE_TRUNC_SIZE - opcode.size(), ' ') + operand;
        Compiler::getCodeLines()[codeLineIndex]._vasm[vasmLineIndex]._vasmSize = Assembler::getAsmOpcodeSize(opcode);
    }

    void updateVasm(std::vector<Compiler::VasmLine>::iterator& it, int offset, const std::string& opcode, const std::string& operand)
    {
        (it + offset)->_opcode = opcode;
        (it + offset)->_operand = operand;
        (it + offset)->_code = opcode + std::string(OPCODE_TRUNC_SIZE - opcode.size(), ' ') + operand;
        (it + offset)->_vasmSize = Assembler::getAsmOpcodeSize(opcode);
    }

    bool optimiseCode(void)
    {

RESTART_OPTIMISE:
        for(int i=0; i<int(Compiler::getCodeLines().size()); i++)
        {
            for(int j=0; j<int(matchSequences.size()); j++)
            {
                for(auto itVasm=Compiler::getCodeLines()[i]._vasm.begin(); itVasm!=Compiler::getCodeLines()[i]._vasm.end();)
                {
                    bool linesDeleted = false;
                    int vasmIndex = int(itVasm - Compiler::getCodeLines()[i]._vasm.begin());

                    // Can only optimise within a BASIC code line, (use multi-statements to optimise across lines)
                    int vasmIndexMax = vasmIndex + int(matchSequences[j]._sequence.size()) - 1;
                    if(vasmIndexMax >= int(Compiler::getCodeLines()[i]._vasm.size()))
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
                        std::string firstOperand = Compiler::getCodeLines()[i]._vasm[firstLine]._operand;

                        // Second operand
                        int secondIndex = matchSequences[j]._secondIndex;
                        int secondLine = vasmIndex + secondIndex;
                        std::string secondOperand = Compiler::getCodeLines()[i]._vasm[secondLine]._operand;

/*************************************************************************************************************************************************************/
/* Opcode matches required, operand matches required                                                                                                         */
/*************************************************************************************************************************************************************/

                        // Find operand match, (temporary variables are a minimum of 4 chars, i.e. '0xd0')
                        //if(firstOperand.substr(0, 4) == secondOperand.substr(0, 4))
                        if(firstOperand == secondOperand)
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
                                    adjustVasmAddresses(i, firstLine, -2);
                                }
                                break;

                                // Match STW/ST LDW/LD, delete LDW/LD
                                case ExtraLdw:
                                case ExtraLd:
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

                                    // Replace ADDW with ADDI and replace ADDW's operand with LDI's operand
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ADDI", ldiOperand);

                                    // Delete STW and LDI
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

                                    // Replace ADDW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ADDW", ldwOperand);

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

                                    // Replace ADDW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ADDW", ldwOperand);

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

                                    // Replace ANDW with ANDI and replace ANDW's operand with LDI's operand
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ANDI", ldiOperand);

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

                                    // Replace ANDW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ANDW", ldwOperand);

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

                                    // Replace ANDW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ANDW", ldwOperand);

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

                                    // Replace XORW with XORI and replace XORW's operand with LDI's operand
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "XORI", ldiOperand);

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

                                    // Replace XORW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "XORW", ldwOperand);

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

                                    // Replace XORW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "XORW", ldwOperand);

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

                                    // Replace ORW with ORI and replace ORW's operand with LDI's operand
                                    std::string ldiOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ORI", ldiOperand);

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

                                    // Replace ORW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ORW", ldwOperand);

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

                                    // Replace ORW's operand with LDW's operand
                                    std::string ldwOperand = Compiler::getCodeLines()[i]._vasm[firstLine + 1]._operand;
                                    updateVasm(i, firstLine + 2, "ORW", ldwOperand);

                                    // Delete STW LDW
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                    adjustLabelAddresses(i, firstLine, -4);
                                    adjustVasmAddresses(i, firstLine, -4);
                                }
                                break;

                                // Match LDW/LD BEQ STW/ST, delete STW/ST
                                case LdwBeqStwTmp:
                                case LdwBeqStwVar:
                                case LdBeqStTmp:
                                case LdBeqStVar:
                                {
                                    // STW/ST can have an internal label
                                    if(!migrateInternalLabel(i, firstLine + 2, firstLine + 3)) break;

                                    // Delete STW/ST
                                    linesDeleted = true;
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 2);
                                    adjustLabelAddresses(i, firstLine + 2, -2);
                                    adjustVasmAddresses(i, firstLine + 2, -2);
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
                                    adjustVasmAddresses(i, firstLine + 2, -2);
                                }
                                break;

                                default: break;
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

                            // Match LDW STW LDWI ADDW PEEK end up with LDWI ADDW PEEK
                            case PeekArrayB:
                            {
                                // Save previous line LDW, if opcode is not LDW then can't optimise
                                if(firstLine - 1 < 0) break;
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLDW._opcode != "LDW") break;

                                // Migrate it's label if it has one
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;

                                // Delete previous line LDW and first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of ADDW with LDW's operand
                                updateVasm(itVasm, 1, "ADDW", savedLDW._operand);
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            break;

                            // Match LDW STW LDWI ADDW ADDW PEEK/DEEK end up with LDWI ADDW ADDW PEEK/DEEK
                            case PeekArray:
                            case DeekArray:
                            {
                                // Save previous line LDW, if opcode is not LDW then can't optimise
                                if(firstLine - 1 < 0) break;
                                Compiler::VasmLine savedLDW = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLDW._opcode != "LDW") break;

                                // Migrate it's label if it has one
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;

                                // Delete previous line LDW and first STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of both ADDW's with LDW's operand
                                updateVasm(itVasm, 1, "ADDW", savedLDW._operand);
                                updateVasm(itVasm, 2, "ADDW", savedLDW._operand);
                                adjustLabelAddresses(i, firstLine - 1, -4);
                                adjustVasmAddresses(i, firstLine - 1, -4);
                            }
                            break;

                            // Match LD<X> STW LDWI STW LDW POKE/DOKE end up with LDWI STW LD<X> POKE/DOKE
                            case PokeArray:
                            case DokeArray:
                            {
                                uint16_t offset = 9;

                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise
                                if(firstLine - 1 < 0) break;
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine - 1];
                                if(savedLD._opcode.find("LD") == std::string::npos) break;
                                if(savedLD._opcode.find("LDWI") != std::string::npos) offset += 1;

                                // Discard it's label, (it's no longer needed), and adjust it's address
                                if(!migrateInternalLabel(i, firstLine - 1, firstLine + 1)) break;
                                savedLD._internalLabel = "";
                                savedLD._address += offset; // LD<X> is moved 9bytes, LDWI is moved 10 bytes

                                // Delete previous line LD<X>, first STW and LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine - 1);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 2);

                                // Replace LDW with saved LD<X> and operand
                                if(offset == 9)
                                {
                                    itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                    adjustLabelAddresses(i, firstLine - 1, -4);
                                    adjustVasmAddresses(i, firstLine - 1, -4);
                                }
                                // LDW is replaced with LDWI so push everything forward starting at the POKE/DOKE by 1 more byte
                                else
                                {
                                    itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
                                    adjustLabelAddresses(i, firstLine - 1, -5);
                                    adjustVasmAddresses(i, firstLine - 1, -5);
                                    adjustLabelAddresses(i, firstLine + 2, 1);
                                    adjustVasmAddresses(i, firstLine + 2, 1);
                                }
                            }
                            break;

                            // Match LD<X> STW LDW STW LDWI ADDW STW LDW POKE
                            case PokeVarArrayB:
                            case PokeTmpArrayB:
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise first phase
                                if(firstLine - 1 < 0) break;
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

                                // Delete first LDW and second STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of ADDW with LDW's operand
                                updateVasm(itVasm, 1, "ADDW", savedLDW._operand);
                                adjustLabelAddresses(i, firstLine, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            break;

                            // Match STW LDW STW LDWI ADDW ADDW STW LDW POKE/DOKE
                            case PokeVarArray:
                            case PokeTmpArray:
                            case DokeVarArray:
                            case DokeTmpArray:
                            {
                                // Save previous line LD<X>, if opcode is not some sort of LD then can't optimise first phase
                                if(firstLine - 1 < 0) break;
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
                                    itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 6); //points to last LDW, after LD<X> and STW were deleted

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

                                // Delete first LDW and second STW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);

                                // Replace operand of both ADDW's
                                updateVasm(itVasm, 1, "ADDW", savedLDW._operand);
                                updateVasm(itVasm, 2, "ADDW", savedLDW._operand);
                                adjustLabelAddresses(i, firstLine, -4);
                                adjustVasmAddresses(i, firstLine, -4);
                            }
                            break;

                            case StwPokeArray:
                            case StwDokeArray:
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

                            // Match LD/LDW STW LD STW LDW SUBW
                            case LdSubLoHi:
                            case LdiSubLoHi:
                            case LdwSubLoHi:
                            {
                                // Save LD<X>
                                Compiler::VasmLine savedLD = Compiler::getCodeLines()[i]._vasm[firstLine];

                                // Migrate LD<X>'s label to LD
                                if(!migrateInternalLabel(i, firstLine, firstLine + 2)) break;
                                savedLD._internalLabel = "";
                                savedLD._address += 8; // LD<X> is moved 8 bytes

                                // Delete LD<X>, first STW and last LDW
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm);
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(itVasm + 2); //points to last LDW, after LD<X> and STW were deleted

                                // Replace LDW with saved LD<X> and operand
                                itVasm = Compiler::getCodeLines()[i]._vasm.insert(itVasm, savedLD);
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
                                std::string operand = vasm._operand;
                                Expression::stringToU8(operand, addi0);
                                vasm = Compiler::getCodeLines()[i]._vasm[firstLine + 1];
                                operand = vasm._operand;
                                Expression::stringToU8(operand, addi1);

                                // Result can't fit in an ADDI operand so exit, (ADDI can't be -ve so result can't be -ve)
                                uint16_t result = addi0 + addi1;
                                if(result > 255) break;

                                // Delete second ADDI
                                linesDeleted = true;
                                itVasm = Compiler::getCodeLines()[i]._vasm.erase(Compiler::getCodeLines()[i]._vasm.begin() + firstLine + 1);

                                // Replace first ADDI's operand with sum of previous two ADDI's
                                updateVasm(itVasm, -1, "ADDI", std::to_string(uint8_t(result)));
                                adjustLabelAddresses(i, firstLine + 1, -2);
                                adjustVasmAddresses(i, firstLine + 1, -2);
                            }
                            break;

                            default: break;
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