/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    SearchStringTable.c

Abstract:

    This module implements routines related to searching for a string within
    a STRING_TABLE structure.

--*/

#include "stdafx.h"

#ifdef USE_SEH_FOR_UNALIGNED_STRING_LOADING

#elif USE_ALIGNMENT_TESTF

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
    CHAR FirstChar;
    USHORT Mask;
    USHORT Bitmap;
    STRING_TABLE_INDEX Index = NO_MATCH_FOUND;
    ULONG Count;
    ULONG Length;
    ULONG Mask;
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
    XMMWORD FirstCharShuffleXmm;
    YMMWORD FirstCharYmm;
    YMMWORD SlotLengthsYmm;
    XMMWORD IgnoreSlotsByFirstChar;
    YMMWORD IgnoreSlotsByLength;
    const XMMWORD ZeroedXmm = _mm_setzero_si128();
    const YMMWORD ZeroedYmm = _mm_setzero_si256();

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    if (StringArray->MinimumLength > String->Length) {
        return NO_MATCH_FOUND;
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
    //     will produce an XMM register with each byte set to either 0x0 if
    //     the first character was found, or 0xff if it wasn't.  We then sign-
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
    // Load the first 16-bytes of the search string.
    //

    LoadSearchStringIntoXmmRegister(Search, String, SearchLength);

    //
    // Broadcast the search string's first character into an Xmm register.
    //

    FirstCharXmm = _mm_set1_epi8(String->Buffer[0]);

    //
    // Load the slot length array into an Ymm register.
    //

    Lengths.SlotsYmm = _mm256_load_si256(&StringTable->Lengths.SlotsYmm);

    //
    // Broadcast the search string's length into a Ymm register.
    //

    LengthXmm = _mm_setzero_si128();
    LengthXmm.m128i_u16[0] = String->Length;
    LengthYmm = _mm256_broadcastw_epi16(LengthXmm);

    //
    // See if the first character is in the table.
    //

    FirstChar = String->Buffer[0];


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
        // such that it's positioned correctly for the next loop's tzcnt. Update
        // the shift count accordingly.
        //

        Bitmap >>= (NumberOfTrailingZeros + 1);
        Shift = Index + 1;

        //
        // Load the slot and its length.
        //

        Slot.Chars128 = _mm_load_si128(&StringTable->Slots[Index].Chars128);
        Length = Lengths.Slots[Index];

        //
        // Compare the slot to the search string.
        //

        Compare.Chars128 = _mm_cmpeq_epi8(Slot.Chars128, Source.Chars128);

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

        Mask = _bzhi_u32(_mm_movemask_epi8(Compare.Chars128), SearchLength);

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

    return NO_MATCH_FOUND;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
