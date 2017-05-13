/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TracerConfigPrivate.h

Abstract:

    This is the private header file for the TracerConfig component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define private function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_ALL_TRACER_MODULES)(
    _In_ PTRACER_CONFIG TracerConfig
    );
typedef LOAD_ALL_TRACER_MODULES *PLOAD_ALL_TRACER_MODULES;
LOAD_ALL_TRACER_MODULES LoadAllTracerModules;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_TRACER_PATH)(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ TRACER_PATH_TYPE PathType,
    _In_ USHORT Index
    );
typedef LOAD_TRACER_PATH *PLOAD_TRACER_PATH;
LOAD_TRACER_PATH LoadTracerPath;

//
// Define helper macros.
//

/*++

    VOID
    READ_REG_DWORD_FLAG(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a local TRACER_FLAGS Flags structure.  If the registry key isn't
    present, an attempt will be made to write the default value.

Arguments:

    Name - Name of the flag to read (e.g. LoadDebugLibraries).  The macro
        resolves this to Flags.Name (e.g. Flags.LoadDebugLibraries).

    Default - Default value to assign to the flag (Flags.Name) if the
        registry key couldn't be read successfully (because it was not
        present, or was an incorrect type).

Return Value:

    None.

--*/
#define READ_REG_DWORD_FLAG(Name, Default) do { \
    ULONG Value;                                \
    ULONG ValueLength = sizeof(Value);          \
    Result = RegGetValueW(                      \
        RegistryKey,                            \
        NULL,                                   \
        L#Name,                                 \
        RRF_RT_REG_DWORD,                       \
        NULL,                                   \
        (PVOID)&Value,                          \
        &ValueLength                            \
    );                                          \
    if (Result == ERROR_SUCCESS) {              \
        Flags.Name = Value;                     \
    } else {                                    \
        Value = Flags.Name = Default;           \
        RegSetValueExW(                         \
            RegistryKey,                        \
            L#Name,                             \
            0,                                  \
            REG_DWORD,                          \
            (const BYTE*)&Value,                \
            ValueLength                         \
        );                                      \
    }                                           \
} while (0)

/*++

    VOID
    READ_REG_DWORD(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into the TRACER_CONFIG structure.  If the registry key isn't present,
    an attempt will be made to write the default value.

Arguments:

    Name - Name of the flag to read (e.g. LoadDebugLibraries).  The macro
        resolves this to Flags.Name (e.g. Flags.LoadDebugLibraries).

    Default - Default value to assign to the field if the registry key couldn't
        be read successfully (because it was not present, or was an incorrect
        type).

Return Value:

    None.

--*/

#ifdef READ_REG_DWORD
#undef READ_REG_DWORD
#endif

#define READ_REG_DWORD(Name, Default) do {      \
    ULONG Value;                                \
    ULONG ValueLength = sizeof(Value);          \
    Result = RegGetValueW(                      \
        RegistryKey,                            \
        NULL,                                   \
        L#Name,                                 \
        RRF_RT_REG_DWORD,                       \
        NULL,                                   \
        (PVOID)&Value,                          \
        &ValueLength                            \
    );                                          \
    if (Result == ERROR_SUCCESS) {              \
        TracerConfig->##Name = Value;           \
    } else {                                    \
        TracerConfig->##Name = Value = Default; \
        RegSetValueExW(                         \
            RegistryKey,                        \
            L#Name,                             \
            0,                                  \
            REG_DWORD,                          \
            (const BYTE*)&Value,                \
            ValueLength                         \
        );                                      \
    }                                           \
} while (0)


/*++

    VOID
    READ_REG_DWORD_RUNTIME_PARAM(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a TRACER_RUNTIME_PARAMETERS structure.  If the registry key isn't
    present, an attempt will be made to write the default value.

Arguments:

    Name - Name of the parameter to read.

    Default - Default value to assign to the parameter if the registry key
        couldn't be read successfully (because it was not present, or was an
        incorrect type).

Return Value:

    None.

--*/
#define READ_REG_DWORD_RUNTIME_PARAM(Name, Default) do {        \
    ULONG Value;                                                \
    ULONG ValueLength = sizeof(Value);                          \
    Result = RegGetValueW(                                      \
        RegistryKey,                                            \
        NULL,                                                   \
        L#Name,                                                 \
        RRF_RT_REG_DWORD,                                       \
        NULL,                                                   \
        (PVOID)&Value,                                          \
        &ValueLength                                            \
    );                                                          \
    if (Result == ERROR_SUCCESS) {                              \
        TracerConfig->RuntimeParameters.Name = Value;           \
    } else {                                                    \
        Value = TracerConfig->RuntimeParameters.Name = Default; \
        RegSetValueExW(                                         \
            RegistryKey,                                        \
            L#Name,                                             \
            0,                                                  \
            REG_DWORD,                                          \
            (const BYTE*)&Value,                                \
            ValueLength                                         \
        );                                                      \
    }                                                           \
} while (0)

//
// The minimum number of bytes for a valid WCHAR path name, including
// the terminating NULL, is 10:
//      length of L"C:\a" (4 * 2 = 8) + NUL (1 * 2) = 10
//

#define MINIMUM_PATH_SIZE_IN_BYTES 10

//
// Set maximum path size at (a somewhat arbitrary) 2048 bytes.
//

#define MAXIMUM_PATH_SIZE_IN_BYTES 2048

/*++

    VOID
    READ_REG_SZ_PATH(
        Name,
        Optional
        )

Routine Description:

    Helper macro for reading REG_SZ path values.  The size of the string
    is obtained first, then an attempt is made to allocate a sufficiently-
    sized buffer using the TracerConfig's Allocator.  If this succeeds,
    a second call to RegGetValueW() is called with the new buffer, and
    the rest of the UNICODE_STRING is initialized (i.e. MaximumLength and
    Length are set).

    If any errors occur, the Error handler is jumped to.

    This macro is intended to be called from within the body of the
    InitializeTracerConfig() routine.

Arguments:

    Name - Name of the path in the TRACER_PATHS structure.

    Optional - If TRUE, silently ignore the condition where the key can't be
        read from the registry.

Return Value:

    None.

--*/
#define READ_REG_SZ_PATH(Name, Optional) do {                \
    BOOL IsValid;                                            \
    ULONG SizeInBytes = 0;                                   \
    PUNICODE_STRING String = &Paths->Name;                   \
                                                             \
    Result = RegGetValueW(                                   \
        RegistryKey,                                         \
        NULL,                                                \
        L#Name,                                              \
        RRF_RT_REG_SZ,                                       \
        NULL,                                                \
        NULL,                                                \
        &SizeInBytes                                         \
    );                                                       \
                                                             \
    IsValid = (                                              \
        Result == ERROR_SUCCESS &&                           \
        SizeInBytes >= MINIMUM_PATH_SIZE_IN_BYTES &&         \
        SizeInBytes <= MAXIMUM_PATH_SIZE_IN_BYTES            \
    );                                                       \
                                                             \
    if (!IsValid) {                                          \
        if (Optional) {                                      \
            String->Length = 0;                              \
            String->MaximumLength = 0;                       \
            String->Buffer = NULL;                           \
            break;                                           \
        } else {                                             \
            goto Error;                                      \
        }                                                    \
    }                                                        \
                                                             \
    String->Buffer = (PWCHAR)(                               \
        Allocator->Malloc(                                   \
            Allocator->Context,                              \
            SizeInBytes                                      \
        )                                                    \
    );                                                       \
    if (!String->Buffer) {                                   \
        goto Error;                                          \
    }                                                        \
                                                             \
    Result = RegGetValueW(                                   \
        RegistryKey,                                         \
        NULL,                                                \
        L#Name,                                              \
        RRF_RT_REG_SZ,                                       \
        NULL,                                                \
        (PVOID)String->Buffer,                               \
        &SizeInBytes                                         \
    );                                                       \
                                                             \
    if (Result != ERROR_SUCCESS) {                           \
        goto Error;                                          \
    }                                                        \
                                                             \
    String->Length =  (USHORT)(SizeInBytes - sizeof(WCHAR)); \
    String->MaximumLength = (USHORT)SizeInBytes;             \
                                                             \
} while (0)


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
