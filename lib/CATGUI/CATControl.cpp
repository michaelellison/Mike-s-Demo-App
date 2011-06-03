//---------------------------------------------------------------------------
/// \file CATControl.cpp
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
//---------------------------------------------------------------------------
#include "CATControl.h"
#include "CATCursor.h"
#include "CATWindow.h"
#include "CATApp.h"
#include "CATFileSystem.h"
#include "CATStream.h"
#include "CATCommand.h"
#include "CATEventDefs.h"
//---------------------------------------------------------------------------
// CATControl constructor (inherited from CATXMLObject)
// \param element - Type name (e.g. "Button", "Label", etc.)
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
//---------------------------------------------------------------------------
CATControl::CATControl(  const CATString&             element, 
                         const CATString&               rootDir)
: CATWidget(element, rootDir)
{
    fEnabled             = true;
    fPressed             = false;
    fValue               = 0.0f;
    fDefValue            = 0.0f;
    fMinValue            = 0.0f;
    fMaxValue            = 1.0f;
    fForegroundColor     = CATColor(0,0,0);
    fBackgroundColor     = CATColor(192,192,192);
    fFgDisColor          = CATColor(192,192,192);
    fBgDisColor          = CATColor(128,128,128);
    fImageDisabled       = 0;
    fImagePressed        = 0;
    fImageFocus          = 0;   
    fImageFocusAct       = 0;   
    fImageActive         = 0;
    fActive              = false;
    fFocused             = false;
    fShowBg              = false;
    fMultiline           = false;
    fAutoScaleText       = false;
    fTextCentered	     = false;
    fTextOffset.x        = 0;
    fTextOffset.y        = 0;
    fTextOffsetPressed.x = 2;
    fTextOffsetPressed.y = 2;
}

//---------------------------------------------------------------------------
CATResult CATControl::Load(		// Optional progress callback information
                           CATPROGRESSCB				progressCB,
                           void*							progressParam,
                           CATFloat32						progMin,
                           CATFloat32						progMax)
{
    CATResult result = CATGuiObj::Load(progressCB, progressParam, progMin, progMax);


    if (CATFAILED(result))
    {
        return result;
    }

    // Sanity check images
    if (fImage != 0)
    {
        CATResult testResult = CAT_SUCCESS;

        if (fImageDisabled)
        {
            testResult = CheckImageSize(fImageDisabled);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageFocus)
        {
            testResult = CheckImageSize(fImageFocus);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageFocusAct)
        {
            testResult = CheckImageSize(fImageFocusAct);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImageActive)
        {
            testResult = CheckImageSize(fImageActive);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }

        if (fImagePressed)
        {
            testResult = CheckImageSize(fImagePressed);
            if (testResult != CAT_SUCCESS)
                result = testResult;
        }
    }

    return result;
}

// CheckImageSize()
// Checks an image against fImage. If their sizes do not match,
// then it creates a matching one with a red background and copies the 
// the available portion of image into it.
//
// This is done to allow the app to run with mismatched images, while
// still making it obvious that the skin should be fixed.
CATResult CATControl::CheckImageSize(CATImage *& image, const CATImage* baseImage)
{
    const CATImage* checkImage = (baseImage == 0)?fImage:baseImage;

    // Check image size should only be called if we have multiple
    // images....
    if ((checkImage == 0) && (image != 0))
    {
        return CATRESULTDESC(CAT_ERR_CONTROL_NO_IMAGE, fName);
    }
    if ((checkImage->Width() != image->Width()) ||
        (checkImage->Height() != image->Height()))
    {      
        CATRect imageRect(0,0,checkImage->Width(), checkImage->Height());
        CATImage* newDisabled = 0;

        if (CATSUCCEEDED(CATImage::CreateImage(newDisabled,checkImage->Width(), checkImage->Height(),false,false)))
        {
            newDisabled->FillRect(imageRect, CATColor(255,0,0));
            newDisabled->CopyOver(image,
                0,0,0,0,
                CATMin(checkImage->Width(), image->Width()),
                CATMin(checkImage->Height(), image->Height()));
            CATImage::ReleaseImage(image);
            image = newDisabled;
        }
        else
        {
            CATImage::ReleaseImage(image);
        }

        return CATRESULTDESC(CAT_STAT_CONTROL_IMAGE_SIZE_MISMATCH, fName);
    }
    return CAT_SUCCESS;
}
//---------------------------------------------------------------------------
// CATControl destructor
//---------------------------------------------------------------------------
CATControl::~CATControl()
{
    if (fImageDisabled)
    {
        CATImage::ReleaseImage(fImageDisabled);
    }

    if (fImagePressed)
    {
        CATImage::ReleaseImage(fImagePressed);      
    }

    if (fImageFocus)
    {
        CATImage::ReleaseImage(fImageFocus);
    }
    if (fImageFocusAct)
    {
        CATImage::ReleaseImage(fImageFocusAct);
    }

    if (fImageActive)
    {
        CATImage::ReleaseImage(fImageActive);
    }
}


/// ParseAttributes() parses the known attributes for an object.
CATResult CATControl::ParseAttributes()
{
    CATResult result = CATWidget::ParseAttributes();
    CATString attrib;

    fText         = GetAttribute(L"Text");
    fMultiline    = GetAttribute(L"Multiline",fMultiline );
    fShowBg       = GetAttribute(L"ShowBG",   fShowBg    );

    fTextCentered = GetAttribute(L"TextCentered",fTextCentered);
    fTextOffset.x = GetAttribute(L"TextOffsetX",fTextOffset.x);
    fTextOffset.y = GetAttribute(L"TextOffsetY",fTextOffset.y);

    fTextOffsetPressed.x = GetAttribute(L"TextOffsetPressedX",fTextOffsetPressed.x);
    fTextOffsetPressed.y = GetAttribute(L"TextOffsetPressedY",fTextOffsetPressed.y);

    fFontName      = GetAttribute(L"FontName",fFontName);
    fFontSize      = GetAttribute(L"FontSize",fFontSize);
    fDefValue      = GetAttribute(L"DefValue",fDefValue);
    
    fAutoScaleText = GetAttribute(L"AutoScaleText",fAutoScaleText);
    
    fValue        = fDefValue;
    fMinValue     = GetAttribute(L"MinValue",fMinValue);
    fMaxValue     = GetAttribute(L"MaxValue",fMaxValue);

    if (fValue < fMinValue)
        fValue = fMinValue;
    if (fValue > fMaxValue)
        fValue = fMaxValue;

    fCmdString    = GetAttribute(L"Command",fCmdString);
    fTarget       = GetAttribute(L"Target", fTarget);
    fCmdType      = GetAttribute(L"CommandType", fCmdType);
    fCmdParam     = GetAttribute(L"Parameter", fCmdParam);

    attrib = GetAttribute(L"ColorBackDis");
    if (attrib.IsEmpty() == 0)
    {
        CATUInt32 rawColor = attrib.FromHex();
        this->fBgDisColor.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fBgDisColor.g = (CATUInt8)((rawColor & 0xff00) >>8);
        this->fBgDisColor.b = (CATUInt8)((rawColor & 0xff));

        this->fBgDisColor.a = 255;
    }
    attrib = GetAttribute(L"ColorForeDis");
    if (attrib.IsEmpty() == 0)
    {
        CATUInt32 rawColor = attrib.FromHex();
        this->fFgDisColor.r = (CATUInt8)((rawColor & 0xff0000) >>16);
        this->fFgDisColor.g = (CATUInt8)((rawColor & 0xff00) >>8);
        this->fFgDisColor.b = (CATUInt8)((rawColor & 0xff));

        this->fFgDisColor.a = 255;
    }


    CATCURSORTYPE cursorType = CATCURSOR_ARROW;
    attrib = GetAttribute(L"Cursor");
    // Select based on value here...
    if          (0 == attrib.Compare("NoAction"))  cursorType = CATCURSOR_NOACTION;
    else if (0 == attrib.Compare("Wait"))      cursorType = CATCURSOR_WAIT;
    else if (0 == attrib.Compare("Text"))      cursorType = CATCURSOR_TEXT;
    else if (0 == attrib.Compare("Hand"))      cursorType = CATCURSOR_HAND;
    else if (0 == attrib.Compare("LeftRight")) cursorType = CATCURSOR_LEFTRIGHT;
    else if (0 == attrib.Compare("TopBottom")) cursorType = CATCURSOR_TOPBOTTOM;
    else if (0 == attrib.Compare("Size"))      cursorType = CATCURSOR_SIZE;
    else if (0 == attrib.Compare("Move"))      cursorType = CATCURSOR_MOVE;

    this->fCursor.SetType(cursorType);

    attrib = GetAttribute(L"ImageDisabled");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fImageDisabled);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }
    attrib = GetAttribute(L"ImagePressed");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fImagePressed);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageFocus");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fImageFocus);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageFocusAct");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fImageFocusAct);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    attrib = GetAttribute(L"ImageActive");
    if (!attrib.IsEmpty())
    {
        CATResult tmpResult = LoadSkinImage(attrib,fImageActive);
        if (CATFAILED(tmpResult))
            result = tmpResult;
    }

    return result;
}

//---------------------------------------------------------------------------
// GetWindow() retrieves the parent window.
//---------------------------------------------------------------------------
CATWindow* CATControl::GetWindow() const
{
    return ((CATGuiObj*)fParent)->GetWindow();
}

//---------------------------------------------------------------------------
// GetValue() retrieves the value of the control.
// \return CATFloat32 - control value 
//---------------------------------------------------------------------------
CATFloat32 CATControl::GetValue() const
{
    return fValue;
}

void CATControl::SetString ( const CATString& text )
{
    fText = text;
}

CATString CATControl::GetString () const
{
    return fText;
}

//---------------------------------------------------------------------------
// SetValue() sets the value of the control 
// \param newValue - float from 0 to 1 for new value.
//---------------------------------------------------------------------------
void CATControl::SetValue(CATFloat32 newValue, bool sendCommand)
{
    fValue = newValue;

    this->MarkDirty();

    if (sendCommand)
    {
        ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);
    }   
}

//---------------------------------------------------------------------------
// ResetValue() resets the control to its default value
//---------------------------------------------------------------------------
void CATControl::ResetValue()
{
    fValue = this->fDefValue;
    this->MarkDirty();
}


//---------------------------------------------------------------------------
// GetColorFore() retrieves the foreground color for the control
//---------------------------------------------------------------------------
CATColor CATControl::GetColorFore() const
{
    if (this->IsEnabled() == false)
    {
        return fFgDisColor;
    }

    return this->fForegroundColor;
}

//---------------------------------------------------------------------------
// GetColorBack() retrieves the background color for the control
//---------------------------------------------------------------------------
CATColor CATControl::GetColorBack() const
{
    if (this->IsEnabled() == false)
    {
        return fBgDisColor;
    }
    return this->fBackgroundColor;
}

//---------------------------------------------------------------------------
// SetColorFore() sets the foreground color for the control
//---------------------------------------------------------------------------
void CATControl::SetColorFore(const CATColor& color)
{
    this->fForegroundColor = color;
    this->MarkDirty();
}

void CATControl::SetColorForeDisabled(const CATColor& color)
{
    this->fFgDisColor = color;
    this->MarkDirty();
}

//---------------------------------------------------------------------------
// SetColorFore() sets the foreground color for the control
//---------------------------------------------------------------------------
void CATControl::SetColorBack(const CATColor& color)
{
    this->fBackgroundColor = color;
    this->MarkDirty();
}

void CATControl::SetColorBackDisabled(const CATColor& color)
{
    this->fBgDisColor = color;
    this->MarkDirty();
}


//---------------------------------------------------------------------------
// GetCursor() retrieves the object's mouse cursor.
//---------------------------------------------------------------------------
CATCursor* CATControl::GetCursor()
{
    return &this->fCursor;
}

//---------------------------------------------------------------------------
// IsFocusable() returns true if the control can receive
// focus, and false otherwise.
//---------------------------------------------------------------------------
bool CATControl::IsFocusable() const
{
    if (this->IsVisible() == false)
        return false;

    return true;
}

//---------------------------------------------------------------------------
// SetFocused() sets the control's focused state.
//
// \param focused - if true, then the control is given focus.
//                  if false, focus is removed.      
//---------------------------------------------------------------------------
void CATControl::SetFocused(bool focused)
{
    // Just in case it was pressed via key but not released
    this->fPressed = false;

    this->fFocused = focused;
    this->MarkDirty();
}

//---------------------------------------------------------------------------
// SetActive() sets the control's active state.
//
// \param active - if true, then the control is under the mouse.
//                  if false, active flag is removed.      
//---------------------------------------------------------------------------
void CATControl::SetActive(bool active)
{
    if (fActive != active)
    {
        this->fActive = active;
        this->MarkDirty();
    }
}

//---------------------------------------------------------------------------
// IsFocused() returns true if the control has the current focus.
//---------------------------------------------------------------------------
bool CATControl::IsFocused() const
{
    return this->fFocused;
}

//---------------------------------------------------------------------------
// IsPressed() returns true if the control is pressed
//---------------------------------------------------------------------------
bool CATControl::IsPressed() const
{
    return this->fPressed;
}

//---------------------------------------------------------------------------
// IsFocused() returns true if the control has the mouse over it.
//---------------------------------------------------------------------------
bool CATControl::IsActive() const
{
    return this->fActive;
}



//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATControl::Draw(CATImage* image, const CATRect& dirtyRect)
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
            if (drawRect.Intersect(CATRect( fRect.left, 
                                            fRect.top, 
                                            fRect.left + fImageDisabled->Width(),
                                            fRect.top  + fImageDisabled->Height()),
                                    &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

                image->Overlay( this->fImageDisabled,
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
            if (this->IsPressed() && (this->fImagePressed))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                                               fRect.top, 
                                               fRect.left + fImagePressed->Width(),
                                               fRect.top  + fImagePressed->Height()),
                                       &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( this->fImagePressed,
                                    drawRect.left,
                                    drawRect.top,
                                    ourRect.left,
                                    ourRect.top,
                                    ourRect.Width(),
                                    ourRect.Height());
                    drawn = true;
                }
            }

            if ((!drawn) &&  ((IsFocused() && IsActive()) || IsPressed()) && (this->fImageFocusAct))
            {
                if (drawRect.Intersect( CATRect(fRect.left, 
                                                fRect.top, 
                                                fRect.left + fImageFocusAct->Width(),
                                                fRect.top  + fImageFocusAct->Height()),
                                        &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( this->fImageFocusAct,
                                    drawRect.left,
                                    drawRect.top,
                                    ourRect.left,
                                    ourRect.top,
                                    ourRect.Width(),
                                    ourRect.Height());
                    drawn = true;
                }
            }
            
            if ((!drawn) &&  (IsFocused() || IsPressed()) && (this->fImageFocus))
            {
                if (drawRect.Intersect( CATRect(fRect.left, 
                                                fRect.top, 
                                                fRect.left + fImageFocus->Width(),
                                                fRect.top  + fImageFocus->Height()),
                                        &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( this->fImageFocus,
                                    drawRect.left,
                                    drawRect.top,
                                    ourRect.left,
                                    ourRect.top,
                                    ourRect.Width(),
                                    ourRect.Height());
                    drawn = true;
                }
            }


            if ((!drawn) && (IsActive() && (this->fImageActive)))
            {
                if (drawRect.Intersect( CATRect(fRect.left, 
                                                fRect.top, 
                                                fRect.left + fImageActive->Width(),
                                                fRect.top  + fImageActive->Height()),
                                        &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay( this->fImageActive,
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

        if ((!drawn) && (this->fImage != 0))
        {
            if (drawRect.Intersect( CATRect(fRect.left, 
                                            fRect.top, 
                                            fRect.left + fImage->Width(),
                                            fRect.top  + fImage->Height()),
                                    &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

                image->Overlay( this->fImage,
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
            // No images or fallbacks.  Ok, fine, draw a manual box.

            if (this->IsEnabled() == false)
            {
                image->FillRect(drawRect, this->fBgDisColor);
            }
            else
            {
                image->FillRect(drawRect, this->fBackgroundColor);
            }
        }
    }
}

//---------------------------------------------------------------------------
// PostDraw() draws the control into the parent window using OS-specific
// code.
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATControl::PostDraw(CATDRAWCONTEXT context, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
        return;

    if ((fText.IsEmpty() == false) || (fShowBg))
    {
        CATColor foreColor(fForegroundColor);
        CATColor backColor(fBackgroundColor);

        if (this->IsEnabled() == false)
        {
            foreColor = fFgDisColor;      
            backColor = fBgDisColor;
        }

        CATRect textRect = fRect;

        if (this->fPressed)
        {
            textRect.top  += fTextOffsetPressed.x;
            textRect.left += fTextOffsetPressed.y;
        }
        else
        {
            textRect.top  += fTextOffset.x;
            textRect.left += fTextOffset.y;
        }
        
        CATRect dRect = dirtyRect;
        this->GetWindow()->WidgetToWindow(this,textRect);
        this->GetWindow()->WidgetToWindow(this,dRect);

        this->GetWindow()->PostDrawText(fText,
                                        context, 
                                        textRect, 
                                        dRect,
                                        foreColor,
                                        fFontName,
                                        fFontSize,
                                        fMultiline,
                                        fShowBg?&backColor:0,
                                        false,
                                        fTextCentered,
                                        fAutoScaleText);
    }
}


//---------------------------------------------------------------------------
// GetCommand() returns the command for the control
//---------------------------------------------------------------------------
CATCommand CATControl::GetCommand() const
{
    return CATCommand(  this->fCmdString, 
                        this->GetValue(), 
                        this->fCmdParam, 
                        this->fTarget, 
                        this->fCmdType);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
CATResult CATControl::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    switch (event.fEventCode)
    {
    case  CATEVENT_GUI_VAL_CHANGE:
        // If our command string is the same as the GUI value, then
        // we should ensure that our value matches the one
        // in the event.
        //
        // fStringParam1 - command string
        // fStringParam2 - String parameter of command
        // fStringParam3 - String value of command, or empty if none
        // fFloatParam1  - Value of control
        // fVoidParam - ptr to control that caused it, or null.
        if (this->fCmdString.Compare(event.fStringParam1) == 0)
        {
            // MAke sure we're not the control that sent it...
            if (this != (CATControl*)event.fVoidParam)
            {

                // Don't send another command!
                this->SetValue(event.fFloatParam1, false);
                retVal++;
            }
        }
        break;

    case  CATEVENT_GUI_VAL_CHANGE_MATCHPARAM_ONLY:
        // As above, except only reflect change if parameter
        // matches as well as command string.
        //
        // fStringParam1 - command string
        // fStringParam2 - String parameter of command
        // fFloatParam1  - Value of control
        // fVoidParam - ptr to control that caused it, or null.
        if (this->fCmdString.Compare(event.fStringParam1) == 0)
        {
            if (this->fCmdParam.Compare(event.fStringParam2) == 0)
            {
                // MAke sure we're not the control that sent it...
                if (this != (CATControl*)event.fVoidParam)
                {

                    // Don't send another command!
                    this->SetValue(event.fFloatParam1, false);
                    retVal++;
                }
            }
        }
        break;
    case CATEVENT_TAB_HIDE:
		  break;

	 case CATEVENT_TAB_SHOW:
 		  MarkDirty();
        break;

    default:
        break;
    }

    return CATWidget::OnEvent(event,retVal);   
}

//---------------------------------------------------------------------------
void CATControl::TrackMouseMove(const CATPOINT& point, bool leftButton, CATMODKEY modKey)
{
    if (leftButton)
    {

        if (fPressed && (!this->fRect.InRect(point)))
        {
            this->fPressed = false;
            this->fActive  = true;
            this->MarkDirty();
        }
        else if ((!fPressed) && (this->fRect.InRect(point)))
        {
            this->fPressed = true;
            this->fActive  = false;
            this->MarkDirty();
        }   
    }
}

void CATControl::TrackMouseTimer(CATMODKEY modKey)
{
}

//---------------------------------------------------------------------------

void CATControl::TrackMouseWheel(  const CATPOINT& point,
                                 CATFloat32        wheelMove,
                                 CATMODKEY		 modKey)
{
    // Will this one be used?   
}      
//---------------------------------------------------------------------------
void CATControl::TrackMouseDown(const CATPOINT& point, CATMODKEY modKey)
{   
    this->fPressed = true;
    this->fActive = false;
    this->MarkDirty();
}      
//---------------------------------------------------------------------------
void CATControl::TrackMouseRelease(const CATPOINT& point, CATMODKEY modKey)
{   
    if (this->fRect.InRect(point))
    {
        // Received a click
        this->OnMouseClick();
        this->fActive  = true;
        this->fPressed = false;
        this->MarkDirty();
    }
    else
    {         
        this->fActive  = false;
        this->fPressed = false;
        this->MarkDirty();
    }                  
}      
//---------------------------------------------------------------------------
// Mouse wheel over control, but not already tracked

void CATControl::OnMouseWheel( const CATPOINT& point,
                              CATFloat32        wheelMove,
                              CATMODKEY			modKey)
{

}
//---------------------------------------------------------------------------
void CATControl::OnMouseClick()
{   
    this->SetFocused(false);
    ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);
}

void CATControl::ResetCursorToDefault()
{
    CATString value = this->GetAttribute(L"Cursor");
    CATCURSORTYPE cursorType = CATCURSOR_ARROW;
    // Select based on value here...
    if (0 == value.Compare("NoAction"))       cursorType = CATCURSOR_NOACTION;
    else if (0 == value.Compare("Wait"))      cursorType = CATCURSOR_WAIT;
    else if (0 == value.Compare("Text"))      cursorType = CATCURSOR_TEXT;
    else if (0 == value.Compare("Hand"))      cursorType = CATCURSOR_HAND;
    else if (0 == value.Compare("LeftRight")) cursorType = CATCURSOR_LEFTRIGHT;
    else if (0 == value.Compare("TopBottom")) cursorType = CATCURSOR_TOPBOTTOM;
    else if (0 == value.Compare("Size"))      cursorType = CATCURSOR_SIZE;
    else if (0 == value.Compare("Move"))      cursorType = CATCURSOR_MOVE;

    this->fCursor.SetType(cursorType);
}

void CATControl::OnKeyDown(const CATKeystroke& keystroke)
{
    if (keystroke.GetNormalKey() == 0x20)
    {
        this->fPressed = true;
        this->fActive = false;
        this->MarkDirty();
    }         
}

void CATControl::OnKeyUp(const CATKeystroke& keystroke)
{
    if (this->fPressed)
    {
        if (keystroke.GetNormalKey() == 0x20)
        {         
            this->fPressed = false;
            this->fActive =  false;
            this->MarkDirty();         
            this->OnMouseClick();
        }
    }
}

void CATControl::OnKeyPress(const CATKeystroke& keystroke)
{
}

// OnParentCreate() is called when the parent window is created.
// 
// Most controls won't need this, but any that create their own
// windows should do so at this point.
void CATControl::OnParentCreate()
{
}


// OnParentDestroy() is called as the parent window is destroyed.
//
// Controls that create their own windows during OnParentCreate()
// should destroy them during OnParentDestroy()
void CATControl::OnParentDestroy()
{
}

// Bounds check the value, return true if was already good, false if had to change.
bool CATControl::BoundsCheckValue()
{
    bool wasValid = true;

    if (fValue > fMaxValue)
    {
        fValue = fMaxValue;
        wasValid = false;
    }
    if (fValue < fMinValue)
    {
        fValue = fMinValue;
        wasValid = false;
    }
    return wasValid;
}

CATFloat32 CATControl::GetValPercent()
{
    BoundsCheckValue();
    return (fValue - fMinValue) / (fMaxValue - fMinValue);
}

CATFloat32 CATControl::GetValRange()
{
    return fMaxValue - fMinValue;
}

CATFloat32 CATControl::GetMinValue()
{
    return fMinValue;
}
CATFloat32 CATControl::GetMaxValue()
{
    return fMaxValue;
}

void CATControl::SetMinMax(CATFloat32 minValue, CATFloat32 maxValue)
{  
    CATASSERT( minValue <= maxValue, "Min/Max are flipped!");
    if (minValue > maxValue)
    {
        CATSwap(minValue,maxValue);
    }

    fMinValue = minValue;
    fMaxValue = maxValue;
    BoundsCheckValue();
    this->MarkDirty();
}

void CATControl::GetFontInfo(CATString& fontName, CATFloat32& fontSize)
{
    fontName = fFontName;
    fontSize = fFontSize;
}

// OnMouseDoubleClick() is called the mouse is double clicked.
//
// \param modKey - Key state modifiers for ctrl/shift/alt/etc.
void CATControl::OnMouseDoubleClick(CATMODKEY modKey)
{
    // By default, treat as another click.
    this->OnMouseClick();
}

CATControl* CATControl::HitTest(const CATPOINT& point)
{
    if (IsEnabled() && IsFocusable() && GetRect().InRect(point) )
    {            
        return this;
    }

    return 0;
}


bool CATControl::ForEachControl(CATCONTROLFUNCB callback, void* userParam)
{
    // Call the control callback, since we're a control...
    if (callback)
    {
        return callback(this,userParam);
    }

    return true;
}

/// Utility func to filter '&'s and make them appear in menus
/// instead of becoming menu mnemonics.
CATString CATControl::FilterGUIString( const CATString& unfiltered)
{
    CATString tmpText = unfiltered;		

#ifdef _WIN32
    // Filter out hotkeys for now. We need '&'s in name strings
    // that aren't *only* used for menus.
    CATUInt32 offset = 0;			
    if (tmpText.Find( '&',offset))
    {
        tmpText = "";				
        CATUInt32 numChars = unfiltered.LengthCalc();
        for (CATUInt32 iChar = 0; iChar < numChars; iChar++)
        {
            CATWChar curChar = unfiltered.GetWChar(iChar);
            if (curChar == '&')
            {
                tmpText << "&&";
            }
            else
            {
                tmpText << curChar;
            }
        }				
    }
#endif

    return tmpText;
}

void CATControl::OnRightMouseClick()
{
    // Nadda for most controls.
    // Probably add edit-box type value mod for knob/slider	
}

CATUInt32 CATControl::GetAccessState()
{
    CATUInt32 state = CATGuiObj::GetAccessState();

    state |= this->IsActive()    ? CAT_STATE_SYSTEM_HOTTRACKED:0;
    state |= this->IsPressed()   ? CAT_STATE_SYSTEM_PRESSED:0;
    state |= this->IsFocused()   ? CAT_STATE_SYSTEM_FOCUSED:0;
    state |= this->IsFocusable() ? CAT_STATE_SYSTEM_FOCUSABLE:0;

    return state;
}