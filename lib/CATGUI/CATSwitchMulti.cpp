//---------------------------------------------------------------------------
/// \file CATSwitchMulti.cpp
/// \brief On/Off switch for GUI
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
#include "CATSwitchMulti.h"
#include "CATApp.h"
#include "CATWindow.h"
//---------------------------------------------------------------------------
// CATSwitchMulti constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Switch")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png loads
//---------------------------------------------------------------------------
CATSwitchMulti::CATSwitchMulti(  const CATString&             element, 
                               const CATString&             rootDir)
                               : CATControl(element,  rootDir)
{
    fValue               = 0.0f;  
    fNumImages				= 0;
    fAutoIncrement			= true;
}
//---------------------------------------------------------------------------
/// CATSwitchMulti destructor
//---------------------------------------------------------------------------
CATSwitchMulti::~CATSwitchMulti()
{
    CATImage* tmpImage = 0;

    while (CATSUCCEEDED(fMasterSet.Pop(tmpImage)))
    {
        CATImage::ReleaseImage(tmpImage);
    }
}

void CATSwitchMulti::OnMouseClick()
{
    if (fAutoIncrement)
    {
        fValue = fValue + 1;

        if (fValue >= fNumImages)
        {
            fValue = 0.0f;
        }
    }

    CATControl::OnMouseClick();
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATSwitchMulti::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    CATImage* tmpImage = 0;

    fNumImages = GetAttribute(L"NumImage", fNumImages);

    CATUInt32 actualImages = 0;
    for (CATUInt32 i = 0; i < fNumImages; i++)
    {
        CATResult tmpResult;
        CATString attribName;

        attribName.Format(L"Image_%d",i+1);
        attrib = GetAttribute(attribName);
        if (!attrib.IsEmpty())
        {
            tmpResult = LoadSkinImage(attrib,tmpImage);

            if (CATFAILED(tmpResult))
            {
                result = tmpResult;
            }
            else
            {
                actualImages++;

                fImageList.push_back(tmpImage);				

                if (fImage == 0)
                    fImage = tmpImage;
                else
                    fMasterSet.Push(tmpImage);
            }
        }

        attribName.Format(L"ImageDisabled_%d",i+1);
        attrib = GetAttribute(attribName);
        if (!attrib.IsEmpty())
        {
            tmpResult = LoadSkinImage(attrib,tmpImage);

            if (CATFAILED(tmpResult))
            {
                result = tmpResult;
            }
            else 
            {
                fImageDisabledList.push_back(tmpImage);
                fMasterSet.Push(tmpImage);
            }
        }

        attribName.Format(L"ImagePressed_%d",i+1);
        attrib = GetAttribute(attribName);
        if (!attrib.IsEmpty())
        {
            tmpResult = LoadSkinImage(attrib,tmpImage);

            if (CATFAILED(tmpResult))
            {
                result = tmpResult;
            }
            else 
            {
                fImagePressedList.push_back(tmpImage);
                fMasterSet.Push(tmpImage);
            }
        }

        attribName.Format(L"ImageFocus_%d",i+1);
        attrib = GetAttribute(attribName);
        if (!attrib.IsEmpty())
        {
            tmpResult = LoadSkinImage(attrib,tmpImage);

            if (CATFAILED(tmpResult))
            {
                result = tmpResult;
            }
            else 
            {
                fImageFocusList.push_back(tmpImage);
                fMasterSet.Push(tmpImage);
            }
        }

        attribName.Format(L"ImageActive_%d",i+1);
        attrib = GetAttribute(attribName);
        if (!attrib.IsEmpty())
        {
            tmpResult = LoadSkinImage(attrib,tmpImage);

            if (CATFAILED(tmpResult))
            {
                result = tmpResult;
            }
            else 
            {
                fImageActiveList.push_back(tmpImage);
                fMasterSet.Push(tmpImage);
            }
        }
    }

    if (actualImages != fNumImages)
    {
        CATTRACE("Warning: actual images != NumImages");
        fNumImages = actualImages;
    }

    fAutoIncrement = GetAttribute(L"AutoIncrement", fAutoIncrement);

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATSwitchMulti::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }

    // sanity check parent image / dirty rectangle
    CATRect imgRect(0,0,image->Width(), image->Height());
    CATASSERT(imgRect.Inside(dirtyRect), "Update rect is outside of img rect!");

    // Find intersection between dirty rect and us
    CATRect drawRect;
    bool   drawn = false;

    CATUInt32 index = (CATUInt32)fValue;

    if (index >= fNumImages)
    {
        CATTRACE("Invalid value for multiswitch");
        index = 0;
    }


    CATImage* disabled = 0;
    CATImage* normal   = 0;
    CATImage* pressed  = 0;
    CATImage* focus    = 0;
    CATImage* active   = 0;


    if (index < fImageDisabledList.size())
        disabled = fImageDisabledList[index];
    if (index < fImageList.size())
        normal = fImageList[index];
    if (index < fImagePressedList.size())
        pressed = fImagePressedList[index];
    if (index < fImageFocusList.size())
        focus = fImageFocusList[index];
    if (index < fImageActiveList.size())
        active = fImageFocusList[index];


    // Gracefully degrade depending on flags and whether the images are
    // available.
    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;
        if ( (this->IsEnabled() == false) && (disabled))
        {
            if (drawRect.Intersect(CATRect(fRect.left, 
                fRect.top, 
                fRect.left + disabled->Width(),
                fRect.top  + disabled->Height()),
                &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

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
        else 
        {
            if (this->IsPressed() && (pressed))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                    fRect.top, 
                    fRect.left + pressed->Width(),
                    fRect.top  + pressed->Height()),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   pressed,
                        drawRect.left,
                        drawRect.top,
                        ourRect.left,
                        ourRect.top,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }
            }

            if ((!drawn) && (IsFocused() || IsPressed()) && (focus))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                    fRect.top, 
                    fRect.left + focus->Width(),
                    fRect.top  + focus->Height()),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   focus,
                        drawRect.left,
                        drawRect.top,
                        ourRect.left,
                        ourRect.top,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }
            }
            if ((!drawn) && ( IsActive() ) && (active))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                    fRect.top, 
                    fRect.left + active->Width(),
                    fRect.top  + active->Height()),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   active,
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

        if ((!drawn) && (normal != 0))
        {
            if (drawRect.Intersect(CATRect(fRect.left, 
                fRect.top, 
                fRect.left + normal->Width(),
                fRect.top  + normal->Height()),
                &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

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

        if (!drawn)
        {

            // Right now, just make a big red rectangle where we should go.
            image->FillRect(drawRect, fBackgroundColor);
        }
    }
}

CATResult CATSwitchMulti::Load(CATPROGRESSCB				progressCB,
                               void*					progressParam,
                               CATFloat32					progMin,
                               CATFloat32					progMax)
{
    CATResult result = CATControl::Load(progressCB, progressParam, progMin, progMax);
    return result;
}

CATString CATSwitchMulti::GetHint() const
{
    CATString retString;
    retString = CATControl::GetHint();
    if (fShowHintValue)
    {
        retString << "( " << (CATUInt32)fValue << " )";
    }
    return retString;
}

