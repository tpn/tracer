/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqliteSchemas.c

Abstract:

    This module defines sqlite3 virtual table schemas.

--*/

#include "stdafx.h"

#define PLACEHOLDER_SCHEMA "CREATE TABLE x(Dummy INTEGER)"

//
// MetadataInfo
//

CONST CHAR TraceStoreMetadataInfoSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3MetadataInfoColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// Allocation
//

CONST CHAR TraceStoreAllocationSchema[] =
    "CREATE TABLE Allocation ("
        "NumberOfRecords BIGINT, "
        "RecordSize BIGINT, "
        "IsDummyAllocation TINYINT"
    ")";

_Use_decl_annotations_
LONG
TraceStoreSqlite3AllocationColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_STORE_ALLOCATION Allocation;

    Allocation = Cursor->CurrentRow.AsAllocation;

    //
    // Define helper macros.
    //

#define ALLOCATION_INT64(Name)                       \
    Sqlite3->ResultInt64(                            \
        Context,                                     \
        (SQLITE3_INT64)Allocation->##Name##.QuadPart \
    )

#define ALLOCATION_TINYINT(Name)        \
    Sqlite3->ResultInt(          \
        Context,                 \
        (LONG)Allocation->##Name \
    )

    switch (ColumnNumber) {

        //
        // NumberOfRecords BIGINT
        //

        case 0:
            ALLOCATION_INT64(NumberOfRecords);
            break;

        //
        // RecordSize BIGINT
        //

        case 1:
            ALLOCATION_INT64(RecordSize);
            break;

        //
        // IsDummyAllocation TINYINT
        //

        case 2:
            ALLOCATION_TINYINT(NumberOfRecords.DummyAllocation2);
            break;

        default:
            __debugbreak();
            return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

//
// Relocation
//

CONST CHAR TraceStoreRelocationSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3RelocationColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// Address
//

CONST CHAR TraceStoreAddressSchema[] =
    "CREATE TABLE Address ("
        "PreferredBaseAddress BIGINT, "
        "BaseAddress BIGINT, "
        "FileOffset BIGINT, "
        "MappedSize BIGINT, "
        "ProcessId INT, "
        "RequestingThreadId INT, "
        "Requested BIGINT, "
        "Prepared BIGINT, "
        "Consumed BIGINT, "
        "Retired BIGINT, "
        "Released BIGINT, "
        "AwaitingPreparation BIGINT, "
        "AwaitingConsumption BIGINT, "
        "Active BIGINT, "
        "AwaitingRelease BIGINT, "
        "MappedSequenceId INT, "
        "RequestingProcGroup SMALLINT, "
        "RequestingNumaNode TINYINT, "
        "FulfillingThreadId INT"
    ")";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AddressColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3AddressColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_STORE_ADDRESS Address;

    Address = Cursor->CurrentRow.AsAddress;

    //
    // Define helper macros for addresses, timestamps and elapsed values.
    //

#define RESULT_ADDRESS(Name) \
    Sqlite3->ResultInt64(Context, (SQLITE3_INT64)Address->##Name)

#define RESULT_TIMESTAMP(Name) \
    Sqlite3->ResultInt64(Context, Address->Timestamp.##Name##.QuadPart);

#define RESULT_ELAPSED(Name) \
    Sqlite3->ResultInt64(Context, Address->Elapsed.##Name##.QuadPart);

    switch (ColumnNumber) {

        //
        // PreferredBaseAddress BIGINT
        //

        case 0:
            RESULT_ADDRESS(PreferredBaseAddress);
            break;

        //
        // BaseAddress BIGINT
        //

        case 1:
            RESULT_ADDRESS(BaseAddress);
            break;

        //
        // FileOffset BIGINT
        //

        case 2:
            Sqlite3->ResultInt64(Context, Address->FileOffset.QuadPart);
            break;

        //
        // MappedSize BIGINT
        //

        case 3:
            Sqlite3->ResultInt64(Context, Address->MappedSize.QuadPart);
            break;

        //
        // ProcessId INT
        //

        case 4:
            Sqlite3->ResultInt(Context, Address->ProcessId);
            break;

        //
        // RequestingThreadId INT
        //

        case 5:
            Sqlite3->ResultInt(Context, Address->RequestingThreadId);
            break;

        //
        // Requested BIGINT
        //

        case 6:
            RESULT_TIMESTAMP(Requested);
            break;


        //
        // Prepared BIGINT
        //

        case 7:
            RESULT_TIMESTAMP(Prepared);
            break;


        //
        // Consumed BIGINT
        //

        case 8:
            RESULT_TIMESTAMP(Consumed);
            break;

        //
        // Retired BIGINT
        //

        case 9:
            RESULT_TIMESTAMP(Retired);
            break;

        //
        // Released BIGINT
        //

        case 10:
            RESULT_TIMESTAMP(Released);
            break;

        //
        // AwaitingPreparation BIGINT
        //

        case 11:
            RESULT_ELAPSED(AwaitingPreparation);
            break;

        //
        // AwaitingConsumption BIGINT
        //

        case 12:
            RESULT_ELAPSED(AwaitingPreparation);
            break;

        //
        // Active BIGINT
        //

        case 13:
            RESULT_ELAPSED(Active);
            break;

        //
        // AwaitingRelease BIGINT
        //

        case 14:
            RESULT_ELAPSED(AwaitingRelease);
            break;

        //
        // MappedSequenceId INT
        //

        case 15:
            Sqlite3->ResultInt(Context, Address->MappedSequenceId);
            break;

        //
        // RequestingProcGroup SMALLINT
        //

        case 16:
            Sqlite3->ResultInt(Context, Address->RequestingProcGroup);
            break;

        //
        // RequestingNumaNode TINYINT
        //

        case 17:
            Sqlite3->ResultInt(Context, Address->RequestingNumaNode);
            break;

        //
        // FulfillingThreadId INT
        //

        case 18:
            Sqlite3->ResultInt64(Context, Address->FulfillingThreadId);
            break;

        default:
            __debugbreak();
            Sqlite3->ResultNull(Context);
            return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

//
// AddressRange
//

CONST CHAR TraceStoreAddressRangeSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3AddressRangeColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// AllocationTimestamp
//

CONST CHAR TraceStoreAllocationTimestampSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3AllocationTimestampColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// AllocationTimestampDelta
//

CONST CHAR TraceStoreAllocationTimestampDeltaSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3AllocationTimestampDeltaColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// Synchronization
//

CONST CHAR TraceStoreSynchronizationSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3SynchronizationColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

//
// Info
//

CONST CHAR TraceStoreInfoSchema[] =
    PLACEHOLDER_SCHEMA;

_Use_decl_annotations_
LONG
TraceStoreSqlite3InfoColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return TraceStoreSqlite3DefaultColumnImpl(Sqlite3,
                                              TraceStore,
                                              Cursor,
                                              Context,
                                              ColumnNumber);
}

CONST LPCSTR TraceStoreSchemas[] = {
    PLACEHOLDER_SCHEMA, // PythonTracer_TraceEvent,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_StringBuffer,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonFunctionTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonFunctionTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonPathTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonPathTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_PageFault,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // StringTable_StringArray,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // StringTable_StringTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // PythonTracer_EventTraitsEx,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_WsWatchInfoEx,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_WorkingSetEx,
    TraceStoreInfoSchema,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_CCallStackTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_CCallStackTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ModuleTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ModuleTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonCallStackTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonCallStackTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonModuleTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // Python_PythonModuleTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_LineTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_LineTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_LineStringBuffer,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_CallStack,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_Performance,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_PerformanceDelta,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SourceCode,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_Bitmap,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ImageFile,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_UnicodeStringBuffer,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_Line,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_Object,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ModuleLoadEvent,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolModule
    TraceStoreInfoSchema,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolFile,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolInfo
    TraceStoreInfoSchema,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolLine,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_SymbolType,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_StackFrame,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoStringBuffer,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_FunctionTable,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_FunctionTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_FunctionAssembly,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_FunctionSourceCode,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ExamineSymbolsLine,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ExamineSymbolsText,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ExaminedSymbol,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_ExaminedSymbolSecondary,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_UnassembleFunctionLine,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_UnassembleFunctionText,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_UnassembledFunction,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_UnassembledFunctionSecondary,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_DisplayTypeLine,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_DisplayTypeText,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_DisplayedType,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
    PLACEHOLDER_SCHEMA, // TraceStore_DisplayedTypeSecondary,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,
};

CONST PTRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3Columns[] = {
    TraceStoreSqlite3DefaultColumnImpl, // PythonTracer_TraceEvent,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_StringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonFunctionTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonFunctionTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonPathTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonPathTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_PageFault,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // StringTable_StringArray,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // StringTable_StringTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // PythonTracer_EventTraitsEx,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_WsWatchInfoEx,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_WorkingSetEx,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_CCallStackTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_CCallStackTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ModuleTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ModuleTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonCallStackTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonCallStackTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonModuleTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // Python_PythonModuleTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_LineTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_LineTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_LineStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_CallStack,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_Performance,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_PerformanceDelta,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SourceCode,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_Bitmap,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ImageFile,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_UnicodeStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_Line,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_Object,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ModuleLoadEvent,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolModule
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolFile,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolInfo
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_SymbolType,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_StackFrame,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_TypeInfoTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_TypeInfoTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_TypeInfoStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_FunctionTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_FunctionTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_FunctionAssembly,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_FunctionSourceCode,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ExamineSymbolsLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ExamineSymbolsText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ExaminedSymbol,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_ExaminedSymbolSecondary,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_UnassembleFunctionLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_UnassembleFunctionText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_UnassembledFunction,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_UnassembledFunctionSecondary,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_DisplayTypeLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_DisplayTypeText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_DisplayedType,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
    TraceStoreSqlite3DefaultColumnImpl, // TraceStoreSqlite3_DisplayedTypeSecondary,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
