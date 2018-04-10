/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InitializeTracedPythonSessionFromPythonDllModule.c

Abstract:

    This module implements routines related to the initialization of a
    TRACED_PYTHON_SESSION structure.  Routines are provided for converting the
    current directory into a unicode string, changing into the PYTHONHOME
    directory, and initializing the TRACED_PYTHON_SESSION structure.

--*/

#include "stdafx.h"

#pragma intrinsic(wcslen)

_Use_decl_annotations_
BOOL
InitializeTracedPythonSessionFromPythonDllModule(
    PRTL Rtl,
    PTRACER_CONFIG TracerConfig,
    PALLOCATOR Allocator,
    HMODULE PythonDllModule,
    PPUNICODE_STRING TraceSessionDirectoryPointer,
    PPTRACED_PYTHON_SESSION SessionPointer
    )
/*++

Routine Description:

    This function initializes a TRACED_PYTHON_SESSION structure.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    Allocator - Optionally supplies a pointer to an alternate ALLOCATOR to use.
        If not present, TracerConfig->Allocator will be used.

    PythonDllModule - Supplies a handle for the target Python module for which
        the traced session is to be initialized.

    TraceSessionDirectoryPointer - Supplies a pointer to a variable that either
        provides the address to a UNICODE_STRING structure that represents the
        trace session directory to open in a read-only context, or, if the
        pointer is NULL, this will receive the address of the newly-created
        UNICODE_STRING structure that matches the trace session directory.

    SessionPointer - Supplies a pointer to a variable that will receive the
        address of the TRACED_PYTHON_SESSION structure allocated.  This pointer
        is immediately cleared (that is, '*SessionPointer = NULL;' is performed
        once SessionPointer is deemed non-NULL), and a value will only be set if
        initialization was successful.

Return Value:

    TRUE on Success, FALSE if an error occurred.  *SessionPointer will be
    updated with the value of the newly created TRACED_PYTHON_SESSION
    structure.

--*/
{
    BOOL Success;
    BOOL Compress;
    BOOL IsReadonly;
    BOOL LockMemoryEnabled;
    BOOL ManageVolumeEnabled;
    ULONG RequiredSize;
    ULONG BufferAllocationFlags;
    SIZE_T BufferSize;
    SIZE_T LargePageMinimum;
    HKEY Key;
    HANDLE LoadingCompleteEvent = NULL;
    PVOID Buffer;
    PPYTHON Python;
    PTRACER_PATHS Paths;
    PTRACED_PYTHON_SESSION Session;
    UNICODE_STRING CommandLine;
    PUNICODE_STRING TraceSessionDirectory;
    TRACE_FLAGS TraceFlags;
    TRACER_FLAGS TracerConfigFlags;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    PSET_ATEXITEX SetAtExitEx;
    PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(SessionPointer)) {

        return FALSE;

    } else {

        //
        // Clear the user's session pointer up-front.
        //

        *SessionPointer = NULL;
    }

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PythonDllModule)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {

        if (!TracerConfig->Allocator) {
            return FALSE;
        }

        Allocator = TracerConfig->Allocator;

    }

    //
    // If the caller provides a valid trace session directory, it implies we're
    // a readonly session.
    //

    if (!ARGUMENT_PRESENT(TraceSessionDirectoryPointer)) {
        return FALSE;
    }

    TraceSessionDirectory = *TraceSessionDirectoryPointer;

    if (!TraceSessionDirectory) {
        IsReadonly = FALSE;
    } else {
        IsReadonly = TRUE;
    }

    Session = (PTRACED_PYTHON_SESSION)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*Session)
        )
    );

    if (!Session) {
        return FALSE;
    }

    //
    // If we reach this point, our Session has been successfully created and
    // the allocation functions have been initialized.  From this point on,
    // any subsequent errors must `goto Error;` in order to ensure the rundown
    // logic executes.
    //

    //
    // Fill out initial fields.
    //

    Session->Size = sizeof(*Session);
    Session->Allocator = Allocator;
    Session->TracerConfig = TracerConfig;

    //
    // Load our modules.
    //

#define LOAD_DLL(Name) do {                                            \
    Session->Name##Module = LoadLibraryW(Paths->Name##DllPath.Buffer); \
    if (!Session->Name##Module) {                                      \
        OutputDebugStringA("Failed to load " #Name);                   \
        goto Error;                                                    \
    }                                                                  \
} while (0)

    Paths = &TracerConfig->Paths;

    LOAD_DLL(Python);
    LOAD_DLL(TraceStore);
    LOAD_DLL(StringTable);
    LOAD_DLL(PythonTracer);

    //
    // Resolve the functions.
    //

#define RESOLVE(Module, Type, Name) do {                                  \
    Session->Name = (Type)GetProcAddress(Session->Module, #Name);         \
    if (!Session->Name) {                                                 \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name "\n"); \
        goto Error;                                                       \
    }                                                                     \
} while (0)

    //
    // TraceStore
    //

    RESOLVE(TraceStoreModule, PINITIALIZE_TRACE_STORES, InitializeTraceStores);

    RESOLVE(TraceStoreModule,
            PINITIALIZE_TRACE_CONTEXT,
            InitializeTraceContext);

    RESOLVE(TraceStoreModule,
            PCLOSE_TRACE_CONTEXT,
            CloseTraceContext);

    RESOLVE(TraceStoreModule,
            PINITIALIZE_TRACE_SESSION,
            InitializeTraceSession);

    RESOLVE(TraceStoreModule,
            PCLOSE_TRACE_STORES,
            CloseTraceStores);

    RESOLVE(TraceStoreModule,
            PINITIALIZE_ALLOCATOR_FROM_TRACE_STORE,
            InitializeAllocatorFromTraceStore);

    //
    // Python
    //

    RESOLVE(PythonModule, PINITIALIZE_PYTHON, InitializePython);

    //
    // PythonTracer
    //

    RESOLVE(PythonTracerModule,
            PINITIALIZE_PYTHON_TRACE_CONTEXT,
            InitializePythonTraceContext);

    RESOLVE(PythonTracerModule,
            PCLOSE_PYTHON_TRACE_CONTEXT,
            ClosePythonTraceContext);

    RESOLVE(PythonTracerModule,
            PTRACE_STORE_FIELD_RELOCS,
            PythonTracerTraceStoreRelocations);

    //
    // StringTable
    //

    Success = LoadStringTableApi(Rtl,
                                 &Session->StringTableModule,
                                 NULL,
                                 sizeof(Session->StringTableApi),
                                 (PSTRING_TABLE_ANY_API)&Session->StringTableApi);

    if (!Success) {
        OutputDebugStringA("LoadStringTableModule() failed.\n");
        goto Error;
    }

    //
    // All of our modules modules use the same pattern for initialization
    // whereby the required structure size can be obtained by passing a NULL
    // for the destination pointer argument and PULONG as the size argument.
    // We leverage that here, using the local &RequiredSize variable and some
    // macro glue.
    //

#undef ALLOCATE
#define ALLOCATE(Target, Type)                                             \
    Session->Target = (Type)(                                              \
        Allocator->Calloc(                                                 \
            Allocator->Context,                                            \
            1,                                                             \
            RequiredSize                                                   \
        )                                                                  \
    );                                                                     \
    if (!Session->Target) {                                                \
        OutputDebugStringA("Allocation failed for " #Target " struct.\n"); \
        goto Error;                                                        \
    }

    Session->Rtl = Rtl;

    //
    // If any of our DLLs use structured exception handling (i.e. contain
    // code that uses __try/__except), they'll also export SetCSpecificHandler
    // which needs to be called now with the __C_specific_handler that Rtl will
    // have initialized.
    //

#define INIT_C_SPECIFIC_HANDLER(Name) do {                           \
    SetCSpecificHandler = (PSET_C_SPECIFIC_HANDLER)(                 \
        GetProcAddress(Session->Name##Module, "SetCSpecificHandler") \
    );                                                               \
    if (SetCSpecificHandler) {                                       \
        SetCSpecificHandler(Rtl->__C_specific_handler);              \
    }                                                                \
} while (0)

    INIT_C_SPECIFIC_HANDLER(Python);
    INIT_C_SPECIFIC_HANDLER(TracerHeap);
    INIT_C_SPECIFIC_HANDLER(TraceStore);
    INIT_C_SPECIFIC_HANDLER(StringTable);
    INIT_C_SPECIFIC_HANDLER(PythonTracer);

    //
    // Use the same approach for any DLLs that require extended atexit support.
    //

#define INIT_ATEXITEX(Name) do {                             \
    SetAtExitEx = (PSET_ATEXITEX)(                           \
        GetProcAddress(Session->Name##Module, "SetAtExitEx") \
    );                                                       \
    if (SetAtExitEx) {                                       \
        SetAtExitEx(Rtl->AtExitEx);                          \
    }                                                        \
} while (0)

    INIT_ATEXITEX(Python);
    INIT_ATEXITEX(TracerHeap);
    INIT_ATEXITEX(TraceStore);
    INIT_ATEXITEX(StringTable);
    INIT_ATEXITEX(PythonTracer);

    //
    // Allocate and initialize TraceSession.
    //

    RequiredSize = 0;
    Session->InitializeTraceSession(Rtl, NULL, &RequiredSize);
    ALLOCATE(TraceSession, PTRACE_SESSION);
    Success = Session->InitializeTraceSession(
        Rtl,
        Session->TraceSession,
        &RequiredSize
    );

    if (!Success) {
        OutputDebugStringA("Session->InitializeTraceSession() failed.\n");
        goto Error;
    }

    //
    // Create a trace session directory if we're not readonly, otherwise, use
    // the directory that the caller passed in.
    //

    if (IsReadonly) {

        Session->TraceSessionDirectory = TraceSessionDirectory;

    } else {

        Success = CreateTraceSessionDirectory(
            TracerConfig,
            &Session->TraceSessionDirectory,
            &Session->SystemTime
        );

        if (!Success) {
            OutputDebugStringA("CreateTraceSessionDirectory() failed.\n");
            goto Error;
        }

        //
        // Update the caller's pointer.
        //

        *TraceSessionDirectoryPointer = Session->TraceSessionDirectory;
    }

    //
    // Take a local copy of the flags.
    //

    TracerConfigFlags = TracerConfig->Flags;

    //
    // See if we've been configured to compress trace store directories.
    //

    Compress = !TracerConfigFlags.DisableTraceSessionDirectoryCompression;

    //
    // Convert into a TRACE_FLAGS representation.
    //

    TraceFlags.AsLong = 0;
    TraceFlags.Compress = Compress;
    TraceFlags.Readonly = IsReadonly;

#define COPY_FLAG(Name) TraceFlags.##Name = TracerConfigFlags.##Name

    COPY_FLAG(DisablePrefaultPages);
    COPY_FLAG(DisableFileFlagOverlapped);
    COPY_FLAG(DisableFileFlagSequentialScan);
    COPY_FLAG(EnableFileFlagRandomAccess);
    COPY_FLAG(EnableFileFlagWriteThrough);
    COPY_FLAG(EnableWorkingSetTracing);
    COPY_FLAG(EnablePerformanceTracing);
    COPY_FLAG(EnableLoaderTracing);
    COPY_FLAG(EnableSymbolTracing);
    COPY_FLAG(EnableTypeInfoTracing);
    COPY_FLAG(EnableAssemblyTracing);
    COPY_FLAG(IgnoreModulesInWindowsSystemDirectory);
    COPY_FLAG(IgnoreModulesInWindowsSxSDirectory);

    //
    // If symbol tracing has been enabled, ensure we can load DbgHelp.
    //

    if (TraceFlags.EnableSymbolTracing) {
        if (!Rtl->LoadDbgHelp(Rtl)) {
            OutputDebugStringA("Rtl->LoadDbgHelp() failed.  "
                               "Disabling symbol tracing.\n");
            TraceFlags.EnableSymbolTracing = FALSE;
        }
    }

    //
    // Get the required size of the TRACE_STORES structure.
    //

    RequiredSize = 0;
    Session->InitializeTraceStores(
        NULL,           // Rtl
        NULL,           // Allocator
        NULL,           // TracerConfig
        NULL,           // BaseDirectory
        NULL,           // TraceStores
        &RequiredSize,  // SizeOfTraceStores
        NULL,           // InitialFileSizes
        NULL,           // MappingSizes
        &TraceFlags,    // TraceFlags
        NULL,           // FieldRelocations
        NULL,           // TraitsArray
        NULL            // ExcludeBitmap
    );

    //
    // Disable pre-faulting of pages if applicable.
    //

    if (TracerConfig->Flags.DisablePrefaultPages) {
        TraceFlags.DisablePrefaultPages = TRUE;
    }

    Session->NumberOfBuffers = 0;

    //
    // Attempt to get the SE_MANAGE_VOLUME privilege.
    //

    ManageVolumeEnabled = Rtl->EnableManageVolumePrivilege();
    LockMemoryEnabled = Rtl->EnableLockMemoryPrivilege();

    LargePageMinimum = GetLargePageMinimum();
    BufferSize = max(1 << 21, LargePageMinimum);
    BufferAllocationFlags = MEM_COMMIT;

    if (LockMemoryEnabled) {
        BufferAllocationFlags |= MEM_LARGE_PAGES;
    }

    Buffer = VirtualAlloc(NULL,
                          BufferSize,
                          BufferAllocationFlags,
                          PAGE_READWRITE);

    if (Buffer) {
        Session->Buffers[Session->NumberOfBuffers++] = Buffer;

        Buffer = VirtualAlloc(NULL,
                              BufferSize,
                              BufferAllocationFlags,
                              PAGE_READWRITE);

        if (Buffer) {
            Session->Buffers[Session->NumberOfBuffers++] = Buffer;
        }
    }

    //
    // Allocate sufficient space, then initialize the stores.
    //

    ALLOCATE(TraceStores, PTRACE_STORES);
    Success = Session->InitializeTraceStores(
        Rtl,
        Allocator,
        Session->TracerConfig,
        Session->TraceSessionDirectory->Buffer,
        Session->TraceStores,
        &RequiredSize,
        NULL,
        NULL,
        &TraceFlags,
        Session->PythonTracerTraceStoreRelocations,
        NULL,
        NULL
    );

    if (!Success) {
        OutputDebugStringA("InitializeTraceStores() failed.\n");
        goto Error;
    }

    //
    // Get the maximum number of processors and create a threadpool
    // and environment.
    //

    Session->MaximumProcessorCount = (
        GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS)
    );

    Session->Threadpool = CreateThreadpool(NULL);
    if (!Session->Threadpool) {
        OutputDebugStringA("CreateThreadpool() failed.\n");
        goto Error;
    }

    //
    // Limit the threadpool to 2 times the number of processors in the system.
    //

    SetThreadpoolThreadMinimum(
        Session->Threadpool,
        Session->MaximumProcessorCount
    );

    SetThreadpoolThreadMaximum(
        Session->Threadpool,
        Session->MaximumProcessorCount * 2
    );

    //
    // Initialize the threadpool environment and bind it to the threadpool.
    //

    InitializeThreadpoolEnvironment(&Session->ThreadpoolCallbackEnviron);
    SetThreadpoolCallbackPool(
        &Session->ThreadpoolCallbackEnviron,
        Session->Threadpool
    );

    //
    // Create a cancellation threadpool with one thread.
    //

    Session->CancellationThreadpool = CreateThreadpool(NULL);
    if (!Session->CancellationThreadpool) {
        OutputDebugStringA("CreateThreadpool() failed for cancellation.\n");
        goto Error;
    }

    SetThreadpoolThreadMinimum(Session->CancellationThreadpool, 1);
    SetThreadpoolThreadMaximum(Session->CancellationThreadpool, 1);
    InitializeThreadpoolEnvironment(
        &Session->CancellationThreadpoolCallbackEnviron
    );

    //
    // Clear the trace context flags.
    //

    //
    // Get the required size of a TRACE_CONTEXT structure.
    //

    RequiredSize = 0;
    Session->InitializeTraceContext(
        NULL,           // Rtl
        NULL,           // Allocator
        NULL,           // TracerConfig
        NULL,           // TraceContext
        &RequiredSize,  // SizeOfTraceContext
        NULL,           // TraceSession
        NULL,           // TraceStores
        NULL,           // ThreadpoolCallbackEnviron
        NULL,           // CancellationThreadpoolCallbackEnviron
        NULL,           // TraceContextFlags
        NULL            // UserData
    );

    //
    // Allocate sufficient space then initialize the trace context.
    //

    ALLOCATE(TraceContext, PTRACE_CONTEXT);
    Success = Session->InitializeTraceContext(
        Rtl,
        Allocator,
        Session->TracerConfig,
        Session->TraceContext,
        &RequiredSize,
        Session->TraceSession,
        Session->TraceStores,
        &Session->ThreadpoolCallbackEnviron,
        &Session->CancellationThreadpoolCallbackEnviron,
        NULL,
        (PVOID)Session
    );

    if (!Success) {
        OutputDebugStringA("InitializeTraceContext() failed.\n");
        goto Error;
    }

    //
    // Make a note of the loading complete event.  If an error is encountered
    // after this point, the error handling logic at the Error: label will wait
    // for this event to be set first before destroying the trace context.
    //

    LoadingCompleteEvent = Session->TraceContext->LoadingCompleteEvent;

    Session->PythonDllModule = PythonDllModule;

    //
    // Call InitializePython() now that we've loaded a Python DLL.
    //

    RequiredSize = 0;
    Session->InitializePython(NULL, NULL, NULL, &RequiredSize);
    ALLOCATE(Python, PPYTHON);
    Success = Session->InitializePython(
        Rtl,
        PythonDllModule,
        Session->Python,
        &RequiredSize
    );

    if (!Success) {
        OutputDebugStringA("InitializePython() failed.\n");
        goto Error;
    }

    Python = Session->Python;

    //
    // Get the required size of a PYTHON_TRACE_CONTEXT structure.
    //

    RequiredSize = 0;
    Session->InitializePythonTraceContext(
        NULL,           // Rtl
        NULL,           // Allocator
        NULL,           // PythonTraceContext
        &RequiredSize,  // SizeOfPythonTraceContext
        NULL,           // Python
        NULL,           // TraceContext
        NULL,           // StringTableApi
        NULL            // UserData
    );

    //
    // Allocate space and initialize the Python trace context.
    //

    ALLOCATE(PythonTraceContext, PPYTHON_TRACE_CONTEXT);
    Success = Session->InitializePythonTraceContext(
        Rtl,
        Allocator,
        Session->PythonTraceContext,
        &RequiredSize,
        Session->Python,
        Session->TraceContext,
        &Session->StringTableApi,
        (PVOID)Session
    );

    if (!Success) {
        OutputDebugStringA("InitializePythonTraceContext() failed.\n");
        goto Error;
    }

    //
    // Initialize alias.
    //

    PythonTraceContext = Session->PythonTraceContext;

    //
    // Set the trace context's system time.
    //

    Success = PythonTraceContext->SetSystemTimeForRunHistory(
        PythonTraceContext,
        &Session->SystemTime
    );

    if (!Success) {
        OutputDebugStringA("SetSystemTimeForRunHistory() failed.\n");
        goto Error;
    }

    //
    // Save the run history registry key to the trace context as well.
    //

    Key = Session->TraceContext->RunHistoryRegistryKey = (
        PythonTraceContext->RunHistoryRegistryKey
    );

    //
    // Define a helper macro for writing strings to the run history.
    //

#define WRITE_REG_SZ(Name, Value)                                    \
    Success = Rtl->WriteRegistryString(                              \
        Rtl,                                                         \
        Allocator,                                                   \
        Key,                                                         \
        L#Name,                                                      \
        Value                                                        \
    );                                                               \
    if (!Success) {                                                  \
        OutputDebugStringA("WriteRegistryString() failed.\nName: "); \
        OutputDebugStringA(#Name);                                   \
        OutputDebugStringA("\nValue: ");                             \
        OutputDebugStringW((Value)->Buffer);                         \
    }

#define WRITE_SESSION_REG_SZ(Name) WRITE_REG_SZ(Name, Session->##Name)

#define WRITE_ENV_VAR_REG_SZ(EnvVarName, RegName)                      \
    Success = Rtl->WriteEnvVarToRegistry(                              \
        Rtl,                                                           \
        Allocator,                                                     \
        Key,                                                           \
        EnvVarName,                                                    \
        RegName                                                        \
    );                                                                 \
    if (!Success) {                                                    \
        OutputDebugStringA("WriteEnvVarToRegistry() failed.\nName: "); \
        OutputDebugStringW(EnvVarName);                                \
        OutputDebugStringA("\nRegName: ");                             \
        OutputDebugStringW(RegName);                                   \
    }

#define WRITE_ENV_VAR(Name) WRITE_ENV_VAR_REG_SZ(L#Name, L#Name)

    WRITE_SESSION_REG_SZ(PythonExePath);
    WRITE_SESSION_REG_SZ(PythonHomePath);
    WRITE_SESSION_REG_SZ(OriginalDirectory);

    //
    // Write the process ID and this thread ID.
    //

    WRITE_REG_DWORD(Key, ProcessId, FastGetCurrentProcessId());
    WRITE_REG_DWORD(Key, MainThreadId, FastGetCurrentThreadId());

    //
    // Write the command line to the run history, wrapping it in a Unicode
    // string structure first.
    //

    CommandLine.Length = (USHORT)(wcslen(Session->CommandLineW) << 1);
    CommandLine.MaximumLength = CommandLine.Length + sizeof(WCHAR);
    CommandLine.Buffer = Session->CommandLineW;
    if (CommandLine.Buffer[CommandLine.Length >> 1] != L'\0') {
        __debugbreak();
        goto Error;
    }

    WRITE_REG_SZ(CommandLine, &CommandLine);
    WRITE_REG_SZ(TraceDirectory, &Session->TraceStores->BaseDirectory);

    WRITE_ENV_VAR(TMP);
    WRITE_ENV_VAR(TEMP);
    WRITE_ENV_VAR(PATH);
    WRITE_ENV_VAR(PYTHONPATH);
    WRITE_ENV_VAR(PYTHONWARNINGS);
    WRITE_ENV_VAR(ALLUSERSPROFILE);
    WRITE_ENV_VAR(USERNAME);
    WRITE_ENV_VAR(USERDOMAIN);
    WRITE_ENV_VAR(USERDOMAIN_ROAMINGPROFILE);
    WRITE_ENV_VAR(USERPROFILE);
    WRITE_ENV_VAR(COMPUTERNAME);
    WRITE_ENV_VAR(SESSIONNAME);
    WRITE_ENV_VAR(NUMBER_OF_PROCESSORS);
    WRITE_ENV_VAR(PROCESSOR_IDENTIFIER);
    WRITE_ENV_VAR(PROCESSOR_LEVEL);
    WRITE_ENV_VAR(PROCESSOR_REVISION);
    WRITE_ENV_VAR(_NT_SYMBOL_PATH);
    WRITE_ENV_VAR(_NT_ALTERNATE_SYMBOL_PATH);
    WRITE_ENV_VAR(_NT_DEBUGGER_EXTENSION_PATH);
    WRITE_ENV_VAR(CONDA_DEFAULT_ENV);
    WRITE_ENV_VAR(CONDA_ENVS_PATH);
    WRITE_ENV_VAR(CONDA_PREFIX);
    WRITE_ENV_VAR(CUDA_PATH_V7_5);
    WRITE_ENV_VAR(CUDA_PATH_V8_0);
    WRITE_ENV_VAR(NVTOOLSEXT_PATH);
    WRITE_ENV_VAR(VS90COMNTOOLS);
    WRITE_ENV_VAR(VS100COMNTOOLS);
    WRITE_ENV_VAR(VS110COMNTOOLS);
    WRITE_ENV_VAR(VS120COMNTOOLS);
    WRITE_ENV_VAR(VS140COMNTOOLS);
    WRITE_ENV_VAR(VS150COMNTOOLS);

    //
    // Initialize the string table and string array allocators.
    //

    Session->StringTableAllocator = &(
        TraceStoreIdToTraceStore(
            Session->TraceStores,
            TraceStoreStringTableId
        )
    )->Allocator;

    Session->StringArrayAllocator = &(
        TraceStoreIdToTraceStore(
            Session->TraceStores,
            TraceStoreStringArrayId
        )
    )->Allocator;

    if (CreateStringTableForTracerModuleNamesEnvironmentVariable(Session)) {
        PSTRING_TABLE ModuleNames = Session->ModuleNamesStringTable;
        PSET_MODULE_NAMES_STRING_TABLE SetModuleNames;

        SetModuleNames = PythonTraceContext->SetModuleNamesStringTable;

        if (!SetModuleNames(PythonTraceContext, ModuleNames)) {
            OutputDebugStringA("SetModuleNamesStringTable() failed.\n");
            goto Error;
        }

        WRITE_ENV_VAR_REG_SZ(TRACER_MODULE_NAMES_ENV_VAR_W,
                             TRACER_MODULE_NAMES_ENV_VAR_W);

    }

    //
    // We're finally done.
    //

    Success = TRUE;

    goto End;

Error:
    Success = FALSE;

    //
    // If the loading complete event handle is non-NULL, we need to wait on
    // this first before we can destroy the traced python session.
    //

    if (LoadingCompleteEvent) {
        HRESULT WaitResult;

        OutputDebugStringA("Destroy: waiting for LoadingCompleteEvent.\n");
        WaitResult = WaitForSingleObject(LoadingCompleteEvent, INFINITE);
        if (WaitResult != WAIT_OBJECT_0) {

            //
            // The wait was abandoned or we encountered some other error.  Go
            // straight to the end and skip the destroy step.
            //

            OutputDebugStringA("Wait failed, skipping destroy.\n");
            goto End;
        }
    }

    DestroyTracedPythonSession(&Session);

End:

    //
    // Update the user's pointer if we were successful.
    //

    if (Success) {
        *SessionPointer = Session;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
