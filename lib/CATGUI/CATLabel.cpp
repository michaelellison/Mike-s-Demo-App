//---------------------------------------------------------------------------
/// \file CATLabel.cpp
/// \brief Text label for GUI
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
#include "CATLabel.h"
#include "CATWindow.h"
#include "CATEventDefs.h"
//---------------------------------------------------------------------------
// CATLabel constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Label")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png load
//---------------------------------------------------------------------------
CATLabel::CATLabel(    const CATString&             element, 
                       const CATString&             rootDir)
: CATControl(element,  rootDir)
{
    fAllowClick = false;
}
//---------------------------------------------------------------------------
// CATLabel destructor
//---------------------------------------------------------------------------
CATLabel::~CATLabel()
{


}

/// IsFocusable() returns true if the control can receive
/// focus, and false otherwise.
bool CATLabel::IsFocusable() const
{
    return fAllowClick;
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATLabel::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    fAllowClick = GetAttribute(L"AllowClick",fAllowClick);

    return result;
}

//---------------------------------------------------------------------------
// Draw() draws the control into the parent window
// \param dirtyRect - portion of control (in window coordinates)
//        that requires redrawing.
//---------------------------------------------------------------------------
void CATLabel::Draw(CATImage* image, const CATRect& dirtyRect)
{
    // post draw only for label
}




void CATLabel::SetString ( const CATString& text )
{
    if (text != fText)
    {
        fText = text;
        this->MarkDirty();
    }
}
CATString CATLabel::GetString () const
{
    return fText;
}

CATResult CATLabel::OnEvent(const CATEvent& event, CATInt32& retVal)
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
                this->SetString(event.fStringParam3);
                this->SetValue(event.fFloatParam1, false);
                retVal++;
            }
        }
        break;
    default:
        return CATControl::OnEvent(event,retVal);
    }
    return CAT_SUCCESS;
}