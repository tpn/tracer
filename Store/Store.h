
#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

typedef struct _STORE_METADATA {
    ULARGE_INTEGER          NumberOfRecords;
    LARGE_INTEGER           RecordSize;
} STORE_METADATA, *PSTORE_METADATA;

typedef struct _STORE STORE, *PSTORE;

typedef _Check_return_ PVOID (*PALLOCATE_RECORDS)(
    _In_    PSTORE          Store,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
);

typedef BOOL (*PGET_ALLOCATION_SIZE)(
    _In_    PSTORE          Store,
    _Inout_ PULARGE_INTEGER TotalSize,
    _Inout_ PULARGE_INTEGER AllocatedSize
);

typedef struct _STORE_THREADPOOL {
    PTP_POOL            Threadpool;
    TP_CALLBACK_ENVIRON CallbackEnvironment;
    PTP_WORK            ExtendStoreCallback;
    HANDLE              ExtendStoreEvent;
} STORE_THREADPOOL, *PSTORE_THREADPOOL;

typedef struct _STORES STORES, *PSTORES;

typedef struct __declspec(align(16)) _STORE_MEMORY_MAP {
    union {
        __declspec(align(16)) SLIST_ENTRY   ListEntry;      // 16       16
        struct {
            __declspec(align(8)) PVOID      PrevAddress;
            __declspec(align(8)) PVOID      Unused1;
        };
    };
    __declspec(align(8))  HANDLE        FileHandle;         // 8        24
    __declspec(align(8))  HANDLE        MappingHandle;      // 8        32
    __declspec(align(8))  LARGE_INTEGER FileOffset;         // 8        40
    __declspec(align(8))  LARGE_INTEGER MappingSize;        // 8        48
    __declspec(align(8))  PVOID         BaseAddress;        // 8        56
    __declspec(align(8))  PVOID         NextAddress;        // 8        64
} STORE_MEMORY_MAP, *PSTORE_MEMORY_MAP, **PPSTORE_MEMORY_MAP;

typedef volatile PSTORE_MEMORY_MAP VPSTORE_MEMORY_MAP;

C_ASSERT(sizeof(STORE_MEMORY_MAP) == 64);

typedef struct _STORE {
    SLIST_HEADER            CloseMemoryMaps;
    SLIST_HEADER            PrepareMemoryMaps;
    SLIST_HEADER            NextMemoryMaps;
    SLIST_HEADER            FreeMemoryMaps;
    SLIST_HEADER            PrefaultMemoryMaps;

    PRTL                    Rtl;
    LARGE_INTEGER           InitialSize;
    LARGE_INTEGER           ExtensionSize;
    LARGE_INTEGER           MappingSize;

    PTP_WORK                PrefaultFuturePageWork;
    PTP_WORK                PrepareNextMemoryMapWork;
    PTP_WORK                CloseMemoryMapWork;
    HANDLE                  NextMemoryMapAvailableEvent;

    PSTORE_MEMORY_MAP       PrevMemoryMap;
    PSTORE_MEMORY_MAP       MemoryMap;

    ULONG                   DroppedRecords;
    ULONG                   ExhaustedFreeMemoryMaps;
    ULONG                   AllocationsOutpacingNextMemoryMapPreparation;
    HANDLE                  AllocationsOutpacingHandle;

    UNICODE_STRING          Path;

    HANDLE                  HeapHandle;
    HANDLE                  FileHandle;
    PVOID                   PrevAddress;

    PSTORE                  MetadataStore;
    PALLOCATE_RECORDS       AllocateRecords;
    PREAD_RECORDS           ReadRecords;
    union {
        union {
            struct {
                ULARGE_INTEGER  NumberOfRecords;
                LARGE_INTEGER   RecordSize;
            };
            STORE_METADATA Metadata;
        };
        PSTORE_METADATA pMetadata;
    };
} STORE, *PSTORE;

static const PCWSTR StoreMetadataSuffix = L":metadata";
static const DWORD StoreMetadataSuffixLength = (
    sizeof(StoreMetadataSuffix) /
    sizeof(WCHAR)
);

typedef struct _STORES {
    USHORT  Size;
    USHORT  NumberOfStores;
    ULONG   Reserved;
    PRTL    Rtl;
    STORE   Stores[1];
} STORES, *PSTORES;

STORE_API
BOOL
InitializeStoreEx(
    _In_        PRTL                    Rtl,
    _In_        PUNICODE_STRING         Path,
    _Inout_opt_ PSTORE                  Store,
    _In_opt_    PULONG                  InitialFileSize,
    _In_opt_    PULONG                  MappingSize,
    _In_        PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_    PVOID                   UserData
);

typedef BOOL (INITIALIZE_STORE)(
    _In_        PRTL                    Rtl,
    _In_        PUNICODE_STRING         Path,
    _Inout_opt_ PSTORE                  Store,
    _In_        PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_    PULONG                  InitialFileSize,
    _In_opt_    PULONG                  MappingSize,
    _In_opt_    HANDLE                  HeapHandle,
    _In_opt_    PVOID                   UserData
    );

STORE_API INITIALIZE_STORE InitializeStore;

typedef INITIALIZE_STORE *PINITIALIZE_STORE;

STORE_API
BOOL
FlushStore(_In_ PSTORE Store);

STORE_API
VOID
SubmitStoreFileExtensionThreadpoolWork(
    _Inout_     PSTORE    Store
);

STORE_API
VOID
CloseStore(PSTORE Store);

STORE_API
BOOL
GetStoreBytesWritten(
    PSTORE Store,
    PULARGE_INTEGER BytesWritten
);

STORE_API
BOOL
GetStoreNumberOfRecords(
    PSTORE          Store,
    PULARGE_INTEGER NumberOfRecords
);

STORE_API
LPVOID
GetNextRecord(
    PSTORE          Store,
    PULARGE_INTEGER RecordSize
);

STORE_API
LPVOID
GetNextRecords(
    _In_ PSTORE         Store,
    _In_ ULARGE_INTEGER RecordSize,
    _In_ ULARGE_INTEGER RecordCount
);

STORE_API
LPVOID
AllocateRecords(
    _In_    PSTORE          Store,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
);

STORE_API
VOID
ReadRecords(
    _In_    PSTORE  Store,
    _Out_   PPVOID  Buffer
    );

STORE_API
BOOL
WriteRecords(
    _In_    PSTORE          Store,
    _In_    ULARGE_INTEGER  RecordSize,
    _In_    ULARGE_INTEGER  NumberOfRecords,
    _In_    PVOID           FirstRecord,
    _In_    PVOID          *DestinationAddress
);

STORE_API
LPVOID
WriteBytes(
    _In_     PSTORE          Store,
    _In_     ULARGE_INTEGER  NumberOfBytes,
    _In_     PVOID           Buffer,
    _In_opt_ PVOID          *DestinationAddress
);

STORE_API
PVOID
GetPreviousAllocationAddress(
    _In_ PSTORE Store
);

STORE_API
BOOL
GetAllocationSize(
    _In_    PSTORE          Store,
    _Inout_ PULARGE_INTEGER TotalSize,
    _Inout_ PULARGE_INTEGER AllocatedSize
);

#ifdef __cpp
} // extern "C"
#endif
