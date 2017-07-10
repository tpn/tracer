/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

typedef struct _TEST_CONTEXT {
    PINJECTION_OBJECTS InjectionObjects;
    ULONG Test1;
    ULONG Test2;
} TEST_CONTEXT;
typedef TEST_CONTEXT *PTEST_CONTEXT;

DECLSPEC_DLLEXPORT
ULONG
TestInjectedObjectsThreadEntry(
    PTEST_CONTEXT TestContext,
    PINJECTION_OBJECTS InjectionObjects,
    PINJECTION_FUNCTIONS Functions
    )
{
    ULONG WaitResult;
    PULONG Dummy;
    PINJECTION_OBJECT Object;
    PINJECTION_OBJECT_FILE_MAPPING Shared;
    PINJECTION_OBJECT_FILE_MAPPING Log;
    PINJECTION_OBJECT_EVENT InjectionComplete;
    PINJECTION_OBJECT_EVENT InjectionContinue;

    Object = InjectionObjects->Objects;

    InjectionComplete = &Object->AsEvent;
    InjectionContinue = &(Object + 1)->AsEvent;
    Shared = &(Object + 4)->AsFileMapping;
    Log = &(Object + 6)->AsFileMapping;

    Dummy = (PULONG)Shared->BaseAddress;

    WaitResult = Functions->SignalObjectAndWait(InjectionComplete->Handle,
                                                InjectionContinue->Handle,
                                                INFINITE,
                                                TRUE);   

    return 0;
}


BOOL
TestInjectionObjects(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    LONG ExitCode = 1;

    BOOL Success;
    USHORT Index;
    //USHORT Count;
    USHORT MappingIndex;
    USHORT NumberOfObjects;
    USHORT NumberOfEvents;
    USHORT NumberOfFileMappings;
    ULONG SizeOfBuffer;
    ULONG LastError;
    ULONG WaitResult;
    ULONG RemoteThreadId;
    HANDLE Handle;
    HANDLE RemoteThreadHandle;
    PWSTR WideBuffer;
    PVOID BaseAddress;
    PVOID RemoteBaseCodeAddress;
    PVOID RemoteDataBufferAddress;
    PUNICODE_STRING Name;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    UNICODE_STRING LogFile = RTL_CONSTANT_STRING(L"ScratchLog.txt");
    INJECTION_OBJECT Objects[7];
    UNICODE_STRING ObjectNames[7];
    UNICODE_STRING ObjectPrefixes[] = {
        RTL_CONSTANT_STRING(L"Event1_"),
        RTL_CONSTANT_STRING(L"Event2_"),
        RTL_CONSTANT_STRING(L"Event3_"),
        RTL_CONSTANT_STRING(L"Event4_"),
        RTL_CONSTANT_STRING(L"SharedMem1_"),
        RTL_CONSTANT_STRING(L"SharedMem2_"),
        RTL_CONSTANT_STRING(L"Log_"),
    };
    PUNICODE_STRING Names[] = {
        &ObjectNames[0],
        &ObjectNames[1],
        &ObjectNames[2],
        &ObjectNames[3],
        &ObjectNames[4],
        &ObjectNames[5],
        &ObjectNames[6],
    };
    PUNICODE_STRING Prefixes[] ={
        &ObjectPrefixes[0],
        &ObjectPrefixes[1],
        &ObjectPrefixes[2],
        &ObjectPrefixes[3],
        &ObjectPrefixes[4],
        &ObjectPrefixes[5],
        &ObjectPrefixes[6],
    };
    ULONG FileMappingDesiredAccess[] = {
        FILE_MAP_READ,
        FILE_MAP_WRITE,
        FILE_MAP_WRITE,
    };
    ULONG FileMappingPageProtection[] = {
        PAGE_READONLY,
        PAGE_READWRITE,
        PAGE_READWRITE,
    };
    LARGE_INTEGER MappingSizes[] = {
        {
            1 << 16,    // 64KB
            0,
        },
        {
            1 << 24,    // 24MB
            0,
        },
        {
            1 << 16,    // 64KB
            0,
        },
    };
    LARGE_INTEGER MappingSize;
    INJECTION_OBJECTS InjectionObjects;
    INJECTION_THUNK_FLAGS InjectionThunkFlags;
    PINJECTION_OBJECT Object;
    PINJECTION_OBJECT_EVENT Event;
    PINJECTION_OBJECT_FILE_MAPPING FileMapping;
    TEST_CONTEXT TestContext;
    PVOID LocalBaseCodeAddress;
    PVOID LocalThunkBufferAddress;
    PVOID LocalUserDataAddress;
    volatile ULONG *Dummy;
    STRING FunctionName = RTL_CONSTANT_STRING("TestInjectedObjectsThreadEntry");

    ZeroStruct(Objects);
    ZeroStruct(InjectionObjects);
    ZeroStruct(TestContext);

    NumberOfObjects = ARRAYSIZE(ObjectNames);
    NumberOfEvents = 4;
    NumberOfFileMappings = 3;

    if (!Rtl->InitializeInjection(Rtl)) {
        goto Error;
    }

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        __debugbreak();
        LastError = GetLastError();
        goto Error;
    }

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           Allocator,
                                           Allocator,
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
        InitializeUnicodeStringFromUnicodeString(&Event->Name, Name);
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

    Object = &Objects[ARRAYSIZE(Objects)-1];
    FileMapping = &Object->AsFileMapping;
    InitializeUnicodeStringFromUnicodeString(&FileMapping->Path, &LogFile);

    for (Index = NumberOfEvents, MappingIndex = 0;
         Index < NumberOfObjects;
         Index++, MappingIndex++) {

        Name = Names[Index];
        Object = &Objects[Index];
        FileMapping = &Object->AsFileMapping;
        InitializeUnicodeStringFromUnicodeString(&FileMapping->Name, Name);
        FileMapping->Type.AsId = FileMappingInjectionObjectId;
        FileMapping->DesiredAccess = FileMappingDesiredAccess[MappingIndex];
        FileMapping->PageProtection = FileMappingPageProtection[MappingIndex];
        MappingSize.QuadPart = MappingSizes[MappingIndex].QuadPart;
        FileMapping->MappingSize.QuadPart = MappingSize.QuadPart;

        if (FileMapping->Path.Buffer) {

            Handle = Rtl->CreateFileW(FileMapping->Path.Buffer,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_DELETE,
                                      NULL,
                                      OPEN_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

            LastError = GetLastError();

            if (!Handle || Handle == INVALID_HANDLE_VALUE) {
                __debugbreak();
                goto Error;
            }

            FileMapping->FileHandle = Handle;

            if (FALSE && LastError == ERROR_ALREADY_EXISTS) {
                FILE_STANDARD_INFO FileInfo;
                FILE_INFO_BY_HANDLE_CLASS Class;

                Class = (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo;

                Success = GetFileInformationByHandleEx(Handle,
                                                       Class,
                                                       &FileInfo,
                                                       sizeof(FileInfo));
                if (!Success) {
                    __debugbreak();
                    goto Error;
                }

                FileMapping->FileOffset.QuadPart = FileInfo.EndOfFile.QuadPart;
            }
        }

        Handle = Rtl->CreateFileMappingW(FileMapping->FileHandle,
                                         NULL,
                                         PAGE_READWRITE,
                                         FileMapping->MappingSize.HighPart,
                                         FileMapping->MappingSize.LowPart,
                                         FileMapping->Name.Buffer);

        LastError = GetLastError();

        if (!Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }

        FileMapping->Handle = Handle;

        BaseAddress = Rtl->MapViewOfFileEx(Handle,
                                           FILE_MAP_ALL_ACCESS,
                                           FileMapping->FileOffset.HighPart,
                                           FileMapping->FileOffset.LowPart,
                                           FileMapping->MappingSize.QuadPart,
                                           FileMapping->BaseAddress);

        if (!BaseAddress) {
            LastError = GetLastError();
            __debugbreak();
            goto Error;
        }

        FileMapping->PreferredBaseAddress = BaseAddress;
    }

    //
    // Initialize the INJECTION_OBJECTS container.
    //

    InjectionObjects.SizeOfStruct = sizeof(InjectionObjects);
    InjectionObjects.SizeOfInjectionObjectInBytes = sizeof(INJECTION_OBJECT);
    InjectionObjects.NumberOfObjects = NumberOfObjects;
    InjectionObjects.TotalAllocSizeInBytes = 0;
    InjectionObjects.Objects = Objects;

    //
    // Perform injection.
    //

    InjectionThunkFlags.AsULong = 0;
    InjectionThunkFlags.DebugBreakOnEntry = TRUE;
    InjectionThunkFlags.HasInjectionObjects = TRUE;
    InjectionThunkFlags.HasModuleAndFunction = TRUE;

    Success = Rtl->Inject(Rtl,
                          Allocator,
                          InjectionThunkFlags,
                          ProcessInfo.hProcess,
                          TestInjectionActiveExePath,     // ModulePath
                          &FunctionName,                  // FunctionName
                          NULL,                           // Apc
                          (PBYTE)&TestContext,
                          sizeof(TestContext),
                          &InjectionObjects,
                          NULL,                   // AdjustUserDataPointers
                          &RemoteThreadHandle,
                          &RemoteThreadId,
                          &RemoteBaseCodeAddress,
                          &RemoteDataBufferAddress,
                          &LocalBaseCodeAddress,
                          &LocalThunkBufferAddress,
                          &LocalUserDataAddress);

    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // Do some SignalObjectAndWait()'s against the new events... some shared
    // memory read/writes, etc.
    //

    WaitResult = WaitForSingleObject(Objects[0].Handle, INFINITE);

    Dummy = (PULONG)Objects[4].AsFileMapping.BaseAddress;

    *Dummy = 1;

    WaitResult = SignalObjectAndWait(Objects[1].Handle, Objects[2].Handle, INFINITE, FALSE);
    
    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

End:

    TerminateProcess(ProcessInfo.hProcess, 0);

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
