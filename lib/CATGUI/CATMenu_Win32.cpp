//---------------------------------------------------------------------------
/// \file CATMenu_Win32.cpp
/// \brief Win32-specific calls for Menu/GUI
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
#include "CATMenu.h"
#include "CATWindow.h"
#include <map>
const CATUInt32 kCATMAXMENUHEIGHT = 400;


void CATMenu::ClearMenu()
{
    fIdMap.clear();

    if (fMenuId == 0)
        return;

    // Destroy submenus in reverse order - otherwise boundschecker horks...
    // Most likely, windows automagically deallocates submenus, but not 100% sure, and 
    // BoundsChecker doesn't seem to think so, so....
    HMENU menuId = 0;
    while (CATSUCCEEDED(fMenuStack.Pop(menuId)))
    {
        DestroyMenu(menuId);
    }

    // Clean up top-level menu handle
    DestroyMenu((HMENU)fMenuId);
    fMenuId = 0;
}


void CATMenu::DoMenu()
{
    BuildMenu();

    if (fMenuId == 0)
        return;

    // Let the user select something from the popup menu.
    // On return, it will return the curItem->id we set when appending the menu.
    // Since id == 0 is illegal, we can safely ignore 0 returns.
    CATRect absRect = this->GetRectAbs(true);

    //CATRect wndRect = this->GetWindow()->OSGetWndRect();

    CATInt32 vAlign  = 0;
    CATInt32 left    = absRect.left; //fRect.left + wndRect.left;
    CATInt32 top     = absRect.top;  //fRect.top + wndRect.top;

    POINT cursorPos;
    ::GetCursorPos(&cursorPos);

    switch (fMenuStyle)
    {     
    case CATMENUSTYLE_UPMOUSE:
        vAlign  = TPM_BOTTOMALIGN;
        left    = cursorPos.x;
        top     = cursorPos.y;
        break;

    case CATMENUSTYLE_DOWNMOUSE:
        vAlign  = TPM_TOPALIGN;
        left    = cursorPos.x;
        top     = cursorPos.y;
        break;

    case CATMENUSTYLE_UP:
        vAlign  = TPM_BOTTOMALIGN;
        break;

    case CATMENUSTYLE_DOWN:
    default:
        vAlign  = TPM_TOPALIGN;
        top     = absRect.bottom; // fRect.bottom + wndRect.top;
        break;
    }

    CATInt32 itemId = TrackPopupMenuEx(	(HMENU)fMenuId,
                                        TPM_LEFTALIGN | vAlign| TPM_RETURNCMD | TPM_LEFTBUTTON,
                                        left,
                                        top,
                                        this->GetWindow()->OSGetWnd(),   
                                        0);


    if (itemId == 0)
        return;

    CATMENUITEM* selected = 0;
    std::map<CATUInt32,CATMENUITEM*>::iterator iter = fIdMap.find(itemId);
    if (iter != fIdMap.end())   
    {
        selected = iter->second;
        // IF use alternate command is selected, issue it and return w/o changing the selection
        if (selected->AltCommand.IsEmpty() == false)
        {

            ((CATGuiObj*)fParent)->OnCommand( CATCommand(selected->AltCommand,1.0f),0);
            return;
        }

        this->SetCurItem(selected);
    }
    else
    {
        CATASSERT(false,"Invalid item id in menu!");
    }

    ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(),this);      
}


void CATMenu::CreateSubMenu( std::vector<CATMENUITEM*>& itemList, HMENU parentMenu)
{
    CATUInt32 numItems = itemList.size();
    CATUInt32 i;

    for (i = 0; i < numItems; i++)
    {
        CATMENUITEM* curItem = itemList[i];

        // Create submenus/menu items
        if (curItem->IsSubMenu)
        {
            HMENU subMenu = ::CreatePopupMenu();
            fMenuStack.Push(subMenu);
            CreateSubMenu(curItem->Children, subMenu);				
            // Create a submenu and recurse

            MENUITEMINFO mi;
            mi.cbSize      = sizeof(mi);
            mi.fMask       = MIIM_ID | MIIM_DATA /*| MIIM_STRING*/ | MIIM_SUBMENU | MIIM_TYPE;            
            mi.fType       = MFT_OWNERDRAW; //MFT_STRING;
            mi.dwItemData  = (ULONG_PTR)curItem;            				
            mi.cch         = curItem->DisplayText.Length();
            mi.dwTypeData  = curItem->DisplayText.GetUnicodeBuffer();
            mi.hSubMenu    = subMenu;
            mi.wID         = this->fIdCount;

            // Store mapping of id -> menu item
            this->fIdMap.insert(std::make_pair(fIdCount,curItem));
            fIdCount++;

            ::InsertMenuItem(parentMenu,i,TRUE,&mi);
            curItem->DisplayText.ReleaseBuffer();            
        }
        else
        {
            // Standard menu item - insert it and continue
            MENUITEMINFO mi;
            mi.cbSize      = sizeof(mi);

            // Normal items...
            if (curItem->DisplayText.IsEmpty() == false)
            {
                mi.fMask       = MIIM_ID | MIIM_DATA /*| MIIM_STRING*/ | MIIM_STATE | MIIM_TYPE;
                mi.fState      = (curItem == this->fCurSel)?MFS_CHECKED:MFS_ENABLED;
                mi.fType       = MFT_OWNERDRAW; //MFT_STRING;
                mi.dwItemData  = (ULONG_PTR)curItem;
                mi.cch         = curItem->DisplayText.Length();
                mi.dwTypeData  = curItem->DisplayText.GetUnicodeBuffer();
                mi.wID         = this->fIdCount;
            }
            // Seperators
            else
            {
                mi.fMask       = MIIM_FTYPE | MIIM_DATA;
                mi.fType       = MFT_SEPARATOR | MFT_OWNERDRAW;
                mi.dwItemData  = (ULONG_PTR)curItem;

            }

            // Store mapping of id -> menu item
            this->fIdMap.insert(std::make_pair(fIdCount,curItem));
            fIdCount++;

            ::InsertMenuItem(parentMenu,i,TRUE,&mi);
            curItem->DisplayText.ReleaseBuffer();                           
        }
    }
}

void CATMenu::BuildMenu()
{      
    ClearMenu();   
    fMenuId = (CATUInt32)::CreatePopupMenu();   
    CreateSubMenu(this->fRootList, (HMENU)fMenuId); 
    fMenuDirty = false;
}

void CATMenu::OSOnMeasureItem(CATMENUITEM* menuItem, CATUInt32& width, CATUInt32& height)
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
    CATString filtered = FilterGUIString(menuItem->DisplayText);

    if (filtered.IsEmpty() == false)
    {
        ::GetTextExtentExPoint( curDC, 
            filtered, 
            filtered.Length(),
            this->GetWindow()->GetRect().Width(),
            NULL,
            NULL,
            &textSize);

    }

    height = CATMax((CATUInt32)(textSize.cy + 2), (CATUInt32)10 );
    width  = textSize.cx + 25;

    // Restore fonts/cleanup
    ::SelectObject(curDC,oldFont);
    ::ReleaseDC(hwnd,curDC);
    this->GetWindow()->OSReleaseFont(measureFont);

    if (menuItem->BaseMenu->ForceWidth())
    {
        CATRect controlRect = menuItem->BaseMenu->GetRect();
        // Force the width of the menu to be the same as the control.
        // Windows seems to add about 1.5 chars for '<' caret and space.
        width  = (CATUInt32)(controlRect.Width() - (textSize.cx/filtered.Length())*1.5);
    }
}

void CATMenu::OSOnDrawItem( CATMENUITEM* menuItem, bool selected, CATDRAWCONTEXT hDC, CATRect rect )
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

    // Draw selection dot.
    if ((menuItem == fCurSel) && (fShowSel))
    {                           
        CATInt32 startX = drawRect.left + 2;
        CATInt32 startY = ((drawRect.top + drawRect.bottom)/2);

        HPEN fgPen = ::CreatePen( PS_SOLID,2,RGB(colorFore.r, colorFore.g, colorFore.b));
        HPEN oldPen = (HPEN)::SelectObject(hDC, fgPen);

        ::MoveToEx( hDC, 
            startX,
            startY - 3,
            0);

        ::LineTo(   hDC,
            startX + 5,
            startY );

        ::MoveToEx( hDC, 
            startX,
            startY + 3,
            0);

        ::LineTo(   hDC,
            startX + 5,
            startY );

        ::SelectObject(hDC, oldPen);
        ::DeleteObject(fgPen);
    }



    if (menuItem->IsSubMenu)
    {
        CATInt32 startX = drawRect.right - 8;
        CATInt32 startY = ((drawRect.top + drawRect.bottom)/2);

        HPEN fgPen = ::CreatePen( PS_SOLID,2,RGB(colorFore.r, colorFore.g, colorFore.b));
        HPEN oldPen = (HPEN)::SelectObject(hDC, fgPen);

        ::MoveToEx( hDC, 
            startX,
            startY - 3,
            0);

        ::LineTo(   hDC,
            startX + 5,
            startY );

        ::MoveToEx( hDC, 
            startX,
            startY + 3,
            0);

        ::LineTo(   hDC,
            startX + 5,
            startY );


        ::SelectObject(hDC, oldPen);
        ::DeleteObject(fgPen);
    }

    int oldMode = ::SetBkMode(hDC,TRANSPARENT);
    COLORREF oldColor = ::SetTextColor(hDC,RGB(colorFore.r,colorFore.g,colorFore.b));

    if (menuItem->DisplayText.IsEmpty())
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


        CATString filtered = FilterGUIString(menuItem->DisplayText);
        ::DrawTextEx( hDC, 
            filtered.GetUnicodeBuffer(), 
            filtered.LengthCalc(),
            &rcText,textStyle,NULL);
        filtered.ReleaseBuffer();
        ::SelectObject(hDC,oldFont);
        this->GetWindow()->OSReleaseFont(drawFont);
    }




    // Restore fonts/cleanup
    ::SetTextColor(hDC,oldColor);
    ::SetBkMode(hDC, oldMode);

    ::DeleteObject(bgBrush);
    ::ExcludeClipRect(hDC,drawRect.left,drawRect.top,drawRect.right, drawRect.bottom);
}