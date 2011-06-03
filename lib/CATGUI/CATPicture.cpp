//---------------------------------------------------------------------------
/// \file CATPicture.cpp
/// \brief Static picture for GUI
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
#include "CATPicture.h"

//---------------------------------------------------------------------------
// CATPicture constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Picture")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png loads
//---------------------------------------------------------------------------
CATPicture::CATPicture(const CATString&             element, 
                       const CATString&             rootDir)
                       : CATControl(element,  rootDir)
{
    fBackgroundColor = CATColor(0,0,255);
    fValue = 0.0f;  
}

/// IsFocusable() returns true if the control can receive
/// focus, and false otherwise.
bool CATPicture::IsFocusable() const
{
    return false;
}
//---------------------------------------------------------------------------
/// CATPicture destructor
//---------------------------------------------------------------------------
CATPicture::~CATPicture()
{

}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATPicture::Draw(CATImage* image, const CATRect& dirtyRect)
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

    CATResult result = CAT_SUCCESS;
    // By default, CATPicture tiles similarly to the window background if the
    // rectangles don't fit.
    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        if (fImage)
        {
            if ((fImage->Width() == fRect.Width()) && 
                (fImage->Height() == fRect.Height()))
            {
                CATRect ourRect = drawRect;
                ourRect.Offset(-fRect.left, -fRect.top);
                // Picture is the size of the rect, so just draw the
                // portion that's dirty.
                result = image->Overlay(fImage,drawRect.left,drawRect.top,ourRect.left,ourRect.top,drawRect.Width(),drawRect.Height());
            }
            else
            {
                // OK... gotta do tiling of the picture, but only within the
                // rectangle.
                CATInt32 yPos = drawRect.top;
                CATInt32 totalHeight = drawRect.Height();
                CATInt32 offsetY = (yPos - fRect.top) % fImage->Height();

                while (totalHeight > 0)
                {
                    CATInt32 xPos = drawRect.left;
                    CATInt32 copyHeight = CATMin(fImage->Height() - offsetY, totalHeight);
                    CATInt32 totalWidth = drawRect.Width();
                    CATInt32 offsetX = (xPos - fRect.left) % fImage->Width();

                    while (totalWidth > 0)
                    {
                        CATInt32 copyWidth = CATMin(fImage->Width() - offsetX, totalWidth);               
                        result = image->Overlay(fImage,xPos,yPos,offsetX,offsetY,copyWidth,copyHeight);
                        xPos += copyWidth;
                        totalWidth -= copyWidth;
                        offsetX = 0;
                    }

                    offsetY = 0;
                    yPos += copyHeight;
                    totalHeight -= copyHeight;
                }
            }
        }
        else
        {
            // No image - 
            result = image->FillRect(drawRect,this->fBackgroundColor);
        }
    }
}

