#ifndef VALIDATER_H
#define VALIDATER_H


namespace Validater
{
    bool initialise(void);

    bool checkForRelocations(void);
    bool checkBranchLabels(void);
    bool checkStatementBlocks(void);
}

#endif
