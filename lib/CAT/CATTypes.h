/// \file CATTypes.h
/// \brief CAT Type definitions
/// \ingroup CAT
///
/// These types are meant to provide for an easy layer to abstract basic types
/// for later port efforts.  For a single platform, I know, it's a pain, but the
/// plan is to port the core to everywhere possible.
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $

#ifndef _CATTypes_H_
#define _CATTypes_H_

#include "CATConfig.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32_WCE
    #include <sys/stat.h>
#endif

#ifdef CAT_CONFIG_WIN32
    #ifndef _WIN32_WCE
        #include <io.h>
        #include <fcntl.h>
    #endif

    #include <memory.h>
    #include <string.h>

    #define CATFONT          HFONT
    #define CATWND           HWND
    #define CATDRAWCONTEXT   HDC
    #define CATOSCURSOR      HCURSOR
    #define CATINSTANCE      HINSTANCE
    #define CATOSIMAGE       HBITMAP
    #define CATOSREGION      HRGN
    #define CATICON          HICON

    // WIN32 / MSVC compatible definitions
    typedef unsigned __int64 CATUInt64;     ///< 64-bit unsigned integer
    typedef __int64          CATInt64;      ///< 64-bit signed   integer
    typedef unsigned __int32 CATUInt32;     ///< 32-bit unsigned integer
    typedef __int32          CATInt32;      ///< 32-bit signed   integer
    typedef unsigned __int16 CATUInt16;     ///< 16-bit unsigned integer
    typedef __int16          CATInt16;      ///< 16-bit signed   integer
    typedef unsigned __int8  CATUInt8;      ///< 8-bit  unsigned integer
    typedef __int8           CATInt8;       ///< 8-bit  signed   integer
    typedef wchar_t          CATWChar;      ///< 16-bit Character
    typedef char             CATChar;       ///< 8-bit  Character

    typedef double           CATFloat64;    ///< 64-bit Float
    typedef float            CATFloat32;    ///< 32-bit Float
    typedef bool             CATBool;       ///< Boolean


   // standardized standard i/o...
   #define xplat_isatty(x)        _isatty(x)
   #define xplat_fileno(x)        _fileno(x)
   #define xplat_fstat(x,y)       _fstati64(x,y)
   #define xplat_stat             _stat64
   #define xplat_write(x,y,z)     _write(x,y,z)
   #define xplat_fileno(x)        _fileno(x)
   #define xplat_readfunc(x,y,z)  _read(x,y,z)
   #define xplat_setmode(x,y)     _setmode(x,y)

   #define xplat_itoa(x,y,z)      _i64toa(x,y,z)

   #define xplat_lseek(x,y,z)     _lseeki64(x,y,z)
   #define xplat_open(x,y,z)      _open(x,y,z)
   #define xplat_close(x)         _close(x)
   #define xplat_commit(x)        _commit(x)

   #define xplat_IsRegularStream  _S_IFREG
   #define xplat_BinaryMode       _O_BINARY
   #define xplat_ReadOnly         _O_RDONLY
   #define xplat_ReadRights       _S_IREAD

   #define xplat_inline           _inline
   #define xplat_wcstoull(x,y,z)  _wcstoui64(x,y,z)
   #define xplat_vsnwprintf(w,x,y,z)   _vsnwprintf(w,x,y,z)
   #define xplat_vsnprintf            _vsnprintf
   #define xplat_ssize_t          CATUInt64

   #if _WIN32_WCE
        #define xplat_swprintf    _snwprintf
        #define xplat_wtof(x)          wcstod(x,0)
   #else
        #define xplat_wtof(x)          _wtof(x)
        #define xplat_swprintf    swprintf
   #endif
#else
   #include <unistd.h>
   #include <errno.h>
   #include <memory.h>
   #include <string.h>

/*
    #define CATFONT          HFONT
    #define CATWND           HWND
    #define CATDRAWCONTEXT   HDC
    #define CATOSCURSOR      HCURSOR
    #define CATINSTANCE      HINSTANCE
    #define CATOSIMAGE       HBITMAP
    #define CATOSREGION      HRGN
    #define CATICON          HICON
*/
   // standardized standard i/o...
   #define xplat_isatty(x)        isatty(x)
   #define xplat_fileno(x)        fileno(x)
   #define xplat_fstat(x,y)       fstat(x,y)
   #define xplat_stat             stat
   #define xplat_write(x,y,z)     write(x,y,z)
   #define xplat_fileno(x)        fileno(x)
   #define xplat_readfunc(x,y,z)  read(x,y,z)
   #define xplat_setmode(x,y)     (0)
   #define xplat_lseek(x,y,z)     lseek(x,y,z)
   #define xplat_close(x)         close(x)

   #define xplat_itoa(x,y,z)      itoa(x,y,z)

   #define xplat_IsRegularStream  S_IFREG
   #define xplat_BinaryMode       O_BINARY
   #define xplat_ReadOnly         O_RDONLY
   #define xplat_ReadRights       S_IREAD

   #define xplat_inline           inline
   #define xplat_wcstoull(x,y,z)  wcstoull(x,y,z)
   #define xplat_wtof(x)          wcstod(x,0)
   #define xplat_vsnwprintf(w,x,y,z)   vswprintf(w,x,y,z)
   #define xplat_vsnprintf             vsnprintf
   #define xplat_swprintf         swprintf

   #define xplat_ssize_t          ssize_t

    typedef UInt64         CATUInt64;     ///< 64-bit unsigned integer
    typedef SInt64          CATInt64;      ///< 64-bit signed   integer
    typedef UInt32         CATUInt32;     ///< 32-bit unsigned integer
    typedef SInt32         CATInt32;      ///< 32-bit signed   integer
    typedef UInt16         CATUInt16;     ///< 16-bit unsigned integer
    typedef SInt16          CATInt16;      ///< 16-bit signed   integer
    typedef UInt8          CATUInt8;      ///< 8-bit  unsigned integer
    typedef SInt8           CATInt8;       ///< 8-bit  signed   integer
    typedef wchar_t          CATWChar;      ///< 16-bit Character
    typedef char             CATChar;       ///< 8-bit  Character

    typedef double           CATFloat64;    ///< 64-bit Float
    typedef float            CATFloat32;    ///< 32-bit Float
    typedef bool             CATBool;       ///< Boolean
    // if EINTR is defined, assume low level i/o can be interrupted and must be retried.
    #if (defined(EINTR))
    static xplat_inline ssize_t xplat_read(int fd,void* buf, unsigned int count)
    {
       ssize_t rc;
       for (;;)
       {
          rc = xplat_readfunc(fd,buf,count);
          if (rc >= 0)
             return rc;
          if (EINTR != errno)
             return rc;
       }
    }
    // otherwise assume non-interrupted.
    #else
       #define xplat_read(x,y,z) xplat_readfunc(x,y,z)
    #endif
#endif


#pragma pack(push)
#pragma pack(1)
struct CATFileOffsetSplit
{
    CATUInt32 lowOffset;
    CATUInt32 highOffset;
};

union CATFileOffset
{
    CATInt64            qOffset;
    CATFileOffsetSplit  dOffset;
};
#pragma pack(pop)


/// Key modifiers for mouse movements.
enum
{
    CATMODKEY_NONE  = 0,
    CATMODKEY_SHIFT = 1,
    CATMODKEY_CTRL  = 2,
    CATMODKEY_ALT   = 4
};

typedef CATInt32 CATMODKEY; ///< Modifier keys for mouse/actions
class CATImage;
typedef void (*CATCaptureCB)(CATImage* image, void* context);


#endif // _CATTypes_H_
