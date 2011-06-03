//---------------------------------------------------------------------------
/// \file CATListBox.h
/// \brief list box for GUI
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
//
//---------------------------------------------------------------------------
#ifndef _CATListBox_H_
#define _CATListBox_H_

#include "CATControlWnd.h"
#include <vector>

class CATListBox;

struct CATLISTINFO
{
    CATString		DisplayText;
    void*			ListInfo;
    CATListBox*	    BaseListBox;
};


/// \class CATListBox
/// \brief list box for GUI
/// \\ingroup CATGUI
class CATListBox : public CATControlWnd
{
public:
    CATListBox(  const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATListBox();

    virtual bool OnControlEvent( const CATEvent& event,  CATInt32& result);
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

    virtual void SetFocused(bool focused);

    virtual CATString GetHint() const   ;

    virtual void OnEscapeChange();

    virtual void OnParentCreate();
    virtual void OnParentDestroy();

	 virtual CATResult ParseAttributes();

	 virtual CATFloat32	GetValue() const;
	 virtual void        SetValue(CATFloat32 newValue, bool sendCommand = true);

    virtual CATInt32    GetCount() const;

    virtual CATInt32    GetCurIndex() const;

    virtual CATResult  SetCurSel( CATInt32 index = -1);

    virtual CATString  GetText( CATInt32 index = -1) const;

    virtual CATResult  SetCurSelByName( const CATString& displayText);

    virtual CATResult  Insert( const CATString&  displayText,
        void*            dataPtr,
        CATInt32           index = -1);

    virtual CATResult  Remove( CATInt32    index);

    virtual CATResult  RemoveByName( const CATString& displayText);


    virtual CATResult  Get( CATInt32       index,
        CATString&    displayTextRef,
        void*&       dataRef) const;

    virtual CATResult  GetByName(  const CATString& displayTextRef,
        void*&          dataRef) const;

    virtual CATResult  Clear    ();

    virtual CATCommand GetCommand() const ;

    // GetString retrieves current string, which is either the nulltext
    // or whatever is selected.
    virtual CATString GetString () const;

    virtual void   OSOnMeasureItem(	CATLISTINFO* listItem, CATUInt32& width, CATUInt32& height);
    virtual void   OSOnDrawItem(		CATLISTINFO* listItem, bool selected, CATDRAWCONTEXT hDC, CATRect rect );
protected:

    CATResult OSEvent(const CATEvent& event, CATInt32& retVal);

    void     OSClearList    ();
    CATResult OSAddItem      ( CATInt32 index, const CATString& displayText, const CATLISTINFO* listInfo);
    CATResult OSRemoveItem   ( CATInt32 index );
    CATInt32   OSGetCurSel    ();
    CATResult OSSetCurSel    ( CATInt32 index );

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_LIST;}
#endif

protected:
    CATFONT      fFont;

    CATInt32      fCurSel;
    std::vector   <CATLISTINFO*> fList;
};

#endif // _CATListBox_H_


