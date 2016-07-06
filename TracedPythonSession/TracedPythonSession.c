#include "stdafx.h"

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

_Success_(return != 0)
BOOL
InitializeTracedPythonSession(
    _Out_   PPTRACED_PYTHON_SESSION SessionPointer,
    _Inopt_ PALLOCATION_ROUTINE AllocationRoutine,
    _Inopt_ PALLOCATION_CONTEXT AllocationContext,
    _Inopt_ PFREE_ROUTINE FreeRoutine,
    _Inopt_ PFREE_CONTEXT FreeContext
    )
/*--
Routine Description:

    This function initializes a TRACED_PYTHON_SESSION structure, using the
    memory allocation functions passed in as parameters.  If any errors occur
    during initialization after some memory has already been allocated, the
    free routine will be called to clean it up.

Arguments:

    SessionPointer - Supplies a pointer that will receive the address of the
        TRACED_PYTHON_SESSION structure allocated by AllocationRoutine.

    AllocationRoutine - Supplies a pointer to an allocation routine that is
        is used to allocate memory buffers for various structure fields in the
        TRACED_PYTHON_SESSION.  If AllocationRoutine is NULL, the default heap
        allocation and free (HeapAlloc, HeapFree) routines will be used.  The
        remaining three parameters must also be NULL otherwise FALSE will be
        returned indicating an error.

        If AllocationRoutine is not NULL, FreeRoutine must also be provided.

    AllocationContext - An optional context that is passed as the first
        parameter to the AllocationRoutine above.

    FreeRoutine - Supplies a pointer to the free routine used to free any memory
        allocated by the AllocationRoutine above.

    FreeContext - An optional context that is passed in as the first parameter
        to the FreeRoutine above.

Returns:

    TRUE on Success, FALSE if an error occurred.

--*/
{
    BOOL Success;
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
    Module = LoadLibraryA(Name);                     \
    if (!Module) {                                   \
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
    // Resolve the initial set of functions we require.
    //

#define RESOLVE(Module, Type, Name) do {                             \
    Session->Name = (Type)GetProcAddress(Module, #Name);             \
    if (!Name) {                                                     \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name); \
        goto Error;                                                  \
    }                                                                \
} while (0)

    RESOLVE(Shell32Module, PCOMMAND_LINE_TO_ARGV, CommandLineToArgvW);
    RESOLVE(Rtl, PINITIALIZE_RTL, InitializeRtl);
    RESOLVE(Tracer, PINITIALIZE_TRACE_STORES, InitializeTraceStores);
    RESOLVE(Tracer, PINITIALIZE_TRACE_CONTEXT, InitializeTraceContext);
    RESOLVE(Tracer, PINITIALIZE_TRACE_SESSION, InitializeTraceSession);

    //
    // The Python and PythonTracer modules need to be deferred until after
    // we've loaded the python.dll/python3.dll.
    //

    RESOLVE(Python, PINITIALIZE_PYTHON, InitializePython);

    RESOLVE(PythonTracer,                       \
            PINITIALIZE_PYTHON_TRACE_CONTEXT,   \
            InitializePythonTraceContext);

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:

    //
    // Update the user's pointer
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
