//---------------------------------------------------------------------------
/// \file CATIconSwitch.cpp
/// \brief Icon-based On/Off switch for GUI
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
#include "CATIconSwitch.h"
#include "CATApp.h"
#include "CATWindow.h"
//---------------------------------------------------------------------------
// CATIconSwitch constructor (inherited from CATSwitch->CATXMLObject)
// \param element - Type name ("Switch")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png loads
//---------------------------------------------------------------------------
CATIconSwitch::CATIconSwitch(  const CATString&             element, 
                               const CATString&             rootDir)
: CATSwitch(element,  rootDir)
{
    fValue               = 0.0f;  
    fIconImage           = 0;
    fIconDisabled        = 0;
    fIconOff    		 = 0;
    fIconOffDisabled     = 0;    
}
//---------------------------------------------------------------------------
/// CATIconSwitch destructor
//---------------------------------------------------------------------------
CATIconSwitch::~CATIconSwitch()
{
    if (fIconImage)
        CATImage::ReleaseImage(fIconImage);
    if (fIconDisabled)
        CATImage::ReleaseImage(fIconDisabled);
    if (fIconOff)
        CATImage::ReleaseImage(fIconOff);
    if (fIconOffDisabled)
        CATImage::ReleaseImage(fIconOffDisabled);
}


/// ParseAttributes() parses the known attributes for an object.
CATResult CATIconSwitch::ParseAttributes()
{
    CATResult result = CATSwitch::ParseAttributes();
    CATString attrib;
    CATResult tmpResult;

    attrib = GetAttribute(L"IconImage");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib,fIconImage);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"IconDisabled");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fIconDisabled);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"IconOff");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fIconOff);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"IconOffDisabled");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fIconOffDisabled);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATIconSwitch::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }

    CATSwitch::Draw(image,dirtyRect);
    // sanity check parent image / dirty rectangle
    CATRect imgRect(0,0,image->Width(), image->Height());
    CATASSERT(imgRect.Inside(dirtyRect), "Update rect is outside of img rect!");

    // Find intersection between dirty rect and us
    CATRect drawRect;
    bool   drawn = false;

    CATImage* disabled = fIconDisabled;
    CATImage* normal   = fIconImage;

    if (fValue < 0.5)
    {
        if (fIconOffDisabled)
            disabled    = fIconOffDisabled;

        if (fIconOff)
            normal      = fIconOff;
    }

    CATRect innerRect;

    innerRect = fRect;
    if (fIconImage)
    {
        innerRect.left  = fRect.left + (fRect.Width() - fIconImage->Width())/2;
        innerRect.right = innerRect.left + fIconImage->Width();
        innerRect.top   = fRect.top  + (fRect.Height() - fIconImage->Height())/2;
        innerRect.bottom= innerRect.top  + fIconImage->Height();
    }

    if (this->IsPressed() || (this->fValue >= 0.5f))
    {
        CATPOINT iconOff;
        iconOff.x = this->fTextOffsetPressed.x - fTextOffset.x;
        iconOff.y = this->fTextOffsetPressed.y - fTextOffset.y;
        innerRect.Offset(iconOff);
    }

    if (innerRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;
        if ( (this->IsEnabled() == false) && (disabled))
        {
            if (drawRect.Intersect(CATRect(innerRect.left, 
                innerRect.top, 
                innerRect.left + disabled->Width(),
                innerRect.top  + disabled->Height()),
                &ourRect))
            {
                ourRect.Offset(-innerRect.left, -innerRect.top);

                image->Overlay(   disabled,
                    drawRect.left, 
                    drawRect.top, 
                    ourRect.left,
                    ourRect.top,
                    ourRect.Width(),
                    ourRect.Height());
                drawn = true;
            }        
        }

        if ((!drawn) && (normal != 0))
        {
            if (drawRect.Intersect(CATRect(innerRect.left, 
                innerRect.top, 
                innerRect.left + normal->Width(),
                innerRect.top  + normal->Height()),
                &ourRect))
            {
                ourRect.Offset(-innerRect.left, -innerRect.top);

                image->Overlay(   normal,
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

