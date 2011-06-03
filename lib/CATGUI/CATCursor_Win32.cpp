//---------------------------------------------------------------------------
/// \file CATCursor_Win32.cpp
/// \brief Cursor class for GUI (win32)
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
#include "CATInternal.h"
#include "CATCursor.h"

CATCursor::CATCursor(CATCURSORTYPE type)
{
    fHidden = false;
    this->fType = type;
}

CATCursor::~CATCursor()
{
}

void CATCursor::SetType(CATCURSORTYPE type)
{
    this->fType = type;
}

CATCURSORTYPE CATCursor::GetType()
{
    return fType;
}

CATOSCURSOR CATCursor::GetOSCursor()
{
    CATWChar* winCursor = 0;

    // Select win32 cursor that matches our cursor
    switch (fType)
    {
    case CATCURSOR_NOACTION:    winCursor = IDC_NO;           break;
    case CATCURSOR_WAIT:        winCursor = IDC_WAIT;         break;
    case CATCURSOR_TEXT:        winCursor = IDC_IBEAM;        break;
    case CATCURSOR_HAND:        winCursor = IDC_HAND;         break;
    case CATCURSOR_LEFTRIGHT:   winCursor = IDC_SIZEWE;       break;
    case CATCURSOR_TOPBOTTOM:   winCursor = IDC_SIZENS;       break;
    case CATCURSOR_SIZE:        winCursor = IDC_SIZENWSE;     break;   
    case CATCURSOR_MOVE:        winCursor = IDC_SIZEALL;      break;      

    default:
    case CATCURSOR_ARROW:       winCursor = IDC_ARROW;
    }

    // Loaded cursors do not need to be tracked or deleted under windows when LoadCursor is used.
    return ::LoadCursor(NULL,winCursor);
}

void CATCursor::HideCursor()
{
    if (!fHidden)
    {
        ::ShowCursor(FALSE);
        fHidden = true;
        CATTRACE("Cursor hidden");
    }
}

void CATCursor::ShowCursor()
{
    if (fHidden)
    {
        ::ShowCursor(TRUE);
        fHidden = false;
        CATTRACE("Cursor shown.");
    }
}

