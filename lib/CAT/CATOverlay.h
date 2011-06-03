/// \file  CATOverlay.h
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

#ifndef _CATOverlay_H_
#define _CATOverlay_H_

#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32
#include "CATImage.h"
#include "CATCritSec.h"

/// \class CATOverlay
/// \brief OpenGL image Overlay
/// \ingroup CAT
class CATOverlay : public CATIntercept
{
    public:
        CATOverlay();
        virtual ~CATOverlay();

        /// Retrieve the screen rect available for the overlay.
        CATRect   GetRect();

        /// Sets an image to overlay the display.  
        ///
        /// OverlayRect is the position of the overlay within the display.
        /// refScreenWidth and refScreenHeight are the virtual size of the display
        /// used to position the overlayRect onto the actual screen.
        ///
        /// So, if you want, you can give it a 640x480 rect, with the ref width/height
        /// of 640x480, and it will fill the screen regardless of the screen's actual
        /// size.
        ///
        /// Note that currently the image that is provided is assumed to be persistent
        /// until another image is sent (or null is passed for overlay). The overlay does 
        /// not create a copy, but will use it asynchronously during the next draw.  If you 
        /// are using for dynamic content, keep the old image around until *after* you set 
        /// a new overlay image.
        ///
        /// \param overlay          Image to overlay onto the screen, or 0 to reset to none.
        /// \param overlayRect      Virtual position/size of the overlay to draw on the screen.
        /// \param refScreenWidth   Virtual width of display, used to scale overlay to actual size.
        /// \param regScreenHeight  Virtual height of the display.
        void      SetOverlayImage(  CATImage*       overlay, 
                                    const CATRect&  overlayRect,
                                    CATInt32        refScreenWidth,
                                    CATInt32        refScreenHeight,
                                    bool            keepAspect = true);
        

    protected:        
        CATCritSec  fLock;                ///< Critical section for image/rect
        CATRect     fRect;                ///< Rect of full display area
        CATImage*   fOverlay;             ///< Overlay image
        CATRect     fOverlayRect;         ///< Virtual rect of overlay
        CATInt32    fRefScreenWidth;      ///< Virtual width of overlay display
        CATInt32    fRefScreenHeight;     ///< Virtual height of overlay display
        bool        fOverlayDirty;        ///< Overlay has changed since last render
        bool        fKeepAspect;
};

#endif // CAT_CONFIG_WIN32
#endif // _CATOverlay_H_