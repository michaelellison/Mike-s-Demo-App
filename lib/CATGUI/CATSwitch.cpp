//---------------------------------------------------------------------------
/// \file CATSwitch.cpp
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
#include "CATSwitch.h"
#include "CATApp.h"
#include "CATWindow.h"
//---------------------------------------------------------------------------
// CATSwitch constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Switch")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png loads
//---------------------------------------------------------------------------
CATSwitch::CATSwitch(  const CATString&             element, 
                       const CATString&             rootDir)
: CATControl(element,  rootDir)
{
    fValue               = 0.0f;  
    fImageOn             = 0;
    fImageDisabledOn     = 0;
    fImagePressedOn		 = 0;
    fImageFocusOn	     = 0;
    fImageFocusActOn	 = 0;
    fImageActiveOn       = 0;
}
//---------------------------------------------------------------------------
/// CATSwitch destructor
//---------------------------------------------------------------------------
CATSwitch::~CATSwitch()
{
    if (fImageOn)
        CATImage::ReleaseImage(fImageOn);
    if (fImageDisabledOn)
        CATImage::ReleaseImage(fImageDisabledOn);
    if (fImagePressedOn)
        CATImage::ReleaseImage(fImagePressedOn);
    if (fImageFocusOn)
        CATImage::ReleaseImage(fImageFocusOn);
    if (fImageFocusActOn)
        CATImage::ReleaseImage(fImageFocusActOn);
    if (fImageActiveOn)
        CATImage::ReleaseImage(fImageActiveOn);
}

void CATSwitch::OnMouseClick()
{
    if (fValue < 0.5f)
    {
        fValue = 1.0f;
    }
    else
    {
        fValue = 0.0f;
    }

    CATControl::OnMouseClick();
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATSwitch::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;
    CATResult tmpResult;

    attrib = GetAttribute(L"ImageOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib,fImageOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageDisabled");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fImageDisabledOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"ImageFocusOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fImageFocusOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"ImageFocusActOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fImageFocusActOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"ImageActiveOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fImageActiveOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }   

    attrib = GetAttribute(L"ImagePressedOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib, fImagePressedOn);
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
void CATSwitch::Draw(CATImage* image, const CATRect& dirtyRect)
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

    CATImage* disabled = fImageDisabled;
    CATImage* normal   = fImage;
    CATImage* pressed  = fImagePressed;
    CATImage* focus    = fImageFocus;
    CATImage* focusAct = fImageFocusAct;
    CATImage* active   = fImageActive;

    if (fValue > 0.5)
    {
        // All on...
        if (fImageDisabledOn)
            disabled    = fImageDisabledOn;

        if (fImageOn)
            normal      = fImageOn;

        if (fImagePressedOn)
            pressed     = fImagePressedOn;

        if (fImageFocusOn)
            focus       = fImageFocusOn;

        if (fImageFocusActOn)
            focusAct    = fImageFocusActOn;

        if (fImageActiveOn)
            active      = fImageActiveOn;
    }


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

                    image->Overlay( pressed,
                                    drawRect.left,
                                    drawRect.top,
                                    ourRect.left,
                                    ourRect.top,
                                    ourRect.Width(),
                                    ourRect.Height());
                    drawn = true;
                }
            }

            if ((!drawn) && ( (IsFocused() && IsActive()) || IsPressed()) && (focusAct))
            {
                if (drawRect.Intersect(CATRect( fRect.left, 
                                                fRect.top, 
                                                fRect.left + focusAct->Width(),
                                                fRect.top  + focusAct->Height()),
                                       &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( focusAct,
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
                if (drawRect.Intersect(CATRect( fRect.left, 
                                                fRect.top, 
                                                fRect.left + focus->Width(),
                                                fRect.top  + focus->Height()),
                                       &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( focus,
                                    drawRect.left,
                                    drawRect.top,
                                    ourRect.left,
                                    ourRect.top,
                                    ourRect.Width(),
                                    ourRect.Height());
                    drawn = true;
                }
            }

            if ((!drawn) && (IsActive()) && (active))
            {
                if (drawRect.Intersect(CATRect( fRect.left, 
                                                fRect.top, 
                                                fRect.left + active->Width(),
                                                fRect.top  + active->Height()),
                                       &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( active,
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

CATResult CATSwitch::Load(CATPROGRESSCB				progressCB ,
                          void*				progressParam,
                          CATFloat32				progMin,
                          CATFloat32				progMax)
{
    CATResult result = CATControl::Load(progressCB, progressParam,progMin,progMax);

    if (CATFAILED(result))
    {
        return result;
    }

    // Sanity check images
    if (fImage != 0)
    {
        CATResult testResult = CAT_SUCCESS;

        if (fImageOn)
        {
            testResult = CheckImageSize(fImageOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageDisabledOn)
        {
            testResult = CheckImageSize(fImageDisabledOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImagePressedOn)
        {
            testResult = CheckImageSize(fImagePressedOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageFocusOn)
        {
            testResult = CheckImageSize(fImageFocusOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageFocusActOn)
        {
            testResult = CheckImageSize(fImageFocusActOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageActiveOn)
        {
            testResult = CheckImageSize(fImageActiveOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

    }

    return result;
}

CATString CATSwitch::GetHint() const
{
    CATString retString;
    retString = CATControl::GetHint();
    if (fShowHintValue)
    {
        if (this->fValue >= 0.5f)
        {
            retString << " ( On )";
        }
        else
        {
            retString << " ( Off )";
        }
    }
    return retString;
}

