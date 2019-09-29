#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>


#define VSYNC_START     -36
#define VSYNC_RATE       60
#define SCAN_LINES       521
#define HLINE_START      0
#define HLINE_END        200
#define HPIXELS_START    13
#define HPIXELS_END      173
#define VSYNC_TIMING_60  0.0166667

#define CLOCK_FREQ   6250000
#define CLOCK_RESET -2

#define STARTUP_DELAY_CLOCKS    (CLOCK_FREQ * 1)
#define CPU_STALL_CLOCKS        500000
#define SINGLE_STEP_STALL_TIME  1000


namespace Timing
{
    bool getFrameUpdate(void);
    uint64_t getFrameCount(void);
    double getFrameTime(void);
    double getTimingHack(void);

    void setFrameUpdate(bool update);
    void setTimingHack(double hack);

    void synchronise(void);
}

#endif

