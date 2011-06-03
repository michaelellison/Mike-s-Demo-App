//---------------------------------------------------------------------------
/// \file CATRadioButton.h
/// \brief Radio button (grouped)
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
//---------------------------------------------------------------------------
#ifndef _CATRadioButton_H_
#define _CATRadioButton_H_

#include "CATSwitch.h"

const int kGGMAXRADIOBUTTONS = 256;

class CATRadioButton;

/// \class CATRadioButton CATRadioButton.h
/// \brief Radio button (grouped)
/// \ingroup CATGUI
class CATRadioButton : public CATSwitch
{
public:
    CATRadioButton(   const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATRadioButton();          

    virtual CATResult Load(	CATPROGRESSCB				progressCB   = 0,
        void*							progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);   

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    virtual void OnMouseClick();
#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_RADIOBUTTON;}
#endif //_WIN32

protected:
    CATFloat32  fRadioValue;
};

#endif // _CATRadioButton_H_


