/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InitializeDebugEngineSession.c

Abstract:

    This module implements functionality related to initializing a debug engine
    session.  Routines are provided for initializing via the command line,
    initializing from the current process, and initializing the structure
    itself.

--*/

#include "stdafx.h"

#define DEBUG_ENGINE_USAGE_STRING \
    "Invalid usage.  Usage: "     \
    "[-p|--pid <pid>]|[-c|--commandline <command line string>]"

_Use_decl_annotations_
BOOL
InitializeFromCommandLine(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    Initialize a debug engine session via command line parameters.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    Session - Supplies a pointer to the debug engine session being initialized.
        The command line options string table must have already been created.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    PCHAR OptionA;
    NTSTATUS Status;
    SHORT MatchIndex;
    SHORT MaxMatchIndex;
    STRING_MATCH Match;
    STRING OptionString;
    HMODULE Shell32Module = NULL;
    PSTRING_TABLE StringTable;
    DEBUG_ENGINE_COMMAND_LINE_OPTION Option;
    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Session)) {
        return FALSE;
    }

    //
    // Extract the command line for the current process.
    //

    LOAD_LIBRARY_A(Shell32Module, Shell32);

    RESOLVE_FUNCTION(CommandLineToArgvW,
                     Shell32Module,
                     PCOMMAND_LINE_TO_ARGVW,
                     CommandLineToArgvW);

    CHECKED_MSG(Session->CommandLineA = GetCommandLineA(), "GetCommandLineA()");
    CHECKED_MSG(Session->CommandLineW = GetCommandLineW(), "GetCommandLineW()");

    Session->ArgvW = CommandLineToArgvW(Session->CommandLineW,
                                        &Session->NumberOfArguments);

    CHECKED_MSG(Session->ArgvW, "Shell32!CommandLineToArgvW()");

    CHECKED_MSG(
        Rtl->ArgvWToArgvA(
            Session->ArgvW,
            Session->NumberOfArguments,
            &Session->ArgvA,
            NULL,
            Allocator
        ),
        "Rtl!ArgvWToArgA"
    );

    CHECKED_MSG(Session->NumberOfArguments >= 3, DEBUG_ENGINE_USAGE_STRING);

    //
    // Extract the ANSI version of the first argument (which should be one of
    // the command options), convert it into a STRING representation, then do
    // a lookup in the string table.
    //

    OptionA = Session->ArgvA[1];
    OptionString.Buffer = OptionA;
    OptionString.Length = (USHORT)strlen(OptionA);
    OptionString.MaximumLength = OptionString.Length;

    StringTable = Session->CommandLineOptionsStringTable;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable, &OptionString, &Match);

    CHECKED_MSG(MatchIndex != NO_MATCH_FOUND, DEBUG_ENGINE_USAGE_STRING);

    MaxMatchIndex = NumberOfCommandLineMatchIndexOptions;

    if (MatchIndex > MaxMatchIndex) {
        __debugbreak();
        return FALSE;
    }

    Option = CommandLineMatchIndexToOption[MatchIndex];

    switch (Option) {

        case PidCommandLineOption:

            //
            // Extract the process ID to attach to.
            //

            CHECKED_NTSTATUS_MSG(
                Rtl->RtlCharToInteger(
                    Session->ArgvA[2],
                    10,
                    &Session->TargetProcessId
                ),
                "Rtl->RtlCharToInteger(ArgvA[1])"
            );

            Session->Flags.AttachedToExistingProcess = TRUE;

            break;

        case CommandLineCommandLineOption:

            Session->TargetCommandLineA = Session->ArgvA[2];
            Session->Flags.CreatedNewProcess = TRUE;

            break;

        default:

            //
            // This should be unreachable.
            //

            __debugbreak();
            return FALSE;
    }

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    if (Session) {
        MAYBE_CLOSE_HANDLE(Session->TargetProcessHandle);
    }


End:
    MAYBE_FREE_LIBRARY(Shell32Module);

    return Success;
}

_Check_return_
_Success_(return != 0)
BOOL
InitializeDebugEngineSessionStringTables(
    _In_ PRTL Rtl,
    _Inout_ PDEBUG_ENGINE_SESSION Session,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString
    )
{

    //
    // Initialize the string table constructor function pointer and the
    // string array and table allocators.
    //

    Session->CreateStringTableFromDelimitedString = (
        CreateStringTableFromDelimitedString
    );
    Session->StringArrayAllocator = StringArrayAllocator;
    Session->StringTableAllocator = StringTableAllocator;

    //
    // Create the string table for the examine symbols prefix output.
    //

    Session->ExamineSymbolsPrefixStringTable = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &ExamineSymbolsPrefixes,
            StringTableDelimiter
        )
    );

    if (!Session->ExamineSymbolsPrefixStringTable) {
        goto Error;
    }

    //
    // Create the string tables for the examine symbols type output.
    //

    Session->ExamineSymbolsBasicTypeStringTable1 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &ExamineSymbolsBasicTypes1,
            StringTableDelimiter
        )
    );

    if (!Session->ExamineSymbolsBasicTypeStringTable1) {
        goto Error;
    }

    Session->ExamineSymbolsBasicTypeStringTable2 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &ExamineSymbolsBasicTypes2,
            StringTableDelimiter
        )
    );

    if (!Session->ExamineSymbolsBasicTypeStringTable2) {
        goto Error;
    }

    Session->NumberOfBasicTypeStringTables = 2;

    //
    // Create the string tables for function arguments.
    //

    Session->FunctionArgumentTypeStringTable1 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &FunctionArgumentTypes1,
            StringTableDelimiter
        )
    );

    if (!Session->FunctionArgumentTypeStringTable1) {
        goto Error;
    }

    Session->FunctionArgumentTypeStringTable2 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &FunctionArgumentTypes2,
            StringTableDelimiter
        )
    );

    if (!Session->FunctionArgumentTypeStringTable2) {
        goto Error;
    }

    Session->FunctionArgumentVectorTypeStringTable1 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &FunctionArgumentVectorTypes1,
            StringTableDelimiter
        )
    );

    if (!Session->FunctionArgumentVectorTypeStringTable1) {
        goto Error;
    }

    //
    // The vector string table doesn't get included in the initial matching
    // logic (because the types come through as `union __m128`, so we set
    // this to 2 instead of 3.  A manual check is done against the string
    // table when parsing unions.
    //

    Session->NumberOfFunctionArgumentTypeStringTables = 2;

    //
    // Create the string table for display types.
    //

    Session->DisplayTypesStringTable1 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &DisplayTypes1,
            StringTableDelimiter
        )
    );

    if (!Session->DisplayTypesStringTable1) {
        goto Error;
    }

    Session->DisplayTypesStringTable2 = (
        CreateStringTableFromDelimitedString(
            Rtl,
            StringTableAllocator,
            StringArrayAllocator,
            &DisplayTypes2,
            StringTableDelimiter
        )
    );

    if (!Session->DisplayTypesStringTable2) {
        goto Error;
    }

    Session->NumberOfDisplayTypesStringTables = 2;

    //
    // If we're initializing from the command line, create the command line
    // options string table.
    //

    if (InitFlags.InitializeFromCommandLine) {

        Session->CommandLineOptionsStringTable = (
            CreateStringTableFromDelimitedString(
                Rtl,
                StringTableAllocator,
                StringArrayAllocator,
                &CommandLineOptions,
                StringTableDelimiter
            )
        );

    }

    return TRUE;

Error:

    return FALSE;
}

_Use_decl_annotations_
BOOL
InitializeDebugEngineSession(
    PRTL Rtl,
    PALLOCATOR Allocator,
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    PTRACER_CONFIG TracerConfig,
    HMODULE StringTableModule,
    PALLOCATOR StringArrayAllocator,
    PALLOCATOR StringTableAllocator,
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
/*++

Routine Description:

    This routine initializes a debug engine session.  This involves loading
    DbgEng.dll, initializing COM, creating an IDebugClient COM object,
    attaching to the process indicated in InitFlags, then creating the
    remaining COM objects and setting relevant client callbacks.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    InitFlags - Supplies initialization flags that are used to control how
        this routine attaches to the initial process.

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.

    StringTableModule - Optionally supplies a handle to the StringTable module
        obtained via an earlier LoadLibrary() call.

    StringArrayAllocator - Optionally supplies a pointer to an allocator to
        use for string array allocations.

    StringTableAllocator - Optionally supplies a pointer to an allocator to
        use for string table allocations.

    SessionPointer - Supplies an address to a variable that will receive the
        address of the newly allocated and initialized DEBUG_ENGINE_SESSION
        structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    PINITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT Initializer;

    Initializer = InitializeDebugEngineSessionWithInjectionIntent;

    return Initializer(Rtl,
                       Allocator,
                       InitFlags,
                       TracerConfig,
                       StringTableModule,
                       StringArrayAllocator,
                       StringTableAllocator,
                       NULL,
                       SessionPointer);
}

_Use_decl_annotations_
BOOL
InitializeDebugEngineSessionCommon(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
{
    BOOL Success;
    LONG_INTEGER AllocSizeInBytes;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE ExitDispatchEngine;
    PDEBUG_ENGINE_SESSION Session = NULL;

    //
    // Calculate the required allocation size.
    //

    AllocSizeInBytes.LongPart = (

        //
        // Account for the DEBUG_ENGINE_SESSION structure.
        //

        sizeof(DEBUG_ENGINE_SESSION) +

        //
        // Account for the primary the DEBUG_ENGINE structure (Session->Engine).
        //

        sizeof(DEBUG_ENGINE) +

        //
        // Account for the exit callbacks engine (Session->ExitDispatchEngine).
        //

        sizeof(DEBUG_ENGINE)
    );

    //
    // Sanity check our size isn't over MAX_USHORT.
    //

    if (AllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    Session = (PDEBUG_ENGINE_SESSION)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSizeInBytes.LowPart
        )
    );

    if (!Session) {
        goto Error;
    }

    //
    // Memory was allocated successfully.  Carve out the primary DEBUG_ENGINE.
    //

    Engine = Session->Engine = (PDEBUG_ENGINE)(
        RtlOffsetToPointer(
            Session,
            sizeof(DEBUG_ENGINE_SESSION)
        )
    );

    //
    // Point the engine at the session.
    //

    Engine->Session = Session;

    //
    // Ditto for the exit callbacks engine.
    //

    ExitDispatchEngine = Session->ExitDispatchEngine = (PDEBUG_ENGINE)(
        RtlOffsetToPointer(
            Session,
            sizeof(DEBUG_ENGINE_SESSION) +
            sizeof(DEBUG_ENGINE)
        )
    );

    ExitDispatchEngine->Session = Session;

    //
    // Initialize and acquire the debug engine locks.
    //

    InitializeSRWLock(&Engine->Lock);
    InitializeSRWLock(&ExitDispatchEngine->Lock);
    AcquireDebugEngineLock(Engine);
    AcquireDebugEngineLock(ExitDispatchEngine);

    //
    // Set the command function pointers.
    //

    Session->WaitForEvent = DebugEngineSessionWaitForEvent;
    Session->DispatchCallbacks = DebugEngineSessionDispatchCallbacks;
    Session->Destroy = DestroyDebugEngineSession;
    Session->DisplayType = DebugEngineDisplayType;
    Session->ExamineSymbols = DebugEngineExamineSymbols;
    Session->UnassembleFunction = DebugEngineUnassembleFunction;

    //
    // Set the meta command function pointers.
    //

    Session->SettingsMeta = DebugEngineSettingsMeta;
    Session->ListSettings = DebugEngineListSettings;

    //
    // Set other function pointers.
    //

    Session->EventLoop = DebugEngineSessionEventLoop;
    Session->EventLoopRunOnce = DebugEngineSessionEventLoopRunOnce;
    Session->ExecuteStaticCommand = DebugEngineSessionExecuteStaticCommand;
    Session->InitializeDebugEngineOutput = InitializeDebugEngineOutput;
    Session->InitializeChildDebugEngineSession = (
        InitializeChildDebugEngineSession
    );

    //
    // Set miscellaneous fields.
    //

    Session->Rtl = Rtl;
    Session->Allocator = Allocator;
    Session->TracerConfig = TracerConfig;

    InitializeGuardedListHead(&Session->Children);

    //
    // Create the APC-related events.
    //

    Session->NumberOfPendingApcs = 0;
    Session->ReadyForApcEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    Session->WaitForApcEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!Session->ReadyForApcEvent || !Session->WaitForApcEvent) {
        goto Error;
    }

    //
    // Create a shutdown event.  (We should probably make this named such that
    // it can be set externally.)
    //

    Session->ShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!Session->ShutdownEvent) {
        goto Error;
    }

    Engine->SizeOfStruct = sizeof(*Engine);
    ExitDispatchEngine->SizeOfStruct = sizeof(*Engine);
    Session->SizeOfStruct = sizeof(*Session);

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    if (Session) {
        __debugbreak();
        Session = NULL;
    }

    //
    // Intentional follow-on to End.
    //

End:

    *SessionPointer = Session;

    return Success;
}

BOOL
ConnectClient(
    _In_ PIDEBUGCLIENT IClient
    )
{
    BOOL Success;
    HRESULT Result;
    PDEBUGCLIENT Client;
    ULONG HistoryLimit;
    ULONG ConnectSessionFlags;

    HistoryLimit = 0;

    ConnectSessionFlags = (
        DEBUG_CONNECT_SESSION_NO_VERSION |
        DEBUG_CONNECT_SESSION_NO_ANNOUNCE
    );

    Client = (PDEBUGCLIENT)IClient->lpVtbl;

    CHECKED_HRESULT_MSG(
        Client->ConnectSession(
            IClient,
            ConnectSessionFlags,
            HistoryLimit
        ),
        "Client->ConnectSession()"
    );

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

_Use_decl_annotations_
BOOL
InitializeDebugEngineSessionWithInjectionIntent(
    PRTL Rtl,
    PALLOCATOR Allocator,
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    PTRACER_CONFIG TracerConfig,
    HMODULE StringTableModule,
    PALLOCATOR StringArrayAllocator,
    PALLOCATOR StringTableAllocator,
    PTRACER_INJECTION_MODULES InjectionModules,
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
/*++

Routine Description:

    This routine initializes a debug engine session.  This involves loading
    DbgEng.dll, initializing COM, creating an IDebugClient COM object,
    attaching to the process indicated in InitFlags, then creating the
    remaining COM objects and setting relevant client callbacks.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    InitFlags - Supplies initialization flags that are used to control how
        this routine attaches to the initial process.

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.

    StringTableModule - Optionally supplies a handle to the StringTable module
        obtained via an earlier LoadLibrary() call.

    StringArrayAllocator - Optionally supplies a pointer to an allocator to
        use for string array allocations.

    StringTableAllocator - Optionally supplies a pointer to an allocator to
        use for string table allocations.

    InjectionModules - Supplies a pointer to an initialized tracer injection
        modules structure.

    SessionPointer - Supplies an address to a variable that will receive the
        address of the newly allocated and initialized DEBUG_ENGINE_SESSION
        structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    HRESULT Result;
    ULONG ExecutionStatus;
    ULONG CreateProcessFlags;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE ExitDispatchEngine;
    ULONG ModuleIndex;
    ULONGLONG ModuleBaseAddress;
    PCUNICODE_STRING DebuggerSettingsXmlPath;
    PDEBUG_ENGINE_SESSION Session = NULL;
    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    //
    // Clear the caller's session pointer up-front.
    //

    *SessionPointer = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringTableModule)) {
        CreateStringTableFromDelimitedString = NULL;
    } else {
        if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
            return FALSE;
        }
        if (!ARGUMENT_PRESENT(StringTableAllocator)) {
            return FALSE;
        }

        CreateStringTableFromDelimitedString = (
            (PCREATE_STRING_TABLE_FROM_DELIMITED_STRING)(
                GetProcAddress(
                    StringTableModule,
                    "CreateStringTableFromDelimitedString"
                )
            )
        );

        if (!CreateStringTableFromDelimitedString) {
            return FALSE;
        }
    }

    //
    // Validate initialization flags.
    //

    if (!InitFlags.InitializeFromCommandLine &&
        !InitFlags.InitializeFromCurrentProcess) {
        OutputDebugStringA("DebugEngine: Invalid InitFlags.\n");
        return FALSE;
    }

    //
    // Parsing the command line requires a string table; make sure we can create
    // one if applicable.
    //

    if (InitFlags.InitializeFromCommandLine &&
        !CreateStringTableFromDelimitedString) {
        __debugbreak();
        return FALSE;
    }

    //
    // Injection intent is only supported when out-of-proc.
    //

    if (InitFlags.InitializeFromCurrentProcess && InjectionModules != NULL) {
        __debugbreak();
        return FALSE;
    }

    //
    // Initialize our __C_specific_handler from Rtl.
    //

    __C_specific_handler_impl = Rtl->__C_specific_handler;

    //
    // Load DbgEng.dll.
    //

    CHECKED_MSG(Rtl->LoadDbgEng(Rtl), "Rtl!LoadDbgEng()");

    //
    // Initialize common structure fields.
    //

    Success = InitializeDebugEngineSessionCommon(Rtl,
                                                 Allocator,
                                                 TracerConfig,
                                                 SessionPointer);

    if (!Success) {
        goto Error;
    }

    Session = *SessionPointer;

    Session->EventDispatch = Session->WaitForEvent;
    Session->InjectionModules = InjectionModules;
    Engine = Session->Engine;
    ExitDispatchEngine = Session->ExitDispatchEngine;

    //
    // Create a shutdown complete event.  This will be signaled when the debug
    // engine has detached from the target process.  This is only done in the
    // parent process.
    //

    Session->ShutdownCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!Session->ShutdownCompleteEvent) {
        goto Error;
    }

    AcquiredLock = TRUE;

    //
    // Set StringTable related fields.
    //

    if (CreateStringTableFromDelimitedString) {
        CHECKED_MSG(
            InitializeDebugEngineSessionStringTables(
                Rtl,
                Session,
                InitFlags,
                StringTableAllocator,
                StringArrayAllocator,
                CreateStringTableFromDelimitedString
            ),
            "InitializeDebugEngineSessionStringTables()"
        );
    }

    //
    // Initialize the debug engine and create the COM interfaces.
    //

    CHECKED_MSG(
        InitializeDebugEngine(Rtl, Engine),
        "InitializeDebugEngine()"
    );

    CHECKED_MSG(
        InitializeChildDebugEngine(Rtl, ExitDispatchEngine, Engine),
        "InitializeChildDebugEngine(Rtl, ExitDispatchEngine, Engine)"
    );

    if (InitFlags.InitializeFromCommandLine) {

        CHECKED_MSG(
            InitializeFromCommandLine(
                Rtl,
                Allocator,
                Session
            ),
            "InitializeDebugEngine()->FromCommandLine"
        );
        Session->Flags.OutOfProc = TRUE;

    } else if (InitFlags.InitializeFromCurrentProcess) {

        Session->TargetProcessId = FastGetCurrentProcessId();
        Session->Flags.InProc = TRUE;
        Session->Flags.AttachedToExistingProcess = TRUE;

    }

    Client = Engine->Client;
    IClient = Engine->IClient;

    if (Session->Flags.AttachedToExistingProcess) {

        //
        // Attach to the process with the debug client.
        //

        CHECKED_HRESULT_MSG(
            Client->AttachProcess(
                IClient,
                0,
                Session->TargetProcessId,
                DEBUG_ATTACH_NONINVASIVE |
                DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND
            ),
            "IDebugClient->AttachProcess()"
        );

        CHECKED_HRESULT_MSG(
            Engine->Control->WaitForEvent(
                Engine->IControl,
                0,
                0
            ),
            "Control->WaitForEvent()"
        );

        CHECKED_HRESULT_MSG(
            Engine->Control->GetExecutionStatus(
                Engine->IControl,
                &ExecutionStatus
            ),
            "Control->GetExecutionStatus()"
        );

        //
        // Attempt to resolve Rtl.
        //

        CHECKED_HRESULT_MSG(
            Engine->Symbols->GetModuleByModuleName(
                Engine->ISymbols,
                "Rtl",
                0,
                &ModuleIndex,
                &ModuleBaseAddress
            ),
            "Symbols->GetModuleByModuleName('Rtl')"
        );

    } else if (Session->Flags.CreatedNewProcess && !InjectionModules) {

        DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask;

        //
        // Register event callbacks.
        //

        Engine->State.RegisteringEventCallbacks = TRUE;

        InterestMask.AsULong = (
            DEBUG_EVENT_BREAKPOINT          |
            DEBUG_EVENT_EXCEPTION           |
            DEBUG_EVENT_CREATE_THREAD       |
            DEBUG_EVENT_EXIT_THREAD         |
            DEBUG_EVENT_CREATE_PROCESS      |
            DEBUG_EVENT_EXIT_PROCESS        |
            DEBUG_EVENT_LOAD_MODULE         |
            DEBUG_EVENT_UNLOAD_MODULE       |
            DEBUG_EVENT_SYSTEM_ERROR        |
            DEBUG_EVENT_CHANGE_SYMBOL_STATE
        );

        //
        // The following block can be useful during debugging to quickly
        // toggle interest masks on and off.
        //

        if (0) {
            InterestMask.AsULong = 0;
            InterestMask.Breakpoint = TRUE;
            InterestMask.Exception = TRUE;
            InterestMask.CreateThread = TRUE;
            InterestMask.ExitThread = TRUE;
            InterestMask.CreateProcess = TRUE;
            InterestMask.ExitProcess = TRUE;
            InterestMask.LoadModule = TRUE;
            InterestMask.UnloadModule = TRUE;
            InterestMask.SystemError = TRUE;
            InterestMask.SessionStatus = TRUE;
            InterestMask.ChangeDebuggeeState = TRUE;
            InterestMask.ChangeEngineState = TRUE;
            InterestMask.ChangeSymbolState = TRUE;
        }

        //
        // DebugEngineSetEventCallbacks() will attempt to acquire the engine
        // lock, so release it now first.
        //

        ReleaseDebugEngineLock(Engine);
        AcquiredLock = FALSE;

        CHECKED_MSG(
            DebugEngineSetEventCallbacks(
                Engine,
                &DebugEventCallbacks,
                &IID_IDEBUG_EVENT_CALLBACKS,
                InterestMask
            ),
            "DebugEngineSetEventCallbacks()"
        );

        //
        // Re-acquire the lock.
        //

        AcquireDebugEngineLock(Engine);
        AcquiredLock = TRUE;

        Engine->State.EventCallbacksRegistered = TRUE;
        Engine->State.RegisteringEventCallbacks = FALSE;

        //
        // Create a new process based on the target command line extracted
        // from this process's command line.
        //

        Engine->State.CreatingProcess = TRUE;

        CreateProcessFlags = (
            DEBUG_PROCESS |
            DEBUG_ECREATE_PROCESS_INHERIT_HANDLES
        );

        CHECKED_HRESULT_MSG(
            Client->CreateProcess(
                IClient,
                0,
                (PSTR)Session->TargetCommandLineA,
                CreateProcessFlags
            ),
            "DebugEngine: Client->CreateProcess"
        );

        Engine->State.CreatingProcess = FALSE;
        Engine->State.ProcessCreated = TRUE;

        //
        // The debugger engine doesn't complete its connection until we call
        // WaitForEvent().  Do that now.
        //

        Result = Engine->Control->WaitForEvent(Engine->IControl, 0, 0);

        if (Result != S_OK && Result != S_FALSE) {
            OutputDebugStringA("Failed: Control->WaitForEvent().\n");
            goto Error;
        }

        //
        // Get the execution status.
        //

        CHECKED_HRESULT_MSG(
            Engine->Control->GetExecutionStatus(
                Engine->IControl,
                &ExecutionStatus
            ),
            "Control->GetExecutionStatus()"
        );

    } else if (Session->Flags.CreatedNewProcess && InjectionModules) {
        ULONG Index;
        ULONG WaitResult;
        HMODULE Module;
        HANDLE ReadyEvent;
        PUNICODE_STRING Path;
        PINITIALIZE_TRACER_INJECTION Initializer;
        DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask;

        //
        // Register event callbacks.
        //

        Engine->State.RegisteringEventCallbacks = TRUE;

        InterestMask.AsULong = (
            DEBUG_EVENT_BREAKPOINT          |
            DEBUG_EVENT_EXCEPTION           |
            DEBUG_EVENT_CREATE_THREAD       |
            DEBUG_EVENT_EXIT_THREAD         |
            DEBUG_EVENT_CREATE_PROCESS      |
            DEBUG_EVENT_EXIT_PROCESS        |
            DEBUG_EVENT_LOAD_MODULE         |
            DEBUG_EVENT_UNLOAD_MODULE       |
            DEBUG_EVENT_SYSTEM_ERROR        |
            DEBUG_EVENT_CHANGE_SYMBOL_STATE
        );

        //
        // The following block can be useful during debugging to quickly
        // toggle interest masks on and off.
        //

        if (0) {
            InterestMask.AsULong = 0;
            InterestMask.Breakpoint = TRUE;
            InterestMask.Exception = TRUE;
            InterestMask.CreateThread = TRUE;
            InterestMask.ExitThread = TRUE;
            InterestMask.CreateProcess = TRUE;
            InterestMask.ExitProcess = TRUE;
            InterestMask.LoadModule = TRUE;
            InterestMask.UnloadModule = TRUE;
            InterestMask.SystemError = TRUE;
            InterestMask.SessionStatus = TRUE;
            InterestMask.ChangeDebuggeeState = TRUE;
            InterestMask.ChangeEngineState = TRUE;
            InterestMask.ChangeSymbolState = TRUE;
        }

        if (1) {
            InterestMask.AsULong = 0;
            InterestMask.CreateProcess = TRUE;
            InterestMask.SessionStatus = TRUE;
            InterestMask.ChangeDebuggeeState = TRUE;
            InterestMask.ChangeEngineState = TRUE;
        }

        //
        // DebugEngineSetEventCallbacks() will attempt to acquire the engine
        // lock, so release it now first.
        //

        ReleaseDebugEngineLock(Engine);
        AcquiredLock = FALSE;

        CHECKED_MSG(
            DebugEngineSetEventCallbacks(
                Engine,
                &DebugEventCallbacks,
                &IID_IDEBUG_EVENT_CALLBACKS,
                InterestMask
            ),
            "DebugEngineSetEventCallbacks()"
        );

        //
        // Re-acquire the lock.
        //

        AcquireDebugEngineLock(Engine);
        AcquiredLock = TRUE;

        Engine->State.EventCallbacksRegistered = TRUE;
        Engine->State.RegisteringEventCallbacks = FALSE;

        //
        // Create a new process based on the target command line extracted
        // from this process's command line.
        //

        Engine->State.CreatingProcess = TRUE;

        CreateProcessFlags = (
            DEBUG_PROCESS |
            DEBUG_ECREATE_PROCESS_INHERIT_HANDLES
        );

        CHECKED_HRESULT_MSG(
            Client->CreateProcess(
                IClient,
                0,
                (PSTR)Session->TargetCommandLineA,
                CreateProcessFlags
            ),
            "DebugEngine: Client->CreateProcess"
        );

        Engine->State.CreatingProcess = FALSE;
        Engine->State.ProcessCreated = TRUE;

        //
        // Initialize all of the injection modules and allow them to connect
        // to the session.
        //

        ReadyEvent = Rtl->CreateEventW(NULL, FALSE, FALSE, NULL);
        if (!ReadyEvent || ReadyEvent == INVALID_HANDLE_VALUE) {
            goto Error;
        }

        Session->AllInjectionInitializersReady = ReadyEvent;
        Session->OutstandingInjectionInitializers = (
            InjectionModules->NumberOfModules
        );

        for (Index = 0; Index < InjectionModules->NumberOfModules; Index++) {

            Path = InjectionModules->Paths[Index];
            Module = InjectionModules->Modules[Index];
            Initializer = InjectionModules->Initializers[Index];

            Success = Initializer(Session);
            if (!Success) {
                __debugbreak();
                goto Error;
            }
        }

        //
        // Wait for all child threads to finish initialization.
        //

        OutputDebugStringA("Parent debug engine session: waiting for "
                           "AllInjectionInitializersReady event.\n");

        WaitResult = Rtl->WaitForSingleObject(ReadyEvent, INFINITE);
        if (WaitResult != WAIT_OBJECT_0) {
            goto Error;
        }

        //
        // All initializer threads have finished, verify our count is as
        // expected.
        //

        Success = FALSE;
        AcquireChildrenLockShared(Session);
        Success = (
            Session->Children.NumberOfEntries ==
            InjectionModules->NumberOfModules
        );
        ReleaseChildrenLockShared(Session);

        if (!Success) {
            __debugbreak();
            goto Error;
        }

        //
        // The debugger engine doesn't complete its connection until we call
        // WaitForEvent().  Do that now.
        //

        Result = Engine->Control->WaitForEvent(Engine->IControl, 0, 0);

        if (Result != S_OK && Result != S_FALSE) {
            OutputDebugStringA("Failed: Control->WaitForEvent().\n");
            goto Error;
        }

        //
        // Get the execution status.
        //

        CHECKED_HRESULT_MSG(
            Engine->Control->GetExecutionStatus(
                Engine->IControl,
                &ExecutionStatus
            ),
            "Control->GetExecutionStatus()"
        );

    }

    //
    // If a debug settings XML path has been indicated, try load it now.
    //

    DebuggerSettingsXmlPath = &TracerConfig->Paths.DebuggerSettingsXmlPath;
    if (IsValidMinimumDirectoryUnicodeString(DebuggerSettingsXmlPath)) {

        //
        // We need to release the debug engine lock here as it's a SRWLOCK
        // and thus can't be acquired recursively, and loading settings will
        // result in DebugEngineExecuteCommand() being called, which will
        // attempt to acquire the lock.
        //

        ReleaseDebugEngineLock(Engine);
        AcquiredLock = FALSE;

        Success = DebugEngineLoadSettings(Session, DebuggerSettingsXmlPath);
        if (!Success) {
            goto Error;
        }

        //
        // Re-acquire the lock.
        //

        AcquireDebugEngineLock(Engine);
        AcquiredLock = TRUE;
    }

    Success = ConnectClient(Session->ExitDispatchEngine->IClient);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // If a server has been requested, start it now.
    //

    if (InitFlags.StartServer) {
        CHECKED_HRESULT_MSG(
            Engine->Client->StartServer(
                Engine->IClient,
                //"npipe:pipe=tracer_%d"
                "npipe:pipe=tracer"
            ),
            "Client->StartServer()"
        );
        Session->Flags.StartedServer = TRUE;
    }

    //
    // Update the caller's pointer.
    //

    *SessionPointer = Session;

    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (AcquiredLock) {
        ReleaseDebugEngineLock(Engine);
    }

    return Success;
}

_Use_decl_annotations_
BOOL
InitializeChildDebugEngineSession(
    PDEBUG_ENGINE_SESSION Parent,
    PCDEBUGEVENTCALLBACKS EventCallbacks,
    PCGUID EventInterfaceId,
    DEBUG_EVENT_CALLBACKS_INTEREST_MASK EventInterestMask,
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
/*++

Routine Description:

    This routine initializes a child debug engine session from a parent.

Arguments:

    Parent - Supplies a pointer to an initialized DEBUG_ENGINE_SESSION structure
        that will be used to prime the child debug engine session.

    EventCallbacks - Supplies event callbacks.

    EventInterfaceId - Supplies the event callback interface ID.

    EventInterestMask - Supplies the interest mask.

    SessionPointer - Supplies an address to a variable that will receive the
        address of the newly allocated and initialized DEBUG_ENGINE_SESSION
        structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    PRTL Rtl;
    PALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PDEBUG_ENGINE_SESSION Session;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE ExitDispatchEngine;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Parent)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SessionPointer)) {
        return FALSE;
    }

    if (EventInterestMask.AsULong != 0) {

        if (!ARGUMENT_PRESENT(EventCallbacks)) {
            return FALSE;
        }

        if (!ARGUMENT_PRESENT(EventInterfaceId)) {
            return FALSE;
        }
    }

    //
    // Clear the caller's pointer up-front.
    //

    *SessionPointer = NULL;

    //
    // Initialize aliases.
    //

    Rtl = Parent->Rtl;
    Allocator = Parent->Allocator;
    TracerConfig = Parent->TracerConfig;

    //
    // Initialize common structure fields.
    //

    Success = InitializeDebugEngineSessionCommon(Rtl,
                                                 Allocator,
                                                 TracerConfig,
                                                 SessionPointer);

    if (!Success) {
        goto Error;
    }

    Session = *SessionPointer;

    Session->EventDispatch = Session->DispatchCallbacks;
    Engine = Session->Engine;
    ExitDispatchEngine = Session->ExitDispatchEngine;

    //
    // Create a debug engine client from the parent session's client.
    //

    CHECKED_MSG(
        InitializeChildDebugEngine(Rtl, Engine, Parent->Engine),
        "InitializeChildDebugEngine(Rtl, Engine, Parent->Engine)"
    );

    CHECKED_MSG(
        InitializeChildDebugEngine(Rtl, ExitDispatchEngine, Engine),
        "InitializeChildDebugEngine(Rtl, ExitDispatchEngine, Engine)"
    );

#define COPY_PARENT(Name) Session->##Name = Parent->##Name

    COPY_PARENT(InjectionModules);

    //
    // Copy string table glue from the parent.
    //

    COPY_PARENT(StringArrayAllocator);
    COPY_PARENT(StringTableAllocator);
    COPY_PARENT(ExamineSymbolsPrefixStringTable);
    COPY_PARENT(ExamineSymbolsBasicTypeStringTable1);
    COPY_PARENT(ExamineSymbolsBasicTypeStringTable2);
    COPY_PARENT(NumberOfBasicTypeStringTables);
    COPY_PARENT(FunctionArgumentTypeStringTable1);
    COPY_PARENT(FunctionArgumentTypeStringTable2);
    COPY_PARENT(FunctionArgumentVectorTypeStringTable1);

    //
    // Connect the client engines.
    //

    Success = ConnectClient(Session->Engine->IClient);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    Success = ConnectClient(Session->ExitDispatchEngine->IClient);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // Register event callbacks.
    //

    Engine->State.RegisteringEventCallbacks = TRUE;

    //
    // DebugEngineSetEventCallbacks() will attempt to acquire the engine
    // lock, so release it now first.
    //

    ReleaseDebugEngineLock(Engine);
    AcquiredLock = FALSE;

    CHECKED_MSG(
        DebugEngineSetEventCallbacks(
            Engine,
            EventCallbacks,
            EventInterfaceId,
            EventInterestMask
        ),
        "InitializeChildDebugEngineSession()->DebugEngineSetEventCallbacks()"
    );

    //
    // Re-acquire the lock.
    //

    AcquireDebugEngineLock(Engine);
    AcquiredLock = TRUE;

    Engine->State.EventCallbacksRegistered = TRUE;
    Engine->State.RegisteringEventCallbacks = FALSE;

    //
    // We're done, indicate success and return.
    //

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Need to destroy the COM interface then free pointer.
    //

    __debugbreak();

    if (Session) {
        Allocator->FreePointer(Allocator->Context, &Session);
    }

    AcquiredLock = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Release the debug engine lock if we acquired it.
    //

    if (AcquiredLock) {
        ReleaseDebugEngineLock(Engine);
    }

    //
    // Update the caller's pointer.
    //

    *SessionPointer = Session;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
