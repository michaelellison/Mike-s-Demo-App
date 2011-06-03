/// \file CATDebug.h
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

#ifndef _CATDebug_H_
#define _CATDebug_H_

// Include our basic types
#include "CATTypes.h"

// Build reminders
#define __CATEXPSTRING__(x) #x
#define __CATSTRINGCONV__(x) __CATEXPSTRING__(x)
#define __CATLOCINFO__ __FILE__ "("__CATSTRINGCONV__(__LINE__)") : "

/// The CATREMINDER macro is used with pragma to print out a reminder 
/// during compile that you can click on from the Visual Studio 
/// interface to go to the code that generated it.  
///
/// It's useful for leaving todo's and such
/// in the code.  The __CAT*__ macros above it are to jump through
/// the hoops required to convert the __LINE__ macro into
/// a line number string.
///
/// Usage: pragma CATREMINDER("Fix this.")
#define CATREMINDER(x) message (__CATLOCINFO__ x)


// Compile out assertions and traces
#ifdef CAT_DEBUG
   /// CATASSERT() halts the program while running if the expression fails.
   /// 
   /// It differs from traditional assertions in that on some supported 
   /// platforms we're supporting ignoring them at runtime and try to 
   /// provide a nice interface.
   ///
   #define CATASSERT(exp,msg) \
          (void)( (exp) || (_CATAssert(#exp, __FILE__, __LINE__, msg), 0) )

   /// CATTrace() generates a trace message to the output debugging stream.
   /// In some applications, this will be a console window. Others, it may
   /// just be the debugger output or even a printf.
   #define CATTRACE(msg) _CATTrace(msg,__FILE__,__LINE__)
#else
   #define CATASSERT(exp,msg)           (void(0))
   #define CATTRACE(msg)                (void(0))
#endif

/// _CATAssert() is the root implementation of the CATASSERT() macro.
/// Do not use this directly - instead use CATASSERT().
void _CATAssert(   const char* expression,
                   const char* file,
                   CATUInt32   lineNumber,
                   const char* msg);

/// _CATTrace() is the root implementation of the CATTrace() macro.
/// Do not use this directly - instead use CATTrace()
void _CATTrace(    const char* msg, 
                   const char* file, 
                   CATUInt32   line);


#endif // _CATDebug_H_


