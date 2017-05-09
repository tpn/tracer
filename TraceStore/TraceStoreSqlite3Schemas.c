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
        "FileOffset BIGINT, "               // LARGE_INTEGER
        "MappedSize BIGINT, "               // LARGE_INTEGER
        "ProcessId INT, "
        "RequestingThreadId INT, "
        "Requested BIGINT, "                // Address->Timestamp.Requested.QuadPart
        "Prepared BIGINT, "                 // Address->Timestamp.Prepared.QuadPart
        "Consumed BIGINT, "                 // Address->Timestamp.Consumed.QuadPart
        "Retired BIGINT, "                  // Address->Timestamp.Retired.QuadPart
        "Released BIGINT, "                 // Address->Timestamp.Released.QuadPart
        "AwaitingPreparation BIGINT, "      // Address->Elapsed.AwaitingPreparation.QuadPart
        "AwaitingConsumption BIGINT, "      // Address->Elapsed.AwaitingConsumption.QuadPart
        "Active BIGINT, "                   // Address->Elapsed.Active.QuadPart
        "AwaitingRelease BIGINT, "          // Address->Elapsed.AwaitingRelease.QuadPart
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

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: PreferredBaseAddress BIGINT
        //

        case 0:
            RESULT_ULONGLONG(Address->PreferredBaseAddress);
            break;

        //
        // 1: BaseAddress BIGINT
        //

        case 1:
            RESULT_ULONGLONG(Address->BaseAddress);
            break;

        //
        // 2: FileOffset BIGINT
        //

        case 2:
            RESULT_LARGE_INTEGER(Address->FileOffset);
            break;

        //
        // 3: MappedSize BIGINT
        //

        case 3:
            RESULT_LARGE_INTEGER(Address->MappedSize);
            break;

        //
        // 4: ProcessId INT
        //

        case 4:
            RESULT_ULONG(Address->ProcessId);
            break;

        //
        // 5: RequestingThreadId INT
        //

        case 5:
            RESULT_ULONG(Address->RequestingThreadId);
            break;

        //
        // 6: Requested BIGINT
        //

        case 6:
            RESULT_ULONGLONG(Address->Timestamp.Requested.QuadPart);
            break;

        //
        // 7: Prepared BIGINT
        //

        case 7:
            RESULT_ULONGLONG(Address->Timestamp.Prepared.QuadPart);
            break;

        //
        // 8: Consumed BIGINT
        //

        case 8:
            RESULT_ULONGLONG(Address->Timestamp.Consumed.QuadPart);
            break;

        //
        // 9: Retired BIGINT
        //

        case 9:
            RESULT_ULONGLONG(Address->Timestamp.Retired.QuadPart);
            break;

        //
        // 10: Released BIGINT
        //

        case 10:
            RESULT_ULONGLONG(Address->Timestamp.Released.QuadPart);
            break;

        //
        // 11: AwaitingPreparation BIGINT
        //

        case 11:
            RESULT_ULONGLONG(Address->Elapsed.AwaitingPreparation.QuadPart);
            break;

        //
        // 12: AwaitingConsumption BIGINT
        //

        case 12:
            RESULT_ULONGLONG(Address->Elapsed.AwaitingConsumption.QuadPart);
            break;

        //
        // 13: Active BIGINT
        //

        case 13:
            RESULT_ULONGLONG(Address->Elapsed.Active.QuadPart);
            break;

        //
        // 14: AwaitingRelease BIGINT
        //

        case 14:
            RESULT_ULONGLONG(Address->Elapsed.AwaitingRelease.QuadPart);
            break;

        //
        // 15: MappedSequenceId INT
        //

        case 15:
            RESULT_ULONG(Address->MappedSequenceId);
            break;

        //
        // 16: RequestingProcGroup SMALLINT
        //

        case 16:
            RESULT_ULONG(Address->RequestingProcGroup);
            break;

        //
        // 17: RequestingNumaNode TINYINT
        //

        case 17:
            RESULT_ULONG(Address->RequestingNumaNode);
            break;

        //
        // 18: FulfillingThreadId INT
        //

        case 18:
            RESULT_ULONG(Address->FulfillingThreadId);
            break;

        default:
           INVALID_COLUMN();

        //
        // End auto-generated section.
        //

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
        "Drive TEXT, "                          // Path->Drive, WCHAR
        "Name TEXT, "                           // Path->Name, UNICODE_STRING
        "Directory TEXT, "                      // Path->Directory, UNICODE_STRING
        "Extension TEXT, "                      // Path->Extension, UNICODE_STRING
        "NumberOfSlashes SMALLINT, "            // Path->NumberOfSlashes
        "NumberOfDots SMALLINT, "               // Path->NumberOfDots
        "AllocSize SMALLINT, "                  // Path->AllocSize
        "IsFile TINYINT, "                      // Path->Flags.IsFile
        "IsDirectory TINYINT, "                 // Path->Flags.IsDirectory
        "IsSymlink TINYINT, "                   // Path->Flags.IsSymlink
        "IsFullyQualified TINYINT, "            // Path->Flags.IsFullyQualified
        "HasParent TINYINT, "                   // Path->Flags.HasParent
        "HasChildren TINYINT, "                 // Path->Flags.HasChildren
        "WithinWindowsDirectory TINYINT, "      // Path->Flags.WithinWindowsDirectory
        "WithinWindowsSxSDirectory TINYINT, "   // Path->Flags.WithinWindowsSxSDirectory
        "WithinWindowsSystemDirectory TINYINT, "// Path->Flags.WithinWindowsSystemDirectory
        "PathFlagsAsLong INT, "                 // Path->Flags.AsLong
        "CreationTime BIGINT, "                 // File->CreationTime, LARGE_INTEGER
        "LastAccessTime BIGINT, "               // File->LastAccessTime, LARGE_INTEGER
        "LastWriteTime BIGINT, "                // File->LastWriteTime, LARGE_INTEGER
        "ChangeTime BIGINT, "                   // File->ChangeTime, LARGE_INTEGER
        "FileAttributes INT, "                  // File->FileAttributes
        "NumberOfPages INT, "                   // File->NumberOfPages
        "EndOfFile BIGINT, "                    // File->EndOfFile, LARGE_INTEGER
        "AllocationSize BIGINT, "               // File->AllocationSize, LARGE_INTEGER
        "FileId BIGINT, "                       // File->FileId, LARGE_INTEGER
        "VolumeSerialNumber BIGINT, "           // File->VolumeSerialNumber
        "FileId128 BLOB, "                      // File->FileId128, sizeof()
        "MD5 BLOB, "                            // File->MD5, sizeof()
        "SHA1 BLOB, "                           // File->SHA1, sizeof()
        "CopyTimeInMicroseconds BIGINT, "       // File->CopyTimeInMicroseconds, LARGE_INTEGER
        "CopiedBytesPerSecond BIGINT, "         // File->CopiedBytesPerSecond, LARGE_INTEGER
        "FileType INT, "                        // File->Type
        "FileFlags INT, "                       // File->Flags.AsULong
        "ContentAddress BIGINT, "               // File->Content
        "HeaderSum INT, "                       // ImageFile->HeaderSum
        "CheckSum INT, "                        // ImageFile->CheckSum
        "Timestamp INT, "                       // ImageFile->Timestamp
        "ModuleInfo BIGINT, "                   // ImageFile->ModuleInfo
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
    PRTL_IMAGE_FILE ImageFile;

    LoadEvent = (PTRACE_MODULE_LOAD_EVENT)Cursor->CurrentRowRaw;
    ModuleTableEntry = LoadEvent->ModuleTableEntry;
    File = &ModuleTableEntry->File;
    ImageFile = &File->ImageFile;
    Path = &File->Path;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: Path TEXT
        //

        case 0:
            RESULT_UNICODE_STRING(Path->Full);
            break;

        //
        // 1: Drive TEXT
        //

        case 1:
            RESULT_WCHAR(Path->Drive);
            break;

        //
        // 2: Name TEXT
        //

        case 2:
            RESULT_UNICODE_STRING(Path->Name);
            break;

        //
        // 3: Directory TEXT
        //

        case 3:
            RESULT_UNICODE_STRING(Path->Directory);
            break;

        //
        // 4: Extension TEXT
        //

        case 4:
            RESULT_UNICODE_STRING(Path->Extension);
            break;

        //
        // 5: NumberOfSlashes SMALLINT
        //

        case 5:
            RESULT_ULONG(Path->NumberOfSlashes);
            break;

        //
        // 6: NumberOfDots SMALLINT
        //

        case 6:
            RESULT_ULONG(Path->NumberOfDots);
            break;

        //
        // 7: AllocSize SMALLINT
        //

        case 7:
            RESULT_ULONG(Path->AllocSize);
            break;

        //
        // 8: IsFile TINYINT
        //

        case 8:
            RESULT_ULONG(Path->Flags.IsFile);
            break;

        //
        // 9: IsDirectory TINYINT
        //

        case 9:
            RESULT_ULONG(Path->Flags.IsDirectory);
            break;

        //
        // 10: IsSymlink TINYINT
        //

        case 10:
            RESULT_ULONG(Path->Flags.IsSymlink);
            break;

        //
        // 11: IsFullyQualified TINYINT
        //

        case 11:
            RESULT_ULONG(Path->Flags.IsFullyQualified);
            break;

        //
        // 12: HasParent TINYINT
        //

        case 12:
            RESULT_ULONG(Path->Flags.HasParent);
            break;

        //
        // 13: HasChildren TINYINT
        //

        case 13:
            RESULT_ULONG(Path->Flags.HasChildren);
            break;

        //
        // 14: WithinWindowsDirectory TINYINT
        //

        case 14:
            RESULT_ULONG(Path->Flags.WithinWindowsDirectory);
            break;

        //
        // 15: WithinWindowsSxSDirectory TINYINT
        //

        case 15:
            RESULT_ULONG(Path->Flags.WithinWindowsSxSDirectory);
            break;

        //
        // 16: WithinWindowsSystemDirectory TINYINT
        //

        case 16:
            RESULT_ULONG(Path->Flags.WithinWindowsSystemDirectory);
            break;

        //
        // 17: PathFlagsAsLong INT
        //

        case 17:
            RESULT_ULONG(Path->Flags.AsLong);
            break;

        //
        // 18: CreationTime BIGINT
        //

        case 18:
            RESULT_LARGE_INTEGER(File->CreationTime);
            break;

        //
        // 19: LastAccessTime BIGINT
        //

        case 19:
            RESULT_LARGE_INTEGER(File->LastAccessTime);
            break;

        //
        // 20: LastWriteTime BIGINT
        //

        case 20:
            RESULT_LARGE_INTEGER(File->LastWriteTime);
            break;

        //
        // 21: ChangeTime BIGINT
        //

        case 21:
            RESULT_LARGE_INTEGER(File->ChangeTime);
            break;

        //
        // 22: FileAttributes INT
        //

        case 22:
            RESULT_ULONG(File->FileAttributes);
            break;

        //
        // 23: NumberOfPages INT
        //

        case 23:
            RESULT_ULONG(File->NumberOfPages);
            break;

        //
        // 24: EndOfFile BIGINT
        //

        case 24:
            RESULT_LARGE_INTEGER(File->EndOfFile);
            break;

        //
        // 25: AllocationSize BIGINT
        //

        case 25:
            RESULT_LARGE_INTEGER(File->AllocationSize);
            break;

        //
        // 26: FileId BIGINT
        //

        case 26:
            RESULT_LARGE_INTEGER(File->FileId);
            break;

        //
        // 27: VolumeSerialNumber BIGINT
        //

        case 27:
            RESULT_ULONGLONG(File->VolumeSerialNumber);
            break;

        //
        // 28: FileId128 BLOB
        //

        case 28:
            RESULT_BLOB(File->FileId128, sizeof(File->FileId128));
            break;

        //
        // 29: MD5 BLOB
        //

        case 29:
            RESULT_BLOB(File->MD5, sizeof(File->MD5));
            break;

        //
        // 30: SHA1 BLOB
        //

        case 30:
            RESULT_BLOB(File->SHA1, sizeof(File->SHA1));
            break;

        //
        // 31: CopyTimeInMicroseconds BIGINT
        //

        case 31:
            RESULT_LARGE_INTEGER(File->CopyTimeInMicroseconds);
            break;

        //
        // 32: CopiedBytesPerSecond BIGINT
        //

        case 32:
            RESULT_LARGE_INTEGER(File->CopiedBytesPerSecond);
            break;

        //
        // 33: FileType INT
        //

        case 33:
            RESULT_ULONG(File->Type);
            break;

        //
        // 34: FileFlags INT
        //

        case 34:
            RESULT_ULONG(File->Flags.AsULong);
            break;

        //
        // 35: ContentAddress BIGINT
        //

        case 35:
            RESULT_ULONGLONG(File->Content);
            break;

        //
        // 36: HeaderSum INT
        //

        case 36:
            RESULT_ULONG(ImageFile->HeaderSum);
            break;

        //
        // 37: CheckSum INT
        //

        case 37:
            RESULT_ULONG(ImageFile->CheckSum);
            break;

        //
        // 38: Timestamp INT
        //

        case 38:
            RESULT_ULONG(ImageFile->Timestamp);
            break;

        //
        // 39: ModuleInfo BIGINT
        //

        case 39:
            RESULT_ULONGLONG(ImageFile->ModuleInfo);
            break;

        //
        // 40: Loaded BIGINT
        //

        case 40:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Loaded);
            break;

        //
        // 41: Unloaded BIGINT
        //

        case 41:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Unloaded);
            break;

        //
        // 42: PreferredBaseAddress BIGINT
        //

        case 42:
            RESULT_ULONGLONG(LoadEvent->PreferredBaseAddress);
            break;

        //
        // 43: BaseAddress BIGINT
        //

        case 43:
            RESULT_ULONGLONG(LoadEvent->BaseAddress);
            break;

        //
        // 44: EntryPoint BIGINT
        //

        case 44:
            RESULT_ULONGLONG(LoadEvent->EntryPoint);
            break;

        default:
           INVALID_COLUMN();

        //
        // End auto-generated section.
        //

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

    PythonFunctionTableEntrySchema, // Python_PythonFunctionTableEntry,
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

    TraceStoreSqlite3PythonFunctionTableEntryColumn, // Python_PythonFunctionTableEntry,
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
