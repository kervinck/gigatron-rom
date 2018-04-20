#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>


#define VSYNC_START   -36
#define VSYNC_RATE     60
#define SCAN_LINES     521
#define HLINE_START    0
#define HLINE_END      200
#define HPIXELS_START  13
#define HPIXELS_END    173
#define TIMING_HACK    0.016666


namespace Timing
{
    bool getFrameUpdate(void);
    uint64_t getFrameCount(void);
    double getFrameTime(void);
    double getTimingHack(void);

    void setTimingHack(double hack);

    void synchronise(void);
}

#endif

