//---------------------------------------------------------------------------
/// \file CATFileSystem_Win32.h
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

#ifndef CATFileSystem_Win32_H_
#define CATFileSystem_Win32_H_

#include "CATInternal.h"
#include "CATFileSystem.h"
#include "CATPlatform.h"
#include <map>

/// \class CATFileSystem_Win32 CATFileSystem_Win32.h
/// \brief File system functions for Win32 platform
/// \ingroup CAT
class CATFileSystem_Win32 : public CATFileSystem
{
    // Use CATPlatform for instantiation!
    friend CATPlatform;

    public:      
        /// Initialize must be called prior to using CATFileSystem!
        /// \return CATResult - CAT_SUCCESS
        virtual CATResult Initialize();

        /// FileExists should return a successful result if the file exists,
        /// or an error otherwise.
        ///
        /// Note: FileExists() fails if a directory of that name is present.
        ///
        /// \param pathname - path to file to check for existance.
        /// \return CATResult - successful result if the file is found.
        virtual CATResult FileExists(  const CATString& pathname   );

        /// DirExists should return a successful result if the dir exists,
        /// or an error otherwise.
        ///
        /// Note: DirExists() fails if a file of the specified name exists.
        ///
        /// \param pathname - path to dir to check for existance.
        /// \return CATResult - successful result if the file is found.
        virtual CATResult DirExists (  const CATString& pathname   );


        /// CreateDir creates the directory if necessary.
        ///
        /// \param pathname - path to dir to check for existance and create if not
        ///                   present.
        /// \return CATResult - CAT_SUCCESS if successful.
        virtual CATResult CreateDir (  const CATString& pathname   );

        /// PathExists should return a successful result if a dir or a file
        /// of that name exists.
        ///
        /// If it is a file, returns CAT_STAT_PATH_IS_FILE.
        /// IF it is a dir, returns CAT_STAT_PATH_IS_DIRECTORY
        ///
        /// Note: DirExists() fails if a file of the specified name exists.
        ///
        /// \param pathname - path to dir to check for existance.
        /// \return CATResult - successful result if the file is found.
        virtual CATResult PathExists(  const CATString& pathname   );

        /// FindFirst() finds the first matching file or directory and
        /// returns it in firstFile.  
        ///
        /// FindFirst *must* be called prior to FindNext().  You must call
        /// FindEnd() when done.
        ///
        /// \param searchMask - path/mask for performing searches with
        /// \param firstFile - ref to a string that receives the filename
        ///                    on success.
        /// \param findHandle - ref to handle returned on success. 
        /// \return CATResult - CAT_STAT_PATH_IS_DIRECTORY if entry is a directory.
        ///                    CAT_STAT_PATH_IS_FILE if it's a file.
        ///                    CAT_ERR_FIND_NO_MATCHES if no matches are found.
        /// \sa FindNext(), FindEnd()
        virtual CATResult FindFirst (  const CATString& searchMask, 
            CATString&       firstFile,                                    
            CATFINDHANDLE&   findHandle);

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
        /// \sa FindFirst(), FindEnd()
        virtual CATResult FindNext  (  CATString&       nextFile,
            CATFINDHANDLE    findHandle);

        /// FindEnd() ends a find operation and performs any necessary cleanup.
        ///
        /// The handle will be set to 0.
        ///
        /// \param findHandle - handle of find from FindFirst()
        /// \return CATResult - CAT_SUCCESS on success.
        /// \sa FindFirst(), FindNext()
        virtual CATResult FindEnd (CATFINDHANDLE& findHandle);

        /// OpenFile() opens or creates a file.
        ///
        /// \param filename - path to file. 
        /// \param mode - open mode for the file
        /// \param stream - ref to receive opened file stream
        /// \sa ReleaseStream()
        virtual CATResult OpenFile(    const CATString& filename, 
            CATStream::OPEN_MODE mode, 
            CATStream*& stream);

        /// OpenCachedFile() opens a file into a memory stream if possible.
        /// By default, it just routes to OpenFile, but child classes
        /// may override.
        ///
        /// \param filename - path to file.       
        /// \param stream - ref to receive opened file stream
        /// \return CATResult - CAT_SUCCESS on success.
        /// \sa ReleaseStream()
        virtual CATResult OpenCachedFile( const CATString&      filename,                                  
            CATStream*&           stream);

        /// ReleaseFile() releases a stream opened with OpenFile().
        /// 
        /// \param stream - reference to stream pointer. Set to 0 when closed.
        /// \return CATResult - CAT_SUCCESS on success.
        /// \sa OpenFile()
        virtual CATResult ReleaseFile(CATStream*& stream);

        /// IsFileReadOnly() returns true if the file is read-only, and false
        /// if not or if it doesn't exist.
        virtual bool	  IsFileReadOnly(const CATString& path);
    protected:
        // Constructor / destructor are protected.
        // Use CATPlatform::GetFileSystem() / Release() for creation and destruction!
        CATFileSystem_Win32(const CATString& basePath = "");
        virtual ~CATFileSystem_Win32();      

        std::map<CATFINDHANDLE,CATString> fFindPaths;
};


#endif // CATFileSystem_Win32_H_


