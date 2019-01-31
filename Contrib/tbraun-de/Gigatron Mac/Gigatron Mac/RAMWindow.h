//
//  RAMWindow.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 08.01.19.
//

#import <Cocoa/Cocoa.h>
#import "Gigatron.h"

#ifndef RAMWindow_h
#define RAMWindow_h

@interface RAMWindow : NSWindow <NSWindowDelegate>

@property (retain) Gigatron* gigatron;
@property NSTextField *pageLabel;
@property NSTextField *pageNumber;
@property NSButton *refreshButton;

@property NSMutableArray *hexLabels;
@property NSMutableArray *rowLabels;

@property unsigned short page;

- (id) initWithGigatron:(Gigatron *)gigatron;
- (void) refresh;

@end

#endif /* RAMWindow_h */
