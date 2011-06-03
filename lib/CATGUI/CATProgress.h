//---------------------------------------------------------------------------
/// \file CATProgress.h
/// \brief Progress bar for GUI
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
#ifndef _CATProgress_H_
#define _CATProgress_H_

#include "CATControl.h"

class CATProgress;

enum CATPROGRESSSTYLE
{
    CATPROGRESS_VERTICAL,
    CATPROGRESS_HORIZONTAL
};

/// \class CATProgress CATProgress.h
/// \brief Progress bar for GUI
/// \ingroup CATGUI
class CATProgress : public CATControl
{
public:
    // CATProgress constructor (inherited from CATControl->CATXMLObject)
    // \param element - Type name ("Progress")
    // \param attribs - attribute information for the window
    // \param parent - parent XML object (should be a "Window" element)
    // \param rootDir - root directory of skin for bin/png loads
    CATProgress( const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATProgress();          

    /// IsFocusable() returns true if the control can receive
    /// focus, and false otherwise.
    virtual bool	     IsFocusable() const;

    /// Load() loads the skin in
    virtual        CATResult Load(CATPROGRESSCB				progressCB   = 0,
        void*							progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);   

    /// Draw() draws the control into the parent's image
    /// \param image - parent's image to draw into
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    /// SetImages() sets the images and resets the control
    virtual void SetImages( CATImage* imageOn, CATImage* imageOff, CATImage* imageDisabled);

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_PROGRESSBAR;}
#endif //_WIN32

protected:
    CATImage*          fImageOn;
    CATPROGRESSSTYLE   fProgressStyle;
};

#endif // _CATProgress_H_


