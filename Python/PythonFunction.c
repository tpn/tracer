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
    LONG EventType,
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

    EventType - Supplies a LONG value representing the trace event type.
        Currently unused.

    ArgObject - Supplies a pointer to a PYOBJECT structure that was provided as
        a parameter to the trace function.  Currently unused.

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

    PRTL Rtl;
    PPYFRAMEOBJECT Frame = (PPYFRAMEOBJECT)FrameObject;
    PPYOBJECT CodeObject;

    PYTHON_FUNCTION FunctionRecord;
    PPYTHON_FUNCTION Function;
    PPYTHON_FUNCTION_TABLE FunctionTable;
    BOOLEAN NewFunction;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

    //
    // Clear the caller's function pointer up front if present.
    //

    if (ARGUMENT_PRESENT(FunctionPointer)) {
        *FunctionPointer = NULL;
    }

    CodeObject = Frame->Code;

    if (CodeObject->Type != Python->PyCode.Type) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    FunctionRecord.CodeObject = CodeObject;

    FunctionTable = Python->FunctionTable;

    Function = Rtl->RtlInsertElementGenericTable(
        &FunctionTable->GenericTable,
        &FunctionRecord,
        sizeof(FunctionRecord),
        &NewFunction
    );

    if (!NewFunction) {

        //
        // We've already seen this function.  Increment the reference count if
        // it's a valid function.
        //

        IsValid = IsValidFunction(Function);

        if (IsValid) {
            Function->ReferenceCount++;
        }

        goto End;
    }

    //
    // This is a new function; attempt to register the underlying filename for
    // this frame.
    //

    Success = GetPathEntryFromFrame(Python,
                                    FrameObject,
                                    EventType,
                                    ArgObject,
                                    &ParentPathEntry);

    if (!Success || !ParentPathEntry || !ParentPathEntry->IsValid) {
        goto Error;
    }

    Function->ParentPathEntry = ParentPathEntry;
    Function->CodeObject = CodeObject;

    Success = RegisterFunction(Python,
                               Function,
                               FrameObject);

    if (Success) {
        IsValid = TRUE;

        //
        // Increment the reference count of the code object so that
        // we can keep it alive during tracing (given that it is our
        // key into the FunctionTable).
        //

        CodeObject->ReferenceCount++;

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
RegisterFunction(
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

        if (!ClassNameBuffer) {
            return FALSE;
        }

        //
        // We were able to extract a class name.  The C Python API will have
        // provided us with a pointer to a NULL-terminated C (char) string,
        // which we'll need to take a copy of, so, record the relevant details
        // here regarding length and buffer.
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
        ModuleName->Length                                +
        1                                                 +
        (ParentName->Length ? ParentName->Length + 1 : 0) +
        (ClassName->Length ? ClassName->Length + 1 : 0)   +
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

    __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
    Dest += ModuleName->Length;
    ModuleName->Buffer = FullName->Buffer;

    *Dest++ = '\\';

    if (ParentName->Length) {
        __movsb(Dest, (PBYTE)ParentName->Buffer, ParentName->Length);
        Dest += ParentName->Length;
        *Dest++ = '\\';

        //
        // If the parent is a file, update the module name to account
        // for the parent's module name, plus the joining slash.
        //

        if (ParentPathEntry->IsFile) {
            ModuleName->Length += ParentName->Length + 1;
            ModuleName->MaximumLength = ModuleName->Length;
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
    // Calculate a final hash value.
    //

    Function->FunctionHash = (
        PathEntry->PathEntryType    ^
        PathEntry->PathHash         ^
        PathEntry->FullNameHash     ^
        Function->CodeObjectHash    ^
        Function->NumberOfCodeLines
    );

    //
    // Initialize the reference count.
    //

    Function->ReferenceCount = 1;

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
        if ((Self = Python->PyDict_GetItemString(Locals, SELFA.Buffer))) {
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

        if (EqualString(&ArgumentName, &SELFA, FALSE)) {
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

    TRUE on success, FALSE on failure.

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

    return FALSE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
