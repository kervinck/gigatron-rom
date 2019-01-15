//
//  CPUWindow2.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 12.01.19.
//

#import <Cocoa/Cocoa.h>
#import "Gigatron.h"

NS_ASSUME_NONNULL_BEGIN

@interface CPUWindow : NSWindowController

@property (weak) Gigatron* gigatron;

- (void) refresh;

@end

NS_ASSUME_NONNULL_END
