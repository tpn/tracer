
#include "targetver.h"

#include <Windows.h>
#include <sal.h>
#include <intrin.h>
#include "../Rtl/Rtl.h"
#include "../StringTable/StringTable.h"
#include "../StringTable/StringTableTestGlue.h"
#include "../StringTable/StringTablePrivate.h"
#include "../StringTable/StringTableConstants.h"
#include "../StringTable/StringLoadStoreOperations.h"

#include "../TracerHeap/AlignedTracerHeap.c"

#include <iostream>

using namespace std;

MAKE_STRING(aardvark);
MAKE_STRING(apple);
MAKE_STRING(aba);
MAKE_STRING(abate);
MAKE_STRING(pre);
MAKE_STRING(present);
MAKE_STRING(presumptuous);
MAKE_STRING(prefault);
MAKE_STRING(predefined);

LARGE_INTEGER Frequency;

typedef enum _IS_PREFIX_METHOD_ID {
    Inline = 1,
    Normal
} IS_PREFIX_METHOD_ID;

typedef struct _PREFIX_METHOD {
    IS_PREFIX_METHOD_ID IsPrefixMethodId;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
    LPCSTR Name;
} PREFIX_METHOD, *PPREFIX_METHOD, **PPPREFIX_METHOD;

static PREFIX_METHOD PrefixMethods[] {
    { Inline,   IsPrefixOfStringInSingleTableInline, "Inline" },
    { Normal,   IsPrefixOfStringInSingleTable_C,     "Normal" }
};

static const ULONG NumberOfPrefixMethods = (
    sizeof(PrefixMethods) /
    sizeof(PREFIX_METHOD)
);

FORCEINLINE
BOOL
NameToPrefixMethod(
    _In_ LPCSTR Name,
    _Outptr_result_maybenull_ PPPREFIX_METHOD PrefixMethod
    )
{
    USHORT Index;

    for (Index = 0; Index < NumberOfPrefixMethods; Index++) {
        if (!strcmp(Name, PrefixMethods[Index].Name)) {
            *PrefixMethod = &PrefixMethods[Index];
            return TRUE;
        }
    }

    return FALSE;
}


VOID
TestPrefixMatchInTable(
    _In_ ULONG RoundsPerIteration,
    _In_ ULONG BestOf,
    _In_ PSTRING_TABLE StringTable,
    _In_ PSTRING Search,
    _In_ PPREFIX_METHOD PrefixMethod
    )
{
    ULONG Index;
    ULONG Round;
    LPCSTR Name = PrefixMethod->Name;
    ULONG64 Fastest = (ULONG64)-1LL;
    LARGE_INTEGER Start;
    LARGE_INTEGER End;
    LARGE_INTEGER TotalStart;
    LARGE_INTEGER TotalEnd;
    ULONG64 Elapsed;
    ULONG64 TotalElapsed;
    ULONG64 TotalIterations;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
    double Duration;
    double TotalDuration;
    double Each;
    double Nanoseconds;

    IsPrefixOfStringInTable = PrefixMethod->IsPrefixOfStringInTable;

    cout << "TestPrefixMatchInTable: " << Name << " " << Search->Buffer << endl;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    QueryPerformanceCounter(&TotalStart);

    for (Round = 0; Round < BestOf; Round++) {

        QueryPerformanceCounter(&Start);

        for (Index = 0; Index < RoundsPerIteration; Index++) {
#pragma noinline
            IsPrefixOfStringInTable(StringTable, Search, NULL);
        }

        QueryPerformanceCounter(&End);

        Elapsed = End.QuadPart - Start.QuadPart;

        if (Elapsed < Fastest) {
            Fastest = Elapsed;
        }
    }

    QueryPerformanceCounter(&TotalEnd);

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    TotalElapsed = TotalEnd.QuadPart - TotalStart.QuadPart;

    TotalIterations = BestOf * RoundsPerIteration;

    cout << "Total Elapsed: " << TotalElapsed << endl;
    cout << "Total Iterations: " << TotalIterations << endl;

    cout << "Fastest Elapsed: " << Elapsed << endl;

    Duration = ((double)Elapsed) / ((double)Frequency.QuadPart);

    TotalDuration = ((double)TotalElapsed) / ((double)Frequency.QuadPart);

    cout << "Duration: " << Duration << endl;

    Each = Duration / (double)RoundsPerIteration;

    cout << "Each: " << Each << endl;

    Nanoseconds = Each * 1.0E9;

    cout << "Nanoseconds: " << Nanoseconds << endl;

    cout << "Frequency: " << Frequency.QuadPart << endl;


}

#define ITERATIONS  1000000
#define BEST_OF     100

#define FUNC TestPrefixMatchInTable

#define TEST(String) \
    FUNC(ITERATIONS, BEST_OF, Table, String, PrefixMethod);

int main(int argc, char **argv)
{
    ALLOCATOR Allocator;
    PSTRING_TABLE Table;
    STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
        aardvark,
        apple,
        aba,
        abate,
        present,
        pre
    );

    PPREFIX_METHOD PrefixMethod;

    if (argc != 2) {
        PrefixMethod = &PrefixMethods[0];
    } else {
        if (!NameToPrefixMethod(argv[1], &PrefixMethod)) {
            cout << "Unknown prefix method: " << argv[1] << endl;
            return 1;
        }
    }

    cout << "Using prefix method: " << PrefixMethod->Name << endl;

    INIT_ALLOCATOR(&Allocator);

    Table = MAKE_TABLE(&StringArray6);

    QueryPerformanceFrequency(&Frequency);

    TEST(&aba);
    TEST(&pre);
    TEST(&present);
    TEST(&presumptuous);
    TEST(&predefined);
    TEST(&aardvark);

    DESTROY_ALLOCATOR(&Allocator);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
