/// \file CATStreamSub.cpp
/// \brief Sub-stream class
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATStreamSub.h"
CATStreamSub::CATStreamSub( CATInt64 offset, CATInt64 length, CATStream* parent) : 
CATStream()
{
   CATASSERT(parent != 0, "Sub streams with no parent aren't worth much....");
   CATASSERT(parent->IsOpen(), "The parent stream must be open, too...");

   fParent = parent;
   fOffset = offset;
   fLength = length;
   fCurPos = 0;
}

//---------------------------------------------------------------------------
// Destructor will clean up the RAM, but it'll
// assert in debug mode if you do this.
//
// Please call Close() before destroying a stream if you
// have opened it previously.
// \sa Close()
//---------------------------------------------------------------------------
CATStreamSub::~CATStreamSub()
{
   CATASSERT(fSubCount == 0, "You have substreams left open on a substream!");
}

//---------------------------------------------------------------------------
// Open() should not be called on substreams. They are always open.
//---------------------------------------------------------------------------
CATResult CATStreamSub::Open(const CATWChar* name, OPEN_MODE mode)
{   
   CATASSERT(false, "Substreams are always open.");
   return CATRESULT(CAT_ERR_OPENING_SUBSTREAM);
}

//---------------------------------------------------------------------------
// Close() should not be called on substreams. 
//
// When you are done with a substream, release it by calling
// CATStream::ReleaseSubStream() with its parent.
// \sa Open()
//---------------------------------------------------------------------------
CATResult CATStreamSub::Close()
{
   CATASSERT(false, "Close() should not be called on substreams.");
   return CATRESULT(CAT_ERR_CLOSING_SUBSTREAM);
}

//---------------------------------------------------------------------------
// IsOpen() returns true if the stream has been opened, and false otherwise.
//
// Substreams should always be open.
//
// \return bool - true if stream is open.
// \sa Open(), Close()
//---------------------------------------------------------------------------
bool CATStreamSub::IsOpen()
{
   return (fParent != 0);
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
CATResult CATStreamSub::Read(void* buffer, CATUInt32& length)
{
   CATASSERT(this->fParent != 0, "Can't read with a null parent.");
   if (this->fParent == 0)
   {
      return CATRESULT(CAT_ERR_SUBSTREAM_NO_PARENT);
   }
   
   if (this->fLength != -1)
   {
      if (length + fCurPos > this->fLength)
      {
         length = (CATUInt32)(fLength - fCurPos);
      }
   }

   CATResult result = fParent->ReadAbs(buffer,length,fCurPos + this->fOffset);
   fCurPos += length;
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
CATResult CATStreamSub::Write(const void* buffer, CATUInt32 length)
{
   CATASSERT(this->fParent != 0, "Can't read with a null parent.");
   if (this->fParent == 0)
   {
      return CATRESULT(CAT_ERR_SUBSTREAM_NO_PARENT);
   }
   
   // If length was specified, then truncate write to our stream section.
   if (this->fLength != -1)
   {
      if (length + fCurPos > this->fLength)
      {
         CATTRACE("Warning! Attempt to write beyond specified end of substream! Write truncated...");
         if (fCurPos < fLength)
         {
            length = (CATUInt32)(fLength - fCurPos);
         }
         else
         {
            return CATRESULT(CAT_ERR_WRITE_PAST_SPECIFIED_END);
         }
      }
   }

   CATResult result = fParent->WriteAbs(buffer,length,fCurPos + this->fOffset);
   
   // Only add our position on success.
   if (CATSUCCEEDED(result))
      fCurPos += length;
      
   return result;
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
CATResult CATStreamSub::Size(CATInt64& filesize)
{
   CATResult result = CAT_SUCCESS;
   CATASSERT(this->fParent != 0, "Can't read with a null parent.");
   if (this->fParent == 0)
   {
      return CATRESULT(CAT_ERR_SUBSTREAM_NO_PARENT);
   }

   if (this->fLength == -1)
   {
      this->fParent->Size(filesize);
      filesize -= this->fOffset;
   }
   else
   {
      filesize = this->fLength;
   }
      
   return result;
}

//---------------------------------------------------------------------------
// Substreams must be seekable.
//---------------------------------------------------------------------------
bool CATStreamSub::IsSeekable()
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
CATResult CATStreamSub::SeekRelative(CATInt32  offset)
{         
   //CATResult result = CAT_SUCCESS;
   CATASSERT(this->fParent != 0, "Can't read with a null parent.");
   if (this->fParent == 0)
   {
      return CATRESULT(CAT_ERR_SUBSTREAM_NO_PARENT);
   }

   if (offset + fCurPos < 0)
   {
      fCurPos = 0;
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,"SubStream");
   }

   if (fLength != -1)
   {
      if (fCurPos + offset > fLength)
      {
         fCurPos = fLength;
         return CATRESULT(CAT_ERR_SEEK_PAST_SPECIFIED_END);
      }
   }

   fCurPos += offset;

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
CATResult CATStreamSub::SeekAbsolute(CATInt64 position)
{
   //CATResult result = CAT_SUCCESS;
   CATASSERT(this->fParent != 0, "Can't read with a null parent.");
   if (this->fParent == 0)
   {
      return CATRESULT(CAT_ERR_SUBSTREAM_NO_PARENT);
   }

   if (position < 0)
   {
      fCurPos = 0;
      return CATRESULTFILE(CAT_ERR_FILE_SEEK,"SubStream");
   }

   if (fLength != -1)
   {
      if (position > fLength)
      {
         fCurPos = fLength;
         return CATRESULT(CAT_ERR_SEEK_PAST_SPECIFIED_END);
      }
   }

   fCurPos = position;

   return CAT_SUCCESS;   
}

//---------------------------------------------------------------------------
// SeekFromEnd() seeks from the end of the file.
//
// \param offset - signed offset from end of stream
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), SeekRelative(), SeekAbsolute()
//---------------------------------------------------------------------------
CATResult CATStreamSub::SeekFromEnd(CATInt32 offset)
{
   CATResult result = CAT_SUCCESS;
   CATInt64 position = 0;
   if (CATFAILED(result = this->Size(position)))
   {
      return result;
   }
   return this->SeekAbsolute(position - offset);
}


//---------------------------------------------------------------------------
// GetPosition() returns the current position in the stream
// in position.
//
// \param position - current position - set on successful return.
// \return CATResult - CAT_SUCCESS on success.
// \sa IsSeekable(), Size()
//---------------------------------------------------------------------------
CATResult CATStreamSub::GetPosition(CATInt64& position)
{
   position = this->fCurPos;
   return CAT_SUCCESS;
}



//---------------------------------------------------------------------------
CATString CATStreamSub::GetName() const
{
   if (fParent == 0)
   {
      return L"NULLSTREAM";
   }
   return this->fParent->GetName();
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
CATResult CATStreamSub::ReadAbs(void *buffer, CATUInt32& length, CATInt64 position)
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
CATResult CATStreamSub::WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position)
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

