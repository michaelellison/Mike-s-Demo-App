//---------------------------------------------------------------------------
/// \file CATFileSystem.h
/// \brief File utility virtual interface
/// \ingroup CAT
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATFileSystem_H_
#define _CATFileSystem_H_

#include "CATInternal.h"
#include "CATMutex.h"
#include "CATPlatform.h"
#include "CATStreamFile.h"

typedef void* CATFINDHANDLE;

/// \class CATFileSystem
/// \brief File utility virtual interface. 
///
/// Use CATPlatform::GetFileSystem() to acquire, and CATPlatform::Release()
/// to release.
///
/// Filesystem functions should be threadsafed using fFSLock.
///
/// You must call Initialize() after receiving a file system from CATPlatform.
class CATFileSystem
{
   friend CATPlatform;
   public:     
      /// Initialize must be called prior to using CATFileSystem!
      /// \return CATResult - CAT_SUCCESS
      virtual CATResult Initialize() = 0;

      /// FileExists should return a successful result if the file exists,
      /// or an error otherwise.
      ///
      /// Note: FileExists() fails if a directory of that name is present.
      ///
      /// \param pathname - path to file to check for existance.
      /// \return CATResult - successful result if the file is found.
      virtual CATResult FileExists(  const CATString& pathname   )  = 0;

      /// DirExists should return a successful result if the dir exists,
      /// or an error otherwise.
      ///
      /// Note: DirExists() fails if a file of the specified name exists.
      ///
      /// \param pathname - path to dir to check for existance.
      /// \return CATResult - successful result if the file is found.
      virtual CATResult DirExists (  const CATString& pathname   )  = 0;

      /// CreateDir creates the directory if necessary.
      ///
      /// \param pathname - path to dir to check for existance and create if not
      ///                   present.
      /// \return CATResult - CAT_SUCCESS if successful.
      virtual CATResult CreateDir (  const CATString& pathname   )  = 0;



      /// PathExists should return a successful result if a dir or a file
      /// of that name exists.
      ///
      /// If it is a file, returns CAT_STAT_PATH_IS_FILE.
      /// IF it is a dir, returns CAT_STAT_PATH_IS_DIRECTORY
      ///
      /// \param pathname - path to dir to check for existance.
      /// \return CATResult - successful result if the file is found.
      virtual CATResult PathExists(  const CATString& pathname   )  = 0;
      
      /// FindFirst() finds the first matching file or directory and
      /// returns it in firstFile.  
      ///
      /// FindFirst *must* be called prior to FindNext().  You must call
      /// FindEnd() when done.
      ///
      /// \param searchMask - mask for performing searches with
      /// \param firstFile - ref to a string that receives the filename
      ///                    on success.
      /// \param findHandle - ref to handle returned on success. 
      /// \return CATResult - CAT_STAT_PATH_IS_DIRECTORY if entry is a directory.
      ///                    CAT_STAT_PATH_IS_FILE if it's a file.
      ///                    CAT_ERR_FIND_NO_MATCHES if no matches are found.
      virtual CATResult FindFirst (  const CATString& searchMask, 
                                    CATString&       firstFile,
                                    CATFINDHANDLE&   findHandle) = 0;      

      /// FindNext() finds the next matching file or directory and
      /// returns it in nextFile.
      ///
      /// You should pass the findHandle returned by a previous call
      /// to FindFirst().
      ///
      /// \param nextFile - ref to string to receive path of next file
      /// \param findHandle - handle for search.
      /// \return CATResult - CAT_STAT_PATH_IS_DIRECTORY if entry is a directory.
      ///                    CAT_STAT_PATH_IS_FILE if it's a file.
      ///                    CAT_ERR_FIND_END if no more files are available.
      virtual CATResult FindNext  (  CATString&       nextFile,
                                    CATFINDHANDLE    findHandle) = 0;

      /// FindEnd() ends a find operation and performs any necessary cleanup.
      ///
      /// The handle will be set to 0.
      ///
      /// \param findHandle - handle of find from FindFirst()
      /// \return CATResult - CAT_SUCCESS on success.
      virtual CATResult FindEnd (CATFINDHANDLE& findHandle) = 0;
               

      /// OpenFile() opens or creates a file.
      ///
      /// \param filename - path to file. 
      /// \param mode - open mode for the file      
      /// \param stream - ref to receive opened file stream
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa ReleaseStream()
      virtual CATResult OpenFile( const CATString&      filename, 
                                 CATStream::OPEN_MODE  mode,
                                 CATStream*&           stream) = 0;

      /// OpenCachedFile() opens a file into a memory stream if possible.
      /// By default, it just routes to OpenFile, but child classes
      /// may override.
      ///
      /// \param filename - path to file.       
      /// \param stream - ref to receive opened file stream
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa ReleaseStream()
      virtual CATResult OpenCachedFile( const CATString&      filename,                                  
                                       CATStream*&           stream)
      {
         return OpenFile(filename, CATStream::READ_ONLY,stream);
      }

      /// ReleaseFile() releases a stream opened with GetStream().
      /// 
      /// \param stream - reference to stream pointer. Set to 0 when closed.
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa GetStream()
      virtual CATResult ReleaseFile(CATStream*& stream) = 0;

		/// IsFileReadOnly() returns true if the file is read-only, and false
		/// if not or if it doesn't exist.
		virtual bool	  IsFileReadOnly(const CATString& path) = 0;

      /// GetFullPath() gets the fully qualified path from a local path.
      ///
      /// IF we create the file system with a base path location, then this
      /// will convert from a subpath of the base path into a fully
      /// qualified path.
      ///
      CATString GetFullPath( const CATString& path )
      {
         return BuildPath(this->fBasePath, path);
      }

      /// BuildPath() combines a directory and a filename into a single path string.
      ///
      /// \param directory - directory path. May or may not be terminated with a seperator.
      /// \param filename - filename.extension for the path.
      /// \return CATString - combined path for the file
      /// \sa SplitPath()
      static CATString BuildPath (  const CATString&   directory,
                                   const CATString&   filename,
											  bool  appendSep = false)
      {
         CATString fullPath;
                                  
         if (directory.IsEmpty() == false)
         {
            fullPath = directory;
            if ((directory.GetWChar(directory.LengthCalc() - 1) != CAT_PATHSEPERATOR) &&
                ((filename.IsEmpty() == true) || (filename.GetWChar(0) != CAT_PATHSEPERATOR)))
            {
               fullPath << CAT_PATHSEPERATOR;
            }
         }

         fullPath << filename;

			if (appendSep)
			{
				if (fullPath.GetWChar( fullPath.Length() - 1) != CAT_PATHSEPERATOR)
				{
					fullPath << CAT_PATHSEPERATOR;
				}
			}

         return fullPath;
      }

		static CATString& EnsureTerminator(CATString& termPath)
		{
			if (termPath.IsEmpty() || (termPath.GetWChar( termPath.Length() - 1) != CAT_PATHSEPERATOR))
			{
				termPath << CAT_PATHSEPERATOR;
			}
			return termPath;
		}


      /// GetFileExtension() retrieves the file extension of a file in a path.
      /// If none, the returned string is empty.
      static CATString GetFileExtension (const CATString& path)
      {  
         CATString extension;
         bool found = false;

         CATUInt32 offset = -1;
         if (path.ReverseFind(CAT_PATHSEPERATOR,offset))
         {
            // file extension marker has to be after last seperator.
            found = path.Find( '.',offset);            
         }
         else 
         {
            offset = 0;
            found = path.Find('.',offset);
         }

         if (found)
         {
            extension = path.Right(offset+1);
         }
         return extension;
      }

      /// StripFileExtension() returns the string w/o any file extension      
      static CATString StripFileExtension (const CATString& path)
      {  
         CATString noExtension = path;

         bool found = false;

         CATUInt32 offset = -1;
			CATUInt32 testOffset = -1;

         if (path.ReverseFind(CAT_PATHSEPERATOR,testOffset))
         {
            // file extension marker has to be after last seperator.
            while (path.Find('.',testOffset))
				{
					offset = testOffset;
					found = true;
					testOffset++;
				}
         }
         else 
         {
            while (path.Find('.',testOffset))
				{
					offset = testOffset;
					found = true;
					testOffset++;
				}
         }

         if (found)
         {
            noExtension = path.Left(offset);   
         }
         return noExtension;
      }

      /// SanitizeFilename() removes any illegal characters from
      /// a filename.
      static CATString SanitizeFilename( const CATString&  filename,
                                        bool       allowExtSep    = true,
                                        bool       allowPathSep   = false,
                                        bool       allowDriveSep  = false)
      {
         const CATUInt32 numChars = 6;
         wchar_t  illegalChars[numChars] = { '*','|','?','<','>','/' };
         
         CATString safeString;
         CATUInt32  nameLen = filename.LengthCalc();

         for (CATUInt32 i=0; i < nameLen; i++)
         {
            wchar_t curChar = filename.GetWChar(i);
            bool skipChar = false;

            for (CATUInt32 j=0; j<numChars; j++)
            {
               if (curChar == illegalChars[j])
                  skipChar = true;
            }

            // If !allowing extension seperator and it's a '.', skip it...
            skipChar |= ((curChar == '.') && (!allowExtSep));
            
            // Check for path/drive seperators...
            skipChar |= ((curChar == CAT_PATHSEPERATOR) && (!allowPathSep));
              
#ifdef kDRIVESEPERATOR
            skipChar |= ((curChar == kDRIVESEPERATOR) && (!allowDriveSep));
#endif

            if (!skipChar)
            {
               safeString << curChar;
            }
         }

         return safeString;
      }



      /// SplitPath splits the string into a directory and a base
      /// filename.
      ///
      /// This function may be somewhat platform independant, although
      /// the path seperator is predefined in CATConfig.
      ///
      /// \param fullPath - the path to split
      /// \param directory - ref to receive the base directory.
      ///        If keepTrailingSep is true, it will end in a seperator.
      /// \param filename - ref to receive the base filename w/o path.
      /// \param keepTrailingSep - if true, leaves a trailing seperator on the path.
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa BuildPath
      static CATResult SplitPath (  const CATString&   fullPath, 
                                    CATString&         directory, 
                                    CATString&         filename,                                     
                                    bool              keepTrailingSep = true)
      {
         CATResult result = CAT_SUCCESS;
         CATUInt32 offset  = -1;
         directory       = "";
         filename        = "";

         if (fullPath.IsEmpty())
         {
            return CATRESULT(CAT_ERR_PATH_EMPTY);
         }   

         if (fullPath.ReverseFind(CAT_PATHSEPERATOR,offset))
         {
            directory = fullPath.Left(offset + (keepTrailingSep ? 1 : 0));
            filename = fullPath.Right(offset + 1);
         }
         else
         {
#ifdef kDRIVESEPERATOR
            offset = -1;
            if (fullPath.ReverseFind(kDRIVESEPERATOR, offset))
            {
               directory = fullPath.Left(offset + 1);
               filename = fullPath.Right(offset + 1);
            }
            else
#endif
            {
               filename = fullPath;         
            }
         }
         
         if (directory.IsEmpty() != true)
         {
            if ( directory.GetWChar(directory.LengthCalc() - 1) !=  CAT_PATHSEPERATOR)
            {
               directory << CAT_PATHSEPERATOR;
            }
         }
           
         if (filename.IsEmpty())
         {
            result = CATRESULTFILE(CAT_STAT_PATH_NO_FILE, fullPath);
         }
         else if (directory.IsEmpty())
         {
            result = CATRESULTFILE(CAT_STAT_PATH_NO_DIRECTORY, fullPath);
         }

         return result;

      }

      /// GetBase() retrieves the base path of the system
      virtual CATString GetBase()
      {
         return fBasePath;
      }

   protected:
      /// Constructors and destructors are protected! Use CATPlatform
      /// to create / releaes!

      /// Constructor takes basePath as an argument -
      /// All file operations should be based off of this path.
      CATFileSystem(const CATString& basePath = "") 
      {
         fBasePath      = basePath;
        
         if ((fBasePath.IsEmpty() == false) && (fBasePath.GetWChar(fBasePath.Length() - 1) != CAT_PATHSEPERATOR))
         {
            fBasePath << CAT_PATHSEPERATOR;
         }
      }

     
      /// Destructor
      virtual ~CATFileSystem()  
      {
         // Wait for any pending commands.
         fFSLock.Wait();
         fFSLock.Release();
      }

      /// Remove fBasePath from a full path and return result.
      CATString RemoveBasePath(CATString& fullPath)
      {         
         if (fBasePath.CompareNoCase(fullPath,fBasePath.Length()) == 0)
         {
            return fullPath.Right(fBasePath.Length());
         }
         else
         {
            return fullPath;
         }
      }

      CATMutex        fFSLock;
      CATString       fBasePath;
};


#endif // _CATFileSystem_H_


