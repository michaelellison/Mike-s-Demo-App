//---------------------------------------------------------------------------
/// \file CATTreeCtrl.h
/// \brief list box for GUI
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
//---------------------------------------------------------------------------
#ifndef CATTreeCtrl_H_
#define CATTreeCtrl_H_

#include "CATControlWnd.h"
#include <commctrl.h>


struct CATTREEINFO
{
    CATString             DisplayText;
    void*                DataPtr;
    struct CATTREEINFO*   Parent;
    std::vector<CATTREEINFO*>   Children;
    CATInt32               ItemHandle;
    CATInt32					ImageNumber;
};


/// \class CATTreeCtrl
/// \brief list box for GUI
/// \ingroup CATGUI
class CATTreeCtrl : public CATControlWnd
{
public:
    CATTreeCtrl(  const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATTreeCtrl();

    virtual CATResult  OnEvent(const CATEvent& event, CATInt32& retVal);


    virtual void SetFocused(bool focused);
	 virtual void PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect);

    virtual void OnEscapeChange();

    virtual void OnParentCreate();
    virtual void OnParentDestroy();

    virtual CATTREEINFO* GetCurItem();
    virtual CATTREEINFO* GetRootItem(CATUInt32 index);
    virtual CATUInt32    GetNumRootItems();
    virtual CATResult    SetCurItem(CATTREEINFO* item, bool sendCommand = true);

    virtual CATResult    SetItemText(CATTREEINFO* item, const CATString& text);
    virtual CATResult    ExpandItem(CATTREEINFO* item, bool expand);

    virtual CATResult    Insert( const CATString&  displayText,
										   CATTREEINFO*      parent,
										   CATTREEINFO*&     newItemRef,
										   void*             dataPtr,
										   CATUInt32         index = -1,
										   CATUInt32		   imageNumber = -1);

    virtual CATTREEINFO* GetItemFromDataPtr( void* dataPtr, CATTREEINFO* rootItem = 0);

    virtual CATResult  Remove( CATTREEINFO*&     item);

    virtual CATResult  Clear();

    virtual CATCommand GetCommand() const ;

    virtual CATString GetString () const;

    virtual void SetEnabled(bool enabled);

    virtual CATResult UseImageList( CATUInt32 imageListId, CATUInt32 width, const CATColor& transparent );

    virtual void ExpandRoot();
	 virtual CATResult   ParseAttributes();
	 virtual bool NeedsArrows() {return true;}
protected:
#ifdef _WIN32
    HIMAGELIST	fImageList;
#endif
    void        ClearTreeItem(CATTREEINFO* item);
    void        OSClearTree    ();
    CATResult    OSAddItem      ( const CATString& displayText, CATTREEINFO* treeItem, CATUInt32 index);
    CATResult    OSRemoveItem   ( CATTREEINFO* treeItem);
    CATTREEINFO* OSGetCurSel    ();
    CATResult    OSSetCurSel    ( CATTREEINFO* newSel );      
    CATResult    OSUpdateText   ( CATTREEINFO* treeItem);
    CATResult    OSEvent        (const CATEvent& event, CATInt32& retVal);
    void        OSRebuildTree(std::vector<CATTREEINFO*>* curList);
    void        OSUpdateTreeColors();
protected:
    CATTREEINFO* fCurSel;
    std::vector<CATTREEINFO*> fRootList;      
    CATFONT      fFont;
    bool		   fRespondSelect;
};

#endif // CATTreeCtrl_H_


