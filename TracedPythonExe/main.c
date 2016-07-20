#include "stdafx.h"

#include "../Python/Python.h"

typedef PPWSTR (*PCOMMAND_LINE_TO_ARGV)(
  _In_  PWSTR  CommandLine,
  _Out_ PLONG  NumberOfArgs
);

typedef PWSTR (WINAPI *PGET_COMMAND_LINE)(VOID);


VOID
WINAPI
mainCRTStartup()
{
    BOOL Success;
    DWORD ExitCode = 1;

    PCOMMAND_LINE_TO_ARGV CommandLineToArgvW;
    PPY_MAIN Py_Main;
    PPY_GET_PREFIX Py_GetPrefix;
    PPY_GET_EXEC_PREFIX Py_GetExecPrefix;
    PPY_GET_PROGRAM_FULL_PATH Py_GetProgramFullPath;

    PPYGC_COLLECT PyGC_Collect;

    //PPYOBJECT_MALLOC PyObject_Malloc;

    //PPYOBJECT_GC_TRACK PyObject_GC_Track;
    //PPYOBJECT_GC_UNTRACK PyObject_GC_UnTrack;
    //PPYOBJECT_GC_DEL PyObject_GC_Del;

    PYTHON PythonRecord;
    PPYTHON Python = &PythonRecord;
    ULONG SizeOfPython = sizeof(*Python);

    HOOKED_FUNCTION Function;
    PHOOKED_FUNCTION HookedFunction;
    PINITIALIZE_HOOKED_FUNCTION InitializeHookedFunction;
    PHOOK_FUNCTION HookFunction;
    PVOID Buffer;
    PCHAR BaseAddress = NULL;
    USHORT Attempts = 10;
    LONG_PTR Offset = 10485760; // 10MB

    PWSTR CommandLine;
    LONG NumberOfArgs;
    PPWSTR UnicodeArgv;
    PPSTR AnsiArgv;
    LONG Index;
    HANDLE HeapHandle;
    ULONG AllocSize;
    //PCHAR Prefix;
    //PCHAR ExecPrefix;
    PTEST_FUNC TestFuncPointer = &TestFunc;
    PCHAR Path;
    USHORT PathLen = sizeof(PythonExePath) / sizeof(PythonExePath[0]);

    RTL RtlRecord;
    PRTL Rtl = &RtlRecord;
    ULONG SizeOfRtl = sizeof(*Rtl);
    ULONG Result;

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
