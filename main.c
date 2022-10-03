#include "main.h"

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
   
    bool bRet;
    MSG message;

    gIsAppRunning = true;
    while (gIsAppRunning) {
        while ((bRet = PeekMessageA(&message, gWindowHandler, 0, 0, PM_REMOVE)) != 0)
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        Sleep(1);
     }
}

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (message)
    {
        case WM_CLOSE: 
        {
            gIsAppRunning = false;
            PostQuitMessage(0);
            break;
        }
        case WM_PAINT: 
        {
            displayImage();
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = (uint32)wParam;
            
            // uint32 WasDown = ((lParam & (1 << 30)) != 0);
            uint32 IsDown = ((lParam & (1 << 31)) == 0);

            if(IsDown)
            {
                // bool AltKeyWasDown = (bool)(lParam & (1 << 29));
                bool isCtrlDown = GetKeyState(VK_CONTROL);
                
                if((VKCode == 'O') && isCtrlDown)
                {
                    OPENFILENAME ofn;       // common dialog box structure
                    char szFile[260];       // buffer for file name
                    
                    // Initialize OPENFILENAME
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = gWindowHandler;
                    ofn.lpstrFile = szFile;
                    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
                    // use the contents of szFile to initialize itself.
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "PPM\0*.PPM\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 0;
                    ofn.lpstrInitialDir = NULL;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    // Display the Open dialog box. 

                    if (GetOpenFileName(&ofn) == TRUE) 
                    {
                        if(loadFile(ofn.lpstrFile, gSource)) {
                            parsePPM(gSource, &gPPM);
                            createImageBuffer();
                            SetWindowPos(gWindowHandler, 0, 0, 0, gPPM.width, gPPM.height, SWP_NOMOVE);
                        }
                    }
                }
            }
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
    windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));

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

inline bool loadFile(const char* filename, char *buffer)
{
    bool result = true;
    FILE *fp = fopen(filename, "r");

    if (fp)
    {
        size_t newLen = fread(buffer, sizeof(char), MAX_BUF_LEN, fp);
        if (newLen == MAX_BUF_LEN) {
            MessageBoxA(NULL, "File is too large!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            result = false;
        }
        if ( ferror( fp ) != 0 ) {
            MessageBoxA(NULL, "Could not read the file content!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            result = false;
        } else {
            buffer[newLen++] = '\0';
        }

        fclose(fp);

    } else {
        MessageBoxA(NULL, "Could not open the file!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        result = false;
    }

    return result;
}

void parsePPM(const char* stream, PPM *result)
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
        stream++;
    }

    uint32 width = 0;
    while (isdigit(*stream)) {
        width *= 10;
        width += *stream++ - '0';
    }

    result->width = width;

    // height
    while (!isdigit(*stream))
    {
        stream++;
    }

    uint32 height = 0;
    while (isdigit(*stream)) {
        height *= 10;
        height += *stream++ - '0';
    }

    result->height = height;
       
    // max color value
    while (!isdigit(*stream))
    {
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

    if (result->pixels) {
        free(result->pixels);
    }
    result->pixels = malloc(bufferSize);
    memcpy(result->pixels, (const void *)pixelValues, bufferSize);
    free(pixelValues);
}

inline void createImageBuffer(void) {
    gBitmap.bitmapInfo.bmiHeader.biSize = sizeof(gBitmap.bitmapInfo.bmiHeader);
    gBitmap.bitmapInfo.bmiHeader.biWidth = gPPM.width;
    gBitmap.bitmapInfo.bmiHeader.biHeight = -gPPM.height;
    gBitmap.bitmapInfo.bmiHeader.biBitCount = 32;
    gBitmap.bitmapInfo.bmiHeader.biCompression = BI_RGB;
    gBitmap.bitmapInfo.bmiHeader.biPlanes = 1;

    if ((gBitmap.memory = VirtualAlloc(NULL, gPPM.width * gPPM.height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL) {
        MessageBoxA(NULL, "Could not allocate video memory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        exit(2);
    }
}

inline void displayImage(void) {
    if (gBitmap.memory) {
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

        HDC deviceContext = GetDC(gWindowHandler);

        RECT rect;
        GetClientRect(gWindowHandler, &rect);
        int dX = (rect.left + rect.right) / 2 - (gPPM.width / 2);
        int dWidth = gPPM.width;
        int dY = (rect.top + rect.bottom) / 2 - (gPPM.height / 2);;
        int dHeight = gPPM.height;
        StretchDIBits(deviceContext, dX, dY, dWidth, dHeight, 0, 0, gPPM.width, gPPM.height, gBitmap.memory, &gBitmap.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        ReleaseDC(gWindowHandler, deviceContext);
    }
        
    
}