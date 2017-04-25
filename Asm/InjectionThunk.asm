        title "Injection Thunk Assembly Routine"
        option nokeyword:<Length>

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

include Asm.inc

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
; Define supporting UNICODE_STRING and STRING structures for ModulePath and
; FunctionName respectively.
;

UNICODE_STRING struct
    Length          dw      ?
    MaximumLength   dw      ?
    Padding         dd      ?
    Buffer          dq      ?
UNICODE_STRING ends

STRING struct
    Length          dw      ?
    MaximumLength   dw      ?
    Padding         dd      ?
    Buffer          dq      ?
STRING ends

;
; Define the RTL_INJECTION_THUNK_CONTEXT structure.
;

Thunk struct
        Flags                   dd              ?
        EntryCount              dw              ?
        UserDataOffset          dw              ?
        FunctionTable           dq              ?
        BaseCodeAddress         dq              ?
        RtlAddFunctionTable     dq              ?
        LoadLibraryW            dq              ?
        GetProcAddress          dq              ?
        ModulePath              UNICODE_STRING  { }
        FunctionName            STRING          { }
Thunk ends

;
; Define thunk flags.
;

DebugBreakOnEntry           equ     1

;
; Define error codes.
;

RtlAddFunctionTableFailed   equ     -1
LoadLibraryWFailed          equ     -2
GetProcAddressFailed        equ     -3

;++
;
; LONG
; InjectionThunk(
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

        NESTED_ENTRY InjectionThunk, _TEXT$00

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
; parameter home space, plus an additional 8 bytes to ensure the stack is
; kept aligned at a 16-byte boundary.
;

        alloc_stack 28h                         ; Reserve home param space.

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
; Check to see if the DebugBreakOnEntry flag is set in the thunk flags.  If it
; is, break.
;

        movsx   r8, word ptr Thunk.Flags[r12]   ; Move flags into r8d.
        test    r8, DebugBreakOnEntry           ; Test for debugbreak flag.
        jz      @F                              ; Flag isn't set.
        int     3                               ; Flag is set, so break.

;
; Register a runtime function for this currently executing piece of code.  This
; is done when we've been copied into memory at runtime.
;

@@:     mov     rcx, Thunk.FunctionTable[r12]           ; Load FunctionTable.
        movsx   rdx, word ptr Thunk.EntryCount[r12]     ; Load EntryCount.
        mov     r8, Thunk.BaseCodeAddress[r12]          ; Load BaseCodeAddress.
        call    Thunk.RtlAddFunctionTable[r12]          ; Invoke function.
        test    rax, rax                                ; Check result.
        jz      short @F                                ; Function failed.
        jmp     short Inj20                             ; Function succeeded.

@@:     mov     rax, RtlAddFunctionTableFailed          ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Prepare for a LoadLibraryW() call against the module path in the thunk.
;

Inj20:  mov     rcx, Thunk.ModulePath.Buffer[r12]       ; Load ModulePath.
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

Inj40:  mov     rcx, rax                                ; Load as 1st param.
        mov     rdx, Thunk.FunctionName.Buffer[r12]     ; Load name as 2nd.
        call    Thunk.GetProcAddress[r12]               ; Call GetProcAddress.
        test    rax, rax                                ; Check return value.
        jz      short @F                                ; Lookup failed.
        jmp     short Inj60                             ; Lookup succeeded.

@@:     mov     rax, GetProcAddressFailed               ; Load error code.
        jmp     short Inj90                             ; Jump to return.

;
; The function name was resolved successfully.  The function address lives in
; rax.  Load the offset of the user's data buffer and calculate the address
; based on the base address of the thunk.  Call the user's function with that
; address.
;

Inj60:  movsx   r8, word ptr Thunk.UserDataOffset[r12]  ; Load offset.
        mov     rcx, r12                                ; Load base address.
        add     rcx, r8                                 ; Add offset.
        call    rax                                     ; Call the function.

;
; Intentional follow-on to Inj90 to exit the function; rax will be returned back
; to the caller.
;

Inj90:

;
; Restore r12 and rbp from home parameter space and return rsp to its original
; value.
;

        mov     r12, Home.SavedR12[rbp]                 ; Restore non-vol r12.
        mov     rbp, Home.SavedRbp[rbp]                 ; Restore non-vol rbp.
        add     rsp, 28h                                ; Restore home space.

        ret

        NESTED_END InjectionThunk, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

end
