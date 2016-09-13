/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    SearchStringTable.c

Abstract:

    This module implements routines related to searching for a string within
    a STRING_TABLE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
SHORT
SearchStringTableSlotsForFirstPrefixMatch(
    PSTRING_TABLE StringTable,
    PSTRING String,
    USHORT Bitmap,
    PSTRING_MATCH Match
    )
/*++

Routine Description:

    This routine searches slots within a string table for the given input
    string based on the bitmap index provided.

Arguments:

    StringTable - Supplies a pointer to a STRING_TABLE structure to search.

    String - Supplies a pointer to a STRING structure to search for.

    Bitmap - Supplies a short integer value where each bit corresponds to a
        slot to search.  At least one bit must be set.

    Match - Optionally supplies a pointer to a STRING_MATCH structure that will
        receive the details of the match, if any.

Return Value:

    Returns the index of the slot for the first prefix match that was satisfied,
    otherwise, returns NO_MATCH_FOUND.

--*/
{
    ULONG Count;
    ULONG Length;
    ULONG Mask;
    ULONG Index;
    ULONG Shift = 0;
    ULONG CharactersMatched;
    ULONG NumberOfTrailingZeros;
    STRING_SLOT Slot;
    STRING_SLOT Compare;
    STRING_SLOT Source;
    SLOT_LENGTHS Lengths;

    //
    // Load the search string into a 128-byte register.  We can't assume
    // anything about whether or not the search string has an optimal buffer
    // alignment or length (such that a SIMD load could be satisfied), so we
    // use a simple rep movsb intrinsic.
    //

    Source.Chars128 = _mm_setzero_si128();
    __movsb(Source.Char, String->Buffer, String->Length);

    //
    // Load the slot lengths.
    //

    Lengths.Slots256 = _mm256_load_si256(&StringTable->Lengths.Slots256);

    //
    // Get the number of bits set.
    //

    Count = __popcnt16(Bitmap);

    while (Count--) {

        //
        // Extract the next index by counting the number of trailing zeros left
        // in the bitmap and adding the amount we've already shifted by.
        //

        NumberOfTrailingZeros = _tzcnt_u32(Bitmap);
        Index = NumberOfTrailingZeros + Shift;

        //
        // Shift the bitmap right, past the zeros and the 1 that was just found,
        // such that it's positioned correctly for the next loop's tzcnt.  Update
        // the shift count accordingly.
        //

        Bitmap >>= (NumberOfTrailingZeros + 1);
        Shift = Index + 1;

        //
        // Decrement the index to account for the 0-based array indexing versus
        // 1-based bit masks, then load the slot and its length.
        //

        Slot.Chars128 = _mm_load_si128(&StringTable->Slots[Index].Chars128);
        Length = Lengths.Slots[Index];

        //
        // Compare the slot to the search string.
        //

        Compare.Chars128 = _mm_cmpeq_epi8(Slot.Chars128, Source.Chars128);

        //
        // Create a mask of the comparison, then filter out high bits from the
        // length onward.
        //

        Mask = _bzhi_u32(_mm_movemask_epi8(Compare.Chars128), String->Length);

        //
        // Count how many characters matched.
        //

        CharactersMatched = __popcnt(Mask);

        if ((USHORT)CharactersMatched == Length) {

            //
            // This slot is a prefix match.  Fill out the Match structure if the
            // caller provided a non-NULL pointer, then return the index of the
            // match.
            //

            if (ARGUMENT_PRESENT(Match)) {

                Match->Index = Index;
                Match->NumberOfMatchedCharacters = (USHORT)CharactersMatched;
                Match->String = &StringTable->pStringArray->Strings[Index];

            }

            return (SHORT)Index;
        }

        //
        // Not enough characters matched, so continue the loop.
        //

    } while (--Count);


    //
    // If we get here, we didn't find a match.
    //

    return NO_MATCH_FOUND;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
