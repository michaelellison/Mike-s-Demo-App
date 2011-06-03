//---------------------------------------------------------------------------
/// \file CATProgress.cpp
/// \brief Progress bar for GUI
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
#include "CATProgress.h"
#include "CATApp.h"
#include "CATWindow.h"

//---------------------------------------------------------------------------
// CATProgress constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Progress")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png loads
//---------------------------------------------------------------------------
CATProgress::CATProgress(  const CATString&             element, 
                         const CATString&             rootDir)
                         : CATControl(element,  rootDir)
{
    fImageOn = 0;
    fProgressStyle = CATPROGRESS_HORIZONTAL;
}

/// IsFocusable() returns true if the control can receive
/// focus, and false otherwise.
bool CATProgress::IsFocusable() const
{
    return false;
}
//---------------------------------------------------------------------------
/// CATProgress destructor
//---------------------------------------------------------------------------
CATProgress::~CATProgress()
{
    if (fImageOn)
    {
        CATImage::ReleaseImage(fImageOn);
    }
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATProgress::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATResult tmpResult;
    CATString attrib;

    attrib = GetAttribute(L"ImageOn");
    if (!attrib.IsEmpty())
    {
        tmpResult = LoadSkinImage(attrib,fImageOn);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ProgressStyle");
    if (!attrib.IsEmpty())
    {
        if ( (attrib.GetWChar(0) | (char)0x20) == 'v')
        {
            fProgressStyle = CATPROGRESS_VERTICAL;
        }
        else
        {
            fProgressStyle = CATPROGRESS_HORIZONTAL;
        }
    }

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATProgress::Draw(CATImage* image, const CATRect& dirtyRect)
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

    // Gracefully degrade depending on flags and whether the images are
    // available.
    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;
        if ( (this->IsEnabled() == false) && (this->fImageDisabled))
        {
            if (drawRect.Intersect(CATRect(fRect.left, 
                fRect.top, 
                fRect.left + this->fImageDisabled->Width(),
                fRect.top  + this->fImageDisabled->Height()),
                &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

                image->Overlay(   this->fImageDisabled,
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
            CATRect normalRect = fRect;
            CATRect onRect = fRect;

            if (this->fImage != 0)
            {
                if (drawRect.Intersect(normalRect,&ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   this->fImage,
                        drawRect.left,
                        drawRect.top,
                        ourRect.left,
                        ourRect.top,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }
            }


            if (this->fImageOn != 0)
            {
                CATRect tmpRect = drawRect;
                CATFloat32 drawPercent = this->GetValPercent();


                if (this->fProgressStyle == CATPROGRESS_HORIZONTAL)
                {
                    onRect.right = (CATInt32)(onRect.left + 
                        ( CATMin(onRect.Width(), fImageOn->Width()) * drawPercent));
                }
                else
                {
                    CATInt32 offset = (CATInt32)(fImageOn->Height() - (fImageOn->Height() * drawPercent));
                    onRect.top += offset;          
                    tmpRect.top += offset;
                    if (tmpRect.top > tmpRect.bottom)
                        tmpRect.top = tmpRect.bottom;
                }

                if (tmpRect.Intersect(onRect,&ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   this->fImageOn,
                        tmpRect.left,
                        tmpRect.top,
                        ourRect.left,
                        ourRect.top,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }
            }
        }

        if (!drawn)
        {

            // Right now, just make a big red rectangle where we should go.
            image->FillRect(drawRect, fBackgroundColor);
        }
    }
}
//---------------------------------------------------------------------------
CATResult CATProgress::Load(	CATPROGRESSCB				progressCB,
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
    if (fImage != 0)
    {
        CATResult testResult = CAT_SUCCESS;

        if (fImageOn)
        {
            testResult = CheckImageSize(fImageOn);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }
    }

    return result;
}

/// SetImages() sets the images and resets the control
void CATProgress::SetImages( CATImage* imageOn, CATImage* imageOff, CATImage* imageDisabled)
{
    if (fImage)
    {
        CATImage::ReleaseImage(fImage);		
    }

    if (fImageOn)
    {
        CATImage::ReleaseImage(fImageOn);
    }

    if (fImageDisabled)
    {
        CATImage::ReleaseImage(fImageDisabled);
    }

    fImage = imageOff;
    fImageOn = imageOn;
    fImageDisabled = imageDisabled;

    this->RectFromAttribs();
    this->MarkDirty();
}

