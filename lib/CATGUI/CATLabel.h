//---------------------------------------------------------------------------
/// \file CATLabel.h
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
#ifndef _CATLabel_H_
#define _CATLabel_H_

#include "CATControl.h"

class CATLabel;

/// \class CATLabel CATLabel.h
/// \brief Text label for GUI
/// \ingroup CATGUI
///
class CATLabel : public CATControl
{
public:
    /// CATLabel constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Label")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png load
    CATLabel(    const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATLabel();          

    /// IsFocusable() returns true if the control can receive
    /// focus, and false otherwise.
    virtual bool	     IsFocusable() const;

    /// Draw() draws the control into the parent's image
    /// \param image - parent image to draw into
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);


    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    virtual void SetString ( const CATString& text );
    virtual CATString GetString () const;

    /// Event handler
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_TEXT;}
#endif


protected:
    bool        fAllowClick;
};

#endif // _CATLabel_H_


