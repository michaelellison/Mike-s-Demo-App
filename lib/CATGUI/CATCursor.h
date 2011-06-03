/// \file  CATCursor.h
/// \brief Cursor class for GUI
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATCursor_H_
#define _CATCursor_H_

#include "CATInternal.h"

/// Cursor types
enum CATCURSORTYPE
{
    CATCURSOR_HIDE,
    CATCURSOR_NOACTION,
    CATCURSOR_ARROW,
    CATCURSOR_WAIT,
    CATCURSOR_TEXT,
    CATCURSOR_HAND,
    CATCURSOR_LEFTRIGHT,
    CATCURSOR_TOPBOTTOM,
    CATCURSOR_SIZE,
    CATCURSOR_MOVE,
    //------------
    CATCURSOR_NUMCURSORS
};

/// \class CATCursor
/// \brief Cursor class for GUI
/// \ingroup CATGUI
///
/// Right now, this mainly just a shell for cursor types. 
/// Eventually, this should be expanded to include custom cursors from
/// skins and the like.
class CATCursor
{
public:
    /// CATCursor constructor - defaults to using the standard
    /// arrow cursor.
    CATCursor(CATCURSORTYPE type = CATCURSOR_ARROW);

    /// CATCursor() destructor
    virtual ~CATCursor();

    /// SetType() sets the type of cursor to be used.
    ///
    /// \param type - The identifier for the cursor (see CATCURSORTYPE)
    /// \sa CATCURSORTYPE
    void           SetType(CATCURSORTYPE type);

    /// GetType() retrieves the cursor type.
    /// \sa CATCURSORTYPE
    CATCURSORTYPE   GetType();

    /// GetOSCursor() retrieves an OS-specific handle for the
    /// cursor.
    ///
    /// For Windows, this will be an HCURSOR.  
    ///
    /// Currently, no release is required - the class should
    /// delete any created cursors on destruction if necessary.
    CATOSCURSOR     GetOSCursor();

    /// ShowCursor() shows the cursor
    void           ShowCursor();

    /// HideCursor() hides the cursor
    void           HideCursor();
protected:
    /// fType holds the current cursor type
    CATCURSORTYPE   fType;

    /// fHidden holds the current cursor state - true if hidden, false otherwise
    bool           fHidden;
};

#endif // _CATCursor_H_


