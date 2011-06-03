//---------------------------------------------------------------------------
/// \file CATEditBox.cpp
/// \brief Text edit box for GUI
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
#include "CATEditBox.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

/// ParseAttributes() parses the known attributes for an object.
CATResult CATEditBox::ParseAttributes()
{
    CATResult result = CATControlWnd::ParseAttributes();
    CATString attrib;

    fReadOnly    = GetAttribute(L"ReadOnly", fReadOnly);
    fNumbersOnly = GetAttribute(L"NumbersOnly", fNumbersOnly);
    fMaxLength   = GetAttribute(L"MaxLength", fMaxLength);

    if (fMaxLength > 0x7FFE)
        fMaxLength = 0x7FFE;

    return result;
}

void CATEditBox::SetFocused(bool focused)
{
    if ((focused == false) && (this->fFocused))
    {
        CATString oldParam = fCmdParam;
        // Loosing focus... store parameter.
        this->OSGetText(fCmdParam);

        if (oldParam.Compare(fCmdParam) != 0)
        {
            ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);
        }
    }
    else if ((focused) && (!fFocused))
    {      
        this->OSSelectAll();
    }

    CATControlWnd::SetFocused(focused);
}

void CATEditBox::OnEscapeChange()
{
    SetString(this->fCmdParam);
    this->MarkDirty();
}

void CATEditBox::SetString(const CATString& newString)
{
    this->fCmdParam = newString;
    this->fValue    = (CATFloat32)newString;
    this->OSSetText(fCmdParam);
    this->MarkDirty();
}

CATString CATEditBox::GetString() const
{
    return this->fCmdParam;
}

CATResult CATEditBox::OnEvent(const CATEvent& event, CATInt32& retVal)
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
        return CATControlWnd::OnEvent(event,retVal);
    }
    return CAT_SUCCESS;
}

CATFloat32 CATEditBox::GetValue() const
{
    return (CATFloat32)this->GetString();
}

CATString CATEditBox::GetHint() const
{
    CATString retString;
    retString = CATControlWnd::GetHint();

    if (fShowHintValue)
    {
        retString << " (" << this->GetString() << " )";
    }
    return retString;
}

bool CATEditBox::IsReadOnly()
{
    return fReadOnly;
}

void CATEditBox::SetReadOnly(bool readOnly)
{
    OSSetReadOnly(readOnly);
    fReadOnly = readOnly;
}

