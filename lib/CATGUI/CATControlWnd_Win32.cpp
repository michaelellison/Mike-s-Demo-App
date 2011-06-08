//---------------------------------------------------------------------------
/// \file CATControlWnd_Win32.cpp
/// \brief Window / OS based control class - base object for OS dependant 
/// controls. Win32 function implementations
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

#include "CATControlWnd.h"
#include "CATWindow.h"
#include "CATOSFuncs.h"
#include "CATEventDefs.h"

// Win32-specific hooked window procedure.
LRESULT WINAPI CATControlWnd::CustomControlProc( HWND hWnd, 
                                                UINT message, 
                                                WPARAM wParam, 
                                                LPARAM lParam)
{
    CATControlWnd* theControl = (CATControlWnd*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    bool     handled = false;
    LRESULT   result = 0;

    // Allow children to override any default handling
    // by overriding the OnControlEvent message.
    if (theControl)
    {
		  CATInt32 tmpRes = 0;
        handled = theControl->OnControlEvent(CATEvent(CATEVENT_WINDOWS_EVENT,(CATInt32)hWnd,message,wParam,lParam), tmpRes);      
		  if (handled)
			  result = tmpRes;
    }

    if (!handled)
    {
        switch (message)
        {
        case WM_MOUSELEAVE:
            theControl->fCaptured = false;
            break;

            // Pass hover and movement to parent window for tooltips and the like
        case WM_MOUSEHOVER:
            {
                POINT pt;
                pt.x = (CATInt32)(CATInt16)(LOWORD(lParam));
                pt.y = (CATInt32)(CATInt16)(HIWORD(lParam));
                ::ClientToScreen(hWnd,&pt);
                ::ScreenToClient(theControl->GetWindow()->OSGetWnd(),&pt);
                ::SendMessage(theControl->GetWindow()->OSGetWnd(), WM_MOUSEHOVER,wParam, MAKELONG(pt.x, pt.y) );
                handled = true;
            }
            break;            
        case WM_MOUSEMOVE:
            {

                if ((::GetActiveWindow() != hWnd) && (!theControl->fCaptured))
                {
                    theControl->GetWindow()->SetTrackMouse(hWnd);
                    theControl->fCaptured = true;
                }

                POINT pt;
                pt.x = (CATInt32)(CATInt16)(LOWORD(lParam));
                pt.y = (CATInt32)(CATInt16)(HIWORD(lParam));
                ::ClientToScreen(hWnd,&pt);
                ::ScreenToClient(theControl->GetWindow()->OSGetWnd(),&pt);
                ::SendMessage(theControl->GetWindow()->OSGetWnd(), WM_MOUSEMOVE,wParam, MAKELONG(pt.x, pt.y) );
                handled = true;
            }
            break;
        case WM_CHAR:
            {
                switch (LOWORD(wParam))
                {
                case 0x1b:
                    theControl->OnEscapeChange();
                    theControl->GetWindow()->KillFocus();
                    result   = 1;
                    handled  = true;
                    break;                     

                case 0x0a:
                case 0x0d:
                    theControl->GetWindow()->KillFocus();
                    result   = 1;
                    handled  = true;
                    break;
                case 0x09:
                    {                        
                        bool shift = (GetKeyState(VK_SHIFT) & 0x8000)?true:false;
                        if (shift)
                        {
                            theControl->GetWindow()->SetFocusPrev();
                        }
                        else
                        {
                            theControl->GetWindow()->SetFocusNext();
                        }
                        result   = 1;
                        handled  = true;
                    }
                    break;
                }
            }
            break;
        case WM_KILLFOCUS:
            theControl->fCaptured = false;
            break;

        case WM_SETFOCUS:
            // It already has focus now, but the parent needs to know.
            theControl->GetWindow()->SetFocus(theControl);
            theControl->fCaptured = false;   				
            break;
        }
    }

    if ((theControl != 0) && (handled == false))
    {
        result = ::CallWindowProc((WNDPROC)theControl->fOldWndProc,hWnd,message,wParam,lParam);
    }

    return result;
}


CATResult CATControlWnd::CreateControlWnd( 
    const CATString& windowClass, 
    CATUInt32 style )
{
    // Destroy old window
    if (this->fControlWnd)
    {
        this->GetWindow()->UnRegCtlWnd(fControlWnd);
        this->GetWindow()->OSDestroyWnd(fControlWnd);   
        fControlWnd = 0;
        fOldWndProc = 0;
    }

    CATRect absRect = this->GetRectAbs();

    this->fControlWnd = 
        ::CreateWindow( windowClass,
                        0,
                        style,
                        absRect.left, absRect.top, absRect.Width(), absRect.Height(),
                        this->GetWindow()->OSGetWnd(),
                        (HMENU)0,
                        (HINSTANCE)::GetWindowLongPtr(this->GetWindow()->OSGetWnd(), GWLP_HINSTANCE),
                        0);

    if (fControlWnd)
    {
        fOldWndProc = (void*)::GetWindowLongPtr(fControlWnd, GWLP_WNDPROC);
        
        ::SetWindowLongPtr(fControlWnd, GWLP_WNDPROC, (LONG_PTR)CustomControlProc);
        ::SetWindowLongPtr(fControlWnd, GWLP_USERDATA, (LONG_PTR)this);
        ::SetWindowPos(fControlWnd,0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);      
        
        this->GetWindow()->RegCtlWnd(this,fControlWnd);
        
        ::EnableWindow(fControlWnd,this->IsEnabled());
        return CAT_SUCCESS;
    }
    else
    {
        fOldWndProc = 0;
        return CATRESULT(CAT_ERR_WINDOW_CREATE_FAILED);
    }

}



// member event handler that can be overridden.
bool CATControlWnd::OnControlEvent( const CATEvent& event,  CATInt32& result)                                       
{
    return false;
}

CATResult CATControlWnd::OSCreate()
{
    if (fFGBrush != 0)
    {
        DeleteObject(fFGBrush);
        fFGBrush = 0;
    }

    if (fFGFocBrush != 0)
    {
        DeleteObject(fFGFocBrush);
        fFGFocBrush = 0;
    }

    if (fBGBrush != 0)
    {
        DeleteObject(fBGBrush);
        fBGBrush = 0;
    }

    if (fBGFocBrush != 0)
    {
        DeleteObject(fBGFocBrush);
        fBGFocBrush = 0;
    }

    if (fFGDisBrush != 0)
    {
        DeleteObject(fFGDisBrush);
        fFGDisBrush = 0;
    }

    if (fBGDisBrush != 0)
    {
        DeleteObject(fBGDisBrush);
        fBGDisBrush = 0;
    }


    fFGBrush    = ::CreateSolidBrush(RGB(this->fForegroundColor.r, fForegroundColor.g, fForegroundColor.b));
    fBGBrush    = ::CreateSolidBrush(RGB(this->fBackgroundColor.r, fBackgroundColor.g, fBackgroundColor.b));
    fFGFocBrush = ::CreateSolidBrush(RGB(this->fFGColor_focus.r, fFGColor_focus.g, fFGColor_focus.b));
    fBGFocBrush = ::CreateSolidBrush(RGB(this->fBGColor_focus.r, fBGColor_focus.g, fBGColor_focus.b));
    fFGDisBrush = ::CreateSolidBrush(RGB(this->fFgDisColor.r, fFgDisColor.g, fFgDisColor.b));
    fBGDisBrush = ::CreateSolidBrush(RGB(this->fBgDisColor.r, fBgDisColor.g, fBgDisColor.b));


    if ((fFGBrush != 0) && (fBGBrush != 0) && (fFGFocBrush != 0) && (fBGFocBrush != 0))
    {
        return CAT_SUCCESS;
    }
    else
    {
        return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
    }
}

HBRUSH CATControlWnd::GetBGBrush()
{
    if (this->fFocused)
    {
        return fBGFocBrush;
    }
    else if (this->IsEnabled() == false)
    {
        return fBGDisBrush;
    }
    else
    {
        return fBGBrush;
    }
}

void CATControlWnd::OSCleanup()
{
    this->GetWindow()->UnRegCtlWnd(fControlWnd);

    if (fFGBrush != 0)
    {
        DeleteObject(fFGBrush);
        fFGBrush = 0;
    }

    if (fFGFocBrush != 0)
    {
        DeleteObject(fFGFocBrush);
        fFGFocBrush = 0;
    }

    if (fBGBrush != 0)
    {
        DeleteObject(fBGBrush);
        fBGBrush = 0;
    }

    if (fBGFocBrush != 0)
    {
        DeleteObject(fBGFocBrush);
        fBGFocBrush = 0;
    }

    if (fControlWnd)
    {
        this->GetWindow()->OSDestroyWnd(fControlWnd);   
        fControlWnd = 0;
    }
    fOldWndProc = 0;
}

void CATControlWnd::OSDrawBorderRect(CATDRAWCONTEXT drawContext, CATRect& rect)
{
    RECT wrect = *(RECT*)(CATRECT*)&rect;
    HPEN fgPen = ::CreatePen(PS_SOLID,1, RGB(fFGColor_border.r,fFGColor_border.g,fFGColor_border.b));
    HPEN bgPen = ::CreatePen(PS_SOLID,1,RGB(fBGColor_border.r, fBGColor_border.g, fBGColor_border.b));

    HPEN oldPen = (HPEN)::SelectObject(drawContext,bgPen);

    ::MoveToEx(drawContext,wrect.left,wrect.bottom,0);
    ::LineTo(drawContext,wrect.left,wrect.top);
    ::LineTo(drawContext,wrect.right,wrect.top);

    ::SelectObject(drawContext,fgPen);

    ::LineTo(drawContext,wrect.right,wrect.bottom);
    ::LineTo(drawContext,wrect.left,wrect.bottom);
    ::SelectObject(drawContext,oldPen);

    DeleteObject(fgPen);
    DeleteObject(bgPen);   
}

void CATControlWnd::OSSetText(const CATString& newText)
{
    if (this->fControlWnd)
    {
        ::SetWindowText(fControlWnd, newText);
    }
}

void CATControlWnd::OSGetText(CATString& newText)
{
    if (this->fControlWnd)
    {
        CATUInt32 textLen = ::GetWindowTextLength(fControlWnd);
        if (textLen == 0)
        {
            newText = "";
        }
        else
        {
            ::GetWindowText(fControlWnd,newText.GetUnicodeBuffer(textLen+2), textLen + 1);
            newText.ReleaseBuffer();
        }
    }
}