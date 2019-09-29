//
//  CPUWindow2.m
//  Gigatron Mac
//
//

#import "CPUWindow.h"
#import "AppDelegate.h"

@interface CPUWindow ()

@property (assign) IBOutlet NSTextField *pcLabel;
@property (assign) IBOutlet NSTextField *irLabel;
@property (assign) IBOutlet NSTextField *dLabel;
@property (assign) IBOutlet NSTextField *acLabel;
@property (assign) IBOutlet NSTextField *xLabel;
@property (assign) IBOutlet NSTextField *yLabel;
@property (assign) IBOutlet NSTextField *outLabel;
@property (assign) IBOutlet NSTextField *outxLabel;
@property (assign) IBOutlet NSTextField *undefLabel;
@property (assign) IBOutlet NSTextField *inLabel;
@property (assign) IBOutlet NSTextField *vPCLabel;
@property (assign) IBOutlet NSTextField *vACLabel;
@property (assign) IBOutlet NSTextField *vLRLabel;
@property (assign) IBOutlet NSTextField *vSPLabel;
@property (assign) IBOutlet NSTextField *sysFnLabel;
@property (assign) IBOutlet NSTextField *breakAtTextField1;
@property (assign) IBOutlet NSTextField *breakAtTextField2;
@property (assign) IBOutlet NSTextField *breakAtTextField3;
@property (assign) IBOutlet NSButton *breakpointRadioButton1;
@property (assign) IBOutlet NSButton *breakpointRadioButton2;
@property (assign) IBOutlet NSButton *breakpointRadioButton3;
@property (assign) IBOutlet NSButton *breakpointRadioButtonDisabled;
@property (assign) IBOutlet NSTextField *breakAtLabel;

@property (assign) IBOutlet NSTextView *ramTextView;
@property (assign) IBOutlet NSTextView *romTextView;

@end

@implementation CPUWindow

-(void) refresh {
    CpuState S = [self.gigatron cpuState];
    uint8_t *RAM = [self.gigatron getRAM];
    ROM_OPCODE *ROM = [self.gigatron getROM];
    
    [self updateLabel:self.pcLabel newString:[NSString stringWithFormat:@"0x%04x", S.PC]];
    [self updateLabel:self.irLabel newString:[NSString stringWithFormat:@"0x%02x", S.IR]];
    [self updateLabel:self.dLabel newString:[NSString stringWithFormat:@"0x%02x", S.D]];
    [self updateLabel:self.acLabel newString:[NSString stringWithFormat:@"0x%02x", S.AC]];
    [self updateLabel:self.xLabel newString:[NSString stringWithFormat:@"0x%02x", S.X]];
    [self updateLabel:self.yLabel newString:[NSString stringWithFormat:@"0x%02x", S.Y]];
    [self updateLabel:self.outLabel newString:[NSString stringWithFormat:@"0x%02x", S.OUT]];
    [self updateLabel:self.outxLabel newString:[NSString stringWithFormat:@"0x%02x", S.OUTX]];
    [self updateLabel:self.undefLabel newString:[NSString stringWithFormat:@"0x%02x", S.undef]];
    [self updateLabel:self.inLabel newString:[NSString stringWithFormat:@"0x%02x", self.gigatron.InRegister]];

    int vPC = RAM[0x16] | (RAM[0x17] << 8);
    [self updateLabel:self.vPCLabel newString:[NSString stringWithFormat:@"0x%04x", vPC]];
    [self updateLabel:self.vACLabel newString:[NSString stringWithFormat:@"0x%04x", RAM[0x18] | (RAM[0x19] << 8)]];
    [self updateLabel:self.vLRLabel newString:[NSString stringWithFormat:@"0x%04x", RAM[0x1A] | (RAM[0x1B] << 8)]];
    [self updateLabel:self.vSPLabel newString:[NSString stringWithFormat:@"0x%02x", RAM[0x1C]]];
    [self updateLabel:self.sysFnLabel newString:[NSString stringWithFormat:@"0x%04x", RAM[0x22] | (RAM[0x23] << 8)]];
    
    NSMutableString *ramString = [[NSMutableString alloc] init];
    int ramPage = vPC & 0xFF00;
    int address = ramPage;
    long currentStart = 0;
    while(address <= ramPage+255) {
        if(address == vPC) {
            [ramString appendString:@"-> "];
            currentStart = [ramString length];
        } else {
            [ramString appendString:@"   "];
        }
        [ramString appendFormat:@"%04x ", address];
        switch(RAM[address]) {
            case 0x5e: [ramString appendFormat:@"ST   0x%02x", RAM[address+1]]; address+=2; break;
            case 0x2b: [ramString appendFormat:@"STW  0x%02x", RAM[address+1]]; address+=2; break;
            case 0xec: [ramString appendFormat:@"STLW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0x1a: [ramString appendFormat:@"LD   0x%02x", RAM[address+1]]; address+=2; break;
            case 0x59: [ramString appendFormat:@"LDI  0x%02x", RAM[address+1]]; address+=2; break;
            case 0x11: [ramString appendFormat:@"LDWI 0x%02x%02x", RAM[address+2], RAM[address+1]]; address+=3; break;
            case 0x21: [ramString appendFormat:@"LDW  0x%02x", RAM[address+1]]; address+=2; break;
            case 0xee: [ramString appendFormat:@"LDLW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xe3: [ramString appendFormat:@"ADDI 0x%02x", RAM[address+1]]; address+=2; break;
            case 0x99: [ramString appendFormat:@"ADDW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xe6: [ramString appendFormat:@"SUBI 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xb8: [ramString appendFormat:@"SUBW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0x83: [ramString appendFormat:@"ANDI 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xf8: [ramString appendFormat:@"ANDW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0x88: [ramString appendFormat:@"ORI  0x%02x", RAM[address+1]]; address+=2; break;
            case 0xfa: [ramString appendFormat:@"ORW  0x%02x", RAM[address+1]]; address+=2; break;
            case 0x8c: [ramString appendFormat:@"XORI 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xfc: [ramString appendFormat:@"XORW 0x%02x", RAM[address+1]]; address+=2; break;
            case 0x93: [ramString appendFormat:@"INC  0x%02x", RAM[address+1]]; address+=2; break;
            case 0xad: [ramString appendFormat:@"PEEK"]; address+=1; break;
            case 0xf6: [ramString appendFormat:@"DEEK"]; address+=1; break;
            case 0xf0: [ramString appendFormat:@"POKE 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xf3: [ramString appendFormat:@"DOKE 0x%02x", RAM[address+1]]; address+=2; break;
            case 0xe9: [ramString appendFormat:@"LSLW"]; address+=1; break;
            case 0x35:
                switch(RAM[address+1]) {
                    case 0x3f: [ramString appendFormat:@"BEQ  0x%02x", RAM[address+2]]; address+=3; break;
                    case 0x4d: [ramString appendFormat:@"BGT  0x%02x", RAM[address+2]]; address+=3; break;
                    case 0x50: [ramString appendFormat:@"BLT  0x%02x", RAM[address+2]]; address+=3; break;
                    case 0x53: [ramString appendFormat:@"BGE  0x%02x", RAM[address+2]]; address+=3; break;
                    case 0x56: [ramString appendFormat:@"BLE  0x%02x", RAM[address+2]]; address+=3; break;
                    case 0x72: [ramString appendFormat:@"BNE  0x%02x", RAM[address+2]]; address+=3; break;
                    default: [ramString appendFormat:@"[35] %02x %02x", RAM[address+1], RAM[address+2]]; address+=3; break;
                } break;
            case 0x90: [ramString appendFormat:@"BRA  0x%02x", RAM[address+1]]; address+=2; break;
            case 0x7f: [ramString appendFormat:@"LUP  0x%02x", RAM[address+1]]; address+=2; break;
            case 0xcf: [ramString appendFormat:@"CALL 0x%02x (%02x%02x)", RAM[address+1], RAM[RAM[address+1]+1], RAM[RAM[address+1]]]; address+=2; break;
            case 0xff: [ramString appendFormat:@"RET"]; address+=1; break;
            case 0x75: [ramString appendFormat:@"PUSH"]; address+=1; break;
            case 0x63: [ramString appendFormat:@"POP"]; address+=1; break;
            case 0xdf: [ramString appendFormat:@"ALLOC %02x", RAM[address+1]]; address+=2; break;
            case 0xcd: [ramString appendFormat:@"DEF  %02x", RAM[address+1]]; address+=2; break;
            case 0xb4: [ramString appendFormat:@"SYS  %02x", RAM[address+1]]; address+=2; break;
            default: [ramString appendFormat:@"[%02x]", RAM[address]]; address+=1; break;
        }
        [ramString appendString:@"\n"];
    }
    [self.ramTextView setString:ramString];
    [self.ramTextView scrollRangeToVisible:NSMakeRange(currentStart, 1)];
    
    NSMutableString *romString = [[NSMutableString alloc] init];
    int romPage = S.PC & 0xFF00;
    address = romPage;
    while(address <= romPage+255) {
        if(address == S.PC) {
            [romString appendString:@"-> "];
            currentStart = [romString length];
        } else {
            [romString appendString:@"   "];
        }
        [romString appendFormat:@"%04x ", address];
        switch(ROM[address].instruction) {
            case 0x00: [romString appendFormat:@".LD   0x%02x", ROM[address].operand]; address+=2; break;
            case 0x02: [romString appendFormat:@".NOP  0x%02x", ROM[address].operand]; address+=2; break;
            case 0x20: [romString appendFormat:@".ANDA 0x%02x", ROM[address].operand]; address+=2; break;
            case 0x40: [romString appendFormat:@".ORA  0x%02x", ROM[address].operand]; address+=2; break;
            case 0x60: [romString appendFormat:@".XORA 0x%02x", ROM[address].operand]; address+=2; break;
            case 0x80: [romString appendFormat:@".ADDA 0x%02x", ROM[address].operand]; address+=2; break;
            case 0xA0: [romString appendFormat:@".SUBA 0x%02x", ROM[address].operand]; address+=2; break;
            case 0xC0: [romString appendFormat:@".ST   0x%02x", ROM[address].operand]; address+=2; break;
            case 0xE0: [romString appendFormat:@".JMP  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xE4: [romString appendFormat:@".BGT  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xE8: [romString appendFormat:@".BLT  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xEC: [romString appendFormat:@".BNE  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xF0: [romString appendFormat:@".BEQ  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xF4: [romString appendFormat:@".BGE  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xF8: [romString appendFormat:@".BLE  0x%02x", ROM[address].operand]; address+=2; break;
            case 0xFC: [romString appendFormat:@".BRA  0x%02x", ROM[address].operand]; address+=2; break;
            default: [romString appendFormat:@".[%02x] 0x%02x", ROM[address].instruction, ROM[address].operand]; address+=2; break;
        }
        [romString appendString:@"\n"];
    }
    
    [self.romTextView setString:romString];
    [self.romTextView scrollRangeToVisible:NSMakeRange(currentStart, 1)];
}

- (void) updateLabel:(NSTextField *)label newString:(NSString *)newString {
    NSString *oldValue = [label stringValue];
    [label setStringValue:newString];
    if(![oldValue isEqualToString:newString]) {
        [label setBackgroundColor:[NSColor colorWithCGColor:CGColorCreateGenericRGB(0.8, 0.8, 0.0, 0.75)]];
        [label setDrawsBackground:YES];
    } else {
        [label setBackgroundColor:NSColor.clearColor];
        [label setDrawsBackground:NO];
    }
}

- (IBAction) textFieldValueChanged:(NSTextField *)sender {
    [self.breakpointRadioButtonDisabled performClick:self.breakpointRadioButtonDisabled];
}

- (IBAction) breakAtRadioButtonClicked:(NSButton *)sender {
    NSTextField *textField = nil;
    if(sender == self.breakpointRadioButton1) {
        textField = self.breakAtTextField1;
    } else if (sender == self.breakpointRadioButton2) {
        textField = self.breakAtTextField2;
    } else if (sender == self.breakpointRadioButton3) {
        textField = self.breakAtTextField3;
    }

    AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
    unsigned int address = -1;
    if(textField != nil) {
        if([[textField stringValue] length] == 0) {
            [textField setStringValue:@"0"];
        }
        
        NSScanner *scanner = [NSScanner scannerWithString:[textField stringValue]];
        
        [scanner setScanLocation:0]; // bypass '#' character
        [scanner scanHexInt:&address];
        
        [textField setStringValue:[NSString stringWithFormat:@"%04X", address]];
        [self.breakAtLabel setStringValue:[NSString stringWithFormat:@"Break at 0x%04X", address]];
    } else {
        [self.breakAtLabel setStringValue:@"Break disabled"];
    }
    
    delegate.gigatronImageView.gigatron.breakAddress = address;
    NSLog(@"Set new breakpoint at 0x%04X", address);
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

@end
