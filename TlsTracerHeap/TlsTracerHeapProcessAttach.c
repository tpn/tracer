#include "stdafx.h"

_Use_decl_annotations_
BOOL
TlsTracerHeapProcessAttach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
{

    TlsIndex = TlsAlloc();

    if (TlsIndex == TLS_OUT_OF_INDEXES) {
        OutputDebugStringA("TlsAlloc() returned TLS_OUT_OF_INDEXES.\n");
        return FALSE;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
