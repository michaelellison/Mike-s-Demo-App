//---------------------------------------------------------------------------
/// \class CATGuiObj.h
/// \brief GUI object base class
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
#ifndef _CATGuiObj_H_
#define _CATGuiObj_H_

#include "CATGUIInternal.h"
#include "CATGUIAccessDefs.h"

class CATWindow;
class CATControl;
class CATGuiObj;

// Callback used for control enumeration. REturn true to continue enumeration.
typedef bool (*CATCONTROLFUNCB)(CATControl* control, void* userParam);

/// \class CATGuiObj
/// \brief GUI object base class
/// \ingroup CATGUI
class CATGuiObj : public CATXMLObject
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATGuiObj(  const CATString&               element, 
                const CATString&               rootDir);

    virtual ~CATGuiObj();

    /// GetName() retrieves the name of the control.
    virtual CATString    GetName() const;

    /// GetHint() retrieves the hint text for the control
    virtual CATString    GetHint() const;

    /// OnEvent() is called when events occur that we should know about.
    virtual CATResult    OnEvent  (  const CATEvent&  event, CATInt32& retVal);

    /// MarkDirty() marks the control as dirty (needing to be redrawn)
    ///
    /// \param dirtyRect - if specified, marks only part of the rectangle
    /// \param force - if true, forces update even if object isn't visible
    virtual void         MarkDirty(CATRect* dirtyRect = 0, bool force = false){};

    /// GetRect() retrieves the rectangle of the GUI object.
    /// Skins will have a null rectangle.
    /// \return CATRect - rectangle of parent
    virtual CATRect      GetRect();

    /// GetRectAbs() retrieves the rect of the GUI object in
    /// absolute coordinates (e.g. w/o layer relocation).
    ///
    /// \param screenCoordinates - if true, returns the rect in screen
    ///                            coordinates. Otherwise, they are in
    ///                            client coordinates.
    virtual CATRect      GetRectAbs( bool screenCoordinates = false);

    /// Load() loads the skin in.
    virtual CATResult    Load(  CATPROGRESSCB	progressCB    = 0,
                                void*			progressParam = 0,
                                CATFloat32		progMin       = 0.0f,
                                CATFloat32		progMax		  = 1.0f);


    /// RectFromAttribs() recalculates the control's rectangle from
    /// the attributes.  This can only be called after ParseAttributes() has
    /// loaded the images.
    virtual CATResult    RectFromAttribs();

    /// Retrieve minimum and maximum size for the object.
    /// 
    /// If there is no minimum or maximum for a given parameter, then
    /// it will be set to 0.
    ///
    /// \param minWidth - ref to receive minimum width of the object
    /// \param minHeight - ref to receive minimum height of the object
    /// \param maxWidth - ref to receive maximum width of the object
    /// \param maxHeight - ref to receive maximum height of the object
    virtual void      GetMinMax   ( CATInt32& minWidth, 
                                    CATInt32& minHeight,
                                    CATInt32& maxWidth, 
                                    CATInt32& maxHeight);

    virtual bool      IsEnabled() const;

    virtual void      SetEnabled(bool enabled);



    /// IsVisible() returns true if the window is currently visible.
    /// \sa SetVisible()
    virtual bool      IsVisible    ( const CATGuiObj* object = 0  ) const;
    virtual void	  SetVisible   ( bool             visible);


    /// GetWindow() retrieves the base window.
    /// By default, return a NULL pointer.
    virtual CATWindow*   GetWindow() const {return 0;}      

    // All widgets must be able to hit test.
    // Layers check all controls within themselves and return any that hit.
    // Controls just check if they're hitable, and return themself if so.
    virtual CATControl*  HitTest(        const CATPOINT&  point) {return 0;}

    /// ForEachControl() calls the callback with each control.
    /// Containers are not sent to the callback.
    virtual bool         ForEachControl( CATCONTROLFUNCB  callback,    
                                         void*            userParam);

    /// Recursively looks for the first object matching the name.
    /// Optionally, may also filter by object type.
    virtual CATGuiObj*   Find(           const CATString& objectName,
                                         const CATString& objectType = L"");

    /// Retrieve a stack of post rectangles from child controls for use in 
    /// clipping the main window's drawing routines.
    ///
    /// \param rectStack - a stack of rectangles to add Post-draw
    /// rects to when building up the clipping region.
    /// \return bool - true if any post rectangles are found.
    virtual bool GetPostRects(CATStack<CATRect>& rectStack);

    /// OnCommand() is called by child controls when they are pressed /
    /// changed.  
    ///
    /// Each clickable control has an XML-scripted
    /// command string and parameter, as well as the control's value.
    /// By default, commands are sent from the control to its parent, 
    /// then up to the window's parent skin, and from there to the
    /// application.
    ///
    /// \param command - the command 
    /// \param ctrl - the control that sent it, or 0.
    virtual void   OnCommand( CATCommand& command,
                              CATControl* ctrl);         

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    virtual CATUInt32       GetAccessRole();
    virtual CATUInt32       GetAccessState();
	 virtual bool				 NeedsArrows() {return false;}
    
protected:      

    /// LoadSkinImage() loads an image from the skin
    CATResult LoadSkinImage( const CATString&  filename, 
                             CATImage*&        imagePtr);

    //---------------------------------------------------------------------
    // Common data members for all objects in a skin
    //---------------------------------------------------------------------
    
    /// Root directory of skin
    CATString       fRootDir;

    /// Name of control
    CATString       fName;  

    /// Hint text for control (for status bar)
    CATString       fHintText;

    /// Normal image for control
    CATImage*       fImage;

    /// Control Rectangle - this is the current control rect
    /// Use RectFromAttribs() to set when the size of the window
    /// changes.
    CATRect         fRect;

    /// fMinWidth is the minimum width of the control.  Even if the control
    /// resizes itself as a result of the parent Window resizing, it will
    /// not go below this width.
    CATInt32        fMinWidth;

    /// fMinHeight is the minimum height of the control.  Even if the control
    /// resizes itself as a result of the parent Window resizing, it will
    /// not go below this height.
    CATInt32        fMinHeight;

    /// fMaxWidth is the maximum width of the control.  If it is set to 0,
    /// then no maximum width is enforced.
    CATInt32        fMaxWidth;

    /// fMaxHeight is the maximum height of the control.  If it is set to 0,
    /// then no maximum height is enforced.
    CATInt32        fMaxHeight;
    //--------------------------------------------------------------------
    // State controls

    /// fEnabled is true if the control is enabled, and false otherwise.
    bool            fEnabled;

    /// fVisible is true if the window is visible
    bool            fVisible;

    /// Show the value of the control in the hint?
    bool            fShowHintValue;      

    /// Foreground color
    CATColor        fForegroundColor;

    /// Background color
    CATColor        fBackgroundColor;
};

#endif // _CATGuiObj_H_


