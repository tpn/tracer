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
; Define a local stack structure to be used within our injection routine.
; The base address of this routine will live in rsp after the prologue has run.
;

Locals struct

    ;
    ; Define home parameter space.
    ;

    CalleeHomeRcx   dq      ?
    CalleeHomeRdx   dq      ?
    CalleeHomeR8    dq      ?
    CalleeHomeR9    dq      ?

    ;
    ; Define space for additional function arguments (5-8) to be passed in via
    ; the stack.
    ;

    union

        ;
        ; Generic 5th parameter.
        ;

        CalleeParam5        dq      ?

        ;
        ; Used by MapViewOfFileExNuma().
        ;

        NumberOfBytesToMap  dq  ?

    ends

    union

        ;
        ; Generic 6th parameter.
        ;

        CalleeParam6            dq      ?

        ;
        ; Used by MapViewOfFileExNuma().
        ;

        PreferredBaseAddress    dq      ?

    ends

    union

        ;
        ; Generic 7th parameter.
        ;

        CalleeParam7            dq      ?

        ;
        ; Used by MapViewOfFileExNuma().
        ;

        PreferredNumaNode       dq      ?

    ends

    CalleeParam8            dq      ?

    ;
    ; Define local variables.
    ;

    Temp1                   dq      ?

    ;
    ; Define non-volatile register storage.
    ;

    union
        FirstNvRegister     dq      ?
        SavedRbp            dq      ?
    ends

    SavedRbx                dq      ?
    SavedRdi                dq      ?
    SavedRsi                dq      ?
    SavedR12                dq      ?
    SavedR13                dq      ?
    SavedR14                dq      ?
    SavedR15                dq      ?

    ;
    ; The return address of our caller is next, followed by home parameter
    ; space provided for rcx-r9.
    ;

    ReturnAddress           dq      ?
    HomeRcx                 dq      ?
    HomeRdx                 dq      ?
    HomeR8                  dq      ?
    HomeR9                  dq      ?

Locals ends

;
; Exclude the return address onward from the frame calculation size.
;

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.ReturnAddress - (sizeof Locals)))

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
PUNICODE_STRING typedef ptr UNICODE_STRING

STRING struct
    Length          dw      ?
    MaximumLength   dw      ?
    Padding         dd      ?
    Buffer          dq      ?
STRING ends

;
; Define the LARGE_INTEGER structure.
;

LARGE_INTEGER union
    struct
        LowPart     dw      ?
        HighPart    dw      ?
    ends
    QuadPart        dq      ?
LARGE_INTEGER ends

;
; Define the APC structure.
;

APC struct
    Routine     dq      ?
    Argument1   dq      ?
    Argument2   dq      ?
    Argument3   dq      ?
APC ends

;
; Define the INJECTION_FUNCTIONS structure.  This encapsulates all function
; pointers available for use as part of injection.
;

INJECTION_FUNCTIONS struct
    RtlAddFunctionTable     dq      ?
    LoadLibraryExW          dq      ?
    GetProcAddress          dq      ?
    GetLastError            dq      ?
    SetLastError            dq      ?
    SetEvent                dq      ?
    ResetEvent              dq      ?
    GetThreadContext        dq      ?
    SetThreadContext        dq      ?
    SuspendThread           dq      ?
    ResumeThread            dq      ?
    OpenEventW              dq      ?
    CloseHandle             dq      ?
    SignalObjectAndWait     dq      ?
    WaitForSingleObjectEx   dq      ?
    OutputDebugStringA      dq      ?
    OutputDebugStringW      dq      ?
    NtQueueApcThread        dq      ?
    NtTestAlert             dq      ?
    QueueUserAPC            dq      ?
    SleepEx                 dq      ?
    ExitThread              dq      ?
    GetExitCodeThread       dq      ?
    DeviceIoControl         dq      ?
    CreateFileW             dq      ?
    CreateFileMappingW      dq      ?
    OpenFileMappingW        dq      ?
    MapViewOfFileEx         dq      ?
    MapViewOfFileExNuma     dq      ?
    FlushViewOfFile         dq      ?
    UnmapViewOfFileEx       dq      ?
    VirtualAllocEx          dq      ?
    VirtualFreeEx           dq      ?
    VirtualProtectEx        dq      ?
    VirtualQueryEx          dq      ?
INJECTION_FUNCTIONS ends

;
; Define the injection object structures.
;

INJECTION_OBJECT_EVENT struct
    EventName       PUNICODE_STRING ?
    EventHandle     dq	            ?
    DesiredAccess   dd              ?
    InheritHandle   dd              ?
INJECTION_OBJECT_EVENT ends

INJECTION_OBJECT_FILE_MAPPING struct
    MappingName             PUNICODE_STRING     ?
    MappingHandle           dq                  ?
    DesiredAccess           dd                  ?
    InheritHandle           dd                  ?
    ShareMode               dd                  ?
    CreationDisposition     dd                  ?
    FlagsAndAttributes      dd                  ?
    AllocationType          dd                  ?
    PreferredNode           dd                  ?
    Padding                 dd                  ?
    FileOffset              dq                  ?
    MappingSize             dq                  ?
    PreferredBaseAddress    dq                  ?
    BaseAddress             dq                  ?
INJECTION_OBJECT_FILE_MAPPING ends

INJECTION_OBJECT_CONTEXT union
    AsEvent         INJECTION_OBJECT_EVENT          { }
    AsFileMapping   INJECTION_OBJECT_FILE_MAPPING   { }
INJECTION_OBJECT_CONTEXT ends
PINJECTION_OBJECT_CONTEXT typedef ptr INJECTION_OBJECT_CONTEXT

INJECTION_OBJECT_TYPE typedef dd
PINJECTION_OBJECT_TYPE typedef ptr INJECTION_OBJECT_TYPE

INJECTION_OBJECT struct
    Name        PUNICODE_STRING             ?
    ObjectType  PINJECTION_OBJECT_TYPE      ?
    Context     PINJECTION_OBJECT_CONTEXT   ?
    Unused      dq                          ?
INJECTION_OBJECT ends
PINJECTION_OBJECT typedef ptr INJECTION_OBJECT

INJECTION_OBJECTS struct
    StructSizeInBytes                       dw                          ?
    NumberOfObjects                         dw                          ?
    SizeOfInjectionObjectInBytes            dw                          ?
    SizeOfInjectionObjectContextInBytes     dw                          ?
    TotalAllocSizeInBytes                   dd                          ?
    Flags                                   dd                          ?
    FirstObject                             PINJECTION_OBJECT           ?
    FirstName                               PUNICODE_STRING             ?
    Types                                   PINJECTION_OBJECT_TYPE      ?
    Contexts                                PINJECTION_OBJECT_CONTEXT   ?
    Errors                                  dq                          ?
INJECTION_OBJECTS ends
PINJECTION_OBJECTS typedef ptr INJECTION_OBJECT

;
; Define helper typedefs for structures that are nicer to work with in assembly
; than their long uppercase names.
;

Object typedef INJECTION_OBJECT
Objects typedef INJECTION_OBJECTS
Unicode typedef UNICODE_STRING
Functions typedef INJECTION_FUNCTIONS
Event typedef INJECTION_OBJECT_EVENT
FileMapping typedef INJECTION_OBJECT_FILE_MAPPING

;
; Define the RTL_INJECTION_THUNK_CONTEXT structure.
;

Thunk struct
    Flags                   dd                      ?
    EntryCount              dw                      ?
    UserDataOffset          dw                      ?
    FunctionTable           dq                      ?
    BaseCodeAddress         dq                      ?
    InjectionFunctions      INJECTION_FUNCTIONS     { }
    InjectionObjects        PINJECTION_OBJECTS      ?
    ModulePath              UNICODE_STRING          { }
    FunctionName            STRING                  { }
    UserApc                 APC                     { }
Thunk ends

;
; Define thunk flags.
;

DebugBreakOnEntry           equ     1
HasInjectionObjects         equ     2
HasModuleAndFunction        equ     4
HasApc                      equ     8

;
; Define injection object types.
;

EventObjectType             equ     1
FileMappingObjectType       equ     2

;
; Define error codes.
;

RtlAddFunctionTableFailed   equ     1000
LoadLibraryWFailed          equ     1001
GetProcAddressFailed        equ     1002
InvalidInjectionContext     equ     1003

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
; Thunk prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE                 ; Allocate stack space.
        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        END_PROLOGUE

;
; Home our Thunk (rcx) parameter register, then save in r12.  The homing of rcx
; isn't technically necessary (as we never re-load it from rcx), but it doesn't
; hurt, and it is useful during development and debugging to help detect certain
; anomalies (like clobbering r12 accidentally, for example).
;

        mov     Locals.HomeRcx[rsp], rcx            ; Home Thunk (rcx) param.
        mov     r12, rcx                            ; Move Thunk into r12.

;
; Check to see if the DebugBreakOnEntry flag is set in the thunk flags.  If it
; is, break.
;

        movsx   r8, word ptr Thunk.Flags[r12]   ; Move flags into r8.
        test    r8, DebugBreakOnEntry           ; Test for debugbreak flag.
        jz      @F                              ; Flag isn't set.
        int     3                               ; Flag is set, so break.

;
; Register a runtime function for this currently executing piece of code.  This
; must be the first step taken by this routine in order to ensure the kernel has
; the appropriate unwind info in place if it needs to unwind this frame.
;

@@:     mov     rcx, Thunk.FunctionTable[r12]           ; Load FunctionTable.
        movsx   rdx, word ptr Thunk.EntryCount[r12]     ; Load EntryCount.
        mov     r8, Thunk.BaseCodeAddress[r12]          ; Load BaseCodeAddress.
        mov     r13, Thunk.InjectionFunctions[r12]      ; Load functions.
        call    Functions.RtlAddFunctionTable[r13]      ; Invoke function.
        test    rax, rax                                ; Check result.
        jz      short @F                                ; Function failed.
        jmp     short Inj10                             ; Function succeeded.

@@:     int     3
        mov     rax, RtlAddFunctionTableFailed          ; Load error code.
        jmp     Inj90                                   ; Jump to epilogue.

;
; Check to see if the thunk has any injection objects.  If it doesn't, proceed
; to the test to see if it has a module path and function name.
;

Inj10:  movsx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasInjectionObjects                 ; Any injection objects?
        jnz     Inj60                                   ; No; check mod+func.

;
; The injection thunk features one or more injection objects.  An object can be
; an event or a file mapping.  We loop through the objects and process each one
; according to its type.
;
; N.B. Register use for this loop is as follows:
;
;   rbx - Supplies the address of the active INJECTION_OBJECT structure.
;
;   rbp - Supplies the address of the active INJECTION_OBJECT_CONTEXT structure.
;
;   r14 - Supplies the address of the INJECTION_OBJECTS structure.
;
;   r15 - Supplies the value of the number of injection objects indicated by
;       the INJECTION_OBJECTS.NumberOfObjects field.
;
;   rdi - Supplies the base address of the INJECTION_OBJECT array.
;
;   rsi - Supplies the current loop index.  This is initialized to 0, and will
;       have a maximum value of NumberOfObjects-1.
;
; N.B. Erroneous conditions are handled by an immediate `int 3` breakpoint.
;

;
; Load the Thunk->InjectionObjects pointer into r14.  Verify that it is not
; NULL, then load the count into r15 and verify it is > 0.  Finally, verify the
; size of the individual injection object matches our sizeof against the struct.
; Break on any errors.
;

        mov     r14, Thunk.InjectionObjects[r12]        ; Load pointer into r14.
        test    r14, r14                                ; Check not-NULL.
        jnz     short @F                                ; Pointer is valid.
        int     3                                       ; Break because NULL ptr

@@:     xor     r15, r15                                ; Clear dependency.
        mov     r15w, Objects.NumberOfObjects[r14]      ; Load # of objects.
        test    r15, r15                                ; At least 1 objects?
        jnz     short @F                                ; Yes, continue.
        int     3                                       ; No, break.

@@:     movsx   rax, word ptr Objects.SizeOfInjectionObjectInBytes[r14]
        cmp     rax, (sizeof INJECTION_OBJECT)          ; Compare object size.
        je      short @F                                ; Sizes are equal, cont.
        int     3                                       ; Sizes != equal, break.

@@:     movsx   rax, word ptr Objects.SizeOfInjectionObjectContextInBytes[r14]
        cmp     rax, (sizeof INJECTION_OBJECT_CONTEXT)  ; Compare ctx size.
        je      short @F                                ; Sizes are equal, cont.
        int     3                                       ; Sizes != equal, break.
        jmp     Inj90                                   ; Jump to end.

;
; Invariant checks passed.  Zero our loop counter (rsi) and load the base
; address of the injection objects array into rdi.
;

        xor     rsi, rsi                                  ; Zero loop counter.
        mov     rdi, Objects.FirstObject[r14]             ; Load first address.

;
; Top of loop.  Load next injection object (into rbx), load the context into
; rbp, the type into r9d, determine whether we're dealing with an event or
; file mapping handle, and process accordingly.
;

        align   16

Inj20:  lea     rbx, [rdi + rsi]                        ; Load next inj. obj.
        mov     rbp, Object.Context[rbx]                ; Load context.
        mov     rax, Object.ObjectType[rbx]             ; Load ptr to obj type.
        mov     r9d, dword ptr [rax]                    ; Load object type.
        cmp     r9d, EventObjectType                    ; Is event type?
        jne     short Inj40                             ; No; try file mapping.

;
; This is an event object type.  Load the arguments for OpenEventW into relevant
; registers and invoke the function.
;

Inj30:  mov     ecx, Event.DesiredAccess[rbp]           ; Load desired access.
        mov     edx, Event.InheritHandle[rbp]           ; Load inherit handle.
        mov     rax, Event.EventName[rbp]               ; Load unicode string.
        mov     r8, Unicode.Buffer[rax]                 ; Load event name.
        call    Functions.OpenEventW[r13]               ; Call OpenEventW().
        test    rax, rax                                ; Check result.
        jnz     short @F                                ; Valid handle, proceed.
        int     3                                       ; Invalid handle, brk.
        call    Functions.GetLastError[r13]             ; Get last error.
        jmp     Inj90                                   ; Jump to end.

@@:     mov     Event.EventHandle[rbp], rax             ; Save event handle.
        jmp     Inj50                                   ; Jump to loop bottom.

;
; This is a file mapping type.  We need to open the file mapping, then map it.
;

Inj40:  mov     ecx, FileMapping.DesiredAccess[rbp]     ; Load desired access.
        mov     edx, FileMapping.InheritHandle[rbp]     ; Load inherit handle.
        mov     rax, FileMapping.MappingName[rbp]       ; Load unicode string.
        mov     r8, Unicode.Buffer[rax]                 ; Load mapping name.
        call    Functions.OpenFileMappingW[r13]         ; Call function.
        test    rax, rax                                ; Check result.
        jnz     short @F                                ; Valid handle, proceed.
        int     3                                       ; Invalid handle, brk.
        call    Functions.GetLastError[r13]             ; Get last error.
        jmp     Inj90                                   ; Jump to end.

;
; We successfully opened the file mapping.  Save the handle, then map it.  For
; now, we use MapViewOfFileExNuma(), although the underlying file mapping struct
; has support for AllocationType, PageProtection and PreferredNode such that
; MapViewOfFileNuma2() could eventually be used.
;

@@:     mov     qword ptr FileMapping.MappingHandle[rbp], rax ; Save handle.

;
; Load the preferred base address into the relevant stack offset now, outside
; of the main body that prepares the rest of the arguments.  This allows us to
; override the address if it couldn't be granted and retry the mapping with
; relative ease.
;

        mov     rax, FileMapping.PreferredBaseAddress[rbp]  ; Load pref. BA.
        mov     Locals.PreferredBaseAddress[rsp], rax       ; ...as 6th arg.

Inj45:  mov     rcx, FileMapping.MappingHandle[rbp]     ; Load handle.
        xor     edx, edx                                ; Clear reg.
        mov     edx, FileMapping.DesiredAccess[rbp]     ; Load desired access.

;
; N.B. The file offset requires a bit of LARGE_INTEGER juggling.
;

        mov     rax, FileMapping.FileOffset[rbp]        ; Load file offset.
        mov     r8, rax                                 ; Load offset into r8.
        shr     r8, 32                                  ; Load high offset.
        xor     r9, r9                                  ; Clear dependency.
        mov     r9d, eax                                ; Load low offset.

;
; Continue loading arguments; preferred address has already been done (see our
; comment above), so that leaves mapping size and preferred NUMA node.  All
; arguments herein are provided via the stack.
;

        mov     rax, FileMapping.MappingSize[rbp]       ; Load mapping size.
        mov     Locals.NumberOfBytesToMap[rsp], rax     ; ...as 5th arg.
        mov     eax, FileMapping.PreferredNode[rbp]     ; Load pref NN.
        mov     Locals.PreferredNumaNode[rsp], rax      ; ...as 7th arg.

;
; Call the function and test to see if we got a valid (non-NULL) address back.
;

        call    Functions.MapViewOfFileExNuma[r13]      ; Invoke.
        test    rax, rax                                ; Check return value.
        jz      short Inj47                             ; Invalid addr.

;
; The view was mapped successfully.  Save the base address back to the context,
; then continue the loop by falling through.
;

        mov     FileMapping.BaseAddress[rbp], rax       ; Save base addr.
        jmp     short Inj50                             ; Jump to loop bottom.

;
; The mapping attempt failed.  If we provided a preferred base address, try the
; request again
;

Inj47:  mov     rax, Locals.PreferredBaseAddress[rbp]   ; Load pref. base.
        test    rax, rax                                ; Is address NULL?
        jnz     short @F                                ; No, jump to retry.

;
; Preferred base address was NULL and the call still failed, so the failure
; wasn't a result of not being able to assign the address we wanted.  This
; is considered fatal, so debugbreak, get last error (to help when stepping
; via debugger), then jump to end.
;

        int     3                                       ; Invalid addr, break.
        call    Functions.GetLastError[r13]             ; Get last error.
        jmp     Inj90                                   ; Jump to end.

;
; Clear the preferred address and re-try the mapping.
;

@@:     xor     rax, rax                                        ; Clear rax.
        mov     qword ptr Locals.PreferredBaseAddress[rbp], rax ; Zero address.
        jmp     short Inj45

;
; Bottom of the loop.  Update loop counter and determine if there are more
; objects to process.  If there are, continue back at the start (Inj20).  If
; not, fall through to the APC test.
;

Inj50:  add     rsi, 1                                  ; Index++
        cmp     rsi, r15                                ; Compare to # objects.
        jl      Inj20                                   ; Index < Count, cont.

;
; Intentional follow-on to to APC test logic.
;

;
; Check to see if we've been requested to call a generic APC routine.
;

Inj55:  mov     r8d, Thunk.Flags[r12]                   ; Move flags into r8d.
        test    r8d, HasApc                             ; APC flag set?
        jz      short Inj60                             ; No; jump to mod+fn.

;
; A generic APC routine has been requested.  We pass the thunk as the first
; parameter irrespective of what the value of Argument1 is.  The remaining two
; arguments are passed along.
;

Inj57:  mov     rcx, r12                                ; Load thunk into arg 1.
        mov     rdx, Thunk.UserApc.Argument2[r12]       ; Load arg 2.
        mov     r8, Thunk.UserApc.Argument3[r12]        ; Load arg 3.
        mov     rax, Thunk.UserApc.Routine[r12]         ; Load the routine.
        call    rax                                     ; Call it.

;
; Check to see if we've been requested to load a module and resolve a function.
;

Inj60:  xor     rax, rax                                ; Clear rax.
        movsx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasModuleAndFunction                ; Has module+func?
        jz      Inj90                                   ; No; we're done.

;
; Prepare for a LoadLibraryW() call against the module path in the thunk.
;

        mov     rcx, Thunk.ModulePath.Buffer[r12]       ; Load ModulePath.
        call    Functions.LoadLibraryExW[r13]           ; Call LoadLibraryExW().
        test    rax, rax                                ; Check Handle != NULL.
        jz      short @F                                ; Handle is NULL.
        jmp     short Inj70                             ; Handle is valid.

@@:     mov     rax, LoadLibraryWFailed                 ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Module was loaded successfully.  The Handle value lives in rax.  Save a copy
; in the thunk, then prepare arguments for a call to GetProcAddress().
;

Inj70:  mov     rcx, rax                                ; Load as 1st param.
        mov     rdx, Thunk.FunctionName.Buffer[r12]     ; Load name as 2nd.
        call    Functions.GetProcAddress[r13]           ; Call GetProcAddress().
        test    rax, rax                                ; Check return value.
        jz      short @F                                ; Lookup failed.
        jmp     short Inj80                             ; Lookup succeeded.

@@:     mov     rax, GetProcAddressFailed               ; Load error code.
        jmp     short Inj90                             ; Jump to return.

;
; The function name was resolved successfully.  The function address lives in
; rax.  Load the offset of the user's data buffer and calculate the address
; based on the base address of the thunk.  Call the user's function with that
; address.
;

Inj80:  movsx   r8, word ptr Thunk.UserDataOffset[r12]  ; Load offset.
        mov     rcx, r12                                ; Load base address.
        add     rcx, r8                                 ; Add offset.
        call    rax                                     ; Call the function.

;
; Intentional follow-on to Inj90 to exit the function; rax will be returned back
; to the caller.
;

Inj90:

;
; Restore non-volatile registers.
;

        mov     rbp, Locals.SavedRbp[rsp]
        mov     rbx, Locals.SavedRbx[rsp]
        mov     rdi, Locals.SavedRdi[rsp]
        mov     rsi, Locals.SavedRsi[rsp]
        mov     r12, Locals.SavedR12[rsp]
        mov     r13, Locals.SavedR13[rsp]
        mov     r14, Locals.SavedR14[rsp]
        mov     r15, Locals.SavedR15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret

        NESTED_END InjectionThunk, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

end
