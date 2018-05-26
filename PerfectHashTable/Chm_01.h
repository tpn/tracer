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

typedef struct _GRAPH_ITERATOR {
    VERTEX Vertex;
    EDGE Edge;
} GRAPH_ITERATOR;
typedef GRAPH_ITERATOR *PGRAPH_ITERATOR;

typedef union _VERTICES {
    struct {
        VERTEX V1;
        VERTEX V2;
    };
    LONGLONG AsLongLong;
    ULONGLONG AsULongLong;
} VERTICES;
typedef VERTICES *PVERTICES;

typedef union _EDGES {
    struct {
        EDGE E1;
        EDGE E2;
    };
    LONGLONG AsLongLong;
    ULONGLONG AsULongLong;
} EDGES;
typedef EDGES *PEDGES;

//
// N.B. I'm not sure why they don't use NULL/0 for the empty edge case.  Using
//      -1 means the edge array needs to be filled with -1s as part of graph
//      initialization, which seems inefficient and unnecessary.
//

#define EMPTY ((ULONG)-1)
#define IsEmpty(Value) ((ULONG)Value == EMPTY)
#define GRAPH_NO_NEIGHBOR ((ULONG)-1)
#define IsNeighborEmpty(Neighbor) ((ULONG)Neighbor == EMPTY)

//
// Define graph flags.
//

typedef union _GRAPH_FLAGS {
    struct {

        //
        // Indicates we've started deletion of edges from the graph.
        //

        ULONG Shrinking:1;

        //
        // When set, indicates the graph has been determined to be acyclic.
        // (This bit is set by IsGraphAcyclic() if the graph is acyclic; it
        //  is checked in GraphAssign().)
        //

        ULONG IsAcyclic:1;

        //
        // Unused bits.
        //

        ULONG Unused:30;
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
    // Deleted edges bitmap buffer size.
    //

    ULONGLONG DeletedEdgesBitmapBufferSizeInBytes;

    //
    // Visited vertices bitmap buffer size.
    //

    ULONGLONG VisitedVerticesBitmapBufferSizeInBytes;

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
    // Graph flags.
    //

    GRAPH_FLAGS Flags;

    //
    // Current edge being processed.
    //

    EDGE CurrentEdge;

    //
    // Inline the GRAPH_DIMENSIONS structure.  This is available from the
    // GRAPH_INFO structure, however, it's accessed frequently, so we inline
    // it to avoid the extra level of indirection.
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
    // Duplicate the context pointer.  (This is also available from Info.)
    //

    PPERFECT_HASH_TABLE_CONTEXT Context;

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
    // Bitmap used to capture deleted edges as part of the acyclic detection
    // stage.  The SizeOfBitMap will reflect TotalNumberOfEdges.
    //

    RTL_BITMAP DeletedEdges;

    //
    // Bitmap used to capture vertices visited as part of the assignment stage.
    // The SizeOfBitMap will reflect NumberOfVertices.
    //

    RTL_BITMAP VisitedVertices;

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

////////////////////////////////////////////////////////////////////////////////
// Algorithm Implementation Typedefs
////////////////////////////////////////////////////////////////////////////////

typedef
VOID
(NTAPI GRAPH_ADD_EDGE)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_ADD_EDGE *PGRAPH_ADD_EDGE;

typedef
VOID
(NTAPI GRAPH_CYCLIC_DELETE_EDGE)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex
    );
typedef GRAPH_CYCLIC_DELETE_EDGE *PGRAPH_CYCLIC_DELETE_EDGE;

typedef
BOOLEAN
(NTAPI GRAPH_FIND_DEGREE1_EDGE)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex,
    _Out_ PEDGE Edge
    );
typedef GRAPH_FIND_DEGREE1_EDGE *PGRAPH_FIND_DEGREE1_EDGE;

typedef
BOOLEAN
(NTAPI IS_GRAPH_ACYCLIC)(
    _In_ PGRAPH Graph
    );
typedef IS_GRAPH_ACYCLIC *PIS_GRAPH_ACYCLIC;

typedef
GRAPH_ITERATOR
(NTAPI GRAPH_NEIGHBORS_ITERATOR)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex
    );
typedef GRAPH_NEIGHBORS_ITERATOR *PGRAPH_NEIGHBORS_ITERATOR;

typedef
VERTEX
(NTAPI GRAPH_NEXT_NEIGHBOR)(
    _In_ PGRAPH Graph,
    _Inout_ PGRAPH_ITERATOR Iterator
    );
typedef GRAPH_NEXT_NEIGHBOR *PGRAPH_NEXT_NEIGHBOR;

typedef
EDGE
(NTAPI GRAPH_EDGE_ID)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_EDGE_ID *PGRAPH_EDGE_ID;

typedef
VOID
(NTAPI GRAPH_DELETE_EDGE)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_DELETE_EDGE *PGRAPH_DELETE_EDGE;

typedef
VOID
(NTAPI GRAPH_DELETE_VERTICES)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_DELETE_VERTICES *PGRAPH_DELETE_VERTICES;

typedef
BOOLEAN
(NTAPI GRAPH_CONTAINS_EDGE)(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_CONTAINS_EDGE *PGRAPH_CONTAINS_EDGE;

typedef
BOOLEAN
(NTAPI GRAPH_CHECK_EDGE)(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge,
    _In_ VERTEX Vertex1,
    _In_ VERTEX Vertex2
    );
typedef GRAPH_CHECK_EDGE *PGRAPH_CHECK_EDGE;

typedef
VOID
(NTAPI GRAPH_TRAVERSE)(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge
    );
typedef GRAPH_TRAVERSE *PGRAPH_TRAVERSE;

typedef
VOID
(NTAPI GRAPH_ASSIGN)(
    _In_ PGRAPH Graph
    );
typedef GRAPH_ASSIGN *PGRAPH_ASSIGN;

FORCEINLINE
EDGE
GetEdge(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge,
    _In_ BOOLEAN Index
    )
{
    ULONG NumberOfEdges = Graph->NumberOfEdges;
    return (Edge % NumberOfEdges + (Index * NumberOfEdges));
}

//
// Temporary debug helper that debugbreaks if we are passed an edge that
// exceeds the number of edges in the graph.  This is in support of determing
// the feasibility of a non-mod-based solution.
//
// N.B. This functionality corresponds to their abs_edge() macro, which seems
//      needlessly inefficient as its constantly mod'ing the input with the
//      number of edges.
//

FORCEINLINE
EDGE
GetEdgeDebug(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge,
    _In_ BOOLEAN Index
    )
{
    ULONG NumberOfEdges = Graph->NumberOfEdges;
    if (Edge >= NumberOfEdges) {
        __debugbreak();
    }
    return (Edge % NumberOfEdges + (Index * NumberOfEdges));
}

FORCEINLINE
EDGE
GetFirstEdgeDebug(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge
    )
{
    ULONG NumberOfEdges = Graph->NumberOfEdges;
    if (Edge >= NumberOfEdges) {
        __debugbreak();
    }
    return Edge % NumberOfEdges;
}

FORCEINLINE
EDGE
GetSecondEdgeDebug(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge
    )
{
    ULONG NumberOfEdges = Graph->NumberOfEdges;
    ULONG TotalNumberOfEdges = Graph->TotalNumberOfEdges;

    //
    // Edge should be within the range (NumberOfEdges .. TotalNumberOfEdges).
    //

    Edge += NumberOfEdges;

    if (Edge < NumberOfEdges) {
        __debugbreak();
    }

    if (Edge > TotalNumberOfEdges) {
        __debugbreak();
    }

    return Edge % NumberOfEdges;
}


#define TestGraphBit(Name, BitNumber) \
    BitTest((PLONG)Graph->##Name##.Buffer, BitNumber)

FORCEINLINE
BOOLEAN
IsDeletedEdge(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge
    )
{
    return TestGraphBit(DeletedEdges, Edge);
}

FORCEINLINE
BOOLEAN
IsVisitedVertex(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex
    )
{
    return TestGraphBit(VisitedVertices, Vertex);
}

#define SetGraphBit(Name, BitNumber) \
    BitTestAndSet((PLONG)Graph->##Name##.Buffer, BitNumber)

FORCEINLINE
VOID
RegisterEdgeDeletion(
    _In_ PGRAPH Graph,
    _In_ EDGE Edge
    )
{
    SetGraphBit(DeletedEdges, Edge);
}

FORCEINLINE
VOID
RegisterVertexVisit(
    _In_ PGRAPH Graph,
    _In_ VERTEX Vertex
    )
{
    SetGraphBit(VisitedVertices, Vertex);
}

FORCEINLINE
BOOLEAN
GraphCheckEdge(
    PGRAPH Graph,
    EDGE Edge,
    VERTEX Vertex1,
    VERTEX Vertex2
    )
{
    EDGE Edge1;
    EDGE Edge2;

    Edge1 = GetFirstEdgeDebug(Graph, Edge);
    Edge2 = GetSecondEdgeDebug(Graph, Edge);

    if (Graph->Edges[Edge1] == Vertex1 && Graph->Edges[Edge2] == Vertex2) {
        return TRUE;
    }

    if (Graph->Edges[Edge1] == Vertex2 && Graph->Edges[Edge2] == Vertex1) {
        return TRUE;
    }

    return FALSE;
}

FORCEINLINE
BOOLEAN
HashKey(
    _In_ PGRAPH Graph,
    _In_ ULONG Key,
    _Out_ PVERTEX Vertex1Pointer,
    _Out_ PVERTEX Vertex2Pointer
    )
{
    ULONG Vertex1;
    ULONG Vertex2;
    ULONG NumberOfEdges = Graph->NumberOfEdges;

    Vertex1 = _mm_crc32_u32(Graph->Seed1, Key) % NumberOfEdges;
    Vertex2 = _mm_crc32_u32(Graph->Seed2, Key) % NumberOfEdges;

    if (Vertex1 == Vertex2) {
        if (++Vertex2 >= NumberOfEdges) {
            Vertex2 = 0;
        }
    }

    if (Vertex1 == Vertex2) {
        return FALSE;
    }

    *Vertex1Pointer = Vertex1;
    *Vertex2Pointer = Vertex2;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
