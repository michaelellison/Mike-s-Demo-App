//---------------------------------------------------------------------------
/// \file CATKnob.h
/// \brief Knob control for GUI
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATKnob_H_
#define CATKnob_H_

#include "CATControl.h"

class CATKnob;
class CATFilterEWMA;
class CATEditBox;

enum CATKNOB_STYLE
{
    CATKNOB_VERTICAL,
    CATKNOB_CIRCLE
};

/// \class CATKnob
/// \brief Knob control for GUI
/// \ingroup CATGUI
class CATKnob : public CATControl
{
public:
    /// CATKnob constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Button")
    /// \param attribs - attribute information for the window
    /// \param rootDir - root directory of skin
    /// \param parent - parent XML object (should be a "Window" element)
    CATKnob(     const CATString&               element, 
        const CATString&               rootDir);

    virtual ~CATKnob();          

    /// RectFromAttribs() recalculates the control's rectangle from
    /// the attributes.  This can only be called after ParseAttributes() has
    /// loaded the images.
    virtual CATResult    RectFromAttribs();

    // Tracked mouse events - only called when the control is initially pressed
    virtual void   TrackMouseMove(	const CATPOINT& point, 
        bool				leftButton, 
        CATMODKEY			modKey);

    virtual void   TrackMouseTimer(	CATMODKEY			modKey);

    virtual void   TrackMouseDown(	const CATPOINT& point, 
        CATMODKEY			modKey);      

    virtual void   TrackMouseRelease(const CATPOINT& point,
        CATMODKEY			modKey);      

    virtual void   OnMouseWheel(		const CATPOINT& point,
        CATFloat32        wheelMove,
        CATMODKEY			modKey);

    virtual void   OnKeyDown(	const CATKeystroke&	keystroke);
    virtual void   OnKeyPress(	const CATKeystroke&	keystroke);

    virtual void   Draw(			CATImage*					image, 
        const CATRect&			dirtyRect);      

    /// PostDraw() draws any stuff that requires an OS-specific draw
    /// context.
    virtual void   PostDraw(	CATDRAWCONTEXT			drawContext, 
        const CATRect&			dirtyRect);

    /// Knob styles
    virtual CATKNOB_STYLE GetKnobStyle();
    virtual void   SetKnobStyle(CATKNOB_STYLE style);

    virtual CATString GetHint() const;


    /// OnMouseDoubleClick() is called the mouse is double clicked.
    ///
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseDoubleClick( CATMODKEY modKey);

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_SLIDER;}
#endif

protected:      
    CATKNOB_STYLE GetActiveStyle(		CATMODKEY			modKey );

    virtual void SetValueFromPoint(	const CATPOINT& point, 
        CATMODKEY			modKey);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();


    bool           fCommandTrack;
    bool				fCursorHidden;

    CATKNOB_STYLE   fKnobStyle;   
    CATKNOB_STYLE   fLastKnobStyle;   
    CATFilterEWMA*  fFilter;
    CATInt32         fLastKnob;
    CATPOINT        fStartPoint;
    CATPOINT        fLastPoint;
    CATPOINT        fLastMove;
    clock_t        fLastTime;
    CATFloat32        fFilterCoef;
    CATFloat32        fMinDegree;
    CATFloat32        fMaxDegree;

    CATVALUE_TYPE	fValueType;
};

#endif // CATKnob_H_


