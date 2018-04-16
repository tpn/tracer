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
; Forward declaration of resume addresses.
;

altentry IntegerDivisionResume

;++
;
; VOID
; ExceptionHandler(
;     _In_ PEXCEPTION_RECORD ExceptionRecord,
;     _In_ PVOID EstablisherFrame,
;     _Inout_ PCONTEXT ContextRecord,
;     _Inout_ PVOID DispatcherContext
;     );
;
; Routine Description:
;
;   This is the exception handler for IntegerDivision_x64.  It catches integer
;   division overflow traps and resumes execution after the idiv instruction.
;
; Arguments:
;
;   ExceptionRecord (rcx) - Supplies a pointer to an exception record.
;
;   EstablisherFrame (rdx) - Supplies the frame pointer of the establisher
;       of this exception handler.
;
;   ContextRecord (r8) - Supplies a pointer to a context record.
;
;   DispatcherContext (r9) - Supplies a pointer to the dispatcher context
;       record.
;
; Return Value:
;
;   ExceptionContinueSearch if this is a STATUS_INTEGER_OVERFLOW fault,
;   otherwise, ExceptionContinueSearch.
;
;--

        LEAF_ENTRY ExceptionHandler, _TEXT$00

;
; Initialize return value to ExceptionContinueSearch.  If this is an unwind,
; return.  Otherwise, verify that the exception code indicates an access
; violation, and that the instruction pointer is where we're expecting it.
;

        mov     eax, ExceptionContinueSearch    ; Initialize return value.
        test    dword ptr ErExceptionFlags[rcx], EXCEPTION_UNWIND
        jnz     short Eh90                      ; This is an unwind, return.

        mov     eax, dword ptr ErExceptionCode[rcx] ; Load exception code.
        cmp     eax, STATUS_INTEGER_OVERFLOW        ; Was this an overflow?
        jne     short Eh90                          ; No, return.

;
; This was an overflow, move the resume address into the rip offset and return.
;

        mov     r11, offset IntegerDivisionResume   ; Load resume address.
        mov     CxRip[r8], r11                      ; Save addr to rip offset.
        mov     eax, ExceptionContinueExecution     ; Load return code.

Eh90:   ret                                         ; Return.

        LEAF_END ExceptionHandler, _TEXT$00

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

        NESTED_ENTRY IntegerDivision_x64_1, _TEXT$00, ExceptionHandler
        END_PROLOGUE

        idiv    rcx
        ALTERNATE_ENTRY IntegerDivisionResume
        nop
        ret

        NESTED_END IntegerDivision_x64_1, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
