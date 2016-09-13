#include "stdafx.h"

_Use_decl_annotations_
BOOL
TlsTracerHeapProcessDetach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
{
    BOOL Success;

    if (TlsIndex == TLS_OUT_OF_INDEXES) {

        //
        // We shouldn't be able to get into this state, as TlsAlloc() is
        // attempted at DLL load time (DLL_PROCESS_ATTACH), and will indicate
        // an error if it failed, blocking the DLL from being loaded.
        //

        goto End;
    }

    Success = TlsFree(TlsIndex);

    if (!Success) {
        OutputDebugStringA("TlsFree() failed.\n");
    }

End:

    //
    // Note that we always return TRUE here, even if we had a failure.  We're
    // only called at DLL_PROCESS_DETACH, so there's not much we can do when
    // there is actually an error anyway.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
