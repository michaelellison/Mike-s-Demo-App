//---------------------------------------------------------------------------
/// \file   CATEventDefs.h
/// \brief   Event Definitions file
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
#ifndef _CATEventDefs_H_
#define _CATEventDefs_H_

/// System/Application events
///
/// Events have a code identifying the event type (as enumerated below or in 
/// the an app header) and any of the following data components:
///
///      CATInt32         fIntParam1;
///      CATInt32         fIntParam2;
///      CATInt32         fIntParam3;
///      CATInt32         fIntParam4;
///      CATFloat32       fFloatParam1;
///      CATString        fStringParam1;
///      CATString        fStringParam2;
///      void*            fVoidParam;
///
/// Events are passed from the Window down, although a window may choose not
/// to pass an event to its controls.
///
/// Event handlers should return a CATResult.
enum CATEVENTDEFS
{
    //-----------------------------------------------------------------------------

    /// CATEVENT_EXIT
    /// App Exit event. Non-negotiable.
    /// fIntParam1 - exit code   
    CATEVENT_EXIT = 0,

    /// CATEVENT_SHUTDOWN
    /// Let everyone know we're shutting down.
    /// return non-zero to disallow.
    CATEVENT_SHUTDOWN,

    /// CATEVENT_WINDOWS_EVENT
    /// Encapsulated Win32 event.
    /// This may be needed by some controls.
    ///
    /// fIntParam1 - hWnd
    /// fIntParam2 - uMsg
    /// fIntParam3 - wParam
    /// fIntParam4 - lParam
    CATEVENT_WINDOWS_EVENT,  


    /// CATEVENT_WINDOW_CLOSE:
    /// A window has received a request to close.
    /// This message is sent to the app by the 
    /// default OnClose() handler in CATWindow.
    ///
    /// fStringParam1 - name of the window from skin.
    /// fVoidParam - CATWindow* to window.  
    CATEVENT_WINDOW_CLOSE,   

    /// CATEVENT_WINDOW_HIDDEN,
    /// A window has closed or been made hidden.
    ///
    /// fStringParam1 - name of the window from skin.
    /// fVoidParam - CATWindow* to window.  
    CATEVENT_WINDOW_HIDDEN,

    /// CATEVENT_WINDOW_SHOWN
    /// A window has opened or been made visible.
    ///
    /// fStringParam1 - name of the window from skin.
    /// fVoidParam - CATWindow* to window.  
    CATEVENT_WINDOW_SHOWN,


    /// CATEVENT_ENABLE_CHANGE
    /// A layer's enabled state has changed.  Most controls
    /// will be automatically updated, but controlwnd's and the
    /// like may need to update their own states.
    CATEVENT_ENABLE_CHANGE,

    /// CATEVENT_GUI_VAL_CHANGE
    ///
    /// Event signaling that the value of a GUI control changed.
    /// Each control should check to see if it has the same command string, and 
    /// if so, reflect the same value.
    ///
    /// Controls with a command name of "SetValue" are ignored and do not
    /// produce this event directly. Instead, they send it to the command string
    /// from their Target member.
    ///
    /// This is typically sent when a command is received.
    ///
    /// fStringParam1 - Command string / value name (fCmdString on CATControls)
    /// fStringParam2 - String parameter of command
    /// fStringParam3 - String value for control, or empty if numeric.
    /// fFloatParam1  - Value of control
    /// fVoidParam - ptr to control that caused it, or null.
    ///
    /// If a control responds to this event, it should increment the
    /// result value.  It should not respond if it is the same as the 
    /// fVoidParam control.
    ///
    CATEVENT_GUI_VAL_CHANGE,

    /// CATEVENT_GUI_VAL_CHANGE_MATCHPARAM_ONLY
    ///
    /// Event signaling that the value of a GUI control changed.
    /// As above, except that only controls with a matching string
    /// parameter should reflect the new value.
    ///
    /// fStringParam1 - Command string / value name (fCmdString on CATControls)
    /// fStringParam2 - String parameter of command - MATCH THIS AS WELL AS COMMAND!
    /// fFloatParam1  - Value of control
    /// fVoidParam - ptr to control that caused it, or null.
    ///
    /// If a control responds to this event, it should increment the
    /// result value.  It should not respond if it is the same as the 
    /// fVoidParam control.
    ///
    CATEVENT_GUI_VAL_CHANGE_MATCHPARAM_ONLY,


    /// CATEVENT_GUI_UPDATE
    ///
    /// Windows should call Update() when they receive this event, then
    /// increment the result value.
    CATEVENT_GUI_UPDATE,

    //-----------------------------------------------------------------------------
    // Control Events
    //-----------------------------------------------------------------------------
    /// In all of the following events, if fStringParam2 is non-empty, 
    /// then it refers to a specific targetted control name, and only the control 
    /// matching that name should perform the action.
    ///
    /// Otherwise, all controls receiving the event should respond.
    ///
    /// Hereafter, events are just marked with 'fStringParam2 - Target' to indicate
    /// this behaviour.
    //-----------------------------------------------------------------------------

    /// CATEVENT_LISTBOX_ADD
    ///
    /// This event adds an item to the specified list box
    ///
    /// fIntParam1    - index to add at, or -1 for end of list
    /// fStringParam1 - text to add
    /// fStringParam2 - target
    /// fVoidParam    - list data
    CATEVENT_LISTBOX_ADD,

    /// CATEVENT_LISTBOX_REMOVE_INDEX
    ///
    /// This event adds an item to the specified list box
    ///
    /// fIntParam1    - index of item to remove
    /// fStringParam2 - target
    CATEVENT_LISTBOX_REMOVE_INDEX,

    /// CATEVENT_LISTBOX_REMOVE_STRING
    ///
    /// This event adds an item to the specified list box
    ///
    /// fStringParam1 - string to find and remove
    /// fStringParam2 - target
    CATEVENT_LISTBOX_REMOVE_STRING,

    /// CATEVENT_LISTBOX_SET_SEL
    ///
    /// Sets the current item - -1 is none
    ///
    /// fIntParam1 - index of selection
    /// fStringParam2 - target
    CATEVENT_LISTBOX_SET_SEL,

    /// CATEVENT_LISTBOX_SET_SEL_STRING
    ///
    /// fStringParam1 - string to find and select
    /// fStringParam2 - target
    CATEVENT_LISTBOX_SET_SEL_STRING,

    /// CATEVENT_LISTBOX_CLEAR
    ///
    /// Clears the listbox
    ///   
    /// fStringParam2 - target
    CATEVENT_LISTBOX_CLEAR,

    /// CATEVENT_LISTBOX_GET_SEL
    ///
    /// Retrieves the current item in retval
    ///
    /// fStringParam2 - target
    CATEVENT_LISTBOX_GET_SEL,

    /// CATEVENT_LISTBOX_GET_SEL_DATA
    ///
    /// Retrieves the currently selected item's data into
    /// a void* pointed to by fVoidParam - i.e. fVoidParam is a
    /// void**.
    ///
    /// fStringParam2 - target
    /// fVoidParam - void** to receive data
    CATEVENT_LISTBOX_GET_SEL_DATA,

    /// CATEVENT_BROWSER_REFRESH
    ///
    /// This event requests browser controls to refresh. 
    ///
    /// fStringParam2 - Target
    CATEVENT_BROWSER_REFRESH,

    /// CATEVENT_BROWSER_GO
    ///
    /// This event requests browser controls to go to a URL. 
    ///
    /// fStringParam1 - URL
    /// fStringParam2 - Target
    CATEVENT_BROWSER_GO,

    /// CATEVENT_BROWSER_STOP
    ///
    /// This event requests browser controls to stop loading any current page. 
    ///
    /// fStringParam2 - Target
    CATEVENT_BROWSER_STOP,

    /// CATEVENT_BROWSER_BACK
    ///
    /// This event requests browser controls to go back in history. 
    ///
    /// fStringParam2 - Target
    CATEVENT_BROWSER_BACK,

    /// CATEVENT_BROWSER_FORWARD
    ///
    /// This event requests browser controls to forward in history. 
    ///
    /// fStringParam2 - Target
    CATEVENT_BROWSER_FORWARD,

    /// CATEVENT_BROWSER_ZOOM
    ///
    /// This event requests browser controls to set their zoom level
    /// to whatever is proportionate to the float value.
    /// So right now, 0.0 becomes a zoom of 1, and 1.0f becomes
    /// a zoom of 4 (the max)
    ///
    /// fFloatParam1 - Zoom level (0.0f - 1.0f)
    /// fStringParam2 - Target
    CATEVENT_BROWSER_ZOOM,

    /// CATEVENT_TAB_HIDE is sent to the controls on a tabbed layer when
    /// a different layer is selected.
    CATEVENT_TAB_HIDE,

    /// CATEVENT_TAB_SHOW is sent to the controls on a tabbed layer when
    /// the layer is selected.
    CATEVENT_TAB_SHOW,      

    /// CATEVENT_PREF_CHANGED is called when prefs are changed.
    /// You must request the pref from the prefs manager to retrieve the value.
    /// fStringParam1 - Pref name   
    CATEVENT_PREF_CHANGED,

    /// CATEVENT_ON_EDITBOX_SELECT
    /// Called when an editbox is selected. This may be used to perform
    /// an alternate action to normal text editing.
    ///
    /// fFloatParam1 - value of control
    /// fStringParam1 - string of control
    /// fStringParam2 - command of control
    /// fStringParam3 - parameter of control
    /// fVoidParam1 - ptr to control
    ///
    /// retVal should be 0 if handling normally, or 1 if handled.
    CATEVENT_ON_EDITBOX_SELECT,


    CATEVENT_APP = 0x1000,
    //-----------------------------------------------------------------------------
    CATEVENT_LAST
};

#endif // _CATEventDefs_H_


