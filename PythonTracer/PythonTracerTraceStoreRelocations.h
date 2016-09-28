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

PTRACE_STORE_FIELD_RELOC PythonFunctionTableRelocations;
PTRACE_STORE_FIELD_RELOC PythonFunctionTableEntryRelocations;
TRACE_STORE_FIELD_RELOCS PythonTracerTraceStoreRelocations;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
