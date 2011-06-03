//---------------------------------------------------------------------------
/// \file CATTreeCtrl_Win32.cpp
/// \brief List box for GUI (win32 implementation)
/// \ingroup CATGUI
///
/// Copyright (c) 2004-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $
//
//
#include "CATTreeCtrl.h"
#include "CATWindow.h"
#include "CATEventDefs.h"
#include "CATApp.h"

CATTreeCtrl::CATTreeCtrl(  const CATString&             element, 
                         const CATString&             rootDir)
                         : CATControlWnd(element,  rootDir)
{
    this->fValue = 1.0f;
    this->fCurSel = 0;
    fFont = 0;   
    fImageList = 0;

    this->fWindowStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP; //TVS_SINGLEEXPAND
    this->fWindowType = WC_TREEVIEW;

    // Initialize common win32 controls
    INITCOMMONCONTROLSEX initStruct;
    initStruct.dwICC = ICC_TREEVIEW_CLASSES;
    initStruct.dwSize = sizeof(initStruct);
    ::InitCommonControlsEx(&initStruct);
    fRespondSelect = true;
}

void CATTreeCtrl::OnParentCreate()
{   
    CATControlWnd::OnParentCreate();
    TreeView_SetBkColor  (fControlWnd, RGB(this->fBackgroundColor.r, this->fBackgroundColor.g, this->fBackgroundColor.b));
    TreeView_SetTextColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));
    TreeView_SetLineColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));
    TreeView_SetInsertMarkColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));

    fFont = GetWindow()->OSGetFont(fFontName,fFontSize);   
    ::SendMessage(fControlWnd,WM_SETFONT,(WPARAM)fFont,TRUE);         

    OSClearTree();
    OSRebuildTree(&fRootList);
    ExpandRoot();
}

void CATTreeCtrl::ExpandRoot()
{
    if (fRootList.size() > 0)
    {
        CATTREEINFO* firstItem = fRootList[0];
        this->ExpandItem(firstItem,true);
    }
}

void CATTreeCtrl::OnParentDestroy()
{
    CATControlWnd::OnParentDestroy();

    if (fFont != 0)
    {
        GetWindow()->OSReleaseFont(fFont);
        fFont = 0;
    }
}

void CATTreeCtrl::OSRebuildTree(std::vector<CATTREEINFO*>* curList)
{
    if (curList == 0)
        return;

    fRespondSelect = false;

    CATUInt32 numItems = curList->size();
    for (CATUInt32 i = 0; i < numItems; i++)
    {
        CATTREEINFO* curInfo = (*curList)[i];
        OSAddItem(curInfo->DisplayText, curInfo, i);

        if (curInfo->Children.size() > 0)
        {
            OSRebuildTree(&curInfo->Children);
        }
    }

    fRespondSelect = true;
}

void CATTreeCtrl::OSClearTree()
{
    TreeView_DeleteAllItems(fControlWnd);
}

CATResult CATTreeCtrl::OSAddItem( const CATString& displayText, CATTREEINFO* listInfo, CATUInt32 index)
{
    TVINSERTSTRUCT tvi;
    memset(&tvi,0,sizeof(tvi));

    if (listInfo->Parent)
    {
        tvi.hParent = (HTREEITEM)listInfo->Parent->ItemHandle;  
    }
    else
    {
        tvi.hParent = 0;      
    }

    if (index == -1)
    {
        tvi.hInsertAfter = TVI_LAST;
    }
    else if (index == 0)
    {
        tvi.hInsertAfter = TVI_FIRST;
    }
    else if (listInfo->Parent == 0)
    {
        tvi.hInsertAfter = TVI_ROOT;      
    }
    else
    {
        CATTREEINFO* prevItem = listInfo->Parent->Children[index-1];
        tvi.hInsertAfter = (HTREEITEM)prevItem->ItemHandle;
    }
    CATString tmpDisplay = displayText;

    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = tmpDisplay.GetUnicodeBuffer();
    tvi.item.lParam = (long)listInfo;

    if ((listInfo->ImageNumber != -1) && (fImageList != 0))
    {
        tvi.item.iImage = listInfo->ImageNumber;
        tvi.item.mask |= TVIF_IMAGE;
    }

    listInfo->ItemHandle = (long)TreeView_InsertItem(fControlWnd,&tvi);
    tmpDisplay.ReleaseBuffer();

    if (listInfo->ItemHandle == 0)
    {
        CATASSERT(false,"Unable to insert item into tree!");
        return CATRESULT(CAT_ERR_TREE_INSERT_ERROR);
    }
    return CAT_SUCCESS;
}

CATResult CATTreeCtrl::OSRemoveItem( CATTREEINFO* treeItem )
{
    if (TreeView_DeleteItem(fControlWnd, treeItem->ItemHandle))
    {
        return CAT_SUCCESS;
    }
    return CATRESULT(CAT_ERR_TREE_REMOVE_ERROR);
}

CATTREEINFO* CATTreeCtrl::OSGetCurSel()
{
    HTREEITEM itemHndl = TreeView_GetSelection(fControlWnd);
    if (itemHndl == 0)
    {
        return 0;
    }
    TVITEM item;
    memset(&item,0,sizeof(item));
    item.hItem = itemHndl;
    item.mask = TVIF_PARAM;

    if (TreeView_GetItem(fControlWnd,&item))
    {
        CATTREEINFO* treeInfo = (CATTREEINFO*)item.lParam;
        return treeInfo;
    }

    CATASSERT(false,"Get selection failed, but something seems to be selected!");
    return 0;
}

CATResult CATTreeCtrl::OSSetCurSel( CATTREEINFO* newSel )
{
    this->fRespondSelect = false;

    if (newSel != 0)   
    {
        TreeView_SelectItem(fControlWnd, newSel->ItemHandle);
    }
    else
    {
        TreeView_SelectItem(fControlWnd, 0);
    }

    this->fRespondSelect = true;
    return CAT_SUCCESS;
}

CATResult CATTreeCtrl::OSUpdateText( CATTREEINFO* treeItem)
{
    if (!treeItem)
    {
        return CATRESULT(CAT_ERR_INVALID_PARAM);
    }


    TVITEM tvItem;
    memset(&tvItem,0,sizeof(tvItem));
    CATString tmpText = treeItem->DisplayText;
    tvItem.mask    = TVIF_TEXT;
    tvItem.hItem   = (HTREEITEM)treeItem->ItemHandle;
    tvItem.cchTextMax = tmpText.Length();
    tvItem.pszText = tmpText.GetUnicodeBuffer();

    if (TreeView_SetItem( fControlWnd, &tvItem))
    {
        tmpText.ReleaseBuffer();
        return CAT_SUCCESS;
    }
    tmpText.ReleaseBuffer();
    return CATRESULT(CAT_ERR_TREE_SET_ITEM_ERROR);
}

// OS-specific event handling from window
CATResult CATTreeCtrl::OSEvent( const CATEvent& event, CATInt32& retVal)
{
    // Hmm... this could be boiled down to a lot less levels, but
    // we may want to look at other events in a bit... leave for now,
    // fix levels later if unused. - MAEDEBUG   
    switch (event.fEventCode)
    {      
    case CATEVENT_ENABLE_CHANGE:
        {
#ifdef _WIN32
            if (fControlWnd)
            {
                ::EnableWindow(fControlWnd, this->IsEnabled()?TRUE:FALSE);
                OSUpdateTreeColors();
            }
#endif
        }   
        break;
    case CATEVENT_TAB_SHOW:
        {
            if ((fControlWnd) && fVisible)
            {
                ::ShowWindow(fControlWnd,SW_SHOW);					              
                this->OSUpdateTreeColors();
                this->MarkDirty();
            }
        }
        break;

    case CATEVENT_TAB_HIDE:
        if ((fControlWnd) && fVisible)
        {
            ::ShowWindow(fControlWnd,SW_HIDE);
            this->OSUpdateTreeColors();
            this->MarkDirty();
        }               
        break;
    case CATEVENT_WINDOWS_EVENT:
        switch (event.fIntParam2)
        {
        case WM_NOTIFY:
            {
                LPNMHDR notifyHdr = (LPNMHDR)event.fIntParam4;
                if (notifyHdr->hwndFrom == this->fControlWnd)
                {
                    switch (notifyHdr->code)
                    {
                    case NM_CUSTOMDRAW:
                        {
                            NMTVCUSTOMDRAW* custDraw = (NMTVCUSTOMDRAW*)event.fIntParam4;
                            switch (custDraw->nmcd.dwDrawStage)
                            {
                            case CDDS_PREPAINT:
                                retVal = CDRF_NOTIFYITEMDRAW;
                                break;
                            case CDDS_ITEMPREPAINT:
                                retVal = CDRF_NEWFONT;
                                if (custDraw->nmcd.uItemState & CDIS_SELECTED)
                                {
                                    custDraw->clrText		= RGB(this->fBackgroundColor.r, this->fBackgroundColor.g, this->fBackgroundColor.b);
                                    custDraw->clrTextBk  = RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b);
                                }
                                break;
                            }
                        }
                        break;

                    case TVN_SELCHANGED:
                        {                           
                            if (fRespondSelect)
                            {
                                NMTREEVIEW* tView = (NMTREEVIEW*)event.fIntParam4;
                                CATTREEINFO* info = (CATTREEINFO*)tView->itemNew.lParam;

                                if (info != 0)
                                {
                                    this->fCurSel = info;
                                    this->SetString(info->DisplayText);

                                    this->fRespondSelect = false;

                                    ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);

                                    this->fRespondSelect = true;

                                    return CAT_SUCCESS;
                                }                              
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
        break;
    }   

    return CATControlWnd::OnEvent(event,retVal);
}

CATResult CATTreeCtrl::ExpandItem(CATTREEINFO* item, bool expand)
{
    if (item == 0)
    {
        return CATRESULT(CAT_ERR_INVALID_PARAM);
    }
    ::ShowWindow(fControlWnd,SW_HIDE);
    TreeView_Expand(fControlWnd, item->ItemHandle, expand?TVE_EXPAND:TVE_COLLAPSE);
    ::ShowWindow(fControlWnd,IsVisible()?SW_SHOW:SW_HIDE);

    return CAT_SUCCESS;   
}

void CATTreeCtrl::SetEnabled(bool enabled)
{   
    CATControlWnd::SetEnabled(enabled);
    OSUpdateTreeColors();
}

void CATTreeCtrl::OSUpdateTreeColors()
{
    if (this->IsEnabled())
    {
        TreeView_SetBkColor  (fControlWnd, RGB(this->fBackgroundColor.r, this->fBackgroundColor.g, this->fBackgroundColor.b));
        TreeView_SetTextColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));
        TreeView_SetLineColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));
        TreeView_SetInsertMarkColor(fControlWnd, RGB(this->fForegroundColor.r, this->fForegroundColor.g, this->fForegroundColor.b));
    }
    else
    {      
        TreeView_SetBkColor  (fControlWnd, RGB(this->fBgDisColor.r, this->fBgDisColor.g, this->fBgDisColor.b));
        TreeView_SetTextColor(fControlWnd, RGB(this->fFgDisColor.r, this->fFgDisColor.g, this->fFgDisColor.b));
        TreeView_SetLineColor(fControlWnd, RGB(this->fFgDisColor.r, this->fFgDisColor.g, this->fFgDisColor.b));
        TreeView_SetInsertMarkColor(fControlWnd, RGB(this->fFgDisColor.r, this->fFgDisColor.g, this->fFgDisColor.b));
    }  
}

CATResult CATTreeCtrl::UseImageList(CATUInt32 imageListId, CATUInt32 width, const CATColor& transparent)
{
    if (fImageList)
    {
        TreeView_SetImageList(fControlWnd, 0,TVSIL_NORMAL);
        ImageList_Destroy(fImageList);
        fImageList = 0;
    }

    if (imageListId == 0)
    {
        return CAT_SUCCESS;
    }

    const int kMaxNumImageListItems = 16;

    fImageList = ImageList_LoadBitmap(	gApp->GetInstance(), 
        MAKEINTRESOURCE(imageListId), 
        width,
        kMaxNumImageListItems,
        RGB(transparent.r, transparent.g, transparent.b));

    if (fImageList != 0)
    {
        TreeView_SetImageList(fControlWnd, fImageList, TVSIL_NORMAL);
        return CAT_SUCCESS;
    }

    return CATRESULT(CAT_ERR_IMAGELIST_FAILED);
}

