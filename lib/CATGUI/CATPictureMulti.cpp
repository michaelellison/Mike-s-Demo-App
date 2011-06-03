//---------------------------------------------------------------------------
/// \file CATPictureMulti.cpp
/// \brief Multiple pictures (like switch multi, but no mouse interaction)
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
//---------------------------------------------------------------------------
#include "CATPictureMulti.h"
#include "CATApp.h"

CATPictureMulti::CATPictureMulti(const CATString&             element, 
                                 const CATString&             rootDir)
                                 : CATControl(element,  rootDir)
{
    fBackgroundColor = CATColor(0,0,255);
    fNumImages	= 0;
    fValue		= 0.0f;  
}

CATPictureMulti::~CATPictureMulti()
{
    // Let parent release fImage!
    CATImage *tmpImage = 0;
    while (CATSUCCEEDED(fMasterSet.Pop(tmpImage)))
    {
        CATImage::ReleaseImage(tmpImage);
    }
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATPictureMulti::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    fNumImages = GetAttribute(L"NumImage",fNumImages);
    CATUInt32 actualImages = 0;

    for (CATUInt32 i = 0; i < fNumImages; i++)
    {
        CATResult tmpResult;
        CATString attribName;
        CATImage* tmpImage = 0;

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
    }

    if (actualImages != fNumImages)
    {
        CATTRACE("Warning: Multi-Picture didn't have specified number of images.");
        fNumImages = actualImages;
    }
	 if (fNumImages)
		SetMinMax(0,(CATFloat32)(fNumImages-1));

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATPictureMulti::Draw(CATImage* image, const CATRect& dirtyRect)
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
		  static bool hasAsserted = false;
		  if (!hasAsserted)
		  {
			  hasAsserted = true;
			  CATASSERT(false,"Invalid value for PictureMulti.");
		  }
        index = 0;
		  return;
    }

    CATImage* normal = fImageList[index];

    CATResult result = CAT_SUCCESS;
    // By default, CATPictureMulti tiles similarly to the window background if the
    // rectangles don't fit.
    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        if (normal)
        {
            if ((normal->Width() == fRect.Width()) && 
                (normal->Height() == fRect.Height()))
            {
                CATRect ourRect = drawRect;
                ourRect.Offset(-fRect.left, -fRect.top);
                // Picture is the size of the rect, so just draw the
                // portion that's dirty.
                result = image->Overlay(normal,drawRect.left,drawRect.top,ourRect.left,ourRect.top,drawRect.Width(),drawRect.Height());
            }
            else
            {
                // OK... gotta do tiling of the picture, but only within the
                // rectangle.
                CATInt32 yPos = drawRect.top;
                CATInt32 totalHeight = drawRect.Height();
                CATInt32 offsetY = (yPos - fRect.top) % normal->Height();

                while (totalHeight > 0)
                {
                    CATInt32 xPos = drawRect.left;
                    CATInt32 copyHeight = CATMin(normal->Height() - offsetY, totalHeight);
                    CATInt32 totalWidth = drawRect.Width();
                    CATInt32 offsetX = (xPos - fRect.left) % normal->Width();

                    while (totalWidth > 0)
                    {
                        CATInt32 copyWidth = CATMin(normal->Width() - offsetX, totalWidth);               
                        result = image->Overlay(normal,xPos,yPos,offsetX,offsetY,copyWidth,copyHeight);
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

void CATPictureMulti::SetValue(CATFloat32 newValue, bool sendCommand)
{
	CATControl::SetValue(newValue,sendCommand);
}