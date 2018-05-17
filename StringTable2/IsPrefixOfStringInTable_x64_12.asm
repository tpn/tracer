        title "IsPrefixOfStringInTable_x64"
        option nokeyword:<Length>
;++
;
; Copyright (c) Trent Nelson, 2018.
;
; Module Name:
;
;   PrefixSearchStringTable_x64_*.asm.
;
; Abstract:
;
;   These modules implement the IsPrefixOfStringInTable routine.
;
;   N.B. Keep this header identical between files to declutter diff output.
;
;--

include StringTable.inc

;++
;
; STRING_TABLE_INDEX
; IsPrefixOfStringInTable_x64_*(
;     _In_ PSTRING_TABLE StringTable,
;     _In_ PSTRING String,
;     _Out_opt_ PSTRING_MATCH Match
;     )
;
; Routine Description:
;
;   Searches a string table to see if any strings "prefix match" the given
;   search string.  That is, whether any string in the table "starts with
;   or is equal to" the search string.
;
;   This routine is based on version 11, but leverages the inner loop logic
;   tweak we used in version 13 of the C version, pointed out by Fabian Giesen
;   (@rygorous).  That is, we do away with the shifting logic and explicit loop
;   counting, and simply use blsr to keep iterating through the bitmap until it
;   is empty.
;
; Arguments:
;
;   StringTable - Supplies a pointer to a STRING_TABLE struct.
;
;   String - Supplies a pointer to a STRING struct that contains the string to
;       search for.
;
;   Match - Optionally supplies a pointer to a variable that contains the
;       address of a STRING_MATCH structure.  This will be populated with
;       additional details about the match if a non-NULL pointer is supplied.
;
; Return Value:
;
;   Index of the prefix match if one was found, NO_MATCH_FOUND if not.
;
;--

        LEAF_ENTRY IsPrefixOfStringInTable_x64_12, _TEXT$00

;
; Load the address of the string buffer into rax.
;

        ;IACA_VC_START

        mov     rax, String.Buffer[rdx]         ; Load buffer addr.

;
; Broadcast the byte-sized string length into xmm4.
;

        vpbroadcastb xmm4, byte ptr String.Length[rdx]  ; Broadcast length.

;
; Load the lengths of each string table slot into xmm3.
;

        vmovdqa xmm3, xmmword ptr StringTable.Lengths[rcx]  ; Load lengths.

;
; Load the search string buffer into xmm0.
;

        vmovdqu xmm0, xmmword ptr [rax]         ; Load search buffer.

;
; Compare the search string's length, which we've broadcasted to all 8-byte
; elements of the xmm4 register, to the lengths of the slots in the string
; table, to find those that are greater in length.
;

        vpcmpgtb    xmm1, xmm3, xmm4            ; Identify long slots.

;
; Shuffle the buffer in xmm0 according to the unique indexes, and store the
; result into xmm5.
;

        vpshufb     xmm5, xmm0, StringTable.UniqueIndex[rcx] ; Rearrange string.

;
; Compare the search string's unique character array (xmm5) against the string
; table's unique chars (xmm2), saving the result back into xmm5.
;

        vpcmpeqb    xmm5, xmm5, StringTable.UniqueChars[rcx] ; Compare to uniq.

;
; Intersect-and-test the unique character match xmm mask register (xmm5) with
; the length match mask xmm register (xmm1).  This affects flags, allowing us
; to do a fast-path exit for the no-match case (where CY = 1 after xmm1 has
; been inverted).
;

        vptest      xmm1, xmm5                  ; Check for no match.
        jnc         short Pfx10                 ; There was a match.

;
; No match, set rax to -1 and return.
;

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        ret                                     ; Return.

        ;IACA_VC_END

;
; (There was at least one match, continue with processing.)
;

;
; Calculate the "search length" for the incoming search string, which is
; equivalent of 'min(String->Length, 16)'.
;
; Once the search length is calculated, deposit it back at the second byte
; location of xmm4.
;
;   r10 and xmm4[15:8] - Search length (min(String->Length, 16))
;
;   r11 - String length (String->Length)
;

Pfx10:  vpextrb     r11, xmm4, 0                ; Load string length.
        mov         r9, 16                      ; Load 16 into r9.
        mov         r10, r11                    ; Copy length into r10.
        cmp         r10w, r9w                   ; Compare against 16.
        cmova       r10w, r9w                   ; Use 16 if length is greater.

;
; Home our parameter register rdx into the base of xmm2.
;

        vpxor       xmm2, xmm2, xmm2            ; Clear xmm2.
        vmovq       xmm2, rdx                   ; Save rcx.

;
; Intersect xmm5 and xmm1 (as we did earlier with the 'vptest xmm1, xmm5'),
; yielding a mask identifying indices we need to perform subsequent matches
; upon.  Convert this into a bitmap and save in xmm2d[2].
;

        vpandn      xmm5, xmm1, xmm5            ; Intersect unique + lengths.
        vpmovmskb   edx, xmm5                   ; Generate a bitmap from mask.

;
; We're finished with xmm5; repurpose it in the same vein as xmm2 above.
;

        vpxor       xmm5, xmm5, xmm5            ; Clear xmm5.
        vmovq       xmm5, r8                    ; Save r8 into xmm5q[0].

;
; Summary of xmm register stashing for the rest of the routine:
;
;   xmm2:
;        0:63   (vpinsrq 0)     rdx (2nd function parameter, String)
;
;   xmm4:
;       0:7     (vpinsrb 0)     length of search string [r11]
;       8:15    (vpinsrb 1)     min(String->Length, 16) [r10]
;
;   xmm5:
;       0:63    (vpinsrq 0)     r8 (3rd function parameter, StringMatch)
;      64:95    (vpinsrd 2)     bitmap of slots to compare
;      96:127   (vpinsrd 3)     index of slot currently being processed
;
; Non-stashing xmm register use:
;
;   xmm0: First 16 characters of search string.
;
;   xmm3: Slot lengths.
;
;   xmm1: Freebie!
;

        align 16

;
; Top of the main comparison loop.  The bitmap will be present in rdx.  Count
; trailing zeros of the bitmap, producing an index (rax) we can use to load the
; corresponding slot.
;
; Volatile register usage at top of loop:
;
;   rcx - StringTable.
;
;   rdx - Bitmap.
;
;   r9 - Constant value of 16.
;
;   r10 - Search length (min(String->Length, 16))
;
;   r11 - Search string length (String->Length).
;
; Use of remaining volatile registers during loop:
;
;   rax - Index.
;
;   r8 - Freebie!
;

Pfx20:  tzcnt       eax, edx                    ; Count trailing zeros = index.

;
; "Scale" the index (such that we can use it in a subsequent vmovdqa) by
; shifting left by 4 (i.e. multiply by '(sizeof STRING_SLOT)', which is 16).
;
; Then, load the string table slot at this index into xmm1.
;

        mov         r8, rax                     ; Copy index (rax) into r8.
        shl         r8, 4                       ; "Scale" the index.
        vmovdqa     xmm1, xmmword ptr [r8 + StringTable.Slots[rcx]]

;
; The search string's first 16 characters are already in xmm0.  Compare this
; against the slot that has just been loaded into xmm1, storing the result back
; into xmm1.
;

        vpcmpeqb    xmm1, xmm0, xmm1            ; Compare search string to slot.

;
; Convert the XMM mask into a 32-bit representation, then zero high bits after
; our "search length", which allows us to ignore the results of the comparison
; above for bytes that were after the search string's length, if applicable.
; Then, count the number of bits remaining, which tells us how many characters
; we matched.
;

        vpmovmskb   r8d, xmm1                   ; Convert into mask.
        bzhi        r8d, r8d, r10d              ; Zero high bits.
        popcnt      r8d, r8d                    ; Count bits.

;
; Determine if less than 16 characters matched, as this avoids needing to do
; a more convoluted test to see if a byte-by-byte string comparison is needed
; (for lengths longer than 16).
;

        cmp         r8w, r9w                    ; Compare chars matched to 16.
        jl          short Pfx30                 ; Less than 16 matched.

;
; All 16 characters matched.  Load the underlying slot's length from the
; relevant offset in the xmm3 register into r11, then check to see if it's
; greater than 16.  If it is, we're going to need to do a string compare,
; handled by Pfx50.
;
; N.B. The approach for loading the slot length here is a little quirky.  We
;      have all the lengths for slots in xmm3, and we have the current match
;      index in rax.  If we move rax into an xmm register (xmm1 in this case),
;      we can use it to shuffle xmm3, such that the length we're interested in
;      will be deposited back into the lowest byte, which we can then extract
;      via vpextrb.
;

        movd        xmm1, rax                   ; Load index into xmm1.
        vpshufb     xmm1, xmm3, xmm1            ; Shuffle length by index.
        vpextrb     r11, xmm1, 0                ; Extract slot length into r11.
        cmp         r11w, r9w                   ; Compare length to 16.
        ja          short Pfx50                 ; Length is > 16.
        je          short Pfx35                 ; Lengths match!
                                                ; Length < 16, fall through.

;
; Less than 16 characters were matched.  Compare this against the length of the
; search string; if equal, this is a match.
;

Pfx30:  cmp         r8d, r10d                   ; Compare against search string.
        je          short Pfx40                 ; Match found!

;
; No match against this slot.  Clear the lowest set bit of the bitmap and check
; to see if there are any bits remaining in it.
;

        blsr        edx, edx                    ; Reposition bitmap.
        test        edx, edx                    ; Is bitmap empty?
        jnz         short Pfx20                 ; Bits remain, continue loop.

;
; No more bits remain set in the bitmap, we're done.  Indicate no match found
; and return.
;

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        ret                                     ; Return.

;
; Pfx35 and Pfx40 are the jump targets for when the prefix match succeeds.  The
; former is used when we need to copy the number of characters matched from r8
; back to rax.  The latter jump target doesn't require this.
;

Pfx35:  mov         rax, r8                     ; Copy numbers of chars matched.

;
; Load the match parameter into r9 and test to see if it's not-NULL, in which
; case we need to fill out a STRING_MATCH structure for the match, handled by
; jump target Pfx80 at the end of this routine.
;

Pfx40:  vpextrq     r9, xmm5, 0                 ; Extract StringMatch.
        test        r9, r9                      ; Is NULL?
        jnz         Pfx80                       ; Not zero, need to fill out.

;
; StringMatch is NULL, we're done.  We can return straight from here, rax will
; still have the index stored.
;

        ret                                     ; StringMatch == NULL, finish.

;
; 16 characters matched and the length of the underlying slot is greater than
; 16, so we need to do a little memory comparison to determine if the search
; string is a prefix match.
;
; Register use on block entry:
;
;   rax - Index.
;
;   rcx - StringTable.
;
;   rdx - Bitmap.
;
;   r9 - Constant value of 16.
;
;   r10 - Search length (min(String->Length, 16))
;
;   r11 - Slot length.
;
; Register use during the block (after we've freed things up and loaded the
; values we need):
;
;   rax - Index/accumulator.
;
;   rcx - Loop counter (for byte comparison).
;
;   rdx - Byte loaded into dl for comparison.
;
;   r8 - Target string buffer.
;
;   r9 - Search string buffer.
;

;
; Initialize r8 such that it's pointing to the slot's String->Buffer address.
; This is a bit fiddly as we need to go through StringTable.pStringArray first
; to get the base address of the STRING_ARRAY, then the relevant STRING offset
; within the array, then the String->Buffer address from that structure.  Then,
; add 16 to it, such that it's ready as the base address for comparison.
;

Pfx50:  mov         r8, StringTable.pStringArray[rcx] ; Load string array addr.
        mov         r9, rax                 ; Copy index into r9.
        shl         r9, 4                   ; "Scale" index; sizeof STRING=16.
        lea         r8, [r9 + StringArray.Strings[r8]] ; Load STRING address.
        mov         r8, String.Buffer[r8]   ; Load String->Buffer address.
        add         r8, r10                 ; Advance it 16 bytes.

;
; Load the string's buffer address into r9.  We need to get the original
; function parameter value (rdx) from xmm2q[0], then load the String->Buffer
; address, then advance it 16 bytes.
;

        vpextrq     r9, xmm2, 0             ; Extract String into r9.
        mov         r9, String.Buffer[r9]   ; Load buffer address.
        add         r9, r10                 ; Advance buffer 16 bytes.

;
; Save the StringTable parameter, currently in rcx, into xmm1, which is a free
; use xmm register at this point.  This frees up rcx, allowing us to copy the
; slot length, currently in r11, and then subtracting 16 (currently in r10),
; in order to account for the fact that we've already matched 16 bytes.  This
; allows us to then use rcx as the loop counter for the byte-by-byte comparison.
;

        vmovq       xmm1, rcx               ; Free up rcx.
        mov         rcx, r11                ; Copy slot length.
        sub         rcx, r10                ; Subtract 16.

;
; We'd also like to use rax as the accumulator within the loop.  It currently
; stores the index, which is important, so, stash that in r10 for now.  (We
; know r10 is always 16 at this point, so it's easy to restore afterward.)
; We set it to -1 as the first instruction in the byte-comparison loop is
; 'inc al'.
;

        mov         r10, rax                ; Save rax to r10.
        xor         eax, eax                ; Clear rax.
        not         al                      ; al = -1

;
; And we'd also like to use rdx/dl to load each byte of the search string.  It
; currently holds the bitmap, which we need, so stash that in r11 for now, which
; is the last of our free volatile registers at this point (after we've copied
; the slot length from it above).
;

        mov         r11, rdx                ; Save rdx to r11.
        xor         edx, edx                ; Clear rdx.

;
; We've got both buffer addresses + 16 bytes loaded in r8 and r9 respectively.
; We need to do a byte-by-byte comparison.  The loop count is in rcx, and rax
; is initialized to -1.  We're ready to go!
;

@@:     inc         al                      ; Increment index.
        mov         dl, byte ptr [rax + r9] ; Load byte from search string.
        cmp         dl, byte ptr [rax + r8] ; Compare to byte in slot.
        jne         short Pfx60             ; Bytes didn't match, exit loop.

;
; The two bytes were equal, decrement rcx, and potentially continue the loop.
;

        dec         cl                      ; Decrement counter.
        jnz         short @B                ; Continue if not 0.

;
; All bytes matched!  The number of characters matched minus 1 will live in rax,
; and we also need to add 16 to it to account for the first chunk that was
; already matched.  However, rax is also our return value, and needs to point at
; the index of the slot that matched.  Exchange it with r8 first, as if we do
; have a StringMatch parameter, the jump target Pfx80 will be expecting r8 to
; hold the number of characters matched, excluding the initial 16 (it adds that
; in itself).
;

        inc         al                          ; Increment chars matched.
        mov         r8, rax                     ; Save characters matched.
        mov         rax, r10                    ; Re-load index from r10.
        vpextrq     r9, xmm5, 0                 ; Extract StringMatch.
        test        r9, r9                      ; Is NULL?
        jnz         short Pfx75                 ; Not zero, need to fill out.

;
; StringMatch is NULL, we're done.  Return rax, which will have the index in it.
;

        ret                                     ; StringMatch == NULL, finish.

;
; The byte comparisons were not equal.  Re-load the bitmap from r11 into rdx,
; reposition it by clearing the lowest set bit, and potentially exit if there
; are no more bits remaining.
;

Pfx60:  mov         rdx, r11                    ; Reload bitmap.
        blsr        edx, edx                    ; Clear lowest set bit.
        test        edx, edx                    ; Is bitmap empty?
        jnz         short Pfx65                 ; Bits remain.

;
; No more bits remain set in the bitmap, we're done.  Indicate no match found
; and return.
;

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        ret                                     ; Return.

;
; We need to continue the loop, having had this oversized string test (length >
; 16 characters) fail.  Before we do that though, restore the registers we
; clobbered to comply with Pfx20's top-of-the-loop register use assumptions.
;

Pfx65:  vpextrb     r11, xmm4, 0                ; Restore string length.
        vpextrq     rcx, xmm1, 0                ; Restore rcx (StringTable).
        mov         r9, 16                      ; Restore constant 16 to r9.
        mov         r10, r9                     ; Restore search length.
                                                ; (We know it's always 16 here.)
        jmp         Pfx20                       ; Continue comparisons.

;
; This is the target for when we need to fill out the StringMatch structure.
; It's located at the end of this routine because we're optimizing for the
; case where the parameter is NULL in the loop body above, and we don't want
; to pollute the code cache with this logic (which is quite convoluted).

; N.B. Pfx75 is the jump target when we need to add 16 to the characters matched
;      count stored in r8.  This particular path is exercised by the long string
;      matching logic (i.e. when strings are longer than 16 and the prefix match
;      is confirmed via byte-by-byte comparison).  We also need to reload rcx
;      from xmm1.
;
; Expected register use at this point:
;
;   rax - Index of match.
;
;   rcx - StringTable.
;
;   r8 - Number of characters matched.
;
;   r9 - StringMatch.
;
;

Pfx75:  add         r8, 16                                  ; Add 16 to count.
        vpextrq     rcx, xmm1, 0                            ; Reload rcx.

Pfx80:  mov         byte ptr StringMatch.NumberOfMatchedCharacters[r9], r8b
        mov         byte ptr StringMatch.Index[r9], al

;
; Final step, loading the address of the string in the string array.  This
; involves going through the StringTable to find the string array address via
; pStringArray, then the relevant STRING offset within the StringArray.Strings
; structure.
;

        mov         rcx, StringTable.pStringArray[rcx]      ; Load string array.
        mov         r8, rax                                 ; Copy index to r8.
        shl         r8, 4                                   ; "Scale" index.
        lea         rdx, [r8 + StringArray.Strings[rcx]]    ; Resolve address.
        mov         qword ptr StringMatch.String[r9], rdx   ; Save STRING ptr.
        ret                                                 ; Return!

        ;IACA_VC_END

        LEAF_END   IsPrefixOfStringInTable_x64_12, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
