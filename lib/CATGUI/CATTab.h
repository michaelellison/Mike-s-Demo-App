//---------------------------------------------------------------------------
/// \file CATTab.h
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
//
//
//---------------------------------------------------------------------------
#ifndef CATTab_H_
#define CATTab_H_

#include "CATLayer.h"

/// \class CATTab CATTab.h
/// \brief GUI Layer object
/// \ingroup CATGUI
class CATTab : public CATLayer
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATTab(    const CATString&               element, 
        const CATString&               rootDir);

    virtual ~CATTab();

    virtual CATResult SetCurTab( CATUInt32 curTab );
    virtual CATUInt32  GetCurTab( );
    virtual CATUInt32  GetNumTabs();

    /// Load() loads the skin in
    virtual        CATResult Load(CATPROGRESSCB				progressCB   = 0,
        void*							progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);

    virtual void OnParentCreate();

    bool IsVisible(const CATGuiObj* object = 0) const;

protected:      
    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();
    virtual CATControl* HitTest    (  const CATPOINT& point);
    virtual void Draw             (  CATImage* image, const CATRect& dirtyRect);
    virtual void PostDraw         (  CATDRAWCONTEXT context, const CATRect& dirtyRect);

    /// OnEvent() is called when events occur that we should know about.
    virtual CATResult  OnEvent  (  const CATEvent&  event, CATInt32& retVal);

    virtual bool GetPostRects(CATStack<CATRect>& rectStack);

    CATUInt32 fCurTab;
};

#endif // CATTab_H_
