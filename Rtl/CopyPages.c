/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CopyPages.c

Abstract:

    This module implements routines that copy memory pages at a time.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
CopyPagesMovsq(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages,
    PPAGE_COPY_TYPE PageCopyType
    )
{
    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            NumberOfPages << PAGE_SHIFT);

    if (ARGUMENT_PRESENT(PageCopyType)) {
        PageCopyType->Movsq = TRUE;
    }
}

_Use_decl_annotations_
VOID
CopyPagesAvx2(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages,
    PPAGE_COPY_TYPE PageCopyType
    )
{
    ULONG Outer;
    USHORT Inner;

    YMMWORD Ymm1;
    YMMWORD Ymm2;
    YMMWORD Ymm3;
    YMMWORD Ymm4;

    YMMWORD Ymm5;
    YMMWORD Ymm6;
    YMMWORD Ymm7;
    YMMWORD Ymm8;

    PYMMWORD DestYmm;
    PYMMWORD SourceYmm;

    DestYmm = (PYMMWORD)Dest;
    SourceYmm = (PYMMWORD)Source;

    for (Outer = 0; Outer < NumberOfPages; Outer++) {

        //
        // Each loop iteration copies 256 bytes.  We repeat it 16 times to copy
        // the entire 4096 byte page.
        //

        for (Inner = 0; Inner < 16; Inner++) {

            Ymm1 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm2 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm1);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm2);

            Ymm3 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm4 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm3);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm4);

            //
            // (128 bytes copied.)
            //

            Ymm5 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm6 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm5);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm6);

            Ymm7 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm8 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm7);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm8);

            //
            // (256 bytes copied.)
            //
        }
    }

    if (ARGUMENT_PRESENT(PageCopyType)) {
        PageCopyType->Avx2 = TRUE;
    }
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
