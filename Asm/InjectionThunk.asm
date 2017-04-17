        title "Injection Thunk Assembly Routine"

;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   InjectionThunk.asm
;
; Abstract:
;
;   This module implements the injection thunk routine.
;
;--

include ksamd64.inc

;
; Define a home parameter + return address structure.
;

Home struct
        ReturnAddress   dq      ?       ; 8     32      40      (28h)
        Param1          dq      ?       ; 8     24      32      (20h)
        Param2          dq      ?       ; 8     16      24      (18h)
        Param3          dq      ?       ; 8     8       16      (10h)
        Param4          dq      ?       ; 8     0       8       (08h)
Home ends

;
; Define the RTL_INJECTION_THUNK_CONTEXT structure.
;

Thunk struct
        VirtualProtect      dq      ?
        ExitThread          dq      ?
        LoadLibraryW        dq      ?
        GetProcAddress      dq      ?
        ModulePath          dq      ?
        ModuleHandle        dq      ?
        FunctionName        dq      ?
        FunctionAddress     dq      ?
Thunk ends

InjLoadLibraryFailed     equ -1
InjGetProcAddressFailed  equ -2

        LEAF_ENTRY InjectionThunk1Handler, _TEXT$00

        mov     eax, ExceptionContinueSearch
        ret

        LEAF_END InjectionThunk1Handler, _TEXT$00

;++
;
; VOID
; InjectionThunk(
;     _In_ PRTL_INJECTION_THUNK_CONTEXT Thunk
;     );
;
; Routine Description:
;
;   This routine fills one or more pages of memory with a given byte pattern
;   using non-temporal hints.
;
; Arguments:
;
;   Thunk (rcx) - Supplies a pointer to the injection context thunk.
;
; Return Value:
;
;   This routine does not return.
;
;--

        ;LEAF_ENTRY InjectionThunk1, _TEXT$00, InjectionThunk1Handler
        NESTED_ENTRY InjectionThunk1, _TEXT$00, InjectionThunk1Handler
        END_PROLOGUE

;
; Put in a dummy nop just so our first byte isn't a jump, and thus, won't be
; skipped over by SkipJumps.
;

        nop

;
; Jump past our inline quadwords reserved for rsp and rcx.
;

        jmp     Inj05

        align   16

        ALTERNATE_ENTRY InjRsp
        dq      ?

        ALTERNATE_ENTRY InjRcx
        dq      ?

        align   16

Inj05:  mov qword ptr [InjRsp], rsp                 ; Save rsp.
        mov qword ptr [InjRcx], rcx                 ; Save rcx.

;
; Move the thunk pointer in rcx to r11, move the path name into rcx, set the
; return address to the instruction after the call, then "call" LoadLibrary by
; way of a long jump.
;

        mov     r11, rcx
        mov     rcx, qword ptr Thunk.ModulePath[rcx]
        mov     rsp, qword ptr [InjectionThunkCallout1]
        jmp     qword ptr Thunk.LoadLibraryW[r11]

        ALTERNATE_ENTRY InjectionThunkCallout1

;
; We should resume execution here after LoadLibraryW() returns.  Verify the
; module was loaded successfully.
;

        test    rax, rax                        ; Is handle NULL?
        jz      Inj70                           ; LoadLibraryW() failed.

;
; Module was loaded successfully.  Load it into rcx, load the function name
; into rdx, set the return address to the instruction after the call, then
; then "call" GetProcAddress by way of a long jump.
;

        mov     r11, qword ptr [InjRcx]             ; Re-load Thunk into r11.
        mov     Thunk.ModuleHandle[r11], rax        ; Save handle.
        mov     rcx, rax                            ; Load module into rcx.
        mov     rdx, Thunk.FunctionName[r11]        ; Load function name.
        mov     rsp, qword ptr [InjectionThunkCallout2] ; Set return address.
        jmp     qword ptr Thunk.GetProcAddress[r11] ; "Call" GetProcAddress.

        ALTERNATE_ENTRY InjectionThunkCallout2

;
; Verify the return value.
;

        test    rax, rax                        ; Is PROC NULL?
        jz      Inj80                           ; Yes; GetProcAddress() failed.

;
; The function was resolved successfully.  Save a copy of the resolved address
; in the thunk.
;

        mov     rcx, qword ptr [InjRcx]         ; Re-load Thunk into rcx.
        mov     Thunk.FunctionAddress[rcx], rax ; Save the function.

;
; Load the original return address back into the return address such that it
; looks like kernel32!BaseThreadInitThunk called us.
;
        mov     rsp, qword ptr [InjRsp]
        jmp     Thunk.FunctionAddress[rcx]  ; Call the thread entry routine.

Inj70:  mov     rcx, InjLoadLibraryFailed       ; Load the error code.
        jmp     short Inj90

Inj80:  mov     rcx, InjGetProcAddressFailed    ; Load the error code.

Inj90:  mov     r11, qword ptr [InjRcx]         ; Move Thunk into r11.
        jmp     Thunk.ExitThread[r11]           ; Terminate thread.

        LEAF_END InjectionThunk1, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql com=:;                 :

end
