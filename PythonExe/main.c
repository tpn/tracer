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

PVOID
HeapAllocationRoutine(
    _In_ HANDLE HeapHandle,
    _In_ ULONG ByteSize
    )
{
    return HeapAlloc(HeapHandle, 0, ByteSize);
}

typedef ULONG (*PTEST_FUNC)(
    _In_    ULONG   Arg1,
    _In_    ULONG   Arg2,
    _In_    ULONG   Arg3,
    _In_    ULONG   Arg4
    );

ULONG
TestFunc(ULONG Arg1, ULONG Arg2, ULONG Arg3, ULONG Arg4)
{
    return Arg1 + Arg2 + Arg3 + Arg4;
}

VOID
CALLBACK
HookEntryCallback(
    _In_    PHOOKED_FUNCTION_CALL Call,
    _In_    LARGE_INTEGER Timestamp
    )
{
    PHOOKED_FUNCTION Function = Call->HookedFunction;

    ULONG Arg1 = (ULONG)Call->Param1.LowPart;
    ULONG Arg2 = (ULONG)Call->Param2.LowPart;
    ULONG Arg3 = (ULONG)Call->Param3.LowPart;
    ULONG Arg4 = (ULONG)Call->Param4.LowPart;


    PrefaultPage(&Arg1);
    PrefaultPage(&Arg2);
    PrefaultPage(&Arg3);
    PrefaultPage(&Arg4);

}

VOID
CALLBACK
HookExitCallback(
    _In_    PHOOKED_FUNCTION_CALL Call,
    _In_    LARGE_INTEGER Timestamp
    )
{
    PHOOKED_FUNCTION Function = Call->HookedFunction;
    ULONG Result;

    ULONG Arg1 = (ULONG)Call->Param1.LowPart;
    ULONG Arg2 = (ULONG)Call->Param2.LowPart;
    ULONG Arg3 = (ULONG)Call->Param3.LowPart;
    ULONG Arg4 = (ULONG)Call->Param4.LowPart;

    Result = (ULONG)Call->ReturnValue.LowPart;


    PrefaultPage(&Arg1);
    PrefaultPage(&Arg2);
    PrefaultPage(&Arg3);
    PrefaultPage(&Arg4);

}


VOID
WINAPI
mainCRTStartup()
{
    BOOL Success;
    DWORD ExitCode = 1;

    HMODULE Kernel32;
    HMODULE Shell32;
    HMODULE PythonModule;
    HMODULE PythonApiModule;
    HMODULE RtlModule;
    HMODULE HookModule;

    PHOOK Hook;
    PUNHOOK Unhook;

    PINITIALIZE_RTL InitializeRtl;
    PINITIALIZE_PYTHON InitializePython;
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

    LOAD(PythonModule, PythonDllPath);

    LOAD(RtlModule, "Rtl");
    RESOLVE(RtlModule, PINITIALIZE_RTL, InitializeRtl);

    LOAD(PythonApiModule, "Python");
    RESOLVE(PythonApiModule, PINITIALIZE_PYTHON, InitializePython);

    if (!InitializeRtl(Rtl, &SizeOfRtl)) {
        OutputDebugStringA("InitializeRtl() failed.");
        goto End;
    }

    Success = InitializePython(Rtl,
                               PythonModule,
                               Python,
                               &SizeOfPython);

    if (!Success) {
        goto End;
    }

    LOAD(Kernel32, "kernel32");

    LOAD(Shell32, "shell32");
    RESOLVE(Shell32, PCOMMAND_LINE_TO_ARGV, CommandLineToArgvW);

    RESOLVE(PythonModule, PPY_MAIN, Py_Main);
    RESOLVE(PythonModule, PPY_GET_PREFIX, Py_GetPrefix);
    RESOLVE(PythonModule, PPY_GET_EXEC_PREFIX, Py_GetExecPrefix);
    RESOLVE(PythonModule, PPY_GET_PROGRAM_FULL_PATH, Py_GetProgramFullPath);
    RESOLVE(PythonModule, PPYGC_COLLECT, PyGC_Collect);

    LOAD(HookModule, "Hook");
    RESOLVE(HookModule, PHOOK, Hook);
    RESOLVE(HookModule, PUNHOOK, Unhook);
    RESOLVE(HookModule, PINITIALIZE_HOOKED_FUNCTION, InitializeHookedFunction);
    RESOLVE(HookModule, PHOOK_FUNCTION, HookFunction);

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        OutputDebugStringA("GetProcessHeap() failed.");
        goto End;
    }

    SecureZeroMemory(&Function, sizeof(Function));

    Function.Name = "TestFunc";
    Function.Module = "PythonExe";
    Function.Signature = "ULONG TestFunc(ULONG, ULONG, ULONG, ULONG)";
    Function.AllocationRoutine = (PALLOCATION_ROUTINE)HeapAllocationRoutine;
    Function.AllocationContext = HeapHandle;
    Function.NumberOfParameters = 4;
    Function.SizeOfReturnValueInBytes = 8;
    Function.OriginalAddress = TestFuncPointer;
    Function.HookEntry = NULL;
    Function.EntryContext = NULL;
    Function.HookExit = NULL;
    Function.ExitContext = NULL;

    Function.Hash.HighPart = 0;
    Function.Hash.LowPart = 581891183;

    //Function.EntryCallback = HookEntryCallback;

    goto DoInitialize;


    SecureZeroMemory(&Function, sizeof(Function));

    Function.Name = "PyObject_New";
    Function.Module = "python27";
    Function.Signature = "PPYOBJECT PyObject_New(PPYTYPEOBJECT)";
    Function.AllocationRoutine = (PALLOCATION_ROUTINE)HeapAllocationRoutine;
    Function.AllocationContext = HeapHandle;
    Function.NumberOfParameters = 1;
    Function.SizeOfReturnValueInBytes = 8;
    Function.OriginalAddress = Python->_PyObject_New;
    Function.HookEntry = NULL;
    Function.EntryContext = NULL;
    Function.HookExit = NULL;
    Function.ExitContext = NULL;

    Function.Hash.HighPart = 0;
    Function.Hash.LowPart = 581891183;

    SecureZeroMemory(&Function, sizeof(Function));

    Function.Name = "Py_Main";
    Function.Module = "python27";
    Function.Signature = "int Py_Main(int argc, char **argv)";
    Function.NtStyleSignature = "LONG Py_Main(LONG argc, PPCHAR argv)";
    Function.AllocationRoutine = (PALLOCATION_ROUTINE)HeapAllocationRoutine;
    Function.AllocationContext = HeapHandle;
    Function.NumberOfParameters = 2;
    Function.SizeOfReturnValueInBytes = 4;
    Function.OriginalAddress = Python->Py_Main;
    Function.HookEntry = NULL;
    Function.EntryContext = NULL;

    Function.HookExit = NULL;
    Function.ExitContext = NULL;

    Function.Hash.HighPart = 0;
    Function.Hash.LowPart = 581891183;

    goto DoInitialize;

    SecureZeroMemory(&Function, sizeof(Function));

    Function.Name = "Py_GetProgramFullPath";
    Function.Module = "python27";
    Function.Signature = "char* Py_GetProgramFullPath(void)";
    Function.NtStyleSignature = "PSTR Py_GetProgramFullPath(VOID)";
    Function.AllocationRoutine = (PALLOCATION_ROUTINE)HeapAllocationRoutine;
    Function.AllocationContext = HeapHandle;
    Function.NumberOfParameters = 0;
    Function.SizeOfReturnValueInBytes = 8;
    Function.OriginalAddress = Python->Py_GetProgramFullPath;
    Function.HookEntry = NULL;
    Function.EntryContext = NULL;
    Function.HookExit = NULL;
    Function.ExitContext = NULL;

    Function.Hash.HighPart = 0;
    Function.Hash.LowPart = 581891183;

DoInitialize:
    InitializeHookedFunction(Rtl, &Function);

    BaseAddress = (PCHAR)PAGE_ALIGN(
        ((ULONG_PTR)Function.OriginalAddress) -
        ((ULONG_PTR)Offset)
    );

    do {
        Buffer = VirtualAlloc(BaseAddress,
                              PAGE_SIZE,
                              MEM_COMMIT | MEM_RESERVE,
                              PAGE_READWRITE);

        if (Buffer) {
            break;
        }

        BaseAddress = (PCHAR)PAGE_ALIGN(
            ((ULONG_PTR)BaseAddress) -
            ((ULONG_PTR)Offset)
        );

    } while (--Attempts);

    if (!Buffer) {
        OutputDebugStringA("Failed to allocate buffer.\n");
        goto End;
    }

    Rtl->RtlCopyMemory(Buffer, &Function, sizeof(Function));
    HookedFunction = (PHOOKED_FUNCTION)Buffer;

    Success = HookFunction(
        Rtl,
        (PPVOID)&TestFuncPointer,
        HookedFunction
    );

    if (!Success) {
        OutputDebugStringA("HookFunction() failed.");
        goto End;
    }

    Result = TestFunc(1, 2, 4, 8);
    goto End;

    //Success = HookFunction(Rtl, (PPVOID)&Python->Py_Main, HookedFunction);

    Success = HookFunction(
        Rtl,
        (PPVOID)&Python->Py_GetProgramFullPath,
        HookedFunction
    );

    if (!Success) {
        OutputDebugStringA("HookFunction() failed.");
        goto End;
    }


    //HOOK((PPVOID)&PyGC_Collect, Our_PyGC_Collect);
    //Original_PyGC_Collect = Our_PyGC_Collect;
    //HOOK((PPVOID)&Py_GetPrefix, Our_Py_GetPrefix);
    //Prefix = Py_GetPrefix();

    //HOOK((PPVOID)&Py_GetExecPrefix, Our_Py_GetExecPrefix);
    //ExecPrefix = Py_GetExecPrefix();

    //HOOK((PPVOID)&Py_GetProgramFullPath, Our_Py_GetProgramFullPath);
    Path = Py_GetProgramFullPath();

    //Rtl->RtlCopyMemory(Path, PythonExePath, sizeof(PythonExePath));

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
