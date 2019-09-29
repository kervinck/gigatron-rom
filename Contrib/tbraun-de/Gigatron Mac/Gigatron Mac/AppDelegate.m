//
//  AppDelegate.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 23.11.18.
//

#import "AppDelegate.h"

@interface AppDelegate ()


@property (assign) IBOutlet NSImageView *led1View;
@property (assign) IBOutlet NSImageView *led2View;
@property (assign) IBOutlet NSImageView *led3View;
@property (assign) IBOutlet NSImageView *led4View;

@property (assign) IBOutlet NSPopUpButton *refreshPopupButton;

@property (assign) IBOutlet NSTextField *totalCycles;
@property (assign) IBOutlet NSTextField *deltaCycles;

@property (assign) long lastCycles;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    self.ramWindows = [[NSMutableArray alloc] init];
    self.removedRamWindows = [[NSMutableArray alloc] init];
    [NSTimer scheduledTimerWithTimeInterval:1.0f / 59.96f
             target:self.gigatronImageView selector:@selector(calculateFrame) userInfo:nil repeats:YES];
    [self.gigatronImageView.gigatron setVolume:75];
    self.wasRunning = YES;
    
    self.cpuWindow = [[CPUWindow alloc] initWithWindowNibName:@"CPUWindow"];
    self.cpuWindow.gigatron = self.gigatronImageView.gigatron;
    [self.cpuWindow showWindow:self];
    
    self.ramGadgetWindow = [[RAMGadgetWindow alloc] initWithWindowNibName:@"RAMGadgetWindow"];
    self.ramGadgetWindow.gigatron = self.gigatronImageView.gigatron;
    [self.ramGadgetWindow showWindow:self];

    [self.refreshPopupButton selectItem:[self.refreshPopupButton itemAtIndex:1]];
    self.refreshInterval = 0.5;
    self.lastRefresh = [NSDate date];
    self.lastCycles = [self.gigatronImageView.gigatron cpuCyclesSinceReset];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void) applicationDidResignActive:(NSNotification *)notification {
    if(self.wasRunning) {
        [self.gigatronImageView.gigatron pause];
    }
}

- (void) applicationDidBecomeActive:(NSNotification *)notification {
    if(self.wasRunning) {
        [self.gigatronImageView.gigatron resume];
        [self updateGigatronRefreshFromNSPopupButton];
    }
    [[self.gigatronImageView window] makeFirstResponder:self.gigatronImageView];
}

- (void) updateLeds:(uint8_t) ledState {
    NSImage *off = [NSImage imageNamed:NSImageNameStatusNone];
    NSImage *on = [NSImage imageNamed:NSImageNameStatusUnavailable];
    if((ledState & 0x01) > 0) {
        [self.led1View setImage:on];
    } else {
        [self.led1View setImage:off];
    }
    if((ledState & 0x02) > 0) {
        [self.led2View setImage:on];
    } else {
        [self.led2View setImage:off];
    }
    if((ledState & 0x04) > 0) {
        [self.led3View setImage:on];
    } else {
        [self.led3View setImage:off];
    }
    if((ledState & 0x08) > 0) {
        [self.led4View setImage:on];
    } else {
        [self.led4View setImage:off];
    }
}

- (IBAction)volumeValueChanged:(NSSlider *)sender {
    NSLog(@"slider value = %f", sender.floatValue);
    [self.gigatronImageView.gigatron setVolume:sender.intValue];
}

- (IBAction)loadButtonPressed:(NSButton *)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setAllowedFileTypes:[NSArray arrayWithObject:@"gt1"]];
    
    NSButton *button = [[NSButton alloc] init];
    [button setButtonType:NSButtonTypeSwitch];
    button.title = NSLocalizedString(@"Watch file", @"");
    [button sizeToFit];
    [panel setAccessoryView:button];
    [panel setAccessoryViewDisclosed:true];
//    panel.delegate = self;
    
    [panel beginWithCompletionHandler:^(NSInteger result){
        panel.delegate = nil; // TODO: Check if this is necessary
        if (result == NSModalResponseOK) {
            BOOL checkboxOn = (((NSButton*)panel.accessoryView).state == NSControlStateValueOn);
            for (NSURL *fileURL in [panel URLs]) {
                [self.gigatronImageView.gigatron loadGt1File:fileURL watchFile:checkboxOn];
            }
        }
    }];
}

- (IBAction)resetButtonBressed:(NSButton *)sender {
    [self.gigatronImageView.gigatron reset];
}

- (IBAction)writeStateButtonPressed:(NSButton *)sender {
    [self.gigatronImageView.gigatron persistState];
}

- (IBAction)restoreStateButtonBressed:(NSButton *)sender {
    [self.gigatronImageView.gigatron restoreState];
}

- (IBAction)ramSizeChanged:(NSPopUpButton *)sender {
    switch([sender indexOfSelectedItem]) {
        case 0:
            [self.gigatronImageView.gigatron setRamSizeTo64kB:false];
            break;
        default:
            [self.gigatronImageView.gigatron setRamSizeTo64kB:true];
            break;
    }
}

- (IBAction) refreshRateChanged:(NSPopUpButton *)sender {
    [self updateGigatronRefreshFromNSPopupButton];
}

- (IBAction) ramWatchButtonPressed:(NSButton *)sender {
    RAMWindow *window = [ [ RAMWindow alloc ] initWithGigatron:[self.gigatronImageView gigatron]];

    [window makeKeyAndOrderFront:nil];
    [window refresh];
    
    [self.ramWindows addObject:window];
}

- (void) removeRamWindow:(RAMWindow *)ramWindow {
    [self.removedRamWindows addObject:ramWindow];
}


- (IBAction) breakButtonPressed:(NSButton *)sender {
    self.refreshInterval = 0;
    self.wasRunning = NO;
    [self.gigatronImageView.gigatron pause];
}

- (IBAction) continueButtonPressed:(NSButton *)sender {
    [self updateGigatronRefreshFromNSPopupButton];
    self.wasRunning = NO;
    [self.gigatronImageView.gigatron resume];
}

- (void) updateGigatronRefreshFromNSPopupButton {
    switch([self.refreshPopupButton indexOfSelectedItem]) {
        case 0:
            self.refreshInterval = 0.1;
            break;
        case 1:
            self.refreshInterval = 0.5;
            break;
        case 2:
            self.refreshInterval = 2;
            break;
        default:
            break;
    }
}

- (IBAction) stepCPUButtonPressed:(NSButton *)sender {
    self.refreshInterval = 0;
    self.wasRunning = NO;
    [self.gigatronImageView.gigatron singleStepCPU];
}

- (IBAction) stepvCPUButtonPressed:(NSButton *)sender {
    self.refreshInterval = 0;
    self.wasRunning = NO;
    [self.gigatronImageView.gigatron singleStepvCPU];
}

- (void) refreshWindows {
    if([self.lastRefresh timeIntervalSinceNow] < 0 - self.refreshInterval) {
        for(RAMWindow *ramWindow in self.ramWindows) {
            if(![self.removedRamWindows containsObject:ramWindow]) {
                [ramWindow refresh];
            }
        }
        [self.cpuWindow refresh];
        [self.ramGadgetWindow refresh];

        long currentCycles = [self.gigatronImageView.gigatron cpuCyclesSinceReset];
        [self.totalCycles setStringValue:[NSString stringWithFormat:@"Cycle #%ld", currentCycles]];
        [self.deltaCycles setStringValue:[NSString stringWithFormat:@"∆ cycles: %ld", currentCycles - self.lastCycles]];
        self.lastCycles = currentCycles;

        self.lastRefresh = [NSDate date];
    }
}

@end
