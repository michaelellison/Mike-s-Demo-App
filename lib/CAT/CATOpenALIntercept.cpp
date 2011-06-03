/// \file  CATOpenALIntercept.cpp
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

#include "CATOpenALIntercept.h"

#ifdef CAT_CONFIG_WIN32

CATINTERCEPT_DLL_TABLE_ENTRY kOpenALIntercept[] =
{
    {"alSourcePlay",    CATOpenALIntercept::OnALSourcePlay,   5},    
    {"alSourceStop",    CATOpenALIntercept::OnALSourceStop,   5},    
    {"alSourceStopV",   CATOpenALIntercept::OnALSourceStopV,  5},    
    {"alQueueBuffers",  CATOpenALIntercept::OnALQueueBuffers, 5},    
    {"alBufferData",    CATOpenALIntercept::OnALBufferData,   5},    
    { 0, 0, 0}
};

CATOpenALIntercept::CATOpenALIntercept()
{
    //(L"ct_oal.dll");
    //(L"OpenAL32.dll");    
    //(L"wrap_oal.dll");    
    fOpenALDLL   = ::LoadLibrary(L"OpenAL32.dll");    
}

CATOpenALIntercept::~CATOpenALIntercept()
{
    // Unhook before base class, since we're possibly unloading the DLLs
    RestoreAll();

    if (fOpenALDLL)
        FreeLibrary(fOpenALDLL);
}

CATResult CATOpenALIntercept::HookFunctions()
{
    CATResult result = CAT_SUCCESS;
    // Hook openAL if it's around

    if (this->fOpenALDLL)
    {
        ::OutputDebugString(L"Hooking OpenAL...\n");
        result = InterceptDLL(fOpenALDLL,&kOpenALIntercept[0],0);
        if (CATFAILED(result))
            ::OutputDebugString(L"Failed hooking OpenAL.\n");
    }
    else
        ::OutputDebugString(L"Failed to load OpenAL.\n");

    return result;
}


CATHOOKFUNC void    CATOpenALIntercept::OnALSourcePlay( CATHOOK*                     hookInst,
                                                        CATUInt32                    sourceId)
{
    CATHOOK_PROLOGUE(1);

    ::OutputDebugString(L"OnALSourcePlay\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 1);

    CATHOOK_EPILOGUE_CDECL(1);
}

CATHOOKFUNC void CATOpenALIntercept::OnALSourcePlayV( CATHOOK*   hookInst,
                                                      CATInt32   numSources,
                                                      CATUInt32* sourceIds)
{
    CATHOOK_PROLOGUE(2);
    ::OutputDebugString(L"OnALSourcePlayV\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 2);
    CATHOOK_EPILOGUE_CDECL(2);
}

CATHOOKFUNC void CATOpenALIntercept::OnALSourceStop( CATHOOK* hookInst,
                                                     CATUInt32 sourceId)
{
    CATHOOK_PROLOGUE(1);

    ::OutputDebugString(L"OnALSourceStop\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 1);
    CATHOOK_EPILOGUE_CDECL(1);
}

CATHOOKFUNC void CATOpenALIntercept::OnALSourceStopV( CATHOOK*   hookInst,
                                                      CATInt32   numSources,
                                                      CATUInt32* sourceIds)
{
    CATHOOK_PROLOGUE(2);

    ::OutputDebugString(L"OnALSourceStopV\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 2);
    CATHOOK_EPILOGUE_CDECL(2);
}

CATHOOKFUNC void CATOpenALIntercept::OnALQueueBuffers( CATHOOK*     hookInst,
                                                       CATUInt32    sourceId,
                                                       CATInt32     numEntries,
                                                       CATUInt32*   bufferIds)
{
    CATHOOK_PROLOGUE(3);

    ::OutputDebugString(L"OnALQueueBuffers\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 3);
    CATHOOK_EPILOGUE_CDECL(3);
}

CATHOOKFUNC void CATOpenALIntercept::OnALBufferData(   CATHOOK*     hookInst,
                                                       CATUInt32    bufferId,
                                                       CATUInt32    format,
                                                       void*        data,
                                                       CATInt32     size,
                                                       CATInt32     frequency)
{
    CATHOOK_PROLOGUE(5);

    //::OutputDebugString(L"OnALBufferData\n");

    CATHOOK_CALLORIGINAL_CDECL(hookInst, 5);
    CATHOOK_EPILOGUE_CDECL(5);
}

#endif // CAT_CONFIG_WIN32
