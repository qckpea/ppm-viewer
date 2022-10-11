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

inline FILETIME
Win32GetLastWriteTime(const char *filename)
{
    FILETIME LastWriteTime = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA Data = {0};
    if(GetFileAttributesEx(filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}