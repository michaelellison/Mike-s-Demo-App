/// \file CATSignal.h
/// \brief Defines an interface for signals (win32 events) for thread syncs
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
//---------------------------------------------------------------------------
#ifndef _CATSignal_H_
#define _CATSignal_H_

#include "CATInternal.h"

/// \class CATSignal
/// \brief Defines an interface for signals (win32 events) for thread syncs
/// \ingroup CAT
/// 
/// CATSignal provides a signal or event interface for synchronization
/// between threads.  The signal acts like a gate - when fired, it lets
/// someone through.  If set to auto-reset, then it only lets the next
/// caller requesting it through. Otherwise, it will allow callers through
/// until it is reset.
class CATSignal
{
   public:
      CATSignal(bool autoReset = true);
      virtual ~CATSignal();

      /// Wait() waits up to [milliseconds] milliseconds for the
      /// signal to be fired. If the CATSignal is set to auto-reset,
      /// the it will be reset when a caller successfully receive
      /// the event through a wait.
      ///
      /// Check the result code here! It can very easily time out.
      ///
      /// The default, however, is an infinite wait.
      ///
      /// \param milliseconds - milliseconds to wait while trying to get synch
      ///
      /// \sa Release()
      CATResult Wait(CATUInt32 milliseconds = 0xFFFFFFFF);
      
      /// Fire() fires the signal, which then allows caller(s) through.
      CATResult Fire();

      /// Reset() resets the signal, making the signal block callers.
      CATResult Reset();

#ifdef CAT_CONFIG_WIN32
      HANDLE   GetWin32Handle();
#endif

   private:
      CATSignal& operator=(const CATSignal& srcSignal)
      {
         CATASSERT(false,"Copying signals is not currently supported.");
         return *this;
      }

      // Platform specific mutex handles
#ifdef CAT_CONFIG_WIN32
      HANDLE fEvent;
#endif 
};


#endif // _CATSignal_H_


