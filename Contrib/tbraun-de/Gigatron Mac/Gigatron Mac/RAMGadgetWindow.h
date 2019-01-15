//
//  RAMGadget.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 15.01.19.
//

#import <Cocoa/Cocoa.h>
#import "RAMGadgetImageView.h"

NS_ASSUME_NONNULL_BEGIN

@interface RAMGadgetWindow : NSWindowController

@property (weak) Gigatron* gigatron;

- (void) refresh;

@end

NS_ASSUME_NONNULL_END
