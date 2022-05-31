#ifndef __HARNESS_H__
#define __HARNESS_H__

typedef void (*SysHook)(void);

extern uint8_t ROM[1<<16][2], RAM[1<<15];
extern void runTest(char* romFile, char* testFile, char* logFile, SysHook sys1);


#endif
