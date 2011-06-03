//---------------------------------------------------------------------------
/// \file CATStreamRAM.cpp
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
#include "CATStreamRAM.h"
#include "CATStreamFile.h"
#include "CATStreamSub.h"

// Default RAM streams to 10k in size. 
const CATInt32 kCATSTREAM_DEFSIZE = 1024*10;

CATStreamRAM::CATStreamRAM() : CATStream()
{
   fRamCache   = 0;
   fCacheSize  = 0;
   fSize       = 0;
   fCurPos     = 0;   
}

//---------------------------------------------------------------------------
// Destructor will clean up the RAM, but it'll
// assert in debug mode if you do this.
//
// Please call Close() before destroying a stream if you
// have opened it previously.
// \sa Close()
//---------------------------------------------------------------------------
CATStreamRAM::~CATStreamRAM()
{
   if (fRamCache != 0)
   {
      this->Close();
   }
}

//---------------------------------------------------------------------------
// Open() opens a named RAM stream.
//
// Call close when done.
//
// \param pathname - CATString providing stream name
// \param mode - combination of OPEN_MODE enumerated flags.
//
// \return CATResult - CAT_SUCCESS on success.
// \sa Close()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::Open(const CATWChar* name, OPEN_MODE mode)
{   
   CATASSERT(fRamCache == 0, "Trying to open an already open stream!");
   if (fRamCache != 0)
   {
      // Argh... let 'em do it in release mode, but complain in debug.
      (void)this->Close();      
   }
      
   // Mode flags are currently ignored.
   fStreamName = name;
   
   fCacheSize = 0;

   try
   {
      fRamCache = new CATUInt8[kCATSTREAM_DEFSIZE];
      fCacheSize = kCATSTREAM_DEFSIZE;
   }
   catch (...)
   {
      fRamCache = 0;      
   }

   if (fRamCache == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// Close() closes a previously opened stream.
// 
// stream must have been previously successfuly opened.
//
// \return CATResult - CAT_SUCCESS on success.
// \sa Open()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::Close()
{
   CATASSERT(fRamCache != 0, "Attempting to close an already closed stream.");

   CATASSERT(fSubCount == 0, "There are still substreams left open!");
   if (fSubCount != 0)
   {
      return CATRESULT(CAT_ERR_FILE_HAS_OPEN_SUBSTREAMS);
   }
    
   fStreamName = "";
   fCacheSize = 0;

   if (fRamCache == 0)
   {      
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   delete [] fRamCache;
   fRamCache = 0;

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// IsOpen() returns true if the stream has been opened, and false otherwise.
//
// \return bool - true if stream is open.
// \sa Open(), Close()
//---------------------------------------------------------------------------
bool CATStreamRAM::IsOpen()
{
   return (fRamCache != 0);
}


//---------------------------------------------------------------------------
// Read() reads the requested amount of data into a buffer.
//
// Will read up to, but not necessarily, [length] bytes.
// On return, length is set to the number of bytes actually read.
// buffer *must* be large enough for max value of length.
// 
// \param buffer - target buffer for read
// \param length - min(length of buffer, desired read length).
//        Set to amount read on return.
// \return CATResult - CAT_SUCCESS on success
// \sa Write()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::Read(void* buffer, CATUInt32& length)
{
   CATASSERT(fRamCache != 0, "Reading from closed file.");
   CATASSERT(buffer != 0, "Null buffer passed to read.");

   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   if (fCurPos >= fSize)
   {
      length = 0;      
      return CATRESULT(CAT_STAT_FILE_AT_EOF);
   }

   CATUInt32 amountRead = 0;
   
   CATResult result = CAT_SUCCESS;

   if ((CATInt32)(fCurPos + length) >= fSize)
   {
      result = CATRESULT(CAT_STAT_FILE_AT_EOF);
      amountRead = fSize - fCurPos;
   }
   else
   {
      amountRead = length;
   }

   memcpy(buffer,fRamCache + fCurPos, amountRead);
   fCurPos += amountRead;
   length = amountRead;

   return result;
}

//---------------------------------------------------------------------------
// Write() writes the requested amount of data from a buffer.
//
// Incomplete writes are treated as an error.
// 
// \param buffer - source buffer to write from.
// \param length - length of data to write
// \return CATResult - CAT_SUCCESS on success
// \sa Read()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::Write(const void* buffer, CATUInt32 length)
{
   CATASSERT(fRamCache != 0, "Reading from closed file.");
   CATASSERT(buffer != 0, "Null buffer passed to read.");

   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   CATUInt32 amountWritten = length; 
   if ((CATInt32)(fCurPos + amountWritten) > fCacheSize)
   {
      CATResult result = ReallocCache(amountWritten + fCurPos);
      if (CATFAILED(result))
      {
         return result;
      }
   }
   
   memcpy(fRamCache + fCurPos, buffer, amountWritten);
   fCurPos += amountWritten;
   if (fCurPos > fSize)
   {
      fSize = fCurPos;
   }
   
   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// Size() returns the size of the object in filesize.
//
// This should be cached and optimized later.
//
// This may not be supported on all stream types.
// \param filesize - 64-bit length of file.
// \return CATResult - CAT_SUCCESS on success
//---------------------------------------------------------------------------
CATResult CATStreamRAM::Size(CATInt64& filesize)
{
   CATASSERT(fRamCache != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   filesize = fSize;

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// IsSeekable() returns true for files.
//---------------------------------------------------------------------------
bool CATStreamRAM::IsSeekable()
{
   return true;
}

//---------------------------------------------------------------------------
// SeekRelative() seeks from current position to a
// relative location.
//
// \param offset - signed offset from current position
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), SeekAbsolute(), SeekFromEnd()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::SeekRelative(CATInt32  offset)
{         
   CATASSERT(fRamCache != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   if (fCurPos + offset < 0)
   {
      fCurPos = 0;
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,this->fStreamName);
   }

   if (fCurPos + offset > fCacheSize)
   {
      CATResult result = ReallocCache(fCurPos + offset);
      if (CATFAILED(result))
         return result;
   }

   fCurPos += offset;
   if (fCurPos > fSize)
      fSize = fCurPos;
            
   return CAT_SUCCESS;
}


//---------------------------------------------------------------------------
// SeekAbsolute() seeks from the start of the file
// to an absolute position.
//
// \param position - unsigned absolute position to seek to.
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), SeekRelative(), SeekFromEnd()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::SeekAbsolute(CATInt64 position)
{
   CATASSERT(fRamCache != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   if (position > fCacheSize)
   {
      CATResult result = ReallocCache((CATInt32)position);
      if (CATFAILED(result))
      {
         return result;
      }
   }

   fCurPos = (CATInt32)position;

   if (fCurPos > fSize)
   {
      fSize = fCurPos;
   }

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// SeekFromEnd() seeks from the end of the file.
//
// \param offset - signed offset from end of stream
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), SeekRelative(), SeekAbsolute()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::SeekFromEnd(CATInt32 offset)
{
   return SeekAbsolute(fSize - offset);
}


//---------------------------------------------------------------------------
// GetPosition() returns the current position in the stream
// in position.
//
// \param position - current position - set on successful return.
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), Size()
//---------------------------------------------------------------------------
CATResult CATStreamRAM::GetPosition(CATInt64& position)
{
   CATASSERT(fRamCache != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   position = fCurPos;
   return CAT_SUCCESS;
}



//---------------------------------------------------------------------------
CATString CATStreamRAM::GetName() const
{
   return fStreamName;
}


//---------------------------------------------------------------------------
// ReallocCache() reallocates the cache memory to at least
// as large as minLength.
//---------------------------------------------------------------------------
CATResult CATStreamRAM::ReallocCache( CATInt32 minLength )
{
   // allocate more ram. Bail on failure.
   CATUInt8*  newRam = 0;

   // Default to doubling each time.
   CATInt32  newSize = fCacheSize*2;
   
   // If we're writing more than double, double the write size + cursize
   if ( minLength > newSize)
   {
      newSize = minLength * 2;
   }

   try 
   {
      newRam = new CATUInt8[newSize];
   }
   catch (...)
   {
      newRam = 0;
   }

   if (newRam == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }
   
   CATTRACE((CATString)"Reallocating from " << fCacheSize << " to " << newSize);

   // Copy and swap buffers
   memcpy(newRam,fRamCache,fSize);   
   delete [] fRamCache;
   fRamCache = newRam;         
   fCacheSize = newSize;
   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// ShrinkCache() shrinks the cache to exactly the current fSize().
//---------------------------------------------------------------------------
CATResult CATStreamRAM::ShrinkCache()
{
   // allocate more ram. Bail on failure.
   CATUInt8*  newRam = 0;

   // Default to doubling each time.
   CATInt32  newSize = fSize;
   
   try 
   {
      newRam = new CATUInt8[newSize];
   }
   catch (...)
   {
      newRam = 0;
   }

   if (newRam == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }
   
   memcpy(newRam, fRamCache, fSize);
   delete [] fRamCache;  
   fRamCache = newRam;
   fCacheSize = newSize;   
   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// FromFile() loads a file into the RAM stream. This is analogous
// to calling Open() on a file stream, only your read/writes
// will be a hell of a lot faster.
//
// As in Open(), please close prior to opening a new one, and 
// close before destroying.
//
// \param pathName - path of file to open
//
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamRAM::FromFile(const CATWChar* pathName)
{
   CATResult result = CAT_SUCCESS;

   CATASSERT(fRamCache == 0, "Trying to open an already open stream!");
   if (fRamCache != 0)
   {
      // Argh... let 'em do it in release mode, but complain in debug.
      (void)this->Close();      
   }

   CATStreamFile *fileStream = new CATStreamFile();
   if (CATFAILED(result = fileStream->Open(pathName,CATStream::READ_ONLY)))
   {
      delete fileStream;
      return result;
   }

   CATInt64 fileSize = 0;
   fileStream->Size(fileSize);

   this->fCacheSize = (CATInt32)fileSize;
   this->fSize = fCacheSize;
   
   try
   {
      this->fRamCache = new CATUInt8[fSize];
   }
   catch (...)
   {
      this->fRamCache = 0;
   }

   if (fRamCache == 0)
   {
      fCacheSize = 0;
      fSize = 0;
      fileStream->Close();
      delete fileStream;
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }

   CATUInt32 readSize = (CATUInt32)fSize;
   result = fileStream->Read(fRamCache,readSize);

   CATASSERT((CATInt32)readSize == fSize, "Error reading entire file!");

   (void)fileStream->Close();
   delete fileStream;

   return result;
}

//---------------------------------------------------------------------------
// ToFile() saves the stream to a file.
//
// \param pathName - path of file to save
// \param overwrite - if false, will return an error if the file already exists.
//
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamRAM::ToFile(const CATWChar* pathName, bool overwrite)
{
   CATResult result = CAT_SUCCESS;

   CATASSERT(fRamCache != 0, "Trying to open an already open stream!");
   if (fRamCache == 0)
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   CATStreamFile *fileStream = new CATStreamFile();
   if (overwrite == false)
   {
      if (CATSUCCEEDED(result = fileStream->Open(pathName,CATStream::READ_ONLY)))
      {
         fileStream->Close();
         delete fileStream;
         return CATRESULT(CAT_ERR_FILE_ALREADY_EXISTS);
      }
   }

   if (CATFAILED(result = fileStream->Open(pathName,CATStream::READ_WRITE_CREATE_TRUNC)))
   {
      fileStream->Close();
      delete fileStream;
      return result;
   }

   result = fileStream->Write(fRamCache,fSize);

   (void)fileStream->Close();
   delete fileStream;


   return result;
}

//---------------------------------------------------------------------------
// GetRawCache() retrieves a raw pointer to the buffer.
// WARNING: this pointer is only valid until another stream command
// is made.  Stream operations may change the cache pointer, causing
// use of the returned pointer to cause an access violation.
// \return CATUInt8* - temporary pointer to cache
//---------------------------------------------------------------------------
CATUInt8* CATStreamRAM::GetRawCache()
{
   return fRamCache;
}

//---------------------------------------------------------------------------
// ReadAbs() reads from the specified location, but does
// not change the current stream position.
//
// ReadAbs() is mainly for use in substreams, and may not be
// available from all stream types.  If you're not implementing
// it, then please return an error from your derived class.
//
// \param buffer - target buffer for read
// \param length - min(length of buffer, desired read length).
//                 set to amount read on return.
// \param position - position within stream to read.
//  \return CATRESULT - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamRAM::ReadAbs(void *buffer, CATUInt32& length, CATInt64 position)
{
   CATResult result = CAT_SUCCESS;
   CATInt64 orgPos = 0;

   if (CATFAILED(result = this->GetPosition(orgPos)))
   {
      return result;
   }
   
   // On errors, at least try to restore the old position.
   if (CATFAILED(result = this->SeekAbsolute(position)))
   {
      this->SeekAbsolute(orgPos);
      return result;
   }

   if (CATFAILED(result = this->Read(buffer,length)))
   {
      this->SeekAbsolute(orgPos);
      return result;
   }

   return this->SeekAbsolute(orgPos);
}

//---------------------------------------------------------------------------
// WriteAbs() Writes from the specified location, but does
// not change the current stream position.
//
// WriteAbs() is mainly for use in substreams, and may not be
// available from all stream types.  If you're not implementing
// it, then please return an error from your derived class.
//
// \param buffer - target buffer for Write
// \param length - length of data to write        
// \param position - position within stream to read.
//  \return CATRESULT - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamRAM::WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position)
{
   CATResult result = CAT_SUCCESS;
   CATInt64 orgPos = 0;

   if (CATFAILED(result = this->GetPosition(orgPos)))
   {
      return result;
   }
   
   // On errors, at least try to restore the old position.
   if (CATFAILED(result = this->SeekAbsolute(position)))
   {
      this->SeekAbsolute(orgPos);
      return result;
   }

   if (CATFAILED(result = this->Write(buffer,length)))
   {
      this->SeekAbsolute(orgPos);
      return result;
   }

   return this->SeekAbsolute(orgPos);
}

