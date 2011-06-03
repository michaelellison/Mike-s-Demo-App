//---------------------------------------------------------------------------
/// \file CATIconButton.cpp
/// \brief Specialized pushbutton with image icon
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
#include "CATIconButton.h"
#include "CATWindow.h"
#include "CATOSFuncs.h"
//---------------------------------------------------------------------------
/// CATIconButton constructor (inherited from CATControl->CATXMLObject)
/// \param element - Type name ("Button")
/// \param rootDir - root directory of skin
//---------------------------------------------------------------------------
CATIconButton::CATIconButton(  const CATString&             element, 
                             const CATString&             rootDir)
: CATButton(element, rootDir)
{
    fIconImage    = 0;    
    fIconDisabled = 0;
}



//---------------------------------------------------------------------------
/// CATIconButton destructor
//---------------------------------------------------------------------------
CATIconButton::~CATIconButton()
{
    if (fIconImage)
    {
        CATImage::ReleaseImage(fIconImage);
    }

    if (fIconDisabled)
    {
        CATImage::ReleaseImage(fIconDisabled);
    }
}


/// ParseAttributes() parses the known attributes for an object.
CATResult CATIconButton::ParseAttributes()
{
    CATResult result = CATButton::ParseAttributes();
    CATString attrib;

    attrib = GetAttribute(L"IconImage");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fIconImage);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }
    attrib = GetAttribute(L"IconDisabled");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fIconDisabled);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    return result;
}

/// Draw() draws the control into the parent's image
/// \param image - parent image to draw into
/// \param dirtyRect - portion of control (in window coordinates)
///        that requires redrawing.
void CATIconButton::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }

    CATButton::Draw(image,dirtyRect);
    

    // sanity check parent image / dirty rectangle
    CATRect imgRect(0,0,image->Width(), image->Height());
    CATASSERT(imgRect.Inside(dirtyRect), "Update rect is outside of img rect!");

    // Find intersection between dirty rect and us
    CATRect drawRect;
    bool   drawn = false;


    CATRect innerRect;

    innerRect = fRect;
    if (fIconImage)
    {
        innerRect.left  = fRect.left + (fRect.Width() - fIconImage->Width())/2;
        innerRect.right = innerRect.left + fIconImage->Width();
        innerRect.top   = fRect.top  + (fRect.Height() - fIconImage->Height())/2;
        innerRect.bottom= innerRect.top  + fIconImage->Height();
    }

    if (this->IsPressed())
    {
        CATPOINT iconOff;
        iconOff.x = this->fTextOffsetPressed.x - fTextOffset.x;
        iconOff.y = this->fTextOffsetPressed.y - fTextOffset.y;
        innerRect.Offset(iconOff);
    }

    // Gracefully degrade depending on flags and whether the images are
    // available.
    if (innerRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;
        if ( (this->IsEnabled() == false) && (this->fIconDisabled))
        {
            if (drawRect.Intersect(CATRect( innerRect.left, 
                                            innerRect.top, 
                                            innerRect.left + fIconDisabled->Width(),
                                            innerRect.top  + fIconDisabled->Height()),
                                    &ourRect))
            {
                ourRect.Offset(-innerRect.left, -innerRect.top);

                image->Overlay( this->fIconDisabled,
                                drawRect.left, 
                                drawRect.top, 
                                ourRect.left,
                                ourRect.top,
                                ourRect.Width(),
                                ourRect.Height());
                drawn = true;
            }        
        }

        if ((!drawn) && (this->fIconImage != 0))
        {
            if (drawRect.Intersect( CATRect(innerRect.left, 
                                            innerRect.top, 
                                            innerRect.left + fIconImage->Width(),
                                            innerRect.top  + fIconImage->Height()),
                                    &ourRect))
            {
                ourRect.Offset(-innerRect.left, -innerRect.top);

                image->Overlay( this->fIconImage,
                                drawRect.left,
                                drawRect.top,
                                ourRect.left,
                                ourRect.top,
                                ourRect.Width(),
                                ourRect.Height());
                drawn = true;
            }
        }
    }
}
