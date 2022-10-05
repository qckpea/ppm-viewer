#include "windows.h"

long long gPerfCountFrequency;

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return(result);
}

inline float
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    float result = ((float)(end.QuadPart - start.QuadPart) /
                     (float)gPerfCountFrequency);
    return(result);
}