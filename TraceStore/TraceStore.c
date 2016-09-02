/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStore.c

Abstract:

    This module implements the main Trace Store functionality required by both
    readers and writers.

--*/

#include "stdafx.h"

_Success_(return != 0)
BOOL
InitializeStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
    )
{
    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!TraceStore->FileHandle) {
        // We're a metadata store.
        TraceStore->FileHandle = CreateFileW(
            Path,
            TraceStore->CreateFileDesiredAccess,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
            NULL
        );
    }

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    TraceStore->InitialSize.HighPart = 0;
    TraceStore->InitialSize.LowPart = InitialSize;
    TraceStore->ExtensionSize.HighPart = 0;
    TraceStore->ExtensionSize.LowPart = InitialSize;

    if (!MappingSize) {
        MappingSize = DefaultTraceStoreMappingSize;
    }

    TraceStore->MappingSize.HighPart = 0;
    TraceStore->MappingSize.LowPart = MappingSize;

    //
    // Cap the mapping size to the maximum if necessary.
    //

    if (TraceStore->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
        TraceStore->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
    }

    TraceStore->AllocateRecords = TraceStoreAllocateRecords;
    //TraceStore->FreeRecords = FreeRecords;

    TraceStore->NumberOfAllocations.QuadPart = 0;
    TraceStore->TotalAllocationSize.QuadPart = 0;

    return TRUE;
error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
InitializeTraceStore(
    _In_        PRTL Rtl,
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _Inout_     PTRACE_STORE AllocationStore,
    _Inout_     PTRACE_STORE AddressStore,
    _Inout_     PTRACE_STORE InfoStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
    )
{
    BOOL Success;
    HRESULT Result;
    WCHAR AllocationPath[_OUR_MAX_PATH];
    WCHAR AddressPath[_OUR_MAX_PATH];
    WCHAR InfoPath[_OUR_MAX_PATH];
    PCWSTR AllocationSuffix = TraceStoreAllocationSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR InfoSuffix = TraceStoreInfoSuffix;

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

#define INIT_METADATA_PATH(Name)                                      \
    SecureZeroMemory(&##Name##Path, sizeof(##Name##Path));            \
    Result = StringCchCopyW(                                          \
        &##Name##Path[0],                                             \
        _OUR_MAX_PATH,                                                \
        Path                                                          \
    );                                                                \
    if (FAILED(Result)) {                                             \
        return FALSE;                                                 \
    }                                                                 \
                                                                      \
    Result = StringCchCatW(                                           \
        &##Name##Path[0],                                             \
        _OUR_MAX_PATH,                                                \
        TraceStore##Name##Suffix                                      \
    );                                                                \
    if (FAILED(Result)) {                                             \
        return FALSE;                                                 \
    }                                                                 \
                                                                      \
    ##Name##Store->Rtl = Rtl;                                         \
    if (!InitializeTraceStorePath(&##Name##Path[0], ##Name##Store)) { \
        return FALSE;                                                 \
    }

    INIT_METADATA_PATH(Allocation);
    INIT_METADATA_PATH(Address);
    INIT_METADATA_PATH(Info);

    TraceStore->Rtl = Rtl;

    if (!InitializeTraceStorePath(Path, TraceStore)) {
        return FALSE;
    }

    //
    // Create the data file first before the streams.
    //

    TraceStore->FileHandle = CreateFileW(
        Path,
        TraceStore->CreateFileDesiredAccess,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        goto Error;
    }

#define INIT_METADATA(Name)                                            \
    Name##Store->IsMetadata = TRUE;                                    \
    Name##Store->IsReadonly = TraceStore->IsReadonly;                  \
    Name##Store->SequenceId = TraceStore->SequenceId;                  \
    Name##Store->CreateFileDesiredAccess = (                           \
        TraceStore->CreateFileDesiredAccess                            \
    );                                                                 \
    Name##Store->CreateFileMappingProtectionFlags = (                  \
        TraceStore->CreateFileMappingProtectionFlags                   \
    );                                                                 \
    Name##Store->MapViewOfFileDesiredAccess = (                        \
        TraceStore->MapViewOfFileDesiredAccess                         \
    );                                                                 \
    Name##Store->Frequency.QuadPart = (                                \
        TraceStore->Frequency.QuadPart                                 \
    );                                                                 \
                                                                       \
    Success = InitializeStore(                                         \
        &##Name##Path[0],                                              \
        ##Name##Store,                                                 \
        Default##Name##TraceStoreSize,                                 \
        Default##Name##TraceStoreMappingSize                           \
    );                                                                 \
                                                                       \
    if (!Success) {                                                    \
        goto Error;                                                    \
    }                                                                  \
                                                                       \
    ##Name##Store->NumberOfRecords.QuadPart = 1;                       \
    ##Name##Store->RecordSize.QuadPart = TraceStore##Name##StructSize; \
                                                                       \
    TraceStore->##Name##Store = ##Name##Store;

    INIT_METADATA(Allocation);
    INIT_METADATA(Address);
    INIT_METADATA(Info);

    TraceStore->IsMetadata = FALSE;

    if (!InitializeStore(Path, TraceStore, InitialSize, MappingSize)) {
        goto Error;
    }

    return TRUE;
Error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
TruncateStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    BOOL Success;
    BOOL IsMetadata;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER TotalAllocationSize;
    FILE_STANDARD_INFO FileInfo;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    IsMetadata = IsMetadataTraceStore(TraceStore);

    TotalAllocationSize.QuadPart = TraceStore->TotalAllocationSize.QuadPart;

    if (!IsMetadata) {

        EndOfFile.QuadPart = TraceStore->pEof->EndOfFile.QuadPart;

        if (EndOfFile.QuadPart != TotalAllocationSize.QuadPart) {
            __debugbreak();
        }

    } else {

        if (TotalAllocationSize.QuadPart == 0) {

            if (TraceStore->NumberOfAllocations.QuadPart != 1) {
                __debugbreak();
            }

            TotalAllocationSize.QuadPart = TraceStore->RecordSize.QuadPart;
        }

        EndOfFile.QuadPart = TotalAllocationSize.QuadPart;
    }

    //
    // Get the file's current end of file info.
    //

    Success = GetFileInformationByHandleEx(
        TraceStore->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &FileInfo,
        sizeof(FileInfo)
    );

    if (!Success) {
        return FALSE;
    }

    if (FileInfo.EndOfFile.QuadPart == EndOfFile.QuadPart) {

        //
        // Nothing more to do.
        //

        return TRUE;
    }

    //
    // Adjust the file pointer to the desired position.
    //

    Success = SetFilePointerEx(TraceStore->FileHandle,
                               EndOfFile,
                               NULL,
                               FILE_BEGIN);

    if (!Success) {
        return FALSE;
    }

    //
    // And set the end of file.
    //

    Success = SetEndOfFile(TraceStore->FileHandle);

    return Success;
}

VOID
CloseStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->MemoryMap);

    while (PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    WaitForSingleObject(TraceStore->AllMemoryMapsAreFreeEvent, INFINITE);

    if (TraceStore->FileHandle) {
        TruncateStore(TraceStore);
        FlushFileBuffers(TraceStore->FileHandle);
        CloseHandle(TraceStore->FileHandle);
        TraceStore->FileHandle = NULL;
    }

    if (TraceStore->NextMemoryMapAvailableEvent) {
        CloseHandle(TraceStore->NextMemoryMapAvailableEvent);
        TraceStore->NextMemoryMapAvailableEvent = NULL;
    }

    if (TraceStore->AllMemoryMapsAreFreeEvent) {
        CloseHandle(TraceStore->AllMemoryMapsAreFreeEvent);
        TraceStore->AllMemoryMapsAreFreeEvent = NULL;
    }

    if (TraceStore->CloseMemoryMapWork) {
        CloseThreadpoolWork(TraceStore->CloseMemoryMapWork);
        TraceStore->CloseMemoryMapWork = NULL;
    }

    if (TraceStore->PrepareNextMemoryMapWork) {
        CloseThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);
        TraceStore->PrepareNextMemoryMapWork = NULL;
    }

    if (TraceStore->PrefaultFuturePageWork) {
        CloseThreadpoolWork(TraceStore->PrefaultFuturePageWork);
        TraceStore->PrefaultFuturePageWork = NULL;
    }

}

VOID
CloseTraceStore(
    _In_ PTRACE_STORE TraceStore
    )
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Close the trace store first before the metadata stores.  This is
    // important because closing the trace store will retire any active memory
    // maps, which will update the underlying MemoryMap->pAddress structures,
    // which will be backed by the AddressStore, which we can only access as
    // long as we haven't closed it.
    //

    CloseStore(TraceStore);

    //
    // Close the metadata stores.
    //

    if (TraceStore->AllocationStore) {
        CloseStore(TraceStore->AllocationStore);
        TraceStore->AllocationStore = NULL;
    }

    if (TraceStore->AddressStore) {
        CloseStore(TraceStore->AddressStore);
        TraceStore->AddressStore = NULL;
    }

    if (TraceStore->InfoStore) {
        CloseStore(TraceStore->InfoStore);
        TraceStore->InfoStore = NULL;
    }

}

VOID
CALLBACK
PrefaultFutureTraceStorePageCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
    )
{
    PrefaultFutureTraceStorePage((PTRACE_STORE)Context);
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
