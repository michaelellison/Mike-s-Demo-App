/// \file    CATCritSec_Win32.cpp
/// \brief   Win32 version of critical sections for thread synchronization.
/// \ingroup CAT
///
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATCritSec.h"

CATCritSec::CATCritSec()
{
#ifndef _WIN32_WCE
    ::InitializeCriticalSectionAndSpinCount(&fCritSec,4000);
#else
    ::InitializeCriticalSection(&fCritSec);
#endif
}

CATCritSec::~CATCritSec()
{
    DeleteCriticalSection(&fCritSec);
}

void CATCritSec::Wait()
{
    EnterCriticalSection(&fCritSec);
}

void CATCritSec::Release()
{
    LeaveCriticalSection(&fCritSec);
}
