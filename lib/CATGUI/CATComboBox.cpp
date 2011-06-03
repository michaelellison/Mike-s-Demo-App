//---------------------------------------------------------------------------
/// \file CATComboBox.cpp
/// \brief GUI Layer object - contains sub-controls
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
#include "CATComboBox.h"
#include "CATApp.h"
#include "CATFileSystem.h"
#include "CATStream.h"
#include "CATEvent.h"
#include "CATWindow.h"
#include "CATEventDefs.h"

#include "CATEditBox.h"
#include "CATMenu.h"
#include "CATPrefs.h"
//---------------------------------------------------------------------------
// CATComboBox constructor (inherited from CATGuiObj)
// \param element - Type name (e.g. "Button", "Label", etc.)
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
//---------------------------------------------------------------------------
CATComboBox::CATComboBox( const CATString&               element, 
                         const CATString&               rootDir)
                         : CATLayer(element,  rootDir)
{   
    fMaxMenuLength = 10;
    fComboEdit     = 0;
    fComboMenu     = 0;   
}

//---------------------------------------------------------------------------
// CATComboBox destructor
//---------------------------------------------------------------------------
CATComboBox::~CATComboBox()
{
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATComboBox::ParseAttributes()
{
    CATResult result = CATLayer::ParseAttributes();

    CATString attrib = GetAttribute(L"HistoryMax");
    if (!attrib.IsEmpty())
        fMaxMenuLength = (CATUInt32)attrib;

    fPrefName = GetAttribute(L"PrefName");

    return result;
}


CATResult CATComboBox::Load(		// Optional progress callback information
                            CATPROGRESSCB				progressCB,
                            void*							progressParam,
                            CATFloat32						progMin,
                            CATFloat32						progMax)
{
    CATResult result = CATLayer::Load(progressCB, progressParam, progMin, progMax);

    // Set prefname if unset
    if (fPrefName.IsEmpty())
        fPrefName = this->fName + (CATString)"_ComboList";

    CATUInt32 numChildren = this->GetNumChildren();
    for (CATUInt32 i = 0; i < numChildren; i++)
    {
        CATGuiObj* childObj = (CATGuiObj*)GetChild(i);
        CATString  curElem  = childObj->GetType();
        if (curElem.Compare("Menu") == 0)
        {
            this->fComboMenu = (CATMenu*)childObj;
        }
        else if (curElem.Compare("EditBox") == 0)
        {
            this->fComboEdit = (CATEditBox*)childObj;
        }
    }   

    if (fComboMenu == 0)
        result = CATRESULT(CAT_ERR_COMBO_MENU_NOT_FOUND);
    if (fComboEdit == 0)
        result = CATRESULT(CAT_ERR_COMBO_EDIT_NOT_FOUND);

    return result;
}

void CATComboBox::OnParentCreate()
{
    CATLayer::OnParentCreate();       
    this->RebuildCombo();   
}

void CATComboBox::OnParentDestroy()
{
    CATLayer::OnParentDestroy();
}

// Process commands from children for combo.
void CATComboBox::OnCommand( CATCommand& command,
                            CATControl* ctrl)
{  
    CATString param = command.GetStringParam();

    if (ctrl == fComboMenu)
    {
        if (fComboEdit)
        {
            fComboEdit->SetString( param );
        }

        AddComboString(param);
        ctrl = fComboEdit;
    }
    else if (ctrl == fComboEdit)
    {
        AddComboString(param);      
    }
    else 
    {
        // Not either of our normal controls - just pass it through.
        CATLayer::OnCommand(command, ctrl);
        return;
    }

    if ((fParent) && (fComboEdit))
    {
        ((CATGuiObj*)fParent)->OnCommand( fComboEdit->GetCommand(), fComboEdit);
    }
}

void CATComboBox::AddComboString(const CATString& newString)
{
    fMenuList.insert(fMenuList.begin(),newString);

    // Cut dupes
    std::vector<CATString>::iterator iter = fMenuList.begin();
    while (iter != fMenuList.end())
    {
        if ( (*iter).Compare(newString) == 0)
            iter = fMenuList.erase(iter);
        else
            ++iter;
    }

    // Cap to max
    while (fMenuList.size() > fMaxMenuLength)
    {
        iter = fMenuList.end();
        --iter;
        fMenuList.erase(iter);      
    }

}

void CATComboBox::RebuildCombo()
{
    if (fComboMenu == 0)
        return;

    fComboMenu->Clear();

    CATUInt32 numEntries = fMenuList.size();
    for (CATUInt32 i = 0; i < numEntries; i++)
    {
        CATString curString = fMenuList[i];      
        CATMENUITEM* newItem = 0;
        fComboMenu->Insert(  curString, 0, newItem, 0);
    }
}

CATEditBox* CATComboBox::GetEditBox()
{
    return this->fComboEdit;
}

CATMenu* CATComboBox::GetMenu()
{
    return this->fComboMenu;
}

CATResult CATComboBox::OnEvent( const CATEvent& event, CATInt32& retVal)
{
    switch (event.fEventCode)
    {
    case CATEVENT_PREF_CHANGED:
        if (event.fStringParam1.Compare(fPrefName) == 0)
        {
            this->RebuildCombo();
        }
        retVal = 1;
        break;
    }

    return CATLayer::OnEvent(event, retVal);
}
