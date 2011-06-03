//---------------------------------------------------------------------------
/// \file CATKeystroke.h
/// \brief Generic keystroke information
/// \ingroup CAT
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
#ifndef _CATKeystroke_H_
#define _CATKeystroke_H_

#include "CATTypes.h"

/// Platform neutral key defs
enum CATKEY_SPECIAL
{
    CATKEY_NONE,

    CATKEY_LEFT,
    CATKEY_RIGHT,
    CATKEY_UP,
    CATKEY_DOWN,
    CATKEY_HOME,
    CATKEY_END,
    CATKEY_INSERT,
    CATKEY_DELETE,
    CATKEY_NEXT,
    CATKEY_PRIOR,
    CATKEY_F1,
    CATKEY_F2,
    CATKEY_F3,
    CATKEY_F4,
    CATKEY_F5,
    CATKEY_F6,
    CATKEY_F7,
    CATKEY_F8,
    CATKEY_F9,
    CATKEY_F10,
    CATKEY_F11,
    CATKEY_F12,

    //-----
    CATKEY_SPECIAL_COUNT
};

/// \class CATKeystroke
/// \brief generic keystroke class
/// \ingroup CAT
class CATKeystroke
{
public:
    CATKeystroke()
    {
        fKey = 0;
        fSpecial = CATKEY_NONE;
        fModKey =  CATMODKEY_NONE;
    }

    CATKeystroke(CATUInt32 key, CATKEY_SPECIAL special, CATMODKEY modKey)
    {
        fKey = key;
        fSpecial = special;
        fModKey = modKey;
    }

    ~CATKeystroke()
    {
    }

    // For overrides, just return fKey
    operator CATUInt32 () const
    {
        return (CATUInt32)fKey;
    }

    CATKeystroke& operator=(char theChar)
    {
        fKey = theChar;
        fSpecial = CATKEY_NONE;
        fModKey = CATMODKEY_NONE;
        return *this;
    }

    CATKeystroke& operator=(CATUInt32 theChar)
    {
        fKey = theChar;
        fSpecial = CATKEY_NONE;
        fModKey = CATMODKEY_NONE;
        return *this;
    }

    CATKeystroke& operator=(CATKEY_SPECIAL specialChar)
    {
        fKey = 0;
        fSpecial = specialChar;
        fModKey = CATMODKEY_NONE;
        return *this;
    }

    bool IsNormal() const
    {
        return (fKey != 0);
    }

    bool IsSpecial() const
    {
        return (fSpecial != CATKEY_NONE);
    }

    bool CtrlDown() const
    {
        return (fModKey & CATMODKEY_CTRL)?true:false;
    }

    bool AltDown() const
    {
        return  (fModKey & CATMODKEY_ALT)?true:false;
    }

    bool ShiftDown() const
    {
        return (fModKey & CATMODKEY_SHIFT)?true:false;
    }

    void Set(CATUInt32 key, CATKEY_SPECIAL special, CATMODKEY modKey)
    {
        fKey     = key;
        fSpecial = special;
        fModKey	= modKey;
    }

    CATKEY_SPECIAL GetSpecialKey() const
    {
        return fSpecial;
    }

    CATUInt32       GetNormalKey() const
    {
        return fKey;
    }

private:
    CATUInt32           fKey;
    CATKEY_SPECIAL      fSpecial;
    CATMODKEY			fModKey;
};

#endif // _CATKeystroke_H_


