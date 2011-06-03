//---------------------------------------------------------------------------
/// \file CATWindow.h
/// \brief Basic windowing class
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef _CATWindow_H_
#define _CATWindow_H_

#include "CATGUIInternal.h"

#include <time.h>
#include "CATImage.h"
#include "CATGuiObj.h"
#include "CATEvent.h"
#include "CATCursor.h"
#include "CATKeystroke.h"
#include "CATStack.h"
#include "CATQueue.h"
#include "CATMutex.h"
#include "CATSignal.h"
#include "CATApp.h"
#include "CATKnob.h"
#include "CATPrefs.h"
#include "CATThread.h"

class CATWindow;
class CATSkin;
class CATControl;
class CATLabel;
class CATControlWnd;
class CATMenu;

#ifdef CAT_CONFIG_WIN32
    #define CATWM_START_MESSAGE WM_USER
#else
    #define CATWM_START_MESSAGE 0 // unknown?
#endif

/// Information about a control and its siblings
struct CATFINDCONTROLSTRUCT
{
    CATString    Name;
    CATString    TypeName;
    CATControl*  FirstControl;
    CATControl*  FoundControl;
    CATControl*  PrevControl;
    CATControl*  NextControl;
    CATControl*  UpControl;
    CATControl*  DownControl;
    CATControl*  LeftControl;
    CATControl*  RightControl;
    CATControl*  CurControl;
    CATControl*  TargetControl;
};


enum CATWM_MESSAGES
{
    /// Messages for Internal (CATWindow) usage.
    CATWM_INTERNAL     = CATWM_START_MESSAGE + 100,          

    /// Kill window is used to terminate windows internally...	
    CATWM_KILLWINDOW,

    /// Timer during mouse tracking that allows filter-only updates.
    /// Frequency is determined by kMOUSEFILTERFREQ
    CATWM_MOUSETIMER,

    // User update timer
    CATWM_UPDATETIMER,

    CATWM_SCANNINGTIMER,

    /// Messages for Framework usage
    CATWM_FRAMEWORK    = CATWM_START_MESSAGE + 250,         
    CATWM_EVENTPOSTED,
    CATWM_ENDWAIT,
    CATWM_SETWAITLABEL,
    CATWM_SETWAITPROGRESS,


    /// Messages for Application-specific usage
    CATWM_APPLICATION  = CATWM_START_MESSAGE + 500,          
    //-------------------------------
    CATWM_LAST
};

/// kMOUSEFILTERFREQ is the milliseconds between updates while the mouse is 
/// being tracked if mouse is held still.  This allows us to filter knobs
/// and yet eventually get to the right place if the user holds the mouse still.
const CATUInt32 kMOUSEFILTERFREQ = 50; 

/// \class CATWindow CATWindow.h
/// \brief Basic windowing class
///
class CATWindow : public CATGuiObj
{
public:
    /// CATWindow() constructor - based off CATXMLObject
    CATWindow(   const CATString&             element, 
                 const CATString&             rootDir);

    /// CATWindow destructor
    virtual ~CATWindow();          

    /// Show() shows the window. Will create the window if it does not
    /// already exist.
    virtual void      Show        (   CATWindow*            parent = 0,
                                      CATPOINT*             origin = 0,
                                      bool                  takeoverWnd = false);

    /// Show as a child with external hwnd....
    virtual void      Show        (   CATWND                parentWnd);

    /// Hide() hides (or destroys) the window.
    ///
    /// It does not destroy the base CATWindow object - just the operating
    /// system specific Window itself.
    ///
    /// \param destroy - if true, destroys the OS's window. Otherwise,
    /// simply hids the window.
    virtual void      Hide        (   bool                  destroy = false );

    /// IsVisible() returns true if the window is currently visible.
    /// \sa Show(), Hide()
    virtual bool      IsVisible   (   const CATGuiObj*      child  = 0) const;

    /// IsPrimary() returns true if the window is the
    /// primary application window.
    ///
    /// This is mainly used to tell if the program should
    /// exit when the App received notification that a window
    /// was destroyed.  If the window was the primary window,
    /// then the app should generally exit.
    ///
    /// \return bool - true if the window is the Primary window.
    virtual bool      IsPrimary   (  );

    virtual bool      IsOverlapped();

    virtual void	  DisplayMessage( const CATString&      msg);
    virtual void	  DisplayError(   const CATResult&      result);
    virtual CATResult DisplayPrompt(  const CATString&      msg, 
                                      CATApp::CATPROMPTTYPE prompt);

    /// Enables/disables scanning of the current focus for switch users.
    virtual void      EnableScanning( bool                  scanning, 
                                      CATFloat32            scanRate = 1.0f);
    virtual bool        IsScanning();
    virtual CATFloat32  GetScanRate();
    virtual void        OnScanningTimer();
    virtual void        PauseScanning();
    virtual void        ResumeScanning();

    /// FindControlAndVerify() finds a control object by name, then verifies
    /// that it is of the appropriate type before returning it.
    ///
    /// It will also return the index of the control in the window's child
    /// list if desired.
    ///
    /// If the type doesn't match then NULL is returned.      
    ///
    /// \param controlName - name of the control to find.
    /// \param controlName - name of the control type to check against.
    /// \param index - ptr to CATUInt32 to receive index, or 0
    /// \return CATControl* - ptr to matching control, or 0 if none is found.
    CATControl* FindControlAndVerify( const CATString&      controlName, 
                                      const CATString&      typeName,
                                      CATUInt32*            index = 0);

    CATResult EnableObject(           const CATString&      controlName, 
                                      bool                  enabled);

    /// MarkDirty() marks a section of the window as dirty and invalidates
    /// it / notifies the OS that the window needs to be repainted.
    ///
    /// \param dirtyRect - specifies the area of the window that is
    /// dirty. If null, the entire window is marked as dirty.
    virtual void      MarkDirty   (  CATRect*               dirtyRect = 0, 
                                     bool                   force     = false);

    /// Update() forces an immediate paint of any dirty regions
    /// in the window.
    virtual void      Update();

    /// Draw() is called when the window should paint itself. 
    ///
    /// \param background - ptr to image to draw into
    /// \param dirtyRect - part of window to redraw.
    virtual void      Draw        (  CATImage*              background,
                                     const CATRect&         dirtyRect);

    /// PostDraw() is used to paint controls that do not
    /// support, or are not easy to do with, the PNG architecture.
    /// These will mainly be platform specific controls.
    ///
    /// Post-Drawn controls are drawn after the normal
    /// image-based controls, and are layered on top. They are,
    /// however, drawn on a back buffer so they shouldn't flicker.
    ///
    /// \param drawContext - OS specific drawing context.
    /// \param dirtyRect - area of window to paint
    virtual void      PostDraw    (  CATDRAWCONTEXT         drawContext,
                                     const CATRect&         dirtyRect);

    /// PostDrawText() draws text from a post-drawn control.
    virtual void      PostDrawText(  const CATString&       text,
                                     CATDRAWCONTEXT         context,
                                     const CATRect&         textRect,
                                     const CATRect&         dirtyRect,
                                     const CATColor&        textColor,
                                     const CATString&       fontName,
                                     CATFloat32             fontSize,
                                     bool                   multiline = false,
                                     CATColor*              bgColor   = 0,
                                     bool                   italics   = false,
                                     bool			        centered  = false,
                                     bool                   autosize  = false);

    /// OnSize() is called when the window changes size.
    virtual void      OnSize      (  const CATRect&         newRect);

    /// OnEvent() is called when events occur that we should know about.
    virtual CATResult  OnEvent  (    const CATEvent&        event, 
                                     CATInt32&              retVal);

    typedef void (CATWindow::*CATCOMMANDFUNC)( CATCommand&   command,
                                              CATControl*   control);

    struct CATWINDOWCMDFUNC
    {
        char*          CommandName;
        CATCOMMANDFUNC  CommandFunc;
        bool		   DisableWindow;	// Disable window during command?
        bool		   Threaded;		// Should be run threaded?
    };	


    virtual void SetVisible(bool visible);
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
    virtual void      OnCommand(CATCommand& command, CATControl* ctrl);

    /// OnThreadedCommand is called on the background command thread.
    /// It should be used for commands that will take a long time, using
    /// the PostThreadedCommand() function.
    ///
    /// \param command - the command 
    virtual void      OnThreadedCommand(CATCommand& command);
                      
    /// OnDestroy() is called just before the OS's window is destroyed.          
    virtual void      OnDestroy();

    /// OnCreate() is called when a new window is created.
    virtual void      OnCreate();

    /// OnClose() is called when a window close message is received.
    /// Return true to allow close, false to disallow.
    virtual bool      OnClose();

    /// OnMove() is called whenever the window moves.
    virtual void      OnMove(const CATPOINT& newPos);

    /// OnMoving() is called while the window is being moved.
    /// If the move rectangle is changed, return true to let the
    /// framework know.
    ///
    /// \param moveRect - the move rectangle. This may be changed by OnMoving().
    /// \param wndRect - actual (OS) window rectangle of current window
    /// \return bool - true if the rect is changed, false otherwise.
    virtual bool      OnMoving(CATRect& moveRect, const CATRect& wndRect);

    /// Load() translates the XML attributes into object variables and
    /// objects for the window and its children.  
    ///
    /// CATGuiObj does most of the work here, but any custom stuff should
    /// be checked after the parent returns.
    ///
    /// \return CATResult - CAT_SUCCESS on success.
    virtual CATResult Load( CATPROGRESSCB				progressCB    = 0,
                            void*						progressParam = 0,
                            CATFloat32					progMin       = 0.0f,
                            CATFloat32					progMax		  = 1.0f);

    /// OSSetMouse() - Operating system specific call to set the mouse 
    /// location and cursor.  
    ///
    /// This is mainly used by linear knobs to  hold the cursor in one 
    /// location.
    ///
    /// \param point - location to set the cursor to
    /// \param cursor - cursor to set
    ///
    void              OSSetMouse(const CATPOINT& point, CATCursor& cursor);


    /// SetCursorType() sets the active cursor type for the window.
    void              SetCursorType(CATCURSORTYPE type);

    /// OSSetCursor activates the current cursor
    void              OSSetCursor();

    /// OSSetCursor for a specific cursor type..
    void              OSSetCursor(CATCursor& cursor);

    /// OSDrawCursor() - Operating system specific call to draw a 
    /// copy of the mouse cursor.
    ///
    /// This is used with linear knobs to draw a stationary cursor
    /// while the knob is turned.  The actual cursor is hidden, as
    /// it jumps around a bit even with the framework resetting its
    /// position.
    ///
    /// \param context - drawing context to draw the cursor to
    /// \param point - location to draw the cursor
    /// \param cursor - the cursor object to draw
    void OSDrawCursor(CATDRAWCONTEXT context, const CATPOINT& point, CATCursor& cursor);

    /// OSGetWnd() returns the operating system-specific window handle for
    /// the window.
    /// \return CATWND - window handle (win32 - HWND for the window)
    CATWND OSGetWnd();

    /// OSGetWndRect() retrieves the actual window rectangle
    /// of the window - including non-client areas like borders
    /// and menu bars.
    virtual CATRect       OSGetWndRect();

    // Minimize the window
    void           OSMinimize();

    // Maximize the window
    void           OSMaximize();

    /// OSWndToScreen() converts x/y coordinates from within the window to screen
    /// coordinates.
    /// \param x - x position to convert
    /// \param y - y position to convert
    void OSWndToScreen( CATInt32& x, CATInt32& y);

    /// OSScreenToWnd() converts x/y coordinates from within the screen to within
    /// the window.
    /// \param x - x position to convert
    /// \param y - y position to convert
    void OSScreenToWnd( CATInt32& x, CATInt32& y);

    /// OSSetFocus sets the operating system-specific window focus.
    ///
    /// This is mainly used by child controls that have their own
    /// windows.
    /// 
    /// \param window - new focus target.
    void OSSetFocus(CATWND window = 0);

    /// OSMoveWnd() moves and/or resizes the specified window (or the base window)
    /// to the requested rectangle.
    /// 
    /// If window == 0, then the base window is used.
    /// \param rect - new window rect
    /// \param wnd - the window, or 0 for base
    virtual void         OSMoveWnd(const CATRect& rect, CATWND wnd = 0);

    /// OSDestroyWnd() is called to specifically destroy a window.
    ///
    /// If window == 0, then the window object's window is destroyed.
    /// Otherwise, you can pass it the window of a control or other
    /// window to nuke.
    /// \param window - window to destroy.
    virtual void         OSDestroyWnd(CATWND window = 0);      

    /// SetFocus() sets the currently focused control.
    ///
    /// \param control - ptr to control to set the focus to.
    virtual void         SetFocus(CATControl* control);

    /// KillFocus() removes the focus from all controls.
    virtual void         KillFocus();

    /// SetFocusNext() goes to the next control, or wraps around
    /// to the beginning.
    ///
    /// This is mainly used when the user pressed TAB.
    ///
    virtual void         SetFocusNext();

    /// Sets the focus to the first control
    virtual void         SetFocusFirst();

    /// Sets the focus to the control above, or previous
    virtual void         SetFocusUp();
    /// Sets the focus to the control to the left, or previous
    virtual void         SetFocusLeft();

    /// Sets the focus to the control below, or next
    virtual void         SetFocusDown();
    
    /// Sets the focus to the control to the right, or next
    virtual void         SetFocusRight();

    /// SetFocusPrev() goes to the previous control, or
    /// wraps around the list to the end.
    ///
    /// This is mainly used by shift-tab.
    ///
    virtual void         SetFocusPrev();

    /// GetFocusControl() retrieves the currently focused control,
    /// or 0 if none.
    virtual CATControl*   GetFocusControl();

    /// RegCtlWnd() registers a window handle from a CATControlWnd.
    /// This allows us to send messages w/o doing full scans for the
    /// control through the skin's tree.
    ///
    /// \param wndCtl - window control
    /// \param wndHandle - window handle of the control
    /// \return CATResult - CAT_SUCCESS on success
    /// \sa UnRegCtlWnd()
    virtual CATResult     RegCtlWnd(CATControlWnd* wndCtl, CATWND wndHandle);

    /// UnRegCtlWnd() unregisters a window handle from a CATControlWnd.
    /// This should be called when the control destroys its handle to
    /// prevent mis-sends.
    /// \param wndHandle - old window handle of the control
    /// \return CATResult - CAT_SUCCESS on success
    /// \sa RegCtlWnd()
    virtual CATResult     UnRegCtlWnd(CATWND wndHandle);

    /// GetCtlWnd() retrieves a CATControlWnd for a given
    /// handle, or 0 if none is registered.
    virtual CATControlWnd* GetCtlWnd(CATWND wndHandle);

    /// RectFromAttribs() recalculates the control's rectangle from
    /// the attributes.  This can only be called after ParseAttributes() has
    /// loaded the images.
    virtual CATResult    RectFromAttribs();

    virtual void SetEnabled(bool enabled);
    virtual void         SetActive();

    static void SetTrackMouse(CATWND hwnd);      

    virtual        bool  IsTemplate();
    virtual  void        SetIsTemplate(bool isTemplate);

    virtual CATFONT      AutoFontSize(    const CATString& text,
                                          CATRect          rect,
                                          const CATString& fontName,
                                          CATFloat32       fontSize,
                                          bool             bold       = false,
                                          bool             italic     = false,
                                          bool             underlined = false);
                                          


    virtual CATFONT       OSGetFont( const CATString&   fontName,
                                     CATFloat32         fontSize,
                                     // these are currently not implemented.
                                     bool               bold        = false,
                                     bool               italic      = false,
                                     bool               underlined  = false);

    virtual void         OSReleaseFont( CATFONT& font);

    /// OnMouseWheel() is called when the mouse wheel is turned.
    ///
    /// If the mouse is over a control, then this function
    /// typically calls the control's TrackMouseWheel() function.
    ///
    /// \param point - location of mouse poitner
    /// \param wheelMove - amount of wheel movement.
    /// Currently, 1 notch should be about 1.0 in distance. 
    /// Positive is 'forward' or away from the user, negative
    /// is backwards or towards the user.
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseWheel(  const CATPOINT& point,
                                        CATFloat32      wheelMove,
                                        CATMODKEY		modKey );

    /// SetKnobOverride() overrides any knobs in a window to behave
    /// as specified (circular or linearly)
    virtual void SetKnobOverride(CATKNOB_STYLE knobStyle);


    /// OnKeyDown() is called when a key is initially pushed down.
    /// See CATKeystroke.h for details on keystroke decoding.
    ///
    /// \param keystroke - the key that was pushed.
    virtual void         OnKeyDown(const CATKeystroke& keystroke);

    /// OnKeyUp() is called when a key is released..
    /// See CATKeystroke.h for details on keystroke decoding.
    ///
    /// \param keystroke - the key that was released.
    virtual void         OnKeyUp(const CATKeystroke& keystroke);

    /// OnKeyPressed() is called when a key event is received
    /// (the key was pushed down then released).
    ///
    /// See CATKeystroke.h for details on keystroke decoding.
    ///
    /// \param keystroke - the key that was pressed.
    virtual void         OnKeyPress(const CATKeystroke& keystroke);

    /// OnHelp() is called by the framework when the user requests
    /// help from an item in the window.
    virtual void         OnHelp();

    // MoveWindow() manually moves a window's top-left.
    virtual void         CenterWindow( CATWindow* parent );

    /// GetWindow() retrieves the parent window.
    virtual CATWindow*   GetWindow() const;      

    virtual void WindowToWidget( CATGuiObj* widget, CATPOINT& pt);
    virtual void WindowToWidget( CATGuiObj* widget, CATRect& rect);
    virtual void WidgetToWindow( CATGuiObj* widget, CATPOINT& pt);
    virtual void WidgetToWindow( CATGuiObj* widget, CATRect& rect);
    /// ResetBackground() resets the fImageCopy background image.
    /// \return CATResult - CAT_SUCCESS on success.
    virtual CATResult ResetBackground();


    virtual CATResult OnDropFile(const CATString& filePath);

    // Command handlers
    virtual void OnClose   (CATCommand& command, CATControl* ctrl);
    virtual void OnMinimize(CATCommand& command, CATControl* ctrl);
    virtual void OnMaximize(CATCommand& command, CATControl* ctrl);

    virtual void OnNoop    (CATCommand& command, CATControl* ctrl);

    /// Templated preference conversion with default val.
    /// Returns default value if pref is not found or is empty.
    /// otherwise, converts the CATString to the proper type.
    ///
    /// Defaults to retrieving the preference for the active window.
    template<class T>
    T GetPref(const CATWChar* prefName, T defaultVal, const CATWChar* prefSection = 0)
    {
        if (prefName == 0)
            return defaultVal;

        if (!gApp)
            return defaultVal;

        CATPrefs* prefs = gApp->GetPrefs();
        if (!prefs)
            return defaultVal;

		CATString sectionName;
		if (prefSection == 0)
			sectionName << fName << L"_Window";
		else
			sectionName = prefSection;

        prefs->GetPref( sectionName, prefName, defaultVal);

        return (T)defaultVal;
    }

    /// SetPref() sets the value of the specified preference.
    /// 
    /// \param prefName    - name of preference
    /// \param prefValue   - value of preference (several overloaded types)
    /// \return bool - true on success.
    /// \sa SetPref()
    template<class T>
    bool SetPref(const CATWChar* prefName, T prefVal, const CATWChar* prefSection = 0)
    {        
        if (prefName == 0)
            return false;

        if (!gApp)
            return false;

        CATPrefs* prefs = gApp->GetPrefs();
        if (!prefs)
            return false;

        bool success = false;

		// Append 'Window' to window pref sections to avoid ambiguous names
		CATString sectionName;
		if (prefSection == 0)
			sectionName << fName << L"_Window";
		else
			sectionName = prefSection;

        success = prefs->SetPref( sectionName, prefName, prefVal);

		CATInt32 lRes = 0;
        this->SendEvent(CATEvent(CATEVENT_PREF_CHANGED,0,0,0,0,0,prefName, prefSection),lRes);
        return success;
    }        


    virtual void OnBuildMenu(CATMenu* menu) {};

    CATWND 	GetBaseWindow();
    CATResult PostEvent(CATEvent* event);
    CATResult PostThreadedCommand(CATCommand& cmd);

    /// GetRectAbs() retrieves the rect of the GUI object in
    /// absolute coordinates (e.g. w/o layer relocation).
    ///
    /// \param screenCoordinates - if true, returns the rect in screen
    ///                            coordinates. Otherwise, they are in
    ///                            client coordinates.
    virtual CATRect GetRectAbs( bool screenCoordinates = false);

    /// HitTest() checks to see if there is a focuseable control
    /// under the point, and returns it if so.
    ///
    /// \param point - location to check
    /// \return CATControl* - ptr to control under the point or 0.
    virtual CATControl*   HitTest(const CATPOINT& point);

    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_WINDOW;}

    /// Returns the scale factors for controls on the window.
    /// If scaling is not enabled, returns an error and 1.0f for scale values.
    virtual CATResult     GetScale(CATFloat32& xScale, CATFloat32& yScale);

    void GetControlStruct(CATFINDCONTROLSTRUCT& ctrlStruct, CATControl* control);
    
protected:
    // SendEvent() sends an event to the app, and if this window is 
    // templated, to the window and its children as well.
    CATResult SendEvent( const CATEvent& event, CATInt32& lRes);

    CATResult ProcessCommandTable(  CATCommand&                  cmd,
                                    CATControl*                  control,
                                    const CATWINDOWCMDFUNC*       cmdTable,
                                    CATUInt32                    cmdTableLen,
                                    bool								  inThread);

    /// CalcSlack is a utility function for calculating docking offsets
    /// \param movePos - requested move position, may be changed on ret
    /// \param opposite - opposite side of requested move, may be changed on ret
    /// \param dockPos - position to check against
    /// \param slackCounter - counter tracking distance we've docked so far on pos
    /// \param inside - are we docking inside the rect or outside?
    /// \return true if movePos/opposite changed and docked, false otherwise.
    bool CalcSlack(  CATInt32&         movePos, 
                     CATInt32&         opposite,
                     const CATInt32&   dockPos, 
                     CATInt32&         slackCounter,
                     bool              inside = false);


    /// CleanBackground() cleans the dirty rectangle in the background
    ///
    /// This keeps us from having to perform a full copy of the background
    /// image each draw.
    ///
    /// \param dirtyRect - the rectangle to draw, or 0 to draw everything.
    /// \return CATResult - CAT_SUCCESS on success.
    virtual CATResult CleanBackground( CATRect* dirtyRect );


    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    //  OS specific functions - implemented in CATWindow_Win32.cpp (or mac, etc)

    /// OSDraw() performs the operating system specific drawing.
    ///
    /// OSDraw() will call back into the window to perform the normal
    /// Draw() command, copy the resulting image into a system-dependant
    /// image, then perform any system-specific control drawing through
    /// PostDraw().  After that, it will display the results to the screen.
    ///
    /// \param drawContext - drawing context for the system / window
    /// \param dirtyRect - the portion of the window to draw, or 0 for all.
    virtual void      OSDraw(  CATDRAWCONTEXT  drawContext,
        CATRect*        dirtyRect);

    /// OSGenRegion() generates a region from a stack of rectangles to 
    /// use for clipping.
    ///
    /// You should call OSFreeRegion() on the region handle when done.
    ///
    /// \return CATOSREGION - regions we can draw in for clipping
    /// \sa OSFreeRegion()
    virtual CATOSREGION   OSGenRegion(CATStack<CATRect>& rectStack);

    /// OSFreeRegion() frees a region previously created by
    /// OSGenRegion().
    ///
    /// \param rgn - region to free, generally previously allocated
    /// by OSGenRegion().
    /// \sa OSGenRegion()
    virtual void         OSFreeRegion(CATOSREGION rgn);

    /// OSGetDockRect() retrieves either the coordinates of the
    /// parent window, or of the screen if no parent window.
    /// \return CATRect - parent bounding rect
    virtual CATRect       OSGetDockRect();

    /// OSCreateWnd() performs the actual OS-specific grunt work
    /// for creating a window.
    ///
    /// \return CATResult - CAT_SUCCESS on success.
    virtual CATResult     OSCreateWnd( CATWND wnd = 0);      

    virtual CATResult     OSTakeoverWnd(CATWindow* parentWnd);
    virtual CATResult     OSRestoreTakenWnd();

    /// OSShowWnd() shows the window.  If the GUI window has not been
    /// created yet, you should call OSCreateWnd() instead.
    /// \sa OSHideWindow()
    virtual void         OSShowWnd();


    /// OSHideWindow() hides the window.  It does not destroy it.
    ///
    /// \sa OSShowWnd()
    virtual void         OSHideWnd();

    /// Retrieve window style indicator based on our parameters for window
    virtual CATUInt32      OSGetWindowStyle();

    /// Retrieve the full window rect
    virtual CATRect        OSGetRealRect();

    /// OSBeginMouseTrack() requests the operating system to begin
    /// sending all mouse movement commands to the interface. When
    /// we start mouse tracking, typically we'll send all mouse 
    /// events only to the control that began it.
    ///
    /// This is called when the mouse is pressed on a focuseable
    /// control.
    virtual void           OSBeginMouseTrack();

    /// OSEndMouseTrack() ends a previously started mouse tracking.
    ///
    /// The is called when the mouse is released.
    /// \sa OSBeginMouseTrack()
    virtual void         OSEndMouseTrack();

    /// OnMouseMove() is called each time the mouse moves in the window.
    /// 
    /// If the mouse is being tracked, it should send the message to
    /// the focused control's TrackMouseMove().
    ///
    /// \param point - location of the mouse
    /// \param leftButton - true if left button is pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseMove(   const CATPOINT& point, 
        bool leftButton, 
        CATMODKEY modKey);

    /// OnMouseHover() is called each time the mouse hovers for a bit
    ///
    /// \param point - location of the mouse
    /// \param leftButton - true if left button is pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseHover(  const CATPOINT& point, 
                                        bool            leftButton, 
                                        CATMODKEY       modKey);

    /// OnMouseDown() is called when a mouse button is presesd.
    ///
    /// It handles the mouse press, finds the control (if any)
    /// that is pressed, and begins tracking.
    ///
    /// \param point - location mouse is pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseDown(       const CATPOINT& point, 
                                            CATMODKEY       modKey);

    /// OnMouseUp() is called when a previously pressed button is 
    /// released.
    ///
    /// This function will end tracking if needed and notify
    /// the control of the release.
    ///
    /// \param point - location mouse was released
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseUp(         const CATPOINT& point, 
                                            CATMODKEY		modKey);

    /// OnRightMouseDown() is called when the right mouse button is presesd.
    ///
    /// \param point - location mouse is pressed
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnRightMouseDown(  const CATPOINT& point, 
                                            CATMODKEY       modKey);

    /// OnRightMouseUp() is called when the right mouse button
    /// is released.
    ///
    /// \param point - location mouse was released
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnRightMouseUp(    const CATPOINT&  point, 
                                            CATMODKEY		 modKey);



    /// OnMouseDoubleClick() is called the mouse is double clicked.
    ///
    /// \param point - location mouse was released
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseDoubleClick(const CATPOINT&  point, 
                                            CATMODKEY		 modKey);




    /// OnMouseLeave() is called when the mouse leaves the window.
    ///
    /// This function will typically remove the active state of
    /// any controls, in case the mouse left the window without
    /// 'hitting' the background.
    virtual void         OnMouseLeave();

    /// OnMouseTimer() is called while the mouse is being tracked
    /// if enough time has occurred since the last movement
    /// of the mouse.
    ///
    /// This is used for filtered controls which must receive multiple
    /// messages to achieve the 'real' value, such as knobs.
    ///
    /// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
    virtual void         OnMouseTimer(CATMODKEY modKey);

    // OnWindowTimer is called when a window timer event occurs
    // These are not accurate. Use for updates and the like.
    virtual void			SetWindowTimer	(CATFloat32 frequencyHz);
    virtual void			KillWindowTimer();
    virtual void			OnWindowTimer	() {}

    /// SetStatusBarHint() sets the status bar, if present, to the
    /// specified hint text.
    virtual void        SetStatusBarHint(const CATString& hintText);

    /// GetStatusBarHint() retrieves the current text in the
    /// status bar.
    virtual CATString   GetStatusBarHint();

    /// RegWindow() registers a window class if needed,
    /// providing a string to reference it in the future.
    /// \return CATResult - CAT_SUCCESS on success.
    CATResult           RegWindow();

    /// CATUnRegWindow() unregisters the image app's window class.
    void                CATUnregWindow();


    void                PostEventNotification();
    void                ProcessPostedEvent();


    static bool FindControlCallback(CATControl* curControl, void* userParam);
    static bool GetControlSiblingsCallback(CATControl* curControl, void* userParam);
    static bool FindDirectionalCallback(CATControl* curControl, void* userParam);
    static bool SetKnobCallback(CATControl* curControl, void* userParam);    

    virtual void OSWaitOnCmdThread();
#ifdef CAT_CONFIG_WIN32

    LRESULT OSOnSize          (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnKeyUp         (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnKeyDown       (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnMinMax        (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSDropFiles       (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSGetControlColors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnAppCmd        (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnPaint         (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnMeasureItem   (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSOnDrawItem      (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);
    LRESULT OSGetObject       (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled);

	 HRGN RegionFromImage(CATImage* image);

    static void OSCmdThread(void *param, CATThread* theThread);

    /// window procedure for win32 - the implementation is in CATWindow_Win32.cpp
    static LRESULT CALLBACK WindowProc(
        HWND hwnd,      // handle to window
        UINT uMsg,      // message identifier
        WPARAM wParam,  // first message parameter
        LPARAM lParam   // second message parameter
        );

#endif         

    /// fStatusBarHint is the current hint text for the window, whether from
    /// the window itself, the active control, or current process.
    CATString       fStatusBarHint;

    /// If non-zero, fMouseTrackTarget is a control with the mouse
    /// currently captured.
    CATControl*     fMouseTrackTarget;   

    /// If non-zero, fRightTrack is the control the right mouse was
    /// pressed on.
    CATControl*		fRightTrack;

    /// fActiveControl points to the currently active control, if any.
    /// This is the control that the mouse is over
    CATControl*     fActiveControl;

    /// fFocusControl points to the currently focused control, if any.
    /// This differs from the active control in that focus does not
    /// change from mouse movements - only clicking and hotkeys.
    CATControl*     fFocusControl;

    /// fMouseInWindow is true if the mouse is currently in the window
    bool           fMouseInWindow;

    /// fPrimary determines if the application exits when the window is closed.
    /// If true, then the application will close with the iwndow.
    bool           fPrimary;

    /// fSizeable determines if the window is sizeable.
    bool           fSizeable;

    /// If fScale is true, elements that support scaling will be scaled when the
    /// window is resized. NOT IMPLEMENTED YET
    bool           fScale;

    /// Base width/height for scaling NOT IMPLEMENTED YET
    CATSIZE        fBaseSize;

    /// fCaptionBar determines if we use an OS-specific caption bar or not
    bool           fCaptionBar;

    /// fDragAnywhere - if true, the window can be dragged from any non-control
    /// area
    bool           fDragAnywhere;
    bool           fDragging;
    CATPOINT       fDragLastPoint;

    /// fPostRegion is an OS-specific combination of rectangles describing
    /// areas that should *not* be drawn over by the background or 
    /// imaged controls.  This includes the areas occupied by controls
    /// such as the browser, edit boxes, etc. that are drawn seperately.
    CATOSREGION    fPostRegion;

    /// fImageCopy is a copy of the background image that is drawn into by
    /// the child controls. It is refreshed when 'dirty' by CleanBackground(),
    /// and completely reset by ResetBackground().
    CATImage*      fImageCopy; 

    /// Window class name. This may just be a win32 thing...
    CATString      fClassName;    

    /// Window title - may differ from name.
    CATString      fTitle;

    /// Path to icon, if any
    CATString      fIconPath;
    HICON          fUserIcon;

    /// fSkin is the base skin object (the Window's parent)
    CATSkin*       fSkin;      

    /// fWindow is an OS-specific identifier for the window
    CATWND         fWindow;

    /// fRegistered is true if the window has been registered
    bool           fRegistered;

    /// fCursor holds the window's current cursor
    CATCursor      fCursor;

    /// fLastMouseUpdate is the time of the last mouse update from
    /// OnMouseMove().  This is used to determine if a timer event
    /// should be sent to the currently active control or not.
    clock_t        fLastMouseUpdate;

    /// If a control called "StatusLabel" is available in the window,
    /// this will be pointing at it. 
    CATLabel*      fStatusLabel;

    /// fLastPoint 
    CATPOINT       fLastPoint;

    /// fAutoDock - if true, snaps to other windows and edges
    bool           fAutoDock;

    /// Slack counters for docking...
    CATInt32       fLeftSlack;
    CATInt32       fTopSlack;
    CATInt32       fRightSlack;
    CATInt32       fBottomSlack;

    // MAximized ?
    bool           fMaximized;

    // Is the window object a template and not 
    // a standalone singleton?
    bool           fIsTemplate;

    // Are we an overlapped child window?
    // Generally only true with VST and other plugin types.
    bool           fIsOverlapped;

    /// Control window tree - maps HWND's to controls for 
    /// certain messages.
    std::map<CATWND, CATControlWnd*>    fCWT;
    CATQueue<CATEvent*>                 fEventQueue;

    /// Threaded commands
    void		ThreadedCmdThread();
    void		OSStartCmdThread();

    CATQueue<CATCommand> fThreadedCmdQueue;
    CATSignal			 fThreadedCmdSignal;
    CATMutex			 fThreadedCmdLock;

    CATThread			 fCmdThread;

    CATMutex             fEventLock;
    bool						 fExiting;
    bool						 fExitThread;
    CATWindow*           fPrevWndOwner;
    bool                 fOSDestruction;
	 
	 bool						 fSetRegion;

    // Actual rect of the window
    CATPOINT             fScreenPos;
    bool                 fScanningEnabled;
    CATFloat32           fScanRate;
};


#endif // _CATWindow_H_



