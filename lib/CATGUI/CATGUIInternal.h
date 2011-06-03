/// \file CATGUIInternal.h
/// \brief CAT GUI Internal primary include
/// \ingroup CATGUI
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATGUIINTERNAL_H_
#define _CATGUIINTERNAL_H_

#include "CATInternal.h"
#include "CATString.h"
#include "CATImage.h"
#include "CATXMLObject.h"
#include "CATCommand.h"
#include "CATEvent.h"
#include "CATStack.h"
#include "CATQueue.h"
#include "CATOSFuncs.h"

#include <commctrl.h>

// Generic progress callback for classes... not sure of a good place for this one..
typedef void (*CATPROGRESSCB)(CATFloat32 progress, const CATString& statString, void* userParam);

#endif // _CATGUIINTERNAL_H_