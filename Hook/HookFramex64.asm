        title "Hook Frame AMD64 Assembly Support Routines"
        option casemap:none
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   HookFramex64.asm
;
; Abstract:
;
;   This module implements support for tracing a hooked C function call.
;
;--


include ksamd64.inc

PVOID typedef dq
ULONG typedef dw
ULONG64 typedef dq

ALLOCATION_ROUTINE typedef proto far AllocationContext:PVOID, ByteSize:ULONG
PALLOCATION_ROUTINE typedef far ptr ALLOCATION_ROUTINE

FUNCTION struct
    Hash                dq      ?
    AllocationRoutine   PALLOCATION_ROUTINE ?
    AllocationContext   dq      ?
    Rtl                 dq      ?
    OriginalAddress     dq      ?
    ContinuationAddress dq      ?
    Name                dq      ?
    Module              dq      ?
    Signature           dq      ?
    NtStyleSignature    dq      ?
    Trampoline          dq      ?
    NumberOfParameters  dw      ?
    SizeOfReturnValue   dw      ?
    Unused              dd      ?
FUNCTION ends

PFUNCTION typedef far ptr FUNCTION

TIMESTAMP_TRACEFRAME macro Name, Reg

        cpuid
        rdtscp
        mov     dword ptr TraceFrame.TscAux.&Name&[&Reg&], ecx

        shl     rdx, 32         ; low part -> high part
        or      rdx, rax        ; merge low part into rdx
        mov     qword ptr TraceFrame.Timestamp.&Name&[&Reg&], rdx

        endm

;++
;
; VOID
; HookFrame(VOID)
;
; Routine Description:
;
;
; Arguments:
;
;   No arguments are passed in registers.
;
; Return Value:
;
;   The return value from the hooked function is returned in rax.
;
;--

; Timestamps = 4 * 8 = 32 (20h)
;       Entered
;       PreCall
;       PostCall
;       PreExit

; rdtscp = 4 * 4 -> 16

TSC_AUX struct
        PreExit                                 dw      ?
        PostCall                                dw      ?
        PreCall                                 dw      ?
        Entered                                 dw      ?
TSC_AUX ends

TSC struct
        PreExit                                 dq      ?
        PostCall                                dq      ?
        PreCall                                 dq      ?
        Entered                                 dq      ?
TSC ends

PARAMS struct

        ReturnAddress                           dq      ?

        union
                HomeRcx                         dq      ?
                Param1                          dq      ?
        ends

        union
                HomeRdx                         dq      ?
                Param2                          dq      ?
        ends

        union
                HomeR8                          dq      ?
                Param3                          dq      ?
        ends

        union
                HomeR9                          dq      ?
                Param4                          dq      ?
        ends


PARAMS ends

TraceFrame struct

    ReturnValue     dq              ?

    TscAux          TSC_AUX         { }

    Timestamp       TSC             { }

    ; 7     (+ 4)

    ProcessId       dw      ?
    ThreadId        dw      ?

    ; 8     (+ 1)

    union
        HookedFunction  FunctionPointer ?
        HookedFuncPtr   Function ptr ?
    ends

    ; 13    (+ 5)

    Param PARAMS  {?}

    ; 18    (+ 5)

    OtherParams             dq 14 dup (?)

    ; 32 * 8 = 256 bytes

TraceFrame ends

echo Size of TraceFrame: %(sizeof TraceFrame)

HomeParams struct

        ReturnAddress           dq      ?       ; 8     32      40      (28h)
        HomeRcx                 dq      ?       ; 8     24      32      (20h)
        HomeRdx                 dq      ?       ; 8     16      24      (18h)
        HomeR8                  dq      ?       ; 8     8       16      (10h)
        HomeR9                  dq      ?       ; 8     0       8       (08h)

HomeParams ends


Locals struct
    ;
    ; Callee register home space.
    ;

    CalleeHomeRcx   dq      ?       ; 8     24      32      (20h)
    CalleeHomeRdx   dq      ?       ; 8     16      24      (18h)
    CalleeHomeR8    dq      ?       ; 8     8       16      (10h)
    CalleeHomeR9    dq      ?       ; 8     0       8       (08h)

    ReturnValue dq  ?
    TscAux TSC_AUX { }

    Timestamp TSC { }

    ; 7     (+ 4)

    ProcessId dw ?
    ThreadId dw ?

    ; 8     (+ 1)

    TraceFrameRecord        dq ?
    union
        HookedFunction  FunctionPointer ?
        HookedFuncPtr   Function ptr ?
    ends

    Rflags  dq ?

Locals ends


        NESTED_ENTRY HookedTraceFrame, _TEXT$00, HookedTraceFrameHandler

        rex_push_reg rbp
        set_frame rbp, 8

        rex_push_eflags

        rex_push_reg rsi
        rex_push_reg rdi
        rex_push_reg rbx
        rex_push_reg r13

        alloc_stack sizeof(Locals)

        END_PROLOGUE

;
; Home our parameter registers.
; pointer below.
;
        save_reg rcx, Params.HomeRcx[rsp]   ; home rcx (param 1)
        save_reg rdx, Params.HomeRdx[rsp]   ; home rdx (param 2)
        save_reg r8,  Params.HomeR8x[rsp]   ; home r8  (param 3)
        save_reg r9,  Params.HomeR9x[rsp]   ; home r9  (param 4)
;
; Use rbp as the base frame pointer.
;

        mov rbp, rsp + sizeof(Locals)


HfEnd:  BEGIN_EPILOGUE

        add rsp, sizeof(Locals)


        lea rsp
        mov 

;
; Generate the entry timestamp.
;

        TIMESTAMP_TRACEFRAME Entered, r10

        ;
        ; Allocate record.
        ;

        ;
        ; Timestamp again.
        ;

        TIMESTAMP_TRACEFRAME PreCall, r10

        ; save timestamp
        ; save tsc aux


        ; call function
        ;mov r11, qword ptr TraceFrame.HookedFunction[r10]
        ;lea r11, Function.ContinuationAddress[r11]
        lea r11, (Function ptr TraceFrame.HookedFunction[r10]).ContinuationAddress
        call r11

        TIMESTAMP_TRACEFRAME PostCall, r10

        ; Save record.

        TIMESTAMP_TRACEFRAME PreExit, r10

;
; Move the EntryFrame/HOOKED_FUNCTION_CALL struct into rcx as the first
; parameter to HookEntry.
;
        mov     rcx, r10

;
; Load the HookedFunction pointer, and then the HookEntry pointer.
;

        mov     r11, EntryFrame.HookedFunction[r10]
        lea     r11, Function.HookEntry[r11]

;
; Call HookEntry(PHOOKED_FUNCTION_CALL Call).
;

        call    qword ptr [r11]

;
; Reload
;

        ;mov     r11,

;
; Load the pointer to the hook entry function.
;


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

        mov     r10, EntryFrame.HookedFunction[rbp]
        lea     r11, Function.ContinuationAddress[r10]

        ;lea     r10, EntryFrame.HookedFunction[rbp]
        ;lea     r11, Function.ContinuationAddress[r10]

        jmp qword ptr r11

;
; The pointer to the function struct will have been pushed to the stack via
; two DWORD pushes of an immediate value.  Load it into the rcx register.

;
; At this point, [rsp+8] will point to an address that matches the struct
; layout of HOOKED_FUNCTION_ENTRY.
;

        pop     rax
        ret

        NESTED_END HookFrame, _TEXT$00

;++
;
; ULONG
; HookFrameExceptionHandler(
;     _In_ PEXCEPTION_RECORD ExceptionRecord,
;     _In_ ULONG_PTR Frame,
;     _Inout_ PCONTEXT Context,
;     _Inout_ PDISPATCHER_CONTEXT DispatcherContext
;     )
;
; Routine Description:
;
;   This is the structured exception handler for HookFrame().  It is responsible
;   for catching EXCEPTION_IN_PAGE_ERROR exceptions that occur when we save the
;   trace frame details to a memory-map-backed trace store buffer.  Essentially,
;   it is the equivalent of this in C:
;
;
;       TRACE_FRAME TraceFrame;             // Local, stack-allocated frame.
;       PTRACE_FRAME TraceFramePointer;     // Pointer to trace store backing.
;
;       //
;       // <snip TraceFrame interaction>
;       //
;
;       TraceFramePointer = AllocationRoutine(Context, sizeof(TRACE_FRAME));
;
;       __try {
;
;           __movsb(TraceFramePointer,
;                   &TraceFrame,
;                   sizeof(TraceFrame));
;
;       } __except(GetExceptionCode() == STATUS_IN_PAGE_ERROR ?
;                  EXCEPTION_EXECUTE_HANDLER :
;                  EXCEPTION_CONTINUE_EXECUTION) {
;
;           TraceFramePointer = NULL;
;       }
;
; Arguments:
;
;   The standard exception handler arguments are passed in.
;
; Return Value:
;
;   XXX TODO
;
;--
        LEAF_ENTRY HookFrameExceptionHandler, _TEXT$00

        ;lea rax,


        LEAF_END HookFrameExceptionHandler, _TEXT$00


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
;   The return value from the hooked function is returned in rax.
;
;--

        NESTED_ENTRY HookProlog, _TEXT$00

        rex_push_reg    rbp     ; push rbp before we clobber it
        ;set_frame       rbp, 8  ; use rbp as our frame pointer

        rex_push_eflags         ; push rflags


;
; The hooking machinery will have loaded the qword ptr to the Function struct
; in rax.  Save it directly on the stack, then push flags and alloc space for
; the remaining items in our frame (entry/exit timestamp, return value).
;
        rex_push_reg    rax     ; function pointer


        alloc_stack 8 + 8 + 8   ; entry+exit timestamp, return value

        alloc_stack 4 * 8       ; our callee-allocated home space

;
;
;
;
; At this point, rsp points to the base of our EntryFrame.  We want to use
; masm's struct notation (i.e. `mov EntryFrame.Param1[rsp]`), but we can't use
; rsp (because we're about to allocate another 32 bytes for caller home params)
; so we use a volatile register, r10, instead.
;
        mov r10, rsp

;
; Now we can allocate space for caller home parameter registers.
;



;
; As r10 is a volatile register, we'll need to reload the
;

;
; And finally, home our current parameter registers.  (Note that this is *our*
; parameters that our caller will have reserved space for, not the space we
; reserved above when we're the callee.)
;

        mov     EntryFrame.HomeRcx[r10], rcx
        mov     EntryFrame.HomeRdx[r10], rdx
        mov     EntryFrame.HomeR8[r10], r8
        mov     EntryFrame.HomeR9[r10], r9

        END_PROLOGUE


;
; Generate the entry timestamp.
;
        lfence                  ; stabilize rdtsc
        rdtsc                   ; get timestamp counter
        shl     rdx, 32         ; low part -> high part
        or      rdx, rax        ; merge low part into rdx
        mov     EntryFrame.EntryTimestamp[r10], rdx  ; save counter

;
; Move the EntryFrame/HOOKED_FUNCTION_CALL struct into rcx as the first
; parameter to HookEntry.
;
        mov     rcx, r10

;
; Load the HookedFunction pointer, and then the HookEntry pointer.
;

        mov     r11, EntryFrame.HookedFunction[r10]
        lea     r11, Function.HookEntry[r11]

;
; Call HookEntry(PHOOKED_FUNCTION_CALL Call).
;

        call    qword ptr [r11]

;
; Reload
;

        ;mov     r11,

;
; Load the pointer to the hook entry function.
;


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

        mov     r10, EntryFrame.HookedFunction[rbp]
        lea     r11, Function.ContinuationAddress[r10]

        ;lea     r10, EntryFrame.HookedFunction[rbp]
        ;lea     r11, Function.ContinuationAddress[r10]

        jmp qword ptr r11

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


; vim:set tw=80 ts=8 sw=4 sts=4 expandtab syntax=masm formatoptions=croql      :

end