/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    PythonDllFiles.h

Abstract:

    This module defines filename constants used by the Python component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DLL(Name, Filename) \
    static CONST UNICODE_STRING Name = RTL_CONSTANT_STRING(Filename);

#define EXE DLL
#define DIRECTORY DLL

//
// Python executable.
//

EXE(PythonExeW, L"python.exe");

//
// Python PATH directory suffixes (directories that are appended to PYTHONHOME)
// and typically added to the PATH environment variable as part of environment
// activation.
//

DIRECTORY(PythonScriptsDirectoryW,    L"\\Scripts");
//DIRECTORY(PythonLibraryBinDirectoryW, L"\\Library\\bin");

static CONST PUNICODE_STRING PythonPathSuffixesW[] = {
    (PUNICODE_STRING)&PythonScriptsDirectoryW //,
    //(PUNICODE_STRING)&PythonLibraryBinDirectoryW
};

#define NUMBER_OF_PYTHON_PATH_SUFFIXES ( \
    sizeof(PythonPathSuffixesW) /        \
    sizeof(PythonPathSuffixesW[0])       \
)

//
// Python DLLs.
//

DLL(Python27DllW, L"python27.dll");
DLL(Python30DllW, L"python30.dll");
DLL(Python31DllW, L"python31.dll");
DLL(Python32DllW, L"python32.dll");
DLL(Python33DllW, L"python33.dll");
DLL(Python34DllW, L"python34.dll");
DLL(Python35DllW, L"python35.dll");
DLL(Python36DllW, L"python36.dll");

//
// We index directly into the DLL character array using this next value, so be
// extra careful we define it correctly with a C_ASSERT() + sizeof().
//

#define PYTHON_MAJOR_VERSION_CHAR_OFFSET (sizeof("python")-1)
C_ASSERT(PYTHON_MAJOR_VERSION_CHAR_OFFSET == 6);

static CONST PUNICODE_STRING PythonDllFilesW[] = {
    (PUNICODE_STRING)&Python27DllW,
    (PUNICODE_STRING)&Python30DllW,
    (PUNICODE_STRING)&Python31DllW,
    (PUNICODE_STRING)&Python32DllW,
    (PUNICODE_STRING)&Python33DllW,
    (PUNICODE_STRING)&Python34DllW,
    (PUNICODE_STRING)&Python35DllW
};

#define NUMBER_OF_PYTHON_DLL_FILES ( \
    sizeof(PythonDllFilesW) /        \
    sizeof(PythonDllFilesW[0])       \
)

static CONST USHORT NumberOfPythonDllFiles = NUMBER_OF_PYTHON_DLL_FILES;

typedef struct _Struct_size_bytes_(Size) _DLL_FILES {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_PATHS)) USHORT Size;

    //
    // Number of elements.
    //

    USHORT NumberOfFiles;

    PPUNICODE_STRING Files;

} DLL_FILES;
typedef DLL_FILES *PDLL_FILES;

static CONST DLL_FILES PythonDllFiles = {
    sizeof(DLL_FILES),
    NUMBER_OF_PYTHON_DLL_FILES,
    (PPUNICODE_STRING)PythonDllFilesW
};

//
// The following DLL strings aren't being used.
//

#if 0

//
// MSVC Runtimes.
//

DLL(Msvcr90DllW, L"msvcr90.dll");
DLL(Msvcr100DllW, L"msvcr100.dll");
DLL(Msvcr110DllW, L"msvcr110.dll");
DLL(Msvcr120DllW, L"msvcr120.dll");
DLL(Vcruntime140DllW, L"vcruntime140.dll");

static CONST PUNICODE_STRING MsvcRuntimeDllsW[] = {
    (PUNICODE_STRING)&Msvcr90DllW,
    (PUNICODE_STRING)&Msvcr100DllW,
    (PUNICODE_STRING)&Msvcr110DllW,
    (PUNICODE_STRING)&Msvcr120DllW,
    (PUNICODE_STRING)&Vcruntime140DllW
};

//
// API Sets.
//

DLL(ApiMsWinBaseUtilL110DllW, L"api-ms-win-base-util-l1-1-0.dll");
DLL(ApiMsWinCoreComL110DllW, L"api-ms-win-core-com-l1-1-0.dll");
DLL(ApiMsWinCoreCommL110DllW, L"api-ms-win-core-comm-l1-1-0.dll");
DLL(ApiMsWinCoreConsoleL110DllW, L"api-ms-win-core-console-l1-1-0.dll");
DLL(ApiMsWinCoreDatetimeL110DllW, L"api-ms-win-core-datetime-l1-1-0.dll");
DLL(ApiMsWinCoreDatetimeL111DllW, L"api-ms-win-core-datetime-l1-1-1.dll");
DLL(ApiMsWinCoreDebugL110DllW, L"api-ms-win-core-debug-l1-1-0.dll");
DLL(ApiMsWinCoreDebugL111DllW, L"api-ms-win-core-debug-l1-1-1.dll");
DLL(ApiMsWinCoreDelayloadL110DllW, L"api-ms-win-core-delayload-l1-1-0.dll");
DLL(ApiMsWinCoreErrorhandlingL110DllW, L"api-ms-win-core-errorhandling-l1-1-0.dll");
DLL(ApiMsWinCoreErrorhandlingL111DllW, L"api-ms-win-core-errorhandling-l1-1-1.dll");
DLL(ApiMsWinCoreFibersL110DllW, L"api-ms-win-core-fibers-l1-1-0.dll");
DLL(ApiMsWinCoreFibersL111DllW, L"api-ms-win-core-fibers-l1-1-1.dll");
DLL(ApiMsWinCoreFileL110DllW, L"api-ms-win-core-file-l1-1-0.dll");
DLL(ApiMsWinCoreFileL120DllW, L"api-ms-win-core-file-l1-2-0.dll");
DLL(ApiMsWinCoreFileL121DllW, L"api-ms-win-core-file-l1-2-1.dll");
DLL(ApiMsWinCoreFileL210DllW, L"api-ms-win-core-file-l2-1-0.dll");
DLL(ApiMsWinCoreFileL211DllW, L"api-ms-win-core-file-l2-1-1.dll");
DLL(ApiMsWinCoreHandleL110DllW, L"api-ms-win-core-handle-l1-1-0.dll");
DLL(ApiMsWinCoreHeapL110DllW, L"api-ms-win-core-heap-l1-1-0.dll");
DLL(ApiMsWinCoreHeapObsoleteL110DllW, L"api-ms-win-core-heap-obsolete-l1-1-0.dll");
DLL(ApiMsWinCoreInterlockedL110DllW, L"api-ms-win-core-interlocked-l1-1-0.dll");
DLL(ApiMsWinCoreIoL110DllW, L"api-ms-win-core-io-l1-1-0.dll");
DLL(ApiMsWinCoreIoL111DllW, L"api-ms-win-core-io-l1-1-1.dll");
DLL(ApiMsWinCoreKernel32LegacyL110DllW, L"api-ms-win-core-kernel32-legacy-l1-1-0.dll");
DLL(ApiMsWinCoreKernel32LegacyL111DllW, L"api-ms-win-core-kernel32-legacy-l1-1-1.dll");
DLL(ApiMsWinCoreKernel32PrivateL110DllW, L"api-ms-win-core-kernel32-private-l1-1-0.dll");
DLL(ApiMsWinCoreKernel32PrivateL111DllW, L"api-ms-win-core-kernel32-private-l1-1-1.dll");
DLL(ApiMsWinCoreLibraryloaderL110DllW, L"api-ms-win-core-libraryloader-l1-1-0.dll");
DLL(ApiMsWinCoreLibraryloaderL111DllW, L"api-ms-win-core-libraryloader-l1-1-1.dll");
DLL(ApiMsWinCoreLocalizationL120DllW, L"api-ms-win-core-localization-l1-2-0.dll");
DLL(ApiMsWinCoreLocalizationL121DllW, L"api-ms-win-core-localization-l1-2-1.dll");
DLL(ApiMsWinCoreLocalizationObsoleteL120DllW, L"api-ms-win-core-localization-obsolete-l1-2-0.dll");
DLL(ApiMsWinCoreMemoryL110DllW, L"api-ms-win-core-memory-l1-1-0.dll");
DLL(ApiMsWinCoreMemoryL111DllW, L"api-ms-win-core-memory-l1-1-1.dll");
DLL(ApiMsWinCoreMemoryL112DllW, L"api-ms-win-core-memory-l1-1-2.dll");
DLL(ApiMsWinCoreNamedpipeL110DllW, L"api-ms-win-core-namedpipe-l1-1-0.dll");
DLL(ApiMsWinCorePrivateprofileL110DllW, L"api-ms-win-core-privateprofile-l1-1-0.dll");
DLL(ApiMsWinCorePrivateprofileL111DllW, L"api-ms-win-core-privateprofile-l1-1-1.dll");
DLL(ApiMsWinCoreProcessenvironmentL110DllW, L"api-ms-win-core-processenvironment-l1-1-0.dll");
DLL(ApiMsWinCoreProcessenvironmentL120DllW, L"api-ms-win-core-processenvironment-l1-2-0.dll");
DLL(ApiMsWinCoreProcessthreadsL110DllW, L"api-ms-win-core-processthreads-l1-1-0.dll");
DLL(ApiMsWinCoreProcessthreadsL111DllW, L"api-ms-win-core-processthreads-l1-1-1.dll");
DLL(ApiMsWinCoreProcessthreadsL112DllW, L"api-ms-win-core-processthreads-l1-1-2.dll");
DLL(ApiMsWinCoreProcesstopologyObsoleteL110DllW, L"api-ms-win-core-processtopology-obsolete-l1-1-0.dll");
DLL(ApiMsWinCoreProfileL110DllW, L"api-ms-win-core-profile-l1-1-0.dll");
DLL(ApiMsWinCoreRealtimeL110DllW, L"api-ms-win-core-realtime-l1-1-0.dll");
DLL(ApiMsWinCoreRegistryL110DllW, L"api-ms-win-core-registry-l1-1-0.dll");
DLL(ApiMsWinCoreRegistryL210DllW, L"api-ms-win-core-registry-l2-1-0.dll");
DLL(ApiMsWinCoreRtlsupportL110DllW, L"api-ms-win-core-rtlsupport-l1-1-0.dll");
DLL(ApiMsWinCoreShlwapiLegacyL110DllW, L"api-ms-win-core-shlwapi-legacy-l1-1-0.dll");
DLL(ApiMsWinCoreShlwapiObsoleteL110DllW, L"api-ms-win-core-shlwapi-obsolete-l1-1-0.dll");
DLL(ApiMsWinCoreShutdownL110DllW, L"api-ms-win-core-shutdown-l1-1-0.dll");
DLL(ApiMsWinCoreStringansiL110DllW, L"api-ms-win-core-stringansi-l1-1-0.dll");
DLL(ApiMsWinCoreStringL110DllW, L"api-ms-win-core-string-l1-1-0.dll");
DLL(ApiMsWinCoreStringL210DllW, L"api-ms-win-core-string-l2-1-0.dll");
DLL(ApiMsWinCoreStringloaderL111DllW, L"api-ms-win-core-stringloader-l1-1-1.dll");
DLL(ApiMsWinCoreStringObsoleteL110DllW, L"api-ms-win-core-string-obsolete-l1-1-0.dll");
DLL(ApiMsWinCoreSynchL110DllW, L"api-ms-win-core-synch-l1-1-0.dll");
DLL(ApiMsWinCoreSynchL120DllW, L"api-ms-win-core-synch-l1-2-0.dll");
DLL(ApiMsWinCoreSysinfoL110DllW, L"api-ms-win-core-sysinfo-l1-1-0.dll");
DLL(ApiMsWinCoreSysinfoL120DllW, L"api-ms-win-core-sysinfo-l1-2-0.dll");
DLL(ApiMsWinCoreSysinfoL121DllW, L"api-ms-win-core-sysinfo-l1-2-1.dll");
DLL(ApiMsWinCoreThreadpoolL120DllW, L"api-ms-win-core-threadpool-l1-2-0.dll");
DLL(ApiMsWinCoreThreadpoolLegacyL110DllW, L"api-ms-win-core-threadpool-legacy-l1-1-0.dll");
DLL(ApiMsWinCoreThreadpoolPrivateL110DllW, L"api-ms-win-core-threadpool-private-l1-1-0.dll");
DLL(ApiMsWinCoreTimezoneL110DllW, L"api-ms-win-core-timezone-l1-1-0.dll");
DLL(ApiMsWinCoreUrlL110DllW, L"api-ms-win-core-url-l1-1-0.dll");
DLL(ApiMsWinCoreUtilL110DllW, L"api-ms-win-core-util-l1-1-0.dll");
DLL(ApiMsWinCoreVersionL110DllW, L"api-ms-win-core-version-l1-1-0.dll");
DLL(ApiMsWinCoreWow64L110DllW, L"api-ms-win-core-wow64-l1-1-0.dll");
DLL(ApiMsWinCoreXstateL110DllW, L"api-ms-win-core-xstate-l1-1-0.dll");
DLL(ApiMsWinCoreXstateL210DllW, L"api-ms-win-core-xstate-l2-1-0.dll");
DLL(ApiMsWinDevicesConfigL110DllW, L"api-ms-win-devices-config-l1-1-0.dll");
DLL(ApiMsWinDevicesConfigL111DllW, L"api-ms-win-devices-config-l1-1-1.dll");
DLL(ApiMsWinEventingClassicproviderL110DllW, L"api-ms-win-eventing-classicprovider-l1-1-0.dll");
DLL(ApiMsWinEventingConsumerL110DllW, L"api-ms-win-eventing-consumer-l1-1-0.dll");
DLL(ApiMsWinEventingControllerL110DllW, L"api-ms-win-eventing-controller-l1-1-0.dll");
DLL(ApiMsWinEventingLegacyL110DllW, L"api-ms-win-eventing-legacy-l1-1-0.dll");
DLL(ApiMsWinEventingProviderL110DllW, L"api-ms-win-eventing-provider-l1-1-0.dll");
DLL(ApiMsWinEventlogLegacyL110DllW, L"api-ms-win-eventlog-legacy-l1-1-0.dll");
DLL(ApiMsWinSecurityBaseL110DllW, L"api-ms-win-security-base-l1-1-0.dll");
DLL(ApiMsWinSecurityCryptoapiL110DllW, L"api-ms-win-security-cryptoapi-l1-1-0.dll");
DLL(ApiMsWinSecurityLsalookupL210DllW, L"api-ms-win-security-lsalookup-l2-1-0.dll");
DLL(ApiMsWinSecurityLsalookupL211DllW, L"api-ms-win-security-lsalookup-l2-1-1.dll");
DLL(ApiMsWinSecurityLsapolicyL110DllW, L"api-ms-win-security-lsapolicy-l1-1-0.dll");
DLL(ApiMsWinSecurityProviderL110DllW, L"api-ms-win-security-provider-l1-1-0.dll");
DLL(ApiMsWinSecuritySddlL110DllW, L"api-ms-win-security-sddl-l1-1-0.dll");
DLL(ApiMsWinServiceCoreL110DllW, L"api-ms-win-service-core-l1-1-0.dll");
DLL(ApiMsWinServiceCoreL111DllW, L"api-ms-win-service-core-l1-1-1.dll");
DLL(ApiMsWinServiceManagementL110DllW, L"api-ms-win-service-management-l1-1-0.dll");
DLL(ApiMsWinServiceManagementL210DllW, L"api-ms-win-service-management-l2-1-0.dll");
DLL(ApiMsWinServicePrivateL110DllW, L"api-ms-win-service-private-l1-1-0.dll");
DLL(ApiMsWinServicePrivateL111DllW, L"api-ms-win-service-private-l1-1-1.dll");
DLL(ApiMsWinServiceWinsvcL110DllW, L"api-ms-win-service-winsvc-l1-1-0.dll");

static CONST PUNICODE_STRING ApiSetFilesW[] = {
    (PUNICODE_STRING)&ApiMsWinBaseUtilL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreComL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreCommL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreConsoleL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreDatetimeL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreDatetimeL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreDebugL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreDebugL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreDelayloadL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreErrorhandlingL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreErrorhandlingL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFibersL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFibersL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFileL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFileL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFileL121DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFileL210DllW,
    (PUNICODE_STRING)&ApiMsWinCoreFileL211DllW,
    (PUNICODE_STRING)&ApiMsWinCoreHandleL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreHeapL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreHeapObsoleteL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreInterlockedL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreIoL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreIoL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreKernel32LegacyL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreKernel32LegacyL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreKernel32PrivateL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreKernel32PrivateL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreLibraryloaderL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreLibraryloaderL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreLocalizationL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreLocalizationL121DllW,
    (PUNICODE_STRING)&ApiMsWinCoreLocalizationObsoleteL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreMemoryL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreMemoryL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreMemoryL112DllW,
    (PUNICODE_STRING)&ApiMsWinCoreNamedpipeL110DllW,
    (PUNICODE_STRING)&ApiMsWinCorePrivateprofileL110DllW,
    (PUNICODE_STRING)&ApiMsWinCorePrivateprofileL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcessenvironmentL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcessenvironmentL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcessthreadsL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcessthreadsL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcessthreadsL112DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProcesstopologyObsoleteL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreProfileL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreRealtimeL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreRegistryL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreRegistryL210DllW,
    (PUNICODE_STRING)&ApiMsWinCoreRtlsupportL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreShlwapiLegacyL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreShlwapiObsoleteL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreShutdownL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreStringansiL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreStringL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreStringL210DllW,
    (PUNICODE_STRING)&ApiMsWinCoreStringloaderL111DllW,
    (PUNICODE_STRING)&ApiMsWinCoreStringObsoleteL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreSynchL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreSynchL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreSysinfoL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreSysinfoL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreSysinfoL121DllW,
    (PUNICODE_STRING)&ApiMsWinCoreThreadpoolL120DllW,
    (PUNICODE_STRING)&ApiMsWinCoreThreadpoolLegacyL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreThreadpoolPrivateL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreTimezoneL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreUrlL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreUtilL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreVersionL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreWow64L110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreXstateL110DllW,
    (PUNICODE_STRING)&ApiMsWinCoreXstateL210DllW,
    (PUNICODE_STRING)&ApiMsWinDevicesConfigL110DllW,
    (PUNICODE_STRING)&ApiMsWinDevicesConfigL111DllW,
    (PUNICODE_STRING)&ApiMsWinEventingClassicproviderL110DllW,
    (PUNICODE_STRING)&ApiMsWinEventingConsumerL110DllW,
    (PUNICODE_STRING)&ApiMsWinEventingControllerL110DllW,
    (PUNICODE_STRING)&ApiMsWinEventingLegacyL110DllW,
    (PUNICODE_STRING)&ApiMsWinEventingProviderL110DllW,
    (PUNICODE_STRING)&ApiMsWinEventlogLegacyL110DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityBaseL110DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityCryptoapiL110DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityLsalookupL210DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityLsalookupL211DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityLsapolicyL110DllW,
    (PUNICODE_STRING)&ApiMsWinSecurityProviderL110DllW,
    (PUNICODE_STRING)&ApiMsWinSecuritySddlL110DllW,
    (PUNICODE_STRING)&ApiMsWinServiceCoreL110DllW,
    (PUNICODE_STRING)&ApiMsWinServiceCoreL111DllW,
    (PUNICODE_STRING)&ApiMsWinServiceManagementL110DllW,
    (PUNICODE_STRING)&ApiMsWinServiceManagementL210DllW,
    (PUNICODE_STRING)&ApiMsWinServicePrivateL110DllW,
    (PUNICODE_STRING)&ApiMsWinServicePrivateL111DllW,
    (PUNICODE_STRING)&ApiMsWinServiceWinsvcL110DllW
};

static CONST USHORT NumberOfApiSetFiles = (
    sizeof(ApiSetFilesW) /
    sizeof(ApiSetFilesW[0])
);

#endif

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
