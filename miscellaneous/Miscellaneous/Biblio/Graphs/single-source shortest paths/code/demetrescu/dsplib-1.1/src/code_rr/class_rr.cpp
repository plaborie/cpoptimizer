//=========================================================================
//  class_rr.cpp
//=========================================================================

// Author:         Camil Demetrescu
// Change log:     CD000430
// Description:    class definition for class_rr


#include "class_rr.h"

#include <LEDA/queue.h>
#include <LEDA/stack.h>

//#define DEBUG_
//#define REFCOUNT_

#ifdef DEBUG_
#include <stdio.h>
#endif

#ifdef REFCOUNT_
    #define INCin_           mRefCountIN++;
    #define INCim_           mRefCountIM++;
    #define INCdn_           mRefCountDN++;
    #define INCdm_           mRefCountDM++;
#else
    #define INCim_
    #define INCin_
    #define INCdm_
    #define INCdn_
#endif


// =========================================================================
//  class_rr( GRAPH<int,int> &inGraph, int inSource  )
// =========================================================================
//  Class constructor

class_rr::class_rr( GRAPH<int,int> &inGraph, bool &outIsNegCycle, int inSource ){

    node                theInNode,
                        theNewNodeX,
                        theNewNodeY;
    edge                theInEdge;
    NodeInfo*           theNodeInfPtr;
    EdgeInfo*           theEdgeInfPtr;

    #ifdef REFCOUNT_
    mRefCountIN=mRefCountIM=
    mRefCountDN=mRefCountDM=
    mTotRefCountIN=mTotRefCountIM=
    mTotRefCountDN=mTotRefCountDM=
    mOpCountI=mOpCountD=0;
    #endif

    // Setup graph nodes
    forall_nodes(theInNode, inGraph) {

        // Create a new node info item
        theNodeInfPtr = new NodeInfo;
        if ( theNodeInfPtr == nil ) _ErrorHandler();

        // Setup node info and create new node in mGraph
        theNodeInfPtr->Node         = mGraph.new_node(theNodeInfPtr);
        theNodeInfPtr->ID           = inGraph.inf(theInNode);
        theNodeInfPtr->Delta        = 0;
        theNodeInfPtr->Dist         = MAXINT;
        theNodeInfPtr->Parent       = nil;
        theNodeInfPtr->Color        = _NoColor;
        theNodeInfPtr->PQueueItem   = 0;

        // Setup node label entry
        mNodeLabel[inGraph.inf(theInNode)] = theNodeInfPtr->Node;
    }

    // Initialize the adjacency matrix
    mAdjMatrix.init(mGraph);

    // Setup graph arcs
    forall_edges(theInEdge, inGraph) {

        // Create a new edge info item
        theEdgeInfPtr = new EdgeInfo;
        if ( theEdgeInfPtr == nil ) _ErrorHandler();

        // Fetch reference to new arc nodes
        theNewNodeX = mNodeLabel[inGraph.inf(inGraph.source(theInEdge))];
        theNewNodeY = mNodeLabel[inGraph.inf(inGraph.target(theInEdge))];

        // Setup edge info and create a new edge in mGraph
        theEdgeInfPtr->Edge          = mGraph.new_edge(theNewNodeX, theNewNodeY, theEdgeInfPtr);
        theEdgeInfPtr->Weight        = inGraph.inf(theInEdge);
        theEdgeInfPtr->SourceInItem  = 0;
        theEdgeInfPtr->SourceOutItem = 0;

        // Setup adjacency matrix
        mAdjMatrix(theNewNodeX, theNewNodeY) = theEdgeInfPtr->Edge;
    }

    // Setup root node
    mRootNode = mNodeLabel[inSource];

    // Compute initial solution
    outIsNegCycle = _BellmanFordMoore();
}


// =========================================================================
//  ~class_rr()
// =========================================================================
//  Class destructor

class_rr::~class_rr() {

    node theNode;
    edge theEdge;

    // Delete graph nodes info
    forall_nodes(theNode, mGraph) delete mGraph.inf(theNode);

    // Delete graph arcs info
    forall_edges(theEdge, mGraph) delete mGraph.inf(theEdge);
}


// PUBLIC METHODS

// =========================================================================
//  ChangeWeight( int inIdX, int inIdY, int inNewWeight )
// =========================================================================
//  Change the weight of arc (inIdX,inIdY) to inNewWeight
//  Return:
//      _OK             if operation was successful
//      _NegativeCycle  if a negative cycle was introduced by the operation
//      _InvalidEdge    if (inIdX,inIdY) is not a valid edge

int
class_rr::ChangeWeight( int inIdX, int inIdY, int inNewWeight ){

    edge    theEdge;
    int     theCurrentWeight;
    int     theResult = _OK;

    // get reference to the edge item
    theEdge = mAdjMatrix( mNodeLabel[inIdX], mNodeLabel[inIdY] );
    if ( theEdge == nil ) { return _InvalidEdge; cout << "INVALID EDGE\n"; }

    // fetch current edge weight
    theCurrentWeight = mGraph.inf(theEdge)->Weight;

    // switch proper operation
         if ( inNewWeight > theCurrentWeight ) {

             #ifdef REFCOUNT_
                 mRefCountIN=mRefCountIM=0;
             #endif

             theResult = _Increase( theEdge, inNewWeight );

             #ifdef REFCOUNT_
                 mTotRefCountIN += ( mRefCountIN + REFCNTPREC_/2 ) / REFCNTPREC_;
                 mTotRefCountIM += ( mRefCountIM + REFCNTPREC_/2 ) / REFCNTPREC_;
                 mOpCountI++;
             #endif
         }
    else if ( inNewWeight < theCurrentWeight ) {

             #ifdef REFCOUNT_
                 mRefCountDN=mRefCountDM=0;
             #endif

             theResult = _Decrease( theEdge, inNewWeight );

             #ifdef REFCOUNT_
                 mTotRefCountDN += ( mRefCountDN + REFCNTPREC_/2 ) / REFCNTPREC_;
                 mTotRefCountDM += ( mRefCountDM + REFCNTPREC_/2 ) / REFCNTPREC_;
                 mOpCountD++;
             #endif
         }

    return theResult;
}


// =========================================================================
//  GetDistance(int inNodeId)
// =========================================================================
//  Return: - node distance from mRootNode if inNodeId is valid,
//          - MAXINT otherwise

int
class_rr::GetDistance(int inNodeId) {
    if (mNodeLabel[inNodeId]==nil) return MAXINT;
    return mGraph.inf(mNodeLabel[inNodeId])->Dist;
}


// =========================================================================
//  GetParent(int inNodeId)
// =========================================================================
//  Return: - node parent ID of inNodeId if inNodeId is valid,
//          - MAXINT otherwise

int
class_rr::GetParent(int inNodeId) {
    if (mNodeLabel[inNodeId]==nil) return MAXINT;
    EdgeInfo* theParentEdge = mGraph.inf(mNodeLabel[inNodeId])->Parent;
    if ( theParentEdge == nil ) return MAXINT;
    return mGraph.inf(mGraph.source(theParentEdge->Edge))->ID;
}


// PRIVATE METHODS

// =========================================================================
//  _BellmanFordMoore()
// =========================================================================
//  Single source shortest paths from mRootNode using a queue
//  (breadth first search) computes for all nodes v:
//      a) cost of shortest path from mRootNode to v
//      b) predecessor edge of v in shortest paths tree

//  Preconditions:  - fields Dist and Parent of all node info items must be
//                    initialized to MAXINT and nil, respectively.
//                  - mRootNode must point to the shortest paths source
//  Returns:        - true if a negative cycle has been found,
//                  - false otherwise

inline bool
class_rr::_BellmanFordMoore() {

    int              theNumNodes,
                     theDistOfX,
                     theNewDistOfY;
    node_list        theQueue;
    node             theNodeX,
                     theNodeY;
    NodeInfo         *theNodeXInf,
                     *theNodeYInf;
    edge             theEdge;
    node_array<int>  theCount(mGraph,0);

    theNumNodes = mGraph.number_of_nodes();

    // Root node has distance zero from itself
    mGraph.inf(mRootNode)->Dist = 0;

    // Enqueue the root node
    theQueue.append(mRootNode);

    while( !theQueue.empty() ) {

        // Extract a node from the queue
        theNodeX = theQueue.pop();
        theNodeXInf = mGraph.inf(theNodeX);

        // Check for negative cycles
        if ( ++theCount[theNodeX] > theNumNodes ) return true;

        theDistOfX = theNodeXInf->Dist;

        // Scan adjacents
        forall_out_edges(theEdge, theNodeX) {

            theNodeY      = mGraph.target(theEdge);
            theNodeYInf   = mGraph.inf(theNodeY);
            theNewDistOfY = theDistOfX + mGraph.inf(theEdge)->Weight;

            // Look for improvement
            if ( theNewDistOfY < theNodeYInf->Dist ) {
                if (theNodeYInf->Parent != nil) mGraph.inf( mGraph.source( theNodeYInf->Parent->Edge ) ) ->
                                                Children.del_item(theNodeYInf->ParentChildItem);
                theNodeYInf->Dist            = theNewDistOfY;
                theNodeYInf->Parent          = mGraph.inf(theEdge);
                theNodeYInf->ParentChildItem = theNodeXInf->Children.push(theNodeYInf);
                if ( !theQueue.member(theNodeY) ) theQueue.append(theNodeY);
            }
        }
    }

    return false;
}


// =========================================================================
//  _SearchEquivalentPath( edge inEdgeXY, int inNewWeight )
// =========================================================================
//  Perform a backward DFS using only in-edges in the shortest paths dag and
//  look for an equivalent parent outside the tree T(y)

inline bool
class_rr::_SearchEquivalentPath( NodeInfo* inNodeInf ){

    queue<NodeInfo*>        theQueue;
    NodeInfo*               theNodeInf;
    NodeInfo*               theChildNodeInf;
    edge                    theParentEdge;
    EdgeInfo*               theParentEdgeInf;
    NodeInfo*               theParentNodeInf;

    if ( inNodeInf->Color == _NoColor ) return true;
    if ( inNodeInf->Color == _Red )     return false;
    if ( inNodeInf->Color == _Blue )    { cerr << "Fatal error: Zero cycle found\n"; exit(1); }

    #ifdef DEBUG_
        printf("Searching equivalent parent for node %d\n",inNodeInf->ID);
    #endif

    // Temporary color the current node
    inNodeInf->Color = _Blue;

    // Scan in-edges of the current node
    forall_in_edges (theParentEdge, inNodeInf->Node) {

        INCim_   // Count ***

        theParentEdgeInf = mGraph.inf(theParentEdge);
        theParentNodeInf = mGraph.inf(mGraph.source(theParentEdge));

        // Check for Bellman conditions
        if (theParentNodeInf->Dist + theParentEdgeInf->Weight == inNodeInf->Dist)
            if ( _SearchEquivalentPath(theParentNodeInf) ) goto Found;
    }

    inNodeInf->Color = _Red;         // The node has no equivalent parent => gets red color
    return false;

Found:

    #ifdef DEBUG_
        printf("Found equivalent parent for node %d\n",inNodeInf->ID);
    #endif

    // Switch parent of node inNodeInf
    if (inNodeInf->Parent != nil) mGraph.inf( mGraph.source( inNodeInf->Parent->Edge ) )->
                                  Children.del_item(inNodeInf->ParentChildItem);
    inNodeInf->Parent           = theParentEdgeInf;
    inNodeInf->ParentChildItem  = theParentNodeInf->Children.push(inNodeInf);

    // Uncolor subtree rooted at inNodeInf

    theQueue.append(inNodeInf);
    do {
        // Extract current node
        theNodeInf        = theQueue.pop();

        // Skip previously uncolored nodes
        if ( theNodeInf->Color == _NoColor ) continue;
        theNodeInf->Color = _NoColor;

        // Enqueue children
        forall ( theChildNodeInf, theNodeInf->Children ) theQueue.append(theChildNodeInf);
    } while (!theQueue.empty());

    return true;
}


// =========================================================================
//  _Increase( edge inEdgeXY, int inNewWeight )
// =========================================================================
//  Increase the weight of arc inEdge to inNewWeight
//  Precondition:  - inNewWeight > current weight of inEdgeXY

inline int
class_rr::_Increase( edge inEdgeXY, int inNewWeight ){

    node                    theNode;
    NodeInfo*               theNodeInf;
    NodeInfo*               theChildNodeInf;
    edge                    theParentEdge;
    EdgeInfo*               theParentEdgeInf;
    NodeInfo*               theParentNodeInf;
    EdgeInfo*               theBestParentEdgeInf;
    NodeInfo*               theBestParentNodeInf;
    EdgeInfo*               theEdgeXYInf = mGraph.inf( inEdgeXY );
    NodeInfo*               theNodeYInf  = mGraph.inf( mGraph.target( inEdgeXY ) );
    p_queue<int,NodeInfo*>  thePQueue;
    queue<NodeInfo*>        theQueue;
    int                     theEpsilon = inNewWeight - theEdgeXYInf->Weight;

    // Change the arc weight
    theEdgeXYInf->Weight = inNewWeight;

    // Return if non-modifying operation
    if ( theNodeYInf->Parent != theEdgeXYInf ) return _OK;

    // Color _Green nodes belonging to the shortest paths subtree rooted at node Y
    #ifdef DEBUG_
        printf("Color _Green nodes belonging to the shortest paths subtree rooted at node Y\n");
    #endif
    theQueue.append(theNodeYInf);
    do {
        theNodeInf        = theQueue.pop();
        theNodeInf->Color = _Green;

        forall ( theChildNodeInf, theNodeInf->Children ) theQueue.append(theChildNodeInf);
    } while (!theQueue.empty());

    #ifdef DEBUG_
    printf("Colors before searching equivalent paths:\n");
    { node n; forall_nodes(n,mGraph) printf("Color[%d]=%d\n",mGraph.inf(n)->ID,mGraph.inf(n)->Color); }
    #endif

    // Look for equivalent parents scanning only nodes in T(y)
    theQueue.append(theNodeYInf);
    do {

        // Extract node
        theNodeInf = theQueue.pop();

        // Skip uncolored nodes
        if ( theNodeInf->Color == _NoColor ) continue;

        // Look for an equivalent parent for the green node theNodeInf
        if ( theNodeInf->Color == _Green )
            if ( _SearchEquivalentPath( theNodeInf ) ) continue;

        // Append children of theNodeInf if they are still colored
        forall ( theChildNodeInf, theNodeInf->Children )
            if ( theChildNodeInf->Color != _NoColor ) theQueue.append(theChildNodeInf);

    } while (!theQueue.empty());

    #ifdef DEBUG_
    printf("Colors after searching equivalent paths:\n");
    { node n; forall_nodes(n,mGraph) printf("Color[%d]=%d\n",mGraph.inf(n)->ID,mGraph.inf(n)->Color); }
    #endif

    // Load priority queue with red nodes scanning only nodes in T(y)
    if ( theNodeYInf->Color == _Red ) theQueue.append(theNodeYInf);

    while (!theQueue.empty()) {

        // Extract node
        theNodeInf = theQueue.pop();

        // Look for the best uncolored parent
        theBestParentEdgeInf = nil;
        forall_in_edges (theParentEdge, theNodeInf->Node) {

            theParentEdgeInf = mGraph.inf(theParentEdge);
            theParentNodeInf = mGraph.inf(mGraph.source(theParentEdge));

            if ( theParentNodeInf->Color == _NoColor )
                if ( theBestParentEdgeInf == nil ? true :
                     theParentNodeInf    ->Dist + theParentEdgeInf    ->Weight <
                     theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight )
                         ( theBestParentEdgeInf = theParentEdgeInf , theBestParentNodeInf = theParentNodeInf );
        }

        // Compute delta for node theNodeInf
        #if !defined(INCREASE_DELTA_HEUR_)
        if ( theBestParentEdgeInf == nil) {
            theNodeInf->Delta      = MAXINT;
            theNodeInf->BestParent = nil;
        }
        #else
        if ( theBestParentEdgeInf == nil  || theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight - theNodeInf->Dist >= theEpsilon ) {
            theNodeInf->Delta      = theEpsilon;
            theNodeInf->BestParent = theNodeInf->Parent;
        }
        #endif
        else {
            theNodeInf->Delta      = theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight - theNodeInf->Dist;
            theNodeInf->BestParent = theBestParentEdgeInf;
        }

        #ifdef DEBUG_
            printf("Put node %d in the priority queue with priority %d and parent %d\n",
                    theNodeInf->ID, theNodeInf->Delta,
                    theNodeInf->BestParent != nil ? mGraph.inf(mGraph.source(theNodeInf->BestParent->Edge))->ID : MAXINT);
        #endif

        // Enqueue node theNodeInf in the priority queue with priority Delta
        theNodeInf->PQueueItem = thePQueue.insert( theNodeInf->Delta, theNodeInf );

        // Append children of theNodeInf if they are red
        forall ( theChildNodeInf, theNodeInf->Children )
            if (theChildNodeInf->Color == _Red) theQueue.append(theChildNodeInf);
    }

    // Update distances and shortest paths tree
    while (!thePQueue.empty()) {

        edge        theEdge;
        EdgeInfo*   theEdgeInf;
        int         theImprov;

        // Fetch the minimum priority node
        theNodeInf = thePQueue.inf(thePQueue.find_min());

        INCin_   // Count ***

        // Change the node distance and remove the node from the priority queue
        theNodeInf->Dist       += thePQueue.del_min();
        theNodeInf->Delta       = 0;
        theNodeInf->PQueueItem  = 0;

        // Uncolor red node
        theNodeInf->Color = _NoColor;

        #ifdef DEBUG_
            printf("Setting distance of node %d to %d\n",theNodeInf->ID,theNodeInf->Dist);
        #endif

        // Switch the parent arc of node h
        if (theNodeInf->Parent != theNodeInf->BestParent) {
            if (theNodeInf->Parent != nil) mGraph.inf( mGraph.source( theNodeInf->Parent->Edge ) )->
                                           Children.del_item(theNodeInf->ParentChildItem);
            theNodeInf->Parent           = theNodeInf->BestParent;
            theNodeInf->ParentChildItem  = mGraph.inf( mGraph.source( theNodeInf->BestParent->Edge ) )->
                                           Children.push(theNodeInf);
        }

        // Scan adjacents
        forall_out_edges( theEdge, theNodeInf->Node ) {

            INCim_   // Count ***

            theChildNodeInf = mGraph.inf( mGraph.target(theEdge) );

            // Filter only nodes still in the priority queue
            if (theChildNodeInf->PQueueItem != 0) {

                theEdgeInf = mGraph.inf( theEdge );
                theImprov  = theNodeInf->Dist + theEdgeInf->Weight - theChildNodeInf->Dist;

                // Look for improvement
                if ( theImprov < theChildNodeInf->Delta ) {
                    thePQueue.decrease_p( theChildNodeInf->PQueueItem, theImprov );
                    theChildNodeInf->Delta      = theImprov;
                    theChildNodeInf->BestParent = theEdgeInf;
                    #ifdef DEBUG_
                        printf("Improving delta of node %d to %d and giving it parent %d\n",
                                theChildNodeInf->ID,theChildNodeInf->Delta,theNodeInf->ID);
                    #endif
                }
            }
        }
    }

    return _OK;
}


// =========================================================================
//  _Decrease( edge inEdgeXY, int inNewWeight )
// =========================================================================
//  Decrease the weight of arc inEdgeXY to inNewWeight
//  Preconditions:  - inNewWeight < current weight of inEdgeXY
//                  - the operation must not introduce a negative cycle
//  Return:         - _OK if no negative cycle has been introduced
//                  - _NegativeCycle otherwise
//  Note:           in case of negative cycle detection the distances are
//                  invalid after the update operation

inline int
class_rr::_Decrease( edge inEdgeXY, int inNewWeight ){

    EdgeInfo*               theEdgeXYInf = mGraph.inf( inEdgeXY );
    NodeInfo*               theNodeXInf  = mGraph.inf( mGraph.source( inEdgeXY ) );
    NodeInfo*               theNodeYInf  = mGraph.inf( mGraph.target( inEdgeXY ) );
    p_queue<int,NodeInfo*>  thePQueue;

    // Change the arc weight
    theEdgeXYInf->Weight = inNewWeight;

    // Return if non-modifying operation
    if ( theNodeXInf->Dist + inNewWeight >= theNodeYInf->Dist ) return _OK;

    // Compute initial delta for node Y
    theNodeYInf->Delta = theNodeXInf->Dist + theEdgeXYInf->Weight - theNodeYInf->Dist;

    // Enqueue node Y with priority Delta
    theNodeYInf->PQueueItem = thePQueue.insert( theNodeYInf->Delta, theNodeYInf );

    // Switch the parent edge of node Y (if necessary)
    if ( theNodeYInf->Parent != theEdgeXYInf ) {
        if ( theNodeYInf->Parent != nil ) mGraph.inf( mGraph.source( theNodeYInf->Parent->Edge ) ) ->
                                          Children.del_item(theNodeYInf->ParentChildItem);
        theNodeYInf->Parent          = theEdgeXYInf;
        theNodeYInf->ParentChildItem = theNodeXInf->Children.push(theNodeYInf);
    }

    // Main loop
    while (!thePQueue.empty()) {

        edge        theEdgeZH;
        EdgeInfo*   theEdgeZHInf;
        NodeInfo*   theNodeZInf;
        NodeInfo*   theNodeHInf;

        // Fetch the minimum priority node
        theNodeZInf = thePQueue.inf(thePQueue.find_min());

        // Change the node distance and remove the node from the priority queue
        theNodeZInf->Dist       += thePQueue.del_min();
        theNodeZInf->Delta       = 0;
        theNodeZInf->PQueueItem  = 0;

        INCdn_   // Count ***

        #ifdef DEBUG_
            printf("Setting distance of node %d to %d\n",theNodeZInf->ID,theNodeZInf->Dist);
        #endif

        // Scan adjacents
        forall_out_edges( theEdgeZH, theNodeZInf->Node ) {

            theEdgeZHInf = mGraph.inf( theEdgeZH );
            theNodeHInf  = mGraph.inf( mGraph.target(theEdgeZH) );

            INCdm_   // Count ***

            // Look for improvement
            if ( theNodeZInf->Dist + theEdgeZHInf->Weight - theNodeHInf->Dist < theNodeHInf->Delta ) {

                // Look for negative cycle
                if ( theNodeHInf->Node == theNodeXInf->Node ) return _NegativeCycle;

                // Compute improved delta for node h
                theNodeHInf->Delta = theNodeZInf->Dist + theEdgeZHInf->Weight - theNodeHInf->Dist;

                // Enqueue the node h or improve it if it is already in the priority queue
                if ( theNodeHInf->PQueueItem ) thePQueue.decrease_p( theNodeHInf->PQueueItem, theNodeHInf->Delta );
                else theNodeHInf->PQueueItem = thePQueue.insert( theNodeHInf->Delta, theNodeHInf );

                // Switch the parent arc of node h (if necessary)
                if ( theNodeHInf->Parent != theEdgeZHInf ) {
                    if (theNodeHInf->Parent != nil) mGraph.inf( mGraph.source( theNodeHInf->Parent->Edge ) ) ->
                                                    Children.del_item(theNodeHInf->ParentChildItem);
                    theNodeHInf->Parent          = theEdgeZHInf;
                    theNodeHInf->ParentChildItem = theNodeZInf->Children.push(theNodeHInf);
                }
            }
        }
    }

    return _OK;
}

