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

        extern HookOverhead:qword
        extern HookEntry:proc

Function struct
        Rtl                 dq      ?
        NumberOfParameters  dw      ?
        Unused1             dw      ?
        Key                 dd      ?
        OldAddress          dq      ?
        NewAddress          dq      ?
        HookedEntry         dq      ?
        HookProlog          dq      ?
        HookEpilog          dq      ?
        HookedExit          dq      ?
        TotalTime           dq      ?
        CallCount           dq      ?
        Name                dq      ?
        Module              dq      ?
Function ends

EntryFrame struct
        ExitTimestamp   dq      ?       ; exit timestamp
        EntryTimestamp  dq      ?       ; entry timestamp
        Rflags          dq      ?       ; rflags
        Function        dq      ?       ; saved function pointer
        ReturnAddress   dq      ?       ; pushed onto the stack before the call
        HomeRcx         dq      ?       ; home param 1
        HomeRdx         dq      ?       ; home param 2
        HomeR8          dq      ?       ; home param 3
        HomeR9          dq      ?       ; home param 4
        OtherParams     dq      ?       ; other
EntryFrame ends

;++
;
; VOID
; HookProlog(VOID)
;
; Routine Description:
;
;    This is the prolog routine.  It is jumped to by the patched function.
;    The key for the patched function will be on the stack.
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

        NESTED_ENTRY HookProlog, _TEXT$00

        push_reg rax            ; save function pointer
        rex_push_eflags         ; push rflags

        alloc_stack 8 + 8       ; for the two timestamp counters

        END_PROLOGUE

;
; Home parameter registers.
;
        mov     EntryFrame.HomeRcx[rsp], rcx
        mov     EntryFrame.HomeRdx[rsp], rdx
        mov     EntryFrame.HomeR8[rsp], r8
        mov     EntryFrame.HomeR9[rsp], r9

;
; Move the entry frame pointer into rcx.
;
        mov     rcx, rsp

;
; Generate the entry timestamp.
;
        lfence                  ; stabilize rdtsc
        rdtsc                   ; get timestamp counter
        shl     rdx, 32         ; low part -> high part
        or      rdx, rax        ; merge low part into rdx
        mov     EntryFrame.EntryTimestamp[rsp], rdx  ; save counter

;
; Move the function pointer into r10.
;
        mov     r10, EntryFrame.Function[rsp]


;
; Reserve space for the home parameters for subsequent calls.
;
        sub     rsp, 20h

;
; Push the epilog's return address onto the stack.
;
        push    Function.HookEpilog[rsp]

;
; Call the hook entry point.
;
        jmp    Function.HookedEntry[r10]

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
