        title "TestStructuredExceptionHandling"
        option nokeyword:<Length>
;++
;
; Copyright (c) Trent Nelson, 2018.
;
; Module Name:
;
;   TestStructuredExceptionHandling.asm.
;
; Abstract:
;
;   The purpose of this module is to ensure our understanding of how structured
;   exception handling works when writing both routine and handlers in assembly.
;
; Compilation:
;
;   This module can be compiled directly into an executable via:
;
;       ml64 /Fe"TestStructuredExceptionHandling.exe" /nologo /W3 TestStructuredExceptionHandling.asm /link /entry:Main /subsystem:console
;
;--

include ksamd64.inc

;
; Forward declaration of fault and resume addresses.
;

altentry RaiseAccessViolationFault
altentry RaiseAccessViolationResume

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
;   This is the exception handler for RaiseAccessViolation.  It verifies that
;   the exception occurred at the RIP we expected, then saves the stack pointer
;   from the originating frame to the context's rax offset, such that it will
;   be available to the faulting routine in the rax register when execution
;   resumes.
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
;   If we correctly detect that the exception was triggered by our expected
;   access violation, we adjust the RIP, copy the sentinel into rax, and
;   return ExceptionContinueExecution.
;
;   Otherwise, ExceptionContinueSearch is returned.
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
        cmp     eax, STATUS_ACCESS_VIOLATION        ; Was this an AV?
        jne     short Eh90                          ; No, return.

;
; Load the known fault address into rax and compare against actual fault IP
; of the triggering exception.
;

        mov     rax, offset RaiseAccessViolationFault   ; Load fault addr.
        cmp     rax, CxRip[r8]                          ; Compare to actual RIP.
        jnz     short Eh90                              ; Not our RIP addr.

;
; We've verified the fault address is what we expect.  Save the stack pointer
; in the context's rax offset, move the resume address into the rip offset, and
; return ExceptionContinueExecution.
;

        mov     r10, CxRsp[r8]                  ; Load stack pointer from ctx.
        mov     CxRax[r8], r10                  ; Save addr to rax offset.
        mov     r11, offset RaiseAccessViolationResume ; Load resume address.
        mov     CxRip[r8], r11                  ; Save addr to rip offset.
        mov     eax, ExceptionContinueExecution ; Load return code.

Eh90:   ret                                     ; Return.

        LEAF_END ExceptionHandler, _TEXT$00

;++
;
; VOID
; RaiseAccessViolation(
;     VOID
;     );
;
; Routine Description:
;
;     This routine dereferences a null pointer in order to generate an access
;     violation.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     None.
;
;--

        NESTED_ENTRY RaiseAccessViolation, _TEXT$00, ExceptionHandler

        END_PROLOGUE

        xor         eax, eax                            ; Clear rax.
        ALTERNATE_ENTRY RaiseAccessViolationFault       ; Mark fault RIP addr.
        mov         rax, [rax]                          ; Dereference NULL.
        ALTERNATE_ENTRY RaiseAccessViolationResume      ; Mark resume RIP addr.

;
; If the fault handler worked correctly, rax should now have the value of our
; stack pointer in it.
;

        cmp         rax, rsp                            ; Does rax == rsp?
        je          @F                                  ; Yes, continue.
        int         3                                   ; No, break.

@@:     ret

        NESTED_END RaiseAccessViolation, _TEXT$00

;++
;
; VOID
; Main(
;     VOID
;     );
;
; Routine Description:
;
;     This is the main entry point of the program.
;
;     It calls RaiseAccessViolation and returns.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     None.
;
;--

        NESTED_ENTRY Main, _TEXT$00

        END_PROLOGUE

        call        RaiseAccessViolation

        ret

        NESTED_END Main, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
