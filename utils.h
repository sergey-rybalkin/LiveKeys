/* -------------------------------------------------------------------------------------------------
 * utils.h - header file for a set of utility functions used through the whole application.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#pragma once

extern HINSTANCE g_hInst ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Reports custom application error to the user
VOID ShowCustomError( UINT uiCaptionStringID , UINT uiMsgStringID ) ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Translates a Win32 error into a text equivalent
VOID ShowWin32Error( DWORD dwErrorCode , UINT uiCaptionStringID = 0 , UINT uiMsgStringID = 0 ) ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Silently prints text equivalent of Win32 error into a diagnostics buffer.
#ifdef _DEBUG

VOID DbgPrintWin32Error( DWORD dwErrorCode , UINT uiMsgStringID = 0 , ... ) ;

#else

#define DbgPrintWin32Error( ErrorCode, MsgStringID, ... )

#endif