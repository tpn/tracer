        title "Hook Library AMD64 Assembly Support Routines"
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   Hookx64.asm
;
; Abstract:
;
;   This module implements the hooking prolog and epilog functions.
;
;--


include ksamd64.inc

Function struct
        Hash                dq      ?
        AllocationRoutine   dq      ?
        AllocationContext   dq      ?
        Rtl                 dq      ?
        OriginalAddress     dq      ?
        ContinuationAddress dq      ?
        HookProlog          dq      ?
        HookEntry           dq      ?
        EntryCallback       dq      ?
        EntryContext        dq      ?
        HookEpilog          dq      ?
        HookExit            dq      ?
        ExitCallback        dq      ?
        ExitContext         dq      ?
        Name                dq      ?
        Module              dq      ?
        Signature           dq      ?
        NtStyleSignature    dq      ?
        Trampoline          dq      ?
        NumberOfParameters  dw      ?
        SizeOfReturnValue   dw      ?
        Unused              dd      ?
Function ends

ExtraFrame struct
        ReturnValue     dq      ?       ; rax value after original func called
        ExitTimestamp   dq      ?       ; exit timestamp
        EntryTimestamp  dq      ?       ; entry timestamp
        Rflags          dq      ?       ; rflags
        HookedFunction  dq      ?       ; saved function pointer
ExtraFrame ends

EntryFrame struct
        ReturnValue     dq      ?       ; rax value after original func called
        ExitTimestamp   dq      ?       ; exit timestamp
        EntryTimestamp  dq      ?       ; entry timestamp
        Rflags          dq      ?       ; rflags
        HookedFunction  dq      ?       ; saved function pointer
        ReturnAddress   dq      ?       ; pushed onto the stack before the call
        HomeRcx         dq      ?       ; home param 1
        HomeRdx         dq      ?       ; home param 2
        HomeR8          dq      ?       ; home param 3
        HomeR9          dq      ?       ; home param 4
EntryFrame ends

;++
;
; VOID
; HookProlog(VOID)
;
; Routine Description:
;
;   This is the prolog routine.  It is jumped to by the patched function.
;   The pointer for the HOOKED_FUNCTION struct (Function above) will be
;   on the stack.
;
; Arguments:
;
;   No arguments are passed in registers.
;
; Return Value:
;
;   XXX TODO.
;
;--

        NESTED_ENTRY HookProlog, _TEXT$00

        .allocstack     8       ; account for the function pointer

        rex_push_eflags         ; push rflags

        alloc_stack 8 + 8 + 8   ; entry+exit timestamp, return value

        rex_push_reg rbp        ; save rbp before clobbering
        lea rbp, [rsp+8]        ; load the base of our EntryFrame into rbp

        ;.setframe rbp, sizeof(ExtraFrame) ; start of frame pointer

        alloc_stack 4 * 8;      ; allocate space for additional home registers

        END_PROLOGUE

;
; Home parameter registers.
;
        mov     EntryFrame.HomeRcx[rbp], rcx
        mov     EntryFrame.HomeRdx[rbp], rdx
        mov     EntryFrame.HomeR8[rbp], r8
        mov     EntryFrame.HomeR9[rbp], r9

;
; Generate the entry timestamp.
;
        lfence                  ; stabilize rdtsc
        rdtsc                   ; get timestamp counter
        shl     rdx, 32         ; low part -> high part
        or      rdx, rax        ; merge low part into rdx
        mov     EntryFrame.EntryTimestamp[rbp], rdx  ; save counter

;
; Move the EntryFrame/HOOKED_FUNCTION_CALL struct into rcx as first parameter
; to HookEntry.
;
        mov     rcx, rbp

;
; Load the effective address of the pointer to the HookedFunction, then update
; the frame accordingly.
;

        lea     r10, EntryFrame.HookedFunction[rbp]
        mov qword ptr EntryFrame.HookedFunction[rbp], r10

;
; Load the pointer to the hook entry function.
;
        lea     r11, Function.HookEntry[r10]


;
; Call the hook entry point.
;

        call    qword ptr [r11]

;
; (Do a test here to see if the actual function needs to be called?)
;


;
; Set up the parameter registers before calling the continuation point.
;

        mov     rcx, EntryFrame.HomeRcx[rbp]
        mov     rdx, EntryFrame.HomeRdx[rbp]
        mov     r8,  EntryFrame.HomeR8[rbp]
        mov     r9,  EntryFrame.HomeR9[rbp]

;
; Push the epilog's return address onto the stack.
;
        push    Function.HookEpilog[rsp]

;
; Now complete the original function call at the continuation point.  Execution
; will resume at the epilog once the target function returns.
;

        lea     r10, EntryFrame.HookedFunction[rbp]
        lea     r11, Function.ContinuationAddress[r10]

        call qword ptr [r11]

;
; The pointer to the function struct will have been pushed to the stack via
; two DWORD pushes of an immediate value.  Load it into the rcx register.

;
; At this point, [rsp+8] will point to an address that matches the struct
; layout of HOOKED_FUNCTION_ENTRY.
;

        pop     rax
        ret

        NESTED_END HookProlog, _TEXT$00

;++
;
; VOID
; HookEpilog(VOID)
;
; Routine Description:
;
;    This is the epilog routine.  It is used as the return address
;    after a function's HookedEntry callback has been called.
;
; Arguments:
;
;    No arguments are passed in registers.
;
; Return Value:
;
;    XXX TODO.
;
;--
        LEAF_ENTRY HookEpilog, _TEXT$00

;
; Deallocate space for home params.
;
        add     rsp, 20h

;
; Generate the exit timestamp.
;
        lfence                  ; stabilize rdtsc
        rdtsc                   ; get timestamp counter
        shl     rdx, 32         ; low part -> high part
        or      rdx, rax        ; merge low part into rdx
        mov     EntryFrame.ExitTimestamp[rsp], rdx  ; save counter

;
; Return to the original return address.
;
        push EntryFrame.ReturnAddress[rsp]
        ret

        LEAF_END HookEpilog, _TEXT$00


;++
;
; Routine Description:
;
;    This routine calls RTDSC.
;
; Arguments:
;
;    eax        ULONG Key
;
; Return Value:
;
;    None.
;
;--

        ;DummyAddress equ 0000000054c7c5d0h
        ;DummyValue   equ 123456789abcdef0h
        DummyValue   equ 12345678h

        LEAF_ENTRY HookPushQWord, _TEXT$00

        push    DummyValue
        pop     rax
        ret

        LEAF_END HookPushQWord, _TEXT$00

        .data
        DummyValue2 dq 123456789abcdef0h

        LEAF_ENTRY HookPush2, _TEXT$00

        push    DummyValue2
        pop     rax
        ret

        LEAF_END HookPush2, _TEXT$00

        .data
        DummyValue3 dq 123456789abcdef0h
        LEAF_ENTRY HookPush3, _TEXT$00

        mov     rax, 123456789abcdef0h
        ret

        LEAF_END HookPush3, _TEXT$00

        .data
        DummyValue4 dq 123456789abcdef0h
        LEAF_ENTRY HookMov1, _TEXT$00

                db  48h
                db  8bh
                db  05h
                db  00h
                db  00h
                db  00h
                db  08h
        ret

        LEAF_END HookMov1, _TEXT$00

        NESTED_ENTRY HookAddRsp4, _TEXT$00
        END_PROLOGUE

                add rsp, 4

        NESTED_END HookAddRsp4, _TEXT$00


; vim:set tw=80 ts=8 sw=8 sts=8 expandtab syntax=masm formatoptions=croql      :
end
