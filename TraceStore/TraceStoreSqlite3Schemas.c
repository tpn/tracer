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
        -1,                     \
        SQLITE_STATIC           \
    )

#define RESULT_PCHAR(Char) \
    Sqlite3->ResultText(   \
        Context,           \
        Char,              \
        -1,                \
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
    Sqlite3->ResultText(       \
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

#define RESULT_FILETIME(FileTime)                  \
    Sqlite3->ResultInt64(                          \
        Context,                                   \
        *((PSQLITE3_INT64)(PULONGLONG)(&FileTime)) \
    )

#define RESULT_SYSTEMTIME(SystemTime)                \
    Sqlite3->ResultInt64(                            \
        Context,                                     \
        *((PSQLITE3_INT64)(PULONGLONG)(&SystemTime)) \
    )

#define RESULT_LARGE_INTEGER(LargeInteger)     \
    Sqlite3->ResultInt64(                      \
        Context,                               \
        (SQLITE3_INT64)LargeInteger##.QuadPart \
    )

#define RESULT_ULONG(ULong) Sqlite3->ResultInt(Context, ULong)
#define RESULT_ULONGLONG(ULongLong) \
    Sqlite3->ResultInt64(Context, (SQLITE3_INT64)ULongLong)

#define RESULT_DOUBLE(Double) Sqlite3->ResultDouble(Context, (DOUBLE)Double)

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
    "CREATE TABLE MetadataInfo ("
        "MetadataInfo_MetadataInfo_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_MetadataInfo_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_MetadataInfo_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_MetadataInfo_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_MetadataInfo_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Stats_DroppedRecords INT, "
        "MetadataInfo_MetadataInfo_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_MetadataInfo_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_MetadataInfo_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_MetadataInfo_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_MetadataInfo_Stats_BlockedAllocations INT, "
        "MetadataInfo_MetadataInfo_Stats_SuspendedAllocations INT, "
        "MetadataInfo_MetadataInfo_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_MetadataInfo_Stats_WastedBytes BIGINT, "
        "MetadataInfo_MetadataInfo_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_MetadataInfo_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_MetadataInfo_Traits_VaryingRecordSize INT, "
        "MetadataInfo_MetadataInfo_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_MetadataInfo_Traits_MultipleRecords INT, "
        "MetadataInfo_MetadataInfo_Traits_StreamingWrite INT, "
        "MetadataInfo_MetadataInfo_Traits_StreamingRead INT, "
        "MetadataInfo_MetadataInfo_Traits_FrequentAllocations INT, "
        "MetadataInfo_MetadataInfo_Traits_BlockingAllocations INT, "
        "MetadataInfo_MetadataInfo_Traits_LinkedStore INT, "
        "MetadataInfo_MetadataInfo_Traits_CoalesceAllocations INT, "
        "MetadataInfo_MetadataInfo_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_MetadataInfo_Traits_AllowPageSpill INT, "
        "MetadataInfo_MetadataInfo_Traits_PageAligned INT, "
        "MetadataInfo_MetadataInfo_Traits_Periodic INT, "
        "MetadataInfo_MetadataInfo_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_MetadataInfo_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_MetadataInfo_Traits_Unused INT, "
        "MetadataInfo_Allocation_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_Allocation_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_Allocation_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Allocation_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_Allocation_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_Allocation_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_Allocation_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_Allocation_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Allocation_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_Allocation_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_Allocation_Stats_DroppedRecords INT, "
        "MetadataInfo_Allocation_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_Allocation_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_Allocation_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_Allocation_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_Allocation_Stats_BlockedAllocations INT, "
        "MetadataInfo_Allocation_Stats_SuspendedAllocations INT, "
        "MetadataInfo_Allocation_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_Allocation_Stats_WastedBytes BIGINT, "
        "MetadataInfo_Allocation_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_Allocation_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Allocation_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_Allocation_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_Allocation_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Allocation_Traits_VaryingRecordSize INT, "
        "MetadataInfo_Allocation_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_Allocation_Traits_MultipleRecords INT, "
        "MetadataInfo_Allocation_Traits_StreamingWrite INT, "
        "MetadataInfo_Allocation_Traits_StreamingRead INT, "
        "MetadataInfo_Allocation_Traits_FrequentAllocations INT, "
        "MetadataInfo_Allocation_Traits_BlockingAllocations INT, "
        "MetadataInfo_Allocation_Traits_LinkedStore INT, "
        "MetadataInfo_Allocation_Traits_CoalesceAllocations INT, "
        "MetadataInfo_Allocation_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_Allocation_Traits_AllowPageSpill INT, "
        "MetadataInfo_Allocation_Traits_PageAligned INT, "
        "MetadataInfo_Allocation_Traits_Periodic INT, "
        "MetadataInfo_Allocation_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_Allocation_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_Allocation_Traits_Unused INT, "
        "MetadataInfo_Relocation_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_Relocation_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_Relocation_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Relocation_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_Relocation_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_Relocation_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_Relocation_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_Relocation_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Relocation_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_Relocation_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_Relocation_Stats_DroppedRecords INT, "
        "MetadataInfo_Relocation_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_Relocation_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_Relocation_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_Relocation_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_Relocation_Stats_BlockedAllocations INT, "
        "MetadataInfo_Relocation_Stats_SuspendedAllocations INT, "
        "MetadataInfo_Relocation_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_Relocation_Stats_WastedBytes BIGINT, "
        "MetadataInfo_Relocation_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_Relocation_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Relocation_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_Relocation_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_Relocation_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Relocation_Traits_VaryingRecordSize INT, "
        "MetadataInfo_Relocation_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_Relocation_Traits_MultipleRecords INT, "
        "MetadataInfo_Relocation_Traits_StreamingWrite INT, "
        "MetadataInfo_Relocation_Traits_StreamingRead INT, "
        "MetadataInfo_Relocation_Traits_FrequentAllocations INT, "
        "MetadataInfo_Relocation_Traits_BlockingAllocations INT, "
        "MetadataInfo_Relocation_Traits_LinkedStore INT, "
        "MetadataInfo_Relocation_Traits_CoalesceAllocations INT, "
        "MetadataInfo_Relocation_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_Relocation_Traits_AllowPageSpill INT, "
        "MetadataInfo_Relocation_Traits_PageAligned INT, "
        "MetadataInfo_Relocation_Traits_Periodic INT, "
        "MetadataInfo_Relocation_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_Relocation_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_Relocation_Traits_Unused INT, "
        "MetadataInfo_Address_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_Address_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_Address_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Address_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_Address_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_Address_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_Address_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_Address_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Address_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_Address_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_Address_Stats_DroppedRecords INT, "
        "MetadataInfo_Address_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_Address_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_Address_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_Address_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_Address_Stats_BlockedAllocations INT, "
        "MetadataInfo_Address_Stats_SuspendedAllocations INT, "
        "MetadataInfo_Address_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_Address_Stats_WastedBytes BIGINT, "
        "MetadataInfo_Address_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_Address_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Address_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_Address_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_Address_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Address_Traits_VaryingRecordSize INT, "
        "MetadataInfo_Address_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_Address_Traits_MultipleRecords INT, "
        "MetadataInfo_Address_Traits_StreamingWrite INT, "
        "MetadataInfo_Address_Traits_StreamingRead INT, "
        "MetadataInfo_Address_Traits_FrequentAllocations INT, "
        "MetadataInfo_Address_Traits_BlockingAllocations INT, "
        "MetadataInfo_Address_Traits_LinkedStore INT, "
        "MetadataInfo_Address_Traits_CoalesceAllocations INT, "
        "MetadataInfo_Address_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_Address_Traits_AllowPageSpill INT, "
        "MetadataInfo_Address_Traits_PageAligned INT, "
        "MetadataInfo_Address_Traits_Periodic INT, "
        "MetadataInfo_Address_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_Address_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_Address_Traits_Unused INT, "
        "MetadataInfo_AddressRange_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_AddressRange_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_AddressRange_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AddressRange_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_AddressRange_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_AddressRange_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_AddressRange_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_AddressRange_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AddressRange_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_AddressRange_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_AddressRange_Stats_DroppedRecords INT, "
        "MetadataInfo_AddressRange_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_AddressRange_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_AddressRange_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_AddressRange_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_AddressRange_Stats_BlockedAllocations INT, "
        "MetadataInfo_AddressRange_Stats_SuspendedAllocations INT, "
        "MetadataInfo_AddressRange_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_AddressRange_Stats_WastedBytes BIGINT, "
        "MetadataInfo_AddressRange_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_AddressRange_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AddressRange_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_AddressRange_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_AddressRange_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AddressRange_Traits_VaryingRecordSize INT, "
        "MetadataInfo_AddressRange_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_AddressRange_Traits_MultipleRecords INT, "
        "MetadataInfo_AddressRange_Traits_StreamingWrite INT, "
        "MetadataInfo_AddressRange_Traits_StreamingRead INT, "
        "MetadataInfo_AddressRange_Traits_FrequentAllocations INT, "
        "MetadataInfo_AddressRange_Traits_BlockingAllocations INT, "
        "MetadataInfo_AddressRange_Traits_LinkedStore INT, "
        "MetadataInfo_AddressRange_Traits_CoalesceAllocations INT, "
        "MetadataInfo_AddressRange_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_AddressRange_Traits_AllowPageSpill INT, "
        "MetadataInfo_AddressRange_Traits_PageAligned INT, "
        "MetadataInfo_AddressRange_Traits_Periodic INT, "
        "MetadataInfo_AddressRange_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_AddressRange_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_AddressRange_Traits_Unused INT, "
        "MetadataInfo_AllocationTimestamp_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_AllocationTimestamp_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_AllocationTimestamp_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_AllocationTimestamp_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_AllocationTimestamp_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Stats_DroppedRecords INT, "
        "MetadataInfo_AllocationTimestamp_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_AllocationTimestamp_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_AllocationTimestamp_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_AllocationTimestamp_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_AllocationTimestamp_Stats_BlockedAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Stats_SuspendedAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_AllocationTimestamp_Stats_WastedBytes BIGINT, "
        "MetadataInfo_AllocationTimestamp_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_AllocationTimestamp_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AllocationTimestamp_Traits_VaryingRecordSize INT, "
        "MetadataInfo_AllocationTimestamp_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_AllocationTimestamp_Traits_MultipleRecords INT, "
        "MetadataInfo_AllocationTimestamp_Traits_StreamingWrite INT, "
        "MetadataInfo_AllocationTimestamp_Traits_StreamingRead INT, "
        "MetadataInfo_AllocationTimestamp_Traits_FrequentAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Traits_BlockingAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Traits_LinkedStore INT, "
        "MetadataInfo_AllocationTimestamp_Traits_CoalesceAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_AllocationTimestamp_Traits_AllowPageSpill INT, "
        "MetadataInfo_AllocationTimestamp_Traits_PageAligned INT, "
        "MetadataInfo_AllocationTimestamp_Traits_Periodic INT, "
        "MetadataInfo_AllocationTimestamp_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_AllocationTimestamp_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_AllocationTimestamp_Traits_Unused INT, "
        "MetadataInfo_AllocationTimestampDelta_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Stats_DroppedRecords INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_BlockedAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_SuspendedAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_WastedBytes BIGINT, "
        "MetadataInfo_AllocationTimestampDelta_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_AllocationTimestampDelta_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_AllocationTimestampDelta_Traits_VaryingRecordSize INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_MultipleRecords INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_StreamingWrite INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_StreamingRead INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_FrequentAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_BlockingAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_LinkedStore INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_CoalesceAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_AllowPageSpill INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_PageAligned INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_Periodic INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_AllocationTimestampDelta_Traits_Unused INT, "
        "MetadataInfo_Synchronization_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_Synchronization_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_Synchronization_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Synchronization_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_Synchronization_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_Synchronization_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_Synchronization_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_Synchronization_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Synchronization_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_Synchronization_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_Synchronization_Stats_DroppedRecords INT, "
        "MetadataInfo_Synchronization_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_Synchronization_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_Synchronization_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_Synchronization_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_Synchronization_Stats_BlockedAllocations INT, "
        "MetadataInfo_Synchronization_Stats_SuspendedAllocations INT, "
        "MetadataInfo_Synchronization_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_Synchronization_Stats_WastedBytes BIGINT, "
        "MetadataInfo_Synchronization_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_Synchronization_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Synchronization_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_Synchronization_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_Synchronization_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Synchronization_Traits_VaryingRecordSize INT, "
        "MetadataInfo_Synchronization_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_Synchronization_Traits_MultipleRecords INT, "
        "MetadataInfo_Synchronization_Traits_StreamingWrite INT, "
        "MetadataInfo_Synchronization_Traits_StreamingRead INT, "
        "MetadataInfo_Synchronization_Traits_FrequentAllocations INT, "
        "MetadataInfo_Synchronization_Traits_BlockingAllocations INT, "
        "MetadataInfo_Synchronization_Traits_LinkedStore INT, "
        "MetadataInfo_Synchronization_Traits_CoalesceAllocations INT, "
        "MetadataInfo_Synchronization_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_Synchronization_Traits_AllowPageSpill INT, "
        "MetadataInfo_Synchronization_Traits_PageAligned INT, "
        "MetadataInfo_Synchronization_Traits_Periodic INT, "
        "MetadataInfo_Synchronization_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_Synchronization_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_Synchronization_Traits_Unused INT, "
        "MetadataInfo_Info_Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "MetadataInfo_Info_Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "MetadataInfo_Info_Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Info_Time_StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "MetadataInfo_Info_Time_StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "MetadataInfo_Info_Time_StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "MetadataInfo_Info_Time_StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "MetadataInfo_Info_Time_StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Info_Time_StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "MetadataInfo_Info_Time_StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "MetadataInfo_Info_Stats_DroppedRecords INT, "
        "MetadataInfo_Info_Stats_ExhaustedFreeMemoryMaps INT, "
        "MetadataInfo_Info_Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "MetadataInfo_Info_Stats_PreferredAddressUnavailable INT, "
        "MetadataInfo_Info_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "MetadataInfo_Info_Stats_BlockedAllocations INT, "
        "MetadataInfo_Info_Stats_SuspendedAllocations INT, "
        "MetadataInfo_Info_Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "MetadataInfo_Info_Stats_WastedBytes BIGINT, "
        "MetadataInfo_Info_Stats_PaddedAllocations BIGINT, "
        "MetadataInfo_Info_Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "MetadataInfo_Info_Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "MetadataInfo_Info_Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "MetadataInfo_Info_Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "MetadataInfo_Info_Traits_VaryingRecordSize INT, "
        "MetadataInfo_Info_Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "MetadataInfo_Info_Traits_MultipleRecords INT, "
        "MetadataInfo_Info_Traits_StreamingWrite INT, "
        "MetadataInfo_Info_Traits_StreamingRead INT, "
        "MetadataInfo_Info_Traits_FrequentAllocations INT, "
        "MetadataInfo_Info_Traits_BlockingAllocations INT, "
        "MetadataInfo_Info_Traits_LinkedStore INT, "
        "MetadataInfo_Info_Traits_CoalesceAllocations INT, "
        "MetadataInfo_Info_Traits_ConcurrentAllocations INT, "
        "MetadataInfo_Info_Traits_AllowPageSpill INT, "
        "MetadataInfo_Info_Traits_PageAligned INT, "
        "MetadataInfo_Info_Traits_Periodic INT, "
        "MetadataInfo_Info_Traits_ConcurrentDataStructure INT, "
        "MetadataInfo_Info_Traits_NoAllocationAlignment INT, "
        "MetadataInfo_Info_Traits_Unused INT"
    ")";

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

    PTRACE_STORE_METADATA_INFO MetadataInfo;

    MetadataInfo = (PTRACE_STORE_METADATA_INFO)Cursor->CurrentRowRaw;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: MetadataInfo_MetadataInfo_Eof_EndOfFile BIGINT
        //

        case 0:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Eof.EndOfFile);
            break;

        //
        // 1: MetadataInfo_MetadataInfo_Time_Frequency BIGINT
        //

        case 1:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Time.Frequency);
            break;

        //
        // 2: MetadataInfo_MetadataInfo_Time_Multiplicand BIGINT
        //

        case 2:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Time.Multiplicand);
            break;

        //
        // 3: MetadataInfo_MetadataInfo_Time_StartTime_FileTimeUtc BIGINT
        //

        case 3:
            RESULT_FILETIME(MetadataInfo->MetadataInfo.Time.StartTime.FileTimeUtc);
            break;

        //
        // 4: MetadataInfo_MetadataInfo_Time_StartTime_FileTimeLocal BIGINT
        //

        case 4:
            RESULT_FILETIME(MetadataInfo->MetadataInfo.Time.StartTime.FileTimeLocal);
            break;

        //
        // 5: MetadataInfo_MetadataInfo_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 5:
            RESULT_SYSTEMTIME(MetadataInfo->MetadataInfo.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 6: MetadataInfo_MetadataInfo_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 6:
            RESULT_SYSTEMTIME(MetadataInfo->MetadataInfo.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 7: MetadataInfo_MetadataInfo_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 7:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Time.StartTime.SecondsSince1970);
            break;

        //
        // 8: MetadataInfo_MetadataInfo_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 8:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 9: MetadataInfo_MetadataInfo_Time_StartTime_PerformanceCounter BIGINT
        //

        case 9:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Time.StartTime.PerformanceCounter);
            break;

        //
        // 10: MetadataInfo_MetadataInfo_Stats_DroppedRecords INT
        //

        case 10:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.DroppedRecords);
            break;

        //
        // 11: MetadataInfo_MetadataInfo_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 11:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 12: MetadataInfo_MetadataInfo_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 12:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 13: MetadataInfo_MetadataInfo_Stats_PreferredAddressUnavailable INT
        //

        case 13:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.PreferredAddressUnavailable);
            break;

        //
        // 14: MetadataInfo_MetadataInfo_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 14:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 15: MetadataInfo_MetadataInfo_Stats_BlockedAllocations INT
        //

        case 15:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.BlockedAllocations);
            break;

        //
        // 16: MetadataInfo_MetadataInfo_Stats_SuspendedAllocations INT
        //

        case 16:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.SuspendedAllocations);
            break;

        //
        // 17: MetadataInfo_MetadataInfo_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 17:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 18: MetadataInfo_MetadataInfo_Stats_WastedBytes BIGINT
        //

        case 18:
            RESULT_ULONGLONG(MetadataInfo->MetadataInfo.Stats.WastedBytes);
            break;

        //
        // 19: MetadataInfo_MetadataInfo_Stats_PaddedAllocations BIGINT
        //

        case 19:
            RESULT_ULONGLONG(MetadataInfo->MetadataInfo.Stats.PaddedAllocations);
            break;

        //
        // 20: MetadataInfo_MetadataInfo_Totals_NumberOfAllocations BIGINT
        //

        case 20:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Totals.NumberOfAllocations);
            break;

        //
        // 21: MetadataInfo_MetadataInfo_Totals_AllocationSize BIGINT
        //

        case 21:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Totals.AllocationSize);
            break;

        //
        // 22: MetadataInfo_MetadataInfo_Totals_NumberOfRecords BIGINT
        //

        case 22:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Totals.NumberOfRecords);
            break;

        //
        // 23: MetadataInfo_MetadataInfo_Totals_RecordSize BIGINT
        //

        case 23:
            RESULT_LARGE_INTEGER(MetadataInfo->MetadataInfo.Totals.RecordSize);
            break;

        //
        // 24: MetadataInfo_MetadataInfo_Traits_VaryingRecordSize INT
        //

        case 24:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.VaryingRecordSize);
            break;

        //
        // 25: MetadataInfo_MetadataInfo_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 25:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 26: MetadataInfo_MetadataInfo_Traits_MultipleRecords INT
        //

        case 26:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.MultipleRecords);
            break;

        //
        // 27: MetadataInfo_MetadataInfo_Traits_StreamingWrite INT
        //

        case 27:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.StreamingWrite);
            break;

        //
        // 28: MetadataInfo_MetadataInfo_Traits_StreamingRead INT
        //

        case 28:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.StreamingRead);
            break;

        //
        // 29: MetadataInfo_MetadataInfo_Traits_FrequentAllocations INT
        //

        case 29:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.FrequentAllocations);
            break;

        //
        // 30: MetadataInfo_MetadataInfo_Traits_BlockingAllocations INT
        //

        case 30:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.BlockingAllocations);
            break;

        //
        // 31: MetadataInfo_MetadataInfo_Traits_LinkedStore INT
        //

        case 31:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.LinkedStore);
            break;

        //
        // 32: MetadataInfo_MetadataInfo_Traits_CoalesceAllocations INT
        //

        case 32:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.CoalesceAllocations);
            break;

        //
        // 33: MetadataInfo_MetadataInfo_Traits_ConcurrentAllocations INT
        //

        case 33:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.ConcurrentAllocations);
            break;

        //
        // 34: MetadataInfo_MetadataInfo_Traits_AllowPageSpill INT
        //

        case 34:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.AllowPageSpill);
            break;

        //
        // 35: MetadataInfo_MetadataInfo_Traits_PageAligned INT
        //

        case 35:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.PageAligned);
            break;

        //
        // 36: MetadataInfo_MetadataInfo_Traits_Periodic INT
        //

        case 36:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.Periodic);
            break;

        //
        // 37: MetadataInfo_MetadataInfo_Traits_ConcurrentDataStructure INT
        //

        case 37:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.ConcurrentDataStructure);
            break;

        //
        // 38: MetadataInfo_MetadataInfo_Traits_NoAllocationAlignment INT
        //

        case 38:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.NoAllocationAlignment);
            break;

        //
        // 39: MetadataInfo_MetadataInfo_Traits_Unused INT
        //

        case 39:
            RESULT_ULONG(MetadataInfo->MetadataInfo.Traits.Unused);
            break;

        //
        // 40: MetadataInfo_Allocation_Eof_EndOfFile BIGINT
        //

        case 40:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Eof.EndOfFile);
            break;

        //
        // 41: MetadataInfo_Allocation_Time_Frequency BIGINT
        //

        case 41:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Time.Frequency);
            break;

        //
        // 42: MetadataInfo_Allocation_Time_Multiplicand BIGINT
        //

        case 42:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Time.Multiplicand);
            break;

        //
        // 43: MetadataInfo_Allocation_Time_StartTime_FileTimeUtc BIGINT
        //

        case 43:
            RESULT_FILETIME(MetadataInfo->Allocation.Time.StartTime.FileTimeUtc);
            break;

        //
        // 44: MetadataInfo_Allocation_Time_StartTime_FileTimeLocal BIGINT
        //

        case 44:
            RESULT_FILETIME(MetadataInfo->Allocation.Time.StartTime.FileTimeLocal);
            break;

        //
        // 45: MetadataInfo_Allocation_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 45:
            RESULT_SYSTEMTIME(MetadataInfo->Allocation.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 46: MetadataInfo_Allocation_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 46:
            RESULT_SYSTEMTIME(MetadataInfo->Allocation.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 47: MetadataInfo_Allocation_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 47:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Time.StartTime.SecondsSince1970);
            break;

        //
        // 48: MetadataInfo_Allocation_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 48:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 49: MetadataInfo_Allocation_Time_StartTime_PerformanceCounter BIGINT
        //

        case 49:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Time.StartTime.PerformanceCounter);
            break;

        //
        // 50: MetadataInfo_Allocation_Stats_DroppedRecords INT
        //

        case 50:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.DroppedRecords);
            break;

        //
        // 51: MetadataInfo_Allocation_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 51:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 52: MetadataInfo_Allocation_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 52:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 53: MetadataInfo_Allocation_Stats_PreferredAddressUnavailable INT
        //

        case 53:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.PreferredAddressUnavailable);
            break;

        //
        // 54: MetadataInfo_Allocation_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 54:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 55: MetadataInfo_Allocation_Stats_BlockedAllocations INT
        //

        case 55:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.BlockedAllocations);
            break;

        //
        // 56: MetadataInfo_Allocation_Stats_SuspendedAllocations INT
        //

        case 56:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.SuspendedAllocations);
            break;

        //
        // 57: MetadataInfo_Allocation_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 57:
            RESULT_ULONG(MetadataInfo->Allocation.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 58: MetadataInfo_Allocation_Stats_WastedBytes BIGINT
        //

        case 58:
            RESULT_ULONGLONG(MetadataInfo->Allocation.Stats.WastedBytes);
            break;

        //
        // 59: MetadataInfo_Allocation_Stats_PaddedAllocations BIGINT
        //

        case 59:
            RESULT_ULONGLONG(MetadataInfo->Allocation.Stats.PaddedAllocations);
            break;

        //
        // 60: MetadataInfo_Allocation_Totals_NumberOfAllocations BIGINT
        //

        case 60:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Totals.NumberOfAllocations);
            break;

        //
        // 61: MetadataInfo_Allocation_Totals_AllocationSize BIGINT
        //

        case 61:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Totals.AllocationSize);
            break;

        //
        // 62: MetadataInfo_Allocation_Totals_NumberOfRecords BIGINT
        //

        case 62:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Totals.NumberOfRecords);
            break;

        //
        // 63: MetadataInfo_Allocation_Totals_RecordSize BIGINT
        //

        case 63:
            RESULT_LARGE_INTEGER(MetadataInfo->Allocation.Totals.RecordSize);
            break;

        //
        // 64: MetadataInfo_Allocation_Traits_VaryingRecordSize INT
        //

        case 64:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.VaryingRecordSize);
            break;

        //
        // 65: MetadataInfo_Allocation_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 65:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 66: MetadataInfo_Allocation_Traits_MultipleRecords INT
        //

        case 66:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.MultipleRecords);
            break;

        //
        // 67: MetadataInfo_Allocation_Traits_StreamingWrite INT
        //

        case 67:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.StreamingWrite);
            break;

        //
        // 68: MetadataInfo_Allocation_Traits_StreamingRead INT
        //

        case 68:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.StreamingRead);
            break;

        //
        // 69: MetadataInfo_Allocation_Traits_FrequentAllocations INT
        //

        case 69:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.FrequentAllocations);
            break;

        //
        // 70: MetadataInfo_Allocation_Traits_BlockingAllocations INT
        //

        case 70:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.BlockingAllocations);
            break;

        //
        // 71: MetadataInfo_Allocation_Traits_LinkedStore INT
        //

        case 71:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.LinkedStore);
            break;

        //
        // 72: MetadataInfo_Allocation_Traits_CoalesceAllocations INT
        //

        case 72:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.CoalesceAllocations);
            break;

        //
        // 73: MetadataInfo_Allocation_Traits_ConcurrentAllocations INT
        //

        case 73:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.ConcurrentAllocations);
            break;

        //
        // 74: MetadataInfo_Allocation_Traits_AllowPageSpill INT
        //

        case 74:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.AllowPageSpill);
            break;

        //
        // 75: MetadataInfo_Allocation_Traits_PageAligned INT
        //

        case 75:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.PageAligned);
            break;

        //
        // 76: MetadataInfo_Allocation_Traits_Periodic INT
        //

        case 76:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.Periodic);
            break;

        //
        // 77: MetadataInfo_Allocation_Traits_ConcurrentDataStructure INT
        //

        case 77:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.ConcurrentDataStructure);
            break;

        //
        // 78: MetadataInfo_Allocation_Traits_NoAllocationAlignment INT
        //

        case 78:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.NoAllocationAlignment);
            break;

        //
        // 79: MetadataInfo_Allocation_Traits_Unused INT
        //

        case 79:
            RESULT_ULONG(MetadataInfo->Allocation.Traits.Unused);
            break;

        //
        // 80: MetadataInfo_Relocation_Eof_EndOfFile BIGINT
        //

        case 80:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Eof.EndOfFile);
            break;

        //
        // 81: MetadataInfo_Relocation_Time_Frequency BIGINT
        //

        case 81:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Time.Frequency);
            break;

        //
        // 82: MetadataInfo_Relocation_Time_Multiplicand BIGINT
        //

        case 82:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Time.Multiplicand);
            break;

        //
        // 83: MetadataInfo_Relocation_Time_StartTime_FileTimeUtc BIGINT
        //

        case 83:
            RESULT_FILETIME(MetadataInfo->Relocation.Time.StartTime.FileTimeUtc);
            break;

        //
        // 84: MetadataInfo_Relocation_Time_StartTime_FileTimeLocal BIGINT
        //

        case 84:
            RESULT_FILETIME(MetadataInfo->Relocation.Time.StartTime.FileTimeLocal);
            break;

        //
        // 85: MetadataInfo_Relocation_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 85:
            RESULT_SYSTEMTIME(MetadataInfo->Relocation.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 86: MetadataInfo_Relocation_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 86:
            RESULT_SYSTEMTIME(MetadataInfo->Relocation.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 87: MetadataInfo_Relocation_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 87:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Time.StartTime.SecondsSince1970);
            break;

        //
        // 88: MetadataInfo_Relocation_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 88:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 89: MetadataInfo_Relocation_Time_StartTime_PerformanceCounter BIGINT
        //

        case 89:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Time.StartTime.PerformanceCounter);
            break;

        //
        // 90: MetadataInfo_Relocation_Stats_DroppedRecords INT
        //

        case 90:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.DroppedRecords);
            break;

        //
        // 91: MetadataInfo_Relocation_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 91:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 92: MetadataInfo_Relocation_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 92:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 93: MetadataInfo_Relocation_Stats_PreferredAddressUnavailable INT
        //

        case 93:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.PreferredAddressUnavailable);
            break;

        //
        // 94: MetadataInfo_Relocation_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 94:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 95: MetadataInfo_Relocation_Stats_BlockedAllocations INT
        //

        case 95:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.BlockedAllocations);
            break;

        //
        // 96: MetadataInfo_Relocation_Stats_SuspendedAllocations INT
        //

        case 96:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.SuspendedAllocations);
            break;

        //
        // 97: MetadataInfo_Relocation_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 97:
            RESULT_ULONG(MetadataInfo->Relocation.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 98: MetadataInfo_Relocation_Stats_WastedBytes BIGINT
        //

        case 98:
            RESULT_ULONGLONG(MetadataInfo->Relocation.Stats.WastedBytes);
            break;

        //
        // 99: MetadataInfo_Relocation_Stats_PaddedAllocations BIGINT
        //

        case 99:
            RESULT_ULONGLONG(MetadataInfo->Relocation.Stats.PaddedAllocations);
            break;

        //
        // 100: MetadataInfo_Relocation_Totals_NumberOfAllocations BIGINT
        //

        case 100:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Totals.NumberOfAllocations);
            break;

        //
        // 101: MetadataInfo_Relocation_Totals_AllocationSize BIGINT
        //

        case 101:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Totals.AllocationSize);
            break;

        //
        // 102: MetadataInfo_Relocation_Totals_NumberOfRecords BIGINT
        //

        case 102:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Totals.NumberOfRecords);
            break;

        //
        // 103: MetadataInfo_Relocation_Totals_RecordSize BIGINT
        //

        case 103:
            RESULT_LARGE_INTEGER(MetadataInfo->Relocation.Totals.RecordSize);
            break;

        //
        // 104: MetadataInfo_Relocation_Traits_VaryingRecordSize INT
        //

        case 104:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.VaryingRecordSize);
            break;

        //
        // 105: MetadataInfo_Relocation_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 105:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 106: MetadataInfo_Relocation_Traits_MultipleRecords INT
        //

        case 106:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.MultipleRecords);
            break;

        //
        // 107: MetadataInfo_Relocation_Traits_StreamingWrite INT
        //

        case 107:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.StreamingWrite);
            break;

        //
        // 108: MetadataInfo_Relocation_Traits_StreamingRead INT
        //

        case 108:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.StreamingRead);
            break;

        //
        // 109: MetadataInfo_Relocation_Traits_FrequentAllocations INT
        //

        case 109:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.FrequentAllocations);
            break;

        //
        // 110: MetadataInfo_Relocation_Traits_BlockingAllocations INT
        //

        case 110:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.BlockingAllocations);
            break;

        //
        // 111: MetadataInfo_Relocation_Traits_LinkedStore INT
        //

        case 111:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.LinkedStore);
            break;

        //
        // 112: MetadataInfo_Relocation_Traits_CoalesceAllocations INT
        //

        case 112:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.CoalesceAllocations);
            break;

        //
        // 113: MetadataInfo_Relocation_Traits_ConcurrentAllocations INT
        //

        case 113:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.ConcurrentAllocations);
            break;

        //
        // 114: MetadataInfo_Relocation_Traits_AllowPageSpill INT
        //

        case 114:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.AllowPageSpill);
            break;

        //
        // 115: MetadataInfo_Relocation_Traits_PageAligned INT
        //

        case 115:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.PageAligned);
            break;

        //
        // 116: MetadataInfo_Relocation_Traits_Periodic INT
        //

        case 116:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.Periodic);
            break;

        //
        // 117: MetadataInfo_Relocation_Traits_ConcurrentDataStructure INT
        //

        case 117:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.ConcurrentDataStructure);
            break;

        //
        // 118: MetadataInfo_Relocation_Traits_NoAllocationAlignment INT
        //

        case 118:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.NoAllocationAlignment);
            break;

        //
        // 119: MetadataInfo_Relocation_Traits_Unused INT
        //

        case 119:
            RESULT_ULONG(MetadataInfo->Relocation.Traits.Unused);
            break;

        //
        // 120: MetadataInfo_Address_Eof_EndOfFile BIGINT
        //

        case 120:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Eof.EndOfFile);
            break;

        //
        // 121: MetadataInfo_Address_Time_Frequency BIGINT
        //

        case 121:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Time.Frequency);
            break;

        //
        // 122: MetadataInfo_Address_Time_Multiplicand BIGINT
        //

        case 122:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Time.Multiplicand);
            break;

        //
        // 123: MetadataInfo_Address_Time_StartTime_FileTimeUtc BIGINT
        //

        case 123:
            RESULT_FILETIME(MetadataInfo->Address.Time.StartTime.FileTimeUtc);
            break;

        //
        // 124: MetadataInfo_Address_Time_StartTime_FileTimeLocal BIGINT
        //

        case 124:
            RESULT_FILETIME(MetadataInfo->Address.Time.StartTime.FileTimeLocal);
            break;

        //
        // 125: MetadataInfo_Address_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 125:
            RESULT_SYSTEMTIME(MetadataInfo->Address.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 126: MetadataInfo_Address_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 126:
            RESULT_SYSTEMTIME(MetadataInfo->Address.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 127: MetadataInfo_Address_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 127:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Time.StartTime.SecondsSince1970);
            break;

        //
        // 128: MetadataInfo_Address_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 128:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 129: MetadataInfo_Address_Time_StartTime_PerformanceCounter BIGINT
        //

        case 129:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Time.StartTime.PerformanceCounter);
            break;

        //
        // 130: MetadataInfo_Address_Stats_DroppedRecords INT
        //

        case 130:
            RESULT_ULONG(MetadataInfo->Address.Stats.DroppedRecords);
            break;

        //
        // 131: MetadataInfo_Address_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 131:
            RESULT_ULONG(MetadataInfo->Address.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 132: MetadataInfo_Address_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 132:
            RESULT_ULONG(MetadataInfo->Address.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 133: MetadataInfo_Address_Stats_PreferredAddressUnavailable INT
        //

        case 133:
            RESULT_ULONG(MetadataInfo->Address.Stats.PreferredAddressUnavailable);
            break;

        //
        // 134: MetadataInfo_Address_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 134:
            RESULT_ULONG(MetadataInfo->Address.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 135: MetadataInfo_Address_Stats_BlockedAllocations INT
        //

        case 135:
            RESULT_ULONG(MetadataInfo->Address.Stats.BlockedAllocations);
            break;

        //
        // 136: MetadataInfo_Address_Stats_SuspendedAllocations INT
        //

        case 136:
            RESULT_ULONG(MetadataInfo->Address.Stats.SuspendedAllocations);
            break;

        //
        // 137: MetadataInfo_Address_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 137:
            RESULT_ULONG(MetadataInfo->Address.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 138: MetadataInfo_Address_Stats_WastedBytes BIGINT
        //

        case 138:
            RESULT_ULONGLONG(MetadataInfo->Address.Stats.WastedBytes);
            break;

        //
        // 139: MetadataInfo_Address_Stats_PaddedAllocations BIGINT
        //

        case 139:
            RESULT_ULONGLONG(MetadataInfo->Address.Stats.PaddedAllocations);
            break;

        //
        // 140: MetadataInfo_Address_Totals_NumberOfAllocations BIGINT
        //

        case 140:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Totals.NumberOfAllocations);
            break;

        //
        // 141: MetadataInfo_Address_Totals_AllocationSize BIGINT
        //

        case 141:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Totals.AllocationSize);
            break;

        //
        // 142: MetadataInfo_Address_Totals_NumberOfRecords BIGINT
        //

        case 142:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Totals.NumberOfRecords);
            break;

        //
        // 143: MetadataInfo_Address_Totals_RecordSize BIGINT
        //

        case 143:
            RESULT_LARGE_INTEGER(MetadataInfo->Address.Totals.RecordSize);
            break;

        //
        // 144: MetadataInfo_Address_Traits_VaryingRecordSize INT
        //

        case 144:
            RESULT_ULONG(MetadataInfo->Address.Traits.VaryingRecordSize);
            break;

        //
        // 145: MetadataInfo_Address_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 145:
            RESULT_ULONG(MetadataInfo->Address.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 146: MetadataInfo_Address_Traits_MultipleRecords INT
        //

        case 146:
            RESULT_ULONG(MetadataInfo->Address.Traits.MultipleRecords);
            break;

        //
        // 147: MetadataInfo_Address_Traits_StreamingWrite INT
        //

        case 147:
            RESULT_ULONG(MetadataInfo->Address.Traits.StreamingWrite);
            break;

        //
        // 148: MetadataInfo_Address_Traits_StreamingRead INT
        //

        case 148:
            RESULT_ULONG(MetadataInfo->Address.Traits.StreamingRead);
            break;

        //
        // 149: MetadataInfo_Address_Traits_FrequentAllocations INT
        //

        case 149:
            RESULT_ULONG(MetadataInfo->Address.Traits.FrequentAllocations);
            break;

        //
        // 150: MetadataInfo_Address_Traits_BlockingAllocations INT
        //

        case 150:
            RESULT_ULONG(MetadataInfo->Address.Traits.BlockingAllocations);
            break;

        //
        // 151: MetadataInfo_Address_Traits_LinkedStore INT
        //

        case 151:
            RESULT_ULONG(MetadataInfo->Address.Traits.LinkedStore);
            break;

        //
        // 152: MetadataInfo_Address_Traits_CoalesceAllocations INT
        //

        case 152:
            RESULT_ULONG(MetadataInfo->Address.Traits.CoalesceAllocations);
            break;

        //
        // 153: MetadataInfo_Address_Traits_ConcurrentAllocations INT
        //

        case 153:
            RESULT_ULONG(MetadataInfo->Address.Traits.ConcurrentAllocations);
            break;

        //
        // 154: MetadataInfo_Address_Traits_AllowPageSpill INT
        //

        case 154:
            RESULT_ULONG(MetadataInfo->Address.Traits.AllowPageSpill);
            break;

        //
        // 155: MetadataInfo_Address_Traits_PageAligned INT
        //

        case 155:
            RESULT_ULONG(MetadataInfo->Address.Traits.PageAligned);
            break;

        //
        // 156: MetadataInfo_Address_Traits_Periodic INT
        //

        case 156:
            RESULT_ULONG(MetadataInfo->Address.Traits.Periodic);
            break;

        //
        // 157: MetadataInfo_Address_Traits_ConcurrentDataStructure INT
        //

        case 157:
            RESULT_ULONG(MetadataInfo->Address.Traits.ConcurrentDataStructure);
            break;

        //
        // 158: MetadataInfo_Address_Traits_NoAllocationAlignment INT
        //

        case 158:
            RESULT_ULONG(MetadataInfo->Address.Traits.NoAllocationAlignment);
            break;

        //
        // 159: MetadataInfo_Address_Traits_Unused INT
        //

        case 159:
            RESULT_ULONG(MetadataInfo->Address.Traits.Unused);
            break;

        //
        // 160: MetadataInfo_AddressRange_Eof_EndOfFile BIGINT
        //

        case 160:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Eof.EndOfFile);
            break;

        //
        // 161: MetadataInfo_AddressRange_Time_Frequency BIGINT
        //

        case 161:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Time.Frequency);
            break;

        //
        // 162: MetadataInfo_AddressRange_Time_Multiplicand BIGINT
        //

        case 162:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Time.Multiplicand);
            break;

        //
        // 163: MetadataInfo_AddressRange_Time_StartTime_FileTimeUtc BIGINT
        //

        case 163:
            RESULT_FILETIME(MetadataInfo->AddressRange.Time.StartTime.FileTimeUtc);
            break;

        //
        // 164: MetadataInfo_AddressRange_Time_StartTime_FileTimeLocal BIGINT
        //

        case 164:
            RESULT_FILETIME(MetadataInfo->AddressRange.Time.StartTime.FileTimeLocal);
            break;

        //
        // 165: MetadataInfo_AddressRange_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 165:
            RESULT_SYSTEMTIME(MetadataInfo->AddressRange.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 166: MetadataInfo_AddressRange_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 166:
            RESULT_SYSTEMTIME(MetadataInfo->AddressRange.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 167: MetadataInfo_AddressRange_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 167:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Time.StartTime.SecondsSince1970);
            break;

        //
        // 168: MetadataInfo_AddressRange_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 168:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 169: MetadataInfo_AddressRange_Time_StartTime_PerformanceCounter BIGINT
        //

        case 169:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Time.StartTime.PerformanceCounter);
            break;

        //
        // 170: MetadataInfo_AddressRange_Stats_DroppedRecords INT
        //

        case 170:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.DroppedRecords);
            break;

        //
        // 171: MetadataInfo_AddressRange_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 171:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 172: MetadataInfo_AddressRange_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 172:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 173: MetadataInfo_AddressRange_Stats_PreferredAddressUnavailable INT
        //

        case 173:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.PreferredAddressUnavailable);
            break;

        //
        // 174: MetadataInfo_AddressRange_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 174:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 175: MetadataInfo_AddressRange_Stats_BlockedAllocations INT
        //

        case 175:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.BlockedAllocations);
            break;

        //
        // 176: MetadataInfo_AddressRange_Stats_SuspendedAllocations INT
        //

        case 176:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.SuspendedAllocations);
            break;

        //
        // 177: MetadataInfo_AddressRange_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 177:
            RESULT_ULONG(MetadataInfo->AddressRange.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 178: MetadataInfo_AddressRange_Stats_WastedBytes BIGINT
        //

        case 178:
            RESULT_ULONGLONG(MetadataInfo->AddressRange.Stats.WastedBytes);
            break;

        //
        // 179: MetadataInfo_AddressRange_Stats_PaddedAllocations BIGINT
        //

        case 179:
            RESULT_ULONGLONG(MetadataInfo->AddressRange.Stats.PaddedAllocations);
            break;

        //
        // 180: MetadataInfo_AddressRange_Totals_NumberOfAllocations BIGINT
        //

        case 180:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Totals.NumberOfAllocations);
            break;

        //
        // 181: MetadataInfo_AddressRange_Totals_AllocationSize BIGINT
        //

        case 181:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Totals.AllocationSize);
            break;

        //
        // 182: MetadataInfo_AddressRange_Totals_NumberOfRecords BIGINT
        //

        case 182:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Totals.NumberOfRecords);
            break;

        //
        // 183: MetadataInfo_AddressRange_Totals_RecordSize BIGINT
        //

        case 183:
            RESULT_LARGE_INTEGER(MetadataInfo->AddressRange.Totals.RecordSize);
            break;

        //
        // 184: MetadataInfo_AddressRange_Traits_VaryingRecordSize INT
        //

        case 184:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.VaryingRecordSize);
            break;

        //
        // 185: MetadataInfo_AddressRange_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 185:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 186: MetadataInfo_AddressRange_Traits_MultipleRecords INT
        //

        case 186:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.MultipleRecords);
            break;

        //
        // 187: MetadataInfo_AddressRange_Traits_StreamingWrite INT
        //

        case 187:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.StreamingWrite);
            break;

        //
        // 188: MetadataInfo_AddressRange_Traits_StreamingRead INT
        //

        case 188:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.StreamingRead);
            break;

        //
        // 189: MetadataInfo_AddressRange_Traits_FrequentAllocations INT
        //

        case 189:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.FrequentAllocations);
            break;

        //
        // 190: MetadataInfo_AddressRange_Traits_BlockingAllocations INT
        //

        case 190:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.BlockingAllocations);
            break;

        //
        // 191: MetadataInfo_AddressRange_Traits_LinkedStore INT
        //

        case 191:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.LinkedStore);
            break;

        //
        // 192: MetadataInfo_AddressRange_Traits_CoalesceAllocations INT
        //

        case 192:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.CoalesceAllocations);
            break;

        //
        // 193: MetadataInfo_AddressRange_Traits_ConcurrentAllocations INT
        //

        case 193:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.ConcurrentAllocations);
            break;

        //
        // 194: MetadataInfo_AddressRange_Traits_AllowPageSpill INT
        //

        case 194:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.AllowPageSpill);
            break;

        //
        // 195: MetadataInfo_AddressRange_Traits_PageAligned INT
        //

        case 195:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.PageAligned);
            break;

        //
        // 196: MetadataInfo_AddressRange_Traits_Periodic INT
        //

        case 196:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.Periodic);
            break;

        //
        // 197: MetadataInfo_AddressRange_Traits_ConcurrentDataStructure INT
        //

        case 197:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.ConcurrentDataStructure);
            break;

        //
        // 198: MetadataInfo_AddressRange_Traits_NoAllocationAlignment INT
        //

        case 198:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.NoAllocationAlignment);
            break;

        //
        // 199: MetadataInfo_AddressRange_Traits_Unused INT
        //

        case 199:
            RESULT_ULONG(MetadataInfo->AddressRange.Traits.Unused);
            break;

        //
        // 200: MetadataInfo_AllocationTimestamp_Eof_EndOfFile BIGINT
        //

        case 200:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Eof.EndOfFile);
            break;

        //
        // 201: MetadataInfo_AllocationTimestamp_Time_Frequency BIGINT
        //

        case 201:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Time.Frequency);
            break;

        //
        // 202: MetadataInfo_AllocationTimestamp_Time_Multiplicand BIGINT
        //

        case 202:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Time.Multiplicand);
            break;

        //
        // 203: MetadataInfo_AllocationTimestamp_Time_StartTime_FileTimeUtc BIGINT
        //

        case 203:
            RESULT_FILETIME(MetadataInfo->AllocationTimestamp.Time.StartTime.FileTimeUtc);
            break;

        //
        // 204: MetadataInfo_AllocationTimestamp_Time_StartTime_FileTimeLocal BIGINT
        //

        case 204:
            RESULT_FILETIME(MetadataInfo->AllocationTimestamp.Time.StartTime.FileTimeLocal);
            break;

        //
        // 205: MetadataInfo_AllocationTimestamp_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 205:
            RESULT_SYSTEMTIME(MetadataInfo->AllocationTimestamp.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 206: MetadataInfo_AllocationTimestamp_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 206:
            RESULT_SYSTEMTIME(MetadataInfo->AllocationTimestamp.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 207: MetadataInfo_AllocationTimestamp_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 207:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Time.StartTime.SecondsSince1970);
            break;

        //
        // 208: MetadataInfo_AllocationTimestamp_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 208:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 209: MetadataInfo_AllocationTimestamp_Time_StartTime_PerformanceCounter BIGINT
        //

        case 209:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Time.StartTime.PerformanceCounter);
            break;

        //
        // 210: MetadataInfo_AllocationTimestamp_Stats_DroppedRecords INT
        //

        case 210:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.DroppedRecords);
            break;

        //
        // 211: MetadataInfo_AllocationTimestamp_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 211:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 212: MetadataInfo_AllocationTimestamp_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 212:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 213: MetadataInfo_AllocationTimestamp_Stats_PreferredAddressUnavailable INT
        //

        case 213:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.PreferredAddressUnavailable);
            break;

        //
        // 214: MetadataInfo_AllocationTimestamp_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 214:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 215: MetadataInfo_AllocationTimestamp_Stats_BlockedAllocations INT
        //

        case 215:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.BlockedAllocations);
            break;

        //
        // 216: MetadataInfo_AllocationTimestamp_Stats_SuspendedAllocations INT
        //

        case 216:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.SuspendedAllocations);
            break;

        //
        // 217: MetadataInfo_AllocationTimestamp_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 217:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 218: MetadataInfo_AllocationTimestamp_Stats_WastedBytes BIGINT
        //

        case 218:
            RESULT_ULONGLONG(MetadataInfo->AllocationTimestamp.Stats.WastedBytes);
            break;

        //
        // 219: MetadataInfo_AllocationTimestamp_Stats_PaddedAllocations BIGINT
        //

        case 219:
            RESULT_ULONGLONG(MetadataInfo->AllocationTimestamp.Stats.PaddedAllocations);
            break;

        //
        // 220: MetadataInfo_AllocationTimestamp_Totals_NumberOfAllocations BIGINT
        //

        case 220:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Totals.NumberOfAllocations);
            break;

        //
        // 221: MetadataInfo_AllocationTimestamp_Totals_AllocationSize BIGINT
        //

        case 221:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Totals.AllocationSize);
            break;

        //
        // 222: MetadataInfo_AllocationTimestamp_Totals_NumberOfRecords BIGINT
        //

        case 222:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Totals.NumberOfRecords);
            break;

        //
        // 223: MetadataInfo_AllocationTimestamp_Totals_RecordSize BIGINT
        //

        case 223:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestamp.Totals.RecordSize);
            break;

        //
        // 224: MetadataInfo_AllocationTimestamp_Traits_VaryingRecordSize INT
        //

        case 224:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.VaryingRecordSize);
            break;

        //
        // 225: MetadataInfo_AllocationTimestamp_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 225:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 226: MetadataInfo_AllocationTimestamp_Traits_MultipleRecords INT
        //

        case 226:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.MultipleRecords);
            break;

        //
        // 227: MetadataInfo_AllocationTimestamp_Traits_StreamingWrite INT
        //

        case 227:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.StreamingWrite);
            break;

        //
        // 228: MetadataInfo_AllocationTimestamp_Traits_StreamingRead INT
        //

        case 228:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.StreamingRead);
            break;

        //
        // 229: MetadataInfo_AllocationTimestamp_Traits_FrequentAllocations INT
        //

        case 229:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.FrequentAllocations);
            break;

        //
        // 230: MetadataInfo_AllocationTimestamp_Traits_BlockingAllocations INT
        //

        case 230:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.BlockingAllocations);
            break;

        //
        // 231: MetadataInfo_AllocationTimestamp_Traits_LinkedStore INT
        //

        case 231:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.LinkedStore);
            break;

        //
        // 232: MetadataInfo_AllocationTimestamp_Traits_CoalesceAllocations INT
        //

        case 232:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.CoalesceAllocations);
            break;

        //
        // 233: MetadataInfo_AllocationTimestamp_Traits_ConcurrentAllocations INT
        //

        case 233:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.ConcurrentAllocations);
            break;

        //
        // 234: MetadataInfo_AllocationTimestamp_Traits_AllowPageSpill INT
        //

        case 234:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.AllowPageSpill);
            break;

        //
        // 235: MetadataInfo_AllocationTimestamp_Traits_PageAligned INT
        //

        case 235:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.PageAligned);
            break;

        //
        // 236: MetadataInfo_AllocationTimestamp_Traits_Periodic INT
        //

        case 236:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.Periodic);
            break;

        //
        // 237: MetadataInfo_AllocationTimestamp_Traits_ConcurrentDataStructure INT
        //

        case 237:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.ConcurrentDataStructure);
            break;

        //
        // 238: MetadataInfo_AllocationTimestamp_Traits_NoAllocationAlignment INT
        //

        case 238:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.NoAllocationAlignment);
            break;

        //
        // 239: MetadataInfo_AllocationTimestamp_Traits_Unused INT
        //

        case 239:
            RESULT_ULONG(MetadataInfo->AllocationTimestamp.Traits.Unused);
            break;

        //
        // 240: MetadataInfo_AllocationTimestampDelta_Eof_EndOfFile BIGINT
        //

        case 240:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Eof.EndOfFile);
            break;

        //
        // 241: MetadataInfo_AllocationTimestampDelta_Time_Frequency BIGINT
        //

        case 241:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Time.Frequency);
            break;

        //
        // 242: MetadataInfo_AllocationTimestampDelta_Time_Multiplicand BIGINT
        //

        case 242:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Time.Multiplicand);
            break;

        //
        // 243: MetadataInfo_AllocationTimestampDelta_Time_StartTime_FileTimeUtc BIGINT
        //

        case 243:
            RESULT_FILETIME(MetadataInfo->AllocationTimestampDelta.Time.StartTime.FileTimeUtc);
            break;

        //
        // 244: MetadataInfo_AllocationTimestampDelta_Time_StartTime_FileTimeLocal BIGINT
        //

        case 244:
            RESULT_FILETIME(MetadataInfo->AllocationTimestampDelta.Time.StartTime.FileTimeLocal);
            break;

        //
        // 245: MetadataInfo_AllocationTimestampDelta_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 245:
            RESULT_SYSTEMTIME(MetadataInfo->AllocationTimestampDelta.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 246: MetadataInfo_AllocationTimestampDelta_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 246:
            RESULT_SYSTEMTIME(MetadataInfo->AllocationTimestampDelta.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 247: MetadataInfo_AllocationTimestampDelta_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 247:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Time.StartTime.SecondsSince1970);
            break;

        //
        // 248: MetadataInfo_AllocationTimestampDelta_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 248:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 249: MetadataInfo_AllocationTimestampDelta_Time_StartTime_PerformanceCounter BIGINT
        //

        case 249:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Time.StartTime.PerformanceCounter);
            break;

        //
        // 250: MetadataInfo_AllocationTimestampDelta_Stats_DroppedRecords INT
        //

        case 250:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.DroppedRecords);
            break;

        //
        // 251: MetadataInfo_AllocationTimestampDelta_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 251:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 252: MetadataInfo_AllocationTimestampDelta_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 252:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 253: MetadataInfo_AllocationTimestampDelta_Stats_PreferredAddressUnavailable INT
        //

        case 253:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.PreferredAddressUnavailable);
            break;

        //
        // 254: MetadataInfo_AllocationTimestampDelta_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 254:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 255: MetadataInfo_AllocationTimestampDelta_Stats_BlockedAllocations INT
        //

        case 255:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.BlockedAllocations);
            break;

        //
        // 256: MetadataInfo_AllocationTimestampDelta_Stats_SuspendedAllocations INT
        //

        case 256:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.SuspendedAllocations);
            break;

        //
        // 257: MetadataInfo_AllocationTimestampDelta_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 257:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 258: MetadataInfo_AllocationTimestampDelta_Stats_WastedBytes BIGINT
        //

        case 258:
            RESULT_ULONGLONG(MetadataInfo->AllocationTimestampDelta.Stats.WastedBytes);
            break;

        //
        // 259: MetadataInfo_AllocationTimestampDelta_Stats_PaddedAllocations BIGINT
        //

        case 259:
            RESULT_ULONGLONG(MetadataInfo->AllocationTimestampDelta.Stats.PaddedAllocations);
            break;

        //
        // 260: MetadataInfo_AllocationTimestampDelta_Totals_NumberOfAllocations BIGINT
        //

        case 260:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Totals.NumberOfAllocations);
            break;

        //
        // 261: MetadataInfo_AllocationTimestampDelta_Totals_AllocationSize BIGINT
        //

        case 261:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Totals.AllocationSize);
            break;

        //
        // 262: MetadataInfo_AllocationTimestampDelta_Totals_NumberOfRecords BIGINT
        //

        case 262:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Totals.NumberOfRecords);
            break;

        //
        // 263: MetadataInfo_AllocationTimestampDelta_Totals_RecordSize BIGINT
        //

        case 263:
            RESULT_LARGE_INTEGER(MetadataInfo->AllocationTimestampDelta.Totals.RecordSize);
            break;

        //
        // 264: MetadataInfo_AllocationTimestampDelta_Traits_VaryingRecordSize INT
        //

        case 264:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.VaryingRecordSize);
            break;

        //
        // 265: MetadataInfo_AllocationTimestampDelta_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 265:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 266: MetadataInfo_AllocationTimestampDelta_Traits_MultipleRecords INT
        //

        case 266:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.MultipleRecords);
            break;

        //
        // 267: MetadataInfo_AllocationTimestampDelta_Traits_StreamingWrite INT
        //

        case 267:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.StreamingWrite);
            break;

        //
        // 268: MetadataInfo_AllocationTimestampDelta_Traits_StreamingRead INT
        //

        case 268:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.StreamingRead);
            break;

        //
        // 269: MetadataInfo_AllocationTimestampDelta_Traits_FrequentAllocations INT
        //

        case 269:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.FrequentAllocations);
            break;

        //
        // 270: MetadataInfo_AllocationTimestampDelta_Traits_BlockingAllocations INT
        //

        case 270:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.BlockingAllocations);
            break;

        //
        // 271: MetadataInfo_AllocationTimestampDelta_Traits_LinkedStore INT
        //

        case 271:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.LinkedStore);
            break;

        //
        // 272: MetadataInfo_AllocationTimestampDelta_Traits_CoalesceAllocations INT
        //

        case 272:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.CoalesceAllocations);
            break;

        //
        // 273: MetadataInfo_AllocationTimestampDelta_Traits_ConcurrentAllocations INT
        //

        case 273:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.ConcurrentAllocations);
            break;

        //
        // 274: MetadataInfo_AllocationTimestampDelta_Traits_AllowPageSpill INT
        //

        case 274:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.AllowPageSpill);
            break;

        //
        // 275: MetadataInfo_AllocationTimestampDelta_Traits_PageAligned INT
        //

        case 275:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.PageAligned);
            break;

        //
        // 276: MetadataInfo_AllocationTimestampDelta_Traits_Periodic INT
        //

        case 276:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.Periodic);
            break;

        //
        // 277: MetadataInfo_AllocationTimestampDelta_Traits_ConcurrentDataStructure INT
        //

        case 277:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.ConcurrentDataStructure);
            break;

        //
        // 278: MetadataInfo_AllocationTimestampDelta_Traits_NoAllocationAlignment INT
        //

        case 278:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.NoAllocationAlignment);
            break;

        //
        // 279: MetadataInfo_AllocationTimestampDelta_Traits_Unused INT
        //

        case 279:
            RESULT_ULONG(MetadataInfo->AllocationTimestampDelta.Traits.Unused);
            break;

        //
        // 280: MetadataInfo_Synchronization_Eof_EndOfFile BIGINT
        //

        case 280:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Eof.EndOfFile);
            break;

        //
        // 281: MetadataInfo_Synchronization_Time_Frequency BIGINT
        //

        case 281:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Time.Frequency);
            break;

        //
        // 282: MetadataInfo_Synchronization_Time_Multiplicand BIGINT
        //

        case 282:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Time.Multiplicand);
            break;

        //
        // 283: MetadataInfo_Synchronization_Time_StartTime_FileTimeUtc BIGINT
        //

        case 283:
            RESULT_FILETIME(MetadataInfo->Synchronization.Time.StartTime.FileTimeUtc);
            break;

        //
        // 284: MetadataInfo_Synchronization_Time_StartTime_FileTimeLocal BIGINT
        //

        case 284:
            RESULT_FILETIME(MetadataInfo->Synchronization.Time.StartTime.FileTimeLocal);
            break;

        //
        // 285: MetadataInfo_Synchronization_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 285:
            RESULT_SYSTEMTIME(MetadataInfo->Synchronization.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 286: MetadataInfo_Synchronization_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 286:
            RESULT_SYSTEMTIME(MetadataInfo->Synchronization.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 287: MetadataInfo_Synchronization_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 287:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Time.StartTime.SecondsSince1970);
            break;

        //
        // 288: MetadataInfo_Synchronization_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 288:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 289: MetadataInfo_Synchronization_Time_StartTime_PerformanceCounter BIGINT
        //

        case 289:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Time.StartTime.PerformanceCounter);
            break;

        //
        // 290: MetadataInfo_Synchronization_Stats_DroppedRecords INT
        //

        case 290:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.DroppedRecords);
            break;

        //
        // 291: MetadataInfo_Synchronization_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 291:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 292: MetadataInfo_Synchronization_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 292:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 293: MetadataInfo_Synchronization_Stats_PreferredAddressUnavailable INT
        //

        case 293:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.PreferredAddressUnavailable);
            break;

        //
        // 294: MetadataInfo_Synchronization_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 294:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 295: MetadataInfo_Synchronization_Stats_BlockedAllocations INT
        //

        case 295:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.BlockedAllocations);
            break;

        //
        // 296: MetadataInfo_Synchronization_Stats_SuspendedAllocations INT
        //

        case 296:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.SuspendedAllocations);
            break;

        //
        // 297: MetadataInfo_Synchronization_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 297:
            RESULT_ULONG(MetadataInfo->Synchronization.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 298: MetadataInfo_Synchronization_Stats_WastedBytes BIGINT
        //

        case 298:
            RESULT_ULONGLONG(MetadataInfo->Synchronization.Stats.WastedBytes);
            break;

        //
        // 299: MetadataInfo_Synchronization_Stats_PaddedAllocations BIGINT
        //

        case 299:
            RESULT_ULONGLONG(MetadataInfo->Synchronization.Stats.PaddedAllocations);
            break;

        //
        // 300: MetadataInfo_Synchronization_Totals_NumberOfAllocations BIGINT
        //

        case 300:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Totals.NumberOfAllocations);
            break;

        //
        // 301: MetadataInfo_Synchronization_Totals_AllocationSize BIGINT
        //

        case 301:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Totals.AllocationSize);
            break;

        //
        // 302: MetadataInfo_Synchronization_Totals_NumberOfRecords BIGINT
        //

        case 302:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Totals.NumberOfRecords);
            break;

        //
        // 303: MetadataInfo_Synchronization_Totals_RecordSize BIGINT
        //

        case 303:
            RESULT_LARGE_INTEGER(MetadataInfo->Synchronization.Totals.RecordSize);
            break;

        //
        // 304: MetadataInfo_Synchronization_Traits_VaryingRecordSize INT
        //

        case 304:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.VaryingRecordSize);
            break;

        //
        // 305: MetadataInfo_Synchronization_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 305:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 306: MetadataInfo_Synchronization_Traits_MultipleRecords INT
        //

        case 306:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.MultipleRecords);
            break;

        //
        // 307: MetadataInfo_Synchronization_Traits_StreamingWrite INT
        //

        case 307:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.StreamingWrite);
            break;

        //
        // 308: MetadataInfo_Synchronization_Traits_StreamingRead INT
        //

        case 308:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.StreamingRead);
            break;

        //
        // 309: MetadataInfo_Synchronization_Traits_FrequentAllocations INT
        //

        case 309:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.FrequentAllocations);
            break;

        //
        // 310: MetadataInfo_Synchronization_Traits_BlockingAllocations INT
        //

        case 310:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.BlockingAllocations);
            break;

        //
        // 311: MetadataInfo_Synchronization_Traits_LinkedStore INT
        //

        case 311:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.LinkedStore);
            break;

        //
        // 312: MetadataInfo_Synchronization_Traits_CoalesceAllocations INT
        //

        case 312:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.CoalesceAllocations);
            break;

        //
        // 313: MetadataInfo_Synchronization_Traits_ConcurrentAllocations INT
        //

        case 313:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.ConcurrentAllocations);
            break;

        //
        // 314: MetadataInfo_Synchronization_Traits_AllowPageSpill INT
        //

        case 314:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.AllowPageSpill);
            break;

        //
        // 315: MetadataInfo_Synchronization_Traits_PageAligned INT
        //

        case 315:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.PageAligned);
            break;

        //
        // 316: MetadataInfo_Synchronization_Traits_Periodic INT
        //

        case 316:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.Periodic);
            break;

        //
        // 317: MetadataInfo_Synchronization_Traits_ConcurrentDataStructure INT
        //

        case 317:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.ConcurrentDataStructure);
            break;

        //
        // 318: MetadataInfo_Synchronization_Traits_NoAllocationAlignment INT
        //

        case 318:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.NoAllocationAlignment);
            break;

        //
        // 319: MetadataInfo_Synchronization_Traits_Unused INT
        //

        case 319:
            RESULT_ULONG(MetadataInfo->Synchronization.Traits.Unused);
            break;

        //
        // 320: MetadataInfo_Info_Eof_EndOfFile BIGINT
        //

        case 320:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Eof.EndOfFile);
            break;

        //
        // 321: MetadataInfo_Info_Time_Frequency BIGINT
        //

        case 321:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Time.Frequency);
            break;

        //
        // 322: MetadataInfo_Info_Time_Multiplicand BIGINT
        //

        case 322:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Time.Multiplicand);
            break;

        //
        // 323: MetadataInfo_Info_Time_StartTime_FileTimeUtc BIGINT
        //

        case 323:
            RESULT_FILETIME(MetadataInfo->Info.Time.StartTime.FileTimeUtc);
            break;

        //
        // 324: MetadataInfo_Info_Time_StartTime_FileTimeLocal BIGINT
        //

        case 324:
            RESULT_FILETIME(MetadataInfo->Info.Time.StartTime.FileTimeLocal);
            break;

        //
        // 325: MetadataInfo_Info_Time_StartTime_SystemTimeUtc BIGINT
        //

        case 325:
            RESULT_SYSTEMTIME(MetadataInfo->Info.Time.StartTime.SystemTimeUtc);
            break;

        //
        // 326: MetadataInfo_Info_Time_StartTime_SystemTimeLocal BIGINT
        //

        case 326:
            RESULT_SYSTEMTIME(MetadataInfo->Info.Time.StartTime.SystemTimeLocal);
            break;

        //
        // 327: MetadataInfo_Info_Time_StartTime_SecondsSince1970 BIGINT
        //

        case 327:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Time.StartTime.SecondsSince1970);
            break;

        //
        // 328: MetadataInfo_Info_Time_StartTime_MicrosecondsSince1970 BIGINT
        //

        case 328:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Time.StartTime.MicrosecondsSince1970);
            break;

        //
        // 329: MetadataInfo_Info_Time_StartTime_PerformanceCounter BIGINT
        //

        case 329:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Time.StartTime.PerformanceCounter);
            break;

        //
        // 330: MetadataInfo_Info_Stats_DroppedRecords INT
        //

        case 330:
            RESULT_ULONG(MetadataInfo->Info.Stats.DroppedRecords);
            break;

        //
        // 331: MetadataInfo_Info_Stats_ExhaustedFreeMemoryMaps INT
        //

        case 331:
            RESULT_ULONG(MetadataInfo->Info.Stats.ExhaustedFreeMemoryMaps);
            break;

        //
        // 332: MetadataInfo_Info_Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 332:
            RESULT_ULONG(MetadataInfo->Info.Stats.AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 333: MetadataInfo_Info_Stats_PreferredAddressUnavailable INT
        //

        case 333:
            RESULT_ULONG(MetadataInfo->Info.Stats.PreferredAddressUnavailable);
            break;

        //
        // 334: MetadataInfo_Info_Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 334:
            RESULT_ULONG(MetadataInfo->Info.Stats.AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 335: MetadataInfo_Info_Stats_BlockedAllocations INT
        //

        case 335:
            RESULT_ULONG(MetadataInfo->Info.Stats.BlockedAllocations);
            break;

        //
        // 336: MetadataInfo_Info_Stats_SuspendedAllocations INT
        //

        case 336:
            RESULT_ULONG(MetadataInfo->Info.Stats.SuspendedAllocations);
            break;

        //
        // 337: MetadataInfo_Info_Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 337:
            RESULT_ULONG(MetadataInfo->Info.Stats.ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 338: MetadataInfo_Info_Stats_WastedBytes BIGINT
        //

        case 338:
            RESULT_ULONGLONG(MetadataInfo->Info.Stats.WastedBytes);
            break;

        //
        // 339: MetadataInfo_Info_Stats_PaddedAllocations BIGINT
        //

        case 339:
            RESULT_ULONGLONG(MetadataInfo->Info.Stats.PaddedAllocations);
            break;

        //
        // 340: MetadataInfo_Info_Totals_NumberOfAllocations BIGINT
        //

        case 340:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Totals.NumberOfAllocations);
            break;

        //
        // 341: MetadataInfo_Info_Totals_AllocationSize BIGINT
        //

        case 341:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Totals.AllocationSize);
            break;

        //
        // 342: MetadataInfo_Info_Totals_NumberOfRecords BIGINT
        //

        case 342:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Totals.NumberOfRecords);
            break;

        //
        // 343: MetadataInfo_Info_Totals_RecordSize BIGINT
        //

        case 343:
            RESULT_LARGE_INTEGER(MetadataInfo->Info.Totals.RecordSize);
            break;

        //
        // 344: MetadataInfo_Info_Traits_VaryingRecordSize INT
        //

        case 344:
            RESULT_ULONG(MetadataInfo->Info.Traits.VaryingRecordSize);
            break;

        //
        // 345: MetadataInfo_Info_Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 345:
            RESULT_ULONG(MetadataInfo->Info.Traits.RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 346: MetadataInfo_Info_Traits_MultipleRecords INT
        //

        case 346:
            RESULT_ULONG(MetadataInfo->Info.Traits.MultipleRecords);
            break;

        //
        // 347: MetadataInfo_Info_Traits_StreamingWrite INT
        //

        case 347:
            RESULT_ULONG(MetadataInfo->Info.Traits.StreamingWrite);
            break;

        //
        // 348: MetadataInfo_Info_Traits_StreamingRead INT
        //

        case 348:
            RESULT_ULONG(MetadataInfo->Info.Traits.StreamingRead);
            break;

        //
        // 349: MetadataInfo_Info_Traits_FrequentAllocations INT
        //

        case 349:
            RESULT_ULONG(MetadataInfo->Info.Traits.FrequentAllocations);
            break;

        //
        // 350: MetadataInfo_Info_Traits_BlockingAllocations INT
        //

        case 350:
            RESULT_ULONG(MetadataInfo->Info.Traits.BlockingAllocations);
            break;

        //
        // 351: MetadataInfo_Info_Traits_LinkedStore INT
        //

        case 351:
            RESULT_ULONG(MetadataInfo->Info.Traits.LinkedStore);
            break;

        //
        // 352: MetadataInfo_Info_Traits_CoalesceAllocations INT
        //

        case 352:
            RESULT_ULONG(MetadataInfo->Info.Traits.CoalesceAllocations);
            break;

        //
        // 353: MetadataInfo_Info_Traits_ConcurrentAllocations INT
        //

        case 353:
            RESULT_ULONG(MetadataInfo->Info.Traits.ConcurrentAllocations);
            break;

        //
        // 354: MetadataInfo_Info_Traits_AllowPageSpill INT
        //

        case 354:
            RESULT_ULONG(MetadataInfo->Info.Traits.AllowPageSpill);
            break;

        //
        // 355: MetadataInfo_Info_Traits_PageAligned INT
        //

        case 355:
            RESULT_ULONG(MetadataInfo->Info.Traits.PageAligned);
            break;

        //
        // 356: MetadataInfo_Info_Traits_Periodic INT
        //

        case 356:
            RESULT_ULONG(MetadataInfo->Info.Traits.Periodic);
            break;

        //
        // 357: MetadataInfo_Info_Traits_ConcurrentDataStructure INT
        //

        case 357:
            RESULT_ULONG(MetadataInfo->Info.Traits.ConcurrentDataStructure);
            break;

        //
        // 358: MetadataInfo_Info_Traits_NoAllocationAlignment INT
        //

        case 358:
            RESULT_ULONG(MetadataInfo->Info.Traits.NoAllocationAlignment);
            break;

        //
        // 359: MetadataInfo_Info_Traits_Unused INT
        //

        case 359:
            RESULT_ULONG(MetadataInfo->Info.Traits.Unused);
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
// Allocation
//

CONST CHAR TraceStoreAllocationSchema[] =
    "CREATE TABLE Allocation ("
        "NumberOfRecords BIGINT, "   // NumberOfRecords
        "RecordSize BIGINT, "        // RecordSize
        "IsDummyAllocation INT"      // IsDummyAllocation
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
    BOOL IsDummyAllocation;
    LONGLONG NumberOfRecords;
    LONGLONG RecordSize;
    PTRACE_STORE_ALLOCATION Allocation;

    Allocation = Cursor->CurrentRow.AsAllocation;
    IsDummyAllocation = (BOOL)Allocation->NumberOfRecords.DummyAllocation2;
    NumberOfRecords = (ULONGLONG)Allocation->NumberOfRecords.SignedQuadPart;
    RecordSize = (LONGLONG)Allocation->RecordSize.QuadPart;

    if (RecordSize <= 0) {
        __debugbreak();
    }

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: NumberOfRecords BIGINT
        //

        case 0:
            RESULT_ULONGLONG(NumberOfRecords);
            break;

        //
        // 1: RecordSize BIGINT
        //

        case 1:
            RESULT_ULONGLONG(RecordSize);
            break;

        //
        // 2: IsDummyAllocation INT
        //

        case 2:
            RESULT_ULONG(IsDummyAllocation);
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
    "CREATE TABLE Info("
        "Eof_EndOfFile BIGINT, "                    // LARGE_INTEGER
        "Time_Frequency BIGINT, "                   // LARGE_INTEGER
        "Time_Multiplicand BIGINT, "                // LARGE_INTEGER
        "StartTime_FileTimeUtc BIGINT, "            // FILETIME
        "StartTime_FileTimeLocal BIGINT, "          // FILETIME
        "StartTime_SystemTimeUtc BIGINT, "          // SYSTEMTIME
        "StartTime_SystemTimeLocal BIGINT, "        // SYSTEMTIME
        "StartTime_SecondsSince1970 BIGINT, "       // LARGE_INTEGER
        "StartTime_MicrosecondsSince1970 BIGINT, "  // LARGE_INTEGER
        "StartTime_PerformanceCounter BIGINT, "     // LARGE_INTEGER
        "Stats_DroppedRecords INT, "
        "Stats_ExhaustedFreeMemoryMaps INT, "
        "Stats_AllocationsOutpacingNextMemoryMapPreparation INT, "
        "Stats_PreferredAddressUnavailable INT, "
        "Stats_AccessViolationsEncounteredDuringAsyncPrefault INT, "
        "Stats_BlockedAllocations INT, "
        "Stats_SuspendedAllocations INT, "
        "Stats_ElapsedSuspensionTimeInMicroseconds INT, "
        "Stats_WastedBytes BIGINT, "
        "Stats_PaddedAllocations BIGINT, "
        "Totals_NumberOfAllocations BIGINT, "       // LARGE_INTEGER
        "Totals_AllocationSize BIGINT, "            // LARGE_INTEGER
        "Totals_NumberOfRecords BIGINT, "           // LARGE_INTEGER
        "Totals_RecordSize BIGINT, "                // LARGE_INTEGER
        "Traits_VaryingRecordSize INT, "
        "Traits_RecordSizeIsAlwaysPowerOf2 INT, "
        "Traits_MultipleRecords INT, "
        "Traits_StreamingWrite INT, "
        "Traits_StreamingRead INT, "
        "Traits_FrequentAllocations INT, "
        "Traits_BlockingAllocations INT, "
        "Traits_LinkedStore INT, "
        "Traits_CoalesceAllocations INT, "
        "Traits_ConcurrentAllocations INT, "
        "Traits_AllowPageSpill INT, "
        "Traits_PageAligned INT, "
        "Traits_Periodic INT, "
        "Traits_ConcurrentDataStructure INT, "
        "Traits_NoAllocationAlignment INT, "
        "Traits_Unused INT, "
        "Intervals_FramesPerSecond DOUBLE, "                        // [(Intervals != NULL)]
        "Intervals_TicksPerIntervalAsDouble DOUBLE, "               // [(Intervals != NULL)]
        "Intervals_TicksPerInterval BIGINT, "                       // [(Intervals != NULL)]
        "Intervals_Frequency BIGINT, "                              // [(Intervals != NULL)]
        "Intervals_NumberOfIntervals BIGINT, "                      // [(Intervals != NULL)]
        "Intervals_IntervalExtractionTimeInMicroseconds BIGINT, "   // [(Intervals != NULL)]
        "Intervals_NumberOfRecords BIGINT, "                        // [(Intervals != NULL)]
        "Intervals_RecordSizeInBytes BIGINT, "                      // [(Intervals != NULL)]
        "Intervals_FirstAllocationTimestampAddress BIGINT, "        // Intervals->FirstAllocationTimestamp, [(Intervals != NULL)]
        "Intervals_FirstAllocationTimestamp BIGINT, "               // *Intervals->FirstAllocationTimestamp, [(Intervals != NULL)]
        "Intervals_LastAllocationTimestampAddress BIGINT, "         // Intervals->LastAllocationTimestamp, [(Intervals != NULL)]
        "Intervals_LastAllocationTimestamp BIGINT, "                // *Intervals->LastAllocationTimestamp, [(Intervals != NULL)]
        "Intervals_FirstRecordAddress BIGINT, "                     // [(Intervals != NULL)]
        "Intervals_LastRecordAddress BIGINT, "                      // [(Intervals != NULL)]
        "Intervals_FirstIntervalAddress BIGINT, "                   // Intervals->FirstInterval, [(Intervals != NULL)]
        "Intervals_LastIntervalAddress BIGINT"                      // Intervals->LastInterval, [(Intervals != NULL)]
    ")";

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
    PTRACE_STORE_INFO Info;
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TIME Time;
    PTRACE_STORE_START_TIME StartTime;
    PTRACE_STORE_STATS Stats;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_TRAITS Traits;
    PTRACE_STORE_INTERVALS Intervals;

    Info = (PTRACE_STORE_INFO)Cursor->CurrentRowRaw;
    Eof = &Info->Eof;
    Time = &Info->Time;
    StartTime = &Time->StartTime;
    Stats = &Info->Stats;
    Totals = &Info->Totals;
    Traits = &Info->Traits;
    Intervals = &TraceStore->TraceStore->Intervals;
    if (!Intervals->NumberOfRecords) {
        Intervals = NULL;
    }

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: Eof_EndOfFile BIGINT
        //

        case 0:
            RESULT_LARGE_INTEGER(Eof->EndOfFile);
            break;

        //
        // 1: Time_Frequency BIGINT
        //

        case 1:
            RESULT_LARGE_INTEGER(Time->Frequency);
            break;

        //
        // 2: Time_Multiplicand BIGINT
        //

        case 2:
            RESULT_LARGE_INTEGER(Time->Multiplicand);
            break;

        //
        // 3: StartTime_FileTimeUtc BIGINT
        //

        case 3:
            RESULT_FILETIME(StartTime->FileTimeUtc);
            break;

        //
        // 4: StartTime_FileTimeLocal BIGINT
        //

        case 4:
            RESULT_FILETIME(StartTime->FileTimeLocal);
            break;

        //
        // 5: StartTime_SystemTimeUtc BIGINT
        //

        case 5:
            RESULT_SYSTEMTIME(StartTime->SystemTimeUtc);
            break;

        //
        // 6: StartTime_SystemTimeLocal BIGINT
        //

        case 6:
            RESULT_SYSTEMTIME(StartTime->SystemTimeLocal);
            break;

        //
        // 7: StartTime_SecondsSince1970 BIGINT
        //

        case 7:
            RESULT_LARGE_INTEGER(StartTime->SecondsSince1970);
            break;

        //
        // 8: StartTime_MicrosecondsSince1970 BIGINT
        //

        case 8:
            RESULT_LARGE_INTEGER(StartTime->MicrosecondsSince1970);
            break;

        //
        // 9: StartTime_PerformanceCounter BIGINT
        //

        case 9:
            RESULT_LARGE_INTEGER(StartTime->PerformanceCounter);
            break;

        //
        // 10: Stats_DroppedRecords INT
        //

        case 10:
            RESULT_ULONG(Stats->DroppedRecords);
            break;

        //
        // 11: Stats_ExhaustedFreeMemoryMaps INT
        //

        case 11:
            RESULT_ULONG(Stats->ExhaustedFreeMemoryMaps);
            break;

        //
        // 12: Stats_AllocationsOutpacingNextMemoryMapPreparation INT
        //

        case 12:
            RESULT_ULONG(Stats->AllocationsOutpacingNextMemoryMapPreparation);
            break;

        //
        // 13: Stats_PreferredAddressUnavailable INT
        //

        case 13:
            RESULT_ULONG(Stats->PreferredAddressUnavailable);
            break;

        //
        // 14: Stats_AccessViolationsEncounteredDuringAsyncPrefault INT
        //

        case 14:
            RESULT_ULONG(Stats->AccessViolationsEncounteredDuringAsyncPrefault);
            break;

        //
        // 15: Stats_BlockedAllocations INT
        //

        case 15:
            RESULT_ULONG(Stats->BlockedAllocations);
            break;

        //
        // 16: Stats_SuspendedAllocations INT
        //

        case 16:
            RESULT_ULONG(Stats->SuspendedAllocations);
            break;

        //
        // 17: Stats_ElapsedSuspensionTimeInMicroseconds INT
        //

        case 17:
            RESULT_ULONG(Stats->ElapsedSuspensionTimeInMicroseconds);
            break;

        //
        // 18: Stats_WastedBytes BIGINT
        //

        case 18:
            RESULT_ULONGLONG(Stats->WastedBytes);
            break;

        //
        // 19: Stats_PaddedAllocations BIGINT
        //

        case 19:
            RESULT_ULONGLONG(Stats->PaddedAllocations);
            break;

        //
        // 20: Totals_NumberOfAllocations BIGINT
        //

        case 20:
            RESULT_LARGE_INTEGER(Totals->NumberOfAllocations);
            break;

        //
        // 21: Totals_AllocationSize BIGINT
        //

        case 21:
            RESULT_LARGE_INTEGER(Totals->AllocationSize);
            break;

        //
        // 22: Totals_NumberOfRecords BIGINT
        //

        case 22:
            RESULT_LARGE_INTEGER(Totals->NumberOfRecords);
            break;

        //
        // 23: Totals_RecordSize BIGINT
        //

        case 23:
            RESULT_LARGE_INTEGER(Totals->RecordSize);
            break;

        //
        // 24: Traits_VaryingRecordSize INT
        //

        case 24:
            RESULT_ULONG(Traits->VaryingRecordSize);
            break;

        //
        // 25: Traits_RecordSizeIsAlwaysPowerOf2 INT
        //

        case 25:
            RESULT_ULONG(Traits->RecordSizeIsAlwaysPowerOf2);
            break;

        //
        // 26: Traits_MultipleRecords INT
        //

        case 26:
            RESULT_ULONG(Traits->MultipleRecords);
            break;

        //
        // 27: Traits_StreamingWrite INT
        //

        case 27:
            RESULT_ULONG(Traits->StreamingWrite);
            break;

        //
        // 28: Traits_StreamingRead INT
        //

        case 28:
            RESULT_ULONG(Traits->StreamingRead);
            break;

        //
        // 29: Traits_FrequentAllocations INT
        //

        case 29:
            RESULT_ULONG(Traits->FrequentAllocations);
            break;

        //
        // 30: Traits_BlockingAllocations INT
        //

        case 30:
            RESULT_ULONG(Traits->BlockingAllocations);
            break;

        //
        // 31: Traits_LinkedStore INT
        //

        case 31:
            RESULT_ULONG(Traits->LinkedStore);
            break;

        //
        // 32: Traits_CoalesceAllocations INT
        //

        case 32:
            RESULT_ULONG(Traits->CoalesceAllocations);
            break;

        //
        // 33: Traits_ConcurrentAllocations INT
        //

        case 33:
            RESULT_ULONG(Traits->ConcurrentAllocations);
            break;

        //
        // 34: Traits_AllowPageSpill INT
        //

        case 34:
            RESULT_ULONG(Traits->AllowPageSpill);
            break;

        //
        // 35: Traits_PageAligned INT
        //

        case 35:
            RESULT_ULONG(Traits->PageAligned);
            break;

        //
        // 36: Traits_Periodic INT
        //

        case 36:
            RESULT_ULONG(Traits->Periodic);
            break;

        //
        // 37: Traits_ConcurrentDataStructure INT
        //

        case 37:
            RESULT_ULONG(Traits->ConcurrentDataStructure);
            break;

        //
        // 38: Traits_NoAllocationAlignment INT
        //

        case 38:
            RESULT_ULONG(Traits->NoAllocationAlignment);
            break;

        //
        // 39: Traits_Unused INT
        //

        case 39:
            RESULT_ULONG(Traits->Unused);
            break;

        //
        // 40: Intervals_FramesPerSecond DOUBLE
        //

        case 40:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_DOUBLE((DOUBLE)Intervals->FramesPerSecond);
            }
            break;

        //
        // 41: Intervals_TicksPerIntervalAsDouble DOUBLE
        //

        case 41:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_DOUBLE((DOUBLE)Intervals->TicksPerIntervalAsDouble);
            }
            break;

        //
        // 42: Intervals_TicksPerInterval BIGINT
        //

        case 42:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->TicksPerInterval);
            }
            break;

        //
        // 43: Intervals_Frequency BIGINT
        //

        case 43:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->Frequency);
            }
            break;

        //
        // 44: Intervals_NumberOfIntervals BIGINT
        //

        case 44:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->NumberOfIntervals);
            }
            break;

        //
        // 45: Intervals_IntervalExtractionTimeInMicroseconds BIGINT
        //

        case 45:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->IntervalExtractionTimeInMicroseconds);
            }
            break;

        //
        // 46: Intervals_NumberOfRecords BIGINT
        //

        case 46:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->NumberOfRecords);
            }
            break;

        //
        // 47: Intervals_RecordSizeInBytes BIGINT
        //

        case 47:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->RecordSizeInBytes);
            }
            break;

        //
        // 48: Intervals_FirstAllocationTimestampAddress BIGINT
        //

        case 48:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->FirstAllocationTimestamp);
            }
            break;

        //
        // 49: Intervals_FirstAllocationTimestamp BIGINT
        //

        case 49:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(*Intervals->FirstAllocationTimestamp);
            }
            break;

        //
        // 50: Intervals_LastAllocationTimestampAddress BIGINT
        //

        case 50:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->LastAllocationTimestamp);
            }
            break;

        //
        // 51: Intervals_LastAllocationTimestamp BIGINT
        //

        case 51:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(*Intervals->LastAllocationTimestamp);
            }
            break;

        //
        // 52: Intervals_FirstRecordAddress BIGINT
        //

        case 52:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->FirstRecordAddress);
            }
            break;

        //
        // 53: Intervals_LastRecordAddress BIGINT
        //

        case 53:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->LastRecordAddress);
            }
            break;

        //
        // 54: Intervals_FirstIntervalAddress BIGINT
        //

        case 54:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->FirstInterval);
            }
            break;

        //
        // 55: Intervals_LastIntervalAddress BIGINT
        //

        case 55:
            if (!((Intervals != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Intervals->LastInterval);
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

CONST CHAR TraceStoreWsWorkingSetExInfoSchema[] =
    "CREATE TABLE TraceStore_WsWorkingSetExInfo("
        "VirtualAddress BIGINT, "
        "VirtualAttributes BIGINT, "    // VirtualAttributes.Flags
        "Valid BIGINT, "                // VirtualAttributes.Valid
        "ShareCount BIGINT, "           // VirtualAttributes.ShareCount
        "Win32Protection BIGINT, "      // VirtualAttributes.Win32Protection
        "Shared BIGINT, "               // VirtualAttributes.Shared
        "Node BIGINT, "                 // VirtualAttributes.Node
        "Locked BIGINT, "               // VirtualAttributes.Locked
        "LargePage BIGINT, "            // VirtualAttributes.LargePage
        "Reserved BIGINT, "             // VirtualAttributes.Reserved
        "Bad BIGINT"                    // VirtualAttributes.Bad
    ");";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3WsWorkingSetExInfoColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3WsWorkingSetExInfoColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PPSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfo;
    PSAPI_WORKING_SET_EX_BLOCK VirtualAttributes;


    WsWorkingSetExInfo = (PPSAPI_WORKING_SET_EX_INFORMATION)(
        Cursor->CurrentRowRaw
    );

    VirtualAttributes.Flags = WsWorkingSetExInfo->VirtualAttributes.Flags;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: VirtualAddress BIGINT
        //

        case 0:
            RESULT_ULONGLONG(WsWorkingSetExInfo->VirtualAddress);
            break;

        //
        // 1: VirtualAttributes BIGINT
        //

        case 1:
            RESULT_ULONGLONG(VirtualAttributes.Flags);
            break;

        //
        // 2: Valid BIGINT
        //

        case 2:
            RESULT_ULONGLONG(VirtualAttributes.Valid);
            break;

        //
        // 3: ShareCount BIGINT
        //

        case 3:
            RESULT_ULONGLONG(VirtualAttributes.ShareCount);
            break;

        //
        // 4: Win32Protection BIGINT
        //

        case 4:
            RESULT_ULONGLONG(VirtualAttributes.Win32Protection);
            break;

        //
        // 5: Shared BIGINT
        //

        case 5:
            RESULT_ULONGLONG(VirtualAttributes.Shared);
            break;

        //
        // 6: Node BIGINT
        //

        case 6:
            RESULT_ULONGLONG(VirtualAttributes.Node);
            break;

        //
        // 7: Locked BIGINT
        //

        case 7:
            RESULT_ULONGLONG(VirtualAttributes.Locked);
            break;

        //
        // 8: LargePage BIGINT
        //

        case 8:
            RESULT_ULONGLONG(VirtualAttributes.LargePage);
            break;

        //
        // 9: Reserved BIGINT
        //

        case 9:
            RESULT_ULONGLONG(VirtualAttributes.Reserved);
            break;

        //
        // 10: Bad BIGINT
        //

        case 10:
            RESULT_ULONGLONG(VirtualAttributes.Bad);
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
// TraceStore_Performance
//

CONST CHAR TraceStorePerformanceSchema[] =
    "CREATE TABLE TraceStore_Performance("
        "SizeOfStruct INT, "
        "ProcessHandleCount INT, "
        "IntervalInMilliseconds INT, "
        "WindowLengthInMilliseconds INT, "
        "Timestamp BIGINT, "                    // LARGE_INTEGER
        "UserTime BIGINT, "                     // FILETIME
        "KernelTime BIGINT, "                   // FILETIME
        "ProcessCycles BIGINT, "
        "ProcessMemoryCountersExSize INT, "
        "PageFaultCount INT, "
        "PeakWorkingSetSize BIGINT, "
        "WorkingSetSize BIGINT, "
        "QuotaPeakPagedPoolUsage BIGINT, "
        "QuotaPagedPoolUsage BIGINT, "
        "QuotaPeakNonPagedPoolUsage BIGINT, "
        "QuotaNonPagedPoolUsage BIGINT, "
        "PagefileUsage BIGINT, "
        "PeakPagefileUsage BIGINT, "
        "PrivateUsage BIGINT, "
        "MemoryLoad INT, "
        "TotalPhys BIGINT, "
        "AvailPhys BIGINT, "
        "TotalPageFile BIGINT, "
        "AvailPageFile BIGINT, "
        "TotalVirtual BIGINT, "
        "AvailVirtual BIGINT, "
        "AvailExtendedVirtual BIGINT, "
        "ReadOperationCount BIGINT, "
        "WriteOperationCount BIGINT, "
        "OtherOperationCount BIGINT, "
        "ReadTransferCount BIGINT, "
        "WriteTransferCount BIGINT, "
        "OtherTransferCount BIGINT, "
        "CommitTotal BIGINT, "
        "CommitLimit BIGINT, "
        "CommitPeak BIGINT, "
        "PhysicalTotal BIGINT, "
        "PhysicalAvailable BIGINT, "
        "SystemCache BIGINT, "
        "KernelTotal BIGINT, "
        "KernelPaged BIGINT, "
        "KernelNonpaged BIGINT, "
        "PageSize BIGINT, "
        "HandleCount INT, "
        "ProcessCount INT, "
        "ThreadCount INT"
    ")";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3PerformanceColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3PerformanceColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_PERFORMANCE Performance;

    Performance = (PTRACE_PERFORMANCE)Cursor->CurrentRowRaw;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: SizeOfStruct INT
        //

        case 0:
            RESULT_ULONG(Performance->SizeOfStruct);
            break;

        //
        // 1: ProcessHandleCount INT
        //

        case 1:
            RESULT_ULONG(Performance->ProcessHandleCount);
            break;

        //
        // 2: IntervalInMilliseconds INT
        //

        case 2:
            RESULT_ULONG(Performance->IntervalInMilliseconds);
            break;

        //
        // 3: WindowLengthInMilliseconds INT
        //

        case 3:
            RESULT_ULONG(Performance->WindowLengthInMilliseconds);
            break;

        //
        // 4: Timestamp BIGINT
        //

        case 4:
            RESULT_LARGE_INTEGER(Performance->Timestamp);
            break;

        //
        // 5: UserTime BIGINT
        //

        case 5:
            RESULT_FILETIME(Performance->UserTime);
            break;

        //
        // 6: KernelTime BIGINT
        //

        case 6:
            RESULT_FILETIME(Performance->KernelTime);
            break;

        //
        // 7: ProcessCycles BIGINT
        //

        case 7:
            RESULT_ULONGLONG(Performance->ProcessCycles);
            break;

        //
        // 8: ProcessMemoryCountersExSize INT
        //

        case 8:
            RESULT_ULONG(Performance->ProcessMemoryCountersExSize);
            break;

        //
        // 9: PageFaultCount INT
        //

        case 9:
            RESULT_ULONG(Performance->PageFaultCount);
            break;

        //
        // 10: PeakWorkingSetSize BIGINT
        //

        case 10:
            RESULT_ULONGLONG(Performance->PeakWorkingSetSize);
            break;

        //
        // 11: WorkingSetSize BIGINT
        //

        case 11:
            RESULT_ULONGLONG(Performance->WorkingSetSize);
            break;

        //
        // 12: QuotaPeakPagedPoolUsage BIGINT
        //

        case 12:
            RESULT_ULONGLONG(Performance->QuotaPeakPagedPoolUsage);
            break;

        //
        // 13: QuotaPagedPoolUsage BIGINT
        //

        case 13:
            RESULT_ULONGLONG(Performance->QuotaPagedPoolUsage);
            break;

        //
        // 14: QuotaPeakNonPagedPoolUsage BIGINT
        //

        case 14:
            RESULT_ULONGLONG(Performance->QuotaPeakNonPagedPoolUsage);
            break;

        //
        // 15: QuotaNonPagedPoolUsage BIGINT
        //

        case 15:
            RESULT_ULONGLONG(Performance->QuotaNonPagedPoolUsage);
            break;

        //
        // 16: PagefileUsage BIGINT
        //

        case 16:
            RESULT_ULONGLONG(Performance->PagefileUsage);
            break;

        //
        // 17: PeakPagefileUsage BIGINT
        //

        case 17:
            RESULT_ULONGLONG(Performance->PeakPagefileUsage);
            break;

        //
        // 18: PrivateUsage BIGINT
        //

        case 18:
            RESULT_ULONGLONG(Performance->PrivateUsage);
            break;

        //
        // 19: MemoryLoad INT
        //

        case 19:
            RESULT_ULONG(Performance->MemoryLoad);
            break;

        //
        // 20: TotalPhys BIGINT
        //

        case 20:
            RESULT_ULONGLONG(Performance->TotalPhys);
            break;

        //
        // 21: AvailPhys BIGINT
        //

        case 21:
            RESULT_ULONGLONG(Performance->AvailPhys);
            break;

        //
        // 22: TotalPageFile BIGINT
        //

        case 22:
            RESULT_ULONGLONG(Performance->TotalPageFile);
            break;

        //
        // 23: AvailPageFile BIGINT
        //

        case 23:
            RESULT_ULONGLONG(Performance->AvailPageFile);
            break;

        //
        // 24: TotalVirtual BIGINT
        //

        case 24:
            RESULT_ULONGLONG(Performance->TotalVirtual);
            break;

        //
        // 25: AvailVirtual BIGINT
        //

        case 25:
            RESULT_ULONGLONG(Performance->AvailVirtual);
            break;

        //
        // 26: AvailExtendedVirtual BIGINT
        //

        case 26:
            RESULT_ULONGLONG(Performance->AvailExtendedVirtual);
            break;

        //
        // 27: ReadOperationCount BIGINT
        //

        case 27:
            RESULT_ULONGLONG(Performance->ReadOperationCount);
            break;

        //
        // 28: WriteOperationCount BIGINT
        //

        case 28:
            RESULT_ULONGLONG(Performance->WriteOperationCount);
            break;

        //
        // 29: OtherOperationCount BIGINT
        //

        case 29:
            RESULT_ULONGLONG(Performance->OtherOperationCount);
            break;

        //
        // 30: ReadTransferCount BIGINT
        //

        case 30:
            RESULT_ULONGLONG(Performance->ReadTransferCount);
            break;

        //
        // 31: WriteTransferCount BIGINT
        //

        case 31:
            RESULT_ULONGLONG(Performance->WriteTransferCount);
            break;

        //
        // 32: OtherTransferCount BIGINT
        //

        case 32:
            RESULT_ULONGLONG(Performance->OtherTransferCount);
            break;

        //
        // 33: CommitTotal BIGINT
        //

        case 33:
            RESULT_ULONGLONG(Performance->CommitTotal);
            break;

        //
        // 34: CommitLimit BIGINT
        //

        case 34:
            RESULT_ULONGLONG(Performance->CommitLimit);
            break;

        //
        // 35: CommitPeak BIGINT
        //

        case 35:
            RESULT_ULONGLONG(Performance->CommitPeak);
            break;

        //
        // 36: PhysicalTotal BIGINT
        //

        case 36:
            RESULT_ULONGLONG(Performance->PhysicalTotal);
            break;

        //
        // 37: PhysicalAvailable BIGINT
        //

        case 37:
            RESULT_ULONGLONG(Performance->PhysicalAvailable);
            break;

        //
        // 38: SystemCache BIGINT
        //

        case 38:
            RESULT_ULONGLONG(Performance->SystemCache);
            break;

        //
        // 39: KernelTotal BIGINT
        //

        case 39:
            RESULT_ULONGLONG(Performance->KernelTotal);
            break;

        //
        // 40: KernelPaged BIGINT
        //

        case 40:
            RESULT_ULONGLONG(Performance->KernelPaged);
            break;

        //
        // 41: KernelNonpaged BIGINT
        //

        case 41:
            RESULT_ULONGLONG(Performance->KernelNonpaged);
            break;

        //
        // 42: PageSize BIGINT
        //

        case 42:
            RESULT_ULONGLONG(Performance->PageSize);
            break;

        //
        // 43: HandleCount INT
        //

        case 43:
            RESULT_ULONG(Performance->HandleCount);
            break;

        //
        // 44: ProcessCount INT
        //

        case 44:
            RESULT_ULONG(Performance->ProcessCount);
            break;

        //
        // 45: ThreadCount INT
        //

        case 45:
            RESULT_ULONG(Performance->ThreadCount);
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
// TraceStore_PerformanceDelta
//

CONST CHAR TraceStorePerformanceDeltaSchema[] =
    "CREATE TABLE TraceStore_PerformanceDelta("
        "ProcessHandleCountDelta INT, "
        "PageFaultCountDelta INT, "
        "PeakWorkingSetSizeDelta BIGINT, "
        "WorkingSetSizeDelta BIGINT, "
        "QuotaPeakPagedPoolUsageDelta BIGINT, "
        "QuotaPagedPoolUsageDelta BIGINT, "
        "QuotaPeakNonPagedPoolUsageDelta BIGINT, "
        "QuotaNonPagedPoolUsageDelta BIGINT, "
        "PagefileUsageDelta BIGINT, "
        "PeakPagefileUsageDelta BIGINT, "
        "PrivateUsageDelta BIGINT, "
        "MemoryLoadDelta INT, "
        "TotalPhysDelta BIGINT, "
        "AvailPhysDelta BIGINT, "
        "TotalPageFileDelta BIGINT, "
        "AvailPageFileDelta BIGINT, "
        "TotalVirtualDelta BIGINT, "
        "AvailVirtualDelta BIGINT, "
        "AvailExtendedVirtualDelta BIGINT, "
        "ReadOperationCountDelta BIGINT, "
        "WriteOperationCountDelta BIGINT, "
        "OtherOperationCountDelta BIGINT, "
        "ReadTransferCountDelta BIGINT, "
        "WriteTransferCountDelta BIGINT, "
        "OtherTransferCountDelta BIGINT, "
        "CommitTotalDelta BIGINT, "
        "CommitLimitDelta BIGINT, "
        "CommitPeakDelta BIGINT, "
        "PhysicalTotalDelta BIGINT, "
        "PhysicalAvailableDelta BIGINT, "
        "SystemCacheDelta BIGINT, "
        "KernelTotalDelta BIGINT, "
        "KernelPagedDelta BIGINT, "
        "KernelNonpagedDelta BIGINT, "
        "PageSizeDelta BIGINT, "
        "HandleCountDelta INT, "
        "ProcessCountDelta INT, "
        "ThreadCountDelta INT"
    ")";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3PerformanceDeltaColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3PerformanceDeltaColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_PERFORMANCE PerformanceDelta;

    PerformanceDelta = (PTRACE_PERFORMANCE)Cursor->CurrentRowRaw;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: ProcessHandleCountDelta INT
        //

        case 0:
            RESULT_ULONG(PerformanceDelta->ProcessHandleCountDelta);
            break;

        //
        // 1: PageFaultCountDelta INT
        //

        case 1:
            RESULT_ULONG(PerformanceDelta->PageFaultCountDelta);
            break;

        //
        // 2: PeakWorkingSetSizeDelta BIGINT
        //

        case 2:
            RESULT_ULONGLONG(PerformanceDelta->PeakWorkingSetSizeDelta);
            break;

        //
        // 3: WorkingSetSizeDelta BIGINT
        //

        case 3:
            RESULT_ULONGLONG(PerformanceDelta->WorkingSetSizeDelta);
            break;

        //
        // 4: QuotaPeakPagedPoolUsageDelta BIGINT
        //

        case 4:
            RESULT_ULONGLONG(PerformanceDelta->QuotaPeakPagedPoolUsageDelta);
            break;

        //
        // 5: QuotaPagedPoolUsageDelta BIGINT
        //

        case 5:
            RESULT_ULONGLONG(PerformanceDelta->QuotaPagedPoolUsageDelta);
            break;

        //
        // 6: QuotaPeakNonPagedPoolUsageDelta BIGINT
        //

        case 6:
            RESULT_ULONGLONG(PerformanceDelta->QuotaPeakNonPagedPoolUsageDelta);
            break;

        //
        // 7: QuotaNonPagedPoolUsageDelta BIGINT
        //

        case 7:
            RESULT_ULONGLONG(PerformanceDelta->QuotaNonPagedPoolUsageDelta);
            break;

        //
        // 8: PagefileUsageDelta BIGINT
        //

        case 8:
            RESULT_ULONGLONG(PerformanceDelta->PagefileUsageDelta);
            break;

        //
        // 9: PeakPagefileUsageDelta BIGINT
        //

        case 9:
            RESULT_ULONGLONG(PerformanceDelta->PeakPagefileUsageDelta);
            break;

        //
        // 10: PrivateUsageDelta BIGINT
        //

        case 10:
            RESULT_ULONGLONG(PerformanceDelta->PrivateUsageDelta);
            break;

        //
        // 11: MemoryLoadDelta INT
        //

        case 11:
            RESULT_ULONG(PerformanceDelta->MemoryLoadDelta);
            break;

        //
        // 12: TotalPhysDelta BIGINT
        //

        case 12:
            RESULT_ULONGLONG(PerformanceDelta->TotalPhysDelta);
            break;

        //
        // 13: AvailPhysDelta BIGINT
        //

        case 13:
            RESULT_ULONGLONG(PerformanceDelta->AvailPhysDelta);
            break;

        //
        // 14: TotalPageFileDelta BIGINT
        //

        case 14:
            RESULT_ULONGLONG(PerformanceDelta->TotalPageFileDelta);
            break;

        //
        // 15: AvailPageFileDelta BIGINT
        //

        case 15:
            RESULT_ULONGLONG(PerformanceDelta->AvailPageFileDelta);
            break;

        //
        // 16: TotalVirtualDelta BIGINT
        //

        case 16:
            RESULT_ULONGLONG(PerformanceDelta->TotalVirtualDelta);
            break;

        //
        // 17: AvailVirtualDelta BIGINT
        //

        case 17:
            RESULT_ULONGLONG(PerformanceDelta->AvailVirtualDelta);
            break;

        //
        // 18: AvailExtendedVirtualDelta BIGINT
        //

        case 18:
            RESULT_ULONGLONG(PerformanceDelta->AvailExtendedVirtualDelta);
            break;

        //
        // 19: ReadOperationCountDelta BIGINT
        //

        case 19:
            RESULT_ULONGLONG(PerformanceDelta->ReadOperationCountDelta);
            break;

        //
        // 20: WriteOperationCountDelta BIGINT
        //

        case 20:
            RESULT_ULONGLONG(PerformanceDelta->WriteOperationCountDelta);
            break;

        //
        // 21: OtherOperationCountDelta BIGINT
        //

        case 21:
            RESULT_ULONGLONG(PerformanceDelta->OtherOperationCountDelta);
            break;

        //
        // 22: ReadTransferCountDelta BIGINT
        //

        case 22:
            RESULT_ULONGLONG(PerformanceDelta->ReadTransferCountDelta);
            break;

        //
        // 23: WriteTransferCountDelta BIGINT
        //

        case 23:
            RESULT_ULONGLONG(PerformanceDelta->WriteTransferCountDelta);
            break;

        //
        // 24: OtherTransferCountDelta BIGINT
        //

        case 24:
            RESULT_ULONGLONG(PerformanceDelta->OtherTransferCountDelta);
            break;

        //
        // 25: CommitTotalDelta BIGINT
        //

        case 25:
            RESULT_ULONGLONG(PerformanceDelta->CommitTotalDelta);
            break;

        //
        // 26: CommitLimitDelta BIGINT
        //

        case 26:
            RESULT_ULONGLONG(PerformanceDelta->CommitLimitDelta);
            break;

        //
        // 27: CommitPeakDelta BIGINT
        //

        case 27:
            RESULT_ULONGLONG(PerformanceDelta->CommitPeakDelta);
            break;

        //
        // 28: PhysicalTotalDelta BIGINT
        //

        case 28:
            RESULT_ULONGLONG(PerformanceDelta->PhysicalTotalDelta);
            break;

        //
        // 29: PhysicalAvailableDelta BIGINT
        //

        case 29:
            RESULT_ULONGLONG(PerformanceDelta->PhysicalAvailableDelta);
            break;

        //
        // 30: SystemCacheDelta BIGINT
        //

        case 30:
            RESULT_ULONGLONG(PerformanceDelta->SystemCacheDelta);
            break;

        //
        // 31: KernelTotalDelta BIGINT
        //

        case 31:
            RESULT_ULONGLONG(PerformanceDelta->KernelTotalDelta);
            break;

        //
        // 32: KernelPagedDelta BIGINT
        //

        case 32:
            RESULT_ULONGLONG(PerformanceDelta->KernelPagedDelta);
            break;

        //
        // 33: KernelNonpagedDelta BIGINT
        //

        case 33:
            RESULT_ULONGLONG(PerformanceDelta->KernelNonpagedDelta);
            break;

        //
        // 34: PageSizeDelta BIGINT
        //

        case 34:
            RESULT_ULONGLONG(PerformanceDelta->PageSizeDelta);
            break;

        //
        // 35: HandleCountDelta INT
        //

        case 35:
            RESULT_ULONG(PerformanceDelta->HandleCountDelta);
            break;

        //
        // 36: ProcessCountDelta INT
        //

        case 36:
            RESULT_ULONG(PerformanceDelta->ProcessCountDelta);
            break;

        //
        // 37: ThreadCountDelta INT
        //

        case 37:
            RESULT_ULONG(PerformanceDelta->ThreadCountDelta);
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
// TraceStore_WsWatchInfoEx
//

CONST CHAR TraceStoreWsWatchInfoExSchema[] =
    "CREATE TABLE TraceStore_WsWatchInfoEx("
        "FaultingPc BIGINT, "
        "FaultingVa BIGINT, "
        "FaultingThreadId BIGINT, "
        "Flags BIGINT"
    ");";


TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3WsWatchInfoExColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3WsWatchInfoExColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_WS_WATCH_INFORMATION_EX WsWatchInfoEx;

    WsWatchInfoEx = (PTRACE_WS_WATCH_INFORMATION_EX)(
        Cursor->CurrentRowRaw
    );

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: FaultingPc BIGINT
        //

        case 0:
            RESULT_ULONGLONG(WsWatchInfoEx->FaultingPc);
            break;

        //
        // 1: FaultingVa BIGINT
        //

        case 1:
            RESULT_ULONGLONG(WsWatchInfoEx->FaultingVa);
            break;

        //
        // 2: FaultingThreadId BIGINT
        //

        case 2:
            RESULT_ULONGLONG(WsWatchInfoEx->FaultingThreadId);
            break;

        //
        // 3: Flags BIGINT
        //

        case 3:
            RESULT_ULONGLONG(WsWatchInfoEx->Flags);
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
// TraceStore_ExaminedSymbol.
//

CONST CHAR TraceStoreExaminedSymbolSchema[] =
    "CREATE TABLE ExaminedSymbol("
        "SizeOfStruct INT, "
        "Flags INT, "                                   // Flags.AsULong
        "Flags_IsPointer TINYINT, "                     // Flags.IsPointer
        "Flags_IsArray TINYINT, "                       // Flags.IsArray
        "Flags_IsCpp TINYINT, "                         // Flags.IsCpp
        "Flags_IsUnknownType TINYINT, "                 // Flags.UnknownType
        "Type INT, "                                    // Type
        "Type_UnknownType TINYINT, "                    // (Type == UnknownType)
        "Type_NoType TINYINT, "                         // (Type == NoType)
        "Type_FunctionType TINYINT, "                   // (Type == FunctionType)
        "Type_CharType TINYINT, "                       // (Type == CharType)
        "Type_WideCharType TINYINT, "                   // (Type == WideCharType)
        "Type_ShortType TINYINT, "                      // (Type == ShortType)
        "Type_LongType TINYINT, "                       // (Type == LongType)
        "Type_Integer64Type TINYINT, "                  // (Type == Integer64Type)
        "Type_IntegerType TINYINT, "                    // (Type == IntegerType)
        "Type_UnsignedCharType TINYINT, "               // (Type == UnsignedCharType)
        "Type_UnsignedWideCharType TINYINT, "           // (Type == UnsignedWideCharType)
        "Type_UnsignedShortType TINYINT, "              // (Type == UnsignedShortType)
        "Type_UnsignedLongType TINYINT, "               // (Type == UnsignedLongType)
        "Type_InlineCallerType TINYINT, "               // (Type == InlineCallerType)
        "Scope INT, "                                   // Scope
        "Scope_PrivateGlobalScope TINYINT, "            // (Scope == PrivateGlobalScope)
        "Scope_PrivateInlineScope TINYINT, "            // (Scope == PrivateInlineScope)
        "Scope_PrivateFunctionScope TINYINT, "          // (Scope == PrivateFunctionScope)
        "Scope_PublicGlobalScope TINYINT, "             // (Scope == PublicGlobalScope)
        "Scope_PublicFunctionScope TINYINT, "           // (Scope == PublicFunctionScope)
        "Address BIGINT, "                              // LARGE_INTEGER
        "Strings_Line TEXT, "                           // Strings->Line, STRING
        "Strings_Scope TEXT, "                          // Strings->Scope, STRING
        "Strings_Address TEXT, "                        // Strings->Address, STRING
        "Strings_Size TEXT, "                           // Strings->Size, STRING
        "Strings_BasicType TEXT, "                      // Strings->BasicType, STRING
        "Strings_TypeName TEXT, "                       // Strings->TypeName, STRING
        "Strings_Array TEXT, "                          // Strings->Array, STRING
        "Strings_ModuleName TEXT, "                     // Strings->ModuleName, STRING
        "Strings_SymbolName TEXT, "                     // Strings->SymbolName, STRING
        "Strings_Remaining TEXT, "                      // Strings->Remaining, STRING
        "Strings_Value TEXT, "                          // Strings->Value, STRING
        "Function_NumberOfArguments INT"                // [(Function != NULL)]
    ")";

_Use_decl_annotations_
LONG
TraceStoreSqlite3ExaminedSymbolColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE Type;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE Scope;
    DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS Flags;
    PDEBUG_ENGINE_EXAMINED_SYMBOL ExaminedSymbol;
    PDEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS Strings;
    PDEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION Function;

    ExaminedSymbol = (PDEBUG_ENGINE_EXAMINED_SYMBOL)Cursor->CurrentRowRaw;

    if (ExaminedSymbol->SizeOfStruct != sizeof(*ExaminedSymbol)) {
        __debugbreak();
    }

    Type = ExaminedSymbol->Type;
    Flags = ExaminedSymbol->Flags;
    Scope = ExaminedSymbol->Scope;
    Strings = &ExaminedSymbol->Strings;

    if (Type == FunctionType) {
        Function = &ExaminedSymbol->Function;
    } else {
        Function = NULL;
    }

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: SizeOfStruct INT
        //

        case 0:
            RESULT_ULONG(ExaminedSymbol->SizeOfStruct);
            break;

        //
        // 1: Flags INT
        //

        case 1:
            RESULT_ULONG(Flags.AsULong);
            break;

        //
        // 2: Flags_IsPointer TINYINT
        //

        case 2:
            RESULT_ULONG(Flags.IsPointer);
            break;

        //
        // 3: Flags_IsArray TINYINT
        //

        case 3:
            RESULT_ULONG(Flags.IsArray);
            break;

        //
        // 4: Flags_IsCpp TINYINT
        //

        case 4:
            RESULT_ULONG(Flags.IsCpp);
            break;

        //
        // 5: Flags_IsUnknownType TINYINT
        //

        case 5:
            RESULT_ULONG(Flags.UnknownType);
            break;

        //
        // 6: Type INT
        //

        case 6:
            RESULT_ULONG(Type);
            break;

        //
        // 7: Type_UnknownType TINYINT
        //

        case 7:
            RESULT_ULONG((Type == UnknownType));
            break;

        //
        // 8: Type_NoType TINYINT
        //

        case 8:
            RESULT_ULONG((Type == NoType));
            break;

        //
        // 9: Type_FunctionType TINYINT
        //

        case 9:
            RESULT_ULONG((Type == FunctionType));
            break;

        //
        // 10: Type_CharType TINYINT
        //

        case 10:
            RESULT_ULONG((Type == CharType));
            break;

        //
        // 11: Type_WideCharType TINYINT
        //

        case 11:
            RESULT_ULONG((Type == WideCharType));
            break;

        //
        // 12: Type_ShortType TINYINT
        //

        case 12:
            RESULT_ULONG((Type == ShortType));
            break;

        //
        // 13: Type_LongType TINYINT
        //

        case 13:
            RESULT_ULONG((Type == LongType));
            break;

        //
        // 14: Type_Integer64Type TINYINT
        //

        case 14:
            RESULT_ULONG((Type == Integer64Type));
            break;

        //
        // 15: Type_IntegerType TINYINT
        //

        case 15:
            RESULT_ULONG((Type == IntegerType));
            break;

        //
        // 16: Type_UnsignedCharType TINYINT
        //

        case 16:
            RESULT_ULONG((Type == UnsignedCharType));
            break;

        //
        // 17: Type_UnsignedWideCharType TINYINT
        //

        case 17:
            RESULT_ULONG((Type == UnsignedWideCharType));
            break;

        //
        // 18: Type_UnsignedShortType TINYINT
        //

        case 18:
            RESULT_ULONG((Type == UnsignedShortType));
            break;

        //
        // 19: Type_UnsignedLongType TINYINT
        //

        case 19:
            RESULT_ULONG((Type == UnsignedLongType));
            break;

        //
        // 20: Type_InlineCallerType TINYINT
        //

        case 20:
            RESULT_ULONG((Type == InlineCallerType));
            break;

        //
        // 21: Scope INT
        //

        case 21:
            RESULT_ULONG(Scope);
            break;

        //
        // 22: Scope_PrivateGlobalScope TINYINT
        //

        case 22:
            RESULT_ULONG((Scope == PrivateGlobalScope));
            break;

        //
        // 23: Scope_PrivateInlineScope TINYINT
        //

        case 23:
            RESULT_ULONG((Scope == PrivateInlineScope));
            break;

        //
        // 24: Scope_PrivateFunctionScope TINYINT
        //

        case 24:
            RESULT_ULONG((Scope == PrivateFunctionScope));
            break;

        //
        // 25: Scope_PublicGlobalScope TINYINT
        //

        case 25:
            RESULT_ULONG((Scope == PublicGlobalScope));
            break;

        //
        // 26: Scope_PublicFunctionScope TINYINT
        //

        case 26:
            RESULT_ULONG((Scope == PublicFunctionScope));
            break;

        //
        // 27: Address BIGINT
        //

        case 27:
            RESULT_LARGE_INTEGER(ExaminedSymbol->Address);
            break;

        //
        // 28: Strings_Line TEXT
        //

        case 28:
            RESULT_STRING(Strings->Line);
            break;

        //
        // 29: Strings_Scope TEXT
        //

        case 29:
            RESULT_STRING(Strings->Scope);
            break;

        //
        // 30: Strings_Address TEXT
        //

        case 30:
            RESULT_STRING(Strings->Address);
            break;

        //
        // 31: Strings_Size TEXT
        //

        case 31:
            RESULT_STRING(Strings->Size);
            break;

        //
        // 32: Strings_BasicType TEXT
        //

        case 32:
            RESULT_STRING(Strings->BasicType);
            break;

        //
        // 33: Strings_TypeName TEXT
        //

        case 33:
            RESULT_STRING(Strings->TypeName);
            break;

        //
        // 34: Strings_Array TEXT
        //

        case 34:
            RESULT_STRING(Strings->Array);
            break;

        //
        // 35: Strings_ModuleName TEXT
        //

        case 35:
            RESULT_STRING(Strings->ModuleName);
            break;

        //
        // 36: Strings_SymbolName TEXT
        //

        case 36:
            RESULT_STRING(Strings->SymbolName);
            break;

        //
        // 37: Strings_Remaining TEXT
        //

        case 37:
            RESULT_STRING(Strings->Remaining);
            break;

        //
        // 38: Strings_Value TEXT
        //

        case 38:
            RESULT_STRING(Strings->Value);
            break;

        //
        // 39: Function_NumberOfArguments INT
        //

        case 39:
            if (!((Function != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(Function->NumberOfArguments);
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
// N.B. Having the Python stuff here is an encapsulation violation, however,
//      it does the job for now.
//

#include "..\Python\Python.h"

//
// Python_PythonPathTableEntry
//

CONST CHAR PythonPathTableEntrySchema[] =
    "CREATE TABLE Python_PythonPathTableEntry("
        "NodeTypeCode SMALLINT, "
        "PrefixNameLength SMALLINT, "
        "PathEntryTypeFlags INT, "
        "IsModuleDirectory INT, "
        "IsNonModuleDirectory INT, "
        "IsFileSystemDirectory INT, "
        "IsFile INT, "
        "IsClass INT, "
        "IsFunction INT, "
        "IsSpecial INT, "
        "IsValid INT, "
        "IsDll INT, "
        "IsC INT, "
        "IsBuiltin INT, "
        "IsInitPy INT,"
        "Prefix TEXT, "                             // PSTRING
        "Path TEXT, "                               // STRING
        "PathLength SMALLINT, "
        "PathMaximumLength SMALLINT, "
        "FullName TEXT, "                           // STRING
        "ModuleName TEXT, "                         // STRING
        "ClassName TEXT, "                          // STRING
        "Name TEXT, "                               // STRING
        "File_CreationTime BIGINT, "                // File->CreationTime, LARGE_INTEGER
        "File_LastAccessTime BIGINT, "              // File->LastAccessTime, LARGE_INTEGER
        "File_LastWriteTime BIGINT, "               // File->LastWriteTime, LARGE_INTEGER
        "File_ChangeTime BIGINT, "                  // File->ChangeTime, LARGE_INTEGER
        "File_FileAttributes INT, "                 // File->FileAttributes
        "File_NumberOfPages INT, "                  // File->NumberOfPages
        "File_EndOfFile BIGINT, "                   // File->EndOfFile, LARGE_INTEGER
        "File_AllocationSize BIGINT, "              // File->AllocationSize, LARGE_INTEGER
        "File_FileId BIGINT, "                      // File->FileId, LARGE_INTEGER
        "File_VolumeSerialNumber BIGINT, "          // File->VolumeSerialNumber
        "File_FileId128 BLOB, "                     // File->FileId128, sizeof()
        "File_MD5 BLOB, "                           // File->MD5, sizeof()
        "File_SHA1 BLOB, "                          // File->SHA1, sizeof()
        "File_CopyTimeInMicroseconds BIGINT, "      // File->CopyTimeInMicroseconds, LARGE_INTEGER
        "File_CopiedBytesPerSecond BIGINT, "        // File->CopiedBytesPerSecond, LARGE_INTEGER
        "File_FileType INT, "                       // File->Type
        "File_FileFlags INT, "                      // File->Flags.AsULong
        "File_ContentAddress BIGINT, "              // File->Content
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
TraceStoreSqlite3PythonPathTableEntryColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PPYTHON_PATH_TABLE_ENTRY PythonPathTableEntry;
    PRTL_FILE File;
    PRTL_PATH Path;
    PRTL_TEXT_FILE TextFile;

    PythonPathTableEntry = (PPYTHON_PATH_TABLE_ENTRY)Cursor->CurrentRowRaw;
    File = &PythonPathTableEntry->File;
    Path = &File->Path;
    TextFile = &File->SourceCode;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: NodeTypeCode SMALLINT
        //

        case 0:
            RESULT_ULONG(PythonPathTableEntry->NodeTypeCode);
            break;

        //
        // 1: PrefixNameLength SMALLINT
        //

        case 1:
            RESULT_ULONG(PythonPathTableEntry->PrefixNameLength);
            break;

        //
        // 2: PathEntryTypeFlags INT
        //

        case 2:
            RESULT_ULONG(PythonPathTableEntry->PathEntryTypeFlags);
            break;

        //
        // 3: IsModuleDirectory INT
        //

        case 3:
            RESULT_ULONG(PythonPathTableEntry->IsModuleDirectory);
            break;

        //
        // 4: IsNonModuleDirectory INT
        //

        case 4:
            RESULT_ULONG(PythonPathTableEntry->IsNonModuleDirectory);
            break;

        //
        // 5: IsFileSystemDirectory INT
        //

        case 5:
            RESULT_ULONG(PythonPathTableEntry->IsFileSystemDirectory);
            break;

        //
        // 6: IsFile INT
        //

        case 6:
            RESULT_ULONG(PythonPathTableEntry->IsFile);
            break;

        //
        // 7: IsClass INT
        //

        case 7:
            RESULT_ULONG(PythonPathTableEntry->IsClass);
            break;

        //
        // 8: IsFunction INT
        //

        case 8:
            RESULT_ULONG(PythonPathTableEntry->IsFunction);
            break;

        //
        // 9: IsSpecial INT
        //

        case 9:
            RESULT_ULONG(PythonPathTableEntry->IsSpecial);
            break;

        //
        // 10: IsValid INT
        //

        case 10:
            RESULT_ULONG(PythonPathTableEntry->IsValid);
            break;

        //
        // 11: IsDll INT
        //

        case 11:
            RESULT_ULONG(PythonPathTableEntry->IsDll);
            break;

        //
        // 12: IsC INT
        //

        case 12:
            RESULT_ULONG(PythonPathTableEntry->IsC);
            break;

        //
        // 13: IsBuiltin INT
        //

        case 13:
            RESULT_ULONG(PythonPathTableEntry->IsBuiltin);
            break;

        //
        // 14: IsInitPy INT,
        //

        case 14:
            RESULT_ULONG(PythonPathTableEntry->IsInitPy);
            break;

        //
        // 15: Prefix TEXT
        //

        case 15:
            RESULT_PSTRING(PythonPathTableEntry->Prefix);
            break;

        //
        // 16: Path TEXT
        //

        case 16:
            RESULT_STRING(PythonPathTableEntry->Path);
            break;

        //
        // 17: PathLength SMALLINT
        //

        case 17:
            RESULT_ULONG(PythonPathTableEntry->PathLength);
            break;

        //
        // 18: PathMaximumLength SMALLINT
        //

        case 18:
            RESULT_ULONG(PythonPathTableEntry->PathMaximumLength);
            break;

        //
        // 19: FullName TEXT
        //

        case 19:
            RESULT_STRING(PythonPathTableEntry->FullName);
            break;

        //
        // 20: ModuleName TEXT
        //

        case 20:
            RESULT_STRING(PythonPathTableEntry->ModuleName);
            break;

        //
        // 21: ClassName TEXT
        //

        case 21:
            RESULT_STRING(PythonPathTableEntry->ClassName);
            break;

        //
        // 22: Name TEXT
        //

        case 22:
            RESULT_STRING(PythonPathTableEntry->Name);
            break;

        //
        // 23: File_CreationTime BIGINT
        //

        case 23:
            RESULT_LARGE_INTEGER(File->CreationTime);
            break;

        //
        // 24: File_LastAccessTime BIGINT
        //

        case 24:
            RESULT_LARGE_INTEGER(File->LastAccessTime);
            break;

        //
        // 25: File_LastWriteTime BIGINT
        //

        case 25:
            RESULT_LARGE_INTEGER(File->LastWriteTime);
            break;

        //
        // 26: File_ChangeTime BIGINT
        //

        case 26:
            RESULT_LARGE_INTEGER(File->ChangeTime);
            break;

        //
        // 27: File_FileAttributes INT
        //

        case 27:
            RESULT_ULONG(File->FileAttributes);
            break;

        //
        // 28: File_NumberOfPages INT
        //

        case 28:
            RESULT_ULONG(File->NumberOfPages);
            break;

        //
        // 29: File_EndOfFile BIGINT
        //

        case 29:
            RESULT_LARGE_INTEGER(File->EndOfFile);
            break;

        //
        // 30: File_AllocationSize BIGINT
        //

        case 30:
            RESULT_LARGE_INTEGER(File->AllocationSize);
            break;

        //
        // 31: File_FileId BIGINT
        //

        case 31:
            RESULT_LARGE_INTEGER(File->FileId);
            break;

        //
        // 32: File_VolumeSerialNumber BIGINT
        //

        case 32:
            RESULT_ULONGLONG(File->VolumeSerialNumber);
            break;

        //
        // 33: File_FileId128 BLOB
        //

        case 33:
            RESULT_BLOB(File->FileId128, sizeof(File->FileId128));
            break;

        //
        // 34: File_MD5 BLOB
        //

        case 34:
            RESULT_BLOB(File->MD5, sizeof(File->MD5));
            break;

        //
        // 35: File_SHA1 BLOB
        //

        case 35:
            RESULT_BLOB(File->SHA1, sizeof(File->SHA1));
            break;

        //
        // 36: File_CopyTimeInMicroseconds BIGINT
        //

        case 36:
            RESULT_LARGE_INTEGER(File->CopyTimeInMicroseconds);
            break;

        //
        // 37: File_CopiedBytesPerSecond BIGINT
        //

        case 37:
            RESULT_LARGE_INTEGER(File->CopiedBytesPerSecond);
            break;

        //
        // 38: File_FileType INT
        //

        case 38:
            RESULT_ULONG(File->Type);
            break;

        //
        // 39: File_FileFlags INT
        //

        case 39:
            RESULT_ULONG(File->Flags.AsULong);
            break;

        //
        // 40: File_ContentAddress BIGINT
        //

        case 40:
            RESULT_ULONGLONG(File->Content);
            break;

        //
        // 41: TextFile_StructSizeInBytes INT
        //

        case 41:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(TextFile->StructSizeInBytes);
            }
            break;

        //
        // 42: TextFile_NumberOfLines INT
        //

        case 42:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONG(TextFile->NumberOfLines);
            }
            break;

        //
        // 43: TextFile_LinesAddress BIGINT
        //

        case 43:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->Lines);
            }
            break;

        //
        // 44: TextFile_CarriageReturnBitmapAddress BIGINT
        //

        case 44:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->CarriageReturnBitmap);
            }
            break;

        //
        // 45: TextFile_LineFeedBitmapAddress BIGINT
        //

        case 45:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineFeedBitmap);
            }
            break;

        //
        // 46: TextFile_LineEndingBitmapAddress BIGINT
        //

        case 46:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineEndingBitmap);
            }
            break;

        //
        // 47: TextFile_LineBitmapAddress BIGINT
        //

        case 47:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->LineBitmap);
            }
            break;

        //
        // 48: TextFile_WhitespaceBitmapAddress BIGINT
        //

        case 48:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->WhitespaceBitmap);
            }
            break;

        //
        // 49: TextFile_TabBitmapAddress BIGINT
        //

        case 49:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->TabBitmap);
            }
            break;

        //
        // 50: TextFile_IndentBitmapAddress BIGINT
        //

        case 50:
            if (!((File->Type == RtlFileTextFileType && TextFile != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(TextFile->IndentBitmap);
            }
            break;

        //
        // 51: TextFile_TrailingWhitespaceBitmapAddress BIGINT
        //

        case 51:
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
    Function = (PPYTHON_FUNCTION)&FunctionTableEntry->PythonFunction;
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
// Continue the encapsulation violation and include PythonTracer's main header
// here too.
//

#include "../PythonTracer/PythonTracer.h"

//
// PythonTracer_TraceEventTraitsEx
//

CONST CHAR PythonEventTraitsExSchema[] =
    "CREATE TABLE PythonTracer_TraceEventTraitsEx("
        "IsCall INT, "
        "IsException INT, "
        "IsLine INT, "
        "IsReturn INT, "
        "IsC INT, "
        "AsEventType INT, "
        "IsReverseJump INT, "
        "LineNumberOrCallStackDepth INT"
    ")";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3PythonEventTraitsExColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3PythonEventTraitsExColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PPYTHON_EVENT_TRAITS_EX PythonEventTraitsEx;

    PythonEventTraitsEx = (PPYTHON_EVENT_TRAITS_EX)Cursor->CurrentRowRaw;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: IsCall INT
        //

        case 0:
            RESULT_ULONG(PythonEventTraitsEx->IsCall);
            break;

        //
        // 1: IsException INT
        //

        case 1:
            RESULT_ULONG(PythonEventTraitsEx->IsException);
            break;

        //
        // 2: IsLine INT
        //

        case 2:
            RESULT_ULONG(PythonEventTraitsEx->IsLine);
            break;

        //
        // 3: IsReturn INT
        //

        case 3:
            RESULT_ULONG(PythonEventTraitsEx->IsReturn);
            break;

        //
        // 4: IsC INT
        //

        case 4:
            RESULT_ULONG(PythonEventTraitsEx->IsC);
            break;

        //
        // 5: AsEventType INT
        //

        case 5:
            RESULT_ULONG(PythonEventTraitsEx->AsEventType);
            break;

        //
        // 6: IsReverseJump INT
        //

        case 6:
            RESULT_ULONG(PythonEventTraitsEx->IsReverseJump);
            break;

        //
        // 7: LineNumberOrCallStackDepth INT
        //

        case 7:
            RESULT_ULONG(PythonEventTraitsEx->LineNumberOrCallStackDepth);
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
// TraceStore_PageFault
//

CONST CHAR TraceStorePageFaultSchema[] =
    "CREATE TABLE TraceStore_PageFault("
        "FaultingPcFlags_DisassemblyAvailable SMALLINT, "
        "FaultingPcFlags_SourceCodeAvailable SMALLINT, "
        "FaultingPcFlags_IsWinSxSModule SMALLINT, "
        "FaultingPcFlags_IsWindowsModule SMALLINT, "
        "FaultingPcFlags_IsTracerModule SMALLINT, "
        "FaultingPcFlags_IsCorePythonModule SMALLINT, "
        "FaultingPcFlags_IsPythonPackageModule SMALLINT, "
        "FaultingVaFlags_IsTraceStoreAddress SMALLINT, "
        "FaultingVaFlags_IsProcessHeap SMALLINT, "
        "FaultingVaFlags_IsFileBacked SMALLINT, "
        "FaultingVaFlags_IsPageFile SMALLINT, "
        "FaultingThreadFlags_IsTracerThread SMALLINT, "
        "FaultingThreadFlags_IsTraceStoreThread SMALLINT, "
        "FaultingThreadFlags_IsPythonThread SMALLINT, "
        "FaultingPcModule_BaseAddress BIGINT"               // FaultingPcModule->BaseAddress, [(FaultingPcModule != NULL)]
    ")";


_Use_decl_annotations_
LONG
TraceStoreSqlite3PageFaultColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_PAGE_FAULT PageFault;
    PTRACE_PAGE_FAULT_PC_FLAGS FaultingPcFlags;
    PTRACE_PAGE_FAULT_VA_FLAGS FaultingVaFlags;
    PTRACE_PAGE_FAULT_THREAD_FLAGS FaultingThreadFlags;
    PTRACE_MODULE_TABLE_ENTRY FaultingPcModule;
    PTRACE_STORE FaultingVaTraceStore;
    PLINKED_STRING Disassembly;

    PageFault = (PTRACE_PAGE_FAULT)Cursor->CurrentRowRaw;
    FaultingPcFlags = &PageFault->FaultingPcFlags;
    FaultingVaFlags = &PageFault->FaultingVaFlags;
    FaultingThreadFlags = &PageFault->FaultingThreadFlags;
    FaultingPcModule = PageFault->FaultingPcModule;
    FaultingVaTraceStore = PageFault->FaultingVaTraceStore;
    Disassembly = PageFault->Disassembly;

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: FaultingPcFlags_DisassemblyAvailable SMALLINT
        //

        case 0:
            RESULT_ULONG(FaultingPcFlags->DisassemblyAvailable);
            break;

        //
        // 1: FaultingPcFlags_SourceCodeAvailable SMALLINT
        //

        case 1:
            RESULT_ULONG(FaultingPcFlags->SourceCodeAvailable);
            break;

        //
        // 2: FaultingPcFlags_IsWinSxSModule SMALLINT
        //

        case 2:
            RESULT_ULONG(FaultingPcFlags->IsWinSxSModule);
            break;

        //
        // 3: FaultingPcFlags_IsWindowsModule SMALLINT
        //

        case 3:
            RESULT_ULONG(FaultingPcFlags->IsWindowsModule);
            break;

        //
        // 4: FaultingPcFlags_IsTracerModule SMALLINT
        //

        case 4:
            RESULT_ULONG(FaultingPcFlags->IsTracerModule);
            break;

        //
        // 5: FaultingPcFlags_IsCorePythonModule SMALLINT
        //

        case 5:
            RESULT_ULONG(FaultingPcFlags->IsCorePythonModule);
            break;

        //
        // 6: FaultingPcFlags_IsPythonPackageModule SMALLINT
        //

        case 6:
            RESULT_ULONG(FaultingPcFlags->IsPythonPackageModule);
            break;

        //
        // 7: FaultingVaFlags_IsTraceStoreAddress SMALLINT
        //

        case 7:
            RESULT_ULONG(FaultingVaFlags->IsTraceStoreAddress);
            break;

        //
        // 8: FaultingVaFlags_IsProcessHeap SMALLINT
        //

        case 8:
            RESULT_ULONG(FaultingVaFlags->IsProcessHeap);
            break;

        //
        // 9: FaultingVaFlags_IsFileBacked SMALLINT
        //

        case 9:
            RESULT_ULONG(FaultingVaFlags->IsFileBacked);
            break;

        //
        // 10: FaultingVaFlags_IsPageFile SMALLINT
        //

        case 10:
            RESULT_ULONG(FaultingVaFlags->IsPageFile);
            break;

        //
        // 11: FaultingThreadFlags_IsTracerThread SMALLINT
        //

        case 11:
            RESULT_ULONG(FaultingThreadFlags->IsTracerThread);
            break;

        //
        // 12: FaultingThreadFlags_IsTraceStoreThread SMALLINT
        //

        case 12:
            RESULT_ULONG(FaultingThreadFlags->IsTraceStoreThread);
            break;

        //
        // 13: FaultingThreadFlags_IsPythonThread SMALLINT
        //

        case 13:
            RESULT_ULONG(FaultingThreadFlags->IsPythonThread);
            break;

        //
        // 14: FaultingPcModule_BaseAddress BIGINT
        //

        case 14:
            if (!((FaultingPcModule != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(FaultingPcModule->BaseAddress);
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
// TraceStore_SymbolInfo.
//

CONST CHAR TraceStoreSymbolInfoSchema[] =
    "CREATE TABLE SymbolInfo("
        "SizeOfStruct INT, "
        "TypeIndex INT, "
        "SymbolIndex INT, "     // SymbolInfo->Index
        "Size INT, "
        "ModBase BIGINT, "
        "Flags INT, "
        "Value BIGINT, "
        "Address BIGINT, "
        "Register INT, "
        "Scope INT, "
        "Tag INT, "
        "NameLen INT, "
        "MaxNameLen INT, "
        "Name TEXT"             // Name, STRING, [(SymbolInfo->NameLen > 0)]
    ")";

_Use_decl_annotations_
LONG
TraceStoreSqlite3SymbolInfoColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PSYMBOL_INFO SymbolInfo;
    STRING Name;

    SymbolInfo = (PSYMBOL_INFO)Cursor->CurrentRowRaw;

    if (SymbolInfo->SizeOfStruct != sizeof(*SymbolInfo)) {
        __debugbreak();
    }

    if (ColumnNumber == 13) {
        if (SymbolInfo->NameLen == 0) {
            ZeroStruct(Name);
        } else {
            Name.Length = (USHORT)SymbolInfo->NameLen;
            Name.MaximumLength = (USHORT)SymbolInfo->MaxNameLen;
            Name.Buffer = (PCHAR)&SymbolInfo->Name;
        }
    }

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: SizeOfStruct INT
        //

        case 0:
            RESULT_ULONG(SymbolInfo->SizeOfStruct);
            break;

        //
        // 1: TypeIndex INT
        //

        case 1:
            RESULT_ULONG(SymbolInfo->TypeIndex);
            break;

        //
        // 2: SymbolIndex INT
        //

        case 2:
            RESULT_ULONG(SymbolInfo->Index);
            break;

        //
        // 3: Size INT
        //

        case 3:
            RESULT_ULONG(SymbolInfo->Size);
            break;

        //
        // 4: ModBase BIGINT
        //

        case 4:
            RESULT_ULONGLONG(SymbolInfo->ModBase);
            break;

        //
        // 5: Flags INT
        //

        case 5:
            RESULT_ULONG(SymbolInfo->Flags);
            break;

        //
        // 6: Value BIGINT
        //

        case 6:
            RESULT_ULONGLONG(SymbolInfo->Value);
            break;

        //
        // 7: Address BIGINT
        //

        case 7:
            RESULT_ULONGLONG(SymbolInfo->Address);
            break;

        //
        // 8: Register INT
        //

        case 8:
            RESULT_ULONG(SymbolInfo->Register);
            break;

        //
        // 9: Scope INT
        //

        case 9:
            RESULT_ULONG(SymbolInfo->Scope);
            break;

        //
        // 10: Tag INT
        //

        case 10:
            RESULT_ULONG(SymbolInfo->Tag);
            break;

        //
        // 11: NameLen INT
        //

        case 11:
            RESULT_ULONG(SymbolInfo->NameLen);
            break;

        //
        // 12: MaxNameLen INT
        //

        case 12:
            RESULT_ULONG(SymbolInfo->MaxNameLen);
            break;

        //
        // 13: Name TEXT
        //

        case 13:
            if (!((SymbolInfo->NameLen > 0))) {
                RESULT_NULL();
            } else {
                RESULT_STRING(Name);
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
// *_Interval
//

CONST CHAR TraceStoreIntervalSchema[] =
    "CREATE TABLE Interval("
        "IntervalIndex BIGINT, "
        "RecordIndex BIGINT, "
        "AllocationTimestamp BIGINT, "
        "Address BIGINT, "
        "NextIntervalIndex BIGINT, "        // Next->IntervalIndex, [(Next != NULL)]
        "NextRecordIndex BIGINT, "          // Next->RecordIndex, [(Next != NULL)]
        "NextAllocationTimestamp BIGINT, "  // Next->AllocationTimestamp, [(Next != NULL)]
        "NextAddress BIGINT, "              // Next->Address, [(Next != NULL)]
        "RecordCount BIGINT"                // Next->RecordIndex - Interval->RecordIndex, [(Next != NULL)]
    ")";

TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3IntervalColumn;

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalColumn(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_STORE_INTERVAL Interval;
    PTRACE_STORE_INTERVAL Next;

    Interval = Cursor->CurrentRow.AsInterval;
    Next = Interval + 1;
    if ((ULONGLONG)Next > Cursor->LastRowRaw) {
        Next = NULL;
    }

    switch (ColumnNumber) {

        //
        // Begin auto-generated section.
        //

        //
        // 0: IntervalIndex BIGINT
        //

        case 0:
            RESULT_ULONGLONG(Interval->IntervalIndex);
            break;

        //
        // 1: RecordIndex BIGINT
        //

        case 1:
            RESULT_ULONGLONG(Interval->RecordIndex);
            break;

        //
        // 2: AllocationTimestamp BIGINT
        //

        case 2:
            RESULT_ULONGLONG(Interval->AllocationTimestamp);
            break;

        //
        // 3: Address BIGINT
        //

        case 3:
            RESULT_ULONGLONG(Interval->Address);
            break;

        //
        // 4: NextIntervalIndex BIGINT
        //

        case 4:
            if (!((Next != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Next->IntervalIndex);
            }
            break;

        //
        // 5: NextRecordIndex BIGINT
        //

        case 5:
            if (!((Next != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Next->RecordIndex);
            }
            break;

        //
        // 6: NextAllocationTimestamp BIGINT
        //

        case 6:
            if (!((Next != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Next->AllocationTimestamp);
            }
            break;

        //
        // 7: NextAddress BIGINT
        //

        case 7:
            if (!((Next != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Next->Address);
            }
            break;

        //
        // 8: RecordCount BIGINT
        //

        case 8:
            if (!((Next != NULL))) {
                RESULT_NULL();
            } else {
                RESULT_ULONGLONG(Next->RecordIndex - Interval->RecordIndex);
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

    PythonPathTableEntrySchema, // Python_PythonPathTableEntry,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStorePageFaultSchema, // TraceStore_PageFault,
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

    PythonEventTraitsExSchema, // PythonTracer_EventTraitsEx,
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStoreWsWatchInfoExSchema, // TraceStore_WsWatchInfoEx
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStoreWsWorkingSetExInfoSchema, // TraceStore_WsWorkingSetExInfo
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

    TraceStorePerformanceSchema, // TraceStore_Performance
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStorePerformanceDeltaSchema, // TraceStore_PerformanceDelta
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SourceCode
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_Bitmap
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_ImageFile
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_UnicodeStringBuffer
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_Line
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_Object
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStoreModuleLoadEventSchema, // TraceStore_ModuleLoadEvent
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolTable
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolTableEntry
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolModuleInfo
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

    TraceStoreSymbolInfoSchema, // TraceStore_SymbolInfo
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolLine
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_SymbolType
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_StackFrame
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoTable
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoTableEntry
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_TypeInfoStringBuffer
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_FunctionTable
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_FunctionTableEntry
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_FunctionAssembly
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_FunctionSourceCode
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_ExamineSymbolsLine
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_ExamineSymbolsText
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    TraceStoreExaminedSymbolSchema, // TraceStore_ExaminedSymbol
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_ExaminedSymbolSecondary
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_UnassembleFunctionLine
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_UnassembleFunctionText
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_UnassembledFunction
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_UnassembledFunctionSecondary
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_DisplayTypeLine
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_DisplayTypeText
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_DisplayedType
    TraceStoreMetadataInfoSchema,
    TraceStoreAllocationSchema,
    TraceStoreRelocationSchema,
    TraceStoreAddressSchema,
    TraceStoreAddressRangeSchema,
    TraceStoreAllocationTimestampSchema,
    TraceStoreAllocationTimestampDeltaSchema,
    TraceStoreSynchronizationSchema,
    TraceStoreInfoSchema,

    PLACEHOLDER_SCHEMA, // TraceStore_DisplayedTypeSecondary
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

    TraceStoreSqlite3PythonPathTableEntryColumn, // Python_PythonPathTableEntry,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3PageFaultColumn, // TraceStore_PageFault,
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

    TraceStoreSqlite3PythonEventTraitsExColumn, // PythonTracer_EventTraitsEx,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3WsWatchInfoExColumn, // TraceStore_WsWatchInfoEx,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3WsWorkingSetExInfoColumn, // TraceStore_WsWorkingSetExInfo,
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

    TraceStoreSqlite3PerformanceColumn, // TraceStore_Performance,
    TraceStoreSqlite3MetadataInfoColumn,
    TraceStoreSqlite3AllocationColumn,
    TraceStoreSqlite3RelocationColumn,
    TraceStoreSqlite3AddressColumn,
    TraceStoreSqlite3AddressRangeColumn,
    TraceStoreSqlite3AllocationTimestampColumn,
    TraceStoreSqlite3AllocationTimestampDeltaColumn,
    TraceStoreSqlite3SynchronizationColumn,
    TraceStoreSqlite3InfoColumn,

    TraceStoreSqlite3PerformanceDeltaColumn, // TraceStore_PerformanceDelta,
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

    TraceStoreSqlite3SymbolInfoColumn, // TraceStore_SymbolInfo
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

    TraceStoreSqlite3ExaminedSymbolColumn, // TraceStore_ExaminedSymbol,
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
