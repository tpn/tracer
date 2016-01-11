#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGE_ALIGN
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

#define MAX_USTRING (sizeof(WCHAR) * ((1 << (sizeof(USHORT) * 8)) / sizeof(WCHAR)))

typedef union _ULONG_INTEGER {
    struct {
        USHORT  LowPart;
        USHORT  HighPart;
    };
    ULONG   LongPart;

} ULONG_INTEGER, *PULONG_INTEGER;

typedef union _LONG_INTEGER {
    struct {
        USHORT  LowPart;
        SHORT   HighPart;
    };
    LONG   LongPart;
} LONG_INTEGER, *PLONG_INTEGER;


typedef CHAR *PSZ;
typedef const CHAR *PCSZ;

typedef VOID (WINAPI *PGETSYSTEMTIMEPRECISEASFILETIME)(_Out_ LPFILETIME lpSystemTimeAsFileTime);
typedef LONG (WINAPI *PNTQUERYSYSTEMTIME)(_Out_ PLARGE_INTEGER SystemTime);

typedef BOOL (*PGETSYSTEMTIMEPRECISEASLARGEINTEGER)(
    _Out_   PLARGE_INTEGER  SystemTime
);

#define _SYSTEM_TIMER_FUNCTIONS_HEAD                                    \
    PGETSYSTEMTIMEPRECISEASFILETIME GetSystemTimePreciseAsFileTime;     \
    PNTQUERYSYSTEMTIME NtQuerySystemTime;

typedef struct _SYSTEM_TIMER_FUNCTION {
    _SYSTEM_TIMER_FUNCTIONS_HEAD
} SYSTEM_TIMER_FUNCTION, *PSYSTEM_TIMER_FUNCTION, **PPSYSTEM_TIMER_FUNCTION;

BOOL
CallSystemTimer(
    _Out_       PFILETIME               SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
);

typedef NTSTATUS (WINAPI *PRTLCHARTOINTEGER)(_In_ PCSZ String, _In_opt_ ULONG Base, _Out_ PULONG Value);

typedef struct _RTL {
    HMODULE     NtdllModule;
    HMODULE     Kernel32Module;

    union {
        SYSTEM_TIMER_FUNCTION   SystemTimerFunction;
        struct {
            _SYSTEM_TIMER_FUNCTIONS_HEAD
        };
    };

} RTL, *PRTL, **PPRTL;

#define RtlOffsetToPointer(B,O)    ((PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))
#define RtlOffsetFromPointer(B,O)  ((PCHAR)( ((PCHAR)(B)) - ((ULONG_PTR)(O))  ))
#define RtlPointerToOffset(B,P)    ((ULONG)( ((PCHAR)(P)) - ((PCHAR)(B))      ))

#ifdef __cpp
} // extern "C"
#endif
