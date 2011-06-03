//---------------------------------------------------------------------------
/// \file CATPicture.h
/// \brief Static picture for GUI
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
///
//---------------------------------------------------------------------------
#ifndef _CATPicture_H_
#define _CATPicture_H_

#include "CATControl.h"

class CATPicture;

/// \class CATPicture CATPicture.h
/// \brief Static picture for GUI
/// \ingroup CATGUI
class CATPicture : public CATControl
{
public:
    /// CATPicture constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("Picture")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png loads
    CATPicture(  const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATPicture();          

    /// IsFocusable() returns true if the control can receive
    /// focus, and false otherwise.
    virtual bool	     IsFocusable() const;

    /// Draw() draws the control into the parent's image
    /// \param image - parent's image to draw into
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

protected:

};

#endif // _CATPicture_H_


