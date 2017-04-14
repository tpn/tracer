
#include "stdafx.h"


#ifdef IMAGEAPI
#undef IMAGEAPI
#endif

#define IMAGEAPI __stdcall

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FIND_FILE_IN_PATH)(
    _In_ HANDLE hprocess,
    _In_opt_ PCSTR SearchPath,
    _In_ PCSTR FileName,
    _In_opt_ PVOID id,
    _In_ DWORD two,
    _In_ DWORD three,
    _In_ DWORD flags,
    _Out_writes_(MAX_PATH + 1) PSTR FoundFile,
    _In_opt_ PFINDFILEINPATHCALLBACK callback,
    _In_opt_ PVOID context
    );
typedef SYM_FIND_FILE_IN_PATH *PSYM_FIND_FILE_IN_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FIND_FILE_IN_PATH_W)(
    _In_ HANDLE hprocess,
    _In_opt_ PCWSTR SearchPath,
    _In_ PCWSTR FileName,
    _In_opt_ PVOID id,
    _In_ DWORD two,
    _In_ DWORD three,
    _In_ DWORD flags,
    _Out_writes_(MAX_PATH + 1) PWSTR FoundFile,
    _In_opt_ PFINDFILEINPATHCALLBACKW callback,
    _In_opt_ PVOID context
    );
typedef SYM_FIND_FILE_IN_PATH_W *PSYM_FIND_FILE_IN_PATH_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI UNMAP_DEBUG_INFORMATION)(
    _Out_writes_(_Inexpressible_(unknown)) struct _IMAGE_DEBUG_INFORMATION *DebugInfo
    );
typedef UNMAP_DEBUG_INFORMATION *PUNMAP_DEBUG_INFORMATION;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SEARCH_TREE_FOR_FILE)(
    _In_ PCSTR RootPath,
    _In_ PCSTR InputPathName,
    _Out_writes_(MAX_PATH + 1) PSTR OutputPathBuffer
    );
typedef SEARCH_TREE_FOR_FILE *PSEARCH_TREE_FOR_FILE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SEARCH_TREE_FOR_FILE_W)(
    _In_ PCWSTR RootPath,
    _In_ PCWSTR InputPathName,
    _Out_writes_(MAX_PATH + 1) PWSTR OutputPathBuffer
    );
typedef SEARCH_TREE_FOR_FILE_W *PSEARCH_TREE_FOR_FILE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUM_DIR_TREE)(
    _In_opt_ HANDLE hProcess,
    _In_ PCSTR RootPath,
    _In_ PCSTR InputPathName,
    _Out_writes_opt_(MAX_PATH + 1) PSTR OutputPathBuffer,
    _In_opt_ PENUMDIRTREE_CALLBACK cb,
    _In_opt_ PVOID data
    );
typedef ENUM_DIR_TREE *PENUM_DIR_TREE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUM_DIR_TREE_W)(
    _In_opt_ HANDLE hProcess,
    _In_ PCWSTR RootPath,
    _In_ PCWSTR InputPathName,
    _Out_writes_opt_(MAX_PATH + 1) PWSTR OutputPathBuffer,
    _In_opt_ PENUMDIRTREE_CALLBACKW cb,
    _In_opt_ PVOID data
    );
typedef ENUM_DIR_TREE_W *PENUM_DIR_TREE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI MAKE_SURE_DIRECTORY_PATH_EXISTS)(
    _In_ PCSTR DirPath
    );
typedef MAKE_SURE_DIRECTORY_PATH_EXISTS *PMAKE_SURE_DIRECTORY_PATH_EXISTS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI STACK_WALK64)(
    _In_ DWORD MachineType,
    _In_ HANDLE hProcess,
    _In_ HANDLE hThread,
    _Inout_ LPSTACKFRAME64 StackFrame,
    _Inout_ PVOID ContextRecord,
    _In_opt_ PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    _In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    _In_opt_ PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    _In_opt_ PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
    );
typedef STACK_WALK64 *PSTACK_WALK64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI STACK_WALK_EX)(
    _In_ DWORD MachineType,
    _In_ HANDLE hProcess,
    _In_ HANDLE hThread,
    _Inout_ LPSTACKFRAME_EX StackFrame,
    _Inout_ PVOID ContextRecord,
    _In_opt_ PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    _In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    _In_opt_ PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    _In_opt_ PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress,
    _In_ DWORD Flags
    );
typedef STACK_WALK_EX *PSTACK_WALK_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI STACK_WALK)(
    DWORD MachineType,
    _In_ HANDLE hProcess,
    _In_ HANDLE hThread,
    _Inout_ LPSTACKFRAME StackFrame,
    _Inout_ PVOID ContextRecord,
    _In_opt_ PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
    _In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
    _In_opt_ PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
    _In_opt_ PTRANSLATE_ADDRESS_ROUTINE TranslateAddress
    );
typedef STACK_WALK *PSTACK_WALK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_PARENT_WINDOW)(
    _In_ HWND hwnd
    );
typedef SYM_SET_PARENT_WINDOW *PSYM_SET_PARENT_WINDOW;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_OMAPS)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 BaseOfDll,
    _Out_ POMAP *OmapTo,
    _Out_ PDWORD64 cOmapTo,
    _Out_ POMAP *OmapFrom,
    _Out_ PDWORD64 cOmapFrom
    );
typedef SYM_GET_OMAPS *PSYM_GET_OMAPS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_CLEANUP)(
    _In_ HANDLE hProcess
    );
typedef SYM_CLEANUP *PSYM_CLEANUP;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_MATCH_STRING)(
    _In_ PCSTR string,
    _In_ PCSTR expression,
    _In_ BOOL fCase
    );
typedef SYM_MATCH_STRING *PSYM_MATCH_STRING;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_MATCH_STRING_A)(
    _In_ PCSTR string,
    _In_ PCSTR expression,
    _In_ BOOL fCase
    );
typedef SYM_MATCH_STRING_A *PSYM_MATCH_STRING_A;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_MATCH_STRING_W)(
    _In_ PCWSTR string,
    _In_ PCWSTR expression,
    _In_ BOOL fCase
    );
typedef SYM_MATCH_STRING_W *PSYM_MATCH_STRING_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SOURCE_FILES)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 ModBase,
    _In_opt_ PCSTR Mask,
    _In_ PSYM_ENUMSOURCEFILES_CALLBACK cbSrcFiles,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SOURCE_FILES *PSYM_ENUM_SOURCE_FILES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SOURCE_FILES_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 ModBase,
    _In_opt_ PCWSTR Mask,
    _In_ PSYM_ENUMSOURCEFILES_CALLBACKW cbSrcFiles,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SOURCE_FILES_W *PSYM_ENUM_SOURCE_FILES_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_MODULES64)(
    _In_ HANDLE hProcess,
    _In_ PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_MODULES64 *PSYM_ENUMERATE_MODULES64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_MODULES_W64)(
    _In_ HANDLE hProcess,
    _In_ PSYM_ENUMMODULES_CALLBACKW64 EnumModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_MODULES_W64 *PSYM_ENUMERATE_MODULES_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_MODULES)(
    _In_ HANDLE hProcess,
    _In_ PSYM_ENUMMODULES_CALLBACK EnumModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_MODULES *PSYM_ENUMERATE_MODULES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUMERATE_LOADED_MODULES_EX)(
    _In_ HANDLE hProcess,
    _In_ PENUMLOADED_MODULES_CALLBACK64 EnumLoadedModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef ENUMERATE_LOADED_MODULES_EX *PENUMERATE_LOADED_MODULES_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUMERATE_LOADED_MODULES_EX_W)(
    _In_ HANDLE hProcess,
    _In_ PENUMLOADED_MODULES_CALLBACKW64 EnumLoadedModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef ENUMERATE_LOADED_MODULES_EX_W *PENUMERATE_LOADED_MODULES_EX_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUMERATE_LOADED_MODULES64)(
    _In_ HANDLE hProcess,
    _In_ PENUMLOADED_MODULES_CALLBACK64 EnumLoadedModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef ENUMERATE_LOADED_MODULES64 *PENUMERATE_LOADED_MODULES64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUMERATE_LOADED_MODULES_W64)(
    _In_ HANDLE hProcess,
    _In_ PENUMLOADED_MODULES_CALLBACKW64 EnumLoadedModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef ENUMERATE_LOADED_MODULES_W64 *PENUMERATE_LOADED_MODULES_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI ENUMERATE_LOADED_MODULES)(
    _In_ HANDLE hProcess,
    _In_ PENUMLOADED_MODULES_CALLBACK EnumLoadedModulesCallback,
    _In_opt_ PVOID UserContext
    );
typedef ENUMERATE_LOADED_MODULES *PENUMERATE_LOADED_MODULES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_UNWIND_INFO)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _Out_writes_bytes_opt_(*Size) PVOID Buffer,
    _Inout_ PULONG Size
    );
typedef SYM_GET_UNWIND_INFO *PSYM_GET_UNWIND_INFO;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_MODULE_INFO64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 qwAddr,
    _Out_ PIMAGEHLP_MODULE64 ModuleInfo
    );
typedef SYM_GET_MODULE_INFO64 *PSYM_GET_MODULE_INFO64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_MODULE_INFO_W64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 qwAddr,
    _Out_ PIMAGEHLP_MODULEW64 ModuleInfo
    );
typedef SYM_GET_MODULE_INFO_W64 *PSYM_GET_MODULE_INFO_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_MODULE_INFO)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr,
    _Out_ PIMAGEHLP_MODULE ModuleInfo
    );
typedef SYM_GET_MODULE_INFO *PSYM_GET_MODULE_INFO;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_MODULE_INFO_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr,
    _Out_ PIMAGEHLP_MODULEW ModuleInfo
    );
typedef SYM_GET_MODULE_INFO_W *PSYM_GET_MODULE_INFO_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_LINES)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCSTR Obj,
    _In_opt_ PCSTR File,
    _In_ PSYM_ENUMLINES_CALLBACK EnumLinesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_LINES *PSYM_ENUM_LINES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_LINES_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCWSTR Obj,
    _In_opt_ PCWSTR File,
    _In_ PSYM_ENUMLINES_CALLBACKW EnumLinesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_LINES_W *PSYM_ENUM_LINES_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_ADDR64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 qwAddr,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINE64 Line64
    );
typedef SYM_GET_LINE_FROM_ADDR64 *PSYM_GET_LINE_FROM_ADDR64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_ADDR_W64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 dwAddr,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINEW64 Line
    );
typedef SYM_GET_LINE_FROM_ADDR_W64 *PSYM_GET_LINE_FROM_ADDR_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_INLINE_CONTEXT)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 qwAddr,
    _In_ ULONG InlineContext,
    _In_opt_ DWORD64 qwModuleBaseAddress,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINE64 Line64
    );
typedef SYM_GET_LINE_FROM_INLINE_CONTEXT *PSYM_GET_LINE_FROM_INLINE_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_INLINE_CONTEXT_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 dwAddr,
    _In_ ULONG InlineContext,
    _In_opt_ DWORD64 qwModuleBaseAddress,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINEW64 Line
    );
typedef SYM_GET_LINE_FROM_INLINE_CONTEXT_W *PSYM_GET_LINE_FROM_INLINE_CONTEXT_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SOURCE_LINES)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCSTR Obj,
    _In_opt_ PCSTR File,
    _In_opt_ DWORD Line,
    _In_ DWORD Flags,
    _In_ PSYM_ENUMLINES_CALLBACK EnumLinesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SOURCE_LINES *PSYM_ENUM_SOURCE_LINES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SOURCE_LINES_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCWSTR Obj,
    _In_opt_ PCWSTR File,
    _In_opt_ DWORD Line,
    _In_ DWORD Flags,
    _In_ PSYM_ENUMLINES_CALLBACKW EnumLinesCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SOURCE_LINES_W *PSYM_ENUM_SOURCE_LINES_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_QUERY_INLINE_TRACE)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 StartAddress,
    _In_ DWORD StartContext,
    _In_ DWORD64 StartRetAddress,
    _In_ DWORD64 CurAddress,
    _Out_ LPDWORD CurContext,
    _Out_ LPDWORD CurFrameIndex
    );
typedef SYM_QUERY_INLINE_TRACE *PSYM_QUERY_INLINE_TRACE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_ADDR)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINE Line
    );
typedef SYM_GET_LINE_FROM_ADDR *PSYM_GET_LINE_FROM_ADDR;

#define IMAGEHLP_LINEW IMAGEHLP_LINEW64
#define PIMAGEHLP_LINEW PIMAGEHLP_LINEW64

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_ADDR_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr,
    _Out_ PDWORD pdwDisplacement,
    _Out_ PIMAGEHLP_LINEW Line
    );
typedef SYM_GET_LINE_FROM_ADDR_W *PSYM_GET_LINE_FROM_ADDR_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_NAME64)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR ModuleName,
    _In_opt_ PCSTR FileName,
    _In_ DWORD dwLineNumber,
    _Out_ PLONG plDisplacement,
    _Inout_ PIMAGEHLP_LINE64 Line
    );
typedef SYM_GET_LINE_FROM_NAME64 *PSYM_GET_LINE_FROM_NAME64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_NAME_W64)(
    _In_ HANDLE hProcess,
    _In_opt_ PCWSTR ModuleName,
    _In_opt_ PCWSTR FileName,
    _In_ DWORD dwLineNumber,
    _Out_ PLONG plDisplacement,
    _Inout_ PIMAGEHLP_LINEW64 Line
    );
typedef SYM_GET_LINE_FROM_NAME_W64 *PSYM_GET_LINE_FROM_NAME_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_FROM_NAME)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR ModuleName,
    _In_opt_ PCSTR FileName,
    _In_ DWORD dwLineNumber,
    _Out_ PLONG plDisplacement,
    _Inout_ PIMAGEHLP_LINE Line
    );
typedef SYM_GET_LINE_FROM_NAME *PSYM_GET_LINE_FROM_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_NEXT64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINE64 Line
    );
typedef SYM_GET_LINE_NEXT64 *PSYM_GET_LINE_NEXT64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_NEXT_W64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINEW64 Line
    );
typedef SYM_GET_LINE_NEXT_W64 *PSYM_GET_LINE_NEXT_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_NEXT)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINE Line
    );
typedef SYM_GET_LINE_NEXT *PSYM_GET_LINE_NEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_NEXT_W)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINEW Line
    );
typedef SYM_GET_LINE_NEXT_W *PSYM_GET_LINE_NEXT_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_PREV64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINE64 Line
    );
typedef SYM_GET_LINE_PREV64 *PSYM_GET_LINE_PREV64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_PREV_W64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINEW64 Line
    );
typedef SYM_GET_LINE_PREV_W64 *PSYM_GET_LINE_PREV_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_PREV)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINE Line
    );
typedef SYM_GET_LINE_PREV *PSYM_GET_LINE_PREV;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_LINE_PREV_W)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_LINEW Line
    );
typedef SYM_GET_LINE_PREV_W *PSYM_GET_LINE_PREV_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_MATCH_FILE_NAME)(
    _In_ PCSTR FileName,
    _In_ PCSTR Match,
    _Outptr_opt_ PSTR *FileNameStop,
    _Outptr_opt_ PSTR *MatchStop
    );
typedef SYM_MATCH_FILE_NAME *PSYM_MATCH_FILE_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_MATCH_FILE_NAME_W)(
    _In_ PCWSTR FileName,
    _In_ PCWSTR Match,
    _Outptr_opt_ PWSTR *FileNameStop,
    _Outptr_opt_ PWSTR *MatchStop
    );
typedef SYM_MATCH_FILE_NAME_W *PSYM_MATCH_FILE_NAME_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCSTR Params,
    _In_ PCSTR FileSpec,
    _Out_writes_(Size) PSTR FilePath,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_FILE *PSYM_GET_SOURCE_FILE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCWSTR Params,
    _In_ PCWSTR FileSpec,
    _Out_writes_(Size) PWSTR FilePath,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_FILE_W *PSYM_GET_SOURCE_FILE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE_TOKEN)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_ PCSTR FileSpec,
    _Outptr_ PVOID *Token,
    _Out_ DWORD *Size
    );
typedef SYM_GET_SOURCE_FILE_TOKEN *PSYM_GET_SOURCE_FILE_TOKEN;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE_TOKEN_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_ PCWSTR FileSpec,
    _Outptr_ PVOID *Token,
    _Out_ DWORD *Size
    );
typedef SYM_GET_SOURCE_FILE_TOKEN_W *PSYM_GET_SOURCE_FILE_TOKEN_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE_FROM_TOKEN)(
    _In_ HANDLE hProcess,
    _In_ PVOID Token,
    _In_opt_ PCSTR Params,
    _Out_writes_(Size) PSTR FilePath,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_FILE_FROM_TOKEN *PSYM_GET_SOURCE_FILE_FROM_TOKEN;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_FILE_FROM_TOKEN_W)(
    _In_ HANDLE hProcess,
    _In_ PVOID Token,
    _In_opt_ PCWSTR Params,
    _Out_writes_(Size) PWSTR FilePath,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_FILE_FROM_TOKEN_W *PSYM_GET_SOURCE_FILE_FROM_TOKEN_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_VAR_FROM_TOKEN)(
    _In_ HANDLE hProcess,
    _In_ PVOID Token,
    _In_opt_ PCSTR Params,
    _In_ PCSTR VarName,
    _Out_writes_(Size) PSTR Value,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_VAR_FROM_TOKEN *PSYM_GET_SOURCE_VAR_FROM_TOKEN;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SOURCE_VAR_FROM_TOKEN_W)(
    _In_ HANDLE hProcess,
    _In_ PVOID Token,
    _In_opt_ PCWSTR Params,
    _In_ PCWSTR VarName,
    _Out_writes_(Size) PWSTR Value,
    _In_ DWORD Size
    );
typedef SYM_GET_SOURCE_VAR_FROM_TOKEN_W *PSYM_GET_SOURCE_VAR_FROM_TOKEN_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SOURCE_FILE_TOKENS)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_ PENUMSOURCEFILETOKENSCALLBACK Callback
    );
typedef SYM_ENUM_SOURCE_FILE_TOKENS *PSYM_ENUM_SOURCE_FILE_TOKENS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_INITIALIZE)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR UserSearchPath,
    _In_ BOOL fInvadeProcess
    );
typedef SYM_INITIALIZE *PSYM_INITIALIZE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_INITIALIZE_W)(
    _In_ HANDLE hProcess,
    _In_opt_ PCWSTR UserSearchPath,
    _In_ BOOL fInvadeProcess
    );
typedef SYM_INITIALIZE_W *PSYM_INITIALIZE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SEARCH_PATH)(
    _In_ HANDLE hProcess,
    _Out_writes_(SearchPathLength) PSTR SearchPath,
    _In_ DWORD SearchPathLength
    );
typedef SYM_GET_SEARCH_PATH *PSYM_GET_SEARCH_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SEARCH_PATH_W)(
    _In_ HANDLE hProcess,
    _Out_writes_(SearchPathLength) PWSTR SearchPath,
    _In_ DWORD SearchPathLength
    );
typedef SYM_GET_SEARCH_PATH_W *PSYM_GET_SEARCH_PATH_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_SEARCH_PATH)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR SearchPath
    );
typedef SYM_SET_SEARCH_PATH *PSYM_SET_SEARCH_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_SEARCH_PATH_W)(
    _In_ HANDLE hProcess,
    _In_opt_ PCWSTR SearchPath
    );
typedef SYM_SET_SEARCH_PATH_W *PSYM_SET_SEARCH_PATH_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UNLOAD_MODULE64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 BaseOfDll
    );
typedef SYM_UNLOAD_MODULE64 *PSYM_UNLOAD_MODULE64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UNLOAD_MODULE)(
    _In_ HANDLE hProcess,
    _In_ DWORD BaseOfDll
    );
typedef SYM_UNLOAD_MODULE *PSYM_UNLOAD_MODULE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UN_D_NAME64)(
    _In_ PIMAGEHLP_SYMBOL64 sym,            // Symbol to undecorate
    _Out_writes_(UnDecNameLength) PSTR UnDecName,   // Buffer to store undecorated name in
    _In_ DWORD UnDecNameLength              // Size of the buffer
    );
typedef SYM_UN_D_NAME64 *PSYM_UN_D_NAME64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UN_D_NAME)(
    _In_ PIMAGEHLP_SYMBOL sym,              // Symbol to undecorate
    _Out_writes_(UnDecNameLength) PSTR UnDecName,   // Buffer to store undecorated name in
    _In_ DWORD UnDecNameLength              // Size of the buffer
    );
typedef SYM_UN_D_NAME *PSYM_UN_D_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REGISTER_CALLBACK64)(
    _In_ HANDLE hProcess,
    _In_ PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    _In_ ULONG64 UserContext
    );
typedef SYM_REGISTER_CALLBACK64 *PSYM_REGISTER_CALLBACK64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REGISTER_CALLBACK_W64)(
    _In_ HANDLE hProcess,
    _In_ PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    _In_ ULONG64 UserContext
    );
typedef SYM_REGISTER_CALLBACK_W64 *PSYM_REGISTER_CALLBACK_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REGISTER_FUNCTION_ENTRY_CALLBACK64)(
    _In_ HANDLE hProcess,
    _In_ PSYMBOL_FUNCENTRY_CALLBACK64 CallbackFunction,
    _In_ ULONG64 UserContext
    );
typedef SYM_REGISTER_FUNCTION_ENTRY_CALLBACK64 *PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REGISTER_CALLBACK)(
    _In_ HANDLE hProcess,
    _In_ PSYMBOL_REGISTERED_CALLBACK CallbackFunction,
    _In_opt_ PVOID UserContext
    );
typedef SYM_REGISTER_CALLBACK *PSYM_REGISTER_CALLBACK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REGISTER_FUNCTION_ENTRY_CALLBACK)(
    _In_ HANDLE hProcess,
    _In_ PSYMBOL_FUNCENTRY_CALLBACK CallbackFunction,
    _In_opt_ PVOID UserContext
    );
typedef SYM_REGISTER_FUNCTION_ENTRY_CALLBACK *PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_CONTEXT)(
    _In_ HANDLE hProcess,
    _In_ PIMAGEHLP_STACK_FRAME StackFrame,
    _In_opt_ PIMAGEHLP_CONTEXT Context
    );
typedef SYM_SET_CONTEXT *PSYM_SET_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_SCOPE_FROM_ADDR)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Address
    );
typedef SYM_SET_SCOPE_FROM_ADDR *PSYM_SET_SCOPE_FROM_ADDR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_SCOPE_FROM_INLINE_CONTEXT)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Address,
    _In_ ULONG InlineContext
    );
typedef SYM_SET_SCOPE_FROM_INLINE_CONTEXT *PSYM_SET_SCOPE_FROM_INLINE_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SET_SCOPE_FROM_INDEX)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ DWORD Index
    );
typedef SYM_SET_SCOPE_FROM_INDEX *PSYM_SET_SCOPE_FROM_INDEX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_PROCESSES)(
    _In_ PSYM_ENUMPROCESSES_CALLBACK EnumProcessesCallback,
    _In_ PVOID UserContext
    );
typedef SYM_ENUM_PROCESSES *PSYM_ENUM_PROCESSES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_ADDR)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _Out_opt_ PDWORD64 Displacement,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_FROM_ADDR *PSYM_FROM_ADDR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_ADDR_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _Out_opt_ PDWORD64 Displacement,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_FROM_ADDR_W *PSYM_FROM_ADDR_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_INLINE_CONTEXT)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _In_ ULONG InlineContext,
    _Out_opt_ PDWORD64 Displacement,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_FROM_INLINE_CONTEXT *PSYM_FROM_INLINE_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_INLINE_CONTEXT_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _In_ ULONG InlineContext,
    _Out_opt_ PDWORD64 Displacement,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_FROM_INLINE_CONTEXT_W *PSYM_FROM_INLINE_CONTEXT_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_TOKEN)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Base,
    _In_ DWORD Token,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_FROM_TOKEN *PSYM_FROM_TOKEN;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_TOKEN_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Base,
    _In_ DWORD Token,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_FROM_TOKEN_W *PSYM_FROM_TOKEN_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_NEXT)(
    _In_ HANDLE hProcess,
    _Inout_ PSYMBOL_INFO si
    );
typedef SYM_NEXT *PSYM_NEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_NEXT_W)(
    _In_ HANDLE hProcess,
    _Inout_ PSYMBOL_INFOW siw
    );
typedef SYM_NEXT_W *PSYM_NEXT_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_PREV)(
    _In_ HANDLE hProcess,
    _Inout_ PSYMBOL_INFO si
    );
typedef SYM_PREV *PSYM_PREV;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_PREV_W)(
    _In_ HANDLE hProcess,
    _Inout_ PSYMBOL_INFOW siw
    );
typedef SYM_PREV_W *PSYM_PREV_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_NAME)(
    _In_ HANDLE hProcess,
    _In_ PCSTR Name,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_FROM_NAME *PSYM_FROM_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_NAME_W)(
    _In_ HANDLE hProcess,
    _In_ PCWSTR Name,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_FROM_NAME_W *PSYM_FROM_NAME_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCSTR Mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SYMBOLS *PSYM_ENUM_SYMBOLS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS_EX)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCSTR Mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext,
    _In_ DWORD Options
    );
typedef SYM_ENUM_SYMBOLS_EX *PSYM_ENUM_SYMBOLS_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCWSTR Mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SYMBOLS_W *PSYM_ENUM_SYMBOLS_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS_EX_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCWSTR Mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext,
    _In_ DWORD Options
    );
typedef SYM_ENUM_SYMBOLS_EX_W *PSYM_ENUM_SYMBOLS_EX_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS_FOR_ADDR)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SYMBOLS_FOR_ADDR *PSYM_ENUM_SYMBOLS_FOR_ADDR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYMBOLS_FOR_ADDR_W)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SYMBOLS_FOR_ADDR_W *PSYM_ENUM_SYMBOLS_FOR_ADDR_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SEARCH)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ DWORD Index,
    _In_opt_ DWORD SymTag,
    _In_opt_ PCSTR Mask,
    _In_opt_ DWORD64 Address,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext,
    _In_ DWORD Options
    );
typedef SYM_SEARCH *PSYM_SEARCH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SEARCH_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ DWORD Index,
    _In_opt_ DWORD SymTag,
    _In_opt_ PCWSTR Mask,
    _In_opt_ DWORD64 Address,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext,
    _In_ DWORD Options
    );
typedef SYM_SEARCH_W *PSYM_SEARCH_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SCOPE)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ DWORD Index,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_GET_SCOPE *PSYM_GET_SCOPE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SCOPE_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ DWORD Index,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_GET_SCOPE_W *PSYM_GET_SCOPE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_INDEX)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ DWORD Index,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_FROM_INDEX *PSYM_FROM_INDEX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_FROM_INDEX_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ DWORD Index,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_FROM_INDEX_W *PSYM_FROM_INDEX_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_TYPE_INFO)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 ModBase,
    _In_ ULONG TypeId,
    _In_ IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    _Out_ PVOID pInfo
    );
typedef SYM_GET_TYPE_INFO *PSYM_GET_TYPE_INFO;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_TYPE_INFO_EX)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 ModBase,
    _Inout_ PIMAGEHLP_GET_TYPE_INFO_PARAMS Params
    );
typedef SYM_GET_TYPE_INFO_EX *PSYM_GET_TYPE_INFO_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_TYPES)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_TYPES *PSYM_ENUM_TYPES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_TYPES_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_TYPES_W *PSYM_ENUM_TYPES_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_TYPES_BY_NAME)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCSTR mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_TYPES_BY_NAME *PSYM_ENUM_TYPES_BY_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_TYPES_BY_NAME_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCWSTR mask,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_TYPES_BY_NAME_W *PSYM_ENUM_TYPES_BY_NAME_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_TYPE_FROM_NAME)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PCSTR Name,
    _Inout_ PSYMBOL_INFO Symbol
    );
typedef SYM_GET_TYPE_FROM_NAME *PSYM_GET_TYPE_FROM_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_TYPE_FROM_NAME_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PCWSTR Name,
    _Inout_ PSYMBOL_INFOW Symbol
    );
typedef SYM_GET_TYPE_FROM_NAME_W *PSYM_GET_TYPE_FROM_NAME_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ADD_SYMBOL)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PCSTR Name,
    _In_ DWORD64 Address,
    _In_ DWORD Size,
    _In_ DWORD Flags
    );
typedef SYM_ADD_SYMBOL *PSYM_ADD_SYMBOL;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ADD_SYMBOL_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PCWSTR Name,
    _In_ DWORD64 Address,
    _In_ DWORD Size,
    _In_ DWORD Flags
    );
typedef SYM_ADD_SYMBOL_W *PSYM_ADD_SYMBOL_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_DELETE_SYMBOL)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCSTR Name,
    _In_ DWORD64 Address,
    _In_ DWORD Flags
    );
typedef SYM_DELETE_SYMBOL *PSYM_DELETE_SYMBOL;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_DELETE_SYMBOL_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_opt_ PCWSTR Name,
    _In_ DWORD64 Address,
    _In_ DWORD Flags
    );
typedef SYM_DELETE_SYMBOL_W *PSYM_DELETE_SYMBOL_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_REFRESH_MODULE_LIST)(
    _In_ HANDLE hProcess
    );
typedef SYM_REFRESH_MODULE_LIST *PSYM_REFRESH_MODULE_LIST;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ADD_SOURCE_STREAM)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCSTR StreamFile,
    _In_reads_bytes_opt_(Size) PBYTE Buffer,
    _In_ size_t Size
    );
typedef SYM_ADD_SOURCE_STREAM *PSYM_ADD_SOURCE_STREAM;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ADD_SOURCE_STREAM_A)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCSTR StreamFile,
    _In_reads_bytes_opt_(Size) PBYTE Buffer,
    _In_ size_t Size
    );
typedef SYM_ADD_SOURCE_STREAM_A *PSYM_ADD_SOURCE_STREAM_A;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ADD_SOURCE_STREAM_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 Base,
    _In_opt_ PCWSTR FileSpec,
    _In_reads_bytes_opt_(Size) PBYTE Buffer,
    _In_ size_t Size
    );
typedef SYM_ADD_SOURCE_STREAM_W *PSYM_ADD_SOURCE_STREAM_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_IS_STORE_W)(
    _In_opt_ HANDLE hProcess,
    _In_ PCWSTR path
    );
typedef SYM_SRV_IS_STORE_W *PSYM_SRV_IS_STORE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_IS_STORE)(
    _In_opt_ HANDLE hProcess,
    _In_ PCSTR path
    );
typedef SYM_SRV_IS_STORE *PSYM_SRV_IS_STORE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEXES)(
    _In_ PCSTR File,
    _Out_ GUID *Id,
    _Out_ PDWORD Val1,
    _Out_opt_ PDWORD Val2,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEXES *PSYM_SRV_GET_FILE_INDEXES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEXES_W)(
    _In_ PCWSTR File,
    _Out_ GUID *Id,
    _Out_ PDWORD Val1,
    _Out_opt_ PDWORD Val2,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEXES_W *PSYM_SRV_GET_FILE_INDEXES_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEX_STRING_W)(
    _In_ HANDLE hProcess,
    _In_opt_ PCWSTR SrvPath,
    _In_ PCWSTR File,
    _Out_writes_(Size) PWSTR Index,
    _In_ size_t Size,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEX_STRING_W *PSYM_SRV_GET_FILE_INDEX_STRING_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEX_STRING)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR SrvPath,
    _In_ PCSTR File,
    _Out_writes_(Size) PSTR Index,
    _In_ size_t Size,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEX_STRING *PSYM_SRV_GET_FILE_INDEX_STRING;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEX_INFO)(
    _In_ PCSTR File,
    _Out_ PSYMSRV_INDEX_INFO Info,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEX_INFO *PSYM_SRV_GET_FILE_INDEX_INFO;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_SRV_GET_FILE_INDEX_INFO_W)(
    _In_ PCWSTR File,
    _Out_ PSYMSRV_INDEX_INFOW Info,
    _In_ DWORD Flags
    );
typedef SYM_SRV_GET_FILE_INDEX_INFO_W *PSYM_SRV_GET_FILE_INDEX_INFO_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYMBOL_FILE)(
    _In_opt_ HANDLE hProcess,
    _In_opt_ PCSTR SymPath,
    _In_ PCSTR ImageFile,
    _In_ DWORD Type,
    _Out_writes_(cSymbolFile) PSTR SymbolFile,
    _In_ size_t cSymbolFile,
    _Out_writes_(cDbgFile) PSTR DbgFile,
    _In_ size_t cDbgFile
    );
typedef SYM_GET_SYMBOL_FILE *PSYM_GET_SYMBOL_FILE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYMBOL_FILE_W)(
    _In_opt_ HANDLE hProcess,
    _In_opt_ PCWSTR SymPath,
    _In_ PCWSTR ImageFile,
    _In_ DWORD Type,
    _Out_writes_(cSymbolFile) PWSTR SymbolFile,
    _In_ size_t cSymbolFile,
    _Out_writes_(cDbgFile) PWSTR DbgFile,
    _In_ size_t cDbgFile
    );
typedef SYM_GET_SYMBOL_FILE_W *PSYM_GET_SYMBOL_FILE_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_FROM_ADDR64)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 qwAddr,
    _Out_opt_ PDWORD64 pdwDisplacement,
    _Inout_ PIMAGEHLP_SYMBOL64  Symbol
    );
typedef SYM_GET_SYM_FROM_ADDR64 *PSYM_GET_SYM_FROM_ADDR64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_FROM_ADDR)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr,
    _Out_opt_ PDWORD pdwDisplacement,
    _Inout_ PIMAGEHLP_SYMBOL Symbol
    );
typedef SYM_GET_SYM_FROM_ADDR *PSYM_GET_SYM_FROM_ADDR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_FROM_NAME64)(
    _In_ HANDLE hProcess,
    _In_ PCSTR Name,
    _Inout_ PIMAGEHLP_SYMBOL64 Symbol
    );
typedef SYM_GET_SYM_FROM_NAME64 *PSYM_GET_SYM_FROM_NAME64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_FROM_NAME)(
    _In_ HANDLE hProcess,
    _In_ PCSTR Name,
    _Inout_ PIMAGEHLP_SYMBOL Symbol
    );
typedef SYM_GET_SYM_FROM_NAME *PSYM_GET_SYM_FROM_NAME;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI FIND_FILE_IN_PATH)(
    _In_ HANDLE hprocess,
    _In_ PCSTR SearchPath,
    _In_ PCSTR FileName,
    _In_ PVOID id,
    _In_ DWORD two,
    _In_ DWORD three,
    _In_ DWORD flags,
    _Out_writes_(MAX_PATH + 1) PSTR FilePath
    );
typedef FIND_FILE_IN_PATH *PFIND_FILE_IN_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI FIND_FILE_IN_SEARCH_PATH)(
    _In_ HANDLE hprocess,
    _In_ PCSTR SearchPath,
    _In_ PCSTR FileName,
    _In_ DWORD one,
    _In_ DWORD two,
    _In_ DWORD three,
    _Out_writes_(MAX_PATH + 1) PSTR FilePath
    );
typedef FIND_FILE_IN_SEARCH_PATH *PFIND_FILE_IN_SEARCH_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUM_SYM)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUM_SYM *PSYM_ENUM_SYM;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_SYMBOLS64)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_SYMBOLS64 *PSYM_ENUMERATE_SYMBOLS64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_SYMBOLS_W64)(
    _In_ HANDLE hProcess,
    _In_ ULONG64 BaseOfDll,
    _In_ PSYM_ENUMSYMBOLS_CALLBACK64W EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_SYMBOLS_W64 *PSYM_ENUMERATE_SYMBOLS_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_SYMBOLS)(
    _In_ HANDLE hProcess,
    _In_ ULONG BaseOfDll,
    _In_ PSYM_ENUMSYMBOLS_CALLBACK EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_SYMBOLS *PSYM_ENUMERATE_SYMBOLS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_ENUMERATE_SYMBOLS_W)(
    _In_ HANDLE hProcess,
    _In_ ULONG BaseOfDll,
    _In_ PSYM_ENUMSYMBOLS_CALLBACKW EnumSymbolsCallback,
    _In_opt_ PVOID UserContext
    );
typedef SYM_ENUMERATE_SYMBOLS_W *PSYM_ENUMERATE_SYMBOLS_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_NEXT64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOL64 Symbol
    );
typedef SYM_GET_SYM_NEXT64 *PSYM_GET_SYM_NEXT64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_NEXT_W64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOLW64 Symbol
    );
typedef SYM_GET_SYM_NEXT_W64 *PSYM_GET_SYM_NEXT_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_NEXT)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOL Symbol
    );
typedef SYM_GET_SYM_NEXT *PSYM_GET_SYM_NEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_NEXT_W)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOLW Symbol
    );
typedef SYM_GET_SYM_NEXT_W *PSYM_GET_SYM_NEXT_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_PREV64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOL64 Symbol
    );
typedef SYM_GET_SYM_PREV64 *PSYM_GET_SYM_PREV64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_PREV_W64)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOLW64 Symbol
    );
typedef SYM_GET_SYM_PREV_W64 *PSYM_GET_SYM_PREV_W64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_PREV)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOL Symbol
    );
typedef SYM_GET_SYM_PREV *PSYM_GET_SYM_PREV;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_GET_SYM_PREV_W)(
    _In_ HANDLE hProcess,
    _Inout_ PIMAGEHLP_SYMBOLW Symbol
    );
typedef SYM_GET_SYM_PREV_W *PSYM_GET_SYM_PREV_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI REPORT_SYMBOL_LOAD_SUMMARY)(
    _In_ HANDLE hProcess,
    _In_opt_ PCWSTR pLoadModule,
    _In_ PDBGHELP_DATA_REPORT_STRUCT pSymbolData
    );
typedef REPORT_SYMBOL_LOAD_SUMMARY *PREPORT_SYMBOL_LOAD_SUMMARY;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI RANGE_MAP_ADD_PE_IMAGE_SECTIONS)(
    _In_ PVOID RmapHandle,
    _In_opt_ PCWSTR ImageName,
    _In_reads_bytes_(MappingBytes) PVOID MappedImage,
    _In_ DWORD MappingBytes,
    _In_ DWORD64 ImageBase,
    _In_ DWORD64 UserTag,
    _In_ DWORD MappingFlags
    );
typedef RANGE_MAP_ADD_PE_IMAGE_SECTIONS *PRANGE_MAP_ADD_PE_IMAGE_SECTIONS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI RANGE_MAP_REMOVE)(
    _In_ PVOID RmapHandle,
    _In_ DWORD64 UserTag
    );
typedef RANGE_MAP_REMOVE *PRANGE_MAP_REMOVE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI RANGE_MAP_READ)(
    _In_ PVOID RmapHandle,
    _In_ DWORD64 Offset,
    _Out_writes_bytes_to_(RequestBytes, *DoneBytes) PVOID Buffer,
    _In_ DWORD RequestBytes,
    _In_ DWORD Flags,
    _Out_opt_ PDWORD DoneBytes
    );
typedef RANGE_MAP_READ *PRANGE_MAP_READ;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI RANGE_MAP_WRITE)(
    _In_ PVOID RmapHandle,
    _In_ DWORD64 Offset,
    _In_reads_bytes_(RequestBytes) PVOID Buffer,
    _In_ DWORD RequestBytes,
    _In_ DWORD Flags,
    _Out_opt_ PDWORD DoneBytes
    );
typedef RANGE_MAP_WRITE *PRANGE_MAP_WRITE;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(WINAPI UNDECORATE_SYMBOL_NAME)(
    _In_ PCSTR name,
    _Out_writes_(maxStringLength) PSTR outputString,
    _In_ DWORD maxStringLength,
    _In_ DWORD flags
    );
typedef UNDECORATE_SYMBOL_NAME *PUNDECORATE_SYMBOL_NAME;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(WINAPI UNDECORATE_SYMBOL_NAME_W)(
    _In_ PCWSTR name,
    _Out_writes_(maxStringLength) PWSTR outputString,
    _In_ DWORD maxStringLength,
    _In_ DWORD flags
    );
typedef UNDECORATE_SYMBOL_NAME_W *PUNDECORATE_SYMBOL_NAME_W;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UNDNAME64)(
    _In_ PIMAGEHLP_SYMBOL64 sym,
    _Out_writes_(UnDecNameLength) PSTR UnDecName,
    _In_ DWORD UnDecNameLength
    );
typedef SYM_UNDNAME64 *PSYM_UNDNAME64;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(IMAGEAPI SYM_UNDNAME)(
    _In_ PIMAGEHLP_SYMBOL sym,
    _Out_writes_(UnDecNameLength) PSTR UnDecName,
    _In_ DWORD UnDecNameLength
    );
typedef SYM_UNDNAME *PSYM_UNDNAME;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI GET_TIMESTAMP_FOR_LOADED_LIBRARY)(
    _In_ HMODULE Module
    );
typedef GET_TIMESTAMP_FOR_LOADED_LIBRARY *PGET_TIMESTAMP_FOR_LOADED_LIBRARY;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_SET_OPTIONS)(
    _In_ DWORD   SymOptions
    );
typedef SYM_SET_OPTIONS *PSYM_SET_OPTIONS;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_GET_OPTIONS)(
    VOID
    );
typedef SYM_GET_OPTIONS *PSYM_GET_OPTIONS;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_GET_MODULE_BASE)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwAddr
    );
typedef SYM_GET_MODULE_BASE *PSYM_GET_MODULE_BASE;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_ADDR_INCLUDE_INLINE_TRACE)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address
    );
typedef SYM_ADDR_INCLUDE_INLINE_TRACE *PSYM_ADDR_INCLUDE_INLINE_TRACE;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_COMPARE_INLINE_TRACE)(
    _In_ HANDLE hProcess,
    _In_ DWORD64 Address1,
    _In_ DWORD   InlineContext1,
    _In_ DWORD64 RetAddress1,
    _In_ DWORD64 Address2,
    _In_ DWORD64 RetAddress2
    );
typedef SYM_COMPARE_INLINE_TRACE *PSYM_COMPARE_INLINE_TRACE;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI SYM_LOAD_MODULE)(
    _In_ HANDLE hProcess,
    _In_opt_ HANDLE hFile,
    _In_opt_ PCSTR ImageName,
    _In_opt_ PCSTR ModuleName,
    _In_ DWORD BaseOfDll,
    _In_ DWORD SizeOfDll
    );
typedef SYM_LOAD_MODULE *PSYM_LOAD_MODULE;

typedef
_Check_return_
_Success_(return != 0)
DWORD64
(IMAGEAPI SYM_LOAD_MODULE_EX)(
    _In_opt_ HANDLE hFile,
    _In_opt_ PCSTR ImageName,
    _In_opt_ PCSTR ModuleName,
    _In_ DWORD64 BaseOfDll,
    _In_ DWORD DllSize,
    _In_opt_ PMODLOAD_DATA Data,
    _In_opt_ DWORD Flags
    );
typedef SYM_LOAD_MODULE_EX *PSYM_LOAD_MODULE_EX;

typedef
_Check_return_
_Success_(return != 0)
DWORD64
(IMAGEAPI SYM_LOAD_MODULE_EX_W)(
    _In_ HANDLE hProcess,
    _In_opt_ HANDLE hFile,
    _In_opt_ PCWSTR ImageName,
    _In_opt_ PCWSTR ModuleName,
    _In_ DWORD64 BaseOfDll,
    _In_ DWORD DllSize,
    _In_opt_ PMODLOAD_DATA Data,
    _In_opt_ DWORD Flags
    );
typedef SYM_LOAD_MODULE_EX_W *PSYM_LOAD_MODULE_EX_W;

typedef
_Check_return_
_Success_(return != 0)
DWORD
(IMAGEAPI GET_SYM_LOAD_ERROR)(
    void
    );
typedef GET_SYM_LOAD_ERROR *PGET_SYM_LOAD_ERROR;

typedef
_Check_return_
_Success_(return != 0)
ULONG
(SYM_GET_FILE_LINE_OFFSETS64)(
    _In_ HANDLE hProcess,
    _In_opt_ PCSTR ModuleName,
    _In_ PCSTR FileName,
    _Out_writes_(BufferLines) PDWORD64 Buffer,
    _In_ ULONG BufferLines
    );
typedef SYM_GET_FILE_LINE_OFFSETS64 *PSYM_GET_FILE_LINE_OFFSETS64;

#undef EnumerateLoadedModules
#undef StackWalk
#undef SymEnumerateModules
#undef SymEnumerateSymbols
#undef SymEnumerateSymbolsW
#undef SymGetLineFromAddr
#undef SymGetLineFromAddrW
#undef SymGetLineFromName
#undef SymGetLineNext
#undef SymGetLinePrev
#undef SymGetModuleBase
#undef SymGetModuleInfo
#undef SymGetModuleInfoW
#undef SymGetSymFromAddr
#undef SymGetSymFromName
#undef SymGetSymNext
#undef SymGetSymNextW
#undef SymGetSymPrev
#undef SymGetSymPrevW
#undef SymLoadModule
#undef SymRegisterCallback
#undef SymRegisterFunctionEntryCallback
#undef SymUnDName
#undef SymUnloadModule


typedef
PIMAGE_NT_HEADERS
(IMAGEAPI IMAGE_NT_HEADER)(
    _In_ PVOID Base
    );
typedef IMAGE_NT_HEADER *PIMAGE_NT_HEADER;

typedef
PVOID
(IMAGEAPI IMAGE_DIRECTORY_ENTRY_TO_DATA)(
    _In_ PVOID Base,
    _In_ BOOLEAN MappedAsImage,
    _In_ USHORT DirectoryEntry,
    _Out_ PULONG Size
    );
typedef IMAGE_DIRECTORY_ENTRY_TO_DATA *PIMAGE_DIRECTORY_ENTRY_TO_DATA;

typedef
PVOID
(IMAGEAPI IMAGE_DIRECTORY_ENTRY_TO_DATA_EX)(
    _In_ PVOID Base,
    _In_ BOOLEAN MappedAsImage,
    _In_ USHORT DirectoryEntry,
    _Out_ PULONG Size,
    _Out_opt_ PIMAGE_SECTION_HEADER *FoundHeader
    );
typedef IMAGE_DIRECTORY_ENTRY_TO_DATA_EX *PIMAGE_DIRECTORY_ENTRY_TO_DATA_EX;

typedef
PIMAGE_SECTION_HEADER
(IMAGEAPI IMAGE_RVA_TO_SECTION)(
    _In_ PIMAGE_NT_HEADERS NtHeaders,
    _In_ PVOID Base,
    _In_ ULONG Rva
    );
typedef IMAGE_RVA_TO_SECTION *PIMAGE_RVA_TO_SECTION;

typedef
PVOID
(IMAGEAPI IMAGE_RVA_TO_VA)(
    _In_ PIMAGE_NT_HEADERS NtHeaders,
    _In_ PVOID Base,
    _In_ ULONG Rva,
    _In_opt_ OUT PIMAGE_SECTION_HEADER *LastRvaSection
    );
typedef IMAGE_RVA_TO_VA *PIMAGE_RVA_TO_VA;

typedef struct _DBGHELP_FUNCTIONSX {
    PENUM_DIR_TREE EnumDirTree;
    PENUM_DIR_TREE_W EnumDirTreeW;
    PENUMERATE_LOADED_MODULES64 EnumerateLoadedModules64;
    PENUMERATE_LOADED_MODULES EnumerateLoadedModules;
    PENUMERATE_LOADED_MODULES_EX EnumerateLoadedModulesEx;
    PENUMERATE_LOADED_MODULES_EX_W EnumerateLoadedModulesExW;
    PENUMERATE_LOADED_MODULES_W64 EnumerateLoadedModulesW64;
    PFIND_FILE_IN_PATH FindFileInPath;
    PFIND_FILE_IN_SEARCH_PATH FindFileInSearchPath;
    PGET_SYM_LOAD_ERROR GetSymLoadError;
    PGET_TIMESTAMP_FOR_LOADED_LIBRARY GetTimestampForLoadedLibrary;
    PMAKE_SURE_DIRECTORY_PATH_EXISTS MakeSureDirectoryPathExists;
    PRANGE_MAP_ADD_PE_IMAGE_SECTIONS RangeMapAddPeImageSections;
    PRANGE_MAP_READ RangeMapRead;
    PRANGE_MAP_REMOVE RangeMapRemove;
    PRANGE_MAP_WRITE RangeMapWrite;
    PREPORT_SYMBOL_LOAD_SUMMARY ReportSymbolLoadSummary;
    PSEARCH_TREE_FOR_FILE SearchTreeForFile;
    PSEARCH_TREE_FOR_FILE_W SearchTreeForFileW;
    PSTACK_WALK64 StackWalk64;
    PSTACK_WALK_EX StackWalkEx;
    PSTACK_WALK StackWalk;
    PSYM_ADDR_INCLUDE_INLINE_TRACE SymAddrIncludeInlineTrace;
    PSYM_ADD_SOURCE_STREAM_A SymAddSourceStreamA;
    PSYM_ADD_SOURCE_STREAM SymAddSourceStream;
    PSYM_ADD_SOURCE_STREAM_W SymAddSourceStreamW;
    PSYM_ADD_SYMBOL SymAddSymbol;
    PSYM_ADD_SYMBOL_W SymAddSymbolW;
    PSYM_CLEANUP SymCleanup;
    PSYM_COMPARE_INLINE_TRACE SymCompareInlineTrace;
    PSYM_DELETE_SYMBOL SymDeleteSymbol;
    PSYM_DELETE_SYMBOL_W SymDeleteSymbolW;
    PSYM_ENUMERATE_MODULES64 SymEnumerateModules64;
    PSYM_ENUMERATE_MODULES SymEnumerateModules;
    PSYM_ENUMERATE_MODULES_W64 SymEnumerateModulesW64;
    PSYM_ENUMERATE_SYMBOLS64 SymEnumerateSymbols64;
    PSYM_ENUMERATE_SYMBOLS SymEnumerateSymbols;
    PSYM_ENUMERATE_SYMBOLS_W64 SymEnumerateSymbolsW64;
    PSYM_ENUMERATE_SYMBOLS_W SymEnumerateSymbolsW;
    PSYM_ENUM_LINES SymEnumLines;
    PSYM_ENUM_LINES_W SymEnumLinesW;
    PSYM_ENUM_PROCESSES SymEnumProcesses;
    PSYM_ENUM_SOURCE_FILES SymEnumSourceFiles;
    PSYM_ENUM_SOURCE_FILES_W SymEnumSourceFilesW;
    PSYM_ENUM_SOURCE_FILE_TOKENS SymEnumSourceFileTokens;
    PSYM_ENUM_SOURCE_LINES SymEnumSourceLines;
    PSYM_ENUM_SOURCE_LINES_W SymEnumSourceLinesW;
    PSYM_ENUM_SYMBOLS_EX SymEnumSymbolsEx;
    PSYM_ENUM_SYMBOLS_EX_W SymEnumSymbolsExW;
    PSYM_ENUM_SYMBOLS_FOR_ADDR SymEnumSymbolsForAddr;
    PSYM_ENUM_SYMBOLS_FOR_ADDR_W SymEnumSymbolsForAddrW;
    PSYM_ENUM_SYMBOLS SymEnumSymbols;
    PSYM_ENUM_SYMBOLS_W SymEnumSymbolsW;
    PSYM_ENUM_SYM SymEnumSym;
    PSYM_ENUM_TYPES_BY_NAME SymEnumTypesByName;
    PSYM_ENUM_TYPES_BY_NAME_W SymEnumTypesByNameW;
    PSYM_ENUM_TYPES SymEnumTypes;
    PSYM_ENUM_TYPES_W SymEnumTypesW;
    PSYM_FIND_FILE_IN_PATH SymFindFileInPath;
    PSYM_FIND_FILE_IN_PATH_W SymFindFileInPathW;
    PSYM_FROM_ADDR SymFromAddr;
    PSYM_FROM_ADDR_W SymFromAddrW;
    PSYM_FROM_INDEX SymFromIndex;
    PSYM_FROM_INDEX_W SymFromIndexW;
    PSYM_FROM_INLINE_CONTEXT SymFromInlineContext;
    PSYM_FROM_INLINE_CONTEXT_W SymFromInlineContextW;
    PSYM_FROM_NAME SymFromName;
    PSYM_FROM_NAME_W SymFromNameW;
    PSYM_FROM_TOKEN SymFromToken;
    PSYM_FROM_TOKEN_W SymFromTokenW;
    PSYM_GET_FILE_LINE_OFFSETS64 SymGetFileLineOffsets64;
    PSYM_GET_LINE_FROM_ADDR64 SymGetLineFromAddr64;
    PSYM_GET_LINE_FROM_ADDR SymGetLineFromAddr;
    PSYM_GET_LINE_FROM_ADDR_W64 SymGetLineFromAddrW64;
    PSYM_GET_LINE_FROM_ADDR_W SymGetLineFromAddrW;
    PSYM_GET_LINE_FROM_INLINE_CONTEXT SymGetLineFromInlineContext;
    PSYM_GET_LINE_FROM_INLINE_CONTEXT_W SymGetLineFromInlineContextW;
    PSYM_GET_LINE_FROM_NAME64 SymGetLineFromName64;
    PSYM_GET_LINE_FROM_NAME SymGetLineFromName;
    PSYM_GET_LINE_FROM_NAME_W64 SymGetLineFromNameW64;
    PSYM_GET_LINE_NEXT64 SymGetLineNext64;
    PSYM_GET_LINE_NEXT SymGetLineNext;
    PSYM_GET_LINE_NEXT_W64 SymGetLineNextW64;
    PSYM_GET_LINE_NEXT_W SymGetLineNextW;
    PSYM_GET_LINE_PREV64 SymGetLinePrev64;
    PSYM_GET_LINE_PREV SymGetLinePrev;
    PSYM_GET_LINE_PREV_W64 SymGetLinePrevW64;
    PSYM_GET_LINE_PREV_W SymGetLinePrevW;
    PSYM_GET_MODULE_BASE SymGetModuleBase;
    PSYM_GET_MODULE_INFO64 SymGetModuleInfo64;
    PSYM_GET_MODULE_INFO SymGetModuleInfo;
    PSYM_GET_MODULE_INFO_W64 SymGetModuleInfoW64;
    PSYM_GET_MODULE_INFO_W SymGetModuleInfoW;
    PSYM_GET_OMAPS SymGetOmaps;
    PSYM_GET_OPTIONS SymGetOptions;
    PSYM_GET_SCOPE SymGetScope;
    PSYM_GET_SCOPE_W SymGetScopeW;
    PSYM_GET_SEARCH_PATH SymGetSearchPath;
    PSYM_GET_SEARCH_PATH_W SymGetSearchPathW;
    PSYM_GET_SOURCE_FILE_FROM_TOKEN SymGetSourceFileFromToken;
    PSYM_GET_SOURCE_FILE_FROM_TOKEN_W SymGetSourceFileFromTokenW;
    PSYM_GET_SOURCE_FILE SymGetSourceFile;
    PSYM_GET_SOURCE_FILE_TOKEN SymGetSourceFileToken;
    PSYM_GET_SOURCE_FILE_TOKEN_W SymGetSourceFileTokenW;
    PSYM_GET_SOURCE_FILE_W SymGetSourceFileW;
    PSYM_GET_SOURCE_VAR_FROM_TOKEN SymGetSourceVarFromToken;
    PSYM_GET_SOURCE_VAR_FROM_TOKEN_W SymGetSourceVarFromTokenW;
    PSYM_GET_SYMBOL_FILE SymGetSymbolFile;
    PSYM_GET_SYMBOL_FILE_W SymGetSymbolFileW;
    PSYM_GET_SYM_FROM_ADDR64 SymGetSymFromAddr64;
    PSYM_GET_SYM_FROM_ADDR SymGetSymFromAddr;
    PSYM_GET_SYM_FROM_NAME64 SymGetSymFromName64;
    PSYM_GET_SYM_FROM_NAME SymGetSymFromName;
    PSYM_GET_SYM_NEXT64 SymGetSymNext64;
    PSYM_GET_SYM_NEXT SymGetSymNext;
    PSYM_GET_SYM_NEXT_W64 SymGetSymNextW64;
    PSYM_GET_SYM_NEXT_W SymGetSymNextW;
    PSYM_GET_SYM_PREV64 SymGetSymPrev64;
    PSYM_GET_SYM_PREV SymGetSymPrev;
    PSYM_GET_SYM_PREV_W64 SymGetSymPrevW64;
    PSYM_GET_SYM_PREV_W SymGetSymPrevW;
    PSYM_GET_TYPE_FROM_NAME SymGetTypeFromName;
    PSYM_GET_TYPE_FROM_NAME_W SymGetTypeFromNameW;
    PSYM_GET_TYPE_INFO_EX SymGetTypeInfoEx;
    PSYM_GET_TYPE_INFO SymGetTypeInfo;
    PSYM_GET_UNWIND_INFO SymGetUnwindInfo;
    PSYM_INITIALIZE SymInitialize;
    PSYM_INITIALIZE_W SymInitializeW;
    PSYM_LOAD_MODULE SymLoadModule;
    PSYM_MATCH_FILE_NAME SymMatchFileName;
    PSYM_MATCH_FILE_NAME_W SymMatchFileNameW;
    PSYM_MATCH_STRING_A SymMatchStringA;
    PSYM_MATCH_STRING SymMatchString;
    PSYM_MATCH_STRING_W SymMatchStringW;
    PSYM_NEXT SymNext;
    PSYM_NEXT_W SymNextW;
    PSYM_PREV SymPrev;
    PSYM_PREV_W SymPrevW;
    PSYM_QUERY_INLINE_TRACE SymQueryInlineTrace;
    PSYM_REFRESH_MODULE_LIST SymRefreshModuleList;
    PSYM_REGISTER_CALLBACK64 SymRegisterCallback64;
    PSYM_REGISTER_CALLBACK SymRegisterCallback;
    PSYM_REGISTER_CALLBACK_W64 SymRegisterCallbackW64;
    PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK64 SymRegisterFunctionEntryCallback64;
    PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK SymRegisterFunctionEntryCallback;
    PSYM_SEARCH SymSearch;
    PSYM_SEARCH_W SymSearchW;
    PSYM_SET_CONTEXT SymSetContext;
    PSYM_SET_OPTIONS SymSetOptions;
    PSYM_SET_PARENT_WINDOW SymSetParentWindow;
    PSYM_SET_SCOPE_FROM_ADDR SymSetScopeFromAddr;
    PSYM_SET_SCOPE_FROM_INDEX SymSetScopeFromIndex;
    PSYM_SET_SCOPE_FROM_INLINE_CONTEXT SymSetScopeFromInlineContext;
    PSYM_SET_SEARCH_PATH SymSetSearchPath;
    PSYM_SET_SEARCH_PATH_W SymSetSearchPathW;
    PSYM_SRV_GET_FILE_INDEXES SymSrvGetFileIndexes;
    PSYM_SRV_GET_FILE_INDEXES_W SymSrvGetFileIndexesW;
    PSYM_SRV_GET_FILE_INDEX_INFO SymSrvGetFileIndexInfo;
    PSYM_SRV_GET_FILE_INDEX_INFO_W SymSrvGetFileIndexInfoW;
    PSYM_SRV_GET_FILE_INDEX_STRING SymSrvGetFileIndexString;
    PSYM_SRV_GET_FILE_INDEX_STRING_W SymSrvGetFileIndexStringW;
    PSYM_SRV_IS_STORE SymSrvIsStore;
    PSYM_SRV_IS_STORE_W SymSrvIsStoreW;
    PSYM_UNDNAME64 SymUnDName64;
    PSYM_UNDNAME SymUnDName;
    PSYM_UNLOAD_MODULE64 SymUnloadModule64;
    PSYM_UNLOAD_MODULE SymUnloadModule;
    PUNDECORATE_SYMBOL_NAME UndecorateSymbolName;
    PUNDECORATE_SYMBOL_NAME_W UndecorateSymbolNameW;
    PUNMAP_DEBUG_INFORMATION UnmapDebugInformation;
    PIMAGE_NT_HEADER ImageNtHeader;
    PIMAGE_DIRECTORY_ENTRY_TO_DATA ImageDirectoryEntryToData;
    PIMAGE_DIRECTORY_ENTRY_TO_DATA_EX ImageDirectoryEntryToDataEx;
    PIMAGE_RVA_TO_SECTION ImageRvaToSection;
    PIMAGE_RVA_TO_VA ImageRvaToV;
} DBGHELP_FUNCTIONSX, *PDBGHELP_FUNCTIONSX;
