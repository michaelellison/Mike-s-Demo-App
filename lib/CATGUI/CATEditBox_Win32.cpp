//---------------------------------------------------------------------------
/// \file CATEditBox_Win32.cpp
/// \brief Text edit box for GUI (win32 implementation)
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
#include "CATEditBox.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

CATEditBox::CATEditBox(  const CATString&             element, 
                       const CATString&             rootDir)
                       : CATControlWnd(element,  rootDir)
{
    this->fWindowStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL;
    this->fWindowType = "EDIT";
    fReadOnly = false;
    fNumbersOnly = false;
    fFont = 0;   
    fMaxLength = kDefEditMaxLength;
}

CATEditBox::~CATEditBox()
{   
}

bool CATEditBox::OnControlEvent( const CATEvent& event,  CATInt32& result)                                       
{   
    switch (event.fEventCode)
    {      
    case CATEVENT_WINDOWS_EVENT:
        switch (event.fIntParam2)
        {
        case WM_SETFOCUS:
            if (event.fIntParam1 == (CATInt32)this->fControlWnd)
            {                  
                CATInt32 retVal = 0;
                // allow parent to handle event if it wishes
                this->GetWindow()->OnEvent(
                    CATEvent(	CATEVENT_ON_EDITBOX_SELECT,
                    0,0,0,0,
                    this->fValue,
                    this->GetString(),
                    this->fCmdString,
                    this->fCmdParam,															
                    this), retVal);

                if (retVal == 0)
                {							
                    this->OSSelectAll();
                }
                else
                {
                    this->GetWindow()->KillFocus();
                    return true;
                }
                result = 0;
                return false;
            }
            break;
        }
        break;
    }


    return false;
}


void CATEditBox::OnParentCreate()
{
    if (this->fMultiline == true)
    {
        fWindowStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_MULTILINE  | WS_VSCROLL | ES_AUTOVSCROLL;
        fShowHintValue = false;      
    }
    else
    {
        fWindowStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL;
    }

    if (this->fReadOnly == true)
    {
        fWindowStyle |= ES_READONLY;
    }

    if (this->fNumbersOnly)
    {
        fWindowStyle |= ES_NUMBER;
    }

    CATControlWnd::OnParentCreate();

    this->OSSetText(fCmdParam);

    fFont = GetWindow()->OSGetFont(fFontName,fFontSize);   
    ::SendMessage(fControlWnd,WM_SETFONT,(WPARAM)fFont,TRUE);         
    ::SendMessage(fControlWnd,EM_LIMITTEXT, (WPARAM)fMaxLength,0);
}

void CATEditBox::OnParentDestroy()
{
    if (fFont != 0)
    {
        GetWindow()->OSReleaseFont(fFont);
    }

    CATControlWnd::OnParentDestroy();
}

void CATEditBox::OSSelectAll()
{
    ::PostMessage(fControlWnd, EM_SETSEL,0,-1);
}

void CATEditBox::OSSetReadOnly(bool readOnly)
{		
    SendMessage(fControlWnd, EM_SETREADONLY, readOnly?TRUE:FALSE,0);
}