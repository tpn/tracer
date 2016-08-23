#include "stdafx.h"

#include "../Rtl/Commandline.h"
#include "../Rtl/Memory.h"
#include "../Rtl/DefaultHeapAllocator.h"
#include "../TracerConfig/TracerConfig.h"

#include "../Python/Python.h"

VOID
WINAPI
mainCRTStartup()
{
    DWORD ExitCode;
    ALLOCATOR Allocator;
    PWSTR CommandLine;
    PTRACER_CONFIG TracerConfig;

    if (!DefaultHeapInitialize(&Allocator)) {
        goto Error;
    }

    CommandLine = GetCommandLineW();
    if (!CommandLine) {
        goto Error;
    }

    PCOMMAND_LINE_TO_ARGV CommandLineToArgvW;
    PPY_MAIN Py_Main;

    PCHAR PythonExePath;
    PWSTR CommandLine;
    LONG NumberOfArgs;
    PPWSTR UnicodeArgv;
    PPSTR AnsiArgv;
    LONG Index;
    HANDLE HeapHandle;
    ULONG AllocSize;

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

Error:
    ExitCode = 1;

End:
    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
