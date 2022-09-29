#include "main.h"

#define MAXBUFLEN 100000
char gSource[MAX_BUF_LEN];
PPM gPPM;

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

    loadFile("test.ppm", gSource);
    parsePPM(gSource, &gPPM);

    bool bRet;
    MSG message;

    gIsAppRunning = true;
    while (gIsAppRunning) {
        while ((bRet = PeekMessageA(&message, gWindowHandler, 0, 0, PM_REMOVE)) != 0)
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
     }
}

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (message)
    {
        case WM_CLOSE: {
            gIsAppRunning = false;
            PostQuitMessage(0);
            break;
        }
        default: {
            result = DefWindowProcA(windowHandler, message, wParam, lParam);
            break;
        }
    }

    return result;
}

DWORD CreateMainWindow(HINSTANCE instance) {
    DWORD result = ERROR_SUCCESS;

    WNDCLASSA windowClass = { 0 };
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconA(instance, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorA(instance, IDC_ARROW);
    windowClass.lpszClassName = "PPMViewerWindowClass";

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

bool loadFile(const char* filename, char *buffer)
{
    bool result = true;
    FILE *fp = fopen(filename, "r");
    if (fp)
    {
        size_t newLen = fread(buffer, sizeof(char), MAX_BUF_LEN, fp);
        if ( ferror( fp ) != 0 ) {
            MessageBoxA(NULL, "Could not read the file content!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        } else {
            buffer[newLen++] = '\0';
        }
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

    uint32 maxColorVal = 0;
    while (isdigit(*stream)) {
        maxColorVal *= 10;
        maxColorVal += *stream++ - '0';
    }

    result->maxColorVal = maxColorVal;

    // pixel data
    uint32 count = 0;
    uint32 pixelValues[48] = {0};
    while (*stream)
    {
        if(isdigit(*stream))
        {
            uint32 colorVal = 0;
            while (isdigit(*stream)) {
                colorVal *= 10;
                colorVal += *stream++ - '0';
            }

            if (colorVal > result->maxColorVal)
            {
                MessageBoxA(NULL, "Corrupt file data! Color value is more than max color value!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            }
            pixelValues[count] = colorVal;
            count++;
        }
        else {
            stream++;
        }
    }

    result->pixels = malloc(count * sizeof(uint32));
    memcpy(result->pixels, (const void *)pixelValues, count * sizeof(uint32));
}