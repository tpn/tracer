/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Chm_01.c

Abstract:

    This module implements the CHM perfect hash table algorithm.  As a time
    saving measure, this module contains *everything* pertaining to the CHM
    implementation, including more general structures like hypergraphs etc.

    The general structures and functions can be moved out into their own
    modules at a later date if we author other algorithms that wish to use them.

    N.B. This implementation attempts to mirror the chm.c implementation as
         best it can, including but not limited to the underlying algorithm
         approach and function names.  This will be used as the baseline for
         evaluating the performance of subsequent revisions.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
CreatePerfectHashTableImplChm01(
    PPERFECT_HASH_TABLE Table
    )
/*++

Routine Description:

    Attempts to create a perfect hash table using the CHM algorithm and a
    2-part random hypergraph.

Arguments:

    Table - Supplies a pointer to a partially-initialized PERFECT_HASH_TABLE
        structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    PULONG Keys;
    PGRAPH Graph;
    BOOLEAN Success;
    PBYTE Buffer;
    PBYTE ExpectedBuffer;
    PVOID BaseAddress = NULL;
    PALLOCATOR Allocator;
    ULONGLONG NextSizeInBytes;
    ULONGLONG PrevSizeInBytes;
    ULONGLONG FirstSizeInBytes;
    ULONGLONG EdgesSizeInBytes;
    ULONGLONG AssignedSizeInBytes;
    ULARGE_INTEGER AllocSize;
    ULARGE_INTEGER NumberOfEdges;
    ULARGE_INTEGER NumberOfVertices;
    ULARGE_INTEGER TotalNumberOfEdges;
    ULARGE_INTEGER BitmapBufferSizeInBytes;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Table)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Rtl = Table->Rtl;
    Keys = (PULONG)Table->Keys->BaseAddress;;
    Allocator = Table->Allocator;

    //
    // The number of edges in our graph is equal to the number of keys in the
    // input data set.
    //

    NumberOfEdges.QuadPart = Table->Keys->NumberOfElements.QuadPart;

    //
    // Sanity check we're under MAX_ULONG.
    //

    ASSERT(!NumberOfEdges.HighPart);

    //
    // The r-graph (r = 2) nature of this implementation results in various
    // arrays having twice the number of elements indicated by the edge count.
    // Capture this number now, as we need it in various size calculations.
    //

    TotalNumberOfEdges.QuadPart = NumberOfEdges.QuadPart;
    TotalNumberOfEdges.QuadPart <<= 1;

    //
    // Another overflow sanity check.
    //

    ASSERT(!TotalNumberOfEdges.HighPart);

    //
    // chm.c uses a size muliplier (c) of 2.09.  Let's avoid the need for
    // doubles and linking with a math library in order to get ceil(), and
    // just use 2.5, which we can calculate by adding the result of right
    // shifting the number of edges by 1 to the result of left shifting
    // said edge count by 1 (simulating multiplication by 0.5).
    //

    NumberOfVertices.QuadPart = NumberOfEdges.LowPart << 1;
    NumberOfVertices.QuadPart += NumberOfEdges.LowPart >> 1;

    //
    // Another sanity check we haven't exceeded MAX_ULONG.
    //

    ASSERT(!NumberOfVertices.HighPart);

    //
    // Calculate the size required for the buffers used by the bitmaps in the
    // graph structure.  Each bitmap consumes 1 bit per edge, and requires the
    // double sizing via the TotalNumberOfEdges count.  Convert the number of
    // bits into a byte representation by shifting right 3 (dividing by 8),
    // then align it up to a 16 byte boundary.
    //

    BitmapBufferSizeInBytes.QuadPart = (
        ALIGN_UP((TotalNumberOfEdges.QuadPart >> 3), 16)
    );

    //
    // Calculate the sizes required for each of the arrays.  We collect them
    // into independent variables as it makes carving up the allocated buffer
    // easier down the track.
    //

    EdgesSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Edges) * TotalNumberOfEdges.QuadPart)
    );

    NextSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Next) * TotalNumberOfEdges.QuadPart)
    );

    FirstSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->First) * NumberOfVertices.QuadPart)
    );

    PrevSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Prev) * TotalNumberOfEdges.QuadPart)
    );

    AssignedSizeInBytes = (
        ALIGN_UP_POINTER(sizeof(*Graph->Assigned) * NumberOfVertices.QuadPart)
    );

    //
    // Calculate the total size required for the underlying graph, such that
    // we can allocate memory via a single call to the allocator.
    //

    AllocSize.QuadPart = ALIGN_UP_POINTER(

        //
        // Account for the size of the graph structure.
        //

        sizeof(GRAPH) +

        //
        // Account for the size of the Graph->Edges array, which is double
        // sized.
        //

        EdgesSizeInBytes +

        //
        // Account for the size of the Graph->Next array; also double sized.
        //

        NextSizeInBytes +

        //
        // Account for the size of the Graph->First array.  This is sized
        // proportional to the number of vertices.
        //

        FirstSizeInBytes +

        //
        // Account for the size of the Graph->Prev array, also double sized.
        //

        PrevSizeInBytes +

        //
        // Account for Graph->Assigned array of vertices.
        //

        AssignedSizeInBytes +

        //
        // Account for the size of the bitmap buffer for Graph->DeletedEdges.
        //

        BitmapBufferSizeInBytes.QuadPart +

        //
        // Account for the size of the bitmap buffer for Graph->VisitedEdges.
        //

        BitmapBufferSizeInBytes.QuadPart

    );

    //
    // Sanity check the size hasn't overflowed.
    //

    ASSERT(!AllocSize.HighPart);

    //
    // Allocate a single chunk of memory for the graph based on the size we
    // just calculated.
    //

    BaseAddress = Allocator->Calloc(Allocator->Context, 1, AllocSize.QuadPart);
    if (!BaseAddress) {
        goto Error;
    }

    //
    // Allocation succeeded.  Carve up the underlying buffer to the various
    // pieces.
    //

    Graph = (PGRAPH)BaseAddress;

    //
    // Initialize the scalar fields of the graph structure.
    //

    Graph->SizeOfStruct = sizeof(*Graph);
    Graph->TotalAllocationSizeInBytes.QuadPart = AllocSize.QuadPart;
    Graph->BaseAddress = BaseAddress;
    Graph->Flags.AsULong = 0;
    Graph->NumberOfEdges = NumberOfEdges.LowPart;
    Graph->NumberOfVertices = NumberOfVertices.LowPart;
    Graph->TotalNumberOfEdges = TotalNumberOfEdges.LowPart;

    //
    // Carve out the backing memory structures for arrays and bitmap buffers.
    // Use the PBYTE Buffer here to make pointer arithmetic a tad easier.  We
    // initialize it to the start of the memory immediately following the graph
    // structure, and then bump it each time we carve out an array or bitmap
    // buffer.
    //

    Buffer = RtlOffsetToPointer(Graph, sizeof(*Graph));

    //
    // Carve out the Graph->Edges array.
    //

    Graph->Edges = (PEDGE)Buffer;
    Buffer += EdgesSizeInBytes;

    //
    // Carve out the Graph->Next array.
    //

    Graph->Next = (PEDGE)Buffer;
    Buffer += NextSizeInBytes;

    //
    // Carve out the Graph->First array.
    //

    Graph->First = (PVERTEX)Buffer;
    Buffer += FirstSizeInBytes;

    //
    // Carve out the Graph->Prev array.
    //

    Graph->Prev = (PVERTEX)Buffer;
    Buffer += PrevSizeInBytes;

    //
    // Carve out the Graph->Assigned array.
    //

    Graph->Assigned = (PVERTEX)Buffer;
    Buffer += AssignedSizeInBytes;

    //
    // Carve out the bitmap buffer for Graph->DeletedEdges.
    //

    Graph->DeletedEdges.Buffer = (PULONG)Buffer;
    Graph->DeletedEdges.SizeOfBitMap = TotalNumberOfEdges.LowPart;
    Buffer += BitmapBufferSizeInBytes.LowPart;

    //
    // Carve out the bitmap buffer for Graph->VisitedEdges.
    //

    Graph->VisitedEdges.Buffer = (PULONG)Buffer;
    Graph->VisitedEdges.SizeOfBitMap = TotalNumberOfEdges.LowPart;
    Buffer += BitmapBufferSizeInBytes.LowPart;

    //
    // If our pointer arithmetic was correct, Buffer should match the base
    // address of the graph plus the total allocation size at this point.
    // Assert this invariant now.
    //

    ExpectedBuffer = RtlOffsetToPointer(Graph, AllocSize.LowPart);
    ASSERT(Buffer == ExpectedBuffer);

    //
    // Graph structure has been allocated and initialized.
    //

    //
    // XXX TODO: continue implementation.
    //

    if (0) {

        //
        // (Silence the unreferenced label warnings whilst in development.)
        //

        goto End;
    }

    //
    // (Temporarily intentional follow-on to Error.)
    //

Error:

    Success = FALSE;

    //
    // N.B. We don't destroy the table here, that is the job of our caller
    //      (CreatePerfectHashTable()) if we return FALSE.  We do need to
    //      free the graph structure though if we allocated one by this point.
    //

    if (BaseAddress) {
        Allocator->FreePointer(Allocator->Context, &BaseAddress);
    }

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
