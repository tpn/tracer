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
IsPrefixOfStringInTable_1(
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
    ULONG Bitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

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
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
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
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

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

            CharactersMatched = IsPrefixMatchAvx2(String, TargetString, 16);

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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// Manually compare each string in a linear fashion.
//

STRING_TABLE_INDEX
IsPrefixOfStringInTable_2(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PSTRING_MATCH Match
    )
{
    BYTE Left;
    BYTE Right;
    ULONG Index;
    ULONG Count;
    PSTRING_ARRAY StringArray;
    PSTRING TargetString;

    //IACA_VC_START();

    StringArray = StringTable->pStringArray;

    if (StringArray->MinimumLength > String->Length) {
        return NO_MATCH_FOUND;
    }

    for (Count = 0; Count < StringArray->NumberOfElements; Count++) {

        TargetString = &StringArray->Strings[Count];

        if (String->Length < TargetString->Length) {
            continue;
        }

        for (Index = 0; Index < TargetString->Length; Index++) {
            Left = String->Buffer[Index];
            Right = TargetString->Buffer[Index];
            if (Left != Right) {
                break;
            }
        }

        if (Index == TargetString->Length) {

            if (ARGUMENT_PRESENT(Match)) {

                Match->Index = (BYTE)Count;
                Match->NumberOfMatchedCharacters = (BYTE)Index;
                Match->String = TargetString;

            }

            return (STRING_TABLE_INDEX)Count;
        }

    }

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}


//
// A prefix search using C-style NULL-terminated strings and no string table
// support.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfCStrInArray(
    PCSZ *StringArray,
    PCSZ String,
    PSTRING_MATCH Match
    )
{
    PCSZ Left;
    PCSZ Right;
    PCSZ *Target;
    ULONG Index = 0;
    ULONG Count;

    for (Target = StringArray; *Target != NULL; Target++, Index++) {
        Count = 0;
        Left = String;
        Right = *Target;

        while (*Left && *Right && *Left++ == *Right++) {
            Count++;
        }

        if (Count > 0 && !*Right) {
            if (ARGUMENT_PRESENT(Match)) {
                Match->Index = (BYTE)Index;
                Match->NumberOfMatchedCharacters = (BYTE)Count;
                Match->String = NULL;
            }
            return (STRING_TABLE_INDEX)Index;
        }
    }

    return NO_MATCH_FOUND;
}

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_3(
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
    ULONG Bitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

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
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
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
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

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

            {
                PBYTE Left;
                PBYTE Right;
                ULONG Count = 0;

                Left = (PBYTE)RtlOffsetToPointer(String->Buffer, 16);
                Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, 16);

                while (*Left && *Right && *Left++ == *Right++) {
                    Count++;
                }

                if (Count > 0 && !*Right) {
                    CharactersMatched = Count + 16;
                } else {
                    CharactersMatched = NO_MATCH_FOUND;
                }
            }

            //CharactersMatched = IsPrefixMatch(String, TargetString, 16);

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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// A variant of v3 above with the SEH search string loader routine.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_4(
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
    ULONG Bitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

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
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
    // vice versa.  However, we assume the benefits afforded by giving the CPU
    // a bunch of independent things to do unconditionally up-front outweigh
    // the cost of putting in branches and conditionally loading things if
    // necessary.
    //

    //
    // Load the first 16-bytes of the search string into an XMM register.
    //

    LoadSearchStringIntoXmmRegister_SEH(Search, String, SearchLength);

    //
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

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

            {
                PBYTE Left;
                PBYTE Right;
                ULONG Count = 0;

                Left = (PBYTE)RtlOffsetToPointer(String->Buffer, 16);
                Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, 16);

                while (*Left && *Right && *Left++ == *Right++) {
                    Count++;
                }

                if (Count > 0 && !*Right) {
                    CharactersMatched = Count + 16;
                } else {
                    CharactersMatched = NO_MATCH_FOUND;
                }
            }

            //CharactersMatched = IsPrefixMatch(String, TargetString, 16);

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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// Variant of v3 with early-exits.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_5(
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
    ULONG Bitmap;
    ULONG CharBitmap;
    ULONG LengthBitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

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
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
    // vice versa.  However, we assume the benefits afforded by giving the CPU
    // a bunch of independent things to do unconditionally up-front outweigh
    // the cost of putting in branches and conditionally loading things if
    // necessary.
    //

    //
    // Load the first 16-bytes of the search string into an XMM register.
    //

    LoadSearchStringIntoXmmRegister_AlwaysMovsb(Search, String, SearchLength);

    //
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);


    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    CharBitmap = _mm_movemask_epi8(IncludeSlotsByUniqueChar);
    if (!CharBitmap) {
        return NO_MATCH_FOUND;
    }

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    LengthBitmap = _mm_movemask_epi8(IncludeSlotsByLength);
    if (!LengthBitmap) {
        return NO_MATCH_FOUND;
    }

    Bitmap = CharBitmap & LengthBitmap;
    if (!Bitmap) {
        return NO_MATCH_FOUND;
    }

    //
    // A popcount against the mask will tell us how many slots we matched, and
    // thus, need to compare.
    //

    Count = __popcnt(Bitmap);

    do {

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

            {
                PBYTE Left;
                PBYTE Right;
                ULONG Count = 0;

                Left = (PBYTE)RtlOffsetToPointer(String->Buffer, 16);
                Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, 16);

                while (*Left && *Right && *Left++ == *Right++) {
                    Count++;
                }

                if (Count > 0 && !*Right) {
                    CharactersMatched = Count + 16;
                } else {
                    CharactersMatched = NO_MATCH_FOUND;
                }
            }

            //CharactersMatched = IsPrefixMatch(String, TargetString, 16);

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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// Variant of v3 with slightly-altered memcmp inner routine.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_6(
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
    ULONG Bitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

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
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
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
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

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

            {
                PBYTE Left;
                PBYTE Right;
                ULONG Matched = 0;
                ULONG Remaining = (TargetString->Length - 16) + 1;

                Left = (PBYTE)RtlOffsetToPointer(String->Buffer, 16);
                Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, 16);

                while (--Remaining && *Left && *Right && *Left++ == *Right++) {
                    Matched++;
                }

                if (Matched > 0 && !*Right) {
                    CharactersMatched += Matched;
                    goto FoundMatch;
                }
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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// A variant of v3 above with an unchecked aligned load.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_7(
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
    ULONG Bitmap;
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
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

    StringArray = StringTable->pStringArray;

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    //if (StringArray->MinimumLength > String->Length) {
    //    goto NoMatch;
    //}

    //
    // Unconditionally do the following five operations before checking any of
    // the results and determining how the search should proceed:
    //
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
    // vice versa.  However, we assume the benefits afforded by giving the CPU
    // a bunch of independent things to do unconditionally up-front outweigh
    // the cost of putting in branches and conditionally loading things if
    // necessary.
    //

    //
    // Load the first 16-bytes of the search string into an XMM register.
    //

    SearchLength = min(String->Length, 16);
    Search.CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);

    //
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

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

            {
                PBYTE Left;
                PBYTE Right;
                ULONG Count = 0;

                Left = (PBYTE)RtlOffsetToPointer(String->Buffer, 16);
                Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, 16);

                while (*Left && *Right && *Left++ == *Right++) {
                    Count++;
                }

                if (Count > 0 && !*Right) {
                    CharactersMatched = Count + 16;
                } else {
                    CharactersMatched = NO_MATCH_FOUND;
                }
            }

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

NoMatch:

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

//
// A variant of v3 with a tweaked bitmap check.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_8(
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
    ULONG Bitmap;
    ULONG Mask;
    ULONG Count;
    ULONG Length;
    ULONG Index;
    ULONG Shift = 0;
    ULONG CharactersMatched;
    ULONG NumberOfTrailingZeros;
    ULONG SearchLength;
    PSTRING TargetString;
    //PSTRING_ARRAY StringArray;
    STRING_SLOT Slot;
    STRING_SLOT Search;
    STRING_SLOT Compare;
    SLOT_LENGTHS Lengths;
    XMMWORD LengthXmm;
    XMMWORD UniqueChar;
    XMMWORD TableUniqueChars;
    XMMWORD IncludeSlotsByUniqueChar;
    XMMWORD IgnoreSlotsByLength;
    XMMWORD IncludeSlotsByLength;
    XMMWORD IncludeSlots;
    const XMMWORD AllOnesXmm = _mm_set1_epi8(0xff);

    //StringArray = StringTable->pStringArray;

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    //if (StringArray->MinimumLength > String->Length) {
    //    goto NoMatch;
    //}

    //
    // Unconditionally do the following five operations before checking any of
    // the results and determining how the search should proceed:
    //
    //  1. Load the search string into an Xmm register, and broadcast the
    //     character indicated by the unique character index (relative to
    //     other strings in the table) across a second Xmm register.
    //
    //  2. Load the string table's unique character array into an Xmm register.
    //
    //  3. Broadcast the search string's length into a Xmm register.
    //
    //  3. Load the string table's slot lengths array into a Xmm register.
    //
    //  4. Compare the unique character from step 1 to the string table's unique
    //     character array set up in step 2.  The result of this comparison
    //     will produce an XMM register with each byte set to either 0xff if
    //     the unique character was found, or 0x0 if it wasn't.
    //
    //  5. Compare the search string's length from step 3 to the string table's
    //     slot length array set up in step 3.  This allows us to identify the
    //     slots that have strings that are of lesser or equal length to our
    //     search string.  As we're doing a prefix search, we can ignore any
    //     slots longer than our incoming search string.
    //
    // We do all five of these operations up front regardless of whether or not
    // they're strictly necessary.  That is, if the unique character isn't in
    // the unique character array, we don't need to load array lengths -- and
    // vice versa.  However, we assume the benefits afforded by giving the CPU
    // a bunch of independent things to do unconditionally up-front outweigh
    // the cost of putting in branches and conditionally loading things if
    // necessary.
    //

    //
    // Load the first 16-bytes of the search string into an XMM register.
    //

    Search.CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);

    //
    // Broadcast the search string's unique characters according to the string
    // table's unique character index.
    //

    UniqueChar = _mm_shuffle_epi8(Search.CharsXmm,
                                  StringTable->UniqueIndex.IndexXmm);

    //
    // Load the slot length array into a XMM register.
    //

    Lengths.SlotsXmm = _mm_load_si128(&StringTable->Lengths.SlotsXmm);

    //
    // Load the string table's unique character array into an XMM register.
    //

    TableUniqueChars = _mm_load_si128(&StringTable->UniqueChars.CharsXmm);

    //
    // Broadcast the search string's length into a XMM register.
    //

    LengthXmm.m128i_u8[0] = (BYTE)String->Length;
    LengthXmm = _mm_broadcastb_epi8(LengthXmm);

    //
    // Compare the search string's unique character with all of the unique
    // characters of strings in the table, saving the results into an XMM
    // register.  This comparison will indicate which slots we can ignore
    // because the characters at a given index don't match.  Matched slots
    // will be 0xff, unmatched slots will be 0x0.
    //

    IncludeSlotsByUniqueChar = _mm_cmpeq_epi8(UniqueChar, TableUniqueChars);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B. Because we default the length of empty slots to 0x7f, they will
    //      handily be included in the ignored set (i.e. their words will also
    //      be set to 0xff), which means they'll also get filtered out when
    //      we invert the mask shortly after.
    //

    IgnoreSlotsByLength = _mm_cmpgt_epi8(Lengths.SlotsXmm, LengthXmm);

    //
    // Invert the result of the comparison; we want 0xff for slots to include
    // and 0x0 for slots to ignore (it's currently the other way around).  We
    // can achieve this by XOR'ing the result against our all-ones XMM register.
    //

    IncludeSlotsByLength = _mm_xor_si128(IgnoreSlotsByLength, AllOnesXmm);

    //
    // We're now ready to intersect the two XMM registers to determine which
    // slots should still be included in the comparison (i.e. which slots have
    // the exact same first character as the string and a length less than or
    // equal to the length of the search string).
    //

    IncludeSlots = _mm_and_si128(IncludeSlotsByUniqueChar,
                                 IncludeSlotsByLength);

    //
    // Generate a mask.
    //

    Bitmap = _mm_movemask_epi8(IncludeSlots);

    if (!Bitmap) {
        return NO_MATCH_FOUND;
    }

    SearchLength = min(String->Length, 16);

    Count = __popcnt(Bitmap);

    do {

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

            //
            // Prefix match failed, continue the search.
            //

            continue;

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

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
