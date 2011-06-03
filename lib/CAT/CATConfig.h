/// \file CATConfig.h
/// \brief CAT Configuration header
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATConfig_H_
#define _CATConfig_H_

#ifdef _DEBUG
    // Define for debug mode
    #define CAT_DEBUG
#endif


// Global platform headers here if applicable
#ifdef _MSC_VER    
    #include <windows.h>
    // Define for Win32 platform (currently, only platform defined)
    #define CAT_CONFIG_WIN32    
    // Define to use wide characters for console operations
    #define CAT_CONFIG_CONSOLE_WIDE    

    #define CAT_LITTLE_ENDIAN
    // CAT_BIG_ENDIAN
    #define CAT_PATHSEPERATOR       '\\'
    #define CAT_DRIVESEPERATOR      ':'
    #define CAT_OPTPATHSEPERATOR    '/'
    #define CAT_EXTSEPERATOR        '.'
#else
    #include <ConditionalMacros.h>
    #include <MacTypes.h>
    #define CAT_LITTLE_ENDIAN
    #define CAT_PATHSEPERATOR       '/'
    #define CAT_DRIVESEPERATOR      '\0'
    #define CAT_OPTPATHSEPERATOR    '/'
    #define CAT_EXTSEPERATOR        '.'
#endif


#endif // _CATConfig_H_
