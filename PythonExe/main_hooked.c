#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable: 4091)  // warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#define _NO_CVCONST_H
#include <DbgHelp.h>
#pragma warning(pop)
#pragma comment(lib, "dbghelp")

typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);

typedef PPY_MAIN *PPPY_MAIN;

typedef CHAR **PPSTR;
typedef WCHAR **PPWSTR;

typedef PPWSTR (*PCOMMAND_LINE_TO_ARGV)(
  _In_  PWSTR  CommandLine,
  _Out_ PLONG  NumberOfArgs
);

typedef PWSTR (WINAPI *PGET_COMMAND_LINE)(VOID);

#define LOAD(Module, Name) do {                      \
    Module = LoadLibraryA(Name);                     \
    if (!Module) {                                   \
        OutputDebugStringA("Failed to load " #Name); \
        goto End;                                    \
    }                                                \
} while (0)

#define RESOLVE(Module, Type, Name) do {                             \
    Name = (Type)GetProcAddress(Module, #Name);                      \
    if (!Name) {                                                     \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name); \
        goto End;                                                    \
    }                                                                \
} while (0)

typedef EXCEPTION_DISPOSITION (__cdecl *P__C_SPECIFIC_HANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
);

P__C_SPECIFIC_HANDLER __C_specific_handler_impl;

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

PPY_MAIN Original_Py_Main = NULL;

INT
OurPyMain(int argc, char **argv)
{
    OutputDebugStringA("Entered OurPyMain()!");
    return Original_Py_Main(argc, argv);
}

PVOID FuncPointer;

ULONG
Func(ULONG a, PCHAR b, CHAR c)
{
    ULONG a2 = a * 2;
    PCHAR b2 = b+1;
    CHAR  c2 = c+1;

    return a2;
}

typedef ULONG (*PFUNC)(ULONG a, PCHAR b, CHAR c);

ULONG
HookedFunc(PHOOKED_FUNCTION_ENTRY Entry)
{
    PFUNCTION Function = Entry->Function;
    DWORD64 HomeRcx = Entry->HomeRcx;
    DWORD64 HomeRdx = Entry->HomeRdx;
    DWORD64 HomeR8 = Entry->HomeR8;
    DWORD64 HomeR9 = Entry->HomeR9;

    ULONG a2 = (ULONG)HomeRcx;
    PCHAR b2 = (PCHAR)HomeRdx;
    CHAR  c2 = (CHAR)HomeR8;

    PFUNC FuncPtr = (PFUNC)Function->NewAddress;

    ULONG Result = FuncPtr(a2+2, b2, c2+1);

    return Result;
}

typedef enum SymTagEnum SYM_TAG_ENUM;

BOOL
CALLBACK
SymEnumSourceFilesCallback(
    _In_     PSOURCEFILE     SourceFile,
    _In_opt_ PVOID           UserContext
)
{
    OutputDebugStringA((PCHAR)SourceFile->FileName);
    OutputDebugStringA("\n");
    return TRUE;
}

typedef BOOL (CALLBACK *PSYM_ENUMERATESYMBOLS_CALLBACKW)(
    _In_     PSYMBOL_INFOW pSymInfo,
    _In_     ULONG         SymbolSize,
    _In_opt_ PVOID         UserContext
);

PVOID
HeapAllocator(
    HANDLE HeapHandle,
    ULONG  Size
)
{
    return HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, Size);
}

typedef TI_FINDCHILDREN_PARAMS *PTI_FINDCHILDREN_PARAMS;
typedef SYMBOL_INFO **PPSYMBOL_INFO;

typedef struct _SYMBOL_TYPE {
    PSYMBOL_INFO    SymbolInfo;
    DWORD           Tag;
    PWSTR           Name;
    PULONG64        Length;
    DWORD           Type;
    DWORD           TypeId;
    DWORD           BaseType;
    DWORD           DataKind;
    DWORD           SymbolIndex;
    DWORD           UdtKind;
} SYMBOL_TYPE, *PSYMBOL_TYPE, **PPSYMBOL_TYPE;

BOOL
GetChildren(
    _In_    HANDLE          ProcessHandle,
    _In_    HANDLE          HeapHandle,
    _In_    DWORD64         ModuleBase,
    _In_    ULONG           TypeIndex,
    _Out_   PPVOID          ChildrenPointer,
    _Out_   PULONG          NumberOfChildren
    )
{
    ULONG Count;
    ULONG ChildrenCount;
    ULONG Size;
    ULONG Index;
    //ULONG TypeId;
    ULONG ChildId;
    BOOL Success;
    PTI_FINDCHILDREN_PARAMS Params = NULL;

    ULONG SymbolInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR);
    PPSYMBOL_INFO ChildSymbolInfo = NULL;
    PSYMBOL_INFO SymbolInfo;
    PPSYMBOL_TYPE ChildSymbolTypes = NULL;
    PSYMBOL_TYPE SymbolType;

    if (!ARGUMENT_PRESENT(NumberOfChildren)) {
        return FALSE;
    }

    Success = SymGetTypeInfo(ProcessHandle,
                             ModuleBase,
                             TypeIndex,
                             TI_GET_CHILDRENCOUNT,
                             &Count);

    if (!Success) {
        return FALSE;
    }

    if (Count == 0) {
        *NumberOfChildren = 0;
        return TRUE;
    }

    Success = SymGetTypeInfo(ProcessHandle,
                             ModuleBase,
                             TypeIndex,
                             TI_GET_CHILDRENCOUNT,
                             &ChildrenCount);

    if (!Success) {
        return FALSE;
    }


    Size = sizeof(TI_FINDCHILDREN_PARAMS) + (Count * sizeof(ULONG));

    Params = (PTI_FINDCHILDREN_PARAMS)HeapAlloc(HeapHandle,
                                                HEAP_ZERO_MEMORY,
                                                Size);

    if (!Params) {
        return FALSE;
    }

    Params->Count = Count;

    Success = SymGetTypeInfo(ProcessHandle,
                             ModuleBase,
                             TypeIndex,
                             TI_FINDCHILDREN,
                             Params);

    if (!Success) {
        goto Error;
    }

    SymbolInfoSize *= Count;
    ChildSymbolInfo = (PPSYMBOL_INFO)HeapAlloc(HeapHandle,
                                               HEAP_ZERO_MEMORY,
                                               SymbolInfoSize);

    if (!ChildSymbolInfo) {
        goto Error;
    }

    ChildSymbolTypes = (PPSYMBOL_TYPE)HeapAlloc(HeapHandle,
                                                HEAP_ZERO_MEMORY,
                                                sizeof(SYMBOL_TYPE) * Count);

    for (Index = 0; Index < Count; Index++) {
        ChildId = Params->ChildId[Index];
        SymbolInfo = (PSYMBOL_INFO)&ChildSymbolInfo[Index];
        SymbolType = (PSYMBOL_TYPE)&ChildSymbolTypes[Index];

        SymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        SymbolInfo->MaxNameLen = MAX_SYM_NAME;
        SymbolInfo->TypeIndex = ChildId;

        SymbolType->SymbolInfo = SymbolInfo;

        Success = SymGetTypeInfo(ProcessHandle,
                                 ModuleBase,
                                 SymbolInfo->TypeIndex,
                                 TI_GET_SYMTAG,
                                 &SymbolType->Tag);

        if (SymbolType->Tag != SymTagFunctionArgType) {
            continue;
        }

        Success = SymFromIndex(ProcessHandle,
                               ModuleBase,
                               ChildId,
                               SymbolInfo);

        if (!Success) {
            goto Error;
        }



        Success = SymGetTypeInfo(ProcessHandle,
                                 ModuleBase,
                                 SymbolInfo->TypeIndex,
                                 TI_GET_UDTKIND,
                                 &SymbolType->UdtKind);

    }

    goto End;

Error:
    if (Params) {
        HeapFree(HeapHandle, 0, Params);
    }

End:
    return Success;
}

BOOL
CALLBACK
SymEnumSymbolsCallback(
    _In_     PSYMBOL_INFO SymbolInfo,
    _In_     ULONG        SymbolSize,
    _In_opt_ PVOID        UserContext
)
{
    ULONG NumberOfChildren;
    PVOID ChildrenBuffer;

    if (strcmp(SymbolInfo->Name, "Py_Main")) {
        return TRUE;
    }

    OutputDebugStringA((PCHAR)SymbolInfo->Name);
    OutputDebugStringA("\n");

    __debugbreak();

    BOOL Success = GetChildren(GetCurrentProcess(),
                               GetProcessHeap(),
                               (DWORD64)0x1e000000,
                               SymbolInfo->TypeIndex,
                               &ChildrenBuffer,
                               &NumberOfChildren);

    return TRUE;
}


VOID
WINAPI
_mainCRTStartupHooked()
{
    DWORD ExitCode = 1;

    HMODULE Kernel32;
    HMODULE Shell32;
    HMODULE Python;
    HMODULE RtlModule;
    HMODULE HookModule;

    PINITIALIZE_RTL InitializeRtl;
    PCOMMAND_LINE_TO_ARGV CommandLineToArgvW;
    PPY_MAIN Py_Main;

    PWSTR CommandLine;
    LONG NumberOfArgs;
    PPWSTR UnicodeArgv;
    PPSTR AnsiArgv;
    LONG Index;
    HANDLE HeapHandle;
    ULONG AllocSize;

    PHOOK Hook;
    PUNHOOK Unhook;

    //PCONTEXT Context;

    RTL RtlRecord;
    PRTL Rtl = &RtlRecord;
    ULONG SizeOfRtl = sizeof(*Rtl);

    ULONG Result;
    CHAR ArgB = 'B';
    PCHAR CharPointer = &ArgB;
    FUNCTION Function;
    PINITIALIZE_FUNCTION InitializeFunction;
    PHOOK_FUNCTION HookFunction;
    HANDLE ProcessHandle;
    DWORD64 Displacement = 0;
    DWORD64 Address;
    PVOID ChildrenPointer;
    ULONG NumberOfChildren = 0;
    DWORD TypeId;
    DWORD Type;

    BOOL Success;
    SYM_TAG_ENUM Tag;

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        goto End;
    }

    ULONG SymbolInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR);
    PSYMBOL_INFO SymbolInfo = (PSYMBOL_INFO)HeapAlloc(HeapHandle,
                                                      HEAP_ZERO_MEMORY,
                                                      SymbolInfoSize);

    if (!SymbolInfo) {
        goto End;
    }

    //ULONG LineSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR);
    //PIMAGEHLP_LINE64 Line = (PIMAGEHLP_LINE64)HeapAlloc(HeapHandle,
    //                                                    HEAP_ZERO_MEMORY,
    //                                                    LineSize);

    IMAGEHLP_LINE64 Line;
    SecureZeroMemory(&Line, sizeof(Line));

    //SymSetOptions(SYMOPT_UNDNAME |
    //              SYMOPT_AUTO_PUBLICS |
    //              SYMOPT_CASE_INSENSITIVE |
    //              SYMOPT_LOAD_LINES);

    SymSetOptions(SYMOPT_UNDNAME |
                  SYMOPT_AUTO_PUBLICS |
                  SYMOPT_CASE_INSENSITIVE |
                  SYMOPT_LOAD_ANYTHING |
                  SYMOPT_LOAD_LINES |
                  SYMOPT_DEBUG);

    ProcessHandle = GetCurrentProcess();
    Success = SymInitialize(ProcessHandle, NULL, TRUE);
    if (!Success) {
        goto End;
    }


    LOAD(RtlModule, "Rtl");
    RESOLVE(RtlModule, PINITIALIZE_RTL, InitializeRtl);

    if (!InitializeRtl(Rtl, &SizeOfRtl)) {
        OutputDebugStringA("InitializeRtl() failed.");
        goto End;
    }

    LOAD(Kernel32, "kernel32");

    __C_specific_handler_impl = (P__C_SPECIFIC_HANDLER)GetProcAddress(Kernel32, "__C_specific_handler");
    //RESOLVE(Kernel32, P__C_SPECIFIC_HANDLER, __C_specific_handler);

    LOAD(Shell32, "shell32");
    RESOLVE(Shell32, PCOMMAND_LINE_TO_ARGV, CommandLineToArgvW);

    LOAD(Python, "python27");
    RESOLVE(Python, PPY_MAIN, Py_Main);

    Original_Py_Main = Py_Main;

    Success = SymRefreshModuleList(ProcessHandle);
    if (!Success) {
        goto End;
    }

    Success = SymEnumSymbols(ProcessHandle,
                             (ULONG64)Python,
                             NULL,
                             SymEnumSymbolsCallback,
                             (PVOID)Rtl);


    SymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    SymbolInfo->MaxNameLen = MAX_SYM_NAME;

    Address = (DWORD64)Py_Main;

    Success = SymFromAddr(ProcessHandle,
                          Address,
                          &Displacement,
                          SymbolInfo);

    if (!Success) {
        goto End;
    }

    TypeId = 0;
    Type = 0;

    Success = SymGetTypeInfo(ProcessHandle,
                             (DWORD64)Python,
                             SymbolInfo->Index,
                             TI_GET_TYPE,
                             &Type);

    Success = SymGetTypeInfo(ProcessHandle,
                             (DWORD64)Python,
                             Type,
                             TI_GET_TYPEID,
                             &TypeId);

    Tag = SymbolInfo->Tag;

    CHAR CurrentSearchPath[_MAX_PATH];
    Success = SymGetSearchPath(ProcessHandle, &CurrentSearchPath[0], sizeof(CurrentSearchPath));

    //CHAR SearchPath[] = "c:\\aroot\\work\\Python-2.7.11";
    //Success = SymSetSearchPath(ProcessHandle, SearchPath);
    DWORD LastError;

    /*
    Success = SymGetTypeFromName(ProcessHandle,
                                 (ULONG64)Python,
                                 "Py_Main",
                                 SymbolInfo);
                                 */

    Tag = SymbolInfo->Tag;

    Success = GetChildren(ProcessHandle,
                          HeapHandle,
                          (DWORD64)Python,
                          TypeId,
                          &ChildrenPointer,
                          &NumberOfChildren);

    Success = GetChildren(ProcessHandle,
                          HeapHandle,
                          (DWORD64)Python,
                          TypeId,
                          &ChildrenPointer,
                          &NumberOfChildren);


    LastError = GetLastError();

    Success = SymEnumTypes(ProcessHandle,
                           (ULONG64)Python,
                           SymEnumSymbolsCallback,
                           (PVOID)Rtl);

    LastError = GetLastError();

    Success = SymEnumSymbols(ProcessHandle,
                             (ULONG64)Python,
                             NULL,
                             SymEnumSymbolsCallback,
                             (PVOID)Rtl);

    Success = SymEnumTypesByName(ProcessHandle,
                                 (ULONG64)Python,
                                 NULL,
                                 SymEnumSymbolsCallback,
                                 (PVOID)Rtl);

    LastError = GetLastError();

    Success = SymEnumSourceFiles(ProcessHandle,
                                 (ULONG64)Python,
                                 NULL,
                                 SymEnumSourceFilesCallback,
                                 (PVOID)Rtl);

    if (!Success) {
        goto End;
    }

    Line.SizeOfStruct = sizeof(Line);

    DWORD LineDisplacement = 0;

    Success = SymGetLineFromAddr64(ProcessHandle,
                                   Address,
                                   &LineDisplacement,
                                   &Line);

    if (!Success) {
        goto End;
    }


    FuncPointer = (PVOID)&Func;

    LOAD(HookModule, "Hook");
    RESOLVE(HookModule, PHOOK, Hook);
    RESOLVE(HookModule, PUNHOOK, Unhook);
    RESOLVE(HookModule, PINITIALIZE_FUNCTION, InitializeFunction);
    RESOLVE(HookModule, PHOOK_FUNCTION, HookFunction);

    Function.Key = 1;
    Function.OldAddress = FuncPointer;
    Function.HookedEntry = (PVOIDFUNC)HookedFunc;
    Function.NumberOfParameters = 3;
    Function.Name = "Func";
    Function.Module = "Hook";

    InitializeFunction(Rtl, &Function);

    if (!HookFunction(Rtl, &Function)) {
        OutputDebugStringA("Hooking attempt failed.");
        goto End;
    }

    /*
    if (!Hook(Rtl, (PPVOID)&Function.OldAddress, HookProlog, &Function)) {
        OutputDebugStringA("Hooking attempt 2 failed.");
        goto End;
    }

    if (!Hook(Rtl, (PPVOID)&Function.OldAddress, HookProlog, &Function)) {
        OutputDebugStringA("Hooking attempt 2 failed.");
        goto End;
    }
    */

    /*
    HookPush();

    if (!Hook(Rtl, (PPVOID)&FuncPointer, HookedFunc, (PVOID)0x12345678)) {
        OutputDebugStringA("Hooking attempt 2 failed.");
        goto End;
    }
    */

    Result = Func(1234, &ArgB, 'C');
    goto End;

    /*
    if (!Hook(Rtl, (PPVOID)&Original_Py_Main, OurPyMain, (PVOID)1)) {
        OutputDebugStringA("Hooking attempt failed.");
        goto End;
    }
    */

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        OutputDebugStringA("GetProcessHeap() failed.");
        goto End;
    }

    CommandLine = GetCommandLineW();
    UnicodeArgv = CommandLineToArgvW(CommandLine, &NumberOfArgs);
    AllocSize = (sizeof(PSTR) * NumberOfArgs) + 1;
    AnsiArgv = (PPSTR)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, AllocSize);

    for (Index = 0; Index < NumberOfArgs; Index++) {
        PWSTR UnicodeArg = UnicodeArgv[Index];
        PSTR AnsiArg;
        INT Size;

        Size = WideCharToMultiByte(
            CP_UTF8,
            0,
            UnicodeArg,
            -1,
            NULL,
            0,
            NULL,
            0
        );

        if (Size <= 0) {
            goto End;
        }

        AnsiArg = (PSTR)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, Size);
        if (!AnsiArg) {
            goto End;
        }

        Size = WideCharToMultiByte(CP_UTF8, 0, UnicodeArg, -1, AnsiArg, Size, NULL, 0);
        if (Size <= 0) {
            goto End;
        }

        AnsiArgv[Index] = AnsiArg;
    }

    ExitCode = Py_Main(NumberOfArgs, AnsiArgv);

    for (Index = 0; Index < NumberOfArgs; Index++) {
        HeapFree(HeapHandle, 0, AnsiArgv[Index]);
    }

    HeapFree(HeapHandle, 0, AnsiArgv);

    goto End;

    __try {
        (*(volatile *)(PCHAR)10) = '1';
    }
    __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH)
    {
        OutputDebugStringA("Access violation!");
    }

End:
    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
