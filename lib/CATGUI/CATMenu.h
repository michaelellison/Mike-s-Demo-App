//---------------------------------------------------------------------------
/// \file CATMenu.h
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
#ifndef _CATMenu_H_
#define _CATMenu_H_

#include "CATControl.h"
#include <vector>

class CATMenu;


/// Menu item structure
struct CATMENUITEM
{
    CATString                 DisplayText;
    bool                      IsSubMenu;
    void*                     DataPtr;
    struct CATMENUITEM*       Parent;
    std::vector<CATMENUITEM*> Children;
    CATInt32                  ItemHandle;
    CATMenu*                  BaseMenu;
    CATString                 AltCommand;
};

/// CATMENUSTYLE - types of menus.
enum CATMENUSTYLE
{
    CATMENUSTYLE_UP,			/// Menu pops upwards from menu label location
    CATMENUSTYLE_DOWN,			/// Menu pops downwards from menu label loc
    CATMENUSTYLE_UPMOUSE,		/// Menu pops upwards from mouse location
    CATMENUSTYLE_DOWNMOUSE	/// Menu pops downwards from mouse location
};

/// \class CATMenu CATMenu.h
/// \brief Text label for GUI
/// \ingroup CATGUI
class CATMenu : public CATControl
{
public:
    /// CATMenu constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Label")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png load
    CATMenu(    const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATMenu();          

    /// IsFocusable() returns true if the control can receive
    /// focus, and false otherwise.
    virtual bool	     IsFocusable() const;


    /// PostDraw() draws any stuff that requires an OS-specific draw
    /// context.
    virtual void   PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();


    // SetString only affects text displayed when no item is selected.
    virtual void SetString ( const CATString& text );

    // GetString retrieves current string, which is either the nulltext
    // or whatever is selected.
    virtual CATString GetString () const;

    /// Event handler
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);


    /// Get the currently selected item data
    /// \return CATMENUITEM* - current item data, or null
    virtual CATMENUITEM* GetCurItem();
    /// Set the current item
    /// \param item - CATMENUITEM* of current item (or null to select none)
    /// \return CAT_SUCCESS on success
    virtual CATResult    SetCurItem(CATMENUITEM* item);

    /// GetNumTopNodes() returns the number of nodes in the top menu
    /// \return CATUInt32 - number of top-level menu nodes
    virtual CATUInt32     GetNumTopNodes();

    /// GetTopNode() returns the requested top-level menu node by index
    /// \param index - index to top-level node (from 0 to GetNumTopNodes())
    /// \return CATMENUITEM* - requested menu node, or 0 if it doesn't exist
    virtual CATMENUITEM* GetTopNode( CATUInt32 index );

    /// Insert() inserts an item into the menu's tree.
    /// \param displayText - text to display for item in menu
    /// \param parent - ptr to parent menu item
    /// \param newItemRef - ref to ptr to receive created menu item on success
    /// \param dataPtr - void* to store caller's data for menu item
    /// \param subMenu - if true, item has no dataPtr and extends to a sub menu
    /// \param index - index to insert at. -1 is at end of current menu
    /// \param altCommand - if true, then instead of sending the scripted menu's command
    ///                     and using dataPtr for information, the item will send the
    ///                     altCommand to the window.
    /// \return CATResult - CAT_SUCCESS on success
    virtual CATResult    Insert( const CATString&  displayText,
                                 CATMENUITEM*      parent,
                                 CATMENUITEM*&     newItemRef,
                                 void*             dataPtr,
                                 bool              subMenu     = false,                                  
                                 CATUInt32         index       = -1,
                                 const CATString&  altCommand  = "");

    /// InsertSeperator() inserts a seperator into the menu
    /// \param parent - parent menu item or 0 for top-level nodes
    /// \param index - index to insert seperator at or -1 for end of current menu
    /// \return CATResult - CAT_SUCCESS on success
    virtual CATResult     InsertSeperator( CATMENUITEM*  parent = 0, 
                                           CATUInt32     index  = -1);

    /// Remove() removes the specified menu item and its children.
    /// \param item - ref to ptr of menu item to remove. set to 0 afterwards
    /// \return CAT_SUCCESS if found and removed.
    virtual CATResult    Remove( CATMENUITEM*&     item);

    /// Clear() clears the menu
    virtual CATResult    Clear();

    /// OnMouseClick() is called if the mouse was pressed, then released within
    /// a control.
    virtual void        OnMouseClick();

    /// GetCommand() returns the command for the control.
    /// \return CATCommand - command from control
    virtual CATCommand  GetCommand() const;

    /// GetHint() retrieves the menu's hint text
    /// \return CATString - hint text for menu
    virtual CATString   GetHint() const;

    /// SetItalics() sets the menu's italicized state
    /// \param italics - if true, menu label is drawn italicized.
    virtual void        SetItalics( bool italics );

    /// GetItalics() retrieves the menu's italicized state
    /// \return bool - true if italicized
    virtual bool        GetItalics() const;      

    /// HasSelectionMark() returns true if menu displays a selection mark when drawn on current item.
    virtual bool        HasSelectionMark();

    virtual bool        ForceWidth() const;
#ifdef _WIN32
    /// Called by OS to measure the items
    virtual void        OSOnMeasureItem( CATMENUITEM* menuItem, CATUInt32& width, CATUInt32& height);

    /// Called by OS to draw the items
    virtual void        OSOnDrawItem(	 CATMENUITEM* menuItem, bool selected, CATDRAWCONTEXT hDC, CATRect rect );

    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_PUSHBUTTON;}
    
#endif // WIN32

    /// Do the menu
    virtual void        DoMenu();   

protected:
    /// Recursive fill of menu from existing data
    void BuildMenu();
    
    /// Clear local menu
    void ClearMenu();
    void ClearMenuItem(CATMENUITEM* item);

#ifdef _WIN32
    /// Win32-only submenu creation 
    void CreateSubMenu( std::vector<CATMENUITEM*>& itemList, HMENU parentMenu);

    CATStack<HMENU>                      fMenuStack;
    std::map<CATUInt32, CATMENUITEM*>    fIdMap;
#endif
    bool                        fUpdateTextOnSel;    // Update the current text on selections?

    bool                        fShowSel;            // Show the current selection with a mark...
    CATMENUITEM*                fCurSel;
    std::vector<CATMENUITEM*>   fRootList;
    bool                        fMenuDirty;
    bool                        fItalics;
    CATMENUSTYLE                 fMenuStyle;
    // platform-specific menu id/handle
    CATUInt32                   fMenuId;
    CATUInt32                   fIdCount;
    bool                        fWindowBuilt;
    bool				        fOutline;
    bool                        fForceWidth;
};

#endif // _CATMenu_H_


