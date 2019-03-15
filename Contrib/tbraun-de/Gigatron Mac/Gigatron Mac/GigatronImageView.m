//
//  GigatronImageView.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 23.11.18.
//

#import "GigatronImageView.h"
#import "AQPlayer.h"
#import "AppDelegate.h"
#import "Gigatron.h"

#define MAC_KEY_LEFT_ALT        @"left-alt"
#define MAC_KEY_LEFT_CMD        @"left-cmd"
#define MAC_KEY_RIGHT_ALT       @"right-alt"
#define MAC_KEY_RIGHT_CMD       @"right-cmd"
#define MAC_KEY_CONTROL         @"control"
#define MAC_CURSOR_UP           @"cursor-up"
#define MAC_CURSOR_DOWN         @"cursor-down"
#define MAC_CURSOR_LEFT         @"cursor-left"
#define MAC_CURSOR_RIGHT        @"cursor-right"
#define MAC_SHIFT               @"shift"
#define MAC_KEY_TAB             @"tab"
#define MAC_KEY_ESC             @"esc"
#define MAC_KEY_BACKSPACE       @"backspace"
#define MAC_KEY_ENTER           @"enter"

#define GIGATRON_BUTTON_A       0x80
#define GIGATRON_BUTTON_B       0x40
#define GIGATRON_BUTTON_SELECT  0x20
#define GIGATRON_BUTTON_START   0x10
#define GIGATRON_BUTTON_UP      0x08
#define GIGATRON_BUTTON_DOWN    0x04
#define GIGATRON_BUTTON_LEFT    0x02
#define GIGATRON_BUTTON_RIGHT   0x01

#define GIGATRON_KEY_ENTER      0x0a
#define GIGATRON_KEY_BACKSPACE  0x7f
#define GIGATRON_KEY_TAB        0x09
#define GIGATRON_KEY_ESCAPE     0x1b

@implementation GigatronImageView

BOOL controlKeyPressed;
CGContextRef bitmapContext;
Gigatron *gigatron;
NSMutableSet *pressedKeys;

- (id) initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    gigatron = [[Gigatron alloc] initWithROMFile:[[NSBundle mainBundle] pathForResource:@"ROMv3" ofType:@"rom"]];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    bitmapContext = CGBitmapContextCreate(gigatron.framebuffer,
                                          gigatron.screenWidth,
                                          gigatron.screenHeight,
                                          8, // bitsPerComponent
                                          4*gigatron.screenWidth, // bytesPerRow
                                          colorSpace,
                                          kCGImageAlphaNoneSkipLast);
    
    CFRelease(colorSpace);
    pressedKeys = [[NSMutableSet alloc] init];
    
    [[self window] makeFirstResponder:self];
    
    return self;
}

- (void) calculateFrame {
    [gigatron calculateFrame];
    [self updateLeds];
    // [self updateRamTextView];
    [self setNeedsDisplay:true];
}

- (BOOL) acceptsFirstResponder {
    return true;
}

- (void) flagsChanged: (NSEvent *)event {
    if([event type] == NSEventTypeFlagsChanged) {
        NSLog(@"modifier flags: %lu", [event modifierFlags]);
        [pressedKeys removeObject:MAC_KEY_LEFT_ALT];
        [pressedKeys removeObject:MAC_KEY_LEFT_CMD];
        [pressedKeys removeObject:MAC_KEY_RIGHT_ALT];
        [pressedKeys removeObject:MAC_KEY_RIGHT_CMD];
        [pressedKeys removeObject:MAC_KEY_CONTROL];
        if([event modifierFlags] & 0x20) [pressedKeys addObject:MAC_KEY_LEFT_ALT];
        if([event modifierFlags] & 0x40) [pressedKeys addObject:MAC_KEY_RIGHT_ALT];
        if([event modifierFlags] & 0x08) [pressedKeys addObject:MAC_KEY_LEFT_CMD];
        if([event modifierFlags] & 0x10) [pressedKeys addObject:MAC_KEY_RIGHT_CMD];
        if([event modifierFlags] & NSEventModifierFlagControl) [pressedKeys addObject:MAC_KEY_CONTROL];
        [self handlePressedKeys];
    }
}

- (void) logPressedKeys {
    NSMutableString *s = [[NSMutableString alloc] initWithString:@"Pressed keys: "];
    for(NSString *key in pressedKeys) {
        [s appendFormat:@"[%@] ", key];
    }
    NSLog(@"%@", s);
}

- (void) keyDown:(NSEvent *)event {
    // Debugger specific keys are only handled in keyDown
    short keyCode = [event keyCode];
    
    switch(keyCode) {
        case 96: // F5
            [gigatron singleStepCPU];
        case 97: // F6
            [gigatron singleStepvCPU];
        case 98: // F7
            [gigatron resume];
            return; // for those three cases: return directly
    }
    
    NSString *pressedKey = [self getStringForKeyEvent:event];
    NSLog(@"Key DOWN: %@", pressedKey);
    
    // if a non-special character is pressed, which is indicated by a string length of "1", then
    // release all non-special keys (numbers, letters, signs), but not shift, alt, cmd or any of the cursor keys
    if([pressedKey length] == 1) {
        for(NSString *key in pressedKeys) {
            if([key length] == 1) {
                [pressedKeys removeObject:key];
            }
        }
    }
    
    [pressedKeys addObject:pressedKey];
    [self handlePressedKeys];

    // [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void) keyUp:(NSEvent *)event {
    NSString *releasedKey = [self getStringForKeyEvent:event];
    NSLog(@"Key UP: %@", releasedKey);
    [pressedKeys removeObject:releasedKey];
    
    // if a non-special character is released, which is indicated by a string length of "1", then
    // release all non-special keys (numbers, letters, signs), but not shift, alt, cmd or any of the cursor keys
    if([releasedKey length] == 1) {
        for(NSString *key in pressedKeys) {
            if([key length] == 1) {
                [pressedKeys removeObject:key];
            }
        }
    }
    
    [self handlePressedKeys];

    // [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (NSString *) getStringForKeyEvent:(NSEvent *)event {
    short keyCode = [event keyCode];
    
    switch(keyCode) {
        case 126:  // Cursor UP
            return MAC_CURSOR_UP;
        case 125:  // Cursor DOWN
            return MAC_CURSOR_DOWN;
        case 123:  // Cursor LEFT
            return MAC_CURSOR_LEFT;
        case 124:  // Cursor RIGHT
            return MAC_CURSOR_RIGHT;
        case 36:   // Enter key
            return MAC_KEY_ENTER;
        case 48:   // TAB
            return MAC_KEY_TAB;
        case 53:   // ESC
            return MAC_KEY_ESC;
        case 51:   // Backspace
            return MAC_KEY_BACKSPACE;
        default:
            if([[event charactersIgnoringModifiers] length] == 1)
                return [event charactersIgnoringModifiers];
    }
    return nil;
}


- (void) handlePressedKeys {
    [self logPressedKeys];
    
    if([pressedKeys count] == 0) {
        gigatron.InRegister = 0xff;
        return;
    }
    
    gigatron.InRegister = 0;
    
    // first, find out if any of the gigatron buttons equivalents is be pressed (START / SELECT / A / B  or a direction key):
    // compose the IN register out of those keys, if there are any
    if([pressedKeys containsObject:MAC_KEY_RIGHT_CMD]) gigatron.InRegister |= GIGATRON_BUTTON_A;
    if([pressedKeys containsObject:MAC_KEY_RIGHT_ALT]) gigatron.InRegister |= GIGATRON_BUTTON_B;
    if([pressedKeys containsObject:MAC_KEY_LEFT_ALT]) gigatron.InRegister |= GIGATRON_BUTTON_SELECT;
    if([pressedKeys containsObject:MAC_KEY_LEFT_CMD]) gigatron.InRegister |= GIGATRON_BUTTON_START;
    if([pressedKeys containsObject:MAC_CURSOR_UP]) gigatron.InRegister |= GIGATRON_BUTTON_UP;
    if([pressedKeys containsObject:MAC_CURSOR_DOWN]) gigatron.InRegister |= GIGATRON_BUTTON_DOWN;
    if([pressedKeys containsObject:MAC_CURSOR_LEFT]) gigatron.InRegister |= GIGATRON_BUTTON_LEFT;
    if([pressedKeys containsObject:MAC_CURSOR_RIGHT]) gigatron.InRegister |= GIGATRON_BUTTON_RIGHT;
    
    if(gigatron.InRegister != 0) {
        gigatron.InRegister = ~gigatron.InRegister;
        return;
    }
    
    // if not continue with special keys (tab, esc, backspace and enter)
    if([pressedKeys containsObject:MAC_KEY_TAB]) gigatron.InRegister = GIGATRON_KEY_TAB;
    if([pressedKeys containsObject:MAC_KEY_ESC]) gigatron.InRegister = GIGATRON_KEY_ESCAPE;
    if([pressedKeys containsObject:MAC_KEY_BACKSPACE]) gigatron.InRegister = GIGATRON_KEY_BACKSPACE;
    if([pressedKeys containsObject:MAC_KEY_ENTER]) gigatron.InRegister = GIGATRON_KEY_ENTER;

    if(gigatron.InRegister != 0) {
        return;
    }

    // if no special gigatron keys are pressed, then continue with any "normal" keyboard keys
    gigatron.InRegister = [((NSString *)[pressedKeys anyObject]) characterAtIndex:0];
    
    // if [control] is pressed, handle this also for gigatron
    if([pressedKeys count] >= 2 && [pressedKeys containsObject:MAC_KEY_CONTROL]) {
        gigatron.InRegister &= 0x1F;
    }
}

- (void) dealloc {
    CGContextRelease(bitmapContext);
}

- (Gigatron *) gigatron {
    return gigatron;
}

- (void) updateLeds {
    AppDelegate *delegate = [[NSApplication sharedApplication] delegate];
    [delegate updateLeds:[gigatron cpuState].OUTX];
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    if(bitmapContext != nil) {
        CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
        NSImage *newUIImage = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
        
        [newUIImage drawInRect:dirtyRect fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0];
        
        CGImageRelease(cgImage);
    }
}

@end
