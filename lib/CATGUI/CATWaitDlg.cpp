/// \file   CATWaitDlg.cpp
/// \brief  Wait dialog
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

#include "CATWaitDlg.h"
#include "CATLabel.h"
#include "CATStreamFile.h"
#include "CATEventDefs.h"

CATWaitDlg::CATWaitDlg(	CATINSTANCE	instance,
                       CATInt32		backgroundBmpId,
                       CATRect&		textRect,
                       CATInt32		progressBmpOnId,	// optional progress bar
                       CATInt32		progressBmpOffId,
                       CATInt32		progressLeft,
                       CATInt32		progressTop)
                       : CATWindow( "Window",  "" )
{
    fAllowClose = false;
    fBmpId	= 0;
    fDepth	= 0;
    fParent	= 0;
    fDisableParent = false;
    fProgBmpOnId	= 0;
    fProgBmpOffId	= 0;
    fProgressBar	= 0;
    fWaitLabel		= 0;

    // Setup window attributes (since we're created direct, not from XML)    
    this->AddAttribute(L"WaitDlg",			L"Name");
    this->AddAttribute(L"Please Wait...",	L"Title");
    this->AddAttribute(L"False",			L"CaptionBar");
    this->AddAttribute(L"True",				L"DragAnywhere");
    this->AddAttribute(L"False",			L"Sizeable");
    this->AddAttribute(L"True",				L"Multiline");
    this->AddAttribute(L"True",				L"TextCentered");


    // Generate background bitmap from Resource
    fBmpId	= backgroundBmpId;
    HBITMAP bgBmp	= (HBITMAP)::LoadImage(	instance, 
        MAKEINTRESOURCE(fBmpId), 
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);


    if (bgBmp != 0)
    {
        CATResult result = CATImage::CreateImageFromDIB(fImage, bgBmp);		

        CATASSERT(CATSUCCEEDED(result), "Unable to create image from bitmap for wait dialog!");
        this->fRect.Set( 0,0, fImage->Width(), fImage->Height());

        ::DeleteObject(bgBmp);
    }




    // Create wait status label

    fWaitLabel = new CATLabel( "Label",this->fRootDir);	
    fWaitLabel->AddAttribute(L"Name",        L"WaitStatus");
    fWaitLabel->AddAttribute(L"XPos",        (CATString)textRect.left);
    fWaitLabel->AddAttribute(L"Width",       (CATString)textRect.Width());
    fWaitLabel->AddAttribute(L"YPos",        (CATString)textRect.top);
    fWaitLabel->AddAttribute(L"Height",      (CATString)textRect.Height());
    fWaitLabel->AddAttribute(L"ColorFore",   (CATString)(CATUInt32)0);
    fWaitLabel->AddAttribute(L"FontName",    L"Arial");
    fWaitLabel->AddAttribute(L"FontSize",    L"10");

    this->AddChild(fWaitLabel);


    // Now for progress bar and bitmaps if available
    if ((progressBmpOnId != 0) && (progressBmpOffId != 0))
    {
        fProgBmpOnId = progressBmpOnId;
        fProgBmpOffId = progressBmpOffId;

        HBITMAP progBmpOn   = (HBITMAP)::LoadImage(instance, MAKEINTRESOURCE(fProgBmpOnId),  IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        HBITMAP progBmpOff   = (HBITMAP)::LoadImage(instance, MAKEINTRESOURCE(fProgBmpOffId), IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);

        if (progBmpOn && progBmpOff)
        {

            CATImage *progOn, *progOff;

            CATImage::CreateImageFromDIB( progOn,  progBmpOn);
            CATImage::CreateImageFromDIB( progOff, progBmpOff);

            fProgressBar = new CATProgress( L"Progress", L"");			
            fProgressBar->AddAttribute(L"XPos", (CATString)progressLeft);
            fProgressBar->AddAttribute(L"YPos", (CATString)progressTop);
            fProgressBar->AddAttribute(L"Name", L"WaitProgress");
            fProgressBar->AddAttribute(L"ProgressStyle", L"Horizontal");

            // Window base-class takes ownership of progress bar.
            this->AddChild(fProgressBar);

            // Progress bar takes ownership of images
            fProgressBar->SetImages(progOn, progOff,0);
        }

        if (progBmpOn)
            DeleteObject(progBmpOn);

        if (progBmpOff)
            DeleteObject(progBmpOff);

    }


    // Initialize window properties and rect
    CATWindow::Load();	
}

CATWaitDlg::~CATWaitDlg()
{
    if (this->IsShowing())
    {
        this->Hide(true);
    }
}

void CATWaitDlg::OnDestroy()
{
    CATWindow::OnDestroy();
}

void CATWaitDlg::StartWait(const CATString& waitText, CATWindow* parent, bool disableParent)
{
    this->SetWaitLabelText(waitText);
    this->SetProgress(0.0f);

    if (fDepth != 0)
    {
        fDepth++;
        this->SetWaitLabelText(waitText);	
        return;
    }


    fDisableParent = disableParent;
    fParent			= parent;

    if ((fDisableParent) && (fParent))
    {
        fParent->SetEnabled(false);
    }

    fAllowClose = false;
    fParent		= parent;

    CATRect wndRect = fRect;

    CATRect parentRect;
    if (fParent)
    {
        parentRect = fParent->GetRectAbs(true);
    }
    else
    {
        HMONITOR monitor = MonitorFromWindow( ::GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monInfo;
        monInfo.cbSize = sizeof(monInfo);
        GetMonitorInfo(monitor, &monInfo);
        parentRect.Set(monInfo.rcWork.left, monInfo.rcWork.top, monInfo.rcWork.right, monInfo.rcWork.bottom);
    }

    CATPOINT centerPos;	
    centerPos.x = parentRect.left + (parentRect.Width() - fRect.Width())/2;
    centerPos.y = parentRect.top  + (parentRect.Height() - fRect.Height())/2;
    wndRect.SetOrigin(centerPos);

    this->AddAttribute(L"XPos",(CATString)wndRect.left);
    this->AddAttribute(L"YPos",(CATString)wndRect.top);
    this->AddAttribute(L"Width",(CATString)wndRect.Width());
    this->AddAttribute(L"Height",(CATString)wndRect.Height());

    this->OnSize(wndRect);


    fDlgReady.Reset();

    fDepth = 1;
    // Create thread for async

	 fThread.StartProc(WindowThread,this);
    fDlgReady.Wait();
}

bool CATWaitDlg::IsShowing()
{
    return (fDepth > 0);
}

void CATWaitDlg::EndWait(bool force)
{
    if (fDepth > 0)
    {
        fDepth--;
    }


    if ((fDepth == 0) || (force))
    {
        fAllowClose = true;
        ::PostMessage(fWindow,CATWM_ENDWAIT,0,0);
		  fThread.WaitStop();
        fDepth = 0;
        fWindow = 0;
        if ((fDisableParent) && (fParent))
        {
            fParent->SetEnabled(true);
        }
    }
}

CATUInt32 CATWaitDlg::GetDepth()
{
    return fDepth;
}

bool CATWaitDlg::OnClose()
{
    return fAllowClose;
}

void CATWaitDlg::WindowThread(void* param, CATThread* theThread)
{
    CATWaitDlg* waitWnd = (CATWaitDlg*)param;

    waitWnd->Show( waitWnd->fParent);

    waitWnd->fRect.ZeroOrigin();
    waitWnd->OnSize(waitWnd->fRect);

    MSG msg;

    waitWnd->fDlgReady.Fire();

    while (::GetMessage(&msg,waitWnd->fWindow,0,0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }   

}

void CATWaitDlg::SetWaitLabelText( const CATString& waitStr )
{
    if (fWindow != 0)
    {
        ::SendMessage(fWindow, CATWM_SETWAITLABEL,(WPARAM)&waitStr,0);
    }
}

/// Draw() is called when the window should paint itself. 
///
/// \param background - ptr to image to draw into
/// \param dirtyRect - part of window to redraw.
void CATWaitDlg::Draw(  CATImage*        background,
                      const CATRect&   dirtyRect)
{
    CATWindow::Draw(background,dirtyRect);
}


void CATWaitDlg::SetProgress( CATFloat32 percent )
{
    if (percent < 0.0f)
        percent = 0.0f;
    if (percent > 1.0f)
        percent = 1.0f;

    if (fWindow != 0)
    {
        ::SendMessage(fWindow,CATWM_SETWAITPROGRESS, (WPARAM)&percent,0);
    }

    //this->fProgressBar->SetValue(percent);
}

CATResult CATWaitDlg::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    CATResult result = CAT_SUCCESS;

    switch (event.fEventCode)
    {      
    case CATEVENT_WINDOWS_EVENT:
        switch (event.fIntParam2)
        {
        case CATWM_SETWAITPROGRESS:
            {
                CATFloat32* progPtr = (float*)event.fIntParam3;
                if ((progPtr != 0) && (fProgressBar != 0))
                {
                    fProgressBar->SetValue(*progPtr);
                }
            }
            break;
        case CATWM_SETWAITLABEL:
            {
                CATString* strPtr = (CATString*)event.fIntParam3;
                if ((strPtr != 0) && (fWaitLabel != 0))
                {
                    fWaitLabel->SetString(*strPtr);
                }
            }
            break;
        case CATWM_ENDWAIT:
            ::PostQuitMessage(0);
            break;
        }
        break;
    default:
        return CATWindow::OnEvent(event,retVal);
    }

    return result;
}