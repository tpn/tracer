        title "Injection Thunk Assembly Routine"

;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   InjectionThunk4.asm
;
; Abstract:
;
;   This module implements the injection thunk routine as a NESTED_ENTRY.
;
;--

include ksamd64.inc

;
; Define a home parameter + return address structure.  Before alloc_stack in
; the prologue, this struct is used against rsp.  After rsp, we use rbp.
;

Home struct
        ReturnAddress   dq      ?       ; 8     32      40      (28h)
        union
            Thunk       dq      ?       ; 8     24      32      (20h)
            SavedRcx    dq      ?       ; 8     24      32      (20h)
            Param1      dq      ?       ; 8     24      32      (20h)
        ends
        union
            SavedR12    dq      ?       ; 8     16      24      (18h)
            Param2      dq      ?       ; 8     16      24      (18h)
        ends
        union
            SavedRbp    dq      ?       ; 8     8       16      (10h)
            SavedR8     dq      ?       ; 8     8       16      (10h)
            Param3      dq      ?       ; 8     8       16      (10h)
        ends
        union
            SavedR9     dq      ?       ; 8     0       8       (08h)
            Param4      dq      ?       ; 8     0       8       (08h)
        ends
Home ends

;
; Define flags for RTL_INJECTION_THUNK_CONTEXT.Flags.
;

AddRuntimeFunction      equ         1
TestExceptionHandler    equ         2
TestAccessViolation     equ         4
TestIllegalInstruction  equ         8

;
; Define the RTL_INJECTION_THUNK_CONTEXT structure.
;


Thunk struct
        Flags               dd      ?
        EntryCount          dd      ?
        FunctionTable       dq      ?
        BaseAddress         dq      ?
        RtlAddFunctionTable dq      ?
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

;
; Define error codes.
;

RtlAddFunctionTableFailed   equ     -1
LoadLibraryWFailed          equ     -2
GetProcAddressFailed        equ     -3

;
; Forward declarations of Fault/Resume addresses in InjectionThunk body.
;

altentry InjectionThunkAVFault
altentry InjectionThunkAVResume
altentry InjectionThunkIllInstFault
altentry InjectionThunkIllInstResume

;++
;
; LONG
; InjectionThunk4ExceptionHandler(
;     _In_ PEXCEPTION_RECORD ExceptionRecord,
;     _In_ PVOID EstablisherFrame,
;     _Inout_ PCONTEXT ContextRecord,
;     _Inout_ DispatcherContext
;     )
;
; Routine Description:
;
;   This is the exception handler for our injection thunk routine.  Its purpose
;   is to trap the access violation that the thunk can be directed to trigger
;   and ignore it, returning execution back to the thunk.  This isn't used in
;   practice -- it's simply a test that we grok how to write exception handlers
;   in assembly, and that they can be effectively relocated as part of dynamic
;   injection.
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
;   access violation, we adjust the RIP and return ExceptionContinueExecution.
;   Return ExceptionContinueSearch in all other cases.
;
;--

        LEAF_ENTRY InjectionThunkHandler, _TEXT$00

;
; If this is an unwind, return ExceptionContinueSearch.
;

        test    dword ptr ErExceptionFlags[rcx], EXCEPTION_UNWIND
        jnz     short Injh90                    ; This is an unwind, return.

;
; Check to see if this is an access violation exception.  If it isn't, it won't
; (or shouldn't) have come from our injection thunk routine.
;

        xor     rax, rax
        mov     eax, dword ptr ErExceptionCode[rcx] ; Load exception code.
        cmp     eax, STATUS_ACCESS_VIOLATION        ; Was this an AV?
        jne     short Injh90                        ; Wasn't an AV, return.

;
; Load the known fault address into rax and compare against the actual fault
; IP of the triggering exception.
;
        lea     rax, InjectionThunkAVFault      ; Load RIP of known fault.
        cmp     rax, CxRip[r8]                  ; Compare to actual RIP.
        jne     short Injh90                    ; Doesn't match, return.

;
; The exception occurred on our fault address.  Update the RIP in the context
; to skip past the faulting exception, then return ExceptionContinueExecution.
;

        lea     rax, InjectionThunkAVResume     ; Load resume address.
        mov     CxRip[r8], rax                  ; Save into RIP.
        mov     eax, ExceptionContinueExecution ; Load return code.
        ret

Injh90: mov     eax, ExceptionContinueSearch
        ret

        LEAF_END InjectionThunkHandler, _TEXT$00


;++
;
; LONG
; InjectionThunk4(
;     _In_ PRTL_INJECTION_THUNK_CONTEXT Thunk
;     );
;
; Routine Description:
;
;   This routine is the initial entry point of our injection logic.  That is,
;   newly created remoted threads in a target process have their start address
;   set to a copy of this routine (that was prepared in a separate process and
;   then injected via WriteProcessMemory()).
;
;   It is responsible for registering a runtime function for itself, such that
;   appropriate unwind data can be found by the kernel if an exception occurs
;   and the stack is being unwound.  It then loads a library designated by the
;   fully qualified path in the thunk's module path field via LoadLibraryW,
;   then calls GetProcAddress on the returned module for the function name also
;   contained within the thunk.  If an address is successfully resolved, the
;   routine is called with the thunk back as the first parameter, and the return
;   value is propagated back to our caller (typically, this will be the routine
;   kernel32!UserBaseInitThunk).
;
;   In practice, the module path we attempt to load is InjectionThunk.dll, and
;   the function name we resolve is "InjectionRemoteThreadEntry".  This routine
;   is responsible for doing more heavy lifting in C prior to calling the actual
;   caller's end routine.
;
; Arguments:
;
;   Thunk (rcx) - Supplies a pointer to the injection context thunk.
;
; Return Value:
;
;   If an error occured in this routine, an error code (see above) will be
;   returned (ranging in value from -1 to -3).  If this routine succeeded,
;   the return value of the function we were requested to execute will be
;   returned instead.  (Unfortunately, there's no guarantee that this won't
;   overlap with our error codes.)
;
;   This return value will end up as the exit code for the thread if being
;   called in the injection context.
;
;--

        NESTED_ENTRY InjectionThunk4, _TEXT$00, InjectionThunkHandler

;
; This routine uses the non-volatile r12 to store the Thunk pointer (initially
; passed in via rcx).  As we only have one parameter, we use the home parameter
; space reserved for rdx for saving r12 (instead of pushing it to the stack).
;

        save_reg r12, Home.SavedR12             ; Use rdx home to save r12.

;
; We use rbp as our frame pointer.  As with r12, we repurpose r8's home area
; instead of pushing to the stack, then call set_frame (which sets rsp to rbp).
;

        save_reg    rbp, Home.SavedRbp          ; Use r8 home to save rbp.
        set_frame   rbp, 0                      ; Use rbp as frame pointer.

;
; As we are calling other functions, we need to reserve 32 bytes for their
; parameter home space.
;

        alloc_stack 20h                         ; Reserve home param space.

        END_PROLOGUE

;
; Home our Thunk parameter register, then save in r12.  The homing of rcx isn't
; technically necessary (as we never re-load it from rcx), but it doesn't hurt,
; and it is useful during development and debugging to help detect anomalies
; (if we clobber r12 accidentally, for example).
;

        mov     Home.Thunk[rbp], rcx            ; Home Thunk (rcx) parameter.
        mov     r12, rcx                        ; Move Thunk into r12.

;
; Determine if we need to register a runtime function entry for this thunk.
;

        mov     r8d, Thunk.Flags[r12]           ; Load flags into r8d.
        test    r8d, AddRuntimeFunction         ; Is flag set?
        jz      short Inj10                     ; No, jump to EH check.

;
; Register a runtime function for this currently executing piece of code.  This
; is done when we've been copied into memory at runtime.
;

        mov     rcx, Thunk.FunctionTable[r12]           ; Load FunctionTable.
        xor     rdx, rdx                                ; Clear rdx.
        xor     r9, r9                                  ; Clear r9.
        mov     edx, dword ptr Thunk.EntryCount[r12]    ; Load EntryCount.
        mov     r8, Thunk.BaseAddress[r12]              ; Load BaseAddress.
        mov     rax, Thunk.RtlAddFunctionTable[r12]     ; Load Function.
        call    rax                                     ; Invoke function.
        test    rax, rax                                ; Check result.
        jz      short @F                                ; Function failed.
        jmp     short Inj10                             ; Function succeeded.

@@:     mov     rax, RtlAddFunctionTableFailed          ; Load error code.
        jmp     Inj90                                   ; Jump to epilogue.

;
; Determine if an access violation test has been requested.  Note that this is
; only ever performed after a runtime function table has been added.  It is used
; solely for testing our exception handling logic.
;

Inj10:  mov     r8d, Thunk.Flags[r12]           ; Load flags into r8d.
        test    r8d, TestExceptionHandler       ; Is flag set?
        jnz     Inj15                           ; Yes, jump to test.
        jmp     Inj20                           ; No, jump to main.

Inj15:  test    r8d, TestAccessViolation        ; Is AV test?
        jnz     Inj18                           ; Yes, jump to test.

        test    r8d, TestIllegalInstruction     ; Is IL test?
        jnz     Inj17                           ; Yes, jump to test.

        int 3                                   ; Invariant failed: no flags set
        jmp     short Inj90

Inj17:  nop
        ALTERNATE_ENTRY InjectionThunkIllInstFault
        db      0fh                             ; Trigger illegal inst fault.
        db      0bh
        ALTERNATE_ENTRY InjectionThunkIllInstResume
        jmp     Inj20                           ; Jump to main.

        align   16
Inj18:  xor     rax, rax                                ; Zero rax.

        ALTERNATE_ENTRY InjectionThunkAVFault
        mov     [rax], rax                              ; Force trap.
        ALTERNATE_ENTRY InjectionThunkAVResume

;
; Prepare for a LoadLibraryW() call against the module path in the thunk.
;

Inj20:  mov     rcx, Thunk.ModulePath[r12]              ; Load ModulePath.
        call    Thunk.LoadLibraryW[r12]                 ; Call LoadLibraryW().
        test    rax, rax                                ; Check Handle != NULL.
        jz      short @F                                ; Handle is NULL.
        jmp     short Inj40                             ; Handle is valid.

@@:     mov     rax, LoadLibraryWFailed                 ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Module was loaded successfully.  The Handle value lives in rax.  Save a copy
; in the thunk, then prepare arguments for a call to GetProcAddress().
;

Inj40:  mov     Thunk.ModuleHandle[r12], rax            ; Save Handle.
        mov     rcx, rax                                ; Load as 1st param.
        mov     rdx, Thunk.FunctionName[r12]            ; Load name as 2nd.
        call    Thunk.GetProcAddress[r12]               ; Call GetProcAddress.
        test    rax, rax                                ; Check return value.
        jz      short @F                                ; Lookup failed.
        jmp     short Inj60                             ; Lookup succeeded.

@@:     mov     rax, GetProcAddressFailed               ; Load error code.
        jmp     short Inj90                             ; Jump to return.

;
; The function name was resolved successfully.  The function address lives in
; rax.  Save a copy in the thunk, and then prepare arguments for a call to it.
;

Inj60:  mov     Thunk.FunctionAddress[r12], rax         ; Save func ptr.
        mov     rcx, r12                                ; Load thunk into rcx.
        call    rax                                     ; Call the function.

;
; Intentional follow-on to Inj90 to exit the function; rax will be returned back
; to the caller.
;

Inj90:

;
; Begin epilogue.  Restore r12 and rbp from home parameter space and return
; rsp to its original value.
;

        mov     r12, Home.SavedR12[rbp]                 ; Restore non-vol r12.
        mov     rbp, Home.SavedRbp[rbp]                 ; Restore non-vol rbp.
        add     rsp, 20h                                ; Restore home space.

        ret

        NESTED_END InjectionThunk4, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

end
