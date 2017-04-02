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

//
// An RTL_INJECTION_CONTEXT is created for each RTL_INJECTION_PACKET requested
// by a caller.  This structure encapsulates internal state needed in order to
// complete the injection request.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_CONTEXT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    RTL_INJECTION_CONTEXT_FLAGS Flags;

    //
    // Function pointers required by the initial injection callback.
    //

    PLOAD_LIBRARY_W LoadLibraryW;
    PGET_PROC_ADDRESS GetProcAddress;
    POPEN_EVENT_A OpenEventA;
    PSET_EVENT SetEvent;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
    PWAIT_FOR_SINGLE_OBJECT WaitForSingleObject;
    PCLOSE_HANDLE CloseHandle;

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
(RTLP_TEST_INJECTION_COMPLETE_CALLBACK)(
    _In_ PRTL Rtl,
    _In_ PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteError,
    _Out_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTLP_TEST_INJECTION_COMPLETE_CALLBACK
      *PRTLP_TEST_INJECTION_COMPLETE_CALLBACK;


#pragma component(browser, off)
RTLP_TEST_INJECTION_COMPLETE_CALLBACK RtlpTestInjectionCompleteCallback;
#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
