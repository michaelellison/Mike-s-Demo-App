/// \file    CATMutex_Win32.cpp
/// \brief Win32 implementation of CATMutex
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATMutex.h"
#include "CATUtil.h"

//---------------------------------------------------------------------------
CATMutex::CATMutex()
{
   fMutexHandle = CreateMutex(0,FALSE,0);
   CATASSERT(fMutexHandle != INVALID_HANDLE_VALUE, "Unable to create mutex.");
   
   if (fMutexHandle == INVALID_HANDLE_VALUE)
      fMutexHandle = 0;
}

//---------------------------------------------------------------------------
CATMutex::~CATMutex()
{
   if (fMutexHandle)
   {
      CloseHandle(fMutexHandle);
      fMutexHandle = 0;
   }
}

//---------------------------------------------------------------------------
CATResult CATMutex::Wait(CATUInt32 milliseconds)
{
   if (fMutexHandle == 0)
   {
      return CATRESULT(CAT_ERR_MUTEX_INVALID_HANDLE);
   }

   if (milliseconds == 0xFFFFFFFF)
   {
      milliseconds = INFINITE;
   }
   
   DWORD result = WaitForSingleObject(fMutexHandle,milliseconds);

   switch (result)
   {
      case WAIT_OBJECT_0:
         return CATRESULT(CAT_SUCCESS);
      case WAIT_TIMEOUT:
         return CATRESULT(CAT_ERR_MUTEX_TIMEOUT);
      default:
         // Error occurred. Handle was invalid or something similar.
         return CATRESULT(CAT_ERR_MUTEX_WAIT_ERROR);
   }
}

//---------------------------------------------------------------------------
CATResult CATMutex::Release()
{
   if (fMutexHandle == 0)
   {
      return CATRESULT(CAT_ERR_MUTEX_INVALID_HANDLE);
   }

   ::ReleaseMutex(fMutexHandle);
   return CATRESULT(CAT_SUCCESS);
}

