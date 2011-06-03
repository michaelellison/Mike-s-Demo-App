//---------------------------------------------------------------------------
/// \file CATSkin.cpp
/// \brief Root skin class
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
#include "CATSkin.h"
#include "CATWindow.h"
#include "CATControl.h"
#include "CATApp.h"
#include "CATEventDefs.h"
#include "CATGuiFactory.h"
/// This is the maximum frequence for updating other window's controls
/// in response to a command of the same name.
const CATFloat32 kUPDATESPEED = 0.05f;

//---------------------------------------------------------------------------
/// CATSkin constructor (inherited from CATXMLObject)
/// \param element - Type name ("Skin")
/// \param attribs - attribute information for the window
/// \param parent - parent XML object (should be NULL)
//---------------------------------------------------------------------------
CATSkin::CATSkin(  const CATString&               element, 
                 const CATString&               skinRootDir,
                 const CATString&               skinPath)
                 : CATGuiObj(element,  skinRootDir)
{
    fTemplateCtr = 0;
    fUpdateTime = 0;
    fSkinRoot = skinRootDir;
    fSkinPath = skinPath;
}

//---------------------------------------------------------------------------
/// CATSkin destructor
//---------------------------------------------------------------------------
CATSkin::~CATSkin()
{
    this->CloseAll();

}

// ParseAttributes() parses the known attributes for an object.
CATResult CATSkin::ParseAttributes()
{
    CATResult result = CATGuiObj::ParseAttributes();
    CATString attrib;

    fAuthor     = GetAttribute(L"Author");
    fURL        = GetAttribute(L"URL");
    fCopyright  = GetAttribute(L"Copyright");
    return result;
}

//---------------------------------------------------------------------------
// RectFromAttribs() recalculates the control's rectangle from
// the attributes.  This can only be called after ParseAttributes() has
// loaded the images.
//---------------------------------------------------------------------------
CATResult CATSkin::RectFromAttribs()
{
    // Skins don't have rects.
    this->fRect.Set(0,0,0,0);
    return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// Retrieve the root directory of the skin.
//---------------------------------------------------------------------------
CATString CATSkin::GetRootDir()
{
    return fSkinRoot;
}

//---------------------------------------------------------------------------
// Retrieve the full path to the skin's .XML file.
//---------------------------------------------------------------------------
CATString CATSkin::GetPath()
{  
    return fSkinPath;
}

//---------------------------------------------------------------------------
// Retrieve the author's name for the skin
//---------------------------------------------------------------------------
CATString CATSkin::GetAuthor()
{
    return GetAttribute(L"Author");
}

//---------------------------------------------------------------------------
// Retrieve the author's URL
//---------------------------------------------------------------------------
CATString CATSkin::GetUrl()
{
    return GetAttribute(L"AuthorUrl");
}

//---------------------------------------------------------------------------
// Retrieve the skin's copyright string.
//---------------------------------------------------------------------------
CATString CATSkin::GetCopyright()
{
    return GetAttribute(L"Copyright");
}

//---------------------------------------------------------------------------
// GetWindowByName() - Retrieve a window by name
//---------------------------------------------------------------------------
CATWindow* CATSkin::GetWindowByName(const CATString& name)
{
    CATXMLObject* curWindow = 0;

    CATUInt32 i;
    CATUInt32 numChildren = GetNumChildren();
    for (i = 0; i < numChildren; i++)
    {
        curWindow = GetChild(i);
        if (name.Compare( ((CATGuiObj*)curWindow)->GetName()) == 0)
        {
            if (((CATGuiObj*)curWindow)->GetWindow() == curWindow)
                return (CATWindow*)curWindow;
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
/// Get a stack of all the controls 
/// that reference a specific command in a window.
/// \param command - the string command of the controls to get
/// \param controlStack - a stack to add the controls to
/// \return CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATSkin::GetControlsByCommand( const CATString& windowName,
                                        const CATString& command,
                                        CATStack<CATControl*>& controlStack)
{
    CATWindow* wnd = GetWindowByName(windowName);
    if (wnd == 0)
    {
        return CATRESULTDESC(CAT_ERR_SKIN_WINDOW_NOT_FOUND, windowName);
    }

    return this->GetControlsByCommand(wnd,command,controlStack);
}
CATResult CATSkin::GetControlsByCommand( CATWindow* wnd, 
                                        const CATString& command,
                                        CATStack<CATControl*>& controlStack)
{
    if (wnd == 0)
    {
        return CATRESULT(CAT_ERR_SKIN_WINDOW_NOT_FOUND);
    }

    CATUInt32 numChildren = wnd->GetNumChildren();
    CATUInt32 i;

    CATXMLObject* curObj = 0;
    for (i = 0; i < numChildren; i++)
    {
        curObj = wnd->GetChild(i);
        CATControl* curControl = (CATControl*)curObj;
        if (curControl->GetCommand().GetCmdString().Compare(command) == 0)
        {
            controlStack.Push(curControl);
        }
    }

    return CAT_SUCCESS;   
}

//---------------------------------------------------------------------------
CATResult CATSkin::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    CATResult result      = CAT_SUCCESS;
    CATResult testResult  = CAT_SUCCESS;
    CATXMLObject* curWindow = 0;


    CATInt32 lRes = 0;

    // Handle any events the skin cares about....
    switch (event.fEventCode)
    {
        // When windows are shown or hidden, we should notify any controls that
        // may be used to toggle them with the DoWindow command of the new state...
    case CATEVENT_WINDOW_SHOWN:
        result = this->OnEvent( CATEvent(CATEVENT_GUI_VAL_CHANGE_MATCHPARAM_ONLY,
            0,0,0,0,1.0f,"DoWindow",event.fStringParam1),lRes);
        break;
    case CATEVENT_WINDOW_HIDDEN:
        result = this->OnEvent( CATEvent(CATEVENT_GUI_VAL_CHANGE_MATCHPARAM_ONLY,
            0,0,0,0,0.0f,"DoWindow",event.fStringParam1),lRes);
        break;
    }

    // Now pass 'em on to children
    CATUInt32 numChildren = GetNumChildren();
    CATUInt32 i;

    for (i = 0; i < numChildren; i++)
    {
        curWindow = GetChild(i);
        if ( ((CATGuiObj*)curWindow)->GetWindow() == curWindow)
        {
            testResult = ((CATWindow*)curWindow)->OnEvent(event, retVal);      
            if (result == CAT_SUCCESS)
            {
                result = testResult;
            }
        }
    }  
    return result;
}

void CATSkin::OnCommand(CATCommand& command, CATControl* ctrl, CATWindow* wnd)
{   
    // Some commands should periodically (or immediately) update all of the
    // windows before the command is executed, in order to make the GUI
    // seem responsive.  In those cases, tag the updateWindows value
    // to true.
    bool updateWindows = false;

    // By default, all commands go on to the app for full processing.
    // However, certain commands may not need to, or must be modified before
    // going to the app (e.g. "SetValue"). 
    // In these cases, the command should be processed by its handler here
    // and processCommand should be set to false.
    bool processCommand = true;
    CATString cmdString = command.GetCmdString();
    CATInt32 evRes = 0;

    if (cmdString.Compare("SetValue") == 0)
    {
        // SetValue is a special command, that just sets the value of
        // a different command string to something specific.  
        // It does not affect other controls with the command string of "SetValue".
        //    
        this->OnEvent(CATEvent(  CATEVENT_GUI_VAL_CHANGE, 
                                0,
                                0,
                                0,
                                0,
                                command.GetValue(), 
                                command.GetTarget(),
                                command.GetStringParam(),
                                "",
                                ctrl), 
                       evRes);   

        // SetValue forces windows to update immediately, since it should typically
        // NOT be used on knobs (doesn't make sense to....)
        updateWindows = (evRes != 0);
        processCommand = false;

        // Send the actual command, not the SetValue, to the app once here.
        CATCommand subCommand(command.GetTarget(), command.GetValue(), command.GetStringParam(), "");
        gApp->OnCommand(command,0,0,this);
    }
    /// DoWindow() opens or closes a window, depending on the value of the command.
    else if (cmdString.Compare("DoWindow") == 0)
    {
        CATResult result = CAT_SUCCESS;
        CATWindow* wndOpen = 0;
        result = OpenWindow(command.GetStringParam(),wndOpen,0);

        if (wndOpen)
        {
            if (wndOpen->IsVisible())
            {
                wndOpen->SetActive();
            }
        }

        if (CATFAILED(result))
        {
            gApp->DisplayError(result);
        }
    }
    else
    {
        if (wnd->IsTemplate())
        {
            // Templates only reflect commands within a window. Otherwise,
            // multiple instances of the same window would have cross-chat,
            // even though the instrument may NOT be the same!
            wnd->OnEvent(CATEvent(  CATEVENT_GUI_VAL_CHANGE, 
                                    0,
                                    0,
                                    0,
                                    0,
                                    command.GetValue(), 
                                    cmdString,
                                    command.GetStringParam(),
                                    "",
                                    ctrl), 
                         evRes);   
        }
        else
        {
            // Update any controls that use the same name for all other commands.
            // i.e. reflection of value between controls      
            this->OnEvent(CATEvent( CATEVENT_GUI_VAL_CHANGE, 
                                    0,
                                    0,
                                    0,
                                    0,
                                    command.GetValue(), 
                                    cmdString,
                                    command.GetStringParam(),
                                    "",
                                    ctrl), 
                          evRes);   
        }

        // Normal command reflection only forces window updates if they have not been
        // updated recently, to reduce processor load when performing fast updates.
        // Even if not forced, the windows will eventually update - they've been marked
        // invalid already.
        if (evRes != 0)
        {                     
            if ( ((CATFloat32)(clock() - fUpdateTime)) / (CATFloat32)CLOCKS_PER_SEC > kUPDATESPEED )
            {
                updateWindows = true;
                fUpdateTime = clock();
            }
        }
    }


    // Skin-level commands that should be reflected and stuff...
    if (cmdString.Compare("Browser",7) == 0)
    {      
        if (cmdString.Compare("Go",0,7) == 0)
        {         
            CATTRACE((CATString)"Browser Go: " << command.GetStringParam());
            this->OnEvent(CATEvent( CATEVENT_BROWSER_GO,0,0,0,0,0,
                                    command.GetStringParam(),
                                    command.GetTarget()),
                          evRes);
        }
        else if (cmdString.Compare("Refresh",0,7) == 0)
        {
            this->OnEvent(CATEvent( CATEVENT_BROWSER_REFRESH,0,0,0,0,0,"",command.GetTarget()),evRes);
        }
        else if (cmdString.Compare("Stop",0,7) == 0)
        {
            this->OnEvent(CATEvent( CATEVENT_BROWSER_STOP,0,0,0,0,0,"",command.GetTarget()),evRes);
        }
        else if (cmdString.Compare("Back",0,7) == 0)
        {
            this->OnEvent(CATEvent( CATEVENT_BROWSER_BACK,0,0,0,0,0,"",command.GetTarget()),evRes);
        }
        else if (cmdString.Compare("Forward",0,7) == 0)
        {
            this->OnEvent(CATEvent( CATEVENT_BROWSER_FORWARD,0,0,0,0,0,"",command.GetTarget()),evRes);
        }
        else if (cmdString.Compare("Zoom",0,7) == 0)
        {
            this->OnEvent(CATEvent( CATEVENT_BROWSER_ZOOM,0,0,0,0,command.GetValue(),"",command.GetTarget()),evRes);
        }
    }

    // Update windows if it's time and someone was set...
    if (updateWindows)
    {
        this->OnEvent(CATEvent(CATEVENT_GUI_UPDATE),evRes);
    }

    if (processCommand)
    {
        gApp->OnCommand(command,ctrl,wnd,this);   
    }
}

// OpenWindow() opens a window from the skin by name.
// If the window is already open, then it just returns the handle.
//
// \param wndName - name of the window to open
// \param window - ref to receive window if opened or already open.
// \return CATResult - CAT_SUCCESS on success
// CAT_STAT_SKIN_WINDOW_ALREADY_OPEN if the window is already open
// CAT_ERR_SKIN_WINDOW_NOT_FOUND if window not found
CATResult CATSkin::OpenWindow(const CATString&  wndName, 
                              CATWindow*&       window, 
                              CATWindow*        parent, 
                              CATPOINT*         origin, 
                              bool              takeoverWnd)
{
    window = this->GetWindowByName(wndName);

    if (window == 0)
    {
        return CATRESULT(CAT_ERR_SKIN_WINDOW_NOT_FOUND);
    }

    if (window->IsVisible())
    {
        return CATRESULT(CAT_STAT_SKIN_WINDOW_ALREADY_OPEN);
    }    

    window->Show(parent,origin,takeoverWnd);

    return CAT_SUCCESS;
}

// CloseWindow() closes the window from the skin by name.
//
// \param wndName - name of window to close      
// \return CATResult - CAT_SUCCESS on success.
//
// Returns an error if the window is not open (CAT_ERR_SKIN_WINDOW_NOT_OPEN)
// Or CAT_ERR_SKIN_WINDOW_NOT_FOUND if the window does not exist.
CATResult CATSkin::CloseWindow(const CATString& wndName)
{
    CATWindow* window = this->GetWindowByName(wndName);

    if (window == 0)
    {
        return CATRESULT(CAT_ERR_SKIN_WINDOW_NOT_FOUND);
    }

    if (window->IsVisible() == false)
    {
        return CATRESULT(CAT_ERR_SKIN_WINDOW_NOT_OPEN);
    }

    window->Hide(true);

    return CAT_SUCCESS;
}

void CATSkin::CloseAll()
{
    CATXMLObject* curWindow = 0;
    CATUInt32 numChildren = GetNumChildren();
    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        curWindow = GetChild(i);
        if (((CATGuiObj*)curWindow)->GetWindow() == curWindow)
            ((CATWindow*)curWindow)->Hide(true);
    }
}

/// Load() loads the skin in.
CATResult CATSkin::Load(  CATPROGRESSCB				progressCB,
                        void*							progressParam,
                        CATFloat32						progMin,
                        CATFloat32						progMax)
{
    CATResult result = CAT_SUCCESS;
    result = CATGuiObj::Load(progressCB, progressParam, progMin, progMax);
    return result;
}
