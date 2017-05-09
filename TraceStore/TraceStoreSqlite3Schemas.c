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

#define RESULT_NULL() Sqlite3->ResultNull(Context);

#define RESULT_WCHAR(WideChar) \
    Sqlite3->ResultText16LE(   \
        Context,               \
        &WideChar,             \
        sizeof(WCHAR),         \
        SQLITE_STATIC          \
    )

#define RESULT_CHAR(Char)  \
    Sqlite3->ResultText(   \
        Context,           \
        &Char,             \
        sizeof(CHAR),      \
        SQLITE_STATIC      \
    )

#define RESULT_PWCHAR(WideChar) \
    Sqlite3->ResultText16LE(    \
        Context,                \
        WideChar,               \
        0,                      \
        SQLITE_STATIC           \
    )

#define RESULT_PCHAR(Char) \
    Sqlite3->ResultText(   \
        Context,           \
        Char,              \
        0,                 \
        SQLITE_STATIC      \
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

#define _INVALID_COLUMN()          \
    __debugbreak();                \
    Sqlite3->ResultNull(Context);  \
    break

//
// Allow functions to undefine INVALID_COLUMN, then set it back to
// _INVALID_COLUMN when they're done.
//

#define INVALID_COLUMN() _INVALID_COLUMN()

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
        "NumberOfRecords BIGINT, "   // LARGE_INTEGER
        "RecordSize BIGINT, "        // LARGE_INTEGER
        "IsDummyAllocation BIGINT"  // Allocation->NumberOfRecords.DummyAllocation2
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

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: NumberOfRecords BIGINT
        //

        case 0:
            RESULT_LARGE_INTEGER(Allocation->NumberOfRecords);
            break;

        //
        // 1: RecordSize BIGINT
        //

        case 1:
            RESULT_LARGE_INTEGER(Allocation->RecordSize);
            break;

        //
        // 2: IsDummyAllocation BIGINT
        //

        case 2:
            RESULT_ULONGLONG(Allocation->NumberOfRecords.DummyAllocation2);
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
        "ImageFile_HeaderSum INT, "             // ImageFile->HeaderSum, [(File->Type == RtlFileImageFileType && ImageFile != NULL)]
        "ImageFile_CheckSum INT, "              // ImageFile->CheckSum, [(File->Type == RtlFileImageFileType && ImageFile != NULL)]
        "ImageFile_Timestamp INT, "             // ImageFile->Timestamp, [(File->Type == RtlFileImageFileType && ImageFile != NULL)]
        "ModuleInfo BIGINT, "                   // ImageFile->ModuleInfo, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_SizeOfStruct INT, "         // ModuleInfo->SizeOfStruct, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_BaseOfImage BIGINT, "       // ModuleInfo->BaseOfImage, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_ImageSize INT, "            // ModuleInfo->ImageSize, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_TimeDateStamp INT, "        // ModuleInfo->TimeDateStamp, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_CheckSum INT, "             // ModuleInfo->CheckSum, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_SymType INT, "              // ModuleInfo->SymType, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_NumSyms INT, "              // ModuleInfo->NumSyms, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_ModuleName TEXT, "          // ModuleInfo->ModuleName, PWCHAR, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_ImageName TEXT, "           // ModuleInfo->ImageName, PWCHAR, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_LoadedImageName TEXT, "     // ModuleInfo->LoadedImageName, PWCHAR, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_LoadedPdbName TEXT, "       // ModuleInfo->LoadedPdbName, PWCHAR, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_CVSig INT, "                // ModuleInfo->CVSig, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_CVData TEXT, "              // ModuleInfo->CVData, PWCHAR, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_PdbSig INT, "               // ModuleInfo->PdbSig, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_PdbSig70 BLOB, "            // ModuleInfo->PdbSig70, sizeof(), [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_PdbAge INT, "               // ModuleInfo->PdbAge, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_PdbUnmatched TINYINT, "     // ModuleInfo->PdbUnmatched, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_DbgUnmatched TINYINT, "     // ModuleInfo->DbgUnmatched, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_LineNumbers TINYINT, "      // ModuleInfo->LineNumbers, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_GlobalSymbols TINYINT, "    // ModuleInfo->GlobalSymbols, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_TypeInfo TINYINT, "         // ModuleInfo->TypeInfo, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_SourceIndexed TINYINT, "    // ModuleInfo->SourceIndexed, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_Publics TINYINT, "          // ModuleInfo->Publics, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_MachineType INT, "          // ModuleInfo->MachineType, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
        "ModuleInfo_Reserved INT, "             // ModuleInfo->Reserved, [(File->Type == RtlFileImageFileType && ModuleInfo != NULL)]
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
    PIMAGEHLP_MODULEW64 ModuleInfo;

    LoadEvent = (PTRACE_MODULE_LOAD_EVENT)Cursor->CurrentRowRaw;
    ModuleTableEntry = LoadEvent->ModuleTableEntry;
    File = &ModuleTableEntry->File;
    ImageFile = &File->ImageFile;
    ModuleInfo = ImageFile->ModuleInfo;
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
        // 36: ImageFile_HeaderSum INT
        //

        case 36:
            if (!((File->Type == RtlFileImageFileType && ImageFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ImageFile->HeaderSum);
            }
            break;

        //
        // 37: ImageFile_CheckSum INT
        //

        case 37:
            if (!((File->Type == RtlFileImageFileType && ImageFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ImageFile->CheckSum);
            }
            break;

        //
        // 38: ImageFile_Timestamp INT
        //

        case 38:
            if (!((File->Type == RtlFileImageFileType && ImageFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ImageFile->Timestamp);
            }
            break;

        //
        // 39: ModuleInfo BIGINT
        //

        case 39:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(ImageFile->ModuleInfo);
            }
            break;

        //
        // 40: ModuleInfo_SizeOfStruct INT
        //

        case 40:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->SizeOfStruct);
            }
            break;

        //
        // 41: ModuleInfo_BaseOfImage BIGINT
        //

        case 41:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(ModuleInfo->BaseOfImage);
            }
            break;

        //
        // 42: ModuleInfo_ImageSize INT
        //

        case 42:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->ImageSize);
            }
            break;

        //
        // 43: ModuleInfo_TimeDateStamp INT
        //

        case 43:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->TimeDateStamp);
            }
            break;

        //
        // 44: ModuleInfo_CheckSum INT
        //

        case 44:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->CheckSum);
            }
            break;

        //
        // 45: ModuleInfo_SymType INT
        //

        case 45:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->SymType);
            }
            break;

        //
        // 46: ModuleInfo_NumSyms INT
        //

        case 46:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->NumSyms);
            }
            break;

        //
        // 47: ModuleInfo_ModuleName TEXT
        //

        case 47:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_PWCHAR(ModuleInfo->ModuleName);
            }
            break;

        //
        // 48: ModuleInfo_ImageName TEXT
        //

        case 48:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_PWCHAR(ModuleInfo->ImageName);
            }
            break;

        //
        // 49: ModuleInfo_LoadedImageName TEXT
        //

        case 49:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_PWCHAR(ModuleInfo->LoadedImageName);
            }
            break;

        //
        // 50: ModuleInfo_LoadedPdbName TEXT
        //

        case 50:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_PWCHAR(ModuleInfo->LoadedPdbName);
            }
            break;

        //
        // 51: ModuleInfo_CVSig INT
        //

        case 51:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->CVSig);
            }
            break;

        //
        // 52: ModuleInfo_CVData TEXT
        //

        case 52:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_PWCHAR(ModuleInfo->CVData);
            }
            break;

        //
        // 53: ModuleInfo_PdbSig INT
        //

        case 53:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->PdbSig);
            }
            break;

        //
        // 54: ModuleInfo_PdbSig70 BLOB
        //

        case 54:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_BLOB(ModuleInfo->PdbSig70, sizeof(ModuleInfo->PdbSig70));
            }
            break;

        //
        // 55: ModuleInfo_PdbAge INT
        //

        case 55:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->PdbAge);
            }
            break;

        //
        // 56: ModuleInfo_PdbUnmatched TINYINT
        //

        case 56:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->PdbUnmatched);
            }
            break;

        //
        // 57: ModuleInfo_DbgUnmatched TINYINT
        //

        case 57:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->DbgUnmatched);
            }
            break;

        //
        // 58: ModuleInfo_LineNumbers TINYINT
        //

        case 58:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->LineNumbers);
            }
            break;

        //
        // 59: ModuleInfo_GlobalSymbols TINYINT
        //

        case 59:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->GlobalSymbols);
            }
            break;

        //
        // 60: ModuleInfo_TypeInfo TINYINT
        //

        case 60:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->TypeInfo);
            }
            break;

        //
        // 61: ModuleInfo_SourceIndexed TINYINT
        //

        case 61:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->SourceIndexed);
            }
            break;

        //
        // 62: ModuleInfo_Publics TINYINT
        //

        case 62:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->Publics);
            }
            break;

        //
        // 63: ModuleInfo_MachineType INT
        //

        case 63:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->MachineType);
            }
            break;

        //
        // 64: ModuleInfo_Reserved INT
        //

        case 64:
            if (!((File->Type == RtlFileImageFileType && ModuleInfo != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(ModuleInfo->Reserved);
            }
            break;

        //
        // 65: Loaded BIGINT
        //

        case 65:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Loaded);
            break;

        //
        // 66: Unloaded BIGINT
        //

        case 66:
            RESULT_LARGE_INTEGER(LoadEvent->Timestamp.Unloaded);
            break;

        //
        // 67: PreferredBaseAddress BIGINT
        //

        case 67:
            RESULT_ULONGLONG(LoadEvent->PreferredBaseAddress);
            break;

        //
        // 68: BaseAddress BIGINT
        //

        case 68:
            RESULT_ULONGLONG(LoadEvent->BaseAddress);
            break;

        //
        // 69: EntryPoint BIGINT
        //

        case 69:
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
        "Path TEXT, "                       // Path->Full, UNICODE_STRING
        "FullName TEXT, "                   // PathTableEntry->FullName, STRING
        "ModuleName TEXT, "                 // PathTableEntry->ModuleName, STRING
        "Name TEXT, "                       // PathTableEntry->Name, STRING
        "ClassName TEXT, "                  // PathTableEntry->ClassName, STRING
        "CodeObject BIGINT, "               // Function->CodeObject
        "PyCFunctionObject BIGINT, "        // Function->PyCFunctionObject
        "MaxCallStackDepth INTEGER, "       // Function->MaxCallStackDepth
        "CallCount INTEGER, "               // Function->CallCount
        "CodeLineNumbers BIGINT, "          // Function->CodeLineNumbers, [Function->CodeObject != NULL]
        "CodeObjectHash INT, "              // Function->CodeObjectHash, [Function->CodeObject != NULL]
        "FirstLineNumber SMALLINT, "        // Function->FirstLineNumber, [Function->CodeObject != NULL]
        "NumberOfLines SMALLINT, "          // Function->NumberOfLines, [Function->CodeObject != NULL]
        "NumberOfCodeLines SMALLINT, "      // Function->NumberOfCodeLines, [Function->CodeObject != NULL]
        "SizeOfByteCodeInBytes SMALLINT, "  // Function->SizeOfByteCodeInBytes, [Function->CodeObject != NULL]
        "Signature BIGINT, "                // Function->Signature
        "IsModuleDirectory TINYINT, "       // PathTableEntry->IsModuleDirectory
        "IsNonModuleDirectory TINYINT, "    // PathTableEntry->IsNonModuleDirectory
        "IsFileSystemDirectory TINYINT, "   // PathTableEntry->IsFileSystemDirectory
        "IsFile TINYINT, "                  // PathTableEntry->IsFile
        "IsClass TINYINT, "                 // PathTableEntry->IsClass
        "IsFunction TINYINT, "              // PathTableEntry->IsFunction
        "IsSpecial TINYINT, "               // PathTableEntry->IsSpecial
        "IsValid TINYINT, "                 // PathTableEntry->IsValid
        "IsDll TINYINT, "                   // PathTableEntry->IsDll
        "IsC TINYINT, "                     // PathTableEntry->IsC
        "IsBuiltin TINYINT, "               // PathTableEntry->IsBuiltin
        "IsInitPy TINYINT,"                 // PathTableEntry->IsInitPy
        "TextFile_StructSizeInBytes INT, "                  // TextFile->StructSizeInBytes, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_NumberOfLines INT, "                      // TextFile->NumberOfLines, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_LinesAddress BIGINT, "                    // TextFile->Lines, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_CarriageReturnBitmapAddress BIGINT, "     // TextFile->CarriageReturnBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_LineFeedBitmapAddress BIGINT, "           // TextFile->LineFeedBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_LineEndingBitmapAddress BIGINT, "         // TextFile->LineEndingBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_LineBitmapAddress BIGINT, "               // TextFile->LineBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_WhitespaceBitmapAddress BIGINT, "         // TextFile->WhitespaceBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_TabBitmapAddress BIGINT, "                // TextFile->TabBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_IndentBitmapAddress BIGINT, "             // TextFile->IndentBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
        "TextFile_TrailingWhitespaceBitmapAddress BIGINT"   // TextFile->TrailingWhitespaceBitmap, [(File->Type == RtlFileTextFileType && TextFile != NULL)]
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
    PPYTHON_PATH_TABLE_ENTRY PathTableEntry;
    PPYTHON_FUNCTION_TABLE_ENTRY FunctionTableEntry;
    PPYTHON_FUNCTION Function;
    PRTL_FILE File;
    PRTL_PATH Path;
    PRTL_TEXT_FILE TextFile;

    FunctionTableEntry = (PPYTHON_FUNCTION_TABLE_ENTRY)Cursor->CurrentRowRaw;
    Function = (PPYTHON_FUNCTION)(
        CONTAINING_RECORD(
            FunctionTableEntry,
            PYTHON_FUNCTION_TABLE_ENTRY,
            PythonFunction
        )
    );
    PathTableEntry = &Function->PathEntry;
    File = &PathTableEntry->File;
    TextFile = &File->SourceCode;
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
        // 1: FullName TEXT
        //

        case 1:
            RESULT_STRING(PathTableEntry->FullName);
            break;

        //
        // 2: ModuleName TEXT
        //

        case 2:
            RESULT_STRING(PathTableEntry->ModuleName);
            break;

        //
        // 3: Name TEXT
        //

        case 3:
            RESULT_STRING(PathTableEntry->Name);
            break;

        //
        // 4: ClassName TEXT
        //

        case 4:
            RESULT_STRING(PathTableEntry->ClassName);
            break;

        //
        // 5: CodeObject BIGINT
        //

        case 5:
            RESULT_ULONGLONG(Function->CodeObject);
            break;

        //
        // 6: PyCFunctionObject BIGINT
        //

        case 6:
            RESULT_ULONGLONG(Function->PyCFunctionObject);
            break;

        //
        // 7: MaxCallStackDepth INTEGER
        //

        case 7:
            RESULT_ULONG(Function->MaxCallStackDepth);
            break;

        //
        // 8: CallCount INTEGER
        //

        case 8:
            RESULT_ULONG(Function->CallCount);
            break;

        //
        // 9: CodeLineNumbers BIGINT
        //

        case 9:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Function->CodeLineNumbers);
            }
            break;

        //
        // 10: CodeObjectHash INT
        //

        case 10:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->CodeObjectHash);
            }
            break;

        //
        // 11: FirstLineNumber SMALLINT
        //

        case 11:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->FirstLineNumber);
            }
            break;

        //
        // 12: NumberOfLines SMALLINT
        //

        case 12:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->NumberOfLines);
            }
            break;

        //
        // 13: NumberOfCodeLines SMALLINT
        //

        case 13:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->NumberOfCodeLines);
            }
            break;

        //
        // 14: SizeOfByteCodeInBytes SMALLINT
        //

        case 14:
            if (!(Function->CodeObject != NULL)) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->SizeOfByteCodeInBytes);
            }
            break;

        //
        // 15: Signature BIGINT
        //

        case 15:
            RESULT_ULONGLONG(Function->Signature);
            break;

        //
        // 16: IsModuleDirectory TINYINT
        //

        case 16:
            RESULT_ULONG(PathTableEntry->IsModuleDirectory);
            break;

        //
        // 17: IsNonModuleDirectory TINYINT
        //

        case 17:
            RESULT_ULONG(PathTableEntry->IsNonModuleDirectory);
            break;

        //
        // 18: IsFileSystemDirectory TINYINT
        //

        case 18:
            RESULT_ULONG(PathTableEntry->IsFileSystemDirectory);
            break;

        //
        // 19: IsFile TINYINT
        //

        case 19:
            RESULT_ULONG(PathTableEntry->IsFile);
            break;

        //
        // 20: IsClass TINYINT
        //

        case 20:
            RESULT_ULONG(PathTableEntry->IsClass);
            break;

        //
        // 21: IsFunction TINYINT
        //

        case 21:
            RESULT_ULONG(PathTableEntry->IsFunction);
            break;

        //
        // 22: IsSpecial TINYINT
        //

        case 22:
            RESULT_ULONG(PathTableEntry->IsSpecial);
            break;

        //
        // 23: IsValid TINYINT
        //

        case 23:
            RESULT_ULONG(PathTableEntry->IsValid);
            break;

        //
        // 24: IsDll TINYINT
        //

        case 24:
            RESULT_ULONG(PathTableEntry->IsDll);
            break;

        //
        // 25: IsC TINYINT
        //

        case 25:
            RESULT_ULONG(PathTableEntry->IsC);
            break;

        //
        // 26: IsBuiltin TINYINT
        //

        case 26:
            RESULT_ULONG(PathTableEntry->IsBuiltin);
            break;

        //
        // 27: IsInitPy TINYINT,
        //

        case 27:
            RESULT_ULONG(PathTableEntry->IsInitPy);
            break;

        //
        // 28: TextFile_StructSizeInBytes INT
        //

        case 28:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(TextFile->StructSizeInBytes);
            }
            break;

        //
        // 29: TextFile_NumberOfLines INT
        //

        case 29:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(TextFile->NumberOfLines);
            }
            break;

        //
        // 30: TextFile_LinesAddress BIGINT
        //

        case 30:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->Lines);
            }
            break;

        //
        // 31: TextFile_CarriageReturnBitmapAddress BIGINT
        //

        case 31:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->CarriageReturnBitmap);
            }
            break;

        //
        // 32: TextFile_LineFeedBitmapAddress BIGINT
        //

        case 32:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineFeedBitmap);
            }
            break;

        //
        // 33: TextFile_LineEndingBitmapAddress BIGINT
        //

        case 33:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineEndingBitmap);
            }
            break;

        //
        // 34: TextFile_LineBitmapAddress BIGINT
        //

        case 34:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineBitmap);
            }
            break;

        //
        // 35: TextFile_WhitespaceBitmapAddress BIGINT
        //

        case 35:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->WhitespaceBitmap);
            }
            break;

        //
        // 36: TextFile_TabBitmapAddress BIGINT
        //

        case 36:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->TabBitmap);
            }
            break;

        //
        // 37: TextFile_IndentBitmapAddress BIGINT
        //

        case 37:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->IndentBitmap);
            }
            break;

        //
        // 38: TextFile_TrailingWhitespaceBitmapAddress BIGINT
        //

        case 38:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->TrailingWhitespaceBitmap);
            }
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
// N.B. The next two arrays are much larger than they need to be; the metadata
//      stores are consistent and never need to vary on a per trace store basis.
//      Fix by reducing the arrays to just the trace stores and alter the module
//      initializer to handle metadata stores accordingly.
//

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
C_ASSERT(ARRAYSIZE(TraceStoreSqlite3Columns) == MAX_TRACE_STORES);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
