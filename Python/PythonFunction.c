/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonFunction.c

Abstract:

    This module implements functionality related to the PYTHON_FUNCTION
    structure, which is based on the PYTHON_PATH_TABLE_ENTRY structure,
    additionally capturing Python function information such as the code
    object, number of lines, etc.

    This module is consumed by Python tracing components, which interface
    to it via the RegisterFrame method, which returns the corresponding
    PYTHON_FUNCTION structure for a given Python frame object.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RegisterFrame(
    PPYTHON Python,
    PPYFRAMEOBJECT FrameObject,
    PYTHON_EVENT_TRAITS EventTraits,
    PPYOBJECT ArgObject,
    PPPYTHON_FUNCTION FunctionPointer
    )
/*++

Routine Description:

    This routine returns a PYTHON_FUNCTION structure for a given Python frame
    object, a process referred to as "frame registration".  Its primary goal
    is to provide rich function information with low overhead, such that it
    can be called for every single line trace event of a potentially long
    running Python program without adversely affecting runtime too much.

    The frame object's underlying code object is tracked in a splaying table
    (RTL_GENERIC_TABLE), which allows us to exploit the temporal locality of
    trace events and code objects.  Thus, the fast path through this routine
    is the most common path: being called with the same underlying code object
    as the previous call.  This ensures the code object's pointer is at the
    root of the splay tree, and can be returned with minimal pointer chasing
    overhead and good cache locality.  (This is the same approach taken by the
    cProfile module within the Python interpreter core.)

    If we're seeing a new code object for the first time, GetPathEntryFromFrame
    will be called.  This routine extracts the PYTHON_PATH_TABLE_ENTRY structure
    for the filename object associated with the code object, with the amount of
    work required to satisfy such a request dependent upon how many things we've
    seen before.

    If the filename hasn't been seen before, and the directory hasn't been seen
    before, the routine will need to enumerate each directory, from longest to
    shortest, seeing if it is a module directory.  That is, whether or not the
    directory contains an __init__.py[co] file.  We stop once we've found our
    first non-module directory, which is also referred to as a root directory.

    Once our root directory is registered, module directories can be registered
    for each intermediate directory between the root and the directory that the
    filename resides in.

    Once we have a PYTHON_PATH_TABLE_ENTRY for the filename, we can finalize
    the creation of a PYTHON_FUNCTION structure, ensuring that is has the right
    module and class name information if applicable.

    The code object is stored in the new PYTHON_FUNCTION's CodeObject field,
    and then inserted into the PYTHON structure's PYTHON_FUNCTION_TABLE, which
    is the RTL_GENERIC_TABLE-based structure referred to earlier.  This will
    also splay the tree such that the newly-inserted function will be at the
    root of the table, ensuring fast subsequent access.

    This routine relies heavily on the PythonPathTableEntry component.  The
    pseudo code of the overall flow for frame registration looks like this:

    RegisterFrame():

        CodeObject = Frame.CodeObject
        Function = Python.FunctionTable[CodeObject]

        if not Function:

            //
            // GetPathEntryForFrame():
            //

            Filename = CodeObject.Filename

            FilePathEntry = Python.PathTable[Filename]

            if not FilePathEntry:

                //
                // RegisterFile():
                //

                DirectoryName = Filename.DirectoryPart
                DirectoryPathEntry = Python.PathTable[DirectoryName]

                if not DirectoryPathEntry:

                    //
                    // GetPathEntryForDirectory():
                    //

                    If a prefix match was found for the part of the directory
                    name, register each intermediate directory from the prefix
                    onward.

                    Otherwise, if no part of the directory has been seen before,
                    add our parent, then our parent's parent, etc, until we find
                    the first non-module directory (i.e. the first directory
                    without an __init__.py file).

                    (A non-module directory is registered via the routine
                     RegisterNonModuleDirectory(), a module directory via the
                     routine RegisterModuleDirectory().)

                    return the DirectoryPathEntry

                FilePathEntry = new PythonPathTableEntry(DirectoryPathEntry)
                Python.PathTable[Filename] = FilePathEntry

            Function = new PythonFunction(FilePathEntry)

        return Function

    Thus, the most expensive path through the code is typically visited the
    least; i.e. registering a new function, filename, directory and all ancestor
    directories as necessary.  Once each intermediate piece has been done, the
    extensive use of prefix trees ensures we don't need to keep repeating the
    expensive operations.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    FrameObject - Supplies a pointer to a Python frame object structure.  That
        is, the PyFrameObject * passed to the Python C trace function.  As the
        layout of this structure has changed over different Python versions,
        the Python->CodeObjectOffsets table is used initially to resolve the
        code object pointer from the frame object.

    EventTraits - Supplies a PYTHON_EVENT_TRAITS value that describes the event.

    ArgObject - Supplies a pointer to a PYOBJECT structure that was provided as
        a parameter to the trace function.

    FunctionPointer - Supplies a pointer to a variable that receives the address
        of a PYTHON_FUNCTION structure for the FrameObject's underlying code
        object.  If the routine fails to resolve a function for the frame, this
        will be set to NULL.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsValid;
    BOOL IsC;
    BOOL IsCall;

    PRTL Rtl;
    PPYFRAMEOBJECT Frame = (PPYFRAMEOBJECT)FrameObject;
    PPYOBJECT CodeObject;
    PPYCFUNCTIONOBJECT PyCFunctionObject;
    PPYMETHODDEF MethodDef;
    PPYCFUNCTION PyCFunctionPointer;
    STRING FilenameString;

    PYTHON_FUNCTION FunctionRecord;
    PPYTHON_FUNCTION Function;
    PPYTHON_FUNCTION_TABLE FunctionTable;
    BOOLEAN NewFunction;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;
    FILENAME_FLAGS FilenameFlags;

    //
    // Clear the caller's function pointer up front if present.
    //

    if (ARGUMENT_PRESENT(FunctionPointer)) {
        *FunctionPointer = NULL;
    }

    //
    // Verify the code object type is something sane.
    //

    CodeObject = Frame->Code;
    if (CodeObject->Type != Python->PyCode.Type) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    IsC = EventTraits.IsC;
    IsCall = EventTraits.IsCall;

    Rtl = Python->Rtl;
    FunctionTable = Python->FunctionTable;

    if (IsC) {

        //
        // Initialize local variables specific to Python C functions.
        //

        PyCFunctionObject = (PPYCFUNCTIONOBJECT)ArgObject;

        //
        // Make sure the underlying type object is what we expect.
        //

        if (PyCFunctionObject->Type != Python->PyCFunction.Type) {
            return FALSE;
        }

        MethodDef = PyCFunctionObject->MethodDef;
        PyCFunctionPointer = MethodDef->FunctionPointer;

        //
        // Use the function pointer as the key.
        //

        FunctionRecord.Key = (ULONG_PTR)PyCFunctionPointer;

    } else {

        //
        // Use the code object as the key.
        //

        FunctionRecord.Key = (ULONG_PTR)CodeObject;
    }

    //
    // Attempt to insert the function into the function table.
    //

    Function = Rtl->RtlInsertElementGenericTable(
        &FunctionTable->GenericTable,
        &FunctionRecord,
        sizeof(FunctionRecord),
        &NewFunction
    );

    if (!NewFunction) {

        //
        // We've already seen this function.  Increment the call count if it's
        // a valid function and this is a call event.
        //

        IsValid = IsValidFunction(Function);

        if (IsValid) {
            if (IsCall) {
                Function->CallCount++;
            }
        }

        goto End;
    }

    //
    // This is a new function.  RtlInsertElementGenericTable() will have copied
    // the stack-backed memory of the FunctionRecord structure over to the new
    // structure, so zero the entire structure now, then reset the code object
    // and key fields.
    //

    SecureZeroMemory(Function, sizeof(*Function));
    Function->CodeObject = CodeObject;
    Function->Key = FunctionRecord.Key;

    //
    // Clear the filename flags.
    //

    FilenameFlags.AsLong = 0;

    //
    // We now need to get the filename to pass to GetPathEntryFromFrame().  How
    // we do this depends on whether or not this is normal Python user code, or
    // a Python C function.
    //

    if (IsC) {

        //
        // We're a Python C function.  Get the underlying DLL handle for the
        // C function pointer, then get the DLL's file name.
        //

        CHAR Path[_MAX_PATH];
        HMODULE Handle;
        ULONG Flags;
        DWORD BufferSizeInChars = sizeof(Path);
        DWORD ActualSizeInChars;
        LPCWSTR Method = (LPCWSTR)PyCFunctionPointer;

        Flags = (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        );

        if (!GetModuleHandleExW(Flags, Method, &Handle)) {
            return FALSE;
        }

        if (!Handle) {
            return FALSE;
        }

        ActualSizeInChars = GetModuleFileNameA(Handle,
                                               (LPSTR)&Path,
                                               BufferSizeInChars);

        if (ActualSizeInChars == 0) {
            DWORD LastError = GetLastError();
            if (LastError == ERROR_INSUFFICIENT_BUFFER) {

                //
                // XXX todo: allocate string buffer.
                //

                __debugbreak();

                FilenameFlags.WeOwnPathBuffer = TRUE;
            }

            return FALSE;
        }

        //
        // N.B. ActualSizeInChars includes the NUL-terminator, so we don't need
        //      to subtract 1 from the length here.
        //

        FilenameString.Length = (USHORT)ActualSizeInChars;
        FilenameString.MaximumLength = (USHORT)BufferSizeInChars;
        FilenameString.Buffer = Path;

        FilenameFlags.IsFullyQualified = TRUE;
        FilenameFlags.IsDll = TRUE;

        Function->ModuleHandle = Handle;

    } else {

        BOOL IsSpecialName;
        PSTRING Path;
        PPYOBJECT FilenameObject;
        PCPYCODEOBJECTOFFSETS CodeObjectOffsets;

        CodeObjectOffsets = Python->PyCodeObjectOffsets;

        FilenameObject = *(
            (PPPYOBJECT)RtlOffsetToPointer(
                FrameObject->Code,
                Python->PyCodeObjectOffsets->Filename
            )
        );

        Path = &FilenameString;

        Success = WrapPythonFilenameStringAsString(
            Python,
            FilenameObject,
            Path,
            &IsSpecialName
        );

        if (!Success) {
            __debugbreak();
            return FALSE;
        }

        if (IsSpecialName) {
            FilenameFlags.IsSpecialName = TRUE;
        }
    }

    //
    // Attempt to get a path entry for this frame and filename object.
    //

    Success = GetPathEntryFromFrame(Python,
                                    FrameObject,
                                    EventTraits,
                                    ArgObject,
                                    &FilenameString,
                                    &FilenameFlags,
                                    &ParentPathEntry);

    if (!Success || !ParentPathEntry || !ParentPathEntry->IsValid) {
        goto Error;
    }

    Function->ParentPathEntry = ParentPathEntry;

    //
    // Finish registration of the function.
    //

    if (IsC) {
        Function->PyCFunctionObject = PyCFunctionObject;
        Success = RegisterPyCFunction(Python,
                                      Function,
                                      FrameObject);
    } else {
        Success = RegisterPythonFunction(Python,
                                         Function,
                                         FrameObject);
    }

    if (Success) {
        IsValid = TRUE;

        //
        // Mark our 'seen' bit in the relevant target object's reference count.
        //

        if (IsC) {
            PyCFunctionObject->RefCountEx.Seen = TRUE;
        } else {
            CodeObject->RefCountEx.Seen = TRUE;
        }

        //
        // Initialize the linked-list head.
        //

        InitializeListHead(&Function->ListEntry);

        goto End;
    }

    //
    // Intentional follow-on to Error.
    //

Error:
    IsValid = Function->PathEntry.IsValid = FALSE;

End:
    if (IsValid) {
        if (!Function->PathEntry.IsValid) {
            __debugbreak();
        }
        if (ARGUMENT_PRESENT(FunctionPointer)) {
            *FunctionPointer = Function;
        }
    }

    return IsValid;
}

_Use_decl_annotations_
BOOL
RegisterPythonFunction(
    PPYTHON Python,
    PPYTHON_FUNCTION Function,
    PPYFRAMEOBJECT FrameObject
    )
/*++

Routine Description:

    This method is responsible for finalizing details about a function, such
    as the function name, class name (if any), module name and first line
    number.  A frame object is provided to assist with resolution of names.

    It is called once per function after a new PYTHON_PATH_TABLE_ENTRY has been
    inserted into the path prefix table.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure to be
        registered by this routine.

    FrameObject - Supplies a pointer to the PYFRAMEOBJECT structure for which
        this function is being registered.  This is required in order to assist
        with the resolution of names.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PRTL Rtl;
    PCHAR Dest;
    PCHAR Start;
    PPYOBJECT Self = NULL;
    PPYOBJECT FunctionNameObject;
    PPYOBJECT CodeObject = Function->CodeObject;
    PCPYCODEOBJECTOFFSETS CodeObjectOffsets = Python->PyCodeObjectOffsets;
    PSTRING Path;
    PSTRING FunctionName;
    PSTRING ClassName;
    PSTRING FullName;
    PSTRING ModuleName;
    PCHAR ClassNameBuffer = NULL;
    USHORT FullNameAllocSize;
    USHORT FullNameLength;
    PSTRING ParentModuleName;
    PSTRING ParentName;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

    //
    // Initialize aliases and set the PathEntry to invalid.
    //

    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)Function;
    ParentPathEntry = Function->ParentPathEntry;
    PathEntry->IsValid = FALSE;

    //
    // Resolve the first line number and function name object using the code
    // object offsets structure.
    //

    Function->FirstLineNumber = (USHORT)*(
        (PULONG)RtlOffsetToPointer(
            CodeObject,
            CodeObjectOffsets->FirstLineNumber
        )
    );

    FunctionNameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            CodeObjectOffsets->Name
        )
    );

    //
    // Wrap the PyString/PyUnicode string in a STRING struct.
    //

    Rtl = Python->Rtl;
    FunctionName = &PathEntry->Name;

    Success = WrapPythonStringAsString(Python,
                                       FunctionNameObject,
                                       FunctionName);

    if (!Success) {
        return FALSE;
    }

    //
    // Attempt to get the "self" object from the current frame, if present.
    //

    Success = GetSelf(Python, Function, FrameObject, &Self);

    if (!Success) {

        //
        // The GetSelf() method failed.  Note that this indicates an actual
        // failure -- not simply that we couldn't resolve the "self" object
        // from the frame.  We test for that below.
        //

        return FALSE;
    }

    //
    // Clear the class name string.
    //

    ClassName = &PathEntry->ClassName;
    ClearString(ClassName);

    if (Self) {

        //
        // We found a "self" object within the frame, which means we're an
        // instance object and we have a class.  So, let's try and extract
        // the class name.
        //

        USHORT ClassNameLength;

        Success = GetClassNameFromSelf(Python,
                                       Function,
                                       FrameObject,
                                       Self,
                                       FunctionName,
                                       &ClassNameBuffer);

        if (!Success) {
            return FALSE;
        }

        if (ClassNameBuffer) {

            //
            // We were able to extract a class name.  The C Python API will have
            // provided us with a pointer to a NULL-terminated C (char) string,
            // which we'll need to take a copy of, so, record the relevant
            // details here regarding length and buffer.
            //

            ClassNameLength = (USHORT)strlen((PCSZ)ClassNameBuffer);

            //
            // Ensure we've got a NUL-terminated string.
            //

            if (ClassNameBuffer[ClassNameLength] != '\0') {
                __debugbreak();
            }

            ClassName->Length = ClassNameLength;
            ClassName->MaximumLength = ClassNameLength;
            ClassName->Buffer = ClassNameBuffer;
        }
    }

    //
    // Initialize the module name, initially pointing at our parent's
    // buffer.
    //

    ParentModuleName = &ParentPathEntry->ModuleName;
    ModuleName = &PathEntry->ModuleName;
    ModuleName->Length = ParentModuleName->Length;
    ModuleName->MaximumLength = ParentModuleName->Length;
    ModuleName->Buffer = ParentModuleName->Buffer;

    ParentName = &ParentPathEntry->Name;

    //
    // Calculate the length of the full name.  The extra +1s are accounting
    // for joining slashes and the final trailing NUL.
    //

    FullNameLength = (
        (ModuleName->Length ? ModuleName->Length + 1 : 0) +
        (ParentName->Length ? ParentName->Length + 1 : 0) +
        (ClassName->Length  ? ClassName->Length  + 1 : 0) +
        FunctionName->Length                              +
        1
    );

    //
    // Ensure we don't exceed name lengths.
    //

    if (FullNameLength > MAX_STRING) {
        return FALSE;
    }

    FullNameAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength);

    if (FullNameAllocSize > MAX_STRING) {
        FullNameAllocSize = (USHORT)MAX_STRING;
    }

    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    FullName = &PathEntry->FullName;

    Success = AllocateStringBuffer(Python, FullNameAllocSize, FullName);
    if (!Success) {
        return FALSE;
    }

    Dest = FullName->Buffer;

    if (ModuleName->Length) {
        __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
        Dest += ModuleName->Length;
        *Dest++ = '\\';
    }

    //
    // Point the module name to the base of the full name buffer now that we've
    // created and copied the original buffer.
    //

    ModuleName->Buffer = FullName->Buffer;

    if (ParentName->Length) {
        __movsb(Dest, (PBYTE)ParentName->Buffer, ParentName->Length);
        Dest += ParentName->Length;
        *Dest++ = '\\';

        if (!ModuleName->Length) {

            //
            // There's no module name currently set, so the parent name
            // becomes our module name.  Inherit the length.
            //

            ModuleName->Length = ParentName->Length;
            ModuleName->MaximumLength = ParentName->Length;

        } else if (ParentPathEntry->IsFile) {

            //
            // The parent is a file, so the module name will be a concatenation
            // of the parent name plus the module name.  Update the module name
            // length to account for the parent name, plus one for the joining
            // slash.
            //

            if (ParentPathEntry->IsFile) {
                ModuleName->Length += ParentName->Length + 1;
                ModuleName->MaximumLength = ModuleName->Length;
            }
        }
    }

    if (ClassName->Length) {
        Start = Dest;
        __movsb(Dest, (PBYTE)ClassName->Buffer, ClassName->Length);
        ClassName->Buffer = Start;
        Dest += ClassName->Length;
        *Dest++ = '\\';
    }

    Start = Dest;
    __movsb(Dest, (PBYTE)FunctionName->Buffer, FunctionName->Length);
    FunctionName->Buffer = Start;
    Dest += FunctionName->Length;
    *Dest++ = '\0';

    //
    // Omit trailing NUL from Length.
    //

    FullName->Length = FullNameLength - 1;
    FullName->MaximumLength = FullNameAllocSize;

    //
    // Point our path at our parent.
    //

    Path = &PathEntry->Path;
    Path->Length = ParentPathEntry->Path.Length;
    Path->MaximumLength = Path->Length;
    Path->Buffer = ParentPathEntry->Path.Buffer;

    PathEntry->IsFunction = TRUE;

    //
    // Resolve line numbers.  This will set the FirstLineNumber and
    // NumberOfCodeLines fields in the Function structure.
    //

    ResolveLineNumbers(Python, Function);

    PathEntry->IsValid = TRUE;

    //
    // Initialize the call count.
    //

    Function->CallCount = 1;

    //
    // Calculate the code object's hash.
    //

    Function->CodeObjectHash = Python->PyObject_Hash(CodeObject);

    //
    // Hash the strings.
    //

    HashString(Python, &PathEntry->Name);
    HashString(Python, &PathEntry->Path);
    HashString(Python, &PathEntry->FullName);
    HashString(Python, &PathEntry->ModuleName);

    if (ClassName->Length) {
        HashString(Python, &PathEntry->ClassName);
    } else {
        PathEntry->ClassName.Hash = 0;
    }

    //
    // Calculate a final hash value and use that as the function signature.
    //

    Function->Signature = (ULONG_PTR)(
        PathEntry->PathEntryType    ^
        PathEntry->PathHash         ^
        PathEntry->FullNameHash     ^
        Function->CodeObjectHash    ^
        Function->NumberOfCodeLines
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
GetSelf(
    PPYTHON Python,
    PPYTHON_FUNCTION Function,
    PPYFRAMEOBJECT FrameObject,
    PPPYOBJECT SelfPointer
    )
/*++

Routine Description:

    This routine attempts to extract the Python "self" object from a frame
    object.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure that has been
        created for the frame object.

    FrameObject - Supplies a pointer to the PYFRAMEOBJECT structure for which
        the "self" object is to be extracted from.

    SelfPointer - Supplies a pointer to a variable that receives the address
        of a PYOBJECT structure representing the "self" variable if one could
        be found.  If no "self" variable could be found, this is set to NULL.

Return Value:

    TRUE on success, FALSE on failure.  Note that TRUE doesn't necessarily mean
    the self variable was found; check the value of SelfPointer for that.

--*/
{
    BOOL Success = FALSE;
    PPYOBJECT Self = NULL;
    PPYOBJECT Locals = FrameObject->Locals;
    PPYOBJECT CodeObject = Function->CodeObject;

    LONG ArgumentCount;
    PPYTUPLEOBJECT ArgumentNames;

    //
    // Attempt to resolve self from the locals dictionary.
    //

    if (Locals && Locals->Type == Python->PyDict.Type) {
        if ((Self = Python->PyDict_GetItemString(Locals, SELF_A.Buffer))) {
            Success = TRUE;
            goto End;
        }
    }

    //
    // If that didn't work, see if the first argument's name was "self", and
    // if so, use that.
    //

    ArgumentCount = *(
        (PLONG)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->ArgumentCount
        )
    );

    ArgumentNames = *(
        (PPPYTUPLEOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->LocalVariableNames
        )
    );

    if (ArgumentCount != 0 && ArgumentNames->Type == Python->PyTuple.Type) {
        PRTL_EQUAL_STRING EqualString = Python->Rtl->RtlEqualString;
        STRING ArgumentName;

        Success = WrapPythonStringAsString(
            Python,
            ArgumentNames->Item[0],
            &ArgumentName
        );

        if (!Success) {
            return FALSE;
        }

        if (EqualString(&ArgumentName, &SELF_A, FALSE)) {
            Self = *(
                (PPPYOBJECT)RtlOffsetToPointer(
                    FrameObject,
                    Python->PyFrameObjectOffsets->LocalsPlusStack
                )
            );
        }
    }

    Success = TRUE;

End:
    *SelfPointer = Self;
    return Success;
}


_Use_decl_annotations_
BOOL
GetClassNameFromSelf(
    PPYTHON Python,
    PPYTHON_FUNCTION Function,
    PPYFRAMEOBJECT FrameObject,
    PPYOBJECT Self,
    PSTRING FunctionName,
    PPCHAR ClassNameBuffer
    )
/*++

Routine Description:

    This routine extracts a class name from a Python "self" object.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure for the frame
        object containing the self object for which the class name will be
        extracted.

    FrameObject - Supplies a pointer to the PYFRAMEOBJECT structure where
        the "self" object resides.

    Self - Supplies a pointer to a PYOBJECT structure representing the self
        object for the current frame.  Derived by GetSelf().

    FunctionName - Supplies a pointer to a STRING structure representing the
        name of the function.

    ClassNameBuffer - Supplies a pointer to a variable that receives the
        address of the NULL-terminated character array representing the class
        name.

Return Value:

    TRUE on success, FALSE on failure.  Note that TRUE doesn't necessarily mean
    the class name was found, check the value of ClassNameBuffer for that.

--*/
{
    BOOL Success;
    ULONG Index;
    PPYTUPLEOBJECT Mro;
    PPYTYPEOBJECT TypeObject;
    PPYOBJECT TypeDict;
    PPYOBJECT CodeObject;
    PYOBJECTEX Func;
    PPYDICT_GETITEMSTRING PyDict_GetItemString;
    PCHAR FuncName;

    if (Python->PyInstance.Type && Self->Type == Python->PyInstance.Type) {
        STRING Class;
        PPYINSTANCEOBJECT Instance = (PPYINSTANCEOBJECT)Self;
        PPYOBJECT ClassNameObject = Instance->OldStyleClass->Name;

        Success = WrapPythonStringAsString(Python, ClassNameObject, &Class);
        if (!Success) {
            return FALSE;
        }

        *ClassNameBuffer = Class.Buffer;
        return TRUE;
    }

    Mro = (PPYTUPLEOBJECT)Self->Type->MethodResolutionOrder;
    if (!Mro || Mro->Type != Python->PyTuple.Type) {

        //
        // We should always have an MRO for new-style classes.
        //

        return FALSE;
    }

    FuncName = FunctionName->Buffer;
    CodeObject = FrameObject->Code;
    PyDict_GetItemString = Python->PyDict_GetItemString;

    //
    // Walk the MRO looking for our method.
    //

    for (Index = 0; Index < Mro->ObjectSize; Index++) {

        TypeObject = (PPYTYPEOBJECT)Mro->Item[Index];

        if (TypeObject->Type != Python->PyType.Type) {
            continue;
        }

        TypeDict = TypeObject->Dict;
        if (TypeDict->Type != Python->PyDict.Type) {
            continue;
        }

        Func.Object = PyDict_GetItemString(TypeDict, FuncName);

        if (!Func.Object || Func.Object->Type != Python->PyFunction.Type) {
            continue;
        }

        if (Func.Function->Code != CodeObject) {
            continue;
        }

        //
        // We've found the function in the MRO whose code object matches the
        // code object of our frame, so use the class name of this type.
        //

        *ClassNameBuffer = (PCHAR)TypeObject->Name;
        return TRUE;
    }

    //
    // We walked the MRO but couldn't find our method.  Clear the caller's
    // class name buffer pointer and return TRUE.
    //

    *ClassNameBuffer = NULL;
    return TRUE;
}

_Use_decl_annotations_
BOOL
RegisterPyCFunction(
    PPYTHON Python,
    PPYTHON_FUNCTION Function,
    PPYFRAMEOBJECT FrameObject
    )
/*++

Routine Description:

    This method is responsible for finalizing details about a C function, such
    as the function name and module name.  A frame object is provided to
    assist with resolution of names.

    It is called once per function after a new PYTHON_PATH_TABLE_ENTRY has been
    inserted into the path prefix table.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure to be
        registered by this routine.

    FrameObject - Supplies a pointer to the PYFRAMEOBJECT structure for which
        this function is being registered.  This is required in order to assist
        with the resolution of names.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL Is2;
    BOOL Is3;
    BOOL IsBuiltin = FALSE;

    CHAR Char;

    USHORT Index;
    USHORT NameLength;
    USHORT ClassNameLength;

    PRTL Rtl;
    PCHAR Dest;
    PCHAR Start;
    PCHAR NameBuffer = NULL;
    PCSZ ClassNameBuffer = NULL;
    PCSZ ModuleNameBuffer = NULL;
    PPYOBJECT SelfObject;
    PPYOBJECT ModuleNameObject;
    PPYMETHODDEF MethodDef;
    PPYCFUNCTIONOBJECT PyCFunctionObject;

    PSTRING Path;
    PSTRING FunctionName;
    PSTRING ClassName;
    PSTRING FullName;
    PSTRING ModuleName;
    USHORT FullNameAllocSize;
    USHORT FullNameLength;
    PSTRING ParentModuleName;
    PSTRING ParentName = NULL;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;
    PRTL_EQUAL_STRING EqualString;

    //
    // Initialize local variables.
    //

    Rtl = Python->Rtl;
    PyCFunctionObject = Function->PyCFunctionObject;
    MethodDef = PyCFunctionObject->MethodDef;
    ModuleNameObject = PyCFunctionObject->Module;
    SelfObject = PyCFunctionObject->Self;
    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)Function;
    ParentPathEntry = Function->ParentPathEntry;
    EqualString = Python->Rtl->RtlEqualString;
    ClassName = &PathEntry->ClassName;
    ModuleName = &PathEntry->ModuleName;
    FunctionName = &PathEntry->Name;
    Is2 = (BOOL)(Python->MajorVersion == 2);
    Is3 = (BOOL)(Python->MajorVersion == 3);

    //
    // Resolve the function name.
    //

    NameBuffer = MethodDef->Name;
    NameLength = (USHORT)strlen((PCSZ)NameBuffer);

    //
    // Ensure we've got a valid function name.
    //

    if (!NameLength || !NameBuffer) {
        __debugbreak();
    }

    if (ModuleNameObject) {

        //
        // The PyCFunction provides a module object.  Use this to derive the
        // module name.
        //

        Success = WrapPythonStringAsString(Python,
                                           ModuleNameObject,
                                           ModuleName);

        if (!Success) {
            __debugbreak();
            return FALSE;
        }

        //
        // See if the module name is a builtin.
        //

        PathEntry->IsBuiltin = (
            (Is2 && EqualString(ModuleName, &__BUILTIN__A, FALSE)) ||
            (Is3 && EqualString(ModuleName, &BUILTINS_A, FALSE))
        );
    }

    if (SelfObject) {

        if (SelfObject->Type == Python->PyModule.Type) {
            __debugbreak();
        }

        if (SelfObject->Type != Python->PyModule.Type) {

            ClassNameBuffer = SelfObject->Type->Name;

            if (ClassNameBuffer) {

                //
                // We were able to extract a class name.  The C Python API will
                // have provided us with a pointer to a NULL-terminated C (char)
                // string, which we'll need to take a copy of, so, record the
                // relevant details here regarding length and buffer.
                //

                ClassNameLength = (USHORT)strlen((PCSZ)ClassNameBuffer);

                //
                // Ensure we've got a NUL-terminated string.
                //

                if (ClassNameBuffer[ClassNameLength] != '\0') {
                    __debugbreak();
                }

                ClassName->Length = ClassNameLength;
                ClassName->MaximumLength = ClassNameLength;
                ClassName->Buffer = (PCHAR)ClassNameBuffer;
            }
        }
    }

    if (!ModuleName->Length && !ClassName->Length) {
        ParentModuleName = &ParentPathEntry->ModuleName;
        if (ParentModuleName->Length != 0) {
            __debugbreak();
        }

        ParentName = &ParentPathEntry->Name;

        if (ParentName->Length == 0) {
            __debugbreak();
        }
    }

    //
    // Calculate the length of the full name.  The extra +1s are accounting
    // for joining slashes and the final trailing NUL.
    //

    FullNameLength = (
        (ParentName != NULL ? ParentName->Length + 1 : 0) +
        (ModuleName->Length ? ModuleName->Length + 1 : 0) +
        (ClassName->Length  ? ClassName->Length  + 1 : 0) +
        NameLength                                        +
        1
    );

    //
    // Ensure we don't exceed name lengths.
    //

    if (FullNameLength > MAX_STRING) {
        return FALSE;
    }

    FullNameAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength);

    if (FullNameAllocSize > MAX_STRING) {
        FullNameAllocSize = (USHORT)MAX_STRING;
    }

    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    FullName = &PathEntry->FullName;

    Success = AllocateStringBuffer(Python, FullNameAllocSize, FullName);
    if (!Success) {
        return FALSE;
    }

    Dest = FullName->Buffer;

    //
    // Copy the parent name if it's present.
    //

    if (ParentName) {
        __movsb(Dest, (PBYTE)ParentName->Buffer, ParentName->Length);
        Dest += ParentName->Length;
        *Dest++ = '\\';
    }

    //
    // Copy the module name if present, replacing dots with backslashes.
    //

    if (ModuleName->Length) {

        for (Index = 0; Index < ModuleName->Length; Index++) {
            Char = ModuleName->Buffer[Index];
            Dest[Index] = (Char == '.' ? '\\' : Char);
        }

        //
        // Point the module name to the base of the full name buffer now that
        // we've created and copied the original buffer.
        //

        ModuleName->Buffer = FullName->Buffer;

        //
        // Bump the destination pointer and add a joining backslash.
        //

        Dest += ModuleName->Length;
        *Dest++ = '\\';
    }

    //
    // Copy the class name if present, replacing dots with backslashes.
    //

    if (ClassName->Length) {

        Start = Dest;

        for (Index = 0; Index < ClassName->Length; Index++) {
            Char = ClassName->Buffer[Index];
            Dest[Index] = (Char == '.' ? '\\' : Char);
        }

        //
        // Point the class name at our new buffer.
        //

        ClassName->Buffer = Start;

        //
        // Bump the destination pointer and add a joining backslash.
        //

        Dest += ClassName->Length;
        *Dest++ = '\\';
    }

    //
    // Copy the function name.
    //

    Start = Dest;
    __movsb(Dest, (PBYTE)NameBuffer, NameLength);
    FunctionName->Length = NameLength;
    FunctionName->MaximumLength = NameLength;
    FunctionName->Buffer = Start;
    Dest += FunctionName->Length;
    *Dest++ = '\0';

    //
    // Omit trailing NUL from Length.
    //

    FullName->Length = FullNameLength - 1;
    FullName->MaximumLength = FullNameAllocSize;

    //
    // Sanity check we've got a trailing NUL where we expect, and non-NUL
    // characters both before that, and at the start of the string.
    //

    if (FullName->Buffer[FullName->Length] != '\0' ||
        FullName->Buffer[FullName->Length-1] == '\0' ||
        FullName->Buffer[0] == '\0') {
        __debugbreak();
    }

    //
    // Point our path at our parent.
    //

    Path = &PathEntry->Path;
    Path->Length = ParentPathEntry->Path.Length;
    Path->MaximumLength = Path->Length;
    Path->Buffer = ParentPathEntry->Path.Buffer;

    //
    // Finalize the path entry.
    //

    PathEntry->IsFunction = TRUE;
    PathEntry->IsValid = TRUE;
    PathEntry->IsC = TRUE;

    //
    // Initialize the call count.
    //

    Function->CallCount = 1;

    //
    // Return success.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
