//---------------------------------------------------------------------------
/// \file CATKnob.cpp
/// \brief Knob control for GUI
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
#include <math.h>

#include "CATKnob.h"
#include "CATWindow.h"
#include "CATFilterEWMA.h"
#include "CATCursor.h"
#include "CATUtil.h"
#include "CATEditBox.h"

// base resolution of knob
const CATFloat32 kKNOBSTEP       = 0.001f;
const CATFloat32 kCIRCLEFILTER   = 0.7f;
const CATFloat32 kVERTICALFILTER = 0.5f;

//---------------------------------------------------------------------------
// CATKnob constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Button")
// \param attribs - attribute information for the window
// \param rootDir - root directory of skin
// \param parent - parent XML object (should be a "Window" element)
//---------------------------------------------------------------------------
CATKnob::CATKnob(      const CATString&             element, 
                 const CATString&             rootDir)
                 : CATControl(element,  rootDir)
{   
    // Defaults
    fCursorHidden	= false;
    fValueType		= CATVALUE_LINEAR;
    fLastKnob      = 0;
    fKnobStyle     = CATKNOB_VERTICAL;
    fLastKnobStyle	= fKnobStyle;
    fFilterCoef    = kVERTICALFILTER;
    fFilter        = new CATFilterEWMA(kVERTICALFILTER);
    fCommandTrack  = false;   
    fMinDegree     = 10.0f;
    fMaxDegree     = 350.0f;
    fCursor.SetType(CATCURSOR_TOPBOTTOM);
	 this->fShowHintValue = false;
}

//---------------------------------------------------------------------------
// CATKnob destructor
//---------------------------------------------------------------------------
CATKnob::~CATKnob()
{
    if (fFilter)
    {
        delete fFilter;
        fFilter = 0;
    }

}

void CATKnob::TrackMouseDown(const CATPOINT& point, CATMODKEY modKey)
{

    CATControl::TrackMouseDown(point,modKey);

    fFilter->Reset();
    fStartPoint = point;      
    fLastPoint = point;
    fLastMove = point;
    fPressed = true;

    // If we're keeping the mouse in the center, set the point and hide mouse
    switch (GetActiveStyle(modKey))
    {
    case CATKNOB_CIRCLE:            
        fFilter->SetCoefficient(fFilterCoef);
        this->SetValueFromPoint(point, modKey);
        break;

    case CATKNOB_VERTICAL:
    default:
        {
            fFilter->SetCoefficient(fFilterCoef);
            this->fCursor.SetType(CATCURSOR_HIDE);
            fCursorHidden = true;
            CATPOINT cursorPoint = fStartPoint;
            this->GetWindow()->WidgetToWindow(this,cursorPoint);
            this->GetWindow()->OSSetMouse(cursorPoint,fCursor);
        }
        break;
    }


}

void CATKnob::TrackMouseRelease(const CATPOINT& point, CATMODKEY modKey)
{   
    // If we're keeping the mouse in the center, set the point and restore mouse
    switch (GetActiveStyle(modKey))
    {
    case CATKNOB_CIRCLE:
        if (fCursorHidden)
        {
            this->ResetCursorToDefault();   
            this->GetWindow()->OSSetCursor(fCursor);
        }
        break;

    case CATKNOB_VERTICAL:
    default:
        {
            CATPOINT cursorPoint = fStartPoint;
            this->ResetCursorToDefault();   
            this->GetWindow()->WidgetToWindow(this,cursorPoint);
            this->GetWindow()->OSSetMouse(cursorPoint,fCursor);         
            fCursorHidden = false;
        }
        break;
    }

    this->fPressed = false;
    this->fActive = true;
    this->MarkDirty();
    this->OnMouseClick();
}

void CATKnob::TrackMouseMove(const CATPOINT& point, bool left, CATMODKEY modKey)
{   
    if (left)
    {
        fLastMove = point;

        switch (GetActiveStyle(modKey))
        {
        case CATKNOB_CIRCLE:
            {
                if (fCursorHidden)
                {
                    ResetCursorToDefault();
                    this->GetWindow()->OSSetCursor(fCursor);
                    fCursorHidden = false;
                }

                SetValueFromPoint(point,modKey);
            }                        
            break;

        case CATKNOB_VERTICAL:
        default:
            {
                if (!fCursorHidden)
                {
                    this->fCursor.SetType(CATCURSOR_HIDE);
                    fCursorHidden = true;
                }
                CATInt32 diffY = fLastPoint.y - point.y;
                CATFloat32 keyStep = (modKey & CATMODKEY_SHIFT)?0.1f:1.0f;      
                CATFloat32 delta = diffY*kKNOBSTEP*keyStep*GetValRange();

                this->fValue += (float)this->fFilter->Filter(delta);

                // If we're keeping the mouse in the control             
                CATPOINT cursorPoint = fStartPoint;
                this->GetWindow()->WidgetToWindow(this,cursorPoint);
                this->GetWindow()->OSSetMouse(cursorPoint,fCursor);      
                fLastPoint = fStartPoint;
            }
            break;
        }


        BoundsCheckValue();

        // fLastPoint = point;
        CATInt32 knobNumber = (int)((47 * GetValPercent() ) + 0.5);
        if (knobNumber != this->fLastKnob)
        {
            this->MarkDirty();
        }

        if (this->fCommandTrack)
        {      
            this->OnMouseClick();
            // Maintain pressed state...
            fPressed = true;
        }
    }
}


void CATKnob::OnMouseWheel(    const CATPOINT& point,
                           CATFloat32        wheelMove,
                           CATMODKEY			modKey)
{

    CATFloat32 keyStep = (modKey & CATMODKEY_SHIFT)?10.0f:100.0f;
    CATFloat32 delta = (wheelMove*kKNOBSTEP)*keyStep*GetValRange();
    this->fValue += (CATFloat32)this->fFilter->Filter(delta);

    BoundsCheckValue();

    this->MarkDirty();      
    this->OnMouseClick();
}
//---------------------------------------------------------------------------
void CATKnob::OnKeyPress(const CATKeystroke& keystroke)
{
    CATControl::OnKeyPress(keystroke);
}

void CATKnob::OnKeyDown(const CATKeystroke& keystroke)
{   
    if (keystroke.IsSpecial())
    {
        CATFloat32 delta = 0;

        switch (keystroke.GetSpecialKey())
        {
        case CATKEY_LEFT:
            delta = -kKNOBSTEP * GetValRange();
            break;      
        case CATKEY_RIGHT:
            delta = kKNOBSTEP * GetValRange();
            break;
        case CATKEY_UP:
            delta = kKNOBSTEP * 10 * GetValRange();
            break;
        case CATKEY_DOWN:
            delta = -kKNOBSTEP * 10 * GetValRange();
            break;
        case CATKEY_NEXT:
            delta = -kKNOBSTEP * 100 * GetValRange();
            break;
        case CATKEY_PRIOR:
            delta = kKNOBSTEP * 100 * GetValRange();
            break;

        case CATKEY_HOME:
            fValue = fMinValue;
            this->MarkDirty();
            this->OnMouseClick();
            return;
            break;

        case CATKEY_END:
            fValue = fMaxValue;
            this->MarkDirty();
            this->OnMouseClick();
            return;
            break;
        }      

        if (delta != 0)
        {
            fValue += delta;
            this->BoundsCheckValue();
            this->MarkDirty();      
            this->OnMouseClick();
        }
    }

    // Allow default handling on stuff we don't do.
    CATControl::OnKeyDown(keystroke);
}

//---------------------------------------------------------------------------
CATResult CATKnob::RectFromAttribs()
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
        // Windows get the null rectangle from the skin, so they ignore
        // the parent rectangle.
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
    }


    CATInt32 xPos       = (CATInt32)this->GetAttribute(L"XPos");
    CATInt32 yPos       = (CATInt32)this->GetAttribute(L"YPos");
    CATInt32 width      = (CATInt32)this->GetAttribute(L"Width");
    CATInt32 height     = (CATInt32)this->GetAttribute(L"Height");

    CATInt32 xMin       = (CATInt32)this->GetAttribute(L"XMin");
    CATInt32 yMin       = (CATInt32)this->GetAttribute(L"YMin");
    CATInt32 xMax       = (CATInt32)this->GetAttribute(L"XMax");
    CATInt32 yMax       = (CATInt32)this->GetAttribute(L"YMax");

    fMinWidth   = (CATInt32)this->GetAttribute(L"MinWidth");
    fMinHeight  = (CATInt32)this->GetAttribute(L"MinHeight");
    fMaxWidth   = (CATInt32)this->GetAttribute(L"MaxWidth");
    fMaxHeight  = (CATInt32)this->GetAttribute(L"MaxHeight");


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
        xPin = (CATInt32)tmpString;
        xPinUsed = true;
    }

    tmpString = this->GetAttribute(L"YPin");
    if (tmpString.IsEmpty() == false)
    {
        yPin = (CATInt32)tmpString;
        yPinUsed = true;
    }


    // Sanity check the pins
    if (xPinUsed)
    {
        if ((xPin > 0) || (width != 0) || (xPos < 0))
        {
            CATString misuseInfo;
            misuseInfo << L"Error in skin XML for control: " << this->GetName();
            result = CATRESULTDESC(CAT_ERR_SKIN_XPIN_MISUSE,misuseInfo);
            xPin = 0;
        }
    }

    if (yPinUsed)
    {
        if ((yPin > 0) || (height != 0) || (yPos < 0))
        {
            CATString misuseInfo;
            misuseInfo << L"Error in skin XML for control: " << this->GetName();
            result = CATRESULTDESC(CAT_ERR_SKIN_YPIN_MISUSE,misuseInfo);
            yPin = 0;
        }
    }


    // Image defaults
    if (this->fImage)
    {      
        if (width == 0)
        {
            width = fImage->Width() / 4;
        }

        if (height == 0)
        {
            height = fImage->Height() / 12;
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
        fRect.top     = xPos;
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


    // Implement sizeable controls later or override for them.
    // Most controls will be satisfied by the above.
    if (fParent)
    {
        ((CATGuiObj*)fParent)->MarkDirty();
    }

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATKnob::Draw(CATImage* image, const CATRect& dirtyRect)
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

    CATInt32 knobNumber = (int)((47 * GetValPercent() ) + 0.5);
    fLastKnob = knobNumber;
    CATInt32 knobCol = knobNumber % 4;
    CATInt32 knobRow = knobNumber / 4;
    CATInt32 knobX = fRect.Width() * knobCol;
    CATInt32 knobY = fRect.Height() * knobRow;


    // Gracefully degrade depending on flags and whether the images are
    // available.
    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;
        if ( (this->IsEnabled() == false) && (this->fImageDisabled))
        {
            if (drawRect.Intersect(CATRect(fRect.left, 
                fRect.top, 
                fRect.left + fImageDisabled->Width()/4,
                fRect.top  + fImageDisabled->Height()/12),
                &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

                image->Overlay(   this->fImageDisabled,
                    drawRect.left, 
                    drawRect.top, 
                    ourRect.left + knobX,
                    ourRect.top + knobY,
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
                    fRect.left + fImagePressed->Width()/4,
                    fRect.top  + fImagePressed->Height()/12),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   this->fImagePressed,
                        drawRect.left, 
                        drawRect.top, 
                        ourRect.left + knobX,
                        ourRect.top + knobY,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }        
            }

            if ((!drawn) && (IsFocused() || IsPressed()) && (this->fImageFocus))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                    fRect.top, 
                    fRect.left + fImageFocus->Width()/4,
                    fRect.top  + fImageFocus->Height()/12),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   this->fImageFocus,
                        drawRect.left, 
                        drawRect.top, 
                        ourRect.left + knobX,
                        ourRect.top + knobY,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }        
            }

            if ((!drawn) && (IsActive()) && (this->fImageActive))
            {
                if (drawRect.Intersect(CATRect(fRect.left, 
                    fRect.top, 
                    fRect.left + fImageActive->Width()/4,
                    fRect.top  + fImageActive->Height()/12),
                    &ourRect))
                {
                    ourRect.Offset(-fRect.left, -fRect.top);

                    image->Overlay(   this->fImageActive,
                        drawRect.left, 
                        drawRect.top, 
                        ourRect.left + knobX,
                        ourRect.top + knobY,
                        ourRect.Width(),
                        ourRect.Height());
                    drawn = true;
                }        
            }

        }

        if ((!drawn) && (this->fImage != 0))
        {
            if (drawRect.Intersect(CATRect(fRect.left, 
                fRect.top, 
                fRect.left + fImage->Width()/4,
                fRect.top  + fImage->Height()/12),
                &ourRect))
            {
                ourRect.Offset(-fRect.left, -fRect.top);

                image->Overlay(   this->fImage,
                    drawRect.left, 
                    drawRect.top, 
                    ourRect.left + knobX,
                    ourRect.top + knobY,
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

/// ParseAttributes() parses the known attributes for an object.
CATResult CATKnob::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    fCommandTrack = GetAttribute(L"CommandTrack",fCommandTrack);

    attrib = GetAttribute(L"ValueType");
    if (attrib.IsEmpty() == false)
    {
        if (attrib.Compare("dB") == 0)
        {
            this->fValueType = CATVALUE_DB;			
        }
        else
        {
            this->fValueType = CATVALUE_LINEAR;
        }		
    }

    attrib = GetAttribute(L"KnobStyle");
    if (!attrib.IsEmpty())
    {
        if (attrib.Compare("V",1) == 0)
        {
            // vertical
            fKnobStyle = CATKNOB_VERTICAL;         
        }
        else if (attrib.Compare("C",1) == 0)
        {
            // circular
            fKnobStyle = CATKNOB_CIRCLE;         
        }      
    }

    fFilterCoef = GetAttribute(L"Filter",fFilterCoef);
    fMinDegree  = GetAttribute(L"MinDegree",fMinDegree);
    fMaxDegree = GetAttribute(L"MaxDegree",fMaxDegree);

    return result;
}

CATKNOB_STYLE CATKnob::GetKnobStyle()
{
    return fKnobStyle;
}

void CATKnob::SetKnobStyle(CATKNOB_STYLE style)
{
    fKnobStyle = style;
    // IF manually set, override cursor 
    switch (fKnobStyle)
    {
    case CATKNOB_CIRCLE:
        fCursor.SetType(CATCURSOR_HAND);
        break;
    case CATKNOB_VERTICAL:         
    default:
        fCursor.SetType(CATCURSOR_TOPBOTTOM);
        break;
    }
}

void CATKnob::SetValueFromPoint(const CATPOINT& point, CATMODKEY modKey)
{
    CATFloat32  curDegree  = (fMaxDegree - fMinDegree) * fValue;
    CATInt32   xCenter    = (fRect.right + fRect.left)/2;
    CATInt32   yCenter    = (fRect.bottom + fRect.top)/2;

    CATInt32 xDist = point.x - xCenter;
    CATInt32 yDist = yCenter - point.y;

    CATFloat32 newDegree   = curDegree;

    if ((xDist == 0) && (yDist == 0))
    {
        // Don't do anything. the mouse is centered.
    }
    else if (xDist == 0)
    {
        // X is 0. y can be pos or neg, but not zero
        if (yDist > 0)
        {
            newDegree = 180;
        }
        else
        {
            newDegree = 0;
        }                  
    }
    else if (xDist > 0)
    {
        // Positive X
        if (yDist == 0)
        {
            newDegree = 270;
        }
        else if (yDist > 0)
        {
            newDegree = 180 + ((CATFloat32)atan2((CATFloat32)xDist,(CATFloat32)yDist) * kCATRADTODEG);
        }
        else
        {
            newDegree = 180 + ((CATFloat32)atan2((CATFloat32)xDist,(CATFloat32)yDist) * kCATRADTODEG);
        }
    }
    else
    {
        // Negative X                  
        if (yDist == 0)
        {
            newDegree = 90;
        }
        else if (yDist > 0)
        {
            newDegree = 180 + ((CATFloat32)atan2((CATFloat32)xDist,(CATFloat32)yDist) * kCATRADTODEG);
        }
        else
        {
            newDegree = 180 + ((CATFloat32)atan2((CATFloat32)xDist,(CATFloat32)yDist) * kCATRADTODEG);
        }
    }

    if (newDegree < fMinDegree)
        newDegree = fMinDegree;

    if (newDegree > fMaxDegree)
        newDegree = fMaxDegree;

    fValue = (CATFloat32)fFilter->Filter(((newDegree - fMinDegree)/(fMaxDegree - fMinDegree)));
}

// PostDraw() draws any stuff that requires an OS-specific draw
// context.
void CATKnob::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{

    switch (fLastKnobStyle)
    {
    case CATKNOB_CIRCLE:            
        CATControl::PostDraw(drawContext,dirtyRect);
        break;

    case CATKNOB_VERTICAL:
    default:
        {  
            CATControl::PostDraw(drawContext,dirtyRect);
            if (this->fPressed)
            {
                CATCursor curs(CATCURSOR_TOPBOTTOM);
                CATPOINT cursP;
                CATRect absRect = this->GetRectAbs();
                cursP.x = (absRect.left + absRect.right)/2 - 5;
                cursP.y = (absRect.top+ absRect.bottom)/2 - 5;
                GetWindow()->OSDrawCursor(drawContext, cursP, curs);
            }
        }
        break;
    }
}

void CATKnob::TrackMouseTimer(CATMODKEY modKey)
{
    if (fKnobStyle == CATKNOB_CIRCLE)
    {
        CATFloat32 tmpVal = fValue;
        SetValueFromPoint(this->fLastMove, modKey);
        if (fabs(tmpVal - fValue) > 0.001)
        {
            CATInt32 knobNumber = (int)((47 * GetValPercent() ) + 0.5);
            if (knobNumber != this->fLastKnob)
            {         
                MarkDirty();
            }

            if (this->fCommandTrack)
            {
                this->OnMouseClick();
                // Maintain pressed state...
                fPressed = true;
            }
        }
    }
}


CATString CATKnob::GetHint() const
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
                hintVal.Format(L" ( %c%.2fdB - %.2f - %.2f)",(dbValue > 0)?'+':'-', fabs(dbValue), CATLinearToDBGain(fValue), fValue );
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
void CATKnob::OnMouseDoubleClick( CATMODKEY modKey)
{
    this->SetValue(fDefValue);
    this->MarkDirty();
}

CATKNOB_STYLE CATKnob::GetActiveStyle( CATMODKEY modKey )
{
    CATKNOB_STYLE curStyle = fKnobStyle;
    // Alt key inverts knob style
    if (modKey & CATMODKEY_ALT)
    {
        switch (fKnobStyle)
        {
        case CATKNOB_CIRCLE:
            curStyle = CATKNOB_VERTICAL;
            break;
        case CATKNOB_VERTICAL:
        default:
            curStyle = CATKNOB_CIRCLE;
            break;
        }
    }
    fLastKnobStyle = curStyle;
    return curStyle;
}

