#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)
#pragma warning(disable: 5045) // spectre mitigation

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>

#include "timer.h"

#define uint8 uint8_t
#define uint16 uint16_t 
#define uint32 uint32_t 
#define int8 int8_t 
#define int16 int16_t 
#define int32 int32_t 

#define APP_NAME "PPM Viewer"
#define MAX_BUF_LEN 100000000

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

char gSource[MAX_BUF_LEN];
HWND gWindowHandler;
bool gIsAppRunning = false;

PPM gPPM;
APP_BITMAP gBitmap;

LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);
void ProcessPendingMessages(void);
DWORD CreateMainWindow(HINSTANCE instance);
bool loadFile(const char* filename, char* buffer);
void parsePPM(const char* source, PPM *result);
void createImageBuffer(void);
void displayImage(void);
