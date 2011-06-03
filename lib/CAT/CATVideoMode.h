/// \file CATVideoMode.h
/// \brief Video Mode handling interface by Michael Ellison
/// \ingroup CAT
///
///
// The MIT License
//
// Copyright (c) 2008 by Michael Ellison
//
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATVIDEOMODE_H_
#define _CATVIDEOMODE_H_

#include <vector>
#include "CATVideoMode_Types.h"

/// \class CATVideoMode
/// \brief Video Mode handling for Win32
/// \ingroup CAT
class CATVideoMode
{
    public:
                        CATVideoMode();
        virtual         ~CATVideoMode();
        

        /// Initializes the video mode object.  Called automatically if not
        /// previously done or if the display information has become dirty
        /// from setting modes.
        ///
        /// If initialize is called more than once, it will uninitialize the
        /// object then re-initialize it.  All of the video mode information
        /// is gathered by this call and persisted, so it should be correct
        /// *at the time of this call*.
        ///
        /// You should call this if the display modes change from
        /// an external source to keep the information up to date.
        ///
        /// \param monitorModesOnly Set to true if you only want monitor
        ///                         supported (and reported) modes.
        ///                         If false, all modes the video card 
        ///                         handles are reported.
        ///                         Does NOT limit mode set requests.
        ///
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      Initialize(bool monitorModesOnly = true);

        /// Uninitializes the video mode object. Called on destruction if not
        /// already done.
        ///
        void            Uninitialize();

        /// Retrieves the number of displays
        /// \return int Number of displays
        CVMUInt32       GetNumberOfDisplays();
        
        /// Retrieves the current display mode for the specified display.
        ///
        /// \param  display     0-based display number
        /// \param  info        Pointer to information struct to fill
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      GetCurrentDisplayMode( 
                         CVMInt32                   display,
                         CVM_INFO_ADAPTER_CURRENT*  info);


        /// Sets the current display mode for the specified display.
        /// 
        /// Builds up a CVM_INFO struct and calls other SetDisplayMode().
        ///
        /// \param  display         0-based display number
        /// \param  width           Desired width in pixels
        /// \param  height          Desired height in pixels
        /// \param  bitsPerPixel    Bits per pixel
        /// \param  refreshRate     Refresh Rate in Hz
        /// \param  orient          Orientation
        /// \param  queueSet        If true, queues the change with the
        ///                         system.  A later call to RealizeDisplayModes
        ///                         will make them all take effect.
        /// \return CVM_RESULT      CVM_SUCCESS on success,
        ///                         CVM_STATUS_ALT_MODE if an alternative
        ///                         mode was set.
        CVM_RESULT      SetDisplayMode( CVMInt32   display,
                                        CVMInt32   width,
                                        CVMInt32   height,
                                        CVMInt32   bitsPerPixel = 0,
                                        CVMInt32   refreshRate  = 0,
                                        CVM_ORIENT orient       = CVM_ROT_ANY,
                                        bool       queueSet     = false);

        /// Sets the current display mode for the specified display.
        ///
        ///
        /// Implementation is platform specific. See CATVideoMode_Win32.cpp.
        ///
        /// \param  display     0-based display number
        /// \param  info        Pointer to display mode information
        /// \param  queueSet    If true, queues the change with the
        ///                     system.  A later call to RealizeDisplayModes
        ///                     will make them all take effect.
        /// \return CVM_RESULT  CVM_SUCCESS on success.  
        ///                     CVM_STATUS_ALT_MODE if an alternative 
        ///                     mode was set.
        CVM_RESULT      SetDisplayMode( CVMInt32   display,
                                        CVM_INFO*  info,
                                        bool       queueSet     = false);
                                        
        
        /// Makes previously set display modes that had the queueSet flag
        /// set to true take effect simultaneously.
        ///
        /// Implementation is platform specific. See CATVideoMode_Win32.cpp.
        ///
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      RealizeDisplayModes(int secondsTimeoutMax = 5);

        /// Retrieves modes that match the following requirements...
        ///
        /// \param  display       Display to retrieve modes for, or -1
        ///                       to retrieve modes for all displays.
        /// \param  modeList      List of modes that discovered modes will
        ///                       be added to.
        /// \param  width         Width to match, or 0 for all
        /// \param  height        Height to match, or 0 for all
        /// \param  refreshRate   Refresh freq to match, or 0 for all.
        /// \param  bitsPerPixel  Bits per pixel to match, or 0 for all.
        /// \param  orient        Orientation to match, or CVM_ROT_ANY for all.
        CVM_RESULT      GetModes( CVMInt32   display,
                                  std::vector<CVM_INFO_ADAPTER>&  modeList,
                                  CVMInt32   width        = 0,
                                  CVMInt32   height       = 0,
                                  CVMInt32   refreshRate  = 0,
                                  CVMInt32   bitsPerPixel = 0,
                                  CVM_ORIENT orient       = CVM_ROT_ANY);

        /// Retrieves the current display mode for the display at the 
        /// specified point.
        ///
        /// \param  x           Horizontal position for point
        /// \param  y           Vertical position for point
        /// \param  findOption  Determines what to return if point is outside
        ///                     of display.
        /// \param  info        On success, filled with the display
        ///                     information on return.
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      GetDisplayFromPoint  ( 
                         CVMInt32                   x, 
                         CVMInt32                   y,
                         CVM_FIND_OPTION            findOption,
                         CVM_INFO_ADAPTER_CURRENT*  info);

        /// Retrieves a sorted list of display(s) under the specified
        /// window. Displays containing the largest portion of the
        /// window will be sorted to the beginning of the list.
        ///
        ///
        /// Implementation is platform specific. See CATVideoMode_Win32.cpp.
        ///
        /// \param  window      Window handle 
        /// \param  info        On success, filled with the display
        ///                     information on return.
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      GetDisplaysFromWindow( 
                         CVM_WND                                 window,
                         std::vector<CVM_INFO_ADAPTER_CURRENT>&  displayList);

        /// Retrieves a sorted list of display(s) under the specified
        /// rectangle. Displays containing the largest portion of the
        /// rectangle will be sorted to the beginning of the list.
        ///
        /// \param  rect        Rectangle to check against displays
        /// \param  info        On success, filled with the display
        ///                     information on return.
        /// \return CVM_RESULT  CVM_SUCCESS on success.
        CVM_RESULT      GetDisplaysFromRect( 
                         CVMRECT                                 rect,
                         std::vector<CVM_INFO_ADAPTER_CURRENT>&  displayList);

        /// Retrieves the rectangle bounding the area covered by all displays.
        ///
        /// \return CVMRECT Rectangle bounding all displays.
        CVMRECT         GetBoundingDisplayRect();

        /// Checks if a point is within a specific display.
        ///
        /// \param  x           Horizontal position of point
        /// \param  y           Vertical position of point
        /// \param  displayInfo Ptr to display information
        /// \return bool        True if point is in the display.
        static bool PointInDisplay(       
                CVMInt32                  x, 
                CVMInt32                  y, 
                CVM_INFO_ADAPTER_CURRENT* displayInfo);

        /// Intersects the srcRect with the display rect.
        /// 
        /// \param  srcRect      Rect to intersect with display
        /// \param  intersect    Ptr to receive intersection, or NULL
        /// \param  displayInfo  Ptr to display information
        /// \return bool         True if rect intersects display
        static bool IntersectRectDisplay( 
                CVMRECT                   srcRect,
                CVMRECT*                  intersect,
                CVM_INFO_ADAPTER_CURRENT* displayInfo);
      
        /// Determines if the inside rect is fully contained
        /// within the display.
        ///
        /// \param  insideRect  Rectangle to check
        /// \param  displayInfo Ptr to display information
        /// \return True if the inside rect is entirely contained within
        ///         the display.
        static bool InsideDisplay(
                CVMRECT                   insideRect, 
                CVM_INFO_ADAPTER_CURRENT* displayInfo);

        /// Retrieve ptr to string for card brand
        static const CVMWChar* CardBrandToString(CVMCARDTYPE cardType);

        static CVMCARDTYPE CheckVendorId(const CVMWChar* devIdString);
    
    protected:
        /// Retrieve all displays and their modes into list.
        ///
        /// Platform specific. See CATVideoMode_Win32 for implementation.
        ///
        static void GetAllDisplays(
                std::vector<CVM_INFO_ADAPTER_CURRENT>& displayList,
                bool                                   monitorModes = true);
        
        /// Scan all modes on the adapter from info struct, add to mode list.
        ///
        /// Platform specific. See CATVideoMode_Win32 for implementation.
        ///
        static void GetAllModes(CVM_INFO_ADAPTER&              info, 
                                std::vector<CVM_INFO_ADAPTER>& modeList,
                                bool                           monitorModes = true);
    
    protected:        
        /// True if using only monitor-supported (and reported) modes.
        /// If false, all modes the video card handles are reported.
        ///
        /// Does NOT effect setting of display.
        bool                                       fMonitorModesOnly;

        /// True if class instance has been initialized and is currently
        /// clean (e.g. no video modes have been set since the last scan).
        bool                                       fInitializedAndClean; 

        /// Active Display information for each detected display.
        std::vector<CVM_INFO_ADAPTER_CURRENT>      fDisplays;    

        /// Vector of all known available modes on all displays.
        std::vector<CVM_INFO_ADAPTER>              fModes;
};

#endif // _CATVIDEOMODE_H_
