#include "stdafx.h"

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

VOID
WINAPI
mainCRTStartup()
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

    RTL RtlRecord;
    PRTL Rtl = &RtlRecord;
    ULONG SizeOfRtl = sizeof(*Rtl);

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

    LOAD(HookModule, "Hook");
    RESOLVE(HookModule, PHOOK, Hook);
    RESOLVE(HookModule, PUNHOOK, Unhook);

    if (!Hook(Rtl, (PPVOID)&Original_Py_Main, OurPyMain)) {
        OutputDebugStringA("Hooking attempt failed.");
        goto End;
    }

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

        Size = WideCharToMultiByte(CP_UTF8, 0, UnicodeArg, -1, NULL, 0, NULL, 0);

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
