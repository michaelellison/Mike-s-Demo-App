/// \file    CATCritSec.h
/// \brief Defines the interface to critical sections for thread synchronization.
/// \ingroup CAT
///
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATCritSec_H_
#define _CATCritSec_H_

#include "CATInternal.h"

/// \class CATCritSec
/// \brief Defines the interface to critical sections for thread synchronization.
/// \ingroup CAT
/// 
/// CATCritSec provides a per-thread synchronization object.  While one thread
/// owns a critical section, another may not receive it and will have to wait until
/// it is released.  A single thread, however, will not be forced to wait
/// if it tries to acquire the critical section twice before releasing it - just make
/// sure to call Release() for each Wait().
class CATCritSec
{
   public:
      CATCritSec();
      virtual ~CATCritSec();

      /// Wait until the critical section is available.
      ///
      /// \sa Release()
      void Wait();

      /// Release() releases an acquired critical section.
      ///
      /// \sa Wait()
      void Release();

   private:
      CATCritSec& operator=(const CATCritSec&)
      {
         CATASSERT(false,"Copying critical sections is not supported.");
         return *this;
      }

      // Platform specific critical section handles
      CRITICAL_SECTION fCritSec;
};


#endif // _CATCritSec_H_


