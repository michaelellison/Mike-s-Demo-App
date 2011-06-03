//---------------------------------------------------------------------------
/// \file CATSlider.cpp
/// \brief Slider control for GUI
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
#include <math.h>

#include "CATSlider.h"
#include "CATWindow.h"
#include "CATFilterEWMA.h"
#include "CATCursor.h"
#include "CATApp.h"
#include "CATUtil.h"

// base resolution of Slider
const CATFloat32 kSLIDERSTEP = 0.001f;

//---------------------------------------------------------------------------
// CATSlider constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Button")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin
//---------------------------------------------------------------------------
CATSlider::CATSlider(      const CATString&             element, 
                     const CATString&             rootDir)
                     : CATControl(element,  rootDir)
{   
    fSliderStyle        = CATSLIDER_VERTICAL;
    fValueType          = CATVALUE_LINEAR;
    // Defaults
    fLastTarget				= fValue;
    fImageSlide          = 0;
    fImageSlideDisabled  = 0;
    fImageSlideFocus     = 0;
    fImageOn             = 0;
    fSlideOffset         = 0;
    fCommandTrack        = false;
    fCursor.SetType(CATCURSOR_TOPBOTTOM);
}

//---------------------------------------------------------------------------
/// CATSlider destructor
//---------------------------------------------------------------------------
CATSlider::~CATSlider()
{
    if (fImageSlide)
    {
        CATImage::ReleaseImage(fImageSlide);
    }
    if (fImageSlideDisabled)
    {
        CATImage::ReleaseImage(fImageSlideDisabled);
    }
    if (fImageSlideFocus)
    {
        CATImage::ReleaseImage(fImageSlideFocus);
    }
}

void CATSlider::TrackMouseDown(const CATPOINT& point, CATMODKEY modKey)
{   
    CATControl::TrackMouseDown(point,modKey);      
    TrackMouseMove(point,true, modKey);
}

void CATSlider::TrackMouseRelease(const CATPOINT& point, CATMODKEY modKey)
{   
    this->fPressed = false;
    this->fActive = true;
    fLastTarget = fValue;
    this->MarkDirty();
    this->OnMouseClick();
}

void CATSlider::TrackMouseMove(const CATPOINT& point, bool left, CATMODKEY modKey)
{   
    CATFloat32 newVal = fValue;
    if (left)
    {      
        switch (fSliderStyle)
        {
        case CATSLIDER_HORIZONTAL:
            {
                newVal = ((CATFloat32)(point.x - fRect.left))/(CATFloat32)(fRect.Width() - fImageSlide->Width()) * GetValRange() + fMinValue;
            }
            break;

        case CATSLIDER_VERTICAL:
        default:
            {
                newVal = ((CATFloat32)(fRect.Height() - fImageSlide->Height()) - 
                    (point.y - fRect.top)) / 
                    (CATFloat32)(fRect.Height() - fImageSlide->Height()) * GetValRange() + fMinValue;
            }
            break;
        }

        if (modKey & CATMODKEY_SHIFT)
        {
            fLastTarget = newVal;
            if (newVal < fValue)
            {
                fValue -= GetValRange() / 1000.0f;
            }
            else
            {
                fValue += GetValRange() / 1000.0f;				
            }
        }
        else
        {
            fValue = newVal;
            fLastTarget = fValue;
        }

        this->BoundsCheckValue();      

        // fLastPoint = point;

        this->MarkDirty();
        if (this->fCommandTrack)
        {
            this->OnMouseClick();
            // Maintain pressed state...
            fPressed = true;
        }
    }
}

void CATSlider::TrackMouseTimer(CATMODKEY modKey)
{
    if ((modKey & CATMODKEY_SHIFT) && (fLastTarget != fValue))
    {
        if (fLastTarget < fValue)
        {
            fValue -= GetValRange() / 1000.0f;
        }
        else
        {
            fValue += GetValRange() / 1000.0f;
        }
        this->BoundsCheckValue();

        this->MarkDirty();

        if (this->fCommandTrack)
        {
            this->OnMouseClick();			
            // Maintain pressed state...
            fPressed = true;
        }
    }
}



void CATSlider::OnMouseWheel(    const CATPOINT& point,
                             CATFloat32        wheelMove,
                             CATMODKEY			modKey)
{

    CATFloat32 keyStep = (modKey & CATMODKEY_SHIFT)?10.0f:100.0f;
    CATFloat32 delta = (wheelMove * kSLIDERSTEP * GetValRange()) * keyStep;
    this->fValue += delta;
    fLastTarget = fValue;

    this->BoundsCheckValue();

    this->MarkDirty();      
    this->OnMouseClick();
}
//---------------------------------------------------------------------------
void CATSlider::OnKeyPress(const CATKeystroke& keystroke)
{
    CATControl::OnKeyPress(keystroke);
}

void CATSlider::OnKeyDown(const CATKeystroke& keystroke)
{   
    if (keystroke.IsSpecial())
    {
        CATFloat32 delta = 0;

        switch (keystroke.GetSpecialKey())
        {
        case CATKEY_LEFT:
            delta = -kSLIDERSTEP * GetValRange();
            break;      
        case CATKEY_RIGHT:
            delta = kSLIDERSTEP * GetValRange();
            break;
        case CATKEY_UP:
            delta = kSLIDERSTEP * 10 * GetValRange();
            break;
        case CATKEY_DOWN:
            delta = -kSLIDERSTEP * 10 * GetValRange();
            break;
        case CATKEY_NEXT:
            delta = -kSLIDERSTEP * 100 * GetValRange();
            break;
        case CATKEY_PRIOR:
            delta = kSLIDERSTEP * 100 * GetValRange();
            break;

        case CATKEY_HOME:
            fValue = fMinValue;
            fLastTarget = fValue;
            this->MarkDirty();
            this->OnMouseClick();
            return;
            break;

        case CATKEY_END:
            fValue = fMaxValue;
            fLastTarget = fValue;
            this->MarkDirty();
            this->OnMouseClick();
            return;
            break;
        }      

        if (delta != 0)
        {
            fValue += delta;
            fLastTarget = fValue;

            this->BoundsCheckValue();

            this->MarkDirty();      
            this->OnMouseClick();
        }
    }

    // Allow default handling on stuff we don't do.
    CATControl::OnKeyDown(keystroke);
}


//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATSlider::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }

    // Draw background for slider
    CATControl::Draw(image,dirtyRect);


    // Draw "On" image if applicable
    if (fImageOn && this->IsEnabled())
    {
        CATRect onRect = fRect;

        if (this->fSliderStyle == CATSLIDER_HORIZONTAL)
        {
            CATInt32 width = CATMin(onRect.Width(), fImageOn->Width());
            if (width > fSlideOffset)
            {
                width -= fSlideOffset;
            }

            onRect.right = (CATInt32)(onRect.left + width*this->GetValPercent());                                

        }
        else
        {
            CATInt32 height = fImageOn->Height();
            CATInt32 onHeight = height;

            if (height > fSlideOffset)
            {
                onHeight -= fSlideOffset;
            }

            onRect.top += (CATInt32)(height - (onHeight * this->GetValPercent()));
        }  

        CATRect layerRect;
        if (onRect.Intersect(dirtyRect, &layerRect))
        {  
            image->Overlay(   this->fImageOn,
                layerRect.left,
                layerRect.top,
                layerRect.left - fRect.left,
                layerRect.top - fRect.top,
                layerRect.Width(),
                layerRect.Height());
        }
    }
    // Now draw slider knob
    CATInt32 xPos, yPos;
    xPos = 0;
    yPos = 0;

    switch (fSliderStyle)
    {
    case CATSLIDER_HORIZONTAL:
        {
            xPos = (CATInt32)(GetValPercent() * (fRect.Width() - this->fImageSlide->Width())) + fRect.left;
            yPos = fRect.top;
        }
        break;

    case CATSLIDER_VERTICAL:
    default:
        {
            xPos = fRect.left;
            yPos = (fRect.bottom - fImageSlide->Height()) - (CATInt32)(GetValPercent() * (fRect.Height() - this->fImageSlide->Height()));
        }
        break;
    }

    CATRect slideRect(xPos,yPos, this->fImageSlide->Width() + xPos, this->fImageSlide->Height() + yPos);
    CATRect drawRect;
    bool drawn = false;
    if (slideRect.Intersect(dirtyRect,&drawRect))
    {
        if ( (this->IsEnabled() == false) && (this->fImageSlideDisabled))
        {
            image->Overlay(   this->fImageSlideDisabled,
                drawRect.left, 
                drawRect.top, 
                drawRect.left - xPos,
                drawRect.top - yPos,
                drawRect.Width(),
                drawRect.Height());
            drawn = true;
        }
        else 
        {         
            if ((!drawn) && (IsFocused() || IsActive() || IsPressed()) && (this->fImageSlideFocus))
            {
                image->Overlay(   this->fImageSlideFocus,
                    drawRect.left, 
                    drawRect.top, 
                    drawRect.left - xPos,
                    drawRect.top - yPos,
                    drawRect.Width(),
                    drawRect.Height());
                drawn = true;
            }
        }

        if ((!drawn) && (this->fImageSlide != 0))
        {
            image->Overlay(   this->fImageSlide,
                drawRect.left, 
                drawRect.top, 
                drawRect.left - xPos,
                drawRect.top - yPos,
                drawRect.Width(),
                drawRect.Height());
            drawn = true;
        }

        if (!drawn)
        {
            CATTRACE("4");

            // Right now, just make a big red rectangle where we should go.
            image->FillRect(drawRect, fBackgroundColor);
        }      
    }
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATSlider::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    fCommandTrack = GetAttribute(L"CommandTrack",fCommandTrack);
    fSlideOffset  = GetAttribute(L"SlideOffset",fSlideOffset);
    attrib = GetAttribute(L"SliderStyle");
    if (!attrib.IsEmpty())
    {
        if ((attrib.GetWChar(0) | (char)0x20) == 'v')
        {
            fSliderStyle = CATSLIDER_VERTICAL;
        }
        else
        {
            fSliderStyle = CATSLIDER_HORIZONTAL;
        }
    }

    CATResult tmpResult;
    attrib = GetAttribute(L"ImageSlide");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib,fImageSlide);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib,fImageOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageSlideDisabled");
    if (!attrib.IsEmpty())	
    {
        tmpResult = LoadSkinImage(attrib,fImageSlideDisabled);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageSlideFocus");
    if (!attrib.IsEmpty())	
    {
        tmpResult = LoadSkinImage(attrib,fImageSlideFocus);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ValueType");
    if (!attrib.IsEmpty())
    {
        if (attrib.Compare("dB") == 0)
        {
            fValueType = CATVALUE_DB;
        }
        else
        {
            fValueType = CATVALUE_LINEAR;
        }
    }

    return result;

}

CATSLIDER_STYLE CATSlider::GetSliderStyle()
{
    return fSliderStyle;
}

void CATSlider::SetSliderStyle(CATSLIDER_STYLE style)
{
    fSliderStyle = style;
    // IF manually set, override cursor 
    switch (fSliderStyle)
    {
    case CATSLIDER_HORIZONTAL:
        fCursor.SetType(CATCURSOR_LEFTRIGHT);
        break;
    case CATSLIDER_VERTICAL:         
    default:
        fCursor.SetType(CATCURSOR_TOPBOTTOM);
        break;
    }
}

CATResult CATSlider::Load(CATPROGRESSCB				progressCB ,
                          void*							progressParam,
                          CATFloat32						progMin,
                          CATFloat32						progMax)
{
    CATResult result = CATControl::Load(progressCB, progressParam, progMin, progMax);

    if (CATFAILED(result))
    {
        return result;
    }

    // Sanity check images
    if (fImageSlide != 0)
    {
        CATResult testResult = CAT_SUCCESS;

        if (fImageSlideDisabled)
        {
            testResult = CheckImageSize(fImageSlideDisabled, fImageSlide);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageSlideFocus)
        {
            testResult = CheckImageSize(fImageSlideFocus, fImageSlide);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }
    }

    return result;
}

CATString CATSlider::GetHint() const
{
    CATString retString = CATControl::GetHint();

    if (fShowHintValue)
    {
        CATString hintVal;
        if (fValueType == CATVALUE_DB)
        {
            if (fValue == 0.0f)
            {
                hintVal.Format(L" ( Off )");
            }
            else
            {
                CATFloat32 dbValue = CATLinearToDBValue(fValue);
                hintVal.Format(L" ( %c%.2fdB - %.2f)",(dbValue > 0)?'+':'-', fabs(dbValue), CATLinearToDBGain(fValue) );
            }
        }
        else
        {
            hintVal.Format(L" ( %.2f )",this->GetValue());
        }
        retString << hintVal;
    }

    return retString;
}


// OnMouseDoubleClick() is called the mouse is double clicked.
//
// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
void CATSlider::OnMouseDoubleClick(CATMODKEY modKey)
{
    this->SetValue(fDefValue);
    fLastTarget = fValue;
    this->MarkDirty();
}
