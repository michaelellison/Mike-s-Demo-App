//---------------------------------------------------------------------------
/// \file    CATMsgThread.h
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

#ifndef CATMsgThread_H_
#define CATMsgThread_H_

#include "CATThread.h"
#include "CATSignal.h"

/// \class   CATMsgThread
/// \brief   Message Thread Base Class
/// \ingroup CAT
///
/// CATMsgThread is the class to inherit from when you want a thread
/// that responds to messages and has a periodic loop.
class CATMsgThread : protected CATThread
{
	public:
		/// Thread construction
		CATMsgThread();

		/// Thread destruction
		virtual ~CATMsgThread();
		
		/// Start a thread. 
		virtual	 bool Start( CATUInt32 pollFreq);

        /// Stop a thread, cleanly if possible.
        virtual  void Stop ( CATUInt32 timeout = (CATUInt32)-1);

        /// Called once per timeout
        virtual  void OnThreadIdle();

        /// Called when messages are received
        virtual  CATUInt32 OnThreadMessage(CATUInt32 msg, void* wParam, void* lParam);

        /// Posts a message to the thread
        bool     Post(CATUInt32 msg, void* wParam, void* lParam);
	
    protected:
		virtual	void ThreadFunction();
        CATUInt32    fPollFreq;
        CATSignal    fExitSignal;
};

#endif //CATMsgThread_H_
