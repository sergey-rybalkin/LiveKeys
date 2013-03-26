/* -------------------------------------------------------------------------------------------------
 * InputSimulator.h - header file for CInputSimulator class that is an OOP wrapper for windows input
 * API.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "hotkeyshandler.h"

class CInputSimulator
{
public:
    CInputSimulator ( DWORD dwEventsCount ) ;
    ~CInputSimulator ( ) ;

    void AppendEvent ( DWORD dwKeyCode , BOOL bKeyDown ) ;
    BOOL Send ( ) ;
private:
    INPUT* m_pInputs ;
    DWORD  m_dwInputsQueued ;
};
