//---------------------------------------------------------------------------
/// \file CATPictureMulti.h
/// \brief Multiple pictures (like switch multi, but no mouse interaction)
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef _CATPictureMulti_H_
#define _CATPictureMulti_H_

#include "CATControl.h"

class CATPictureMulti;

/// \class CATPictureMulti
/// \brief Multiple pictures (like switch multi, but no mouse interaction)
/// \ingroup CATGUI
class CATPictureMulti : public CATControl
{
public:
    /// CATPictureMulti constructor (inherited from CATControl->CATXMLObject)
    /// \param element - Type name ("PictureMulti")
    /// \param attribs - attribute information for the window
    /// \param parent - parent XML object (should be a "Window" element)
    /// \param rootDir - root directory of skin for bin/png loads
    CATPictureMulti(  const CATString&             element, 
        const CATString&             rootDir);

    virtual ~CATPictureMulti();          

    /// Draw() draws the control into the parent's image
    /// \param image - parent's image to draw into
    /// \param dirtyRect - portion of control (in window coordinates)
    ///        that requires redrawing.
    virtual void Draw(CATImage* image, const CATRect& dirtyRect);

    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

	 virtual void SetValue(CATFloat32 newValue, bool sendCommand=true);
protected:
    CATUInt32				fNumImages;      
    std::vector<CATImage*>	fImageList;
    CATStack<CATImage*>	fMasterSet;
};

#endif // _CATPictureMulti_H_


