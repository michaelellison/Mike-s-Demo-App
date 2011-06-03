/// \file    CATMutex.h
/// \brief Defines the interface to mutexes for thread synchronization.
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATMutex_H_
#define _CATMutex_H_

#include "CATInternal.h"

/// \class CATMutex
/// \brief Defines the interface to mutexes for thread synchronization.
/// \ingroup CAT
/// 
/// CATMutex provides a per-thread synchronization object.  While one thread
/// owns a mutex, another may not receive it and will have to wait until
/// it is released.  A single thread, however, will not be forced to wait
/// if it tries to acquire the mutex twice before releasing it - just make
/// sure to call Release() for each Wait().
class CATMutex
{
   public:
      CATMutex();
      virtual ~CATMutex();

      /// Wait() waits up to [milliseconds] milliseconds to obtain the
      /// mutex.
      ///
      /// Check the result code here! It can very easily time out.
      ///
      /// The default, however, is an infinite wait.
      ///
      /// You must call Release() when done.
      ///
      /// Remember that CATMutex only blocks per-thread.  A single thread
      /// will not deadlock if it waits on the mutex twice in a row. However,
      /// it should also release it twice in a row if it does so.
      /// 
      /// \param milliseconds - milliseconds to wait while trying to get synch
      ///
      /// \sa Release()
      CATResult Wait(CATUInt32 milliseconds = 0xFFFFFFFF);

      /// Release() releases an acquired mutex.
      ///
      /// \sa Wait()
      CATResult Release();

   private:
      CATMutex& operator=(const CATMutex& srcMutex)
      {
         CATASSERT(false,"Copying mutexes is not supported.");
         return *this;
      }

      // Platform specific mutex handles
      HANDLE fMutexHandle;
};


#endif // _CATMutex_H_


