//---------------------------------------------------------------------------
/// \file CATWindow.cpp
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
//---------------------------------------------------------------------------
#include <math.h>

#include "CATWindow.h"
#include "CATOSFuncs.h"
#include "CATApp.h"
#include "CATControl.h"
#include "CATControlWnd.h"
#include "CATEventDefs.h"
#include "CATLabel.h"
#include "CATPrefs.h"
#include "CATSwitch.h"

/// Docking distance - if autodock is on, and we are within this number
/// of pixels of an edge, we dock.
const CATInt32 kDOCKDISTANCE = 10;

//---------------------------------------------------------------------------
// CATWindow constructor (inherited from CATXMLObject)
// \param element - Type name ("Window")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Skin" element)
//---------------------------------------------------------------------------
CATWindow::CATWindow(  const CATString&             element, 
                       const CATString&             rootDir)
: CATGuiObj(element,  rootDir)
{
	 fSetRegion			 = false;
    fIsOverlapped     = false;
    fIsTemplate       = false;
    fMaximized        = false;
    fAutoDock         = true;
    fRightTrack			= 0;
    fMouseInWindow    = 0;
    fMouseTrackTarget = 0;
    fActiveControl    = 0;
    fFocusControl     = 0;
    fLastMouseUpdate  = 0;
    fStatusLabel      = 0;   
    fScreenPos.x      = 0;
    fScreenPos.y      = 0;
    fUserIcon         = 0;
    
    fScanningEnabled  = 0;
    fScanRate         = 1.0f;

    fOSDestruction    = false;

    fPrevWndOwner     = 0;
    fBaseSize.cx      = 0;
    fBaseSize.cy      = 0;
    fLeftSlack        = fRightSlack = fTopSlack = fBottomSlack = 0;

    fPrimary          = false;
    fCaptionBar       = true;

    fDragAnywhere     = false;
    fDragging         = false;
    fDragLastPoint.x  = fDragLastPoint.y = 0;

    fSizeable         = true;
    fPostRegion       = 0;
    fImageCopy        = 0;
    fRegistered       = false;
    fVisible          = false;
    fWindow           = 0;   
    fSkin             = 0;
    fExiting		  = false;
    fExitThread		  = false;
}

CATResult CATWindow::PostThreadedCommand(CATCommand& cmd)
{
    CATResult result = CAT_SUCCESS;

    if (CATFAILED(result = fThreadedCmdLock.Wait()))
        return result;

    result = fThreadedCmdQueue.Queue(cmd);

    fThreadedCmdLock.Release();

    if  (CATSUCCEEDED(result))
    {
        fThreadedCmdSignal.Fire();
    }

    return result;
}

void CATWindow::ThreadedCmdThread()
{
    while (true)
    {
        fThreadedCmdSignal.Wait();
        if (fExitThread)
        {
            CATTRACE("Received posted exit for window.");
            return;
        }

        fThreadedCmdLock.Wait();
        while (fThreadedCmdQueue.Size())
        {
            CATCommand curCommand;
            if (CATSUCCEEDED(fThreadedCmdQueue.Next(curCommand)))
            {
                // Release lock on commands to allow new commands issued 
                fThreadedCmdLock.Release();


                this->OnThreadedCommand(curCommand);


                // Re-aquire lock on commands after command has processed for loop
                fThreadedCmdLock.Wait();
            }			
        }		

        // Currently own lock, so size won't change
        fThreadedCmdSignal.Reset();

        // Release lock.
        fThreadedCmdLock.Release();
    }
}

//---------------------------------------------------------------------------
// Load() loads the skin in
//---------------------------------------------------------------------------
CATResult CATWindow::Load(CATPROGRESSCB				progressCB,
                          void*						progressParam,
                          CATFloat32						progMin,
                          CATFloat32						progMax)
{
    CATResult testResult = CAT_SUCCESS;
    CATResult result = CAT_SUCCESS;

    if (CATFAILED(result = CATGuiObj::Load(progressCB, progressParam, progMin, progMax)))
    {
        return result;
    }   

    // Attempt to register this window - throw on failure
    fClassName  = gApp->GetAppName() << "_" << fName;

    if (CATFAILED(testResult = RegWindow()))
    {
        return testResult;
    }
    fRegistered = true;
    fStatusBarHint = fHintText;   
    testResult = ResetBackground();
    if (CATFAILED(testResult))
    {
        return testResult;
    }

    fStatusLabel = (CATLabel*)this->FindControlAndVerify("StatusLabel","Label");
    EnableObject("MaximizeSwitch",fSizeable);


    return result;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CATResult CATWindow::ResetBackground()
{
    CATResult result = CAT_SUCCESS;
    if (fImageCopy)
    {
        CATImage::ReleaseImage(fImageCopy);
    }

    if (CATFAILED(result = CATImage::CreateImage(   fImageCopy, 
                                                    fRect.Width(), 
                                                    fRect.Height(), 
                                                    false, 
                                                    false)))
    {
        return result;
    }

    // If we have an image, copy it or tile it into the background
    if (fImage)
    {
        if ((fImage->Width() == fImageCopy->Width()) && 
            (fImage->Height() == fImageCopy->Height()))
        {
            // Window is same size as image. Good. Just do a copy.
            result =fImageCopy->CopyOver(fImage,0,0,0,0,0,0);
        }
        else
        {
            // OK... gotta do tiling of the background                
            CATInt32 yPos = 0;
            CATInt32 totalHeight = fImageCopy->Height();
            while (totalHeight > 0)
            {
                CATInt32 xPos = 0;
                CATInt32 copyHeight = CATMin(fImage->Height(), totalHeight);
                CATInt32 totalWidth = fImageCopy->Width();

                while (totalWidth > 0)
                {
                    CATInt32 copyWidth = CATMin(fImage->Width(), totalWidth);               
                    result = fImageCopy->CopyOver(fImage,xPos,yPos,0,0,copyWidth,copyHeight);
                    xPos += copyWidth;
                    totalWidth -= copyWidth;
                }

                yPos += copyHeight;
                totalHeight -= copyHeight;
            }
        }
    }
    else
    {
        // No image - 
        result = fImageCopy->FillRect(fRect,this->fBackgroundColor);
    }

    // Reset region information
    if (fPostRegion)
    {
        OSFreeRegion(fPostRegion);
        fPostRegion = 0;
    }

    // Generate new region
    CATStack<CATRect> postRects;
    if (this->GetPostRects(postRects))
    {
        // Got some post rects...
        this->fPostRegion = this->OSGenRegion(postRects);      
    }

    return result;
}

//---------------------------------------------------------------------------
/// CleanBackground() cleans the dirty rectangle in the background
//---------------------------------------------------------------------------
CATResult CATWindow::CleanBackground( CATRect* dirtyRect )
{
    CATResult result = CAT_SUCCESS;

    // If no background copy to clean, reset it.
    if (fImageCopy == 0)
    {
        return this->ResetBackground();
    }

    // If no rectangle was passed, do the whole thing.
    CATRect cleanRect(fRect);
    if (dirtyRect != 0)
    {
        cleanRect = *dirtyRect;
    }

    // If no background image, just fill with red.
    if (fImage == 0)
    {
        return fImageCopy->FillRect(cleanRect, fBackgroundColor);
    }


    // Got an image - copy or tile it
    if ((fImage->Width() == fImageCopy->Width()) && 
        (fImage->Height() == fImageCopy->Height()))
    {
        CATRect drawRect;
        CATRect imageRect(0,0,fImage->Width(), fImage->Height());
        cleanRect.Intersect(imageRect,&drawRect);
        // Window is same size as image. Good. Just do a copy.
        result = fImageCopy->CopyOver(  fImage,
                                        drawRect.left,
                                        drawRect.top,
                                        drawRect.left,
                                        drawRect.top,
                                        drawRect.Width(),
                                        drawRect.Height());
    }
    else
    {
        // OK... gotta do tiling of the background, but only within the
        // rectangle.
        CATInt32 yPos        = cleanRect.top;
        CATInt32 totalHeight = cleanRect.Height();
        CATInt32 offsetY     = yPos % fImage->Height();

        while (totalHeight > 0)
        {
            CATInt32 xPos       = cleanRect.left;
            CATInt32 copyHeight = CATMin(fImage->Height() - offsetY, totalHeight);
            CATInt32 totalWidth = cleanRect.Width();
            CATInt32 offsetX    = xPos % fImage->Width();

            while (totalWidth > 0)
            {
                CATInt32 copyWidth = CATMin(fImage->Width() - offsetX, totalWidth);               
                result = fImageCopy->CopyOver(fImage,xPos,yPos,offsetX,offsetY,copyWidth,copyHeight);
                xPos += copyWidth;
                totalWidth -= copyWidth;
                offsetX = 0;
            }

            offsetY = 0;
            yPos += copyHeight;
            totalHeight -= copyHeight;
        }
    }

    return result;
}

//---------------------------------------------------------------------------
// CATWindow destructor
//---------------------------------------------------------------------------
CATWindow::~CATWindow()
{

    fExiting = true;

    // Kill any events.
    fEventLock.Wait();
    while (fEventQueue.Size() > 0)
    {
        CATEvent* event = 0;
        if (CATSUCCEEDED(fEventQueue.Next(event)))
        {
            delete event;
        }
    }
    fEventLock.Release();

    // If the window is still visible when we're deleted, we need to
    // nuke it.
    if (fWindow)
    {
        this->Hide(true);
    }

    if (fPostRegion)
    {
        OSFreeRegion(fPostRegion);
        fPostRegion = 0;
    }

    if (fImageCopy)
    {
        CATImage::ReleaseImage(fImageCopy);
    }

    if (fRegistered)
    {
        CATUnregWindow();
        fRegistered = false;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::MarkDirty(CATRect* dirtyRect, bool force )
{
    // even if force is true, on a window we ignore if not visible.
    if (!fVisible)
        return;

    CATRect invRect(this->fRect);

    if (dirtyRect != 0)
    {
        invRect = *dirtyRect;
    }

    if (fWindow != 0)
    {
        CATInvalidateRect( this->fWindow, invRect);
    }
}


//---------------------------------------------------------------------------
void CATWindow::Draw(CATImage* image, const CATRect& dirtyRect)
{	 
    CATXMLObject* curChild = 0;
    CATUInt32 index;
    CATUInt32 numChildren = this->GetNumChildren();
    for (index = 0; index < numChildren; index++)
    {
        if (0 != (curChild = GetChild(index)))
        {
            CATWidget* curControl = (CATWidget*)curChild;            
				if (dirtyRect.Intersect(curControl->GetRectAbs()))
					curControl->Draw(image,dirtyRect);         
        }
    }
}
//---------------------------------------------------------------------------
void CATWindow::PostDraw(CATDRAWCONTEXT context, const CATRect& dirtyRect)
{
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
				if (dirtyRect.Intersect(curControl->GetRectAbs()))
					curControl->PostDraw(context,dirtyRect);               
        }
    }   
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::OnCreate()
{
    // Let children know and respond to creation if they need to.
    CATUInt32 numControls = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numControls; i++)
    {
        CATXMLObject* curChild = 0;
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->OnParentCreate();
        }
    }

    this->OSStartCmdThread();
}

void CATWindow::OnDestroy()
{
    // Finish any pending commands
    OSWaitOnCmdThread();

    if (this->fSizeable)
    {
        SetPref(L"Width",       fRect.Width());
        SetPref(L"Height",      fRect.Height());
    }

    SetPref(L"XPos",            fScreenPos.x);
    SetPref(L"YPos",            fScreenPos.y);

    SetPref(L"ScanningEnabled", fScanningEnabled);
    SetPref(L"ScanRate",        fScanRate);

    this->fCWT.clear();

    // Let children know and respond to destruction if they need to.
    CATUInt32 numControls = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numControls; i++)
    {
        CATXMLObject* curChild = 0;
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->OnParentDestroy();
        }
    }

    if (fPrevWndOwner)
    {
        this->OSRestoreTakenWnd();
    }

    this->fVisible = false;
    this->fWindow = 0;

    // Let everyone know we're hidden now
    CATInt32 lRes = 0;
    gApp->OnEvent( CATEvent(CATEVENT_WINDOW_HIDDEN,0,0,0,0,0,this->fName,"","",this),lRes);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::OnMove(const CATPOINT& newPos)
{   
    fScreenPos = newPos;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool CATWindow::CalcSlack(  CATInt32&  movePos, 
                          CATInt32&  opposite,
                          const    CATInt32& dockPos, 
                          CATInt32&  slackCounter,
                          bool     inside)
{

    // Are we within docking range?
    CATInt32 diff = (movePos - dockPos) * (inside?1:-1);
    if ( CATAbs(diff) < kDOCKDISTANCE)
    {
        // In docking range
        slackCounter += diff;
        if (CATAbs(slackCounter) < kDOCKDISTANCE)
        {
            movePos -= diff;
            opposite -= diff;
            return true;
        }
        else
        {
            movePos += slackCounter;
            opposite += slackCounter;
            slackCounter = 0;
            return true;
        }
    }
    else
    {
        if (slackCounter)
        {
            movePos += slackCounter;
            opposite += slackCounter;
            slackCounter = 0;
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
bool CATWindow::OnMoving(CATRect& moveRect, const CATRect& wndRect)
{
    if (fAutoDock == false)
    {
        return false;
    }

    // AutoDocking...
    bool dockChanged = false;
    CATRect dockRect = OSGetDockRect();   

    // Dock to screen for baseline..
    CATRect tmpRect = moveRect;
    dockChanged |= CalcSlack(moveRect.left,   moveRect.right,   dockRect.left,    fLeftSlack,      true);
    dockChanged |= CalcSlack(moveRect.top,    moveRect.bottom,  dockRect.top,     fTopSlack,       true);
    dockChanged |= CalcSlack(moveRect.right,  moveRect.left,    dockRect.right,   fRightSlack,     true);
    dockChanged |= CalcSlack(moveRect.bottom, moveRect.top,     dockRect.bottom,  fBottomSlack,    true);

    // dock to other windows in skin? feature for later....

    return dockChanged;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CATResult CATWindow::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    CATResult result = CAT_SUCCESS;
    CATResult testResult = CAT_SUCCESS;

    switch (event.fEventCode)
    {
    case CATEVENT_GUI_UPDATE:
        // Window Update request.
        this->Update();
        // increment result value
        retVal++;
        return CAT_SUCCESS;
        break;

    default:
        break;
    }

    if (CAT_SUCCESS != (testResult = CATGuiObj::OnEvent(event,retVal)))
    {
        result = testResult;
    }

    return result;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::Show( CATWND parentWnd)
{
    fVisible = false;
    if (fWindow == 0)
    {      
        CATResult result;
        if (CATFAILED(result = OSCreateWnd(parentWnd)))
        {
            DisplayError(result);
            return;
        }
        //SetFocusNext();

        if (fScanningEnabled)
            this->EnableScanning(fScanningEnabled,fScanRate);        
    }
	
    if (fWindow != 0)
    {  
        OSShowWnd();
        fVisible = true;
    }

    // Let everyone know we're showing now.
    CATInt32 lRes = 0;
    gApp->OnEvent( CATEvent(CATEVENT_WINDOW_SHOWN,0,0,0,0,0,this->fName,"","",this),lRes);
}


void CATWindow::Show( CATWindow* parent, CATPOINT* origin, bool takeoverWnd)
{
    fVisible = false;
    HWND parentWnd = 0;
    if (parent)
        parentWnd = parent->OSGetWnd();

    if (parentWnd && takeoverWnd)
    {
        OSTakeoverWnd(parent);
    }

    if (fWindow == 0)
    {
        CATResult result;        
        if (CATFAILED(result = OSCreateWnd(parentWnd)))
        {
            DisplayError(result);
            return;
        }
        //SetFocusNext();
        if (fScanningEnabled)
            this->EnableScanning(fScanningEnabled,fScanRate);
    }

    if (fWindow != 0)
    {  
        if (origin)
        {
            CATRect wndRect = OSGetWndRect();
            wndRect.SetOrigin(*origin);
            this->OSMoveWnd(wndRect);
        }
        OSShowWnd();
        fVisible = true;
    }

    // Let everyone know we're showing now.
    CATInt32 lRes = 0;
    gApp->OnEvent( CATEvent(CATEVENT_WINDOW_SHOWN,0,0,0,0,0,this->fName,"","",this),lRes);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CATWindow::Hide(bool destroyWnd)
{
    if (fPrevWndOwner)
    {
        OSRestoreTakenWnd();
        CATInt32 lRes = 0;
        gApp->OnEvent( CATEvent(CATEVENT_WINDOW_HIDDEN,0,0,0,0,0,this->fName,"","",this),lRes);
        return;
    }

    if (fWindow)
    {
        if (!destroyWnd)
        {
            if (fVisible)
            {
                OSHideWnd();
                // Let everyone know we're hidden now
                CATInt32 lRes = 0;
                gApp->OnEvent( CATEvent(CATEVENT_WINDOW_HIDDEN,0,0,0,0,0,this->fName,"","",this),lRes);
            }
        }
        else
        {
            OSDestroyWnd();
        }
    }

    fVisible = false;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CATWindow::IsVisible(const CATGuiObj* child) const
{
    return fVisible;
}

void CATWindow::OnSize(const CATRect& newRect)
{   
    CATResult result = CAT_SUCCESS;
    if ((newRect.Width() == 0) && (newRect.Height() == 0))
        return;

    // Set our new size
    this->fRect = newRect;

    // Let the kids know...
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;


            if (CATFAILED(result = curControl->RectFromAttribs()))
            {
                DisplayError(result);
            }

        }
    }
    
    // Reset the background
    if (CATFAILED(result = this->ResetBackground()))
    {
        //DisplayError(result);
    }
}

bool CATWindow::OnClose()
{   
    // Default behavior is to notify the app and use its response.
    CATInt32 result = 1;
    gApp->OnEvent( CATEvent(CATEVENT_WINDOW_CLOSE,0,0,0,0,0,this->fName,"","",this), result);

    // True allows the window to close, which is default behaviour.
    return (result != 0);  
}


// RectFromAttribs() recalculates the control's rectangle from
// the attributes.  This can only be called after ParseAttributes() has
// loaded the images.
CATResult CATWindow::RectFromAttribs()
{

    CATResult result = CATGuiObj::RectFromAttribs();
    if (CATFAILED(result))
    {
        return result;
    }

    // Adjust position and height from
    fScreenPos.x = fRect.left;
    fScreenPos.y = fRect.top;

    fRect.ZeroOrigin();

    CATInt32 left = 0;
    CATInt32 top = 0;

    // Sizeable windows save their size, too.
    if (this->fSizeable)
    {
        fRect.right  = GetPref(L"Width",fRect.right);
        fRect.bottom = GetPref(L"Height",fRect.bottom);
    }

    bool remember = false;
    remember = this->GetAttribute(L"RememberPos",remember);

    CATString posString = this->GetAttribute(L"XPos");
    CATString tmpPos;

    if (remember)
    {
        fScreenPos.x = GetPref(L"XPos",fScreenPos.x);
        fScreenPos.y = GetPref(L"YPos",fScreenPos.y);
    }


    if (fBaseSize.cx == 0)
        fBaseSize.cx = fRect.Width();
    
    if (fBaseSize.cy == 0)
        fBaseSize.cy = fRect.Height();

    return result;
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATWindow::ParseAttributes()
{
    if (this->fParent)
    {
        if (0 == wcscmp(L"Skin",fParent->GetType()))
        {
            this->fSkin = (CATSkin*)this->fParent;
        }
    }

    CATResult result = CATGuiObj::ParseAttributes();
    CATString attrib;

    fPrimary     = GetAttribute(L"Primary",     fPrimary);
    fSizeable    = GetAttribute(L"Sizeable",    fSizeable);
    fScale       = GetAttribute(L"Scale",       fScale);
    fBaseSize.cx = GetAttribute(L"BaseWidth",   fBaseSize.cx);
    fBaseSize.cy = GetAttribute(L"BaseHeight",  fBaseSize.cy);
    fCaptionBar  = GetAttribute(L"CaptionBar",fCaptionBar);

	 fSetRegion   = GetAttribute(L"SetRegion",   fSetRegion);
	 if (fSetRegion)
	 {
		 fCaptionBar = false;
	 }
    
    if (fCaptionBar == false)
        fDragAnywhere = true;

    fDragAnywhere   = GetAttribute(L"DragAnywhere",fDragAnywhere);
    fAutoDock       = GetAttribute(L"AutoDock",    fAutoDock);
    fIconPath = GetAttribute(L"Icon");

    fIsOverlapped = GetAttribute(L"Overlapped",fIsOverlapped);   
    fTitle = GetAttribute(L"Title");	

    // Overrideable prefs - first get attribute from skin, then override from
    // prefs file if present.
    fScanningEnabled = GetAttribute(L"ScanningEnabled",fScanningEnabled);
    fScanRate        = GetAttribute(L"ScanRate",       fScanRate);

    fScanningEnabled = GetPref(L"ScanningEnabled",fScanningEnabled);
    fScanRate        = GetPref(L"ScanRate",fScanRate);


    return result;
}

bool CATWindow::IsPrimary()
{
    return fPrimary;
}

//---------------------------------------------------------------------------
CATControl* CATWindow::HitTest(const CATPOINT& point)
{
    CATControl* finalHit = 0;
    CATUInt32 numControls = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numControls; i++)
    {
        CATXMLObject* curChild = 0;
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            if (curControl->IsEnabled())
            {
                CATControl* hitControl = curControl->HitTest(point);
                if (hitControl)
                {
                    finalHit = hitControl;
                }
            }         
        }
    }
    return finalHit;
}

void CATWindow::OnMouseMove(const CATPOINT& point, bool leftButton, CATMODKEY modKey)
{
    fLastMouseUpdate = clock();
    fMouseInWindow = true; 
    if (fDragging)
    {
        if (!leftButton)
        {
            fDragging = false;         
            this->SetCursorType(CATCURSOR_ARROW);         
            this->OSEndMouseTrack();
            return;
        }

        CATPOINT screenPoint = point;
        this->OSWndToScreen(screenPoint.x, screenPoint.y);

        if (this->fIsOverlapped == false)
        {
            CATRect newRect = this->OSGetWndRect();
            newRect.Offset(screenPoint.x - fDragLastPoint.x, screenPoint.y - fDragLastPoint.y);
            this->OSMoveWnd(newRect);
        }
        else
        {			
#ifdef _WIN32
            HWND curWnd = this->OSGetWnd();
            HWND parWnd = this->GetBaseWindow();
            RECT parRect;

            ::GetWindowRect(parWnd,&parRect);

            POINT tl;
            tl.x = parRect.left;
            tl.y = parRect.top;

            if (::GetWindowLong(parWnd, GWL_STYLE) & WS_CHILD)
            {
                HWND rootWnd = ::GetParent(parWnd);
                if ((rootWnd != 0) && (rootWnd != parWnd))
                {					
                    ::ScreenToClient(rootWnd, &tl);
                }
            }

            ::SetWindowPos(
                parWnd,
                0,
                tl.x + (screenPoint.x - fDragLastPoint.x), 
                tl.y + (screenPoint.y - fDragLastPoint.y),
                0,
                0,
                SWP_NOSIZE | SWP_NOZORDER);
#endif
        }
        fDragLastPoint = screenPoint;      

        // No need to mark dirty - size isn't changing, just position here.
        // this->MarkDirty();

        // Force update - removes the 'dragging behind' effect.
        this->Update();

        return;
    }

    // Retrieve control under mouse, if any
    CATControl* curControl = this->HitTest(point);

    if (this->fMouseTrackTarget == 0)
    {      
        // No current mouse tracking - just do the control activation and 
        // status bar code.
        if ((curControl) && (curControl->IsFocusable()))
        {
            // Unset previous control if any
            if ((this->fActiveControl) && (fActiveControl != curControl))
            {
                if (fActiveControl)
                {
                    fActiveControl->SetActive(false);
                    fActiveControl = 0;
                }
            }

            // Set the active control
            if (fActiveControl == 0)
            {
                fActiveControl = curControl;
                fActiveControl->SetActive(true);
                this->fCursor = *curControl->GetCursor();
                this->SetStatusBarHint(fActiveControl->GetHint());
            }
        }
        else
        {
            // Unset any controls         
            if (this->fActiveControl)
            {
                if (fActiveControl)
                {
                    fActiveControl->SetActive(false);
                    fActiveControl = 0;
                }
                this->SetStatusBarHint(this->fHintText);
            }
            else
            {
                if (this->fStatusBarHint.Compare(fHintText) != 0)
                {
                    this->SetStatusBarHint(this->fHintText);
                }
            }
            this->SetCursorType(CATCURSOR_ARROW);
        }
    }
    else
    {
        // We're tracking for a control....
        CATPOINT objPoint = point;
        WindowToWidget(fMouseTrackTarget, objPoint);
        this->fMouseTrackTarget->TrackMouseMove(objPoint,leftButton, modKey);
        this->SetStatusBarHint(fMouseTrackTarget->GetHint());

    }

}

void CATWindow::OnMouseLeave()
{
    fMouseInWindow = false;
    if (fActiveControl)
    {
        fActiveControl->SetActive(false);
        fActiveControl = 0;
    }
}

void CATWindow::OnMouseWheel(  const CATPOINT& point,
                             CATFloat32        wheelMove,
                             CATMODKEY			modKey)
{
    if (this->fMouseTrackTarget)
    {
        CATPOINT objPoint = point;
        WindowToWidget(fMouseTrackTarget, objPoint);
        this->fMouseTrackTarget->TrackMouseWheel( objPoint, 
            wheelMove, 
            modKey);
        this->SetStatusBarHint(fMouseTrackTarget->GetHint());
    }
    else
    {
        CATControl* hit = 0;
        if (hit = this->HitTest(point))
        {
            CATPOINT objPoint = point;
            WindowToWidget(hit, objPoint);

            hit->OnMouseWheel(objPoint, wheelMove,modKey);

            this->SetStatusBarHint(hit->GetHint());
        }
    }
}

void CATWindow::OnMouseDown(const CATPOINT& point, CATMODKEY modKey)
{
    CATControl* hitControl = this->HitTest(point);
    if (hitControl)
    {
        this->SetFocus(hitControl);
        OSBeginMouseTrack();      
        this->fMouseTrackTarget = hitControl;
        CATPOINT objPoint = point;
        WindowToWidget(hitControl, objPoint);

        hitControl->TrackMouseDown(objPoint, modKey);

        this->SetStatusBarHint(fMouseTrackTarget->GetHint());

    }
    else
    {
//      KillFocus();

        // Left button down not on a control - begin move if we don't have a caption bar
        if (fDragAnywhere)
        {         
            fDragLastPoint = point;
            this->OSWndToScreen(fDragLastPoint.x, fDragLastPoint.y);
            this->OSBeginMouseTrack();
            this->SetCursorType(CATCURSOR_MOVE);         
            fDragging = true;     
        }
    }
}


void CATWindow::OnRightMouseDown(const CATPOINT& point, CATMODKEY modKey)
{
    CATControl* hitControl = this->HitTest(point);
    if (hitControl)
    {
        fRightTrack = hitControl;
    }
    else
    {
        fRightTrack = 0;
    }
}

void CATWindow::OnRightMouseUp(const CATPOINT& point, CATMODKEY modKey)
{
    if (fRightTrack != 0)
    {
        CATControl* hitControl = this->HitTest(point);
        if (hitControl == fRightTrack)
        {
            // Right-click
            hitControl->OnRightMouseClick();
        }
    }

    fRightTrack = 0;
}

// OnMouseDoubleClick() is called the mouse is double clicked.
//
// \param point - location mouse was released
// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
void CATWindow::OnMouseDoubleClick( const CATPOINT&	point, 
                                   CATMODKEY			modKey)
{
    CATControl* hitControl = this->HitTest(point);
    if (hitControl)
    {
        this->SetFocus(hitControl);
        hitControl->OnMouseDoubleClick(modKey);

    }
    else
    {
        KillFocus();
    }
}


void CATWindow::OnMouseHover(const CATPOINT& point, bool left, CATMODKEY modKey)
{
    CATControl* hitControl = this->HitTest(point);
}

void CATWindow::OnMouseUp(const CATPOINT& point, CATMODKEY modKey)
{
    if (fMouseTrackTarget)
    {
        OSEndMouseTrack();

        CATPOINT objPoint = point;
        WindowToWidget(fMouseTrackTarget, objPoint);
        fMouseTrackTarget->TrackMouseRelease(objPoint, modKey);

        // Mouse release engages a click, which could kill mouse target in some controls.
        if (fMouseTrackTarget)
        {
            this->SetStatusBarHint(fMouseTrackTarget->GetHint());

            fMouseTrackTarget = 0;
        }
    }

    // End moving
    if (fDragging)
    {
        fDragging = false;
        this->SetCursorType(CATCURSOR_ARROW);
        this->OSEndMouseTrack();
    }
}

void CATWindow::SetStatusBarHint(const CATString& hintText)
{
    if (hintText != fStatusBarHint)
    {
        fStatusBarHint = hintText;

        if (fStatusLabel)
        {
            fStatusLabel->SetString(fStatusBarHint);
        }
    }
}

CATString CATWindow::GetStatusBarHint()
{
    return fStatusBarHint;
}


void CATWindow::KillFocus()
{    
    if (fFocusControl != 0)
    {
        CATString focusInfo = fFocusControl->GetName() << L" losing focus.";
        CATTRACE(focusInfo);
        fFocusControl->SetFocused(false);
        fFocusControl = 0;		
    }
}

void CATWindow::SetFocus(CATControl* control)
{
    if (control == 0)
        CATTRACE("Focus set to 0.");
    else    
    {
        CATString focusInfo = control->GetName() << L" receiving focus.";
        CATTRACE(focusInfo);
    
    }

    if (control != fFocusControl)
    {
        KillFocus();
        fFocusControl = control;
        if (fFocusControl)
        {      
            if (fFocusControl == 0)
            {
                CATTRACE("Control reset focus to 0.");
            }
        }
    }

    if (fFocusControl)
    {
        fFocusControl->SetFocused(true);
        SetStatusBarHint(fFocusControl->GetHint());
    }
}


CATResult CATWindow::EnableObject(const CATString& controlName, bool enabled)
{
    CATGuiObj* curObj = Find(controlName);

    if (curObj == 0)
    {
        return CATRESULT(CAT_ERR_CONTROL_NOT_FOUND);
    }

    curObj->SetEnabled(enabled);
    return CAT_SUCCESS;
}

bool CATWindow::FindControlCallback(CATControl* curControl, void* userParam)
{
    CATFINDCONTROLSTRUCT* ctrlStruct = (CATFINDCONTROLSTRUCT*)userParam;
    ctrlStruct->CurControl = curControl;

    if (curControl->GetName().Compare(ctrlStruct->Name) == 0)
    {
        if (0 == wcscmp(curControl->GetType(),ctrlStruct->TypeName))
        {
            ctrlStruct->FoundControl = curControl;
            return false;
        }
    }
    return true;
}

CATControl* CATWindow::FindControlAndVerify(const CATString& controlName, 
                                            const CATString& typeName,
                                            CATUInt32* index)
{   
    CATFINDCONTROLSTRUCT ctrlStruct;
    ctrlStruct.Name         = controlName;
    ctrlStruct.TypeName     = typeName;
    ctrlStruct.UpControl    = 0;
    ctrlStruct.DownControl  = 0;
    ctrlStruct.LeftControl  = 0;
    ctrlStruct.RightControl = 0;
    ctrlStruct.FoundControl = 0;
    ctrlStruct.PrevControl  = 0;
    ctrlStruct.FirstControl = 0;
    ctrlStruct.NextControl  = 0;
    ctrlStruct.CurControl   = 0;
    ctrlStruct.TargetControl= 0;

    ForEachControl(FindControlCallback, &ctrlStruct);

    // MAEDEBUG - set index here!

    return ctrlStruct.FoundControl;
}

bool CATWindow::FindDirectionalCallback(CATControl* curControl, void* userParam)
{
    CATFINDCONTROLSTRUCT* ctrlStruct = (CATFINDCONTROLSTRUCT*)userParam;
    // Ignore stuff that's not available.
    if ((curControl->IsEnabled() == false) || (curControl->IsFocusable() == false) || (curControl->IsVisible() == false))
    {
        return true;
    }
    
    if ((ctrlStruct->TargetControl == curControl))
    {
        return true;
    }

	 if (curControl->NeedsArrows())
		 return true;

    if (!ctrlStruct->TargetControl)
    {
        ctrlStruct->LeftControl = ctrlStruct->RightControl 
                                = ctrlStruct->UpControl 
                                = ctrlStruct->DownControl 
                                = ctrlStruct->FirstControl;
        return true;
    }

    CATRect targetRect = ctrlStruct->TargetControl->GetRect();
    CATRect testRect   = curControl->GetRect();

    if ((testRect.left <= targetRect.right) && (testRect.right >= targetRect.left))
    {
        // hits for up and down... check which
        if (testRect.CenterY() > targetRect.CenterY())
        {
            // below
            if (ctrlStruct->DownControl == 0)
                ctrlStruct->DownControl = curControl;
            else if (testRect.CenterY() < ctrlStruct->DownControl->GetRect().CenterY() )
                ctrlStruct->DownControl = curControl;
        }

        if (testRect.CenterY() < targetRect.CenterY())
        {
            // above
            if (ctrlStruct->UpControl == 0)
                ctrlStruct->UpControl = curControl;
            else if (testRect.CenterY() > ctrlStruct->UpControl->GetRect().CenterY() )
                ctrlStruct->UpControl = curControl;
        }
    }

    if ((testRect.top <= targetRect.bottom) && (testRect.bottom >= targetRect.top))
    {
        if (testRect.CenterX() > targetRect.CenterX())
        {
            // right
            if (ctrlStruct->RightControl == 0)
                ctrlStruct->RightControl = curControl;
            else if (testRect.CenterX() < ctrlStruct->RightControl->GetRect().CenterX() )
                ctrlStruct->RightControl = curControl;
        }

        if (testRect.CenterX() < targetRect.CenterX())
        {
            // left
            if (ctrlStruct->LeftControl == 0)
                ctrlStruct->LeftControl = curControl;
            else if (testRect.CenterX() > ctrlStruct->LeftControl->GetRect().CenterX() )
                ctrlStruct->LeftControl = curControl;
        }
    }

    return true;
}

bool CATWindow::GetControlSiblingsCallback(CATControl* curControl, void* userParam)
{
    CATFINDCONTROLSTRUCT* ctrlStruct = (CATFINDCONTROLSTRUCT*)userParam;

    // Ignore stuff that's not available.
    if ((curControl->IsEnabled() == false) || (curControl->IsFocusable() == false) || (curControl->IsVisible() == false))
    {
        return true;
    }

    // Set first control if unset.
    if (ctrlStruct->FirstControl == 0)
    {
        ctrlStruct->FirstControl = curControl;
    }

    // If target control is 0, then first is our target.
    if (ctrlStruct->TargetControl == 0)
    {
        if (ctrlStruct->NextControl == 0)
        {         
            ctrlStruct->NextControl = curControl;
        }

        // Prev control is the last one we visit.
        ctrlStruct->PrevControl = curControl;
        ctrlStruct->CurControl = curControl;
        return true;
    }
    else if (curControl == ctrlStruct->TargetControl)
    {
        ctrlStruct->PrevControl = ctrlStruct->CurControl;
        ctrlStruct->FoundControl = curControl;
        ctrlStruct->CurControl = curControl;
        return true;
    }
    else if ((ctrlStruct->FoundControl != 0) && (ctrlStruct->NextControl == 0))
    {
        ctrlStruct->NextControl = curControl;
    }

    ctrlStruct->CurControl = curControl;

    return true;
}

void CATWindow::GetControlStruct(CATFINDCONTROLSTRUCT& ctrlStruct, CATControl* control)
{
    ctrlStruct.FoundControl    = 0;
    ctrlStruct.PrevControl     = 0;
    ctrlStruct.FirstControl    = 0;
    ctrlStruct.NextControl     = 0;
    ctrlStruct.CurControl      = 0;
    ctrlStruct.UpControl       = 0;
    ctrlStruct.DownControl     = 0;
    ctrlStruct.LeftControl     = 0;
    ctrlStruct.RightControl    = 0;
    ctrlStruct.TargetControl   = control;   
    ForEachControl(GetControlSiblingsCallback, &ctrlStruct);
    ForEachControl(FindDirectionalCallback,&ctrlStruct);
}

void CATWindow::SetFocusNext()
{
    CATFINDCONTROLSTRUCT ctrlStruct;

    GetControlStruct(ctrlStruct,this->fFocusControl);

    if (ctrlStruct.NextControl)
    {
        CATTRACE((CATString)"SetFocusNext from: " << (CATString)(fFocusControl?fFocusControl->GetName():"Null") << " to " << ctrlStruct.NextControl->GetName());

        this->SetFocus(ctrlStruct.NextControl);
    }
    else if (ctrlStruct.FirstControl)
    {
        this->SetFocus(ctrlStruct.FirstControl);
    }
    else
    {
        KillFocus();
    }

    // MAEDEBUG - 
    // Gah. Rework for layers...
}

void CATWindow::SetFocusPrev()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    ctrlStruct.FoundControl    = 0;
    ctrlStruct.PrevControl     = 0;
    ctrlStruct.FirstControl    = 0;
    ctrlStruct.NextControl     = 0;
    ctrlStruct.CurControl      = 0;
    ctrlStruct.UpControl       = 0;
    ctrlStruct.DownControl     = 0;
    ctrlStruct.LeftControl     = 0;
    ctrlStruct.RightControl    = 0;
    ctrlStruct.TargetControl   = this->fFocusControl;   

    ForEachControl(GetControlSiblingsCallback, &ctrlStruct);

    if (ctrlStruct.PrevControl)
    {
        CATTRACE((CATString)"SetFocusPrev from: " << (CATString)(fFocusControl?fFocusControl->GetName():"Null")  << " to " << ctrlStruct.PrevControl->GetName());
        this->SetFocus(ctrlStruct.PrevControl);
    }
    else if (ctrlStruct.CurControl)
    {
        this->SetFocus(ctrlStruct.CurControl);
    }
    else
    {
        KillFocus();
    }
    // MAEDEBUG - 
    // Gah. Rework for layers...
}

void CATWindow::SetFocusFirst()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    GetControlStruct(ctrlStruct,this->fFocusControl);

    if (ctrlStruct.FirstControl) 
        this->SetFocus(ctrlStruct.FirstControl);
//    else if (ctrlStruct.PrevControl)  this->SetFocus(ctrlStruct.PrevControl);
//    else if (ctrlStruct.FirstControl) this->SetFocus(ctrlStruct.FirstControl);
//    else    KillFocus();
}

void CATWindow::SetFocusUp()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    GetControlStruct(ctrlStruct,this->fFocusControl);    
    if (ctrlStruct.UpControl)    this->SetFocus(ctrlStruct.UpControl);

// This is debateable.... right now I prefer it only moving to the control
// directly in the direction it is requested, as this allows one to reach
// corner controls on a well laid-out GUI without using sight at all - 
// just pound the up and left keys a lot to get to the top-left control. - mae

//    else if (ctrlStruct.PrevControl)  this->SetFocus(ctrlStruct.PrevControl);
//    else if (ctrlStruct.FirstControl) this->SetFocus(ctrlStruct.FirstControl);
//    else    KillFocus();
}

void CATWindow::SetFocusLeft()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    GetControlStruct(ctrlStruct,this->fFocusControl);

         if (ctrlStruct.LeftControl)  this->SetFocus(ctrlStruct.LeftControl);
//    else if (ctrlStruct.PrevControl)  this->SetFocus(ctrlStruct.PrevControl);
//    else if (ctrlStruct.FirstControl) this->SetFocus(ctrlStruct.FirstControl);
//    else    KillFocus();
}
void CATWindow::SetFocusRight()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    GetControlStruct(ctrlStruct,this->fFocusControl);

         if (ctrlStruct.RightControl) this->SetFocus(ctrlStruct.RightControl);
//    else if (ctrlStruct.NextControl)  this->SetFocus(ctrlStruct.NextControl);
//    else if (ctrlStruct.FirstControl) this->SetFocus(ctrlStruct.FirstControl);
//    else    KillFocus();
}

void CATWindow::SetFocusDown()
{
    CATFINDCONTROLSTRUCT ctrlStruct;
    GetControlStruct(ctrlStruct,this->fFocusControl);

         if (ctrlStruct.DownControl)  this->SetFocus(ctrlStruct.DownControl);
//    else if (ctrlStruct.NextControl)  this->SetFocus(ctrlStruct.NextControl);
//    else if (ctrlStruct.FirstControl) this->SetFocus(ctrlStruct.FirstControl);
//    else    KillFocus();
}


CATControl* CATWindow::GetFocusControl()
{
    return this->fFocusControl;
}

void CATWindow::OnKeyDown(const CATKeystroke& keystroke)
{
    if (this->fFocusControl != 0)
    {
        fFocusControl->OnKeyDown(keystroke);
        this->SetStatusBarHint(fFocusControl->GetHint());

    }
}

void CATWindow::OnKeyUp(const CATKeystroke& keystroke)
{
    if (this->fFocusControl != 0)
    {
        fFocusControl->OnKeyUp(keystroke);
    }
}

void CATWindow::OnKeyPress(const CATKeystroke& keystroke)
{
    if (this->fFocusControl != 0)
    {
        fFocusControl->OnKeyPress(keystroke);      
    }
}

void CATWindow::OnHelp()
{
    gApp->OnHelp();
}


void CATWindow::OnClose(CATCommand& command, CATControl* ctrl)
{
    this->Hide(true);
}

void CATWindow::OnMinimize(CATCommand& command, CATControl* ctrl)
{
    this->OSMinimize();
}

void CATWindow::OnMaximize(CATCommand& command, CATControl* ctrl)
{
    this->OSMaximize();
}

const CATWindow::CATWINDOWCMDFUNC BaseCmdTable[3] =
{
    "Close",    &CATWindow::OnClose,		false,	false,
    "Minimize", &CATWindow::OnMinimize,	    false,	false,
    "Maximize", &CATWindow::OnMaximize,	    false,	false
};

void CATWindow::OnCommand(CATCommand& command, CATControl* ctrl)
{	
    CATResult result = ProcessCommandTable(command,ctrl,BaseCmdTable,3,false);


    if (CATFAILED(result))
    {
        // If we have a parent skin (not a template) then
        // pass command up to it if we're unaware of how to
        // handle it.
        CATSkin* skin = (CATSkin*)this->fParent;
        if (skin != 0)
        {                      
            CATCURSORTYPE oldType = fCursor.GetType();
            this->SetCursorType(CATCURSOR_WAIT);
            if (this->IsScanning())
                PauseScanning();
// This might be nice to automatically do optionally, but
// for instant commands it makes it blinky.
//
//            this->SetEnabled(false);
//            this->Update();

            skin->OnCommand(command, ctrl, this);   
            
            this->SetCursorType(oldType);
            if (this->IsScanning())
                ResumeScanning();
//            this->SetEnabled(true);
        }
    }


    if (ctrl)
    {      
        SetStatusBarHint(ctrl->GetHint());
    }
}


void CATWindow::OnThreadedCommand(CATCommand& command)
{	
    CATResult result = ProcessCommandTable(command,0,BaseCmdTable,3,true);
}


void CATWindow::OnMouseTimer(CATMODKEY modKey)
{
    CATFloat32 timeSinceLast = ((CATFloat32)(clock() - fLastMouseUpdate)) / (CATFloat32)(CLOCKS_PER_SEC);
    if ( timeSinceLast  < 0.01f )
    {
        return;
    }

    fLastMouseUpdate = clock();

    if (fMouseTrackTarget)
    {
        this->fMouseTrackTarget->TrackMouseTimer(modKey);
    }
}

CATWND CATWindow::OSGetWnd()
{
    return fWindow;
}

// RegCtlWnd() registers a window handle from a CATControlWnd.
// This allows us to send messages w/o doing full scans for the
// control through the skin's tree.
//
// \param wndCtl - window control
// \param wndHandle - window handle of the control
// \return CATResult - CAT_SUCCESS on success
// \sa UnRegCtlWnd()
CATResult CATWindow::RegCtlWnd(CATControlWnd* wndCtl, CATWND wndHandle)
{
    this->fCWT.insert(std::make_pair(wndHandle,wndCtl));
    return CAT_SUCCESS;
}

/// UnRegCtlWnd() unregisters a window handle from a CATControlWnd.
/// This should be called when the control destroys its handle to
/// prevent mis-sends.
/// \param wndHandle - old window handle of the control
/// \return CATResult - CAT_SUCCESS on success
/// \sa RegCtlWnd()
CATResult CATWindow::UnRegCtlWnd(CATWND wndHandle)
{
    CATControlWnd* wndCtl = 0;
    std::map<CATWND, CATControlWnd*>::iterator iter = fCWT.find(wndHandle);
    if (iter != fCWT.end())
    {
        fCWT.erase(iter);    
        return CAT_SUCCESS;
    }   
    return CAT_ERR_INVALID_PARAM;
}

// GetCtlWnd() retrieves a CATControlWnd for a given
// handle, or 0 if none is registered.
CATControlWnd* CATWindow::GetCtlWnd(CATWND wndHandle)
{
    CATControlWnd* wndCtl = 0;
    std::map<CATWND, CATControlWnd*>::iterator iter = fCWT.find(wndHandle);
    if (iter != fCWT.end())
    {
        wndCtl = iter->second;
    }   
    return wndCtl;
}

void CATWindow::SetCursorType(CATCURSORTYPE type)
{
    this->fCursor.SetType(type);
    OSSetCursor();
}

void CATWindow::SetEnabled(bool enabled)
{
    CATGuiObj::SetEnabled(enabled);
#ifdef _WIN32
    if (this->fWindow != 0)
    {
        ::EnableWindow(this->OSGetWnd(), enabled);
    }		
#endif

    if (enabled == false)
    {
        this->SetCursorType(CATCURSOR_WAIT);
    }
    else
    {
        this->SetCursorType(CATCURSOR_ARROW);
    }

    CATInt32 dummy = 0;
    this->OnEvent(CATEvent(CATEVENT_ENABLE_CHANGE), dummy);
    this->MarkDirty();
}  


bool  CATWindow::IsTemplate()
{
    return fIsTemplate;
}

void  CATWindow::SetIsTemplate(bool isTemplate)
{
    fIsTemplate = isTemplate;
}

CATResult CATWindow::PostEvent(CATEvent* event)
{

    CATResult result = CAT_SUCCESS;
    if (CATFAILED(result = fEventLock.Wait()))
    {
        return result;
    }

    if (fExiting)
    {
        delete event;
        fEventLock.Release();
        return CAT_SUCCESS;
    }

    result = fEventQueue.Queue(event);

    fEventLock.Release();
    PostEventNotification();
    return result;
}

void CATWindow::ProcessPostedEvent()
{
    CATResult result = CAT_SUCCESS;
    if (CATFAILED(result = fEventLock.Wait()))
    {
        return;
    }

    CATEvent* event = 0;
    while (CATSUCCEEDED(result = fEventQueue.Next(event)))
    {
        if ((CATSUCCEEDED(result)) && (event != 0))
        {
            CATInt32 retVal = 0;
            this->OnEvent( *event,retVal);
            delete event;
        }
    }
    fEventLock.Release();
}

bool CATWindow::IsOverlapped()
{
    return fIsOverlapped;
}



// CAllback for SEtKnobOverride
bool CATWindow::SetKnobCallback(CATControl* curControl, void* userParam)
{
    CATKNOB_STYLE knobStyle = (CATKNOB_STYLE)(int)userParam;
    if (wcscmp(curControl->GetType(),L"Knob") == 0)
    {
        ((CATKnob*)curControl)->SetKnobStyle(knobStyle);
    }
    return true;
}


void CATWindow::SetKnobOverride(CATKNOB_STYLE knobStyle)
{
    this->ForEachControl(SetKnobCallback, (void *)knobStyle);
}

/// GetWindow() retrieves the parent window.
CATWindow* CATWindow::GetWindow() const
{
    return (CATWindow*)this;
}      

CATRect CATWindow::GetRectAbs(bool screenCoordinates)
{
    CATRect rect = fRect;

    if (screenCoordinates)
    {
        CATPOINT screenPoint;
        screenPoint.x = 0;
        screenPoint.y = 0;

        OSWndToScreen(screenPoint.x, screenPoint.y);
        rect.Offset(screenPoint.x, screenPoint.y);
    }

    return rect;
}


void CATWindow::WindowToWidget( CATGuiObj* widget, CATPOINT& pt)
{
    if (widget == 0)
        return;

    CATGuiObj* parObj = (CATGuiObj*)widget->GetParent();

    if (parObj)
    {
        CATRect offsetRect = parObj->GetRectAbs();
        pt.x -= offsetRect.left;
        pt.y -= offsetRect.top;
    }
}

void CATWindow::WindowToWidget( CATGuiObj* widget, CATRect& rect)
{
    if (widget == 0)
        return;

    CATGuiObj* parObj = (CATGuiObj*)widget->GetParent();

    if (parObj)
    {
        CATRect offsetRect = parObj->GetRectAbs();
        rect.Offset( -offsetRect.left, -offsetRect.top);
    }
}

void CATWindow::WidgetToWindow( CATGuiObj* widget, CATPOINT& pt)
{
    if (widget == 0)
        return;

    CATGuiObj* parObj = (CATGuiObj*)widget->GetParent();

    if (parObj)
    {
        CATRect offsetRect = parObj->GetRectAbs();
        pt.x += offsetRect.left;
        pt.y += offsetRect.top;
    }
}

void CATWindow::WidgetToWindow( CATGuiObj* widget, CATRect& rect)
{
    if (widget == 0)
        return;

    CATGuiObj* parObj = (CATGuiObj*)widget->GetParent();

    if (parObj)
    {
        CATRect offsetRect = parObj->GetRectAbs();
        rect.Offset( offsetRect.left, offsetRect.top);
    }
}

CATResult CATWindow::OnDropFile(const CATString& filePath)
{
    return CAT_SUCCESS;
}

CATResult CATWindow::ProcessCommandTable(   CATCommand&              cmd,
                                         CATControl*              control,
                                         const CATWINDOWCMDFUNC*  cmdTable,
                                         CATUInt32                 cmdTableLen,
                                         bool							inThread)

{   
    this->SetFocus(control);

    for (CATUInt32 i = 0; i < cmdTableLen; i++)
    {
        CATString cmdString = cmd.GetCmdString();
        if (cmdString.Compare( cmdTable[i].CommandName) == 0)
        {
            if ((cmdTable[i].Threaded) && (!inThread))
            {
                if (cmdTable[i].DisableWindow)
                    this->SetEnabled(false);


                this->PostThreadedCommand(cmd);
            }
            else
            {				
                if (cmdTable[i].DisableWindow)
                {
                    this->SetEnabled(false);
                    this->Update();
                }

                (this->*cmdTable[i].CommandFunc)(cmd,control);


                if (cmdTable[i].DisableWindow)
                    this->SetEnabled(true);
            }

            return CAT_SUCCESS;
        }
    }

    return CATRESULT(CAT_ERR_CMD_NOT_FOUND);
}

void CATWindow::OnNoop(CATCommand& command, CATControl* ctrl)
{
    CATTRACE("NOOP command - not implemented.");
}



CATResult CATWindow::SendEvent( const CATEvent& event, CATInt32& lRes)
{

    CATResult result = gApp->OnEvent(event, lRes);
    if (this->IsTemplate())
    {
        result = this->OnEvent(event,lRes);
    }

    return result;
}


void CATWindow::DisplayMessage( const CATString& msg)
{
    if (gApp != 0)
    {
        gApp->DisplayMessage(msg,this);
    }
}

void CATWindow::DisplayError( const CATResult& result)
{
    if (gApp != 0)
    {
        gApp->DisplayError(result,this);
    }
}

CATResult CATWindow::DisplayPrompt( const CATString& msg, CATApp::CATPROMPTTYPE prompt)
{
    CATResult result = CAT_SUCCESS;
    if (gApp != 0)
    {
        result = gApp->DisplayPrompt(msg,prompt,this);
    }
    return result;
}

CATResult CATWindow::GetScale(CATFloat32& xScale, CATFloat32& yScale)
{
    xScale = 1.0f;
    yScale = 1.0f;
    return CAT_ERR_NOT_IMPLEMENTED;
}

void CATWindow::SetVisible(bool visible)
{
    CATGuiObj::SetVisible(visible);

    if (!visible)
        OSHideWnd();
    else
        OSShowWnd();
}


bool CATWindow::IsScanning()
{
    return fScanningEnabled;
}

void CATWindow::OnScanningTimer()
{
    this->SetFocusNext();
}

CATFloat32 CATWindow::GetScanRate()
{
    return fScanRate;
}

