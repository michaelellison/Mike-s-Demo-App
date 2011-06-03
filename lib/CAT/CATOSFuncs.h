//---------------------------------------------------------------------------
/// \file CATOSFuncs.h
/// \brief Functions that can be abstracted between platforms directly
/// \ingroup CAT
/// 
/// These functions have direct (or nearly direct) correlations between
/// platforms, so we can abstract them at a function level to keep our
/// classes cleaner.
///
/// Or at least I hope they have direct correlations.. if not, then
/// remove them and put them back in the class(es) that call them.
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATOSFuncs_H_
#define CATOSFuncs_H_

#include "CATTypes.h"
#include "CATResult.h"
#include "CATDebug.h"
#include "CATUtil.h"
#include "CATRect.h"
#include "CATString.h"

class CATWindow;
class CATImage;
/// CATInvalidateRect invalidates a rectangular region within a
/// window so that it is drawn on the next paint.
///
/// \param window - OS defined window to invalidate
/// \param rect   - Rectangle in window to mark as invalid.
void CATInvalidateRect( CATWND window,
                        const CATRect& rect );

/// CATPostQuit() posts a quit message to a GUI interface...
void CATPostQuit( CATInt32 exitCode);


/// CATExecute() executes a shell command. Be careful.
CATResult CATExecute(const CATWChar* shellCommand, CATWND wnd = 0);

CATResult OSLoadIconImage(const CATWChar* appPath,CATICON* image, CATString& appName);
CATResult OSGetWindowIcon(const CATWChar* windowName,CATICON* icon);

CATMODKEY GetModifierKeys();

CATRect GetPrimaryMonitorRect();

CATString GetInstallLoc(const CATWChar* uninstKey);

// Snags a value string from HKLM\\regString - full path to value, including value name.
CATString GetSoftwareReg(const CATWChar* regString);

// Implement these....................
// CATUnicodeToASCII
// CATASCIIToUnicode

#endif // CATOSFuncs_H_


