#include "stdafx.h"

#include "PathHack.h"

/*
typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);
typedef PCHAR (*PPY_GET_PREFIX)(VOID);
typedef PCHAR (*PPY_GET_EXEC_PREFIX)(VOID);
typedef PCHAR (*PPY_GET_PROGRAM_FULL_PATH)(VOID);
typedef PCHAR (*PPY_GET_PROGRAM_NAME)(VOID);

typedef PPY_MAIN *PPPY_MAIN;
typedef PPY_GET_PREFIX *PPPY_GET_PREFIX;
typedef PPY_GET_EXEC_PREFIX *PPPY_GET_EXEC_PREFIX;
typedef PPY_GET_PROGRAM_NAME *PPPY_GET_PROGRAM_NAME;
typedef PPY_GET_PROGRAM_FULL_PATH *PPPY_GET_PROGRAM_FULL_PATH;
*/


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

#define HOOK(Source, Dest) do { \
    if (!Hook(Rtl, Source, Dest, NULL)) { \
        OutputDebugStringA("Failed to hook " #Source " ->" #Dest); \
        goto End; \
    } \
} while (0);


PCHAR
Our_Py_GetPath(void)
{
    return (PCHAR)"";
}

PCHAR
Our_Py_GetPrefix(void)
{
    return (PCHAR)PythonPrefix;
}

CONST PCHAR
Our_Py_GetExecPrefix(void)
{
    return Our_Py_GetPrefix();
}

PCHAR
Our_Py_GetProgramFullPath(void)
{
    return (PCHAR)PythonExePath;
}

PPYGC_COLLECT Original_PyGC_Collect = NULL;

SSIZE_T
Our_PyGC_Collect(VOID)
{
    OutputDebugStringA("Entered Our_PyGC_Collect()\n");
    return Original_PyGC_Collect();
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

    PHOOK Hook;
    PUNHOOK Unhook;

    PINITIALIZE_RTL InitializeRtl;
    PCOMMAND_LINE_TO_ARGV CommandLineToArgvW;
    PPY_MAIN Py_Main;
    PPY_GET_PREFIX Py_GetPrefix;
    PPY_GET_EXEC_PREFIX Py_GetExecPrefix;
    PPY_GET_PROGRAM_FULL_PATH Py_GetProgramFullPath;

    PPYGC_COLLECT PyGC_Collect;

    PWSTR CommandLine;
    LONG NumberOfArgs;
    PPWSTR UnicodeArgv;
    PPSTR AnsiArgv;
    LONG Index;
    HANDLE HeapHandle;
    ULONG AllocSize;
    PCHAR Prefix;
    //PCHAR ExecPrefix;
    PCHAR Path;
    USHORT PathLen = sizeof(PythonExePath) / sizeof(PythonExePath[0]);

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

    LOAD(Python, PythonDllPath);
    RESOLVE(Python, PPY_MAIN, Py_Main);
    RESOLVE(Python, PPY_GET_PREFIX, Py_GetPrefix);
    RESOLVE(Python, PPY_GET_EXEC_PREFIX, Py_GetExecPrefix);
    RESOLVE(Python, PPY_GET_PROGRAM_FULL_PATH, Py_GetProgramFullPath);
    RESOLVE(Python, PPYGC_COLLECT, PyGC_Collect);

    LOAD(HookModule, "Hook");
    RESOLVE(HookModule, PHOOK, Hook);
    RESOLVE(HookModule, PUNHOOK, Unhook);

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        OutputDebugStringA("GetProcessHeap() failed.");
        goto End;
    }

    //HOOK((PPVOID)&PyGC_Collect, Our_PyGC_Collect);
    //Original_PyGC_Collect = Our_PyGC_Collect;
    HOOK((PPVOID)&Py_GetPrefix, Our_Py_GetPrefix);
    Prefix = Py_GetPrefix();

    //HOOK((PPVOID)&Py_GetExecPrefix, Our_Py_GetExecPrefix);
    //ExecPrefix = Py_GetExecPrefix();

    //HOOK((PPVOID)&Py_GetProgramFullPath, Our_Py_GetProgramFullPath);
    Path = Py_GetProgramFullPath();
    Rtl->RtlCopyMemory(Path, PythonExePath, sizeof(PythonExePath));

    //Prefix = Py_GetPrefix();
    //ExecPrefix = Py_GetExecPrefix();
    //Path = Py_GetProgramFullPath();

    CommandLine = GetCommandLineW();
    UnicodeArgv = CommandLineToArgvW(CommandLine, &NumberOfArgs);
    AllocSize = (sizeof(PSTR) * NumberOfArgs) + 1;
    AnsiArgv = (PPSTR)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, AllocSize);
    if (!AnsiArgv) {
        goto End;
    }

    for (Index = 0; Index < NumberOfArgs; Index++) {
        PWSTR UnicodeArg = UnicodeArgv[Index];
        PSTR AnsiArg;
        INT Size;

        if (Index == 0) {
            AnsiArgv[Index] = (PCHAR)PythonExePath;
            continue;
        }

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

    for (Index = 1; Index < NumberOfArgs; Index++) {
        HeapFree(HeapHandle, 0, AnsiArgv[Index]);
    }

    HeapFree(HeapHandle, 0, AnsiArgv);

End:
    ExitProcess(ExitCode);
}
