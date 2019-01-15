//
//  AQPlayer.h
//  Gigatron Mac
//
//  Created by Tobias Braun on 04.12.18.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface AQPlayer : NSObject

- (AQPlayer*) init;
- (void) stopPlaying;
- (void) startPlaying;
- (BOOL) isPlaying;
- (void) addSampleLeftRightTuple:(short[2])samples;
- (int) getAudiobufFree;
- (void) setVolume:(int)volume;

@end

NS_ASSUME_NONNULL_END
