/// \file CATStream.cpp
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

#include "CATStream.h"
#include "CATStreamSub.h"

/// This is the default substream builder - it creates just CATStreamSub*'s.
CATStream* CATStream::DefSubStreamBuilder(CATInt64   offset, 
                                          CATInt64   length, 
                                          CATStream* parent, 
                                          void*      param1, 
                                          CATUInt32  param2)
{
   CATStream* subStream = 0;
   try
   {
      subStream = new CATStreamSub(offset,length,parent);
   }
   catch (...)
   {
      return 0;
   }
   return subStream;      
}

// CreateSubStream() creates a substream that uses this stream for I/O at
// a specified offset and length.  
//
// You must call ReleaseSubStream()  when you are done with the substream.
CATStream* CATStream::CreateSubStream(CATInt64            streamOffset, 
                                      CATInt64            streamLength,                                           
                                      CATSUBSTREAMBUILDER builder,
                                      void*               param1,
                                      CATUInt32           param2)
{
   CATResult  result    = CAT_SUCCESS;
   CATStream* subStream = 0;
   if (!IsOpen())
   {
      CATASSERT(false,"File must be open to create a sub stream.");
      return 0;
   }
   
   CATASSERT(builder != 0, "Must pass a builder - use the default if nothing else.");
   if (builder == 0)
   {
      return 0;
   }

   subStream = builder(streamOffset,streamLength,this,param1,param2);

   if (subStream != 0)
   {
      if (CATFAILED(result = ((CATStreamSub*)subStream)->OnAcquireSub()))
      {
         delete subStream;
         subStream = 0;
         return 0;
      }
      
      fSubCount++;
      return subStream;
   }

   return 0;
}

// ReleaseSubStream() releases a previously allocated substream.         
//
// \param subStream - ref to sub stream. Set to 0 on successful release.
// \return CATResult - CAT_SUCCESS on success.
CATResult CATStream::ReleaseSubStream( CATStream*& subStream )
{
   CATASSERT(subStream != 0, "Null substream passed to ReleaseSubStream().");
   if (subStream == 0)
   {
      return CATRESULT(CAT_ERR_INVALID_PARAM);
   }

   CATASSERT(fSubCount != 0, "No substreams registered.");
   CATResult result = ((CATStreamSub*)subStream)->OnReleaseSub();   
   delete subStream;
   subStream = 0;
   fSubCount--;

   return result;
}

/// Copy from one stream to another using the specified buffer size
CATResult CATStream::CopyToStream( CATStream*	outputStream, 
                                   CATUInt32	bufSize, 
                                   CATInt64		offset, 
                                   CATInt64		length)
{
    CATResult result;

    if (outputStream == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    if (bufSize == 0)
        bufSize = kCAT_DEFAULT_STREAM_BUF_SIZE;

    this->SeekAbsolute(offset);
    if (length == 0)
    {
        this->Size(length);
        length -= offset;
    }

    CATUInt8 *buffer = new CATUInt8[bufSize];
    if (buffer == 0)
    {
        return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
    }

    while (length)
    {
        CATUInt32 amountRead = bufSize;
        if (length < bufSize)
        {
            amountRead = (CATUInt32)length;
        }

        if (CATFAILED(result = this->Read(buffer,amountRead)))
        {
            delete [] buffer;
            return result;
        }

        if (CATFAILED(result = outputStream->Write(buffer, amountRead)))
        {
            delete [] buffer;
            return result;
        }

        if (amountRead == 0)
            length = 0;

        length -= amountRead;
    }

    delete [] buffer;
    return CATRESULT(CAT_SUCCESS);
}
