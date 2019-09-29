//
//  RAMGadgetImageView.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 15.01.19.
//

#import <Cocoa/Cocoa.h>
#import "Gigatron.h"

NS_ASSUME_NONNULL_BEGIN

@interface RAMGadgetImageView : NSImageView

@property (weak) Gigatron* gigatron;

-(void)setPalette:(long)palette;

@end

NS_ASSUME_NONNULL_END
