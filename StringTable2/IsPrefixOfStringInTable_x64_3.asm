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

;
; Define a locals struct for saving flags, rsi and rdi.
;

Locals struct

    Padding             dq      ?
    SavedRdi            dq      ?
    SavedRsi            dq      ?
    SavedFlags          dq      ?

    ReturnAddress       dq      ?
    HomeRcx             dq      ?
    HomeRdx             dq      ?
    HomeR8              dq      ?
    HomeR9              dq      ?

Locals ends

;
; Exclude the return address onward from the frame calculation size.
;

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.ReturnAddress - (sizeof Locals)))

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
;   This routine is based off version 2.  It has been converted into a nested
;   entry (version 2 is a leaf entry), and uses 'rep cmpsb' to do the string
;   comparison for long strings (instead of the byte-by-byte comparison used
;   in version 2).  This requires use of the rsi and rdi registers, and the
;   direction flag.  These are all non-volatile registers and thus, must be
;   saved to the stack in the function prologue (hence the need to make this
;   a nested entry).
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

        NESTED_ENTRY IsPrefixOfStringInTable_x64_3, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE                     ; Allocate stack space.

        push_eflags                                 ; Save flags.
        save_reg    rsi, Locals.SavedRsi            ; Save non-volatile rsi.
        save_reg    rdi, Locals.SavedRdi            ; Save non-volatile rdi.

        END_PROLOGUE

;
; Load the string buffer into xmm0, and the unique indexes from the string table
; into xmm1.  Shuffle the buffer according to the unique indexes, and store the
; result into xmm5.
;

        ;IACA_VC_START

        mov     rax, String.Buffer[rdx]
        vmovdqu xmm0, xmmword ptr [rax]                 ; Load search buffer.
        vmovdqa xmm1, xmmword ptr StringTable.UniqueIndex[rcx] ; Load indexes.
        vpshufb xmm5, xmm0, xmm1

;
; Load the string table's unique character array into xmm2.

        vmovdqa xmm2, xmmword ptr StringTable.UniqueChars[rcx]  ; Load chars.

;
; Compare the search string's unique character array (xmm5) against the string
; table's unique chars (xmm2), saving the result back into xmm5.
;

        vpcmpeqb    xmm5, xmm5, xmm2            ; Compare unique chars.

;
; Load the lengths of each string table slot into xmm3.
;
        vmovdqa xmm3, xmmword ptr StringTable.Lengths[rcx]      ; Load lengths.

;
; Set xmm2 to all ones.  We use this later to invert the length comparison.
;

        vpcmpeqq    xmm2, xmm2, xmm2            ; Set xmm2 to all ones.

;
; Broadcast the byte-sized string length into xmm4.
;

        vpbroadcastb xmm4, byte ptr String.Length[rdx]  ; Broadcast length.

;
; Compare the search string's length, which we've broadcasted to all 8-byte
; elements of the xmm4 register, to the lengths of the slots in the string
; table, to find those that are greater in length.  Invert the result, such
; that we're left with a masked register where each 0xff element indicates
; a slot with a length less than or equal to our search string's length.
;

        vpcmpgtb    xmm1, xmm3, xmm4            ; Identify long slots.
        vpxor       xmm1, xmm1, xmm2            ; Invert the result.

;
; Intersect-and-test the unique character match xmm mask register (xmm5) with
; the length match mask xmm register (xmm1).  This affects flags, allowing us
; to do a fast-path exit for the no-match case (where ZF = 1).
;

        vptest      xmm5, xmm1                  ; Check for no match.
        jnz         short Pfx10                 ; There was a match.

;
; No match, set rax to -1 and return.
;

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        jmp         Pfx90                       ; Return.

        ;IACA_VC_END

;
; (There was at least one match, continue with processing.)
;

;
; Calculate the "search length" for the incoming search string, which is
; equivalent of 'min(String->Length, 16)'.  (The search string's length
; currently lives in xmm4, albeit as a byte-value broadcasted across the
; entire register, so extract that first.)
;
; Once the search length is calculated, deposit it back at the second byte
; location of xmm4.
;
;   r10 and xmm4[15:8] - Search length (min(String->Length, 16))
;
;   r11 - String length (String->Length)
;

Pfx10:  vpextrb     r11, xmm4, 0                ; Load length.
        mov         rax, 16                     ; Load 16 into rax.
        mov         r10, r11                    ; Copy into r10.
        cmp         r10w, ax                    ; Compare against 16.
        cmova       r10w, ax                    ; Use 16 if length is greater.
        vpinsrb     xmm4, xmm4, r10d, 1         ; Save back to xmm4b[1].

;
; Home our parameter registers into xmm registers instead of their stack-backed
; location, to avoid memory writes.
;

        vpxor       xmm2, xmm2, xmm2            ; Clear xmm2.
        vpinsrq     xmm2, xmm2, rcx, 0          ; Save rcx into xmm2q[0].
        vpinsrq     xmm2, xmm2, rdx, 1          ; Save rdx into xmm2q[1].

;
; Intersect xmm5 and xmm1 (as we did earlier with the 'vptest xmm5, xmm1'),
; yielding a mask identifying indices we need to perform subsequent matches
; upon.  Convert this into a bitmap and save in xmm2d[2].
;

        vpand       xmm5, xmm5, xmm1            ; Intersect unique + lengths.
        vpmovmskb   edx, xmm5                   ; Generate a bitmap from mask.

;
; We're finished with xmm5; repurpose it in the same vein as xmm2 above.
;

        vpxor       xmm5, xmm5, xmm5            ; Clear xmm5.
        vpinsrq     xmm5, xmm5, r8, 0           ; Save r8 into xmm5q[0].

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
;      16:23    (vpinsrb 2)     loop counter (when doing long string compares)
;      24:31    (vpinsrb 3)     shift count
;
; xmm5:
;       0:63    (vpinsrq 0)     r8 (3rd function parameter, StringMatch)
;      64:95    (vpinsrd 2)     bitmap of slots to compare
;      96:127   (vpinsrd 3)     index of slot currently being processed
;

;
; Initialize rcx as our counter register by doing a popcnt against the bitmap
; we just generated in edx, and clear our shift count register (r9).
;

        popcnt      ecx, edx                    ; Count bits in bitmap.
        xor         r9, r9                      ; Clear r9.

        align 16

;
; Top of the main comparison loop.  The bitmap will be present in rdx.  Count
; trailing zeros of the bitmap, and then add in the shift count, producing an
; index (rax) we can use to load the corresponding slot.
;
; Register usage at top of loop:
;
;   rax - Index.
;
;   rcx - Loop counter.
;
;   rdx - Bitmap.
;
;   r9 - Shift count.
;
;   r10 - Search length.
;
;   r11 - String length.
;

Pfx20:  tzcnt       r8d, edx                    ; Count trailing zeros.
        mov         eax, r8d                    ; Copy tzcnt to rax,
        add         rax, r9                     ; Add shift to create index.
        inc         r8                          ; tzcnt + 1
        shrx        rdx, rdx, r8                ; Reposition bitmap.
        mov         r9, rax                     ; Copy index back to shift.
        inc         r9                          ; Shift = Index + 1
        vpinsrd     xmm5, xmm5, eax, 3          ; Store the raw index xmm5d[3].

;
; "Scale" the index (such that we can use it in a subsequent vmovdqa) by
; shifting left by 4 (i.e. multiply by '(sizeof STRING_SLOT)', which is 16).
;
; Then, load the string table slot at this index into xmm1, then shift rax back.
;

        shl         eax, 4
        vpextrq     r8, xmm2, 0
        vmovdqa     xmm1, xmmword ptr [rax + StringTable.Slots[r8]]
        shr         eax, 4

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
; If 16 characters matched, and the search string's length is longer than 16,
; we're going to need to do a comparison of the remaining strings.
;

        cmp         r8w, 16                     ; Compare chars matched to 16.
        je          short @F                    ; 16 chars matched.
        jmp         Pfx30                       ; Less than 16 matched.

;
; All 16 characters matched.  Load the underlying slot's length from the
; relevant offset in the xmm3 register, then check to see if it's greater than,
; equal or less than 16.
;

@@:     movd        xmm1, rax                   ; Load into xmm1.
        vpshufb     xmm1, xmm3, xmm1            ; Shuffle length...
        vpextrb     rax, xmm1, 0                ; And extract back into rax.
        cmp         al, 16                      ; Compare length to 16.
        ja          Pfx50                       ; Length is > 16.
        je          short Pfx35                 ; Lengths match!
                                                ; Length <= 16, fall through...

;
; Less than or equal to 16 characters were matched.  Compare this against the
; length of the search string; if equal, this is a match.
;

Pfx30:  cmp         r8d, r10d                   ; Compare against search string.
        je          short Pfx35                 ; Match found!

;
; No match against this slot, decrement counter and either continue the loop
; or terminate the search and return no match.
;

        dec         cx                          ; Decrement counter.
        jnz         Pfx20                       ; cx != 0, continue.

        xor         eax, eax                    ; Clear rax.
        not         al                          ; al = -1
        jmp         Pfx90                       ; Return.

;
; Pfx35 and Pfx40 are the jump targets for when the prefix match succeeds.  The
; former is used when we need to copy the number of characters matched from r8
; back to rax.  The latter jump target doesn't require this.
;

Pfx35:  mov         rax, r8                     ; Copy numbers of chars matched.

;
; Load the match parameter back into r8 and test to see if it's not-NULL, in
; which case we need to fill out a STRING_MATCH structure for the match.
;

Pfx40:  vpextrq     r8, xmm5, 0                 ; Extract StringMatch.
        test        r8, r8                      ; Is NULL?
        jnz         short @F                    ; Not zero, need to fill out.

;
; StringMatch is NULL, we're done. Extract index of match back into rax and ret.
;

        vpextrd     eax, xmm5, 3                ; Extract raw index for match.
        jmp         Pfx90                       ; StringMatch == NULL, finish.

;
; StringMatch is not NULL.  Fill out characters matched (currently rax), then
; reload the index from xmm5 into rax and save.
;

@@:     mov         byte ptr StringMatch.NumberOfMatchedCharacters[r8], al
        vpextrd     eax, xmm5, 3                ; Extract raw index for match.
        mov         byte ptr StringMatch.Index[r8], al

;
; Final step, loading the address of the string in the string array.  This
; involves going through the StringTable, so we need to load that parameter
; back into rcx, then resolving the string array address via pStringArray,
; then the relevant STRING offset within the StringArray.Strings structure.
;

        vpextrq     rcx, xmm2, 0            ; Extract StringTable into rcx.
        mov         rcx, StringTable.pStringArray[rcx] ; Load string array.

        shl         eax, 4                  ; Scale the index; sizeof STRING=16.
        lea         rdx, [rax + StringArray.Strings[rcx]] ; Resolve address.
        mov         qword ptr StringMatch.String[r8], rdx ; Save STRING ptr.
        shr         eax, 4                  ; Revert the scaling.

        jmp         Pfx90

;
; 16 characters matched and the length of the underlying slot is greater than
; 16, so we need to do a little memory comparison to determine if the search
; string is a prefix match.
;
; The slot length is stored in rax at this point, and the search string's
; length is stored in r11.  We know that the search string's length will
; always be longer than or equal to the slot length at this point, so, we
; can subtract 16 (currently stored in r10) from rax, and use the resulting
; value as a loop counter, comparing the search string with the underlying
; string slot byte-by-byte to determine if there's a match.
;

Pfx50:  sub         rax, r10                ; Subtract 16 from search length.

;
; Free up some registers by stashing their values into various xmm offsets.
;

        vpinsrd     xmm5, xmm5, edx, 2      ; Free up rdx register.
        vpinsrb     xmm4, xmm4, ecx, 2      ; Free up rcx register.
        mov         rcx, rax                ; Free up rax, rcx is now counter.

;
; Load the search string buffer and advance it 16 bytes.
;

        vpextrq     r11, xmm2, 1            ; Extract String into r11.
        mov         r11, String.Buffer[r11] ; Load buffer address.
        add         r11, r10                ; Advance buffer 16 bytes.

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
; Set up rsi/rdi so we can do a 'rep cmps'.
;

        cld
        mov         rsi, r11
        mov         rdi, r8
        repe        cmpsb

        test        cl, 0
        jnz         short Pfx60                 ; Not all bytes compared, jump.

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
        jmp Pfx90                               ; Return.

;
; More comparisons remain; restore the registers we clobbered and continue loop.
;

@@:     vpextrb     r10, xmm4, 1                ; Restore r10.
        vpextrb     r11, xmm4, 0                ; Restore r11.
        vpextrd     edx, xmm5, 2                ; Restore rdx bitmap.
        jmp         Pfx20                       ; Continue comparisons.

        ;IACA_VC_END

        align   16

Pfx90:  mov     rsi, Locals.SavedRsi[rsp]       ; Restore rsi.
        mov     rdi, Locals.SavedRdi[rsp]       ; Restore rdi.
        popfq                                   ; Restore flags.
        add     rsp, LOCALS_SIZE                ; Deallocate stack space.

        ret

        NESTED_END   IsPrefixOfStringInTable_x64_3, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
