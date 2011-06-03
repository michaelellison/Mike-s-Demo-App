/// \file CATStreamSub.h
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

#ifndef _CATStreamSub_H_
#define _CATStreamSub_H_

#include "CATStream.h"

/// \class CATStreamSub CATStreamSub.h
/// \brief sub stream class - should work for all seekable stream types.
/// \ingroup CAT
///
class CATStreamSub : public CATStream
{
   public:
         /// Substreams should only be constructed by CreateSubStream() or
         /// similar methods. They *must* have a parent stream.
         CATStreamSub( CATInt64 offset, CATInt64 length, CATStream* parent);

         /// CATStreamSub() - substream destructor. This should only be
         /// called by ReleaseSubStream() or similar methods.
         virtual ~CATStreamSub();

         /// Open() should not be called on substreams. They are always open.
         /// \sa Close()
         virtual CATResult Open(const CATWChar* pathname, OPEN_MODE mode);

         /// OnAcquireSub is used in place of open, and should only be called
         /// by the parent stream.
         virtual CATResult OnAcquireSub()
         {
            return CAT_SUCCESS;
         }
         
         /// OnReleaseSub is used in place of close, and should only be
         /// called by the parent stream.         
         virtual CATResult OnReleaseSub()
         {
            return CAT_SUCCESS;
         }


         /// Close() should not be called on substreams. 
         ///
         /// When you are done with a substream, release it by calling
         /// CATStream::ReleaseSubStream() with its parent.
         /// \sa Open()
         virtual CATResult Close();

         /// IsOpen() returns true if the file has been opened, and false otherwise.
         ///
         /// Substreams should always be open.
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
         ///  \return CATResult - CAT_SUCCESS on success.
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
         ///  \return CATResult - CAT_SUCCESS on success.
         virtual CATResult WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position);


      protected:
         CATStream*  fParent;
         CATInt64    fOffset;
         CATInt64    fLength;
         CATInt64    fCurPos;
};

#endif // _CATStreamSub_H_


