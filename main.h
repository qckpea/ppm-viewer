#pragma once

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include <stdbool.h>
#include <stdio.h>

#define APP_NAME "PPM Viewer"

HWND gWindowHandler;
bool gIsAppRunning = false;

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE instance);

