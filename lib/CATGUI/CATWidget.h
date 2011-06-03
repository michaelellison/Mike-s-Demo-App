//---------------------------------------------------------------------------
/// \file CATWidget.h
/// \brief GUI Widget object - parent class for all CATGuiObject's below
///        CATWindow
/// \ingroup CATGUI
///
/// Copyright (c) 2004-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATWidget_H_
#define CATWidget_H_

#include "CATGuiObj.h"

/// \class CATWidget CATWidget.h
/// \brief GUI Widget object - parent class for all CATGuiObject's below
///        CATWindow
/// \ingroup CATGUI
class CATWidget : public CATGuiObj
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATWidget(    const CATString& element, 
                  const CATString& rootDir)
    : CATGuiObj(element,rootDir)
    {
    }

    virtual ~CATWidget()
    {
    }

    /// MarkDirty() marks the control as dirty (needing to be redrawn)
    ///
    /// \param dirtyRect - if specified, marks only part of the rectangle
    /// as dirty. Should be in window coordinates - i.e. 0,0 is top left in window,
    /// not control.
    /// \param force - if true, invalidates rect even if control is not
    /// visible.  This should be done when initially hiding a control.  The
    /// rest of the time though, if a control already wasn't visible, it
    /// shouldn't cause an invalidation of the area its in when it changes.
    virtual void MarkDirty(CATRect* dirtyRect = 0, bool force = false)
    {  
        if (!force)
        {
            if (this->IsVisible() == false)
            {
                return;
            }
        }

        CATRect invRect = fRect;

        if (dirtyRect)
        {
            invRect = *dirtyRect;

        }

        CATRect parRect = ((CATGuiObj*)this->GetParent())->GetRect();
        invRect.Offset( parRect.left, parRect.top );
        ((CATGuiObj*)this->GetParent())->MarkDirty(&invRect);
    }

    /// GetWindow() retrieves the parent window.
    virtual CATWindow*   GetWindow() const
    {
        return ((CATGuiObj*)fParent)->GetWindow();
    }

    virtual void PostDraw(CATDRAWCONTEXT context, const CATRect& dirtyRect) = 0;
    virtual void Draw(CATImage* image, const CATRect& dirtyRect)            = 0;

    /// OnParentCreate() is called when the parent window is created.
    /// 
    /// Most controls won't need this, but any that create their own
    /// windows should do so at this point.
    virtual void   OnParentCreate() = 0;

    /// OnParentDestroy() is called as the parent window is destroyed.
    ///
    /// Controls that create their own windows during OnParentCreate()
    /// should destroy them during OnParentDestroy()
    virtual void   OnParentDestroy() = 0;      
};

#endif // CATWidget_H_
