//---------------------------------------------------------------------------
/// \file CATWindow_Win32.cpp
/// \brief Win32 CATWindow implementation
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

// Using new win32 features (post 95)
#include "CATWindow.h"
#include "CATApp.h"
#include "CATEventDefs.h"
#include "CATCursor.h"
#include "CATKeystroke.h"
#include "CATColor.h"
#include "CATControlWnd.h"
#include "CATMenu.h"
#include "CATListBox.h"
#include "CATGuiResDefs.h"
#include "CATOSFuncs.h"

__inline CATPOINT CATLParamToPoint(LPARAM lParam)
{
    CATPOINT pt;
    pt.x = (CATInt32)(CATInt16)(LOWORD(lParam));
    pt.y = (CATInt32)(CATInt16)(HIWORD(lParam));
    return pt;
}



LRESULT CALLBACK CATWindow::WindowProc( HWND hwnd,      // handle to window
                                        UINT uMsg,      // message identifier
                                        WPARAM wParam,  // first message parameter
                                        LPARAM lParam )  // second message parameter
{
    // Add our information into window on create 
    // (don't return yet, we handle the WM_CREATE the normal way too later)
    if (uMsg == WM_CREATE)
    {
        // Store this* for our object in the window
        LPCREATESTRUCT lpc = (LPCREATESTRUCT)lParam;
        SetWindowLong(hwnd,GWL_USERDATA,(LONG)(UINT_PTR)lpc->lpCreateParams);      

        // Store halftone palette in the window as well.
        HDC hdc = GetDC(hwnd);
        HPALETTE palette = CreateHalftonePalette(hdc);
        ReleaseDC(hwnd,hdc);      
        SetWindowLong( hwnd,0, (CATInt32)(UINT_PTR)palette);

        // Flush changes to window data
        SetWindowPos(hwnd,0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);      
    }

    // Retrieve information stored in the window
    CATWindow* theWnd     = (CATWindow*)(UINT_PTR)GetWindowLong(hwnd,GWL_USERDATA);
    HPALETTE  appPalette = (HPALETTE)(UINT_PTR)GetWindowLong(hwnd,0);


    if (theWnd == 0)
    {
        return DefWindowProc(hwnd,uMsg,wParam,lParam);
    }


    // Handle WM's that we wont need to override, convert
    // any we might be interest in into CATEvent's and pass
    // to our OnEvent() handler.   
    bool handled = false;
    LRESULT lRes = 0;

    if (theWnd->fWindow == hwnd)
    {
        switch (uMsg)
        {
            case CATWM_EVENTPOSTED:    theWnd->ProcessPostedEvent(); break;
            case WM_GETOBJECT:     lRes = theWnd->OSGetObject    (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_DROPFILES:     lRes = theWnd->OSDropFiles    (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_SIZE:          lRes = theWnd->OSOnSize       (hwnd, uMsg, wParam, lParam, handled); break;                
            case WM_GETMINMAXINFO: lRes = theWnd->OSOnMinMax     (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_KEYDOWN:       lRes = theWnd->OSOnKeyDown    (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_KEYUP:         lRes = theWnd->OSOnKeyUp      (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_APPCOMMAND:    lRes = theWnd->OSOnAppCmd     (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_PAINT:         lRes = theWnd->OSOnPaint      (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_MEASUREITEM:   lRes = theWnd->OSOnMeasureItem(hwnd, uMsg, wParam, lParam, handled); break;
            case WM_DRAWITEM:      lRes = theWnd->OSOnDrawItem   (hwnd, uMsg, wParam, lParam, handled); break;
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORSCROLLBAR: 
                                lRes = theWnd->OSGetControlColors(hwnd, uMsg, wParam, lParam, handled); break;                                

            case WM_MOUSEWHEEL:
                {
                    CATPOINT    pt       = CATLParamToPoint(lParam);
                    CATMODKEY   modKey   = GetModifierKeys();
                    CATFloat32  rotation = ((CATFloat32)(CATInt16)HIWORD(wParam)) / (CATFloat32)WHEEL_DELTA;

                    ::ScreenToClient(hwnd,(LPPOINT)&pt);

                    theWnd->OnMouseWheel(pt,rotation,modKey);
                }
                break;

            case WM_MOUSEHOVER:
                {
                    CATPOINT pt = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();  

                    bool leftBtn = (wParam & MK_LBUTTON)?true:false;

                    theWnd->OnMouseHover(pt,leftBtn,modKey);
                    
                    SetTrackMouse(hwnd);
                }
                break;
            case WM_MOUSEMOVE:
                {
					 static clock_t last = clock();
					 clock_t cur = clock();
					 if (cur - last < 20)
					 {
						 handled = true;
						 break;
					 }
					 last = cur;
                    if (theWnd->fMouseInWindow == false)
                    {
                        // Setup window to track mouse events      
                        SetTrackMouse(hwnd);
                    }

                    CATPOINT pt = CATLParamToPoint(lParam);

                    if ((pt.x != theWnd->fLastPoint.x) ||
                        (pt.y != theWnd->fLastPoint.y))
                    {
	                     CATMODKEY modKey = GetModifierKeys();               
                        theWnd->fLastPoint = pt;
                        theWnd->OnMouseMove( pt, 
                                             (wParam & MK_LBUTTON)?true:false,
                                             modKey);
                    }
                    handled = true;
                }
                break;

            case WM_MOUSELEAVE:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    CATRect absRect = theWnd->GetRectAbs(true);
                    if (!PtInRect((RECT*)&(CATRECT)absRect,pt))
                    {
                        theWnd->OnMouseLeave();
                    }
                    else
                    {
                        theWnd->fMouseInWindow = false;
                    }
                }
                break;
            case WM_LBUTTONDBLCLK:
                {                    
                    CATPOINT  pt     = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();
                    theWnd->OnMouseDoubleClick(pt,modKey);
                    handled = true;
                }
                break;
            case WM_LBUTTONDOWN: 
                {
                    ::SetFocus(theWnd->fWindow);
                    ::SetTimer(hwnd, CATWM_MOUSETIMER, kMOUSEFILTERFREQ, 0);
                    CATPOINT  pt     = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();
                    theWnd->OnMouseDown(pt, modKey);
                    handled = true;
                }
                break;
            case WM_RBUTTONDOWN:
                {
                    CATPOINT  pt     = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();
                    theWnd->OnRightMouseDown(pt, modKey);
                    handled = true;
                }
                break;
            case WM_RBUTTONUP:
                {
                    CATPOINT  pt     = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();
                    theWnd->OnRightMouseUp(pt, modKey);
                    handled = true;
                }
                break;
            case WM_LBUTTONUP:
                {
                    ::KillTimer(hwnd, CATWM_MOUSETIMER);            
                    CATPOINT  pt     = CATLParamToPoint(lParam);
                    CATMODKEY modKey = GetModifierKeys();
                    theWnd->OnMouseUp(pt,modKey);
                    handled = true;
                }
                break;      
            case WM_MOVE:
                {
                    CATPOINT pt = CATLParamToPoint(lParam);
                    // WM_MOVE is client position, so we're good to go.
                    theWnd->OnMove(pt);
                    handled = true;
                }
                break;
                //-------------------------------------------------------
                //-------------------------------------------------------
            case WM_SETCURSOR:
                {
                    if (LOWORD(lParam) == HTCLIENT)
                    {

                        SetCursor(theWnd->fCursor.GetOSCursor());
                        handled = true;
                        lRes    = 1;
                    }
                }
                break;
                //-------------------------------------------------------
            case WM_SYSCHAR:
            case WM_SYSCOMMAND:
                break;
            case WM_CHAR:
                {
                    CATMODKEY modKey = GetModifierKeys();               
                    theWnd->OnKeyPress(CATKeystroke(wParam,CATKEY_NONE,modKey));
                    handled = true;
                }
                break;

            case WM_TIMER:
                {
                    switch (wParam)
                    {
                        case CATWM_SCANNINGTIMER:
                            theWnd->OnScanningTimer();
                            lRes    = 0;
                            handled = true;
                            break;
                        case CATWM_MOUSETIMER:
                            {
                                CATMODKEY modKey = GetModifierKeys();
                                theWnd->OnMouseTimer(modKey);
                                lRes = 0;
                                handled = true;
                            }  
                            break;
                        case CATWM_UPDATETIMER:
                            {
                                theWnd->OnWindowTimer();
                            }
                            break;
                    }
                }
                break;

            case WM_SETFOCUS:
                theWnd->ResumeScanning();
                break;

            case WM_GETDLGCODE :
                lRes = DLGC_WANTALLKEYS;
                handled = true;
                break;   

            case WM_MOVING:
                {
                    RECT* moveRect = (RECT*)lParam;
                    CATRect checkRect( moveRect->left,
                        moveRect->top, 
                        moveRect->right, 
                        moveRect->bottom);

                    // Framework really only knows about client area, and WM_MOVING is in
                    // non-client coords, so we need to give it the actual window rect for 
                    // comparisons.

                    CATRect wndRect = theWnd->OSGetWndRect();

                    // If the window changes the move rect and returns true, set
                    // the move rect to the changes.
                    if (theWnd->OnMoving(checkRect,wndRect))
                    {
                        moveRect->left = checkRect.left;
                        moveRect->top = checkRect.top;
                        moveRect->right = checkRect.right;
                        moveRect->bottom = checkRect.bottom;
                        lRes = 1;
                        handled = true;
                    }
                }
                break;
                //-------------------------------------------------------
                // When focus is killed, it may be to a child control.
                // We should set that control as focused if it isn't.
            case WM_KILLFOCUS:
                {    
                    if ((CATWND)wParam != theWnd->fWindow)
                    {
                        theWnd->PauseScanning();
                    }

                    std::map<CATWND,CATControlWnd*>::iterator iter = theWnd->fCWT.find((CATWND)wParam);
                    if (iter != theWnd->fCWT.end())
                    {
                        CATControlWnd* curControl = iter->second;
                        theWnd->SetFocus(curControl);
                    }                    
                }  
                break;
                //-------------------------------------------------------
            case WM_CREATE:
                {
                    handled = true;
                    lRes    = 0;
                }
                break;
                //-------------------------------------------------------
            case WM_QUERYNEWPALETTE:
                if ((HWND)wParam == theWnd->fWindow)
                {			
                    HDC hDC = ::GetDC(theWnd->fWindow);
                    ::SelectPalette(hDC,appPalette,FALSE);
                    ::RealizePalette(hDC);
                    ::InvalidateRect(theWnd->fWindow,0,FALSE);
                    ::ReleaseDC(theWnd->fWindow,hDC);
                    handled  = true;
                    lRes     = 1;
                }
                break;
                //-------------------------------------------------------
            case WM_PALETTECHANGED:
                {
                    HDC hDC = ::GetDC(theWnd->fWindow);
                    ::SelectPalette(hDC,appPalette,FALSE);            

                    if (wParam != (DWORD)theWnd->fWindow)
                    {
                        ::RealizePalette(hDC);
                    }

                    ::UpdateColors(hDC);
                    ::ReleaseDC(theWnd->fWindow,hDC);				
                    handled  = true;
                    lRes     = 0;
                }
                break;

            case WM_DESTROY:
                theWnd->fOSDestruction = true;
                theWnd->OnDestroy();                  
                DeleteObject(appPalette);
                handled  = true;
                lRes     = 0;
                break;

                //-------------------------------------------------------
            case WM_ERASEBKGND:
                handled  = true;
                lRes     = TRUE;
                break;            
            //-------------------------------------------------------

        }   
    }

    if (!handled)
    {
        CATInt32 eventResult = (CATInt32)lRes;
        CATResult result = theWnd->OnEvent(CATEvent(CATEVENT_WINDOWS_EVENT,(CATInt32)hwnd,uMsg,wParam,lParam), eventResult);
        if (CATFAILED(result))
        {
            theWnd->DisplayError(result);
        }
        if (eventResult == 0)
        {
            lRes = ::DefWindowProc(hwnd,uMsg,wParam,lParam);
        }
        else
        {
            lRes = (LRESULT)eventResult;
        }
    }

    return lRes;
}

// RegWindow registers a window class if needed,
// providing a string to reference it in the future.
// \return CATResult - CAT_SUCCESS on success.
CATResult CATWindow::RegWindow()
{
    // Setup struct for registering class
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));


    wc.cbClsExtra     = 0;                 // No class data right now..
    wc.cbWndExtra     = 4;  // Reserve space to store HPALETTE in the window
    wc.style		  = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc	  = WindowProc;
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wc.hInstance	  = gApp->GetInstance();

    // Convert fIcon into an HICON rather than a raw image...

    HICON curIcon = 0;

    if (fIconPath.IsEmpty() == false)
    {
        CATString fullPath = gApp->GetSkin()->GetRootDir() + fIconPath;
        fUserIcon = curIcon = (HICON)::LoadImage(0,fullPath,IMAGE_ICON,0,0,LR_DEFAULTSIZE | LR_LOADFROMFILE);
    }

    if (curIcon == 0)
    {
        curIcon = LoadIcon(gApp->GetInstance(),MAKEINTRESOURCE(IDI_CATAPPICON));
    }

    wc.hIcon			= curIcon;
    wc.hCursor			= fCursor.GetOSCursor();	   
    wc.lpszClassName	= fClassName.GetUnicodeBuffer();   

    // Purposefully leaving fClassName locked....
    ATOM resultClass = RegisterClass(&wc);
    fClassName.ReleaseBuffer();

    if (resultClass != 0)
    {
        return CAT_SUCCESS;
    }

    return CAT_ERR_WINDOW_CLASSREG_FAILED;
}

void CATWindow::CATUnregWindow()
{
    CATASSERT(fRegistered == true, "Only unreg if already registered.");
    ::UnregisterClass(fClassName,gApp->GetInstance());

    if (fUserIcon)
    {
#ifdef _WIN32
        ::DestroyIcon(fUserIcon);
        fUserIcon = 0;
#endif
    }

}

CATResult CATWindow::OSTakeoverWnd(CATWindow* parentWnd)
{   
    if ((!parentWnd) || (!parentWnd->fWindow))
        return CAT_ERROR;

    fOSDestruction = false;

    bool scanning = parentWnd->IsScanning();    
    PauseScanning();

    this->fWindow = parentWnd->fWindow;
    fPrevWndOwner = parentWnd;
    
    SetWindowLong(fWindow,GWL_USERDATA,(LONG)(UINT_PTR)this);

    parentWnd->fWindow = 0;
    parentWnd->fVisible = false;
    parentWnd->fEnabled = false;

    
    if (fTitle.IsEmpty() == false)
    {
        ::SetWindowText(fWindow,fTitle);
    }
    else
    {
        ::SetWindowText(fWindow,fName);
    }

    CATRect realRect = this->OSGetRealRect();
    RECT rect;
    ::GetWindowRect(fWindow,&rect);
    realRect.SetOrigin(rect.left,rect.top);
    this->OSMoveWnd(realRect,fWindow);
    
    this->OnCreate();
    this->SetVisible(true);
    this->SetEnabled(true);
    this->OSSetFocus(fWindow);
    this->SetFocusFirst();
    this->EnableScanning(scanning,parentWnd->fScanRate);

    return CAT_SUCCESS;
}


CATResult CATWindow::OSRestoreTakenWnd()
{
    if (!fPrevWndOwner)
        return CAT_ERROR;
    
    fPrevWndOwner->fWindow = fWindow;
    fPrevWndOwner->fVisible = true;

    PauseScanning();

    SetWindowLong(fWindow,GWL_USERDATA,(LONG)(UINT_PTR)fPrevWndOwner);    

    CATRect realRect = fPrevWndOwner->OSGetRealRect();
    RECT rect;
    ::GetWindowRect(fWindow,&rect);
    realRect.SetOrigin(rect.left,rect.top);
    fPrevWndOwner->OSMoveWnd(realRect,fWindow);

    fWindow  = 0;
    fVisible = false;
    fEnabled = false;

    if (fPrevWndOwner->fTitle.IsEmpty() == false)
    {
        ::SetWindowText(fPrevWndOwner->fWindow,fPrevWndOwner->fTitle);
    }
    else
    {
        ::SetWindowText(fPrevWndOwner->fWindow,fPrevWndOwner->fName);
    }
    
    
    fPrevWndOwner->SetVisible(true);
    fPrevWndOwner->SetEnabled(true);        
    
    if (fOSDestruction)
    {
        fPrevWndOwner->fOSDestruction = true;
        fPrevWndOwner->OnDestroy();
        fPrevWndOwner = 0;
        return CAT_SUCCESS;
    }
    

    fPrevWndOwner->Update();
    OSSetFocus(fPrevWndOwner->fWindow);
    fPrevWndOwner->SetFocusFirst();
    fPrevWndOwner->ResumeScanning();

    fPrevWndOwner = 0;
    
    OnDestroy();
    return CAT_SUCCESS;
}

CATResult CATWindow::OSCreateWnd( CATWND parentWnd)
{
    fOSDestruction = false;

    CATASSERT(fWindow == 0, "Window already created... call ShowWnd() instead.");

    CATUInt32 style    = OSGetWindowStyle();
    CATRect   realRect = OSGetRealRect();


    fWindow = ::CreateWindowEx( WS_EX_ACCEPTFILES,
                                fClassName,
                                fName,
                                style,
                                realRect.left,
                                realRect.top,
                                realRect.right - realRect.left,
                                realRect.bottom - realRect.top,
                                parentWnd, 
                                0, 
                                gApp->GetInstance(),
                                this);

    ::SetFocus(fWindow);
    this->SetTrackMouse(fWindow);

    fVisible = true;

    if (this->IsOverlapped() == false)
    {    
        if (fTitle.IsEmpty() == false)
        {
            ::SetWindowText(fWindow,fTitle);
        }
        else
        {
            ::SetWindowText(fWindow,fName);
        }
    }

    if (fWindow)
    {
		  if (fSetRegion && fImage)
		  {
				HRGN rgn = RegionFromImage(fImage);
				::SetWindowRgn(this->fWindow,rgn,TRUE);
		  }


        this->OnCreate();
        // Show and update the window      
        return CAT_SUCCESS;
    }
    else
    {
        CATASSERT(false,"Error creating window!");
    }

    return CATRESULT(CAT_ERR_WINDOW_CREATE_FAILED);

}

void CATWindow::OSShowWnd()
{
    CATASSERT(fWindow != 0, "Call CreateWnd instead.");

    ShowWindow(fWindow, SW_SHOW);   
    UpdateWindow(fWindow);   
    fVisible = true;
}


void CATWindow::OSHideWnd()
{
    if (fWindow)
    {
        ShowWindow(fWindow,SW_HIDE);
    }
    fVisible = false;
}

void CATWindow::OSDestroyWnd(CATWND wnd)
{
    if (wnd == 0)
    {
        if (fWindow != 0)
        {
            ::DestroyWindow(fWindow);
            fWindow = 0;
        }
    }
    else
    {
        ::DestroyWindow(wnd);
    }
}

void CATWindow::OSDraw(CATDRAWCONTEXT drawContext, CATRect* dirtyRect)
{
    // Figure out our drawing rectangle
    CATRect drawRect(this->fRect); 

    if ((dirtyRect != 0) && (dirtyRect->Width() != 0) && (dirtyRect->Height() != 0))
    {
        drawRect = *dirtyRect;
    }

    // Copy over dirty area of image copy
    CleanBackground(&drawRect);   

    // Draw the normal controls to a copy of our image
    this->Draw(fImageCopy, drawRect);


    // Create a background device context for double buffering the draw
    HDC imageDC = CreateCompatibleDC(drawContext);  

    // Create a bitmap from the dirty part of our image
    // Create a 24-bit dib section we can modify...
    BITMAPINFO bmpInfo;   
    CATUInt8* bmpBits = 0;
    memset(&bmpInfo,0,sizeof(bmpInfo));
    
    bmpInfo.bmiHeader.biSize		 = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biPlanes	     = 1;
    bmpInfo.bmiHeader.biWidth        = drawRect.Width();
    bmpInfo.bmiHeader.biHeight       = -drawRect.Height();
    bmpInfo.bmiHeader.biBitCount     = 24;
    bmpInfo.bmiHeader.biSizeImage    = 0;
    bmpInfo.bmiHeader.biCompression  = BI_RGB;
    
    HBITMAP imageBmp = CreateDIBSection(imageDC, &bmpInfo, DIB_RGB_COLORS, (void**)&bmpBits, 0, 0);
    
    CATASSERT(imageBmp != 0, "Failed to create DIB for window drawing.");

    CATInt32 widthBytes = (((drawRect.Width() *3) + 3) / 4 ) * 4;

    if (bmpBits != 0)
    {
        fImageCopy->CopyOutBGR( bmpBits,
                                drawRect.left, 
                                drawRect.top, 
                                drawRect.Width(), 
                                drawRect.Height(),
                                widthBytes);
    }

    // Select it into context
    SelectObject(imageDC, imageBmp);      

    // Let controls that need a device context draw to the background device   
    SetViewportOrgEx(imageDC, -drawRect.left, -drawRect.top, 0);
    this->PostDraw(imageDC, drawRect);
    SetViewportOrgEx(imageDC, 0, 0, 0);

    // Exclude post-draw controls
    HRGN curRgn  = 0;
    HRGN tmpRgn  = 0;
    HRGN orgRgn  = 0;
    if (fPostRegion != 0)
    {		
        curRgn = CreateRectRgn( 0, 0, 0, 0);      
        orgRgn = CreateRectRgn( 0, 0, 0, 0);
        tmpRgn = CreateRectRgn( drawRect.left,
                                drawRect.top,
                                drawRect.right,
                                drawRect.bottom);

        CombineRgn(curRgn,tmpRgn,fPostRegion,RGN_DIFF);		
        ::GetClipRgn(drawContext,orgRgn);
        SelectObject(drawContext,curRgn);		

    }

    // Blt our image into the background context
    // Using StretchBlt for halftoning.  It's slower, but looks *so*
    // much better.
    StretchBlt( drawContext,
                drawRect.left,
                drawRect.top,
                drawRect.Width(), 
                drawRect.Height(),
                imageDC,
                0,
                0,
                drawRect.Width(),
                drawRect.Height(),
                SRCCOPY);

    // Free DC and bitmap
    DeleteDC(imageDC);
    DeleteObject(imageBmp);


    // Restore regions
    if (curRgn != 0)
    {
        // Clean up
        SelectClipRgn(drawContext,orgRgn);
        DeleteObject(orgRgn);
        DeleteObject(tmpRgn);
        DeleteObject(curRgn);
    }

}

//---------------------------------------------------------------------------
// Generate region from a series of rectangles for clipping.
//---------------------------------------------------------------------------
CATOSREGION CATWindow::OSGenRegion(CATStack<CATRect>& rectStack)
{
    CATOSREGION newRgn = CreateRectRgn(0,0,0,0);

    CATRect curRect;
    while (CATSUCCEEDED(rectStack.Pop(curRect)))
    {
        CATOSREGION tmpRgn = ::CreateRectRgn(curRect.left, curRect.top, curRect.right, curRect.bottom);
        ::CombineRgn(newRgn,newRgn,tmpRgn,RGN_OR);
        ::DeleteObject(tmpRgn);
    }

    return newRgn;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::OSFreeRegion(CATOSREGION region)
{
    if (region)
    {
        ::DeleteObject(region);
    }
}
//---------------------------------------------------------------------------
void CATWindow::OSBeginMouseTrack()
{
    SetCapture(this->fWindow);
}
//---------------------------------------------------------------------------
void CATWindow::OSEndMouseTrack()
{
    ReleaseCapture();
}
//---------------------------------------------------------------------------
void CATWindow::OSSetMouse(const CATPOINT& point, CATCursor& cursor)
{
    POINT pt;
    pt.x = point.x;
    pt.y = point.y;
    ::ClientToScreen(this->fWindow,&pt);
    ::SetCursorPos(pt.x,pt.y);
    if (cursor.GetType() == CATCURSOR_HIDE)
    {
        cursor.HideCursor();
    }
    else
    {
        cursor.ShowCursor();
    }   
}
//---------------------------------------------------------------------------
void CATWindow::Update()
{
    if (this->fVisible)
    {
        ::UpdateWindow(this->fWindow);
    }
}

//---------------------------------------------------------------------------
void CATWindow::OSDrawCursor(CATDRAWCONTEXT context, const CATPOINT& point, CATCursor& cursor)
{
    DrawIcon(	context,
        point.x,
        point.y,
        cursor.GetOSCursor());
}

void CATWindow::PostDrawText( const CATString&    text,
                              CATDRAWCONTEXT      context,
                              const CATRect&      textRect,
                              const CATRect&      dirtyRect,
                              const CATColor&     tColor,
                              const CATString&    fontName,
                              CATFloat32          fontSize,
                              bool                multiline,
                              CATColor*           bgColor,
                              bool                italics,
                              bool				  centered,
                              bool                autoSize)
{
    CATString filtered = CATControl::FilterGUIString(text);

    // Right now we're redrawing the entire string over itself when redrawing
    // any portion of it.  This isn't optimal - should create a clip region at
    // least.  However, we're in a ViewPort mode for the confiltered we're
    // passed, so be careful if this is implemented.
    CATRect drawRect;
    if (textRect.Intersect(dirtyRect, &drawRect))
    {      
        // Optimize - move this elsewhere so fonts are precreated!!!!
        HFONT font    = 0;
        HFONT oldFont = 0;

        if (!autoSize)
            font = this->OSGetFont(fontName,fontSize,false,italics);
        else
            font = this->AutoFontSize(filtered,                                     
                                      textRect,
                                      fontName,
                                      fontSize,
                                      false,
                                      italics);

        oldFont = (HFONT)::SelectObject(context,font);


        RECT rect = *(RECT*)(CATRECT*)&textRect;

        // Convert colors
        COLORREF textColor = RGB( tColor.r ,tColor.g, tColor.b);

        // Setup colors and background mode
        int oldMode       = ::SetBkMode(context,bgColor?OPAQUE:TRANSPARENT);
        if (bgColor != 0)
        {
            HBRUSH fillBrush = ::CreateSolidBrush(RGB(bgColor->r,bgColor->g,bgColor->b));
            ::FillRect(context,&rect,fillBrush);
            DeleteObject(fillBrush);
            ::SetBkColor(context,RGB(bgColor->r,bgColor->g,bgColor->b));
        }

        COLORREF oldColor = ::SetTextColor(context,textColor);

        // Select our font

        // Draw
        DWORD textStyle = DT_VCENTER | DT_END_ELLIPSIS | ((multiline)?DT_WORDBREAK:DT_SINGLELINE) ;
        if (centered)
        {
            textStyle |= DT_CENTER;
        }
        else
        {
            textStyle |= DT_LEFT;
        }

        // Fudge factor for readability
        rect.left += 2;

        ::DrawTextEx(context, filtered.GetUnicodeBuffer(), filtered.LengthCalc(), &rect, textStyle, NULL);
        filtered.ReleaseBuffer();

        // Restore the colors and background
        ::SetTextColor(context,oldColor);
        ::SetBkMode(context, oldMode);

        // Clean up font (see note!)
        if (oldFont != 0)
        {
            ::SelectObject(context,oldFont);
        }

        if (font != 0)
        {
            OSReleaseFont(font);
        }

    }
}

void CATWindow::OSSetFocus(CATWND window)
{
    if (window == 0)
    {
        window = this->fWindow;
    }
    ::SetFocus(window);
}

// OSMoveWnd() moves and/or resizes the specified window (or the base window)
// to the requested rectangle.
// 
// If window == 0, then the base window is used.
// \param rect - new window rect
// \param wnd - the window, or 0 for base
void CATWindow::OSMoveWnd(const CATRect& rect, CATWND wnd)
{
    if (wnd == 0)
    {
        wnd = this->fWindow;
    }

    HWND parent = 0;
    // IF we're a child window, we need to handle moving the parent, not us.
    if (fIsOverlapped)
    {
        RECT wndRect;
        CATRect absRect = GetRectAbs(true);
        POINT tl = {absRect.left, absRect.top};
        POINT br = {absRect.right, absRect.bottom};

        parent = ::GetParent(fWindow);
        ::ClientToScreen(fWindow,&tl);
        ::ClientToScreen(fWindow,&br);
        ::GetWindowRect(parent,&wndRect);

        wndRect.left   += (rect.left  - tl.x);
        wndRect.top    += (rect.top   - tl.y);
        wndRect.right  += (rect.right - br.x);
        wndRect.bottom += (rect.bottom - br.y);

        ::SetWindowPos(parent,0,wndRect.left,wndRect.top,wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, SWP_NOZORDER);

    }
    else
    {           
        ::SetWindowPos(wnd,0,rect.left, rect.top, rect.Width(), rect.Height(),SWP_NOZORDER);      
    }
}

void CATWindow::CenterWindow(CATWindow* parent)
{
    if (parent == 0)
    {
        return;
    }

    CATRect parRect;
    CATRect newRect;

    parRect = parent->OSGetWndRect();
    newRect = this->GetRect();

    ::AdjustWindowRect((RECT*)&parRect, parent->OSGetWindowStyle(), FALSE);
    ::AdjustWindowRect((RECT*)&newRect, OSGetWindowStyle(), FALSE);

    newRect.SetOrigin( parRect.left + (parRect.Width()  - newRect.Width())/2,
                       parRect.top  + (parRect.Height() - newRect.Height()) / 2);

    ::SetWindowPos(this->fWindow,0, newRect.left, newRect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

CATRect CATWindow::OSGetDockRect()
{
    CATRect rect;

    if (fIsOverlapped)
    {
        HWND parent = ::GetParent(this->fWindow);
        RECT clientRect;
        ::GetClientRect(parent,&clientRect);
        POINT tl;
        tl.x        = clientRect.left;
        tl.y        = clientRect.top;
        ::ClientToScreen(parent,&tl);
        rect.left   = tl.x;
        rect.top    = tl.y;
        rect.right  = tl.x + (clientRect.right - clientRect.left);     
        rect.bottom = tl.y + (clientRect.bottom - clientRect.top);
    }
    else
    {  
        CATRect absRect = GetRectAbs(true);
        RECT ourRect;
        SetRect(&ourRect,absRect.left, absRect.top, absRect.right, absRect.bottom);
        HMONITOR monitor = MonitorFromRect(&ourRect,MONITOR_DEFAULTTONEAREST);
        MONITORINFO monInfo;
        monInfo.cbSize = sizeof(monInfo);
        GetMonitorInfo(monitor, &monInfo);
        rect.Set(monInfo.rcWork.left, monInfo.rcWork.top, monInfo.rcWork.right, monInfo.rcWork.bottom);
    }

    return rect;
}

CATRect CATWindow::OSGetWndRect()
{
    RECT wRect;
    
    GetWindowRect(this->fWindow,&wRect);

    return CATRect( wRect.left, 
                    wRect.top, 
                    wRect.right, 
                    wRect.bottom);
}


/// OSWndToScreen() converts x/y coordinates from within the window to screen
/// coordinates.
/// \param x - x position to convert
/// \param y - y position to convert
void CATWindow::OSWndToScreen( CATInt32& x, CATInt32& y)
{
    POINT point;
    point.x = x;
    point.y = y;
    ::ClientToScreen(this->fWindow, &point);
    x = point.x;
    y = point.y;
}

/// OSScreenToWnd() converts x/y coordinates from within the screen to within
/// the window.
/// \param x - x position to convert
/// \param y - y position to convert
void CATWindow::OSScreenToWnd( CATInt32& x, CATInt32& y)
{
    POINT point;
    point.x = x;
    point.y = y;
    ::ScreenToClient(this->fWindow, &point);
    x = point.x;
    y = point.y;
}

void CATWindow::OSSetCursor()
{
    ::SetCursor(fCursor.GetOSCursor());
}

void CATWindow::OSSetCursor(CATCursor& cursor)
{
    ::SetCursor(cursor.GetOSCursor());
    if (cursor.GetType() == CATCURSOR_HIDE)
    {
        cursor.HideCursor();
    }
    else
    {
        cursor.ShowCursor();
    }   
}

void CATWindow::OSMinimize()
{
    ::ShowWindow(this->fWindow, SW_MINIMIZE);
}

void CATWindow::OSMaximize()
{
    if ((this->fSizeable == true) && (fMaximized == false))
    {
        ::ShowWindow(this->fWindow, SW_MAXIMIZE);
        fMaximized = true;
    }
    else
    {
        ::ShowWindow(this->fWindow, SW_RESTORE);
        fMaximized = false;
    }
}

void CATWindow::SetActive()
{
    ::SetActiveWindow(fWindow);
}


void CATWindow::SetTrackMouse(CATWND hwnd)
{
    TRACKMOUSEEVENT eventTrack;
    memset(&eventTrack,0,sizeof(eventTrack));
    eventTrack.hwndTrack = hwnd;
    eventTrack.cbSize    = sizeof(eventTrack);
    eventTrack.dwFlags   = TME_QUERY;
    if (!TrackMouseEvent(&eventTrack))
    {
        CATASSERT(false,"TrackMouseEvent() failed.");
    }

	 eventTrack.dwHoverTime = 1000;
    eventTrack.hwndTrack = hwnd;
    eventTrack.dwFlags   |= TME_HOVER | TME_LEAVE;
    if (!TrackMouseEvent(&eventTrack))
    {
        CATASSERT(false,"TrackMouseEvent() failed.");
    }
}

CATUInt32 CATWindow::OSGetWindowStyle()
{
    CATUInt32 style = 0;
	 if (fSetRegion)
	 {
		 style = WS_POPUP;
	 }
    else if (fIsOverlapped)
    {
        style = WS_CHILD | WS_CLIPSIBLINGS;
    }
    else if (fCaptionBar)
    {
        style = WS_POPUPWINDOW | WS_CAPTION | WS_CLIPCHILDREN | WS_MINIMIZEBOX;

        if (fSizeable)
        {      
            style |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME;
        }
    }
    else
    {
        style = WS_POPUPWINDOW;   

        if (fSizeable)
        {
            style |= WS_THICKFRAME;
        }
    }


    return style;
}


CATFONT CATWindow::OSGetFont(  const CATString& fontName,
                                CATFloat32      fontSize,
                                bool            bold,
                                bool            italic,
                                bool            underlined)
{
    CATFONT font = 0;

    HDC context = ::GetDC(this->fWindow);   
    if (fontName.IsEmpty() == false)
    {
        font = CreateFont(
                            -MulDiv((int)fontSize, GetDeviceCaps(context, LOGPIXELSY), 72),
                            0,
                            0,
                            0,
                            bold?FW_BOLD:FW_NORMAL,
                            italic?TRUE:FALSE, //italic
                            underlined?TRUE:FALSE, //underline
                            FALSE, //strike
                            ANSI_CHARSET, // charset
                            OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY,
                            DEFAULT_PITCH,
                            fontName);         

        // Kludge to force parameters for font even if used on screen DC
        HFONT oldFont = (HFONT)SelectObject(context,font);
        ::SelectObject(context,oldFont);
    }
    else
    {
        font = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);         
    }

    ::ReleaseDC(fWindow,context);
    return font;
}

CATFONT CATWindow::AutoFontSize(   const CATString& text,
                                   CATRect          rect,
                                   const CATString& fontName,
                                   CATFloat32       fontSize,
                                   bool             bold,
                                   bool             italic,
                                   bool             underlined)
{
    // Ugh... implement later.
    CATFONT font = OSGetFont(fontName,fontSize,bold,italic,underlined);
    return font;
}


void CATWindow::OSReleaseFont( CATFONT& font)
{
    ::DeleteObject(font);
    font = 0;
}

void CATWindow::PostEventNotification()
{
    ::PostMessage( this->OSGetWnd(), CATWM_EVENTPOSTED, 0, 0);
}

void CATWindow::SetWindowTimer	(CATFloat32 frequencyHz)
{
    CATUInt32 msec = (CATUInt32)(1000.0f / frequencyHz);
    ::SetTimer(this->OSGetWnd(), CATWM_UPDATETIMER, msec, 0);
}

void CATWindow::KillWindowTimer()
{
    ::KillTimer(this->OSGetWnd(), CATWM_UPDATETIMER);
}

CATWND CATWindow::GetBaseWindow()
{
    if (this->fIsOverlapped == false)
    {
        return this->OSGetWnd();
    }

    HWND rootWnd = this->OSGetWnd();
    while (rootWnd != 0)
    {
        if (::GetWindowLong(rootWnd, GWL_STYLE) & WS_CAPTION)
        {
            // Found caption bar - could be MDI or what-not, but it's our base
            return rootWnd;
        }

        if (!::GetParent(rootWnd))
            return rootWnd;

        rootWnd = ::GetParent(rootWnd);
    }

    // Hit null parent... just return our own window.
    return OSGetWnd();
}

void CATWindow::OSCmdThread(void *param, CATThread* thread)
{
    CATWindow* wnd = (CATWindow*)param;
    if (wnd != 0)
    {
        wnd->ThreadedCmdThread();
    }    
}

void CATWindow::OSStartCmdThread()
{
	fCmdThread.StartProc(OSCmdThread,this);
}

void CATWindow::OSWaitOnCmdThread()
{	
    fExitThread = true;
    fThreadedCmdLock.Wait();
    CATTRACE("Posting exit for window");
    fThreadedCmdSignal.Fire();
    fThreadedCmdLock.Release();

	 
    if (!fCmdThread.WaitStop(10000))
    {
		 fCmdThread.ForceStop();
       CATASSERT(false,"WARNING: had to terminate window command thread!");
    }
    
    fExitThread = false;
}

LRESULT CATWindow::OSOnSize(   HWND hwnd,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam,
                               bool& handled)
{
    CATRect newRect;
    RECT wndRect;
    ::GetClientRect(hwnd, (RECT*)(CATRECT*)&newRect);
    ::GetWindowRect(hwnd,&wndRect);

    CATRect testRect = newRect;

    // If we're a top-level window, adjust for borders.
    if (fIsOverlapped == false)
    {
        ::AdjustWindowRect( (RECT*)(CATRECT*)&testRect,  OSGetWindowStyle(), FALSE);
    }

    CATInt32 xOff = newRect.left - testRect.left;
    CATInt32 yOff = newRect.top - testRect.top;

    newRect.Offset(wndRect.left + xOff,wndRect.top + yOff);

    CATPOINT offsetPt;
    offsetPt.x = newRect.left;
    offsetPt.y = newRect.top;
    OnMove(offsetPt);

    newRect.ZeroOrigin();
    OnSize(newRect);
    handled = true;
    return 0;
}
LRESULT CATWindow::OSOnKeyUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    CATMODKEY modKey = GetModifierKeys();

    switch (wParam)
    {
        case VK_SPACE:    OnKeyUp(CATKeystroke(0x20,CATKEY_NONE,modKey)); break;
        case VK_LEFT:     OnKeyUp(CATKeystroke(0,CATKEY_LEFT,   modKey)); break; 
        case VK_RIGHT:    OnKeyUp(CATKeystroke(0,CATKEY_RIGHT,  modKey)); break;                    
        case VK_UP:       OnKeyUp(CATKeystroke(0,CATKEY_UP,     modKey)); break;                    
        case VK_DOWN:     OnKeyUp(CATKeystroke(0,CATKEY_DOWN,   modKey)); break;                    
        case VK_HOME:     OnKeyUp(CATKeystroke(0,CATKEY_HOME,   modKey)); break;                    
        case VK_END:      OnKeyUp(CATKeystroke(0,CATKEY_END,    modKey)); break;
        case VK_NEXT:     OnKeyUp(CATKeystroke(0,CATKEY_NEXT,   modKey)); break;
        case VK_PRIOR:    OnKeyUp(CATKeystroke(0,CATKEY_PRIOR,  modKey)); break;
        case VK_INSERT:   OnKeyUp(CATKeystroke(0,CATKEY_INSERT, modKey)); break;
        case VK_DELETE:   OnKeyUp(CATKeystroke(0,CATKEY_DELETE, modKey)); break;
        case VK_F2:       OnKeyUp(CATKeystroke(0,CATKEY_F2,     modKey)); break;
        case VK_F3:       OnKeyUp(CATKeystroke(0,CATKEY_F3,     modKey)); break;
        case VK_F4:       OnKeyUp(CATKeystroke(0,CATKEY_F4,     modKey)); break;
        case VK_F5:       OnKeyUp(CATKeystroke(0,CATKEY_F5,     modKey)); break;
        case VK_F6:       OnKeyUp(CATKeystroke(0,CATKEY_F6,     modKey)); break;
        case VK_F7:       OnKeyUp(CATKeystroke(0,CATKEY_F7,     modKey)); break;
        case VK_F8:       OnKeyUp(CATKeystroke(0,CATKEY_F8,     modKey)); break;
        case VK_F9:       OnKeyUp(CATKeystroke(0,CATKEY_F9,     modKey)); break;
        case VK_F10:      OnKeyUp(CATKeystroke(0,CATKEY_F10,    modKey)); break;
        case VK_F11:      OnKeyUp(CATKeystroke(0,CATKEY_F11,    modKey)); break;
        case VK_F12:      OnKeyUp(CATKeystroke(0,CATKEY_F12,    modKey)); break;
    }            
    return 0;
}

LRESULT CATWindow::OSOnKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    CATTRACE("Got WM_KEYDOWN...");
    CATMODKEY modKey = GetModifierKeys();

    switch (wParam)
    {
        // Special purpose keys               
        case VK_F1:       
            OnHelp();
            break;

        // Tab for focus selection
        case VK_TAB:
            {
                if (modKey & CATMODKEY_SHIFT)
                {
                    SetFocusPrev();
                }
                else
                {
                    SetFocusNext();
                }
                lRes = 1;
                handled = true;
            }
            break;						
        case VK_ESCAPE:
            if (fFocusControl != 0)
            {
                KillFocus();
                lRes = 1;
                handled = true;
            }
            break;
        
        case VK_LEFT:
                SetFocusLeft();
                lRes = 1;
                handled = true;
                break;
        case VK_UP:
                SetFocusUp();
                lRes = 1;
                handled = true;
                break;

        case VK_RIGHT:
                SetFocusRight();
                lRes = 1;
                handled = true;
                break;

        case VK_DOWN:
                SetFocusDown();
                lRes = 1;
                handled = true;
                break;

        // "standard" special keystrokes
        case VK_HOME:     OnKeyDown(CATKeystroke(0,CATKEY_HOME,modKey));    break;                    
        case VK_END:      OnKeyDown(CATKeystroke(0,CATKEY_END,modKey));     break;
        case VK_NEXT:     OnKeyDown(CATKeystroke(0,CATKEY_NEXT,modKey));    break;
        case VK_PRIOR:    OnKeyDown(CATKeystroke(0,CATKEY_PRIOR,modKey));   break;
        case VK_INSERT:   OnKeyDown(CATKeystroke(0,CATKEY_INSERT,modKey));  break;
        case VK_DELETE:   OnKeyDown(CATKeystroke(0,CATKEY_DELETE,modKey));  break;
        case VK_F2:       OnKeyDown(CATKeystroke(0,CATKEY_F2,modKey));      break;
        case VK_F3:       OnKeyDown(CATKeystroke(0,CATKEY_F3,modKey));      break;
        case VK_F4:       OnKeyDown(CATKeystroke(0,CATKEY_F4,modKey));      break;
        case VK_F5:       OnKeyDown(CATKeystroke(0,CATKEY_F5,modKey));      break;
        case VK_F6:       OnKeyDown(CATKeystroke(0,CATKEY_F6,modKey));      break;
        case VK_F7:       OnKeyDown(CATKeystroke(0,CATKEY_F7,modKey));      break;
        case VK_F8:       OnKeyDown(CATKeystroke(0,CATKEY_F8,modKey));      break;
        case VK_F9:       OnKeyDown(CATKeystroke(0,CATKEY_F9,modKey));      break;
        case VK_F10:      OnKeyDown(CATKeystroke(0,CATKEY_F10,modKey));     break;
        case VK_F11:      OnKeyDown(CATKeystroke(0,CATKEY_F11,modKey));     break;
        case VK_F12:      OnKeyDown(CATKeystroke(0,CATKEY_F12,modKey));     break;
        
        // Keys we track up/down for
        default:          OnKeyDown(CATKeystroke(wParam,CATKEY_NONE,modKey)); lRes=1;handled=true;break;
    }            
    return lRes;
}


LRESULT CATWindow::OSOnMinMax(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    MINMAXINFO* minMax = (MINMAXINFO*)lParam;
    CATInt32 minx,miny,maxx,maxy;
    GetMinMax(minx,miny,maxx,maxy);
    // Argh... we've got minmax for client rect, now
    // translate to window rect coordinates....
    CATUInt32 style = OSGetWindowStyle();

    RECT minRect;
    RECT maxRect;
    ::SetRect(&maxRect,0,0,maxx,maxy);
    ::SetRect(&minRect,0,0,minx,miny);
    AdjustWindowRect(&minRect,style,FALSE);
    AdjustWindowRect(&maxRect,style,FALSE);
    if (minx != 0)
        minx = minRect.right - minRect.left;
    if (miny != 0)
        miny = minRect.bottom - minRect.top;
    if (maxx != 0)
        maxx = maxRect.right - maxRect.left;
    if (maxy != 0)
        maxy = maxRect.bottom - maxRect.top;



    // Okay, NOW we have the right info
    if (minx != 0)
        minMax->ptMinTrackSize.x = minx;
    if (miny != 0)
        minMax->ptMinTrackSize.y = miny;

    if (maxx != 0)
        minMax->ptMaxTrackSize.x = maxx;

    if (maxy != 0)
        minMax->ptMaxTrackSize.y = maxy;

    handled = true;
    return 0;
}


LRESULT CATWindow::OSDropFiles(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    CATUInt32 numFiles = ::DragQueryFile( (HDROP)wParam, -1, 0, 0);

    for (CATUInt32 curFile = 0; curFile < numFiles; curFile++)
    {
        CATWChar filenameBuf[_MAX_PATH+1];
        if (0 != ::DragQueryFile( (HDROP)wParam, curFile, filenameBuf, _MAX_PATH))
        {
            CATResult result;
            if (CATFAILED(result = OnDropFile(filenameBuf)))
            {
                DisplayError(result);
            }
        }
    }
    handled = true;
    return 0;
}            

LRESULT CATWindow::OSGetControlColors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    HWND ctlhwnd = (HWND)lParam;
    CATControlWnd* theControl = 0;
    if ( (theControl = GetCtlWnd(ctlhwnd)) != 0 )
    {
        HDC dc = (HDC)wParam;

        CATColor bgColor = theControl->GetColorBack();
        CATColor fgColor = theControl->GetColorFore();

        SetBkColor((HDC)wParam, RGB(bgColor.r, bgColor.g, bgColor.b));
        SetTextColor((HDC)wParam, RGB(fgColor.r, fgColor.g, fgColor.b));
        lRes = (LRESULT)theControl->GetBGBrush();
        handled  = true;
    }
    return lRes;
}

LRESULT CATWindow::OSOnAppCmd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    switch (GET_APPCOMMAND_LPARAM(lParam))
    {
        case APPCOMMAND_BROWSER_BACKWARD:
        case APPCOMMAND_BROWSER_FORWARD:
        case APPCOMMAND_BROWSER_HOME:
        case APPCOMMAND_BROWSER_REFRESH:
        case APPCOMMAND_BROWSER_SEARCH:
        case APPCOMMAND_BROWSER_STOP:
        case APPCOMMAND_MEDIA_NEXTTRACK:
        case APPCOMMAND_MEDIA_PREVIOUSTRACK:
        case APPCOMMAND_MEDIA_PLAY_PAUSE:
        case APPCOMMAND_MEDIA_STOP:
        case APPCOMMAND_TREBLE_DOWN:
        case APPCOMMAND_TREBLE_UP:
        case APPCOMMAND_BASS_DOWN:
        case APPCOMMAND_BASS_UP:
        case APPCOMMAND_VOLUME_DOWN:
        case APPCOMMAND_VOLUME_UP:
        case APPCOMMAND_VOLUME_MUTE:     
            break;       
    }  
    return lRes;
}

LRESULT CATWindow::OSOnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    // Get the update region prior to calling BeginPaint,
    // as BeginPaint destroys it.
    HRGN updateRgn      = ::CreateRectRgn(0,0,0,0);
    int rgnType         = ::GetUpdateRgn(hwnd,updateRgn,FALSE);
    HPALETTE appPalette = (HPALETTE)(UINT_PTR)GetWindowLong(hwnd,0);

    // Call get paint anyway - we don't want to mess
    // with validating stuff ourselves.
    PAINTSTRUCT ps;
    HDC drawContext = ::BeginPaint(hwnd,&ps);

    // Default rect to draw everything...
    CATRect defaultRect = fRect;


    SetStretchBltMode (drawContext,HALFTONE);
    SetBrushOrgEx     (drawContext,0,0,NULL);
    SelectPalette     (drawContext,appPalette,FALSE);
    RealizePalette    (drawContext);

    DWORD rgnSize = ::GetRegionData(updateRgn,0,0);
    if (rgnSize == 0)
    {        
        OSDraw(drawContext, &defaultRect);
    }
    else
    {
        RGNDATA* rgnData = (RGNDATA*)new char[rgnSize];
        if (rgnData != 0)
        {
            memset(rgnData,0,rgnSize);
            rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
            if ((::GetRegionData(updateRgn,rgnSize,rgnData) == rgnSize) && (rgnData->rdh.nCount > 0))
            {                        
                // Got the data... now draw.
                for (CATUInt32 i = 0; i < rgnData->rdh.nCount; i++)
                {                              
                    RECT curRect = *(RECT*)(rgnData->Buffer + i * sizeof(RECT));
                    CATRect dirtyRect;
                    dirtyRect.Set(curRect.left, curRect.top, curRect.right, curRect.bottom);                           
                    OSDraw(drawContext, &dirtyRect);                     
                }                                                         
            }
            else
            {
                // No rects in region - draw it all it's a full paint.                
                OSDraw(drawContext, &defaultRect);
            }
            delete [] (char*)rgnData;
        }
    }
    ::DeleteObject(updateRgn);                  
    ::ReleaseDC(hwnd, drawContext);
    ::EndPaint(hwnd,&ps);               
    //  Validate the window
    // ::ValidateRect(hwnd,0);
    handled  = true;    
    return 0;
}

LRESULT CATWindow::OSOnMeasureItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    LPMEASUREITEMSTRUCT measureInfo = (LPMEASUREITEMSTRUCT)lParam;
    switch (measureInfo->CtlType)
    {
        case ODT_MENU:
            {
                // Owner drawn menus... the itemData will be a CATMENUITEM structure from CATMenu.h
                CATMENUITEM* menuItem = (CATMENUITEM*)measureInfo->itemData;
                if (menuItem != 0)
                {
                    CATMenu* menu = menuItem->BaseMenu;
                    if (menu)
                    {
                        menu->OSOnMeasureItem(  menuItem, 
                                                (CATUInt32&)measureInfo->itemWidth, 
                                                (CATUInt32&)measureInfo->itemHeight);
                        lRes     = TRUE;
                        handled  = true;
                    }
                }
            }
            break;

        case ODT_LISTBOX:
            {
                // Owner drawn menus... the itemData will be a CATLISTINFO structure from CATListBox.h
                CATLISTINFO* listItem = (CATLISTINFO*)measureInfo->itemData;
                if (listItem != 0)
                {
                    CATListBox* listBox = listItem->BaseListBox;
                    
                    if (listBox)
                    {
                        listBox->OSOnMeasureItem( listItem, 
                                                 (CATUInt32&)measureInfo->itemWidth, 
                                                 (CATUInt32&)measureInfo->itemHeight);
                        lRes     = TRUE;
                        handled  = true;
                    }
                }
            }
            break;
        case ODT_COMBOBOX:
        case ODT_LISTVIEW:
            break;
    }

    return lRes;
}

LRESULT CATWindow::OSOnDrawItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    LPDRAWITEMSTRUCT drawInfo = (LPDRAWITEMSTRUCT)lParam;
    switch (drawInfo->CtlType)
    {
        case ODT_MENU:
            {                        
                CATMENUITEM* menuItem = (CATMENUITEM*)drawInfo->itemData;
                if (menuItem != 0)
                {
                    CATMenu* menu = menuItem->BaseMenu;
                    if (menu)
                    {
                        menu->OSOnDrawItem(  menuItem, 
                                            (drawInfo->itemState & ODS_SELECTED)?true:false,
                                            drawInfo->hDC,
                                            CATRect(drawInfo->rcItem.left, 
                                            drawInfo->rcItem.top, 
                                            drawInfo->rcItem.right, 
                                            drawInfo->rcItem.bottom));
                        lRes     = TRUE;
                        handled  = true;
                    }
                }
            }
            break;

        case ODT_LISTBOX:
            {                        
                CATLISTINFO* listItem = (CATLISTINFO*)drawInfo->itemData;
                if (listItem != 0)
                {
                    CATListBox* listBox = listItem->BaseListBox;
                    if (listBox)
                    {
                        listBox->OSOnDrawItem(  listItem, 
                                                (drawInfo->itemState & ODS_SELECTED)?true:false,
                                                drawInfo->hDC,
                                                CATRect(drawInfo->rcItem.left, 
                                                drawInfo->rcItem.top, 
                                                drawInfo->rcItem.right, 
                                                drawInfo->rcItem.bottom));
                        lRes     = TRUE;
                        handled  = true;
                    }
                }
            }
            break;

        case ODT_BUTTON:
        case ODT_COMBOBOX:
        case ODT_LISTVIEW:
        case ODT_STATIC:
        case ODT_TAB:
            break;
    }

    return lRes;
}

LRESULT CATWindow::OSGetObject(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& handled)
{
    LRESULT lRes = 0;
    DWORD dwObjId = lParam;  
	 // Add in accessibility hooks here TODO
    return lRes;
}

CATRect CATWindow::OSGetRealRect()
{
    CATUInt32 style = OSGetWindowStyle();
    CATRect realRect = fRect;
    realRect.SetOrigin(this->fScreenPos);    
    
    // Add in border size so client is at the desired size...
    AdjustWindowRect((RECT*)&realRect,style,FALSE);

    // prevent under-shooting.... over shooting shouldn't really be possible yet
    if (realRect.left < 0)
    {
        CATInt32 xOff = -realRect.left;
        realRect.left += xOff;
        realRect.right += xOff;
    }

    if (realRect.top < 0)
    {
        CATInt32 yOff = -realRect.top;
        realRect.top += yOff;
        realRect.bottom += yOff;
    }
    return realRect;
}

void CATWindow::EnableScanning(bool scanning, CATFloat32 scanRate)
{
    if (scanning)
    {
        if (this->fScanningEnabled)
        {
            // remove old
            KillTimer(this->fWindow,CATWM_SCANNINGTIMER);
        }

        fScanRate = scanRate;
        fScanningEnabled = true;
        this->SetFocusFirst();
        ::SetTimer(this->fWindow, CATWM_SCANNINGTIMER, (DWORD)(fScanRate*1000), 0);
    }
    else
    {
         fScanningEnabled = false;
         KillTimer(this->fWindow,CATWM_SCANNINGTIMER);
         this->SetFocusFirst();
    }

    SetPref(L"ScanningEnabled",fScanningEnabled);
    SetPref(L"ScanRate",fScanRate);
}

void CATWindow::PauseScanning()
{
    if (fScanningEnabled)
        KillTimer(this->fWindow,CATWM_SCANNINGTIMER);
}

void CATWindow::ResumeScanning()
{
    if (fScanningEnabled)
    {
        SetFocusFirst();
        ::SetTimer(this->fWindow, CATWM_SCANNINGTIMER, (DWORD)(fScanRate*1000), 0);
    }
}

HRGN CATWindow::RegionFromImage(CATImage* image)
{
	struct RegionInfo
	{
		RGNDATAHEADER rdh;
		RECT rects[100];
	};	

	RegionInfo rgnInfo;

	memset(&rgnInfo,0,sizeof(rgnInfo));
	rgnInfo.rdh.dwSize = sizeof(RGNDATAHEADER);
	rgnInfo.rdh.iType = RDH_RECTANGLES;

	HRGN tmpRgn = 0;
	HRGN bmpRgn = 0;
	
	CATInt32 x,y;
	RECT boundRect;
	CATInt32 startPos	= 0;
	CATUInt32 curRect	= 0;

	unsigned char* srcPtr = image->GetRawDataPtr();
	for (y=0; y<image->Height(); y++)
	{
		curRect = 0;

		for (x=0; x < image->Width(); x++)		
		{
			startPos = x;			
			bool inScan = false;
			do
			{
				if (srcPtr[3] > 180)
				{
					x++;					
					inScan = true;									
				}
				else
				{					
					inScan = false;
				}
				srcPtr += 4;
			} while ((x < image->Width()) && (inScan));			

			//
			// If we have a run of valid pixels, save it as a rect
			//
			if (startPos+1 < x)
			{
				rgnInfo.rects[curRect].left		= startPos;
				rgnInfo.rects[curRect].top			= y;
				rgnInfo.rects[curRect].bottom		= y+1;
				rgnInfo.rects[curRect].right		= x;

				if (curRect == 0)
				{
					boundRect.left		= startPos;
					boundRect.top		= y;
					boundRect.bottom	= y+1;
					boundRect.right	= x;
				}
				else
				{
					boundRect.left		= (boundRect.left		< startPos	)?boundRect.left	:startPos;
					boundRect.top		= (boundRect.top		< y			)?boundRect.top	:y;
					boundRect.bottom	= (boundRect.bottom	> y+1			)?boundRect.bottom:y+1;
					boundRect.right	= (boundRect.right	> x			)?boundRect.right	:x;
				}
            
				curRect++;

				//
				// At *least* Every hundred rects, we dump a region from 'em...
				//
				if (curRect >= 100)
				{
					rgnInfo.rdh.nCount	= curRect;
					rgnInfo.rdh.rcBound	= boundRect;
					
					tmpRgn = ExtCreateRegion(0,curRect*sizeof(RECT)+sizeof(RGNDATAHEADER),(RGNDATA*)&rgnInfo);
					
					if (bmpRgn == 0)
					{
						bmpRgn = tmpRgn;
						tmpRgn = 0;
					}
					else
					{
						CombineRgn(bmpRgn,bmpRgn,tmpRgn,RGN_OR);
						DeleteObject(tmpRgn);
						tmpRgn = 0;
					}					
					curRect = 0;
				}
			}
		}
	
		// Make regions for any additinal rects we've received and merge 'em
		if (curRect > 0)
		{
			rgnInfo.rdh.nCount	= curRect;
			rgnInfo.rdh.rcBound	= boundRect;

         tmpRgn = ExtCreateRegion(0,curRect*sizeof(RECT)+sizeof(RGNDATAHEADER),(RGNDATA*)&rgnInfo);
			
			if (bmpRgn == 0)
			{
				bmpRgn = tmpRgn;
				tmpRgn = 0;
			}
			else
			{
				CombineRgn(bmpRgn,bmpRgn,tmpRgn,RGN_OR);
				DeleteObject(tmpRgn);
				tmpRgn = 0;
			}
			curRect = 0;
		}
	}
	return bmpRgn;
}
