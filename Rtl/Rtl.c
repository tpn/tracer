#include "stdafx.h"

static PRTL_COMPARE_STRING _RtlCompareString = NULL;

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

PVECTORED_EXCEPTION_HANDLER VectoredExceptionHandler = NULL;

INIT_ONCE InitOnceCSpecificHandler = INIT_ONCE_STATIC_INIT;

CONST static UNICODE_STRING ExtendedLengthVolumePrefixW = \
    RTL_CONSTANT_STRING(L"\\\\?\\");

CONST static STRING ExtendedLengthVolumePrefixA = \
    RTL_CONSTANT_STRING("\\\\?\\");

//
// As we don't link to the CRT, we don't get a __C_specific_handler entry,
// which the linker will complain about as soon as we use __try/__except.
// What we do is define a __C_specific_handler_impl pointer to the original
// function (that lives in ntdll), then implement our own function by the
// same name that calls the underlying impl pointer.  In order to do this
// we have to disable some compiler/linker warnings regarding mismatched
// stuff.
//

static P__C_SPECIFIC_HANDLER __C_specific_handler_impl = NULL;

#pragma warning(push)
#pragma warning(disable: 4028 4273)

EXCEPTION_DISPOSITION
__cdecl
__C_specific_handler(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
)
{
    return __C_specific_handler_impl(ExceptionRecord,
                                     Frame,
                                     Context,
                                     Dispatch);
}

#pragma warning(pop)

_Check_return_
PVOID
CopyToMemoryMappedMemory(
    PRTL Rtl,
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
    )
{

    //
    // Writing to memory mapped memory could raise a STATUS_IN_PAGE_ERROR
    // if there has been an issue with the backing store (such as memory
    // mapping a file on a network drive, then having the network fail).
    // Catch such exceptions and return NULL.
    //

    __try {

        return Rtl->RtlCopyMemory(Destination, Source, Size);

    } __except (GetExceptionCode() == STATUS_IN_PAGE_ERROR ?
                EXCEPTION_EXECUTE_HANDLER :
                EXCEPTION_CONTINUE_SEARCH)
    {
        return NULL;
    }
}

BOOL
TestExceptionHandler(VOID)
{
    //
    // Try assigning '1' to the memory address 0x10.
    //

    __try {

        (*(volatile *)(PCHAR)10) = '1';

    }
    __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH)
    {
        return TRUE;
    }

    //
    // This should be unreachable.
    //

    return FALSE;
}

_Use_decl_annotations_
BOOL
PrefaultPages(
    PVOID Address,
    ULONG NumberOfPages
    )
{
    ULONG Index;
    PCHAR Pointer = Address;

    __try {

        for (Index = 0; Index < NumberOfPages; Index++) {
            PrefaultPage(Pointer);
            Pointer += PAGE_SIZE;
        }

    } __except (GetExceptionCode() == STATUS_IN_PAGE_ERROR ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        return FALSE;
    }

    return TRUE;
}

BOOL
LoadShlwapiFunctions(
    _In_    HMODULE             ShlwapiModule,
    _In_    PSHLWAPI_FUNCTIONS  ShlwapiFunctions
    )
{
    if (!ARGUMENT_PRESENT(ShlwapiModule)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ShlwapiFunctions)) {
        return FALSE;
    }

#define TRY_RESOLVE_SHLWAPI_FUNCTION(Type, Name) ( \
    ShlwapiFunctions->##Name = (Type)(             \
        GetProcAddress(                            \
            ShlwapiModule,                         \
            #Name                                  \
        )                                          \
    )                                              \
)

#define RESOLVE_SHLWAPI_FUNCTION(Type, Name)                         \
    if (!TRY_RESOLVE_SHLWAPI_FUNCTION(Type, Name)) {                 \
        OutputDebugStringA("Failed to resolve Shlwapi!" #Name "\n"); \
        return FALSE;                                                \
    }


    RESOLVE_SHLWAPI_FUNCTION(PPATH_CANONICALIZEA, PathCanonicalizeA);

    return TRUE;

}

RTL_API
BOOL
LoadShlwapi(PRTL Rtl)
{
    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (Rtl->ShlwapiModule) {
        return TRUE;
    }

    if (!(Rtl->ShlwapiModule = LoadLibraryA("shlwapi"))) {
        return FALSE;
    }

    return LoadShlwapiFunctions(Rtl->ShlwapiModule, &Rtl->ShlwapiFunctions);
}

BOOL
CALLBACK
SetCSpecificHandlerCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
)
{
    PROC Handler;
    HMODULE Module;
    BOOL Success = FALSE;

    Module = (HMODULE)Parameter;

    if (Handler = GetProcAddress(Module, "__C_specific_handler")) {
        __C_specific_handler_impl = (P__C_SPECIFIC_HANDLER)Handler;
        Success = TRUE;
    }

    return Success;
}

BOOL
SetCSpecificHandler(_In_ HMODULE Module)
{
    BOOL Status;

    Status = InitOnceExecuteOnce(
        &InitOnceCSpecificHandler,
        SetCSpecificHandlerCallback,
        Module,
        NULL
    );

    return Status;
}

_Success_(return != 0)
BOOL
CALLBACK
GetSystemTimerFunctionCallback(
    _Inout_     PINIT_ONCE  InitOnce,
    _Inout_     PVOID       Parameter,
    _Inout_opt_ PVOID       *lpContext
)
{
    HMODULE Module;
    FARPROC Proc;
    static SYSTEM_TIMER_FUNCTION SystemTimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandle(TEXT("kernel32"));
    if (Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (
            (PGETSYSTEMTIMEPRECISEASFILETIME)Proc
        );
    } else {
        Module = LoadLibrary(TEXT("ntdll"));
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddress(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        SystemTimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPSYSTEM_TIMER_FUNCTION)lpContext) = &SystemTimerFunction;
    return TRUE;
}

PSYSTEM_TIMER_FUNCTION
GetSystemTimerFunction()
{
    BOOL Status;
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;

    Status = InitOnceExecuteOnce(
        &InitOnceSystemTimerFunction,
        GetSystemTimerFunctionCallback,
        NULL,
        (LPVOID *)&SystemTimerFunction
    );

    if (!Status) {
        return NULL;
    } else {
        return SystemTimerFunction;
    }
}

_Check_return_
BOOL
CallSystemTimer(
    _Out_       PFILETIME   SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
)
{
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction = NULL;

    if (ppSystemTimerFunction) {
        if (*ppSystemTimerFunction) {
            SystemTimerFunction = *ppSystemTimerFunction;
        } else {
            SystemTimerFunction = GetSystemTimerFunction();
            *ppSystemTimerFunction = SystemTimerFunction;
        }
    } else {
        SystemTimerFunction = GetSystemTimerFunction();
    }

    if (!SystemTimerFunction) {
        return FALSE;
    }

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(SystemTime);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        BOOL Success = SystemTimerFunction->NtQuerySystemTime(
            (PLARGE_INTEGER)SystemTime
        );
        if (!Success) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    return TRUE;
}


BOOL
FindCharsInUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               CharToFind,
    _Inout_  PRTL_BITMAP         Bitmap,
    _In_     BOOL                Reverse
)
{
    USHORT Index;
    USHORT NumberOfCharacters = String->Length >> 1;
    ULONG  Bit;
    WCHAR  Char;

    //
    // We use two loop implementations in order to avoid an additional
    // branch during the loop (after we find a character match).
    //

    if (Reverse) {
        for (Index = 0; Index < NumberOfCharacters; Index++) {
            Char = String->Buffer[Index];
            if (Char == CharToFind) {
                Bit = NumberOfCharacters - Index;
                FastSetBit(Bitmap, Bit);
            }
        }
    }
    else {
        for (Index = 0; Index < NumberOfCharacters; Index++) {
            Char = String->Buffer[Index];
            if (Char == CharToFind) {
                FastSetBit(Bitmap, Index);
            }
        }
    }

    return TRUE;
}

BOOL
FindCharsInString(
    _In_     PRTL         Rtl,
    _In_     PSTRING      String,
    _In_     CHAR         CharToFind,
    _Inout_  PRTL_BITMAP  Bitmap,
    _In_     BOOL         Reverse
)
{
    USHORT Index;
    USHORT NumberOfCharacters = String->Length;
    ULONG  Bit;
    CHAR Char;
    PRTL_SET_BIT RtlSetBit = Rtl->RtlSetBit;

    //
    // We use two loop implementations in order to avoid an additional
    // branch during the loop (after we find a character match).
    //

    if (Reverse) {
        for (Index = 0; Index < NumberOfCharacters; Index++) {
            Char = String->Buffer[Index];
            if (Char == CharToFind) {
                Bit = NumberOfCharacters - Index;
                FastSetBit(Bitmap, Bit);
            }
        }
    }
    else {
        for (Index = 0; Index < NumberOfCharacters; Index++) {
            Char = String->Buffer[Index];
            if (Char == CharToFind) {
                FastSetBit(Bitmap, Index);
            }
        }
    }

    return TRUE;
}


_Check_return_
BOOL
CreateBitmapIndexForUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               Char,
    _Inout_  PHANDLE             HeapHandlePointer,
    _Inout_  PPRTL_BITMAP        BitmapPointer,
    _In_     BOOL                Reverse,
    _In_opt_ PFIND_CHARS_IN_UNICODE_STRING FindCharsFunction
    )

/*++

Routine Description:

    This is a helper function that simplifies creating bitmap indexes for
    UNICODE_STRING structures.  The routine will use the user-supplied bitmap
    if it is big enough (govered by the SizeOfBitMap field).  If it isn't,
    a new buffer will be allocated.  If no bitmap is provided at all, the
    entire structure plus the bitmap buffer space will be allocated from the
    heap.

    Typically, callers would provide their own pointer to a stack-allocated
    RTL_BITMAP struct if they only need the bitmap for the scope of their
    function call.  For longer-lived bitmaps, a pointer to a NULL pointer
    would be provided, indicating that the entire structure should be heap
    allocated.

    Caller is responsible for freeing either the entire RTL_BITMAP or the
    underlying Bitmap->Buffer if a heap allocation took place.

Arguments:

    Rtl - Supplies the pointer to the RTL structure (mandatory).

    String - Supplies the UNICODE_STRING structure to create the bitmap
        index for (mandatory).

    Char - Supplies the character to create the bitmap index for.  This is
        passed directly to FindCharsInUnicodeString().

    HeapHandlePointer - Supplies a pointer to the underlying heap handle
        to use for allocation.  If a heap allocation is required and this
        pointer points to a NULL value, the default process heap handle
        will be used (obtained via GetProcessHeap()), and the pointed-to
        location will be updated with the handle value.  (The caller will
        need this in order to perform the subsequent HeapFree() of the
        relevant structure.)

    BitmapPointer - Supplies a pointer to a PRTL_BITMAP structure.  If the
        pointed-to location is NULL, additional space for the RTL_BITMAP
        structure will be allocated on top of the bitmap buffer space, and
        the pointed-to location will be updated with the resulting address.
        If the pointed-to location is non-NULL and the SizeOfBitMap field
        is greater than or equal to the required bitmap size, the bitmap
        will be used directly and no heap allocations will take place.
        The SizeOfBitMap field in this example will be altered to match the
        required size.  If a heap allocation takes place, user is responsible
        for cleaning it up (i.e. either freeing the entire PRTL_BITMAP struct
        returned, or just the Bitmap->Buffer, depending on usage).

    Reverse - Supplies a boolean flag indicating the bitmap index should be
        created in reverse.  This is passed to FindCharsInUnicodeString().

Return Value:

    TRUE on success, FALSE on error.

Examples:

    A stack-allocated bitmap structure and buffer:

        CHAR StackBuffer[_MAX_FNAME >> 3];
        RTL_BITMAP Bitmap = { _MAX_FNAME, (PULONG)&StackBuffer };
        PRTL_BITMAP BitmapPointer = &Bitmap;
        HANDLE HeapHandle;

        BOOL Success = CreateBitmapIndexForUnicodeString(Rtl,
                                                         String,
                                                         L'\\',
                                                         &HeapHandle,
                                                         &BitmapPointer,
                                                         FALSE);

        ...

Error:
    if (HeapHandle) {

        //
        // HeapHandle will be set if a new bitmap had to be allocated
        // because our stack-allocated one was too small.
        //

        if ((ULONG_PTR)Bitmap.Buffer == (ULONG_PTR)BitmapPointer->Buffer) {

            //
            // This should never happen.  If HeapHandle is set, the buffers
            // should differ.
            //

            __debugbreak();
        }

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

--*/

{
    USHORT NumberOfCharacters;
    USHORT AlignedNumberOfCharacters;
    SIZE_T BitmapBufferSizeInBytes;
    BOOL UpdateBitmapPointer;
    BOOL UpdateHeapHandlePointer;
    BOOL Success;
    HANDLE HeapHandle = NULL;
    PRTL_BITMAP Bitmap = NULL;
    PFIND_CHARS_IN_UNICODE_STRING FindChars;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HeapHandlePointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BitmapPointer)) {
        return FALSE;
    }

    //
    // Resolve the number of characters, then make sure it's aligned to the
    // platform's pointer size.
    //

    NumberOfCharacters = String->Length >> 1;
    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfCharacters
        )
    );

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;


    //
    // If *BitmapPointer is non-NULL, see if it's big enough to hold the bitmap.
    //

    if (*BitmapPointer) {

        if ((*BitmapPointer)->SizeOfBitMap >= AlignedNumberOfCharacters) {

            //
            // The user-provided bitmap is big enough.  Jump straight to the
            // starting point.
            //

            Bitmap = *BitmapPointer;
            UpdateHeapHandlePointer = FALSE;
            UpdateBitmapPointer = FALSE;

            goto Start;
        }
    }

    if (!*HeapHandlePointer) {

        //
        // If the pointer to the heap handle to use is NULL, default to using
        // the default process heap via GetProcessHeap().  Note that we also
        // assign back to the user's pointer, such that they get a copy of the
        // heap handle that was used for allocation.
        //

        HeapHandle = GetProcessHeap();

        if (!HeapHandle) {
            return FALSE;
        }

        UpdateHeapHandlePointer = TRUE;
    }
    else {

        //
        // Use the handle the user provided.
        //

        HeapHandle = *HeapHandlePointer;
        UpdateHeapHandlePointer = FALSE;
    }

    if (!*BitmapPointer) {

        //
        // If the pointer to the PRTL_BITMAP structure is NULL, the caller
        // wants us to allocate the space for the RTL_BITMAP structure as
        // well.
        //

        SIZE_T AllocationSize = BitmapBufferSizeInBytes + sizeof(RTL_BITMAP);

        Bitmap = (PRTL_BITMAP)HeapAlloc(HeapHandle, 0, AllocationSize);

        if (!Bitmap) {
            return FALSE;
        }

        //
        // Point the bitmap buffer to the end of the RTL_BITMAP struct.
        //

        Bitmap->Buffer = (PULONG)(
            RtlOffsetToPointer(
                Bitmap,
                sizeof(RTL_BITMAP)
            )
        );

        //
        // Make a note that we need to update the user's bitmap pointer.
        //

        UpdateBitmapPointer = TRUE;

    }
    else {

        //
        // The user has provided an existing PRTL_BITMAP structure, so we
        // only need to allocate memory for the actual underlying bitmap
        // buffer.
        //

        Bitmap = *BitmapPointer;

        Bitmap->Buffer = (PULONG)(
            HeapAlloc(
                HeapHandle,
                0,
                BitmapBufferSizeInBytes
            )
        );

        if (!Bitmap->Buffer) {
            return FALSE;
        }

        //
        // Make a note that we do *not* need to update the user's bitmap
        // pointer.
        //

        UpdateBitmapPointer = FALSE;

    }

Start:

    //
    // There will be one bit per character.
    //

    Bitmap->SizeOfBitMap = AlignedNumberOfCharacters;

    if (!Bitmap->Buffer) {
        __debugbreak();
    }

    //
    // Clear all bits in the bitmap.
    //

    Rtl->RtlClearAllBits(Bitmap);


    //
    // Fill in the bitmap index.
    //

    FindChars = FindCharsFunction;
    if (!FindChars) {
        FindChars = FindCharsInUnicodeString;
    }

    Success = FindChars(Rtl, String, Char, Bitmap, Reverse);

    if (!Success && HeapHandle) {

        //
        // HeapHandle will only be set if we had to do heap allocations.
        //

        if (UpdateBitmapPointer) {

            //
            // Free the entire structure.
            //

            HeapFree(HeapHandle, 0, Bitmap);

        }
        else {

            //
            // Free just the buffer.
            //

            HeapFree(HeapHandle, 0, Bitmap->Buffer);

        }

        return FALSE;
    }

    //
    // Update caller's pointers if applicable, then return successfully.
    //

    if (UpdateBitmapPointer) {
        *BitmapPointer = Bitmap;
    }

    if (UpdateHeapHandlePointer) {
        *HeapHandlePointer = HeapHandle;
    }

    return TRUE;
}

_Check_return_
BOOL
CreateBitmapIndexForString(
    _In_     PRTL           Rtl,
    _In_     PSTRING        String,
    _In_     CHAR           Char,
    _Inout_  PHANDLE        HeapHandlePointer,
    _Inout_  PPRTL_BITMAP   BitmapPointer,
    _In_     BOOL           Reverse,
    _In_opt_ PFIND_CHARS_IN_STRING FindCharsFunction
    )

/*++

Routine Description:

    This is a helper function that simplifies creating bitmap indexes for
    STRING structures.  The routine will use the user-supplied bitmap
    if it is big enough (govered by the SizeOfBitMap field).  If it isn't,
    a new buffer will be allocated.  If no bitmap is provided at all, the
    entire structure plus the bitmap buffer space will be allocated from the
    heap.

    Typically, callers would provide their own pointer to a stack-allocated
    RTL_BITMAP struct if they only need the bitmap for the scope of their
    function call.  For longer-lived bitmaps, a pointer to a NULL pointer
    would be provided, indicating that the entire structure should be heap
    allocated.

    Caller is responsible for freeing either the entire RTL_BITMAP or the
    underlying Bitmap->Buffer if a heap allocation took place.

Arguments:

    Rtl - Supplies the pointer to the RTL structure (mandatory).

    String - Supplies the STRING structure to create the bitmap index for.

    Char - Supplies the character to create the bitmap index for.  This is
        passed directly to FindCharsInString().

    HeapHandlePointer - Supplies a pointer to the underlying heap handle
        to use for allocation.  If a heap allocation is required and this
        pointer points to a NULL value, the default process heap handle
        will be used (obtained via GetProcessHeap()), and the pointed-to
        location will be updated with the handle value.  (The caller will
        need this in order to perform the subsequent HeapFree() of the
        relevant structure.)

    BitmapPointer - Supplies a pointer to a PRTL_BITMAP structure.  If the
        pointed-to location is NULL, additional space for the RTL_BITMAP
        structure will be allocated on top of the bitmap buffer space, and
        the pointed-to location will be updated with the resulting address.
        If the pointed-to location is non-NULL and the SizeOfBitMap field
        is greater than or equal to the required bitmap size, the bitmap
        will be used directly and no heap allocations will take place.
        The SizeOfBitMap field in this example will be altered to match the
        required size.  If a heap allocation takes place, user is responsible
        for cleaning it up (i.e. either freeing the entire PRTL_BITMAP struct
        returned, or just the Bitmap->Buffer, depending on usage).

    Reverse - Supplies a boolean flag indicating the bitmap index should be
        created in reverse.  This is passed to FindCharsInUnicodeString().

Return Value:

    TRUE on success, FALSE on error.

Examples:

    A stack-allocated bitmap structure and buffer:

        CHAR StackBuffer[_MAX_FNAME >> 3];
        RTL_BITMAP Bitmap = { _MAX_FNAME, (PULONG)&StackBuffer };
        PRTL_BITMAP BitmapPointer = &Bitmap;
        HANDLE HeapHandle;

        BOOL Success = CreateBitmapIndexForString(Rtl,
                                                  String,
                                                  '\\',
                                                  &HeapHandle,
                                                  &BitmapPointer,
                                                  FALSE);

        ...

Error:
    if (HeapHandle) {

        //
        // HeapHandle will be set if a new bitmap had to be allocated
        // because our stack-allocated one was too small.
        //

        if ((ULONG_PTR)Bitmap.Buffer == (ULONG_PTR)BitmapPointer->Buffer) {

            //
            // This should never happen.  If HeapHandle is set, the buffers
            // should differ.
            //

            __debugbreak();
        }

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

--*/

{
    USHORT NumberOfCharacters;
    USHORT AlignedNumberOfCharacters;
    SIZE_T BitmapBufferSizeInBytes;
    BOOL UpdateBitmapPointer;
    BOOL UpdateHeapHandlePointer;
    BOOL Success;
    HANDLE HeapHandle = NULL;
    PRTL_BITMAP Bitmap = NULL;
    PFIND_CHARS_IN_STRING FindChars;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HeapHandlePointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BitmapPointer)) {
        return FALSE;
    }

    //
    // Resolve the number of characters, then make sure it's aligned to the
    // platform's pointer size.
    //

    NumberOfCharacters = String->Length;
    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfCharacters
        )
    );

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    //
    // If *BitmapPointer is non-NULL, see if it's big enough to hold the bitmap.
    //

    if (*BitmapPointer) {

        if ((*BitmapPointer)->SizeOfBitMap >= AlignedNumberOfCharacters) {

            //
            // The user-provided bitmap is big enough.  Jump straight to the
            // starting point.
            //

            Bitmap = *BitmapPointer;
            UpdateHeapHandlePointer = FALSE;
            UpdateBitmapPointer = FALSE;

            goto Start;
        }
    }

    if (!*HeapHandlePointer) {

        //
        // If the pointer to the heap handle to use is NULL, default to using
        // the default process heap via GetProcessHeap().  Note that we also
        // assign back to the user's pointer, such that they get a copy of the
        // heap handle that was used for allocation.
        //

        HeapHandle = GetProcessHeap();

        if (!HeapHandle) {
            return FALSE;
        }

        UpdateHeapHandlePointer = TRUE;
    }
    else {

        //
        // Use the handle the user provided.
        //

        HeapHandle = *HeapHandlePointer;
        UpdateHeapHandlePointer = FALSE;
    }

    if (!*BitmapPointer) {

        //
        // If the pointer to the PRTL_BITMAP structure is NULL, the caller
        // wants us to allocate the space for the RTL_BITMAP structure as
        // well.
        //

        SIZE_T AllocationSize = BitmapBufferSizeInBytes + sizeof(RTL_BITMAP);

        Bitmap = (PRTL_BITMAP)HeapAlloc(HeapHandle, 0, AllocationSize);

        if (!Bitmap) {
            return FALSE;
        }

        //
        // Point the bitmap buffer to the end of the RTL_BITMAP struct.
        //

        Bitmap->Buffer = (PULONG)(
            RtlOffsetToPointer(
                Bitmap,
                sizeof(RTL_BITMAP)
            )
        );

        //
        // Make a note that we need to update the user's bitmap pointer.
        //

        UpdateBitmapPointer = TRUE;

    }
    else {

        //
        // The user has provided an existing PRTL_BITMAP structure, so we
        // only need to allocate memory for the actual underlying bitmap
        // buffer.
        //

        Bitmap = *BitmapPointer;

        Bitmap->Buffer = (PULONG)(
            HeapAlloc(
                HeapHandle,
                0,
                BitmapBufferSizeInBytes
            )
        );

        if (!Bitmap->Buffer) {
            return FALSE;
        }

        //
        // Make a note that we do *not* need to update the user's bitmap
        // pointer.
        //

        UpdateBitmapPointer = FALSE;

    }

Start:

    //
    // There will be one bit per character.
    //

    Bitmap->SizeOfBitMap = AlignedNumberOfCharacters;

    if (!Bitmap->Buffer) {
        __debugbreak();
    }

    //
    // Clear all bits in the bitmap.
    //

    Rtl->RtlClearAllBits(Bitmap);


    //
    // Fill in the bitmap index.
    //

    FindChars = FindCharsFunction;
    if (!FindChars) {
        FindChars = FindCharsInString;
    }

    Success = FindChars(Rtl, String, Char, Bitmap, Reverse);

    if (!Success && HeapHandle) {

        //
        // HeapHandle will only be set if we had to do heap allocations.
        //

        if (UpdateBitmapPointer) {

            //
            // Free the entire structure.
            //

            HeapFree(HeapHandle, 0, Bitmap);

        }
        else {

            //
            // Free just the buffer.
            //

            HeapFree(HeapHandle, 0, Bitmap->Buffer);

        }

        return FALSE;
    }

    //
    // Update caller's pointers if applicable, then return successfully.
    //

    if (UpdateBitmapPointer) {
        *BitmapPointer = Bitmap;
    }

    if (UpdateHeapHandlePointer) {
        *HeapHandlePointer = HeapHandle;
    }

    return TRUE;
}


_Check_return_
BOOL
FilesExistW(
    _In_      PRTL             Rtl,
    _In_      PUNICODE_STRING  Directory,
    _In_      USHORT           NumberOfFilenames,
    _In_      PPUNICODE_STRING Filenames,
    _Out_     PBOOL            Exists,
    _Out_opt_ PUSHORT          WhichIndex,
    _Out_opt_ PPUNICODE_STRING WhichFilename
    )
{
    USHORT Index;
    PWCHAR HeapBuffer;
    ULONG CombinedSizeInBytes;
    USHORT DirectoryLength;
    USHORT MaxFilenameLength = 0;
    UNICODE_STRING Path;
    PUNICODE_STRING Filename;
    DWORD Attributes;
    BOOL Success = FALSE;
    HANDLE HeapHandle = NULL;
    WCHAR StackBuffer[_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (NumberOfFilenames == 0) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Filenames) || !ARGUMENT_PRESENT(Filenames[0])) {
        return FALSE;
    }

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        BOOL SanityCheck;

        Filename = Filenames[Index];

        //
        // Quick sanity check that the Filename pointer in the array
        // entry is non-NULL, the Length member is greater than 0,
        // and the buffer has a non-NULL value.
        //

        SanityCheck = (
            Filename &&
            Filename->Length > 0 &&
            Filename->Buffer != NULL
        );

        if (!SanityCheck) {
            __debugbreak();
        }

        //
        // Update our local maximum filename length variable if applicable.
        //

        if (Filename->Length > MaxFilenameLength) {
            MaxFilenameLength = Filename->Length;
        }
    }


    //
    // See if the combined size of the extended volume prefix ("\\?\"),
    // directory, joining backslash, maximum filename length and terminating
    // NUL is less than or equal to _MAX_PATH.  If it is, we can use the
    // stack-allocated Path buffer above; if not, allocate a new buffer from
    // the default heap.
    //

    CombinedSizeInBytes = (
        ExtendedLengthVolumePrefixW.Length  +
        Directory->Length                   +
        sizeof(WCHAR)                       + // joining backslash
        MaxFilenameLength                   +
        sizeof(WCHAR)                         // terminating NUL
    );

    //
    // Point Path->Buffer at the stack or heap buffer depending on the
    // combined size.
    //

    if (CombinedSizeInBytes <= _MAX_PATH) {

        //
        // We can use our stack buffer.
        //

        Path.Buffer = &StackBuffer[0];

    } else if (CombinedSizeInBytes > MAX_USTRING) {

        goto Error;

    } else {

        //
        // The combined size exceeds _MAX_PATH so allocate the required memory
        // from the heap.
        //

        HeapHandle = GetProcessHeap();
        if (!HeapHandle) {
            return FALSE;
        }

        HeapBuffer = (PWCHAR)HeapAlloc(HeapHandle, 0, CombinedSizeInBytes);

        if (!HeapBuffer) {
            return FALSE;
        }

        Path.Buffer = HeapBuffer;

    }

    Path.Length = 0;
    Path.MaximumLength = (USHORT)CombinedSizeInBytes;

    //
    // Copy the volume prefix, then append the directory and joining backslash.
    //

    Rtl->RtlCopyUnicodeString(&Path, &ExtendedLengthVolumePrefixW);

    if (FAILED(Rtl->RtlAppendUnicodeStringToString(&Path, Directory)) ||
        !AppendUnicodeCharToUnicodeString(&Path, L'\\')) {

        goto Error;
    }

    //
    // Make a note of the length at this point as we'll need to revert to it
    // after each unsuccessful file test.
    //

    DirectoryLength = Path.Length;

    //
    // Enumerate over the array of filenames and look for the first one that
    // exists.
    //

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        Filename = Filenames[Index];

        //
        // We've already validated our lengths, so these should never fail.
        //

        if (FAILED(Rtl->RtlAppendUnicodeStringToString(&Path, Filename)) ||
            !AppendUnicodeCharToUnicodeString(&Path, L'\0')) {

            goto Error;
        }

        //
        // We successfully constructed the path, so we can now look up the file
        // attributes.
        //

        Attributes = GetFileAttributesW(Path.Buffer);

        if (Attributes == INVALID_FILE_ATTRIBUTES ||
            (Attributes & FILE_ATTRIBUTE_DIRECTORY)) {

            //
            // File doesn't exist or is a directory.  Reset the path length
            // and continue.
            //

            Path.Length = DirectoryLength;

            continue;
        }

        //
        // Success!  File exists and *isn't* a directory.  We're done.
        //

        Success = TRUE;
        break;
    }

    if (!Success) {

        *Exists = FALSE;

        //
        // The files didn't exist, but no error occurred, so we return success.
        //

        Success = TRUE;

    } else {

        *Exists = TRUE;

        //
        // Update the user's pointers if applicable.
        //

        if (ARGUMENT_PRESENT(WhichIndex)) {
            *WhichIndex = Index;
        }

        if (ARGUMENT_PRESENT(WhichFilename)) {
            *WhichFilename = Filename;
        }

    }

    //
    // Intentional follow-on to "Error"; Success code will be set
    // appropriately by this stage.
    //

Error:
    if (HeapHandle) {
        HeapFree(HeapHandle, 0, Path.Buffer);
    }

    return Success;
}

_Check_return_
BOOL
FilesExistExW(
    _In_      PRTL             Rtl,
    _In_      PUNICODE_STRING  Directory,
    _In_      USHORT           NumberOfFilenames,
    _In_      PPUNICODE_STRING Filenames,
    _Out_     PBOOL            Exists,
    _Out_opt_ PUSHORT          WhichIndex,
    _Out_opt_ PPUNICODE_STRING WhichFilename,
    _In_      PALLOCATOR       Allocator
    )
{
    USHORT Index;
    PWCHAR HeapBuffer;
    ULONG CombinedSizeInBytes;
    USHORT DirectoryLength;
    USHORT MaxFilenameLength = 0;
    UNICODE_STRING Path;
    PUNICODE_STRING Filename;
    DWORD Attributes;
    BOOL Success = FALSE;
    HANDLE HeapHandle = NULL;
    WCHAR StackBuffer[_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (NumberOfFilenames == 0) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Filenames) || !ARGUMENT_PRESENT(Filenames[0])) {
        return FALSE;
    }

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        BOOL SanityCheck;

        Filename = Filenames[Index];

        //
        // Quick sanity check that the Filename pointer in the array
        // entry is non-NULL, the Length member is greater than 0,
        // and the buffer has a non-NULL value.
        //

        SanityCheck = (
            Filename &&
            Filename->Length > 0 &&
            Filename->Buffer != NULL
        );

        if (!SanityCheck) {
            __debugbreak();
        }

        //
        // Update our local maximum filename length variable if applicable.
        //

        if (Filename->Length > MaxFilenameLength) {
            MaxFilenameLength = Filename->Length;
        }
    }


    //
    // See if the combined size of the extended volume prefix ("\\?\"),
    // directory, joining backslash, maximum filename length and terminating
    // NUL is less than or equal to _MAX_PATH.  If it is, we can use the
    // stack-allocated Path buffer above; if not, allocate a new buffer from
    // the default heap.
    //

    CombinedSizeInBytes = (
        ExtendedLengthVolumePrefixW.Length  +
        Directory->Length                   +
        sizeof(WCHAR)                       + // joining backslash
        MaxFilenameLength                   +
        sizeof(WCHAR)                         // terminating NUL
    );

    //
    // Point Path->Buffer at the stack or heap buffer depending on the
    // combined size.
    //

    if (CombinedSizeInBytes <= _MAX_PATH) {

        //
        // We can use our stack buffer.
        //

        Path.Buffer = &StackBuffer[0];

    } else if (CombinedSizeInBytes > MAX_USTRING) {

        goto Error;

    } else {

        //
        // The combined size exceeds _MAX_PATH so allocate the required memory.
        //

        HeapBuffer = (PWCHAR)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                CombinedSizeInBytes
            )
        );

        if (!HeapBuffer) {
            return FALSE;
        }

        Path.Buffer = HeapBuffer;

    }

    Path.Length = 0;
    Path.MaximumLength = (USHORT)CombinedSizeInBytes;

    //
    // Copy the volume prefix, then append the directory and joining backslash.
    //

    Rtl->RtlCopyUnicodeString(&Path, &ExtendedLengthVolumePrefixW);

    if (FAILED(Rtl->RtlAppendUnicodeStringToString(&Path, Directory)) ||
        !AppendUnicodeCharToUnicodeString(&Path, L'\\')) {

        goto Error;
    }

    //
    // Make a note of the length at this point as we'll need to revert to it
    // after each unsuccessful file test.
    //

    DirectoryLength = Path.Length;

    //
    // Enumerate over the array of filenames and look for the first one that
    // exists.
    //

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        Filename = Filenames[Index];

        //
        // We've already validated our lengths, so these should never fail.
        //

        if (FAILED(Rtl->RtlAppendUnicodeStringToString(&Path, Filename)) ||
            !AppendUnicodeCharToUnicodeString(&Path, L'\0')) {

            goto Error;
        }

        //
        // We successfully constructed the path, so we can now look up the file
        // attributes.
        //

        Attributes = GetFileAttributesW(Path.Buffer);

        if (Attributes == INVALID_FILE_ATTRIBUTES ||
            (Attributes & FILE_ATTRIBUTE_DIRECTORY)) {

            //
            // File doesn't exist or is a directory.  Reset the path length
            // and continue.
            //

            Path.Length = DirectoryLength;

            continue;
        }

        //
        // Success!  File exists and *isn't* a directory.  We're done.
        //

        Success = TRUE;
        break;
    }

    if (!Success) {

        *Exists = FALSE;

        //
        // The files didn't exist, but no error occurred, so we return success.
        //

        Success = TRUE;

    } else {

        *Exists = TRUE;

        //
        // Update the user's pointers if applicable.
        //

        if (ARGUMENT_PRESENT(WhichIndex)) {
            *WhichIndex = Index;
        }

        if (ARGUMENT_PRESENT(WhichFilename)) {
            *WhichFilename = Filename;
        }

    }

    //
    // Intentional follow-on to "Error"; Success code will be set
    // appropriately by this stage.
    //

Error:
    if (HeapBuffer) {
        Allocator->Free(Allocator->Context, HeapBuffer);
    }

    return Success;
}

_Success_(return != 0)
_Check_return_
BOOL
FilesExistA(
    _In_      PRTL      Rtl,
    _In_      PSTRING   Directory,
    _In_      USHORT    NumberOfFilenames,
    _In_      PPSTRING  Filenames,
    _Out_     PBOOL     Exists,
    _Out_opt_ PUSHORT   WhichIndex,
    _Out_opt_ PPSTRING  WhichFilename
    )
{
    USHORT Index;
    PCHAR HeapBuffer;
    ULONG CombinedSizeInBytes;
    USHORT DirectoryLength;
    USHORT MaxFilenameLength = 0;
    STRING Path;
    PSTRING Filename;
    DWORD Attributes;
    BOOL Success = FALSE;
    HANDLE HeapHandle = NULL;
    CHAR StackBuffer[_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (NumberOfFilenames == 0) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Filenames) || !ARGUMENT_PRESENT(Filenames[0])) {
        return FALSE;
    }

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        BOOL SanityCheck;

        Filename = Filenames[Index];

        //
        // Quick sanity check that the Filename pointer in the array
        // entry is non-NULL, the Length member is greater than 0,
        // and the buffer has a non-NULL value.
        //

        SanityCheck = (
            Filename &&
            Filename->Length > 0 &&
            Filename->Buffer != NULL
        );

        if (!SanityCheck) {
            __debugbreak();
        }

        if (Filename->Length > MaxFilenameLength) {
            MaxFilenameLength = Filename->Length;
        }
    }


    //
    // See if the combined size of the extended volume prefix ("\\?\"),
    // directory, joining backslash, maximum filename length and terminating
    // NUL is less than or equal to _MAX_PATH.  If it is, we can use the
    // stack-allocated Path buffer above; if not, allocate a new buffer from
    // the default heap.
    //

    CombinedSizeInBytes = (
        ExtendedLengthVolumePrefixA.Length +
        Directory->Length                  +
        sizeof(CHAR)                       + // joining backslash
        MaxFilenameLength                  +
        sizeof(CHAR)                         // terminating NUL
    );

    //
    // Point Path->Buffer at the stack or heap buffer depending on the
    // combined size.
    //

    if (CombinedSizeInBytes <= _MAX_PATH) {

        //
        // We can use our stack buffer.
        //

        Path.Buffer = &StackBuffer[0];

    } else if (CombinedSizeInBytes > MAX_STRING) {

        goto Error;

    } else {

        //
        // The combined size exceeds _MAX_PATH so allocate the required memory
        // from the heap.
        //

        HeapHandle = GetProcessHeap();
        if (!HeapHandle) {
            return FALSE;
        }

        HeapBuffer = (PCHAR)HeapAlloc(HeapHandle, 0, CombinedSizeInBytes);

        if (!HeapBuffer) {
            return FALSE;
        }

        Path.Buffer = HeapBuffer;

    }

    Path.Length = 0;
    Path.MaximumLength = (USHORT)CombinedSizeInBytes;

    //
    // Copy the volume prefix, then append the directory and joining backslash.
    //

    if (!CopyString(&Path, &ExtendedLengthVolumePrefixA)) {
        goto Error;
    }

    if (!AppendStringAndCharToString(&Path, Directory, '\\')) {
        goto Error;
    }

    //
    // Make a note of the length at this point as we'll need to revert to it
    // after each unsuccessful file test.
    //

    DirectoryLength = Path.Length;

    //
    // Enumerate over the array of filenames and look for the first one that
    // exists.
    //

    for (Index = 0; Index < NumberOfFilenames; Index++) {
        Filename = Filenames[Index];

        //
        // We've already validated our lengths, so these should never fail.
        //

        if (!AppendStringAndCharToString(&Path, Filename, '\0')) {
            goto Error;
        }

        //
        // We successfully constructed the path, so we can now look up the file
        // attributes.
        //

        Attributes = GetFileAttributesA(Path.Buffer);

        if (Attributes == INVALID_FILE_ATTRIBUTES ||
            (Attributes & FILE_ATTRIBUTE_DIRECTORY)) {

            //
            // File doesn't exist or is a directory.  Reset the path length
            // and continue.
            //

            Path.Length = DirectoryLength;

            continue;
        }

        //
        // Success!  File exists and *isn't* a directory.  We're done.
        //

        Success = TRUE;
        break;
    }

    if (!Success) {

        *Exists = FALSE;

        //
        // The files didn't exist, but no error occurred, so we return success.
        //

        Success = TRUE;

    } else {

        *Exists = TRUE;

        //
        // Update the user's pointers if applicable.
        //

        if (ARGUMENT_PRESENT(WhichIndex)) {
            *WhichIndex = Index;
        }

        if (ARGUMENT_PRESENT(WhichFilename)) {
            *WhichFilename = Filename;
        }

    }

    //
    // Intentional follow-on to "Error"; Success code will be set
    // appropriately by this stage.
    //

Error:
    if (HeapHandle) {
        HeapFree(HeapHandle, 0, Path.Buffer);
    }

    return Success;
}

_Success_(return != 0)
BOOL
CreateUnicodeString(
    _In_  PRTL                  Rtl,
    _In_  PCUNICODE_STRING      Source,
    _Out_ PPUNICODE_STRING      Destination,
    _In_  PALLOCATION_ROUTINE   AllocationRoutine,
    _In_  PVOID                 AllocationContext
    )
{
    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Source)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Destination)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AllocationRoutine)) {
        return FALSE;
    }

    return CreateUnicodeStringInline(Rtl,
                                     Source,
                                     Destination,
                                     AllocationRoutine,
                                     AllocationContext);

}

_Check_return_
BOOL
LoadRtlSymbols(_Inout_ PRTL Rtl)
{

    if (!(Rtl->Kernel32Module = LoadLibraryA("kernel32"))) {
        return FALSE;
    }

    if (!(Rtl->NtdllModule = LoadLibraryA("ntdll"))) {
        return FALSE;
    }

    if (!(Rtl->NtosKrnlModule = LoadLibraryA("ntoskrnl.exe"))) {
        return FALSE;
    }

    Rtl->GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)
        GetProcAddress(Rtl->Kernel32Module, "GetSystemTimePreciseAsFileTime");

    Rtl->NtQuerySystemTime = (PNTQUERYSYSTEMTIME)
        GetProcAddress(Rtl->NtdllModule, "NtQuerySystemTime");

    if (Rtl->GetSystemTimePreciseAsFileTime) {
        Rtl->SystemTimerFunction.GetSystemTimePreciseAsFileTime =
            Rtl->GetSystemTimePreciseAsFileTime;
    } else if (Rtl->NtQuerySystemTime) {
        Rtl->SystemTimerFunction.NtQuerySystemTime =
            Rtl->NtQuerySystemTime;
    } else {
        return FALSE;
    }

    //
    // Start of auto-generated function resolutions.  Any manual modifications
    // will be lost; run `tracerdev sync-rtl-header` to keep in sync.  If you
    // need to tweak the template, make sure the first block (RtlCharToInteger)
    // matches the new template -- the sync-rtl-header command relies on the
    // first block to match in order to determine the starting point for where
    // the blocks should be placed.
    //

    if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
        GetProcAddress(Rtl->NtdllModule, "RtlCharToInteger"))) {

        if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCharToInteger"))) {

            if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
                GetProcAddress(Rtl->Kernel32Module, "RtlCharToInteger"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCharToInteger'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeGenericTable"))) {

        if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeGenericTable"))) {

            if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitializeGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTable"))) {

        if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTable"))) {

            if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertElementGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTableFull"))) {

        if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTableFull"))) {

            if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertElementGenericTableFull"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTableFull'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteElementGenericTable"))) {

        if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDeleteElementGenericTable"))) {

            if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlDeleteElementGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlDeleteElementGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTable"))) {

        if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTable"))) {

            if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupElementGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTableFull"))) {

        if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTableFull"))) {

            if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupElementGenericTableFull"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTableFull'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTable"))) {

        if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTable"))) {

            if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableWithoutSplaying"))) {

        if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTableWithoutSplaying"))) {

            if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateGenericTableWithoutSplaying"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTableWithoutSplaying'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetElementGenericTable"))) {

        if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlGetElementGenericTable"))) {

            if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlGetElementGenericTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlGetElementGenericTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberGenericTableElements"))) {

        if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberGenericTableElements"))) {

            if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
                GetProcAddress(Rtl->Kernel32Module, "RtlNumberGenericTableElements"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlNumberGenericTableElements'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
        GetProcAddress(Rtl->NtdllModule, "RtlIsGenericTableEmpty"))) {

        if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlIsGenericTableEmpty"))) {

            if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
                GetProcAddress(Rtl->Kernel32Module, "RtlIsGenericTableEmpty"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlIsGenericTableEmpty'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitializeGenericTableAvl = (PRTL_INITIALIZE_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeGenericTableAvl"))) {

        if (!(Rtl->RtlInitializeGenericTableAvl = (PRTL_INITIALIZE_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeGenericTableAvl"))) {

            if (!(Rtl->RtlInitializeGenericTableAvl = (PRTL_INITIALIZE_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitializeGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertElementGenericTableAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTableAvl"))) {

        if (!(Rtl->RtlInsertElementGenericTableAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTableAvl"))) {

            if (!(Rtl->RtlInsertElementGenericTableAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertElementGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertElementGenericTableFullAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTableFullAvl"))) {

        if (!(Rtl->RtlInsertElementGenericTableFullAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTableFullAvl"))) {

            if (!(Rtl->RtlInsertElementGenericTableFullAvl = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertElementGenericTableFullAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTableFullAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlDeleteElementGenericTableAvl = (PRTL_DELETE_ELEMENT_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteElementGenericTableAvl"))) {

        if (!(Rtl->RtlDeleteElementGenericTableAvl = (PRTL_DELETE_ELEMENT_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDeleteElementGenericTableAvl"))) {

            if (!(Rtl->RtlDeleteElementGenericTableAvl = (PRTL_DELETE_ELEMENT_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlDeleteElementGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlDeleteElementGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupElementGenericTableAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTableAvl"))) {

        if (!(Rtl->RtlLookupElementGenericTableAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTableAvl"))) {

            if (!(Rtl->RtlLookupElementGenericTableAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupElementGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupElementGenericTableFullAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTableFullAvl"))) {

        if (!(Rtl->RtlLookupElementGenericTableFullAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTableFullAvl"))) {

            if (!(Rtl->RtlLookupElementGenericTableFullAvl = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupElementGenericTableFullAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTableFullAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateGenericTableAvl = (PRTL_ENUMERATE_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableAvl"))) {

        if (!(Rtl->RtlEnumerateGenericTableAvl = (PRTL_ENUMERATE_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTableAvl"))) {

            if (!(Rtl->RtlEnumerateGenericTableAvl = (PRTL_ENUMERATE_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateGenericTableWithoutSplayingAvl = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableWithoutSplayingAvl"))) {

        if (!(Rtl->RtlEnumerateGenericTableWithoutSplayingAvl = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTableWithoutSplayingAvl"))) {

            if (!(Rtl->RtlEnumerateGenericTableWithoutSplayingAvl = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateGenericTableWithoutSplayingAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTableWithoutSplayingAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupFirstMatchingElementGenericTableAvl = (PRTL_LOOKUP_FIRST_MATCHING_ELEMENT_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupFirstMatchingElementGenericTableAvl"))) {

        if (!(Rtl->RtlLookupFirstMatchingElementGenericTableAvl = (PRTL_LOOKUP_FIRST_MATCHING_ELEMENT_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupFirstMatchingElementGenericTableAvl"))) {

            if (!(Rtl->RtlLookupFirstMatchingElementGenericTableAvl = (PRTL_LOOKUP_FIRST_MATCHING_ELEMENT_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupFirstMatchingElementGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupFirstMatchingElementGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateGenericTableLikeADirectory = (PRTL_ENUMERATE_GENERIC_TABLE_LIKE_A_DICTIONARY)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableLikeADirectory"))) {

        if (!(Rtl->RtlEnumerateGenericTableLikeADirectory = (PRTL_ENUMERATE_GENERIC_TABLE_LIKE_A_DICTIONARY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTableLikeADirectory"))) {

            if (!(Rtl->RtlEnumerateGenericTableLikeADirectory = (PRTL_ENUMERATE_GENERIC_TABLE_LIKE_A_DICTIONARY)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateGenericTableLikeADirectory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTableLikeADirectory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlGetElementGenericTableAvl = (PRTL_GET_ELEMENT_GENERIC_TABLE_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlGetElementGenericTableAvl"))) {

        if (!(Rtl->RtlGetElementGenericTableAvl = (PRTL_GET_ELEMENT_GENERIC_TABLE_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlGetElementGenericTableAvl"))) {

            if (!(Rtl->RtlGetElementGenericTableAvl = (PRTL_GET_ELEMENT_GENERIC_TABLE_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlGetElementGenericTableAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlGetElementGenericTableAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlNumberGenericTableElementsAvl = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberGenericTableElementsAvl"))) {

        if (!(Rtl->RtlNumberGenericTableElementsAvl = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberGenericTableElementsAvl"))) {

            if (!(Rtl->RtlNumberGenericTableElementsAvl = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlNumberGenericTableElementsAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlNumberGenericTableElementsAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlIsGenericTableEmptyAvl = (PRTL_IS_GENERIC_TABLE_EMPTY_AVL)
        GetProcAddress(Rtl->NtdllModule, "RtlIsGenericTableEmptyAvl"))) {

        if (!(Rtl->RtlIsGenericTableEmptyAvl = (PRTL_IS_GENERIC_TABLE_EMPTY_AVL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlIsGenericTableEmptyAvl"))) {

            if (!(Rtl->RtlIsGenericTableEmptyAvl = (PRTL_IS_GENERIC_TABLE_EMPTY_AVL)
                GetProcAddress(Rtl->Kernel32Module, "RtlIsGenericTableEmptyAvl"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlIsGenericTableEmptyAvl'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
        GetProcAddress(Rtl->NtdllModule, "PfxInitialize"))) {

        if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxInitialize"))) {

            if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
                GetProcAddress(Rtl->Kernel32Module, "PfxInitialize"))) {

                OutputDebugStringA("Rtl: failed to resolve 'PfxInitialize'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxInsertPrefix"))) {

        if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxInsertPrefix"))) {

            if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "PfxInsertPrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'PfxInsertPrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxRemovePrefix"))) {

        if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxRemovePrefix"))) {

            if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "PfxRemovePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'PfxRemovePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxFindPrefix"))) {

        if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxFindPrefix"))) {

            if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "PfxFindPrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'PfxFindPrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlPrefixString = (PRTL_PREFIX_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlPrefixString"))) {

        if (!(Rtl->RtlPrefixString = (PRTL_PREFIX_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlPrefixString"))) {

            if (!(Rtl->RtlPrefixString = (PRTL_PREFIX_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlPrefixString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlPrefixString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlPrefixUnicodeString"))) {

        if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlPrefixUnicodeString"))) {

            if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlPrefixUnicodeString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlPrefixUnicodeString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlCreateHashTable"))) {

        if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCreateHashTable"))) {

            if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlCreateHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCreateHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteHashTable"))) {

        if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDeleteHashTable"))) {

            if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlDeleteHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlDeleteHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertEntryHashTable"))) {

        if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertEntryHashTable"))) {

            if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlRemoveEntryHashTable"))) {

        if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlRemoveEntryHashTable"))) {

            if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlRemoveEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlRemoveEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupEntryHashTable"))) {

        if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupEntryHashTable"))) {

            if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlLookupEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLookupEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetNextEntryHashTable"))) {

        if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlGetNextEntryHashTable"))) {

            if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlGetNextEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlGetNextEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateEntryHashTable"))) {

        if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateEntryHashTable"))) {

            if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlEnumerateEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndEnumerationHashTable"))) {

        if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEndEnumerationHashTable"))) {

            if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlEndEnumerationHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEndEnumerationHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitWeakEnumerationHashTable"))) {

        if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitWeakEnumerationHashTable"))) {

            if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitWeakEnumerationHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitWeakEnumerationHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlWeaklyEnumerateEntryHashTable"))) {

        if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlWeaklyEnumerateEntryHashTable"))) {

            if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlWeaklyEnumerateEntryHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlWeaklyEnumerateEntryHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndWeakEnumerationHashTable"))) {

        if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEndWeakEnumerationHashTable"))) {

            if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlEndWeakEnumerationHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEndWeakEnumerationHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlExpandHashTable"))) {

        if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlExpandHashTable"))) {

            if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlExpandHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlExpandHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlContractHashTable"))) {

        if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlContractHashTable"))) {

            if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
                GetProcAddress(Rtl->Kernel32Module, "RtlContractHashTable"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlContractHashTable'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeBitMap"))) {

        if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeBitMap"))) {

            if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitializeBitMap"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeBitMap'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBit"))) {

        if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearBit"))) {

            if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
                GetProcAddress(Rtl->Kernel32Module, "RtlClearBit"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlClearBit'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBit"))) {

        if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetBit"))) {

            if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
                GetProcAddress(Rtl->Kernel32Module, "RtlSetBit"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlSetBit'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlTestBit"))) {

        if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlTestBit"))) {

            if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
                GetProcAddress(Rtl->Kernel32Module, "RtlTestBit"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlTestBit'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearAllBits"))) {

        if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearAllBits"))) {

            if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlClearAllBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlClearAllBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetAllBits"))) {

        if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetAllBits"))) {

            if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlSetAllBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlSetAllBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBits"))) {

        if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearBits"))) {

            if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindClearBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBits"))) {

        if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindSetBits"))) {

            if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindSetBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindSetBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBitsAndSet"))) {

        if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearBitsAndSet"))) {

            if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindClearBitsAndSet"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearBitsAndSet'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBitsAndClear"))) {

        if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindSetBitsAndClear"))) {

            if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindSetBitsAndClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindSetBitsAndClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBits"))) {

        if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearBits"))) {

            if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlClearBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlClearBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBits"))) {

        if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetBits"))) {

            if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlSetBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlSetBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearRuns"))) {

        if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearRuns"))) {

            if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindClearRuns"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearRuns'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindLongestRunClear"))) {

        if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindLongestRunClear"))) {

            if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindLongestRunClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindLongestRunClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfClearBits"))) {

        if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberOfClearBits"))) {

            if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlNumberOfClearBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlNumberOfClearBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfSetBits"))) {

        if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberOfSetBits"))) {

            if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
                GetProcAddress(Rtl->Kernel32Module, "RtlNumberOfSetBits"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlNumberOfSetBits'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsClear"))) {

        if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAreBitsClear"))) {

            if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlAreBitsClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlAreBitsClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsSet"))) {

        if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAreBitsSet"))) {

            if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
                GetProcAddress(Rtl->Kernel32Module, "RtlAreBitsSet"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlAreBitsSet'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindFirstRunClear = (PRTL_FIND_FIRST_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindFirstRunClear"))) {

        if (!(Rtl->RtlFindFirstRunClear = (PRTL_FIND_FIRST_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindFirstRunClear"))) {

            if (!(Rtl->RtlFindFirstRunClear = (PRTL_FIND_FIRST_RUN_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindFirstRunClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindFirstRunClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindNextForwardRunClear"))) {

        if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindNextForwardRunClear"))) {

            if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindNextForwardRunClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindNextForwardRunClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindLastBackwardRunClear = (PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindLastBackwardRunClear"))) {

        if (!(Rtl->RtlFindLastBackwardRunClear = (PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindLastBackwardRunClear"))) {

            if (!(Rtl->RtlFindLastBackwardRunClear = (PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindLastBackwardRunClear"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindLastBackwardRunClear'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitializeUnicodePrefix = (PRTL_INITIALIZE_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeUnicodePrefix"))) {

        if (!(Rtl->RtlInitializeUnicodePrefix = (PRTL_INITIALIZE_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeUnicodePrefix"))) {

            if (!(Rtl->RtlInitializeUnicodePrefix = (PRTL_INITIALIZE_UNICODE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitializeUnicodePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeUnicodePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInsertUnicodePrefix = (PRTL_INSERT_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertUnicodePrefix"))) {

        if (!(Rtl->RtlInsertUnicodePrefix = (PRTL_INSERT_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertUnicodePrefix"))) {

            if (!(Rtl->RtlInsertUnicodePrefix = (PRTL_INSERT_UNICODE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "RtlInsertUnicodePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInsertUnicodePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlRemoveUnicodePrefix = (PRTL_REMOVE_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlRemoveUnicodePrefix"))) {

        if (!(Rtl->RtlRemoveUnicodePrefix = (PRTL_REMOVE_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlRemoveUnicodePrefix"))) {

            if (!(Rtl->RtlRemoveUnicodePrefix = (PRTL_REMOVE_UNICODE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "RtlRemoveUnicodePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlRemoveUnicodePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFindUnicodePrefix = (PRTL_FIND_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlFindUnicodePrefix"))) {

        if (!(Rtl->RtlFindUnicodePrefix = (PRTL_FIND_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindUnicodePrefix"))) {

            if (!(Rtl->RtlFindUnicodePrefix = (PRTL_FIND_UNICODE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "RtlFindUnicodePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFindUnicodePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlNextUnicodePrefix = (PRTL_NEXT_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlNextUnicodePrefix"))) {

        if (!(Rtl->RtlNextUnicodePrefix = (PRTL_NEXT_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNextUnicodePrefix"))) {

            if (!(Rtl->RtlNextUnicodePrefix = (PRTL_NEXT_UNICODE_PREFIX)
                GetProcAddress(Rtl->Kernel32Module, "RtlNextUnicodePrefix"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlNextUnicodePrefix'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCopyUnicodeString = (PRTL_COPY_UNICODE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlCopyUnicodeString"))) {

        if (!(Rtl->RtlCopyUnicodeString = (PRTL_COPY_UNICODE_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCopyUnicodeString"))) {

            if (!(Rtl->RtlCopyUnicodeString = (PRTL_COPY_UNICODE_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlCopyUnicodeString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCopyUnicodeString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlInitString = (PRTL_INIT_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlInitString"))) {

        if (!(Rtl->RtlInitString = (PRTL_INIT_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitString"))) {

            if (!(Rtl->RtlInitString = (PRTL_INIT_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlInitString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlInitString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCopyString = (PRTL_COPY_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlCopyString"))) {

        if (!(Rtl->RtlCopyString = (PRTL_COPY_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCopyString"))) {

            if (!(Rtl->RtlCopyString = (PRTL_COPY_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlCopyString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCopyString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlAppendUnicodeToString = (PRTL_APPEND_UNICODE_TO_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlAppendUnicodeToString"))) {

        if (!(Rtl->RtlAppendUnicodeToString = (PRTL_APPEND_UNICODE_TO_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAppendUnicodeToString"))) {

            if (!(Rtl->RtlAppendUnicodeToString = (PRTL_APPEND_UNICODE_TO_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlAppendUnicodeToString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlAppendUnicodeToString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlAppendUnicodeStringToString = (PRTL_APPEND_UNICODE_STRING_TO_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlAppendUnicodeStringToString"))) {

        if (!(Rtl->RtlAppendUnicodeStringToString = (PRTL_APPEND_UNICODE_STRING_TO_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAppendUnicodeStringToString"))) {

            if (!(Rtl->RtlAppendUnicodeStringToString = (PRTL_APPEND_UNICODE_STRING_TO_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlAppendUnicodeStringToString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlAppendUnicodeStringToString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlUnicodeStringToAnsiSize = (PRTL_UNICODE_STRING_TO_ANSI_SIZE)
        GetProcAddress(Rtl->NtdllModule, "RtlUnicodeStringToAnsiSize"))) {

        if (!(Rtl->RtlUnicodeStringToAnsiSize = (PRTL_UNICODE_STRING_TO_ANSI_SIZE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlUnicodeStringToAnsiSize"))) {

            if (!(Rtl->RtlUnicodeStringToAnsiSize = (PRTL_UNICODE_STRING_TO_ANSI_SIZE)
                GetProcAddress(Rtl->Kernel32Module, "RtlUnicodeStringToAnsiSize"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlUnicodeStringToAnsiSize'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlUnicodeStringToAnsiString = (PRTL_UNICODE_STRING_TO_ANSI_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlUnicodeStringToAnsiString"))) {

        if (!(Rtl->RtlUnicodeStringToAnsiString = (PRTL_UNICODE_STRING_TO_ANSI_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlUnicodeStringToAnsiString"))) {

            if (!(Rtl->RtlUnicodeStringToAnsiString = (PRTL_UNICODE_STRING_TO_ANSI_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlUnicodeStringToAnsiString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlUnicodeStringToAnsiString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEqualString = (PRTL_EQUAL_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlEqualString"))) {

        if (!(Rtl->RtlEqualString = (PRTL_EQUAL_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEqualString"))) {

            if (!(Rtl->RtlEqualString = (PRTL_EQUAL_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlEqualString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEqualString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlEqualUnicodeString = (PRTL_EQUAL_UNICODE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlEqualUnicodeString"))) {

        if (!(Rtl->RtlEqualUnicodeString = (PRTL_EQUAL_UNICODE_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEqualUnicodeString"))) {

            if (!(Rtl->RtlEqualUnicodeString = (PRTL_EQUAL_UNICODE_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlEqualUnicodeString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlEqualUnicodeString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCompareString = (PRTL_COMPARE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlCompareString"))) {

        if (!(Rtl->RtlCompareString = (PRTL_COMPARE_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCompareString"))) {

            if (!(Rtl->RtlCompareString = (PRTL_COMPARE_STRING)
                GetProcAddress(Rtl->Kernel32Module, "RtlCompareString"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCompareString'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCompareMemory = (PRTL_COMPARE_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "RtlCompareMemory"))) {

        if (!(Rtl->RtlCompareMemory = (PRTL_COMPARE_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCompareMemory"))) {

            if (!(Rtl->RtlCompareMemory = (PRTL_COMPARE_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "RtlCompareMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCompareMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlPrefetchMemoryNonTemporal = (PRTL_PREFETCH_MEMORY_NON_TEMPORAL)
        GetProcAddress(Rtl->NtdllModule, "RtlPrefetchMemoryNonTemporal"))) {

        if (!(Rtl->RtlPrefetchMemoryNonTemporal = (PRTL_PREFETCH_MEMORY_NON_TEMPORAL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlPrefetchMemoryNonTemporal"))) {

            if (!(Rtl->RtlPrefetchMemoryNonTemporal = (PRTL_PREFETCH_MEMORY_NON_TEMPORAL)
                GetProcAddress(Rtl->Kernel32Module, "RtlPrefetchMemoryNonTemporal"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlPrefetchMemoryNonTemporal'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlMoveMemory = (PRTL_MOVE_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "RtlMoveMemory"))) {

        if (!(Rtl->RtlMoveMemory = (PRTL_MOVE_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlMoveMemory"))) {

            if (!(Rtl->RtlMoveMemory = (PRTL_MOVE_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "RtlMoveMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlMoveMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCopyMemory = (PRTL_COPY_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "RtlCopyMemory"))) {

        if (!(Rtl->RtlCopyMemory = (PRTL_COPY_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCopyMemory"))) {

            if (!(Rtl->RtlCopyMemory = (PRTL_COPY_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "RtlCopyMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCopyMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCopyMappedMemory = (PRTL_COPY_MAPPED_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "RtlCopyMappedMemory"))) {

        if (!(Rtl->RtlCopyMappedMemory = (PRTL_COPY_MAPPED_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCopyMappedMemory"))) {

            if (!(Rtl->RtlCopyMappedMemory = (PRTL_COPY_MAPPED_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "RtlCopyMappedMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCopyMappedMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFillMemory = (PRTL_FILL_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "RtlFillMemory"))) {

        if (!(Rtl->RtlFillMemory = (PRTL_FILL_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFillMemory"))) {

            if (!(Rtl->RtlFillMemory = (PRTL_FILL_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "RtlFillMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFillMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlLocalTimeToSystemTime = (PRTL_LOCAL_TIME_TO_SYSTEM_TIME)
        GetProcAddress(Rtl->NtdllModule, "RtlLocalTimeToSystemTime"))) {

        if (!(Rtl->RtlLocalTimeToSystemTime = (PRTL_LOCAL_TIME_TO_SYSTEM_TIME)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLocalTimeToSystemTime"))) {

            if (!(Rtl->RtlLocalTimeToSystemTime = (PRTL_LOCAL_TIME_TO_SYSTEM_TIME)
                GetProcAddress(Rtl->Kernel32Module, "RtlLocalTimeToSystemTime"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlLocalTimeToSystemTime'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlTimeToSecondsSince1970 = (PRTL_TIME_TO_SECONDS_SINCE_1970)
        GetProcAddress(Rtl->NtdllModule, "RtlTimeToSecondsSince1970"))) {

        if (!(Rtl->RtlTimeToSecondsSince1970 = (PRTL_TIME_TO_SECONDS_SINCE_1970)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlTimeToSecondsSince1970"))) {

            if (!(Rtl->RtlTimeToSecondsSince1970 = (PRTL_TIME_TO_SECONDS_SINCE_1970)
                GetProcAddress(Rtl->Kernel32Module, "RtlTimeToSecondsSince1970"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlTimeToSecondsSince1970'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->bsearch = (PBSEARCH)
        GetProcAddress(Rtl->NtdllModule, "bsearch"))) {

        if (!(Rtl->bsearch = (PBSEARCH)
            GetProcAddress(Rtl->NtosKrnlModule, "bsearch"))) {

            if (!(Rtl->bsearch = (PBSEARCH)
                GetProcAddress(Rtl->Kernel32Module, "bsearch"))) {

                OutputDebugStringA("Rtl: failed to resolve 'bsearch'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->qsort = (PQSORT)
        GetProcAddress(Rtl->NtdllModule, "qsort"))) {

        if (!(Rtl->qsort = (PQSORT)
            GetProcAddress(Rtl->NtosKrnlModule, "qsort"))) {

            if (!(Rtl->qsort = (PQSORT)
                GetProcAddress(Rtl->Kernel32Module, "qsort"))) {

                OutputDebugStringA("Rtl: failed to resolve 'qsort'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->K32GetProcessMemoryInfo = (PGET_PROCESS_MEMORY_INFO)
        GetProcAddress(Rtl->NtdllModule, "K32GetProcessMemoryInfo"))) {

        if (!(Rtl->K32GetProcessMemoryInfo = (PGET_PROCESS_MEMORY_INFO)
            GetProcAddress(Rtl->NtosKrnlModule, "K32GetProcessMemoryInfo"))) {

            if (!(Rtl->K32GetProcessMemoryInfo = (PGET_PROCESS_MEMORY_INFO)
                GetProcAddress(Rtl->Kernel32Module, "K32GetProcessMemoryInfo"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32GetProcessMemoryInfo'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->GetProcessIoCounters = (PGET_PROCESS_IO_COUNTERS)
        GetProcAddress(Rtl->NtdllModule, "GetProcessIoCounters"))) {

        if (!(Rtl->GetProcessIoCounters = (PGET_PROCESS_IO_COUNTERS)
            GetProcAddress(Rtl->NtosKrnlModule, "GetProcessIoCounters"))) {

            if (!(Rtl->GetProcessIoCounters = (PGET_PROCESS_IO_COUNTERS)
                GetProcAddress(Rtl->Kernel32Module, "GetProcessIoCounters"))) {

                OutputDebugStringA("Rtl: failed to resolve 'GetProcessIoCounters'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->GetProcessHandleCount = (PGET_PROCESS_HANDLE_COUNT)
        GetProcAddress(Rtl->NtdllModule, "GetProcessHandleCount"))) {

        if (!(Rtl->GetProcessHandleCount = (PGET_PROCESS_HANDLE_COUNT)
            GetProcAddress(Rtl->NtosKrnlModule, "GetProcessHandleCount"))) {

            if (!(Rtl->GetProcessHandleCount = (PGET_PROCESS_HANDLE_COUNT)
                GetProcAddress(Rtl->Kernel32Module, "GetProcessHandleCount"))) {

                OutputDebugStringA("Rtl: failed to resolve 'GetProcessHandleCount'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->SearchPathW = (PSEARCHPATHW)
        GetProcAddress(Rtl->NtdllModule, "SearchPathW"))) {

        if (!(Rtl->SearchPathW = (PSEARCHPATHW)
            GetProcAddress(Rtl->NtosKrnlModule, "SearchPathW"))) {

            if (!(Rtl->SearchPathW = (PSEARCHPATHW)
                GetProcAddress(Rtl->Kernel32Module, "SearchPathW"))) {

                OutputDebugStringA("Rtl: failed to resolve 'SearchPathW'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->CreateToolhelp32Snapshot = (PCREATE_TOOLHELP32_SNAPSHOT)
        GetProcAddress(Rtl->NtdllModule, "CreateToolhelp32Snapshot"))) {

        if (!(Rtl->CreateToolhelp32Snapshot = (PCREATE_TOOLHELP32_SNAPSHOT)
            GetProcAddress(Rtl->NtosKrnlModule, "CreateToolhelp32Snapshot"))) {

            if (!(Rtl->CreateToolhelp32Snapshot = (PCREATE_TOOLHELP32_SNAPSHOT)
                GetProcAddress(Rtl->Kernel32Module, "CreateToolhelp32Snapshot"))) {

                OutputDebugStringA("Rtl: failed to resolve 'CreateToolhelp32Snapshot'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->Thread32First = (PTHREAD32_FIRST)
        GetProcAddress(Rtl->NtdllModule, "Thread32First"))) {

        if (!(Rtl->Thread32First = (PTHREAD32_FIRST)
            GetProcAddress(Rtl->NtosKrnlModule, "Thread32First"))) {

            if (!(Rtl->Thread32First = (PTHREAD32_FIRST)
                GetProcAddress(Rtl->Kernel32Module, "Thread32First"))) {

                OutputDebugStringA("Rtl: failed to resolve 'Thread32First'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->Thread32Next = (PTHREAD32_NEXT)
        GetProcAddress(Rtl->NtdllModule, "Thread32Next"))) {

        if (!(Rtl->Thread32Next = (PTHREAD32_NEXT)
            GetProcAddress(Rtl->NtosKrnlModule, "Thread32Next"))) {

            if (!(Rtl->Thread32Next = (PTHREAD32_NEXT)
                GetProcAddress(Rtl->Kernel32Module, "Thread32Next"))) {

                OutputDebugStringA("Rtl: failed to resolve 'Thread32Next'");
                return FALSE;
            }
        }
    }

    //
    // End of auto-generated function resolutions.
    //

    //
    // This is a hack; we need RtlCompareString() from within PCRTCOMPARE-type
    // functions passed to bsearch/qsort.
    //

    _RtlCompareString = Rtl->RtlCompareString;

    return TRUE;
}


RTL_API
BOOLEAN
RtlCheckBit(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG BitPosition
    )
{
#ifdef _M_AMD64
    return BitTest64((LONG64 const *)BitMapHeader->Buffer, (LONG64)BitPosition);
#else
    return BitTest((LONG const *)BitMapHeader->Buffer, (LONG)BitPosition);
#endif
}

//
// Functions for Splay Macros
//

RTL_API
VOID
RtlInitializeSplayLinks(
    _Out_ PRTL_SPLAY_LINKS Links
    )
{
    Links->Parent = Links;
    Links->LeftChild = NULL;
    Links->RightChild = NULL;
}

RTL_API
PRTL_SPLAY_LINKS
RtlParent(_In_ PRTL_SPLAY_LINKS Links)
{
    return Links->Parent;
}

RTL_API
PRTL_SPLAY_LINKS
RtlLeftChild(_In_ PRTL_SPLAY_LINKS Links)
{
    return Links->LeftChild;
}

RTL_API
PRTL_SPLAY_LINKS
RtlRightChild(_In_ PRTL_SPLAY_LINKS Links)
{
    return Links->RightChild;
}

RTL_API
BOOLEAN
RtlIsRoot(_In_ PRTL_SPLAY_LINKS Links)
{
    return (RtlParent(Links) == Links);
}

RTL_API
BOOLEAN
RtlIsLeftChild(_In_ PRTL_SPLAY_LINKS Links)
{
    return (RtlLeftChild(RtlParent(Links)) == Links);
}

RTL_API
BOOLEAN
RtlIsRightChild(_In_ PRTL_SPLAY_LINKS Links)
{
    return (RtlRightChild(RtlParent(Links)) == Links);
}

RTL_API
VOID
RtlInsertAsLeftChild (
    _Inout_ PRTL_SPLAY_LINKS ParentLinks,
    _Inout_ PRTL_SPLAY_LINKS ChildLinks
    )
{
    ParentLinks->LeftChild = ChildLinks;
    ChildLinks->Parent = ParentLinks;
}

RTL_API
VOID
RtlInsertAsRightChild (
    _Inout_ PRTL_SPLAY_LINKS ParentLinks,
    _Inout_ PRTL_SPLAY_LINKS ChildLinks
    )
{
    ParentLinks->RightChild = ChildLinks;
    ChildLinks->Parent = ParentLinks;
}

RTL_API
LONG
CompareStringCaseInsensitive(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2
    )
{
    return _RtlCompareString(String1, String2, FALSE);
}

_Check_return_
BOOL
LoadRtlExFunctions(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTLEXFUNCTIONS RtlExFunctions
    )
{
    if (!RtlExModule) {
        return FALSE;
    }

    if (!RtlExFunctions) {
        return FALSE;
    }

    //
    // Start of auto-generated section.
    //

    if (!(RtlExFunctions->DestroyRtl = (PDESTROY_RTL)
        GetProcAddress(RtlExModule, "DestroyRtl"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DestroyRtl'");
        return FALSE;
    }

    if (!(RtlExFunctions->PrefaultPages = (PPREFAULT_PAGES)
        GetProcAddress(RtlExModule, "PrefaultPages"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'PrefaultPages'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlCheckBit = (PRTL_CHECK_BIT)
        GetProcAddress(RtlExModule, "RtlCheckBit"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlCheckBit'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlInitializeSplayLinks = (PRTL_INITIALIZE_SPLAY_LINKS)
        GetProcAddress(RtlExModule, "RtlInitializeSplayLinks"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlInitializeSplayLinks'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlParent = (PRTL_PARENT)
        GetProcAddress(RtlExModule, "RtlParent"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlParent'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlLeftChild = (PRTL_LEFT_CHILD)
        GetProcAddress(RtlExModule, "RtlLeftChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlLeftChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlRightChild = (PRTL_RIGHT_CHILD)
        GetProcAddress(RtlExModule, "RtlRightChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlRightChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlIsRoot = (PRTL_IS_ROOT)
        GetProcAddress(RtlExModule, "RtlIsRoot"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlIsRoot'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlIsLeftChild = (PRTL_IS_LEFT_CHILD)
        GetProcAddress(RtlExModule, "RtlIsLeftChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlIsLeftChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlIsRightChild = (PRTL_IS_RIGHT_CHILD)
        GetProcAddress(RtlExModule, "RtlIsRightChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlIsRightChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlInsertAsLeftChild = (PRTL_INSERT_AS_LEFT_CHILD)
        GetProcAddress(RtlExModule, "RtlInsertAsLeftChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlInsertAsLeftChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlInsertAsRightChild = (PRTL_INSERT_AS_RIGHT_CHILD)
        GetProcAddress(RtlExModule, "RtlInsertAsRightChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlInsertAsRightChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyToMemoryMappedMemory = (PCOPY_TO_MEMORY_MAPPED_MEMORY)
        GetProcAddress(RtlExModule, "CopyToMemoryMappedMemory"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyToMemoryMappedMemory'");
        return FALSE;
    }

    if (!(RtlExFunctions->FindCharsInUnicodeString = (PFIND_CHARS_IN_UNICODE_STRING)
        GetProcAddress(RtlExModule, "FindCharsInUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FindCharsInUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->CreateBitmapIndexForUnicodeString = (PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING)
        GetProcAddress(RtlExModule, "CreateBitmapIndexForUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CreateBitmapIndexForUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->FindCharsInString = (PFIND_CHARS_IN_STRING)
        GetProcAddress(RtlExModule, "FindCharsInString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FindCharsInString'");
        return FALSE;
    }

    if (!(RtlExFunctions->CreateBitmapIndexForString = (PCREATE_BITMAP_INDEX_FOR_STRING)
        GetProcAddress(RtlExModule, "CreateBitmapIndexForString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CreateBitmapIndexForString'");
        return FALSE;
    }

    if (!(RtlExFunctions->FilesExistW = (PFILES_EXISTW)
        GetProcAddress(RtlExModule, "FilesExistW"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FilesExistW'");
        return FALSE;
    }

    if (!(RtlExFunctions->FilesExistA = (PFILES_EXISTA)
        GetProcAddress(RtlExModule, "FilesExistA"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FilesExistA'");
        return FALSE;
    }

    if (!(RtlExFunctions->TestExceptionHandler = (PTEST_EXCEPTION_HANDLER)
        GetProcAddress(RtlExModule, "TestExceptionHandler"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'TestExceptionHandler'");
        return FALSE;
    }

    if (!(RtlExFunctions->ArgvWToArgvA = (PARGVW_TO_ARGVA)
        GetProcAddress(RtlExModule, "ArgvWToArgvA"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'ArgvWToArgvA'");
        return FALSE;
    }

    if (!(RtlExFunctions->UnicodeStringToPath = (PUNICODE_STRING_TO_PATH)
        GetProcAddress(RtlExModule, "UnicodeStringToPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'UnicodeStringToPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->DestroyPath = (PDESTROY_PATH)
        GetProcAddress(RtlExModule, "DestroyPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DestroyPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->GetModulePath = (PGET_MODULE_PATH)
        GetProcAddress(RtlExModule, "GetModulePath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'GetModulePath'");
        return FALSE;
    }

    if (!(RtlExFunctions->CurrentDirectoryToUnicodeString = (PCURRENT_DIRECTORY_TO_UNICODE_STRING)
        GetProcAddress(RtlExModule, "CurrentDirectoryToUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CurrentDirectoryToUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->CurrentDirectoryToPath = (PCURRENT_DIRECTORY_TO_PATH)
        GetProcAddress(RtlExModule, "CurrentDirectoryToPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CurrentDirectoryToPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadPathEnvironmentVariable = (PLOAD_PATH_ENVIRONMENT_VARIABLE)
        GetProcAddress(RtlExModule, "LoadPathEnvironmentVariable"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadPathEnvironmentVariable'");
        return FALSE;
    }

    if (!(RtlExFunctions->DestroyPathEnvironmentVariable = (PDESTROY_PATH_ENVIRONMENT_VARIABLE)
        GetProcAddress(RtlExModule, "DestroyPathEnvironmentVariable"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DestroyPathEnvironmentVariable'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadShlwapi = (PLOAD_SHLWAPI)
        GetProcAddress(RtlExModule, "LoadShlwapi"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadShlwapi'");
        return FALSE;
    }

    //
    // End of auto-generated section.
    //

    return TRUE;
}

_Check_return_
BOOL
LoadRtlExSymbols(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTL    Rtl
)
{
    HMODULE Module;

    if (!Rtl) {
        return FALSE;
    }

    if (RtlExModule) {
        Module = RtlExModule;

    } else {

        DWORD Flags = (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS          |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        );

        if (!GetModuleHandleEx(Flags, (LPCTSTR)&LoadRtlExFunctions, &Module)) {
            return FALSE;
        }

        if (!Module) {
            return FALSE;
        }
    }

    if (!LoadRtlExFunctions(Module, &Rtl->RtlExFunctions)) {
        return FALSE;
    }

    return TRUE;

}

_Use_decl_annotations_
BOOL
InitializeRtl(
    PRTL   Rtl,
    PULONG SizeOfRtl
    )
{
    HANDLE HeapHandle;

    if (!Rtl) {
        if (SizeOfRtl) {
            *SizeOfRtl = sizeof(*Rtl);
        }
        return FALSE;
    }

    if (!SizeOfRtl) {
        return FALSE;
    }

    if (*SizeOfRtl < sizeof(*Rtl)) {
        *SizeOfRtl = sizeof(*Rtl);
        return FALSE;
    } else {
        *SizeOfRtl = sizeof(*Rtl);
    }

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        return FALSE;
    }

    SecureZeroMemory(Rtl, sizeof(*Rtl));

    if (!LoadRtlSymbols(Rtl)) {
        return FALSE;
    }

    SetCSpecificHandler(Rtl->NtdllModule);

    Rtl->HeapHandle = HeapHandle;

    if (!LoadRtlExSymbols(NULL, Rtl)) {
        return FALSE;
    }

    return TRUE;
}

RTL_API
BOOL
InitializeRtlManually(PRTL Rtl, PULONG SizeOfRtl)
{
    return InitializeRtlManuallyInline(Rtl, SizeOfRtl);
}

_Use_decl_annotations_
VOID
DestroyRtl(
    PPRTL RtlPointer
    )
{
    PRTL Rtl;

    if (!ARGUMENT_PRESENT(RtlPointer)) {
        return;
    }

    Rtl = *RtlPointer;

    if (!ARGUMENT_PRESENT(Rtl)) {
        return;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *RtlPointer = NULL;

    if (Rtl->NtdllModule) {
        FreeLibrary(Rtl->NtdllModule);
        Rtl->NtdllModule = NULL;
    }

    if (Rtl->Kernel32Module) {
        FreeLibrary(Rtl->Kernel32Module);
        Rtl->Kernel32Module = NULL;
    }

    if (Rtl->NtosKrnlModule) {
        FreeLibrary(Rtl->NtosKrnlModule);
        Rtl->NtosKrnlModule = NULL;
    }

    return;
}

VOID
Debugbreak()
{
    __debugbreak();
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
