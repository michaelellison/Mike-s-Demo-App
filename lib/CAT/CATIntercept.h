/// \file  CATIntercept.h
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

#ifndef _CATIntercept_H_
#define _CATIntercept_H_

#include "CATInternal.h"

#ifdef CAT_CONFIG_WIN32
#include <psapi.h>

class CATIntercept;


#pragma pack(push)
#pragma pack(1)
/// The CATHOOK structure contains all the information about a hook, plus directly executable code
/// to bootstrap our hook functions and help return to the original function from them.
struct CATHOOK
{
    CATUInt8    PrePushHook;        ///< First, we push the CATHOOK address onto the stack for the
    CATUInt32   PrePushHookAddress; ///< receiving hook function.
    
    CATUInt32   StackSwap1[2];      ///< Then, we swap the caller's return address with the CATHOOK
    CATUInt16   StackSwap2;         ///< address, so the latter may be read as a parameter.

    CATUInt8    HookJmp;            ///< Finally, jump to the user-defined hook function
    CATUInt32   HookJmpLoc;    
    
    CATUInt8    OrgInst[32];        ///< These are the original instructions that were at the location
                                    ///< we patched with a jump to us. All additional space is filled
                                    ///< with NOPs.  We'll execute this directly.

    CATUInt8    PostPatchJump;      ///< This jump jumps back into the original target right *after*
    CATUInt32   PostPatchJumpLoc;   ///< the jump patch we applied.

    void*       Target;             ///< Start address of target function
    void*       HookFunc;           ///< Start address of hook function
    CATUInt32   OrgInstLen;         ///< Number of bytes grabbed into OrgInst
    void*       UserParam;          ///< User-specified context 
    CATIntercept* InterceptObj;     ///< Parent CATIntercept object.
};
#pragma pack(pop)

/// Request the compiler to NOT introduce prologue/epilogue code into our hook functions
#define CATHOOKFUNC         _declspec(naked) 

/// Our hook function prologue takes the *original* number of parameters for the function
/// being hooked.  It stores all the registers and sets up our stack frame and return values.
/// We then repush relevant parameters onto the stack so that our C/C++ can in hook functions
/// can be pretty.  Also note that it has a trailing '{' that must be closed by a
/// CATHOOK_EPILOGUE macro.
///
#define CATHOOK_PROLOGUE(numParams)     \
    __asm {push    ebp}                 \
    __asm {mov     ebp,esp}             \
    __asm {push    eax}                 \
    __asm {pushfd}                      \
    __asm {pushad}                      \
    __asm {sub     esp,__LOCAL_SIZE}    \
    __asm {mov     ecx,numParams+1 }    \
    __asm {mov     ebx,(numParams+1)*4} \
    __asm {repush_regs:}                \
    __asm {push    [ebp+ebx]}           \
    __asm {sub     ebx,4}               \
    __asm {loop    repush_regs}{

//    __asm {sub     esp,8}               \
//    __asm {fstp    qword ptr [esp]}     \

/// The epilogue restore all the registers saved by CATHOOK_PROLOGUE and sets the stack
/// up to return properly to the original caller.  Note that numParams refers to the number
/// of parameters in the *original* function, not the hook function.
#define CATHOOK_EPILOGUE_RAW(numParams)    \
    }                                      \
    __asm {add     esp,(numParams+1)*4}    \
    __asm {add     esp,__LOCAL_SIZE}       \
    __asm {popad}                          \
    __asm {popfd}                          \
    __asm {pop     eax}                    \
    __asm {mov     esp,ebp}                \
    __asm {pop     ebp}                    \
    __asm {xchg    eax,[esp+4]}            \
    __asm {pop     eax}                    \
    __asm {xchg    eax,[esp]}              

//    __asm {fld     qword ptr [esp]}        \
//    __asm {add     esp,8}                  \

#define CATHOOK_EPILOGUE_WINAPI(numParams) \
    CATHOOK_EPILOGUE_RAW(numParams)        \
    __asm {ret     numParams*4}

/// This is the CDECL version - only use with CDECL functions!
#define CATHOOK_EPILOGUE_CDECL(numParams)  \
    CATHOOK_EPILOGUE_RAW(numParams)        \
    __asm {ret     0}

/// CATHOOK_CALLORIGINAL() calls the original function from within a hook function.  The return
/// value for EAX is automatically updated, but may be overridden using CATHOOK_SETRETURN.
/// This may only be used between CATHOOK_PROLOGUE and CATHOOK_EPILOGUE.
#define CATHOOK_CALLORIGINAL_WINAPI(hookInst,numParams) \
    void* func =  &hookInst->OrgInst;                   \
    __asm {mov    ecx,numParams}                        \
    __asm {or     ecx,ecx}                              \
    __asm {jz     skip_push}                            \
    __asm {mov    ebx,(numParams+2)*4}                  \
    __asm {repush_host:}                                \
    __asm {push   [ebp+ebx]}                            \
    __asm {sub    ebx,4}                                \
    __asm {loop   repush_host}                          \
    __asm {skip_push:}                                  \
    __asm {call   func}                                 \
    __asm {mov    [ebp-4],eax}

/// CDECL version of call original - removes params off stack after call
#define CATHOOK_CALLORIGINAL_CDECL(hookInst,numParams) \
    CATHOOK_CALLORIGINAL_WINAPI(hookInst,numParams)    \
    __asm {add    esp,(numParams)*4}                   

/// Sets the return value from a hook function.
#define CATHOOK_SETRETURN(retVal) \
    __asm {mov    eax,retVal}     \
    __asm {mov    [ebp-4],eax}   

/// Entry for tables used by InterceptCOMObject(). Specifies the virtual table index of the
/// target function and the target hook function to receive the call.
///
/// The last entry should have a value of -1 for its VTableIndex to end the table.
struct CATINTERCEPT_COM_TABLE_ENTRY
{
    CATUInt32   VTableIndex;
    void*       HookFunction;
    CATUInt32   StubLength;
};

/// Entry for tables used by InterceptDLL().  Specifies the exported function name of 
/// the target function and the target hook to receive the call.
///
/// The last entry should have a NULL FunctionName entry to specify the end of the table.
struct CATINTERCEPT_DLL_TABLE_ENTRY
{
    const CATChar*  FunctionName;
    void*           HookFunction;
    CATUInt32       StubLength;
};

/// \class CATIntercept
/// \brief Function interception class for Win32.
/// \ingroup CAT
/// 
/// CATIntercept provides a way to directly hook functions within the current process.
/// To use it, first create a hook function for the function you wish to hook.
/// It needs to be akin to the following:
///
/// \code
/// CATHOOKFUNC int HookedFunc(CATHOOK* hookInfo, PARAMTYPE param_1, ..., PARAMTYPE param_n)
/// {
///     CATHOOK_PROLOGUE(numParams);
///
///     /* Your code here to execute prior to original function */
///
///     CATHOOK_CALLORIGINAL(hookInfo,numParams);
///
///     /* Your code here for post-execution */
///
///     /* Set the return value if desired... */
///     CATHOOK_SETRETURN(returnValue);
///
///     CATHOOK_EPILOGUE(numParams);
/// }
/// \endcode
///
///
/// Once you've created the hook function for each function you wish to intercept, you may instantiate a 
/// CATIntercept object and call Intercept() for each function.
///
/// CATIntercept works by overwriting the target function's first 5 with a jump directly into the
/// returned CATHOOK structure.  The CATHOOK structure then sets up the registers for the hook function and
/// passes control to it.  To call the original function within the hook function, CATHOOK_CALLORIGINAL
/// executes the original bytes from the start of the target function, then jumps to just *after* the jump
/// it patched the function with.
///
/// While this allows us to be reentrant on hooked functions and not have to beat up import tables, it does 
/// present a problem - the code at the start of the target function may not be exactly 5 bytes in length
/// for a proper decode.
///
/// At the moment, you'll need to debug into the target function and figure out how many bytes to save, then
/// pass that as the stubLength to Intercept().  Eventually, this is screaming to have a basic disassembler
/// written for it to determine the proper number of bytes automatically.  Note that we do currently follow
/// 0xe9 jumps, so it will find the actual function within the jump table (or other similar hooks!)
///
/// \todo 
/// Add disassembler component to determine number of bytes to use for stubLength automatically.
///
/// \note
/// The hook functions should be find for multiple threads (at least, the skeleton provided should be),
/// but currently the CATIntercept object is not.  Instantiate a new one for use on each thread or serialize
/// calls to Intercept/Restore.  Also note that intercepting/restoring a function while it is being called
/// will probably crash as well.
///
class CATIntercept
{
    public:
        CATIntercept();
        virtual ~CATIntercept();    

        static void* GetFunctionFromVTable(void* objectPtr, CATUInt32 vtableIndex);
        /// Intercept the targetFunc so that hookFunc gets called instead.  See the docs above
        /// for the CATIntercept class and the CATHOOK_PROLOGUE, CATHOOK_CALLORIGINAL, and 
        /// CATHOOK_EPILOGUE macros for additional information.
        ///
        /// \param  targetFunc  Ptr to the function to hook.
        /// \param  hookFunc    Ptr to hook function to receive calls
        /// \param  stubLength  Number of bytes (minimum of 5) to save from the target function
        ///                     to be executed within our hook prior to calling the target. The
        ///                     length MUST specify full instructions, or it will crash.  Relative
        ///                     addressing functions will also cause crashes.
        /// \param  newHook     On success, this is set to point to the CATHOOK struct created for
        ///                     hook.
        /// \param  userParam   Optional context that will be added to CATHOOK structure.
        /// \return CATResult   CAT_SUCCESS on success.
        CATResult Intercept ( void*         targetFunc, 
                              void*         hookFunc, 
                              CATUInt32     stubLength, 
                              CATHOOK*&     newHook,
                              void*         userParam = 0);

        /// Restores the target function and removes the hook.
        ///
        /// \param  hookInfo    The hook to remove. Set ot 0 on successful return.
        /// \return CATResult   CAT_SUCCESS on success.
        CATResult Restore   ( CATHOOK*& hookInfo);

        void      RestoreAll();

        /// Save the interception data for a COM intercept table to the registry,
        /// so we don't have to do all the nasty COM creation later.
        CATResult SaveInterceptData (   const CATWChar*               objectName,
                                        void*                         comObject,
                                        CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                        void*                         userParam);
        /// Load interception data from the registry if it's available
        CATResult LoadAndHook       (   const CATWChar*               objectName,
                                        CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                        void*                         userParam);

        /// Hooks all the functions in a COM interface that are specified in a table.
        CATResult InterceptCOMObject(   void*                         comObject,
                                        CATINTERCEPT_COM_TABLE_ENTRY* interceptTable,
                                        void*                         userParam);
        
        CATResult InterceptDLL      (   HMODULE                       module,
                                        CATINTERCEPT_DLL_TABLE_ENTRY* interceptTable,
                                        void*                         userParam);
                                     
    protected:
        std::vector<CATHOOK*> fHooks;
};

#endif // CAT_CONFIG_WIN32
#endif // _CATIntercept_H_
