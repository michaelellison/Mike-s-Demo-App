/// \file  CATOverlay.cpp
/// \brief Base image overlay for drawing on top of games
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATOverlay.h"
#include "CATString.h"

#ifdef CAT_CONFIG_WIN32
#include <gl/gl.h>

CATOverlay::CATOverlay()
:CATIntercept()
{
    fOverlay         = 0;
    fOverlayDirty    = false;
    fRefScreenWidth  = 0;
    fRefScreenHeight = 0;
    fKeepAspect      = true;
}

CATOverlay::~CATOverlay()
{
    RestoreAll();
}

CATRect CATOverlay::GetRect()
{
    fLock.Wait();
    
    CATRect retRect = fRect;
    
    fLock.Release();    
    return retRect;
}

void CATOverlay::SetOverlayImage(   CATImage*      overlay, 
                                    const CATRect&  overlayRect,
                                    CATInt32        refScreenWidth,
                                    CATInt32        refScreenHeight,
                                    bool            keepAspect)
                                         
{
    fLock.Wait();
    
    fOverlay         = overlay;
    fOverlayDirty    = true;
    fOverlayRect     = overlayRect;
    fRefScreenWidth  = refScreenWidth;
    fRefScreenHeight = refScreenHeight;
    fKeepAspect      = keepAspect;
    fLock.Release();
}

#endif // CAT_CONFIG_WIN32
