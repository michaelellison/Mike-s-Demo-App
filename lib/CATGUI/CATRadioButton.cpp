//---------------------------------------------------------------------------
/// \file CATRadioButton.cpp
/// \brief Momentary push button for GUI
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
//---------------------------------------------------------------------------
#include "CATRadioButton.h"


//---------------------------------------------------------------------------
/// CATRadioButton constructor (inherited from CATControl->CATXMLObject)
/// \param element - Type name ("Button")
/// \param attribs - attribute information for the window
/// \param parent - parent XML object (should be a "Window" element)
/// \param rootDir - root directory of skin
//---------------------------------------------------------------------------
CATRadioButton::CATRadioButton(  const CATString&             element, 
                               const CATString&             rootDir)
                               : CATSwitch(element,  rootDir)
{
    fValue      = 0.0f;
    fRadioValue = 0.0f;
    fMinValue   = 0.0f;
    fMaxValue   = (CATFloat32)kGGMAXRADIOBUTTONS;
}


/// ParseAttributes() parses the known attributes for an object.
CATResult CATRadioButton::ParseAttributes()
{
    CATResult result = CATSwitch::ParseAttributes();
    CATString attrib;

    fRadioValue = GetAttribute(L"RadioValue",fRadioValue);

    return result;   
}

//---------------------------------------------------------------------------
/// CATRadioButton destructor
//---------------------------------------------------------------------------
CATRadioButton::~CATRadioButton()
{
}


CATResult CATRadioButton::Load( CATPROGRESSCB				progressCB,
                               void*							progressParam,
                               CATFloat32						progMin,
                               CATFloat32						progMax)
{
    CATResult result = CATSwitch::Load(progressCB, progressParam, progMin, progMax);  
    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATRadioButton::Draw(CATImage* image, const CATRect& dirtyRect)
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
    CATImage* active   = fImageActive;

    if ((int)fValue == (int)fRadioValue)
    {
        // All on...
        if (fImageOn)
            normal      = fImageOn;

        // Fallback to on-state with radio button.
        if (fImagePressedOn)
            pressed     = fImagePressedOn;
        else if (fImageOn)
            pressed     = fImageOn;

        if (fImageFocusOn)
            focus       = fImageFocusOn;
        else if (fImageOn)
            focus       = fImageOn;

        if (fImageActiveOn)
            active      = fImageActiveOn;
        else if (fImageOn)
            active      = fImageOn;

        if (fImageDisabledOn)
            disabled    = fImageDisabledOn;
        else if (fImageOn)
            disabled    = fImageOn;
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

            if ((!drawn) && (IsActive()) && (active))
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

void CATRadioButton::OnMouseClick()
{
    if (fValue != fRadioValue)
    {
        fValue = fRadioValue;
        CATControl::OnMouseClick();
    }
}
