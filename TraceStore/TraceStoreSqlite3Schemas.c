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
// Define helper macros for returning results.
//

#define RESULT_WCHAR(WideChar) \
    Sqlite3->ResultText16LE(   \
        Context,               \
        &WideChar,             \
        sizeof(WCHAR),         \
        SQLITE_STATIC          \
    )

#define RESULT_BLOB(Blob, Size) \
    Sqlite3->ResultBlob(        \
        Context,                \
        &Blob,                  \
        Size,                   \
        SQLITE_STATIC           \
    )

#define RESULT_PBLOB(Blob, Size) \
    Sqlite3->ResultBlob(         \
        Context,                 \
        Blob,                    \
        Size,                    \
        SQLITE_STATIC            \
    )

#define RESULT_BLOB64(Blob, Size) \
    Sqlite3->ResultBlob64(        \
        Context,                  \
        &Blob,                    \
        (SQLITE3_UINT64)Size,     \
        SQLITE_STATIC             \
    )

#define RESULT_PBLOB64(Blob, Size) \
    Sqlite3->ResultBlob64(         \
        Context,                   \
        Blob,                      \
        (SQLITE3_UINT64)Size,      \
        SQLITE_STATIC              \
    )

#define RESULT_PSTRING(String) \
    Sqlite3->ResultText16LE(   \
        Context,               \
        String##->Buffer,      \
        String##->Length,      \
        SQLITE_STATIC          \
    )

#define RESULT_STRING(String) \
    Sqlite3->ResultText(      \
        Context,              \
        String##.Buffer,      \
        String##.Length,      \
        SQLITE_STATIC         \
    )

#define RESULT_PUNICODE_STRING(UnicodeString) \
    Sqlite3->ResultText16LE(                  \
        Context,                              \
        UnicodeString##->Buffer,              \
        UnicodeString##->Length,              \
        SQLITE_STATIC                         \
    )

#define RESULT_UNICODE_STRING(UnicodeString) \
    Sqlite3->ResultText16LE(                 \
        Context,                             \
        UnicodeString##.Buffer,              \
        UnicodeString##.Length,              \
        SQLITE_STATIC                        \
    )


#define RESULT_LARGE_INTEGER(LargeInteger)     \
    Sqlite3->ResultInt64(                      \
        Context,                               \
        (SQLITE3_INT64)LargeInteger##.QuadPart \
    )

#define RESULT_ULONG(ULong) Sqlite3->ResultInt(Context, ULong)
#define RESULT_ULONGLONG(ULongLong) \
    Sqlite3->ResultInt64(Context, (SQLITE3_INT64)ULongLong)

#define INVALID_COLUMN()          \
    __debugbreak();               \
    Sqlite3->ResultNull(Context); \
    break

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

#define ALLOCATION_TINYINT(Name) \
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

#define RESULT_ADDRESS(Name)                                      \
    Sqlite3->ResultInt64(Context, (SQLITE3_INT64)Address->##Name)

#define RESULT_TIMESTAMP(Name)                                           \
    Sqlite3->ResultInt64(Context, Address->Timestamp.##Name##.QuadPart);

#define RESULT_ELAPSED(Name)                                           \
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
    "CREATE TABLE AllocationTimestamp("
        "Timestamp BIGINT"
    ")";

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
    Sqlite3->ResultInt64(Context, *Cursor->CurrentRow.AsSqlite3Int64);

    return SQLITE_OK;
}

//
// AllocationTimestampDelta
//

CONST CHAR TraceStoreAllocationTimestampDeltaSchema[] =
    "CREATE TABLE AllocationTimestampDelta("
        "Delta BIGINT"
    ")";

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
    Sqlite3->ResultInt64(Context, *Cursor->CurrentRow.AsSqlite3Int64);

    return SQLITE_OK;
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

//
// TraceStore_ModuleLoadEvent
//

CONST CHAR TraceStoreModuleLoadEventSchema[] =
    "CREATE TABLE TraceStore_ModuleLoadEvent("
        "Path TEXT, "                           // Path->Full, UNICODE_STRING
        "NumberOfSlashes SMALLINT, "            // Path->NumberOfSlashes
        "NumberOfDots SMALLINT, "               // Path->NumberOfDots
        "Drive TEXT, "                          // Path->Drive, WCHAR
        "CreationTime BIGINT, "                 // File->CreationTime
        "LastAccessTime BIGINT, "               // File->LastAccessTime
        "LastWriteTime BIGINT, "                // File->LastWriteTime
        "ChangeTime BIGINT, "                   // File->ChangeTime
        "FileAttributes INTEGER, "              // File->FileAttributes
        "NumberOfPages INTEGER, "               // File->NumberOfChanges
        "EndOfFile BIGINT, "                    // File->EndOfFile
        "AllocationSize BIGINT, "               // File->AllocationSize
        "FileId BIGINT, "                       // File->FileId
        "VolumeSerialNumber BIGINT, "           // File->VolumeSerialNumber
        "FileId128 BLOB, "                      // File->FileId128, sizeof()
        "MD5 BLOB, "                            // File->MD5, sizeof()
        "SHA1 BLOB, "                           // File->SHA1, sizeof()
        "CopyTimeInMicroseconds BIGINT, "       // File->CopyTimeInMicroseconds, LARGE_INTEGER
        "CopiedBytesPerSecond BIGINT, "         // File->CopiedBytesPerSecond, LARGE_INTEGER
        "Loaded BIGINT, "                       // LoadEvent->Timestamp.Loaded, LARGE_INTEGER
        "Unloaded BIGINT, "                     // LoadEvent->Timestamp.Unloaded, LARGE_INTEGER
        "PreferredBaseAddress BIGINT, "         // LoadEvent->PreferredBaseAddress
        "BaseAddress BIGINT, "                  // LoadEvent->BaseAddress
        "EntryPoint BIGINT"                     // LoadEvent->EntryPoint
    ")";

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleLoadEventColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_MODULE_LOAD_EVENT LoadEvent;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;
    PRTL_FILE File;
    PRTL_PATH Path;

    LoadEvent = (PTRACE_MODULE_LOAD_EVENT)Cursor->CurrentRowRaw;
    ModuleTableEntry = LoadEvent->ModuleTableEntry;
    File = &ModuleTableEntry->File;
    Path = &File->Path;

    switch (ColumnNumber) {

        case 0:
            RESULT_UNICODE_STRING(Path->Full);
            break;

        //
        // Loaded BIGINT
        //

        case 1:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Loaded);
            break;

        //
        // Unloaded BIGINT
        //

        case 2:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Unloaded);
            break;

        //
        // PreferredBaseAddress BIGINT
        //

        case 3:
            RESULT_ULONGLONG(LoadEvent->PreferredBaseAddress);
            break;

        //
        // BaseAddress BIGINT
        //

        case 4:
            RESULT_ULONGLONG(LoadEvent->BaseAddress);
            break;

        //
        // EntryPoint BIGINT
        //

        case 5:
            RESULT_ULONGLONG(LoadEvent->EntryPoint);
            break;

        default:
            __debugbreak();
            Sqlite3->ResultNull(Context);
            return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

//
// N.B. Having the Python stuff here is an encapsulation violation, however,
//      it does the job for now.
//

#include "..\Python\Python.h"

//
// Python_PythonFunctionTableEntry
//

CONST CHAR PythonFunctionTableEntrySchema[] =
    "CREATE TABLE Python_PythonFunctionTableEntry("
        "Path TEXT, "
        "FullName TEXT, " 
        "ModuleName TEXT, "
        "Name TEXT, "
        "ClassName TEXT, "
        "MaxCallStackDepth INTEGER, "
        "CallCount INTEGER, "
        "FirstLineNumber SMALLINT, "
        "NumberOfLines SMALLINT, "
        "NumberOfCodeLines SMALLINT, "
        "SizeOfByteCodeInBytes SMALLINT, "
        "Signature BIGINT, "
        "IsModuleDirectory TINYINT, "
        "IsNonModuleDirectory TINYINT, "
        "IsFileSystemDirectory TINYINT, "
        "IsFile TINYINT, "
        "IsClass TINYINT, "
        "IsFunction TINYINT, "
        "IsSpecial TINYINT, "
        "IsValid TINYINT, "
        "IsDll TINYINT, "
        "IsC TINYINT, "
        "IsBuiltin TINYINT, "
        "IsInitPy TINYINT, "
    ")";

_Use_decl_annotations_
LONG
TraceStoreSqlite3PythonFunctionTableEntryColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_MODULE_LOAD_EVENT LoadEvent;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;
    PRTL_FILE File;
    PRTL_PATH Path;

    LoadEvent = (PTRACE_MODULE_LOAD_EVENT)Cursor->CurrentRowRaw;
    ModuleTableEntry = LoadEvent->ModuleTableEntry;
    File = &ModuleTableEntry->File;
    Path = &File->Path;

    switch (ColumnNumber) {

        //
        // Path TEXT
        //

        case 0:
            RESULT_UNICODE_STRING(Path->Full);
            break;

        //
        // Loaded BIGINT
        //

        case 1:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Loaded);
            break;

        //
        // Unloaded BIGINT
        //

        case 2:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Unloaded);
            break;

        //
        // PreferredBaseAddress BIGINT
        //

        case 3:
            RESULT_ULONGLONG(LoadEvent->PreferredBaseAddress);
            break;

        //
        // BaseAddress BIGINT
        //

        case 4:
            RESULT_ULONGLONG(LoadEvent->BaseAddress);
            break;

        //
        // EntryPoint BIGINT
        //

        case 5:
            RESULT_ULONGLONG(LoadEvent->EntryPoint);
            break;

        default:
            __debugbreak();
            Sqlite3->ResultNull(Context);
            return SQLITE_ERROR;
    }

    return SQLITE_OK;
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

    PLACEHOLDER_SCHEMA, // TraceStore_WsWorkingSetEx,
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

    TraceStoreModuleLoadEventSchema, // TraceStore_ModuleLoadEvent,
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

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolModuleInfo,
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
C_ASSERT(ARRAYSIZE(TraceStoreSchemas) == MAX_TRACE_STORES);

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

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_StringBuffer,
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

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_PageFault,
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

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_WsWatchInfoEx,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_WsWorkingSetExInfo,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_CCallStackTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_CCallStackTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ModuleTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ModuleTableEntry,
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

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_LineTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_LineTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_LineStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_CallStack,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_Performance,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_PerformanceDelta,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SourceCode,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_Bitmap,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ImageFile,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_UnicodeStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_Line,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_Object,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3ModuleLoadEventColumn, // TraceStore_ModuleLoadEvent,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolModuleInfo
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolFile,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolInfo
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_SymbolType,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_StackFrame,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_TypeInfoTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_TypeInfoTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_TypeInfoStringBuffer,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_FunctionTable,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_FunctionTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_FunctionAssembly,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_FunctionSourceCode,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ExamineSymbolsLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ExamineSymbolsText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ExaminedSymbol,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_ExaminedSymbolSecondary,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_UnassembleFunctionLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_UnassembleFunctionText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_UnassembledFunction,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_UnassembledFunctionSecondary,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_DisplayTypeLine,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_DisplayTypeText,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_DisplayedType,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3DefaultColumnImpl, // TraceStore_DisplayedTypeSecondary,
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
//C_ASSERT((sizeof(TraceStoreSqlite3Columns) / sizeof(ULONG_PTR)) == MAX_TRACE_STORES);
C_ASSERT(ARRAYSIZE(TraceStoreSqlite3Columns) == MAX_TRACE_STORES);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
