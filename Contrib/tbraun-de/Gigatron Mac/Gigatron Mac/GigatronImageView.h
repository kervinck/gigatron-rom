//
//  GigatronImageView.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 23.11.18.
//

#import <Cocoa/Cocoa.h>
#import "Gigatron.h"

NS_ASSUME_NONNULL_BEGIN

@interface GigatronImageView : NSImageView

// @property (assign) IBOutlet NSTextView *ramTextView;
@property (readonly) Gigatron *gigatron;

- (void) updateLeds;
- (BOOL) handleKey:(NSEvent *)event;
- (void) calculateFrame;

// - (void) updateRamTextView;

@end

NS_ASSUME_NONNULL_END
