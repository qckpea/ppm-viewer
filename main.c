#include "main.h"

#define UNICODE

int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    DWORD result = CreateMainWindow(hInstance);
    if (result != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Could not create window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
    }

    int monitorRefreshHz = 60;
    HDC refreshDC = GetDC(gWindowHandler);
    int win32RefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
    ReleaseDC(gWindowHandler, refreshDC);
    if(win32RefreshRate > 1)
    {
        monitorRefreshHz = win32RefreshRate;
    }
    float updateHz = (monitorRefreshHz / 2.0f);
    float targetSecondsPerFrame = 1.0f / (float)updateHz;

    UINT desiredSchedulerMS = 1;
    bool isSleepGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);

    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    gPerfCountFrequency = perfCountFrequencyResult.QuadPart;
    LARGE_INTEGER lastCounter = Win32GetWallClock();

    gIsAppRunning = true;
    while (gIsAppRunning) {
        
        ProcessPendingMessages();

        LARGE_INTEGER workCounter = Win32GetWallClock();
        float secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter, workCounter);
        
        if(secondsElapsedForFrame < targetSecondsPerFrame)
        {                        
            if(isSleepGranular)
            {
                DWORD sleepInMS = (DWORD)(1000.0f * (targetSecondsPerFrame - secondsElapsedForFrame));
                if(sleepInMS > 0)
                {
                    Sleep(sleepInMS);

                    if (gLoadedFileData.fileName && strlen(gLoadedFileData.fileName) > 1) {
                        FILETIME newFileWriteTime = Win32GetLastWriteTime(gLoadedFileData.fileName);
                        if (CompareFileTime(&newFileWriteTime, &gLoadedFileData.gLastFileTime) != 0)
                        {
                            Win32OpenFile();
                            DisplayImage();
                        }
                    }
                }
            }
            
            float testSecondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter, Win32GetWallClock());
            if(testSecondsElapsedForFrame < targetSecondsPerFrame)
            {
                // missed sleep
            }
            
            while(secondsElapsedForFrame < targetSecondsPerFrame)
            {                            
                secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter, Win32GetWallClock());
            }
        }

        LARGE_INTEGER endCounter = Win32GetWallClock();
        //float elapsedMSPerFrame = 1000.0f * Win32GetSecondsElapsed(lastCounter, endCounter);
        lastCounter = endCounter;

         //char FPSBuffer[256];
         //_snprintf(FPSBuffer, sizeof(FPSBuffer),"%.02fms/f\n", elapsedMSPerFrame );
         //OutputDebugStringA(FPSBuffer);
    }

    return 0;
}

void ProcessPendingMessages(void) {
    MSG message;
    while(PeekMessageA(&message, gWindowHandler, 0, 0, PM_REMOVE))
    {
        
        switch(message.message) {
            case WM_QUIT:
            case WM_CLOSE:
            {
                gIsAppRunning = false;
                PostQuitMessage(0);
                
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)message.wParam;
                
                // uint32 WasDown = ((lParam & (1 << 30)) != 0);
                uint32 IsDown = ((message.lParam & (1 << 31)) == 0);

                if(IsDown)
                {
                    // bool AltKeyWasDown = (bool)(lParam & (1 << 29));
                    bool isCtrlDown = GetKeyState(VK_CONTROL);
                    
                    if((VKCode == 'O') && isCtrlDown)
                    {
                        OPENFILENAME openFileName; // common dialog box structure
                        char szFile[260];       // buffer for loadedFileData name

                        // Initialize OPENFILENAME
                        ZeroMemory(&openFileName, sizeof(openFileName));
                        openFileName.lStructSize = sizeof(openFileName);
                        openFileName.hwndOwner = gWindowHandler;
                        openFileName.lpstrFile = szFile;
                        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
                        // use the contents of szFile to initialize itself.
                        openFileName.lpstrFile[0] = '\0';
                        openFileName.nMaxFile = sizeof(szFile);
                        openFileName.lpstrFilter = "PPM\0*.PPM\0";
                        openFileName.nFilterIndex = 1;
                        openFileName.lpstrFileTitle = NULL;
                        openFileName.nMaxFileTitle = 0;
                        openFileName.lpstrInitialDir = NULL;
                        openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                        // Display the Open dialog box. 

                        if (GetOpenFileName(&openFileName) == TRUE)
                        {
                            size_t fileNameLenght = strlen(openFileName.lpstrFile);
                            gLoadedFileData.fileName = realloc((void*)gLoadedFileData.fileName, fileNameLenght);
                            strcpy(gLoadedFileData.fileName, openFileName.lpstrFile);
                            Win32OpenFile();
                        }
                    }
                }
            } break;

            case WM_MOUSEWHEEL:
            {
                short zDelta = GET_WHEEL_DELTA_WPARAM(message.wParam);
                if (zDelta < 0) {
                    gZoomLevel -= 0.2f;
                    if (gZoomLevel < 0.0f) {
                        gZoomLevel = 0.0f;
                    }
                } else {
                    gZoomLevel += 0.2f;
                    if (gZoomLevel > 3.0f) {
                        gZoomLevel = 3.0f;
                    }
                }

                InvalidateRect(gWindowHandler, NULL, FALSE);

            } break;
                
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
}

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(gWindowHandler, &ps);
            DisplayImage();
            EndPaint(gWindowHandler, &ps);
            break;
        }
        case WM_DESTROY:
        {
            gIsAppRunning = false;
        }
        
        default: 
        {
            result = DefWindowProcA(windowHandler, message, wParam, lParam);
            break;
        }
    }

    return result;
}

inline DWORD CreateMainWindow(HINSTANCE instance) {
    DWORD result = ERROR_SUCCESS;

    WNDCLASSA windowClass = { 0 };
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconA(instance, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorA(instance, IDC_ARROW);
    windowClass.lpszClassName = "PPMViewerWindowClass";
    windowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    if (RegisterClassA(&windowClass) == 0) {
        MessageBoxA(NULL, "Could not register window class!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        result = GetLastError();
        goto Exit;
    }

    gWindowHandler = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        APP_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // or ShowWindow(windowHandler, TRUE) instead of WS_VISIBLE;
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        instance,
        NULL
    );

    if (gWindowHandler == NULL) {
        MessageBoxA(NULL, "Could not create window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        result = GetLastError();
        goto Exit;
    }

Exit:
    return result;
}

inline bool LoadFileContentIntoBuffer(const char* filename)
{
    bool result = false;
    
    HANDLE FileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);

            if (gSource) {
                VirtualFree(gSource, 0, MEM_RELEASE);
            }
            gSource = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            if(gSource)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, gSource, FileSize32, &BytesRead, 0) &&
                (FileSize32 == BytesRead))
                {
                    // File read successfully
                    gLoadedFileData.gLastFileTime = Win32GetLastWriteTime(gLoadedFileData.fileName);
                    result = true;
                }
                else
                {                    
                    // Logging
                    MessageBoxA(NULL, "Could not open the file!", "Error!", MB_ICONEXCLAMATION | MB_OK);
                    VirtualFree(gSource, 0, MEM_RELEASE);
                }
            }
            else
            {
                // Logging
                MessageBoxA(NULL, "Could not open the file!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            }
        }
        else
        {
            // Logging
            MessageBoxA(NULL, "Could not open the file!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // Logging
        // TODO: this path is sometimes triggered at file reload
    }

    return result;
}

void ParsePPM(const char* stream, PPM *result)
{  
    // header
    result->header[0] = *(stream++);
    if (result->header[0] != 'P') {
        MessageBoxA(NULL, "Invalid file header!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    }
    result->header[1] = *(stream++);

    // width
    while(!isdigit(*stream))
    {
        if (*stream == '#') {
            while(!iscntrl(*stream)) {
                stream++;
            }
        }
        stream++;
    }

    uint32 width = 0;
    while (isdigit(*stream)) {
        width *= 10;
        width += *stream++ - '0';
    }

    result->width = width;

    // height
    while(!isdigit(*stream))
    {
        if (*stream == '#') {
            while(!iscntrl(*stream)) {
                stream++;
            }
        }
        stream++;
    }

    uint32 height = 0;
    while (isdigit(*stream)) {
        height *= 10;
        height += *stream++ - '0';
    }

    result->height = height;
    
    // max color value
    while(!isdigit(*stream))
    {
        if (*stream == '#') {
            while(!iscntrl(*stream)) {
                stream++;
            }
        }
        stream++;
    }

    uint16 maxColorVal = 0;
    while (isdigit(*stream)) {
        maxColorVal *= 10;
        maxColorVal += *stream++ - '0';
    }

    result->maxColorVal = maxColorVal;

    // pixel data
    uint32 count = 0;
    uint32 rgbaChanelCount = 0;
    uint32 bufferSize = height * width * 4 * sizeof(uint8);
    uint8 *pixelValues = malloc(bufferSize);
    
    // pixel data is in ascii/text format
    if (result->header[1] == '3') {
        
        for (uint32 i = 0; i < height * width * 3; i++) {
            
            while (!isdigit(*stream)) {
                stream++;
            }
            
            uint16 colorVal = 0;
            while (isdigit(*stream)) {
                colorVal *= 10;
                colorVal += *stream++ - '0';
            }

            if (colorVal > result->maxColorVal)
            {
                MessageBoxA(NULL, "Corrupt file data! Color value is more than max color value!", "Error!", MB_ICONEXCLAMATION | MB_OK);
                break;
            }

            pixelValues[count] = (uint8)((255.0f / gPPM.maxColorVal) * colorVal);

            count++;
            rgbaChanelCount++;

            if ((rgbaChanelCount) % 3 == 0) {
                // add alpha manually
                pixelValues[count] = 255;
                count++;
            }  
        }
    } 
    // pixel data is in binary format
    else if (result->header[1] == '6')
    {
        // advance the stream to the first pixel data
        while(!iscntrl(*stream)) {
            stream++;
        }
        stream++;

        // start parsing the pixel data
        for (uint32 i = 0; i < height * width * 3; i++) {
            uint8 colorVal = *stream++;
            pixelValues[count] = (uint8)((255.0f / gPPM.maxColorVal) * colorVal);
                
            count++;
            rgbaChanelCount++;

            if ((rgbaChanelCount) % 3 == 0) {
                // add alpha manually
                pixelValues[count] = 255;
                count++;
            }  
        }
    }

    if (result->pixels) {
        free(result->pixels);
    }
    result->pixels = malloc(bufferSize);
    memcpy(result->pixels, (const void *)pixelValues, bufferSize);
    free(pixelValues);
}

inline void CreateImageBuffer(void) {
    gBitmap.bitmapInfo.bmiHeader.biSize = sizeof(gBitmap.bitmapInfo.bmiHeader);
    gBitmap.bitmapInfo.bmiHeader.biWidth = gPPM.width;
    gBitmap.bitmapInfo.bmiHeader.biHeight = -gPPM.height;
    gBitmap.bitmapInfo.bmiHeader.biBitCount = 32;
    gBitmap.bitmapInfo.bmiHeader.biCompression = BI_RGB;
    gBitmap.bitmapInfo.bmiHeader.biPlanes = 1;

    if (gBitmap.memory) {
        VirtualFree(gBitmap.memory, 0, MEM_RELEASE);
    }
    if ((gBitmap.memory = VirtualAlloc(NULL, gPPM.width * gPPM.height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL) {
        MessageBoxA(NULL, "Could not allocate video memory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        exit(2);
    }
}

inline void FillImageBuffer(void) {
    const uint8 channelCount = 4;
    __m128i* p = (__m128i*)gBitmap.memory;
    for (uint32 i = 0; i < (gPPM.height * gPPM.width * channelCount); i += (sizeof(__m128i))) {
        
        __m128i v = {0, 0, 0, 0};
        uint32 count = 0;
        for (uint32 j = i; j < (i + channelCount * sizeof(uint32)); j += channelCount) {

            PIXEL32 pixel = {0};
            pixel.red =   (*((uint8*)gPPM.pixels + j + 0));
            pixel.green = (*((uint8*)gPPM.pixels + j + 1));
            pixel.blue =  (*((uint8*)gPPM.pixels + j + 2));
            pixel.alpha = (*((uint8*)gPPM.pixels + j + 3));

            v.m128i_u32[count++] = (uint32)(pixel.blue  << 0)  |
                                    (uint32)(pixel.green << 8)  |
                                    (uint32)(pixel.red   << 16) |
                                    (uint32)(pixel.alpha << 24) ;

        }

        _mm_store_si128((__m128i*)p, v);
        p++;
    }
}

void DisplayImage(void) {
    if (gBitmap.memory) {
        HDC deviceContext = GetDC(gWindowHandler);

        RECT rect;
        GetClientRect(gWindowHandler, &rect);
        int dX = (rect.left + rect.right) / 2 - (int)(gPPM.width * gZoomLevel / 2);
        int dWidth = (int)(gPPM.width * gZoomLevel);
        int dY = (rect.top + rect.bottom) / 2 - (int)(gPPM.height * gZoomLevel / 2);;
        int dHeight = (int)(gPPM.height * gZoomLevel);

        if (rect.right > dWidth) {
            // left
            PatBlt(deviceContext, 0, dY, dX, dHeight, BLACKNESS);
            // right
            PatBlt(deviceContext, dX + dWidth, dY, rect.right - dWidth, dHeight, BLACKNESS);
        }

        if (rect.bottom > dHeight) {
            // top
            PatBlt(deviceContext, 0, 0, rect.right, dY, BLACKNESS);
            // bottom
            PatBlt(deviceContext, 0, dY + dHeight, rect.right, rect.bottom - dHeight, BLACKNESS);
        }
        
        StretchDIBits(deviceContext, dX, dY, dWidth, dHeight, 0, 0, gPPM.width, gPPM.height, gBitmap.memory, &gBitmap.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        ReleaseDC(gWindowHandler, deviceContext);
    }
}

void Win32OpenFile(void) {
    if (LoadFileContentIntoBuffer(gLoadedFileData.fileName)) {
        ParsePPM((const char*)gSource, &gPPM);
        CreateImageBuffer();
        FillImageBuffer();
        SetWindowPos(gWindowHandler, 0, 0, 0, gPPM.width, gPPM.height, SWP_NOMOVE);
    }
}