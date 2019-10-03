#include <algorithm>

#include <SDL.h>
#include "timing.h"


namespace Timing
{
    bool _frameUpdate = false;
    uint64_t _frameCount = 0;
    double _frameTime = 0.0;
    double _timingAdjust = VSYNC_TIMING_60;


    bool getFrameUpdate(void) {return _frameUpdate;}
    uint64_t getFrameCount(void) {return _frameCount;}
    double getFrameTime(void) {return _frameTime;}
    double getTimingHack(void) {return _timingAdjust;}

    void setFrameUpdate(bool update) {_frameUpdate = update;}
    void setTimingHack(double hack) {_timingAdjust = hack;}


    void synchronise(void)
    {
        static uint64_t prevFrameCounter = 0;

        do
        {
            _frameTime = double(SDL_GetPerformanceCounter() - prevFrameCounter) / double(SDL_GetPerformanceFrequency());
        }
        while(_frameTime < _timingAdjust);
        prevFrameCounter = SDL_GetPerformanceCounter();

        _frameCount++;

        // Used for updating at a constant 60 times per second no matter what the FPS is
        _frameUpdate = ((_frameCount % int(VSYNC_TIMING_60/std::min(_frameTime, VSYNC_TIMING_60))) == 0);
    }
}