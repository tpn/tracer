/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    Scratch/testing file.

--*/

#include "stdafx.h"

#if 0

typedef struct _TEST1 {
    CHAR Char;
    LONGLONG LongLong;
    WCHAR WideChar;
    union {
        ULONG Bitfield1:1;
        PVOID VoidPointer;
        ULONG Bitfield2:1;
    };
} TEST1;

typedef struct _TEST2 {
    BYTE Foo[1];
    BYTE Bar[2][3];
    WORD Moo[4][5][6];
    GUID Guid;
    XMMWORD Xmm0;
    YMMWORD Ymm0;
    FLOAT Float;
    DOUBLE Double;
    union {
        ULONG Bitfield1:1;
        PVOID VoidPointer;
        ULONG Bitfield2:1;
    };
    ALLOCATOR Allocator3DArray[1][2][3];
    CHAR Char;
    union {
        LONG Long;
        LONGLONG LongLong;
    } NamedUnion1;

    struct {
        SHORT Short;
        USHORT UShort;
    } NamedStruct2;

} TEST2;

typedef struct _TEST3 {
    union {
        SLIST_ENTRY ListEntry;
        struct {
            PSLIST_ENTRY Next;
            PVOID Unused;
        };
    };
} TEST3;

#endif

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

RTL GlobalRtl;
ALLOCATOR GlobalAllocator;

PRTL Rtl;
PALLOCATOR Allocator;

HMODULE GlobalModule = 0;

STRING_TABLE_API_EX GlobalApi;
PSTRING_TABLE_API_EX Api;

HMODULE GlobalRtlModule = 0;
HMODULE GlobalStringTableModule = 0;

VOID
Scratch1(
    VOID
    )
{
    LONG ExitCode = 0;
    LONG SizeOfRtl = sizeof(GlobalRtl);
    HMODULE RtlModule;
    RTL_BOOTSTRAP Bootstrap;

    if (!BootstrapRtl(&RtlModule, &Bootstrap)) {
        ExitCode = 1;
        goto Error;
    }

    if (!Bootstrap.InitializeHeapAllocator(&GlobalAllocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        Bootstrap.InitializeRtl(&GlobalRtl, &SizeOfRtl),
        "InitializeRtl()"
    );

    Rtl = &GlobalRtl;
    Allocator = &GlobalAllocator;

    SetCSpecificHandler(Rtl->__C_specific_handler);

Error:
    return;

}

VOID
AlignmentScratch(
    VOID
    )
{
    BYTE Index;
    BYTE Count;
    ULONG_PTR Address;
    ULONG_PTR AlignedUpAddress[10];
    ULONG_PTR AlignedDownAddress[10];
    ULONG_PTR AddressAlignment[40];
    ULONG_PTR Addresses[] = {
        0x00007ffd11483294,
        0x00007ffd114832c1,
        0x00007ffd1148329d,
    };

    ZeroStruct(AlignedUpAddress);
    ZeroStruct(AlignedDownAddress);
    ZeroStruct(AddressAlignment);

    Count = ARRAYSIZE(Addresses);

    for (Index = 0; Index < Count; Index++) {
        Address = Addresses[Index];

        AddressAlignment[Index] = GetAddressAlignment((PVOID)Address);

        AlignedUpAddress[0] = ALIGN_UP(Address, 2);
        AlignedUpAddress[1] = ALIGN_UP(Address, 4);
        AlignedUpAddress[2] = ALIGN_UP(Address, 8);
        AlignedUpAddress[3] = ALIGN_UP(Address, 16);
        AlignedUpAddress[4] = ALIGN_UP(Address, 256);
        AlignedUpAddress[5] = ALIGN_UP(Address, 512);

        AlignedDownAddress[0] = ALIGN_DOWN(Address, 2);
        AlignedDownAddress[1] = ALIGN_DOWN(Address, 4);
        AlignedDownAddress[2] = ALIGN_DOWN(Address, 8);
        AlignedDownAddress[3] = ALIGN_DOWN(Address, 16);
        AlignedDownAddress[4] = ALIGN_DOWN(Address, 256);
        AlignedDownAddress[5] = ALIGN_DOWN(Address, 512);
    }
}

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    LONG ExitCode = 0;

#if 1
    ULONGLONG Value;

    Value = RoundUpPowerOf2(0);
    Value = RoundUpPowerOf2(2);
    Value = RoundUpPowerOf2(3);
    Value = RoundUpPowerOf2(31);
    Value = RoundUpPowerOf2(32);
    Value = RoundUpPowerOf2(33);

    Value = RoundUpNextPowerOf2(31);
    Value = RoundUpNextPowerOf2(32);
    Value = RoundUpNextPowerOf2(32);

    ASSERT(RoundUpPowerOf2(0) == 2);
    ASSERT(RoundUpPowerOf2(2) == 2);
    ASSERT(RoundUpPowerOf2(3) == 4);
    ASSERT(RoundUpPowerOf2(31) == 32);
    ASSERT(RoundUpPowerOf2(32) == 32);
    ASSERT(RoundUpPowerOf2(33) == 64);

    ASSERT(RoundUpNextPowerOf2(31) == 32);
    ASSERT(RoundUpNextPowerOf2(32) == 64);
    ASSERT(RoundUpNextPowerOf2(32) == 64);

#endif

#if 0

    AlignmentScratch();
    //Scratch1();
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
    BOOL Success;
    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    //HANDLE EventHandle;
    ALLOCATOR Allocator;
    const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING Name;

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

    Success = Rtl->CreateSingleRandomObjectName(Rtl,
                                                &Allocator,
                                                &Allocator,
                                                &Prefix,
                                                &Name);

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
