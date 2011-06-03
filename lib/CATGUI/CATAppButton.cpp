//---------------------------------------------------------------------------
/// \file CATAppButton.cpp
/// \brief Specialized pushbutton for launching apps
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
#include "CATAppButton.h"
#include "CATWindow.h"
#include "CATOSFuncs.h"
//---------------------------------------------------------------------------
/// CATAppButton constructor (inherited from CATControl->CATXMLObject)
/// \param element - Type name ("Button")
/// \param rootDir - root directory of skin
//---------------------------------------------------------------------------
CATAppButton::CATAppButton(  const CATString&             element, 
                             const CATString&             rootDir)
: CATButton(element, rootDir)
{
    fIcon           = 0;    
    fBorder         = 0;
    fIconImage      = 0;
    fIconDisabled   = 0;
}



//---------------------------------------------------------------------------
/// CATAppButton destructor
//---------------------------------------------------------------------------
CATAppButton::~CATAppButton()
{
    if (fIcon)
    {
#ifdef _WIN32
        DestroyIcon(fIcon);  
#endif
        fIcon = 0;
    }

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
CATResult CATAppButton::ParseAttributes()
{
    CATResult result = CATButton::ParseAttributes();
    fAppPath = GetAttribute(L"AppPath");
    
    bool found = false;

    if (!fAppPath.IsEmpty())
    {
        CATResult tmpResult = OSLoadIconImage(fAppPath,&fIcon,fAppName);
        
        this->fHintText = GetAttribute(L"HintText");

        if (CATSUCCEEDED(tmpResult))
        {
            fHintText << fAppName;
            found = true;
        }
    }

    fBorder = GetAttribute(L"Border",fBorder);

    CATString uninstId = GetAttribute(L"UninstallId");
    CATString appFile  = GetAttribute(L"AppFile");

    if ( (!appFile.IsEmpty()) && (!uninstId.IsEmpty()))
    {
        fAppPath = GetInstallLoc(uninstId);        
        if (fAppPath.GetWChar(fAppPath.Length()-1) != '\\')
            fAppPath << L"\\";

        fAppPath << appFile;
        
        CATResult tmpResult = OSLoadIconImage(fAppPath,&fIcon,fAppName);
        
        this->fHintText = GetAttribute(L"HintText");

        if (CATSUCCEEDED(tmpResult))
        {
            this->AddAttribute(L"AppPath",fAppPath);
            fHintText << fAppName;
            found = true;
        }
    }

    if (!found)
    {
        CATString regPath = GetAttribute(L"RegPath");

        if (!appFile.IsEmpty() && (!regPath.IsEmpty()))
        {
            fAppPath = GetSoftwareReg(regPath);
            if (fAppPath.GetWChar(fAppPath.Length()-1) != '\\')
                fAppPath << L"\\";

            fAppPath << appFile;
            
            CATResult tmpResult = OSLoadIconImage(fAppPath,&fIcon,fAppName);
                
            this->fHintText = GetAttribute(L"HintText");

            if (CATSUCCEEDED(tmpResult))
            {
                this->AddAttribute(L"AppPath",fAppPath);
                fHintText << fAppName;
                found = true;
            }
        }
    }


    if (!found)
        this->SetEnabled(false);

    return CAT_SUCCESS;
}


/// Draw() draws the control into the parent's image
/// \param image - parent image to draw into
/// \param dirtyRect - portion of control (in window coordinates)
///        that requires redrawing.
void CATAppButton::Draw(CATImage* image, const CATRect& dirtyRect)
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

/*
void CATAppButton::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{
    if ((this->IsVisible() == false) || (!fIconImage))
    {
        return;
    }

    CATRect drawRect = fRect;

    this->GetWindow()->WidgetToWindow(this,drawRect);    

    drawRect.left   += fBorder;
    drawRect.right  -= fBorder;
    drawRect.bottom -= fBorder;
    drawRect.top    += fBorder;

    if (fPressed)
    {
        drawRect.left += fTextOffsetPressed.x;
        drawRect.top  += fTextOffsetPressed.y;
    }
    else
    {
        drawRect.right  -= fTextOffsetPressed.x;
        drawRect.bottom -= fTextOffsetPressed.y;
    }

    ::DrawIconEx(drawContext,
                 drawRect.left,
                 drawRect.top,
                 fIconImage,
                 drawRect.Width(), 
                 drawRect.Height(),
                 0,
                 NULL,
                 DI_NORMAL);

    CATButton::PostDraw(drawContext,dirtyRect);
}
*/

CATResult CATAppButton::Load(CATPROGRESSCB progressCB, 
                             void*         progressParam, 
                             CATFloat32    progMin, 
                             CATFloat32    progMax)
{
    this->Reset();

    CATResult result = CATButton::Load(progressCB,progressParam,progMin,progMax);
    if (CATFAILED(result))
        return result;

    // Convert icon to our internal image class at the right size.
    if (fIcon)
    {
        CATImage::CreateImageFromIcon(  fIconImage,   
                                        fRect.Width() - fBorder*2, 
                                        fRect.Height() - fBorder*2,
                                        fBackgroundColor,
                                        fIcon);
        if (fIconImage)
        {
            CATImage::CopyImage(fIconImage,fIconDisabled);
            fIconDisabled->MakeDisabled();
        }
    }

    return result;
}

void CATAppButton::Reset()
{
    if (fIcon)
    {
        ::DestroyIcon(fIcon);
        fIcon = 0;
    }
    if (fIconDisabled != 0)
    {
        CATImage::ReleaseImage(fIconDisabled);
    }
    if (fIconImage != 0)
    {
        CATImage::ReleaseImage(fIconImage);
    }
    this->MarkDirty();
}
