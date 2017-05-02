/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Ext.h

Abstract:

    This is the main header file for the TraceStore sqlite3 extension component.
    It defines structures and functions related to the implementation of the
    component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define the trace store database structure.
//

typedef union _TRACE_STORE_SQLITE3_DB_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_DB_FLAGS;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_DB_FLAGS) == sizeof(ULONG));

typedef union _TRACE_STORE_SQLITE3_DB_STATE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_DB_STATE;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_DB_STATE) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORE_SQLITE3_DB {

    _Field_range_(==, sizeof(struct _TRACE_STORE_SQLITE3_DB))
    ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRACE_STORE_SQLITE3_DB_FLAGS Flags;

    //
    // State.
    //

    TRACE_STORE_SQLITE3_DB_STATE State;

    //
    // Sqlite3-specific fields.
    //

    PALLOCATOR Sqlite3Allocator;
    PSQLITE3_DB Sqlite3Db;
    PCSQLITE3 Sqlite3;

    //
    // Our internal fields.
    //

    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING TraceSessionDirectory;

    HANDLE OwningModule;
    PRTL_PATH OwningModulePath;

    //
    // Our core tracing/support modules.
    //

    HMODULE TracerHeapModule;
    HMODULE TraceStoreModule;
    HMODULE StringTableModule;

    //
    // TracerHeap-specific functions.
    //

    PINITIALIZE_HEAP_ALLOCATOR_EX InitializeHeapAllocatorEx;

    //
    // StringTable-specific functions.
    //

    PCREATE_STRING_TABLE CreateStringTable;
    PDESTROY_STRING_TABLE DestroyStringTable;

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
        CreateStringTableFromDelimitedEnvironmentVariable;

    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;

    //
    // TraceStore-specific initializers.
    //

    PINITIALIZE_TRACE_STORES InitializeTraceStores;
    PINITIALIZE_TRACE_CONTEXT InitializeTraceContext;
    PINITIALIZE_TRACE_SESSION InitializeTraceSession;
    PCLOSE_TRACE_STORES CloseTraceStores;
    PCLOSE_TRACE_CONTEXT CloseTraceContext;
    PINITIALIZE_ALLOCATOR_FROM_TRACE_STORE InitializeAllocatorFromTraceStore;

    //
    // Threadpool and callback environment.
    //

    ULONG MaximumProcessorCount;
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON ThreadpoolCallbackEnviron;

    //
    // A threadpool with 1 thread limited to cancellations.
    //

    PTP_POOL CancellationThreadpool;
    TP_CALLBACK_ENVIRON CancellationThreadpoolCallbackEnviron;

    ALLOCATOR Allocator;

    PTRACE_SESSION TraceSession;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;

} TRACE_STORE_SQLITE3_DB;
typedef TRACE_STORE_SQLITE3_DB *PTRACE_STORE_SQLITE3_DB;


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
