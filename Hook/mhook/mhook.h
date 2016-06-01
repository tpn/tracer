//Copyright (c) 2007-2008, Marton Anka
//
//Permission is hereby granted, free of charge, to any person obtaining a
//copy of this software and associated documentation files (the "Software"),
//to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the
//Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included
//in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#ifdef _M_IX86
#define _M_IX86_X64
#elif defined _M_X64
#define _M_IX86_X64
#endif

#include "../../Rtl/Rtl.h"
#include "../disasm/disasm.h"

/*
#define memmove(dest, src, length) Rtl->RtlMoveMemory(dest, src, length)
#define malloc(bytes) HeapAlloc(Rtl->HeapHandle, HEAP_ZERO_MEMORY, bytes)
#define free(ptr) HeapFree(Rtl->HeapHandle, 0, ptr)
*/

BOOL Mhook_SetHook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key);
BOOL Mhook_Unhook(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key);

//=========================================================================
#ifndef cntof
#define cntof(a) (sizeof(a)/sizeof(a[0]))
#endif

//=========================================================================
#ifndef GOOD_HANDLE
#define GOOD_HANDLE(a) ((a!=INVALID_HANDLE_VALUE)&&(a!=NULL))
#endif

//=========================================================================
#ifndef gle
#define gle GetLastError
#endif

//=========================================================================
#ifndef ODPRINTF

#ifdef _DEBUG
//#define ODPRINTF(a) odprintf a
#define ODPRINTF(a) odprintf a
#else
#define ODPRINTF(a)
#endif

FORCEINLINE
void
__cdecl
odprintf(PCWSTR format, ...)
{
    OutputDebugStringW(format);
}

/*
inline void __cdecl odprintf(PCSTR format, ...) {
    va_list args;
    va_start(args, format);
    int len = _vscprintf(format, args);
    if (len > 0) {
        len += (1 + 2);
        PSTR buf = (PSTR) malloc(len);
        if (buf) {
            len = vsprintf_s(buf, len, format, args);
            if (len > 0) {
                while (len && isspace(buf[len-1])) len--;
                buf[len++] = '\r';
                buf[len++] = '\n';
                buf[len] = 0;
                OutputDebugStringA(buf);
            }
            free(buf);
        }
        va_end(args);
    }
}

inline void __cdecl odprintfw(PCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    int len = _vscwprintf(format, args);
    if (len > 0) {
        len += (1 + 2);
        PWSTR buf = (PWSTR) malloc(sizeof(WCHAR)*len);
        if (buf) {
            len = vswprintf_s(buf, len, format, args);
            if (len > 0) {
                while (len && iswspace(buf[len-1])) len--;
                buf[len++] = L'\r';
                buf[len++] = L'\n';
                buf[len] = 0;
                OutputDebugStringW(buf);
            }
            free(buf);
        }
        va_end(args);
    }
}
*/

#endif //#ifndef ODPRINTF

//=========================================================================
#define MHOOKS_MAX_CODE_BYTES   32
#define MHOOKS_MAX_RIPS          4

typedef struct _MHOOKS_TRAMPOLINE MHOOKS_TRAMPOLINE;
typedef MHOOKS_TRAMPOLINE *PMHOOKS_TRAMPOLINE;

//=========================================================================
// The trampoline structure - stores every bit of info about a hook
typedef struct _MHOOKS_TRAMPOLINE {
    PBYTE   pSystemFunction;                                // the original system function
    DWORD   cbOverwrittenCode;                              // number of bytes overwritten by the jump
    PBYTE   pHookFunction;                                  // the hook function that we provide
    BYTE    codeJumpToHookFunction[MHOOKS_MAX_CODE_BYTES];  // placeholder for code that jumps to the hook function
    BYTE    codeTrampoline[MHOOKS_MAX_CODE_BYTES];          // placeholder for code that holds the first few
                                                            //   bytes from the system function and a jump to the remainder
                                                            //   in the original location
    BYTE    codeUntouched[MHOOKS_MAX_CODE_BYTES];           // placeholder for unmodified original code
                                                            //   (we patch IP-relative addressing)
    PMHOOKS_TRAMPOLINE pPrevTrampoline;                     // When in the free list, thess are pointers to the prev and next entry.
    PMHOOKS_TRAMPOLINE pNextTrampoline;                     // When not in the free list, this is a pointer to the prev and next trampoline in use.
} MHOOKS_TRAMPOLINE, *PMHOOKS_TRAMPOLINE, **PPMHOOKS_TRAMPOLINE;

//=========================================================================
// The patch data structures - store info about rip-relative instructions
// during hook placement
typedef struct _MHOOKS_RIPINFO
{
    USHORT  InstructionLength;
    USHORT  DisplacementLength;
    USHORT  DisplacementOffset;
    USHORT  OffsetOfRipRelativeInstructionFromFunctionAddress;
    USHORT  OffsetOfDisp32AddressFromFunctionAddress;
    USHORT  Unused1;
    LONG    dwOffset;
    S64     nDisplacement;
    LONG    DisplacementValue;
    LONG    NewDisplacementValue;
    PCHAR   FunctionAddress;
    PCHAR   AddressOfRipRelativeInstruction;
    PCHAR   AddressOfNextInstruction;
    PCHAR   EffectiveRipRelativeAddress;
    PCHAR   AddressOfDisp32Operand;
} MHOOKS_RIPINFO, *PMHOOKS_RIPINFO;

typedef struct _MHOOKS_PATCHDATA
{
    S64             nLimitUp;
    S64             nLimitDown;
    DWORD           nRipCnt;
    MHOOKS_RIPINFO  rips[MHOOKS_MAX_RIPS];
} MHOOKS_PATCHDATA, *PMHOOKS_PATCHDATA;
