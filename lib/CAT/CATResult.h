/// \file CATResult.h
/// \brief CAT Result definitions and utilities
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATResult_H_
#define _CATResult_H_

#include "CATTypes.h"
/// General 32-bit result code for use in the CAT library. 
/// High-bit set indicates error. 0 is success. Others are status codes,
/// but treated as successful by the CATSUCCEEDED macro.
///
/// We may want to retype this as a class later for an richer error handling
/// system.
typedef CATUInt32 CATResult;     

#define CATFAILED(x)    ( ((x) & 0x80000000) != 0 ) ///< Macro, returns true if result fails
#define CATSUCCEEDED(x) ( ((x) & 0x80000000) == 0 ) ///< Macro, returns true if result succeeds

// For now, we're just using raw error codes.  Eventually, we'll probably want a richer
// error/string handling class. These macros should make the transition a bit more friendly if
// we start using them now, then implement the additional features later.
//
// For example, CATRESULTFILE() might later be used to call an object's constructor to create
// a CATRESULT() object containing the result, the filename, and the file/line of the source
// that created it.

#define CATRESULT(x)                        (x)
#define CATRESULTFILE(x,filename)           (x)
#define CATRESULTFILEDESC(x,filename,desc)  (x)
#define CATRESULTDESC(x,desc)               (x)


/// \ingroup CAT
/// \todo 
/// Eventually, the CATResult codes need to be broken out into tables 
/// that are generated, with the associated strings loaded at runtime based 
/// on language.
///
/// Enumeration of error and status codes to be used as CATResult's.
enum 
{
    CAT_SUCCESS = 0,                    ///< Success
    //------------------------------------------------------------------------
    // Status codes
    CAT_STATUS            = 0x60008000, ///< Start of non-error statuses    
    CAT_STAT_CORE_LAST    = 0x60008FFF, ///< Last of status codes for lib
    CAT_STAT_APP          = 0x60009000, ///< First App status code
    CAT_STAT_APP_LAST     = 0x60009FFF, ///< Last reserved App status code
    
    //------------------------------------------------------------------------
    
    
    //------------------------------------------------------------------------
    // String IDs (non-status related)
    CAT_STRING         = 0x6000C000, ///< Start of string ids        
    CAT_STR_CORE_LAST  = 0x6000CFFF, ///< Last of allocated strings in core
    CAT_STR_APP        = 0x6000D000, ///< First App string code
    CAT_STR_APP_LAST   = 0x6000DFFF, ///< Last reserved app string code
    
    //------------------------------------------------------------------------


    //------------------------------------------------------------------------
    // Error codes
    CAT_ERROR             = 0xE0008000, ///< Start of error codes
    CAT_ERR_CORE_LAST     = 0xE0008FFF, ///< Last in core errors
    CAT_ERR_APP           = 0xE0009000, ///< First App error code
    CAT_ERR_APP_LAST      = 0xE0009FFF, ///< Last App error code
    
    //------------------------------------------------------------------------
};



#endif // _CATResult_H_