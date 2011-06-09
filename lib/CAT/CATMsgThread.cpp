//---------------------------------------------------------------------------
/// \file    CATMsgThread.cpp
/// \brief   Message Thread Base Class
/// \ingroup CAT
///
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------

#include "CATMsgThread.h"

// Thread construction
CATMsgThread::CATMsgThread()
:CATThread()
{
    fPollFreq = 0;
}

// Thread destruction
CATMsgThread::~CATMsgThread()
{
    Stop(1000);
}

// Start a thread. 
bool CATMsgThread::Start( CATUInt32 pollFreq )
{
    fPollFreq = pollFreq;
    fExitSignal.Reset();
    return CATThread::Start(this);    
}

// Stop a thread, cleanly if possible.
void CATMsgThread::Stop ( CATUInt32 timeout )
{
    fExitSignal.Fire();
    if (false == WaitStop(timeout))
    {
        ForceStop();
    }
}

// Called once per timeout
void CATMsgThread::OnThreadIdle()
{
    CATTRACE("On thread idle...");
}


// Called when messages are received
CATUInt32 CATMsgThread::OnThreadMessage(CATUInt32 msg, void* wParam, void* lParam)
{    
    CATTRACE("On thread msg...");
    return 0;
}

// Posts a message to the thread
bool CATMsgThread::Post(CATUInt32 msg, void* wParam, void* lParam)
{
    if (::PostThreadMessage(fThreadId,msg,(WPARAM)wParam,(LPARAM)lParam))
        return true;

    return false;
}

void CATMsgThread::ThreadFunction()
{
    HANDLE eventHandles[2];
    eventHandles[0] = fExitSignal.GetWin32Handle();    
    while (MsgWaitForMultipleObjects(1, &eventHandles[0],FALSE, this->fPollFreq,QS_ALLEVENTS) != WAIT_OBJECT_0)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        { 
            // If it is a quit message, exit.
            if (msg.message == WM_QUIT)  
            {
                fExitSignal.Fire();
                break;
            }
            else
            {
                CATUInt32 result = OnThreadMessage(msg.message, (void*)msg.lParam, (void*)msg.wParam);
            }
        }

        OnThreadIdle();
    }
}

