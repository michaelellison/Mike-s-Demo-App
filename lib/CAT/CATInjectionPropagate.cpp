/// \file  CATInjectionPropagate.cpp
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

#include "CATInjectionPropagate.h"
#ifdef CAT_CONFIG_WIN32
#include "CATString.h"
#include "CATDLLInjector.h"

// Kernel32 intercepts for time warp
CATINTERCEPT_DLL_TABLE_ENTRY CATInjectionPropagate::kKernel32Funcs[] = 
{
    {"CreateProcessW", CATInjectionPropagate::OnCreateProcessW, 5},    
    { 0, 0, 0}
};

CATInjectionPropagate::CATInjectionPropagate(const CATString& dllPath)
{
    fDLLPath    = dllPath;
    fKernelDLL  = ::LoadLibrary(L"kernel32.dll");    
}

CATInjectionPropagate::~CATInjectionPropagate()
{
    RestoreAll();
    if (fKernelDLL)
        FreeLibrary(fKernelDLL);
}

CATResult CATInjectionPropagate::HookFunctions()
{
    CATResult result = CAT_SUCCESS;

    result = InterceptDLL(fKernelDLL,&kKernel32Funcs[0],0);
    if (CATFAILED(result))
        ::OutputDebugString(L"Failed hooking Kernel32.\n");

    return result;
}

CATHOOKFUNC void CATInjectionPropagate::OnCreateProcessW( 
                              CATHOOK*              hookInst,
                              LPCWSTR               lpApplicationName,
                              LPWSTR                lpCommandLine,
                              LPSECURITY_ATTRIBUTES lpProcessAttributes,
                              LPSECURITY_ATTRIBUTES lpThreadAttributes,
                              BOOL                  bInheritHandles,
                              DWORD                 dwCreationFlags,
                              LPVOID                lpEnvironment,
                              LPCWSTR               lpCurrentDirectory,
                              LPSTARTUPINFO         lpStartupInfo,
                              LPPROCESS_INFORMATION lpProcessInformation)
{
    CATHOOK_PROLOGUE(10);

    // Go ahead and create the process as requested
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 10);
    
    // Toss the DLL into it :)
    ProcessFix(hookInst,lpProcessInformation);

    CATHOOK_EPILOGUE_WINAPI(10);
}

void CATInjectionPropagate::ProcessFix(CATHOOK*              hookInst,                               
                                       LPPROCESS_INFORMATION procInf)
{
    if ((procInf) && (procInf->hProcess != 0))
    {
        CATDLLInjector::InjectIntoProcess(((CATInjectionPropagate*)hookInst->InterceptObj)->fDLLPath,procInf->dwProcessId);
    }
}


#endif // CAT_CONFIG_WIN32
