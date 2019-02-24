//
//  AQPlayer.m
//  Gigatron Mac
//
//  Created by Tobias Braun on 04.12.18.
//

/*
 * This file is based on the AudioQueue Player in XMP (extended module player) by Claudio Matsuoka. Original file copyright:
 * ---
 * This file is copyright:
 *
 * Extended Module Player
 * Copyright (C) 1996-2007 Claudio Matsuoka and Hipolito Carraro Jr
 * CoreAudio helpers (C) 2000 Timothy J. Wood
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 *
 * $Id: osx.c,v 1.26 2007/10/22 10:33:08 cmatsuoka Exp $
 * ---
 */

#import "AQPlayer.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <unistd.h>
#include <AudioToolbox/AudioQueue.h>

#define NUM_BUFFERS     2
#define BUFSIZE         32000
#define SAMPLE_RATE     44100

typedef struct AQCallbackStruct {
    AudioQueueRef                    queue;
    UInt32                            frameCount;
    AudioQueueBufferRef                mBuffers[NUM_BUFFERS];
    AudioStreamBasicDescription        mDataFormat;
} AQCallbackStruct;

AQCallbackStruct in;
int writeSampleNr = 0;
int readSampleNr = 0;
BOOL playing = false;
short *ringbuffer;

static void AQBufferCallback(void *in,    AudioQueueRef inQ, AudioQueueBufferRef outQB) {
    // Assign struct and pointer for output buffer
    AQCallbackStruct *inData = (AQCallbackStruct *)in;
    short *coreAudioBuffer = (short*) outQB->mAudioData;
    
    // as long as frameCount > 0, we don't have data to process
    if (inData->frameCount > 0) {
        // Assign buffer size
        outQB->mAudioDataByteSize = 4*inData->frameCount; // two shorts per sample * 2 channels
        
        // For every sample by channel
        if(playing == TRUE) {
            // printf("read_nr: %d, write_nr: %d", readSampleNr, writeSampleNr);
            // printf("Rendering %d bytes of data in buffer %d", inData->frameCount, outQB);
            /*int bufFree = 0;
            if (writeSampleNr >= readSampleNr) {
                bufFree = (BUFSIZE - writeSampleNr) + readSampleNr;
            } else {
                bufFree = readSampleNr - writeSampleNr;
            }
            printf("Audio buf free: %d\n", bufFree);*/
            for(int i=0; i<inData->frameCount*2; i+=2) {
                coreAudioBuffer[i] = ringbuffer[readSampleNr]; // left channel
                coreAudioBuffer[i+1] = ringbuffer[readSampleNr+1]; // right channel
                readSampleNr += 2;
                if(readSampleNr >= BUFSIZE) {
                    readSampleNr -= BUFSIZE;
                }
            }
        } else {
            // empty buffer
            memset(coreAudioBuffer, 0, inData->frameCount*4);
        }
        
        // Purge buffer
        AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
    }
}


@implementation AQPlayer

- (int) getAudiobufFree {
    if(!playing) return 0;
    
    if (writeSampleNr >= readSampleNr) {
        return (BUFSIZE - writeSampleNr) + readSampleNr;
    } else {
        return readSampleNr - writeSampleNr;
    }
}

- (void) addSampleLeftRightTuple:(short[2])samples {
    // if we're writing samples to close to the read pointer, re-adjust the write pointer.
    int bufFree = [self getAudiobufFree];
    if(bufFree < ((BUFSIZE * 10) / 100) || bufFree > ((BUFSIZE * 90) / 100)) {
        writeSampleNr = readSampleNr - (BUFSIZE * 50)/100;
        while(writeSampleNr < 0) {
            writeSampleNr += BUFSIZE;
        }
        while(writeSampleNr >= BUFSIZE) {
            writeSampleNr = BUFSIZE;
        }
    }
    ringbuffer[writeSampleNr] = samples[0];
    if(++writeSampleNr >= BUFSIZE)
        writeSampleNr -= BUFSIZE;
    
    ringbuffer[writeSampleNr] = samples[1];
    if(++writeSampleNr >= BUFSIZE)
        writeSampleNr -= BUFSIZE;
}

- (AQPlayer*) init
{
    self = [super init];
    
    int i;
    
    ringbuffer = malloc(BUFSIZE*2); // we have shorts
    
    in.mDataFormat.mSampleRate = SAMPLE_RATE;
    in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
    in.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    in.mDataFormat.mBytesPerPacket = 4;
    in.mDataFormat.mFramesPerPacket = 1;
    in.mDataFormat.mBytesPerFrame = 4;
    in.mDataFormat.mChannelsPerFrame = 2;
    in.mDataFormat.mBitsPerChannel = 16;
    in.frameCount = 1024;
    
    UInt32 err = AudioQueueNewOutput(&in.mDataFormat,
                                     AQBufferCallback,
                                     &in,
                                     CFRunLoopGetCurrent(),
                                     kCFRunLoopCommonModes,
                                     0,
                                     &in.queue);
    if(err) fprintf(stderr, "AudioQueueNewOutput err %d\n", err);
    
    // Calcluate buffer size
    UInt32 bufferBytes = in.frameCount * in.mDataFormat.mBytesPerFrame;
    
    // allocate buffers.
    for (i=0; i< NUM_BUFFERS; i++) {
        err = AudioQueueAllocateBuffer(in.queue, bufferBytes, &in.mBuffers[i]);
        if(err) fprintf(stderr, "AudioQueueAllocateBuffer [%d] err %d\n",i, err);
        // First initial call of the callback function to pre-fill the buffers
        AQBufferCallback (&in, in.queue, in.mBuffers[i]);
    }
    
    // Set volume
    err = AudioQueueSetParameter(in.queue, kAudioQueueParam_Volume, 1.0);
    if(err) fprintf(stderr, "AudioQueueSetParameter err %d\n", err);
    
    err = AudioQueueStart(in.queue, NULL);
    if(err) fprintf(stderr, "AudioQueueStart err %d\n", err);
    
    return self;
}

- (void) stopPlaying {
    playing = FALSE;
}

- (void) startPlaying {
    playing = TRUE;
    writeSampleNr = 0;
    readSampleNr = 0;
}

- (BOOL) isPlaying {
    return playing;
}

- (void) setVolume:(int)vol {
    AudioQueueSetParameter(in.queue, kAudioQueueParam_Volume, (float)vol / 100);
}

@end




