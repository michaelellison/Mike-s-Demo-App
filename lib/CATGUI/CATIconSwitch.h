//---------------------------------------------------------------------------
/// \file CATIconSwitch.h
/// \brief Icon-based On/Off switch for GUI
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
#ifndef _CATIconSwitch_H_
#define _CATIconSwitch_H_

#include "CATControl.h"
#include "CATSwitch.h"

class CATIconSwitch;

/// \class CATIconSwitch
/// \brief Icon-based On/Off switch for GUI
/// \ingroup CATGUI
class CATIconSwitch : public CATSwitch
{
public:
    /// CATIconSwitch constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Switch")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png loads
    CATIconSwitch(   const CATString&             element, 
                     const CATString&             rootDir);

    virtual ~CATIconSwitch();          

    /// Draw() draws the control into the parent's image
    /// \param image - parent image to draw into   
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

protected:
    CATImage*    fIconImage;
    CATImage*    fIconDisabled;
    CATImage*    fIconOff;
    CATImage*    fIconOffDisabled;    
};

#endif // _CATIconSwitch_H_


