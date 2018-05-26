/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Chm_01.h

Abstract:

    This is the header file for the Chm_01.c module, which is our first pass
    at the CHM perfect hash table algorithm.  It defines types related to the
    implementation of the CHM algorithm.

--*/

#include "stdafx.h"

//
// Define the primitive edge and vertex structures.
//

typedef ULONG EDGE;
typedef ULONG VERTEX;
typedef EDGE *PEDGE;
typedef VERTEX *PVERTEX;

//
// Define graph flags.  Currently unused.
//

typedef union _GRAPH_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} GRAPH_FLAGS;
typedef GRAPH_FLAGS *PGRAPH_FLAGS;

//
// Define the primary dimensions governing the graph size.
//

typedef struct _GRAPH_DIMENSIONS {

    //
    // Number of edges in the graph.  This corresponds to the number of keys
    // in our input set.
    //

    ULONG NumberOfEdges;

    //
    // Total number of edges in the graph.  This will be twice the size of the
    // NumberOfEdges value above, due to the quirky way the underlying r-graph
    // algorithm captures two hash values in the same list and offsets the
    // second set after the first.
    //

    ULONG TotalNumberOfEdges;

    //
    // Number of vertices in the graph.  This is currently calculated by taking
    // the number of edges and multiplying it by 2.5.
    //
    // N.B. The chm.c algorithm uses 2.09 + doubles and ceils().  If we use
    //      2.5 we can calculate that with bit shifts, thus avoiding the need
    //      to link with a math library.
    //

    ULONG NumberOfVertices;

    //
    // Pad out to an 8 byte boundary.
    //

    ULONG Unused;

} GRAPH_DIMENSIONS;
typedef GRAPH_DIMENSIONS *PGRAPH_DIMENSIONS;

//
// Define various memory offsets associated with a given graph structure.
// This allows parallel worker threads to reset their local graph copy back
// to the initial state each time they want to try a new random seed.
//

typedef struct _GRAPH_INFO {

    //
    // Number of pages consumed by the entire graph and all backing arrays.
    //

    ULONG NumberOfPagesPerGraph;

    //
    // Page size (e.g. 4096, 2MB).
    //

    ULONG PageSize;

    //
    // Total number of graphs created.  This will match the maximum concurrency
    // level of the upstream context.
    //

    ULONG NumberOfGraphs;

    //
    // Number of RTL_BITMAP structures used by the graph.
    //

    USHORT NumberOfBitmaps;

    //
    // Size of the GRAPH structure.
    //

    USHORT SizeOfGraphStruct;

    //
    // Inline the GRAPH_DIMENSIONS structure.
    //

    union {

        struct {
            ULONG NumberOfEdges;
            ULONG TotalNumberOfEdges;
            ULONG NumberOfVertices;
            ULONG Unused;
        };

        GRAPH_DIMENSIONS Dimensions;
    };

    //
    // Pointer to the owning context.
    //

    PPERFECT_HASH_TABLE_CONTEXT Context;

    //
    // Base address of the entire graph allocation.
    //

    union {
        PVOID BaseAddress;
        struct _GRAPH *FirstGraph;
    };

    //
    // Array sizes.
    //

    ULONGLONG EdgesSizeInBytes;
    ULONGLONG NextSizeInBytes;
    ULONGLONG FirstSizeInBytes;
    ULONGLONG PrevSizeInBytes;
    ULONGLONG AssignedSizeInBytes;

    //
    // Bitmap buffer size.
    //

    ULONGLONG BitmapBufferSizeInBytes;

    //
    // The allocation size of the graph, including structure size and all
    // array and bitmap buffer sizes.
    //

    ULONGLONG AllocSize;

    //
    // Allocation size rounded up to the nearest page size multiple.
    //

    ULONGLONG FinalSize;

} GRAPH_INFO;
typedef GRAPH_INFO *PGRAPH_INFO;

//
// Define the graph structure.  This represents an r-graph, and a hypergraph,
// and an r-partite 2-uniform graph, and any other seemingly unlimited number
// of names floating around in academia for what appears to be exactly the same
// thing.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _GRAPH {

    //
    // List entry used to push the graph onto the context's work list.
    //

    SLIST_ENTRY ListEntry;

    //
    // Pointer to the info structure describing various sizes.
    //

    PGRAPH_INFO Info;

    //
    // Graph attempt.  This ID is derived from an interlocked increment against
    // Context->Attempts.
    //

    ULONG Attempt;

    //
    // Thread ID of the thread that owns us.  Each callback thread is provided
    // a single graph, and will attempt to solve the perfect hash table until
    // told otherwise.  Thus, there's a 1:1 relationship between graph instance
    // and owning thread.
    //

    ULONG ThreadId;

    //
    // Edges array.  The number of elements in this array is governed by the
    // TotalNumberOfEdges field, and will be twice the number of edges.
    //

    PEDGE Edges;

    //
    // Array of the "next" edge array, as per the referenced papers.  The number
    // of elements in this array is also governed by TotalNumberOfEdges.
    //

    PEDGE Next;

    //
    // Array of vertices.
    //

    PVERTEX First;

    //
    // The original CHM paper in 1996 references a "prev" array to "facilitate
    // fast deletion".  However, the chmp project appears to have switched to
    // using bitmaps.  Let's reserve a slot for the "prev" array anyway.
    //

    PVERTEX Prev;

    //
    // Array of assigned vertices.  Number of elements is governed by the
    // NumberOfVertices field.
    //

    PVERTEX Assigned;

    //
    // Bitmap used to capture "deleted" edges.  The SizeOfBitMap will reflect
    // TotalNumberOfEdges.
    //

    RTL_BITMAP DeletedEdges;

    //
    // Bitmap used to capture "deleted" edges.  The SizeOfBitMap will reflect
    // TotalNumberOfEdges.
    //

    RTL_BITMAP VisitedEdges;

    //
    // Capture the seeds used for each hash function employed by the graph.
    //

    union {
        struct {
            ULONG Seed1;
            ULONG Seed2;
        };
        ULARGE_INTEGER Seeds;
    };

} GRAPH;
typedef GRAPH *PGRAPH;

PERFECT_HASH_TABLE_WORK_CALLBACK ProcessGraphCallbackChm01;

typedef
VOID
(NTAPI INITIALIZE_GRAPH)(
    _In_ PGRAPH_INFO Info,
    _In_ PGRAPH Graph
    );
typedef INITIALIZE_GRAPH *PINITIALIZE_GRAPH;

typedef
BOOLEAN
(NTAPI SOLVE_GRAPH)(
    _In_ PGRAPH Graph
    );
typedef SOLVE_GRAPH *PSOLVE_GRAPH;

typedef
BOOLEAN
(NTAPI SHOULD_WE_CONTINUE_TRYING_TO_SOLVE_GRAPH)(
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context
    );
typedef SHOULD_WE_CONTINUE_TRYING_TO_SOLVE_GRAPH
      *PSHOULD_WE_CONTINUE_TRYING_TO_SOLVE_GRAPH;


SOLVE_GRAPH SolveGraph;
INITIALIZE_GRAPH InitializeGraph;
SHOULD_WE_CONTINUE_TRYING_TO_SOLVE_GRAPH ShouldWeContinueTryingToSolveGraph;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
