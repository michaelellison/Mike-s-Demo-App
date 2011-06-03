//---------------------------------------------------------------------------
/// \file    CATThread.cpp
/// \brief   Base Thread Class
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------

#include "CATThread.h"

#ifndef _WIN32_WCE
    #include <process.h>
    /// OS specific thread - note, the param is NOT the user param, 
    /// rather it is a pointer to the thread object....
    unsigned int WINAPI W32ThreadProc(void *param);
#else
    DWORD WINAPI W32ThreadProc(void *param);
#endif


// Thread construction
CATThread::CATThread()
{
    this->fThreadHandle	= 0;
    this->fUserParam	= 0;
    this->fCallback		= 0;
}

// Thread destruction
CATThread::~CATThread()
{
    if (this->fThreadHandle != 0)
    {
        if (!this->WaitStop(1000))
        {
            CATTRACE("Warning: Forcing thread to stop...");
            this->ForceStop();
        }
    }
}


// Start a thread. This is the one used if you're deriving from
// CATThread for your own threaded class.  Override ThreadFunction()
// for your subclass...
bool CATThread::Start(void *param)
{
    CATASSERT(fThreadHandle == 0, "Starting a thread that's already running. Bad form....");
    if (fThreadHandle != 0)
    {
        return false;
    }

    this->fCallback  = 0;
    this->fUserParam = param;
#ifdef _WIN32_WCE
    DWORD threadId;
    fThreadHandle = (HANDLE)::CreateThread(0,0,W32ThreadProc,this,0,&threadId);
    fThreadId = (CATUInt32)threadId;
#else
    fThreadHandle = (HANDLE)_beginthreadex(0,0,W32ThreadProc,this,0,&fThreadId);
#endif

    // If startup was successful, return true
    if (fThreadHandle != INVALID_HANDLE_VALUE)
    {
        return true;
    }

    // Thread couldn't start - bail.
    fThreadHandle = 0;
    return false;
}

// Start a thread procedure.  You can use this directly w/o deriving
// just by creating your procedure from the CATTHREADPROC prototype.
bool CATThread::StartProc(CATTHREADPROC proc, void* param)
{
    CATASSERT(fThreadHandle == 0, "Starting a thread that's already running. Bad form...");
    if (fThreadHandle != 0)
    {
        return false;
    }

    this->fCallback = proc;
    this->fUserParam = param;

#ifdef _WIN32_WCE
    DWORD threadId;
    fThreadHandle = ::CreateThread(0,0,W32ThreadProc,this,0,&threadId);
    fThreadId = (CATUInt32)threadId;
#else
    fThreadHandle = (HANDLE)_beginthreadex(0,0,W32ThreadProc,this,0,&fThreadId);
#endif
    if (fThreadHandle != INVALID_HANDLE_VALUE)
    {
        return true;
    }

    fThreadHandle = 0;
    return false;
}

// Wait until the thread stops or the timer times out.
// If successful, clears the thread handle. Start or StartProc must
// be called before other thread commands are used.
bool CATThread::WaitStop(CATUInt32 timeout, CATUInt32* exitCode )
{
    if (fThreadHandle == 0)
    {
        return true;
    }

    if (timeout == -1)
    {
        timeout = INFINITE;
    }

    if (WAIT_TIMEOUT == WaitForSingleObject(fThreadHandle,timeout))
    {
        return false;
    }

    if (exitCode)
    {   
        GetExitCodeThread(fThreadHandle,(DWORD*)exitCode);
    }
    
    CloseHandle(fThreadHandle);

    fThreadHandle	= 0;
    fCallback		= 0;
    fUserParam		= 0;
    return true;
}

// Forces a thread to stop - use sparingly.  As WaitStop does, this
// one clears the thread handle.  Start or StartProc must be called
// prior to calling other commands after ForceStop is issued.
void CATThread::ForceStop()
{
    if (fThreadHandle == 0)
    {
        return;
    }

    TerminateThread(fThreadHandle,-1);
    CloseHandle(fThreadHandle);
    fThreadHandle = 0;
    fCallback = 0;
    fUserParam = 0;
}

// Pause the thread. Thread must have been started before use.
bool CATThread::Pause()
{
    CATASSERT(fThreadHandle != 0, "Invalid thread handle in Pause - start it first!");

    if (fThreadHandle == 0)
    {
        return false;
    }

    return (SuspendThread(fThreadHandle) >= 0);
}

// Resume the thread. Thread must have been started before use.
bool CATThread::Resume()
{
    CATASSERT(fThreadHandle != 0, "Invalid thread handle in Resume - start it first!");

    if (fThreadHandle == 0)
    {
        return false;
    }

    return (ResumeThread(fThreadHandle) >= 0);
}

// Thread function - either override this if you are deriving
// from the class, or leave as is and it will call the CATTHREADPROC
// procedure from a StartProc, then exit.
void CATThread::ThreadFunction()
{
    if (this->fCallback)
    {
        fCallback(this->fUserParam,this);
    }
}


// OS specific thread - note, the param is NOT the user param, 
// rather it is a pointer to the thread object....
#ifndef _WIN32_WCE
unsigned int WINAPI W32ThreadProc(void *param)
#else
DWORD WINAPI W32ThreadProc(void *param)
#endif
{
    CATThread* theThread = (CATThread*)param;
    theThread->ThreadFunction();
    return 0;
}
