//
//  RAMWindow.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 08.01.19.
//

#import <Foundation/Foundation.h>
#import "RAMWindow.h"
#import "Gigatron.h"
#import "AppDelegate.h"

@implementation RAMWindow

- (id) initWithGigatron:(Gigatron *)myGigatron {
    NSRect windowRect = NSMakeRect( 200.0, 200.0, 580.0, 213.0 );
    self = [super initWithContentRect:windowRect styleMask:( NSWindowStyleMaskClosable | NSWindowStyleMaskTitled ) backing:NSBackingStoreBuffered defer:NO];
    [self setReleasedWhenClosed:true];
    self.delegate = self;
    
    self.gigatron = myGigatron;

    NSFont *tinyFixedWidthFont = [NSFont fontWithName:@"Monaco" size:8];
    NSFont *regularFixedWidthFont = [NSFont fontWithName:@"Monaco" size:10];
    NSFont *regularFont = [NSFont fontWithName:@"Helvetica" size:12];
    NSFont *smallFont = [NSFont fontWithName:@"Helvetica" size:9];
    [self addTextFieldWithFrame:NSMakeRect(20, 174, 46, 17) string:@"Page #" font:regularFont];
    
    self.pageNumber = [[NSTextField alloc] initWithFrame:NSMakeRect(72, 174, 96, 21)];
    [self.pageNumber setPlaceholderString:@"0x0100"];
    [[self contentView] addSubview: self.pageNumber];
    
    self.refreshButton = [NSButton buttonWithTitle:@"Refresh" target:self action:@selector(refreshButtonPressed:)];
    [self.refreshButton setFrame:NSMakeRect(170, 167, 88, 32)];
    [[self contentView] addSubview: self.refreshButton];

    self.hexLabels = [[NSMutableArray alloc] init];
    self.rowLabels = [[NSMutableArray alloc] init];

    int x=62;
    
    for(int i=0; i<=0xFF; i++) {
        [self.hexLabels addObject:@""]; // pre-fill array with dummy string objects
    }
    
    for (int i=0; i<=0x1F; i++) {
        if(i>0 && i%4 == 0) {
            x+= 7;
        }
        [self addTextFieldWithFrame:NSMakeRect(x+1, 149, 15, 11) string:[NSString stringWithFormat:@"%02X", i] font:tinyFixedWidthFont];
        
        int y=132;
        for (int j=0; j<=0xE0; j+=0x20) {
            if(j==0x80) {
                y -= 7;
            }
            [self.hexLabels replaceObjectAtIndex:i+j withObject:[self addTextFieldWithFrame:NSMakeRect(x, y, 17, 14) string:[NSString stringWithFormat:@"%02X", i+j] font:regularFixedWidthFont]];
            y -= 15;
        }
        x += 14;
    }
    
    int y=132;
    for(int i=0; i<8; i++) {
        if(i==4) {
            y -= 7;
        }
        
        [self.rowLabels addObject:[self addTextFieldWithFrame:NSMakeRect(22, y, 40, 13) string:[NSString stringWithFormat:@"0x%04X", i<<5] font:smallFont]];
        
        y -= 15;
    }
    
    return self;
}

- (NSTextField *) addTextFieldWithFrame:(NSRect)rect string:(NSString *)string font:(NSFont *)font {
    NSTextField *textField = [[NSTextField alloc] initWithFrame:rect];
    [textField setStringValue:string];
    [textField setEditable:NO];
    [textField setSelectable:NO];
    [textField setTextColor:NSColor.blackColor];
    [textField setBackgroundColor:NSColor.controlColor];
    [textField setDrawsBackground:NO];
    [textField setBezeled:NO];
    [textField setAlignment:NSTextAlignmentNatural];
    [textField setFont:font];
    [textField setLineBreakMode:NSLineBreakByClipping];
    [[self contentView] addSubview: textField];
    
    return textField;
}

- (void) refreshButtonPressed:(NSButton *)sender {
    [self refresh];
    unsigned myPage = 0x0000;
    NSScanner *scanner = [NSScanner scannerWithString:[self.pageNumber stringValue]];
    
    [scanner setScanLocation:0]; // bypass '#' character
    [scanner scanHexInt:&myPage];
    
    if([self.gigatron isRamSize64Kb]) {
        self.page = myPage & 0xFF00;
    } else {
        self.page = myPage & 0x7F00;
    }
    [self.pageNumber setStringValue:[NSString stringWithFormat:@"0x%04X", self.page]];
}

- (void) refresh {
    uint8_t *RAM = [self.gigatron getRAM];
    
    for(int i=0; i<=0xFF; i++) {
        NSTextField *label = [self.hexLabels objectAtIndex:i];
        NSString *oldValue = [label stringValue];
        NSString *newValue =[NSString stringWithFormat:@"%02X", RAM[self.page+i]];
        [label setStringValue:newValue];
        if(![oldValue isEqualToString:newValue]) {
            [label setBackgroundColor:[NSColor colorWithCGColor:CGColorCreateGenericRGB(0.8, 0.8, 0.0, 0.75)]];
            [label setDrawsBackground:YES];
        } else {
            [label setBackgroundColor:NSColor.clearColor];
            [label setDrawsBackground:NO];
        }
    }
    
    for(int i=0; i<8; i++) {
        NSTextField *label = [self.rowLabels objectAtIndex:i];
        [label setStringValue:[NSString stringWithFormat:@"0x%04X", self.page + (i<<5)]];
    }
}

- (void) windowWillClose:(NSNotification *)notification {
    AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
    [delegate removeRamWindow:self];
}

@end
