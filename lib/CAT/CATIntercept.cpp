/// \file  CATIntercept.cpp
/// \brief Function interception base class
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#include "CATIntercept.h"
#ifdef CAT_CONFIG_WIN32
#include "CATString.h"


CATIntercept::CATIntercept()
{
    // Make sure we have the rights to tweak protection levels
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {    
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid); 
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);        
        CloseHandle(hToken);
    }
}

CATIntercept::~CATIntercept()
{
    RestoreAll();
}

void CATIntercept::RestoreAll()
{
    while (fHooks.size())
    {
        Restore(fHooks[0]);        
    }
}

CATResult CATIntercept::Intercept( void*        target, 
                                   void*        hookFunc, 
                                   CATUInt32    stubBytes,
                                   CATHOOK*&    newHook,
                                   void*        userParam)
{
    newHook = 0;
    CATHOOK* hookInfo = (CATHOOK*)VirtualAlloc(0,sizeof(CATHOOK),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    
    // Fill with NOPs
    memset(hookInfo,0x90,sizeof(CATHOOK));
    
    hookInfo->HookFunc         = hookFunc;
    hookInfo->Target           = target;
    hookInfo->OrgInstLen       = stubBytes;
    hookInfo->UserParam        = userParam;
    hookInfo->InterceptObj     = this;

    // Check for jump table - if it is, follow it to real code and hook there instead.
    while (*(CATUInt8*)target == 0xe9)
    {
        target = ((CATUInt8*)target + *(CATUInt32*)((CATUInt8*)target +1)) + 5;
        hookInfo->Target = target;
    }
    
    // Add our hookInfo struct to the stack for the hook function
    hookInfo->PrePushHook        = 0x68;
    hookInfo->PrePushHookAddress = (CATUInt32)(UINT_PTR)hookInfo;

    // Add swap between the hook push on the stack and the return address.    
    // xchg eax,[esp]
    // xchg eax,[esp+4]
    // xchg eax,[esp]
    hookInfo->StackSwap1[0] = 0x87240487;
    hookInfo->StackSwap1[1] = 0x87042444;
    hookInfo->StackSwap2    = 0x2404;

    // Set a jump to go to the hook function from our HookInfo struct.
    hookInfo->HookJmp           = 0xe9;
    hookInfo->HookJmpLoc        = ( ((CATUInt32)(UINT_PTR)hookFunc)) -
                                   (((CATUInt32)(UINT_PTR)hookInfo) + offsetof(CATHOOK,HookJmp) + 5);


    // Set up jump to go back to host, after stub bytes
    hookInfo->PostPatchJump    = 0xe9;
    hookInfo->PostPatchJumpLoc = ( ((CATUInt32)(UINT_PTR)target)) -
                                 (((CATUInt32)(UINT_PTR)hookInfo) + offsetof(CATHOOK,PostPatchJump) + 5) + stubBytes;


    // store original bytes
    DWORD oldProtect;
    if (0 == VirtualProtectEx(GetCurrentProcess(),target,stubBytes,PAGE_EXECUTE_READWRITE,&oldProtect))
        return CATRESULT(CAT_ERR_HOOK_PROTECT_FAILED);

    memcpy(hookInfo->OrgInst,target,stubBytes);

    fHooks.push_back(hookInfo);
    newHook = hookInfo;  

    // Patch original function to jump to our CATHOOK struct
    *((CATUInt8*)target)                  = 0xe9;
    *(CATUInt32*)((CATUInt8*)target + 1)  = ((CATUInt32)(UINT_PTR)hookInfo) - (((CATUInt32)(UINT_PTR)target)+5);
    
    if (0 == VirtualProtectEx(GetCurrentProcess(),target,stubBytes,oldProtect,&oldProtect))
        return CATRESULT(CAT_ERR_HOOK_PROTECT_FAILED);

    // Flush cache just in case. 
    FlushInstructionCache(GetCurrentProcess(),0,0);

    return CAT_SUCCESS;
}

CATResult CATIntercept::Restore(CATHOOK*& hookInfo)
{
    if (hookInfo == 0)
        return CAT_ERR_INVALID_PARAM;

    // Remove hook
    DWORD oldProtect;
    
    if (0 == VirtualProtectEx(GetCurrentProcess(),
                              hookInfo->Target,
                              hookInfo->OrgInstLen,
                              PAGE_EXECUTE_READWRITE,
                              &oldProtect))
    {
        return CATRESULT(CAT_ERR_HOOK_PROTECT_FAILED);
    }
    
    memcpy(hookInfo->Target,hookInfo->OrgInst,hookInfo->OrgInstLen);
    
    if (0 == VirtualProtectEx(GetCurrentProcess(),
                              hookInfo->Target,
                              hookInfo->OrgInstLen,
                              oldProtect,
                              &oldProtect))
    {
        return CATRESULT(CAT_ERR_HOOK_PROTECT_FAILED);
    }
    
    FlushInstructionCache(GetCurrentProcess(),0,0);

    // Remove from vector and free it
    std::vector<CATHOOK*>::iterator iter = fHooks.begin();
    while (iter != fHooks.end())
    {
        if ( (*iter) == hookInfo )
        {            
            VirtualFree(hookInfo,sizeof(CATHOOK),MEM_RELEASE);
            hookInfo = 0;
            fHooks.erase(iter);
            return CAT_SUCCESS;
        }
        ++iter;
    }    

    return CATRESULT(CAT_ERR_HOOK_NOT_FOUND);
}

void* CATIntercept::GetFunctionFromVTable(void* objectPtr, CATUInt32 vtableIndex)
{
    UINT_PTR* vtable = (UINT_PTR*)*(UINT_PTR*)objectPtr;
    void *func = 0;
    
    // just return 0 on invalid reference.
    try
    {
        func = (void*)vtable[vtableIndex];
    }
    catch (...)
    {
        func = 0;
    }

    return func;
}

CATResult CATIntercept::InterceptCOMObject(   void*                         comObject,
                                              CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                              void*                         userParam)
{
    if (interceptTable == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    CATResult result = CAT_SUCCESS;
    while (interceptTable->VTableIndex != (CATUInt32)-1)
    {
        void* func = GetFunctionFromVTable(comObject, interceptTable->VTableIndex);
        if (func)
        {
            CATHOOK* tmpHook = 0;
            result = Intercept( func, 
                                interceptTable->HookFunction, 
                                interceptTable->StubLength,                                
                                tmpHook, 
                                userParam);
            if (CATFAILED(result))
                return result;
        }
        interceptTable++;
    }

    return CAT_SUCCESS;
}

// Save the interception data for a COM intercept table to the registry,
// so we don't have to do all the nasty COM creation later.
CATResult CATIntercept::SaveInterceptData (   const CATWChar*               objectName,
                                              void*                         comObject,
                                              CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                              void*                         userParam)
{
    if (interceptTable == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    // Create keys to 
    HKEY interceptKey = 0;
    ::RegCreateKeyEx(HKEY_CURRENT_USER,
                     L"SOFTWARE\\GameAccessSuite\\CATIntercept",
                     0,0,
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS,
                     NULL,
                     &interceptKey,
                     0);

    if (interceptKey == 0)
        return CAT_ERROR;
    
    HKEY objectKey = 0;
    ::RegCreateKeyEx(interceptKey,objectName,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&objectKey,0);

    if (objectKey == 0)
    {
        ::RegCloseKey(interceptKey);
        return CAT_ERROR;
    }


    CATResult result = CAT_SUCCESS;
    while (interceptTable->VTableIndex != (CATUInt32)-1)
    {
        void* func = GetFunctionFromVTable(comObject, interceptTable->VTableIndex);

        CATString vindex = interceptTable->VTableIndex;

        ::RegSetValueEx(objectKey,vindex,0,REG_DWORD,(BYTE*)&func,4);

        interceptTable++;
    }

    ::RegCloseKey(objectKey);
    ::RegCloseKey(interceptKey);
    return CAT_SUCCESS;
}

/// Load interception data from the registry if it's available
CATResult CATIntercept::LoadAndHook       (   const CATWChar*               objectName,
                                              CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                              void*                         userParam)
{
    if (interceptTable == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    // Create keys to 
    HKEY interceptKey = 0;
    ::RegCreateKeyEx(HKEY_CURRENT_USER,
                     L"SOFTWARE\\GameAccessSuite\\CATIntercept",
                     0,0,
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS,
                     NULL,
                     &interceptKey,
                     0);

    if (interceptKey == 0)
        return CAT_ERROR;
    
    HKEY objectKey = 0;
    ::RegCreateKeyEx(interceptKey,objectName,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&objectKey,0);

    if (objectKey == 0)
    {
        ::RegCloseKey(interceptKey);
        return CAT_ERROR;
    }

    CATINTERCEPT_COM_TABLE_ENTRY* start = interceptTable;
    CATResult result = CAT_SUCCESS;
    // First, scan for all requested functions. If any aren't there, going to have to do it from scratch.
    while (interceptTable->VTableIndex != (CATUInt32)-1)
    {
        CATString vindex = interceptTable->VTableIndex;
        void* func = 0;
        DWORD type = 0;
        DWORD dataLen = 4;
        ::RegQueryValueEx(objectKey,vindex,0,&type,(BYTE*)&func,&dataLen);
        
        if (func == 0)
        {
            ::RegCloseKey(objectKey);
            ::RegCloseKey(interceptKey);
            return CAT_ERROR;
        }
                    
        interceptTable++;
    }

    // Got 'em.  Do the hook.
    interceptTable = start;
    while (interceptTable->VTableIndex != (CATUInt32)-1)
    {
        CATString vindex = interceptTable->VTableIndex;
        void* func = 0;
        DWORD type = 0;
        DWORD dataLen = 4;
        ::RegQueryValueEx(objectKey,vindex,0,&type,(BYTE*)&func,&dataLen);
        
        if (func)
        {
            CATHOOK* tmpHook = 0;
            result = Intercept( func, 
                                interceptTable->HookFunction, 
                                interceptTable->StubLength,                                
                                tmpHook, 
                                userParam);
            if (CATFAILED(result))
                return result;
        }
                    
        interceptTable++;
    }

    ::RegCloseKey(objectKey);
    ::RegCloseKey(interceptKey);
    return CAT_SUCCESS;
}


CATResult CATIntercept::InterceptDLL(   HMODULE                       module,
                                        CATINTERCEPT_DLL_TABLE_ENTRY* interceptTable,
                                        void*                         userParam)
{
    if (interceptTable == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    CATResult result = CAT_SUCCESS;
    while (interceptTable->FunctionName != 0)
    {
        void* func = ::GetProcAddress(module,interceptTable->FunctionName);
        if (func)
        {
            CATHOOK* tmpHook = 0;
            result = Intercept( func, 
                                interceptTable->HookFunction, 
                                interceptTable->StubLength,                                
                                tmpHook, 
                                userParam);
            if (CATFAILED(result))
                return result;
        }
        interceptTable++;
    }

    return CAT_SUCCESS;
}

#endif //CAT_CONFIG_WIN32
