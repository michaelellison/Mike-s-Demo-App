/// \file CATDebug.cpp
/// \brief Debugging utility functions
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#include "CATDebug.h"

#include <stdio.h>
#include <time.h>

// Right now, these are win32 only.
#ifdef CAT_CONFIG_WIN32
    
    // _CATAssert() is the root implementation of the CATASSERT() macro.
    // Do not use this directly - instead use CATASSERT().
    void _CATAssert(   const char*   expression,
                       const char*   file,
                       CATUInt32     lineNumber,
                       const char*   msg)
    {
       DWORD lastError = ::GetLastError();

       _CATTrace(msg,file,lineNumber);

       char* msgString = new char[_MAX_PATH + 100 + strlen(expression) + strlen(msg)];
       sprintf(msgString,"%s(%d) :%s\nExpression: %s",file,lineNumber,msg,expression);
#ifndef _WIN32_WCE
       DWORD result = ::MessageBoxA(NULL,msgString,"Assertion Failed",MB_ABORTRETRYIGNORE);
#else
       DWORD result = IDRETRY;
#endif
       delete [] msgString;

       switch (result)
       {
          case IDRETRY:
             // If you hit here, an assertion occurred.  Check the tracelogs for the reason.
             // You should be able to trace back to the location of the assertion by stepping
             // out of this function in your debugger.
              DebugBreak();
             //_asm {int 3};
             break;
          case IDIGNORE:
             // Ignoring assertion and continuing.
             break;
          
          default:
          case IDABORT:
             // Halt program.
             _exit(-1);
             break;
       }
       
       SetLastError(lastError);
    }


    // _CATTrace() is the root implementation of the CATTrace() macro.
    // Do not use this directly - instead use CATTrace()
    void _CATTrace(    const char*     msg, 
                       const char*     file, 
                       CATUInt32       lineNumber)
    {
       DWORD lastError = ::GetLastError();      
       
       char* msgString = new char[_MAX_PATH + 100 + strlen(msg)];
       sprintf(msgString,"%s(%d) :%s\n",file,lineNumber,msg);
#ifndef _WIN32_WCE
       ::OutputDebugStringA(msgString);
#else
       printf(msgString);
#endif
       
       delete [] msgString;
       SetLastError(lastError);
    }
#else
#endif