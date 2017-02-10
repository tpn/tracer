/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerTraceStoreRelocations.c

Abstract:

    This module is responsible for defining trace store relocation information
    for the PythonTracer component.  For each structure stored in a trace store,
    relocation fields are defined for pointers within the structures such that
    they can be re-mapped when a read-only trace store can't be mapped at its
    preferred base addresses.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// PYTHON_FUNCTION_TABLE relocations.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOC PythonFunctionTableRelocations[] = {

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, TableRoot),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, InsertOrderFlink),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, InsertOrderBlink),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, OrderedPointer),
        TraceStorePythonFunctionTableEntryId
    },


    //
    // Null-out the routine function pointers and the table context.
    //

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, CompareRoutine),
        TraceStoreNullId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, AllocateRoutine),
        TraceStoreNullId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, FreeRoutine),
        TraceStoreNullId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE, TableContext),
        TraceStoreNullId,
    },

    LAST_TRACE_STORE_FIELD_RELOC

};

//
// PYTHON_FUNCTION_TABLE_ENTRY relocations.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOC PythonFunctionTableEntryRelocations[] = {

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, Parent),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, LeftChild),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, RightChild),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, Flink),
        TraceStorePythonFunctionTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, Blink),
        TraceStorePythonFunctionTableEntryId
    },

    //
    // Define some helper macros that simplify the field offset calculation
    // through one or more embedded structures.
    //

#define PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(Name)          \
    FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, PythonFunction) + \
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Name)

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(NextPrefixTree),
        TraceStorePythonPathTableEntryId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(Parent),
        TraceStorePythonPathTableEntryId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(LeftChild),
        TraceStorePythonPathTableEntryId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(RightChild),
        TraceStorePythonPathTableEntryId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(Prefix),
        TraceStorePythonPathTableEntryId
    },

    //
    // String buffer pointers.
    //

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(PathBuffer),
        TraceStoreStringBufferId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(FullNameBuffer),
        TraceStoreStringBufferId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(ModuleNameBuffer),
        TraceStoreStringBufferId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(NameBuffer),
        TraceStoreStringBufferId
    },

    {
        PATH_TABLE_ENTRY_OFFSET_THROUGH_FUNCTION(ClassNameBuffer),
        TraceStoreStringBufferId
    },

#define OFFSET_THROUGH_FUNCTION(Name)                           \
    FIELD_OFFSET(PYTHON_FUNCTION_TABLE_ENTRY, PythonFunction) + \
    FIELD_OFFSET(PYTHON_FUNCTION, Name)

    {
        OFFSET_THROUGH_FUNCTION(ParentPathEntry),
        TraceStorePythonPathTableEntryId
    },

    LAST_TRACE_STORE_FIELD_RELOC

};

//
// PYTHON_PATH_TABLE relocations.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOC PythonPathTableRelocations[] = {

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE, NextPrefixTree),
        TraceStorePythonPathTableEntryId
    },

    LAST_TRACE_STORE_FIELD_RELOC

};

//
// PYTHON_PATH_TABLE_ENTRY relocations.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOC PythonPathTableEntryRelocations[] = {

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NextPrefixTree),
        TraceStorePythonPathTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Parent),
        TraceStorePythonPathTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, LeftChild),
        TraceStorePythonPathTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, RightChild),
        TraceStorePythonPathTableEntryId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Prefix),
        TraceStorePythonPathTableEntryId
    },

    //
    // String buffer pointers.
    //

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathBuffer),
        TraceStoreStringBufferId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameBuffer),
        TraceStoreStringBufferId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameBuffer),
        TraceStoreStringBufferId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameBuffer),
        TraceStoreStringBufferId
    },

    {
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameBuffer),
        TraceStoreStringBufferId
    },

    LAST_TRACE_STORE_FIELD_RELOC

};

//
// PYTHON_TRACE_EVENT relocations.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOC PythonTraceEvent1Relocations[] = {

    {
        FIELD_OFFSET(PYTHON_TRACE_EVENT1, Function),
        TraceStorePythonFunctionTableEntryId
    },

    LAST_TRACE_STORE_FIELD_RELOC
};

//
// Container structure for each individual relocation structure.
//

DECLSPEC_ALIGN(128)
TRACE_STORE_FIELD_RELOCS PythonTracerTraceStoreRelocations[] = {

    {
        TraceStorePythonFunctionTableId,
        (PTRACE_STORE_FIELD_RELOC)&PythonFunctionTableRelocations
    },

    {
        TraceStorePythonFunctionTableEntryId,
        (PTRACE_STORE_FIELD_RELOC)&PythonFunctionTableEntryRelocations
    },

    {
        TraceStorePythonPathTableId,
        (PTRACE_STORE_FIELD_RELOC)&PythonPathTableRelocations
    },

    {
        TraceStorePythonPathTableEntryId,
        (PTRACE_STORE_FIELD_RELOC)&PythonPathTableEntryRelocations
    },

    {
        TraceStoreEventId,
        (PTRACE_STORE_FIELD_RELOC)&PythonTraceEvent1Relocations
    },

    LAST_TRACE_STORE_FIELD_RELOCS

};


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
