/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableHash.c

Abstract:

    This module implements routines that hash a 32-bit value into a 64-bit
    value comprised of two independent 32-bit hashes.  Each routine corresponds
    to one of the PERFECT_HASH_TABLE_HASH_FUNCTION_ID enumerations.

--*/

#include "stdafx.h"

_Use_decl_annotations_
HRESULT
PerfectHashTableSeededHash01(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    ULONG NumberOfSeeds,
    PULONG Seeds,
    PULONGLONG Hash
    )
/*++

Routine Description:

    This hash routine uses a combination of CRC32 and rotates.

Arguments:

    Table - Supplies a pointer to the table for which the hash is being created.

    Input - Supplies the input value to hash.

    NumberOfSeeds - Supplies the number of elements in the Seeds array.

    Seeds - Supplies an array of ULONG seed values.

    Masked - Receives two 32-bit hashes merged into a 64-bit value.

Return Value:

    S_OK on success.  If the two 32-bit hash values are identical, E_FAIL.

--*/
{
    ULONG A;
    ULONG B;
    ULONG C;
    ULONG D;
    ULONG Seed1;
    ULONG Seed2;
    ULONG Seed3;
    ULONG Vertex1;
    ULONG Vertex2;
    ULARGE_INTEGER Result;

    ASSERT(NumberOfSeeds >= 3);

    //
    // Initialize aliases.
    //

    //IACA_VC_START();

    Seed1 = Seeds[0];
    Seed2 = Seeds[1];
    Seed3 = Seeds[2];

    //
    // Calculate the individual hash parts.
    //

    A = _mm_crc32_u32(Seed1, Input);
    B = _mm_crc32_u32(Seed2, _rotl(Input, 15));
    C = Seed3 ^ Input;
    D = _mm_crc32_u32(B, C);

    //IACA_VC_END();

    Vertex1 = A;
    Vertex2 = D;

    if (Vertex1 == Vertex2) {
        return E_FAIL;
    }

    Result.LowPart = Vertex1;
    Result.HighPart = Vertex2;

    *Hash = Result.QuadPart;

    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableHash01(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    PULONGLONG Hash
    )
{
    return PerfectHashTableSeededHash01(Table,
                                        Input,
                                        Table->Header->NumberOfSeeds,
                                        &Table->Header->FirstSeed,
                                        Hash);
}

_Use_decl_annotations_
HRESULT
PerfectHashTableSeededHash02(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    ULONG NumberOfSeeds,
    PULONG Seeds,
    PULONGLONG Hash
    )
/*++

Routine Description:

    This hash routine uses some rotates and xor, inspired by nothing in
    particular other than it would be nice having a second hash routine,
    ideally with poor randomness characteristics, such that it makes a marked
    difference on the ability to solve the graphs.

Arguments:

    Table - Supplies a pointer to the table for which the hash is being created.

    Input - Supplies the input value to hash.

    NumberOfSeeds - Supplies the number of elements in the Seeds array.

    Seeds - Supplies an array of ULONG seed values.

    Masked - Receives two 32-bit hashes merged into a 64-bit value.

Return Value:

    S_OK on success.  If the two 32-bit hash values are identical, E_FAIL.

--*/
{
    ULONG A;
    ULONG B;
    ULONG C;
    ULONG D;
    ULONG Seed1;
    ULONG Seed2;
    ULONG Seed3;
    ULONG Seed4;
    ULONG Vertex1;
    ULONG Vertex2;
    ULARGE_INTEGER Result;

    ASSERT(NumberOfSeeds >= 3);

    //
    // Initialize aliases.
    //

    Seed1 = Seeds[0];
    Seed2 = Seeds[1];
    Seed3 = Seeds[2];
    Seed4 = Seeds[3];

    //
    // Calculate the individual hash parts.
    //

    A = _rotl(Input ^ Seed1, 15);
    B = _rotl(Input + Seed2, 7);
    C = _rotr(Input - Seed3, 11);
    D = _rotr(Input ^ Seed4, 20);

    Vertex1 = A ^ C;
    Vertex2 = B ^ D;

    if (Vertex1 == Vertex2) {
        return E_FAIL;
    }

    Result.LowPart = Vertex1;
    Result.HighPart = Vertex2;

    *Hash = Result.QuadPart;
    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableHash02(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    PULONGLONG Hash
    )
{
    return PerfectHashTableSeededHash02(Table,
                                        Input,
                                        Table->Header->NumberOfSeeds,
                                        &Table->Header->FirstSeed,
                                        Hash);
}

_Use_decl_annotations_
HRESULT
PerfectHashTableSeededHash03(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    ULONG NumberOfSeeds,
    PULONG Seeds,
    PULONGLONG Hash
    )
/*++

Routine Description:

    This hash routine is based off version 2, with fewer rotates and xors.

Arguments:

    Table - Supplies a pointer to the table for which the hash is being created.

    Input - Supplies the input value to hash.

    NumberOfSeeds - Supplies the number of elements in the Seeds array.

    Seeds - Supplies an array of ULONG seed values.

    Masked - Receives two 32-bit hashes merged into a 64-bit value.

Return Value:

    S_OK on success.  If the two 32-bit hash values are identical, E_FAIL.

--*/
{
    ULONG A;
    ULONG B;
    ULONG C;
    ULONG D;
    ULONG Seed1;
    ULONG Seed2;
    ULONG Seed3;
    ULONG Vertex1;
    ULONG Vertex2;
    ULARGE_INTEGER Result;

    ASSERT(NumberOfSeeds >= 3);

    //
    // Initialize aliases.
    //

    Seed1 = Seeds[0];
    Seed2 = Seeds[1];
    Seed3 = Seeds[2];

    //
    // Calculate the individual hash parts.
    //

    A = Input + Seed1;
    B = Input - Seed2;
    C = A ^ B;
    D = C ^ Seed3;

    Vertex1 = A;
    Vertex2 = D;

    if (Vertex1 == Vertex2) {
        return E_FAIL;
    }

    Result.LowPart = Vertex1;
    Result.HighPart = Vertex2;

    *Hash = Result.QuadPart;
    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableHash03(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    PULONGLONG Hash
    )
{
    return PerfectHashTableSeededHash03(Table,
                                        Input,
                                        Table->Header->NumberOfSeeds,
                                        &Table->Header->FirstSeed,
                                        Hash);
}


_Use_decl_annotations_
HRESULT
PerfectHashTableSeededHash04(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    ULONG NumberOfSeeds,
    PULONG Seeds,
    PULONGLONG Hash
    )
/*++

Routine Description:

    This is the simplest possible hash I could think of, simply xor'ing the
    input with each seed value.

Arguments:

    Table - Supplies a pointer to the table for which the hash is being created.

    Input - Supplies the input value to hash.

    NumberOfSeeds - Supplies the number of elements in the Seeds array.

    Seeds - Supplies an array of ULONG seed values.

    Masked - Receives two 32-bit hashes merged into a 64-bit value.

Return Value:

    S_OK on success.  If the two 32-bit hash values are identical, E_FAIL.

--*/
{
    ULONG Seed1;
    ULONG Seed2;
    ULONG Vertex1;
    ULONG Vertex2;
    ULARGE_INTEGER Result;

    ASSERT(NumberOfSeeds >= 2);

    IACA_VC_START();

    //
    // Initialize aliases.
    //

    Seed1 = Seeds[0];
    Seed2 = Seeds[1];

    //
    // Calculate the individual hash parts.
    //

    Vertex1 = Input ^ Seed1;
    Vertex2 = Input ^ Seed2;

    if (Vertex1 == Vertex2) {
        return E_FAIL;
    }

    Result.LowPart = Vertex1;
    Result.HighPart = Vertex2;

    *Hash = Result.QuadPart;

    IACA_VC_END();

    return S_OK;
}

_Use_decl_annotations_
HRESULT
PerfectHashTableHash04(
    PPERFECT_HASH_TABLE Table,
    ULONG Input,
    PULONGLONG Hash
    )
{
    return PerfectHashTableSeededHash04(Table,
                                        Input,
                                        Table->Header->NumberOfSeeds,
                                        &Table->Header->FirstSeed,
                                        Hash);
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
