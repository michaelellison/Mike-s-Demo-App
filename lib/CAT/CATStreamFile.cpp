/// \file CATStreamFile.cpp
/// \brief File stream class
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATStreamFile.h"
#include "CATStreamSub.h"

CATStreamFile::CATStreamFile() : CATStream()
{
   fFileHandle = 0;
}

//---------------------------------------------------------------------------
// Destructor will close file handle if its unclosed, but
// will assert in debug mode if you do this.
//
// Please call Close() before destroying a file if you
// have opened it previously.
// \sa Close()
//---------------------------------------------------------------------------
CATStreamFile::~CATStreamFile()
{   
   CATASSERT(fFileHandle == 0, "Close your streams....");
   if (fFileHandle != 0)
   {
      this->Close();
   }
}

// stopgap for now for wide/char
#ifndef CAT_CONFIG_WIN32
FILE* _wfopen(const CATWChar* pathname, const CATWChar* mode)
{
    CATString path = pathname;
    CATString openMode = mode;
    return fopen(path,openMode);
}
#endif
// Open() opens a file from a pathname.
//
// Call close when done.
CATResult CATStreamFile::Open(const CATWChar* pathname, OPEN_MODE mode)
{   
   CATASSERT(fFileHandle == 0, "Trying to open an already open stream!");
   if (fFileHandle != 0)
   {
      // Argh... let 'em do it in release mode, but complain in debug.
      (void)this->Close();      
   }
      
   // fopen() interface doesn't split these cleanly.
   // 
   // Sharing is unsupported in generic CATStreamFile,
   // thus we can just switch on modes.

   switch (mode)
   {
      case READ_ONLY:
         fFileHandle = _wfopen(pathname,L"rb");
         break;

      case READ_WRITE_EXISTING_ONLY:
         fFileHandle = _wfopen(pathname,L"rb+");
         break;

      case READ_WRITE_EXISTING_FIRST:
         fFileHandle = _wfopen(pathname,L"rb+");
         if (fFileHandle == 0)
         {
            fFileHandle = _wfopen(pathname,L"wb+");
         }
         break;

      case READ_WRITE_CREATE_TRUNC:  
          {
#ifdef CAT_CONFIG_WIN32
              DWORD attribs = ::GetFileAttributes(pathname);
              if (attribs != 0xFFFFFFFF)
                  ::DeleteFile(pathname);
#else
              CATString pname = pathname;
              unlink(pname);
#endif
              
             fFileHandle = _wfopen(pathname,L"wb+");
          }
         break;

		case WRITE_CREATE_ONLY:
			fFileHandle = _wfopen(pathname,L"wb");
			break;

      default:
         CATASSERT(false,"Unsupported open mode.");
         return CATRESULT(CAT_ERR_FILE_UNSUPPORTED_MODE);
         break;
   }

   if (fFileHandle == 0)
   {
      return CATRESULTFILE(CAT_ERR_FILE_OPEN,pathname);
   }

   fFilename = pathname;

   return CATRESULT(CAT_SUCCESS);
}

//---------------------------------------------------------------------------
// Close() closes a previously opened file.
// 
// File must have been previously successfuly opened.
//
// \return CATResult - CAT_SUCCESS on success.
// \sa Open()
//---------------------------------------------------------------------------
CATResult CATStreamFile::Close()
{
   CATASSERT(fFileHandle != 0, "Attempting to close an already closed file.");
 
   CATASSERT(fSubCount == 0, "There are still substreams left open!");
   if (fSubCount != 0)
   {
      return CATRESULT(CAT_ERR_FILE_HAS_OPEN_SUBSTREAMS);
   }
 
   


   if (fFileHandle == 0)
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   fFilename = L"";
   
   fclose(fFileHandle);
   fFileHandle = 0;

   return CATRESULT(CAT_SUCCESS);
}

//---------------------------------------------------------------------------
// IsOpen() returns true if the file has been opened, and false otherwise.
//
// \return bool - true if file is open.
// \sa Open(), Close()
//---------------------------------------------------------------------------
bool CATStreamFile::IsOpen()
{
   return (fFileHandle != 0);
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
CATResult CATStreamFile::Read(void* buffer, CATUInt32& length)
{
   CATASSERT(fFileHandle != 0, "Reading from closed file.");
   CATASSERT(buffer != 0, "Null buffer passed to read.");

   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   CATUInt32 amountRead = (CATUInt32)fread(buffer,1,length,fFileHandle);
   if (length != amountRead)
   {
      if (feof(fFileHandle))
      {
         // File is at eof. Store amount read, return status code.
         // Note: this status is not an error code - user must
         // check explicitly.
         length = amountRead;
         return CATRESULT(CAT_STAT_FILE_AT_EOF);
      }

      length = amountRead;
      return CATRESULTFILE(CAT_ERR_FILE_READ,fFilename);
   }
   else
   {
      return CATRESULT(CAT_SUCCESS);
   }
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
CATResult CATStreamFile::Write(const void* buffer, CATUInt32 length)
{
   CATASSERT(fFileHandle != 0, "Reading from closed file.");
   CATASSERT(buffer != 0, "Null buffer passed to read.");

   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   CATUInt32 amountWritten = (CATUInt32)fwrite(buffer,1,length,fFileHandle);

   if (length != amountWritten)
   {
      return CATRESULTFILE(CAT_ERR_FILE_WRITE,fFilename);
   }
   else
   {
      // Allow for immediate read
      fflush(fFileHandle);
      return CAT_SUCCESS;
   }
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
CATResult CATStreamFile::Size(CATInt64& filesize)
{
   CATASSERT(fFileHandle != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   // Get current position so we can restore it later.
   fpos_t curPos;
   if (0 != fgetpos(fFileHandle, &curPos))
   {
      return CATRESULTFILE(CAT_ERR_FILE_GET_POSITION,fFilename);
   }

   // Go to the end of the file, then get the position.
   if (0 != fseek(fFileHandle,0,SEEK_END))
   {
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,fFilename);
   }
   
   fpos_t eofPos;

   if (0 != fgetpos(fFileHandle, &eofPos))
   {
      return CATRESULTFILE(CAT_ERR_FILE_GET_POSITION,fFilename);
   }

   // Return to original pos
   if (0 != fsetpos(fFileHandle,&curPos))
   {
      return CATRESULTFILE(CAT_ERR_FILE_SET_POSITION,fFilename);
   }

   filesize = eofPos;

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// IsSeekable() returns true for files.
//---------------------------------------------------------------------------
bool CATStreamFile::IsSeekable()
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
CATResult CATStreamFile::SeekRelative(CATInt32  offset)
{
   CATASSERT(fFileHandle != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   if ( 0 != fseek(fFileHandle,offset,SEEK_CUR))
   {
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,fFilename);
   }
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
CATResult CATStreamFile::SeekAbsolute(CATInt64 position)
{
   CATASSERT(fFileHandle != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   fpos_t filePos = position;
   if (0 != fsetpos(fFileHandle,&filePos))
   {
      return CATRESULTFILE(CAT_ERR_FILE_SET_POSITION,fFilename);
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
CATResult CATStreamFile::SeekFromEnd(CATInt32 offset)
{
   CATASSERT(fFileHandle != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   if (0 != fseek(fFileHandle,-offset,SEEK_END))
   {
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,fFilename);
   }

   return CAT_SUCCESS;
}


//---------------------------------------------------------------------------
// GetPosition() returns the current position in the stream
// in position.
//
// \param position - current position - set on successful return.
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), Size()
//---------------------------------------------------------------------------
CATResult CATStreamFile::GetPosition(CATInt64& position)
{
   CATASSERT(fFileHandle != 0, "File must be opened first.");
   if (!IsOpen())
   {
      return CATRESULT(CAT_ERR_FILE_NOT_OPEN);
   }

   fpos_t curPos;
   if (0 != fgetpos(fFileHandle, &curPos))
   {
      return CATRESULTFILE(CAT_ERR_FILE_GET_POSITION,fFilename);
   }

   position = curPos;

   return CAT_SUCCESS;
}



//---------------------------------------------------------------------------
CATString CATStreamFile::GetName() const
{
   return fFilename;
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
//  \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamFile::ReadAbs(void *buffer, CATUInt32& length, CATInt64 position)
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
//  \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATStreamFile::WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position)
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

