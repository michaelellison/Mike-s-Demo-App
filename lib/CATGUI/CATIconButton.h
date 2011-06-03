//---------------------------------------------------------------------------
/// \file CATIconButton.h
/// \brief Specialized pushbutton with image icon
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
#ifndef CATIconButton_H_
#define CATIconButton_H_

#include "CATButton.h"

/// \class CATIconButton CATIconButton.h
/// \brief Specialized pushbutton with image icon
/// \ingroup CATGUI
///
///
/// CATIconButtons are useful when you want a lot of buttons
/// to share the same basic look, with just the center
/// image changing between buttons.
///
class CATIconButton : public CATButton
{
    public:
        CATIconButton(   const CATString&             element, 
                         const CATString&             rootDir);

        virtual ~CATIconButton();          

        /// ParseAttributes() parses the known attributes for an object.
        virtual CATResult ParseAttributes();

        /// Draw() draws the control into the parent's image
        /// \param image - parent image to draw into
        /// \param dirtyRect - portion of control (in window coordinates)
        ///        that requires redrawing.
        virtual void   Draw(CATImage* image, const CATRect& dirtyRect);

    protected:
        CATImage*   fIconImage;
        CATImage*   fIconDisabled;
};

#endif // CATIconButton_H_


