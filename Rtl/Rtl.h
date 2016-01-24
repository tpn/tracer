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

typedef NTSTATUS (WINAPI *PRTLCHARTOINTEGER)(
    _In_ PCSZ String,
    _In_opt_ ULONG Base,
    _Out_ PULONG Value
);

typedef EXCEPTION_DISPOSITION (__cdecl *PCSPECIFICHANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
);

#define _RTLFUNCTIONS_HEAD                                          \
    PRTLCHARTOINTEGER RtlCharToInteger;                             \
    PCSPECIFICHANDLER __C_specific_handler;

typedef struct _RTLFUNCTIONS {
    _RTLFUNCTIONS_HEAD
} RTLFUNCTIONS, *PRTLFUNCTIONS, **PPRTLFUNCTIONS;

typedef PVOID (*PCOPYTOMAPPEDMEMORY)(
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
);

#define _RTLEXFUNCTIONS_HEAD \
    PCOPYTOMAPPEDMEMORY CopyToMappedMemory;

typedef struct _RTLEXFUNCTIONS {
    _RTLEXFUNCTIONS_HEAD
} RTLEXFUNCTIONS, *PRTLEXFUNCTIONS, **PPRTLEXFUNCTIONS;

typedef struct _RTL {
    ULONG       Size;
    HMODULE     NtdllModule;
    HMODULE     Kernel32Module;

    union {
        SYSTEM_TIMER_FUNCTION   SystemTimerFunction;
        struct {
            _SYSTEM_TIMER_FUNCTIONS_HEAD
        };
    };

    union {
        RTLFUNCTIONS RtlFunctions;
        struct {
            _RTLFUNCTIONS_HEAD
        };
    };

    union {
        RTLEXFUNCTIONS RtlExFunctions;
        struct {
            _RTLEXFUNCTIONS_HEAD
        };
    };

} RTL, *PRTL, **PPRTL;

#define RtlOffsetToPointer(B,O)    ((PCHAR)(     ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))
#define RtlOffsetFromPointer(B,O)  ((PCHAR)(     ((PCHAR)(B)) - ((ULONG_PTR)(O))  ))
#define RtlPointerToOffset(B,P)    ((ULONG_PTR)( ((PCHAR)(P)) - ((PCHAR)(B))      ))

#define PrefaultPage(Address) (*(volatile *)(PCHAR)(Address))
#define PrefaultNextPage(Address) (*(volatile *)(PCHAR)((ULONG_PTR)Address + PAGE_SIZE))

#define ALIGN_DOWN(Address, Alignment) ((ULONG_PTR)(Address) & ~((ULONG_PTR)(Alignment)-1))
#define ALIGN_UP(Address, Alignment) (ALIGN_DOWN((Address) + (Alignment) - 1), (Alignment))

#define RTL_API __declspec(dllexport)

RTL_API
BOOL
InitializeRtl(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
);

RTL_API
PVOID
CopyToMappedMemory(
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
);

#ifdef __cpp
} // extern "C"
#endif
