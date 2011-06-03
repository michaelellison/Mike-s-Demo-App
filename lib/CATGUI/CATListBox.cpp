//---------------------------------------------------------------------------
/// \file CATListBox.cpp
/// \brief List box for GUI
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
#include "CATListBox.h"
#include "CATWindow.h"
#include "CATEvent.h"

#include "CATEventDefs.h"

CATListBox::~CATListBox()
{
    while (fList.size())
    {
        delete fList[0];
        fList.erase(fList.begin());
    }    
}

void CATListBox::SetFocused(bool focused)
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

void CATListBox::OnEscapeChange()
{
    this->SetCurSel(fCurSel);
    this->fValue = (CATFloat32)fCurSel;
    this->MarkDirty();
}

CATInt32 CATListBox::GetCount() const
{
    return this->fList.size();
}

CATInt32 CATListBox::GetCurIndex() const
{
    return this->fCurSel;   
}

CATResult CATListBox::Insert( const CATString&  displayText,
                             void*             dataPtr,
                             CATInt32          index)
{
    CATResult result = CAT_SUCCESS;

    // Create a list info object and add it to list.
    // Let the list own it so it will be deleted on removal.
    CATLISTINFO* listInfo = new CATLISTINFO;
    listInfo->DisplayText = displayText;
    listInfo->ListInfo = dataPtr;
    listInfo->BaseListBox = this;

    if (index == -1)
        index = this->fList.size();

    std::vector<CATLISTINFO*>::iterator iter = fList.begin();
    iter += index;

    iter = this->fList.insert(iter,listInfo);

    fMaxValue++;
    return OSAddItem(index,displayText,listInfo);
}

CATResult CATListBox::Remove( CATInt32    index)
{
    if (index >= (CATInt32)fList.size())
        return CAT_ERR_INVALID_PARAM;

    std::vector<CATLISTINFO*>::iterator iter = fList.begin();
    iter += index;
    delete (*iter);
    fList.erase(iter);

    fMaxValue--;

    return OSRemoveItem(index);   
}

CATResult CATListBox::RemoveByName( const CATString& displayText)
{
    CATResult result = CAT_SUCCESS;
    CATLISTINFO* listInfo = 0;

    for (CATUInt32 i = 0; i < fList.size(); i++)
    {
        listInfo = fList[i];
        if (listInfo->DisplayText.Compare(displayText) == 0)
        {
            return this->Remove(i);        
        }
    }

    return CATRESULT(CAT_ERR_LIST_ITEM_NOT_FOUND);
}


CATResult CATListBox::Get( CATInt32       index,
                          CATString&    displayTextRef,
                          void*&       dataRef) const
{
    CATLISTINFO* listInfo = fList[index];
    displayTextRef = listInfo->DisplayText;
    dataRef = listInfo->ListInfo;
    return CAT_SUCCESS;
}

CATResult CATListBox::GetByName(  const CATString& displayTextRef,
                                void*&          dataRef) const
{
    CATResult result = CAT_SUCCESS;
    CATLISTINFO* listInfo = 0;

    for (CATUInt32 i = 0; i < fList.size(); i++)
    {
        listInfo = fList[i];
        if (listInfo->DisplayText.Compare(displayTextRef) == 0)
        {
            dataRef = listInfo->ListInfo;
            return CAT_SUCCESS;
        }
    }

    return CATResult(CAT_ERR_LIST_ITEM_NOT_FOUND);
}

CATResult CATListBox::Clear()
{
    while (fList.size())
    {
        delete fList[0];
        fList.erase(fList.begin());
    }

    OSClearList();
    fMaxValue = -1;
    return CAT_SUCCESS;
}


CATResult CATListBox::SetCurSel( CATInt32 index)
{
    if ((index >= -1) && (index < (CATInt32)fList.size()))
    {
        this->fCurSel = index;
        this->fValue = (CATFloat32)fCurSel;
        return OSSetCurSel(index);      
    }
    else
    {
        fCurSel = -1;
        this->fValue = (CATFloat32)fCurSel;
        return OSSetCurSel(-1);
    }   
}


CATResult CATListBox::SetCurSelByName( const CATString& displayText)
{
    CATResult result = CAT_SUCCESS;
    CATLISTINFO* listInfo = 0;

    for (CATUInt32 i = 0; i < fList.size(); i++)
    {
        listInfo = fList[i];
        if (listInfo->DisplayText.Compare(displayText) == 0)
        {
            return this->SetCurSel(i);         
        }
    }

    return CATRESULT(CAT_ERR_LIST_ITEM_NOT_FOUND);
}


// Event handler
CATResult CATListBox::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    CATResult result = CAT_SUCCESS;

    // handle incoming browser events....
    // All of our events are targetted by fStringParam2 of the event.
    // IF it is empty or matches our control name, then we should respond.
    if ((event.fStringParam2.IsEmpty()) || (event.fStringParam2.Compare(this->fName) == 0))
    {      
        switch (event.fEventCode)
        {
            /// CATEVENT_LISTBOX_ADD
            ///
            /// This event adds an item to the specified list box
            ///
            /// fIntParam1    - index
            /// fStringParam1 - text to add
            /// fStringParam2 - target
            /// fVoidParam    - list data
        case CATEVENT_LISTBOX_ADD:
            return this->Insert(event.fStringParam1,event.fVoidParam,(CATInt32)event.fIntParam1);

            /// CATEVENT_LISTBOX_REMOVE_INDEX
            ///
            /// This event adds an item to the specified list box
            ///
            /// fIntParam1    - index of item to remove
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_REMOVE_INDEX:
            return this->Remove((CATInt32)event.fIntParam1);

            /// CATEVENT_LISTBOX_REMOVE_STRING
            ///
            /// This event adds an item to the specified list box
            ///
            /// fStringParam1 - string to find and remove
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_REMOVE_STRING:
            return this->RemoveByName(event.fStringParam1);

            /// CATEVENT_LISTBOX_SET_SEL
            ///
            /// Sets the current item - -1 is none
            ///
            /// fIntParam1 - index of selection
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_SET_SEL:
            return this->SetCurSel((CATInt32)event.fIntParam1);

            /// CATEVENT_LISTBOX_SET_SEL_STRING
            ///
            /// fStringParam1 - string to find and select
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_SET_SEL_STRING:
            return this->SetCurSelByName(event.fStringParam1);

            /// CATEVENT_LISTBOX_CLEAR
            ///
            /// Clears the listbox
            ///   
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_CLEAR:
            return this->Clear();

            /// CATEVENT_LISTBOX_GET_SEL
            ///
            /// Retrieves the current item in retval
            ///
            /// fStringParam2 - target
        case CATEVENT_LISTBOX_GET_SEL:
            retVal = this->GetCurIndex();
            return CAT_SUCCESS;

            /// CATEVENT_LISTBOX_GET_SEL_DATA
            ///
            /// Retrieves the currently selected item's data into
            /// a void* pointed to by fVoidParam - i.e. fVoidParam is a
            /// void**.
            ///
            /// fStringParam2 - target
            /// fVoidParam - void** to receive data
        case CATEVENT_LISTBOX_GET_SEL_DATA:

            if ((event.fVoidParam != 0) && (this->fCurSel >= 0) && (this->fCurSel < this->GetCount()))
            {
                CATString dummyText;
                return this->Get(this->fCurSel, dummyText, *((void**)event.fVoidParam));
            }
            return CATRESULT(CAT_ERR_LIST_OUT_OF_RANGE);            
        }
    }


    return OSEvent(event,retVal);
}


CATString CATListBox::GetHint() const
{
    CATString retString;
    retString = CATControl::GetHint();
    if (fShowHintValue)
    {
        if (this->GetCurIndex() != -1)
        {
            retString << " ( " << this->GetText() << " )";
        }
    }
    return retString;
}

CATString CATListBox::GetText( CATInt32 index) const
{   
    CATString retString;

    if (index == -1)
    {
        index = this->fCurSel;
    }

    if (index == -1)
        return retString;

    void* data = 0;
    this->Get(index, retString,  data);

    return retString;
}

//---------------------------------------------------------------------------
// GetCommand() returns the command for the control
//---------------------------------------------------------------------------
CATCommand CATListBox::GetCommand() const
{   
    return CATCommand(this->fCmdString, this->fValue, this->GetString(), this->fTarget, this->fCmdType);
}

CATString CATListBox::GetString() const
{
    return this->GetText();
}

CATFloat32	CATListBox::GetValue() const
{
	//return this->GetCurIndex();
	return CATControlWnd::GetValue();
}

void CATListBox::SetValue(CATFloat32 newValue, bool sendCommand)
{
	//SetCurSel(newValue);
	CATControlWnd::SetValue(newValue,sendCommand);
}

CATResult CATListBox::ParseAttributes()
{
	CATResult res = CATControlWnd::ParseAttributes();
	return res;
}