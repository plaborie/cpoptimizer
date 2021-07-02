//=========================================================================
//  run_bfm.c
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD000430
// Description:     Bellman-Ford-Moore algorithm driver


#include<LEDA/stream.h>
#include<LEDA/graph.h>
#include<LEDA/h_array.h>
#include<LEDA/node_matrix.h>

#include<stdio.h>
#include<stdlib.h>

#include"util/utility.h"
#include"_bellman_ford.h"


// Macros

#define MORE_SCREEN_DATA_
// #define DEBUG_

#define FILTER_BFM_ACTIVATION_


// FUNCTION DEFINITIONS

//=========================================================================
//  main
//=========================================================================

// argv[0]: this program name
// argv[1]: source node
// argv[2]: input graph file name
// argv[3]: input sequence file name
// argv[4]: results file name
// argv[5]: min weight
// argv[6]: max weight
// argv[7]: seed
// argv[8]: options - m=modifying sequence (bfm must be always called)

int main(int argc, char *argv[]){

    int            theNumNodes,
                   theNumEdges,
                   theSource,
                   theNumOp,
                   theMinWeight,
                   theMaxWeight,
                   theSeed,
                   theDecrOpCount,
                   theIncrOpCount,
                   theTotTrivial;
    GRAPH<int,int> theGraph;
    bool           isModifyingSeq;
    float          theTotInitTime,
                   theTotDecrTime,
                   theTotIncrTime,
                   theTotTime,
                   theAverDecrOpTime,
                   theAverIncrOpTime,
                   theMaxDecrOpTime,
                   theMaxIncrOpTime,
                   theRefTime,
                   theOpRefTime,
                   theOpTime,
                   theAverNodeExtr,
                   theAverEdgeRef;

    // Check argument count
    if ( argc == 1 ) {
        cerr << argv[0] << ": no argument given.\n";
        return 1;
    }
    if ( argc < 8 ) {
        cerr << argv[0] << ": wrong argument number.\n";
        return 1;
    }

    // Load graph
    switch (theGraph.read(argv[2])) {
        case 1: cerr <<"ERROR: file "<<argv[2]<<" not found\n"; return 0;
        case 2: cerr <<"ERROR: graph type doesn't match\n"; return 0;
    }

    // Variables initialization
    theSource        = atoi(argv[1]);
    theMinWeight     = atoi(argv[5]);
    theMaxWeight     = atoi(argv[6]);
    theSeed          = atoi(argv[7]);
    isModifyingSeq   = argc>8 ? strchr(argv[8],'m')!=NULL : false;
    theNumNodes      = theGraph.number_of_nodes();
    theNumEdges      = theGraph.number_of_edges();
    theMaxDecrOpTime =
    theMaxIncrOpTime =
    theTotDecrTime   =
    theTotIncrTime   = 0.0;
    theTotTrivial    =
    theDecrOpCount   =
    theIncrOpCount   = 0;

    #ifdef DEBUG_
        cerr << "\nGraph: " << argv[2]     << "\n";
        cerr << "# nodes: " << theNumNodes << "\n";
        cerr << "# edges: " << theNumEdges << "\n";
        cerr << "Source: "  << theSource   << "\n\n";
    #endif

    // Initialization time start
    theRefTime=stop_watch();

    // Initialize cost, dist and parent arrays
    edge_array<int> & theCost=theGraph.edge_data();
    node_array<int>   theDist(theGraph);
    node_array<edge>  thePred(theGraph);
    h_array<int,node> theNodeLabel;
    node              theNode;
    edge              theEdge;
    node_matrix<edge> theAdjMatrix(theGraph);

    // Create a mapping between node labels and nodes
    forall_nodes(theNode,theGraph) {
        theNodeLabel[theGraph.inf(theNode)]=theNode;
        #ifdef DEBUG_
            cerr << "Node " << theGraph.inf(theNode) << endl;
        #endif
    }

    // Create a mapping between node pairs and edges
    forall_edges(theEdge,theGraph) {

        theAdjMatrix(theGraph.source(theEdge),theGraph.target(theEdge))=theEdge;

        #ifdef DEBUG_
            cout << "Edge " << theGraph.inf(theGraph.source(theEdge)) << " "
                            << theGraph.inf(theGraph.target(theEdge)) << " "
                            << theGraph.inf(theEdge) << endl;
        #endif
    }

    // Compute initial solution
    if (!BELLMAN_FORD(theGraph, theNodeLabel[theSource], theCost, theDist, thePred)) {
        cerr << "Cycle found: wrong input graph\n";
        exit(1);
    }

    // Reset reference counters
    #ifdef REFCOUNT_
    gTotRefCountN = gTotRefCountM = 0;
    #endif

    // Initialization time stop
    theTotInitTime = stop_watch(theRefTime);

    #ifdef DEBUG_
        forall_nodes(theNode,theGraph) { cerr << "Dist[" << theGraph.inf(theNode) << "]==" << theDist[theNode] << endl; }
    #endif

    // Sequence-related declarations
    file_istream I(argv[3]);
    string       theDummyStr;
    int          theDummyInt,
                 theNode1,
                 theNode2,
                 theWeight;
    char         theOpType;

    // Read sequence file header
    I >> theDummyStr >> theDummyStr;
    I >> theDummyInt;
    I >> theNumOp;
    I >> theDummyInt >> theDummyInt;

    // Main updates loop
    for (int i=0; i<theNumOp; i++) {

        // Read operation from file
        I >> theNode1 >> theNode2 >> theWeight >> theOpType;

        // Operation time start
        theOpRefTime = stop_watch();

        // Perform operation over the graph
        node theXn=theNodeLabel[theNode1];
        node theYn=theNodeLabel[theNode2];
        edge theEdge=theAdjMatrix(theXn, theYn);

        if (theEdge==nil) cout << '\07';

        #ifdef DEBUG_
            cout << "Changing the weight of " << theGraph.inf(theGraph.source(theEdge))
                 << " "                       << theGraph.inf(theGraph.target(theEdge))
                 << " from "                  << theGraph.inf(theEdge)
                 << " to "                    << theWeight
                 << endl;
        #endif

        int theOldWeight=theGraph.inf(theEdge);
        theGraph.assign(theEdge,theWeight);

        #ifdef FILTER_BFM_ACTIVATION_
        if (isModifyingSeq || theDist[theXn]+theWeight<theDist[theYn] ||        // effective decrease
            theDist[theXn]+theWeight>theDist[theYn] &&
            theNode2!=theSource &&
            theXn==theGraph.source(thePred[theYn])                              // increase of tree arc
            )
        #endif

        {
            node theSourceNode;

            #ifdef KEEP_DECR_DIST_
                if (theWeight<theOldWeight) theSourceNode = theGraph.source(theEdge);
                else theSourceNode = theNodeLabel[theSource];
            #else
                theSourceNode = theNodeLabel[theSource];
            #endif

            // Compute new solution
            if (!BELLMAN_FORD(theGraph, theSourceNode, theCost, theDist, thePred, theWeight<theOldWeight )) {
                cerr << "Cycle found: wrong input graph\n";
                exit(1);
            }
        }

        #ifdef FILTER_BFM_ACTIVATION_
        else {
            #ifdef DEBUG_
            cout << "Trivial operation. Don't run Bellman-Ford-Moore\n" ;
            #endif
            theTotTrivial++;
        }
        #endif

        // Operation time stop
        theOpTime = stop_watch(theOpRefTime);

        // Update counters
             if ( theOldWeight > theWeight ) {
                 theMaxDecrOpTime = Max(theMaxDecrOpTime,theOpTime);
                 theTotDecrTime  += theOpTime;
                 theDecrOpCount++;
             }
        else if ( theOldWeight < theWeight ) {
                 theMaxIncrOpTime = Max(theMaxIncrOpTime,theOpTime);
                 theTotIncrTime += theOpTime;
                 theIncrOpCount++;
             }
    }

    // Close the sequence file
    I.close();

    // Overall computation time stop
    theTotTime=stop_watch(theRefTime);

    // Compute average operation measures
    theAverDecrOpTime = (theDecrOpCount ? (float)theTotDecrTime/theDecrOpCount : 0.0);
    theAverIncrOpTime = (theIncrOpCount ? (float)theTotIncrTime/theIncrOpCount : 0.0);
    theNumOp          = theDecrOpCount + theIncrOpCount;
    theAverNodeExtr   = (float)REFCNTPREC_ * (theNumOp ? (float)gTotRefCountN/theNumOp : 0.0);
    theAverEdgeRef    = (float)REFCNTPREC_ * (theNumOp ? (float)gTotRefCountM/theNumOp : 0.0);

    // Stdout results
    cout << "\nTotal initialization time: " << theTotInitTime    << "\n";
    cout << "Total time for decreases: "    << theTotDecrTime    << "\n";
    cout << "Total time for increases: "    << theTotIncrTime    << "\n";

    #ifdef MORE_SCREEN_DATA_
    cout << "Max decr operation time: "     << theMaxDecrOpTime  << "\n";
    cout << "Mean decr operation time: "    << theAverDecrOpTime << "\n";
    cout << "Max incr operation time: "     << theMaxIncrOpTime  << "\n";
    cout << "Mean incr operation time: "    << theAverIncrOpTime << "\n";
    #ifdef REFCOUNT_
    cout << "Mean op node extraction: "     << theAverNodeExtr << "\n";
    cout << "Mean op edge references: "     << theAverEdgeRef << "\n";
    #endif
    cout << "Times bfm was not called: "    << theTotTrivial << "\n";
    cout << "Total processing time for "    << argv[3] << " = " << theTotTime << endl;
    #endif

    // Create experimentation results table
    AppendToTable(argv[4],
                  theNumNodes,
                  theNumEdges,
                  theMinWeight,
                  theMaxWeight,
                  theSeed,
                  theDecrOpCount,
                  theIncrOpCount,
                  theTotInitTime,
                  theTotDecrTime,
                  theTotIncrTime,
                  theMaxDecrOpTime,
                  theAverDecrOpTime,
                  theMaxIncrOpTime,
                  theAverIncrOpTime
                  #ifdef REFCOUNT_
                  ,theAverNodeExtr
                  ,theAverEdgeRef
                  ,theAverNodeExtr
                  ,theAverEdgeRef
                  #endif
                 );
}
