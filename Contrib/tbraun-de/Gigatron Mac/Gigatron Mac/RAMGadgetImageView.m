//
//  RAMGadgetImageView.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 15.01.19.
//

#import "RAMGadgetImageView.h"
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

@implementation RAMGadgetImageView

Byte* gadgetFramebuffer;
CGContextRef gadgetBitmapContext;
long palette;

- (id) initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    gadgetFramebuffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    memset(gadgetFramebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    gadgetBitmapContext = CGBitmapContextCreate(gadgetFramebuffer,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          8, // bitsPerComponent
                                          4*SCREEN_WIDTH, // bytesPerRow
                                          colorSpace,
                                          kCGImageAlphaNoneSkipLast);
    
    CFRelease(colorSpace);
    
    return self;
}

-(void)setPalette:(long)myPalette {
    palette = myPalette;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    uint8_t *RAM = [self.gigatron getRAM];
    BOOL ramSize64Kb = [self.gigatron isRamSize64Kb];
    for(int y=0; y<256; y++) {
        for(int x=0; x<256; x++) {
            int pixelAddress = y*SCREEN_WIDTH*4 + x*4;
            uint8_t ramContent = 0;
            if(ramSize64Kb || y<=127) {
                ramContent = RAM[y*SCREEN_WIDTH + x];
            }
            switch(palette) {
                case 0:
                    if(ramContent < 64) {
                        gadgetFramebuffer[pixelAddress] = (ramContent & 0x03) << 6;
                        gadgetFramebuffer[pixelAddress + 1] = (ramContent & 0x0C) << 4;
                        gadgetFramebuffer[pixelAddress + 2] = (ramContent & 0x30) << 2;
                    } else {
                        gadgetFramebuffer[pixelAddress] = ramContent;
                        gadgetFramebuffer[pixelAddress + 1] = ramContent;
                        gadgetFramebuffer[pixelAddress + 2] = ramContent;
                    }
                    break;
                case 1:
                    gadgetFramebuffer[pixelAddress] = ramContent;
                    gadgetFramebuffer[pixelAddress + 1] = ramContent;
                    gadgetFramebuffer[pixelAddress + 2] = ramContent;
                    break;
            }
        }
    }
    
    if(gadgetBitmapContext != nil) {
        CGImageRef cgImage = CGBitmapContextCreateImage(gadgetBitmapContext);
        NSImage *newUIImage = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
        
        [newUIImage drawInRect:dirtyRect fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0];
        
        CGImageRelease(cgImage);
    }
}

- (void) dealloc {
    free(gadgetFramebuffer);
}

@end
