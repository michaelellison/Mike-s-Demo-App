//---------------------------------------------------------------------------
/// \file CATLayer.h
/// \brief GUI Layer object - contains sub-controls
/// \ingroup CATGUI
///
/// Copyright (c) 2004-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef _CATLayer_H_
#define _CATLayer_H_

#include "CATWidget.h"

class CATLayer : public CATWidget
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATLayer(    const CATString&               element, 
        const CATString&               rootDir);

    virtual ~CATLayer();

    virtual void   SetEnabled(bool enabled);
protected:      
    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    virtual CATControl* HitTest    (  const CATPOINT& point);
    virtual void Draw             (  CATImage* image, const CATRect& dirtyRect);
    virtual void PostDraw         (  CATDRAWCONTEXT context, const CATRect& dirtyRect);
    virtual void OnParentCreate   (  );
    virtual void OnParentDestroy  (  );      

};

#endif // _CATLayer_H_
