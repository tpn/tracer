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
       ;movzx       r11d, byte ptr String.Length[rdx] ; Load string length.
        mov         r9, 16                      ; Load 16 into r9.
        mov         r10, r11                    ; Copy length into r10.
        cmp         r10w, r9w                   ; Compare against 16.
        cmova       r10w, r9w                   ; Use 16 if length is greater.
        vpinsrb     xmm4, xmm4, r10d, 1         ; Save back to xmm4b[1].

;
; Home our parameter registers into xmm registers instead of their stack-backed
; location, to avoid memory writes.
;

        vpxor       xmm2, xmm2, xmm2            ; Clear xmm2.
        vmovq       xmm2, rcx                   ; Save rcx into xmm2q[0].
        vpinsrq     xmm2, xmm2, rdx, 1          ; Save rdx into xmm2q[1].

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
; xmm2:
;        0:63   (vpinsrq 0)     rcx (1st function parameter, StringTable)
;       64:127  (vpinsrq 1)     rdx (2nd function paramter, String)
;
; xmm4:
;       0:7     (vpinsrb 0)     length of search string
;       8:15    (vpinsrb 1)     min(String->Length, 16)
;      16:23    (vpinsrb 2)     <free>
;      24:31    (vpinsrb 3)     <free>
;
; xmm5:
;       0:63    (vpinsrq 0)     r8 (3rd function parameter, StringMatch)
;      64:95    (vpinsrd 2)     bitmap of slots to compare
;      96:127   (vpinsrd 3)     index of slot currently being processed
;

        align 16

;
; Top of the main comparison loop.  The bitmap will be present in rdx.  Count
; trailing zeros of the bitmap, producing an index (rax) we can use to load the
; corresponding slot.
;
; Volatile register usage at top of loop:
;
;   rax - Index.
;
;   rcx - StringTable.
;
;   rdx - Bitmap.
;
;   r9 - Constant value of 16.
;
;   r10 - Search length.
;
;   r11 - String length.
;
; Use of remaining volatile registers during loop:
;
;   r8 - Freebie!
;

Pfx20:  tzcnt       eax, edx                    ; Count trailing zeros.

       ;blsr        edx, edx                    ; Reposition bitmap.
       ;vpinsrd     xmm5, xmm5, eax, 3          ; Store the raw index xmm5d[3].

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

;@@:     
        movd        xmm1, rax                   ; Load index into xmm1.
        vpshufb     xmm1, xmm3, xmm1            ; Shuffle length by index.
        vpextrb     r11, xmm1, 0                ; Extract slot length into r11.
        cmp         r11w, r9w                   ; Compare length to 16.
        ja          short Pfx50                 ; Length is > 16.
        je          short Pfx40                 ; Lengths match!

;
; (Can this bit be reached?!  I don't think it can.  Which makes the `je` bit
; above moot, as it'll always be equal, so could be converted into a direct
; jump to Pfx40 to finish handling the match processing.)
;

        int         3                           ; Length < 16, fall through...

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
; Register use during the block (after we've freed things up and loaded the
; values we need):
;
;   rax - Misc.
;
;   rcx - Loop counter (for byte comparison).
;
;   rdx - Byte loaded into dl for comparison.
;
;   r8 - Target string buffer.
;
;   r11 - Seach string buffer.
;

Pfx50:  vpinsrd     xmm5, xmm5, eax, 3      ; Free up rax register.
        vpinsrd     xmm5, xmm5, edx, 2      ; Free up rdx register.
        vpinsrq     xmm4, xmm4, rcx, 2      ; Free up rcx register.

        mov         rcx, r11                ; Copy search length into rcx.
        sub         cl, 16                  ; Subtract 16.

;
; Load the search string buffer and advance it 16 bytes.
;

        vpextrq     r11, xmm2, 1            ; Extract String into r11.
        mov         r11, String.Buffer[r11] ; Load buffer address.
        add         r11, 16                 ; Advance buffer 16 bytes.

;
; Loading the slot is more involved as we have to go to the string table, then
; the pStringArray pointer, then the relevant STRING offset within the string
; array (which requires re-loading the index from xmm5d[3]), then the string
; buffer from that structure.
;

        vpextrq     r8, xmm2, 0             ; Extract StringTable into r8.
        mov         r8, StringTable.pStringArray[r8] ; Load string array.

        vpextrd     eax, xmm5, 3            ; Extract index from xmm5.
        shl         eax, 4                  ; Scale the index; sizeof STRING=16.

        lea         r8, [rax + StringArray.Strings[r8]] ; Resolve address.
        mov         r8, String.Buffer[r8]   ; Load string table buffer address.
        add         r8, r10                 ; Advance buffer 16 bytes.

        mov         rax, rcx                ; Copy counter.

;
; We've got both buffer addresses + 16 bytes loaded in r11 and r8 respectively.
; Do a byte-by-byte comparison.
;

        align       16
@@:     mov         dl, byte ptr [rax + r11]    ; Load byte from search string.
        cmp         dl, byte ptr [rax + r8]     ; Compare against target.
        jne         short Pfx60                 ; If not equal, jump.

;
; The two bytes were equal, update rax, decrement rcx and potentially continue
; the loop.
;

        inc         ax                          ; Increment index.
        loopnz      @B                          ; Decrement cx and loop back.

;
; All bytes matched!  Add 16 (still in r10) back to rax such that it captures
; how many characters we matched, and then jump to Pfx40 for finalization.
;

        add         rax, r10
        jmp         Pfx40

;
; Byte comparisons were not equal.  Restore the rcx loop counter and decrement
; it.  If it's zero, we have no more strings to compare, so we can do a quick
; exit.  If there are still comparisons to be made, restore the other registers
; we trampled then jump back to the start of the loop Pfx20.
;

Pfx60:  vpextrb     rcx, xmm4, 2                ; Restore rcx counter.
        dec         cx                          ; Decrement counter.
        jnz         short @F                    ; Jump forward if not zero.

;
; No more comparisons remaining, return.
;

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        ret                                     ; Return.

;
; More comparisons remain; restore the registers we clobbered and continue loop.
;

@@:     vpextrb     r10, xmm4, 1                ; Restore r10.
        vpextrb     r11, xmm4, 0                ; Restore r11.
        vpextrd     edx, xmm5, 2                ; Restore rdx bitmap.
        jmp         Pfx20                       ; Continue comparisons.

;
; This is the target for when we need to fill out the StringMatch structure.
; It's located at the end of this routine because we're optimizing for the
; case where the parameter is NULL in the loop body above, and we don't want
; to pollute the code cache with this logic (which is quite convoluted).
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
