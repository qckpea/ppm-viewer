#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define uint8 uint8_t
#define uint16 uint16_t 
#define uint32 uint32_t 
#define int8 int8_t 
#define int16 int16_t 
#define int32 int32_t 

#define APP_NAME "PPM Viewer"
#define MAX_BUF_LEN 100000

char gSource[MAX_BUF_LEN];
HWND gWindowHandler;
bool gIsAppRunning = false;

typedef struct PPM
{
    char header[2];
    int16 _unusedPadding;
    uint32 width;
    uint32 height;
    uint32 maxColorVal;
    void *pixels;
} PPM;

typedef struct APP_BITMAP 
{
	void* memory;
	BITMAPINFO bitmapInfo;
    int32 _unusedPadding;
} APP_BITMAP;

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE instance);
bool loadFile(const char* filename, char* buffer);
void parsePPM(const char* source, PPM *result);
