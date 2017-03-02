/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Rtl.c

Abstract:

    This module provides implementations for most Rtl (Run-time Library)
    routines.

--*/

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
#pragma warning(disable: 4028 4273 28251)

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

_Use_decl_annotations_
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
    // mapping a file on a network drive, then having the network fail,
    // or running out of disk space on the volume).  Catch such exceptions
    // and return NULL.
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

    TRY_MAPPED_MEMORY_OP {

        for (Index = 0; Index < NumberOfPages; Index++) {
            PrefaultPage(Pointer);
            Pointer += PAGE_SIZE;
        }

    } CATCH_STATUS_IN_PAGE_ERROR {
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
LoadDbg(
    _In_ HMODULE DbgHelpModule,
    _In_ PDBG Dbg
    )
{
    if (!ARGUMENT_PRESENT(DbgHelpModule)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Dbg)) {
        return FALSE;
    }

    //
    // Start of auto-generated section.
    //

    if (!(Dbg->EnumDirTree = (PENUM_DIR_TREE)
        GetProcAddress(DbgHelpModule, "EnumDirTree"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumDirTree'");
    }

    if (!(Dbg->EnumDirTreeW = (PENUM_DIR_TREE_W)
        GetProcAddress(DbgHelpModule, "EnumDirTreeW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumDirTreeW'");
    }

    if (!(Dbg->EnumerateLoadedModules64 = (PENUMERATE_LOADED_MODULES64)
        GetProcAddress(DbgHelpModule, "EnumerateLoadedModules64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumerateLoadedModules64'");
    }

    if (!(Dbg->EnumerateLoadedModules = (PENUMERATE_LOADED_MODULES)
        GetProcAddress(DbgHelpModule, "EnumerateLoadedModules"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumerateLoadedModules'");
    }

    if (!(Dbg->EnumerateLoadedModulesEx = (PENUMERATE_LOADED_MODULES_EX)
        GetProcAddress(DbgHelpModule, "EnumerateLoadedModulesEx"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumerateLoadedModulesEx'");
    }

    if (!(Dbg->EnumerateLoadedModulesExW = (PENUMERATE_LOADED_MODULES_EX_W)
        GetProcAddress(DbgHelpModule, "EnumerateLoadedModulesExW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumerateLoadedModulesExW'");
    }

    if (!(Dbg->EnumerateLoadedModulesW64 = (PENUMERATE_LOADED_MODULES_W64)
        GetProcAddress(DbgHelpModule, "EnumerateLoadedModulesW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'EnumerateLoadedModulesW64'");
    }

    if (!(Dbg->FindFileInPath = (PFIND_FILE_IN_PATH)
        GetProcAddress(DbgHelpModule, "FindFileInPath"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'FindFileInPath'");
    }

    if (!(Dbg->FindFileInSearchPath = (PFIND_FILE_IN_SEARCH_PATH)
        GetProcAddress(DbgHelpModule, "FindFileInSearchPath"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'FindFileInSearchPath'");
    }

    if (!(Dbg->GetSymLoadError = (PGET_SYM_LOAD_ERROR)
        GetProcAddress(DbgHelpModule, "GetSymLoadError"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'GetSymLoadError'");
    }

    if (!(Dbg->GetTimestampForLoadedLibrary = (PGET_TIMESTAMP_FOR_LOADED_LIBRARY)
        GetProcAddress(DbgHelpModule, "GetTimestampForLoadedLibrary"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'GetTimestampForLoadedLibrary'");
    }

    if (!(Dbg->MakeSureDirectoryPathExists = (PMAKE_SURE_DIRECTORY_PATH_EXISTS)
        GetProcAddress(DbgHelpModule, "MakeSureDirectoryPathExists"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'MakeSureDirectoryPathExists'");
    }

    if (!(Dbg->RangeMapAddPeImageSections = (PRANGE_MAP_ADD_PE_IMAGE_SECTIONS)
        GetProcAddress(DbgHelpModule, "RangeMapAddPeImageSections"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'RangeMapAddPeImageSections'");
    }

    if (!(Dbg->RangeMapRead = (PRANGE_MAP_READ)
        GetProcAddress(DbgHelpModule, "RangeMapRead"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'RangeMapRead'");
    }

    if (!(Dbg->RangeMapRemove = (PRANGE_MAP_REMOVE)
        GetProcAddress(DbgHelpModule, "RangeMapRemove"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'RangeMapRemove'");
    }

    if (!(Dbg->RangeMapWrite = (PRANGE_MAP_WRITE)
        GetProcAddress(DbgHelpModule, "RangeMapWrite"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'RangeMapWrite'");
    }

    if (!(Dbg->ReportSymbolLoadSummary = (PREPORT_SYMBOL_LOAD_SUMMARY)
        GetProcAddress(DbgHelpModule, "ReportSymbolLoadSummary"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'ReportSymbolLoadSummary'");
    }

    if (!(Dbg->SearchTreeForFile = (PSEARCH_TREE_FOR_FILE)
        GetProcAddress(DbgHelpModule, "SearchTreeForFile"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SearchTreeForFile'");
    }

    if (!(Dbg->SearchTreeForFileW = (PSEARCH_TREE_FOR_FILE_W)
        GetProcAddress(DbgHelpModule, "SearchTreeForFileW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SearchTreeForFileW'");
    }

    if (!(Dbg->StackWalk64 = (PSTACK_WALK64)
        GetProcAddress(DbgHelpModule, "StackWalk64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'StackWalk64'");
    }

    if (!(Dbg->StackWalkEx = (PSTACK_WALK_EX)
        GetProcAddress(DbgHelpModule, "StackWalkEx"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'StackWalkEx'");
    }

    if (!(Dbg->StackWalk = (PSTACK_WALK)
        GetProcAddress(DbgHelpModule, "StackWalk"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'StackWalk'");
    }

    if (!(Dbg->SymAddrIncludeInlineTrace = (PSYM_ADDR_INCLUDE_INLINE_TRACE)
        GetProcAddress(DbgHelpModule, "SymAddrIncludeInlineTrace"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddrIncludeInlineTrace'");
    }

    if (!(Dbg->SymAddSourceStreamA = (PSYM_ADD_SOURCE_STREAM_A)
        GetProcAddress(DbgHelpModule, "SymAddSourceStreamA"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddSourceStreamA'");
    }

    if (!(Dbg->SymAddSourceStream = (PSYM_ADD_SOURCE_STREAM)
        GetProcAddress(DbgHelpModule, "SymAddSourceStream"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddSourceStream'");
    }

    if (!(Dbg->SymAddSourceStreamW = (PSYM_ADD_SOURCE_STREAM_W)
        GetProcAddress(DbgHelpModule, "SymAddSourceStreamW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddSourceStreamW'");
    }

    if (!(Dbg->SymAddSymbol = (PSYM_ADD_SYMBOL)
        GetProcAddress(DbgHelpModule, "SymAddSymbol"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddSymbol'");
    }

    if (!(Dbg->SymAddSymbolW = (PSYM_ADD_SYMBOL_W)
        GetProcAddress(DbgHelpModule, "SymAddSymbolW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymAddSymbolW'");
    }

    if (!(Dbg->SymCleanup = (PSYM_CLEANUP)
        GetProcAddress(DbgHelpModule, "SymCleanup"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymCleanup'");
    }

    if (!(Dbg->SymCompareInlineTrace = (PSYM_COMPARE_INLINE_TRACE)
        GetProcAddress(DbgHelpModule, "SymCompareInlineTrace"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymCompareInlineTrace'");
    }

    if (!(Dbg->SymDeleteSymbol = (PSYM_DELETE_SYMBOL)
        GetProcAddress(DbgHelpModule, "SymDeleteSymbol"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymDeleteSymbol'");
    }

    if (!(Dbg->SymDeleteSymbolW = (PSYM_DELETE_SYMBOL_W)
        GetProcAddress(DbgHelpModule, "SymDeleteSymbolW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymDeleteSymbolW'");
    }

    if (!(Dbg->SymEnumerateModules64 = (PSYM_ENUMERATE_MODULES64)
        GetProcAddress(DbgHelpModule, "SymEnumerateModules64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateModules64'");
    }

    if (!(Dbg->SymEnumerateModules = (PSYM_ENUMERATE_MODULES)
        GetProcAddress(DbgHelpModule, "SymEnumerateModules"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateModules'");
    }

    if (!(Dbg->SymEnumerateModulesW64 = (PSYM_ENUMERATE_MODULES_W64)
        GetProcAddress(DbgHelpModule, "SymEnumerateModulesW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateModulesW64'");
    }

    if (!(Dbg->SymEnumerateSymbols64 = (PSYM_ENUMERATE_SYMBOLS64)
        GetProcAddress(DbgHelpModule, "SymEnumerateSymbols64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateSymbols64'");
    }

    if (!(Dbg->SymEnumerateSymbols = (PSYM_ENUMERATE_SYMBOLS)
        GetProcAddress(DbgHelpModule, "SymEnumerateSymbols"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateSymbols'");
    }

    if (!(Dbg->SymEnumerateSymbolsW64 = (PSYM_ENUMERATE_SYMBOLS_W64)
        GetProcAddress(DbgHelpModule, "SymEnumerateSymbolsW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateSymbolsW64'");
    }

    if (!(Dbg->SymEnumerateSymbolsW = (PSYM_ENUMERATE_SYMBOLS_W)
        GetProcAddress(DbgHelpModule, "SymEnumerateSymbolsW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumerateSymbolsW'");
    }

    if (!(Dbg->SymEnumLines = (PSYM_ENUM_LINES)
        GetProcAddress(DbgHelpModule, "SymEnumLines"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumLines'");
    }

    if (!(Dbg->SymEnumLinesW = (PSYM_ENUM_LINES_W)
        GetProcAddress(DbgHelpModule, "SymEnumLinesW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumLinesW'");
    }

    if (!(Dbg->SymEnumProcesses = (PSYM_ENUM_PROCESSES)
        GetProcAddress(DbgHelpModule, "SymEnumProcesses"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumProcesses'");
    }

    if (!(Dbg->SymEnumSourceFiles = (PSYM_ENUM_SOURCE_FILES)
        GetProcAddress(DbgHelpModule, "SymEnumSourceFiles"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSourceFiles'");
    }

    if (!(Dbg->SymEnumSourceFilesW = (PSYM_ENUM_SOURCE_FILES_W)
        GetProcAddress(DbgHelpModule, "SymEnumSourceFilesW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSourceFilesW'");
    }

    if (!(Dbg->SymEnumSourceFileTokens = (PSYM_ENUM_SOURCE_FILE_TOKENS)
        GetProcAddress(DbgHelpModule, "SymEnumSourceFileTokens"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSourceFileTokens'");
    }

    if (!(Dbg->SymEnumSourceLines = (PSYM_ENUM_SOURCE_LINES)
        GetProcAddress(DbgHelpModule, "SymEnumSourceLines"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSourceLines'");
    }

    if (!(Dbg->SymEnumSourceLinesW = (PSYM_ENUM_SOURCE_LINES_W)
        GetProcAddress(DbgHelpModule, "SymEnumSourceLinesW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSourceLinesW'");
    }

    if (!(Dbg->SymEnumSymbolsEx = (PSYM_ENUM_SYMBOLS_EX)
        GetProcAddress(DbgHelpModule, "SymEnumSymbolsEx"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbolsEx'");
    }

    if (!(Dbg->SymEnumSymbolsExW = (PSYM_ENUM_SYMBOLS_EX_W)
        GetProcAddress(DbgHelpModule, "SymEnumSymbolsExW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbolsExW'");
    }

    if (!(Dbg->SymEnumSymbolsForAddr = (PSYM_ENUM_SYMBOLS_FOR_ADDR)
        GetProcAddress(DbgHelpModule, "SymEnumSymbolsForAddr"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbolsForAddr'");
    }

    if (!(Dbg->SymEnumSymbolsForAddrW = (PSYM_ENUM_SYMBOLS_FOR_ADDR_W)
        GetProcAddress(DbgHelpModule, "SymEnumSymbolsForAddrW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbolsForAddrW'");
    }

    if (!(Dbg->SymEnumSymbols = (PSYM_ENUM_SYMBOLS)
        GetProcAddress(DbgHelpModule, "SymEnumSymbols"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbols'");
    }

    if (!(Dbg->SymEnumSymbolsW = (PSYM_ENUM_SYMBOLS_W)
        GetProcAddress(DbgHelpModule, "SymEnumSymbolsW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSymbolsW'");
    }

    if (!(Dbg->SymEnumSym = (PSYM_ENUM_SYM)
        GetProcAddress(DbgHelpModule, "SymEnumSym"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumSym'");
    }

    if (!(Dbg->SymEnumTypesByName = (PSYM_ENUM_TYPES_BY_NAME)
        GetProcAddress(DbgHelpModule, "SymEnumTypesByName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumTypesByName'");
    }

    if (!(Dbg->SymEnumTypesByNameW = (PSYM_ENUM_TYPES_BY_NAME_W)
        GetProcAddress(DbgHelpModule, "SymEnumTypesByNameW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumTypesByNameW'");
    }

    if (!(Dbg->SymEnumTypes = (PSYM_ENUM_TYPES)
        GetProcAddress(DbgHelpModule, "SymEnumTypes"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumTypes'");
    }

    if (!(Dbg->SymEnumTypesW = (PSYM_ENUM_TYPES_W)
        GetProcAddress(DbgHelpModule, "SymEnumTypesW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymEnumTypesW'");
    }

    if (!(Dbg->SymFindFileInPath = (PSYM_FIND_FILE_IN_PATH)
        GetProcAddress(DbgHelpModule, "SymFindFileInPath"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFindFileInPath'");
    }

    if (!(Dbg->SymFindFileInPathW = (PSYM_FIND_FILE_IN_PATH_W)
        GetProcAddress(DbgHelpModule, "SymFindFileInPathW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFindFileInPathW'");
    }

    if (!(Dbg->SymFromAddr = (PSYM_FROM_ADDR)
        GetProcAddress(DbgHelpModule, "SymFromAddr"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromAddr'");
    }

    if (!(Dbg->SymFromAddrW = (PSYM_FROM_ADDR_W)
        GetProcAddress(DbgHelpModule, "SymFromAddrW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromAddrW'");
    }

    if (!(Dbg->SymFromIndex = (PSYM_FROM_INDEX)
        GetProcAddress(DbgHelpModule, "SymFromIndex"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromIndex'");
    }

    if (!(Dbg->SymFromIndexW = (PSYM_FROM_INDEX_W)
        GetProcAddress(DbgHelpModule, "SymFromIndexW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromIndexW'");
    }

    if (!(Dbg->SymFromInlineContext = (PSYM_FROM_INLINE_CONTEXT)
        GetProcAddress(DbgHelpModule, "SymFromInlineContext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromInlineContext'");
    }

    if (!(Dbg->SymFromInlineContextW = (PSYM_FROM_INLINE_CONTEXT_W)
        GetProcAddress(DbgHelpModule, "SymFromInlineContextW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromInlineContextW'");
    }

    if (!(Dbg->SymFromName = (PSYM_FROM_NAME)
        GetProcAddress(DbgHelpModule, "SymFromName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromName'");
    }

    if (!(Dbg->SymFromNameW = (PSYM_FROM_NAME_W)
        GetProcAddress(DbgHelpModule, "SymFromNameW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromNameW'");
    }

    if (!(Dbg->SymFromToken = (PSYM_FROM_TOKEN)
        GetProcAddress(DbgHelpModule, "SymFromToken"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromToken'");
    }

    if (!(Dbg->SymFromTokenW = (PSYM_FROM_TOKEN_W)
        GetProcAddress(DbgHelpModule, "SymFromTokenW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymFromTokenW'");
    }

    if (!(Dbg->SymGetFileLineOffsets64 = (PSYM_GET_FILE_LINE_OFFSETS64)
        GetProcAddress(DbgHelpModule, "SymGetFileLineOffsets64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetFileLineOffsets64'");
    }

    if (!(Dbg->SymGetLineFromAddr64 = (PSYM_GET_LINE_FROM_ADDR64)
        GetProcAddress(DbgHelpModule, "SymGetLineFromAddr64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromAddr64'");
    }

    if (!(Dbg->SymGetLineFromAddr = (PSYM_GET_LINE_FROM_ADDR)
        GetProcAddress(DbgHelpModule, "SymGetLineFromAddr"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromAddr'");
    }

    if (!(Dbg->SymGetLineFromAddrW64 = (PSYM_GET_LINE_FROM_ADDR_W64)
        GetProcAddress(DbgHelpModule, "SymGetLineFromAddrW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromAddrW64'");
    }

    if (!(Dbg->SymGetLineFromInlineContext = (PSYM_GET_LINE_FROM_INLINE_CONTEXT)
        GetProcAddress(DbgHelpModule, "SymGetLineFromInlineContext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromInlineContext'");
    }

    if (!(Dbg->SymGetLineFromInlineContextW = (PSYM_GET_LINE_FROM_INLINE_CONTEXT_W)
        GetProcAddress(DbgHelpModule, "SymGetLineFromInlineContextW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromInlineContextW'");
    }

    if (!(Dbg->SymGetLineFromName64 = (PSYM_GET_LINE_FROM_NAME64)
        GetProcAddress(DbgHelpModule, "SymGetLineFromName64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromName64'");
    }

    if (!(Dbg->SymGetLineFromName = (PSYM_GET_LINE_FROM_NAME)
        GetProcAddress(DbgHelpModule, "SymGetLineFromName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromName'");
    }

    if (!(Dbg->SymGetLineFromNameW64 = (PSYM_GET_LINE_FROM_NAME_W64)
        GetProcAddress(DbgHelpModule, "SymGetLineFromNameW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineFromNameW64'");
    }

    if (!(Dbg->SymGetLineNext64 = (PSYM_GET_LINE_NEXT64)
        GetProcAddress(DbgHelpModule, "SymGetLineNext64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineNext64'");
    }

    if (!(Dbg->SymGetLineNext = (PSYM_GET_LINE_NEXT)
        GetProcAddress(DbgHelpModule, "SymGetLineNext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineNext'");
    }

    if (!(Dbg->SymGetLineNextW64 = (PSYM_GET_LINE_NEXT_W64)
        GetProcAddress(DbgHelpModule, "SymGetLineNextW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLineNextW64'");
    }

    if (!(Dbg->SymGetLinePrev64 = (PSYM_GET_LINE_PREV64)
        GetProcAddress(DbgHelpModule, "SymGetLinePrev64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLinePrev64'");
    }

    if (!(Dbg->SymGetLinePrev = (PSYM_GET_LINE_PREV)
        GetProcAddress(DbgHelpModule, "SymGetLinePrev"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLinePrev'");
    }

    if (!(Dbg->SymGetLinePrevW64 = (PSYM_GET_LINE_PREV_W64)
        GetProcAddress(DbgHelpModule, "SymGetLinePrevW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetLinePrevW64'");
    }

    if (!(Dbg->SymGetModuleBase = (PSYM_GET_MODULE_BASE)
        GetProcAddress(DbgHelpModule, "SymGetModuleBase"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetModuleBase'");
    }

    if (!(Dbg->SymGetModuleInfo64 = (PSYM_GET_MODULE_INFO64)
        GetProcAddress(DbgHelpModule, "SymGetModuleInfo64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetModuleInfo64'");
    }

    if (!(Dbg->SymGetModuleInfo = (PSYM_GET_MODULE_INFO)
        GetProcAddress(DbgHelpModule, "SymGetModuleInfo"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetModuleInfo'");
    }

    if (!(Dbg->SymGetModuleInfoW64 = (PSYM_GET_MODULE_INFO_W64)
        GetProcAddress(DbgHelpModule, "SymGetModuleInfoW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetModuleInfoW64'");
    }

    if (!(Dbg->SymGetModuleInfoW = (PSYM_GET_MODULE_INFO_W)
        GetProcAddress(DbgHelpModule, "SymGetModuleInfoW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetModuleInfoW'");
    }

    if (!(Dbg->SymGetOmaps = (PSYM_GET_OMAPS)
        GetProcAddress(DbgHelpModule, "SymGetOmaps"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetOmaps'");
    }

    if (!(Dbg->SymGetOptions = (PSYM_GET_OPTIONS)
        GetProcAddress(DbgHelpModule, "SymGetOptions"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetOptions'");
    }

    if (!(Dbg->SymGetScope = (PSYM_GET_SCOPE)
        GetProcAddress(DbgHelpModule, "SymGetScope"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetScope'");
    }

    if (!(Dbg->SymGetScopeW = (PSYM_GET_SCOPE_W)
        GetProcAddress(DbgHelpModule, "SymGetScopeW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetScopeW'");
    }

    if (!(Dbg->SymGetSearchPath = (PSYM_GET_SEARCH_PATH)
        GetProcAddress(DbgHelpModule, "SymGetSearchPath"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSearchPath'");
    }

    if (!(Dbg->SymGetSearchPathW = (PSYM_GET_SEARCH_PATH_W)
        GetProcAddress(DbgHelpModule, "SymGetSearchPathW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSearchPathW'");
    }

    if (!(Dbg->SymGetSourceFileFromToken = (PSYM_GET_SOURCE_FILE_FROM_TOKEN)
        GetProcAddress(DbgHelpModule, "SymGetSourceFileFromToken"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFileFromToken'");
    }

    if (!(Dbg->SymGetSourceFileFromTokenW = (PSYM_GET_SOURCE_FILE_FROM_TOKEN_W)
        GetProcAddress(DbgHelpModule, "SymGetSourceFileFromTokenW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFileFromTokenW'");
    }

    if (!(Dbg->SymGetSourceFile = (PSYM_GET_SOURCE_FILE)
        GetProcAddress(DbgHelpModule, "SymGetSourceFile"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFile'");
    }

    if (!(Dbg->SymGetSourceFileToken = (PSYM_GET_SOURCE_FILE_TOKEN)
        GetProcAddress(DbgHelpModule, "SymGetSourceFileToken"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFileToken'");
    }

    if (!(Dbg->SymGetSourceFileTokenW = (PSYM_GET_SOURCE_FILE_TOKEN_W)
        GetProcAddress(DbgHelpModule, "SymGetSourceFileTokenW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFileTokenW'");
    }

    if (!(Dbg->SymGetSourceFileW = (PSYM_GET_SOURCE_FILE_W)
        GetProcAddress(DbgHelpModule, "SymGetSourceFileW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceFileW'");
    }

    if (!(Dbg->SymGetSourceVarFromToken = (PSYM_GET_SOURCE_VAR_FROM_TOKEN)
        GetProcAddress(DbgHelpModule, "SymGetSourceVarFromToken"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceVarFromToken'");
    }

    if (!(Dbg->SymGetSourceVarFromTokenW = (PSYM_GET_SOURCE_VAR_FROM_TOKEN_W)
        GetProcAddress(DbgHelpModule, "SymGetSourceVarFromTokenW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSourceVarFromTokenW'");
    }

    if (!(Dbg->SymGetSymbolFile = (PSYM_GET_SYMBOL_FILE)
        GetProcAddress(DbgHelpModule, "SymGetSymbolFile"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymbolFile'");
    }

    if (!(Dbg->SymGetSymbolFileW = (PSYM_GET_SYMBOL_FILE_W)
        GetProcAddress(DbgHelpModule, "SymGetSymbolFileW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymbolFileW'");
    }

    if (!(Dbg->SymGetSymFromAddr64 = (PSYM_GET_SYM_FROM_ADDR64)
        GetProcAddress(DbgHelpModule, "SymGetSymFromAddr64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymFromAddr64'");
    }

    if (!(Dbg->SymGetSymFromAddr = (PSYM_GET_SYM_FROM_ADDR)
        GetProcAddress(DbgHelpModule, "SymGetSymFromAddr"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymFromAddr'");
    }

    if (!(Dbg->SymGetSymFromName64 = (PSYM_GET_SYM_FROM_NAME64)
        GetProcAddress(DbgHelpModule, "SymGetSymFromName64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymFromName64'");
    }

    if (!(Dbg->SymGetSymFromName = (PSYM_GET_SYM_FROM_NAME)
        GetProcAddress(DbgHelpModule, "SymGetSymFromName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymFromName'");
    }

    if (!(Dbg->SymGetSymNext64 = (PSYM_GET_SYM_NEXT64)
        GetProcAddress(DbgHelpModule, "SymGetSymNext64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymNext64'");
    }

    if (!(Dbg->SymGetSymNext = (PSYM_GET_SYM_NEXT)
        GetProcAddress(DbgHelpModule, "SymGetSymNext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymNext'");
    }

    if (!(Dbg->SymGetSymPrev64 = (PSYM_GET_SYM_PREV64)
        GetProcAddress(DbgHelpModule, "SymGetSymPrev64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymPrev64'");
    }

    if (!(Dbg->SymGetSymPrev = (PSYM_GET_SYM_PREV)
        GetProcAddress(DbgHelpModule, "SymGetSymPrev"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetSymPrev'");
    }

    if (!(Dbg->SymGetTypeFromName = (PSYM_GET_TYPE_FROM_NAME)
        GetProcAddress(DbgHelpModule, "SymGetTypeFromName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetTypeFromName'");
    }

    if (!(Dbg->SymGetTypeFromNameW = (PSYM_GET_TYPE_FROM_NAME_W)
        GetProcAddress(DbgHelpModule, "SymGetTypeFromNameW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetTypeFromNameW'");
    }

    if (!(Dbg->SymGetTypeInfoEx = (PSYM_GET_TYPE_INFO_EX)
        GetProcAddress(DbgHelpModule, "SymGetTypeInfoEx"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetTypeInfoEx'");
    }

    if (!(Dbg->SymGetTypeInfo = (PSYM_GET_TYPE_INFO)
        GetProcAddress(DbgHelpModule, "SymGetTypeInfo"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetTypeInfo'");
    }

    if (!(Dbg->SymGetUnwindInfo = (PSYM_GET_UNWIND_INFO)
        GetProcAddress(DbgHelpModule, "SymGetUnwindInfo"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymGetUnwindInfo'");
    }

    if (!(Dbg->SymInitialize = (PSYM_INITIALIZE)
        GetProcAddress(DbgHelpModule, "SymInitialize"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymInitialize'");
    }

    if (!(Dbg->SymInitializeW = (PSYM_INITIALIZE_W)
        GetProcAddress(DbgHelpModule, "SymInitializeW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymInitializeW'");
    }

    if (!(Dbg->SymLoadModule = (PSYM_LOAD_MODULE)
        GetProcAddress(DbgHelpModule, "SymLoadModule"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymLoadModule'");
    }

    if (!(Dbg->SymLoadModuleEx = (PSYM_LOAD_MODULE_EX)
        GetProcAddress(DbgHelpModule, "SymLoadModuleEx"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymLoadModuleEx'");
    }

    if (!(Dbg->SymLoadModuleExW = (PSYM_LOAD_MODULE_EX_W)
        GetProcAddress(DbgHelpModule, "SymLoadModuleExW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymLoadModuleExW'");
    }

    if (!(Dbg->SymMatchFileName = (PSYM_MATCH_FILE_NAME)
        GetProcAddress(DbgHelpModule, "SymMatchFileName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymMatchFileName'");
    }

    if (!(Dbg->SymMatchFileNameW = (PSYM_MATCH_FILE_NAME_W)
        GetProcAddress(DbgHelpModule, "SymMatchFileNameW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymMatchFileNameW'");
    }

    if (!(Dbg->SymMatchStringA = (PSYM_MATCH_STRING_A)
        GetProcAddress(DbgHelpModule, "SymMatchStringA"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymMatchStringA'");
    }

    if (!(Dbg->SymMatchString = (PSYM_MATCH_STRING)
        GetProcAddress(DbgHelpModule, "SymMatchString"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymMatchString'");
    }

    if (!(Dbg->SymMatchStringW = (PSYM_MATCH_STRING_W)
        GetProcAddress(DbgHelpModule, "SymMatchStringW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymMatchStringW'");
    }

    if (!(Dbg->SymNext = (PSYM_NEXT)
        GetProcAddress(DbgHelpModule, "SymNext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymNext'");
    }

    if (!(Dbg->SymNextW = (PSYM_NEXT_W)
        GetProcAddress(DbgHelpModule, "SymNextW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymNextW'");
    }

    if (!(Dbg->SymPrev = (PSYM_PREV)
        GetProcAddress(DbgHelpModule, "SymPrev"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymPrev'");
    }

    if (!(Dbg->SymPrevW = (PSYM_PREV_W)
        GetProcAddress(DbgHelpModule, "SymPrevW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymPrevW'");
    }

    if (!(Dbg->SymQueryInlineTrace = (PSYM_QUERY_INLINE_TRACE)
        GetProcAddress(DbgHelpModule, "SymQueryInlineTrace"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymQueryInlineTrace'");
    }

    if (!(Dbg->SymRefreshModuleList = (PSYM_REFRESH_MODULE_LIST)
        GetProcAddress(DbgHelpModule, "SymRefreshModuleList"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRefreshModuleList'");
    }

    if (!(Dbg->SymRegisterCallback64 = (PSYM_REGISTER_CALLBACK64)
        GetProcAddress(DbgHelpModule, "SymRegisterCallback64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRegisterCallback64'");
    }

    if (!(Dbg->SymRegisterCallback = (PSYM_REGISTER_CALLBACK)
        GetProcAddress(DbgHelpModule, "SymRegisterCallback"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRegisterCallback'");
    }

    if (!(Dbg->SymRegisterCallbackW64 = (PSYM_REGISTER_CALLBACK_W64)
        GetProcAddress(DbgHelpModule, "SymRegisterCallbackW64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRegisterCallbackW64'");
    }

    if (!(Dbg->SymRegisterFunctionEntryCallback64 = (PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK64)
        GetProcAddress(DbgHelpModule, "SymRegisterFunctionEntryCallback64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRegisterFunctionEntryCallback64'");
    }

    if (!(Dbg->SymRegisterFunctionEntryCallback = (PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK)
        GetProcAddress(DbgHelpModule, "SymRegisterFunctionEntryCallback"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymRegisterFunctionEntryCallback'");
    }

    if (!(Dbg->SymSearch = (PSYM_SEARCH)
        GetProcAddress(DbgHelpModule, "SymSearch"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSearch'");
    }

    if (!(Dbg->SymSearchW = (PSYM_SEARCH_W)
        GetProcAddress(DbgHelpModule, "SymSearchW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSearchW'");
    }

    if (!(Dbg->SymSetContext = (PSYM_SET_CONTEXT)
        GetProcAddress(DbgHelpModule, "SymSetContext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetContext'");
    }

    if (!(Dbg->SymSetOptions = (PSYM_SET_OPTIONS)
        GetProcAddress(DbgHelpModule, "SymSetOptions"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetOptions'");
    }

    if (!(Dbg->SymSetParentWindow = (PSYM_SET_PARENT_WINDOW)
        GetProcAddress(DbgHelpModule, "SymSetParentWindow"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetParentWindow'");
    }

    if (!(Dbg->SymSetScopeFromAddr = (PSYM_SET_SCOPE_FROM_ADDR)
        GetProcAddress(DbgHelpModule, "SymSetScopeFromAddr"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetScopeFromAddr'");
    }

    if (!(Dbg->SymSetScopeFromIndex = (PSYM_SET_SCOPE_FROM_INDEX)
        GetProcAddress(DbgHelpModule, "SymSetScopeFromIndex"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetScopeFromIndex'");
    }

    if (!(Dbg->SymSetScopeFromInlineContext = (PSYM_SET_SCOPE_FROM_INLINE_CONTEXT)
        GetProcAddress(DbgHelpModule, "SymSetScopeFromInlineContext"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetScopeFromInlineContext'");
    }

    if (!(Dbg->SymSetSearchPath = (PSYM_SET_SEARCH_PATH)
        GetProcAddress(DbgHelpModule, "SymSetSearchPath"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetSearchPath'");
    }

    if (!(Dbg->SymSetSearchPathW = (PSYM_SET_SEARCH_PATH_W)
        GetProcAddress(DbgHelpModule, "SymSetSearchPathW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSetSearchPathW'");
    }

    if (!(Dbg->SymSrvGetFileIndexes = (PSYM_SRV_GET_FILE_INDEXES)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexes"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexes'");
    }

    if (!(Dbg->SymSrvGetFileIndexesW = (PSYM_SRV_GET_FILE_INDEXES_W)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexesW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexesW'");
    }

    if (!(Dbg->SymSrvGetFileIndexInfo = (PSYM_SRV_GET_FILE_INDEX_INFO)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexInfo"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexInfo'");
    }

    if (!(Dbg->SymSrvGetFileIndexInfoW = (PSYM_SRV_GET_FILE_INDEX_INFO_W)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexInfoW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexInfoW'");
    }

    if (!(Dbg->SymSrvGetFileIndexString = (PSYM_SRV_GET_FILE_INDEX_STRING)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexString"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexString'");
    }

    if (!(Dbg->SymSrvGetFileIndexStringW = (PSYM_SRV_GET_FILE_INDEX_STRING_W)
        GetProcAddress(DbgHelpModule, "SymSrvGetFileIndexStringW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvGetFileIndexStringW'");
    }

    if (!(Dbg->SymSrvIsStore = (PSYM_SRV_IS_STORE)
        GetProcAddress(DbgHelpModule, "SymSrvIsStore"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvIsStore'");
    }

    if (!(Dbg->SymSrvIsStoreW = (PSYM_SRV_IS_STORE_W)
        GetProcAddress(DbgHelpModule, "SymSrvIsStoreW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymSrvIsStoreW'");
    }

    if (!(Dbg->SymUnDName64 = (PSYM_UNDNAME64)
        GetProcAddress(DbgHelpModule, "SymUnDName64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymUnDName64'");
    }

    if (!(Dbg->SymUnDName = (PSYM_UNDNAME)
        GetProcAddress(DbgHelpModule, "SymUnDName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymUnDName'");
    }

    if (!(Dbg->SymUnloadModule64 = (PSYM_UNLOAD_MODULE64)
        GetProcAddress(DbgHelpModule, "SymUnloadModule64"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymUnloadModule64'");
    }

    if (!(Dbg->SymUnloadModule = (PSYM_UNLOAD_MODULE)
        GetProcAddress(DbgHelpModule, "SymUnloadModule"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'SymUnloadModule'");
    }

    if (!(Dbg->UnDecorateSymbolName = (PUNDECORATE_SYMBOL_NAME)
        GetProcAddress(DbgHelpModule, "UnDecorateSymbolName"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'UnDecorateSymbolName'");
    }

    if (!(Dbg->UnDecorateSymbolNameW = (PUNDECORATE_SYMBOL_NAME_W)
        GetProcAddress(DbgHelpModule, "UnDecorateSymbolNameW"))) {

        OutputDebugStringA("DbgHelp: failed to resolve 'UnDecorateSymbolNameW'");
    }


    //
    // End of auto-generated section.
    //

    return TRUE;
}

RTL_API
BOOL
LoadDbgHelp(PRTL Rtl)
{
    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (Rtl->DbgHelpModule) {
        return TRUE;
    }

    if (!(Rtl->DbgHelpModule = LoadLibraryA("dbghelp"))) {
        OutputDebugStringA("Rtl: Failed to load dbghelp.");
        return FALSE;
    }

    return LoadDbg(Rtl->DbgHelpModule, &Rtl->Dbg);
}

RTL_API
BOOL
InitializeCom(
    _In_ PRTL Rtl
    )
{

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (Rtl->ComInitialized) {
        return TRUE;
    }

    if (!(Rtl->Ole32Module = LoadLibraryA("ole32"))) {
        OutputDebugStringA("Rtl: Failed to load ole32.");
        return FALSE;
    }

    if (!(Rtl->CoInitializeEx = (PCO_INITIALIZE_EX)
        GetProcAddress(Rtl->Ole32Module, "CoInitializeEx"))) {
        OutputDebugStringA("Failed to resolve CoInitializeEx.\n");
        return FALSE;
    }

    Rtl->ComInitialized = TRUE;
    return TRUE;
}

RTL_API
BOOL
LoadDbgEng(
    _In_ PRTL Rtl
    )
{

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (Rtl->DbgEngModule) {
        return TRUE;
    }

    if (!Rtl->InitializeCom(Rtl)) {
        return FALSE;
    }

    if (!(Rtl->DbgEngModule = LoadLibraryA("dbgeng"))) {
        OutputDebugStringA("Rtl: Failed to load dbgeng.");
        return FALSE;
    }

    if (!(Rtl->DebugCreate = (PDEBUG_CREATE)
        GetProcAddress(Rtl->DbgEngModule, "DebugCreate"))) {

        OutputDebugStringA("DbgEng: failed to resolve 'DebugCreate'");
        return FALSE;
    }

    return TRUE;
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
    if (!Module || Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (
            (PGETSYSTEMTIMEPRECISEASFILETIME)Proc
        );
    } else {
        Module = LoadLibrary(TEXT("ntdll"));
        if (!Module || Module == INVALID_HANDLE_VALUE) {
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
    _In_     PCUNICODE_STRING    String,
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
    _In_     PCSTRING     String,
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
    _In_     PCUNICODE_STRING    String,
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
    if it is big enough (governed by the SizeOfBitMap field).  If it isn't,
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
    _In_     PCSTRING       String,
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
    if it is big enough (governed by the SizeOfBitMap field).  If it isn't,
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
                HEAP_ZERO_MEMORY,
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

    if (!Bitmap->Buffer) {
        __debugbreak();
    }

    //
    // Clear the bitmap.  We use SecureZeroMemory() instead of RtlClearAllBits()
    // as the latter is dependent upon the SizeOfBitMap field, which a) isn't
    // set here, and b) will be set to NumberOfCharacters when it is set, which
    // may be less than AlignedNumberOfCharacters, which means some trailing
    // bits could be non-zero if we are re-using the caller's stack-allocated
    // bitmap buffer.
    //

    SecureZeroMemory(Bitmap->Buffer, BitmapBufferSizeInBytes);

    //
    // There will be one bit per character.
    //
    //

    Bitmap->SizeOfBitMap = NumberOfCharacters;

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

    if (!(Rtl->RtlCharToInteger = (PRTL_CHAR_TO_INTEGER)
        GetProcAddress(Rtl->NtdllModule, "RtlCharToInteger"))) {

        if (!(Rtl->RtlCharToInteger = (PRTL_CHAR_TO_INTEGER)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCharToInteger"))) {

            if (!(Rtl->RtlCharToInteger = (PRTL_CHAR_TO_INTEGER)
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

    if (!(Rtl->memset = (PMEMSET)
        GetProcAddress(Rtl->NtdllModule, "memset"))) {

        if (!(Rtl->memset = (PMEMSET)
            GetProcAddress(Rtl->NtosKrnlModule, "memset"))) {

            if (!(Rtl->memset = (PMEMSET)
                GetProcAddress(Rtl->Kernel32Module, "memset"))) {

                OutputDebugStringA("Rtl: failed to resolve 'memset'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->MmGetMaximumFileSectionSize = (PMM_GET_MAXIMUM_FILE_SECTION_SIZE)
        GetProcAddress(Rtl->NtdllModule, "MmGetMaximumFileSectionSize"))) {

        if (!(Rtl->MmGetMaximumFileSectionSize = (PMM_GET_MAXIMUM_FILE_SECTION_SIZE)
            GetProcAddress(Rtl->NtosKrnlModule, "MmGetMaximumFileSectionSize"))) {

            if (!(Rtl->MmGetMaximumFileSectionSize = (PMM_GET_MAXIMUM_FILE_SECTION_SIZE)
                GetProcAddress(Rtl->Kernel32Module, "MmGetMaximumFileSectionSize"))) {

                OutputDebugStringA("Rtl: failed to resolve 'MmGetMaximumFileSectionSize'");
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

    if (!(Rtl->K32GetPerformanceInfo = (PGET_PERFORMANCE_INFO)
        GetProcAddress(Rtl->NtdllModule, "K32GetPerformanceInfo"))) {

        if (!(Rtl->K32GetPerformanceInfo = (PGET_PERFORMANCE_INFO)
            GetProcAddress(Rtl->NtosKrnlModule, "K32GetPerformanceInfo"))) {

            if (!(Rtl->K32GetPerformanceInfo = (PGET_PERFORMANCE_INFO)
                GetProcAddress(Rtl->Kernel32Module, "K32GetPerformanceInfo"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32GetPerformanceInfo'");
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

    if (!(Rtl->K32InitializeProcessForWsWatch = (PINITIALIZE_PROCESS_FOR_WS_WATCH)
        GetProcAddress(Rtl->NtdllModule, "K32InitializeProcessForWsWatch"))) {

        if (!(Rtl->K32InitializeProcessForWsWatch = (PINITIALIZE_PROCESS_FOR_WS_WATCH)
            GetProcAddress(Rtl->NtosKrnlModule, "K32InitializeProcessForWsWatch"))) {

            if (!(Rtl->K32InitializeProcessForWsWatch = (PINITIALIZE_PROCESS_FOR_WS_WATCH)
                GetProcAddress(Rtl->Kernel32Module, "K32InitializeProcessForWsWatch"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32InitializeProcessForWsWatch'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->K32GetWsChanges = (PGET_WS_CHANGES)
        GetProcAddress(Rtl->NtdllModule, "K32GetWsChanges"))) {

        if (!(Rtl->K32GetWsChanges = (PGET_WS_CHANGES)
            GetProcAddress(Rtl->NtosKrnlModule, "K32GetWsChanges"))) {

            if (!(Rtl->K32GetWsChanges = (PGET_WS_CHANGES)
                GetProcAddress(Rtl->Kernel32Module, "K32GetWsChanges"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32GetWsChanges'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->K32GetWsChangesEx = (PGET_WS_CHANGES_EX)
        GetProcAddress(Rtl->NtdllModule, "K32GetWsChangesEx"))) {

        if (!(Rtl->K32GetWsChangesEx = (PGET_WS_CHANGES_EX)
            GetProcAddress(Rtl->NtosKrnlModule, "K32GetWsChangesEx"))) {

            if (!(Rtl->K32GetWsChangesEx = (PGET_WS_CHANGES_EX)
                GetProcAddress(Rtl->Kernel32Module, "K32GetWsChangesEx"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32GetWsChangesEx'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->K32QueryWorkingSet = (PQUERY_WORKING_SET)
        GetProcAddress(Rtl->NtdllModule, "K32QueryWorkingSet"))) {

        if (!(Rtl->K32QueryWorkingSet = (PQUERY_WORKING_SET)
            GetProcAddress(Rtl->NtosKrnlModule, "K32QueryWorkingSet"))) {

            if (!(Rtl->K32QueryWorkingSet = (PQUERY_WORKING_SET)
                GetProcAddress(Rtl->Kernel32Module, "K32QueryWorkingSet"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32QueryWorkingSet'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->K32QueryWorkingSetEx = (PQUERY_WORKING_SET_EX)
        GetProcAddress(Rtl->NtdllModule, "K32QueryWorkingSetEx"))) {

        if (!(Rtl->K32QueryWorkingSetEx = (PQUERY_WORKING_SET_EX)
            GetProcAddress(Rtl->NtosKrnlModule, "K32QueryWorkingSetEx"))) {

            if (!(Rtl->K32QueryWorkingSetEx = (PQUERY_WORKING_SET_EX)
                GetProcAddress(Rtl->Kernel32Module, "K32QueryWorkingSetEx"))) {

                OutputDebugStringA("Rtl: failed to resolve 'K32QueryWorkingSetEx'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwQueryInformationProcess = (PZW_QUERY_INFORMATION_PROCESS)
        GetProcAddress(Rtl->NtdllModule, "ZwQueryInformationProcess"))) {

        if (!(Rtl->ZwQueryInformationProcess = (PZW_QUERY_INFORMATION_PROCESS)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwQueryInformationProcess"))) {

            if (!(Rtl->ZwQueryInformationProcess = (PZW_QUERY_INFORMATION_PROCESS)
                GetProcAddress(Rtl->Kernel32Module, "ZwQueryInformationProcess"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwQueryInformationProcess'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->LdrRegisterDllNotification = (PLDR_REGISTER_DLL_NOTIFICATION)
        GetProcAddress(Rtl->NtdllModule, "LdrRegisterDllNotification"))) {

        if (!(Rtl->LdrRegisterDllNotification = (PLDR_REGISTER_DLL_NOTIFICATION)
            GetProcAddress(Rtl->NtosKrnlModule, "LdrRegisterDllNotification"))) {

            if (!(Rtl->LdrRegisterDllNotification = (PLDR_REGISTER_DLL_NOTIFICATION)
                GetProcAddress(Rtl->Kernel32Module, "LdrRegisterDllNotification"))) {

                OutputDebugStringA("Rtl: failed to resolve 'LdrRegisterDllNotification'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->LdrUnregisterDllNotification = (PLDR_UNREGISTER_DLL_NOTIFICATION)
        GetProcAddress(Rtl->NtdllModule, "LdrUnregisterDllNotification"))) {

        if (!(Rtl->LdrUnregisterDllNotification = (PLDR_UNREGISTER_DLL_NOTIFICATION)
            GetProcAddress(Rtl->NtosKrnlModule, "LdrUnregisterDllNotification"))) {

            if (!(Rtl->LdrUnregisterDllNotification = (PLDR_UNREGISTER_DLL_NOTIFICATION)
                GetProcAddress(Rtl->Kernel32Module, "LdrUnregisterDllNotification"))) {

                OutputDebugStringA("Rtl: failed to resolve 'LdrUnregisterDllNotification'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->LdrLockLoaderLock = (PLDR_LOCK_LOADER_LOCK)
        GetProcAddress(Rtl->NtdllModule, "LdrLockLoaderLock"))) {

        if (!(Rtl->LdrLockLoaderLock = (PLDR_LOCK_LOADER_LOCK)
            GetProcAddress(Rtl->NtosKrnlModule, "LdrLockLoaderLock"))) {

            if (!(Rtl->LdrLockLoaderLock = (PLDR_LOCK_LOADER_LOCK)
                GetProcAddress(Rtl->Kernel32Module, "LdrLockLoaderLock"))) {

                OutputDebugStringA("Rtl: failed to resolve 'LdrLockLoaderLock'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->LdrUnlockLoaderLock = (PLDR_UNLOCK_LOADER_LOCK)
        GetProcAddress(Rtl->NtdllModule, "LdrUnlockLoaderLock"))) {

        if (!(Rtl->LdrUnlockLoaderLock = (PLDR_UNLOCK_LOADER_LOCK)
            GetProcAddress(Rtl->NtosKrnlModule, "LdrUnlockLoaderLock"))) {

            if (!(Rtl->LdrUnlockLoaderLock = (PLDR_UNLOCK_LOADER_LOCK)
                GetProcAddress(Rtl->Kernel32Module, "LdrUnlockLoaderLock"))) {

                OutputDebugStringA("Rtl: failed to resolve 'LdrUnlockLoaderLock'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwAllocateVirtualMemory = (PZW_ALLOCATE_VIRTUAL_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "ZwAllocateVirtualMemory"))) {

        if (!(Rtl->ZwAllocateVirtualMemory = (PZW_ALLOCATE_VIRTUAL_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwAllocateVirtualMemory"))) {

            if (!(Rtl->ZwAllocateVirtualMemory = (PZW_ALLOCATE_VIRTUAL_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "ZwAllocateVirtualMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwAllocateVirtualMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwFreeVirtualMemory = (PZW_FREE_VIRTUAL_MEMORY)
        GetProcAddress(Rtl->NtdllModule, "ZwFreeVirtualMemory"))) {

        if (!(Rtl->ZwFreeVirtualMemory = (PZW_FREE_VIRTUAL_MEMORY)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwFreeVirtualMemory"))) {

            if (!(Rtl->ZwFreeVirtualMemory = (PZW_FREE_VIRTUAL_MEMORY)
                GetProcAddress(Rtl->Kernel32Module, "ZwFreeVirtualMemory"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwFreeVirtualMemory'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCreateHeap = (PRTL_CREATE_HEAP)
        GetProcAddress(Rtl->NtdllModule, "RtlCreateHeap"))) {

        if (!(Rtl->RtlCreateHeap = (PRTL_CREATE_HEAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCreateHeap"))) {

            if (!(Rtl->RtlCreateHeap = (PRTL_CREATE_HEAP)
                GetProcAddress(Rtl->Kernel32Module, "RtlCreateHeap"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCreateHeap'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlDestroyHeap = (PRTL_DESTROY_HEAP)
        GetProcAddress(Rtl->NtdllModule, "RtlDestroyHeap"))) {

        if (!(Rtl->RtlDestroyHeap = (PRTL_DESTROY_HEAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDestroyHeap"))) {

            if (!(Rtl->RtlDestroyHeap = (PRTL_DESTROY_HEAP)
                GetProcAddress(Rtl->Kernel32Module, "RtlDestroyHeap"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlDestroyHeap'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlAllocateHeap = (PRTL_ALLOCATE_HEAP)
        GetProcAddress(Rtl->NtdllModule, "RtlAllocateHeap"))) {

        if (!(Rtl->RtlAllocateHeap = (PRTL_ALLOCATE_HEAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAllocateHeap"))) {

            if (!(Rtl->RtlAllocateHeap = (PRTL_ALLOCATE_HEAP)
                GetProcAddress(Rtl->Kernel32Module, "RtlAllocateHeap"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlAllocateHeap'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlFreeHeap = (PRTL_FREE_HEAP)
        GetProcAddress(Rtl->NtdllModule, "RtlFreeHeap"))) {

        if (!(Rtl->RtlFreeHeap = (PRTL_FREE_HEAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFreeHeap"))) {

            if (!(Rtl->RtlFreeHeap = (PRTL_FREE_HEAP)
                GetProcAddress(Rtl->Kernel32Module, "RtlFreeHeap"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlFreeHeap'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->RtlCaptureStackBackTrace = (PRTL_CAPTURE_STACK_BACK_TRACE)
        GetProcAddress(Rtl->NtdllModule, "RtlCaptureStackBackTrace"))) {

        if (!(Rtl->RtlCaptureStackBackTrace = (PRTL_CAPTURE_STACK_BACK_TRACE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCaptureStackBackTrace"))) {

            if (!(Rtl->RtlCaptureStackBackTrace = (PRTL_CAPTURE_STACK_BACK_TRACE)
                GetProcAddress(Rtl->Kernel32Module, "RtlCaptureStackBackTrace"))) {

                OutputDebugStringA("Rtl: failed to resolve 'RtlCaptureStackBackTrace'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwCreateSection = (PZW_CREATE_SECTION)
        GetProcAddress(Rtl->NtdllModule, "ZwCreateSection"))) {

        if (!(Rtl->ZwCreateSection = (PZW_CREATE_SECTION)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwCreateSection"))) {

            if (!(Rtl->ZwCreateSection = (PZW_CREATE_SECTION)
                GetProcAddress(Rtl->Kernel32Module, "ZwCreateSection"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwCreateSection'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwMapViewOfSection = (PZW_MAP_VIEW_OF_SECTION)
        GetProcAddress(Rtl->NtdllModule, "ZwMapViewOfSection"))) {

        if (!(Rtl->ZwMapViewOfSection = (PZW_MAP_VIEW_OF_SECTION)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwMapViewOfSection"))) {

            if (!(Rtl->ZwMapViewOfSection = (PZW_MAP_VIEW_OF_SECTION)
                GetProcAddress(Rtl->Kernel32Module, "ZwMapViewOfSection"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwMapViewOfSection'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwUnmapViewOfSection = (PZW_UNMAP_VIEW_OF_SECTION)
        GetProcAddress(Rtl->NtdllModule, "ZwUnmapViewOfSection"))) {

        if (!(Rtl->ZwUnmapViewOfSection = (PZW_UNMAP_VIEW_OF_SECTION)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwUnmapViewOfSection"))) {

            if (!(Rtl->ZwUnmapViewOfSection = (PZW_UNMAP_VIEW_OF_SECTION)
                GetProcAddress(Rtl->Kernel32Module, "ZwUnmapViewOfSection"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwUnmapViewOfSection'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwCreateProcess = (PZW_CREATE_PROCESS)
        GetProcAddress(Rtl->NtdllModule, "ZwCreateProcess"))) {

        if (!(Rtl->ZwCreateProcess = (PZW_CREATE_PROCESS)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwCreateProcess"))) {

            if (!(Rtl->ZwCreateProcess = (PZW_CREATE_PROCESS)
                GetProcAddress(Rtl->Kernel32Module, "ZwCreateProcess"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwCreateProcess'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwCreateProcessEx = (PZW_CREATE_PROCESS_EX)
        GetProcAddress(Rtl->NtdllModule, "ZwCreateProcessEx"))) {

        if (!(Rtl->ZwCreateProcessEx = (PZW_CREATE_PROCESS_EX)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwCreateProcessEx"))) {

            if (!(Rtl->ZwCreateProcessEx = (PZW_CREATE_PROCESS_EX)
                GetProcAddress(Rtl->Kernel32Module, "ZwCreateProcessEx"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwCreateProcessEx'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwCreateThread = (PZW_CREATE_THREAD)
        GetProcAddress(Rtl->NtdllModule, "ZwCreateThread"))) {

        if (!(Rtl->ZwCreateThread = (PZW_CREATE_THREAD)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwCreateThread"))) {

            if (!(Rtl->ZwCreateThread = (PZW_CREATE_THREAD)
                GetProcAddress(Rtl->Kernel32Module, "ZwCreateThread"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwCreateThread'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwOpenThread = (PZW_OPEN_THREAD)
        GetProcAddress(Rtl->NtdllModule, "ZwOpenThread"))) {

        if (!(Rtl->ZwOpenThread = (PZW_OPEN_THREAD)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwOpenThread"))) {

            if (!(Rtl->ZwOpenThread = (PZW_OPEN_THREAD)
                GetProcAddress(Rtl->Kernel32Module, "ZwOpenThread"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwOpenThread'");
                return FALSE;
            }
        }
    }

    if (!(Rtl->ZwTerminateThread = (PZW_TERMINATE_THREAD)
        GetProcAddress(Rtl->NtdllModule, "ZwTerminateThread"))) {

        if (!(Rtl->ZwTerminateThread = (PZW_TERMINATE_THREAD)
            GetProcAddress(Rtl->NtosKrnlModule, "ZwTerminateThread"))) {

            if (!(Rtl->ZwTerminateThread = (PZW_TERMINATE_THREAD)
                GetProcAddress(Rtl->Kernel32Module, "ZwTerminateThread"))) {

                OutputDebugStringA("Rtl: failed to resolve 'ZwTerminateThread'");
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

    if (!(RtlExFunctions->ArgvWToArgvA = (PARGVW_TO_ARGVA)
        GetProcAddress(RtlExModule, "ArgvWToArgvA"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'ArgvWToArgvA'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyPagesMovsq = (PCOPY_PAGES_EX)
        GetProcAddress(RtlExModule, "CopyPagesMovsq"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyPagesMovsq'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyPagesAvx2 = (PCOPY_PAGES_EX)
        GetProcAddress(RtlExModule, "CopyPagesAvx2"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyPagesAvx2'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyPagesMovsq_C = (PCOPY_PAGES)
        GetProcAddress(RtlExModule, "CopyPagesMovsq_C"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyPagesMovsq_C'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyPagesAvx2_C = (PCOPY_PAGES)
        GetProcAddress(RtlExModule, "CopyPagesAvx2_C"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyPagesAvx2_C'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyPagesNonTemporalAvx2 = (PCOPY_PAGES)
        GetProcAddress(RtlExModule, "CopyPagesNonTemporalAvx2"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyPagesNonTemporalAvx2'");
        return FALSE;
    }

    if (!(RtlExFunctions->CopyToMemoryMappedMemory = (PCOPY_TO_MEMORY_MAPPED_MEMORY)
        GetProcAddress(RtlExModule, "CopyToMemoryMappedMemory"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CopyToMemoryMappedMemory'");
        return FALSE;
    }

    if (!(RtlExFunctions->CreateBitmapIndexForString = (PCREATE_BITMAP_INDEX_FOR_STRING)
        GetProcAddress(RtlExModule, "CreateBitmapIndexForString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CreateBitmapIndexForString'");
        return FALSE;
    }

    if (!(RtlExFunctions->CreateBitmapIndexForUnicodeString = (PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING)
        GetProcAddress(RtlExModule, "CreateBitmapIndexForUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CreateBitmapIndexForUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->CurrentDirectoryToRtlPath = (PCURRENT_DIRECTORY_TO_RTL_PATH)
        GetProcAddress(RtlExModule, "CurrentDirectoryToRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CurrentDirectoryToRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->CurrentDirectoryToUnicodeString = (PCURRENT_DIRECTORY_TO_UNICODE_STRING)
        GetProcAddress(RtlExModule, "CurrentDirectoryToUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'CurrentDirectoryToUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->DestroyPathEnvironmentVariable = (PDESTROY_PATH_ENVIRONMENT_VARIABLE)
        GetProcAddress(RtlExModule, "DestroyPathEnvironmentVariable"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DestroyPathEnvironmentVariable'");
        return FALSE;
    }

    if (!(RtlExFunctions->DestroyRtlPath = (PDESTROY_RTL_PATH)
        GetProcAddress(RtlExModule, "DestroyRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DestroyRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableCreateSymbolicLinkPrivilege = (PDISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableCreateSymbolicLinkPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableCreateSymbolicLinkPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableDebugPrivilege = (PDISABLE_DEBUG_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableDebugPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableDebugPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableIncreaseWorkingSetPrivilege = (PDISABLE_INCREASE_WORKING_SET_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableIncreaseWorkingSetPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableIncreaseWorkingSetPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableLockMemoryPrivilege = (PDISABLE_LOCK_MEMORY_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableLockMemoryPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableLockMemoryPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableManageVolumePrivilege = (PDISABLE_MANAGE_VOLUME_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableManageVolumePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableManageVolumePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisablePrivilege = (PDISABLE_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisablePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisablePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableProfileSingleProcessPrivilege = (PDISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableProfileSingleProcessPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableProfileSingleProcessPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->DisableSystemProfilePrivilege = (PDISABLE_SYSTEM_PROFILE_PRIVILEGE)
        GetProcAddress(RtlExModule, "DisableSystemProfilePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'DisableSystemProfilePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableCreateSymbolicLinkPrivilege = (PENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableCreateSymbolicLinkPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableCreateSymbolicLinkPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableDebugPrivilege = (PENABLE_DEBUG_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableDebugPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableDebugPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableIncreaseWorkingSetPrivilege = (PENABLE_INCREASE_WORKING_SET_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableIncreaseWorkingSetPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableIncreaseWorkingSetPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableLockMemoryPrivilege = (PENABLE_LOCK_MEMORY_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableLockMemoryPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableLockMemoryPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableManageVolumePrivilege = (PENABLE_MANAGE_VOLUME_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableManageVolumePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableManageVolumePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnablePrivilege = (PENABLE_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnablePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnablePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableProfileSingleProcessPrivilege = (PENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableProfileSingleProcessPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableProfileSingleProcessPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->EnableSystemProfilePrivilege = (PENABLE_SYSTEM_PROFILE_PRIVILEGE)
        GetProcAddress(RtlExModule, "EnableSystemProfilePrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'EnableSystemProfilePrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->FilesExistA = (PFILES_EXISTA)
        GetProcAddress(RtlExModule, "FilesExistA"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FilesExistA'");
        return FALSE;
    }

    if (!(RtlExFunctions->FilesExistW = (PFILES_EXISTW)
        GetProcAddress(RtlExModule, "FilesExistW"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FilesExistW'");
        return FALSE;
    }

    if (!(RtlExFunctions->FindCharsInString = (PFIND_CHARS_IN_STRING)
        GetProcAddress(RtlExModule, "FindCharsInString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FindCharsInString'");
        return FALSE;
    }

    if (!(RtlExFunctions->FindCharsInUnicodeString = (PFIND_CHARS_IN_UNICODE_STRING)
        GetProcAddress(RtlExModule, "FindCharsInUnicodeString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'FindCharsInUnicodeString'");
        return FALSE;
    }

    if (!(RtlExFunctions->GetModuleRtlPath = (PGET_MODULE_RTL_PATH)
        GetProcAddress(RtlExModule, "GetModuleRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'GetModuleRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->InitializeRtlFile = (PINITIALIZE_RTL_FILE)
        GetProcAddress(RtlExModule, "InitializeRtlFile"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'InitializeRtlFile'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadDbgEng = (PLOAD_DBGENG)
        GetProcAddress(RtlExModule, "LoadDbgEng"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadDbgEng'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadDbgHelp = (PLOAD_DBGHELP)
        GetProcAddress(RtlExModule, "LoadDbgHelp"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadDbgHelp'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadPathEnvironmentVariable = (PLOAD_PATH_ENVIRONMENT_VARIABLE)
        GetProcAddress(RtlExModule, "LoadPathEnvironmentVariable"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadPathEnvironmentVariable'");
        return FALSE;
    }

    if (!(RtlExFunctions->LoadShlwapi = (PLOAD_SHLWAPI)
        GetProcAddress(RtlExModule, "LoadShlwapi"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'LoadShlwapi'");
        return FALSE;
    }

    if (!(RtlExFunctions->PrefaultPages = (PPREFAULT_PAGES)
        GetProcAddress(RtlExModule, "PrefaultPages"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'PrefaultPages'");
        return FALSE;
    }

    if (!(RtlExFunctions->RegisterDllNotification = (PREGISTER_DLL_NOTIFICATION)
        GetProcAddress(RtlExModule, "RegisterDllNotification"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RegisterDllNotification'");
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

    if (!(RtlExFunctions->RtlIsRoot = (PRTL_IS_ROOT)
        GetProcAddress(RtlExModule, "RtlIsRoot"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlIsRoot'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlLeftChild = (PRTL_LEFT_CHILD)
        GetProcAddress(RtlExModule, "RtlLeftChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlLeftChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlParent = (PRTL_PARENT)
        GetProcAddress(RtlExModule, "RtlParent"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlParent'");
        return FALSE;
    }

    if (!(RtlExFunctions->RtlRightChild = (PRTL_RIGHT_CHILD)
        GetProcAddress(RtlExModule, "RtlRightChild"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'RtlRightChild'");
        return FALSE;
    }

    if (!(RtlExFunctions->SetPrivilege = (PSET_PRIVILEGE)
        GetProcAddress(RtlExModule, "SetPrivilege"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'SetPrivilege'");
        return FALSE;
    }

    if (!(RtlExFunctions->StringToExistingRtlPath = (PSTRING_TO_EXISTING_RTL_PATH)
        GetProcAddress(RtlExModule, "StringToExistingRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'StringToExistingRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->StringToRtlPath = (PSTRING_TO_RTL_PATH)
        GetProcAddress(RtlExModule, "StringToRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'StringToRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->TestExceptionHandler = (PTEST_EXCEPTION_HANDLER)
        GetProcAddress(RtlExModule, "TestExceptionHandler"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'TestExceptionHandler'");
        return FALSE;
    }

    if (!(RtlExFunctions->UnicodeStringToExistingRtlPath = (PUNICODE_STRING_TO_EXISTING_RTL_PATH)
        GetProcAddress(RtlExModule, "UnicodeStringToExistingRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'UnicodeStringToExistingRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->UnicodeStringToRtlPath = (PUNICODE_STRING_TO_RTL_PATH)
        GetProcAddress(RtlExModule, "UnicodeStringToRtlPath"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'UnicodeStringToRtlPath'");
        return FALSE;
    }

    if (!(RtlExFunctions->UnregisterDllNotification = (PUNREGISTER_DLL_NOTIFICATION)
        GetProcAddress(RtlExModule, "UnregisterDllNotification"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'UnregisterDllNotification'");
        return FALSE;
    }

    if (!(RtlExFunctions->WriteEnvVarToRegistry = (PWRITE_ENV_VAR_TO_REGISTRY)
        GetProcAddress(RtlExModule, "WriteEnvVarToRegistry"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'WriteEnvVarToRegistry'");
        return FALSE;
    }

    if (!(RtlExFunctions->WriteRegistryString = (PWRITE_REGISTRY_STRING)
        GetProcAddress(RtlExModule, "WriteRegistryString"))) {

        OutputDebugStringA("RtlEx: failed to resolve 'WriteRegistryString'");
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

_Check_return_
_Success_(return != 0)
BOOL
InitializeWindowsDirectories(
    _In_ PRTL Rtl
    )
{
    PWSTR Dest;
    ULONG_INTEGER SizeInBytesExcludingNull;
    ULONG_INTEGER SizeInBytesIncludingNull;
    ULONG_INTEGER LengthInCharsExcludingNull;
    ULONG_INTEGER LengthInCharsIncludingNull;
    PUNICODE_STRING WindowsDirectory;
    PUNICODE_STRING WindowsSxSDirectory;
    PUNICODE_STRING WindowsSystemDirectory;
    const UNICODE_STRING WinSxS = RTL_CONSTANT_STRING(L"\\WinSxS");

    //
    // Initialize aliases.
    //

    WindowsDirectory = &Rtl->WindowsDirectory;

    LengthInCharsIncludingNull.LongPart = GetWindowsDirectory(NULL, 0);
    if (!LengthInCharsIncludingNull.LongPart) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Sanity check the size isn't above MAX_USHORT.
    //

    SizeInBytesIncludingNull.LongPart = (
        LengthInCharsIncludingNull.LongPart << 1
    );

    if (SizeInBytesIncludingNull.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate space for the buffer.
    //

    WindowsDirectory->Buffer = (PWSTR)(
        HeapAlloc(
            Rtl->HeapHandle,
            0,
            SizeInBytesIncludingNull.LongPart
        )
    );

    if (!WindowsDirectory->Buffer) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Initialize lengths.
    //

    SizeInBytesExcludingNull.LongPart = (
        SizeInBytesIncludingNull.LongPart - sizeof(WCHAR)
    );
    WindowsDirectory->Length = SizeInBytesExcludingNull.LowPart;
    WindowsDirectory->MaximumLength = SizeInBytesIncludingNull.LowPart;

    //
    // Call GetWindowsDirectory() again with the newly allocated buffer.
    //

    LengthInCharsExcludingNull.LongPart = (
        GetWindowsDirectory(
            WindowsDirectory->Buffer,
            LengthInCharsIncludingNull.LongPart
        )
    );

    if (LengthInCharsExcludingNull.LongPart + 1 !=
        LengthInCharsIncludingNull.LongPart) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Now process WinSxS directory.
    //

    WindowsSxSDirectory = &Rtl->WindowsSxSDirectory;

    //
    // Add the length of the "\\WinSxS" suffix.  Use WinSxS.Length as we've
    // already accounted for the trailing NULL.
    //

    LengthInCharsIncludingNull.LongPart += WinSxS.Length >> 1;

    //
    // Convert into size in bytes.
    //

    SizeInBytesIncludingNull.LongPart = (
        LengthInCharsIncludingNull.LongPart << 1
    );

    //
    // Sanity check the size isn't above MAX_USHORT.
    //

    if (SizeInBytesIncludingNull.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate space for the buffer.
    //

    WindowsSxSDirectory->Buffer = (PWSTR)(
        HeapAlloc(
            Rtl->HeapHandle,
            0,
            SizeInBytesIncludingNull.LongPart
        )
    );

    if (!WindowsSxSDirectory->Buffer) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Initialize lengths.
    //

    SizeInBytesExcludingNull.LongPart = (
        SizeInBytesIncludingNull.LongPart - sizeof(WCHAR)
    );
    WindowsSxSDirectory->Length = SizeInBytesExcludingNull.LowPart;
    WindowsSxSDirectory->MaximumLength = SizeInBytesIncludingNull.LowPart;

    //
    // Copy the Windows directory prefix over, excluding the terminating NULL.
    //

    Dest = WindowsSxSDirectory->Buffer;
    __movsw((PWORD)Dest,
            (PWORD)WindowsDirectory->Buffer,
            WindowsDirectory->Length >> 1);

    //
    // Copy the "\\WinSxS" suffix.
    //

    Dest += (WindowsDirectory->Length >> 1);
    __movsw((PWORD)Dest,
            (PWORD)WinSxS.Buffer,
            WinSxS.Length >> 1);

    //
    // Add terminating NULL.
    //

    Dest += (WinSxS.Length >> 1);
    *Dest = L'\0';

    //
    // Sanity check things are where they should be.
    //

    if (WindowsSxSDirectory->Buffer[WindowsDirectory->Length >> 1] != L'\\') {
        __debugbreak();
    }

    if (WindowsSxSDirectory->Buffer[WindowsSxSDirectory->Length>>1] != L'\0') {
        __debugbreak();
    }

    //
    // Now do the Windows system directory.
    //

    WindowsSystemDirectory = &Rtl->WindowsSystemDirectory;

    LengthInCharsIncludingNull.LongPart = GetSystemDirectory(NULL, 0);
    if (!LengthInCharsIncludingNull.LongPart) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Sanity check the size isn't above MAX_USHORT.
    //

    SizeInBytesIncludingNull.LongPart = (
        LengthInCharsIncludingNull.LongPart << 1
    );

    if (SizeInBytesIncludingNull.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate space for the buffer.
    //

    WindowsSystemDirectory->Buffer = (PWSTR)(
        HeapAlloc(
            Rtl->HeapHandle,
            0,
            SizeInBytesIncludingNull.LongPart
        )
    );

    if (!WindowsSystemDirectory->Buffer) {
        Rtl->LastError = GetLastError();
        return FALSE;
    }

    //
    // Initialize lengths.
    //

    SizeInBytesExcludingNull.LongPart = (
        SizeInBytesIncludingNull.LongPart - sizeof(WCHAR)
    );
    WindowsSystemDirectory->Length = SizeInBytesExcludingNull.LowPart;
    WindowsSystemDirectory->MaximumLength = SizeInBytesIncludingNull.LowPart;

    //
    // Call GetSystemDirectory() again with the newly allocated buffer.
    //

    LengthInCharsExcludingNull.LongPart = (
        GetSystemDirectory(
            WindowsSystemDirectory->Buffer,
            LengthInCharsIncludingNull.LongPart
        )
    );

    if (LengthInCharsExcludingNull.LongPart + 1 !=
        LengthInCharsIncludingNull.LongPart) {
        Rtl->LastError = GetLastError();
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
    BOOL Success;
    HANDLE HeapHandle;
    PRTL_LDR_NOTIFICATION_TABLE Table;

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

    Rtl->SizeOfStruct = sizeof(*Rtl);

    SetCSpecificHandler(Rtl->NtdllModule);
    Rtl->__C_specific_handler = __C_specific_handler_impl;
    if (!Rtl->__C_specific_handler) {
        return FALSE;
    }

    Rtl->HeapHandle = HeapHandle;

    if (!LoadRtlExSymbols(NULL, Rtl)) {
        return FALSE;
    }

    if (!InitializeWindowsDirectories(Rtl)) {
        return FALSE;
    }

    Rtl->atexit = atexit_impl;
    Rtl->AtExitEx = AtExitExImpl;

    Rtl->MaximumFileSectionSize = Rtl->MmGetMaximumFileSectionSize();

    Table = Rtl->LoaderNotificationTable = (PRTL_LDR_NOTIFICATION_TABLE)(
        HeapAlloc(
            HeapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(*Rtl->LoaderNotificationTable)
        )
    );

    if (!Table) {
        return FALSE;
    }

    Success = InitializeRtlLdrNotificationTable(Rtl, Table);
    if (!Success) {
        HeapFree(HeapHandle, 0, Table);
        Rtl->LoaderNotificationTable = NULL;
    }

    Rtl->Multiplicand.QuadPart = TIMESTAMP_TO_SECONDS;
    QueryPerformanceFrequency(&Rtl->Frequency);

    Success = CryptAcquireContextW(&Rtl->CryptProv,
                                   NULL,
                                   NULL,
                                   PROV_RSA_FULL,
                                   CRYPT_VERIFYCONTEXT);

    if (!Success) {
        Rtl->LastError = GetLastError();
    }

    Rtl->InitializeCom = InitializeCom;
    Rtl->LoadDbgEng = LoadDbgEng;

    return Success;
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

_Use_decl_annotations_
PLIST_ENTRY
RemoveHeadGuardedListTsx(
    PGUARDED_LIST GuardedList
    )
{
    return RemoveHeadGuardedListTsxInline(GuardedList);
}

_Use_decl_annotations_
PLIST_ENTRY
RemoveTailGuardedListTsx(
    PGUARDED_LIST GuardedList
    )
{
    return RemoveTailGuardedListTsxInline(GuardedList);
}

_Use_decl_annotations_
VOID
InsertTailGuardedListTsx(
    PGUARDED_LIST GuardedList,
    PLIST_ENTRY Entry
    )
{
    InsertTailGuardedListTsxInline(GuardedList, Entry);
}

_Use_decl_annotations_
VOID
AppendTailGuardedListTsx(
    PGUARDED_LIST GuardedList,
    PLIST_ENTRY Entry
    )
{
    AppendTailGuardedListTsxInline(GuardedList, Entry);
}

#ifndef VECTORCALL
#define VECTORCALL __vectorcall
#endif

RTL_API
XMMWORD
VECTORCALL
DummyVectorCall1(
    _In_ XMMWORD Xmm0,
    _In_ XMMWORD Xmm1,
    _In_ XMMWORD Xmm2,
    _In_ XMMWORD Xmm3
    )
{
    XMMWORD Temp1;
    XMMWORD Temp2;
    Temp1 = _mm_xor_si128(Xmm0, Xmm1);
    Temp2 = _mm_xor_si128(Xmm2, Xmm3);
    return _mm_xor_si128(Temp1, Temp2);
}

typedef struct _TEST_HVA3 {
    XMMWORD X;
    XMMWORD Y;
    XMMWORD Z;
} TEST_HVA3;

RTL_API
TEST_HVA3
VECTORCALL
DummyHvaCall1(
    _In_ TEST_HVA3 Hva3
    )
{
    Hva3.X = _mm_xor_si128(Hva3.Y, Hva3.Z);
    return Hva3;
}

#if 0
typedef struct _TEST_HFA3 {
    DOUBLE X;
    DOUBLE Y;
    DOUBLE Z;
} TEST_HFA3;

RTL_API
TEST_HFA3
VECTORCALL
DummyHfaCall1(
    _In_ TEST_HFA3 Hfa3
)
{
    __m128d Double;
    Double = _mm_setr_pd(Hfa3.Y, Hfa3.Z);
    Hfa3.X = Double.m128d_f64[0];
    return Hfa3;
}
#endif


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
