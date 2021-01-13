#ifndef VALIDATER_H
#define VALIDATER_H


namespace Validater
{
    bool initialise(void);

    void adjustLabelAddresses(uint16_t address, int offset);
    void adjustVasmAddresses(int codeLineIndex, uint16_t address, int offset);

    bool checkForRelocations(void);
    bool checkBranchLabels(void);
    bool checkStatementBlocks(void);
    bool checkCallProcFuncData(void);
    bool checkRuntimeVersion(void);
}

#endif
