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
    WaitForSingleObject     dq      ?
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
    GetModuleHandleW        dq      ?
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
    ObjectType              dd                      ?
    ObjectFlags             dd                      ?
    ObjectName              UNICODE_STRING          { }
    Handle                  dq                      ?
    DesiredAccess           dd                      ?
    InheritHandle           dd                      ?
INJECTION_OBJECT_EVENT ends
.erre (size INJECTION_OBJECT_EVENT eq 40), @CatStr(<Size error: INJECTION_OBJECT_EVENT: >, %(size INJECTION_OBJECT_EVENT))

INJECTION_OBJECT_FILE_MAPPING struct
    ObjectType              dd                      ?
    ObjectFlags             dd                      ?
    ObjectName              UNICODE_STRING          { }
    Handle                  dq                      ?
    DesiredAccess           dd                      ?
    InheritHandle           dd                      ?
    ShareMode               dd                      ?
    CreationDisposition     dd                      ?
    FlagsAndAttributes      dd                      ?
    AllocationType          dd                      ?
    PageProtection          dd                      ?
    PreferredNumaNode       dd                      ?
    FileOffset              dq                      ?
    MappingSize             dq                      ?
    PreferredBaseAddress    dq                      ?
    BaseAddress             dq                      ?
    Path                    UNICODE_STRING          { }
    FileHandle              dq                      ?
    Padding                 dq                      ?
INJECTION_OBJECT_FILE_MAPPING ends
.erre (size INJECTION_OBJECT_FILE_MAPPING eq 128), @CatStr(<Size error: INJECTION_OBJECT_FILE_MAPPING: >, %(size INJECTION_OBJECT_FILE_MAPPING))

INJECTION_OBJECT union
    struct
        ObjectType      dd                          ?
        ObjectFlags     dd                          ?
        ObjectName      UNICODE_STRING              { }
        Handle          dq                          ?
        DesiredAccess   dd                          ?
        InheritHandle   dd                          ?
        Padding         dq 11 dup                   (?)
    ends
    AsEvent         INJECTION_OBJECT_EVENT          { }
    AsFileMapping   INJECTION_OBJECT_FILE_MAPPING   { }
INJECTION_OBJECT ends
.erre (size INJECTION_OBJECT eq 128), @CatStr(<Size error: INJECTION_OBJECT: >, %(size INJECTION_OBJECT))

PINJECTION_OBJECT typedef ptr INJECTION_OBJECT

INJECTION_OBJECTS struct
    SizeOfStruct                    dw              ?
    SizeOfInjectionObjectInBytes    dw              ?
    NumberOfObjects                 dd              ?
    TotalAllocSizeInBytes           dd              ?
    Flags                           dd              ?
    FirstObject                     dq              ?
    ModuleHandle                    dq              ?
    FunctionPointer                 dq              ?
    Padding                         dq 11 dup       (?)
INJECTION_OBJECTS ends
.erre (size INJECTION_OBJECTS eq 128), @CatStr(<Size error: INJECTION_OBJECTS: >, %(size INJECTION_OBJECTS))
PINJECTION_OBJECTS typedef ptr INJECTION_OBJECT

;
; Define the INJECTION_THUNK_CONTEXT structure.
;

INJECTION_THUNK_CONTEXT struct
    Flags                   dd                      ?
    EntryCount              dw                      ?
    UserDataOffset          dw                      ?
    FunctionTable           dq                      ?
    BaseCodeAddress         dq                      ?
    InjectionObjects        PINJECTION_OBJECTS      ?
    ModulePath              UNICODE_STRING          { }
    FunctionName            STRING                  { }
    UserApc                 APC                     { }
    InjectionFunctions      INJECTION_FUNCTIONS     { }
INJECTION_THUNK_CONTEXT ends

;
; Define helper typedefs for structures that are nicer to work with in assembly
; than their long uppercase names.
;

Thunk typedef INJECTION_THUNK_CONTEXT
Object typedef INJECTION_OBJECT
Objects typedef INJECTION_OBJECTS
Unicode typedef UNICODE_STRING
Functions typedef INJECTION_FUNCTIONS
Event typedef INJECTION_OBJECT_EVENT
FileMapping typedef INJECTION_OBJECT_FILE_MAPPING

;
; Define thunk flags.
;

DebugBreakOnEntry               equ     1
HasInjectionObjects             equ     2
HasModuleAndFunction            equ     4
HasApc                          equ     8
WaitOnFirstEvent                equ     16
DebugBreakAfterFirstEvent       equ     32

;
; Define injection object event flags.
;

ManualReset                     equ     1
WaitOnEventAfterOpening         equ     2
DebugBreakAfterWaitSatisfied    equ     4
SetEventAfterOpening            equ     8

;
; Define injection object types.
;

EventObjectType                 equ     1
FileMappingObjectType           equ     2

;
; Define error codes.
;

RtlAddFunctionTableFailed       equ     1000
LoadLibraryWFailed              equ     1001
GetProcAddressFailed            equ     1002

;++
;
; LONG
; InjectionThunk(
;     _In_ PINJECTION_THUNK_CONTEXT Thunk
;     );
;
; Routine Description:
;
;   This routine is the initial entry point of our injection logic.  That is,
;   newly created remoted threads in a target process have their start address
;   set to a copy of this routine (that was prepared in a separate process and
;   then injected via WriteProcessMemory(), currently via CopyFunction()).
;
;   It is responsible for registering a runtime function for itself, such that
;   appropriate unwind data can be found by the kernel if an exception occurs
;   and the stack is being unwound.
;
;   It then enumerates any injection objects associated with the thunk and
;   initializes the object according to its type; i.e. named events are opened
;   via OpenEventW(), named file mappings are opened via OpenFileMappingW() and
;   then mapped at the requested address via MapViewOfFileExNuma().
;
;   If an APC structure has been indicated, the routine will invoke the function
;   with the first argument as the thunk context, and second and third arguments
;   as indicated by the APC struct's Argument2 and Argument3 fields.
;
;   It then loads a DLL referenced by the injection thunk via LoadLibraryExW(),
;   then resolves the address of a function pointer for the given public name,
;   also provided by the thunk.  This function is then invoked with the user's
;   context, thunk context and injection functions as the first, second and
;   third parameters, respectively.  The thunk then returns the value returned
;   by this routine -- this value will end up as the exit code for the thread.
;
;   The routine will immediately trap (int 3) on any errors or failed functions.
;
; Arguments:
;
;   Thunk (rcx) - Supplies a pointer to the injection thunk context.
;
; Return Value:
;
;   If successful, the return value will be the result of the routine from
;   the loaded DLL.  On error, the routine will immediately debugbreak, and
;   jump straight to the epilogue.  The return value is undefined in this
;   situation.
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

        movzx   r8, word ptr Thunk.Flags[r12]   ; Move flags into r8.
        test    r8, DebugBreakOnEntry           ; Test for debugbreak flag.
        jz      @F                              ; Flag isn't set.
        int     3                               ; Flag is set, so break.

;
; Register a runtime function for this currently executing piece of code.  This
; must be the first step taken by this routine in order to ensure the kernel has
; the appropriate unwind info in place if it needs to unwind this frame.
;
; N.B. We also use this opportunity to load the base address of the functions
;      struct (which is contained within the thunk) into r13.
;

@@:     lea     r13, Thunk.InjectionFunctions[r12]      ; Load Functions.
        mov     rcx, Thunk.FunctionTable[r12]           ; Load FunctionTable.
        movzx   rdx, word ptr Thunk.EntryCount[r12]     ; Load EntryCount.
        mov     r8, Thunk.BaseCodeAddress[r12]          ; Load BaseCodeAddress.
        call    Functions.RtlAddFunctionTable[r13]      ; Invoke function.
        test    rax, rax                                ; Check result.
        jz      short @F                                ; Function failed.
        jmp     short Inj10                             ; Function succeeded.

@@:     int     3
        mov     rax, RtlAddFunctionTableFailed          ; Load error code.
        jmp     Inj90                                   ; Jump to epilogue.

;
; Check to see if the thunk has any injection objects.  If it doesn't, proceed
; to the test to see if it has a module path and function name.  We clear r14
; up front such that we can test for non-NULL later on in the routine when we
; may need to persist the module handle and function pointer.
;

Inj10:  xor     r14, r14                                ; Zero ptr for inj obs.
        movzx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasInjectionObjects                 ; Any injection objects?
        jz      Inj60                                   ; No; check mod+func.

;
; The injection thunk features one or more injection objects.  An object can be
; an event or a file mapping.  We loop through the objects and process each one
; according to its type.
;
; N.B. Register use for this loop is as follows:
;
;   rbx - Supplies the address of the active INJECTION_OBJECT structure.  This
;       varies upon each loop invocation.
;
;   ebp - Supplies the value of the injection object type.
;
;   r13 - Supplies the base address of the INJECTION_FUNCTIONS structure, which
;       has already been initialized for us at this point.
;
;   r14 - Supplies the address of the INJECTION_OBJECTS structure.  This is
;       indempotent for the duration of the loop.
;
;   r15 - Initially supplies the number of injection objects indicated by
;       the INJECTION_OBJECTS.NumberOfObjects field.  After this is validated
;       as being greater than zero, it is multiplied by `size INJECTION_OBJECT`
;       such that it can be used to test for loop termination against rsi.
;
;   rdi - Supplies the base address of the INJECTION_OBJECT array.
;
;   rsi - Supplies the scaling index relative to rdi for the current injection
;       object such that `lea rbx, [rdi + rsi]` loads the appropriate address
;       of the INJECTION_OBJECT structure within the objects array.  This is
;       incremented by `size INJECTION_OBJECT` each loop iteration.
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

@@:     mov     r15d, Objects.NumberOfObjects[r14]      ; Load num of objects.
        test    r15, r15                                ; At least 1 object?
        jnz     short @F                                ; Yes, continue.
        int     3                                       ; No, break.

@@:     movzx   rax, word ptr Objects.SizeOfInjectionObjectInBytes[r14]
        cmp     rax, (sizeof INJECTION_OBJECT)          ; Compare object size.
        je      short @F                                ; Sizes are equal, cont.
        int     3                                       ; Sizes != equal, break.

;
; Invariant checks passed.  Convert r15 into the final scaling index offset
; such that it can be used to test if the loop has finished.
;

@@:     imul    r15, (sizeof INJECTION_OBJECT)

;
; Zero our loop counter (rsi) and load the base address of the injection objects
; array into rdi.
;

        xor     rsi, rsi                                  ; Zero loop counter.
        mov     rdi, Objects.FirstObject[r14]             ; Load first address.

;
; Top of loop.  Load next injection object (into rbx), load the type into ebp,
; determine whether we're dealing with an event or file mapping handle, and
; process accordingly.
;

        align   16

Inj20:  lea     rbx, [rdi + rsi]                        ; Load next inj. obj.
        mov     ebp, dword ptr Object.ObjectType[rbx]   ; Load object type.
        cmp     ebp, EventObjectType                    ; Is it an event?
        jne     Inj30                                   ; No; try file mapping.

;
; This is an event object type.  Load the arguments for OpenEventW into relevant
; registers and invoke the function.
;

        mov     ecx, Event.DesiredAccess[rbx]           ; Load desired access.
        mov     edx, Event.InheritHandle[rbx]           ; Load inherit handle.
        lea     rax, Event.ObjectName[rbx]              ; Load unicode string.
        mov     r8, Unicode.Buffer[rax]                 ; Load event name.
        call    Functions.OpenEventW[r13]               ; Call OpenEventW().
        test    rax, rax                                ; Check result.
        jnz     short @F                                ; Valid handle, proceed.
        int     3                                       ; Invalid handle, brk.
        call    Functions.GetLastError[r13]             ; Get last error.
        jmp     Inj90                                   ; Jump to end.

@@:     mov     Event.Handle[rbx], rax                  ; Save event handle.

;
; See if this event object has the WaitOnEventAfterOpening flag set, and if so,
; perform a wait on it.
;

        mov     ecx, dword ptr Event.ObjectFlags[rbx]   ; Load event flags.
        test    ecx, WaitOnEventAfterOpening            ; Wait on this event?
        jz      Inj25                                   ; No, check set event.

        mov     rcx, rax                                ; Load handle param 1.
        xor     rdx, rdx                                ; Clear rdx.
        not     rdx                                     ; Invert to all 1s.
        call    Functions.WaitForSingleObject[r13]      ; Invoke wait.
        test    rax, rax                                ; Wait result == 0?
        jz      @F                                      ; Yes, continue.
        int     3                                       ; No, abort.
        jmp     Inj90

;
; We successfully opened the event.  Re-load the flags into ecx and see if the
; DebugBreakAfterWaitSatisfied bit is set.  If so, break.  If not, continue the
; loop by jumping to the bottom (Inj50).
;

@@:     mov     ecx, dword ptr Event.ObjectFlags[rbx]   ; Re-load flags.
        test    ecx, DebugBreakAfterWaitSatisfied       ; Break now?
        jz      Inj50                                   ; No, jump to bottom.
        int     3                                       ; Yes, break.
        jmp     Inj50                                   ; Jump to bottom.

;
; See if this event object has the SetEventAfterOpening flag set, and if so,
; signal the handle via SetEvent().  Otherwise, jump to the bottom of the loop.
;

Inj25:  mov     ecx, dword ptr Event.ObjectFlags[rbx]   ; Re-load flags.
        test    ecx, SetEventAfterOpening               ; Call SetEvent()?
        jz      Inj50                                   ; No, jump to bottom.
        mov     rcx, rax                                ; Load handle param 1.
        call    Functions.SetEvent[r13]                 ; Invoke function.
        test    rax, rax                                ; Check result.
        jnz     Inj50                                   ; Success, continue.
        int     3                                       ; Failed, break.
        call    Functions.GetLastError[r13]             ; Get last error.
        jmp     Inj90                                   ; Jump to end.

;
; Determine if this is a file mapping type.
;
; N.B. This is currently the last object type we know about, so if the type
;      *isn't* a file mapping type, we debugbreak.
;

Inj30:  cmp     ebp, FileMappingObjectType              ; Is it a file mapping?
        je      short Inj40                             ; Yes, continue.
        int     3                                       ; No, break.
        jmp     Inj90                                   ; Return.

;
; This is a file mapping type.  We need to open the file mapping, then map it.
;

Inj40:  mov     ecx, FileMapping.DesiredAccess[rbx]     ; Load desired access.
        mov     edx, FileMapping.InheritHandle[rbx]     ; Load inherit handle.
        lea     rax, FileMapping.ObjectName[rbx]        ; Load unicode string.
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

@@:     mov     FileMapping.Handle[rbx], rax            ; Save handle.

;
; Load the preferred base address into the relevant stack offset now, outside
; of the main body that prepares the rest of the arguments.  This allows us to
; override the address if it couldn't be granted and retry the mapping.
;

        mov     rax, FileMapping.PreferredBaseAddress[rbx]  ; Load pref. BA.
        mov     Locals.PreferredBaseAddress[rsp], rax       ; ...as 6th arg.

Inj45:  mov     rcx, FileMapping.Handle[rbx]            ; Load handle.
        mov     edx, FileMapping.DesiredAccess[rbx]     ; Load desired access.

;
; N.B. The file offset requires a bit of LARGE_INTEGER juggling.
;

        mov     r9, FileMapping.FileOffset[rbx]         ; Load file offset.
        mov     r8, r9                                  ; Duplicate into r8.
        shr     r8, 32                                  ; Load high offset.

;
; Continue loading arguments; preferred address has already been done (see our
; comment above), so that leaves mapping size and preferred NUMA node.  All
; arguments herein are provided via the stack.
;

        mov     rax, FileMapping.MappingSize[rbx]       ; Load mapping size.
        mov     Locals.NumberOfBytesToMap[rsp], rax     ; ...as 5th arg.
        mov     eax, FileMapping.PreferredNumaNode[rbx] ; Load pref NN.
        mov     Locals.PreferredNumaNode[rsp], rax      ; ...as 7th arg.

;
; Call the function and test to see if we got a valid (non-NULL) address back.
;

        call    Functions.MapViewOfFileExNuma[r13]      ; Invoke.
        test    rax, rax                                ; Check return value.
        jz      short Inj47                             ; Invalid addr.

;
; The view was mapped successfully.  Save the base address back to the context,
; then jump to the bottom of the loop (Inj50).
;

        mov     FileMapping.BaseAddress[rbx], rax       ; Save base addr.
        jmp     short Inj50                             ; Jump to loop bottom.

;
; The mapping attempt failed.  If we provided a preferred base address, try the
; request again with a NULL base address, allowing the system to provide one for
; us.
;

Inj47:  mov     rax, Locals.PreferredBaseAddress[rsp]   ; Load pref. base.
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

@@:     xor     rax, rax                                ; Clear rax.
        mov     Locals.PreferredBaseAddress[rsp], rax   ; Zero address.
        jmp     short Inj45

;
; Bottom of the loop.  Update loop counter and determine if there are more
; objects to process.  If there are, continue back at the start (Inj20).  If
; not, fall through to the APC test.
;

Inj50:  add     rsi, (size INJECTION_OBJECT)            ; Update index.
        cmp     rsi, r15                                ; Compare to end.
        jl      Inj20                                   ; Items remaining.

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
        movzx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasModuleAndFunction                ; Has module+func?
        jz      Inj90                                   ; No; we're done.

;
; Prepare for a LoadLibraryW() call against the module path in the thunk.
;

        mov     rcx, Thunk.ModulePath.Buffer[r12]       ; Load ModulePath.
        xor     rdx, rdx                                ; Clear 2nd parameter.
        xor     r8, r8                                  ; Clear 3rd parameter.
        call    Functions.LoadLibraryExW[r13]           ; Call LoadLibraryExW().
        test    rax, rax                                ; Check Handle != NULL.
        je      short @F                                ; Handle is NULL.
        jmp     short Inj70                             ; Handle is valid.

@@:     mov     rax, LoadLibraryWFailed                 ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Module was loaded successfully.  The handle value lives in rax.  Copy it to
; rcx (first parameter for GetProcAddress) and rsi (so that it's available in
; a non-volatile register in order to save it later in the routine), then
; prepare the second parameter (rdx) with the function name we wish to resolve.
;

Inj70:  mov     rcx, rax                                ; Load as 1st param.
        mov     rsi, rax                                ; Also stash in rsi.
        mov     rdx, Thunk.FunctionName.Buffer[r12]     ; Load name as 2nd.
        call    Functions.GetProcAddress[r13]           ; Call GetProcAddress().
        test    rax, rax                                ; Check return value.
        jz      short @F                                ; Lookup failed.
        jmp     short Inj80                             ; Lookup succeeded.

@@:     mov     rax, GetProcAddressFailed               ; Load error code.
        jmp     short Inj90                             ; Jump to return.

;
; The function name was resolved successfully, and its address lives in rax.
; Save both the module handle and resulting function pointer to the relevant
; location in the injection objects structure (assuming it is not NULL).
;

Inj80:  test    r14, r14                                ; Inj. objects avail?
        jz      short @F                                ; NULL ptr, don't save.
        mov     Objects.ModuleHandle[r14], rsi          ; Save module handle.
        mov     Objects.FunctionPointer[r14], rax       ; Save func ptr.

;
; Load the offset of the user's data buffer and calculate the address based on
; the base address of the thunk.  Call the user's function with that address as
; the first parameter, the address of the injection objects structure (which
; still lives in r14) as the second parameter, and the address of the injection
; functions structure as the third parameter.
;

@@:     movzx   r8, word ptr Thunk.UserDataOffset[r12]  ; Load offset.
        mov     rcx, r12                                ; Load base address.
        add     rcx, r8                                 ; Add offset.
        mov     rdx, r14                                ; Load inj. objs ptr.
        mov     r8, r13                                 ; Include functions.
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

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
