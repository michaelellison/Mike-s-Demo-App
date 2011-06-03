/// \file  CATOverlayOpenGL.h
/// \brief OpenGL image overlay
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATOverlayOpenGL_H_
#define _CATOverlayOpenGL_H_

#include "CATOverlay.h"
#ifdef CAT_CONFIG_WIN32
#include "CATImage.h"
#include "CATCritSec.h"

/// \class CATOverlayOpenGL
/// \brief OpenGL image Overlay
/// \ingroup CAT
class CATOverlayOpenGL : public CATOverlay
{
    public:
        CATOverlayOpenGL();
        virtual ~CATOverlayOpenGL();

        CATResult HookFunctions();

    protected:        
        void DrawToScene(   HDC dc );

        ///< OpenGL functions to intercept. 
        static CATINTERCEPT_DLL_TABLE_ENTRY kOpenGLInterceptTable[];       

        /// Hook function for glSwapBuffers() - called to display the back buffer.
        /// We draw our overlay immediate before the swap.
        static void     OnSwapBuffers    (  CATHOOK*             hookInst,
                                            HDC                  hdc);

        CATFloat32  fTexScaleX;         ///< Scale factor from texture to actual coordinates (X)            
        CATFloat32  fTexScaleY;         ///< Scale factor from texture to actual coordinates (Y)
        CATUInt32   fTexture;           ///< Texture ID containing image - remember is power of 2 for dimensions
        HMODULE     fOpenGLDLL;         ///< opengl32.dll module handle
};

#endif // CAT_CONFIG_WIN32
#endif // _CATOverlayOpenGL_H_