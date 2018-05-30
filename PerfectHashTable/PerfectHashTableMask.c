/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableMask.c

Abstract:

    This module implements routines that mask a value such that it remains
    within the bounds of a given hash table's underlying size.  Each routine
    corresponds to one of the PERFECT_HASH_TABLE_MASKING_TYPE enumeration
    values.

--*/

#include "stdafx.h"

_Use_decl_annotations_
HRESULT
PerfectHashTableMaskModulus(
    PPERFECT_HASH_TABLE Table,
    ULONGLONG Input,
    PULONG Masked
    )
/*++

Routine Description:

    Returns the input value modulus the table size.

Arguments:

    Table - Supplies a pointer to the table for which the mask will be derived.

    Input - Supplies the input value to mask.

    Masked - Receives the masked value.

Return Value:

    S_OK.

--*/
{
    *Masked = (ULONG)(Input % Table->Size);
    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableMaskShift(
    PPERFECT_HASH_TABLE Table,
    ULONGLONG Input,
    PULONG Masked
    )
/*++

Routine Description:

    Returns the input value shifted by the table shift amount.  This routine
    should only be used for tables that are powers-of-2 sized.

Arguments:

    Table - Supplies a pointer to the table for which the mask will be derived.

    Input - Supplies the input value to mask.

    Masked - Receives the masked value.

Return Value:

    S_OK.

--*/
{
    *Masked = (ULONG)(Input >> Table->Shift);
    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableMaskAnd(
    PPERFECT_HASH_TABLE Table,
    ULONGLONG Input,
    PULONG Masked
    )
/*++

Routine Description:

    Returns the input value masked (AND'd) by the table shift value minus 1.

Arguments:

    Table - Supplies a pointer to the table for which the mask will be derived.

    Input - Supplies the input value to mask.

    Masked - Receives the masked value.

Return Value:

    S_OK.

--*/
{
    *Masked = (ULONG)(Input & (Table->Shift - 1));
    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableMaskXorAnd(
    PPERFECT_HASH_TABLE Table,
    ULONGLONG Input,
    PULONG Masked
    )
/*++

Routine Description:

    Returns the high dword and low dword of the input XORd, then masked
    by the table shift value minus 1.

Arguments:

    Table - Supplies a pointer to the table for which the mask will be derived.

    Input - Supplies the input value to mask.

    Masked - Receives the masked value.

Return Value:

    S_OK.

--*/
{
    ULARGE_INTEGER Mask;

    Mask.QuadPart = Input;

    *Masked = (ULONG)((Mask.HighPart ^ Mask.LowPart) & (Table->Shift - 1));
    return S_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
