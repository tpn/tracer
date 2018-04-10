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
STRING_TABLE_INDEX
IsStringInTable(
    PSTRING_TABLE StringTable,
    PCSTRING String,
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
    ULONG SearchLength;
    PSTRING TargetString;
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

    SearchLength = min(String->Length, 16);
    SecureZeroMemory(&Source, sizeof(Source));
    __movsb(Source.Char, String->Buffer, SearchLength);

    //
    // Load the slot lengths.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

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
        // such that it's positioned correctly for the next loop's tzcnt. Update
        // the shift count accordingly.
        //

        Bitmap >>= (NumberOfTrailingZeros + 1);
        Shift = Index + 1;

        //
        // Load the slot and its length.
        //

        Slot.CharsXmm = _mm_load_si128(&StringTable->Slots[Index].CharsXmm);
        Length = Lengths.Slots[Index];

        //
        // Compare the slot to the search string.
        //

        Compare.CharsXmm = _mm_cmpeq_epi8(Slot.CharsXmm, Source.CharsXmm);

        //
        // Create a mask of the comparison, then filter out high bits from the
        // search string's length (which is capped at 16).  (This shouldn't be
        // technically necessary as the string array buffers should have been
        // calloc'd and zeroed, but optimizing compilers can often ignore the
        // zeroing request -- which can produce some bizarre results where the
        // debug build is correct (because the buffers were zeroed) but the
        // release build fails because the zeroing got ignored and there are
        // junk bytes past the NULL terminator, which get picked up in our
        // 128-bit loads.)
        //

        Mask = _bzhi_u32(_mm_movemask_epi8(Compare.CharsXmm), SearchLength);

        //
        // Count how many characters matched.
        //

        CharactersMatched = __popcnt(Mask);

        if ((USHORT)CharactersMatched == 16 && Length > 16) {

            //
            // The first 16 characters in the string matched against this
            // slot, and the slot is oversized (longer than 16 characters),
            // so do a direct comparison between the remaining buffers.
            //

            TargetString = &StringTable->pStringArray->Strings[Index];

            CharactersMatched = IsPrefixMatch(String, TargetString, 16);

            if (CharactersMatched == NO_MATCH_FOUND) {

                //
                // The prefix match failed, continue our search.
                //

                continue;

            } else {

                //
                // We successfully prefix matched the search string against
                // this slot.  The code immediately following us deals with
                // handling a successful prefix match at the initial slot
                // level; let's avoid an unnecessary branch and just jump
                // directly into it.
                //

                goto FoundMatch;
            }
        }

        if ((USHORT)CharactersMatched == Length) {

FoundMatch:

            //
            // This slot is a prefix match.  Fill out the Match structure if the
            // caller provided a non-NULL pointer, then return the index of the
            // match.
            //


            if (ARGUMENT_PRESENT(Match)) {

                Match->Index = (BYTE)Index;
                Match->NumberOfMatchedCharacters = (BYTE)CharactersMatched;
                Match->String = &StringTable->pStringArray->Strings[Index];

            }

            return (STRING_TABLE_INDEX)Index;
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
