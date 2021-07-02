//=========================================================================
//  class_dfmn.cpp
//=========================================================================

// Author:         Camil Demetrescu
// Change log:     CD000430
// Description:    class definition for class_dfmn


#include "class_dfmn.h"

#include <LEDA/queue.h>
#include <LEDA/stack.h>

//#define DEBUG_

//#define INCREASE_DELTA_HEUR_
//#define INCREASE_CD_VERSION_
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
//  class_dfmn( GRAPH<int,int> &inGraph, int inSource  )
// =========================================================================
//  Class constructor

class_dfmn::class_dfmn( GRAPH<int,int> &inGraph, bool &outIsNegCycle, int inSource ){

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
        theEdgeInfPtr->Edge = mGraph.new_edge(theNewNodeX, theNewNodeY, theEdgeInfPtr);
        theEdgeInfPtr->Weight = inGraph.inf(theInEdge);
        theEdgeInfPtr->Visited = false;

        // Setup adjacency matrix
        mAdjMatrix(theNewNodeX, theNewNodeY) = theEdgeInfPtr->Edge;
    }

    // Setup root node
    mRootNode = mNodeLabel[inSource];

    // Compute initial solution
    outIsNegCycle = _BellmanFordMoore();
}


// =========================================================================
//  ~class_dfmn()
// =========================================================================
//  Class destructor

class_dfmn::~class_dfmn() {

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
class_dfmn::ChangeWeight( int inIdX, int inIdY, int inNewWeight ){

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
class_dfmn::GetDistance(int inNodeId) {
    if (mNodeLabel[inNodeId]==nil) return MAXINT;
    return mGraph.inf(mNodeLabel[inNodeId])->Dist;
}


// =========================================================================
//  GetParent(int inNodeId)
// =========================================================================
//  Return: - node parent ID of inNodeId if inNodeId is valid,
//          - MAXINT otherwise

int
class_dfmn::GetParent(int inNodeId) {
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
class_dfmn::_BellmanFordMoore() {

    int             theNumNodes,
                    theDistOfX,
                    theNewDistOfY;
    node_list       theQueue;
    node            theNodeX,
                    theNodeY;
    NodeInfo        *theNodeXInf,
                    *theNodeYInf;
    edge            theEdge;
    node_array<int> theCount(mGraph,0);

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


#ifndef INCREASE_CD_VERSION_

// =========================================================================
//  _UncolorSubtree( NodeInfo* inEquivParent, list<NodeInfo*> &thruBlueList )
// =========================================================================
//  Uncolor all green and blue nodes reachable from inEquivParent and remove
//  them from the list of blue nodes. Setup new parents.

inline void
class_dfmn::_UncolorSubtree( NodeInfo* inEquivParent, list<NodeInfo*> &thruBlueList ){

    queue<NodeInfo*>        theQueue;
    NodeInfo*               theNodeInf;
    edge                    theEdge;

    theQueue.append(inEquivParent);
    do {

        // Extract node theNodeInf from the queue
        theNodeInf = theQueue.pop();

        // Scan its adjacents
        forall_out_edges( theEdge, theNodeInf->Node ) {

            INCim_   // Count ***

            EdgeInfo* theEdgeInf      = mGraph.inf( theEdge );
            NodeInfo* theChildNodeInf = mGraph.inf( mGraph.target(theEdge) );

            if ( (theChildNodeInf->Color == _Blue || theChildNodeInf->Color == _Green) &&
                 theNodeInf->Dist + theEdgeInf->Weight == theChildNodeInf->Dist) {

                if ( theChildNodeInf->Color == _Blue ) {

                    #ifdef DEBUG_
                        printf("Removing blue node %d from blue list\n",theChildNodeInf->ID);
                    #endif

                    // Remove blue adjacent from the list of blue nodes
                    thruBlueList.del_item(theChildNodeInf->BlueListItem);
                }

                // Switch the parent if necessary
                if ( theChildNodeInf->Parent != theEdgeInf ) {
                    #ifdef DEBUG_
                        printf("Switching its parent\n");
                    #endif
                    if (theChildNodeInf->Parent != nil) mGraph.inf( mGraph.source( theChildNodeInf->Parent->Edge ) )->
                                                            Children.del_item(theChildNodeInf->ParentChildItem);
                    theChildNodeInf->Parent           = theEdgeInf;
                    theChildNodeInf->ParentChildItem  = theNodeInf->Children.push(theChildNodeInf);
                }

                #ifdef DEBUG_
                    printf("Uncoloring node %d\n",theChildNodeInf->ID);
                #endif

                 // Uncolor it
                theChildNodeInf->Color = _NoColor;

                // Enqueue it
                theQueue.append(theChildNodeInf);
            }
        }

    } while (!theQueue.empty());
}


// =========================================================================
//  _Increase( edge inEdgeXY, int inNewWeight )
// =========================================================================
//  Increase the weight of arc inEdge to inNewWeight
//  Precondition:  - inNewWeight > current weight of inEdgeXY

inline int
class_dfmn::_Increase( edge inEdgeXY, int inNewWeight ){

    node                    theNode;
    NodeInfo*               theNodeInf;
    NodeInfo*               theChildNodeInf;
    edge                    theParentEdge;
    EdgeInfo*               theParentEdgeInf;
    NodeInfo*               theParentNodeInf;
    EdgeInfo*               theBestParentEdgeInf;
    NodeInfo*               theBestParentNodeInf;
    EdgeInfo*               theEdgeXYInf = mGraph.inf( inEdgeXY );
    NodeInfo*               theNodeXInf  = mGraph.inf( mGraph.source( inEdgeXY ) );
    NodeInfo*               theNodeYInf  = mGraph.inf( mGraph.target( inEdgeXY ) );
    p_queue<int,NodeInfo*>  thePQueue;
    queue<NodeInfo*>        theQueue;
    stack<NodeInfo*>        theStack;
    list<NodeInfo*>         theBlueList;
    list<NodeInfo*>         theRedList;

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
    { node n; forall_nodes(n,mGraph) printf("Color[%d]=%d\n",mGraph.inf(n)->ID,mGraph.inf(n)->Color); }
    #endif

    // Look for equivalent paths for nodes in T(Y)
    theQueue.append(theNodeYInf);
    do {

        // Extract a node theNodeInf from the queue
        theNodeInf = theQueue.pop();

        #ifdef DEBUG_
            printf("Extracted %d from queue\n",theNodeInf->ID);
        #endif

        // Skip non _Green nodes
        if ( theNodeInf->Color != _Green ) continue;

        // Color _Blue the _Green node theNodeInf and add it to the list of blue nodes
        theNodeInf->Color = _Blue;
        theNodeInf->BlueListItem = theBlueList.push(theNodeInf);

        // Look for equivalent paths for theNodeInf
        theStack.push(theNodeInf);
        do {

            // Fetch the top item
            theNodeInf = theStack.top();

            #ifdef DEBUG_
                printf("Fetched %d from the top of the stack\n",theNodeInf->ID);
            #endif

            // Look for the next best nonred neighbor
            theBestParentEdgeInf = nil;
            forall_in_edges (theParentEdge, theNodeInf->Node) {

                INCim_   // Count ***

                theParentEdgeInf = mGraph.inf(theParentEdge);
                theParentNodeInf = mGraph.inf(mGraph.source(theParentEdge));

                if ( ! theParentEdgeInf->Visited && theParentNodeInf->Color != _Red )
                    if ( theBestParentEdgeInf == nil ? true :
                         theParentNodeInf    ->Dist + theParentEdgeInf    ->Weight <
                         theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight )
                         ( theBestParentEdgeInf = theParentEdgeInf , theBestParentNodeInf = theParentNodeInf );
            }

            // If the best parent does not exist or it is not a candidate parent then pop the stack
            if ( theBestParentEdgeInf == nil ? true :
                 theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight > theNodeInf->Dist ) {
                #ifdef DEBUG_
                    printf("No Candidate parent found\n");
                #endif
                theStack.pop();
            }

            // Else the best parent is a candidate parent
            else {

                #ifdef DEBUG_
                    printf("Candidate parent %d found\n",theBestParentNodeInf->ID);
                #endif

                // Mark visited the candidate parent edge
                theBestParentEdgeInf->Visited = true;

                // The best parent is an equivalent parent (doesn't change distance)
                if ( theBestParentNodeInf->Color == _NoColor ) {

                    #ifdef DEBUG_
                        printf("It is an equivalent parent\n");
                    #endif

                    // Uncolor all blue and green nodes reachable from the equivalent parent
                    // Use only edges (x,y) s.t. d(x)+w(x,y)=d(y)
                    _UncolorSubtree(theBestParentNodeInf,theBlueList);

                    // Cleanup the stack
                    theStack.clear();
                    break;
                }

                #ifdef DEBUG_
                    printf("Can't tell if it's an equivalent parent\n");
                #endif

                // The best parent is not an equivalent parent: push it if not already blue
                if ( theBestParentNodeInf->Color != _Blue ) {
                    theStack.push(theBestParentNodeInf);
                    theBestParentNodeInf->Color = _Blue;
                    theBestParentNodeInf->BlueListItem = theBlueList.push(theBestParentNodeInf);

                    #ifdef DEBUG_
                        printf("Push %d\n",theBestParentNodeInf->ID);
                    #endif
                }
            }

        } while (!theStack.empty());

        #ifdef DEBUG_
            printf("Scanning remaining blue nodes ...\n");
            { node n; forall_nodes(n,mGraph) printf("Color[%d]=%d\n",mGraph.inf(n)->ID,mGraph.inf(n)->Color); }
        #endif

        // Scan remaining blue nodes, enqueue them in the priority queue and their children in the queue
        forall (theNodeInf, theBlueList) {

            edge        theParentEdge;

            // Color the node _Red
            theNodeInf->Color = _Red;

            // Add node to the list of Red nodes
            theRedList.push(theNodeInf);

            // Enqueue green children of theNodeInf in the queue
            forall ( theChildNodeInf, theNodeInf->Children ) {

                if ( theChildNodeInf->Color == _Green ) {

                    theQueue.append(theChildNodeInf);

                    #ifdef DEBUG_
                        printf("Enqueueing node %d\n",theChildNodeInf->ID);
                    #endif
                }
            }

            // Unmark ingoing edges
            forall_in_edges ( theParentEdge, theNodeInf->Node ) mGraph.inf(theParentEdge)->Visited=false;
        }

        // Cleanup the blue list
        theBlueList.clear();

    } while (!theQueue.empty());

    // Load priority queue scanning all red nodes
    forall (theNodeInf, theRedList) {

        // Look for the best nonred neighbor
        theBestParentEdgeInf = nil;
        forall_in_edges (theParentEdge, theNodeInf->Node) {

            INCim_   // Count ***

            theParentEdgeInf = mGraph.inf(theParentEdge);
            theParentNodeInf = mGraph.inf(mGraph.source(theParentEdge));

            if ( theParentNodeInf->Color != _Red )
                if ( theBestParentEdgeInf == nil ? true :
                     theParentNodeInf    ->Dist + theParentEdgeInf    ->Weight <
                     theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight )
                         ( theBestParentEdgeInf = theParentEdgeInf , theBestParentNodeInf = theParentNodeInf );
        }

        // Compute delta for node theNodeInf
        if ( theBestParentEdgeInf == nil )
             theNodeInf->Delta = MAXINT;
        else theNodeInf->Delta = theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight - theNodeInf->Dist;

        theNodeInf->BestParent = theBestParentEdgeInf;

        #ifdef DEBUG_
            printf("Put red node %d in the priority queue with priority %d\n",theNodeInf->ID,theNodeInf->Delta);
        #endif

        // Enqueue node theNodeInf in the priority queue with priority Delta
        theNodeInf->PQueueItem = thePQueue.insert( theNodeInf->Delta, theNodeInf );
    }

    // Update distances and shortest paths tree
    while (!thePQueue.empty()) {

        edge        theEdge;
        EdgeInfo*   theEdgeInf;
        int         theImprov;

        // Fetch the minimum priority node
        theNodeInf = thePQueue.inf(thePQueue.find_min());

        // Change the node distance and remove the node from the priority queue
        theNodeInf->Dist       += thePQueue.del_min();
        theNodeInf->Delta       = 0;
        theNodeInf->PQueueItem  = 0;

        // Uncolor red node
        theNodeInf->Color = _NoColor;

        INCin_   // Count ***

        #ifdef DEBUG_
            printf("Updating distance of node %d to %d\n",theNodeInf->ID,theNodeInf->Dist);
        #endif

        // Switch the parent arc of node h
        if (theNodeInf->Parent != nil) mGraph.inf( mGraph.source( theNodeInf->Parent->Edge ) )->
                                       Children.del_item(theNodeInf->ParentChildItem);
        theNodeInf->Parent           = theNodeInf->BestParent;
        theNodeInf->ParentChildItem  = mGraph.inf( mGraph.source( theNodeInf->BestParent->Edge ) )->
                                       Children.push(theNodeInf);

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

#else

// =========================================================================
//  _Increase( edge inEdgeXY, int inNewWeight )
// =========================================================================
//  Increase the weight of arc inEdge to inNewWeight
//  Precondition:  - inNewWeight > current weight of inEdgeXY

inline int
class_dfmn::_Increase( edge inEdgeXY, int inNewWeight ){

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

    #ifdef INCREASE_DELTA_HEUR_
    ( mGraph.inf( mGraph.source( inEdgeXY ) ) )->Delta = theEpsilon;
    #endif

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
    { node n; forall_nodes(n,mGraph) printf("Color[%d]=%d\n",mGraph.inf(n)->ID,mGraph.inf(n)->Color); }
    #endif

    // Load priority queue scanning all green nodes
    theQueue.append(theNodeYInf);
    do {

        // Extract green node
        theNodeInf = theQueue.pop();

        // Look for the best uncolored parent
        theBestParentEdgeInf = nil;
        forall_in_edges (theParentEdge, theNodeInf->Node) {

            INCim_   // Count ***

            theParentEdgeInf = mGraph.inf(theParentEdge);
            theParentNodeInf = mGraph.inf(mGraph.source(theParentEdge));

            if ( theParentNodeInf->Color == _NoColor )
                if ( theBestParentEdgeInf == nil ? true :
                     theParentNodeInf    ->Dist + theParentEdgeInf    ->Weight <
                     theBestParentNodeInf->Dist + theBestParentEdgeInf->Weight )
                         ( theBestParentEdgeInf = theParentEdgeInf , theBestParentNodeInf = theParentNodeInf );
        }

        #ifdef INCREASE_DELTA_HEUR_
        theEpsilon = ( mGraph.inf( mGraph.source( theNodeInf->Parent->Edge ) ) )->Delta;
        #endif

        #ifdef DEBUG_
            printf( "Delta upper bound for node %d = %d\n", theNodeInf->ID, theEpsilon );
        #endif

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

        // Append children of theNodeInf
        forall ( theChildNodeInf, theNodeInf->Children ) theQueue.append(theChildNodeInf);

    } while (!theQueue.empty());

    // Update distances and shortest paths tree
    while (!thePQueue.empty()) {

        edge        theEdge;
        EdgeInfo*   theEdgeInf;
        int         theImprov;

        // Fetch the minimum priority node
        theNodeInf = thePQueue.inf(thePQueue.find_min());

        // Change the node distance and remove the node from the priority queue
        theNodeInf->Dist       += thePQueue.del_min();
        theNodeInf->Delta       = 0;
        theNodeInf->PQueueItem  = 0;

        // Uncolor green node
        theNodeInf->Color = _NoColor;

        INCin_   // Count ***

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

    #ifdef INCREASE_DELTA_HEUR_
    ( mGraph.inf( mGraph.source( inEdgeXY ) ) )->Delta = 0;
    #endif

    return _OK;
}

#endif


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
class_dfmn::_Decrease( edge inEdgeXY, int inNewWeight ){

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

