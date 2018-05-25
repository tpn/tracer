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
// Define the graph structure.  This represents an r-graph, and a hypergraph,
// and an r-partite 2-uniform graph, and any other seemingly unlimited number
// of names floating around in academia for what appears to be exactly the same
// thing.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _GRAPH {

    //
    // Reserve a Vtbl slot.
    //

    PPVOID Vtbl;

    //
    // Total allocation size for the graph and all backing structures.
    //

    ULARGE_INTEGER TotalAllocationSizeInBytes;

    //
    // Base address of the graph.  This is captured to facilitate fast
    // persistence of the entire structure to disk.
    //

    PVOID BaseAddress;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _GRAPH)) ULONG SizeOfStruct;

    //
    // Graph flags.
    //

    GRAPH_FLAGS Flags;

    //
    // (32 bytes consumed.)
    //

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
    // Pad out to 8 bytes.
    //

    ULONG Padding1;

    //
    // (64 bytes consumed.)
    //

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
    // (104 bytes consumed.)
    //

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
    // (128 bytes consumed.)
    //

    //
    // Capture the seeds used for each hash function employed by the graph.
    //

    ULONG Seed1;
    ULONG Seed2;

    //
    // (136 bytes consumed.)
    //

} GRAPH;
//C_ASSERT(sizeof(GRAPH) == 136);
typedef GRAPH *PGRAPH;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
