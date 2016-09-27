/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStores.c

Abstract:

    This module implements functionality related to a collection of trace store
    structures, referred to as "trace stores".

--*/

#include "stdafx.h"

DWORD
GetTraceStoresAllocationSize(const USHORT NumberOfTraceStores)
{
    SHORT Delta;
    USHORT ExtraSize;
    USHORT DefaultNumberOfTraceStores = (
        RTL_FIELD_SIZE(TRACE_STORES, Stores) /
        sizeof(TRACE_STORE)
    );

    Delta = (NumberOfTraceStores - DefaultNumberOfTraceStores);

    if (Delta <= 0) {
        return sizeof(TRACE_STORES);
    }

    ExtraSize = sizeof(TRACE_STORE) * Delta * ElementsPerTraceStore;

    return sizeof(TRACE_STORES) + ExtraSize;
}


BOOL
InitializeTraceStores(
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes,
    _In_        PTRACE_FLAGS    TraceFlags
    )
{
    BOOL Success;
    BOOL Readonly;
    BOOL Compress;
    HRESULT Result;
    DWORD Index;
    DWORD StoreIndex;
    DWORD LastError;
    DWORD CreateFileDesiredAccess;
    DWORD CreateFileMappingProtectionFlags;
    DWORD CreateFileFlagsAndAttributes;
    DWORD MapViewOfFileDesiredAccess;
    TRACE_FLAGS Flags;
    LPWSTR FileNameDest;
    DWORD LongestFilename = GetLongestTraceStoreFileName();
    DWORD TraceStoresAllocationSize = (
        GetTraceStoresAllocationSize(
            NumberOfTraceStores *
            ElementsPerTraceStore
        )
    );
    DWORD LongestPossibleDirectoryLength = (
        _OUR_MAX_PATH   -
        1               - // '\'
        1               - // final NUL
        LongestFilename
    );
    LARGE_INTEGER DirectoryLength;
    LARGE_INTEGER RemainingChars;
    PULONG Sizes = InitialFileSizes;
    WCHAR Path[_OUR_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!SizeOfTraceStores) {
        return FALSE;
    }

    if (!TraceStores || *SizeOfTraceStores < TraceStoresAllocationSize) {
        *SizeOfTraceStores = TraceStoresAllocationSize;
        return FALSE;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!BaseDirectory) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceFlags)) {
        return FALSE;
    }

    Flags = *TraceFlags;
    Compress = Flags.Compress;
    Readonly = Flags.Readonly;

    if (!Sizes) {
        Sizes = (PULONG)&InitialTraceStoreFileSizes[0];
    }

    Result = StringCchLengthW(
        BaseDirectory,
        LongestPossibleDirectoryLength,
        (PSIZE_T)&DirectoryLength.QuadPart
    );

    if (FAILED(Result)) {
        return FALSE;
    }

    if (DirectoryLength.HighPart != 0) {
        return FALSE;
    }

    SecureZeroMemory(&Path, sizeof(Path));

    Result = StringCchCopyW(
        &Path[0],
        LongestPossibleDirectoryLength,
        BaseDirectory
    );
    if (FAILED(Result)) {
        return FALSE;
    }

    Path[DirectoryLength.LowPart] = L'\\';
    FileNameDest = &Path[DirectoryLength.LowPart+1];
    RemainingChars.QuadPart = (
        _OUR_MAX_PATH -
        DirectoryLength.LowPart -
        2
    );

    SecureZeroMemory(TraceStores, TraceStoresAllocationSize);
    TraceStores->SizeOfStruct = (USHORT)sizeof(TRACE_STORES);
    TraceStores->SizeOfAllocation = (USHORT)TraceStoresAllocationSize;

    if (Readonly) {
        CreateFileDesiredAccess = GENERIC_READ;
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_READ;
    }
    else {
        CreateFileDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        CreateFileMappingProtectionFlags = PAGE_READWRITE;
        MapViewOfFileDesiredAccess = FILE_MAP_READ | FILE_MAP_WRITE;
    }

    //
    // Create the appropriate dwFileAndAttributes mask based on the flags.
    //

    if (Flags.EnableFileFlagRandomAccess) {
        CreateFileFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS;
    } else if (!Flags.DisableFileFlagSequentialScan) {
        CreateFileFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN;
    }

    if (!Flags.DisableFileFlagOverlapped) {
        CreateFileFlagsAndAttributes |= FILE_FLAG_OVERLAPPED;
    }

    if (Flags.EnableFileFlagWriteThrough) {
        CreateFileFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }

    Success = CreateDirectory(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            return FALSE;
        }
    }

    if (!Readonly && Compress) {
        HANDLE DirectoryHandle;
        USHORT CompressionFormat = COMPRESSION_FORMAT_DEFAULT;
        DWORD BytesReturned = 0;

        DirectoryHandle = CreateFileW(
            BaseDirectory,
            CreateFileDesiredAccess,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        if (DirectoryHandle == INVALID_HANDLE_VALUE) {
            LastError = GetLastError();
            return FALSE;
        }

        Success = DeviceIoControl(
            DirectoryHandle,                // hDevice
            FSCTL_SET_COMPRESSION,          // dwIoControlCode
            &CompressionFormat,             // lpInBuffer
            sizeof(CompressionFormat),      // nInBufferSize
            NULL,                           // lpOutBuffer
            0,                              // nOutBufferSize
            &BytesReturned,                 // lpBytesReturned
            NULL                            // lpOverlapped
        );

        if (!Success) {
            OutputDebugStringA("Failed to enable compression.\n");
        }

        CloseHandle(DirectoryHandle);
    }

    TraceStores->Rtl = Rtl;

    TraceStores->NumberOfTraceStores = NumberOfTraceStores;
    TraceStores->ElementsPerTraceStore = ElementsPerTraceStore;

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE AllocationStore = TraceStore + 1;
        PTRACE_STORE AddressStore = TraceStore + 2;
        PTRACE_STORE InfoStore = TraceStore + 3;

        LPCWSTR FileName = TraceStoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        ULONG MappingSize = DefaultTraceStoreMappingSize;

        if (StoreIndex == TraceStoreEventIndex) {
            MappingSize = DefaultTraceStoreEventMappingSize;
        }

        Result = StringCchCopyW(
            FileNameDest,
            (SIZE_T)RemainingChars.QuadPart,
            FileName
        );

        if (FAILED(Result)) {
            return FALSE;
        }

        TraceStore->IsReadonly = Readonly;
        TraceStore->SequenceId = Index;
        TraceStore->CreateFileDesiredAccess = CreateFileDesiredAccess;
        TraceStore->CreateFileMappingProtectionFlags = (
            CreateFileMappingProtectionFlags
        );
        TraceStore->CreateFileFlagsAndAttributes = (
            CreateFileFlagsAndAttributes
        );
        TraceStore->MapViewOfFileDesiredAccess = (
            MapViewOfFileDesiredAccess
        );

        Success = InitializeTraceStore(
            Rtl,
            Path,
            TraceStore,
            AllocationStore,
            AddressStore,
            InfoStore,
            InitialSize,
            MappingSize,
            &Flags
        );

        if (!Success) {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
CloseTraceStores(
    _In_ PTRACE_STORES TraceStores
    )
{
    USHORT Index;
    USHORT StoreIndex;

    if (!TraceStores) {
        return;
    }

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        CloseTraceStore(&TraceStores->Stores[StoreIndex]);
    }
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
