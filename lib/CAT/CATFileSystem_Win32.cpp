//---------------------------------------------------------------------------
/// \file CATFileSystem_Win32.cpp
/// \brief File system functions for Win32 platform
/// \ingroup CAT
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#include <memory.h>

#include "CATFileSystem_Win32.h"
#include "CATStreamFile.h"
#include "CATStreamRAM.h"
#include "CATPlatform.h"
// Some versions of wince don't define.
#ifndef INVALID_FILE_ATTRIBUTES
	#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif
//---------------------------------------------------------------------------
CATFileSystem_Win32::CATFileSystem_Win32(const CATString& basePath)
:CATFileSystem(basePath)
{

}

//---------------------------------------------------------------------------
CATFileSystem_Win32::~CATFileSystem_Win32()
{

}

//---------------------------------------------------------------------------
// Initialize() must be called prior to using CATFileSystem!
// \return CATResult - CAT_SUCCESS
//
// For CATFileSystem_Win32, we don't really use this right now.
//---------------------------------------------------------------------------
CATResult CATFileSystem_Win32::Initialize()
{
   CATResult result = CAT_SUCCESS;
   fFSLock.Wait();
   

   fFSLock.Release();
   return result;
}

//---------------------------------------------------------------------------
// FileExists should return a successful result if the file exists,
// or an error otherwise.
//
// Note: FileExists() fails if a directory of that name is present.
//
// \param pathname - path to file to check for existance.
// \return CATResult - successful result if the file is found.
CATResult CATFileSystem_Win32::FileExists(  const CATString& pathname   )
{
   CATString fullPath = BuildPath(fBasePath,pathname);

   DWORD attribs = ::GetFileAttributes(fullPath);
   if (INVALID_FILE_ATTRIBUTES == attribs)
   {
      return CATRESULTFILE(CAT_ERR_FILE_DOES_NOT_EXIST,pathname);
      
   }

   if (attribs & FILE_ATTRIBUTE_DIRECTORY)
   {
      return CATRESULTFILE(CAT_ERR_FILE_IS_DIRECTORY,pathname);
   }

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// DirExists should return a successful result if the dir exists,
// or an error otherwise.
//
// Note: DirExists() fails if a file of the specified name exists.
//
// \param pathname - path to dir to check for existance.
// \return CATResult - successful result if the file is found.
CATResult CATFileSystem_Win32::DirExists (  const CATString& pathname   )
{
   CATString fullPath = BuildPath(fBasePath,pathname);

   DWORD attribs = ::GetFileAttributes(fullPath);
   if (INVALID_FILE_ATTRIBUTES == attribs)
   {
      return CATRESULTFILE(CAT_ERR_DIR_DOES_NOT_EXIST,fBasePath);
   }

   if (!(attribs & FILE_ATTRIBUTE_DIRECTORY))
   {
      return CATRESULTFILE(CAT_ERR_DIR_IS_FILE,fBasePath);
   }

   return CAT_SUCCESS;
}


/// CreateDir creates the directory if necessary.
///
/// \param pathname - path to dir to check for existance and create if not
///                   present.
/// \return CATResult - CAT_SUCCESS if successful.
CATResult CATFileSystem_Win32::CreateDir(  const CATString& pathname   )
{
   CATUInt32 offset = 0;

   if (pathname.IsEmpty())
   {
      return CATRESULT(CAT_ERR_NULL_PARAM);
   }
   
   while (offset < pathname.LengthCalc())
   {
      // Check for full path - return if it exists.
      if (CATSUCCEEDED(DirExists(pathname)))
         return CAT_SUCCESS;

      CATString curPath = pathname;
      
      if (pathname.Find(CAT_PATHSEPERATOR,offset))
      {
         CATString curPath = pathname.Left(offset);
         offset++;
      }
      else
      {
         offset = pathname.LengthCalc();
      }

		if (curPath.IsEmpty())
		{
		   return CATRESULT(CAT_ERR_FILESYSTEM_CREATE_DIR);
		}
#ifdef kDRIVESEPERATOR
      if ((curPath.GetWChar(curPath.LengthCalc() - 1) != kDRIVESEPERATOR) && 
          (CATFAILED(DirExists(curPath))))
#else
      if (CATFAILED(DirExists(curPath)) )
#endif
      {
         CATString fullPath = BuildPath(fBasePath,curPath);
         if (!::CreateDirectory(fullPath,0))
         {
            return CATRESULT(CAT_ERR_FILESYSTEM_CREATE_DIR);
         }
      }
   } 

   return DirExists(pathname);
}

//---------------------------------------------------------------------------
// PathExists should return a successful result if a dir or a file
// of that name exists.
//
// If it is a file, returns CAT_STAT_PATH_IS_FILE.
// IF it is a dir, returns CAT_STAT_PATH_IS_DIRECTORY
//
// Note: DirExists() fails if a file of the specified name exists.
//
// \param pathname - path to dir to check for existance.
// \return CATResult - successful result if the file is found.
CATResult CATFileSystem_Win32::PathExists(  const CATString& pathname   )
{
   CATString fullPath = BuildPath(fBasePath,pathname);

   DWORD attribs = ::GetFileAttributes(fullPath);
   if (INVALID_FILE_ATTRIBUTES == attribs)
   {
      return CATRESULTFILE(CAT_ERR_PATH_DOES_NOT_EXIST,fBasePath);
   }
   
   if (attribs & FILE_ATTRIBUTE_DIRECTORY)
      return CAT_STAT_PATH_IS_DIRECTORY;
   
   return CAT_STAT_PATH_IS_FILE;   
}

//---------------------------------------------------------------------------
// FindFirst() finds the first matching file or directory and
// returns it in firstFile.  
//
// FindFirst *must* be called prior to FindNext().  Only one
// search at a time is allowed.
// \param searchMAsk - mask for performing searches with
// \param firstFile - ref to a string that receives the filename
//                    on success.
// \param findHandle - ref to handle returned on success. 
// \result CATResult - if the file is a directory, returns
// CAT_STAT_PATH_IS_DIRECTORY.  If the file is a normal
// file, then CAT_STAT_PATH-IS-FILE is returned instead.
CATResult CATFileSystem_Win32::FindFirst (  const CATString& searchPath, 
                                          CATString&       firstFile,
                                          CATFINDHANDLE&   findHandle)
                                         
{
   fFSLock.Wait();
   CATString fullPath = BuildPath(fBasePath,searchPath);

   firstFile = "";
   findHandle = 0;

   WIN32_FIND_DATA findData;
   memset(&findData,0,sizeof(findData));
   findHandle = ::FindFirstFile(fullPath,&findData);
   
   if (INVALID_HANDLE_VALUE == findHandle)
   {
      findHandle = 0;
      fFSLock.Release();
      return CATRESULTDESC(CAT_ERR_FIND_NO_MATCHES, fullPath);
   }

   bool gotFile = true;

   CATString testFile = findData.cFileName;
   
   while ((gotFile) && ((testFile.Compare(".") == 0) || (testFile.Compare("..") == 0)))
   {      
      gotFile = ::FindNextFile(findHandle,&findData)?true:false;
      if (gotFile)
      {
         testFile = findData.cFileName;
      }
   }

   if (!gotFile)
   {
      ::FindClose(findHandle);
      findHandle = 0;
      fFSLock.Release();
      return CATRESULTDESC(CAT_ERR_FIND_NO_MATCHES,fullPath);
   }

   CATString searchDir;   
   CATString searchMask;
   
   fullPath = RemoveBasePath(fullPath);
   
   SplitPath( fullPath,searchDir,searchMask,true);

   CATResult result = CAT_SUCCESS;   

   CATString tmpDir = searchDir;
   fFindPaths.insert(std::make_pair<CATFINDHANDLE,CATString>(findHandle,tmpDir));

   firstFile = BuildPath(searchDir, findData.cFileName);
   
   fFSLock.Release();

   if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return CAT_STAT_PATH_IS_DIRECTORY;

   return CAT_STAT_PATH_IS_FILE;
}      

//---------------------------------------------------------------------------
// FindNext() finds the next matching file or directory and
// returns it in firstFile.  
CATResult CATFileSystem_Win32::FindNext  (  CATString&       nextFile,
                                          CATFINDHANDLE   findHandle)
{
   WIN32_FIND_DATA findData;
   memset(&findData,0,sizeof(findData));

   nextFile = "";

   if (findHandle == 0)
   {
      CATASSERT(false,"You must call find first before find next...");
      return CATRESULT(CAT_ERR_FIND_CALL_FINDFIRST);
   }

   // Bail if no more matches.
   if (! ::FindNextFile(findHandle, &findData))
   {
      return CATRESULT(CAT_ERR_FIND_END);
   }

   bool gotFile = true;

   CATString testFile = findData.cFileName;
   
   while ((gotFile) && ((testFile.Compare(".") == 0) || (testFile.Compare("..") == 0)))
   {      
      gotFile = ::FindNextFile(findHandle,&findData)?true:false;
      if (gotFile)
      {
         testFile = findData.cFileName;
      }
   }

   if (!gotFile)
   {
      return CATRESULT(CAT_ERR_FIND_END);
   }

   fFSLock.Wait();

   CATString searchDir;
   std::map<CATFINDHANDLE,CATString>::iterator iter =  fFindPaths.find(findHandle);
   if (iter == fFindPaths.end())
   {
      fFSLock.Release();
      return CATRESULT(CAT_ERR_FIND_CALL_FINDFIRST);
   }
   searchDir = iter->second;

   fFSLock.Release();

   nextFile = BuildPath(searchDir, findData.cFileName);
   
   if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return CAT_STAT_PATH_IS_DIRECTORY;

   return CAT_STAT_PATH_IS_FILE;
}

// FindEnd() ends a find operation and performs any necessary cleanup.
//
// The handle will be set to 0.
//
// \param findHandle - handle of find from FindFirst()
// \return CATResult - CAT_SUCCESS on success.
// \sa FindFirst(), FindNext()
CATResult CATFileSystem_Win32::FindEnd (CATFINDHANDLE& findHandle)
{
   fFSLock.Wait();

   if (findHandle != 0)
   {
      CATString path;
      std::map<CATFINDHANDLE,CATString>::iterator iter = fFindPaths.find(findHandle);
      if (iter == fFindPaths.end())
      {
          CATASSERT(false,"Find handle not found in path tree.");
      }
      else
      {
          fFindPaths.erase(iter);
      }
      ::FindClose(findHandle);
   }
   findHandle = 0;

   fFSLock.Release();

   return CAT_SUCCESS;
}


//---------------------------------------------------------------------------
// OpenFile() opens or creates a file.
//
// \param filepath - path to file. 
// \param mode - open mode for the file      
// \param CATStream*& - ref to receive opened file stream
// \return CATResult - CAT_SUCCESS on success.
// \sa ReleaseStream()
CATResult CATFileSystem_Win32::OpenFile( const CATString&      filename, 
                                       CATStream::OPEN_MODE  mode,
                                       CATStream*&           stream)
{
   stream = 0;
   CATString fullPath = BuildPath(this->fBasePath,filename);
   
   try
   {
      stream = new CATStreamFile();
   }
   catch (...)
   {
      stream = 0;
   }

   if (stream == 0)
   {
      return CATRESULTFILE(CAT_ERR_OUT_OF_MEMORY,filename);
   }

   CATResult result = CAT_SUCCESS;
   if (CATFAILED(result = stream->Open(fullPath,mode)))
   {
      delete stream;
      stream = 0;
      return result;
   }

   return result;
}

//---------------------------------------------------------------------------
// OpenCachedFile() opens a RAM stream filled with the contents
// of the specified file. 
//
// \param filepath - path to file. 
// \param CATStream*& - ref to receive opened file stream
// \return CATResult - CAT_SUCCESS on success.
// \sa ReleaseStream()
CATResult CATFileSystem_Win32::OpenCachedFile( const CATString&      filename,                                        
                                       CATStream*&           stream)
{
   stream = 0;
   try
   {
      stream = new CATStreamRAM();
   }
   catch (...)
   {
      stream = 0;
   }

   if (stream == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }

   return ((CATStreamRAM*)stream)->FromFile(filename);
}

//---------------------------------------------------------------------------
// ReleaseFile() releases a stream opened with OpenFile().
// 
// \param CATStream*& - reference to stream pointer. Set to 0 when closed.
// \return CATResult - CAT_SUCCESS on success.
// \sa OpenFile()
CATResult CATFileSystem_Win32::ReleaseFile(CATStream*& stream)
{
   CATResult result = CAT_SUCCESS;
   if (stream == 0)
      return result;

   if (stream->IsOpen())
   {
      (void)stream->Close();
   }

   delete stream;
   stream = 0;
   
   return result;
}


/// IsFileReadOnly() returns true if the file is read-only, and false
/// if not or if it doesn't exist.
bool CATFileSystem_Win32::IsFileReadOnly(const CATString& path)
{
	if (CATSUCCEEDED(this->FileExists(path)))
	{
		DWORD attribs = ::GetFileAttributes(path);
		return attribs & FILE_ATTRIBUTE_READONLY;

	}
	else
	{
		return false;
	}
}
