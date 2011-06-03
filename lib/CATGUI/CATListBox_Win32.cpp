//---------------------------------------------------------------------------
/// \file GGListBox_Win32.cpp
/// \brief List box for GUI (win32 implementation)
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
#include "CATListBox.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

CATListBox::CATListBox(  const CATString&             element, 
                       const CATString&             rootDir)
                       : CATControlWnd(element,  rootDir)
{
    fCurSel = -1;
    fValue  = -1;
    fMinValue = -1;
    fMaxValue = -1;
    fFont   = 0;

    this->fWindowStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_DISABLENOSCROLL | WS_VSCROLL | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED;
    this->fWindowType = "LISTBOX";
}


bool CATListBox::OnControlEvent( const CATEvent& event,  CATInt32& result)                                       
{  
    return false;
}

CATResult CATListBox::OSEvent( const CATEvent& event, CATInt32& retVal)
{
    // Hmm... this could be boiled down to a lot less levels, but
    // we may want to look at other events in a bit... leave for now,
    // fix levels later if unused. - MAEDEBUG
    switch (event.fEventCode)
    {      
    case CATEVENT_WINDOWS_EVENT:
        switch (event.fIntParam2)
        {
        case WM_COMMAND:
            if (event.fIntParam4 == (CATInt32)this->fControlWnd)
            {
                switch (HIWORD((CATUInt32)event.fIntParam3))
                {
                case LBN_SELCHANGE:
                    {
                        int item = ::SendMessage(fControlWnd,LB_GETCURSEL,0,0);
                        if (item >= 0)
                        {
                            CATLISTINFO* listInfo = 0;
                            listInfo = (CATLISTINFO*)::SendMessage(fControlWnd,LB_GETITEMDATA,item,0);
                            if (listInfo != 0)
                            {
                                this->fCurSel = item;
                                this->fValue = (CATFloat32)fCurSel;                                 
                                ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);
                                return CAT_SUCCESS;
                            }                              
                        }
                    }
                    break;
                }
            }
            break;
        }
        break;
    }   

    return CATControlWnd::OnEvent(event,retVal);
}

void CATListBox::OnParentCreate()
{
    CATControlWnd::OnParentCreate();

    this->SetCurSel(-1);

    fFont = GetWindow()->OSGetFont(fFontName,fFontSize);   
    ::SendMessage(this->fControlWnd, WM_SETFONT,(WPARAM)fFont,0);
}


void CATListBox::OnParentDestroy()
{
    if (fFont != 0)
    {
        GetWindow()->OSReleaseFont(fFont);
    }
    CATControlWnd::OnParentDestroy();
}


void CATListBox::OSClearList()
{
    SendMessage(fControlWnd, LB_RESETCONTENT,0,0);
}

CATResult CATListBox::OSAddItem( CATInt32 index, const CATString& displayText, const CATLISTINFO* listInfo)
{
    CATInt32 item = SendMessage(fControlWnd,LB_ADDSTRING,0,(LONG)(const char*)displayText);
    if (item < 0)
    {
        return false;
    }

    SendMessage(fControlWnd, LB_SETITEMDATA, item, (LONG)listInfo);
    return true;

}

CATResult CATListBox::OSRemoveItem( CATInt32 index )
{
    SendMessage(fControlWnd,LB_DELETESTRING, index, 0);   
    return CAT_SUCCESS;
}

CATInt32 CATListBox::OSGetCurSel()
{
    CATInt32 item = SendMessage(fControlWnd,LB_GETCURSEL,0,0);
    return item;
}

CATResult CATListBox::OSSetCurSel( CATInt32 index )
{
    SendMessage(fControlWnd,LB_SETCURSEL,index,0);
    return CAT_SUCCESS;
}

void CATListBox::OSOnMeasureItem(CATLISTINFO* listItem, CATUInt32& width, CATUInt32& height)
{

    // Setup fonts
    CATFONT measureFont = this->GetWindow()->OSGetFont(fFontName,fFontSize);
    HWND hwnd = this->GetWindow()->OSGetWnd();
    HDC curDC = ::GetDC(hwnd);
    CATFONT oldFont = (HFONT)::SelectObject(curDC,measureFont);

    // Calc size of text
    SIZE textSize;
    textSize.cx = 0;
    textSize.cy = 0;
    CATString filtered = FilterGUIString(listItem->DisplayText);

    if (filtered.IsEmpty() == false)
    {
        ::GetTextExtentExPoint( curDC, 
            filtered.GetUnicodeBuffer(), 
            filtered.LengthCalc(),
            this->GetWindow()->GetRect().Width(),
            NULL,
            NULL,
            &textSize);

        filtered.ReleaseBuffer();
    }

    height = CATMax((CATUInt32)(textSize.cy + 2), (CATUInt32)10 );
    width  = textSize.cx + 25;

    // Restore fonts/cleanup
    ::SelectObject(curDC,oldFont);
    ::ReleaseDC(hwnd,curDC);
    this->GetWindow()->OSReleaseFont(measureFont);
}

void CATListBox::OSOnDrawItem( CATLISTINFO* listItem, bool selected, CATDRAWCONTEXT hDC, CATRect rect )
{                        
    CATColor  colorFore = GetColorFore();
    CATColor  colorBack = GetColorBack();

    RECT drawRect;
    ::SetRect(&drawRect, rect.left, rect.top,rect.right,rect.bottom);

    if (selected & ODS_SELECTED)
    {
        CATSwap(colorFore,colorBack);
    }

    HBRUSH bgBrush = ::CreateSolidBrush(RGB(colorBack.r, colorBack.g, colorBack.b));
    ::FillRect(hDC, &drawRect, bgBrush);

    int oldMode = ::SetBkMode(hDC,TRANSPARENT);
    COLORREF oldColor = ::SetTextColor(hDC,RGB(colorFore.r,colorFore.g,colorFore.b));

    if (listItem->DisplayText.IsEmpty())
    {
        // Just do a line - no text there.
        CATInt32 yPos    = (drawRect.top + drawRect.bottom)/2;
        HPEN linePen   = ::CreatePen(PS_SOLID,1,RGB(colorFore.r, colorFore.g, colorFore.b));
        HPEN oldPen    = (HPEN)::SelectObject(hDC, linePen);

        ::MoveToEx  (hDC, drawRect.left  + 2, yPos, NULL);
        ::LineTo    (hDC, drawRect.right - 2, yPos);

        ::SelectObject(hDC,oldPen);
        ::DeleteObject(oldPen);

    }
    else
    {
        // Setup fonts      
        CATFONT drawFont = this->GetWindow()->OSGetFont(fFontName,fFontSize);
        CATFONT oldFont = (HFONT)::SelectObject(hDC,drawFont);

        // Draw
        DWORD textStyle = DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE;

        RECT rcText;
        ::SetRect(&rcText, drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
        rcText.left += 10;


        CATString filtered = FilterGUIString(listItem->DisplayText);
        ::DrawTextEx( hDC, 
            filtered.GetUnicodeBuffer(), 
            -1,
            &rcText,textStyle,NULL);        
        ::SelectObject(hDC,oldFont);
        this->GetWindow()->OSReleaseFont(drawFont);
    }




    // Restore fonts/cleanup
    ::SetTextColor(hDC,oldColor);
    ::SetBkMode(hDC, oldMode);

    ::DeleteObject(bgBrush);

}