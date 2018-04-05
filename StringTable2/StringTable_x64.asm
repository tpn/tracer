        title "StringTable_x64"
        option nokeyword:<Length>
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   StringTable_x64.asm.
;
; Abstract:
;
;   This module implements various StringTable routines in assembly.
;
;--

include StringTable.inc

;
; Define a Locals structure used for referencing our register homing space from
; rsp.
;

Locals struct
    Temp dq ?

    ;
    ; Define non-volatile register storage.
    ;

    union
        FirstNvRegister     dq      ?
        SavedRbp            dq      ?
    ends

    SavedRbx                dq      ?
    SavedRdi                dq      ?
    SavedRsi                dq      ?
    SavedR12                dq      ?
    SavedR13                dq      ?
    SavedR14                dq      ?

    SavedXmm6               XMMWORD { }
    SavedXmm7               XMMWORD { }
    SavedXmm8               XMMWORD { }
    SavedXmm9               XMMWORD { }
    SavedXmm10              XMMWORD { }
    SavedXmm11              XMMWORD { }
    SavedXmm12              XMMWORD { }
    SavedXmm13              XMMWORD { }
    SavedXmm14              XMMWORD { }
    SavedXmm15              XMMWORD { }

    ;
    ; Stash R15 after the return address to ensure the XMM register space
    ; is aligned on a 16 byte boundary, as we use movdqa (i.e. aligned move)
    ; which will fault if we're only 8 byte aligned.
    ;

    SavedR15                dq  ?

    ReturnAddress           dq  ?
    HomeRcx                 dq  ?
    HomeRdx                 dq  ?
    HomeR8                  dq  ?
    HomeR9                  dq  ?
Locals ends

;
; Exclude the return address onward from the frame calculation size.
;

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.ReturnAddress - (sizeof Locals)))


;++
;
; BOOL
; IsPrefixOfStringInTable_x64_SSE42(
;     _In_ PSTRING_TABLE StringTable,
;     _In_ PSTRING String,
;     _Out_ PSTRING_MATCH StringMatch
;     )
;
; Routine Description:
;
;     This routine searches for a prefix match of String in the given
;     StringTable structure.
;
; Arguments:
;
;     StringTable - Supplies a pointer to a STRING_TABLE structure to search.
;
;     String - Supplies a pointer to a STRING structure that a prefix match
;         is searched for.
;
;     StringMatch - Supplies a pointer to a STRING_MATCH structure that will
;         receive the results of the string match.
;
; Return Value:
;
;    Returns TRUE on sucess, FALSE on failure.
;
;--

        LEAF_ENTRY IsPrefixOfStringInTable_x64_1, _TEXT$00

;
; Load the string buffer into xmm0, and the unique indexes from the string table
; into xmm1.  Shuffle the buffer according to the unique indexes, and store the
; result back into xmm0.
;

        mov     rax, String.Buffer[rdx]
        vmovdqu xmm0, xmmword ptr [rax]                 ; Load search buffer.
        vmovdqa xmm1, xmmword ptr StringTable.UniqueIndex[rcx] ; Load indexes.
        vpshufb xmm0, xmm0, xmm1

;
; Load the string table's unique character array into xmm2, and the lengths for
; each string slot into xmm3.
;

        vmovdqa xmm2, xmmword ptr StringTable.UniqueChars[rcx]  ; Load chars.
        vmovdqa xmm3, xmmword ptr StringTable.Lengths[rcx]      ; Load lengths.

;
; Set xmm5 to all ones.  This is used later.
;

        vpcmpeqq    xmm5, xmm5, xmm5                    ; Set xmm5 to all ones.

;
; Broadcast the byte-sized string length into xmm4.
;

        vpbroadcastb xmm4, byte ptr String.Length[rdx]  ; Broadcast length.

;
; Compare the search string's unique character array (xmm0) against the string
; table's unique chars (xmm2), saving the result back into xmm0.
;

        vpcmpeqb    xmm0, xmm0, xmm2            ; Compare unique chars.

;
; Compare the search string's length, which we've broadcasted to all 8-byte
; elements of the xmm4 register, to the lengths of the slots in the string
; table, to find those that are greater in length.  Invert the result, such
; that we're left with a masked register where each 0xff element indicates
; a slot with a length less than or equal to our search string's length.
;

        vpcmpgtb    xmm1, xmm4, xmm3            ; Identify long slots.
        vpxor       xmm1, xmm1, xmm5            ; Invert the result.

;
; Intersect xmm0 and xmm1 to identify string slots of a suitable length with
; a matching unique character.
;

;
; Test the final xmm0 register for all zeros (indicating no match).
;

        vptest      xmm0, xmm1                  ; Check for no match.
        ;jnz         short @F                    ; There was a match.

;
; No match, set rax to -1 and return.
;

        xor         rax, rax                    ;
        sub         rax, 1                      ; rax = -1
        ret

        ;IACA_VC_END

        LEAF_END   IsPrefixOfStringInTable_x64_1, _TEXT$00

;
; Continue implementation.
;

        LEAF_ENTRY IsPrefixOfStringInTable_x64_2, _TEXT$00

        ;IACA_VC_START

;
; Load the string buffer into xmm0, and the unique indexes from the string table
; into xmm1.  Shuffle the buffer according to the unique indexes, and store the
; result back into xmm0.
;

        mov     rax, String.Buffer[rdx]
        vmovdqa xmm0, xmmword ptr [rax]                 ; Load search buffer.
        vmovdqa xmm1, xmmword ptr StringTable.UniqueIndex[rcx] ; Load indexes.
        vpshufb xmm5, xmm0, xmm1

;
; Load the string table's unique character array into xmm2, and the lengths for
; each string slot into xmm3.
;

        vmovdqa xmm2, xmmword ptr StringTable.UniqueChars[rcx]  ; Load chars.
        vmovdqa xmm3, xmmword ptr StringTable.Lengths[rcx]      ; Load lengths.

;
; Broadcast the byte-sized string length into xmm4.
;

        vpbroadcastb xmm4, byte ptr String.Length[rdx]  ; Broadcast length.

;
; Compare the search string's unique character array (xmm5) against the string
; table's unique chars (xmm2), saving the result back into xmm5.
;

        vpcmpeqb    xmm5, xmm5, xmm2            ; Compare unique chars.


;
; Compare the search string's length, which we've broadcasted to all 8-byte
; elements of the xmm4 register, to the lengths of the slots in the string
; table, to find those that are greater in length.  Invert the result, such
; that we're left with a masked register where each 0xff element indicates
; a slot with a length less than or equal to our search string's length.
;

        vpcmpeqq    xmm2, xmm2, xmm2            ; Set xmm2 to all ones.
        vpcmpgtb    xmm1, xmm3, xmm4            ; Identify long slots.
        vpxor       xmm1, xmm1, xmm2            ; Invert the result.

;
; Intersect-and-test the unique character match xmm mask register (xmm5) with
; the length match mask xmm register (xmm1).  This affects flags, allowing us
; to do a fast-path exit for the no-match case (where ZF = 1).
;

        vptest      xmm5, xmm1                  ; Check for no match.
        jnz         short @F                    ; There was a match.

;
; No match, set rax to -1 and return.
;

        xor         rax, rax                    ; Clear rax.
        sub         rax, 1                      ; Set rax to -1.
        ret                                     ; Return.

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

@@:     vpextrb     r11, xmm4, 0                ; Load length.
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
        je          short Pfx40                 ; Lengths match!
                                                ; Length <= 16, fall through...

;
; Less than or equal to 16 characters were matched.  Compare this against the
; length of the search string; if equal, this is a match.
;

Pfx30:  cmp         r8d, r10d                   ; Compare against search string.
        je          short Pfx40                 ; Match found!

;
; No match against this slot, decrement counter and either continue the loop
; or terminate the search and return no match.
;

        dec         cx                          ; Decrement counter.
        jnz         Pfx20                       ; cx != 0, continue.

        xor         eax, eax                    ;
        sub         rax, 1                      ; eax = -1
        ret

;
; The prefix match succeeded.  Load the match parameter back into r9 and test
; to see if it's not-NULL, in which case we need to fill out a STRING_MATCH
; structure for the match.
;

Pfx40:

        ; xxx todo: load r9 and fill out.

        vpextrd     eax, xmm5, 3                ; Extract raw index for match.
        ret

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

        xor         eax, eax                ; Clear eax.

;
; We've got both buffer addresses + 16 bytes loaded in r11 and r8 respectively.
; Do a byte-by-byte comparison.
;

        align 16
@@:     mov         dl, byte ptr [rax + r11]    ; Load byte from search string.
        cmp         dl, byte ptr [rax + r8]     ; Compare against target.
        jne         short Pfx60                 ; If not equal, jump.

;
; The two bytes were equal, update rax, decrement rcx and potentially continue
; the loop.
;

       ;inc         rax                         ; Increment index.
        add         rax, 1                      ; Increment index.
       ;loopnz      @B                          ; Decrement cx and loop back.
        dec         cx
        jnz         short @B
        ;loopnz      @B                          ; Decrement cx and loop back.

;
; All bytes matched!  Jump to Pfx40 for finalization.
;

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

        xor         eax, eax                    ;
        sub         eax, 1                      ; eax = -1
        ret

;
; More comparisons remain; restore the registers we clobbered and continue loop.
;

@@:     vpextrb     r10, xmm4, 1                ; Restore r10.
        vpextrb     r11, xmm4, 0                ; Restore r11.
        vpextrd     edx, xmm5, 2                ; Restore rdx bitmap.
        jmp         Pfx20                       ; Continue comparisons.

        ;IACA_VC_END

        LEAF_END   IsPrefixOfStringInTable_x64_2, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
