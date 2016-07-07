#include "TracedPythonSession.h"

//
// Forward decls of DLL exports.
//

TRACER_API INITIALIZE_TRACED_PYTHON_SESSION InitializeTracedPythonSession;
TRACER_API DESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

PVOID
HeapAllocationRoutine(
    _In_ HANDLE HeapHandle,
    _In_ ULONG ByteSize
    )
{
    return HeapAlloc(HeapHandle, 0, ByteSize);
}

VOID
HeapFreeRoutine(
    _In_ HANDLE HeapHandle,
    _In_ PVOID Buffer
    )
{
    HeapFree(HeapHandle, 0, Buffer);
}

_Use_decl_annotations_
BOOL
InitializeTracedPythonSession(
    PPTRACED_PYTHON_SESSION SessionPointer,
    PALLOCATION_ROUTINE AllocationRoutine,
    PALLOCATION_CONTEXT AllocationContext,
    PFREE_ROUTINE FreeRoutine,
    PFREE_CONTEXT FreeContext
    )
/*--
Routine Description:

    This function initializes a TRACED_PYTHON_SESSION structure, using the
    memory allocation functions passed in as parameters.  If any errors occur
    during initialization after some memory has already been allocated, the
    free routine will be called to clean it up.

Arguments:

    SessionPointer - Supplies a pointer that will receive the address of the
        TRACED_PYTHON_SESSION structure allocated by AllocationRoutine.  This
        pointer is immediately cleared (that is, '*SessionPointer = NULL;' is
        performed once SessionPointer is deemed non-NULL), and a value will
        only be set if initialization was successful.

    AllocationRoutine - Supplies a pointer to an allocation routine that is
        is used to allocate memory buffers for various structure fields in the
        TRACED_PYTHON_SESSION.  If AllocationRoutine is NULL, the default heap
        allocation and free (HeapAlloc, HeapFree) routines will be used.  The
        remaining three parameters must also be NULL otherwise FALSE will be
        returned indicating an error.  If AllocationRoutine is not NULL,
        FreeRoutine must also be provided.

    AllocationContext - An optional context that is passed as the first
        parameter to the AllocationRoutine above.  Must be NULL if
        AllocationRoutine is not provided.

    FreeRoutine - Supplies a pointer to the free routine used to free any memory
        allocated by the AllocationRoutine above.  Must be NULL if
        AllocationRoutine is not provided.

    FreeContext - An optional context that is passed in as the first parameter
        to the FreeRoutine above.  Must be NULL if AllocationRoutine is not
        provided.

Returns:

    TRUE on Success, FALSE if an error occurred.  *SessionPointer will be
    updated with the value of the newly created TRACED_PYTHON_SESSION structure.

--*/
{
    BOOL Success;
    ULONG RequiredSize;
    PRTL Rtl;
    PTRACED_PYTHON_SESSION Session;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(SessionPointer)) {

        return FALSE;

    } else {

        //
        // Clear the user's session pointer up-front.
        //

        *SessionPointer = NULL;
    }

    if (!ARGUMENT_PRESENT(AllocationRoutine)) {
        HANDLE HeapHandle;

        //
        // If AllocationRoutine hasn't been provided, ensure the other
        // parameters are also NULL.
        //

        BOOL AnyNonNullOtherParams = (
            ARGUMENT_PRESENT(AllocationContext) ||
            ARGUMENT_PRESENT(FreeRoutine)       ||
            ARGUMENT_PRESENT(FreeContext)
        );

        if (AnyNonNullOtherParams) {
            return FALSE;
        }

        //
        // Default to using the current process heap and the HeapAlloc/Free
        // routines if no AllocationRoutine has been provided.
        //

        HeapHandle = GetProcessHeap();
        if (!HeapHandle) {
            OutputDebugStringA("GetProcessHeap() failed.");
            return FALSE;
        }

        Session = (PTRACED_PYTHON_SESSION)(
            HeapAlloc(HeapHandle,
                      HEAP_ZERO_MEMORY,
                      sizeof(*Session)
            )
        );

        if (!Session) {
            OutputDebugStringA("HeapAlloc() failed.");
            return FALSE;
        }

        //
        // We successfully allocated space for the TRACED_PYTHON_SESSION struct,
        // so fill in our allocator/free information.
        //

        Session->AllocationRoutine = HeapAllocationRoutine;
        Session->AllocationContext = (PALLOCATION_CONTEXT)HeapHandle;

        Session->FreeRoutine = HeapFreeRoutine;
        Session->FreeContext = (PFREE_CONTEXT)HeapHandle;

    } else {

        //
        // A custom allocation routine was present, make sure a free routine
        // was also present.
        //

        if (!ARGUMENT_PRESENT(FreeRoutine)) {
            return FALSE;
        }

        //
        // Allocation and free routines were provided.  Use the allocation
        // routine to allocate space for the TRACED_PYTHON_SESSION struct.
        //

        Session = (PTRACED_PYTHON_SESSION)(
            AllocationRoutine(AllocationContext,
                              sizeof(*Session))
        );

        if (!Session) {
            OutputDebugStringA("User's AllocationRoutine() failed.");
            return FALSE;
        }

        SecureZeroMemory(Session, sizeof(*Session));

        //
        // Fill in the allocator/free fields.
        //

        Session->AllocationRoutine = AllocationRoutine;
        Session->AllocationContext = AllocationContext;

        Session->FreeRoutine = FreeRoutine;
        Session->FreeContext = FreeContext;

    }

    //
    // If we reach this point, our Session has been successfully created and
    // the allocation functions have been initialized.  From this point on,
    // any subsequent errors must `goto Error;` in order to ensure the rundown
    // logic executes.
    //

    Session->Size = sizeof(*Session);

    //
    // Load the system modules.
    //

#define LOAD(Module, Name) do {                      \
    Session->Module = LoadLibraryA(Name);            \
    if (!Session->Module) {                          \
        OutputDebugStringA("Failed to load " #Name); \
        goto Error;                                  \
    }                                                \
} while (0)

    LOAD(Kernel32Module, "kernel32");
    LOAD(Shell32Module, "shell32");
    LOAD(User32Module, "user32");
    LOAD(Advapi32Module, "advapi32");

    //
    // Load our modules.
    //

    LOAD(RtlModule, "Rtl");
    LOAD(HookModule, "HookModule");
    LOAD(TracerModule, "TracerModule");
    LOAD(PythonModule, "PythonModule");
    LOAD(PythonTracerModule, "PythonTracerModule");

    //
    // Resolve the functions.
    //

#define RESOLVE(Module, Type, Name) do {                                  \
    Session->Name = (Type)GetProcAddress(Session->Module, #Name);         \
    if (!Session->Name) {                                                 \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name "\n"); \
        goto Error;                                                       \
    }                                                                     \
} while (0)

    RESOLVE(Shell32Module, PCOMMAND_LINE_TO_ARGV, CommandLineToArgvW);

    RESOLVE(RtlModule, PINITIALIZE_RTL, InitializeRtl);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_STORES, InitializeTraceStores);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_CONTEXT, InitializeTraceContext);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_SESSION, InitializeTraceSession);

    RESOLVE(PythonModule, PINITIALIZE_PYTHON, InitializePython);

    RESOLVE(PythonTracerModule,
            PINITIALIZE_PYTHON_TRACE_CONTEXT,
            InitializePythonTraceContext);

    //
    // All of our modules modules use the same pattern for initialization
    // whereby the required structure size can be obtained by passing a NULL
    // for the destination pointer argument and PULONG as the size argument.
    // We leverage that here, using the local &RequiredSize variable and some
    // macro glue.
    //

#undef ALLOCATE
#define ALLOCATE(Target, Type) do {                                        \
    Session->Target = (Type)(                                              \
        Session->AllocationRoutine(                                        \
            Session->AllocationContext,                                    \
            RequiredSize                                                   \
        )                                                                  \
    );                                                                     \
    if (!Session->Target) {                                                \
        OutputDebugStringA("Allocation failed for " #Target " struct.\n"); \
        goto Error;                                                        \
    }                                                                      \
} while (0)


    //
    // Allocate and initialize Rtl.
    //

    RequiredSize = 0;
    Session->InitializeRtl(NULL, &RequiredSize);
    ALLOCATE(Rtl, PRTL);
    if (!Session->InitializeRtl(Session->Rtl, &RequiredSize)) {
        OutputDebugStringA("Session->InitializeRtl() failed.\n");
        goto Error;
    }

    //
    // Rtl was successfully allocated and initialized.  Take a local copy of
    // the pointer as it's nicer to work with than going through Session->Rtl.
    //

    Rtl = Session->Rtl;

    //
    // Allocate and initialize TraceSession.
    //

    RequiredSize = 0;
    Session->InitializeTraceSession(Rtl, NULL, &RequiredSize);
    ALLOCATE(TraceSession, PTRACE_SESSION);
    Success = Session->InitializeTraceSession(
        Rtl,
        Session->TraceSession,
        &RequiredSize
    );

    if (!Success) {
        OutputDebugStringA("Session->InitializeTraceSession() failed.\n");
        goto Error;
    }


    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    DestroyTracedPythonSession(&Session);

End:

    //
    // Update the user's pointer if we were successful.
    //

    if (Success) {
        *SessionPointer = Session;
    }

    return Success;
}

_Use_decl_annotations_
VOID
DestroyTracedPythonSession(
    PPTRACED_PYTHON_SESSION SessionPointer
    )
/*--
Routine Description:

    This function destroys a TRACED_PYTHON_SESSION structure that was created
    by InitializeTracedPythonSession().  This routine can be safely called
    regardless of the state of SessionPointer; that is, it will correctly handle
    a partially-initialized session that may have failed half-way through
    initialization.

Arguments:

    SessionPointer - Pointer to the address of the TRACED_PYTHON_SESSION struct
        to destroy.  This pointer is cleared by the routine.  (That is, once
        its contents is deemed to be valid, '*SessionPointer = NULL' will be
        performed.)  The rationale behind this logic is that you'd typically
        have a single, global pointer to an instance of this structure, and
        passing in the address of that pointer (versus the target contents)
        allows it to be cleared as part of destruction, ensuring it can no
        longer be dereferenced/accessed.

Returns:

    None.

--*/
{
    PTRACED_PYTHON_SESSION Session;

    //
    // Exit if SessionPointer is invalid.
    //

    if (!ARGUMENT_PRESENT(SessionPointer)) {
        return;
    }

    //
    // Dereference the pointer so we can just work with Session for the bulk
    // of the method (instead of *SessionPointer->).  (This should be wrapped
    // in a try/except ideally, however, we'd need to do some fidding to ensure
    // __C_specific_handler worked without relying on Session->Rtl, which may
    // not be available as *SessionPointer may fault.)
    //

    Session = *SessionPointer;

    //
    // Clear the originating pointer.
    //

    *SessionPointer = NULL;

    //
    // XXX todo: remaining deallocation logic.
    //

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
