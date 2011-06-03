//---------------------------------------------------------------------------
/// \file CATButton.cpp
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
#include "CATButton.h"
#include "CATWindow.h"
//---------------------------------------------------------------------------
/// CATButton constructor (inherited from CATControl->CATXMLObject)
/// \param element - Type name ("Button")
/// \param rootDir - root directory of skin
//---------------------------------------------------------------------------
CATButton::CATButton(  const CATString&             element, 
                       const CATString&             rootDir)
: CATControl(element, rootDir)
{
    fValue = 1.0f;
}



//---------------------------------------------------------------------------
/// CATButton destructor
//---------------------------------------------------------------------------
CATButton::~CATButton()
{
}

