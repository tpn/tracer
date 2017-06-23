/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

LONG
FindStreams(
    PCUNICODE_STRING Path
    )
{
    HANDLE Handle;
    ULONG LastError;
    WIN32_FIND_STREAM_DATA StreamData;

    Handle = FindFirstStreamW(Path->Buffer,
                              FindStreamInfoStandard,
                              &StreamData,
                              0);

    if (Handle == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        return LastError;
    }

    OutputDebugStringW(StreamData.cStreamName);

    while (FindNextStreamW(Handle, &StreamData)) {
        OutputDebugStringW(StreamData.cStreamName);
    }

    FindClose(Handle);

    return 0;
}

//
// Begin injection glue.
//

typedef struct _INJECTION_FUNCTIONS {
    PRTL_ADD_FUNCTION_TABLE RtlAddFunctionTable;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PGET_PROC_ADDRESS GetProcAddress;
    PGET_LAST_ERROR GetLastError;
    PSET_LAST_ERROR SetLastError;
    PSET_EVENT SetEvent;
    PRESET_EVENT ResetEvent;
    PGET_THREAD_CONTEXT GetThreadContext;
    PSET_THREAD_CONTEXT SetThreadContext;
    PSUSPEND_THREAD SuspendThread;
    PRESUME_THREAD ResumeThread;
    POPEN_EVENT_W OpenEventW;
    PCLOSE_HANDLE CloseHandle;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
    PWAIT_FOR_SINGLE_OBJECT_EX WaitForSingleObjectEx;
    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    POUTPUT_DEBUG_STRING_W OutputDebugStringW;
    PNT_QUEUE_APC_THREAD NtQueueApcThread;
    PNT_TEST_ALERT NtTestAlert;
    PQUEUE_USER_APC QueueUserAPC;
    PSLEEP_EX SleepEx;
    PEXIT_THREAD ExitThread;
    PGET_EXIT_CODE_THREAD GetExitCodeThread;
    PDEVICE_IO_CONTROL DeviceIoControl;
    PCREATE_FILE_W CreateFileW;
    PCREATE_FILE_MAPPING_W CreateFileMappingW;
    POPEN_FILE_MAPPING_W OpenFileMappingW;
    PMAP_VIEW_OF_FILE_EX MapViewOfFileEx;
    PMAP_VIEW_OF_FILE_EX_NUMA MapViewOfFileExNuma;
    PFLUSH_VIEW_OF_FILE FlushViewOfFile;
    PUNMAP_VIEW_OF_FILE_EX UnmapViewOfFileEx;
    PVIRTUAL_ALLOC_EX VirtualAllocEx;
    PVIRTUAL_FREE_EX VirtualFreeEx;
    PVIRTUAL_PROTECT_EX VirtualProtectEx;
    PVIRTUAL_QUERY_EX VirtualQueryEx;
} INJECTION_FUNCTIONS;
typedef INJECTION_FUNCTIONS *PINJECTION_FUNCTIONS;
typedef const INJECTION_FUNCTIONS *PCINJECTION_FUNCTIONS;

typedef union _INJECTION_THUNK_EX_FLAGS {
    struct {
        ULONG DebugBreakOnEntry:1;
        ULONG HasInjectionObjects:1;
        ULONG HasModuleAndFunction:1;
        ULONG HasApc:1;
        ULONG Unused:28;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_THUNK_EX_FLAGS;
C_ASSERT(sizeof(INJECTION_THUNK_EX_FLAGS) == sizeof(ULONG));

//
// Define injection objects structure.
//

typedef enum _Enum_is_bitflag_ _INJECTION_OBJECT_ID {
    NullInjectionObjectId               =        0,
    EventInjectionObjectId              =        1,
    FileMappingInjectionObjectId        =  (1 << 1),

    //
    // Make sure the expression within parenthesis below is identical to the
    // last enumeration above.
    //

    InvalidInjectionObjectId            =  (1 << 1) + 1
} INJECTION_OBJECT_ID;
typedef INJECTION_OBJECT_ID *PINJECTION_OBJECT_ID;

typedef union _INJECTION_OBJECT_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Event:1;
        ULONG FileMapping:1;
        ULONG Unused:30;
    };
    LONG AsLong;
    ULONG AsULong;
    INJECTION_OBJECT_ID AsId;
} INJECTION_OBJECT_TYPE;
C_ASSERT(sizeof(INJECTION_OBJECT_TYPE) == sizeof(ULONG));
typedef INJECTION_OBJECT_TYPE *PINJECTION_OBJECT_TYPE;

typedef union _INJECTION_OBJECT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_FLAGS *PINJECTION_OBJECT_FLAGS;

typedef struct _INJECTION_OBJECT_HEADER {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_HEADER;

typedef union _INJECTION_OBJECT_EVENT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG ManualReset:1;
        ULONG Unused:31;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_EVENT_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_EVENT_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_EVENT_FLAGS *PINJECTION_OBJECT_EVENT_FLAGS;

typedef struct _INJECTION_OBJECT_EVENT {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_EVENT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_EVENT;
typedef INJECTION_OBJECT_EVENT *PINJECTION_OBJECT_EVENT;

typedef struct _INJECTION_OBJECT_FILE_MAPPING {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
    ULONG ShareMode;
    ULONG CreationDisposition;
    ULONG FlagsAndAttributes;
    ULONG AllocationType;
    ULONG PageProtection;
    ULONG PreferredNumaNode;
    LARGE_INTEGER FileOffset;
    LARGE_INTEGER MappingSize;
    PVOID PreferredBaseAddress;
    PVOID BaseAddress;
} INJECTION_OBJECT_FILE_MAPPING;
typedef INJECTION_OBJECT_FILE_MAPPING *PINJECTION_OBJECT_FILE_MAPPING;

typedef union _INJECTION_OBJECT {

    //
    // All subtypes currently share the name, handle, desired access and inherit
    // handle values.  Provide direct access to them here.
    //

    struct {
        INJECTION_OBJECT_TYPE Type;
        INJECTION_OBJECT_FLAGS Flags;
        PUNICODE_STRING Name;
        HANDLE Handle;
        ULONG DesiredAccess;
        ULONG InheritHandle;
    };
    INJECTION_OBJECT_EVENT AsEvent;
    INJECTION_OBJECT_FILE_MAPPING AsFileMapping;
} INJECTION_OBJECT;
typedef INJECTION_OBJECT *PINJECTION_OBJECT;

typedef union _INJECTION_OBJECTS_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECTS_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECTS_FLAGS) == sizeof(ULONG));

typedef struct _INJECTION_OBJECTS {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _INJECTION_OBJECTS))
        USHORT StructSizeInBytes;

    //
    // Size of an individual INJECTION_OBJECT structure.  This must be filled
    // in with the value of `sizeof(INJECTION_OBJECT)`.  The injection thunk
    // assembly routine compares this size with the size of its similarly
    // defined struct and ensures they match.
    //

    USHORT SizeOfInjectionObjectInBytes;

    //
    // Number of objects captured by this structure.  This value governs the
    // number of elements in the arrays for names, contexts etc.
    //

    ULONG NumberOfObjects;

    //
    // Total number of bytes allocated in support of this structure.  This will
    // include all UNICODE_STRING structures and backing wide character buffers,
    // and all handle and error code arrays, plus the size of this structure.
    //

    ULONG TotalAllocSizeInBytes;

    //
    // Flags related to this structure.
    //

    INJECTION_OBJECTS_FLAGS Flags;

    //
    // Base address of first element in objects array.
    //

    PINJECTION_OBJECT Objects;
} INJECTION_OBJECTS;
typedef INJECTION_OBJECTS *PINJECTION_OBJECTS;

//
// End injection glue.
//

#if 0

typedef
VOID
(TEST_PARAMS1)(
    _In_ LONG Param1,
    _In_ LONG Param2,
    _In_ LONG Param3,
    _In_ LONG Param4,
    _In_ LONG Param5,
    _In_ LONG Param6,
    _In_ LONG Param7,
    _In_ LONG Param8
    );
typedef TEST_PARAMS1 *PTEST_PARAMS1;

typedef
VOID
(TEST_PARAMS2)(
    _In_ LONG Param1,
    _In_ LONG Param2,
    _In_ LONG Param3,
    _In_ LONG Param4,
    _In_ LONG Param5,
    _In_ LONG Param6,
    _In_ LONG Param7,
    _In_ LONG Param8
    );
typedef TEST_PARAMS2 *PTEST_PARAMS2;

extern TEST_PARAMS1 TestParams1;
extern TEST_PARAMS2 TestParams2;

#endif

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    LONG ExitCode = 0;

#if 0

    TestParams1(1, 2, 3, 4, 5, 6, 7, 8);
    TestParams2(1, 2, 3, 4, 5, 6, 7, 8);

#endif

#if 0
    HMODULE Module;
    PROC Proc;
    PTEST_PARAMS1 TestParams1;
    PTEST_PARAMS2 TestParams2;

    Module = LoadLibraryA("T:\\Users\\Trent\\src\\tracer\\x64\\Release\\Asm.dll");
    Proc = GetProcAddress(Module, "TestParams1");
    TestParams1 = (PTEST_PARAMS1)Proc;

    TestParams1(1, 2, 3, 4, 5, 6, 7, 8);

    Proc = GetProcAddress(Module, "TestParams2");
    TestParams2 = (PTEST_PARAMS2)Proc;

    TestParams2(1, 2, 3, 4, 5, 6, 7, 8);

#endif

#if 1
    BOOL Success;
    ULONG SizeOfBuffer;
    ULONG LastError;
    USHORT Index;
    USHORT MappingIndex;
    USHORT NumberOfObjects;
    USHORT NumberOfEvents;
    USHORT NumberOfFileMappings;
    HANDLE Handle;
    PWSTR WideBuffer;
    PRTL Rtl;
    PVOID BaseAddress;
    PTRACER_CONFIG TracerConfig;
    ALLOCATOR Allocator;
    PUNICODE_STRING Name;
    //const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING ObjectNames[6];
    UNICODE_STRING ObjectPrefixes[] = {
        RTL_CONSTANT_STRING(L"Event1_"),
        RTL_CONSTANT_STRING(L"Event2_"),
        RTL_CONSTANT_STRING(L"Event3_"),
        RTL_CONSTANT_STRING(L"Event4_"),
        RTL_CONSTANT_STRING(L"SharedMem1_"),
        RTL_CONSTANT_STRING(L"SharedMem2_"),
    };
    PUNICODE_STRING Names[] = {
        &ObjectNames[0],
        &ObjectNames[1],
        &ObjectNames[2],
        &ObjectNames[3],
        &ObjectNames[4],
        &ObjectNames[5],
    };
    PUNICODE_STRING Prefixes[] ={
        &ObjectPrefixes[0],
        &ObjectPrefixes[1],
        &ObjectPrefixes[2],
        &ObjectPrefixes[3],
        &ObjectPrefixes[4],
        &ObjectPrefixes[5],
    };
    ULONG FileMappingDesiredAccess[] = {
        FILE_MAP_READ,
        FILE_MAP_WRITE,
    };
    ULONG FileMappingPageProtection[] = {
        PAGE_READONLY,
        PAGE_READWRITE,
    };
    LARGE_INTEGER MappingSizes[] = {
        {
            1 << 16,    // 16KB
            0,
        },
        {
            1 << 24,    // 24MB
            0,
        },
    };
    LARGE_INTEGER MappingSize;
    INJECTION_OBJECT Objects[6];
    INJECTION_OBJECTS InjectionObjects;
    PINJECTION_OBJECT Object;
    PINJECTION_OBJECT_EVENT Event;
    PINJECTION_OBJECT_FILE_MAPPING FileMapping;

    ZeroStruct(Objects);
    ZeroStruct(InjectionObjects);

    NumberOfObjects = ARRAYSIZE(ObjectNames);
    NumberOfEvents = 4;
    NumberOfFileMappings = 2;

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            (PUNICODE_STRING)&TracerRegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           &Allocator,
                                           &Allocator,
                                           ARRAYSIZE(ObjectNames),
                                           64,
                                           NULL,
                                           (PPUNICODE_STRING)&Names,
                                           (PPUNICODE_STRING)&Prefixes,
                                           &SizeOfBuffer,
                                           &WideBuffer);

    if (!Success) {
        __debugbreak();
    }

    for (Index = 0; Index < NumberOfEvents; Index++) {
        Name = Names[Index];
        Object = &Objects[Index];
        Event = &Object->AsEvent;
        Event->Name = Name;
        Event->Type.AsId = EventInjectionObjectId;
        Event->Flags.ManualReset = FALSE;
        Event->DesiredAccess = EVENT_MODIFY_STATE;

        Event->Handle = Rtl->CreateEventW(NULL,
                                          Event->Flags.ManualReset,
                                          FALSE,
                                          Name->Buffer);

        LastError = GetLastError();

        if (!Event->Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }
    }

    for (Index = NumberOfEvents, MappingIndex = 0;
         Index < NumberOfObjects;
         Index++, MappingIndex++) {

        Name = Names[Index];
        Object = &Objects[Index];
        FileMapping = &Object->AsFileMapping;
        FileMapping->Name = Name;
        FileMapping->Type.AsId = FileMappingInjectionObjectId;
        FileMapping->DesiredAccess = FileMappingDesiredAccess[MappingIndex];
        FileMapping->PageProtection = FileMappingPageProtection[MappingIndex];
        MappingSize.QuadPart = MappingSizes[MappingIndex].QuadPart;
        FileMapping->MappingSize.QuadPart = MappingSize.QuadPart;

        Handle = Rtl->CreateFileMappingW(NULL,
                                         NULL,
                                         PAGE_READWRITE,
                                         FileMapping->MappingSize.HighPart,
                                         FileMapping->MappingSize.LowPart,
                                         FileMapping->Name->Buffer);

        LastError = GetLastError();

        if (!Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }

        FileMapping->Handle = Handle;

        BaseAddress = Rtl->MapViewOfFileEx(Handle,
                                           FILE_MAP_ALL_ACCESS,
                                           0,
                                           0,
                                           FileMapping->MappingSize.QuadPart,
                                           NULL);

        if (!BaseAddress) {
            __debugbreak();
            goto Error;
        }

        FileMapping->PreferredBaseAddress = BaseAddress;
    }

#endif

#if 0
    BOOL Success;
    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    HANDLE EventHandle;
    ALLOCATOR Allocator;
    const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING EventName;
    WCHAR EventNameBuffer[256];

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            (PUNICODE_STRING)&TracerRegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    EventName.Length = 0;
    EventName.MaximumLength = sizeof(EventNameBuffer);
    EventName.Buffer = (PWSTR)&EventNameBuffer;

    Success = Rtl->CreateNamedEvent(Rtl,
                                    &Allocator,
                                    &EventHandle,
                                    NULL,
                                    FALSE,
                                    FALSE,
                                    (PCUNICODE_STRING)&Prefix,
                                    NULL,
                                    &EventName);

    if (!Success) {
        __debugbreak();
    }


#endif

#if 0
    UNICODE_STRING Path = RTL_CONSTANT_STRING(L"\\\\?\\S:\\trace\\2017-04-29-194742.551\\TraceStore_ImageFile.dat");

    ExitCode = FindStreams(&Path);

#endif

#if 0
    RTL Rtl;
    ULONG SizeOfRtl = sizeof(Rtl);

    if (!InitializeRtl(&Rtl, &SizeOfRtl)) {
        ExitCode = 1;
        goto Error;
    }

    if (Rtl.TsxAvailable) {
        NOTHING;
    }

#endif

    //SleepEx(INFINITE, TRUE);

    goto Error;

Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
