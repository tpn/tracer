/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStorePath.c

Abstract:

    This module implements functions related to trace store path handling.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
FindLongestTraceStoreFileName(
    PUSHORT LengthPointer
    )
{
    DWORD Index;
    NTSTATUS Result;
    ULARGE_INTEGER Longest = { 0 };
    ULARGE_INTEGER Length = { 0 };
    DWORD MaxPath = (
        _OUR_MAX_PATH   -
        3               - /* C:\ */
        1               - // NUL
        LongestTraceStoreSuffixLength
    );

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        LPCWSTR FileName = TraceStoreFileNames[Index];
        Result = StringCchLengthW(
            FileName,
            MaxPath,
            (PSIZE_T)&Length.QuadPart
        );
        if (FAILED(Result)) {
            return FALSE;
        }
        if (Length.QuadPart > Longest.QuadPart) {
            Longest.QuadPart = Length.QuadPart;
        }
    }
    Longest.QuadPart += LongestTraceStoreSuffixLength;

    if (Longest.QuadPart > MAX_STRING) {
        return FALSE;
    }

    *LengthPointer = (USHORT)Longest.LowPart;

    return TRUE;
}

_Use_decl_annotations_
ULONG
GetLongestTraceStoreFileName(VOID)
{
    BOOL Success;
    USHORT Length;

    Success = FindLongestTraceStoreFileName(&Length);

    if (!Success) {
        return 0;
    }

    return Length;
}

_Use_decl_annotations_
BOOL
InitializeTraceStorePath(
    PCWSTR Path,
    PTRACE_STORE TraceStore
    )
{
#ifdef _TRACE_STORE_EMBED_PATH
    HRESULT Result;
    ULARGE_INTEGER Length;
    ULARGE_INTEGER MaximumLength;

    MaximumLength.HighPart = 0;
    MaximumLength.LowPart = sizeof(TraceStore->PathBuffer);

    Result = StringCbLengthW(Path, MaximumLength.QuadPart, &Length.QuadPart);

    if (FAILED(Result)) {
        return FALSE;
    }

    TraceStore->Path.Length = (USHORT)Length.LowPart;
    TraceStore->Path.MaximumLength = (USHORT)MaximumLength.LowPart;
    TraceStore->Path.Buffer = &TraceStore->PathBuffer[0];

    __movsw((PWORD)TraceStore->Path.Buffer,
            (PWORD)Path,
            (TraceStore->Path.Length >> 1));

    TraceStore->PathBuffer[TraceStore->Path.Length >> 1] = L'\0';

#endif
    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
