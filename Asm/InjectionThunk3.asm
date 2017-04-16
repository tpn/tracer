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
        ContextAddress      dq      ?
        BasePointer         dq      ?
        StackPointer        dq      ?
        ReturnAddress       dq      ?
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

;++
;
; VOID
; InjectionThunk3(
;     _In_ PRTL_INJECTION_THUNK_CONTEXT Thunk
;     );
;
; Routine Description:
;
;   TBD.
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

        LEAF_ENTRY InjectionThunk3, _TEXT$00

;
; Save the current stack pointer, which will be the return address, into the
; thunk.  Then save rcx into a few different places.
;

        mov     Thunk.ContextAddress[rcx], rcx
        mov     Thunk.BasePointer[rcx], rbp
        mov     Thunk.StackPointer[rcx], rsp
        mov     rax, [rsp]
        mov     Thunk.ReturnAddress[rcx], rax
        mov     rbp, rcx

;
; Move the path name into rcx, set the return address to the instruction after
; the call, then "call" LoadLibrary by
; way of a long jump.
;

        mov     rcx, Thunk.ModulePath[rbp]
        mov     rax, InjectionThunkCallout1
        mov     [rsp], rax
        jmp     Thunk.LoadLibraryW[rbp]

        align 16
        ALTERNATE_ENTRY InjectionThunkCallout1
        mov     rsp, Thunk.StackPointer[rbp]

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

        mov     Thunk.ModuleHandle[rbp], rax
        mov     rcx, rax                            ; Load module into rcx.
        mov     rdx, Thunk.FunctionName[rbp]        ; Load function name.
        mov     rax, InjectionThunkCallout2
        mov     [rsp], rax
        jmp     Thunk.GetProcAddress[rbp]           ; "Call" GetProcAddress.

        align 16
        ALTERNATE_ENTRY InjectionThunkCallout2
        mov     rsp, Thunk.StackPointer[rbp]

;
; Verify the return value.
;

        test    rax, rax                        ; Is PROC NULL?
        jz      Inj80                           ; Yes; GetProcAddress() failed.

;
; The function was resolved successfully.  Save a copy of the resolved address
; in the thunk.
;
        mov     Thunk.FunctionAddress[rbp], rax ; Save the function.

;
; Load the original return address back into the return address such that it
; looks like kernel32!BaseThreadInitThunk called us.
;
        mov     rcx, rbp                        ; Load thunk back into rcx.
        mov     rbp, Thunk.BasePointer[rcx]     ; Restore the base pointer.
        mov     rax, Thunk.ReturnAddress[rcx]   ; Load the original stack ptr.
        mov     [rsp], rax                      ; Restore it.
        jmp     Thunk.FunctionAddress[rcx]      ; Call the thread entry routine.

Inj70:  mov     rax, InjLoadLibraryFailed       ; Load the error code.
        jmp     short Inj90

Inj80:  mov     rax, InjGetProcAddressFailed    ; Load the error code.

Inj90:  mov     rcx, rbp                        ; Move Thunk into rdx.
        mov     rbp, Thunk.BasePointer[rcx]     ; Restore base pointer.
        mov     rdx, Thunk.ReturnAddress[rcx]   ; Load return address.
        mov     [rsp], rdx                      ; Save to stack pointer.

        ret

        LEAF_END InjectionThunk3, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql com=:;                 :

end
