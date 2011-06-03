/// \file  CATVideoMode_Types.h
/// \brief Type/struct definitions for CATVideoMode
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

#ifndef _CVM_TYPES_H_
#define _CVM_TYPES_H_

// Starting points of info and error codes.  Define these externally if you
// want to offset them to fit within your application's error structure.
// Make sure that the error codes have the high bit set, and the info codes
// do not.
#ifndef CVM_ERROR_START
    #define CVM_ERROR_START 0xe0000000
#endif

#ifndef CVM_INFO_START
    #define CVM_INFO_START  0x30000000
#endif

/// Result type used by CATVideoMode. High bit indicates error, other 
/// results are success/info.
//typedef unsigned __int32 CVM_RESULT;

enum CVM_RESULT
{
    CVM_SUCCESS = 0,                ///< Success

    CVM_STATUS = CVM_INFO_START,    ///< Status code
    CVM_STATUS_ALT_MODE,            ///< Alternative mode was set
    CVM_STATUS_LINKED,              ///< Indicates that display is linked
                                    ///< to another display.
    
    CVM_ERROR = CVM_ERROR_START,    ///< Error code
    CVM_ERR_INVALID_PARAM,          ///< Invalid parameter
    CVM_ERR_INVALID_DISPLAY,        ///< Invalid display number
    CVM_ERR_CANNOT_BREAK_LINK,      ///< Channels could not be unlinked.
    CVM_ERR_NO_MATCHING_DISPLAY,    ///< No matching display found.
    CVM_ERR_INVALID_WND,            ///< Invalid window
    CVM_ERR_SET_MODE,               ///< General error setting mode.
    CVM_ERR_NVIDIA_WONT_ROTATE_BELOW_1024_768, ///< NVidia cards don't like 
                                    ///< rotating below 1024x768    
    CVM_ERR_SET_MODE_DUAL_VIEW,     ///< Set failed because display in DualView
    CVM_ERR_SET_MODE_BAD_FLAGS,     ///< Set failed - bad flags
    CVM_ERR_SET_MODE_INVALID_MODE,  ///< Set failed - invalid mode requested
    CVM_ERR_SET_MODE_BAD_PARAM,     ///< Set failed - bad parameter
    CVM_ERR_SET_MODE_FAILED,        ///< Set failed.
    CVM_ERR_SET_MODE_REGISTRY,      ///< Set failed - unable to write to reg
    CVM_ERR_SET_MODE_REBOOT,         ///< Reboot required for set.
    
    CVM_ERR_LAST = 0x7fffffff
};

// Types in case someone ports to a different platform.
typedef __int32          CVMInt32;     ///< 32-bit signed integer
typedef unsigned __int32 CVMUInt32;    ///< 32-bit unsigned integer
typedef wchar_t          CVMWChar;     ///< Wide char type 


/// Inline function to allow simple checking of success.
_inline bool CVM_FAILED(CVM_RESULT result) 
{
    return (result & 0x80000000)?true:false;
}
/// CVMMin returns the minimum of two values. Types must be the same.
template<class T>
inline T const& CVMMin(T const& a, T const& b)
{
   // return minimum
   return a < b ? a : b;
}

/// CVMMax returns the maximum of two values. Types must be the same.
template<class T>
inline T const& CVMMax(T const& a, T const& b)
{
   // return maximum
   return a > b ? a : b;
}

/// CVMSwap swaps to values of the same type
template<class T>
inline void CVMSwap(T& a, T& b)
{
   T c = a;
   a = b;
   b = c;
}


enum CVM_ORIENT
{
    CVM_ROT_ANY  = 0,   ///< any rotation
    CVM_ROT_NONE = 1,   ///< No display rotation
    CVM_ROT_90   = 2,   ///< 90 degree display rotation
    CVM_ROT_180  = 3,   ///< 180 degree display rotation
    CVM_ROT_270  = 4,   ///< 270 degree display rotation
};

/// Used when searching for a monitor
enum CVM_FIND_OPTION
{
    CVM_DEFAULT_NULL,      ///< If search criteria do not match a monitor
                           ///< returns null.
    CVM_DEFAULT_NEAREST,   ///< Returns nearest monitor to search criteria.
    CVM_DEFAULT_PRIMARY    ///< Returns primary monitor if search fails.
};

/// Mode settings for a display
const CVMInt32 kCVM_ADAPTER_MAX    = 260;
const CVMInt32 kCVM_DISPLAY_MAX    = 260;
const CVMInt32 kCVM_VER_MAX        = 64;
const CVMInt32 kCVMVersion_Current = 1;

/// Generic window pointer - HWND on Windows.
typedef void* CVM_WND;

#pragma pack(push)
#pragma pack(1)
/// These properties can be directly set from the interface and are
/// viable on any display (although not all values/combinations 
/// will be supported).
///
/// (Generic level - used for setting modes)
struct CVM_INFO
{
    CVMInt32    CVMVersion;              ///< CVM_INFO Version
    CVMInt32    DisplayNumber;           ///< Display number
    bool        Primary;                 ///< True if primary display.

    CVMInt32    WidthPixels;             ///< Width of current mode
    CVMInt32    HeightPixels;            ///< Height of current mode
    CVMInt32    BitsPerPixel;            ///< Bits per pixel
    CVMInt32    RefreshRateHz;           ///< Refresh rate in Hz
    CVM_ORIENT  Orientation;             ///< Orientation 
};

enum CVMCARDTYPE
{
    CVMCARD_UNKNOWN = -1, ///< Unknown card type. Uused internally when 
                          ///< identifying: should be GENERIC in returned data.

    CVMCARD_GENERIC = 0,  ///< Generic card, uses MS method for rot if any

    CVMCARD_NVIDIA,       ///< NVidia. Supports rotation, but not through MS.
    CVMCARD_ATI,          ///< ATI. Supports rotation, need to figure out how.
    CVMCARD_MATROX,       ///< Matrox. ?
    CVMCARD_INTEL,        ///< Uses MS method for rot, but has other issues
    CVMCARD_VIA,          ///< VIA. Not sure yet.
    CVMCARD_S3,           ///< S3.  "" 
};

/// CVM_INFO_ADAPTER contains all the setable information from 
/// CVM_INFO, plus additional information regarding the specific
/// adapter and mode.
///
/// (Machine-specific level)
struct CVM_INFO_ADAPTER : public CVM_INFO
{
    CVMWChar    DisplayName[kCVM_DISPLAY_MAX+1]; ///< Name of the display (system)
    CVMWChar    AdapterName[kCVM_ADAPTER_MAX+1]; ///< Name of the adapter
    CVMWChar    DisplayType[kCVM_DISPLAY_MAX+1]; ///< Name of the display (friendly)

    CVMWChar    DriverVersion[kCVM_VER_MAX+1];   ///< Version string
    
    bool        LinkedDisplay;       ///< True if linked (need more detail)
    bool        RawModeList;         ///< True if from raw mode list
                                     ///< (might be supported, might not)
    CVMCARDTYPE CardBrand;           ///< Brand of card / driver type.
};

/// CVM_INFO_ADAPTER_CURRENT contains all the information
/// regarding a video mode and machine-specific adapter information,
/// plus information specific to the current mode and configuration
/// of the displays.
///
/// (Active settings - specific level)
struct CVM_INFO_ADAPTER_CURRENT : public CVM_INFO_ADAPTER
{
    CVMInt32    BitPlanes;       ///< Color/Bit planes (0 if unknown)
    CVMInt32    Left;            ///< Leftmost position
    CVMInt32    Top;             ///< Topmost position
    CVMInt32    WorkLeft;        ///< Leftmost of work area
    CVMInt32    WorkTop;         ///< Topmost of work area
    CVMInt32    WorkWidth;       ///< Width of work area
    CVMInt32    WorkHeight;      ///< Height of work area
};

/// Basic packed rectangle struct 
struct CVMRECT
{
   CVMInt32 left;   ///< Leftmost position of rectangle
   CVMInt32 top;    ///< Topmost position
   CVMInt32 right;  ///< Right
   CVMInt32 bottom; ///< Bottom
};

#pragma pack(pop)

#endif //_CVM_TYPES_H_
