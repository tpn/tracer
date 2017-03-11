/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CopyPages.c

Abstract:

    This module implements routines that copy memory pages at a time.

--*/

#include "stdafx.h"

#ifdef _DEBUG_COPY_PAGES
_Use_decl_annotations_
VOID
CopyPagesMovsq(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages,
    PPAGE_COPY_TYPE PageCopyType
    )
{
    ULONG_PTR BytesToCopy = NumberOfPages << PAGE_SHIFT;
    ULONG_PTR QuadWordsToCopy = BytesToCopy >> 3;

    __debugbreak();

    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            QuadWordsToCopy);

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
    ULONG Outer = 0;
    USHORT Inner = 0;
    ULONG_PTR BytesToCopy;
    ULONG_PTR BytesCopied = 0;
    ULONG_PTR PagesCopied = 0;

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

    PCHAR DestEnd;
    PCHAR SourceEnd;

    PCHAR DestPtr;
    PCHAR SourcePtr;

    DestYmm = (PYMMWORD)Dest;
    SourceYmm = (PYMMWORD)Source;

    DestPtr = Dest;
    SourcePtr = Source;

    BytesToCopy = NumberOfPages << PAGE_SHIFT;
    DestEnd = RtlOffsetToPointer(Dest, BytesToCopy);
    SourceEnd = RtlOffsetToPointer(Source, BytesToCopy);

    //__debugbreak();

    for (Outer = 0; Outer < NumberOfPages; Outer++) {

        if (PagesCopied > NumberOfPages) {
            __debugbreak();
        }

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

            SourcePtr += 128;
            DestPtr += 128;
            BytesCopied += 128;

            if (SourcePtr >= SourceEnd) {
                __debugbreak();
            }

            if (DestPtr >= DestEnd) {
                __debugbreak();
            }

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

            SourcePtr += 128;
            DestPtr += 128;
            BytesCopied += 128;

            //
            // (256 bytes copied.)
            //

            if (SourcePtr > SourceEnd) {
                __debugbreak();
            }

            if (DestPtr > DestEnd) {
                __debugbreak();
            }

        }

        PagesCopied += 1;
    }

    if (ARGUMENT_PRESENT(PageCopyType)) {
        PageCopyType->Avx2 = TRUE;
    }
}

#else

_Use_decl_annotations_
VOID
CopyPagesMovsq_C(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages
    )
{
    ULONG_PTR BytesToCopy = NumberOfPages << PAGE_SHIFT;
    ULONG_PTR QuadWordsToCopy = BytesToCopy >> 3;

    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            QuadWordsToCopy);
}

_Use_decl_annotations_
VOID
CopyPagesAvx2_C(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages
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
        // Each loop iteration copies 512 bytes.  We repeat it 8 times to copy
        // the entire 4096 byte page.
        //

        for (Inner = 0; Inner < 8; Inner++) {

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

            Ymm1 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm2 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm1);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm2);

            Ymm3 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm4 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm3);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm4);

            //
            // (384 bytes copied.)
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
            // (512 bytes copied.)
            //
        }

    }
}

_Use_decl_annotations_
VOID
CopyPagesMovsq(
    PCHAR Dest,
    PCHAR Source,
    ULONG NumberOfPages,
    PPAGE_COPY_TYPE PageCopyType
    )
{
    CopyPagesMovsq_C(Dest, Source, NumberOfPages);
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
#if 0
    CopyPagesAvx2_x64(Dest, Source, NumberOfPages);
    if (ARGUMENT_PRESENT(PageCopyType)) {
        PageCopyType->Avx2C = TRUE;
    }
#else
    CopyPagesNonTemporalAvx2_v4(Dest, Source, NumberOfPages);
    if (ARGUMENT_PRESENT(PageCopyType)) {
        PageCopyType->Avx2NonTemporal = TRUE;
    }
#endif
}

#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
