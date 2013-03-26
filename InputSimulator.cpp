/* -------------------------------------------------------------------------------------------------
 * InputSimulator.cpp - implementation file for CInputSimulator class that is an OOP wrapper for 
 * windows input API.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "InputSimulator.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals and constants

CInputSimulator::CInputSimulator ( DWORD dwEventsCount )
{
    m_pInputs = ( LPINPUT ) HeapAlloc ( GetProcessHeap ( ) , 
                                        HEAP_ZERO_MEMORY , 
                                        sizeof ( INPUT ) * dwEventsCount ) ;
    m_dwInputsQueued = 0 ;
}

CInputSimulator::~CInputSimulator(void)
{
    if ( NULL != m_pInputs )
        HeapFree ( GetProcessHeap ( ) , 0 , m_pInputs ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Appends event to the queue that will be simulated
void CInputSimulator::AppendEvent ( DWORD dwKeyCode , BOOL bKeyDown )
{
    if ( NULL == m_pInputs )
        return ;

    INPUT input = { 0 } ;
    input.type = INPUT_KEYBOARD ;
    input.ki.wVk = ( WORD ) dwKeyCode ;
    if ( !bKeyDown )
        input.ki.dwFlags = KEYEVENTF_KEYUP ;

    *( m_pInputs + m_dwInputsQueued++ ) = input ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends input stored in the current instance
BOOL CInputSimulator::Send ( )
{
    return m_dwInputsQueued == SendInput ( m_dwInputsQueued , m_pInputs , sizeof ( INPUT ) ) ;
}