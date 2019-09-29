//
//  GigatronSimulatorApplication.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 05.03.19.
//  Copyright © 2019 640k.de. All rights reserved.
//

#import "GigatronSimulatorApplication.h"

@implementation GigatronSimulatorApplication

- (void) sendEvent:(NSEvent *)event {
    // This works around an AppKit bug, where key up events while holding
    // down the command key don't get sent to the key window.
    if ([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask)) {
        [[self keyWindow] sendEvent:event];
    } else {
        [super sendEvent:event];
    }
}
@end
