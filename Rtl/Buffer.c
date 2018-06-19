/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Buffer.c

Abstract:

    Work-in-progress of a COM-style generic character buffer interface.

--*/

#include "stdafx.h"

CGUID CLSID_BUFFER;
CGUID IID_IUNKNOWN;
CGUID IID_CHAR_BUFFER;

DEFINE_GUID_EX(IID_IUNKNOWN, 0x00000000, 0x0000, 0x0000,
               0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

//
// CLSID_BUFFER: {8B742F20-FD40-4d31-874E-3AE802EDE172}
//

DEFINE_GUID_EX(CLSID_BUFFER, 0x8b742f20, 0xfd40, 0x4d31,
               0x87, 0x4e, 0x3a, 0xe8, 0x2, 0xed, 0xe1, 0x72);

//
// IID_CHAR_BUFFER: {E1E779DE-52B7-4dba-99F8-E20328805891}
//

DEFINE_GUID_EX(IID_CHAR_BUFFER, 0xe1e779de, 0x52b7, 0x4dba,
               0x99, 0xf8, 0xe2, 0x3, 0x28, 0x80, 0x58, 0x91);

struct _CHAR_BUFFER {
    struct _CHAR_BUFFER_VTBL *Vtbl;
    PRTL Rtl;
    PALLOCATOR Allocator;
    HANDLE Handle;
    PCHAR Buffer;
    PCHAR BaseAddress;
    ULONGLONG SizeInBytes;
    VSPRINTF_S vsprintf_s;
    volatile ULONGLONG ReferenceCount;
} CHAR_BUFFER;
struct CHAR_BUFFER *PCHAR_BUFFER;

typedef
HRESULT
(NTAPI CHAR_BUFFER_QUERY_INTERFACE)(
    _In_ PCHAR_BUFFER Buffer,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef CHAR_BUFFER_QUERY_INTERFACE *PCHAR_BUFFER_QUERY_INTERFACE;

typedef
ULONG
(NTAPI CHAR_BUFFER_ADD_REF)(
    _In_ PCHAR_BUFFER Buffer
    );
typedef CHAR_BUFFER_ADD_REF *PCHAR_BUFFER_ADD_REF;

typedef
ULONG
(NTAPI CHAR_BUFFER_RELEASE)(
    _In_ PCHAR_BUFFER Buffer
    );
typedef CHAR_BUFFER_RELEASE *PCHAR_BUFFER_RELEASE;

typedef
HRESULT
(NTAPI CHAR_BUFFER_INITIALIZE)(
    _In_ PCHAR_BUFFER Buffer,
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ ULONG NumberOfPages,
    _In_ HANDLE OutputHandle,
    _In_opt_ PCHAR Separator
    );
typedef CHAR_BUFFER_INITIALIZE *PCHAR_BUFFER_INITIALIZE;

typedef struct _CHAR_BUFFER_VTBL {
    PCHAR_BUFFER_QUERY_INTERFACE QueryInterface;
    PCHAR_BUFFER_ADD_REF AddRef;
    PCHAR_BUFFER_RELEASE Release;
    PCHAR_BUFFER_INITIALIZE Initialize;
#if 0
    PCHAR_BUFFER_FLUSH Flush;
    PCHAR_BUFFER_WRITE_INT WriteInteger;
    PCHAR_BUFFER_WRITE_SEP WriteSep;
    PCHAR_BUFFER_WRITE_CSTR WriteCStr;
    PCHAR_BUFFER_WRITE_STRING WriteString;
    PCHAR_BUFFER_WRITE_DOUBLE WriteDouble;
    PCHAR_BUFFER_WRITE_FORMAT WriteFormat;
#endif
} CHAR_BUFFER_VTBL;
typedef CHAR_BUFFER_VTBL *PCHAR_BUFFER_VTBL;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
