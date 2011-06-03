//---------------------------------------------------------------------------
/// \file CATControlWnd.cpp
/// \brief Window / OS based control class - base object for OS dependant 
/// controls.
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $
//---------------------------------------------------------------------------

#include "CATControlWnd.h"
#include "CATWindow.h"
#include "CATOSFuncs.h"
#include "CATApp.h"
#include "CATEventDefs.h"

// Constructor - mirrors CATXMLObject() constructor for now.
CATControlWnd::CATControlWnd(  const CATString&               element, 
                             const CATString&               rootDir)
                             : CATControl(element, rootDir)
{
    fControlWnd = 0;
    fFocusSet = false;
    fBorderSet = false;
    fCaptured = false;
#ifdef CAT_CONFIG_WIN32
	 fWindowStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
    fFGBrush = 0;
    fBGBrush = 0;
    fFGDisBrush = 0;
    fBGDisBrush = 0;
    fFGFocBrush = 0;
    fBGFocBrush = 0;
    fOldWndProc = 0;
#endif // CAT_CONFIG_WIN32
}

CATControlWnd::~CATControlWnd()
{
    if (fControlWnd != 0)
    {
        CATASSERT(false,"Parent MUST have called onParentDestroy() previously!");
        this->OnParentDestroy();
    }
}

// SetFocused() sets the control's focused state.
//
// \param focused - if true, then the control is given focus.
//                  if false, focus is removed.      
void CATControlWnd::SetFocused(bool focused)
{
    bool wasFocused = fFocused;

    if (focused)
    {
        if ((fControlWnd) && (!wasFocused))
        {
            this->GetWindow()->OSSetFocus(fControlWnd);
            this->MarkDirty();
        }
    }
    else if (wasFocused)
    {      
        this->GetWindow()->OSSetFocus();
        this->MarkDirty();
    }

    CATControl::SetFocused(focused);
}

// MarkDirty() marks the control as dirty (needing to be redrawn)
//
// \param dirtyRect - if specified, marks only part of the rectangle
// as dirty. Should be in window coordinates - i.e. 0,0 is top left in window,
// not control.
void CATControlWnd::MarkDirty(CATRect* dirtyRect, bool force)
{	
    CATControl::MarkDirty(dirtyRect, force);
    if ((this->IsVisible()) || (force))
    {
        CATRect absRect = this->GetRectAbs();

        CATWND wnd = this->GetWindow()->OSGetWnd();
        if (wnd)
        {  
            CATInvalidateRect(wnd, absRect);
        }
    }

}

// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
void CATControlWnd::Draw(CATImage* image, const CATRect& dirtyRect)
{

}

// Event handler
CATResult CATControlWnd::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    switch (event.fEventCode)
    {
    case CATEVENT_ENABLE_CHANGE:
        {
#ifdef _WIN32
            ::EnableWindow(fControlWnd, this->IsEnabled()?TRUE:FALSE);
#endif
        }   
        break;
    case CATEVENT_TAB_SHOW:
        {
#ifdef _WIN32
            if ((this->fControlWnd) && (fVisible))
            {
                ::ShowWindow(fControlWnd,SW_SHOW);
            }               
#endif
            //this->MarkDirty();
        }
        break;

    case CATEVENT_TAB_HIDE:
#ifdef _WIN32
        if (this->fControlWnd)
        {
            ::ShowWindow(fControlWnd,SW_HIDE);
        }               
#endif
        //this->MarkDirty();
        break;
    }
    return CATControl::OnEvent(event,retVal);
}


void CATControlWnd::SetVisible(bool visible)
{
    fVisible = visible;

    if (fControlWnd)
    {
        ::ShowWindow(fControlWnd, IsVisible()?SW_SHOW:SW_HIDE);		
    }

    MarkDirty();
}

// PostDraw() draws any stuff that requires an OS-specific draw
// context.
void CATControlWnd::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{
    CATControl::PostDraw(drawContext, dirtyRect);
}

// GetPostRects() retrieves a rect, if any, that
// should be reserved for post-draw style drawing.
bool CATControlWnd::GetPostRects(CATStack<CATRect>& rectStack)
{

    rectStack.Push(fWndRect);   
    return true;
}


// OnParentCreate() is called when the parent window is created.
// 
// Most controls won't need this, but any that create their own
// windows should do so at this point.
void CATControlWnd::OnParentCreate()
{
    CATResult result = CAT_SUCCESS;
    if (CATFAILED(result = this->CreateControlWnd(fWindowType, fWindowStyle)))
    {
        if (this->GetWindow() != 0)
            this->GetWindow()->DisplayError(result);
        else
            gApp->DisplayError(result);
    }

    if (CATFAILED(result = OSCreate()))
    {
        if (this->GetWindow() != 0)
            this->GetWindow()->DisplayError(result);
        else
            gApp->DisplayError(result);
    }

    if (fControlWnd)
    {
        ::ShowWindow(fControlWnd,this->IsVisible()?SW_SHOW:SW_HIDE);
    }
}


// OnParentDestroy() is called as the parent window is destroyed.
//
// Controls that create their own windows during OnParentCreate()
// should destroy them during OnParentDestroy()
void CATControlWnd::OnParentDestroy()
{
    if (this->fControlWnd)
    {
        this->GetWindow()->OSDestroyWnd(fControlWnd);
        fControlWnd = 0;

        OSCleanup();
    }
}

// RectFromAttribs() recalculates the control's rectangle from
// the attributes.  This can only be called after ParseAttributes() has
// loaded the images.
CATResult CATControlWnd::RectFromAttribs()
{
    CATResult result = CATControl::RectFromAttribs();

    fWndRect = this->GetRectAbs();

    if (this->fControlWnd)
    {
        this->GetWindow()->OSMoveWnd(fWndRect, fControlWnd);      
    }

    this->MarkDirty();   
    return result;
}
CATResult CATControlWnd::Load(			CATPROGRESSCB				progressCB,
                              void*							progressParam,
                              CATFloat32						progMin,
                              CATFloat32						progMax)
{
    CATResult result = CATControl::Load(progressCB, progressParam, progMin, progMax);
    if (fFocusSet == false)
    {
        this->fBGColor_focus = this->fBackgroundColor;
        this->fFGColor_focus = this->fForegroundColor;
    }

    if (fBorderSet == false)
    {
        this->fFGColor_border.r = 192;
        this->fFGColor_border.g = 192;
        this->fFGColor_border.b = 192;
        this->fBGColor_border.r = 64;
        this->fBGColor_border.g = 64;
        this->fBGColor_border.b = 64;
    }

    this->MarkDirty();   
    return result;
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATControlWnd::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    attrib = GetAttribute(L"ColorForeFocus");
    if (!attrib.IsEmpty())
    {
        // Values are RGB, not RGBA
        CATUInt32 rawColor = attrib.FromHex();
        this->fFGColor_focus.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fFGColor_focus.g = (CATUInt8)((rawColor & 0xff00) >> 8);
        this->fFGColor_focus.b = (CATUInt8)(rawColor & 0xff);
        this->fFGColor_focus.a = 255;
        fFocusSet = true;
    }   

    attrib = GetAttribute(L"ColorBackFocus");
    if (!attrib.IsEmpty())
    {
        // Values are RGB, not RGBA
        CATUInt32 rawColor = attrib.FromHex();
        this->fBGColor_focus.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fBGColor_focus.g = (CATUInt8)((rawColor & 0xff00) >> 8);
        this->fBGColor_focus.b = (CATUInt8)(rawColor & 0xff);
        this->fBGColor_focus.a = 255;      
        fFocusSet = true;
    }

    attrib = GetAttribute(L"ColorForeBorder");
    if (!attrib.IsEmpty())   
    {
        // Values are RGB, not RGBA
        CATUInt32 rawColor = attrib.FromHex();
        this->fFGColor_border.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fFGColor_border.g = (CATUInt8)((rawColor & 0xff00) >> 8);
        this->fFGColor_border.b = (CATUInt8)(rawColor & 0xff);

        this->fFGColor_border.a = 255;
        fBorderSet = true;

    }

    attrib = GetAttribute(L"ColorBackBorder");
    if (!attrib.IsEmpty())   
    {
        // Values are RGB, not RGBA
        CATUInt32 rawColor = attrib.FromHex();
        this->fBGColor_border.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fBGColor_border.g = (CATUInt8)((rawColor & 0xff00) >> 8);
        this->fBGColor_border.b = (CATUInt8)(rawColor & 0xff);
        this->fBGColor_border.a = 255;      
        fBorderSet = true;
    }

    return result;
}

// GetColorFore() retrieves the foreground color for the control
// \return CATColor - foreground color
CATColor CATControlWnd::GetColorFore() const
{
    if (this->fFocused)
    {
        return this->fFGColor_focus;
    }
    else if (this->IsEnabled() == false)
    {  
        return this->fFgDisColor;
    }
    else
    {
        return this->fForegroundColor;
    }
}

// GetColorBack() retrieves the background color for the control
// \return CATColor - background color
CATColor CATControlWnd::GetColorBack() const
{
    if (this->fFocused)
    {
        return this->fBGColor_focus;
    }
    else if (this->IsEnabled() == false)
    {  
        return this->fBgDisColor;
    }
    else
    {
        return this->fBackgroundColor;
    }
}

void CATControlWnd::SetEnabled(bool enabled)
{
    CATGuiObj::SetEnabled(enabled);
#ifdef _WIN32
    if (fControlWnd != 0)
    {
        ::EnableWindow(fControlWnd,this->IsEnabled());
    }
#endif
}

CATString CATControlWnd::GetHint() const
{
    return CATControl::GetHint();
}


