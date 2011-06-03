//---------------------------------------------------------------------------
/// \file CATControl.h
/// \brief Basic Control class
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
#ifndef CATControl_H_
#define CATControl_H_

#include "CATColor.h"
#include "CATCursor.h"
#include "CATKeystroke.h"
#include "CATWidget.h"

class CATWindow;

/// \class CATControl CATControl.h
/// \brief Basic Control class
/// \ingroup CATGUI
class CATControl : public CATWidget
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATControl(  const CATString&               element, 
                 const CATString&               rootDir);
    
    virtual ~CATControl();          

    /// GetWindow() retrieves the parent window.
    virtual CATWindow*  GetWindow() const;      

    /// GetValue() retrieves the value of the control.
    /// \return CATFloat32 - control value (0.0 - 1.0)
    virtual CATFloat32  GetValue() const;

    /// These by default try to convert to float. String-type controls override them.
    virtual void        SetString ( const CATString& text );
    virtual CATString   GetString () const;

    /// SetValue() sets the value of the control 
    /// \param newValue - float from 0 to 1 for new value.
    /// \param sendCommand - if true, will send a command with the new value.
    /// This is useful when you want to set a value w/o sending a command,
    /// for example when updating a control from another control's state.
    virtual void        SetValue(CATFloat32 newValue, bool sendCommand = true);

    /// ResetValue() resets the control to its default value
    virtual void        ResetValue();

    /// GetCommand() returns the command for the control.
    /// \return CATCommand - command from control
    virtual CATCommand  GetCommand() const;

    /// GetColorFore() retrieves the foreground color for the control
    /// \return CATColor - foreground color
    virtual CATColor    GetColorFore() const;

    /// GetColorBack() retrieves the background color for the control
    /// If control is disabled, returns the disabled color right now...
    /// \return CATColor - background color
    virtual CATColor    GetColorBack() const;

    /// SetColorFore() sets the foreground color for the control
    /// If control is disabled, returns the disabled color right now...
    /// \param color - new foreground color
    virtual void        SetColorFore(const CATColor& color);

    // As above, for disabled state
    virtual void        SetColorForeDisabled( const CATColor& color);


    /// SetColorBack() sets the backgroundcolor for the control
    /// This is rarely used...
    ///
    /// \param color - color to set the background to.
    virtual void        SetColorBack(const CATColor& color);
    // As above, for disabled state
    virtual void        SetColorBackDisabled( const CATColor& color);

    /// GetCursor() retrieves the object's mouse cursor.
    virtual CATCursor*  GetCursor();

    /// IsFocusable() returns true if the control can receive
    /// focus, and false otherwise.
    virtual bool	    IsFocusable() const;

    /// SetFocused() sets the control's focused state.
    ///
    /// \param focused - if true, then the control is given focus.
    ///                  if false, focus is removed.      
    virtual void        SetFocused(bool focused);

    /// SetActive() sets the control's active state.
    ///
    /// \param active - if true, then the control is under the mouse
    ///                  if false, active flag is removed
    virtual void        SetActive(bool active);

    /// IsFocused() returns true if the control has the current focus.
    virtual bool	    IsFocused() const;
    /// IsPressed() returns true ifthe control is currently pressed.
    virtual bool        IsPressed() const;
    // IsActive() returns true if the mouse is over it
    virtual bool        IsActive()  const;

    /// Draw() draws the control into the parent's image
    /// \param image - parent image to draw into
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    /// Event handler
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

    /// PostDraw() draws any stuff that requires an OS-specific draw
    /// context.
    virtual void   PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect);

    /// Load() loads the skin in
    virtual        CATResult Load(	CATPROGRESSCB	progressCB    = 0,
                                    void*			progressParam = 0,
                                    CATFloat32		progMin       = 0.0f,
                                    CATFloat32		progMax		  = 1.0f);

    /// OnMouseClick() is called if the mouse was pressed, then released within
    /// a control.
    virtual void   OnMouseClick();

    /// OnRightMouseClick is called if the right button was pressed, then released
    /// within a control.
    virtual void   OnRightMouseClick();

    /// OnMouseDoubleClick() is called the mouse is double clicked.
    ///
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseDoubleClick(  CATMODKEY			modKey);



    /// Tracked mouse movements - only called when the mouse was initially
    /// pressed over this control.
    ///
    /// \param point - location of the mouse
    /// \param leftButton - true if left button is pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void   TrackMouseMove(  const CATPOINT& point, 
                                    bool            leftButton, 
                                    CATMODKEY		modKey);

    /// TrackMouseTimer() is called when no movement has been detected on
    /// the mouse to update filtered controls towards their resting point.
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void   TrackMouseTimer(CATMODKEY modKey);

    /// TrackMouseWheel() is called if the mouse wheel is used while
    /// a button is down.
    ///
    /// The mouse wheel movement is set to where each 'notch' should be about
    /// 1.0 coming in from the OS.  Positive values are 'up' or away from the
    /// user, negative values are down or towards the user.
    ///
    /// \param point - location of the mouse pointer when the wheel is moved
    /// \param wheelMove - distance moved
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void   TrackMouseWheel(  const CATPOINT& point,
                                     CATFloat32      wheelMove,
                                     CATMODKEY	  	 modKey);

    /// TrackMouseDown() is called when a button is pressed inside the control.
    /// 
    /// Generally, this should toggle the control into a pressed state, but not
    /// register a click or anything yet until TrackMouseRelease() is released
    /// within the same control.
    ///
    /// After TrackMouseDown() is called, the control will begin receiving 
    /// TrackMouseMove() calls if the mouse pointer is moved.
    ///
    /// \param point - location the mouse was pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void   TrackMouseDown(  const CATPOINT& point, 
                                    CATMODKEY       modKey);      


    /// TrackMouseRelease() is called when a button is released 
    /// 
    /// IF the release is within the control, then it should generally register
    /// a mouse click by calling OnMouseClicked().  
    ///
    /// The control should also set its state to unpressed, but active if the mouse is 
    /// within the control (these are the default actions)
    ///
    /// \param point - location the mouse was pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void   TrackMouseRelease(const CATPOINT& point,
        CATMODKEY			modKey);      

    /// OnMouseWheel() is called when the mouse wheel is moved over control, 
    /// but the mouse is not already tracked - this is the usual case.
    ///
    /// The mouse wheel movement is set to where each 'notch' should be about
    /// 1.0 coming in from the OS.  Positive values are 'up' or away from the
    /// user, negative values are down or towards the user.
    ///
    /// \param point - location of the mouse pointer when the wheel is moved
    /// \param wheelMove - distance moved
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseWheel(  const CATPOINT&   point,
                                        CATFloat32        wheelMove,
                                        CATMODKEY		  modKey);

    /// OnKeyDown() is called when a key is depressed and the control has
    /// focus.
    ///
    /// The key may be a standard key, or it may be one of the special
    /// keys - see CATKeystroke.h for details.
    ///
    /// For most keys, you'll want to trap them here - that way you will
    /// receive repeats and such.  However, if you want just full key
    /// press/releases, use OnKeyPress() instead.
    ///
    /// \param keystroke - the key that was pressed
    /// \sa OnKeyUp(), OnKeyPress()
    virtual void   OnKeyDown(const CATKeystroke& keystroke);

    /// OnKeyUp() is called when a key is released and the control has
    /// focus.
    ///
    /// The key may be a standard key, or it may be one of the special
    /// keys - see CATKeystroke.h for details.
    ///
    /// For most keys, you'll want to trap them in OnKeyDown() instead.
    /// However, for certain keys such as SPACE that may be mapped to
    /// work like mouse buttons and the like, this is synonymous to
    /// the TrackMouseRelease() function.
    ///
    /// \param keystroke - the key that was pressed
    /// \sa OnKeyDown(), OnKeyPress()
    virtual void   OnKeyUp(const CATKeystroke& keystroke);

    /// OnKeyPress() is called when a key is pressed, then released and
    /// the control has focus.
    ///
    /// The key may be a standard key, or it may be one of the special
    /// keys - see CATKeystroke.h for details.
    ///
    /// Use this functoin to receive keystrokes where you only want
    /// to respond to full key press/releases.  You will not receive
    /// repeated calls if the key is held 
    /// 
    /// \param keystroke - the key that was pressed
    virtual void   OnKeyPress(const CATKeystroke& keystroke);

    /// OnParentCreate() is called when the parent window is created.
    /// 
    /// Most controls won't need this, but any that create their own
    /// windows should do so at this point.
    virtual void   OnParentCreate();

    /// OnParentDestroy() is called as the parent window is destroyed.
    ///
    /// Controls that create their own windows during OnParentCreate()
    /// should destroy them during OnParentDestroy()
    virtual void   OnParentDestroy();

    // Retrieve font info... need a set, but not now.
    virtual void   GetFontInfo(CATString& fontName, CATFloat32& fontSize);

    virtual CATControl* HitTest(const CATPOINT& point);

    virtual bool ForEachControl(CATCONTROLFUNCB callback, void* userParam);

    /// Platform-specific call to filter a string prior to it being used
    /// in the GUI.  For Windows, it doubles the '&' symbol to prevent
    /// unwanted menu/label mnemonics.
    static  CATString FilterGUIString( const CATString& unfiltered);

    virtual CATUInt32 GetAccessState();

protected:      
    virtual bool BoundsCheckValue();

    // Retrieve the value in terms of percentage from min to max
    virtual CATFloat32 GetValPercent();
    virtual CATFloat32 GetValRange();
    virtual CATFloat32 GetMinValue();
    virtual CATFloat32 GetMaxValue();
    virtual void       SetMinMax  ( CATFloat32 minValue, 
                                    CATFloat32 maxValue);

    /// ResetCursorToDefault() restores the control's cursor to its
    /// configured default.
    ///
    /// This is mainly used on controls like knobs that have special
    /// cursor control while they are being manipulated.
    virtual void   ResetCursorToDefault();

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();


    /// CheckImageSize() performs a sanity check on the image vs. the base
    /// image of the control.
    ///
    /// Many images in the framework are required to be the same size. For
    /// instance, the enabled and disabled images of any control should have
    /// the same dimensions - if they don't, then the drawing will be
    /// corrupted (or rather, it would, but CheckImageSize() works around
    /// that).
    ///
    /// By default, image is compared against the class' fImage. If the image
    /// sizes do not match, then image is destroyed and recreated in a size
    /// that will match - any extra space is filled with red.
    ///
    /// If baseImage is provided, then it is used as the point of reference
    /// instead of fImage.
    ///
    /// \param image - Image to check size of. If the size is not correct,
    /// it will be destroyed and recreated (hence the ref).
    /// \param baseImage - the image to check against, or null to use
    /// fImage of the control.
    virtual CATResult CheckImageSize( CATImage *&     image, 
                                      const CATImage* baseImage = 0);

    //---------------------------------------------------------------------
    // Common data members for all controls from skin
    //---------------------------------------------------------------------

    /// Value of the control 
    CATFloat32  fValue;

    /// Default value of control.
    CATFloat32  fDefValue;   
    CATFloat32  fMinValue;
    CATFloat32  fMaxValue;  

    /// Command to send when control is pressed/activated
    CATString   fCmdString;

    /// Target for command, if any. Empty for general commands.
    CATString   fTarget;

    /// Additional parameter to send when control is pressed/activated
    CATString   fCmdParam;

    /// Command Type (empty or "VSTParam" for vst parameters)
    CATString   fCmdType;

    /// Foreground disabled color
    CATColor    fFgDisColor;

    /// Background disabled color
    CATColor    fBgDisColor;

    /// Default Cursor for control
    CATCursor   fCursor;

    /// Inactive image for control
    CATImage*   fImageDisabled;

    /// pressed  image for control
    CATImage*   fImagePressed;

    /// Focus image
    CATImage*   fImageFocus;

    /// Focus image w/Active
    CATImage*   fImageFocusAct;

    /// MouseOver image for control
    CATImage*   fImageActive;

    CATString       fText;
    bool            fAutoScaleText;
    bool		    fTextCentered;
    CATPOINT        fTextOffset;
    CATPOINT        fTextOffsetPressed;
    bool            fShowBg;
    bool            fMultiline;
    //--------------------------------------------------------------------

    CATString   fFontName;
    CATFloat32  fFontSize;

    // State controls

    /// Is the control pressed (mouse currently down on it) ?
    bool        fPressed;

    /// Is the control focused?
    bool        fFocused;

    /// Is the control active (mouse over)?
    bool        fActive;

};

#endif // CATControl_H_


