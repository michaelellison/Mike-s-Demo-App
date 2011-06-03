//---------------------------------------------------------------------------
/// \file CATSkin.h
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
//
//---------------------------------------------------------------------------
#ifndef _CATSkin_H_
#define _CATSkin_H_

#include <time.h>

#include "CATImage.h"
#include "CATGuiObj.h"
#include "CATStack.h"

class CATSkin;
class CATWindow;
class CATControl;

/// \class CATSkin CATSkin.h
/// \brief Basic windowing class
/// \ingroup CATGUI
class CATSkin : public CATGuiObj
{
public:
    /// CATSkin constructor - base CATXMLObject contructor,
    /// but with the root directory of the skin and the
    /// full path of the skin's .XML file.
    ///
    CATSkin(  const CATString&               element, 
              const CATString&               skinRootDir,
              const CATString&               skinPath);

    virtual ~CATSkin();          

    /// Retrieve the root directory of the skin.
    CATString    GetRootDir();

    /// Retrieve the full path to the skin's .XML file.
    CATString    GetPath();

    /// Retrieve the author's name for the skin
    CATString    GetAuthor();

    /// Retrieve the author's URL
    CATString    GetUrl();

    /// Retrieve the skin's copyright string.
    CATString    GetCopyright();

    /// Retrieve a window by name
    /// \param name - name of the window to get 
    /// \return CATWindow* - ptr to named window or 0 if not found
    CATWindow*   GetWindowByName(const CATString& name);


    /// GetControlsByCommand() gets a stack of all the controls 
    /// that reference a specific command.
    /// 
    /// \param windowName - name of the window 
    /// \param command - the string command of the controls to get
    /// \param controlStack - a stack to add the controls to
    /// \return CAT_SUCCESS on success.
    CATResult   GetControlsByCommand( const CATString&       windowName,
                                      const CATString&       command,
                                      CATStack<CATControl*>& controlStack);

    /// GetControlsByCommand() gets a stack of all the controls 
    /// that reference a specific command.
    /// 
    /// \param wnd - ptr to specific window
    /// \param command - the string command of the controls to get
    /// \param controlStack - a stack to add the controls to
    /// \return CAT_SUCCESS on success.
    CATResult   GetControlsByCommand( CATWindow*             wnd, 
                                      const CATString&       command,
                                      CATStack<CATControl*>& controlStack);

    /// Event handler
    /// If an event is sent to the skin, it is sent to *all* windows.
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);


    /// OpenWindow() opens a window from the skin by name.
    /// If the window is already open, then it just returns the handle.
    ///
    /// \param wndName - name of the window to open
    /// \param window - ref to receive window if opened or already open.
    /// \return CATResult - CAT_SUCCESS on success
    /// CAT_STAT_SKIN_WINDOW_ALREADY_OPEN if the window is already open
    /// CAT_ERR_SKIN_WINDOW_NOT_FOUND if window not found
    CATResult OpenWindow(const CATString& wndName, 
                         CATWindow*&      window, 
                         CATWindow*       parent = 0, 
                         CATPOINT*        origin = 0,
                         bool             takeoverWnd = false);

    /// CloseWindow() closes the window from the skin by name.
    ///
    /// \param wndName - name of window to close      
    /// \return CATResult - CAT_SUCCESS on success.
    ///
    /// Returns an error if the window is not open (CAT_ERR_SKIN_WINDOW_NOT_OPEN)
    /// Or CAT_ERR_SKIN_WINDOW_NOT_FOUND if the window does not exist.
    virtual CATResult CloseWindow(const CATString& wndName);      

    // Upwards command from control
    virtual void         OnCommand( CATCommand& command, 
                                    CATControl* ctrl, 
                                    CATWindow*  wnd);

    /// CloseAll() closes all the windows. Remember, this can kill the process
    /// if a window is set to primary... Of course, typically it's called
    /// by the skin's destructor anyway, so that's fine.
    virtual void CloseAll();

    /// Load() loads the skin in.
    virtual CATResult    Load(  CATPROGRESSCB	progressCB   = 0,
                                void*			progressParam= 0,
                                CATFloat32		progMin      = 0.0f,
                                CATFloat32		progMax		 = 1.0f);   

protected:
    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult    ParseAttributes();

    /// RectFromAttribs() recalculates the control's rectangle from
    /// the attributes.  This can only be called after ParseAttributes() has
    /// loaded the images.
    virtual CATResult    RectFromAttribs();

    // MarkDirty() has no meaning for skins currently. Maybe make it force everything
    // to redraw for after a skin change?
    virtual void         MarkDirty(CATRect* dirtyRect = 0, bool force = false) {}
    // Hit Testing skins is pretty useless at the moment as well... maybe
    // later allow full screen coords and all windows? hmmm...
    virtual CATControl*  HitTest(const CATPOINT& pt) { return 0;}


    clock_t     fUpdateTime;

    CATString          fSkinRoot;
    CATString          fSkinPath;
    CATString          fAuthor;
    CATString          fURL;
    CATString          fCopyright;
    CATUInt32          fTemplateCtr;
};

#endif // _CATSkin_H_


