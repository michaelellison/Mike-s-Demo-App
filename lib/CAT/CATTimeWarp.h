/// \file  CATTimeWarp.h
/// \brief Time function interception, made for slowing down video games.
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATTimeWarp_H_
#define _CATTimeWarp_H_

#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32
#include "CATCritSec.h"

/// \class CATTimeWarp
/// \brief Time function interception, made for slowing down video games.
/// \ingroup CAT
///
/// CATTimeWarp works by intercepting the timing functions that games typically use
/// and scaling them to the desired speed.  
///
/// To avoid the horrible results of negative diff values, it stores the last value 
/// each of the hooked functions returned natively and its internal calculated value.  
/// The next time that function is called, the result is the scaled time difference
/// between the last call and the current call.
///
/// Currently hooking timeGetTime(), GetTickCount(), and QueryPerformanceCounter().
///
/// Usage:
///     -# First call HookFunctions() to start it.  It will hook all of the functions
///        and set the speed to 1.0 (normal).
///     -# Call SetSpeed() with the percentage of normal speed that you wish to
///        run the process at (recommended 0.1f - 1.0f).
///
///
class CATTimeWarp : public CATIntercept
{
    public:
        CATTimeWarp();
        virtual ~CATTimeWarp();

        /// Hooks all functions and sets the speed to 1.0f.
        ///
        /// \return CAT_SUCCESS on success.
        CATResult       HookFunctions();
    
        /// Sets the speed to run the timers at.
        ///
        /// \param speed Percentage of normal speed to run (0 - 1)
        ///              Recommend using 0.1f-1.0f to as range.
        ///
        /// \return CAT_SUCCESS on success.
        CATResult       SetSpeed(CATFloat32 speed);
        CATFloat32      GetSpeed();
               
    protected:        
        /// Adjusts the passed in value(s) according to speed, and saves off the real timer
        /// result and calculated result for the next query.
        ///
        /// \param  lastPerf    On entry, should point to system's result 
        ///                     from QueryPerformanceCounter. 
        ///                     On return, set to scaled counter value.
        ///
        /// \param  lastTick    On entry, should point to the system's result
        ///                     from GetTickCount(). 
        ///                     On return, set to scaled tick count.
        ///
        /// \param  lastTime    On entry, should point to the system's result
        ///                     from timeGetTime().
        ///                     On return, set to scaled time value.
        ///
        /// \note
        /// Null parameters will be ignored.
        void            AdjustSaveTime(             LARGE_INTEGER*  lastPerf, 
                                                    DWORD*          lastTick, 
                                                    DWORD*          lastTime);

        /// Hook function - receives control when timeGetTime is called.
        static void     OnTimeGetTime(              CATHOOK*        hookInst);
        
        /// Hook function - receives control when QueryPerformanceCounter is called.
        static void     OnQueryPerformanceCounter(  CATHOOK*        hookInst,
                                                    LARGE_INTEGER*  lpCount);

        /// Hook function - receives control when GetTickCount is called.
        static void     OnGetTickCount(             CATHOOK*        hookInst);

        /// Adjust timer for QueryPerformanceCounter
        static void     FixupQPC (CATHOOK* hookInst, LARGE_INTEGER* lpCount);

        /// Adjust timer for timeGetTime()
        static void     FixupTime(CATHOOK* hookInst, DWORD* time);        

        /// Adjust timer for GetTickCount()
        static void     FixupTick(CATHOOK* hookInst, DWORD* tick);

    protected:
        static CATINTERCEPT_DLL_TABLE_ENTRY kKernel32Funcs[];    ///< Kernel32 functions to hook
        
        static CATINTERCEPT_DLL_TABLE_ENTRY kWinMMFuncs[];       ///< WinMM functions to hook


        CATCritSec    fLock;                ///< Critical section for timer data
        CATFloat32    fSpeed;               ///< Speed (0.1 - 1.0) to run at
        HMODULE       fWinmmDLL;            ///< winmm.dll module handle
        HMODULE       fKernelDLL;           ///< kernel32.dll module handle

        LARGE_INTEGER fLastPerfCounter;     ///< Last calculated result for QueryPerformanceCounter()
        DWORD         fLastTimeGetTime;     ///< Last calculated result for timeGetTime()
        DWORD         fLastTickCount;       ///< Last calculated result for GetTickCount()

        LARGE_INTEGER fLastRealPerfCounter; ///< Last real system result for QueryPerformanceCounter()
        DWORD         fLastRealTimeGetTime; ///< Last real system result for timeGetTime()
        DWORD         fLastRealTickCount;   ///< Last real system result for GetTickCount()
};

#endif // CAT_CONFIG_WIN32
#endif // _CATTimeWarp_H_