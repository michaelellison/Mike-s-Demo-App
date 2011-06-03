//---------------------------------------------------------------------------
/// \class CATGUIAccessDefs.h
/// \brief Accessibility definitions (states, roles, etc)
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
#ifndef CATGUIAccessDefs_H_
#define CATGUIAccessDefs_H_

#include "CATInternal.h"

enum CATACCESS_NAVDIR
{
    CAT_NAVDIR_MIN          = 0,
    CAT_NAVDIR_UP,	
    CAT_NAVDIR_DOWN,	
    CAT_NAVDIR_LEFT,	
    CAT_NAVDIR_RIGHT,
    CAT_NAVDIR_NEXT,	
    CAT_NAVDIR_PREVIOUS,
    CAT_NAVDIR_FIRSTCHILD,
    CAT_NAVDIR_LASTCHILD,
    CAT_NAVDIR_MAX
};

/// Object state
enum CATACCESS_STATE
{
    CAT_STATE_SYSTEM_UNAVAILABLE        = 0x00000001,  // Disabled
    CAT_STATE_SYSTEM_SELECTED           = 0x00000002,
    CAT_STATE_SYSTEM_FOCUSED            = 0x00000004,
    CAT_STATE_SYSTEM_PRESSED            = 0x00000008,
    CAT_STATE_SYSTEM_CHECKED            = 0x00000010,
    CAT_STATE_SYSTEM_MIXED              = 0x00000020,  // 3-state checkbox or toolbar button
    CAT_STATE_SYSTEM_INDETERMINATE      = CAT_STATE_SYSTEM_MIXED,
    CAT_STATE_SYSTEM_READONLY           = 0x00000040,
    CAT_STATE_SYSTEM_HOTTRACKED         = 0x00000080,
    CAT_STATE_SYSTEM_DEFAULT            = 0x00000100,
    CAT_STATE_SYSTEM_EXPANDED           = 0x00000200,
    CAT_STATE_SYSTEM_COLLAPSED          = 0x00000400,
    CAT_STATE_SYSTEM_BUSY               = 0x00000800,
    CAT_STATE_SYSTEM_FLOATING           = 0x00001000, // Children "owned" not "contained" by parent
    CAT_STATE_SYSTEM_MARQUEED           = 0x00002000,
    CAT_STATE_SYSTEM_ANIMATED           = 0x00004000,
    CAT_STATE_SYSTEM_INVISIBLE          = 0x00008000,
    CAT_STATE_SYSTEM_OFFSCREEN          = 0x00010000,
    CAT_STATE_SYSTEM_SIZEABLE           = 0x00020000,
    CAT_STATE_SYSTEM_MOVEABLE           = 0x00040000,
    CAT_STATE_SYSTEM_SELFVOICING        = 0x00080000,
    CAT_STATE_SYSTEM_FOCUSABLE          = 0x00100000,
    CAT_STATE_SYSTEM_SELECTABLE         = 0x00200000,
    CAT_STATE_SYSTEM_LINKED             = 0x00400000,
    CAT_STATE_SYSTEM_TRAVERSED          = 0x00800000,
    CAT_STATE_SYSTEM_MULTISELECTABLE    = 0x01000000,  // Supports multiple selection
    CAT_STATE_SYSTEM_EXTSELECTABLE      = 0x02000000,  // Supports extended selection
    CAT_STATE_SYSTEM_ALERT_LOW          = 0x04000000,  // This information is of low priority
    CAT_STATE_SYSTEM_ALERT_MEDIUM       = 0x08000000,  // This information is of medium priority
    CAT_STATE_SYSTEM_ALERT_HIGH         = 0x10000000,  // This information is of high priority
    CAT_STATE_SYSTEM_PROTECTED          = 0x20000000,  // access to this is restricted
    CAT_STATE_SYSTEM_VALID              = 0x3FFFFFFF
};

/// Object role
enum CAT_ACCESS_ROLE
{
    CAT_ROLE_SYSTEM_TITLEBAR            = 0x1 ,
    CAT_ROLE_SYSTEM_MENUBAR	            = 0x2 ,
    CAT_ROLE_SYSTEM_SCROLLBAR	        = 0x3 ,
    CAT_ROLE_SYSTEM_GRIP	            = 0x4 ,
    CAT_ROLE_SYSTEM_SOUND	            = 0x5 ,
    CAT_ROLE_SYSTEM_CURSOR	            = 0x6 ,
    CAT_ROLE_SYSTEM_CARET	            = 0x7 ,
    CAT_ROLE_SYSTEM_ALERT	            = 0x8 ,
    CAT_ROLE_SYSTEM_WINDOW	            = 0x9 ,
    CAT_ROLE_SYSTEM_CLIENT	            = 0xa ,
    CAT_ROLE_SYSTEM_MENUPOPUP	        = 0xb ,
    CAT_ROLE_SYSTEM_MENUITEM	        = 0xc ,
    CAT_ROLE_SYSTEM_TOOLTIP	            = 0xd ,
    CAT_ROLE_SYSTEM_APPLICATION	        = 0xe ,
    CAT_ROLE_SYSTEM_DOCUMENT	        = 0xf ,
    CAT_ROLE_SYSTEM_PANE	            = 0x10 ,
    CAT_ROLE_SYSTEM_CHART	            = 0x11 ,
    CAT_ROLE_SYSTEM_DIALOG	            = 0x12 ,
    CAT_ROLE_SYSTEM_BORDER	            = 0x13 ,
    CAT_ROLE_SYSTEM_GROUPING	        = 0x14 ,
    CAT_ROLE_SYSTEM_SEPARATOR	        = 0x15 ,
    CAT_ROLE_SYSTEM_TOOLBAR	            = 0x16 ,
    CAT_ROLE_SYSTEM_STATUSBAR	        = 0x17 ,
    CAT_ROLE_SYSTEM_TABLE	            = 0x18 ,
    CAT_ROLE_SYSTEM_COLUMNHEADER        = 0x19 ,
    CAT_ROLE_SYSTEM_ROWHEADER	        = 0x1a ,
    CAT_ROLE_SYSTEM_COLUMN	            = 0x1b ,
    CAT_ROLE_SYSTEM_ROW	                = 0x1c ,
    CAT_ROLE_SYSTEM_CELL	            = 0x1d ,
    CAT_ROLE_SYSTEM_LINK	            = 0x1e ,
    CAT_ROLE_SYSTEM_HELPBALLOON	        = 0x1f ,
    CAT_ROLE_SYSTEM_CHARACTER	        = 0x20 ,
    CAT_ROLE_SYSTEM_LIST	            = 0x21 ,
    CAT_ROLE_SYSTEM_LISTITEM	        = 0x22 ,
    CAT_ROLE_SYSTEM_OUTLINE	            = 0x23 ,
    CAT_ROLE_SYSTEM_OUTLINEITEM	        = 0x24 ,
    CAT_ROLE_SYSTEM_PAGETAB	            = 0x25 ,
    CAT_ROLE_SYSTEM_PROPERTYPAGE        = 0x26 ,
    CAT_ROLE_SYSTEM_INDICATOR	        = 0x27 ,
    CAT_ROLE_SYSTEM_GRAPHIC	            = 0x28 ,
    CAT_ROLE_SYSTEM_STATICTEXT	        = 0x29 ,
    CAT_ROLE_SYSTEM_TEXT	            = 0x2a ,
    CAT_ROLE_SYSTEM_PUSHBUTTON	        = 0x2b ,
    CAT_ROLE_SYSTEM_CHECKBUTTON	        = 0x2c ,
    CAT_ROLE_SYSTEM_RADIOBUTTON	        = 0x2d ,
    CAT_ROLE_SYSTEM_COMBOBOX	        = 0x2e ,
    CAT_ROLE_SYSTEM_DROPLIST	        = 0x2f ,
    CAT_ROLE_SYSTEM_PROGRESSBAR	        = 0x30 ,
    CAT_ROLE_SYSTEM_DIAL	            = 0x31 ,
    CAT_ROLE_SYSTEM_HOTKEYFIELD	        = 0x32 ,
    CAT_ROLE_SYSTEM_SLIDER	            = 0x33 ,
    CAT_ROLE_SYSTEM_SPINBUTTON	        = 0x34 ,
    CAT_ROLE_SYSTEM_DIAGRAM	            = 0x35 ,
    CAT_ROLE_SYSTEM_ANIMATION	        = 0x36 ,
    CAT_ROLE_SYSTEM_EQUATION	        = 0x37 ,
    CAT_ROLE_SYSTEM_BUTTONDROPDOWN	    = 0x38 ,
    CAT_ROLE_SYSTEM_BUTTONMENU	        = 0x39 ,
    CAT_ROLE_SYSTEM_BUTTONDROPDOWNGRID  = 0x3a ,
    CAT_ROLE_SYSTEM_WHITESPACE	        = 0x3b ,
    CAT_ROLE_SYSTEM_PAGETABLIST	        = 0x3c ,
    CAT_ROLE_SYSTEM_CLOCK	            = 0x3d ,
    CAT_ROLE_SYSTEM_SPLITBUTTON	        = 0x3e ,
    CAT_ROLE_SYSTEM_IPADDRESS	        = 0x3f ,
    CAT_ROLE_SYSTEM_OUTLINEBUTTON	    = 0x40 
};
#endif //CATGUIAccessDefs_H_