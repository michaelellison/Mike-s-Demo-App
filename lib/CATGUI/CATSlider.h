//---------------------------------------------------------------------------
/// \file CATSlider.h
/// \brief Slider control for GUI
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
#ifndef _CATSlider_H_
#define _CATSlider_H_

#include "CATControl.h"

class CATSlider;
class CATFilter;

enum CATSLIDER_STYLE
{
    CATSLIDER_VERTICAL,
    CATSLIDER_HORIZONTAL
};

/// \class CATSlider CATSlider.h
/// \brief Slider control for GUI
/// \ingroup CATGUI
class CATSlider : public CATControl
{
public:
    /// CATSlider constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Button")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin
    CATSlider(     const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATSlider();          
    // Tracked mouse events - only called when the control is initially pressed
    virtual void   TrackMouseMove(const CATPOINT& point, bool leftButton, CATMODKEY modKey);
    virtual void   TrackMouseDown(const CATPOINT& point, CATMODKEY modKey);      
    virtual void   TrackMouseRelease(const CATPOINT& point, CATMODKEY modKey);      

    // Mouse wheel over control, but not already tracked

    virtual void         OnMouseWheel(  const CATPOINT& point,
        CATFloat32        wheelMove,
        CATMODKEY			modKey);

    virtual void   OnKeyDown(const CATKeystroke& keystroke);
    virtual void   OnKeyPress(const CATKeystroke& keystroke);
    virtual void   Draw(CATImage* image, const CATRect& dirtyRect);      

    /// Load() loads the skin in
    virtual        CATResult Load(	CATPROGRESSCB				progressCB   = 0,
        void*							progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);

    /// Slider styles
    virtual CATSLIDER_STYLE  GetSliderStyle();
    virtual void            SetSliderStyle(CATSLIDER_STYLE style);

    virtual CATString GetHint() const;

    /// OnMouseDoubleClick() is called the mouse is double clicked.
    ///
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseDoubleClick(  CATMODKEY modKey);

    virtual void   TrackMouseTimer(CATMODKEY modKey);

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_SLIDER;}
#endif //_WIN32

protected:      
    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    CATSLIDER_STYLE fSliderStyle;
    bool           fCommandTrack;
    CATImage*       fImageSlide;
    CATImage*       fImageSlideDisabled;
    CATImage*       fImageSlideFocus;
    CATImage*       fImageOn;
    CATInt32         fSlideOffset;
    CATVALUE_TYPE	fValueType;
    CATFloat32			fLastTarget;
};

#endif // _CATSlider_H_


