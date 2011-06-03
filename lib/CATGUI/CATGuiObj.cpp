//---------------------------------------------------------------------------
/// \file CATGuiObj.cpp
/// \brief Base GUI control class
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#include "CATGuiObj.h"
#include "CATApp.h"
#include "CATFileSystem.h"
#include "CATStream.h"
#include "CATEvent.h"
#include "CATWindow.h"

//---------------------------------------------------------------------------
// CATGuiObj constructor (inherited from CATXMLObject)
// \param element  Type name (e.g. "Button", "Label", etc.)
// \param parent   parent XML object
//---------------------------------------------------------------------------
CATGuiObj::CATGuiObj(  const CATString&                element, 
                       const CATString&                rootDir)
: CATXMLObject(element)
{
    // Init vars
    fImage               = 0;
    fEnabled             = true;
    fVisible             = true;
    fShowHintValue       = true;
    fRootDir             = rootDir;
}

//---------------------------------------------------------------------------
CATResult CATGuiObj::Load(CATPROGRESSCB				progressCB,
                          void*						progressParam,
                          CATFloat32				progMin,
                          CATFloat32				progMax)
{
    CATResult result = CAT_SUCCESS;
    CATResult testResult = CAT_SUCCESS;

    ParseAttributes();
    if (CATFAILED(result = this->RectFromAttribs()))
    {
        return result;
    }


    CATXMLObject* curChild = 0;	

    CATUInt32 i;
    CATUInt32 numChildren = GetNumChildren();

    if (progressCB)
    {
        progressCB( progMin, this->GetName(), progressParam);
    }

    if (numChildren)
    {
        CATFloat32 progStep = (progMax - progMin) / (CATFloat32)numChildren;

        for (i = 0; i < numChildren; i++)
        {

            if (0 != (curChild = GetChild(i)))
            {
                CATGuiObj* curObj = (CATGuiObj*)curChild;
                if (CAT_SUCCESS != (testResult = curObj->Load(	progressCB, 
                    progressParam, 
                    progMin + progStep*i, 
                    progMin + (progStep*(i+1)) )))
                {
                    result = testResult;
                }
            }
        }
    }

    return result;
}
//---------------------------------------------------------------------------
// CATGuiObj destructor
//---------------------------------------------------------------------------
CATGuiObj::~CATGuiObj()
{
    if (fImage)
    {
        CATImage::ReleaseImage(fImage);      
    }
}

//---------------------------------------------------------------------------
// ParseAttributes() parses known attributes for the object.
//---------------------------------------------------------------------------
CATResult CATGuiObj::ParseAttributes()
{
    CATResult result    = CAT_SUCCESS;
    CATResult tmpResult = CAT_SUCCESS;
    CATString attrib;

    fName          = GetAttribute(L"Name");
    fHintText      = GetAttribute(L"HintText");
    fShowHintValue = GetAttribute(L"ShowHintValue",fShowHintValue);

    attrib         = GetAttribute(L"Image");
    if (attrib.IsEmpty() == false)
    {
        tmpResult = LoadSkinImage(attrib,fImage);		
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib         = GetAttribute(L"Enabled");
    if (attrib.IsEmpty() == false)
        SetEnabled((bool)attrib);

    attrib         = GetAttribute(L"ColorFore");    
    if (attrib.IsEmpty() == false)
    {
        CATUInt32 rawColor = attrib.FromHex();
        // Values are RGB, not RGBA      
        this->fForegroundColor.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fForegroundColor.g = (CATUInt8)((rawColor & 0xff00) >> 8);
        this->fForegroundColor.b = (CATUInt8)(rawColor & 0xff);
        this->fForegroundColor.a = 255;
    }

    attrib         = GetAttribute(L"ColorBack");    
    if (attrib.IsEmpty() == false)
    {
        CATUInt32 rawColor = attrib.FromHex();
        // Values are RGB, not RGBA      
        this->fBackgroundColor.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fBackgroundColor.g = (CATUInt8)((rawColor & 0xff00) >>8);
        this->fBackgroundColor.b = (CATUInt8)((rawColor & 0xff));
        this->fBackgroundColor.a = 255;
    }

    return result;
}

//---------------------------------------------------------------------------
// LoadSkinImage()
//---------------------------------------------------------------------------
CATResult CATGuiObj::LoadSkinImage(const CATString& filename, CATImage*& imagePtr)
{
    // This retrieves a member of app - no need to release
    CATFileSystem* fs = gApp->GetGlobalFileSystem();

    imagePtr = 0;

    // rootDir contains our base directory for skin.
    // Use that to find images...
    CATString imageFile   = fs->BuildPath(fRootDir,filename);
    CATStream* stream     = 0;
    CATResult result      = CAT_SUCCESS;

    // If we already have the image cached, just return it.
    // GetResourceImage() will increment the reference count for us.
    if (CATSUCCEEDED(result = gApp->GetResourceImage(imageFile, imagePtr)))
    {
        return result;
    }

    // Don't have it in our resource map - load it directly.
    if (CATSUCCEEDED(result = fs->OpenFile(imageFile,CATStream::READ_ONLY,stream)))
    {
        result = CATImage::Load(stream,imagePtr);
        fs->ReleaseFile(stream);

        if (CATSUCCEEDED(result))
        {
            // Got it loaded. Add to our resource map.
            gApp->AddResourceImage(imageFile,imagePtr);
        }
    }

    if (CATFAILED(result))
    {
        result = CATRESULTFILE(result,imageFile);
        CATASSERT(false,(CATString)"Unable to load skin image!" << imageFile);
    }

    return result;
}



//---------------------------------------------------------------------------
// GetName() retrieves the name of the control.
//---------------------------------------------------------------------------
CATString CATGuiObj::GetName() const
{
    return this->fName;
}

//---------------------------------------------------------------------------
// GetHint() retrieves the hint text for the control
//---------------------------------------------------------------------------
CATString CATGuiObj::GetHint() const
{
    return this->fHintText;
}

//---------------------------------------------------------------------------
// RectFromAttribs() recalculates the control's rectangle from
// the attributes.  This can only be called after ParseAttributes() has
// loaded the images.
//---------------------------------------------------------------------------
CATResult CATGuiObj::RectFromAttribs()
{
    CATResult result = CAT_SUCCESS;
    CATRect parentRect;
    if (this->fParent != 0)
    {
        // Right now, the order is:
        // Skin->Window->Control.
        // Skins have a null rectangle (0,0,0,0),
        // Windows have a rectangle in screen coordinates,
        // Controls have a rectangle that's relative to their window.
        //
        // So... 
        // Skins don't have a parent or the positioning, so they
        // just kinda ignore this function.
        //
        // Windows get the null rectangle from the skin, so they replace it
        // with the screen's work area. They use the primary monitor for 
        // positioning.
        //
        // Controls get the parent window's rectangle. 
        // Since we zero the origin, it's in the proper coordinates for the 
        // control.
        //
        // In otherwords, this code works for the way it's currently designed,
        // but if you change the heirarchy, have embedded controls inside controls,
        // or some crazyness like that, it'll need to change.
        parentRect = ((CATGuiObj*)fParent)->GetRect();
        parentRect.ZeroOrigin();

        if (parentRect.Width() == 0)
        {
            parentRect = GetPrimaryMonitorRect();
        }
    }

    CATInt32 width      = (CATInt32)this->GetAttribute(L"Width");
    CATInt32 height     = (CATInt32)this->GetAttribute(L"Height");

    CATString posString = this->GetAttribute(L"XPos");

    CATInt32 xPos;   
    if ((posString.Compare(L"CenterObject") == 0))
    {
        xPos = parentRect.CenterX() - (width/2);
        if ((width == 0) && (fImage != 0))
        {
            xPos = parentRect.CenterX() - (fImage->Width()/2);
        }
    }
    else if (posString.Compare(L"Center",6) == 0)
    {
        xPos = parentRect.CenterX();      
        if (posString.Length() > 6)
        {
            CATString xOffString = posString.Right(6);
            xPos += ((CATInt32)xOffString);
        }
    }
    else
        xPos = ((CATInt32)posString);


    CATInt32 yPos;
    posString = this->GetAttribute(L"YPos");

    if ((posString.Compare(L"CenterObject") == 0) )
    {
        yPos = parentRect.CenterY() - height/2;
        if ((height == 0) && (fImage != 0))
        {
            yPos = parentRect.CenterY() - (fImage->Height()/2);
        }
            
    }
    else if (posString.Compare(L"Center",6) == 0)
    {
        yPos = parentRect.CenterY();
        if (posString.Length() > 6)
        {
            CATString yOffString = posString.Right(6);
            yPos += ((CATInt32)yOffString);
        }
    }
    else
        yPos = ((CATInt32)posString);


    CATInt32 xMin       = ((CATInt32)this->GetAttribute(L"XMin"));
    CATInt32 yMin       = ((CATInt32)this->GetAttribute(L"YMin"));
    CATInt32 xMax       = ((CATInt32)this->GetAttribute(L"XMax"));
    CATInt32 yMax       = ((CATInt32)this->GetAttribute(L"YMax"));

    fMinWidth   = this->GetAttribute(L"MinWidth", 10);
    fMinHeight  = this->GetAttribute(L"MinHeight",10);
    fMaxWidth   = this->GetAttribute(L"MaxWidth", 0);
    fMaxHeight  = this->GetAttribute(L"MaxHeight",0);



    // XPin and YPin may be zero, but we need to know if they are 
    // specified.
    bool xPinUsed = false;
    bool yPinUsed = false;
    CATInt32 xPin   = 0;
    CATInt32 yPin   = 0;


    // Get XPin and YPin and flag if present
    CATString tmpString = this->GetAttribute(L"XPin");
    if (tmpString.IsEmpty() == false)
    {
        xPinUsed = true;
        if (tmpString.Compare("Center",6) == 0)
        {
            // Must be negative to get the right effect..
            xPin = -parentRect.CenterX();  
            if (tmpString.Length() > 6)
            {
                CATString xOffString = tmpString.Right(6);
                xPin += ((CATInt32)xOffString);
            }

        }
        else
        {
            xPin = ((CATInt32)tmpString);
        }
    }

    tmpString = this->GetAttribute(L"YPin");
    if (tmpString.IsEmpty() == false)
    {
        yPinUsed = true;
        if (tmpString.Compare("Center",6) == 0)
        {
            // Must be negative to get the right effect..
            yPin = parentRect.CenterY();
            if (tmpString.Length() > 6)
            {
                CATString yOffString = tmpString.Right(6);
                yPos += ((CATInt32)yOffString);
            }
        }
        else
        {
            yPin = ((CATInt32)tmpString);
        }
    }


    // Sanity check the pins
    if (xPinUsed)
    {
        if ((xPin > 0) || (width != 0) || (xPos < 0))
        {
            CATString misuseInfo;
            misuseInfo << "Error in skin XML for control: " << this->GetName();
            result = CATRESULTDESC(CAT_ERR_SKIN_XPIN_MISUSE,misuseInfo);
            xPin = 0;
        }
    }

    if (yPinUsed)
    {
        if ((yPin > 0) || (height != 0) || (yPos < 0))
        {
            CATString misuseInfo;
            misuseInfo << "Error in skin XML for control: " << this->GetName();
            result = CATRESULTDESC(CAT_ERR_SKIN_YPIN_MISUSE,misuseInfo);
            yPin = 0;
        }
    }


    // Image defaults
    if (this->fImage)
    {      
        if (width == 0)
        {
            width = fImage->Width();
        }

        if (height == 0) 
        {
            height = fImage->Height();
        }
    }   


    // Get the basic rect
    //
    // If xPinUsed is true, then it's scaled in relation to left/right.
    //
    // If the parentRect's width is zero, then it's a window
    // or skin and negative values are treated as negative values.
    //
    // If the parentRect's width != 0, then it's a control
    // and negative values mean measure from the right of the
    // parent for location.
    //
    // See comments above for design notes.... 
    if (xPinUsed)
    {
        fRect.left    = xPos;
        fRect.right   = parentRect.right + xPin + 1;
        if ((xMax) && (xMax < fRect.right))
        {
            fRect.right = xMax;
        }      
    }
    else if ((xPos >= 0) || (parentRect.Width() == 0))
    {
        fRect.left    = xPos;
        fRect.right   = width + xPos;
    }
    else
    {      
        fRect.right   = parentRect.right + xPos;
        fRect.left    = fRect.right - width;
        if ((xMin) && (fRect.left < xMin))
        {
            fRect.left = xMin;
            fRect.right = fRect.left + width;
        }
    }

    if (yPinUsed)
    {
        fRect.top     = yPos;
        fRect.bottom  = parentRect.bottom + yPin + 1;
        if ((yMax) && (yMax < fRect.bottom))
        {
            fRect.bottom = yMax;
        }      
    }
    else if ((yPos >= 0) || (parentRect.Height() == 0))
    {
        fRect.top     = yPos;
        fRect.bottom  = height + yPos;
    }
    else
    {
        fRect.bottom  = parentRect.bottom + yPos;
        fRect.top     = fRect.bottom - height;
        if ((yMin) && (fRect.top < yMin))
        {
            fRect.top = yMin;
            fRect.bottom = fRect.top + height;
        }
    }





    // Check min/max width and height
    if (fMinWidth != 0)
    {
        if (fRect.Width() < fMinWidth)
        {
            fRect.right = fRect.left + fMinWidth;
        }
    }
    // Don't let rect go negative even if fMinWidth isn't there
    else if (fRect.Width() < 1)
    {
        fRect.right = fRect.left + 1;
    }

    if (fMaxWidth != 0)
    {
        if (fRect.Width() > fMaxWidth)
        {
            fRect.right = fRect.left + fMaxWidth;
        }
    }

    // Now for height
    if (fMinHeight != 0)
    {
        if (fRect.Height() < fMinHeight)
        {
            fRect.bottom = fRect.top + fMinHeight;
        }
    }
    // Don't let rect go negative even if fMinHeight isn't there
    else if (fRect.Height() < 1)
    {
        fRect.bottom = fRect.top + 1;
    }


    if (fMaxHeight != 0)
    {
        if (fRect.Height() > fMaxHeight)
        {
            fRect.bottom = fRect.top + fMaxHeight;
        }
    }   


    // Resize the kids...
    CATResult testResult = CAT_SUCCESS;
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATGuiObj* curControl = (CATGuiObj*)curChild;
            CATRect curRect;

            if (CAT_SUCCESS != (testResult = curControl->RectFromAttribs()))
            {
                result = testResult;
            }

        }
    }

    // Implement sizeable controls later or override for them.
    // Most controls will be satisfied by the above.
    if (fParent)
    {
        ((CATGuiObj*)fParent)->MarkDirty();
    }

    return result;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
CATResult CATGuiObj::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    CATResult result = CAT_SUCCESS;
    CATResult testResult = CAT_SUCCESS;

    // Pass to controls...
    CATXMLObject* curChild = 0;
    CATUInt32 numChildren = this->GetNumChildren();

    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        if (0 != (curChild = GetChild(i)))
        {
            CATGuiObj* curControl = (CATGuiObj*)curChild;
            CATRect curRect;

            if (CAT_SUCCESS != (testResult = curControl->OnEvent(event, retVal)))
            {
                result = testResult;
            }

        }
    }

    return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CATRect CATGuiObj::GetRect(  )
{
    return fRect;
}

// Retrieve minimum and maximum size for the object.
// 
// If there is no minimum or maximum for a given parameter, then
// it will be set to 0.
//
// \param minWidth - ref to receive minimum width of the object
// \param minHeight - ref to receive minimum height of the object
// \param maxWidth - ref to receive maximum width of the object
// \param maxHeight - ref to receive maximum height of the object
void CATGuiObj::GetMinMax   ( CATInt32& minWidth, CATInt32& minHeight,
                             CATInt32& maxWidth, CATInt32& maxHeight)
{
    minWidth = this->fMinWidth;
    minHeight = this->fMinHeight;
    maxWidth = this->fMaxWidth;
    maxHeight = this->fMaxHeight;
}

bool CATGuiObj::IsEnabled() const
{
    bool enabled = fEnabled;

    // If parent is disabled, we are also disabled.
    if ((enabled) && (this->fParent))
    {
        enabled = ((CATGuiObj*)fParent)->IsEnabled();
    }

    return enabled;
}

void CATGuiObj::SetEnabled(bool enabled)
{
    fEnabled = enabled;

    if (fParent)
    {
        ((CATGuiObj*)fParent)->MarkDirty();
    }
}

//---------------------------------------------------------------------------
// GetPostRects() retrieves a rect, if any, that
// should be reserved for post-draw style drawing via OS-specific calls.
//---------------------------------------------------------------------------
bool CATGuiObj::GetPostRects(CATStack<CATRect>& rectStack)
{
    bool retVal = false;

    CATXMLObject* curChild = 0;
    CATUInt32 index;
    CATUInt32 numChildren = this->GetNumChildren();
    for (index = 0; index < numChildren; index++)
    {
        if (0 != (curChild = GetChild(index)))
        {
            CATGuiObj* curControl = (CATGuiObj*)curChild;
            retVal |= curControl->GetPostRects(rectStack);
        }
    }  

    return retVal;
}

// By default, be a container. Controls will call the callback.
bool CATGuiObj::ForEachControl(CATCONTROLFUNCB callback, void* userParam)
{
    CATXMLObject* curChild = 0;
    CATUInt32 index;
    CATUInt32 numChildren = this->GetNumChildren();
    for (index = 0; index < numChildren; index++)
    {
        if (0 != (curChild = GetChild(index)))
        {
            CATGuiObj* curControl = (CATGuiObj*)curChild;
            if (!curControl->ForEachControl(callback,userParam))
                return false;
        }
    }  

    return true;
}

// Recursively looks for the first object matching the name.
CATGuiObj* CATGuiObj::Find( const CATString& objectName,
                            const CATString& objectType)
{
    // Check if we match. IF so, return.
    if (this->GetName().Compare(objectName) == 0)
    {
        if (objectType.IsEmpty())
        {
            return this;
        }
        else if (objectType.Compare(this->GetType()) == 0)
        {
            return this;
        }
    }

    CATXMLObject* curChild = 0;
    CATUInt32 index;
    CATUInt32 numChildren = this->GetNumChildren();

    for (index = 0; index < numChildren; index++)
    {
        if (0 != (curChild = GetChild(index)))
        {         
            CATGuiObj* curControl = (CATGuiObj*)curChild;
            CATGuiObj* foundControl = curControl->Find( objectName );
            if (foundControl)
            {
                return foundControl;
            }
        }
    }  
    return 0;
}

CATRect CATGuiObj::GetRectAbs( bool screenCoordinates)
{
    CATRect absRect = this->fRect;
    if (fParent != 0)
    {
        CATRect parRect = ((CATGuiObj*)fParent)->GetRectAbs(screenCoordinates);
        absRect.Offset( parRect.left, parRect.top );
    }

    return absRect;
}

bool CATGuiObj::IsVisible(const CATGuiObj* object) const
{
    if (fVisible)
    {
        return ((CATGuiObj*)fParent)->IsVisible(this);
    }
    return false;   
}


void CATGuiObj::SetVisible(bool visible)
{
    this->fVisible = visible;
    // Force dirty when visibility changes.
    this->MarkDirty();
}


void CATGuiObj::OnCommand( CATCommand& command, CATControl* ctrl)
{
    if (fParent)
    {
        ((CATGuiObj*)fParent)->OnCommand( command, ctrl);
    }
}


CATUInt32 CATGuiObj::GetAccessRole()
{
    return 0;
}

CATUInt32 CATGuiObj::GetAccessState()
{
    CATUInt32 state = 0;
    state |= (this->IsEnabled() ? 0 : CAT_STATE_SYSTEM_UNAVAILABLE);
    state |= (this->IsVisible() ? 0 : CAT_STATE_SYSTEM_INVISIBLE);
    return state;
}