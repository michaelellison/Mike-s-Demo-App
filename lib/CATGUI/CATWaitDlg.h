/// \file   CATWaitDlg.h
/// \brief  Wait dialog
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//

#ifndef _CATWaitDlg_H_
#define _CATWaitDlg_H_

#include "CATGUIInternal.h"
#include "CATWindow.h"
#include "CATSignal.h"
#include "CATProgress.h"
#include "CATMutex.h"

/// \class CATWaitDlg CATWaitDlg.h
/// \brief Wait dialog
/// \class CATWaitDlg
class CATWaitDlg : private CATWindow
{
public:
    CATWaitDlg(	CATINSTANCE	instance,
        CATInt32		backgroundBmpId,
        CATRect&		textRect,
        CATInt32		progressBmpOnId  = 0,	// optional progress bar
        CATInt32		progressBmpOffId = 0,
        CATInt32		progressLeft	  = 0,
        CATInt32		progressTop		  = 0);

    virtual ~CATWaitDlg();

    virtual void StartWait(	const CATString& waitMsg,										
                            CATWindow*       parent   = 0,
                            bool		     disableParent = false);		

    virtual void		EndWait(bool force = false);
    virtual bool        OnClose();
    virtual void		OnDestroy();
    virtual void		SetProgress( CATFloat32 percent );

    virtual void		SetWaitLabelText( const CATString& waitStr );
    virtual CATResult   OnEvent  (        const CATEvent&  event, 
                                          CATInt32&        retVal);

    CATUInt32	GetDepth();
    bool		IsShowing();

    /// Draw() is called when the window should paint itself. 
    ///
    /// \param background - ptr to image to draw into
    /// \param dirtyRect - part of window to redraw.
    virtual void      Draw        (  CATImage*        background,
                                     const CATRect&   dirtyRect);

protected:
    static	void WindowThread(void* param, CATThread* thread);
    CATThread		fThread;
    CATInt32	    fBmpId;
    CATUInt32	    fDepth;
    CATLabel*		fWaitLabel;
    CATInt32	    fProgBmpOnId;
    CATInt32	    fProgBmpOffId;
    CATProgress*    fProgressBar;
    CATMutex	    fUpdateLock;
    CATSignal	    fDlgReady;
    CATWindow*	    fParent;
    bool			fDisableParent;
    bool			fAllowClose;
};


#endif // _CATWaitDlg_H_