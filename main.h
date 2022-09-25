#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include <stdbool.h>
#include <stdio.h>

#define APP_NAME "PPM Viewer"
#define MAXBUFLEN 100000

char gSource[MAXBUFLEN];
HWND gWindowHandler;
bool gIsAppRunning = false;

typedef struct PPM
{
    const char* header;
    unsigned int height;
    unsigned int width;
    unsigned int maxColorVal;
    int _unusedPadding;
    void *pixels;
} PPM;

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE instance);
bool loadFile(const char* filename, char* buffer);
