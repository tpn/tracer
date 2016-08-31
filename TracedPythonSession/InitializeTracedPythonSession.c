#include "stdafx.h"

#include "TracedPythonSessionPrivate.h"

PUNICODE_STRING
CurrentDirectoryToUnicodeString(
    PALLOCATOR Allocator
    )
{
    LONG_INTEGER NumberOfChars;
    LONG_INTEGER AllocSize;
    LONG_INTEGER AlignedAllocSize;
    PUNICODE_STRING String;

    NumberOfChars.LongPart = GetCurrentDirectoryW(0, NULL);
    if (NumberOfChars.LongPart <= 0) {
        return NULL;
    }

    if (NumberOfChars.HighPart) {
        return NULL;
    }

    AllocSize.LongPart = (
        (NumberOfChars.LongPart << 1) +
        sizeof(UNICODE_STRING)
    );
    AlignedAllocSize.LongPart = ALIGN_UP_POINTER(AllocSize.LongPart);

    if (AlignedAllocSize.HighPart) {
        return NULL;
    }

    String = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedAllocSize.LongPart
        )
    );

    if (!String) {
        return NULL;
    }

    String->Length = (USHORT)((NumberOfChars.LowPart - 1) << 1);
    String->MaximumLength = String->Length + sizeof(WCHAR);

    String->Buffer = (PWCHAR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

    if (GetCurrentDirectoryW(String->MaximumLength, String->Buffer) <= 0) {
        Allocator->Free(Allocator->Context, String);
        return NULL;
    }

    return String;
}

BOOL
ChangeIntoPythonHomeDirectory(
    PTRACED_PYTHON_SESSION Session
    )
{
    PRTL Rtl;
    PALLOCATOR Allocator;
    PUNICODE_STRING TargetDirectory;
    PUNICODE_STRING CurrentDirectory;

    Allocator = Session->Allocator;
    Rtl = Session->Rtl;

    CurrentDirectory = CurrentDirectoryToUnicodeString(Allocator);
    if (!CurrentDirectory) {
        return FALSE;
    }

    TargetDirectory = Session->PythonHomePath;

    if (!Rtl->RtlEqualUnicodeString(CurrentDirectory, TargetDirectory, TRUE)) {
        SetCurrentDirectoryW(TargetDirectory->Buffer);
    }

    Session->OriginalDirectory = CurrentDirectory;

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeTracedPythonSession(
    PPTRACED_PYTHON_SESSION SessionPointer,
    PTRACER_CONFIG TracerConfig,
    PALLOCATOR Allocator,
    HMODULE OwningModule
    )
/*++

Routine Description:

    This function initializes a TRACED_PYTHON_SESSION structure.

Arguments:

    SessionPointer - Supplies a pointer that will receive the address of the
        TRACED_PYTHON_SESSION structure allocated.  This pointer is immediately
        cleared (that is, '*SessionPointer = NULL;' is performed once
        SessionPointer is deemed non-NULL), and a value will only be set if
        initialization was successful.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    Allocator - Optionally supplies a pointer to an alternate ALLOCATOR to use.
        If not present, TracerConfig->Allocator will be used.

Return Value:

    TRUE on Success, FALSE if an error occurred.  *SessionPointer will be
    updated with the value of the newly created TRACED_PYTHON_SESSION
    structure.

--*/
{
    BOOL Success;
    BOOL Compress;
    CHAR MajorVersion;
    USHORT Index;
    ULONG RequiredSize;
    HMODULE PythonDllModule;
    PRTL Rtl;
    PPYTHON Python;
    PTRACER_PATHS Paths;
    PTRACED_PYTHON_SESSION Session;
    PUNICODE_STRING Path;
    PUNICODE_STRING Directory;
    PUNICODE_STRING PythonDllPath;
    PUNICODE_STRING PythonExePath;
    PDLL_DIRECTORY_COOKIE DirectoryCookie;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(SessionPointer)) {

        return FALSE;

    } else {

        //
        // Clear the user's session pointer up-front.
        //

        *SessionPointer = NULL;
    }

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {

        if (!TracerConfig->Allocator) {
            return FALSE;
        }

        Allocator = TracerConfig->Allocator;

    }

    Session = (PTRACED_PYTHON_SESSION)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*Session)
        )
    );

    if (!Session) {
        return FALSE;
    }

    //
    // If we reach this point, our Session has been successfully created and
    // the allocation functions have been initialized.  From this point on,
    // any subsequent errors must `goto Error;` in order to ensure the rundown
    // logic executes.
    //

    //
    // Fill out initial fields.
    //

    Session->Size = sizeof(*Session);
    Session->Allocator = Allocator;
    Session->TracerConfig = TracerConfig;

    //
    // Load the system modules.
    //

#define LOAD(Module, Name) do {                      \
    Session->Module = LoadLibraryA(Name);            \
    if (!Session->Module) {                          \
        OutputDebugStringA("Failed to load " #Name); \
        goto Error;                                  \
    }                                                \
} while (0)

    LOAD(Kernel32Module, "kernel32");
    LOAD(Shell32Module, "shell32");
    LOAD(User32Module, "user32");
    LOAD(Advapi32Module, "advapi32");
    LOAD(Winsock2Module, "WS2_32");

    //
    // Set our "owning" module.
    //

    if (OwningModule) {

        Session->OwningModule = OwningModule;

    } else {

        //
        // No module was specified, default to the default module for the
        // process (typically the launching .exe).
        //

        Session->OwningModule = GetModuleHandle(NULL);
    }

    //
    // Load our modules.
    //

#define LOAD_DLL(Name) do {                                            \
    Session->Name##Module = LoadLibraryW(Paths->Name##DllPath.Buffer); \
    if (!Session->Name##Module) {                                      \
        OutputDebugStringA("Failed to load " #Name);                   \
        goto Error;                                                    \
    }                                                                  \
} while (0)

    Paths = &TracerConfig->Paths;

    LOAD_DLL(Rtl);
    LOAD_DLL(Tracer);
    LOAD_DLL(Python);
    LOAD_DLL(PythonTracer);

    //
    // Resolve the functions.
    //

#define RESOLVE(Module, Type, Name) do {                                  \
    Session->Name = (Type)GetProcAddress(Session->Module, #Name);         \
    if (!Session->Name) {                                                 \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name "\n"); \
        goto Error;                                                       \
    }                                                                     \
} while (0)

    RESOLVE(Shell32Module, PCOMMAND_LINE_TO_ARGVW, CommandLineToArgvW);

    RESOLVE(RtlModule, PINITIALIZE_RTL, InitializeRtl);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_STORES, InitializeTraceStores);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_CONTEXT, InitializeTraceContext);
    RESOLVE(TracerModule, PINITIALIZE_TRACE_SESSION, InitializeTraceSession);
    RESOLVE(TracerModule, PCLOSE_TRACE_STORES, CloseTraceStores);

    RESOLVE(PythonModule, PFIND_PYTHON_DLL_AND_EXE, FindPythonDllAndExe);
    RESOLVE(PythonModule, PINITIALIZE_PYTHON, InitializePython);

    RESOLVE(PythonTracerModule,
        PINITIALIZE_PYTHON_TRACE_CONTEXT,
        InitializePythonTraceContext);

    //
    // All of our modules modules use the same pattern for initialization
    // whereby the required structure size can be obtained by passing a NULL
    // for the destination pointer argument and PULONG as the size argument.
    // We leverage that here, using the local &RequiredSize variable and some
    // macro glue.
    //

#undef ALLOCATE
#define ALLOCATE(Target, Type)                                             \
    Session->Target = (Type)(                                              \
        Allocator->Calloc(                                                 \
            Allocator->Context,                                            \
            1,                                                             \
            RequiredSize                                                   \
        )                                                                  \
    );                                                                     \
    if (!Session->Target) {                                                \
        OutputDebugStringA("Allocation failed for " #Target " struct.\n"); \
        goto Error;                                                        \
    }

    //
    // Allocate and initialize Rtl.
    //

    RequiredSize = 0;
    Session->InitializeRtl(NULL, &RequiredSize);
    ALLOCATE(Rtl, PRTL);
    if (!Session->InitializeRtl(Session->Rtl, &RequiredSize)) {
        OutputDebugStringA("Session->InitializeRtl() failed.\n");
        goto Error;
    }

    //
    // Rtl was successfully allocated and initialized.  Take a local copy of
    // the pointer as it's nicer to work with than going through Session->Rtl.
    //

    Rtl = Session->Rtl;

    //
    // Load our owning module name.
    //

    Success = Rtl->GetModulePath(
        Rtl,
        Session->OwningModule,
        Allocator,
        &Session->OwningModulePath
    );

    if (!Success) {
        OutputDebugStringA("Rtl->GetModulePath() failed.\n");
        goto Error;
    }

    //
    // Attempt to load the relevant Python DLL for this session.  Look in our
    // module's directory first, which will pick up the common case where we
    // live in the same directory as python.exe, and thus, the relevant Python
    // DLL (e.g. python27.dll, python35.dll etc).
    //

    Directory = &Session->OwningModulePath->Directory;
    Success = Session->FindPythonDllAndExe(
        Rtl,
        Allocator,
        Directory,
        &PythonDllPath,
        &PythonExePath,
        &Session->NumberOfPathEntries,
        &Session->PathEntries,
        &Session->PythonMajorVersion,
        &Session->PythonMinorVersion
    );

    if (!Success) {
        goto Error;
    }

    if (PythonDllPath) {
        goto FoundPython;
    }

    //
    // No Python DLL file was found next to us.  Try the DefaultPythonDirectory
    // in the registry/TracerConfig.
    //

    Directory = &Paths->DefaultPythonDirectory;

    if (Directory) {

        Success = Session->FindPythonDllAndExe(
            Rtl,
            Allocator,
            Directory,
            &PythonDllPath,
            &PythonExePath,
            &Session->NumberOfPathEntries,
            &Session->PathEntries,
            &Session->PythonMajorVersion,
            &Session->PythonMinorVersion
        );

        if (!Success) {
            goto Error;
        }

        if (PythonDllPath) {
            goto FoundPython;
        }

        OutputDebugStringA("Failed to find a Python DLL to load.\n");

        goto Error;
    }

FoundPython:

    MajorVersion = Session->PythonMajorVersion;

    if (MajorVersion != '2' && MajorVersion != '3') {
        OutputDebugStringA("Invalid major version!\n");
        goto Error;
    }

    Session->PythonDllPath = PythonDllPath;
    Session->PythonExePath = PythonExePath;
    Session->PythonHomePath = Directory;

    if (Session->NumberOfPathEntries == 0) {
        OutputDebugStringA("Session->NumberOfPathEntries invalid.\n");
        goto Error;
    }

    //
    // Allocate the array for DLL_DIRECTORY_COOKIE values.
    //

    Session->PathDirectoryCookies = (PDLL_DIRECTORY_COOKIE)(
        Allocator->Calloc(
            Allocator->Context,
            Session->NumberOfPathEntries,
            sizeof(DLL_DIRECTORY_COOKIE)
        )
    );

    if (!Session->PathDirectoryCookies) {
        OutputDebugStringA("Failed to allocate PathDirectoryCookies.\n");
        goto Error;
    }

    //
    // Create a UTF-8 version of the fully-qualified python.exe path so that
    // we can use it as argv[0].
    //

    Success = ConvertUtf16StringToUtf8String(
        PythonExePath,
        &Session->PythonExePathA,
        Allocator
    );

    if (!Success) {
        OutputDebugStringA("Exe:ConvertUtf16StringToUtf8String() failed.\n");
        goto Error;
    }

    //
    // Create a UTF-8 version of the fully-qualified directory containing the
    // python.exe and Python DLL so that we can set it as PYTHONHOME.
    //

    Success = ConvertUtf16StringToUtf8String(
        Session->PythonHomePath,
        &Session->PythonHomePathA,
        Allocator
    );

    if (!Success) {
        OutputDebugStringA("Home:ConvertUtf16StringToUtf8String() failed.\n");
        goto Error;
    }

    //
    // Remove conflicting PATH entries from our environment variable.
    //

    Success = SanitizePathEnvironmentVariableForPython(Session);
    if (!Success) {
        OutputDebugStringA("Failed to remove conflicting Python PATHs.\n");
    }

    //
    // Temporarily change into the Python HOME directory.
    //

    AddDllDirectory(Directory->Buffer);

    Success = ChangeIntoPythonHomeDirectory(Session);
    if (!Success) {
        goto Error;
    }

    //
    // Add the Python PATH entries to the list of DLL directories.
    //

    goto LoadPythonDll;
    Path = Session->PathEntries;
    DirectoryCookie = Session->PathDirectoryCookies;
    for (Index = 0; Index < Session->NumberOfPathEntries; Index++) {
        DLL_DIRECTORY_COOKIE Cookie = AddDllDirectory((PCWSTR)Path->Buffer);
        if (!Cookie) {
            OutputDebugStringA("AddDllDirectory() failed.\n");
            OutputDebugStringW(Path->Buffer);
            OutputDebugStringA("\n");
            goto Error;
        }
        Path++;
        *DirectoryCookie++ = Cookie;
    }

    //
    // Load the library.
    //

LoadPythonDll:
    PythonDllModule = LoadLibraryW(PythonDllPath->Buffer);

    if (!PythonDllModule) {
        OutputDebugStringA("LoadLibraryW() of Python DLL failed.\n");
        goto Error;
    }

    Session->PythonDllModule = PythonDllModule;

    //
    // Call InitializePython() now that we've loaded a Python DLL.
    //

    RequiredSize = 0;
    Session->InitializePython(NULL, NULL, NULL, &RequiredSize);
    ALLOCATE(Python, PPYTHON);
    Success = Session->InitializePython(
        Rtl,
        PythonDllModule,
        Session->Python,
        &RequiredSize
    );

    if (!Success) {
        OutputDebugStringA("InitializePython() failed.\n");
        goto Error;
    }

    Python = Session->Python;

    if (Python->VersionString[0] != MajorVersion) {

        //
        // The version derived from the DLL name did not match the version
        // derived from Py_GetVersion().
        //

        goto Error;
    }

    //
    // Get ascii and unicode versions of the command line.
    //

    Session->CommandLineA = GetCommandLineA();
    Session->CommandLineW = GetCommandLineW();

    if (!Session->CommandLineA && !Session->CommandLineW) {
        goto Error;
    }

    //
    // Convert the unicode commandline into an argc/argv equivalent.
    //

    Session->ArgvW = Session->CommandLineToArgvW(
        Session->CommandLineW,
        &Session->NumberOfArguments
    );

    if (!Session->ArgvW) {
        goto Error;
    }

    if (Session->NumberOfArguments == 0) {
        goto Error;
    }

    //
    // Replace argv[0] with the name of the Python executable.
    //

    Session->ArgvW[0] = PythonExePath->Buffer;

    //
    // Convert the unicode argv into an ansi argv.
    //

    Success = Rtl->ArgvWToArgvA(
        Session->ArgvW,
        Session->NumberOfArguments,
        &Session->ArgvA,
        NULL,
        Allocator
    );

    if (!Success) {
        goto Error;
    }

    //
    // Skip the first argument (the executable name) for what we pass to Python.
    //

    Session->PythonNumberOfArguments = Session->NumberOfArguments - 1;
    Session->PythonArgvA = Session->ArgvA + 1;
    Session->PythonArgvW = Session->ArgvW + 1;

    //
    // If we need to hook functions such that we've got hooks in place before
    // anything is called, we'd do it here.
    //


    //
    // Set the Python program name to the "python.exe" path, set PYTHONHOME
    // to the containing directory, initialize the interpreter, then set args.
    //
    // Use the ANSI versions if we're Python 2, wide character versions if 3.
    //

    if (MajorVersion == '2') {
        LONG Argc = Session->PythonNumberOfArguments;
        PPSTR Argv = Session->PythonArgvA;

        Python->Py_SetProgramNameA(Session->PythonExePathA->Buffer);
        Python->Py_SetPythonHomeA(Session->PythonHomePathA->Buffer);

        SetEnvironmentVariableA(
            "PYTHONHOME",
            Session->PythonHomePathA->Buffer
        );

        Python->Py_Initialize();

        if (Python->PySys_SetArgvExA) {

            //
            // Available since 2.6.6.
            //

            Python->PySys_SetArgvExA(Argc, Argv, 0);

        } else {

            Python->PySys_SetArgvA(Argc, Argv);

        }

    } else {

        LONG Argc = Session->PythonNumberOfArguments;
        PPWSTR Argv = Session->PythonArgvW;

        Python->Py_SetProgramNameW(Session->PythonExePath->Buffer);
        Python->Py_SetPythonHomeW(Session->PythonHomePath->Buffer);

        SetEnvironmentVariableW(
            L"PYTHONHOME",
            Session->PythonHomePath->Buffer
        );

        Python->Py_Initialize();

        if (Python->PySys_SetArgvExW) {

            //
            // Available since 3.1.3.
            //

            Python->PySys_SetArgvExW(Argc, Argv, 0);

        } else {

            Python->PySys_SetArgvW(Argc, Argv);

        }
    }

    //
    // Change back to the original directory.
    //

    if (Session->OriginalDirectory) {
        SetCurrentDirectoryW(Session->OriginalDirectory->Buffer);
    }

    //
    // Allocate and initialize TraceSession.
    //

    RequiredSize = 0;
    Session->InitializeTraceSession(Rtl, NULL, &RequiredSize);
    ALLOCATE(TraceSession, PTRACE_SESSION);
    Success = Session->InitializeTraceSession(
        Rtl,
        Session->TraceSession,
        &RequiredSize
    );

    if (!Success) {
        OutputDebugStringA("Session->InitializeTraceSession() failed.\n");
        goto Error;
    }

    //
    // Create a trace session directory.
    //

    Success = CreateTraceSessionDirectory(
        TracerConfig,
        &Session->TraceSessionDirectory
    );

    if (!Success) {
        OutputDebugStringA("CreateTraceSessionDirectory() failed.\n");
        goto Error;
    }

    //
    // See if we've been configured to compress trace store directories.
    //

    Compress = !TracerConfig->Flags.DisableTraceSessionDirectoryCompression;

    //
    // Get the required size of the TRACE_STORES structure.
    //

    RequiredSize = 0;
    Session->InitializeTraceStores(
        NULL,
        NULL,
        NULL,
        &RequiredSize,
        NULL,
        FALSE,
        FALSE
    );

    //
    // Allocate sufficient space, then initialize the stores.
    //

    ALLOCATE(TraceStores, PTRACE_STORES);
    Success = Session->InitializeTraceStores(
        Rtl,
        Session->TraceSessionDirectory->Buffer,
        Session->TraceStores,
        &RequiredSize,
        NULL,
        FALSE,
        Compress
    );

    if (!Success) {
        OutputDebugStringA("InitializeTraceStores() failed.\n");
        goto Error;
    }

    //
    // Get the maximum number of processors and create a threadpool
    // and environment.
    //

    Session->MaximumProcessorCount = (
        GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS)
    );

    Session->Threadpool = CreateThreadpool(NULL);
    if (!Session->Threadpool) {
        OutputDebugStringA("CreateThreadpool() failed.\n");
        goto Error;
    }

    //
    // Limit the threadpool to the number of processors in the system.
    //

    SetThreadpoolThreadMinimum(
        Session->Threadpool,
        Session->MaximumProcessorCount
    );

    SetThreadpoolThreadMaximum(
        Session->Threadpool,
        Session->MaximumProcessorCount
    );

    //
    // Initialize the threadpool environment and bind it to the threadpool.
    //

    InitializeThreadpoolEnvironment(&Session->ThreadpoolCallbackEnviron);
    SetThreadpoolCallbackPool(
        &Session->ThreadpoolCallbackEnviron,
        Session->Threadpool
    );

    //
    // Get the required size of a TRACE_CONTEXT structure.
    //

    RequiredSize = 0;
    Session->InitializeTraceContext(
        NULL,
        NULL,
        &RequiredSize,
        NULL,
        NULL,
        NULL,
        NULL,
        FALSE,
        FALSE
    );

    //
    // Allocate sufficient space then initialize the trace context.
    //

    ALLOCATE(TraceContext, PTRACE_CONTEXT);
    Success = Session->InitializeTraceContext(
        Rtl,
        Session->TraceContext,
        &RequiredSize,
        Session->TraceSession,
        Session->TraceStores,
        &Session->ThreadpoolCallbackEnviron,
        (PVOID)Session,
        FALSE,
        Compress
    );

    if (!Success) {
        OutputDebugStringA("InitializeTraceContext() failed.\n");
        goto Error;
    }

    //
    // Get the required size of a PYTHON_TRACE_CONTEXT structure.
    //

    RequiredSize = 0;
    Session->InitializePythonTraceContext(
        NULL,
        NULL,
        &RequiredSize,
        NULL,
        NULL,
        NULL,
        NULL
    );

    //
    // Allocate space and initialize the Python trace context.
    //

    ALLOCATE(PythonTraceContext, PPYTHON_TRACE_CONTEXT);
    Success = Session->InitializePythonTraceContext(
        Rtl,
        Session->PythonTraceContext,
        &RequiredSize,
        Session->Python,
        Session->TraceContext,
        NULL,
        (PVOID)Session
    );

    if (!Success) {
        OutputDebugStringA("InitializePythonTraceContext() failed.\n");
        goto Error;
    }

    //
    // We're finally done.
    //

    Success = TRUE;

    goto End;

Error:
    Success = FALSE;

    DestroyTracedPythonSession(&Session);

End:

    //
    // Update the user's pointer if we were successful.
    //

    if (Success) {
        *SessionPointer = Session;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
