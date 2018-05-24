/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableTestData.h

Abstract:

    This is the header file for the test data module of the PerfectHashTable
    component.  It is responsible for defining the test data structure.

--*/

#ifdef __cplusplus
extern "C" {
#endif

#include "../Rtl/Rtl.h"

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_TEST_DATA {

    //
    // The size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE_TEST_DATA))
        ULONG SizeOfStruct;

} PERFECT_HASH_TABLE_TEST_DATA;
typedef PERFECT_HASH_TABLE_TEST_DATA *PPERFECT_HASH_TABLE_TEST_DATA;

extern PERFECT_HASH_TABLE_TEST_DATA PerfectHashTableTestData;

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
