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
    USHORT Index;
    PULONG Keys;
    PGRAPH Graph;
    PBYTE Buffer;
    BOOLEAN Success;
    USHORT PageSize;
    PVOID BaseAddress = NULL;
    ULONG WaitResult;
    GRAPH_INFO Info;
    PBYTE Unusable;
    BOOLEAN CaughtException;
    PALLOCATOR Allocator;
    HANDLE ProcessHandle;
    USHORT NumberOfGraphs;
    USHORT NumberOfPagesPerGraph;
    USHORT NumberOfGuardPages;
    USHORT TotalNumberOfPages;
    USHORT NumberOfBitmaps;
    ULONGLONG NextSizeInBytes;
    ULONGLONG PrevSizeInBytes;
    ULONGLONG FirstSizeInBytes;
    ULONGLONG EdgesSizeInBytes;
    ULONGLONG AssignedSizeInBytes;
    ULONGLONG TotalBufferSizeInBytes;
    ULONGLONG UsableBufferSizeInBytesPerBuffer;
    ULONGLONG ExpectedTotalBufferSizeInBytes;
    ULONGLONG ExpectedUsableBufferSizeInBytesPerBuffer;
    ULONGLONG GraphSizeInBytesIncludingGuardPage;
    ULARGE_INTEGER AllocSize;
    ULARGE_INTEGER NumberOfEdges;
    ULARGE_INTEGER NumberOfVertices;
    ULARGE_INTEGER TotalNumberOfEdges;
    ULARGE_INTEGER BitmapBufferSizeInBytes;
    PPERFECT_HASH_TABLE_CONTEXT Context = Table->Context;
    HANDLE Events[4];
    USHORT NumberOfEvents = ARRAYSIZE(Events);


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
    Context = Table->Context;

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
    // Capture the number of bitmaps here, where it's close to the two lines
    // above that indicate how many bitmaps we're dealing with.  The number
    // of bitmaps accounted for above should match this number.
    //

    NumberOfBitmaps = 2;

    //
    // Sanity check the size hasn't overflowed.
    //

    ASSERT(!AllocSize.HighPart);

    //
    // Calculate the number of pages required by each graph, then extrapolate
    // the number of guard pages and total number of pages.  We currently use
    // 4KB for the page size (i.e. we're not using large pages).
    //

    PageSize = PAGE_SIZE;
    NumberOfGraphs = (USHORT)Context->MaximumConcurrency;
    NumberOfPagesPerGraph = ROUND_TO_PAGES(AllocSize.LowPart);
    NumberOfGuardPages = (USHORT)Context->MaximumConcurrency;
    TotalNumberOfPages = NumberOfPagesPerGraph + NumberOfGuardPages;
    GraphSizeInBytesIncludingGuardPage = (
        (NumberOfPagesPerGraph * PageSize) + PageSize
    );

    //
    // Create multiple buffers separated by guard pages using a single call
    // to VirtualAllocEx().
    //

    Success = Rtl->CreateMultipleBuffers(Rtl,
                                         &ProcessHandle,
                                         PageSize,
                                         NumberOfGraphs,
                                         NumberOfPagesPerGraph,
                                         NULL,
                                         NULL,
                                         &UsableBufferSizeInBytesPerBuffer,
                                         &TotalBufferSizeInBytes,
                                         &BaseAddress);

    if (!Success) {
        return FALSE;
    }

    //
    // N.B. Subsequent errors must 'goto Error' at this point to ensure our
    //      cleanup logic kicks in.
    //

    //
    // Assert the sizes returned by the buffer allocation match what we're
    // expecting.
    //

    ExpectedTotalBufferSizeInBytes = TotalNumberOfPages * PageSize;
    ExpectedUsableBufferSizeInBytesPerBuffer = NumberOfPagesPerGraph * PageSize;

    ASSERT(TotalBufferSizeInBytes == ExpectedTotalBufferSizeInBytes);
    ASSERT(UsableBufferSizeInBytesPerBuffer ==
           ExpectedUsableBufferSizeInBytesPerBuffer);

    //
    // Initialize the GRAPH_INFO structure with all the sizes captured earlier.
    // (We zero it first just to ensure any of the padding fields are cleared.)
    //

    ZeroStruct(Info);

    Info.PageSize = PageSize;
    Info.AllocSize = AllocSize.QuadPart;
    Info.Context = Context;
    Info.BaseAddress = BaseAddress;
    Info.NumberOfPagesPerGraph = NumberOfPagesPerGraph;
    Info.NumberOfGraphs = NumberOfGraphs;
    Info.NumberOfBitmaps = NumberOfBitmaps;
    Info.SizeOfGraphStruct = sizeof(GRAPH);
    Info.EdgesSizeInBytes = EdgesSizeInBytes;
    Info.NextSizeInBytes = NextSizeInBytes;
    Info.FirstSizeInBytes = FirstSizeInBytes;
    Info.PrevSizeInBytes = PrevSizeInBytes;
    Info.AssignedSizeInBytes = AssignedSizeInBytes;
    Info.BitmapBufferSizeInBytes = BitmapBufferSizeInBytes.QuadPart;
    Info.AllocSize = AllocSize.QuadPart;
    Info.FinalSize = UsableBufferSizeInBytesPerBuffer;

    //
    // Copy the dimensions over.
    //

    Info.NumberOfEdges = NumberOfEdges.LowPart;
    Info.TotalNumberOfEdges = TotalNumberOfEdges.LowPart;
    Info.NumberOfVertices = NumberOfVertices.LowPart;

    //
    // Set the context work callback to our worker routine, and the algo
    // context to our graph info structure.
    //

    Context->MainCallback = ProcessGraphCallbackChm01;
    Context->AlgorithmContext = &Info;

    //
    // We're ready to create threadpool work for the graph.
    //

    Buffer = (PBYTE)BaseAddress;
    Unusable = Buffer;

    for (Index = 0; Index < NumberOfGraphs; Index++) {

        //
        // Verify the guard page is working properly.
        //

        Unusable += UsableBufferSizeInBytesPerBuffer;

        CaughtException = FALSE;

        TRY_PROBE_MEMORY {

            *Unusable = 1;

        } CATCH_EXCEPTION_ACCESS_VIOLATION {

            CaughtException = TRUE;

        }

        ASSERT(CaughtException);

        //
        // Guard page is working properly.  Buffer points to the base address
        // for this graph.
        //

        //
        // Advance the buffer past the graph size and guard page.
        //

        Buffer += GraphSizeInBytesIncludingGuardPage;

    }

    //
    // Wait on the context's events.
    //

    Events[0] = Context->SucceededEvent;
    Events[1] = Context->CompletedEvent;
    Events[2] = Context->ShutdownEvent;
    Events[3] = Context->FailedEvent;

    WaitResult = WaitForMultipleObjects(NumberOfEvents,
                                        Events,
                                        FALSE,
                                        INFINITE);

    //
    // Ignore the wait result; determine if the graph solving was successful
    // by the finished count of the context.
    //

    Success = (Context->FinishedCount > 0);

    if (BaseAddress) {

        Rtl->DestroyBuffer(Rtl, ProcessHandle, &BaseAddress);

    }

    return Success;
}

//
// The entry point into the actual per-thread solving attempts is the following
// routine.
//

_Use_decl_annotations_
VOID
ProcessGraphCallbackChm01(
    PPERFECT_HASH_TABLE_CONTEXT Context,
    PSLIST_ENTRY ListEntry
    )
{
    PRTL Rtl;
    PGRAPH Graph;
    PGRAPH_INFO Info;
    PFILL_PAGES FillPages;

    //
    // Resolve the graph base address from the list entry.  Nothing will be
    // filled in initially.
    //

    Graph = CONTAINING_RECORD(ListEntry, GRAPH, ListEntry);

    //
    // Resolve aliases.
    //

    Rtl = Context->Rtl;
    FillPages = Rtl->FillPages;

    //
    // The graph info structure will be stashed in the algo context field.
    //

    Info = (PGRAPH_INFO)Context->AlgorithmContext;

    //
    // Begin the solving loop.  InitializeGraph() generates new seed data,
    // so each loop iteration will be attempting to solve the graph uniquely.
    //

    while (ShouldWeContinueTryingToSolveGraph(Context)) {

        InitializeGraph(Info, Graph);

        if (SolveGraph(Graph)) {

            //
            // Hey, we were the ones to solve it, great!
            //

            break;
        }

        //
        // Our attempt at solving failed.  Zero all pages associated with the
        // graph and then try again with new seed data.
        //

        FillPages((PCHAR)Graph, 0, Info->NumberOfPagesPerGraph);

    }

    return;
}

_Use_decl_annotations_
VOID
InitializeGraph(
    PGRAPH_INFO Info,
    PGRAPH Graph
    )
{
    PBYTE Buffer;
    PBYTE ExpectedBuffer;
    USHORT BitmapCount = 0;

    //
    // Obtain new seed data.
    //

    GetRandomSeedsBlocking(&Graph->Seeds);

    //
    // Carve out the backing memory structures for arrays and bitmap buffers.
    // Use the PBYTE Buffer here to make pointer arithmetic a tad easier.  We
    // initialize it to the start of the memory immediately following the graph
    // structure, and then bump it each time we carve out an array or bitmap
    // buffer.
    //

    ASSERT(sizeof(*Graph) == Info->SizeOfGraphStruct);
    Buffer = RtlOffsetToPointer(Graph, sizeof(*Graph));

    //
    // Carve out the Graph->Edges array.
    //

    Graph->Edges = (PEDGE)Buffer;
    Buffer += Info->EdgesSizeInBytes;

    //
    // Carve out the Graph->Next array.
    //

    Graph->Next = (PEDGE)Buffer;
    Buffer += Info->NextSizeInBytes;

    //
    // Carve out the Graph->First array.
    //

    Graph->First = (PVERTEX)Buffer;
    Buffer += Info->FirstSizeInBytes;

    //
    // Carve out the Graph->Prev array.
    //

    Graph->Prev = (PVERTEX)Buffer;
    Buffer += Info->PrevSizeInBytes;

    //
    // Carve out the Graph->Assigned array.
    //

    Graph->Assigned = (PVERTEX)Buffer;
    Buffer += Info->AssignedSizeInBytes;

    //
    // Carve out the bitmap buffer for Graph->DeletedEdges.
    //

    Graph->DeletedEdges.Buffer = (PULONG)Buffer;
    Graph->DeletedEdges.SizeOfBitMap = Info->TotalNumberOfEdges;
    Buffer += Info->BitmapBufferSizeInBytes;
    BitmapCount++;

    //
    // Carve out the bitmap buffer for Graph->VisitedEdges.
    //

    Graph->VisitedEdges.Buffer = (PULONG)Buffer;
    Graph->VisitedEdges.SizeOfBitMap = Info->TotalNumberOfEdges;
    Buffer += Info->BitmapBufferSizeInBytes;
    BitmapCount++;

    //
    // Verify we visited the number of bitmaps we were expecting to visit.
    //

    ASSERT(Info->NumberOfBitmaps == BitmapCount);

    //
    // If our pointer arithmetic was correct, Buffer should match the base
    // address of the graph plus the total allocation size at this point.
    // Assert this invariant now.
    //

    ExpectedBuffer = RtlOffsetToPointer(Graph, Info->AllocSize);
    ASSERT(Buffer == ExpectedBuffer);

    //
    // Set the current thread ID and capture an attempt number from context.
    // Save the info address.
    //

    Graph->ThreadId = GetCurrentThreadId();
    Graph->Attempt = InterlockedIncrement(&Info->Context->Attempts);
    Graph->Info = Info;

    //
    // Initialization complete!
    //

    return;
}

_Use_decl_annotations_
BOOLEAN
ShouldWeContinueTryingToSolveGraph(
    PPERFECT_HASH_TABLE_CONTEXT Context
    )
{
    ULONG WaitResult;
    HANDLE Events[] = {
        Context->ShutdownEvent,
        Context->SucceededEvent,
        Context->FailedEvent,
        Context->CompletedEvent,
    };
    USHORT NumberOfEvents = ARRAYSIZE(Events);

    //
    // N.B. We should probably switch this to simply use volatile field of the
    //      context structure to indicate whether or not the context is active.
    //      WaitForMultipleObjects() on four events seems a bit... excessive.
    //

    WaitResult = WaitForMultipleObjects(NumberOfEvents,
                                        Events,
                                        FALSE,
                                        0);

    //
    // The only situation where we continue attempting to solve the graph is
    // if the result from the wait is WAIT_TIMEOUT, which indicates none of
    // the events have been set.  We treat any other situation as an indication
    // to stop processing.  (This includes wait failures and abandonment.)
    //

    return (WaitResult == WAIT_TIMEOUT ? TRUE : FALSE);
}

_Use_decl_annotations_
BOOLEAN
SolveGraph(PGRAPH Graph)
{
    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
