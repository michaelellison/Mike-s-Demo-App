//---------------------------------------------------------------------------
/// \file CATSwitchMulti.h
/// \brief On/Off switch for GUI
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
//---------------------------------------------------------------------------
#ifndef _CATSwitchMulti_H_
#define _CATSwitchMulti_H_

#include "CATControl.h"

class CATSwitchMulti;

/// \class CATSwitchMulti
/// \brief On/Off switch for GUI
/// \ingroup CATGUI
class CATSwitchMulti : public CATControl
{
public:
    /// CATSwitchMulti constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Switch")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png loads
    CATSwitchMulti(   const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATSwitchMulti();          

    virtual void OnMouseClick();

    /// Draw() draws the control into the parent's image
    /// \param image - parent image to draw into   
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    /// Load() loads the skin in
    virtual        CATResult Load(	 CATPROGRESSCB				progressCB   = 0,
        void*						progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);   

    virtual  CATString GetHint() const;

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_PUSHBUTTON;}
#endif //_WIN32

protected:
    bool					fAutoIncrement;
    CATUInt32				fNumImages;      

    std::vector<CATImage*>	fImageList;
    std::vector<CATImage*>	fImageDisabledList;
    std::vector<CATImage*>	fImagePressedList;
    std::vector<CATImage*>	fImageFocusList;
    std::vector<CATImage*>  fImageActiveList;
    CATStack<CATImage*>	fMasterSet;
};

#endif // _CATSwitchMulti_H_


