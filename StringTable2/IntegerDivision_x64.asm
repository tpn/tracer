        title "IntegerDivision_x64"
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
;   These modules implement the IntegerDivision routine.
;
;   N.B. Keep this header identical between files to declutter diff output.
;
;--

include StringTable.inc

;
;++
;
; ULONGLONG
; IntegerDivision_x64_*(
;     _In_ PSTRING_TABLE StringTable,
;     _In_ PSTRING String,
;     _Out_ PSTRING_MATCH StringMatch
;     )
;
; Routine Description:
;
;     This routine divides the address supplied by the String parameter by
;     the address supplied by the StringTable parameter, then returns.  It
;     is solely used as a reference point whilst benchmarking.
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
;    Returns the quotient of the division operation.
;
;--

        LEAF_ENTRY IntegerDivision_x64_1, _TEXT$00

        idiv    rcx
        ret

        LEAF_END IntegerDivision_x64_1, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
