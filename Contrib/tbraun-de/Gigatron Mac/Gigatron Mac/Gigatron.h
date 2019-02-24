//
//  Gigatron.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 02.01.19.
//

#ifndef Gigatron_h
#define Gigatron_h

typedef struct { // TTL state that the CPU controls
    uint16_t PC;
    uint8_t IR, D, AC, X, Y, OUT, OUTX, undef;
} CpuState;

typedef struct {
    uint8_t instruction;
    uint8_t operand;
} ROM_OPCODE;

@interface Gigatron : NSObject

@property (readonly) const int screenWidth;
@property (readonly) const int screenHeight;
@property (readonly) Byte* framebuffer;
@property (assign) uint8_t InRegister;
@property (assign) int breakAddress;

- (id) initWithROMFile:(NSString *)romFile;
- (void) calculateFrame;
- (void) loadGt1File:(NSURL *)gt1File watchFile:(BOOL) watch;
- (void) setRamSizeTo64kB:(BOOL)is64Kb;
- (BOOL) isRamSize64Kb;
- (uint8_t *)getRAM;
- (ROM_OPCODE *)getROM;
- (CpuState) cpuState;
- (long) cpuCyclesSinceReset;
- (void) setVolume:(int)volume;
- (void) reset;
- (void) persistState;
- (void) restoreState;
- (void) pause;
- (void) resume;
- (void) singleStepCPU;
- (void) singleStepvCPU;

@end

#endif /* Gigatron_h */
