/// \file  CATDirectSoundIntercept.h
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

#ifndef _CATDirectSoundIntercept_H_
#define _CATDirectSoundIntercept_H_

#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32

struct IDirectSound;
struct IDirectSound8;
struct IDirectMusicSegment;
struct IDirectMusicSegmentState;
struct IDirectMusicPerformance;
/// \class CATDirectSoundIntercept
/// \brief DirectSound function interception
/// \ingroup CAT
class CATDirectSoundIntercept : public CATIntercept
{
    public:
        CATDirectSoundIntercept();
        virtual ~CATDirectSoundIntercept();

        CATResult HookFunctions();

        // Hooked functions:

        /// Captured IDirectSoundBuffer->Play()
        static HRESULT OnPlayBuffer(    CATHOOK*        hookInst, 
                                        IDirectSound*   dsound, 
                                        DWORD           dwReserved1,
                                        DWORD           dwPriority,
                                        DWORD           dwFlags);

        /// Captured IDirectSoundBuffer->Play()
        static HRESULT OnPlayBufferEAX( CATHOOK*        hookInst, 
                                        IDirectSound*   dsound, 
                                        DWORD           dwReserved1,
                                        DWORD           dwPriority,
                                        DWORD           dwFlags);

        /// Captured IDirectSoundBuffer->Lock()
        static HRESULT OnLockBuffer(    CATHOOK*        hookInst,
                                        IDirectSound*   dsound,
                                        DWORD           dwOffset,
                                        DWORD           dwBytes,
                                        LPVOID*         ppvAudioPtr1,
                                        LPDWORD         pdwAudioBytes1,
                                        LPVOID*         ppvAudioPtr2,
                                        LPDWORD         pdwAudioBytes2,
                                        DWORD           dwFlags);

        /// Captured IDirectSoundBuffer->Unlock()
        static HRESULT OnUnlockBuffer(  CATHOOK*        hookInst,
                                        IDirectSound*   dsound,
                                        LPVOID          pvAudioPtr1,
                                        DWORD           dwAudioBytes1,
                                        LPVOID          pvAudioPtr2,
                                        DWORD           dwAudioBytes2);

        /// Captured IDirectMusicPerformance8->PlaySegment
        static HRESULT OnPlaySegment(    CATHOOK*                    hookInst, 
                                         IDirectMusicPerformance*    performance,
                                         IDirectMusicSegment*        pSegment,
                                         DWORD                       dwFlags,
                                         DWORD                       startTimeLow,
                                         DWORD                       startTimeHigh,                                         
                                         IDirectMusicSegmentState**  ppSegmentState);
        
        static HRESULT OnPlaySegmentEx(  CATHOOK*                    hookInst, 
                                         IDirectMusicPerformance*    performance,
                                         IUnknown*                   pSource,
                                         WCHAR*                      pwzSegmentName,
                                         IUnknown*                   pTransition,
                                         DWORD                       dwFlags,
                                         DWORD                       startTimeLow,
                                         DWORD                       startTimeHigh,                                         
                                         IDirectMusicSegmentState**  ppSegmentState,
                                         IUnknown*                   pFrom, 
                                         IUnknown*                   pAudioPath);

    protected:
        


        // DirectSoundCreate8
        CATResult HookDSound(IDirectSound8* tmpDS8,CATINTERCEPT_COM_TABLE_ENTRY* interceptTable);
        typedef HRESULT (WINAPI *DSoundCreate8Func)(LPCGUID lpcGuidDevice,void** ppDS8,LPUNKNOWN pUnkOuter);

        HMODULE                     fDSoundDLL;
        HMODULE                     fDSound3DDLL;
        HMODULE                     fDSound3DEaxDLL;
};

#endif // CAT_CONFIG_WIN32
#endif // _CATDirectSoundIntercept_H_