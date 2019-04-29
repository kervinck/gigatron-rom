#ifndef __HARNESS_H__
#define __HARNESS_H__

extern uint8_t ROM[1<<16][2], RAM[1<<15];
extern void runTest(char* romFile, char* testFile, char* logFile);


#endif
