/// \file  CATDLLInjector.h
/// \brief DLL Injection functions for Win32
/// \ingroup CAT
///
/// This class includes utilities useful for injecting DLLs into remote processes on Win32-based
/// platforms.
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATInternal.h"

#ifdef CAT_CONFIG_WIN32

#ifndef _CATDLLInjector_H_
#define _CATDLLInjector_H_

/// \class CATDLLInjector
/// \brief Collection of utility functions for injecting a DLL into a process.
/// \ingroup CAT
///
/// This class contains utility functions to inject a DLL into a process.
///
/// There are several methods available to do this, and all of them have 
/// tradeoffs. The two I expect to be most used are the CreateRemoteThread() 
/// method to force a LoadLibrary() call on a new thread into the target process, 
/// and the CreateProcess() method of creating a process in a halted state, 
/// inserting the DLL, then continuing execution.  The first is embodied in 
/// CATDLLInjector::InjectIntoProcess(), the latter in 
/// CATDLLInjector::StartDLLWithProcess().
///
/// Much of the information on how to do DLL Injection and function hooking
/// was gleaned from Jeffrey Richter's 
/// "Programming Applications for Microsoft Windows" (1572319968). 
/// Additional information is available in John Robbins'
/// "Debugging Applications for Microsoft .NET and Microsoft Windows" (0735615365)
/// and Feng Yuan's
/// "Windows Graphics Programming: Win32 GDI and DirectDraw" (0130869856).
///
/// I'm avoiding the debugger methods and windows hook methods currently for
/// a few reasons (although they may prove useful in the future and will get 
/// tossed in here if so).
///
/// -# Easier to detect, so some games' anti-cheat/anti-crack logic might be
///    more likely to trigger on it than the techniques I've used.
///    ... and some antivirus software for that matter, although I'm not sure why
///    they don't appear to trigger on these techniques.  I'm using F-Secure 2008,
///    and while it complains horribly when some applications toss DLLs into
///    others with hooks, so far it seems ok when my apps do the same with 
///    these techniques...
/// -# Ideally, we want the option to exit the injector process immediately.
///    Both the debugger and windows hook methods of injection make that
///    somewhat more difficult.
/// -# I hadn't found a pretty implementation of the StartDLLWithProcess technique
///    that I liked. Richter discusses the idea, but doesn't give implementation 
///    details in his books that I've seen. Enjoy :) 
///
/// When using StartDLLWithProcess, you'll need the following undecorated function
/// exported from your injected DLL (e.g. use a .def file for it too):
/// \code
/// extern "C" {
///    void*        gPassedData    = 0;
///    unsigned int gPassedDataLen = 0;
///    void _declspec(dllexport) UnpatchProcess(void*        passData,
///                                             unsigned int passDataLen,
///                                             void*        startLoc,
///                                             unsigned int patchSize,
///                                             void*        hostBuffer,
///                                             unsigned int hostProcId)
///    {
///        // Open host process and read the stored bytes into our 
///        // hooked process over the start location, restoring the
///        // original executable code.
///        HANDLE hostProcess = ::OpenProcess(PROCESS_ALL_ACCESS,
///                                           FALSE,
///                                           hostProcId);
///        if (hostProcess != 0)
///        {
///            DWORD amountRead = 0;
///            ReadProcessMemory(hostProcess,hostBuffer,startLoc,patchSize,&amountRead);
///            if (passDataLen && passData)
///            {
///                gPassedDataLen = passDataLen;
///                gPassedData = new BYTE[passDataLen];
///                ::ReadProcessMemory(hostProcess,passData,gPassedData,passDataLen,&amountRead);
///            }
///            CloseHandle(hostProcess);
///        }
///        // Got the data from the parent. Open the wait event and set it.
///        CATWChar eventName[64];
///        wsprintf(eventName,L"UnpatchProcess_%d",hostProcId);
///        HANDLE waitEvent = ::OpenEvent(GENERIC_READ|GENERIC_WRITE,FALSE,eventName);
///        if (waitEvent != 0)
///        {
///            SetEvent(waitEvent);
///            CloseHandle(waitEvent);
///        }
///        // Now, restore the stack and go back to the starting location                
///        __asm
///        {         
///            mov esp,ebp  // Get original base pointer
///            pop ebp      
///            add esp,0x1c // Restore stack point to start of our code
///            popfd
///            popad
///            ret
///        }
///    }
///}
/// \endcode
///
/// \todo
/// Add named-pipe parameter passing (or similar) to injection by pid.
class CATDLLInjector
{
    public:
        /// Retrieves a process id for a process name.
        /// If multiple processes matching the name are present, it
        /// will return the first one.
        ///
        /// \param  processName     process name (e.g. Notepad.exe)
        /// \param  pid             ref to pid, set on success.
        /// \param  procIndex       optional index of process if multiple
        /// \return CATResult       CAT_SUCCESS on success. 
        ///                         CAT_STAT_MULTIPLE_PROCS if multiple.
        static CATResult GetProcessId( const CATWChar* processName,
                                       CATUInt32&      pid,
                                       CATUInt32       procIndex = -1);

        /// Injects the specified DLL into the process.
        /// 
        /// \param  dllPath     Path to DLL file to inject into process
        /// \param  pid         Process ID to inject DLL into
        /// \return CATResult   0 on success
        static CATResult InjectIntoProcess   (const CATWChar* dllPath, 
                                              CATUInt32       pid);


        
        /// Creates a process using the specified execFile path and 
        /// injects the DLL into the process at startup.
        ///
        /// \param  dllPath      Path to DLL file to inject into process
        /// \param  execFile     Path of executable file to start
        /// \param  commandLine  full command line to pass
        /// \param  passData     Ptr to data to pass to DLL, or null for none.
        /// \param  passDataLen  Length of data to pass, or 0 for none.
        /// \return CATResult    0 on success
        static CATResult StartDLLWithProcess (const CATWChar* dllPath, 
                                              const CATWChar* execFile,
                                              const CATWChar* commandLine,
                                              const void*     passData      = 0,
                                              CATUInt32       passDataLen   = 0);

    private:
};



#endif //_CATDLLInjector_H_

#endif //CAT_CONFIG_WIN32
