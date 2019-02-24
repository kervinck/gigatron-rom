//
//  Gigatron.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 02.01.19.
//

#import <Foundation/Foundation.h>
#import "AppDelegate.h"
#import "Gigatron.h"
#import "AQPlayer.h"

#define DEBUG_GIGATRON   false
#define DEBUG_VCPU       false
#define CPU_HZ              6250000
#define AUDIO_SAMPLERATE    44100
#define SCREEN_WIDTH        160
#define SCREEN_HEIGHT       480

@implementation Gigatron

BOOL ramSizeIs64Kb = false;

uint8_t RAM[1<<16], IN;
ROM_OPCODE ROM[1<<16];

CpuState S;
long t;
long audioCycle;
int vgaX, vgaY;

BOOL frameCompleted;
BOOL haveNewVSync;
BOOL cpuHalted = false;
BOOL waitForNonVCPUOperation = false;
NSString *romFile;
NSURL *watchFile;
NSDate *watchFileLastModified;
Byte *framebuffer;
int oldVirtualProgramCounter = 0;
AQPlayer *player;
BOOL debugVCPU = false;
BOOL singleSteppingCPU = false;
BOOL singleSteppingvCPU = false;
BOOL singleStepCompleted = false;

- (id) initWithROMFile:(NSString *)myRomFile {
    self = [super init];
    IN = 0xff;
    framebuffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    romFile = myRomFile;
    
    [self reset];
    
    player = [[AQPlayer alloc] init];
    [player startPlaying];
    
    [NSTimer scheduledTimerWithTimeInterval:1.0f
             target:self selector:@selector(watchFile) userInfo:nil repeats:YES];
    
    return self;
}

- (void) dealloc {
    free(framebuffer);
}

- (void) watchFile {
    if(!watchFile) {
        return;
    }
    NSError *err = nil;
    NSFileManager *mgr= [NSFileManager defaultManager];
    NSDate *date = [[mgr attributesOfItemAtPath:[watchFile path] error:&err] fileModificationDate];
    if (err)
    {
        NSLog(@"Error occurred while getting attributes of %@, aborting. Details: %@", [watchFile path], err);
        return;
    }
    if(!watchFileLastModified || [watchFileLastModified isLessThan:date]) {
        NSLog(@"Watched file %@ changed, reloading...", [watchFile path]);
        watchFileLastModified = date;
        [self loadGt1File:watchFile watchFile:true];
    }
}

- (void) loadGt1File:(NSURL *)gt1File watchFile:(BOOL)watch {
    NSError *err = nil;
    
    if(watch) {
        watchFile = gt1File;
    } else {
        watchFile = nil;
    }
    
    NSFileHandle *handle = [NSFileHandle fileHandleForReadingFromURL:gt1File error:&err];
    if (err)
    {
        NSLog(@"Error occurred, aborting. Details: %@", err);
        return;
    }
    
    NSData *gt1Data = [handle readDataToEndOfFile];
    uint8_t *gt1Bytes = (uint8_t *)[gt1Data bytes];
    unsigned long gt1Length = [gt1Data length];
    
    // wait until gigatron does not process vCPU code anymore, see https://forum.gigatron.io/viewtopic.php?f=4&t=77
    // then halt the emulation until the file has been loaded
    waitForNonVCPUOperation = true;
    [self calculateFrame];
    waitForNonVCPUOperation = false;
    cpuHalted = true;
    
    /*    Gt1File :=
     n * ( <hiAddress>
     + <loAddress>
     + <segmentSize & 255>
     + segmentSize * <dataByte> )
     + <0>
     + <hiStart>
     + <loStart>
     EOF */
    
#define posInc      if (++pos >= gt1Length) { break; }
    
    int pos = 0;
    BOOL started = false;
    while(true) {
        int hiAddress = gt1Bytes[pos];
        posInc
        int loAddress = gt1Bytes[pos];
        posInc
        int segmentSize = gt1Bytes[pos] & 255;
        posInc
        
        int startAddress = (hiAddress << 8) | loAddress;
        NSLog(@"Writing 0x%02x bytes starting at address 0x%04x", segmentSize, startAddress);
        for(int i=0; i<segmentSize; i++) {
            RAM[startAddress + i] = gt1Bytes[pos];
            posInc
        }
        if(gt1Bytes[pos] == 0) {
            posInc
            int hiStart = gt1Bytes[pos];
            posInc
            int loStart = gt1Bytes[pos];
            pos++;
            if (pos != gt1Length) {
                NSLog(@"Didn't reach end of GT1 file while parsing!");
                break;
            } else {
                // Set virtual program counter
                NSLog(@"Starting program at 0x%02x%02x", hiStart, loStart);
                RAM[0x16] = loStart - 2;
                RAM[0x17] = hiStart;
                RAM[0x1a] = loStart - 2;
                RAM[0x1b] = hiStart;
                started = TRUE;
                debugVCPU = DEBUG_VCPU;
            }
            break;
        }
    }
    if (!started) {
        NSLog(@"Error parsing GT1 file!");
    }
    
    cpuHalted = false;
}

- (uint8_t) InRegister {
    return IN;
}

- (void) setInRegister:(uint8_t)newIn {
    IN = newIn;
}

- (int) screenHeight {
    return SCREEN_HEIGHT;
}

- (int) screenWidth {
    return SCREEN_WIDTH;
}

- (Byte *) framebuffer {
    return framebuffer;
}

- (void) setRamSizeTo64kB:(BOOL)is64Kb {
    ramSizeIs64Kb = is64Kb;
    
    [self reset];
}

- (BOOL) isRamSize64Kb {
    return ramSizeIs64Kb;
}

- (long) cpuCyclesSinceReset {
    return t;
}

- (void) reset {
    srand((unsigned int)time(NULL)); // Initialize with randomized data
    garble((void*)RAM, sizeof RAM);
    memset((void*)&S, 0, sizeof S);
    t = -2;
    audioCycle = 0;
    vgaX = 0;
    vgaY = 0;
    frameCompleted = false;
    haveNewVSync = true;
    
    garble((void*)ROM, sizeof ROM);
    
    FILE *fp = fopen([romFile cStringUsingEncoding:NSUTF8StringEncoding], "rb");
    if (!fp) {
        fprintf(stderr, "Error: failed to open ROM file\n");
        exit(EXIT_FAILURE);
    }
    fread(ROM, 1, sizeof ROM, fp);
    fclose(fp);
}

- (void) persistState {
    FILE *stateFile = fopen("gigatron.state", "w");
    if(stateFile) {
        fwrite((void*)ROM, sizeof ROM, sizeof(uint8_t), stateFile);
        fwrite((void*)RAM, sizeof RAM, sizeof(uint8_t), stateFile);
        fwrite((void*)&S, sizeof S, 1, stateFile);
        fwrite(&t, sizeof(t), 1, stateFile);
        fwrite(&vgaX, sizeof(vgaX), 1, stateFile);
        fwrite(&vgaY, sizeof(vgaY), 1, stateFile);
        fwrite(&frameCompleted, sizeof(frameCompleted), 1, stateFile);
        fwrite(&haveNewVSync, sizeof(haveNewVSync), 1, stateFile);
        fclose(stateFile);
    } else {
        NSLog(@"Couldn't open 'gigatron.state' for writing!");
    }
}

- (void) restoreState {
    FILE *stateFile = fopen("gigatron.state", "r");
    if(stateFile) {
        fread((void*)ROM, sizeof ROM, sizeof(uint8_t), stateFile);
        fread((void*)RAM, sizeof RAM, sizeof(uint8_t), stateFile);
        fread((void*)&S, sizeof S, 1, stateFile);
        fread(&t, sizeof(t), 1, stateFile);
        fread(&vgaX, sizeof(vgaX), 1, stateFile);
        fread(&vgaY, sizeof(vgaY), 1, stateFile);
        fread(&frameCompleted, sizeof(frameCompleted), 1, stateFile);
        fread(&haveNewVSync, sizeof(haveNewVSync), 1, stateFile);
        fclose(stateFile);
    } else {
        NSLog(@"Couldn't open 'gigatron.state' for reading!");
    }
}

- (CpuState) cpuState {
    CpuState cpuStateCopy = S;
    return cpuStateCopy;
}

- (uint8_t *)getRAM {
    return RAM;
}

- (ROM_OPCODE *)getROM {
    return ROM;
}

- (void) setVolume:(int)volume {
    [player setVolume:volume];
}

- (void) pause {
    cpuHalted = true;
    [player stopPlaying];
}

- (void) resume {
    cpuHalted = false;
    singleSteppingvCPU = false;
    singleSteppingCPU = false;
    singleStepCompleted = false;
    AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
    [delegate updateGigatronRefreshFromNSPopupButton];
    [player startPlaying];
}

- (void) singleStepCPU {
    singleSteppingCPU = YES;
    singleSteppingvCPU = NO;
    singleStepCompleted = NO;
}

- (void) singleStepvCPU {
    singleSteppingCPU = NO;
    singleSteppingvCPU = YES;
    singleStepCompleted = NO;
}

- (void) calculateFrame {
    if(cpuHalted ||
       ((singleSteppingvCPU || singleSteppingCPU ) && singleStepCompleted)) {
        return;
    }
    
    frameCompleted = false;
    
    // Usually, we return once a frame has been completed. To stay responsive in cases where
    // no vSync is sent due to errors in the executed ROM or RAM, we return also after
    // at most a CPU-second of calculations.
    long cycleCountdown = CPU_HZ;
    do {
        t++;
        if (t < 0) S.PC = 0; // MCP100 Power-On Reset
        
        CpuState T = cpuCycle(S); // Update CPU
        
        if(DEBUG_GIGATRON) {
            NSLog(@"--- PC %04x: %02x%02x IR:%02x D:%02x AC:%02x X:%02x Y:%02x OUT:%02x UNDEF:%02x\n",
                   S.PC, ROM[S.PC].instruction, ROM[S.PC].operand, S.IR, S.D, S.AC, S.X, S.Y, S.OUT, S.undef);
        }
        
        int newVirtualProgramCounter = RAM[0x17] << 8 | RAM[0x16];
        if(newVirtualProgramCounter != oldVirtualProgramCounter) {
            // program counter in our virtual CPU changed
            if(debugVCPU) {
                [self logVpcState];
            }
            if(self.breakAddress == newVirtualProgramCounter) {
                singleSteppingCPU = NO;
                singleSteppingvCPU = YES;
                singleStepCompleted = YES;
                AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
                delegate.wasRunning = NO;
                delegate.refreshInterval = 0;
            }
            if(singleSteppingvCPU) {
                singleStepCompleted = YES;
            }
        }
        oldVirtualProgramCounter = RAM[0x17] << 8 | RAM[0x16];
    
        int hSync = (T.OUT & 0x40) - (S.OUT & 0x40); // "VGA monitor" (use simple stdout)
        int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
        if (vSync < 0) {
            vgaY = -36; // Falling vSync edge
            if(haveNewVSync) {
                frameCompleted = true;
                haveNewVSync = false;
            }
        } else {
            haveNewVSync = true;
        }
        if (vgaX++ < 200) {
            if(DEBUG_GIGATRON) {
                if (hSync) putchar('|');              // Visual indicator of hSync
                else if (vgaX == 200) putchar('>');   // Too many pixels
                else if (~S.OUT & 0x80) putchar('^'); // Visualize vBlank pulse
            }
            if (!hSync && vgaX != 200 && !(~S.OUT & 0x80)){
                // putchar(32 + (S.OUT & 63));      // Plot pixel
                int outputX = vgaX - 13;
                if(outputX >= 0 && outputX < SCREEN_WIDTH && vgaY >= 0 && vgaY < SCREEN_HEIGHT) {
                    int pixelAddress = vgaY*SCREEN_WIDTH*4 + outputX*4;
                    framebuffer[pixelAddress] = (S.OUT & 0x03) * 85;
                    framebuffer[pixelAddress + 1] = ((S.OUT & 0x0C) >> 2) * 85;
                    framebuffer[pixelAddress + 2] = ((S.OUT & 0x30) >> 4) * 85;
                }
            }
        }
        if (hSync > 0) { // Rising hSync edge
            if(DEBUG_GIGATRON) {
                NSLog(@"%s line %-3d xout %02x t %0.3f\n",
                       vgaX!=200 ? "~" : "", // Mark horizontal cycle errors
                       vgaY, T.AC, t/6.250e+06);
            }
            vgaX = 0;
            vgaY++;
            T.undef = rand() & 0xff; // Change this once in a while
            T.OUTX = T.AC;
        }
        S=T;
        
        audioCycle += AUDIO_SAMPLERATE;
        if(audioCycle >= CPU_HZ) {
            audioCycle -= CPU_HZ;
            uint8_t sample = ((S.OUTX >> 4) << 2) + 128;
            short samples[2];
            samples[0] = (short)sample << 8;
            samples[1] = (short)sample << 8;
            [player addSampleLeftRightTuple:samples];
        }
        
        if(singleSteppingCPU) {
            singleStepCompleted = YES;
        }
    } while(!frameCompleted
            && --cycleCountdown > 0
            && !cpuHalted
            && !(waitForNonVCPUOperation && S.PC < 0x0200)
            && !singleStepCompleted);
    
    AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
    [delegate refreshWindows];
}

CpuState cpuCycle(const CpuState S)
{
    CpuState T = S; // New state is old state unless something changes
    
    T.IR = ROM[S.PC].instruction; // Instruction Fetch
    T.D  = ROM[S.PC].operand;
    
    int ins = S.IR >> 5;       // Instruction
    int mod = (S.IR >> 2) & 7; // Addressing mode (or condition)
    int bus = S.IR&3;          // Busmode
    int W = (ins == 6);        // Write instruction?
    int J = (ins == 7);        // Jump instruction?
    
    uint8_t lo=S.D, hi=0, *to=NULL; // Mode Decoder
    int incX=0;
    if (!J)
        switch (mod) {
#define E(p) (W?0:p) // Disable AC and OUT loading during RAM write
            case 0: to=E(&T.AC);                          break;
            case 1: to=E(&T.AC); lo=S.X;                  break;
            case 2: to=E(&T.AC);         hi=S.Y;          break;
            case 3: to=E(&T.AC); lo=S.X; hi=S.Y;          break;
            case 4: to=  &T.X;                            break;
            case 5: to=  &T.Y;                            break;
            case 6: to=E(&T.OUT);                         break;
            case 7: to=E(&T.OUT); lo=S.X; hi=S.Y; incX=1; break;
        }
    uint16_t addr = (hi << 8) | lo;
    
    int B = S.undef; // Data Bus
    switch (bus) {
        case 0: B=S.D;                        break;
        case 1: if (!W) B = RAM[addr&(ramSizeIs64Kb ? 0xffff : 0x7fff)]; break;
        case 2: B=S.AC;                       break;
        case 3: B=IN;                         break;
    }
    
    if (W) RAM[addr&(ramSizeIs64Kb ? 0xffff : 0x7fff)] = B; // Random Access Memory
    
    uint8_t ALU; // Arithmetic and Logic Unit
    switch (ins) {
        case 0: ALU =        B; break; // LD
        case 1: ALU = S.AC & B; break; // ANDA
        case 2: ALU = S.AC | B; break; // ORA
        case 3: ALU = S.AC ^ B; break; // XORA
        case 4: ALU = S.AC + B; break; // ADDA
        case 5: ALU = S.AC - B; break; // SUBA
        case 6: ALU = S.AC;     break; // ST
        case 7: ALU = -S.AC;    break; // Bcc/JMP
        default: ALU = 0; break;
    }
    
    if (to) *to = ALU; // Load value into register
    if (incX) T.X = S.X + 1; // Increment X
    
    T.PC = S.PC + 1; // Next instruction
    if (J) {
        if (mod != 0) { // Conditional branch within page
            int cond = (S.AC>>7) + 2*(S.AC==0);
            if (mod & (1 << cond)) // 74153
                T.PC = (S.PC & 0xff00) | B;
        } else
            T.PC = (S.Y << 8) | B; // Unconditional far jump
    }
    return T;
}

void garble(uint8_t mem[], int len)
{
    for (int i=0; i<len; i++) mem[i] = rand();
    // memset(mem, 0, len);
}


- (void) logVpcState {
    int vPC = RAM[0x17] << 8 | RAM[0x16];
    switch(RAM[vPC]) {
        case 0x5e: NSLog(@"vPC: 0x%02x%02x  ST   0x%02x%02x  vAC=%02x%02x", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1], RAM[0x19], RAM[0x18]); break;
        case 0x2b: NSLog(@"vPC: 0x%02x%02x  STW  0x%02x%02x  vAC=%02x%02x", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1], RAM[0x19], RAM[0x18]); break;
        case 0xec: NSLog(@"vPC: 0x%02x%02x  STLW 0x%02x%02x  vAC=%02x%02x", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1], RAM[0x19], RAM[0x18]); break;
        case 0x1a: NSLog(@"vPC: 0x%02x%02x  LD   0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x59: NSLog(@"vPC: 0x%02x%02x  LDI  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0x11: NSLog(@"vPC: 0x%02x%02x  LDWI 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x21: NSLog(@"vPC: 0x%02x%02x  LDW  0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xee: NSLog(@"vPC: 0x%02x%02x  LDLW 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xe3: NSLog(@"vPC: 0x%02x%02x  ADDI 0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0x99: NSLog(@"vPC: 0x%02x%02x  ADDW 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xe6: NSLog(@"vPC: 0x%02x%02x  SUBI 0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xb8: NSLog(@"vPC: 0x%02x%02x  SUBW 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x83: NSLog(@"vPC: 0x%02x%02x  ANDI 0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xf8: NSLog(@"vPC: 0x%02x%02x  ANDW 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x88: NSLog(@"vPC: 0x%02x%02x  ORI  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xfa: NSLog(@"vPC: 0x%02x%02x  ORW  0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x8c: NSLog(@"vPC: 0x%02x%02x  XORI 0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xfc: NSLog(@"vPC: 0x%02x%02x  XORW 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x93: NSLog(@"vPC: 0x%02x%02x  INC  0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xad: NSLog(@"vPC: 0x%02x%02x  PEEK", RAM[0x17], RAM[0x16]); break;
        case 0xf6: NSLog(@"vPC: 0x%02x%02x  DEEK", RAM[0x17], RAM[0x16]); break;
        case 0xf0: NSLog(@"vPC: 0x%02x%02x  POKE 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xf3: NSLog(@"vPC: 0x%02x%02x  DOKE 0x%02x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xe9: NSLog(@"vPC: 0x%02x%02x  LSLW", RAM[0x17], RAM[0x16]); break;
        case 0x35:
            switch(RAM[vPC+1]) {
                case 0x3f: NSLog(@"vPC: 0x%02x%02x  BEQ  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                case 0x4d: NSLog(@"vPC: 0x%02x%02x  BGT  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                case 0x50: NSLog(@"vPC: 0x%02x%02x  BLT  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                case 0x54: NSLog(@"vPC: 0x%02x%02x  BGE  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                case 0x56: NSLog(@"vPC: 0x%02x%02x  BLE  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                case 0x73: NSLog(@"vPC: 0x%02x%02x  BNE  0x%02x  ", RAM[0x17], RAM[0x16], RAM[vPC+2]); break;
                default: NSLog(@"!!! Unknown Branch opcode %02x", RAM[vPC+1]); break;
            } break;
        case 0x90: NSLog(@"vPC: 0x%02x%02x  BRA  0x%02x%02x", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0x7f: NSLog(@"vPC: 0x%02x%02x  LUP  0x%02x", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xcf: NSLog(@"vPC: 0x%02x%02x  CALL 0x%02x%02x", RAM[0x17], RAM[0x16], RAM[vPC+2], RAM[vPC+1]); break;
        case 0xff: NSLog(@"vPC: 0x%02x%02x  RET", RAM[0x17], RAM[0x16]); break;
        case 0x75: NSLog(@"vPC: 0x%02x%02x  PUSH", RAM[0x17], RAM[0x16]); break;
        case 0x63: NSLog(@"vPC: 0x%02x%02x  POP", RAM[0x17], RAM[0x16]); break;
        case 0xdf: NSLog(@"vPC: 0x%02x%02x  ALLOC %02x", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xcd: NSLog(@"vPC: 0x%02x%02x  DEF %02x", RAM[0x17], RAM[0x16], RAM[vPC+1]); break;
        case 0xb4: NSLog(@"vPC: 0x%02x%02x  SYS", RAM[0x17], RAM[0x16]); break;
        default: NSLog(@"!!! Unknown opcode %02x", RAM[vPC]);
    }
}

@end
