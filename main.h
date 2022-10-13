#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)
#pragma warning(disable: 5045) // spectre mitigation

#include <stdbool.h>
#include <stdio.h>
#include <emmintrin.h>

#include "timer.h"
#include "utils.h"


#define APP_NAME "PPM Viewer"

static float gZoomLevel = 1.0f;

typedef struct PPM
{
    char header[2];
    int16 _unusedPadding;
    uint32 width;
    int32 height;
    uint16 maxColorVal;
    int16 _unusedPadding2;
    void *pixels;
} PPM;

typedef struct APP_BITMAP 
{
	void* memory;
	BITMAPINFO bitmapInfo;
    int32 _unusedPadding;
} APP_BITMAP;

typedef struct PIXEL32 {
	uint8 blue;
	uint8 green;
	uint8 red;
	uint8 alpha;
} PIXEL32;

void * gSource = 0;
HWND gWindowHandler;
bool gIsAppRunning = false;

PPM gPPM;
APP_BITMAP gBitmap;

typedef struct LOADED_FILE_DATA {
    FILETIME gLastFileTime;
    char * fileName;
} LOADED_FILE_DATA;

LOADED_FILE_DATA gLoadedFileData = {0};

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);
void ProcessPendingMessages(void);
DWORD CreateMainWindow(HINSTANCE instance);
bool LoadFileContentIntoBuffer(const char* filename);
void ParsePPM(const char* source, PPM *result);
void CreateImageBuffer(void);
void FillImageBuffer(void);
void DisplayImage(void);
void Win32OpenFile(void);