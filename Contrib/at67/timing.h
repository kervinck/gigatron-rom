#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>


#define VSYNC_START        -36
#define VSYNC_RATE          60
#define SCAN_LINES          521
#define HLINE_START         0
#define HLINE_END           200
#define HPIXELS_START       13
#define HPIXELS_END         173
#define VSYNC_TIMING_60     0.01667222407469 // 59.98Hz
#define NON_CRITICAL_TIMING (VSYNC_TIMING_60*3.0)

#define CLOCK_FREQ   6250000
#define CLOCK_RESET -2

// Minimum number of clocks before ROM type is valid in RAM; in the future could be ROM dependant, currently this value works with ROMS v1 through v4
#define STARTUP_DELAY_CLOCKS     (CLOCK_FREQ * 0.2)
#define CPU_STALL_CLOCKS         500000
#define SINGLE_STEP_STALL_CLOCKS 2000
#define MAX_SINGLE_STEP_CLOCKS   1000


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