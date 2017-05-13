/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreKernels.cu

Abstract:

    This module implements CUDA kernels for various trace store functions.

--*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Cu.cuh"
#include <no_sal2.h>

GLOBAL
VOID
SinglePrecisionAlphaXPlusY(
    _In_ LONG Total,
    _In_ FLOAT Alpha,
    _In_ PFLOAT X,
    _Out_ PFLOAT Y
    )
{
    LONG Index;

    FOR_EACH_1D(Index, Total) {
        Y[Index] = Alpha * X[Index] + Y[Index];
    }
}

GLOBAL
VOID
DeltaTimestamp(
    _In_ ULONG64 Total,
    _In_ PULONG64 Timestamp,
    _Out_ PULONG64 Delta
    )
{
    ULONG64 Index;

    if (ThreadIndex.x % 32 == 0) {
        return;
    }

    FOR_EACH_1D(Index, Total) {
        Delta[Index] = Timestamp[Index] - Timestamp[Index-1];
    }
}

#ifdef __cplusplus
}
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
