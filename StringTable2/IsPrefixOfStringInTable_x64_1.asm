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
;++
;
; BOOL
; IsPrefixOfStringInTable_x64_*(
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

        ;IACA_VC_START

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

        xor         eax, eax                    ;
        not         al                          ; rax = -1
        ret

        ;IACA_VC_END

        LEAF_END   IsPrefixOfStringInTable_x64_1, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
