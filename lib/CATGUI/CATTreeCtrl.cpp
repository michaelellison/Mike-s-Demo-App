//---------------------------------------------------------------------------
/// \file CATTreeCtrl.cpp
/// \brief List box for GUI
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
//
//---------------------------------------------------------------------------
#include "CATTreeCtrl.h"
#include "CATWindow.h"
#include "CATEvent.h"

#include "CATEventDefs.h"

CATTreeCtrl::~CATTreeCtrl()
{
    Clear();
}


void CATTreeCtrl::SetFocused(bool focused)
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

    CATControlWnd::SetFocused(focused);
}

void CATTreeCtrl::OnEscapeChange()
{
    this->MarkDirty();
}

CATTREEINFO* CATTreeCtrl::GetCurItem()
{
    return fCurSel;
}

CATUInt32  CATTreeCtrl::GetNumRootItems()
{
    return this->fRootList.size();
}

CATTREEINFO* CATTreeCtrl::GetRootItem(CATUInt32 index)
{
    if (index >= fRootList.size())
        return 0;
    return fRootList[index];
}

CATResult CATTreeCtrl::SetCurItem(CATTREEINFO* newSel, bool sendCommand)
{
    fCurSel = newSel;
    this->OSSetCurSel(newSel);

    if (sendCommand)
    {
        fRespondSelect = false;
        ((CATGuiObj*)fParent)->OnCommand(this->GetCommand(), this);
        fRespondSelect = true;
    }

    return CAT_SUCCESS;
}

CATTREEINFO* CATTreeCtrl::GetItemFromDataPtr( void* dataPtr, CATTREEINFO* rootItem)
{
    CATTREEINFO* curItem = 0;
    if (rootItem == 0)
    {
        for (CATUInt32 i = 0; i < fRootList.size(); i++)
        {
            curItem = fRootList[i];
            if (dataPtr == curItem->DataPtr)
            {
                return curItem;
            }

            if (0 != (curItem = GetItemFromDataPtr(dataPtr,curItem)))
            {
                return curItem;
            }
        }
    }
    else
    {
        for (CATUInt32 i = 0; i < rootItem->Children.size(); i++)
        {
            curItem = rootItem->Children[i];
            if (dataPtr == curItem->DataPtr)
            {
                return curItem;
            }

            if (0 != (curItem = GetItemFromDataPtr(dataPtr,curItem)))
            {
                return curItem;
            }
        }
    }


    return 0;
}

CATResult CATTreeCtrl::Insert( const CATString&  displayText,
                              CATTREEINFO*      parent,
                              CATTREEINFO*&     newInfo,
                              void*            dataPtr,
                              CATUInt32          index,
                              CATUInt32				imageNumber)
{
    CATResult result = CAT_SUCCESS;

    newInfo  = new CATTREEINFO;
    newInfo->DataPtr     = dataPtr;
    newInfo->DisplayText = displayText;
    newInfo->ItemHandle  = 0;
    newInfo->Parent      = parent;
    newInfo->ImageNumber = imageNumber;

    if (parent == 0)
    {
        if (index == -1)
        {
            index = fRootList.size();
        }
        this->fRootList.insert(fRootList.begin()+index,newInfo);
    }
    else
    {
        if (index == -1)
        {
            index = parent->Children.size();
        }
        parent->Children.insert(parent->Children.begin() + index, newInfo);
    }

    // May be building prior to creation
    if (fControlWnd == 0)
    {
        return CAT_SUCCESS;
    }
    return OSAddItem(displayText,newInfo,index);   
}

CATResult CATTreeCtrl::Remove( CATTREEINFO*&     item)
{
    if (item == 0)
    {
        CATASSERT(item != 0, "Invalid item remove requested.");
        return CATRESULT(CAT_ERR_INVALID_PARAM);
    }

    // Remove from gui first...
    CATResult result = CAT_SUCCESS;
    result = OSRemoveItem(item);
    if (CATFAILED(result))
    {
        return result;
    }

    CATTREEINFO* curItem = 0;
    CATUInt32 numChildren = 0;

    // Now remove and delete
    if (item->Parent == 0)
    {
        std::vector<CATTREEINFO*>::iterator iter = fRootList.begin();
        while (iter != fRootList.end())
        {
            if ( (*iter) == item)
            {
                fRootList.erase(iter);
                ClearTreeItem(item);
                delete item;
                item = 0;
                return CAT_SUCCESS;
            }

            ++iter;
        }
        return CATRESULT(CAT_ERR_TREE_ITEM_NOT_FOUND);
    }

    CATTREEINFO* parent = item->Parent;
    numChildren = parent->Children.size();

    std::vector<CATTREEINFO*>::iterator iter = parent->Children.begin();
    while (iter != parent->Children.end())
    {
        if ((*iter) != item)
        {
            parent->Children.erase(iter);
            ClearTreeItem(item);
            delete item;
            item = 0;
            return CAT_SUCCESS;
        }
        ++iter;
    }

    return CATRESULT(CAT_ERR_TREE_ITEM_NOT_FOUND);
}

void CATTreeCtrl::ClearTreeItem(CATTREEINFO* item)

{
    while (item->Children.size())
    {
        CATTREEINFO* curItem = item->Children[0];
        ClearTreeItem(curItem);
        delete curItem;
        item->Children.erase(item->Children.begin());
    }
}

CATResult CATTreeCtrl::Clear()
{
    fCurSel = 0;

    while (fRootList.size())
    {
        CATTREEINFO* curItem = fRootList[0];
        ClearTreeItem(curItem);
        delete curItem;
        fRootList.erase(fRootList.begin());
    }    

    if (fControlWnd != 0)
        OSClearTree();

    return CAT_SUCCESS;
}


// Event handler
CATResult CATTreeCtrl::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    return OSEvent(event,retVal);   
}

//---------------------------------------------------------------------------
// GetCommand() returns the command for the control
//---------------------------------------------------------------------------
CATCommand CATTreeCtrl::GetCommand() const
{   
    return CATCommand(this->fCmdString, 1, this->GetString(), this->fTarget, this->fCmdType);
}



CATString CATTreeCtrl::GetString() const
{
    if (this->fCurSel != 0)
    {
        return fCurSel->DisplayText;
    }

    return "No Selection";
}

CATResult CATTreeCtrl::SetItemText(CATTREEINFO* item, const CATString& text)
{
    if (item == 0)
    {
        return CATRESULT(CAT_ERR_INVALID_PARAM);
    }

    item->DisplayText = text;
    return OSUpdateText(item);
}

CATResult CATTreeCtrl::ParseAttributes()
{
 	 CATResult res = CATControlWnd::ParseAttributes();
	 CATUInt32 itemNum = 1;
	 for (;;)
	 {
		 CATString itemName = "Item_";
		 itemName << itemNum;
		 CATString tempStr = GetAttribute(itemName);
		 if (tempStr.IsEmpty())
			 break;
		 CATTREEINFO* newItem = 0;
		 this->Insert(tempStr,0,newItem,(void*)itemNum);
		 itemNum++;
	 }

	return res;
}

void CATTreeCtrl::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{
	CATControlWnd::PostDraw(drawContext,dirtyRect);
}