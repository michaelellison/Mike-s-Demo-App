/// \file  CATInjectionPropagate.h
/// \brief Interception of new process creation with injected dll propagation.
/// \ingroup CAT
///
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef CATInjectionPropagate_H_
#define CATInjectionPropagate_H_

#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32

#include "CATString.h"

/// \class CATInjectionPropagate
/// \brief Interception of new process creation with injected dll propagation.
/// \ingroup CAT
///
/// This interception class handles a common and annoying problem with DLL injection - 
/// the application we've injected our DLL goes off and executes something else.
///
/// A lot of games have launchers that must be used to properly start the game. This
/// class helps make sure that when the launcher executes the game itself, our injected
/// DLL will get bounced into the real game as well.
///
/// \todo
/// Currently just handling CreateProcessW().  Add others when we get a chance.
class CATInjectionPropagate : public CATIntercept
{
    public:
        CATInjectionPropagate(const CATString& dllPath);
        virtual ~CATInjectionPropagate();

        /// Hooks all functions.
        ///
        /// \return CAT_SUCCESS on success.
        CATResult       HookFunctions();


    protected:
        /// Hook function - CreateProcess hook
        static void     OnCreateProcessW( CATHOOK*              hookInst,
                                          LPCWSTR               lpApplicationName,
                                          LPWSTR                lpCommandLine,
                                          LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                          LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                          BOOL                  bInheritHandles,
                                          DWORD                 dwCreationFlags,
                                          LPVOID                lpEnvironment,
                                          LPCWSTR               lpCurrentDirectory,
                                          LPSTARTUPINFO         lpStartupInfo,
                                          LPPROCESS_INFORMATION lpProcessInformation);

        
        /// Handler for process creation
        static void ProcessFix(CATHOOK*              hookInst, 
                               LPPROCESS_INFORMATION procInf);

        static CATINTERCEPT_DLL_TABLE_ENTRY kKernel32Funcs[];    ///< Kernel32 functions to hook

    protected:
        HMODULE     fKernelDLL;
        CATString   fDLLPath;
};

#endif // CAT_CONFIG_WIN32
#endif // CATExecutionIntercept_H_