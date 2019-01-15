//
//  RAMGadget.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 15.01.19.
//

#import "RAMGadgetWindow.h"

@interface RAMGadgetWindow ()

@property (assign) IBOutlet RAMGadgetImageView *ramGadget;
@property (assign) IBOutlet NSPopUpButton *palettePopup;

@end

@implementation RAMGadgetWindow

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];

    return self;
}

- (IBAction)paletteChanged:(NSPopUpButton *)sender {
    [self.ramGadget setPalette:[sender indexOfSelectedItem]];
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    self.ramGadget.gigatron = self.gigatron;
}

- (void) refresh {
    [self.ramGadget setNeedsDisplay:YES];
}

@end
