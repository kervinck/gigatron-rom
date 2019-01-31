//
//  AppDelegate.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 23.11.18.
//

#import <Cocoa/Cocoa.h>
#import "GigatronImageView.h"
#import "Gigatron.h"
#import "RAMWindow.h"
#import "CPUWindow.h"
#import "RAMGadgetWindow.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property NSMutableArray *ramWindows;
@property NSMutableArray *removedRamWindows;
@property CPUWindow *cpuWindow;
@property RAMGadgetWindow *ramGadgetWindow;
@property NSDate *lastRefresh;
@property float refreshInterval;

- (void) updateLeds:(uint8_t) ledState;
- (void) refreshWindows;
- (void) removeRamWindow:(RAMWindow *)ramWindow;

@end

