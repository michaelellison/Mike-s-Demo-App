/// \file CATEvent.h
/// \brief Evironmental event encapsulation
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATEvent_H_
#define _CATEvent_H_

typedef CATUInt32 CATEventCode;

#include "CATInternal.h"

/// \class CATEvent
/// \brief Evironmental event encapsulation
/// \ingroup CATGUI
///
/// Events are occurances from the external environment targetted to a specific
/// window. They should be acted on immediately if possible, and are encountered
/// and parsed on the primary GUI thread.
///
/// Events are identified by a 32-bit enumerated code (CATEventCode).  These
/// codes are stored in CATEventDefs.h, along with a description of what the
/// parameters mean.
///
/// The main use of events is to take operating system / environmental messages
/// and coax them into a single uniform type that can be dealt with by one
/// set of code for all platforms.
///
/// \sa CATEventDefs, CATApp, CATWindow, CATCommand
class CATEvent
{
public:      
    CATEvent( CATEventCode     eventCode,
        CATInt64          iParam1       = 0,
        CATInt64          iParam2       = 0,
        CATInt64          iParam3       = 0,
        CATInt64          iParam4       = 0,
        CATFloat32        fParam1       = 0.0f,
        const CATString   sParam1       = "",
        const CATString   sParam2       = "",
        const CATString   sParam3       = "",
        void*             vParam        = 0)
    {
        fEventCode     = eventCode;
        fIntParam1     = iParam1;
        fIntParam2     = iParam2;
        fIntParam3     = iParam3;
        fIntParam4     = iParam4;
        fFloatParam1   = fParam1;         
        fStringParam1  = sParam1;
        fStringParam2  = sParam2;
        fStringParam3  = sParam3;
        fVoidParam     = vParam;
    }


    virtual ~CATEvent()
    {
    } 

    CATEvent& operator=(const CATEvent& event)
    {
        fEventCode     = event.fEventCode;
        fIntParam1     = event.fIntParam1;
        fIntParam2     = event.fIntParam2;
        fIntParam3     = event.fIntParam3;
        fIntParam4     = event.fIntParam4;
        fFloatParam1   = event.fFloatParam1;
        fStringParam1  = event.fStringParam1;
        fStringParam2  = event.fStringParam2;
        fStringParam3  = event.fStringParam3;
        fVoidParam     = event.fVoidParam;
        return *this;
    }

    // Event data....
    CATEventCode    fEventCode;
    CATInt64         fIntParam1;
    CATInt64         fIntParam2;
    CATInt64         fIntParam3;
    CATInt64         fIntParam4;
    CATFloat32        fFloatParam1;
    CATString       fStringParam1;
    CATString       fStringParam2;
    CATString       fStringParam3;
    void*          fVoidParam;
};


#endif // _CATEvent_H_


