//=========================================================================
//  class_dfmn.h
//=========================================================================

// Author:         Camil Demetrescu
// Change log:     CD000430
// Description:    class header for class_fmns


#ifndef __class_dfmn__
#define __class_dfmn__


// Includes

#include <LEDA/graph.h>
#include <LEDA/node_matrix.h>
#include <LEDA/h_array.h>
#include <LEDA/p_queue.h>


// Macros

// If REFCOUNT_ is defined, the class is able to provide node/arc references count
#define REFCOUNT_

#ifdef REFCOUNT_
    #define REFCNTPREC_     1
#endif

// Class declaration

class class_dfmn {

public:

    // Result code for method invocations
    enum ResultType {
        _OK,
        _NegativeCycle,
        _InvalidEdge
    };

    // Information associated to arcs
    struct EdgeInfo {
        edge                    Edge;               // LEDA edge reference
        int                     Weight;             // edge weight
        bool                    Visited;            // edge marking
    };

    // Information associated to nodes
    struct NodeInfo {
        node                    Node;               // LEDA node reference
        int                     ID;                 // node ID
        int                     Delta;              // computed distance variation during both _Increase and _Decrease
        int                     Dist;               // current distance from the source
        EdgeInfo*               Parent;             // parent edge info in the shortest paths tree
        list<struct NodeInfo*>  Children;           // list of children in the shortest paths tree
        pq_item                 PQueueItem;         // reference to the priority queue item the node belongs to
                                                    // 0 if the node is not enqueued
        list_item               ParentChildItem;    // node list item in the children list of the parent (if any)
        list_item               BlueListItem;       // node list item in the list of blue nodes (_Increase only)
        EdgeInfo*               BestParent;         // best parent edge info (_Increase only)
        int                     Color;              // node marking (_Increase only)
    };

                                // Class constructor and destructor
                                class_dfmn(GRAPH<int,int> &inGraph, bool &outIsNegCycle, int inSource=0);
                               ~class_dfmn();

                                // Public methods
    int                         ChangeWeight(int inIdX, int inIdY, int inNewWeight);
    int                         GetDistance(int inId);
    int                         GetParent(int inId);

private:

    // Node colors
    enum NodeColor {
        _NoColor,               // the node is unmarked
        _Green,                 // the node belongs to a subtree (temporary color)
        _Blue,                  // the node is marked during equivalent path search (temporary color)
        _Red                    // the node will change its distance from the source
    };

                                // Private methods
    inline int                  _Increase(edge inEdge, int inEdgeXY);
    inline int                  _Decrease(edge inEdge, int inEdgeXY);

    inline bool                 _BellmanFordMoore();
    inline void                 _UncolorSubtree(NodeInfo* inEquivParent, list<NodeInfo*> &thruBlueList);
    int                         _ErrorHandler() { cerr << "Internal error\n"; exit(1); }

public:
                                // Data members
    node                        mRootNode;                  // shortest paths source
    GRAPH<NodeInfo*,EdgeInfo*>  mGraph;                     // directed graph instance
    node_matrix<edge>           mAdjMatrix;                 // directed graph adjacency matrix
    h_array<int,node>           mNodeLabel;                 // map from node labels to nodes

    #ifdef REFCOUNT_
    unsigned long               mRefCountIN;
    unsigned long               mRefCountIM;
    unsigned long               mRefCountDN;
    unsigned long               mRefCountDM;
    unsigned long               mTotRefCountIN;
    unsigned long               mTotRefCountIM;
    unsigned long               mTotRefCountDN;
    unsigned long               mTotRefCountDM;
    unsigned long               mOpCountI;
    unsigned long               mOpCountD;
    #endif
};

#endif
