//---------------------------------------------------------------------------
/// \file CATLayer.cpp
/// \brief GUI Layer object - contains sub-controls
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
#include "CATLayer.h"
#include "CATApp.h"
#include "CATFileSystem.h"
#include "CATStream.h"
#include "CATEvent.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

//---------------------------------------------------------------------------
// CATLayer constructor (inherited from CATGuiObj)
// \param element - Type name (e.g. "Button", "Label", etc.)
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
//---------------------------------------------------------------------------
CATLayer::CATLayer(  const CATString&             element, 
                   const CATString&               rootDir)
                   : CATWidget(element,  rootDir)
{
}

//---------------------------------------------------------------------------
// CATLayer destructor
//---------------------------------------------------------------------------
CATLayer::~CATLayer()
{
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATLayer::ParseAttributes()
{
    return CATWidget::ParseAttributes();
}



//---------------------------------------------------------------------------
CATControl* CATLayer::HitTest(const CATPOINT& point)
{
    if (!fRect.InRect(point))
    {
        return 0;
    }

    CATPOINT layerPt = point;
    layerPt.x -= fRect.left;
    layerPt.y -= fRect.top;
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
                // In case of overlapped, always use the last in order.
                CATControl* hitControl = curControl->HitTest(layerPt);
                if (hitControl)
                {
                    finalHit = hitControl;
                }
            }         
        }
    }

    return finalHit;
}

//---------------------------------------------------------------------------
void CATLayer::Draw(CATImage* image, const CATRect& dirtyRect)
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

    CATImage* subImage = 0;

    if (CATSUCCEEDED(CATImage::CreateSub( image, 
        subImage, 
        fRect.left, 
        fRect.top, 
        CATMin(image->Width() - fRect.left, fRect.Width()), 
        CATMin(image->Height() - fRect.top, fRect.Height()) )))
    {
        // Draw children
        CATXMLObject* curChild = 0;
        CATUInt32 index;
        CATUInt32 numChildren = this->GetNumChildren();
        for (index = 0; index < numChildren; index++)
        {
            if (0 != (curChild = GetChild(index)))
            {
                CATWidget* curControl = (CATWidget*)curChild;
                curControl->Draw(subImage,layerRect);         
            }
        }

        CATImage::ReleaseImage(subImage);
    }
}
//---------------------------------------------------------------------------
void CATLayer::PostDraw(CATDRAWCONTEXT context, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }
    // Offset rectangle
    CATRect layerRect;
    if (!fRect.Intersect( dirtyRect, &layerRect))
        return;
    layerRect.Offset( -fRect.left, -fRect.top);

    // Call children
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->PostDraw(context,layerRect);
        }
    }   
}

// OnParentCreate() is called when the parent window is created.
// 
// Most controls won't need this, but any that create their own
// windows should do so at this point.
void CATLayer::OnParentCreate()
{
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->OnParentCreate();
        }
    }   
}

// OnParentDestroy() is called as the parent window is destroyed.
//
// Controls that create their own windows during OnParentCreate()
// should destroy them during OnParentDestroy()
void CATLayer::OnParentDestroy()
{
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            curControl->OnParentDestroy();
        }
    }   
}

void CATLayer::SetEnabled(bool enabled)
{
    CATWidget::SetEnabled(enabled);

    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATWidget* curControl = (CATWidget*)curChild;
            CATInt32 evRes = 0;
            (void)curControl->OnEvent(CATEvent(CATEVENT_ENABLE_CHANGE),evRes);
        }
    }     
}
