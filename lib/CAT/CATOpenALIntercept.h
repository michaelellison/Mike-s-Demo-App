/// \file  CATOpenALIntercept.h
/// \brief OpenAL function interception
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATOpenALIntercept_H_
#define _CATOpenALIntercept_H_

#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32

/// \class CATOpenALIntercept
/// \brief OpenAL function interception
/// \ingroup CAT
class CATOpenALIntercept : public CATIntercept
{
    public:
        CATOpenALIntercept();
        virtual ~CATOpenALIntercept();

        CATResult HookFunctions();

        // Hooked functions:


        static  void OnALSourcePlay(     CATHOOK*                     hookInst,
                                         CATUInt32                    sourceId);
        
        static  void OnALSourcePlayV(    CATHOOK*   hookInst,
                                         CATInt32   numSources,
                                         CATUInt32* sourceIds);
        
        static  void OnALSourceStop(     CATHOOK* hookInst,
                                         CATUInt32 sourceId);

        static  void OnALSourceStopV(    CATHOOK*   hookInst,
                                         CATInt32   numSources,
                                         CATUInt32* sourceIds);
        
        static  void OnALQueueBuffers(   CATHOOK*     hookInst,
                                         CATUInt32    sourceId,
                                         CATInt32     numEntries,
                                         CATUInt32*   bufferIds);
       
        static void OnALBufferData(       CATHOOK*     hookInst,
                                         CATUInt32    bufferId,
                                         CATUInt32    format,
                                         void*        data,
                                         CATInt32     size,
                                         CATInt32     frequency);

    protected:        
        HMODULE                     fOpenALDLL;
};

#endif // CAT_CONFIG_WIN32
#endif // _CATOpenALIntercept_H_