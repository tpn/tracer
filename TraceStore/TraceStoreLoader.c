/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreLoader.c

Abstract:

    This module implements functionality related to capturing library loading
    information whilst a process is being traced.  This is done by hooking into
    loader's DLL notification callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
LoaderStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the Loader trace store.  It calls the normal trace store bind complete
    routine, then initializes loader tracing via Rtl.  This acquires the loader
    lock, walks the current list of loaded modules and invokes our callback,
    registers our callback and the subsequent DLL load/unload notification
    routine, releases the lock, and then returns.

Arguments:

    TraceContext - Supplies a pointer to the TRACE_CONTEXT structure to which
        the trace store was bound.

    TraceStore - Supplies a pointer to the bound TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to the first TRACE_STORE_MEMORY_MAP
        used by the trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PRTL Rtl;
    PVOID Cookie;
    DLL_NOTIFICATION_FLAGS NotificationFlags = { 0 };

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    //
    // Initialize loader notifications.
    //

    Rtl = TraceContext->Rtl;

    Success = Rtl->RegisterDllNotification(Rtl,
                                           TraceStoreDllNotificationCallback,
                                           &NotificationFlags,
                                           TraceContext,
                                           &Cookie);

    return Success;
}

_Use_decl_annotations_
VOID
CALLBACK
TraceStoreDllNotificationCallback(
    DLL_NOTIFICATION_REASON Reason,
    PDLL_NOTIFICATION_DATA Data,
    PVOID Context
    )
/*++

Routine Description:

    This routine is the callback function invoked automatically by the loader
    when a DLL has been loaded or unloaded.

Arguments:

    Reason - Supplies the reason for the callback (DLL load or DLL unload).

    Data - Supplies a pointer to notification data for this loader operation.

    Context - Supplies a pointer to the TRACE_CONTEXT structure that registered
        the loader DLL notifications.

Return Value:

    None.

--*/
{
    PRTL Rtl;
    PTRACE_CONTEXT TraceContext;
    PLDR_DATA_TABLE_ENTRY Module;
    PLDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    PCLDR_DLL_NOTIFICATION_DATA NotifData;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return;
    }

    //
    // Load aliases.
    //

    TraceContext = (PTRACE_CONTEXT)Context;
    Rtl = TraceContext->Rtl;

    Module = Data->LoaderDataTableEntry;
    NotifData = Data->NotificationData;
    Loaded = (PLDR_DLL_LOADED_NOTIFICATION_DATA)&NotifData->Loaded;

    if (Reason.LoadedInitial) {
        OutputDebugStringA("Loaded Initial\n");
    } else if (Reason.Loaded) {
        OutputDebugStringA("Loaded.\n");
    } else {
        OutputDebugStringA("Unloaded.\n");
    }

    if (Module) {
        OutputDebugStringA("Module present.  Path:\n");
        OutputDebugStringW(Module->FullDllName.Buffer);
        OutputDebugStringA("\n");
    }

    if (Loaded) {
        OutputDebugStringA("Notif data present.  Path:\n");
        OutputDebugStringW(Loaded->FullDllName->Buffer);
        OutputDebugStringA("\n");
    }

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
