// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <strsafe.h>
#include <shellapi.h>
#include <iostream>

// Fake resources
#define IDS_ERROR_ALREADY_RUNNING       104
#define IDS_CAPTION_ERROR               105
#define IDS_ERROR_HOOK_FAILED           106
#define IDI_SHELLEXTENDER               107
#define IDS_ERROR_NO_EXECUTOR_KEY       107
#define IDS_ERROR_CANNOT_ENUM_EXECUTOR_KEY 108
#define IDC_SHELLEXTENDER               109
#define IDS_ERROR_KEY_NAME_LONG         110
#define IDS_ERROR_ASYNC_KEY_MASK_READ   111
#define IDS_ERROR_MAIN_KEY_READ         112
#define IDS_ERROR_PARAMS_READ           113
#define IDS_ERROR_PATH_READ             114