/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PrefixSearchStringTable.c

Abstract:

    This module implements routines related to searching for a string within
    a STRING_TABLE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInSingleTableInline(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PSTRING_MATCH Match
    )
/*++

Routine Description:

    Searches a string table to see if any strings "prefix match" the given
    search string.  That is, whether any string in the table "starts with
    or is equal to" the search string.

Arguments:

    StringTable - Supplies a pointer to a STRING_TABLE struct.

    String - Supplies a pointer to a STRING struct that contains the string to
        search for.

    Match - Optionally supplies a pointer to a variable that contains the
        address of a STRING_MATCH structure.  This will be populated with
        additional details about the match if a non-NULL pointer is supplied.

Return Value:

    Index of the prefix match if one was found, NO_MATCH_FOUND if not.

--*/
{
    USHORT Bitmap;
    ULONG Mask;
    ULONG Count;
    ULONG Length;
    ULONG Index;
    ULONG Shift = 0;
    ULONG CharactersMatched;
    ULONG NumberOfTrailingZeros;
    ULONG SearchLength;
    PSTRING TargetString;
    PSTRING_ARRAY StringArray;
    STRING_SLOT Slot;
    STRING_SLOT Search;
    STRING_SLOT Compare;
    SLOT_LENGTHS Lengths;
    XMMWORD LengthXmm;
    YMMWORD LengthYmm;
    XMMWORD FirstCharXmm;
    XMMWORD TableFirstChars;
    XMMWORD IncludeSlotsByFirstCharXmm;
    YMMWORD IncludeSlotsByFirstChar;
    YMMWORD IgnoreSlotsByLength;
    YMMWORD IncludeSlotsByLength;
    YMMWORD IncludeSlots;
    YMMWORD IncludeSlotsShifted;
    const YMMWORD AllOnesYmm = _mm256_set1_epi8(0xff);

    StringArray = StringTable->pStringArray;

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    if (StringArray->MinimumLength > String->Length) {
        goto NoMatch;
    }

    //
    // Unconditionally do the following five operations before checking any of
    // the results and determining how the search should proceed:
    //
    //  1. Load the search string into an Xmm register, and broadcast the first
    //     character of the string across a second Xmm register.
    //
    //  2. Load the string table's first character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Ymm register.
    //
    //  3. Load the string table's slot lengths array into a Ymm register.
    //
    //  4. Compare the first character from step 1 to the string table's first
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the first character was found, or 0x0 if it wasn't.  We then sign-
    //     extend this XMM register of 8-bit values into a YMM register of
    //     16-bit values, such that it can be subsequently compared to the YMM
    //     register generated in the next step.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the first character isn't in
    // the first character array, we don't need to load array lengths -- and
    // vice versa.  However, we assume the benefits afforded by giving the CPU
    // a bunch of independent things to do unconditionally up-front outweigh
    // the cost of putting in branches and conditionally loading things if
    // necessary.
    //

    //
    // Load the first 16-bytes of the search string into an XMM register.
    //

    LoadSearchStringIntoXmmRegister(Search, String, SearchLength);

    //
    // Broadcast the search string's first character into an XMM register.
    //

    FirstCharXmm = _mm_set1_epi8(String->Buffer[0]);

    //
    // Load the slot length array into a YMM register.
    //

    Lengths.SlotsYmm = _mm256_load_si256(&StringTable->Lengths.SlotsYmm);

    //
    // Load the string table's first character array into an XMM register.
    //

    TableFirstChars = _mm_load_si128(&StringTable->FirstChars.CharsXmm);

    //
    // Broadcast the search string's length into a YMM register.
    //

    LengthXmm = _mm_setzero_si128();
    LengthXmm.m128i_u16[0] = String->Length;
    LengthYmm = _mm256_broadcastw_epi16(LengthXmm);

    //
    // Compare the search string's first character with all of the first
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because they don't start with the same character as the search string.
    // Matched slots will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByFirstCharXmm = _mm_cmpeq_epi8(FirstCharXmm, TableFirstChars);

    //
    // Sign-extend 16 x 8-bit results from the first character comparison into
    // a 16 x 16-bit 256-bit YMM register.
    //

    IncludeSlotsByFirstChar = _mm256_cvtepi8_epi16(IncludeSlotsByFirstCharXmm);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B.: because we default the length of empty slots to 0x7ffff, they will
    //       handily be included in the ignored set (i.e. their words will also
    //       be set to 0xffff), which means they'll also get filtered out when
    //       we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm256_cmpgt_epi16(Lengths.SlotsYmm, LengthYmm);

    //
    // Invert the result of the comparison; we want 0xffff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones YMM register.
    //

    IncludeSlotsByLength = _mm256_xor_si256(IgnoreSlotsByLength, AllOnesYmm);

    //
    // We're now ready to intersect the two YMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm256_and_si256(IncludeSlotsByFirstChar,
                                    IncludeSlotsByLength);

    //
    // Shift each 16-bit element to the right by 8 bits, zero-filling the upper
    // bits.  This will remove the leading high byte from coming up in the mask
    // we generate below, allowing us to use popcount to get the number of slots
    // to count in the subsequent step.
    //

    IncludeSlotsShifted = _mm256_srli_epi16(IncludeSlots, 8);

    //
    // Generate a mask.  (Sure would be nice to have _mm256_movemask_epi16().)
    //

    Bitmap = (USHORT)_mm256_movemask_epi8(IncludeSlotsShifted);

    if (!Bitmap) {

        //
        // No bits were set, so there are no strings in this table starting
        // with the same character and of a lesser or equal length as the
        // search string.
        //

        goto NoMatch;
    }

    //
    // A popcount against the mask will tell us how many slots we matched, and
    // thus, need to compare.
    //

    Count = __popcnt(Bitmap);

    do {

        //
        // Extract the next index by counting the number of trailing zeros left
        // in the bitmap and adding the amount we've already shifted by.  We
        // shift the trailing zero count right by 1 to compensate for the fact
        // that _mm256_movemask_epi8() created our mask based on a vector of
        // 8-bit elements rather than the 16-bit elements we actually had.
        //
        // (If there were an _mm256_movemask_epi16() AVX2 instruction, we
        //  wouldn't have to do this.  (AVX512 has _mm256_move_pi16_mask()
        //  intrinsic for vpmovw2m, which we could use in the future.).)
        //

        NumberOfTrailingZeros = _tzcnt_u32(Bitmap);
        Index = (NumberOfTrailingZeros >> 1) + Shift;

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

        Compare.CharsXmm = _mm_cmpeq_epi8(Slot.CharsXmm, Search.CharsXmm);

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

NoMatch:

    return NO_MATCH_FOUND;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
