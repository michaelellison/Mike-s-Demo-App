/// \file  CATDirectSoundIntercept.cpp
/// \brief DirectSound function interception
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATDirectSoundIntercept.h"
#ifdef CAT_CONFIG_WIN32

#define DIRECTSOUND_VERSION 0x0900  /* Version 9.0 */
#define INITGUID
#include <dsound.h>
#undef INITGUID
#include <dmusicc.h>
#include <dmusici.h>
#include <cguid.h>

/// This table contains all of the functions we wish to intercept
/// from IDirectSoundBuffer and the targets for those functions.
CATINTERCEPT_COM_TABLE_ENTRY kBufferInterceptTable[] =
{
    // VTableIndex  HookFunction                                 StubLength
    {  12,          CATDirectSoundIntercept::OnPlayBuffer,       5},  
//    {  11,          CATDirectSoundIntercept::OnLockBuffer,       5},
//    {  19,          CATDirectSoundIntercept::OnUnlockBuffer,     5},

    // End marker
    {(CATUInt32)-1,0,-1}
};

/// This table contains all of the functions we wish to intercept
/// from IDirectSoundBuffer and the targets for those functions.
CATINTERCEPT_COM_TABLE_ENTRY kEAXBufferInterceptTable[] =
{
    // VTableIndex  HookFunction                                 StubLength
    {  12,          CATDirectSoundIntercept::OnPlayBufferEAX,       5},  
//    {  11,          CATDirectSoundIntercept::OnLockBufferEAX,       5},
//    {  19,          CATDirectSoundIntercept::OnUnlockBufferEAX,     5},

    // End marker
    {(CATUInt32)-1,0,-1}
};

CATINTERCEPT_COM_TABLE_ENTRY kPerformanceInterceptTable[] =
{
    // VTableIndex  HookFunction                                 StubLength
    {  4,          CATDirectSoundIntercept::OnPlaySegment,       5},  
    {  46,         CATDirectSoundIntercept::OnPlaySegmentEx,     5},
    // End marker
    {(CATUInt32)-1,0,-1}
};

CATDirectSoundIntercept::CATDirectSoundIntercept()
{
    CoInitializeEx(0,0);
    fDSoundDLL   = ::LoadLibrary(L"dsound.dll");
    fDSound3DDLL = ::LoadLibrary(L"dsound3d.dll");
    fDSound3DEaxDLL = ::LoadLibrary(L"eax.dll");
}

CATDirectSoundIntercept::~CATDirectSoundIntercept()
{
    // Unhook before base class, since we're possibly unloading the DLLs
    RestoreAll();

    // Now unload the DLLs and free our count on COM
    if (fDSound3DDLL)
        FreeLibrary(fDSound3DDLL);

    if (fDSoundDLL)
        FreeLibrary(fDSoundDLL);

    if (fDSound3DEaxDLL)
        FreeLibrary(fDSound3DEaxDLL);

    CoUninitialize();
}

CATResult CATDirectSoundIntercept::HookFunctions()
{
    // Here we snag all the pointers to the DirectSound objects we're interested in
    // for the default audio device.  The objects themselves may be released afterwards -
    // we just need the addresses of their vtables to play with.
    DSoundCreate8Func           tmpDSoundCreate8   = 0;
    IDirectSound8*              tmpDS8             = 0;

    if (!fDSoundDLL)
        return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);

    tmpDSoundCreate8 = (DSoundCreate8Func)::GetProcAddress(fDSoundDLL,"DirectSoundCreate8");
    if (!tmpDSoundCreate8)
        return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);

    tmpDSoundCreate8(0,(void**)&tmpDS8, 0);
    if (tmpDS8 == 0)
        return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);    

    tmpDS8->SetCooperativeLevel(::GetDesktopWindow(),DSSCL_PRIORITY);

    ::OutputDebugString(L"Hooking DirectSound8...\n");
    CATResult result = HookDSound(tmpDS8,&kBufferInterceptTable[0]);
    if (CATFAILED(result))
        ::OutputDebugString(L"Failed hooking DirectSound8.\n");

    if (tmpDS8)
        tmpDS8->Release();

    if (fDSound3DEaxDLL)
    {
        // Now do the same for EAX.
        tmpDSoundCreate8 = (DSoundCreate8Func)::GetProcAddress(fDSound3DEaxDLL,"EAXDirectSoundCreate8");
        if (tmpDSoundCreate8)
        {
            tmpDSoundCreate8(0,(void**)&tmpDS8, 0);
            if (tmpDS8)
            {
                tmpDS8->SetCooperativeLevel(::GetDesktopWindow(),DSSCL_PRIORITY);
                ::OutputDebugString(L"Hooking EAX DirectSound...\n");
                result = HookDSound(tmpDS8,&kEAXBufferInterceptTable[0]);
                if (CATFAILED(result))
                    ::OutputDebugString(L"Failed hooking EAX DirectSound.\n");
                tmpDS8->Release();
            }
        }
    }


    // Next.... hit the DirectMusic stuff, not sure how commonly used it is, but it appears to be used in some 
    // cases.

    IDirectMusicPerformance8*   tmpPerformance = 0;
    
    CoCreateInstance(CLSID_DirectMusicPerformance, 
                     NULL, 
                     CLSCTX_INPROC,
                     IID_IDirectMusicPerformance8, 
                     (void**)&tmpPerformance );

    if (tmpPerformance)
    {
        result = InterceptCOMObject(tmpPerformance,&kPerformanceInterceptTable[0],0);
        
        tmpPerformance->Release();
    }
    
    return result;
}

CATResult CATDirectSoundIntercept::HookDSound(IDirectSound8* tmpDS8,CATINTERCEPT_COM_TABLE_ENTRY* interceptTable)
{    
    IDirectSoundBuffer*         tmpDSPrimaryBuffer = 0;
    IDirectSoundBuffer*         tmpDSBuffer        = 0;
    IDirectSoundBuffer8*        tmpDSBuffer8       = 0;
    IDirectSound3DBuffer*       tmpDSBuffer3D      = 0;
    IDirectSound3DListener*     tmpDSListener3D    = 0;
    // Get the primary buffer
    DSBUFFERDESC bufferDesc; 
    memset(&bufferDesc, 0, sizeof(DSBUFFERDESC)); 
    bufferDesc.dwSize        = sizeof(DSBUFFERDESC); 
    bufferDesc.dwFlags       = DSBCAPS_CTRL3D |  DSBCAPS_PRIMARYBUFFER;
    
    HRESULT hr = S_OK;
    // Create primary DirectSound buffer
    if (SUCCEEDED(hr = tmpDS8->CreateSoundBuffer(&bufferDesc, &tmpDSPrimaryBuffer, NULL)))
    { 
        // Set the wave format for primary
        WAVEFORMATEX waveFormat; 
        memset(&waveFormat, 0, sizeof(WAVEFORMATEX)); 
        waveFormat.wFormatTag       = WAVE_FORMAT_PCM; 
        waveFormat.nChannels        = 2; 
        waveFormat.nSamplesPerSec   = 22050; 
        waveFormat.nBlockAlign      = 4; 
        waveFormat.wBitsPerSample   = 16; 
        waveFormat.nAvgBytesPerSec  = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; 
        hr = tmpDSPrimaryBuffer->SetFormat(&waveFormat);

        // Get the listener object from the primary buffer
        hr = tmpDSPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&tmpDSListener3D);           

        // Now create a buffer... single channel on this one so it can be 3D.
        waveFormat.nChannels   = 1;
        waveFormat.nBlockAlign = 2;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; 

        bufferDesc.dwFlags          = DSBCAPS_CTRL3D;
        bufferDesc.lpwfxFormat      = &waveFormat;
        bufferDesc.guid3DAlgorithm  = DS3DALG_NO_VIRTUALIZATION;
        bufferDesc.dwBufferBytes    = waveFormat.nAvgBytesPerSec;
        hr = tmpDS8->CreateSoundBuffer(&bufferDesc,&tmpDSBuffer,0);
        if (tmpDSBuffer)
        {
            // Get a pointer to the DirectSound8 interface if available
            hr = tmpDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&tmpDSBuffer8);

            // Get the 3D buffer interface
            hr = tmpDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,   (LPVOID*)&tmpDSBuffer3D);
        }
    } 

    // Got all the interfaces now... find the functions we're interested in...
    CATResult result;
    result = InterceptCOMObject(tmpDSBuffer,interceptTable,0);


    // Clean up
    if (tmpDSBuffer)
        tmpDSBuffer->Release();

    if (tmpDSBuffer3D)
        tmpDSBuffer3D->Release();

    if (tmpDSListener3D)
        tmpDSListener3D->Release();
    
    if (tmpDSBuffer8)
        tmpDSBuffer8->Release();

    if (tmpDSPrimaryBuffer)
        tmpDSPrimaryBuffer->Release();

    return result;
}


CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnPlayBuffer(CATHOOK*      hookInst,
                                                          IDirectSound* dsound,
                                                          DWORD         dwReserved1,
                                                          DWORD         dwPriority,
                                                          DWORD         dwFlags)
{
    CATHOOK_PROLOGUE(4);
    ::OutputDebugString(L"ds:Play\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 4);
    CATHOOK_EPILOGUE_WINAPI(4);
}

CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnPlayBufferEAX(CATHOOK*      hookInst,
                                                          IDirectSound* dsound,
                                                          DWORD         dwReserved1,
                                                          DWORD         dwPriority,
                                                          DWORD         dwFlags)
{
    CATHOOK_PROLOGUE(4);
    ::OutputDebugString(L"eax:Play\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 4);
    CATHOOK_EPILOGUE_WINAPI(4);
}

CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnLockBuffer(CATHOOK*        hookInst,
                                                          IDirectSound*   dsound,
                                                          DWORD           dwOffset,
                                                          DWORD           dwBytes,
                                                          LPVOID*         ppvAudioPtr1,
                                                          LPDWORD         pdwAudioBytes1,
                                                          LPVOID*         ppvAudioPtr2,
                                                          LPDWORD         pdwAudioBytes2,
                                                          DWORD           dwFlags)
{
    CATHOOK_PROLOGUE(8);
    //::OutputDebugString(L"ds:Lock\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 8);


    CATHOOK_EPILOGUE_WINAPI(8);
}
        
CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnUnlockBuffer(  CATHOOK*        hookInst,
                                                              IDirectSound*   dsound,
                                                              LPVOID          pvAudioPtr1,
                                                              DWORD           dwAudioBytes1,
                                                              LPVOID          pvAudioPtr2,
                                                              DWORD           dwAudioBytes2)
{
    CATHOOK_PROLOGUE(5);
    //::OutputDebugString(L"ds:Unlock\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 5);


    CATHOOK_EPILOGUE_WINAPI(5);
}

CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnPlaySegment(  CATHOOK*                    hookInst, 
                                                             IDirectMusicPerformance*    performance,
                                                             IDirectMusicSegment*        pSegment,
                                                             DWORD                       dwFlags,
                                                             DWORD                       startTimeLow,
                                                             DWORD                       startTimeHigh,                                         
                                                             IDirectMusicSegmentState**  ppSegmentState)
{
    CATHOOK_PROLOGUE(6);
    ::OutputDebugString(L"ds:PlaySegment\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 6);


    CATHOOK_EPILOGUE_WINAPI(6);
}

CATHOOKFUNC HRESULT CATDirectSoundIntercept::OnPlaySegmentEx( CATHOOK*                    hookInst, 
                                                              IDirectMusicPerformance*    performance,
                                                              IUnknown*                   pSource,
                                                              WCHAR*                      pwzSegmentName,
                                                              IUnknown*                   pTransition,
                                                              DWORD                       dwFlags,
                                                              DWORD                       startTimeLow,
                                                              DWORD                       startTimeHigh,                                         
                                                              IDirectMusicSegmentState**  ppSegmentState,
                                                              IUnknown*                   pFrom, 
                                                              IUnknown*                   pAudioPath)
{
    CATHOOK_PROLOGUE(10);
    ::OutputDebugString(L"ds:PlaySegmentEx\n");
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 10);


    CATHOOK_EPILOGUE_WINAPI(10);
}

#endif // CAT_CONFIG_WIN32
