//---------------------------------------------------------------------------
/// \file CATMenu.cpp
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
#include "CATMenu.h"
#include "CATWindow.h"
#include "CATEventDefs.h"
//---------------------------------------------------------------------------
// CATMenu constructor (inherited from CATControl->CATXMLObject)
// \param element - Type name ("Label")
// \param attribs - attribute information for the window
// \param parent - parent XML object (should be a "Window" element)
// \param rootDir - root directory of skin for bin/png load
//---------------------------------------------------------------------------
CATMenu::CATMenu(    const CATString&             element, 
                     const CATString&             rootDir)
: CATControl(element,  rootDir)
{
    fUpdateTextOnSel = true;
    fShowSel         = false;
    fCurSel          = 0;   
    // Values are meaningless in menus
    fForceWidth      = false;
    fValue           = 0;
    fMinValue        = 0;
    fMaxValue        = 0;
    fMultiline       = false;
    fShowBg          = true;
    fMenuDirty       = true;
    fMenuId          = 0;
    fIdCount         = 100; // Arbitrary starting point for menu id's   
    fItalics         = false;
    fWindowBuilt     = false;
    fOutline	     = true;
    fMenuStyle       = CATMENUSTYLE_DOWN;
}

//---------------------------------------------------------------------------
// CATMenu destructor
//---------------------------------------------------------------------------
CATMenu::~CATMenu()
{
    ClearMenu();

}


/// IsFocusable() returns true if the control can receive
/// focus, and false otherwise.
bool CATMenu::IsFocusable() const
{
    return true;
}

/// ParseAttributes() parses the known attributes for an object.
CATResult CATMenu::ParseAttributes()
{
    CATResult result = CATControl::ParseAttributes();
    CATString attrib;

    fShowSel         = GetAttribute(L"ShowSelection",fShowSel);
    fUpdateTextOnSel = GetAttribute(L"UpdateText",   fUpdateTextOnSel);
    fOutline         = GetAttribute(L"Outline",      fOutline);
    fWindowBuilt     = GetAttribute(L"WindowBuilt",  fWindowBuilt);
    fForceWidth      = GetAttribute(L"ForceWidth",   fForceWidth);

	 CATUInt32 itemNum = 1;
	 for (;;)
	 {
		 CATString itemName = "Item_";
		 itemName << itemNum;
		 CATString tempStr = GetAttribute(itemName);
		 if (tempStr.IsEmpty())
			 break;
		 CATMENUITEM* newItem = 0;
		 this->Insert(tempStr,0,newItem,(void*)(itemNum-1));
		 itemNum++;
	 }

    attrib = GetAttribute(L"MenuStyle");
    if (!attrib.IsEmpty())
    {
        if (attrib.Compare("Up") == 0)
        {
            fMenuStyle = CATMENUSTYLE_UP;
        }
        else if (attrib.Compare("Down") == 0)
        {
            fMenuStyle = CATMENUSTYLE_DOWN;
        }
        else if (attrib.Compare("UpMouse") == 0)
        {
            fMenuStyle = CATMENUSTYLE_UPMOUSE;
        }
        else if (attrib.Compare("DownMouse") == 0)
        {
            fMenuStyle = CATMENUSTYLE_DOWNMOUSE;
        }
        else
        {
            CATTRACE("Unknown menu style....");
        }
    }

    return result;
}

// PostDraw() draws any stuff that requires an OS-specific draw
// context.
void CATMenu::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
    {
        return;
    }


    if ((fText.IsEmpty() == false) || (fImage == 0))
    {
        CATColor foreColor(fForegroundColor);
        CATColor backColor(fBackgroundColor);

        if (this->IsEnabled() == false)
        {
            foreColor = fFgDisColor;      
            backColor = fBgDisColor;
        }

        CATRect textRect = fRect;

        if (fOutline && (fImage == 0))
        {
#ifdef _WIN32
            CATRect frameRect = fRect;
            this->GetWindow()->WidgetToWindow(this,frameRect);

            HBRUSH frameColor = (HBRUSH)(this->IsEnabled() ? (::GetStockObject(BLACK_BRUSH)) : 
                                                             (::GetStockObject(GRAY_BRUSH))) ;            

            ::FrameRect(drawContext, (RECT*)(CATRECT*)&frameRect,frameColor);

            textRect.left++;
            textRect.top++;
            textRect.right--;
            textRect.bottom--;
#endif
        }

        if (this->fPressed)
        {
            textRect.top  += fTextOffsetPressed.x;
            textRect.left += fTextOffsetPressed.y;
        }
        else
        {
            textRect.top  += fTextOffset.x;
            textRect.left += fTextOffset.y;
        }

        CATRect dRect = dirtyRect;
        this->GetWindow()->WidgetToWindow(this,textRect);
        this->GetWindow()->WidgetToWindow(this,dRect);
        this->GetWindow()->PostDrawText( fText,
                                         drawContext, 
                                         textRect, 
                                         dRect,
                                         foreColor,
                                         fFontName,
                                         fFontSize,
                                         fMultiline,
                                         fShowBg?&backColor:0,
                                         fItalics,
                                         fTextCentered, fAutoScaleText);
    }
}


// SetString 
void CATMenu::SetString ( const CATString& text )
{
    if (text != fText)
    {
        fText = text;
        this->MarkDirty();
    }
}

// GetString retrieves current string, which is either the nulltext
// or whatever is selected.
CATString CATMenu::GetString () const
{
    return fText;
}

CATResult CATMenu::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    return CATControl::OnEvent(event,retVal);
}

// Tree building / modification
CATMENUITEM* CATMenu::GetCurItem()
{
    return fCurSel;
}

CATResult CATMenu::SetCurItem(CATMENUITEM* item)
{
    fMenuDirty = true;


    fCurSel = item;

    if ((item) && (fUpdateTextOnSel))
    {
        fText = item->DisplayText;
    }

    this->MarkDirty();
    return CAT_SUCCESS;
}

CATUInt32 CATMenu::GetNumTopNodes()
{
    return (CATUInt32)this->fRootList.size();
}

CATMENUITEM* CATMenu::GetTopNode( CATUInt32 index )
{
    if (index >= fRootList.size())
        return 0;

    return fRootList[index];
}


CATResult CATMenu::InsertSeperator( CATMENUITEM* parent, CATUInt32 index)
{
    CATMENUITEM* sepItem = 0;
    return this->Insert( "",parent, sepItem,0,false,index);
}

CATResult CATMenu::Insert(    const CATString&        displayText,
                          CATMENUITEM*      parent,
                          CATMENUITEM*&     newItemRef,
                          void*            dataPtr,
                          bool             subMenu,
                          CATUInt32          index,
                          const CATString&       altCommand)
{
    fMenuDirty = true;

    newItemRef = new CATMENUITEM;

    if (newItemRef == 0)
    {
        return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
    }

    newItemRef->BaseMenu    = this;
    newItemRef->DataPtr     = dataPtr;
    newItemRef->DisplayText = displayText;
    newItemRef->IsSubMenu   = false;
    newItemRef->ItemHandle  = 0;
    newItemRef->Parent      = parent;
    newItemRef->IsSubMenu = subMenu;
    newItemRef->AltCommand = altCommand;

    if (parent)
    {
        parent->Children.push_back(newItemRef);
    }
    else
    {
        this->fRootList.push_back(newItemRef);      
    }

    return CAT_SUCCESS;
}


CATResult CATMenu::Remove( CATMENUITEM*&     item)
{
    fMenuDirty = true;

    if (item == 0)
    {
        return CATRESULT(CAT_ERR_INVALID_PARAM);
    }

    CATUInt32 numChildren = 0;

    if (item->Parent)
    {
        std::vector<CATMENUITEM*>::iterator iter = item->Parent->Children.begin();
        while (iter != item->Parent->Children.end())
        {
            if (item == (*iter))
            {
                iter = item->Parent->Children.erase(iter);
                ClearMenuItem(item);
                delete item;
                item = 0;
                return CAT_SUCCESS;
            }
            ++iter;
        }
    }
    else
    {
        std::vector<CATMENUITEM*>::iterator iter = fRootList.begin();      
        while (iter != fRootList.end())
        {
            if (item == (*iter))
            {
                iter = fRootList.erase(iter);
                ClearMenuItem(item);
                delete item;
                item = 0;
                return CAT_SUCCESS;
            }
        }
    }   
    return CATRESULT(CAT_ERR_LIST_ITEM_NOT_FOUND);
}

void CATMenu::ClearMenuItem(CATMENUITEM* item)

{
    while (item->Children.size())
    {
        CATMENUITEM* curItem = item->Children[0];
        ClearMenuItem(curItem);
        delete curItem;
        item->Children.erase(item->Children.begin());
    }
}

CATResult CATMenu::Clear()
{
    while (fRootList.size())
    {
        CATMENUITEM* curItem = fRootList[0];
        ClearMenuItem(curItem);
        delete curItem;
        fRootList.erase(fRootList.begin());
    }
    fCurSel     = 0;   

    fMenuDirty  = true;
    return CAT_SUCCESS;
}



//---------------------------------------------------------------------------
// GetCommand() returns the command for the control
//---------------------------------------------------------------------------
CATCommand CATMenu::GetCommand() const
{   
    CATString stringParam = this->GetString();

    if (this->fCurSel)
    {
        stringParam = fCurSel->DisplayText;
    }

	 return CATCommand(this->fCmdString, (CATFloat32)(CATUInt32)fCurSel->DataPtr, stringParam, this->fTarget, this->fCmdType);
}

CATString CATMenu::GetHint() const
{
    CATString retString;
    retString = CATControl::GetHint();
    if (fShowHintValue)
    {
        retString << " ( " << this->GetString() << " )";
    }
    return retString;
}

void CATMenu::SetItalics(bool italics)
{
    fItalics = italics;
    this->MarkDirty();
}

bool CATMenu::GetItalics() const
{
    return fItalics;
}

bool CATMenu::HasSelectionMark()
{
    return fShowSel;
}

void CATMenu::OnMouseClick()
{        
    if (fWindowBuilt == true)
    {
        this->GetWindow()->OnBuildMenu(this);
    }

    DoMenu();
}

bool CATMenu::ForceWidth() const
{
    return fForceWidth;
}