//---------------------------------------------------------------------------
/// \file CATButton.h
/// \brief Momentary push button for GUI
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
#ifndef CATButton_H_
#define CATButton_H_

#include "CATControl.h"

/// \class CATButton CATButton.h
/// \brief Momentary push button for GUI
/// \ingroup CATGUI
class CATButton : public CATControl
{
    public:
        CATButton(   const CATString&             element, 
                     const CATString&             rootDir);

        virtual ~CATButton();          

        virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_PUSHBUTTON;}

    protected:
};

#endif // CATButton_H_


