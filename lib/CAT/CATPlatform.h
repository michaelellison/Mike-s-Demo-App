//---------------------------------------------------------------------------
/// \file CATPlatform.h
/// \brief Platform-specific object creation
/// \ingroup CAT
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef CATPlatform_H_
#define CATPlatform_H_

#include "CATTypes.h"
#include "CATString.h"

class CATFileSystem;

/// \class CATPlatform
/// \brief Platform-specific object creation
/// \ingroup CAT
///
/// When a base class needs more than a few members that are platform-
/// specific, it can get downright fugly to use #ifdef.  CATPlatform tries
/// to solve this problem.
///
/// For such classes, create a pure virtual interface class.  Then, create a 
/// child class for each platform. Then, include the pure virtual class
/// in whatever source you need to create such an object in and include
/// CATPlatform.  When you need the object, acquire it from CATPlatform by
/// calling Get[ClassName]() - CATPlatform will create your object for you.
/// 
/// You can then use it using the pure interface. To free it, call 
/// CATPlatform::Release() on the object.
///
/// Note: Currently, most of the classes done this way are NOT reference
/// counted.  Instead, you get an actual new object each time, then it really
/// goes away when you call CATPlatform::Release().  Don't try tricky copying
/// stuff and expect the copy to work after you release the original or vise-
/// versa.  In the future, some objects may be made singleton or reference
/// counted here, however.  Your best bet is to call the Get* functions when
/// you need an object, and the Release() when you're done with it - leave
/// system-wide optimizations to the CATPlatform object.
///
/// Release() also sets the incoming referenced pointer to NULL to discourage
/// hanging pointer errors.
///
/// There is no CATPlatform.cpp.  Instead, for each platform, there is a 
/// seperate CATPlatform_PLATFORM.cpp file.  Only place the appropriate 
/// platform file in your project (or exclude the others from compile).
class CATPlatform
{
   public:
      CATPlatform();
      virtual ~CATPlatform();

      /// GetFileSystem() acquires the appropriate file system for the path.
      ///
      /// Call CATFileSystem::Initialize() on the returned file system
      /// before using it. Call CATPlatform::Release() on the object
      /// when done.
      ///      
      /// \param basePath - base path to start file system at.
      CATFileSystem* GetFileSystem(  const CATString&   basePath = "");      
      
      /// Release() function for CATFileSystem objects.
      /// \param fileSys - ref to ptr to CATFileSystem. Set to 0 on release.
      void          Release(CATFileSystem*& fileSys);
};

extern CATPlatform* gPlatform;

#endif // CATPlatform_H_
