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

#include "../stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

#pragma intrinsic(memset)

//#include "../disasm/disasm.h"
//#include "mhook.h"

//=========================================================================
// Global vars
static BOOL g_bVarsInitialized = FALSE;
static CRITICAL_SECTION g_cs;
static MHOOKS_TRAMPOLINE* g_pHooks = NULL;
static MHOOKS_TRAMPOLINE* g_pFreeList = NULL;
static DWORD g_nHooksInUse = 0;
static HANDLE* g_hThreadHandles = NULL;
static DWORD g_nThreadHandles = 0;
#define MHOOK_JMPSIZE 5
#define MHOOK_MINALLOCSIZE 4096

//=========================================================================
// Toolhelp defintions so the functions can be dynamically bound to
typedef HANDLE (WINAPI * _CreateToolhelp32Snapshot)(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

typedef BOOL (WINAPI * _Thread32First)(
                                       HANDLE hSnapshot,
                                       LPTHREADENTRY32 lpte
                                       );

typedef BOOL (WINAPI * _Thread32Next)(
                                      HANDLE hSnapshot,
                                      LPTHREADENTRY32 lpte
                                      );

//=========================================================================
// Bring in the toolhelp functions from kernel32
/*
_CreateToolhelp32Snapshot fnCreateToolhelp32Snapshot;
_Thread32First fnThread32First;
_Thread32Next fnThread32Next;
*/
#define fnCreateToolhelp32Snapshot(dwFlags, th32ProcessID) Rtl.CreateToolhelp32Snapshot(dwFlags, th32ProcessID)
#define fnThread32First(hSnapshot, lpte) Rtl.Thread32First(hSnapshot, lpte)
#define fnThread32Next(hSnapshot, lpte) Rtl.Thread32Next(hSnapshot, lpte)

//=========================================================================
// Internal function:
//
// Remove the trampoline from the specified list, updating the head pointer
// if necessary.
//=========================================================================
static VOID ListRemove(MHOOKS_TRAMPOLINE** pListHead, MHOOKS_TRAMPOLINE* pNode) {
    if (pNode->pPrevTrampoline) {
        pNode->pPrevTrampoline->pNextTrampoline = pNode->pNextTrampoline;
    }

    if (pNode->pNextTrampoline) {
        pNode->pNextTrampoline->pPrevTrampoline = pNode->pPrevTrampoline;
    }

    if ((*pListHead) == pNode) {
        (*pListHead) = pNode->pNextTrampoline;
        assert((*pListHead)->pPrevTrampoline == NULL);
    }

    pNode->pPrevTrampoline = NULL;
    pNode->pNextTrampoline = NULL;
}

//=========================================================================
// Internal function:
//
// Prepend the trampoline from the specified list and update the head pointer.
//=========================================================================
static VOID ListPrepend(MHOOKS_TRAMPOLINE** pListHead, MHOOKS_TRAMPOLINE* pNode) {
    pNode->pPrevTrampoline = NULL;
    pNode->pNextTrampoline = (*pListHead);
    if ((*pListHead)) {
        (*pListHead)->pPrevTrampoline = pNode;
    }
    (*pListHead) = pNode;
}

//=========================================================================
static VOID EnterCritSec() {
    if (!g_bVarsInitialized) {
        InitializeCriticalSection(&g_cs);
        g_bVarsInitialized = TRUE;
    }
    EnterCriticalSection(&g_cs);
}

//=========================================================================
static VOID LeaveCritSec() {
    LeaveCriticalSection(&g_cs);
}

//=========================================================================
// Internal function:
//
// Skip over jumps that lead to the real function. Gets around import
// jump tables, etc.
//=========================================================================
static PBYTE SkipJumps(PBYTE pbCode) {
    PBYTE pbOrgCode = pbCode;
#ifdef _M_IX86_X64
#ifdef _M_IX86
    //mov edi,edi: hot patch point
    if (pbCode[0] == 0x8b && pbCode[1] == 0xff)
        pbCode += 2;
    // push ebp; mov ebp, esp; pop ebp;
    // "collapsed" stackframe generated by MSVC
    if (pbCode[0] == 0x55 && pbCode[1] == 0x8b && pbCode[2] == 0xec && pbCode[3] == 0x5d)
        pbCode += 4;
#endif
    if (pbCode[0] == 0xff && pbCode[1] == 0x25) {
#ifdef _M_IX86
        // on x86 we have an absolute pointer...
        PBYTE pbTarget = *(PBYTE *)&pbCode[2];
        // ... that shows us an absolute pointer.
        return SkipJumps(*(PBYTE *)pbTarget);
#elif defined _M_X64
        // on x64 we have a 32-bit offset...
        INT32 lOffset = *(INT32 *)&pbCode[2];
        // ... that shows us an absolute pointer
        return SkipJumps(*(PBYTE*)(pbCode + 6 + lOffset));
    } else if (pbCode[0] == 0x48 && pbCode[1] == 0xff && pbCode[2] == 0x25) {
        // or we can have the same with a REX prefix
        INT32 lOffset = *(INT32 *)&pbCode[3];
        // ... that shows us an absolute pointer
        return SkipJumps(*(PBYTE*)(pbCode + 7 + lOffset));
#endif
    } else if (pbCode[0] == 0xe9) {
        // here the behavior is identical, we have...
        // ...a 32-bit offset to the destination.
        return SkipJumps(pbCode + 5 + *(INT32 *)&pbCode[1]);
    } else if (pbCode[0] == 0xeb) {
        // and finally an 8-bit offset to the destination
        return SkipJumps(pbCode + 2 + *(CHAR *)&pbCode[1]);
    }
#else
#error unsupported platform
#endif
    return pbOrgCode;
}

//=========================================================================
// Internal function:
//
// Writes code at pbCode that jumps to pbJumpTo. Will attempt to do this
// in as few bytes as possible. Important on x64 where the long jump
// (0xff 0x25 ....) can take up 14 bytes.
//=========================================================================
static PBYTE EmitJump(PBYTE pbCode, PBYTE pbJumpTo) {
#ifdef _M_IX86_X64
    PBYTE pbJumpFrom = pbCode + 5;
    SIZE_T cbDiff = pbJumpFrom > pbJumpTo ? pbJumpFrom - pbJumpTo : pbJumpTo - pbJumpFrom;
    ODPRINTF((L"mhooks: EmitJump: Jumping from %p to %p, diff is %p", pbJumpFrom, pbJumpTo, cbDiff));
    if (cbDiff <= 0x7fff0000) {
        pbCode[0] = 0xe9;
        pbCode += 1;
        *((PDWORD)pbCode) = (DWORD)(DWORD_PTR)(pbJumpTo - pbJumpFrom);
        pbCode += sizeof(DWORD);
    } else {
        pbCode[0] = 0xff;
        pbCode[1] = 0x25;
        pbCode += 2;
#ifdef _M_IX86
        // on x86 we write an absolute address (just behind the instruction)
        *((PDWORD)pbCode) = (DWORD)(DWORD_PTR)(pbCode + sizeof(DWORD));
#elif defined _M_X64
        // on x64 we write the relative address of the same location
        *((PDWORD)pbCode) = (DWORD)0;
#endif
        pbCode += sizeof(DWORD);
        *((PDWORD_PTR)pbCode) = (DWORD_PTR)(pbJumpTo);
        pbCode += sizeof(DWORD_PTR);
    }
#else
#error unsupported platform
#endif
    return pbCode;
}

static PBYTE EmitPushImm8(PBYTE pbCode, BYTE Key)
{
    *pbCode++ = 0x6a;
    *pbCode++ = Key;
    return pbCode;
}

static PBYTE EmitPushImm16(PBYTE pbCode, USHORT Key)
{
    *pbCode++ = 0x66; // 16-bit operand prefix
    *pbCode++ = 0x6a; // push imm16
    *((PUSHORT)pbCode)++ = Key;
    return pbCode;
}

static PBYTE EmitPushImm32(PBYTE pbCode, ULONG Key)
{
    *pbCode++ = 0x68;           // push
    *((PULONG)pbCode)++ = Key;  // imm32
    return pbCode;
}

static PBYTE EmitPushImm32x2(PBYTE pbCode, ULONGLONG Key)
{
    //
    // Push an 8 byte immediate value via two 4-byte pushes.
    // The low bytes are pushed first, then the high bytes.
    // This is done to facilitate the following:
    //      pop ecx         ; pop high bytes
    //      shl rcx, 32     ; shift into top 32-bits
    //      pop ecx         ; pop low bytes into bottom 32-bits
    //

    ULARGE_INTEGER Buf;
    Buf.QuadPart = (ULONGLONG)Key;
    pbCode = EmitPushImm32(pbCode, Buf.LowPart);
    // sub rsp, 8
    pbCode = EmitPushImm32(pbCode, Buf.HighPart);
    return pbCode;
}

static PBYTE EmitPushULongPtr(PBYTE pbCode, ULONG_PTR Key)
{
#ifndef _M_X64
    pbCode = EmitPushImm32(pbCode, Key);
#else
    return EmitPushImm32x2(pbCode, Key);
#endif
}

static PBYTE EmitMovRaxImm64(PBYTE pbCode, ULONGLONG Key)
{
    *pbCode++ = 0x48;   // rex.w
    *pbCode++ = 0xb8;   // mov rax,
    *((PULONGLONG)pbCode)++ = (ULONGLONG)Key;
    return pbCode;
}

static PBYTE EmitMovRaxRipRelative64(PBYTE pbCode, ULONGLONG Key)
{
    //
    // This doesn't work.
    //
    *pbCode++ = 0x48;   // rex.w
    *pbCode++ = 0x8b;   // mov rax,
    *pbCode++ = 0x05;   // RIP relative
    *((PULONG)pbCode)++ = (ULONG)0x08; // Byte offset from RIP
    *pbCode++ = 0x07;   //
    *pbCode++ = 0;
    *pbCode++ = 0;
    *pbCode++ = 0;
    *((PULONGLONG)pbCode)++ = (ULONGLONG)Key;
    return pbCode;
}

static
PBYTE
EmitPushRipRelativePointer(PBYTE Code, PVOID Pointer)
{
    BOOL Match;
    PCHAR AddressOfRipRelativeInstruction;
    PCHAR AddressOfNextInstruction;
    PCHAR NewEffectiveRipRelativeAddress;
    USHORT ThisInstructionLength = 7;
    LARGE_INTEGER Displacement;
    LONG Disp32;

    AddressOfRipRelativeInstruction = (PCHAR)Code;

    AddressOfNextInstruction = (PCHAR)(
        RtlOffsetToPointer(
            AddressOfRipRelativeInstruction,
            ThisInstructionLength
        )
    );

    //
    // Calculate the signed 32-bit displacement of the address we want to
    // push to the stack.
    //

    Displacement.QuadPart = (LONG_PTR)(
        (LONG_PTR)AddressOfNextInstruction -
        (LONG_PTR)Pointer
    );

    if (Displacement.HighPart) {
        __debugbreak();
    }

    Disp32 = (LONG)Displacement.LowPart;

    if ((PCHAR)Pointer < AddressOfNextInstruction) {

        //
        // Negative displacement from the RIP.
        //

        Disp32 = -1 * (Disp32 - 1);

        NewEffectiveRipRelativeAddress = (PCHAR)(
            ((LONG_PTR)AddressOfNextInstruction + Disp32) - 1
        );

    } else {

        NewEffectiveRipRelativeAddress = (PCHAR)(
            (LONG_PTR)AddressOfNextInstruction + Disp32
        );

    }

    //
    // Verify the logic.
    //

    Match = (
        ((LONG_PTR)Pointer) ==
        ((LONG_PTR)NewEffectiveRipRelativeAddress)
    );

    if (!Match) {
        __debugbreak();
    }

    *Code++ = 0xff;
    *Code++ = 0x35;
    *((UNALIGNED PLONG)Code)++ = Disp32;

    return Code;
}


//=========================================================================
// Internal function:
//
// Round down to the next multiple of rndDown
//=========================================================================
static size_t RoundDown(size_t addr, size_t rndDown)
{
    return (addr / rndDown) * rndDown;
}

//=========================================================================
// Internal function:
//
// Will attempt allocate a block of memory within the specified range, as
// near as possible to the specified function.
//=========================================================================
static MHOOKS_TRAMPOLINE* BlockAlloc(PBYTE pSystemFunction, PBYTE pbLower, PBYTE pbUpper) {
    SYSTEM_INFO sSysInfo;
    SecureZeroMemory(&sSysInfo, sizeof(sSysInfo));
    GetSystemInfo(&sSysInfo);

    // Always allocate in bulk, in case the system actually has a smaller allocation granularity than MINALLOCSIZE.
    const ptrdiff_t cAllocSize = max(sSysInfo.dwAllocationGranularity, MHOOK_MINALLOCSIZE);

    MHOOKS_TRAMPOLINE* pRetVal = NULL;
    PBYTE pModuleGuess = (PBYTE) RoundDown((size_t)pSystemFunction, cAllocSize);
    int loopCount = 0;
    for (PBYTE pbAlloc = pModuleGuess; pbLower < pbAlloc && pbAlloc < pbUpper; ++loopCount) {
        // determine current state
        MEMORY_BASIC_INFORMATION mbi;
        ODPRINTF((L"mhooks: BlockAlloc: Looking at address %p", pbAlloc));
        if (!VirtualQuery(pbAlloc, &mbi, sizeof(mbi)))
            break;
        // free & large enough?
        if (mbi.State == MEM_FREE && mbi.RegionSize >= (unsigned)cAllocSize) {
            // and then try to allocate it
            pRetVal = (MHOOKS_TRAMPOLINE*) VirtualAlloc(pbAlloc, cAllocSize, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (pRetVal) {
                size_t trampolineCount = cAllocSize / sizeof(MHOOKS_TRAMPOLINE);
                ODPRINTF((L"mhooks: BlockAlloc: Allocated block at %p as %d trampolines", pRetVal, trampolineCount));

                pRetVal[0].pPrevTrampoline = NULL;
                pRetVal[0].pNextTrampoline = &pRetVal[1];

                // prepare them by having them point down the line at the next entry.
                for (size_t s = 1; s < trampolineCount; ++s) {
                    pRetVal[s].pPrevTrampoline = &pRetVal[s - 1];
                    pRetVal[s].pNextTrampoline = &pRetVal[s + 1];
                }

                // last entry points to the current head of the free list
                pRetVal[trampolineCount - 1].pNextTrampoline = g_pFreeList;
                break;
            }
        }

        // This is a spiral, should be -1, 1, -2, 2, -3, 3, etc. (* cAllocSize)
        ptrdiff_t bytesToOffset = (cAllocSize * (loopCount + 1) * ((loopCount % 2 == 0) ? -1 : 1));
        pbAlloc = pbAlloc + bytesToOffset;
    }

    return pRetVal;
}

//=========================================================================
// Internal function:
//
// Will try to allocate a big block of memory inside the required range.
//=========================================================================
static MHOOKS_TRAMPOLINE* FindTrampolineInRange(PBYTE pLower, PBYTE pUpper) {
    if (!g_pFreeList) {
        return NULL;
    }

    // This is a standard free list, except we're doubly linked to deal with soem return shenanigans.
    MHOOKS_TRAMPOLINE* curEntry = g_pFreeList;
    while (curEntry) {
        if ((MHOOKS_TRAMPOLINE*) pLower < curEntry && curEntry < (MHOOKS_TRAMPOLINE*) pUpper) {
            ListRemove(&g_pFreeList, curEntry);

            return curEntry;
        }

        curEntry = curEntry->pNextTrampoline;
    }

    return NULL;
}

//=========================================================================
// Internal function:
//
// Will try to allocate the trampoline structure within 2 gigabytes of
// the target function.
//=========================================================================
static MHOOKS_TRAMPOLINE* TrampolineAlloc(PBYTE pSystemFunction, S64 nLimitUp, S64 nLimitDown) {

    MHOOKS_TRAMPOLINE* pTrampoline = NULL;

    // determine lower and upper bounds for the allocation locations.
    // in the basic scenario this is +/- 2GB but IP-relative instructions
    // found in the original code may require a smaller window.
    PBYTE pLower = pSystemFunction + nLimitUp;
    pLower = pLower < (PBYTE)(DWORD_PTR)0x0000000080000000 ?
                        (PBYTE)(0x1) : (PBYTE)(pLower - (PBYTE)0x7fff0000);
    PBYTE pUpper = pSystemFunction + nLimitDown;
    pUpper = pUpper < (PBYTE)(DWORD_PTR)0xffffffff80000000 ?
        (PBYTE)(pUpper + (DWORD_PTR)0x7ff80000) : (PBYTE)(DWORD_PTR)0xfffffffffff80000;
    ODPRINTF((L"mhooks: TrampolineAlloc: Allocating for %p between %p and %p", pSystemFunction, pLower, pUpper));

    // try to find a trampoline in the specified range
    pTrampoline = FindTrampolineInRange(pLower, pUpper);
    if (!pTrampoline) {
        // if it we can't find it, then we need to allocate a new block and
        // try again. Just fail if that doesn't work
        g_pFreeList = BlockAlloc(pSystemFunction, pLower, pUpper);
        pTrampoline = FindTrampolineInRange(pLower, pUpper);
    }

    // found and allocated a trampoline?
    if (pTrampoline) {
        ListPrepend(&g_pHooks, pTrampoline);
    }

    return pTrampoline;
}

//=========================================================================
// Internal function:
//
// Return the internal trampoline structure that belongs to a hooked function.
//=========================================================================
static MHOOKS_TRAMPOLINE* TrampolineGet(PBYTE pHookedFunction) {
    MHOOKS_TRAMPOLINE* pCurrent = g_pHooks;

    while (pCurrent) {
        if (pCurrent->pHookFunction == pHookedFunction) {
            return pCurrent;
        }

        pCurrent = pCurrent->pNextTrampoline;
    }

    return NULL;
}

//=========================================================================
// Internal function:
//
// Free a trampoline structure.
//=========================================================================
static VOID TrampolineFree(MHOOKS_TRAMPOLINE* pTrampoline, BOOL bNeverUsed) {
    ListRemove(&g_pHooks, pTrampoline);

    // If a thread could feasinbly have some of our trampoline code
    // on its stack and we yank the region from underneath it then it will
    // surely crash upon returning. So instead of freeing the
    // memory we just let it leak. Ugly, but safe.
    if (bNeverUsed) {
        ListPrepend(&g_pFreeList, pTrampoline);
    }

    g_nHooksInUse--;
}

//=========================================================================
// Internal function:
//
// Suspend a given thread and try to make sure that its instruction
// pointer is not in the given range.
//=========================================================================
static HANDLE SuspendOneThread(PRTL Rtl, DWORD dwThreadId, PBYTE pbCode, DWORD cbBytes) {
    // open the thread
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
    if (GOOD_HANDLE(hThread)) {
        // attempt suspension
        DWORD dwSuspendCount = SuspendThread(hThread);
        if (dwSuspendCount != -1) {
            // see where the IP is
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_CONTROL;
            int nTries = 0;
            while (GetThreadContext(hThread, &ctx)) {
#ifdef _M_IX86
                PBYTE pIp = (PBYTE)(DWORD_PTR)ctx.Eip;
#elif defined _M_X64
                PBYTE pIp = (PBYTE)(DWORD_PTR)ctx.Rip;
#endif
                if (pIp >= pbCode && pIp < (pbCode + cbBytes)) {
                    if (nTries < 3) {
                        // oops - we should try to get the instruction pointer out of here.
                        ODPRINTF((L"mhooks: SuspendOneThread: suspended thread %d - IP is at %p - IS COLLIDING WITH CODE", dwThreadId, pIp));
                        ResumeThread(hThread);
                        Sleep(100);
                        SuspendThread(hThread);
                        nTries++;
                    } else {
                        // we gave it all we could. (this will probably never
                        // happen - unless the thread has already been suspended
                        // to begin with)
                        ODPRINTF((L"mhooks: SuspendOneThread: suspended thread %d - IP is at %p - IS COLLIDING WITH CODE - CAN'T FIX", dwThreadId, pIp));
                        ResumeThread(hThread);
                        CloseHandle(hThread);
                        hThread = NULL;
                        break;
                    }
                } else {
                    // success, the IP is not conflicting
                    ODPRINTF((L"mhooks: SuspendOneThread: Successfully suspended thread %d - IP is at %p", dwThreadId, pIp));
                    break;
                }
            }
        } else {
            // couldn't suspend
            CloseHandle(hThread);
            hThread = NULL;
        }
    }
    return hThread;
}

//=========================================================================
// Internal function:
//
// Resumes all previously suspended threads in the current process.
//=========================================================================
static VOID ResumeOtherThreads(PRTL Rtl) {
    // make sure things go as fast as possible
    INT nOriginalPriority = GetThreadPriority(GetCurrentThread());
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    // go through our list
    for (DWORD i=0; i<g_nThreadHandles; i++) {
        // and resume & close thread handles
        ResumeThread(g_hThreadHandles[i]);
        CloseHandle(g_hThreadHandles[i]);
    }
    // clean up
    free(g_hThreadHandles);
    g_hThreadHandles = NULL;
    g_nThreadHandles = 0;
    SetThreadPriority(GetCurrentThread(), nOriginalPriority);
}

//=========================================================================
// Internal function:
//
// Suspend all threads in this process while trying to make sure that their
// instruction pointer is not in the given range.
//=========================================================================
static BOOL SuspendOtherThreads(PRTL Rtl, PBYTE pbCode, DWORD cbBytes) {
    BOOL bRet = FALSE;
    // make sure we're the most important thread in the process
    INT nOriginalPriority = GetThreadPriority(GetCurrentThread());
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    // get a view of the threads in the system
    HANDLE hSnap = Rtl->CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
    if (GOOD_HANDLE(hSnap)) {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        // count threads in this process (except for ourselves)
        DWORD nThreadsInProcess = 0;
        if (Rtl->Thread32First(hSnap, &te)) {
            do {
                if (te.th32OwnerProcessID == GetCurrentProcessId()) {
                    if (te.th32ThreadID != GetCurrentThreadId()) {
                        nThreadsInProcess++;
                    }
                }
                te.dwSize = sizeof(te);
            } while(Rtl->Thread32Next(hSnap, &te));
        }
        ODPRINTF((L"mhooks: SuspendOtherThreads: counted %d other threads", nThreadsInProcess));
        if (nThreadsInProcess) {
            // alloc buffer for the handles we really suspended
            g_hThreadHandles = (HANDLE*)malloc(nThreadsInProcess*sizeof(HANDLE));
            if (g_hThreadHandles) {
                ZeroMemory(g_hThreadHandles, nThreadsInProcess*sizeof(HANDLE));
                DWORD nCurrentThread = 0;
                BOOL bFailed = FALSE;
                te.dwSize = sizeof(te);
                // go through every thread
                if (Rtl->Thread32First(hSnap, &te)) {
                    do {
                        if (te.th32OwnerProcessID == GetCurrentProcessId()) {
                            if (te.th32ThreadID != GetCurrentThreadId()) {
                                // attempt to suspend it
                                g_hThreadHandles[nCurrentThread] = SuspendOneThread(Rtl, te.th32ThreadID, pbCode, cbBytes);
                                if (GOOD_HANDLE(g_hThreadHandles[nCurrentThread])) {
                                    ODPRINTF((L"mhooks: SuspendOtherThreads: successfully suspended %d", te.th32ThreadID));
                                    nCurrentThread++;
                                } else {
                                    ODPRINTF((L"mhooks: SuspendOtherThreads: error while suspending thread %d: %d", te.th32ThreadID, gle()));
                                    // TODO: this might not be the wisest choice
                                    // but we can choose to ignore failures on
                                    // thread suspension. It's pretty unlikely that
                                    // we'll fail - and even if we do, the chances
                                    // of a thread's IP being in the wrong place
                                    // is pretty small.
                                    // bFailed = TRUE;
                                }
                            }
                        }
                        te.dwSize = sizeof(te);
                    } while(Rtl->Thread32Next(hSnap, &te) && !bFailed);
                }
                g_nThreadHandles = nCurrentThread;
                bRet = !bFailed;
            }
        }
        CloseHandle(hSnap);
        //TODO: we might want to have another pass to make sure all threads
        // in the current process (including those that might have been
        // created since we took the original snapshot) have been
        // suspended.
    } else {
        ODPRINTF((L"mhooks: SuspendOtherThreads: can't CreateToolhelp32Snapshot: %d", gle()));
    }
    SetThreadPriority(GetCurrentThread(), nOriginalPriority);
    if (!bRet) {
        ODPRINTF((L"mhooks: SuspendOtherThreads: Had a problem (or not running multithreaded), resuming all threads."));
        ResumeOtherThreads(Rtl);
    }
    return bRet;
}

//=========================================================================
// if IP-relative addressing has been detected, fix up the code so the
// offset points to the original location

static void
FixupIPRelativeAddressingNew(
    PBYTE NewAddress,
    PBYTE OriginalAddress,
    PMHOOKS_PATCHDATA PatchData
    )
{
    BOOL Match;
    USHORT Index;
    LONG  Displacement;
    PCHAR AddressOfRipRelativeInstruction;
    PCHAR AddressOfNextInstruction;
    PCHAR NewEffectiveRipRelativeAddress;
    PCHAR AddressOfDisp32Operand;
    PMHOOKS_RIPINFO RipInfo;
    UNALIGNED PLONG Target;

    for (Index = 0; Index < PatchData->nRipCnt; Index++) {
        RipInfo = &PatchData->rips[Index];

        if (RipInfo->FunctionAddress != OriginalAddress) {
            __debugbreak();
        }

        AddressOfDisp32Operand = (PCHAR)(
            (ULONG_PTR)NewAddress +
            RipInfo->OffsetOfDisp32AddressFromFunctionAddress
        );

        AddressOfRipRelativeInstruction = (PCHAR)(
            (ULONG_PTR)NewAddress +
            RipInfo->OffsetOfRipRelativeInstructionFromFunctionAddress
        );

        AddressOfNextInstruction = (PCHAR)(
            (ULONG_PTR)AddressOfRipRelativeInstruction +
            RipInfo->DisplacementLength
        );

        //
        // Calculate the number of bytes AddressOfNextInstruction needs to be
        // displaced in order for the resulting value to equal
        // RipInfo->EffectiveRipRelativeAddress.
        //

        Displacement = (LONG)(
            (LONG_PTR)RipInfo->EffectiveRipRelativeAddress -
            (LONG_PTR)AddressOfNextInstruction
        );

        //
        // Verify the logic.
        //

        NewEffectiveRipRelativeAddress = (PCHAR)(
            (LONG_PTR)AddressOfNextInstruction + Displacement
        );

        Match = (
            RipInfo->EffectiveRipRelativeAddress ==
            NewEffectiveRipRelativeAddress
        );

        if (!Match) {
            __debugbreak();
        }

        Target = (UNALIGNED PLONG)AddressOfDisp32Operand;
        *Target = Displacement;
    }
}

static void FixupIPRelativeAddressing(PBYTE pbNew, PBYTE pbOriginal, MHOOKS_PATCHDATA* pdata)
{
#if defined _M_X64
    if (pdata->rips[0].dwOffset == 0) {
        FixupIPRelativeAddressingNew(pbNew, pbOriginal, pdata);
        return;
    }
    S64 diff = pbNew - pbOriginal;
    for (DWORD i = 0; i < pdata->nRipCnt; i++) {
        DWORD dwNewDisplacement = (DWORD)(pdata->rips[i].nDisplacement - diff);
        ODPRINTF((L"mhooks: fixing up RIP instruction operand for code at 0x%p: "
            L"old displacement: 0x%8.8x, new displacement: 0x%8.8x",
            pbNew + pdata->rips[i].dwOffset,
            (DWORD)pdata->rips[i].nDisplacement,
            dwNewDisplacement));
        *(PDWORD)(pbNew + pdata->rips[i].dwOffset) = dwNewDisplacement;
    }
#endif
}


//=========================================================================
// Examine the machine code at the target function's entry point, and
// skip bytes in a way that we'll always end on an instruction boundary.
// We also detect branches and subroutine calls (as well as returns)
// at which point disassembly must stop.
// Finally, detect and collect information on IP-relative instructions
// that we can patch.
static DWORD DisassembleAndSkip(PRTL Rtl, PVOID pFunction, DWORD dwMinLen, MHOOKS_PATCHDATA* pdata) {
    DWORD dwRet = 0;
    pdata->nLimitDown = 0;
    pdata->nLimitUp = 0;
    pdata->nRipCnt = 0;
#ifdef _M_IX86
    ARCHITECTURE_TYPE arch = ARCH_X86;
#elif defined _M_X64
    ARCHITECTURE_TYPE arch = ARCH_X64;
#else
    #error unsupported platform
#endif
    DISASSEMBLER dis;
    if (InitDisassembler(Rtl, &dis, arch)) {
        INSTRUCTION* pins = NULL;
        U8* pLoc = (U8*)pFunction;
        DWORD dwFlags = DISASM_DECODE | DISASM_DISASSEMBLE | DISASM_ALIGNOUTPUT;
        S64 nAdjustedDisplacement;

        ODPRINTF((L"mhooks: DisassembleAndSkip: Disassembling %p", pLoc));
        while ( (dwRet < dwMinLen) && (pins = GetInstruction(Rtl, &dis, (ULONG_PTR)pLoc, pLoc, dwFlags)) ) {
            ODPRINTF((L"mhooks: DisassembleAndSkip: %p:(0x%2.2x) %s", pLoc, pins->Length, pins->String));
            if (pins->Type == ITYPE_RET     ) break;
            if (pins->Type == ITYPE_BRANCH  ) break;
            if (pins->Type == ITYPE_BRANCHCC) break;
            if (pins->Type == ITYPE_CALL    ) break;
            if (pins->Type == ITYPE_CALLCC  ) break;

            #if defined _M_X64
                BOOL bProcessRip = FALSE;
                // mov or lea to register from rip+imm32
                if ((pins->Type == ITYPE_MOV || pins->Type == ITYPE_LEA) && (pins->X86.Relative) &&
                    (pins->X86.OperandSize == 8) && (pins->OperandCount == 2) &&
                    (pins->Operands[1].Flags & OP_IPREL) && (pins->Operands[1].Register == AMD64_REG_RIP))
                {
                    // rip-addressing "mov reg, [rip+imm32]"
                    ODPRINTF((L"mhooks: DisassembleAndSkip: found OP_IPREL on operand %d with displacement 0x%x (in memory: 0x%x)", 1, pins->X86.Displacement, *(PDWORD)(pLoc+3)));
                    bProcessRip = TRUE;
                }
                // mov or lea to rip+imm32 from register
                else if ((pins->Type == ITYPE_MOV || pins->Type == ITYPE_LEA) && (pins->X86.Relative) &&
                    (pins->X86.OperandSize == 8) && (pins->OperandCount == 2) &&
                    (pins->Operands[0].Flags & OP_IPREL) && (pins->Operands[0].Register == AMD64_REG_RIP))
                {
                    // rip-addressing "mov [rip+imm32], reg"
                    ODPRINTF((L"mhooks: DisassembleAndSkip: found OP_IPREL on operand %d with displacement 0x%x (in memory: 0x%x)", 0, pins->X86.Displacement, *(PDWORD)(pLoc+3)));
                    bProcessRip = TRUE;
                }
                else if ( (pins->OperandCount == 2) && (pins->Operands[0].Flags & OP_IPREL) )
                {
                    PMHOOKS_RIPINFO RipInfo = &pdata->rips[pdata->nRipCnt++];
                    LONG ExpectedDisplacement;
                    UNALIGNED PLONG ActualDisplacement;
                    RipInfo->dwOffset = 0;
                    RipInfo->nDisplacement = 0;
                    RipInfo->DisplacementOffset = 1;
                    if (pins->X86.HasModRM) {
                        RipInfo->DisplacementOffset++;
                    }
                    if (pins->PrefixCount) {
                        RipInfo->DisplacementOffset += (USHORT)pins->PrefixCount;
                    }
                    RipInfo->DisplacementValue = (LONG)pins->X86.Displacement;
                    RipInfo->DisplacementLength = (USHORT)pins->Operands[0].Length;
                    RipInfo->InstructionLength = (USHORT)pins->Length;
                    RipInfo->FunctionAddress = (PCHAR)pFunction;
                    RipInfo->AddressOfRipRelativeInstruction = (PCHAR)(
                        (ULONG_PTR)pFunction +
                        dwRet
                    );
                    RipInfo->AddressOfNextInstruction = (PCHAR)(
                        (ULONG_PTR)RipInfo->AddressOfRipRelativeInstruction +
                        RipInfo->InstructionLength
                    );
                    RipInfo->AddressOfDisp32Operand = (PCHAR)(
                        (ULONG_PTR)RipInfo->AddressOfRipRelativeInstruction +
                        RipInfo->DisplacementOffset
                    );
                    RipInfo->EffectiveRipRelativeAddress = (PCHAR)(
                        (ULONG_PTR)RipInfo->AddressOfNextInstruction +
                        RipInfo->DisplacementValue
                    );
                    RipInfo->OffsetOfRipRelativeInstructionFromFunctionAddress = (USHORT)(
                        (ULONG_PTR)RipInfo->AddressOfRipRelativeInstruction -
                        (ULONG_PTR)RipInfo->FunctionAddress
                    );
                    RipInfo->OffsetOfDisp32AddressFromFunctionAddress = (USHORT)(
                        (ULONG_PTR)RipInfo->AddressOfDisp32Operand -
                        (ULONG_PTR)RipInfo->FunctionAddress
                    );

                    ExpectedDisplacement = (LONG)(
                        (LONG_PTR)RipInfo->EffectiveRipRelativeAddress -
                        (LONG_PTR)RipInfo->AddressOfNextInstruction
                    );

                    ActualDisplacement = (UNALIGNED PLONG)RipInfo->AddressOfDisp32Operand;
                    if (ExpectedDisplacement != *ActualDisplacement) {
                        __debugbreak();
                    }

                    /*
                    // unsupported rip-addressing
                    ODPRINTF((L"mhooks: DisassembleAndSkip: found unsupported OP_IPREL on operand %d", 0));
                    // dump instruction bytes to the debug output
                    for (DWORD i=0; i<pins->Length; i++) {
                        ODPRINTF((L"mhooks: DisassembleAndSkip: instr byte %2.2d: 0x%2.2x", i, pLoc[i]));
                    }
                    break;
                    */
                }
                else if ( (pins->OperandCount == 2) && (pins->Operands[1].Flags & OP_IPREL) )
                {
                    __debugbreak();
                    bProcessRip = TRUE;
                    /*
                    // unsupported rip-addressing
                    ODPRINTF((L"mhooks: DisassembleAndSkip: found unsupported OP_IPREL on operand %d", 1));
                    // dump instruction bytes to the debug output
                    for (DWORD i=0; i<pins->Length; i++) {
                        ODPRINTF((L"mhooks: DisassembleAndSkip: instr byte %2.2d: 0x%2.2x", i, pLoc[i]));
                    }
                    break;
                    */
                }
                else if ( (pins->OperandCount == 3) && (pins->Operands[2].Flags & OP_IPREL) )
                {
                    __debugbreak();
                    /*
                    // unsupported rip-addressing
                    ODPRINTF((L"mhooks: DisassembleAndSkip: found unsupported OP_IPREL on operand %d", 2));
                    // dump instruction bytes to the debug output
                    for (DWORD i=0; i<pins->Length; i++) {
                        ODPRINTF((L"mhooks: DisassembleAndSkip: instr byte %2.2d: 0x%2.2x", i, pLoc[i]));
                    }
                    break;
                    */
                }
                // follow through with RIP-processing if needed
                if (bProcessRip) {
                    // calculate displacement relative to function start
                    nAdjustedDisplacement = pins->X86.Displacement + (pLoc - (U8*)pFunction);

                    // store displacement values furthest from zero (both positive and negative)
                    if (nAdjustedDisplacement < pdata->nLimitDown)
                        pdata->nLimitDown = nAdjustedDisplacement;
                    if (nAdjustedDisplacement > pdata->nLimitUp)
                        pdata->nLimitUp = nAdjustedDisplacement;
                    // store patch info
                    if (pdata->nRipCnt < MHOOKS_MAX_RIPS) {
                        pdata->rips[pdata->nRipCnt].dwOffset = dwRet + 3;
                        pdata->rips[pdata->nRipCnt].nDisplacement = pins->X86.Displacement;
                        pdata->nRipCnt++;
                    } else {
                        // no room for patch info, stop disassembly
                        break;
                    }
                }
            #endif

            dwRet += pins->Length;
            pLoc  += pins->Length;
        }

        CloseDisassembler(Rtl, &dis);
    }

    return dwRet;
}

//=========================================================================
BOOL Mhook_SetHook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key) {
    BOOL PushKey = TRUE;
    PHOOKED_FUNCTION Function = (PHOOKED_FUNCTION)Key;
    MHOOKS_TRAMPOLINE* pTrampoline = NULL;
    PVOID pSystemFunction = *ppSystemFunction;
    if (!Rtl->CreateToolhelp32Snapshot) {
        __debugbreak();
    }
    // ensure thread-safety
    EnterCritSec();
    ODPRINTF((L"mhooks: Mhook_SetHook: Started on the job: %p / %p", pSystemFunction, pHookFunction));
    // find the real functions (jump over jump tables, if any)
    pSystemFunction = SkipJumps((PBYTE)pSystemFunction);
    pHookFunction   = SkipJumps((PBYTE)pHookFunction);
    ODPRINTF((L"mhooks: Mhook_SetHook: Started on the job: %p / %p", pSystemFunction, pHookFunction));
    // figure out the length of the overwrite zone
    MHOOKS_PATCHDATA patchdata;
    SecureZeroMemory(&patchdata, sizeof(patchdata));

    DWORD dwInstructionLength = DisassembleAndSkip(Rtl, pSystemFunction, MHOOK_JMPSIZE, &patchdata);
    if (dwInstructionLength >= MHOOK_JMPSIZE) {
        ODPRINTF((L"mhooks: Mhook_SetHook: disassembly signals %d bytes", dwInstructionLength));
        // suspend every other thread in this process, and make sure their IP
        // is not in the code we're about to overwrite.
        SuspendOtherThreads(Rtl, (PBYTE)pSystemFunction, dwInstructionLength);
        // allocate a trampoline structure (TODO: it is pretty wasteful to get
        // VirtualAlloc to grab chunks of memory smaller than 100 bytes)
        pTrampoline = TrampolineAlloc((PBYTE)pSystemFunction, patchdata.nLimitUp, patchdata.nLimitDown);
        if (pTrampoline) {
            ODPRINTF((L"mhooks: Mhook_SetHook: allocated structure at %p", pTrampoline));
            DWORD dwOldProtectSystemFunction = 0;
            DWORD dwOldProtectTrampolineFunction = 0;
            // set the system function to PAGE_EXECUTE_READWRITE
            if (VirtualProtect(pSystemFunction, dwInstructionLength, PAGE_EXECUTE_READWRITE, &dwOldProtectSystemFunction)) {
                ODPRINTF((L"mhooks: Mhook_SetHook: readwrite set on system function"));
                // mark our trampoline buffer to PAGE_EXECUTE_READWRITE
                if (VirtualProtect(pTrampoline, sizeof(MHOOKS_TRAMPOLINE), PAGE_EXECUTE_READWRITE, &dwOldProtectTrampolineFunction)) {
                    ODPRINTF((L"mhooks: Mhook_SetHook: readwrite set on trampoline structure"));
                    DWORD_PTR Distance;
                    PBYTE ValueAddress = NULL;
                    PBYTE JumpAddress = NULL;
                    // create our trampoline function
                    PBYTE pbCode = pTrampoline->codeTrampoline;
                    // save original code..
                    for (DWORD i = 0; i<dwInstructionLength; i++) {
                        pTrampoline->codeUntouched[i] = pbCode[i] = ((PBYTE)pSystemFunction)[i];
                    }
                    pbCode += dwInstructionLength;

                    // plus a jump to the continuation in the original location
                    pbCode = EmitJump(pbCode, ((PBYTE)pSystemFunction) + dwInstructionLength);
                    ODPRINTF((L"mhooks: Mhook_SetHook: updated the trampoline"));

                    // fix up any IP-relative addressing in the code
                    FixupIPRelativeAddressing(pTrampoline->codeTrampoline, (PBYTE)pSystemFunction, &patchdata);

                    if ((PBYTE)pHookFunction < (PBYTE)pSystemFunction) {
                        Distance = (PBYTE)pSystemFunction - (PBYTE)pHookFunction;
                    }
                    else {
                        Distance = (PBYTE)pHookFunction - (PBYTE)pSystemFunction;
                    }

                    if (Distance > 0x7fff0000) {
                        // create a stub that jumps to the replacement function.
                        // we need this because jumping from the API to the hook directly
                        // will be a long jump, which is 14 bytes on x64, and we want to
                        // avoid that - the API may or may not have room for such stuff.
                        // (remember, we only have 5 bytes guaranteed in the API.)
                        // on the other hand we do have room, and the trampoline will always be
                        // within +/- 2GB of the API, so we do the long jump in there.
                        // the API will jump to the "reverse trampoline" which
                        // will jump to the user's hook code.
                        pbCode = pTrampoline->codeJumpToHookFunction;
                        pbCode = EmitJump(pbCode, (PBYTE)pHookFunction);

                        ODPRINTF((L"mhooks: Mhook_SetHook: created reverse trampoline"));

                        FlushInstructionCache(
                            GetCurrentProcess(),
                            pTrampoline->codeJumpToHookFunction,
                            pbCode - pTrampoline->codeJumpToHookFunction
                        );

                        // update the API itself
                        pbCode = (PBYTE)pSystemFunction;
                        if (Key) {
                            if (PushKey) {

                                pbCode = EmitPushRipRelativePointer(
                                    pbCode,
                                    Key
                                );

                            } else {
                                pbCode = EmitMovRaxImm64(pbCode, (ULONGLONG)Key);
                            }
                        }

                        pbCode = EmitJump(pbCode, pTrampoline->codeJumpToHookFunction);

                    } else {
                        // the jump will be at most 5 bytes so we can do it directly
                        // update the API itself
                        pbCode = (PBYTE)pSystemFunction;
                        if (Key) {
                            if (PushKey) {

                                pbCode = EmitPushRipRelativePointer(
                                    pbCode,
                                    Key
                                );

                            } else {
                                pbCode = EmitMovRaxImm64(pbCode, (ULONGLONG)Key);
                            }
                        }
                        pbCode = EmitJump(pbCode, (PBYTE)pHookFunction);
                    }

                    // update data members
                    pTrampoline->cbOverwrittenCode = dwInstructionLength;
                    pTrampoline->pSystemFunction = (PBYTE)pSystemFunction;
                    pTrampoline->pHookFunction = (PBYTE)pHookFunction;

                    // flush instruction cache and restore original protection
                    FlushInstructionCache(GetCurrentProcess(), pTrampoline->codeTrampoline, dwInstructionLength);
                    VirtualProtect(
                        pTrampoline,
                        sizeof(MHOOKS_TRAMPOLINE),
                        dwOldProtectTrampolineFunction,
                        &dwOldProtectTrampolineFunction
                    );

                } else {
                    ODPRINTF((L"mhooks: Mhook_SetHook: failed VirtualProtect 2: %d", gle()));
                }
                // flush instruction cache and restore original protection
                FlushInstructionCache(GetCurrentProcess(), pSystemFunction, dwInstructionLength);
                VirtualProtect(pSystemFunction, dwInstructionLength, dwOldProtectSystemFunction, &dwOldProtectSystemFunction);
            } else {
                ODPRINTF((L"mhooks: Mhook_SetHook: failed VirtualProtect 1: %d", gle()));
            }
            if (pTrampoline->pSystemFunction) {
                // this is what the application will use as the entry point
                // to the "original" unhooked function.
                *ppSystemFunction = pTrampoline->codeTrampoline;
                ODPRINTF((L"mhooks: Mhook_SetHook: Hooked the function!"));
            } else {
                // if we failed discard the trampoline (forcing VirtualFree)
                TrampolineFree(pTrampoline, TRUE);
                pTrampoline = NULL;
            }
        }
        // resume everybody else
        ResumeOtherThreads(Rtl);
    } else {
        ODPRINTF((L"mhooks: disassembly signals %d bytes (unacceptable)", dwInstructionLength));
    }
    LeaveCritSec();
    return (pTrampoline != NULL);
}

BOOL
Mhook_SetFunctionHook(
    _In_    PRTL Rtl,
    _Inout_ PPVOID SystemFunctionPointer,
    _In_    PHOOKED_FUNCTION Function
    )
{
    BOOL Success;
    USHORT Count;
    MHOOKS_PATCHDATA PatchData;
    PMHOOKS_TRAMPOLINE Trampoline = NULL;
    ULONG InstructionLength;
    PBYTE Byte;
    PBYTE Code;
    PBYTE Untouched;
    PBYTE ValueAddress = NULL;
    PBYTE JumpAddress = NULL;
    DWORD OldProtectSystemFunction = 0;
    DWORD OldProtectTrampolineFunction = 0;
    HANDLE CurrentProcess = GetCurrentProcess();
    PROC HookProlog = Function->HookProlog;
    PVOID HookedFunction = HookProlog;
    PVOID SystemFunction = *SystemFunctionPointer;

    if (!Rtl->CreateToolhelp32Snapshot) {
        __debugbreak();
    }

    EnterCritSec();

    SystemFunction = SkipJumps((PBYTE)Function->OriginalAddress);

    SecureZeroMemory(&PatchData, sizeof(PatchData));

    InstructionLength = DisassembleAndSkip(
        Rtl,
        SystemFunction,
        MHOOK_JMPSIZE,
        &PatchData
    );

    if (InstructionLength < MHOOK_JMPSIZE) {
        goto End;
    }

    SuspendOtherThreads(Rtl, (PBYTE)SystemFunction, InstructionLength);

    Trampoline = TrampolineAlloc(
        (PBYTE)SystemFunction,
        PatchData.LimitUp,
        PatchData.LimitDown
    );

    if (!Trampoline) {
        goto End;
    }

    Success = VirtualProtect(
        SystemFunction,
        InstructionLength,
        PAGE_EXECUTE_READWRITE,
        &OldProtectSystemFunction
    );

    if (!Success) {
        goto Error;
    }

    Success = VirtualProtect(
        Trampoline,
        sizeof(MHOOKS_TRAMPOLINE),
        PAGE_EXECUTE_READWRITE,
        &OldProtectTrampolineFunction
    );

    if (!Success && (OldProtectSystemFunction != PAGE_EXECUTE_READWRITE)) {

        VirtualProtect(
            SystemFunction,
            InstructionLength,
            OldProtectSystemFunction,
            NULL
        );

        goto Error;
    }

    Code = Trampoline->CodeTrampoline;
    Byte = (PBYTE)SystemFunction;
    Untouched = (PBYTE)Trampoline->CodeUntouched;

    Count = (USHORT)InstructionLength;

    //
    // Copy the original code twice; once to the pristine "untouched"
    // area, and once to the new code trampoline area.
    //

    do {
        *Untouched++ = *Code++ = *Byte++;
        //PBYTE Next = Byte++;
        //*Untouched++ = Next;
        //*Code++ = Next;
    } while (--Count);

    /*
    __movsb((PBYTE)Trampoline->CodeUntouched,
            (PBYTE)SystemFunction,
            InstructionLength);

    Code = (PBYTE)Trampoline->CodeTrampoline;

    __movsb(Code,
            (PBYTE)SystemFunction,
            InstructionLength);

    Code += InstructionLength;
    */

    /*
    for (Index = 0; Index < InstructionLength; Index++) {
        Trampoline->CodeUntouched[Index] =  \
            Code[Index] =                   \
                ((PBYTE)SystemFunction[Index]);
    }
    */

    // save original code..
    //for (DWORD i = 0; i<InstructionLength; i++) {
    //    Trampoline->codeUntouched[i] = Code[i] = ((PBYTE)SystemFunction)[i];
    //}
    //Code += InstructionLength;

    //
    // Emit a jump to the continuation in the original location.
    //

    Code = EmitJump(Code, ((PBYTE)SystemFunction) + InstructionLength);

    FixupIPRelativeAddressing(
        Trampoline->CodeTrampoline,
        (PBYTE)SystemFunction,
        &PatchData
    );

    //
    // Now prepare the area that we short (imm32) jump to from the
    // original system function.  This code pushes a RIP-relative
    // imm32 displacement (that references the QWORD_PTR to the
    // HOOKED_FUNCTION struct), then emits the long jump to the
    // hook prolog (the HookedFunction).
    //

    Code = Trampoline->CodeJumpToHookFunction;
    Code = EmitPushRipRelativePointer(Code, Function);
    Code = EmitJump(Code, (PBYTE)HookedFunction);

    FlushInstructionCache(
        CurrentProcess,
        Trampoline->CodeJumpToHookFunction,
        Code - Trampoline->CodeJumpToHookFunction
    );

    //
    // Finally, update the system function with the jump to our area
    // prepared above.
    //

    Code = (PBYTE)SystemFunction;
    Code = EmitJump(Code, Trampoline->CodeJumpToHookFunction);

    Trampoline->OverwrittenCode = InstructionLength;
    Trampoline->SystemFunction = (PBYTE)SystemFunction;
    Trampoline->HookedFunction = (PBYTE)HookedFunction;

    FlushInstructionCache(
        CurrentProcess,
        Trampoline->CodeTrampoline,
        InstructionLength
    );

    FlushInstructionCache(
        CurrentProcess,
        SystemFunction,
        InstructionLength
    );

    //
    // Restore protection.
    //

    VirtualProtect(
        Trampoline,
        sizeof(MHOOKS_TRAMPOLINE),
        OldProtectTrampolineFunction,
        NULL
    );

    VirtualProtect(
        SystemFunction,
        InstructionLength,
        OldProtectSystemFunction,
        NULL
    );

    Function->Trampoline = Trampoline;
    Function->ContinuationAddress = Trampoline->CodeTrampoline;
    *SystemFunctionPointer = Function->ContinuationAddress;

    goto End;

Error:
    TrampolineFree(Trampoline, TRUE);
    Trampoline = NULL;

End:
    ResumeOtherThreads(Rtl);
    LeaveCritSec();
    return (Trampoline != NULL);
}

BOOL Mhook_ForceHook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key) {
    BOOL Success;
    PHOOKED_FUNCTION Function = (PHOOKED_FUNCTION)Key;
    MHOOKS_TRAMPOLINE* pTrampoline = NULL;
    PVOID pSystemFunction = *ppSystemFunction;
    if (!Rtl->CreateToolhelp32Snapshot) {
        __debugbreak();
    }
    // ensure thread-safety
    EnterCritSec();
    ODPRINTF((L"mhooks: Mhook_SetHook: Started on the job: %p / %p", pSystemFunction, pHookFunction));
    // find the real functions (jump over jump tables, if any)
    pSystemFunction = SkipJumps((PBYTE)pSystemFunction);
    pHookFunction = SkipJumps((PBYTE)pHookFunction);
    ODPRINTF((L"mhooks: Mhook_SetHook: Started on the job: %p / %p", pSystemFunction, pHookFunction));
    // figure out the length of the overwrite zone
    MHOOKS_PATCHDATA patchdata;
    SecureZeroMemory(&patchdata, sizeof(patchdata));

    DWORD dwInstructionLength = 5;
    SuspendOtherThreads(Rtl, (PBYTE)pSystemFunction, dwInstructionLength);
    // allocate a trampoline structure (TODO: it is pretty wasteful to get
    // VirtualAlloc to grab chunks of memory smaller than 100 bytes)
    pTrampoline = TrampolineAlloc((PBYTE)pSystemFunction, patchdata.nLimitUp, patchdata.nLimitDown);
    if (!pTrampoline) {
        goto Error;
    }

    DWORD dwOldProtectSystemFunction = 0;
    DWORD dwOldProtectTrampolineFunction = 0;
    // set the system function to PAGE_EXECUTE_READWRITE
    if (!VirtualProtect(pSystemFunction,
                        dwInstructionLength,
                        PAGE_EXECUTE_READWRITE,
                        &dwOldProtectSystemFunction))
    {
        goto Error;
    }

    ODPRINTF((L"mhooks: Mhook_SetHook: readwrite set on system function"));
    // mark our trampoline buffer to PAGE_EXECUTE_READWRITE
    if (!VirtualProtect(pTrampoline,
                        sizeof(MHOOKS_TRAMPOLINE),
                        PAGE_EXECUTE_READWRITE,
                        &dwOldProtectTrampolineFunction))
    {
        goto Error;
    }

    DWORD_PTR Distance;
    PBYTE ValueAddress = NULL;
    PBYTE JumpAddress = NULL;
    // create our trampoline function
    PBYTE pbCode = pTrampoline->codeTrampoline;
    // save original code..
    /*
    for (DWORD i = 0; i<dwInstructionLength; i++) {
        pTrampoline->codeUntouched[i] = pbCode[i] = ((PBYTE)pSystemFunction)[i];
    }
    pbCode += dwInstructionLength;
    */

    // plus a jump to the continuation in the original location
    //pbCode = EmitJump(pbCode, ((PBYTE)pSystemFunction) + dwInstructionLength);

    if ((PBYTE)pHookFunction < (PBYTE)pSystemFunction) {
        Distance = (PBYTE)pSystemFunction - (PBYTE)pHookFunction;
    }
    else {
        Distance = (PBYTE)pHookFunction - (PBYTE)pSystemFunction;
    }

    if (Distance > 0x7fff0000) {
        // create a stub that jumps to the replacement function.
        // we need this because jumping from the API to the hook directly
        // will be a long jump, which is 14 bytes on x64, and we want to
        // avoid that - the API may or may not have room for such stuff.
        // (remember, we only have 5 bytes guaranteed in the API.)
        // on the other hand we do have room, and the trampoline will always be
        // within +/- 2GB of the API, so we do the long jump in there.
        // the API will jump to the "reverse trampoline" which
        // will jump to the user's hook code.
        pbCode = pTrampoline->codeJumpToHookFunction;
        pbCode = EmitJump(pbCode, (PBYTE)pHookFunction);
        ODPRINTF((L"mhooks: Mhook_SetHook: created reverse trampoline"));
        FlushInstructionCache(GetCurrentProcess(), pTrampoline->codeJumpToHookFunction,
            pbCode - pTrampoline->codeJumpToHookFunction);

        // update the API itself
        pbCode = (PBYTE)pSystemFunction;
        if (Key) {
            pbCode = EmitMovRaxImm64(pbCode, (ULONGLONG)Key);
        }
        pbCode = EmitJump(pbCode, pTrampoline->codeJumpToHookFunction);
    }
    else {
        // the jump will be at most 5 bytes so we can do it directly
        // update the API itself
        pbCode = (PBYTE)pSystemFunction;
        if (Key) {
            pbCode = EmitMovRaxImm64(pbCode, (ULONGLONG)Key);
        }
        pbCode = EmitJump(pbCode, (PBYTE)pHookFunction);
    }

    // update data members
    pTrampoline->cbOverwrittenCode = dwInstructionLength;
    pTrampoline->pSystemFunction = (PBYTE)pSystemFunction;
    pTrampoline->pHookFunction = (PBYTE)pHookFunction;

    // flush instruction cache and restore original protection
    FlushInstructionCache(GetCurrentProcess(), pTrampoline->codeTrampoline, dwInstructionLength);
    VirtualProtect(pTrampoline, sizeof(MHOOKS_TRAMPOLINE), dwOldProtectTrampolineFunction, &dwOldProtectTrampolineFunction);
    // flush instruction cache and restore original protection
    FlushInstructionCache(GetCurrentProcess(), pSystemFunction, dwInstructionLength);
    VirtualProtect(pSystemFunction, dwInstructionLength, dwOldProtectSystemFunction, &dwOldProtectSystemFunction);

    Success = TRUE;

Error:
    ResumeOtherThreads(Rtl);
    LeaveCritSec();

    if (!Success) {
        TrampolineFree(pTrampoline, TRUE);
        pTrampoline = NULL;
    }

    return (pTrampoline != NULL);
}

//=========================================================================
BOOL Mhook_Unhook(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key) {
    ODPRINTF((L"mhooks: Mhook_Unhook: %p", *ppHookedFunction));
    BOOL bRet = FALSE;
    EnterCritSec();
    // get the trampoline structure that corresponds to our function
    MHOOKS_TRAMPOLINE* pTrampoline = TrampolineGet((PBYTE)*ppHookedFunction);
    if (pTrampoline) {
        // make sure nobody's executing code where we're about to overwrite a few bytes
        SuspendOtherThreads(Rtl, pTrampoline->pSystemFunction, pTrampoline->cbOverwrittenCode);
        ODPRINTF((L"mhooks: Mhook_Unhook: found struct at %p", pTrampoline));
        DWORD dwOldProtectSystemFunction = 0;
        // make memory writable
        if (VirtualProtect(pTrampoline->pSystemFunction, pTrampoline->cbOverwrittenCode, PAGE_EXECUTE_READWRITE, &dwOldProtectSystemFunction)) {
            ODPRINTF((L"mhooks: Mhook_Unhook: readwrite set on system function"));
            PBYTE pbCode = (PBYTE)pTrampoline->pSystemFunction;
            for (DWORD i = 0; i<pTrampoline->cbOverwrittenCode; i++) {
                pbCode[i] = pTrampoline->codeUntouched[i];
            }
            // flush instruction cache and make memory unwritable
            FlushInstructionCache(GetCurrentProcess(), pTrampoline->pSystemFunction, pTrampoline->cbOverwrittenCode);
            VirtualProtect(pTrampoline->pSystemFunction, pTrampoline->cbOverwrittenCode, dwOldProtectSystemFunction, &dwOldProtectSystemFunction);
            // return the original function pointer
            *ppHookedFunction = pTrampoline->pSystemFunction;
            bRet = TRUE;
            ODPRINTF((L"mhooks: Mhook_Unhook: sysfunc: %p", *ppHookedFunction));
            // free the trampoline while not really discarding it from memory
            TrampolineFree(pTrampoline, FALSE);
            ODPRINTF((L"mhooks: Mhook_Unhook: unhook successful"));
        } else {
            ODPRINTF((L"mhooks: Mhook_Unhook: failed VirtualProtect 1: %d", gle()));
        }
        // make the other guys runnable
        ResumeOtherThreads(Rtl);
    }
    LeaveCritSec();
    return bRet;
}

//=========================================================================
