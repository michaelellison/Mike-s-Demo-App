//---------------------------------------------------------------------------
/// \file CATPlatform_Win32.cpp
/// \brief Platform-specific object creation (Win32)
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
#include "CATPlatform.h"

#include "CATFileSystem.h"
#include "CATFileSystem_Win32.h"
#include "CATMutex.h"

CATPlatform* gPlatform = 0;

//---------------------------------------------------------------------------
CATPlatform::CATPlatform()
{
}

CATPlatform::~CATPlatform()
{
}
//---------------------------------------------------------------------------
CATFileSystem* CATPlatform::GetFileSystem( const CATString& basePath )
{
   CATFileSystem* fileSystem = new CATFileSystem_Win32(basePath);
   CATASSERT(fileSystem != 0, "Failed to create filesystem!");
   return fileSystem;
}

//---------------------------------------------------------------------------
void CATPlatform::Release(CATFileSystem*& fileSystem)
{
   // Filesystems are reference counted for their child objects. Only
   // delete if count hits zero.
   if (fileSystem != 0)
   {
      delete fileSystem;
      fileSystem = 0;
   }
}

