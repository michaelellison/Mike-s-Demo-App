//---------------------------------------------------------------------------
/// \file CATStreamRAM.h
/// \brief RAM stream
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef CATStreamRAM_H_
#define CATStreamRAM_H_

#include "CATStream.h"

/// \class CATStreamRAM
/// \brief Memory-based stream class - acts as a file in RAM
/// \ingroup CAT
class CATStreamRAM : public CATStream
{
   public:
         /// Default constructor doesn't do much - you'll need
         /// to call Open() before trying to do much.
         CATStreamRAM();

         /// Destructor will close file handle if its unclosed, but
         /// will assert in debug mode if you do this.
         ///
         /// Please call Close() before destroying a file if you
         /// have opened it previously.
         /// \sa Close()
         virtual ~CATStreamRAM();

         /// Open() opens a file from a pathname.
         ///
         /// Call close when done.
         ///
         /// \param name - not really critical, but you're welcome to name ram files.
         /// \param mode - currently ignored.
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa Close()
         virtual CATResult Open(const CATWChar* name, OPEN_MODE mode);
         
         /// Close() closes a previously opened file.
         /// 
         /// File must have been previously successfuly opened.
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa Open()
         virtual CATResult Close();

         /// IsOpen() returns true if the file has been opened, and false otherwise.
         ///
         /// \return bool - true if file is open.
         /// \sa Open(), Close()
         virtual bool IsOpen();

         /// Read() reads the requested amount of data into a buffer.
         ///
         /// Will read up to, but not necessarily, [length] bytes.
         /// On return, length is set to the number of bytes actually read.
         /// buffer *must* be large enough for max value of length.
         /// 
         /// \param buffer - target buffer for read
         /// \param length - min(length of buffer, desired read length).
         ///        Set to amount read on return.
         /// \return CATResult - CAT_SUCCESS on success
         /// \sa Write()
         virtual CATResult Read(void* buffer, CATUInt32& length);

         /// Write() writes the requested amount of data from a buffer.
         ///
         /// Incomplete writes are treated as an error.
         /// 
         /// \param buffer - source buffer to write from.
         /// \param length - length of data to write.
         /// \return CATResult - CAT_SUCCESS on success
         /// \sa Read()
         virtual CATResult Write(const void* buffer, CATUInt32 length);
         
         /// Size() returns the size of the object in filesize.
         ///
         /// This may not be supported on all stream types.
         /// \param filesize - 64-bit length of file.
         /// \return CATResult - CAT_SUCCESS on success
         virtual CATResult Size(CATInt64& filesize);

         /// IsSeekable() returns true for files.
         virtual bool     IsSeekable();
         
         /// SeekRelative() seeks from current position to a
         /// relative location.
         ///
         /// \param offset - signed offset from current position
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekAbsolute(), SeekFromEnd()
         virtual CATResult SeekRelative(CATInt32  offset);


         /// SeekAbsolute() seeks from the start of the file
         /// to an absolute position.
         ///
         /// \param position - unsigned absolute position to seek to.
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekRelative(), SeekFromEnd()
         virtual CATResult SeekAbsolute(CATInt64 position);

         /// SeekFromEnd() seeks from the end of the file.
         ///
         /// \param offset - signed offset from end of stream
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekRelative(), SeekAbsolute()
         virtual CATResult SeekFromEnd(CATInt32 offset);
         
         /// GetPosition() returns the current position in the stream
         /// in position.
         ///
         /// \param position - current position - set on successful return.
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), Size()
         virtual CATResult GetPosition(CATInt64& position);

         /// GetName() retrieves the filename of the stream.
         virtual CATString GetName() const;

         /// ReadAbs() reads from the specified location, but does
         /// not change the current stream position.
         ///
         /// ReadAbs() is mainly for use in substreams, and may not be
         /// available from all stream types.  If you're not implementing
         /// it, then please return an error from your derived class.
         ///
         /// \param buffer - target buffer for read
         /// \param length - min(length of buffer, desired read length).
         ///                 set to amount read on return.
         /// \param position - position within stream to read.
         ///  \return CATRESULT - CAT_SUCCESS on success.
         virtual CATResult ReadAbs(void *buffer, CATUInt32& length, CATInt64 position);

         /// WriteAbs() Writes from the specified location, but does
         /// not change the current stream position.
         ///
         /// WriteAbs() is mainly for use in substreams, and may not be
         /// available from all stream types.  If you're not implementing
         /// it, then please return an error from your derived class.
         ///
         /// \param buffer - target buffer for Write
         /// \param length - length of data to write        
         /// \param position - position within stream to read.
         ///  \return CATRESULT - CAT_SUCCESS on success.
         virtual CATResult WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position);

         /// ReallocCache() reallocates the cache memory to at least
         /// as large as minLength.
         CATResult ReallocCache( CATInt32 minLength );

         /// ShrinkCache() shrinks the cache to exactly the current fSize().
         CATResult ShrinkCache();

         /// FromFile() loads a file into the RAM stream. This is analogous
         /// to calling Open() on a file stream, only your read/writes
         /// will be a hell of a lot faster.
         ///
         /// As in Open(), please close prior to opening a new one, and 
         /// close before destroying.
         ///
         /// \param pathName - path of file to open
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         CATResult FromFile(const CATWChar* pathName);

         /// ToFile() saves the stream to a file.
         ///
         /// \param pathName - path of file to save
         /// \param overwrite - if false, will return an error if the file already exists.
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         CATResult ToFile(const CATWChar* pathName, bool overwrite = true);

         /// GetRawCache() retrieves a raw pointer to the buffer.
         /// WARNING: this pointer is only valid until another stream command
         /// is made.  Stream operations may change the cache pointer, causing
         /// use of the returned pointer to cause an access violation.
         /// \return CATUInt8* - temporary pointer to cache
         CATUInt8* GetRawCache();


         
   private:
         
         CATStreamRAM& operator=(const CATStreamRAM& srcStream)
         {
            CATASSERT(false,"Copy operator not currently supported for files.");
            return *this;
         }      

         CATUInt8*     fRamCache;
         CATInt32      fCacheSize;
         CATInt32      fSize;
         CATInt32      fCurPos;         
         CATString     fStreamName;         
};

#endif // CATStreamRAM_H_


