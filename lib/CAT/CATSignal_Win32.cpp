/// \file CATSignal_Win32.cpp
/// \brief Win32 implementation of signal events
/// \ingroup CAT
/// 
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATSignal.h"

CATSignal::CATSignal(bool autoReset)
{
   fEvent = ::CreateEvent(0,autoReset?FALSE:TRUE,FALSE,0);
   if (fEvent == INVALID_HANDLE_VALUE)
   {
      CATASSERT(false,"Could not create event!");
      fEvent = 0;
   }
}

CATSignal::~CATSignal()
{
   if (fEvent)
   {
      CloseHandle(fEvent);
      fEvent = 0;
   }
}

// Wait() waits up to [milliseconds] milliseconds for the
// signal to be fired. If the CATSignal is set to auto-reset,
// the it will be reset when a caller successfully receive
// the event through a wait.
//
// Check the result code here! It can very easily time out.
//
// The default, however, is an infinite wait.
//
// \param milliseconds - milliseconds to wait while trying to get synch
//
// \sa Release()
CATResult CATSignal::Wait(CATUInt32 milliseconds)
{
   if (!fEvent)
   {
      return CATRESULT(CAT_ERR_SIGNAL_INVALID_HANDLE);
   }

   if (milliseconds == 0xFFFFFFFF)
   {
      milliseconds = INFINITE;
   }
   DWORD waitRes = ::WaitForSingleObject(fEvent,milliseconds);   

   switch (waitRes)
   {
      case WAIT_OBJECT_0:
         return CATRESULT(CAT_SUCCESS);
      case WAIT_TIMEOUT:
         return CATRESULT(CAT_ERR_SIGNAL_TIMEOUT);
      default:
         // Error occurred. Handle was invalid or something similar.
         return CATRESULT(CAT_ERR_SIGNAL_WAIT_ERROR);
   }   
}

      
// Fire() fires the signal, which then allows caller(s) through.
CATResult CATSignal::Fire()
{
   if (!fEvent)
   {
      return CATRESULT(CAT_ERR_SIGNAL_INVALID_HANDLE);
   }
   ::SetEvent(fEvent);
   
   return CAT_SUCCESS;
}

/// Reset() resets the signal, making the signal block callers.
CATResult CATSignal::Reset()
{
   if (!fEvent)
   {
      return CATRESULT(CAT_ERR_SIGNAL_INVALID_HANDLE);
   }
   ::ResetEvent(fEvent);

   return CAT_SUCCESS;
}

HANDLE CATSignal::GetWin32Handle()
{
   return fEvent;
}


