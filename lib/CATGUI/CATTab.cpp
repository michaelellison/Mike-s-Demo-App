//---------------------------------------------------------------------------
/// \file CATTab.cpp
/// \brief GUI Layer object - contains sub-controls
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $
//
//
//---------------------------------------------------------------------------
#include "CATTab.h"
#include "CATApp.h"
#include "CATFileSystem.h"
#include "CATStream.h"
#include "CATEvent.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

//---------------------------------------------------------------------------
// CATTab constructor (inherited from CATGuiObj)
// \param element - Type name (e.g. "Button", "Label", etc.)
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
//---------------------------------------------------------------------------
CATTab::CATTab(  const CATString&             element, 
               const CATString&               rootDir)
               : CATLayer(element,  rootDir)
{
    fCurTab  = 0;
    fVisible = false;
}

//---------------------------------------------------------------------------
// CATTab destructor
//---------------------------------------------------------------------------
CATTab::~CATTab()
{
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATTab::ParseAttributes()
{
    CATResult result = CATLayer::ParseAttributes();
    CATString attrib;

    fCurTab = GetAttribute(L"StartTab",fCurTab);

    return result;

}



//---------------------------------------------------------------------------
CATControl* CATTab::HitTest(const CATPOINT& point)
{
    if (!fRect.InRect(point))
    {
        return 0;
    }

    CATPOINT layerPt = point;
    layerPt.x -= fRect.left;
    layerPt.y -= fRect.top;   

    CATUInt32 numControls = this->GetNumChildren();
    if ((fCurTab < 0) || (fCurTab >= numControls))
    {
        CATASSERT(false,"Invalid value for fCurTab!");
        fCurTab = 0;
    }

    CATControl* finalHit = 0;

    CATXMLObject* curChild = GetChild(fCurTab);
    if (curChild)
    {
        CATWidget* curControl = (CATWidget*)curChild;
        if (curControl->IsEnabled())
        {
            CATControl* hitControl = curControl->HitTest(layerPt);
            if (hitControl)
            {
                finalHit = hitControl;
            }
        }         
    }

    return finalHit;
}

//---------------------------------------------------------------------------
void CATTab::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }

    // Layer offset
    CATRect layerRect;
    if (!fRect.Intersect( dirtyRect, &layerRect))
        return;

    layerRect.Offset( -fRect.left, -fRect.top);

    CATUInt32 numControls = this->GetNumChildren();
    if ((fCurTab < 0) || (fCurTab >= numControls))
    {
        CATASSERT(false,"Invalid value for fCurTab!");
        fCurTab = 0;
    }

    CATImage* subImage = 0;

    if (CATSUCCEEDED(CATImage::CreateSub( image, 
        subImage, 
        fRect.left, 
        fRect.top, 
        CATMin(image->Width() - fRect.left, fRect.Width()), 
        CATMin(image->Height() - fRect.top, fRect.Height()) )))
    {
        // Draw children
        CATXMLObject* curChild = GetChild(fCurTab);
        if (curChild)
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->Draw(subImage,layerRect);         
        }

        CATImage::ReleaseImage(subImage);
    }
}
//---------------------------------------------------------------------------
void CATTab::PostDraw(CATDRAWCONTEXT context, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
		 return;
    // Offset rectangle
    CATRect layerRect;
    if (!fRect.Intersect( dirtyRect, &layerRect))
        return;
    layerRect.Offset( -fRect.left, -fRect.top);


    CATUInt32 numControls = this->GetNumChildren();
    if ((fCurTab < 0) || (fCurTab >= numControls))
    {
        CATASSERT(false,"Invalid value for fCurTab!");
        fCurTab = 0;
    }

    // Call children
    CATXMLObject* curChild = GetChild(fCurTab);
    if (curChild)
    {
        CATWidget* curControl = (CATWidget*)curChild;
        curControl->PostDraw(context,layerRect);
    }
}


CATResult CATTab::OnEvent(  const CATEvent&  event, CATInt32& retVal)
{
    // If the event is for tab hiding / showing, only
    // pass through to active tab - otherwise parent tabs
    // will totally hose us!
    if (  (event.fEventCode == CATEVENT_TAB_SHOW) ||
        (event.fEventCode == CATEVENT_TAB_HIDE) )
    {
        CATUInt32 numControls = this->GetNumChildren();
        if ((fCurTab < 0) || (fCurTab >= numControls))
        {
            CATASSERT(false,"Invalid value for fCurTab!");
            fCurTab = 0;
        }
        CATXMLObject* curChild = GetChild(fCurTab);
        if (curChild)
        {
            CATWidget* curControl = (CATWidget*)curChild;
            return curControl->OnEvent( event, retVal);
        }
    }

    return CATLayer::OnEvent(event,retVal);
}


CATResult CATTab::SetCurTab( CATUInt32 curTab )
{
    if (curTab >= this->GetNumChildren())
    {
        return CATRESULT(CAT_ERR_INVALID_TAB);
    }
	 CATUInt32 oldTab = fCurTab;
	 fCurTab = curTab;
	 
    // Only send show/hide events if the tab itself is
    // visible.
    if (this->IsVisible())
    {
		  
        CATInt32 retVal;
		  CATXMLObject* curChild;
		  curChild = this->GetChild(oldTab);
		  if (curChild)
				((CATWidget*)curChild)->OnEvent(CATEVENT_TAB_HIDE,retVal);
		  
		  curChild = this->GetChild(fCurTab);
		  if (curChild)
		  {
			  ((CATWidget*)curChild)->OnEvent(CATEVENT_TAB_SHOW,retVal);
		  }
	 }
    
	 GetWindow()->ResetBackground(); 
	 MarkDirty(0,true);
	 return CAT_SUCCESS;
}

CATUInt32 CATTab::GetCurTab( )
{
    return fCurTab;
}

CATUInt32 CATTab::GetNumTabs()
{
    return this->GetNumChildren();
}

CATResult CATTab::Load(CATPROGRESSCB							progressCB,
                       void*						progressParam,
                       CATFloat32						progMin,
                       CATFloat32						progMax)
{
    CATResult result = CATLayer::Load(progressCB, progressParam, progMin, progMax);
    this->SetCurTab(fCurTab);

    return result;
}

void CATTab::OnParentCreate()
{
    CATLayer::OnParentCreate();

    this->fVisible = true;
    this->SetCurTab(fCurTab);
}

bool CATTab::GetPostRects(CATStack<CATRect>& rectStack)
{
    CATUInt32 numControls = this->GetNumChildren();
    if ((fCurTab < 0) || (fCurTab >= numControls))
    {
        CATASSERT(false,"Invalid value for fCurTab!");
        fCurTab = 0;
    }

    // Call children
    CATXMLObject* curChild = GetChild(fCurTab);
    if (curChild)
    {
        CATWidget* curControl = (CATWidget*)curChild;
        return curControl->GetPostRects(rectStack);
    }

    return false;
}

bool CATTab::IsVisible(const CATGuiObj* object) const
{   
    if (fVisible)
    {
        if (object == 0)
        {
            return ((CATGuiObj*)fParent)->IsVisible(this);
        }

        // Check child
        CATXMLObject* curChild = GetChild(fCurTab);
        if (curChild)
        {
            if ((CATGuiObj*)curChild == object)
            {
                return ((CATGuiObj*)fParent)->IsVisible(this);
            }         
        }
    }
    return false;   
}
