#include "main.h"

bool loadFile(const char* filename, void* memory)
{
    UNREFERENCED_PARAMETER(memory);
    bool result = true;
    FILE *f = fopen(filename, "r");
    if (f)
    {
        printf("File opened: %s\n", filename);
        char line[512];
        while(fgets(line, _countof(line), f) != NULL) {
            printf("%s\n", line);
        }
    } else {
        printf("File open failed: %s\n", filename);
        result = false;
    }

    return result;
}

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

    void* m = 0;
    loadFile("test.ppm", m);

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
        printf("Error: Could not register window class!\n");
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
        printf("Error: Could not create window!\n");
        result = GetLastError();
        goto Exit;
    }

Exit:
    return result;
}

