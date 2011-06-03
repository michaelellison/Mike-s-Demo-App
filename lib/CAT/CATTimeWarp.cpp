/// \file  CATTimeWarp.cpp
/// \brief Time function interception
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATTimeWarp.h"
#include "CATString.h"

#ifdef CAT_CONFIG_WIN32

// WinMM intercepts for time warp
CATINTERCEPT_DLL_TABLE_ENTRY CATTimeWarp::kWinMMFuncs[] =
{
    {"timeGetTime",  CATTimeWarp::OnTimeGetTime,  7},
    { 0, 0, 0}
};

// Kernel32 intercepts for time warp
CATINTERCEPT_DLL_TABLE_ENTRY CATTimeWarp::kKernel32Funcs[] = 
{
    {"QueryPerformanceCounter", CATTimeWarp::OnQueryPerformanceCounter, 5},
    {"GetTickCount",            CATTimeWarp::OnGetTickCount, 5},
    { 0, 0, 0}
};

CATTimeWarp::CATTimeWarp()
{
    fSpeed      = 1.0f;
    fWinmmDLL   = ::LoadLibrary(L"winmm.dll");    
    fKernelDLL  = ::LoadLibrary(L"kernel32.dll");    

    fLastTickCount                   = 0;
    fLastTimeGetTime                 = 0;
    fLastPerfCounter.QuadPart        = 0;

    fLastRealTickCount               = 0;
    fLastRealTimeGetTime             = 0;
    fLastRealPerfCounter.QuadPart    = 0;
}

CATTimeWarp::~CATTimeWarp()
{
    // Unhook before base class, since we're possibly unloading the DLLs
    RestoreAll();

    if (fWinmmDLL)
        FreeLibrary(fWinmmDLL);
    if (fKernelDLL)
        FreeLibrary(fKernelDLL);
}

CATResult CATTimeWarp::HookFunctions()
{
    CATResult result = CAT_SUCCESS;
   
    fLock.Wait();

    // Get starting values
    fLastTickCount               = GetTickCount();
    fLastTimeGetTime             = timeGetTime();
    QueryPerformanceCounter(&fLastPerfCounter);
    
    fLastRealTickCount  = fLastTickCount;
    fLastRealTimeGetTime = fLastTimeGetTime;
    fLastRealPerfCounter = fLastPerfCounter;


    if (this->fWinmmDLL)
    {
        ::OutputDebugString(L"Hooking WinMM time functions...\n");
        result = InterceptDLL(fWinmmDLL,&kWinMMFuncs[0],0);
        if (CATFAILED(result))
            ::OutputDebugString(L"Failed hooking WinMM.\n");
    }
    else
        ::OutputDebugString(L"Failed to load WinMM.\n");

    if (this->fKernelDLL)
    {        
        ::OutputDebugString(L"Hooking Kernel32 performance counter...\n");
        result = InterceptDLL(fKernelDLL,&kKernel32Funcs[0],0);
        if (CATFAILED(result))
            ::OutputDebugString(L"Failed hooking Kernel32.\n");
    }   

    SetSpeed(1.0f);
    fLock.Release();

    return result;
}


void CATTimeWarp::FixupQPC(CATHOOK* hookInst, LARGE_INTEGER* lpCount)
{
    CATTimeWarp* timeWarp = ((CATTimeWarp*)hookInst->InterceptObj);
    timeWarp->AdjustSaveTime(lpCount, 0,0);
}

void CATTimeWarp::FixupTime(CATHOOK* hookInst, DWORD* timePtr)
{   
    CATTimeWarp* timeWarp = ((CATTimeWarp*)hookInst->InterceptObj);
    timeWarp->AdjustSaveTime(0,0,timePtr);
}

void CATTimeWarp::FixupTick(CATHOOK* hookInst, DWORD* tickPtr)
{
    CATTimeWarp* timeWarp = ((CATTimeWarp*)hookInst->InterceptObj);    
    timeWarp->AdjustSaveTime(0,tickPtr,0);
}

CATHOOKFUNC void CATTimeWarp::OnTimeGetTime( CATHOOK* hookInst)
{
    CATHOOK_PROLOGUE(0);

    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 0);

    // Adjust time based on speed.
    __asm
    {
        mov  eax,ebp    // Ptr to return value (push as time argument for fixup)
        sub  eax,4
        push eax
        push hookInst
        call FixupTime
        add  esp,8        
    }

    CATHOOK_EPILOGUE_WINAPI(0);
}

CATHOOKFUNC void CATTimeWarp::OnQueryPerformanceCounter(    CATHOOK*       hookInst,
                                                            LARGE_INTEGER* lpCount)
{
    CATHOOK_PROLOGUE(1);
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 1);

    // No assembler needed since it's a passed argument ;)
    FixupQPC(hookInst,lpCount);

    CATHOOK_EPILOGUE_WINAPI(1);
}


CATHOOKFUNC void CATTimeWarp::OnGetTickCount( CATHOOK* hookInst)
{
    CATHOOK_PROLOGUE(0);

    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 0);

    // Adjust time based on speed
    _asm
    {
        mov  eax,ebp    // Ptr to return value (push as time argument for fixup)
        sub  eax,4
        push eax
        push hookInst
        call FixupTick
        add  esp,8        
    }
    CATHOOK_EPILOGUE_WINAPI(0);
}

CATResult CATTimeWarp::SetSpeed(CATFloat32 speed)
{
    fLock.Wait();
    
//    LARGE_INTEGER perf;
//    QueryPerformanceCounter(&perf);
//    timeGetTime();
//    GetTickCount();
   
    fSpeed = speed;


    fLock.Release();

    return CAT_SUCCESS;
}

CATFloat32 CATTimeWarp::GetSpeed()
{
    fLock.Wait();
    CATFloat32 speed = fSpeed;
    fLock.Release();
    return speed;
}

void CATTimeWarp::AdjustSaveTime(LARGE_INTEGER* lastPerf, DWORD* lastTick, DWORD* lastTime)
{
    fLock.Wait();    
    {
        // Scale interval between last call and now and add to result of last call for each
        // requested (Non-NULL) timer.

        if (lastPerf)
        {            
            LARGE_INTEGER nextPerf;
            nextPerf.QuadPart = (LONGLONG)((lastPerf->QuadPart - fLastRealPerfCounter.QuadPart)*fSpeed 
                                    + fLastPerfCounter.QuadPart);
            

            fLastRealPerfCounter.QuadPart = lastPerf->QuadPart;
            fLastPerfCounter.QuadPart     = nextPerf.QuadPart;
            lastPerf->QuadPart            = nextPerf.QuadPart;
        }

        if (lastTick)
        {
            DWORD nextTick      = (DWORD)(((*lastTick) - fLastRealTickCount)*fSpeed + fLastTickCount);
            

            fLastRealTickCount  = (*lastTick);
            fLastTickCount      = nextTick;
            *lastTick           = nextTick; 
        }
        
        if (lastTime)
        {
            DWORD nextTime       = (DWORD)(((*lastTime) - fLastRealTimeGetTime)*fSpeed + fLastTimeGetTime);
            

            fLastRealTimeGetTime = (*lastTime);
            fLastTimeGetTime     = nextTime;
            *lastTime            = nextTime; 
        }        
    }

    fLock.Release();
}

#endif // CAT_CONFIG_WIN32
