/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionRemoteThreadEntryThunk.c

Abstract:

    This module implements the thread entry routine that is used as the start
    address of injected threads.  This routine is copied directly into the
    target process's memory space, and thus, care must be taken regarding the
    types of instructions that can be used.

    Keeping the routine in a separate module increases flexibility with regards
    to potentially specifying different compiler (optimization) flags for this
    module only.

--*/

#include "stdafx.h"

#pragma optimize("", off)

_Use_decl_annotations_
ULONGLONG
RtlpInjectionRemoteThreadEntryThunk(
    PRTL_INJECTION_CONTEXT Context
    )
/*++

Routine Description:

    This is the main thread entry point for a remote thread created as part of
    injection.  It is responsible for calling the initial injection context
    protocol callback function, then loading the Rtl module and initializing an
    RTL structure (allocated on our stack).

    We then call Context->InjectedThreadRemoteEntry(Context) to enter the main
    work loop of the remote thread.

    This method is somewhat unique in that it is copied into the remote address
    space of a different process; this is why we use function pointers stored in
    the Context structure for GetProcAddress() and LoadLibraryExW().

Arguments:

    Context - Supplies a pointer to the RTL_INJECTION_CONTEXT structure that is
        being used for this injection.

Return Value:

    A LONG integer, the value of which will depend on the context.

--*/
{
    BOOL Success;
    PRTL Rtl;
    RTL RtlLocal;
    ULONG SizeOfRtl;
    ULONG Token;
    ULONGLONG Result;
    HMODULE Module;
    PINITIALIZE_RTL InitRtl;
    PGET_PROC_ADDRESS GetProcAddress;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY InjectionThreadEntry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return InjectedRemoteThreadNullContext;
    }

    //
    // Adhere to the context callback protocol.
    //

    if (Context->IsInjectionContextProtocolCallback(Context, &Token)) {
        return Token;
    }

    //
    // Initialize local function pointers.
    //

    GetProcAddress = Context->GetProcAddress;
    LoadLibraryExW = Context->LoadLibraryExW;

    //
    // Load the Rtl module.
    //

    Module = LoadLibraryExW(Context->RtlDllPath.Buffer, NULL, 0);

    if (!Module || Module == INVALID_HANDLE_VALUE) {
        return InjectedRemoteThreadLoadLibraryRtlFailed;
    }

    //
    // Get the address of the InitializeRtl() function.
    //

    InitRtl = (PINITIALIZE_RTL)GetProcAddress(Module, "InitializeRtl");
    if (!InitRtl) {
        return InjectedRemoteThreadResolveInitializeRtlFailed;
    }

    //
    // Initialize our local, stack-allocated RTL structure.
    //

    Rtl = &RtlLocal;
    SizeOfRtl = sizeof(*Rtl);

    if (!InitRtl(Rtl, &SizeOfRtl)) {
        return InjectedRemoteThreadInitializeRtlFailed;
    }

    //
    // Store a pointer to RTL in the context.
    //

    Context->Rtl = Rtl;

    //
    // Defer to our main dispatch routine.
    //

    InjectionThreadEntry = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY)(
        Rtl->InjectionRemoteThreadEntry
    );

    Success = InjectionThreadEntry(Context, &Result);

    if (!Success) {

        //
        // We can't do anything here.
        //

        NOTHING;
    }

    return Result;
}
#pragma optimize("", on)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
