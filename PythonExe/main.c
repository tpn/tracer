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


VOID
WINAPI
mainCRTStartup()
{
    DWORD ExitCode = 1;

    HMODULE Kernel32;
    HMODULE Shell32;
    HMODULE Python;
    HMODULE RtlModule;

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

    LOAD(Shell32, "shell32");
    RESOLVE(Shell32, PCOMMAND_LINE_TO_ARGV, CommandLineToArgvW);

    LOAD(Python, "python27.dll");
    RESOLVE(Python, PPY_MAIN, Py_Main);

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

        Size = WideCharToMultiByte(
            CP_UTF8,
            0,
            UnicodeArg,
            -1,
            AnsiArg,
            Size,
            NULL,
            0
        );

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

End:
    ExitProcess(ExitCode);
}
