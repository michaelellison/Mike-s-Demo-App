//---------------------------------------------------------------------------
/// \file CATEditBox.h
/// \brief Text edit box for GUI
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
#ifndef _CATEditBox_H_
#define _CATEditBox_H_

#include "CATControlWnd.h"

const CATUInt32 kDefEditMaxLength = 255;

class CATEditBox;

/// \class CATEditBox CATEditBox.h
/// \brief Text edit box for GUI
/// \ingroup CATGUI
///
/// Edit boxes currently ignore their fValue - setting it to the value of the
/// string if applicable, but otherwise not really doing anything with it.
class CATEditBox : public CATControlWnd
{
public:
    CATEditBox(  const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATEditBox();

    virtual bool OnControlEvent( const CATEvent& event,  CATInt32& result);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    virtual void SetFocused(bool focused);
    virtual void OnEscapeChange();
    virtual void OnParentCreate();
    virtual void OnParentDestroy();

    /// GetValue() retrieves the value of the control.
    /// \return CATFloat32 - control value (0.0 - 1.0)
    virtual CATFloat32     GetValue() const;

    virtual void SetString(const CATString& newString);
    virtual CATString GetString() const;
    /// Event handler
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

    virtual CATString GetHint() const;

    virtual void OSSelectAll();

    virtual bool IsReadOnly();
    virtual void SetReadOnly(bool readOnly);

    virtual void OSSetReadOnly(bool readOnly);

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_TEXT;}
#endif

protected:
    bool        fNumbersOnly;
    bool        fReadOnly;
    CATUInt32     fMaxLength;

#ifdef _WIN32
    HFONT       fFont;
#endif
};

#endif // _CATEditBox_H_


