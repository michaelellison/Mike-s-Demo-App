//---------------------------------------------------------------------------
/// \file    CATThread.h
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

#ifndef CATThread_H
#define CATThread_H
#include "CATInternal.h"

/// \class   CATThread
/// \brief   Base Thread Class
/// \ingroup CAT
///
/// CATThread is your basic thread class.  You can derive classes from it or
/// use as-is with a user-defined CATTHREADPROC
class CATThread
{
	public:
		typedef  void (*CATTHREADPROC)(void *param, CATThread* theThread);

		/// Thread construction
		CATThread();

		/// Thread destruction
		virtual ~CATThread();
		
		/// Start a thread. This is the one used if you're deriving from
		/// CATThread for your own threaded class.  Override ThreadFunction()
		/// for your subclass...
		virtual	 bool Start(void *param);

		/// Start a thread procedure.  You can use this directly w/o deriving
		/// just by creating your procedure from the CATTHREADPROC prototype.
		virtual  bool StartProc(CATTHREADPROC proc, void* param);
		
		/// Wait until the thread stops or the timer times out.
		/// If successful, clears the thread handle. Start or StartProc must
		/// be called before other thread commands are used.
		virtual	 bool WaitStop(CATUInt32  timeout  = (CATUInt32)-1,
                               CATUInt32* exitCode = 0);

		/// Forces a thread to stop - use sparingly.  As WaitStop does, this
		/// one clears the thread handle.  Start or StartProc must be called
		/// prior to calling other commands after ForceStop is issued.
		virtual	 void ForceStop();

		/// Pause the thread. Thread must have been started before use.
		virtual	 bool Pause();

		/// Resume the thread. Thread must have been started before use.
		virtual  bool Resume();
		
		/// Thread function - either override this if you are deriving
		/// from the class, or leave as is and it will call the CATTHREADPROC
		/// procedure from a StartProc, then exit.
		virtual	void ThreadFunction();

	protected:
        CATUInt32       fThreadId;          ///< thread id
		HANDLE			fThreadHandle;		///< Thread handle
		CATTHREADPROC	fCallback;          ///< Callback
		void*			fUserParam;			///< User parameter for thread
};

#endif //CATThread_H
