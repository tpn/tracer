/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main module for the TracerDebugEngineExe component.

    It implements a Main() function and provides a mainCRTStartup() entry point
    which calls Main().

--*/

#include "stdafx.h"

ULONG
Main(VOID)
{
    BOOL Success;
    PRTL Rtl;
    ULONG ExitCode;
    HRESULT Result;
    ALLOCATOR Allocator;
    ALLOCATOR StringTableAllocator;
    ALLOCATOR StringArrayAllocator;

    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PUNICODE_STRING DebugEngineDllPath;
    PUNICODE_STRING TracingDllPath = NULL;
    PDEBUG_ENGINE_SESSION Session;
    PDESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags = { 0 };

    //
    // Initialize the default heap allocator.  This is a thin wrapper around
    // the generic Win32 Heap functions.
    //

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        goto Error;
    }

    //
    // Zero the allocators for the StringTable component.
    //

    SecureZeroMemory(&StringArrayAllocator, sizeof(StringArrayAllocator));
    SecureZeroMemory(&StringTableAllocator, sizeof(StringTableAllocator));

    //
    // Initialize TracerConfig and Rtl.
    //

    RegistryPath = (PUNICODE_STRING)&TracerRegistryPath;

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            RegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    //
    // Extract the target process ID and thread ID from the command line.
    //

    InitFlags.InitializeFromCommandLine = TRUE;
    DebugEngineDllPath = &TracerConfig->Paths.DebugEngineDllPath;
    Success = LoadAndInitializeDebugEngineSession(
        DebugEngineDllPath,
        Rtl,
        &Allocator,
        InitFlags,
        TracerConfig,
        &TracerConfig->Paths.StringTableDllPath,
        &StringArrayAllocator,
        &StringTableAllocator,
        &Session,
        &DestroyDebugEngineSession
    );

    if (!Success) {
        OutputDebugStringA("LoadAndInitializeDebugEngineSession() failed.\n");
        goto Error;
    };

    //
    // Ensure we've got the name of the first program.
    //

    while (!Session->InitialModuleNameW.Length) {

        Result = Session->WaitForEvent(Session, 0, 100);

        if (Result != S_OK && Result != S_FALSE) {
            goto Error;
        }
    }

    /*
    Success = TracerConfig->GetTracingDllForModuleName(
        Rtl,
        TracerConfig,
        Allocator,
        Session->InitialModuleNameW,
        &TracingDllPath
    );

    if (!Success) {
        goto Error;
    }

    if (!TracingDllPath) {

        //
        // Detach from process?  Go into dormant state where we listen to calls
        // to LoadModule?
        //

        __debugbreak();

        goto Error;
    }
    */

    //
    // Once we've determined that we want to start tracing this process, we
    // need to inject the tracing DLL.  The current plan for doing this is as
    // follows:
    //
    //  1.  Getting the module address of kernel32!LoadLibraryW in the target
    //      process.
    //
    //  2.  VirtualAllocEx()'ing a chunk of memory in the target process.
    //
    //  3.  Coming up with a stub function that has a signature along the lines
    //      of:
    //          BOOL
    //          LoadLibraryStub(
    //              _In_ PLOAD_LIBRARY LoadLibraryFunc,
    //              _In_ PWSTR Path
    //              )
    //
    //      ....and implementing it as a LEAF_ENTRY (that has no RIP-relative
    //      code) in assembly.
    //
    //  4.  WriteProcessMemory()'ing the contents of the stub function into
    //      the target process, making it executable, then creating a remote
    //      thread with the start address of that function.
    //
    //  5.  Registering a debug engine CreateThread event callback that gets
    //      invoked when the remote thread is detected; this callback will use
    //      the IDebugRegisters interface to set the RCX and RDX registers to
    //      the two values we want, and then continue execution.
    //
    //  6.  Waiting for the subsequent LoadModule() callback via the debug
    //      engine for the previous step.
    //
    //  7.  Dispatching some form of something that results in a remote thread
    //      in the target process calling the newly-loaded DLL's Main() entry
    //      point (e.g. Main()).  Ideas:
    //
    //       1. Convert the LoadLibraryStub() into something that waits on an
    //          event after the library and then continues execution...
    //
    //       2. Extend the API such that it's something like:
    //
    //          BOOL
    //          LoadLibraryAndWaitForMain(
    //              _In_ PLOAD_LIBRARY LoadLibraryFunc,
    //              _In_ PGET_PROC_ADDRESS GetProcAddressFunc,
    //              _In_ POPEN_EVENT OpenEventFunc,
    //              _In_ PWAIT_FOR_SINGLE_OBJECT WaitForSingleObjectFunc,
    //              _In_ PWSTR Path,
    //              _In_ PWSTR MainProcName,
    //              _In_ PWSTR EventName
    //              );
    //
    //          That would do the following:
    //
    //              Module = LoadLibraryFunc(Path);
    //              MainProc = GetProcAddressFunc(MainProcName);
    //              Event = OpenEventFunc(EventName);
    //
    //
    //              WaitResult = WaitForSingleObjectFunc(Event, INFINITE);
    //              if (WaitResult == WAIT_OBJECT_0) {
    //                  MainProc();
    //              }
    //
    //      The above approach would be easier if the parameters were extracted
    //      into a structure, e.g. RTL_THREAD_ENTRY_THUNK.  We'd then use
    //      VirtualAllocEx() + WriteProcessMemory() to create the necessary
    //      structure and then simply set RCX in the CreateThread() callback.
    //

    while (TRUE) {
        Result = Session->WaitForEvent(Session, 0, INFINITE);

        if (Result != S_OK && Result != S_FALSE) {
            goto Error;
        }
    }

    ExitCode = 0;
    goto End;

Error:
    ExitCode = 1;

End:
    return ExitCode;
}

VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(Main());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
