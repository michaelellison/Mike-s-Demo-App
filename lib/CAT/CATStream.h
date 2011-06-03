/// \file CATStream.h
/// \brief Base stream interface
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $


#ifndef _CATStream_H_
#define _CATStream_H_

#include "CATInternal.h"
#include "CATString.h"

const int kCAT_DEFAULT_STREAM_BUF_SIZE = 4096;

/// \class CATStream CATStream.h
/// \brief Base interface for streams
/// \ingroup CAT
///
/// CATStream provides a generic stream interface class to be
/// inherited by files, encrypted files, memory streams, and anything
/// else that just needs basic read/write stream operations.
///
/// \todo
/// Need to add test cases to CATTests for all stream classes. They were converted
/// from my old personal library, and haven't really been tested since conversion.
class CATStream
{
    /// CATSUBSTREAMBUILDER defines a factory function that child classes can use to
    /// create substreams of a requested type.  Each stream type should have a
    /// CATStreamXXXX_Builder() function of this type. Pass it into the
    /// CreateSubStream() function to build a substream of that type.
    ///
    /// The meaning of the parameters depends upon the type of substream.
    typedef CATStream* (*CATSUBSTREAMBUILDER)(CATInt64      offset, 
                                              CATInt64      length, 
                                              CATStream*    parent, 
                                              void*         param1, 
                                              CATUInt32     param2);
   public:         
         /// File open modes.
         ///
         /// More are possible, but I'd like to only add as we need them
         /// beyond the basics. If you add any, ensure that conflicts
         /// aren't created between sections.
         ///
         /// You can combine flags from different sections,
         /// but don't mix flags within a section.
         ///
         /// Binary mode is assumed. Currently no support for text mode
         /// is provided. Text mode is fugly - specify your carriage returns
         /// accordingly.
         ///   
         /// Default mode is read only / existing only / shareable.
         enum OPEN_MODE
         {
            //-------------------------
            // Read / write / create
            READ_ONLY                 = 0x0,
            READ_WRITE_EXISTING_ONLY  = 0x1,
            READ_WRITE_EXISTING_FIRST = 0x2,
            READ_WRITE_CREATE_TRUNC   = 0x3,
			WRITE_CREATE_ONLY	      = 0x4,
            //-------------------------
            // Share flags
            //-------------------------
            SHARE_ALL            = 0x0,
            SHARE_NONE           = 0x100
         };

         CATStream()
         {
            fSubCount = 0;
         }

         virtual ~CATStream()
         {            
            CATASSERT(fSubCount == 0, "SubStreams are still active.");
         }

         /// Open() opens a stream from a pathname.
         ///
         /// Call close when done.
         ///
         /// \param pathname - Ptr to path string.  Platform-specific
         ///        issues should be taken care of by child classes if needed.
         /// \param mode - combination of OPEN_MODE enumerated flags.
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa Close()
         virtual CATResult Open(const CATWChar* pathname, OPEN_MODE mode)   = 0;
         
         /// Close() closes a previously opened stream.
         /// 
         /// Stream must have been previously successfuly opened.
         ///
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa Open()
         virtual CATResult Close() = 0;

         /// IsOpen() returns true if the stream has been opened, and false otherwise.
         ///
         /// \return bool - true if stream is open.
         /// \sa Open(), Close()
         virtual bool     IsOpen() = 0;

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
         virtual CATResult Read(void* buffer, CATUInt32& length) = 0;

         /// Write() writes the requested amount of data from a buffer.
         ///
         /// Incomplete writes are treated as an error.
         /// 
         /// \param buffer - source buffer to write from.
         /// \param length - length of data to write.
         /// \return CATResult - CAT_SUCCESS on success
         /// \sa Read()
         virtual CATResult Write(const void* buffer, CATUInt32 length) = 0;
         
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
         virtual CATResult ReadAbs(void *buffer, CATUInt32& length, CATInt64 position) = 0;

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
         virtual CATResult WriteAbs(const void *buffer, CATUInt32 length, CATInt64 position) = 0;

         /// CreateSubStream() creates a substream that uses this stream for I/O at
         /// a specified offset and length.  
         ///
         /// You must call ReleaseSubStream()  when you are done with the substream.
         ///
         /// \param streamOffset - offset within the stream that should be the start of the
         ///                       substream. This is an absolute position, and is not
         ///                       related to the stream's current position.         
         ///
         /// \param streamLength - length of the substream. If set to -1, it will 
         ///                       use the entire length of the parent stream.
         /// \param  builder - substream builder for specialized substream types
         /// \param  param1 - void*, meaning depends on substream type
         /// \param param2 - CATUInt32, meaning depends on substream type
         /// \return CATStream* - ptr to new stream or null on failure
         /// \sa ReleaseSubStream()
         virtual CATStream* CreateSubStream(  CATInt64            streamOffset, 
                                              CATInt64            streamLength,                                           
                                              CATSUBSTREAMBUILDER builder = CATStream::DefSubStreamBuilder,
                                              void*               param1  = 0,
                                              CATUInt32           param2  = 0);

         /// ReleaseSubStream() releases a previously allocated substream.         
         ///
         /// \param subStream - ref to sub stream. Set to 0 on successful release.
         /// \return CATResult - CAT_SUCCESS on success.
         virtual CATResult ReleaseSubStream( CATStream*& subStream );


         /// Size() returns the size of the object in filesize.
         ///
         /// This may not be supported on all stream types.
         /// \param filesize - 64-bit length of stream.
         /// \return CATResult - CAT_SUCCESS on success
         virtual CATResult Size(CATInt64& filesize) = 0;

         /// IsSeekable() returns true if the stream is a seekable type.
         ///
         /// Some streams might not be seekable - check before seeking.
         ///
         /// \return bool - true if stream is seekable.
         /// \sa SeekRelative(), SeekAbsolute(), SeekFromEnd()
         virtual bool     IsSeekable() = 0;
         
         /// SeekRelative() seeks from current position to a
         /// relative location.
         ///
         /// Some streams might not be seekable - see IsSeekable().
         ///
         /// \param offset - signed offset from current position
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekAbsolute(), SeekFromEnd()
         virtual CATResult SeekRelative(CATInt32  offset) = 0;


         /// SeekAbsolute() seeks from the start of the stream
         /// to an absolute position.
         ///
         /// Some streams might not be seekable - see IsSeekable().
         ///
         /// \param position - unsigned absolute position to seek to.
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekRelative(), SeekFromEnd()
         virtual CATResult SeekAbsolute(CATInt64 position) = 0;

         /// SeekFromEnd() seeks from the end of the stream.
         ///
         /// For example, an offset of 5 will be 5 bytes before
         /// the end of the stream.
         ///
         /// Some streams might not be seekable - see IsSeekable()
         ///
         /// \param offset - signed offset from end of stream
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), SeekRelative(), SeekAbsolute()
         virtual CATResult SeekFromEnd(CATInt32 offset) = 0;        
         
         /// GetPosition() returns the current position in the stream
         /// in position.
         ///
         /// Stream must be seekable to know the position.
         ///
         /// \param position - current position - set on successful return.
         /// \return CATResult - CAT_SUCCESS on success.
         /// \sa IsSeekable(), Size()
         virtual CATResult GetPosition(CATInt64& position) = 0;


         /// GetName() retrieves the name of the stream.
         ///
         /// Typically, this is a path or filename type name.
         ///
         virtual CATString GetName() const = 0;

		 /// Copy from one stream to another using the specified buffer size
		 virtual CATResult CopyToStream( CATStream*  outputStream, 
                                         CATUInt32   bufSize     = kCAT_DEFAULT_STREAM_BUF_SIZE, 
                                         CATInt64    offset      = 0, 
                                         CATInt64    length      = 0);

         /// This is the default substream builder - it creates just CATStreamSub*'s.
         static CATStream* DefSubStreamBuilder( CATInt64   offset, 
                                                CATInt64   length, 
                                                CATStream* parent, 
                                                void*      param1, 
                                                CATUInt32  param2);
   protected:
      CATUInt32  fSubCount;   

   private:
      CATStream& operator=(const CATStream& stream)
      {
         CATASSERT(false,"Copying not currently supported.");
         return *this;
      }

      
};


#endif // _CATStream_H_
