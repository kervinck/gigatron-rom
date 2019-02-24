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

#define BUTTON_A            0x80
#define BUTTON_B            0x40
#define BUTTON_SELECT       0x20
#define BUTTON_START        0x10
#define BUTTON_UP           0x08
#define BUTTON_DOWN         0x04
#define BUTTON_LEFT         0x02
#define BUTTON_RIGHT        0x01

#define KEY_ENTER           0x0a
#define KEY_BACKSPACE       0x7f
#define KEY_TAB             0x09
#define KEY_ESCAPE          0x1b

@implementation GigatronImageView

BOOL controlKeyPressed;
CGContextRef bitmapContext;
Gigatron *gigatron;

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
        [self handleKey:event];
    }
}

- (void) keyDown:(NSEvent *)event {
    if([self handleKey:event] != true) {
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
    }
}

- (void) keyUp:(NSEvent *)event {
    if([self handleKey:event] != true) {
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
    }
}

- (BOOL) handleKey:(NSEvent *)event {
    short keyCode = [event keyCode];
    NSEventType type = [event type];

    uint8_t mappedKey = 0x00;
    BOOL gamepadButton = true;
    
    if(type == NSEventTypeKeyDown) {
        NSLog(@"Pressed key %d", keyCode);
    } else if(type == NSEventTypeKeyUp) {
        NSLog(@"Released key %d", keyCode);
    }
    
    switch(keyCode) {
        case 96: // F5
            if(type == NSEventTypeKeyDown) {
                [gigatron singleStepCPU];
            }
            break;
        case 97: // F6
            if(type == NSEventTypeKeyDown) {
                [gigatron singleStepvCPU];
            }
            break;
        case 98: // F7
            if(type == NSEventTypeKeyDown) {
                [gigatron resume];
            }
            break;
        case 126:  // Cursor UP
            mappedKey = BUTTON_UP; break;
        case 125:  // Cursor DOWN
            mappedKey = BUTTON_DOWN; break;
        case 123:  // Cursor LEFT
            mappedKey = BUTTON_LEFT; break;
        case 124:  // Cursor RIGHT
            mappedKey = BUTTON_RIGHT; break;
        case 58:   // Left ALT
            mappedKey = BUTTON_SELECT; break;
        case 55:   // Left CMD
            mappedKey = BUTTON_START; break;
        case 61:   // Right ALT
            mappedKey = BUTTON_B; break;
        case 54:   // Right CMD
            mappedKey = BUTTON_A; break;
        case 36:   // ENTER
            mappedKey = KEY_ENTER;
            gamepadButton = false;
            break;
        case 48:   // TAB
            mappedKey = KEY_TAB;
            gamepadButton = false;
            break;
        case 53:   // ESC
            mappedKey = KEY_ESCAPE;
            gamepadButton = false;
            break;
        case 51:   // Backspace
            mappedKey = KEY_BACKSPACE;
            gamepadButton = false;
            break;
        case 59:   // ctrl
            controlKeyPressed = !controlKeyPressed; return true;
        default:
            if(type != NSEventTypeFlagsChanged && [[event charactersIgnoringModifiers] length] > 0) {
                const char *characters = [[event charactersIgnoringModifiers] cStringUsingEncoding:NSUTF8StringEncoding];
                if(type == NSEventTypeKeyDown) {
                    if(controlKeyPressed) {
                        mappedKey = characters[0] & 0x1F;
                    } else {
                        mappedKey = characters[0];
                    }
                }
            }
            gamepadButton = false;
            break;
    }
    
    if(gamepadButton) {
        if(type == NSEventTypeKeyDown) {
            gigatron.InRegister &= ~mappedKey;
        } else if(type == NSEventTypeKeyUp) {
            gigatron.InRegister |= mappedKey;
        } else if(type == NSEventTypeFlagsChanged) {
            gigatron.InRegister ^= mappedKey;
        }
        return true;
    }
    
    if(type == NSEventTypeKeyDown) {
        gigatron.InRegister = mappedKey;
        return true;
    } else {
        gigatron.InRegister = 0xff;
        return true;
    }

    return false;
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
