/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Loader.c

Abstract:

    This module implements various loader related routines.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
TestWalkLoader(
    PRTL Rtl
    )
/*++

Routine Description:

    TBD

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

Return Value:

    None.

--*/
{
    HRESULT Result;
    ULONG Length;
    ULONG FramesToSkip;
    ULONG FramesToCapture;
    ULONG Disposition;
    PVOID Cookie;
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    PPEB Peb;
    PPEB Peb1;
    PPEB Peb2;
    PCRITICAL_SECTION LoaderLock;
    HANDLE CurrentProcess;
    HANDLE CurrentThread;
    USHORT FramesCaptured;
    PVOID BackTrace;
    ULONG BackTraceHash;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PLDR_DATA_TABLE_ENTRY Module;

    CurrentProcess = GetCurrentProcess();
    CurrentThread = GetCurrentThread();

    Result = Rtl->ZwQueryInformationProcess(CurrentProcess,
                                            ProcessBasicInformation,
                                            &ProcessBasicInfo,
                                            sizeof(ProcessBasicInfo),
                                            &Length);

    if (FAILED(Result)) {
        return TRUE;
    }

    Peb1 = ProcessBasicInfo.PebBaseAddress;
    Peb2 = NtCurrentPeb();
    Peb = Peb1;

    if (Peb1 != Peb2) {
        __debugbreak();
    }

    LoaderLock = Peb->LoaderLock;

    FramesToSkip = 0;
    FramesToCapture = 32;
    FramesCaptured = Rtl->RtlCaptureStackBackTrace(FramesToSkip,
                                                   FramesToCapture,
                                                   &BackTrace,
                                                   &BackTraceHash);

    if (FramesCaptured == 0) {
        __debugbreak();
    }

    Result = Rtl->LdrLockLoaderLock(0, &Disposition, &Cookie);
    if (Disposition != LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED) {
        __debugbreak();
    }

    ListHead = &Peb->Ldr->InLoadOrderModuleList;
    ListEntry = ListHead->Flink;

    while (ListEntry != ListHead) {
        Module = CONTAINING_RECORD(ListEntry,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderModuleList);

        OutputDebugStringW(Module->FullDllName.Buffer);

        ListEntry = ListEntry->Flink;
    }

    Result = Rtl->LdrUnlockLoaderLock(0, Cookie);

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
