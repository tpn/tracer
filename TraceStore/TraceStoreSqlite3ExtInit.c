/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Init.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

TRACE_STORE_SQLITE3_EXT_INIT TraceStoreSqlite3ExtInit;

_Use_decl_annotations_
LONG
TraceStoreSqlite3ExtInit(
    PRTL Rtl,
    PALLOCATOR Sqlite3Allocator,
    PTRACER_CONFIG TracerConfig,
    PSQLITE3_DB Sqlite3Db,
    PCSZ *ErrorMessagePointer,
    PCSQLITE3 Sqlite3
    )
/*++

Routine Description:

    This is the main entry point for the TraceStore sqlite3 extension module.
    It is called by our extension loader thunk (TraceStoreSqlite3ExtLoader).

    It is responsible for registering as a module to sqlite3.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Sqlite3Allocator - Supplies a pointer to an ALLOCATOR structure that has
        been initialized from the sqlite3 API (i.e. it will dispatch malloc etc
        calls to Sqlite3->Malloc64).

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    Sqlite3Db - Supplies a pointer to the active sqlite3 database.

    ErrorMessagePointer - Supplies a variable that optionally receives the
        address of an error message if initialization fails.

    Sqlite3 - Supplies a pointer to the sqlite3 API routines.

Return Value:

    SQLITE_OK on success, either SQLITE_NOMEM or SQLITE_ERROR on error.

--*/
{
    BOOL Success;
    PCHAR Char;
    USHORT Count;
    USHORT Length;
    ULONG Result;
    ULONG RequiredSize;
    PCSZ DatabaseFilename;
    STRING Filename;
    PALLOCATOR Allocator;
    PUNICODE_STRING Path;
    PTRACER_PATHS Paths;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PTRACE_SESSION TraceSession;
    PTRACE_STORE_SQLITE3_DB Db;
    HANDLE LoadingCompleteEvent = NULL;
    TRACE_FLAGS TraceFlags;
    TRACE_CONTEXT_FLAGS TraceContextFlags;
    PSET_ATEXITEX SetAtExitEx;
    PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(Sqlite3Allocator)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(Sqlite3Db)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(ErrorMessagePointer)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(Sqlite3)) {
        return SQLITE_ERROR;
    }

    //
    // Allocate space for the TRACE_STORE_SQLITE3_DB structure.
    //

    Db = (PTRACE_STORE_SQLITE3_DB)Sqlite3->Malloc64(sizeof(*Db));
    if (!Db) {
        return SQLITE_NOMEM;
    }

    SecureZeroMemory(Db, sizeof(*Db));

    //
    // Initialize fields.
    //

    Db->Rtl = Rtl;
    Db->Sqlite3 = Sqlite3;
    Db->Sqlite3Db = Sqlite3Db;
    Db->TracerConfig = TracerConfig;
    Db->Sqlite3Allocator = Sqlite3Allocator;

    Db->SizeOfStruct = sizeof(*Db);

    //
    // Initialize aliases.
    //

    TraceStores = Db->TraceStores;
    TraceContext = Db->TraceContext;
    TraceSession = Db->TraceSession;

#define LOAD_DLL(Name) do {                                            \
    Db->Name##Module = LoadLibraryW(Paths->Name##DllPath.Buffer);      \
    if (!Db->Name##Module) {                                           \
        OutputDebugStringA("Failed to load " #Name);                   \
        goto Error;                                                    \
    }                                                                  \
} while (0)

    Paths = &TracerConfig->Paths;

    LOAD_DLL(TracerHeap);
    LOAD_DLL(TraceStore);
    LOAD_DLL(StringTable);

    //
    // Resolve the functions.
    //

#define RESOLVE(Module, Type, Name) do {                                  \
    Db->Name = (Type)GetProcAddress(Db->Module, #Name);                   \
    if (!Db->Name) {                                                      \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name "\n"); \
        goto Error;                                                       \
    }                                                                     \
} while (0)

    RESOLVE(TracerHeapModule,
            PINITIALIZE_HEAP_ALLOCATOR_EX,
            InitializeHeapAllocatorEx);

    //
    // Initialize our Db-specific Allocator.
    //

    Allocator = &Db->Allocator;
    Success = Db->InitializeHeapAllocatorEx(&Db->Allocator, 0, 0, 0);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // Load our owning module name.
    //

    Db->OwningModule = GetModuleHandle(NULL);

    Success = Rtl->GetModuleRtlPath(Rtl,
                                    Db->OwningModule,
                                    Allocator,
                                    &Db->OwningModulePath);

    if (!Success) {
        OutputDebugStringA("Rtl->GetModuleRtlPath() failed.\n");
        goto Error;
    }

    //
    // Path initialization.
    //

    DatabaseFilename = Sqlite3->DbFilename(Sqlite3Db, "main");
    Length = (USHORT)strlen(DatabaseFilename);

    //
    // Point our Char pointer at the end of the string.
    //

    Count = 0;
    Char = (PSZ)(DatabaseFilename + Length);
    while (*Char != '\\') {
        Char--;
        Count++;
    }

    Filename.Buffer = (PSZ)DatabaseFilename;
    Filename.Length = Length - Count;
    Filename.MaximumLength = Filename.Length + 1;

    Success = ConvertUtf8StringToUtf16StringSlow(&Filename,
                                                 &Db->TraceSessionDirectory,
                                                 Allocator,
                                                 NULL);

    if (!Success) {
        __debugbreak();
        goto Error;
    }

    Path = Db->TraceSessionDirectory;
    if (Path->Buffer[Path->Length >> 1] != L'\0') {
        __debugbreak();
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
    Db->Target = (Type)(                                                   \
        Allocator->Calloc(                                                 \
            Allocator->Context,                                            \
            1,                                                             \
            RequiredSize                                                   \
        )                                                                  \
    );                                                                     \
    if (!Db->Target) {                                                     \
        OutputDebugStringA("Allocation failed for " #Target " struct.\n"); \
        goto Error;                                                        \
    }

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
    // StringTable
    //

    RESOLVE(StringTableModule,
            PCREATE_STRING_TABLE,
            CreateStringTable);

    RESOLVE(StringTableModule,
            PCREATE_STRING_TABLE_FROM_DELIMITED_STRING,
            CreateStringTableFromDelimitedString);

    RESOLVE(StringTableModule,
            PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE,
            CreateStringTableFromDelimitedEnvironmentVariable);

    //
    // If any of our DLLs use structured exception handling (i.e. contain
    // code that uses __try/__except), they'll also export SetCSpecificHandler
    // which needs to be called now with the __C_specific_handler that Rtl will
    // have initialized.
    //

#define INIT_C_SPECIFIC_HANDLER(Name) do {                           \
    SetCSpecificHandler = (PSET_C_SPECIFIC_HANDLER)(                 \
        GetProcAddress(Db->Name##Module, "SetCSpecificHandler")      \
    );                                                               \
    if (SetCSpecificHandler) {                                       \
        SetCSpecificHandler(Rtl->__C_specific_handler);              \
    }                                                                \
} while (0)

    INIT_C_SPECIFIC_HANDLER(TracerHeap);
    INIT_C_SPECIFIC_HANDLER(TraceStore);
    INIT_C_SPECIFIC_HANDLER(StringTable);

    //
    // Use the same approach for any DLLs that require extended atexit support.
    //

#define INIT_ATEXITEX(Name) do {                             \
    SetAtExitEx = (PSET_ATEXITEX)(                           \
        GetProcAddress(Db->Name##Module, "SetAtExitEx")      \
    );                                                       \
    if (SetAtExitEx) {                                       \
        SetAtExitEx(Rtl->AtExitEx);                          \
    }                                                        \
} while (0)

    INIT_ATEXITEX(TracerHeap);
    INIT_ATEXITEX(TraceStore);
    INIT_ATEXITEX(StringTable);


    //
    // Allocate and initialize TraceSession.
    //

    RequiredSize = 0;
    Db->InitializeTraceSession(Rtl, NULL, &RequiredSize);
    ALLOCATE(TraceSession, PTRACE_SESSION);
    Success = Db->InitializeTraceSession(Rtl,
                                         Db->TraceSession,
                                         &RequiredSize);

    if (!Success) {
        OutputDebugStringA("Db->InitializeTraceSession() failed.\n");
        goto Error;
    }

    //
    // Initialize flags.
    //

    TraceFlags.AsULong = 0;
    TraceFlags.Readonly = TRUE;

    //
    // Get the required size of the TRACE_STORES structure.
    //

    RequiredSize = 0;
    Db->InitializeTraceStores(
        NULL,           // Rtl
        NULL,           // Allocator
        NULL,           // TracerConfig
        NULL,           // BaseDirectory
        NULL,           // TraceStores
        &RequiredSize,  // SizeOfTraceStores
        NULL,           // InitialFileSizes
        NULL,           // MappingSizes
        &TraceFlags,    // TraceFlags
        NULL            // FieldRelocations
    );

    //
    // Allocate sufficient space, then initialize the stores.
    //

    ALLOCATE(TraceStores, PTRACE_STORES);
    Success = Db->InitializeTraceStores(
        Rtl,
        Allocator,
        Db->TracerConfig,
        Db->TraceSessionDirectory->Buffer,
        Db->TraceStores,
        &RequiredSize,
        NULL,
        NULL,
        &TraceFlags,
        NULL //Db->PythonTracerTraceStoreRelocations
    );

    if (!Success) {
        OutputDebugStringA("InitializeTraceStores() failed.\n");
        goto Error;
    }

    //
    // Get the maximum number of processors and create a threadpool
    // and environment.
    //

    Db->MaximumProcessorCount = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);

    Db->Threadpool = CreateThreadpool(NULL);
    if (!Db->Threadpool) {
        OutputDebugStringA("CreateThreadpool() failed.\n");
        goto Error;
    }

    //
    // Limit the threadpool to 2 times the number of processors in the system.
    //

    SetThreadpoolThreadMinimum(Db->Threadpool, Db->MaximumProcessorCount);
    SetThreadpoolThreadMaximum(Db->Threadpool, Db->MaximumProcessorCount * 2);

    //
    // Initialize the threadpool environment and bind it to the threadpool.
    //

    InitializeThreadpoolEnvironment(&Db->ThreadpoolCallbackEnviron);
    SetThreadpoolCallbackPool(
        &Db->ThreadpoolCallbackEnviron,
        Db->Threadpool
    );

    //
    // Create a cancellation threadpool with one thread.
    //

    Db->CancellationThreadpool = CreateThreadpool(NULL);
    if (!Db->CancellationThreadpool) {
        OutputDebugStringA("CreateThreadpool() failed for cancellation.\n");
        goto Error;
    }

    SetThreadpoolThreadMinimum(Db->CancellationThreadpool, 1);
    SetThreadpoolThreadMaximum(Db->CancellationThreadpool, 1);
    InitializeThreadpoolEnvironment(
        &Db->CancellationThreadpoolCallbackEnviron
    );

    //
    // Clear the trace context flags and set readonly.
    //

    TraceContextFlags.AsULong = 0;
    TraceContextFlags.Readonly = TRUE;

    //
    // Get the required size of a TRACE_CONTEXT structure.
    //

    RequiredSize = 0;
    Db->InitializeTraceContext(
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
    Success = Db->InitializeTraceContext(
        Rtl,
        Allocator,
        Db->TracerConfig,
        Db->TraceContext,
        &RequiredSize,
        Db->TraceSession,
        Db->TraceStores,
        &Db->ThreadpoolCallbackEnviron,
        &Db->CancellationThreadpoolCallbackEnviron,
        &TraceContextFlags,
        (PVOID)Db
    );

    if (!Success) {
        *ErrorMessagePointer = "InitializeTraceContext() failed.\n";
        goto Error;
    }

    LoadingCompleteEvent = Db->TraceContext->LoadingCompleteEvent;

    //
    // We've successfully loaded the trace stores in this directory.  Proceed
    // with creation of the sqlite3 modules.
    //

    Result = Sqlite3->CreateModule(Sqlite3Db,
                                   "TraceStore",
                                   &TraceStoreSqlite3Module,
                                   NULL);

    Result = SQLITE_OK;

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

    //DestroyTracedPythonSession(&Session);

End:

    return Result;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
