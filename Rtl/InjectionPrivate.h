/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionPrivate.h

Abstract:

    This is the private header file for the injection functionality of the Rtl
    component.  It defines data structures, function typedefs and function
    declarations for all major (i.e. not local to the module) functions
    available for use by individual modules within this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

typedef enum _RTL_INJECTION_REMOTE_THREAD_EXIT_CODE {

    InjectedRemoteThreadNoError = 0,

    //
    // Start the remaining error codes off at an arbitrarily high number, such
    // that it's easy to distinguish between an error code and the size of the
    // function if a context protocol callback is being dispatched.
    //

    InjectedRemoteThreadNullContext = 1 << 20,
    InjectedRemoteThreadLoadLibraryRtlFailed,
    InjectedRemoteThreadResolveInitializeRtlFailed,
    InjectedRemoteThreadInitializeRtlFailed,

} RTL_INJECTION_REMOTE_THREAD_EXIT_CODE;


typedef
VOID
(CALLBACK RTL_INJECTION_CALLBACK)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTL_INJECTION_CALLBACK *PRTL_INJECTION_CALLBACK;

typedef union _RTL_INJECTION_CONTEXT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_CONTEXT_FLAGS;
typedef RTL_INJECTION_CONTEXT_FLAGS *PRTL_INJECTION_CONTEXT_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_CONTEXT_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
BOOL
(CALLBACK RTL_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK)(
    _In_ struct _RTL_INJECTION_CONTEXT const *Context,
    _Outptr_result_maybenull_ PVOID Token
    );
typedef RTL_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK
      *PRTL_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK;

//
// An RTL_INJECTION_CONTEXT is created for each RTL_INJECTION_PACKET requested
// by a caller.  This structure encapsulates internal state needed in order to
// complete the injection request.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_CONTEXT {

    //
    // As with RTL_INJECTION_PACKET, the first member of this structure will
    // always be the function pointer that a remote thread is required to call
    // before executing any other code.
    //

    PRTL_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK
        IsInjectionContextProtocolCallback;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_CONTEXT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    RTL_INJECTION_CONTEXT_FLAGS Flags;

    //
    // Fully-qualified path of the Rtl.dll module to load.
    //

    UNICODE_STRING RtlDllPath;

    //
    // A handle to the Rtl.dll module.  Only valid in the target process.
    //

    HMODULE RtlModuleHandle;

    //
    // An instance of the RTL structure initialized in the target process.
    //

    PRTL Rtl;

    //
    // The name of the event that will be signalled by the injection callback
    // prior to waiting on the wait event name.
    //

    STRING SignalEventName;

    //
    // The name of the event that will be waited on by the initial injection
    // callback prior to dispatching the caller's injection complete routine.
    //

    STRING WaitEventName;

    //
    // Handles to the events.
    //

    HANDLE SignalEventHandle;
    HANDLE WaitEventHandle;

    //
    // The original pointer value provided by the caller as the Code argument,
    // or the initial result of GetProcAddress().
    //

    union {
        PBYTE OriginalCode;
        PRTL_INJECTION_COMPLETE_CALLBACK OriginalCallback;
    };

    //
    // The result of SkipJumps(OriginalCode).  May be identical to OriginalCode
    // above, may not be.
    //

    union {
        PBYTE Code;
        PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    };

    //
    // The start address of our initial remote thread thunk, allocated in the
    // target process's memory space.
    //

    LPTHREAD_START_ROUTINE ThreadStartRoutine;

    //
    // Approximate size of our thread start routine.
    //

    LONG SizeOfThreadStartRoutineInBytes;

    //
    // The ID of the remote thread we created to handle the initial injection.
    //

    ULONG RemoteThreadId;

    //
    // An approximate size of the callback function code.
    //

    LONG SizeOfCallbackCodeInBytes;

    //
    // Pad out to 8 bytes.
    //

    ULONG Reserved;

    //
    // Total size of the data bytes that were allocated in the remote process.
    //

    LONG_INTEGER TotalDataAllocSize;

    //
    // Total size of the code bytes that were allocated in the remote process.
    //

    LONG_INTEGER TotalCodeAllocSize;

    //
    // Base address of the data allocation, the size of which is indicated by
    // the TotalDataAllocSize struct member above.
    //

    LPVOID BaseDataAddress;

    //
    // Base address of the code allocation, the size of which is indicated by
    // the TotalCodeAllocSize struct member above.
    //

    LPVOID BaseCodeAddress;

    //
    // Function pointers required by the initial injection callback.
    //

    PSET_EVENT SetEvent;
    POPEN_EVENT_A OpenEventA;
    PCLOSE_HANDLE CloseHandle;
    PGET_PROC_ADDRESS GetProcAddress;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
    PWAIT_FOR_SINGLE_OBJECT WaitForSingleObject;


    //
    // The injection packet.
    //

    RTL_INJECTION_PACKET Packet;

} RTL_INJECTION_CONTEXT;
typedef RTL_INJECTION_CONTEXT *PRTL_INJECTION_CONTEXT;
typedef const RTL_INJECTION_CONTEXT *PCRTL_INJECTION_CONTEXT;


typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTLP_VERIFY_INJECTION_CALLBACK)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_VERIFY_INJECTION_CALLBACK *PRTLP_VERIFY_INJECTION_CALLBACK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTLP_INJECTION_BOOTSTRAP)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_INJECTION_BOOTSTRAP *PRTLP_INJECTION_BOOTSTRAP;

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_SHARED {
    INIT_ONCE InitOnce;
    SRWLOCK Lock;
    ALLOCATOR Allocator;
    //LIST_HEAD ListHead;
} RTL_INJECTION_SHARED;
typedef RTL_INJECTION_SHARED *PRTL_INJECTION_SHARED;


#pragma component(browser, off)
RTLP_VERIFY_INJECTION_CALLBACK RtlpVerifyInjectionCallback;
RTLP_INJECTION_BOOTSTRAP RtlpInjectionBootstrap;
RTL_INJECTION_COMPLETE_CALLBACK RtlpPreInjectionCallbackImpl;
RTL_INJECTION_COMPLETE_CALLBACK RtlpInjectionCompleteCallbackImpl;
#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
