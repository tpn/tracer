/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonConstants.c

Abstract:

    This module defines constants used by the Python component.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

CONST USHORT TargetSizeOfPythonPathTableEntry = 128;
CONST USHORT TargetSizeOfPythonFunctionTableEntry = 256;

CONST UNICODE_STRING W__init__py  = RTL_CONSTANT_STRING(L"__init__.py");
CONST UNICODE_STRING W__init__pyc = RTL_CONSTANT_STRING(L"__init__.pyc");
CONST UNICODE_STRING W__init__pyo = RTL_CONSTANT_STRING(L"__init__.pyo");

CONST PUNICODE_STRING InitPyFilesW[3] = {
    (PUNICODE_STRING)&W__init__py,
    (PUNICODE_STRING)&W__init__pyc,
    (PUNICODE_STRING)&W__init__pyo
};

CONST STRING A__init__py  = RTL_CONSTANT_STRING("__init__.py");
CONST STRING A__init__pyc = RTL_CONSTANT_STRING("__init__.pyc");
CONST STRING A__init__pyo = RTL_CONSTANT_STRING("__init__.pyo");

CONST PUNICODE_STRING InitPyFilesA[3] = {
    (PUNICODE_STRING)&A__init__py,
    (PUNICODE_STRING)&A__init__pyc,
    (PUNICODE_STRING)&A__init__pyo
};

CONST USHORT NumberOfInitPyFiles = (
    sizeof(InitPyFilesA) /
    sizeof(InitPyFilesA[0])
);

CONST STRING SELFA = RTL_CONSTANT_STRING("self");

CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets25_27 = {
    FIELD_OFFSET(PYCODEOBJECT25_27, ArgumentCount),
    0, // KeywordOnlyArgCount
    FIELD_OFFSET(PYCODEOBJECT25_27, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT25_27, StackSize),
    FIELD_OFFSET(PYCODEOBJECT25_27, Flags),
    FIELD_OFFSET(PYCODEOBJECT25_27, Code),
    FIELD_OFFSET(PYCODEOBJECT25_27, Constants),
    FIELD_OFFSET(PYCODEOBJECT25_27, Names),
    FIELD_OFFSET(PYCODEOBJECT25_27, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, Filename),
    FIELD_OFFSET(PYCODEOBJECT25_27, Name),
    FIELD_OFFSET(PYCODEOBJECT25_27, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT25_27, LineNumberTable),
    0 // ZombieFrame
};

CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets30_32 = {
    FIELD_OFFSET(PYCODEOBJECT30_32, ArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT30_32, KeywordOnlyArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT30_32, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT30_32, StackSize),
    FIELD_OFFSET(PYCODEOBJECT30_32, Flags),
    FIELD_OFFSET(PYCODEOBJECT30_32, Code),
    FIELD_OFFSET(PYCODEOBJECT30_32, Constants),
    FIELD_OFFSET(PYCODEOBJECT30_32, Names),
    FIELD_OFFSET(PYCODEOBJECT30_32, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, Filename),
    FIELD_OFFSET(PYCODEOBJECT30_32, Name),
    FIELD_OFFSET(PYCODEOBJECT30_32, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT30_32, LineNumberTable),
    0 // ZombieFrame
};

CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets33_35 = {
    FIELD_OFFSET(PYCODEOBJECT33_35, ArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT33_35, KeywordOnlyArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT33_35, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT33_35, StackSize),
    FIELD_OFFSET(PYCODEOBJECT33_35, Flags),
    FIELD_OFFSET(PYCODEOBJECT33_35, Code),
    FIELD_OFFSET(PYCODEOBJECT33_35, Constants),
    FIELD_OFFSET(PYCODEOBJECT33_35, Names),
    FIELD_OFFSET(PYCODEOBJECT33_35, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, Filename),
    FIELD_OFFSET(PYCODEOBJECT33_35, Name),
    FIELD_OFFSET(PYCODEOBJECT33_35, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT33_35, LineNumberTable),
    FIELD_OFFSET(PYCODEOBJECT33_35, ZombieFrame),
};

CONST PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets25_33 = {
    FIELD_OFFSET(PYFRAMEOBJECT25_33, ThreadState),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LastInstruction),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LineNumber),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, BlockIndex),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, BlockStack),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LocalsPlusStack),
    0, // Generator
    0  // StillExecuting
};

CONST PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets34_35 = {
    0, // ThreadState
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LastInstruction),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LineNumber),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, BlockIndex),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, BlockStack),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LocalsPlusStack),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, Generator),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, StillExecuting)
};

CONST PYTHON_PATH_TABLE_ENTRY_OFFSETS PythonPathTableEntryOffsets = {
    sizeof(PYTHON_PATH_TABLE_ENTRY),
    (sizeof(PYTHON_PATH_TABLE_ENTRY_OFFSETS) / sizeof(USHORT)) - 2,

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NodeTypeCode),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PrefixNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathEntryType),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NextPrefixTree),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Links),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Parent),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, LeftChild),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, RightChild),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Prefix),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Path),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Name),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameBuffer),
};

CONST PYTHON_FUNCTION_OFFSETS PythonFunctionOffsets = {
    sizeof(PYTHON_FUNCTION),
    (sizeof(PYTHON_FUNCTION_OFFSETS) / sizeof(USHORT)) - 2,

    FIELD_OFFSET(PYTHON_FUNCTION, PathEntry),
    FIELD_OFFSET(PYTHON_FUNCTION, ParentPathEntry),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeObject),

    FIELD_OFFSET(PYTHON_FUNCTION, LineNumbersBitmap),
    FIELD_OFFSET(PYTHON_FUNCTION, Histogram),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeLineNumbers),

    FIELD_OFFSET(PYTHON_FUNCTION, ReferenceCount),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeObjectHash),
    FIELD_OFFSET(PYTHON_FUNCTION, FunctionHash),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused1),

    FIELD_OFFSET(PYTHON_FUNCTION, FirstLineNumber),
    FIELD_OFFSET(PYTHON_FUNCTION, NumberOfLines),
    FIELD_OFFSET(PYTHON_FUNCTION, NumberOfCodeLines),
    FIELD_OFFSET(PYTHON_FUNCTION, SizeOfByteCode),

    FIELD_OFFSET(PYTHON_FUNCTION, Unused2),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused3),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused4),
};


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
