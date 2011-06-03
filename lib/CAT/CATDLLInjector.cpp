/// \file  CATDLLInjector.cpp
/// \brief DLL Injection functions for Win32
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATDLLInjector.h"
#include "CATStreamFile.h"

// Only compile for win32 platform.
#ifdef CAT_CONFIG_WIN32
#include <psapi.h>

// Injects the specified DLL into the process.
CATResult CATDLLInjector::InjectIntoProcess(const CATWChar* dllPath, 
                                            CATUInt32       pid)
{
    // Get full pathname for DLL
    CATWChar dllFile[_MAX_PATH+1];
    LPWSTR* filePart = 0;
    if (0 == ::GetFullPathName(dllPath,_MAX_PATH+1,dllFile,filePart))
    {
        return CAT_ERR_FILE_NOT_FOUND;        
    }

    HMODULE testLoad = 0;
    if (0 == (testLoad = LoadLibraryEx(dllFile, 0, LOAD_LIBRARY_AS_DATAFILE)))
    {
        return CAT_ERR_FILE_NOT_FOUND;
    }
    FreeLibrary(testLoad);

    // Give ourselves debug rights on other processes.
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
                         &hToken))
    {    
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid); 
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
        AdjustTokenPrivileges(hToken, 
                              FALSE, 
                              &tkp, 
                              0, 
                              (PTOKEN_PRIVILEGES) NULL, 
                              0);        
        CloseHandle(hToken);
    }

    // Open the requested process
    HANDLE proc = ::OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
    if (NULL == proc)
    {
        return CAT_ERR_UNABLE_TO_OPEN_PROCESS;        
    }

    // Allocate some memory for name of DLL
    SIZE_T dllNameLen = (wcslen(dllFile)+1)*sizeof(CATWChar);

    LPVOID procMem = ::VirtualAllocEx(proc,
                                      0,
                                      dllNameLen,
                                      MEM_RESERVE|MEM_COMMIT,
                                      PAGE_READWRITE);
    if (!procMem)
    {
        CloseHandle(proc);        
        return CAT_ERR_REMOTE_ALLOC_RAM;
    }

    // Write name to process and start a LoadLibraryW pointing at our DLL.
    SIZE_T amountWritten = 0;
    if (!WriteProcessMemory(proc,procMem,dllFile,dllNameLen,&amountWritten))
    {
        CloseHandle(proc);
        return CAT_ERR_REMOTE_WRITE;
    }

    // Get load library address (using unicode)
    FARPROC loadLibFunc = ::GetProcAddress(
                                ::GetModuleHandle(L"kernel32.dll"),
                                "LoadLibraryW");

    // Create the remote thread at the LoadLibraryW function's start address 
    // with param of the memory we filled with the name of the .dll
    DWORD threadId = 0;
    HANDLE remoteThread = ::CreateRemoteThread(
                                         proc,
                                         0,
                                         0,
                                         (LPTHREAD_START_ROUTINE)loadLibFunc,
                                         procMem,
                                         0,
                                         &threadId);

    if (remoteThread == 0)
    {
        CloseHandle(proc);
        return CAT_ERR_REMOTE_CREATE_THREAD;
    }

    // Wait for remote thread to exit (e.g. for load library to complete)
    DWORD waitResult;
    waitResult = WaitForSingleObject(remoteThread,30000);

    if (waitResult != WAIT_OBJECT_0)
    {
        CloseHandle(proc);
        CloseHandle(remoteThread);
        return CAT_ERR_REMOTE_THREAD_TIMEOUT;
    }

    // Check exit code on LoadLibrary    
    CATResult result = 0;
    DWORD exitCode   = 0;

    ::GetExitCodeThread(remoteThread,&exitCode);
    if (exitCode == 0)
    {
        result = CAT_ERR_REMOTE_THREAD_INVALID_EXIT;
    }
    else
    {
        result = (CATResult)exitCode;
    }
           
    // Clean up, we're done.
    CloseHandle(remoteThread);
    ::VirtualFreeEx(proc,procMem,wcslen(dllFile)*sizeof(CATWChar),MEM_RELEASE);
    CloseHandle(proc);

    return exitCode;
}

CATResult CATDLLInjector::GetProcessId(const CATWChar *processName, 
                                       CATUInt32&      pid, 
                                       CATUInt32       procIndex)
{   
    CATASSERT(processName != 0, "Invalid process name passed to GetProcessId()");
    if (processName == 0)
        return CATRESULT(CAT_ERR_INVALID_PARAM);

    DWORD     procIdArray[1024];
    DWORD     lengthNeeded = 0;
    CATUInt32 numFound     = 0;
    CATResult result       = CAT_SUCCESS;   

    pid = 0;


    // Get process IDs into procIdArray (1024 seems like enough... stupid API)
    if ( !EnumProcesses( procIdArray, 
                         sizeof(procIdArray), 
                         &lengthNeeded ) )
    {
        return CATRESULT(CAT_ERR_ENUM_PROCS);
    }

    DWORD numProcs = lengthNeeded / sizeof(DWORD);

    CATWChar testName[_MAX_PATH+1];

    CATUInt32 curProcIndex = 0;

    // Loop through all procIds, open the processes and 
    // check their names against the one passed in.
    for ( DWORD i = 0; i < numProcs; i++ )
    {
        HANDLE procHandle = OpenProcess( PROCESS_QUERY_INFORMATION |
                                         PROCESS_VM_READ,
                                         FALSE, procIdArray[i] );

        if (NULL != procHandle )
        {
            HMODULE module       = 0;
            DWORD   lengthNeeded = 0;

            if ( EnumProcessModules( procHandle, 
                                     &module, 
                                     sizeof(module), 
                                     &lengthNeeded) )
            {
                memset(testName,0,sizeof(testName));
                GetModuleBaseName( procHandle, 
                                   module, 
                                   testName,
                                   sizeof(testName) );

                if (0 == _wcsicmp(testName, processName))
                {
                    // Got a match. If we aren't being picky (e.g. procIndex == -1)
                    // or we've found the requested index, set it.
                    if ((procIndex == -1) || (curProcIndex == procIndex))
                    {
                        pid = procIdArray[i];                        
                    }
                    curProcIndex++;
                    numFound++;
                }
            }
            else
            {
               CloseHandle(procHandle);
            }
        }
        else
        {
            continue;
        }
    }

    if (numFound == 0)
    {
        return CATRESULT(CAT_ERR_NO_MATCHING_PROC);
    }
    else if (numFound == 1)
    {
        return CAT_SUCCESS;
    }

    return CATRESULT(CAT_STAT_MULTIPLE_PROCS);
}


/// kAsmPatch is used by StartDLLWithProcess to force a load of our
/// target DLL before the main thread of the application is started.
/// It then calls UnpatchProcess() in the DLL to restore the 
/// executable and continue its execution.
///
const CATUInt8 kAsmPatch[] =
{
                               // off desc
    
    // Put our starting location, registers, and flags onto the stack
    // so we can restore them.  Extra NOPs are in case we forget something :)
    // 
    0x90,                      // 0   Breakpoint for debugging or NOP
    0x68,		 			   // 1   push dword
    0x00,0x00,0x00,0x00,       // 2     kStartLocReturn
                               //     Pushes our start location onto the stack
    0x60,0x9c,                 // 6   pushad, pushfd, save starting regs/flags
    0x90,0x90,0x90,            // 8   nop    

    // First, call load library for the target DLL file
    0x68,		 			   // 11  push dword
    0x00,0x00,0x00,0x00,       // 12    kDLLNameOffAddr:
                               //     this is the address part.... 
                               //     point it to the full path of marker dll
    
    0xff,0x15,                 // 16  far call [address]    
    0x00,0x00,0x00,0x00,       // 18    kLoadLibraryOffset:                                 
                               //     point this one to the LoadLibrary 
                               //     function address

    // Now, get the proc address for UnpatchProcess() in the DLL
    0x68,		 			   // 22  push dword
    0x00,0x00,0x00,0x00,       // 23    kUnpatchFuncOffset
                               //     point it to the full path of marker dll  
    0x50,                      // 27  Push EAX (module handle)

    0xff,0x15,                 // 28  far call [address]
    0x00,0x00,0x00,0x00,       // 30    kGetProcAddressOffset:
                               

    // Now, push the params for UnpatchProcess onto the stack and 
    // call into it. It will restore control to the host from there.        
    0x68,		 			   // 34   push dword 
    0x00,0x00,0x00,0x00,       // 35     kHostProcOffset
    0x68,		 			   // 39   push dword 
    0x00,0x00,0x00,0x00,       // 40     kHostBufferOffset
    0x68,	                   // 44   push dword 
    0x00,0x00,0x00,0x00,       // 45     kPatchSizeOffset
    0x68,	                   // 49   push dword 
    0x00,0x00,0x00,0x00,       // 50     kStartLocOffset
    0x68,                      // 54   push dword
    0x00,0x00,0x00,0x00,       // 55     kPassDataLenOffset
    0x68,                      // 59   push dword
    0x00,0x00,0x00,0x00,       // 60     kPassDataOffset
    
    0xff,0xd0,                 // 64   call eax
    0xcc,0x90,                 // 66   Debug break if it returns...
        
    0x00,0x00,0x00,0x00,       // 68  kLoadLibraryAddress: 
    0x00,0x00,0x00,0x00,       // 72  kGetProcAddress:
    
    0x55,0x6E,0x70,0x61,0x74,  // 76 'UnpatchProcess' function name
    0x63,0x68,0x50,0x72,0x6F,  //
    0x63,0x65,0x73,0x73,0x00   //
};

// Offsets into patch of variables to set values to
const int kStartLocReturn        = 2;
const int kDLLNameOffAddr        = 12;
const int kLoadLibraryOffset     = 18;
const int kUnpatchFuncOffset     = 23;
const int kGetProcAddressOffset  = 30;
const int kHostProcOffset        = 35;
const int kHostBufferOffset      = 40;
const int kPatchSizeOffset       = 45;
const int kStartLocOffset        = 50;
const int kPassDataLenOffset     = 55;
const int kPassDataOffset        = 60;
const int kLoadLibraryAddress    = 68;
const int kGetProcAddress        = 72;
const int kUnpatchFuncNameAddress= 76;
const int kDLLNameOffset         = sizeof(kAsmPatch);

// Creates a process using the specified execFile path and 
// injects the DLL into the process at startup.
CATResult CATDLLInjector::StartDLLWithProcess (const CATWChar* dllPath, 
                                               const CATWChar* execFile,
                                               const CATWChar* commandLine,
                                               const void*     passData,
                                               CATUInt32       passDataLen)
{  
    // Tweak our protection levels, if possible.
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

    CATResult result;

    // Get full name of .exe file
    CATWChar targDir[_MAX_PATH+1];
    wcscpy(targDir,execFile);
    CATWChar* slash = wcsrchr(targDir,'\\');
    if (slash != 0)
    {
        *slash = 0;
    }
    

    // Get full pathname for DLL
    CATWChar dllFile[_MAX_PATH+1];
    LPWSTR filePart = 0;
    if (0 == ::GetFullPathName(dllPath,_MAX_PATH+1,dllFile,&filePart))
    {
        return CATRESULTFILE(CAT_ERR_FILE_NOT_FOUND,dllPath);        
    }
    
    HMODULE testLoad = 0;
    if (0 == (testLoad = LoadLibraryEx(dllFile, 0, LOAD_LIBRARY_AS_DATAFILE)))
    {
        return CATRESULTFILE(CAT_ERR_FILE_NOT_FOUND,dllFile);
    }
    FreeLibrary(testLoad);


    // Open up the executable file directly
    CATStreamFile targetFile;
    if (CATFAILED(result = targetFile.Open(execFile,CATStream::READ_ONLY)))
    {
        return result;
    }


    
    CATUInt8                dosHeader[0x40];
    IMAGE_NT_HEADERS	    peHeader;	
    CATUInt32				peHeaderOffset  = 0;	
    CATUInt32				amountRead      = 0;	
    void*                   startLoc        = 0;
    CATUInt8*               storedBytes     = 0;
    SIZE_T                  storedLength    = 0;
    
    // Load in the DOS header for the exe
    amountRead = 0x40;
    result = targetFile.Read(dosHeader,amountRead);
    
    if ((result == CAT_STAT_FILE_AT_EOF) || (CATFAILED(result)))
    {
        targetFile.Close();
        return CATRESULT(CAT_ERR_READING_TARGET_EXEC);
    }

    if (dosHeader[0] == 'M' && dosHeader[1] == 'Z')
    {
        // Okay, it's at least a valid exe of some sort, we think...
        // So now get offset of PE header...
        peHeaderOffset = *((long *)(&dosHeader[0x3c]));
        
        if (peHeaderOffset == 0)
        {
            // But it isn't a PE :(
            targetFile.Close();			
            return CAT_ERR_EXE_NOT_PE_FORMAT;
        }

        // Go to PE header offset
        CATFileOffset peOffset;
        peOffset.dOffset.highOffset = 0;
        peOffset.dOffset.lowOffset = peHeaderOffset;
        if (CATFAILED(result = targetFile.SeekAbsolute(peOffset.qOffset)))
        {
            targetFile.Close();
            return result;
        }
        
        // Read in COFF and Optional headers
        amountRead = sizeof(IMAGE_NT_HEADERS);
        if (CATFAILED(result = targetFile.Read(&peHeader,amountRead)) || 
            (amountRead != sizeof(IMAGE_NT_HEADERS)) )
        {
            targetFile.Close();			
            return CATRESULT(CAT_ERR_READING_TARGET_EXEC);
        }

        targetFile.Close();

        // Calculate starting point of execution
        startLoc = (void *)(UINT_PTR)
                   (peHeader.OptionalHeader.AddressOfEntryPoint +
                    peHeader.OptionalHeader.ImageBase);
    }
    else
    {
        targetFile.Close();        
        return CAT_ERR_EXE_NOT_PE_FORMAT;
    }

    // Create the process in a suspended state
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startupInfo;
    
    memset(&procInfo,0,sizeof(PROCESS_INFORMATION));
    memset(&startupInfo,0,sizeof(STARTUPINFO));
    
    startupInfo.cb = sizeof(STARTUPINFO);
        
    if (!CreateProcess(execFile,
                      (LPWSTR)commandLine,
                      0,
                      0,
                      0,
                      CREATE_SUSPENDED,
                      0,
                      targDir,
                      &startupInfo,
                      &procInfo))
    {
        return CAT_ERR_PROCESS_CREATE;
    }

    if ((procInfo.hThread  == INVALID_HANDLE_VALUE) || 
        (procInfo.hProcess == INVALID_HANDLE_VALUE))
    {
        return CAT_ERR_PROCESS_CREATE;
    }
        
    // Our patch is our codesize plus the length of the DLL filename.
    CATUInt32 patchSize = (CATUInt32)(sizeof(kAsmPatch) + 
                          (wcslen(dllFile)*2) + 2);

    // Make sure we have access to read/write to the startup location. 
    DWORD oldProtect = 0;
    VirtualProtectEx(   procInfo.hProcess,
                        startLoc,
                        patchSize,
                        PAGE_EXECUTE_READWRITE,
                        &oldProtect);
    
    // Read in bytes from the startup code
    storedBytes = new CATUInt8[patchSize];
    
    SIZE_T memReadBytes = 0;
    ReadProcessMemory(  procInfo.hProcess,
                        startLoc,
                        storedBytes,
                        patchSize,
                        &memReadBytes);

    storedLength = memReadBytes;
    
    // Start setting up the code we're patching the target with
    CATUInt8* writeBytes = new CATUInt8[patchSize];
    
    // Copy in the static code bytes of the patch
    memcpy(writeBytes,kAsmPatch,sizeof(kAsmPatch));

    // Copy in the dll name we've allocated space for
    wcscpy( (CATWChar*)(writeBytes + sizeof(kAsmPatch)),dllFile);
    
    // Store location of marker.dll string
    *((CATUInt32*)&writeBytes[kDLLNameOffAddr]) = 
        (CATUInt32)(UINT_PTR)(((CATUInt8*)startLoc) + kDLLNameOffset);

    // Store unpatch name address
    *((CATUInt32*)&writeBytes[kUnpatchFuncOffset]) = 
        (CATUInt32)(UINT_PTR)(((CATUInt8*)startLoc) + kUnpatchFuncNameAddress);

    // Address for LoadLibraryW. kernel32.dll addresses should be fine across 
    // process boundaries.
    //
    // NOTE: If you're running boundschecker, and debugging this code, 
    //       you're hosed.
    //
    // Boundschecker hooks the kernel32.dll module in the debugged process, but 
    // doesn't pick it up in the loaded one, so the address will be garbage when 
    // passed into the target executable. So much for boundschecking
    // this app...
    HMODULE kernelHandle = GetModuleHandle(L"kernel32.dll");
    FARPROC loadLibFunc	 = GetProcAddress(kernelHandle,"LoadLibraryW");	
    FARPROC getProcFunc  = GetProcAddress(kernelHandle,"GetProcAddress");
    
    // Store the address of the address of load library
    // we're doing a call far dword ptr [address].
    *((CATUInt32*)&writeBytes[kLoadLibraryOffset]) = 
        (CATUInt32)(UINT_PTR)(((CATUInt8*)startLoc) + kLoadLibraryAddress);
    
    // Same for get proc address
    *((CATUInt32*)&writeBytes[kGetProcAddressOffset]) = 
        (CATUInt32)(UINT_PTR)(((CATUInt8*)startLoc) + kGetProcAddress);

    // Store the address of the load library address
    *((CATUInt32*)&writeBytes[kLoadLibraryAddress]) = 
        (CATUInt32)(UINT_PTR)loadLibFunc;

    // Store the address of the getproc address
    *((CATUInt32*)&writeBytes[kGetProcAddress]) =
        (CATUInt32)(UINT_PTR)getProcFunc;

    // Store *our* processes ID and a pointer in our memory space to
    // the original bytes. This allows the DLL's UnpatchProcess() to 
    // simply read the bytes from our process directly over our
    // patch to restore the executable.
    *((CATUInt32*)&writeBytes[kHostProcOffset])    = (CATUInt32)_getpid();
    *((CATUInt32*)&writeBytes[kHostBufferOffset])  = (CATUInt32)(UINT_PTR)storedBytes;
    *((CATUInt32*)&writeBytes[kPatchSizeOffset])   = (CATUInt32)patchSize;
    *((CATUInt32*)&writeBytes[kStartLocOffset])    = (CATUInt32)(UINT_PTR)startLoc;
    *((CATUInt32*)&writeBytes[kPassDataOffset])    = (CATUInt32)(UINT_PTR)passData;
    *((CATUInt32*)&writeBytes[kPassDataLenOffset]) = passDataLen;
    // Also save start loc in the initial push at the beginning, so we'll have
    // our address on the stack when we restore it.
    *((CATUInt32*)&writeBytes[kStartLocReturn])  = (CATUInt32)(UINT_PTR)startLoc;

    // Write the patch to target process
    memReadBytes = 0;
    WriteProcessMemory(procInfo.hProcess,
                       startLoc,
                       writeBytes,
                       patchSize,
                       &memReadBytes);
      
    // Clean up write bytes
    delete [] writeBytes;
    

    // Create an event that the DLL can ping once it's restored the process
    CATWChar eventName[64];
    wsprintf(eventName,L"UnpatchProcess_%d",_getpid());
    HANDLE unpatchEvent = ::CreateEvent(0,TRUE,FALSE,eventName);

    // Start the patched thread
    ResumeThread(procInfo.hThread);
    
    // Wait for the unpatch function to ping event, signaling
    // that it has completed restoring the executable and no 
    // longer needs the stored bytes.
    if (WAIT_TIMEOUT == WaitForSingleObject(unpatchEvent,60000))
    {
        delete [] storedBytes;
        CloseHandle(unpatchEvent);
        return CAT_ERR_UNPATCH_TIMEOUT;
    }

    // Restore memory protections to original 
    VirtualProtectEx(procInfo.hProcess,
                     startLoc,
                     patchSize,
                     oldProtect,
                     &oldProtect);
   
    // Got event - program successfully restored, DLL injected.
    delete [] storedBytes;
    CloseHandle(procInfo.hThread);
    CloseHandle(procInfo.hProcess);
    CloseHandle(unpatchEvent);    

    return CAT_SUCCESS;
}

#endif // CAT_CONFIG_WIN32
