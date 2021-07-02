//=========================================================================
//  gen_seq_rnd_nnc_dfmn.c
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD000503
// Description:     random op sequence generator - no negative cycles


#include<LEDA/graph.h>
#include<LEDA/graph_alg.h>
#include<LEDA/node_array.h>
#include<LEDA/h_array.h>
#include<LEDA/stream.h>

#include"code_dfmn/class_dfmn.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>


//#define DEBUG_

// define this macro if you want the generation progress percent to be shown
#define PRINT_PROGRESS_


//=========================================================================
//  main
//=========================================================================

// argv[0]: this program name
// argv[1]: input graph
// argv[2]: number of operations
// argv[3]: min weight
// argv[4]: max weight
// argv[5]: output file name
// argv[6]: rnd generator seed
// argv[7]: options -s shallow solution structure
//                  -d deep solution structure
//                  -m modifying sequence
//                  -z allow zero length cycles
//                  -a alternated operations (may require more computation time)
// argv[8]: potentials file [optional]

int main(int argc, char* argv[]) {

    GRAPH<int,int>  theGraph;
    bool            theIsModifying = false;
    bool            theSolutionStructure = false;
    bool            theAlternatedOp = false;
    int             theNonZero=1;
    int             theEdgeID;

    // Check arguments
    if (argc==1) {
        cout << argv[0] << ": No argument given.\n";
        exit(1);
    }

    if (argc<7) {
        cout << argv[0] << ": Wrong argument number.\n";
        exit(1);
    }

    if (argc>=8) {
        if (strchr(argv[7],'s')!=NULL || strchr(argv[7],'d')!=NULL) theSolutionStructure = true;
        if (strchr(argv[7],'m')!=NULL)                              theIsModifying = true;
        if (strchr(argv[7],'a')!=NULL)                              theAlternatedOp = true;
        if (strchr(argv[7],'z')!=NULL)                              theNonZero = 0;
    }

    #ifdef DEBUG_
        if (theIsModifying) cout << "Modifying\n";
        else cout << "Not modifying\n";
    #endif

    // Read the graph
    if (theGraph.read(argv[1])!=0) {
        cout<<"Can't read the graph\n";
        exit(1);
    }

    h_array<int, edge>  theEdgeLabel;
    h_array<int, node>  theNodeLabel;
    int                 theIncrCount=0,
                        theDecrCount=0,
                        theFailures=0,
                        theEdgeCount=0,
                        theNumOp     = atoi(argv[2]),
                        theMinWeight = atoi(argv[3]),
                        theMaxWeight = atoi(argv[4]),
                        theSeed      = atoi(argv[6]),
                        theProgressPercent,
                        theOldProgressPercent = 0;
    node                theNode;
    edge                theEdge;
    random_source       S;
    random_source       S_p;
    file_ostream        O(argv[5]);
    node_array<int>     thePotential;
    FILE*               thePotFilePtr = NULL;

    // extend interval upper bound in case of solution structure
    if ( theSolutionStructure ) {
//        theMinWeight *= theGraph.number_of_nodes();
        theMaxWeight *= theGraph.number_of_nodes();
    }

    // Initialize random sources
    S.set_seed(theSeed);
    S_p.set_seed(theSeed);

    // Write file header
    O << "Incr_decr.\n";
    O << "int\n";
    O << theGraph.number_of_nodes() << "\n";
    O << theNumOp << "\n";
    O << theMinWeight << "\n";
    O << theMaxWeight << "\n";

    // Create instance of DFMN class
    bool theNegCycleFound;
    class_dfmn SP(theGraph,theNegCycleFound,0);

    if ( theNegCycleFound ){
        cout << "Cycle found: wrong input graph\n";
        exit(1);
    }

    // Try to open the potentials file
    if (argc>=9) thePotFilePtr = fopen(argv[8],"r");

    if ( thePotFilePtr != NULL ) {

        thePotential.init(SP.mGraph);

        // Initialize the node map
        forall_nodes(theNode, SP.mGraph) theNodeLabel[SP.mGraph.inf(theNode)->ID]=theNode;

        // read potentials from file
        forall_nodes(theNode, SP.mGraph) {
            int theNodeID, thePotVal;
            fscanf(thePotFilePtr, "%d %d", &theNodeID, &thePotVal);

            #ifdef DEBUG_
            //    cout << "Read potential " << thePotVal << " for node " << theNodeID << endl;
            #endif

            thePotential[ theNodeLabel[theNodeID] ] = thePotVal;
        }

        // close the potentials file
        fclose(thePotFilePtr);
    }

    // Initialize the edge map
    forall_edges(theEdge, SP.mGraph) theEdgeLabel[theEdgeCount++]=theEdge;

    // Initialize cost, dist and parent arrays
    node_array<int>                      theOldDist(SP.mGraph);
    node_array<class_dfmn::EdgeInfo*>    theOldPred(SP.mGraph);

    while (1) {

        // Randomly choose an edge (or keep previous to force an alternate decrease operation)
        if (!theAlternatedOp || (theIncrCount + theDecrCount)%2==0) theEdgeID = S(0,theEdgeCount-1);

        // Get a reference to the chosen edge
        theEdge = theEdgeLabel[theEdgeID];

        node    theNode1   = SP.mGraph.source(theEdge);
        node    theNode2   = SP.mGraph.target(theEdge);
        int     theNode1ID = SP.mGraph.inf(theNode1)->ID;
        int     theNode2ID = SP.mGraph.inf(theNode2)->ID;

        // Change the edge weight
        int theNewWeight;
        int theOldWeight = SP.mGraph.inf(theEdge)->Weight;

        // Iterate until a different new weight is found (with no potentials)
        if (thePotFilePtr == NULL)
            while ( (theNewWeight = S_p(theMinWeight, theMaxWeight)) == theOldWeight ) {
                #ifdef DEBUG_
                    cout << "Failure: same weight as before\n";
                #endif
            }

        // Iterate until a different new weight is found (with potentials)
        else {
            int thePotDelta = thePotential[theNode2] - thePotential[theNode1] + theNonZero;

            if (thePotDelta >= theMaxWeight) {
                #ifdef DEBUG_
                    cout << "Failure: potential difference equal to the weight upper bound\n";
                #endif
                continue;
            }

            if (!theAlternatedOp || (theIncrCount + theDecrCount)%2==0)
                while ( ( theNewWeight = S_p(thePotDelta,theMaxWeight) ) == theOldWeight ) {
                    #ifdef DEBUG_
                        cout << "Failure: same weight as before\n";
                    #endif
                }
            else theNewWeight = thePotDelta;
        }

        #ifdef DEBUG_
            cout << "Trying edge (" << theNode1ID << "," << theNode2ID << ") from " << theOldWeight << " to " << theNewWeight << "\n";
        #endif

        // Modifying operation
        if (theIsModifying &&
            ( ( theNewWeight > theOldWeight &&
                SP.mGraph.inf(theNode2)->Parent != SP.mGraph.inf(theEdge) ) ||
              ( theNewWeight < theOldWeight &&
                SP.mGraph.inf(theNode1)->Dist + theNewWeight >= SP.mGraph.inf(theNode2)->Dist ) ) ) {

            if (!theAlternatedOp || (theIncrCount + theDecrCount)%2==0) {
                theFailures++;
                #ifdef DEBUG_
                    cout << "Failure: not modifying " << (theNewWeight > theOldWeight ? "increase" : "decrease") << " operation\n";
                #endif
                continue;
            }
        }

	// Check for alternated operations
	if (theAlternatedOp)
             if ( (theNewWeight > theOldWeight && (theIncrCount + theDecrCount)%2==1) ||
                  (theNewWeight < theOldWeight && (theIncrCount + theDecrCount)%2==0) ) {
                 
                theFailures++;
                #ifdef DEBUG_
                    cout << "Failure: not alternated " << (theNewWeight > theOldWeight ? "increase" :
                            "decrease") << " operation\n";
                #endif
                continue;
             }

        #ifdef DEBUG_
            cout << "OK " << (theNewWeight > theOldWeight ? "increase" : "decrease") << " operation\n";
            cout << "Chosen edge (" << theNode1ID << "," << theNode2ID << ") from " << theOldWeight << " to " << theNewWeight << ": ";
        #endif

        // Save previous distances and parents (if no potentials are used)
        if (thePotFilePtr == NULL)
            forall_nodes(theNode,SP.mGraph) {
                theOldDist[theNode] = SP.mGraph.inf(theNode)->Dist;
                theOldPred[theNode] = SP.mGraph.inf(theNode)->Parent;
            }

        // Dynamically change the edge weight
        if ( SP.ChangeWeight(theNode1ID, theNode2ID, theNewWeight) != class_dfmn::_OK ) {

            if (thePotFilePtr != NULL) { cout << "FATAL ERROR: edge weights were incorrectly generated\n"; exit(1); }

            // Rollback previous solution
            SP.mGraph.inf(theEdge)->Weight = theOldWeight;
            forall_nodes(theNode,SP.mGraph) {

                class_dfmn::NodeInfo* theNodeInf    = SP.mGraph.inf(theNode);

                theNodeInf->Dist                    = theOldDist[theNode];
                theNodeInf->Delta                   = 0;
                theNodeInf->PQueueItem              = 0;

                // Restore the parent arc of node theNode
                if ( theNodeInf->Parent != theOldPred[theNode] ) {
                    if ( theNodeInf->Parent != nil ) SP.mGraph.inf( SP.mGraph.source( theNodeInf->Parent->Edge ) ) ->
                                                     Children.del_item(theNodeInf->ParentChildItem);
                    theNodeInf->Parent = theOldPred[theNode];
                    if ( theNodeInf->Parent != nil )
                        theNodeInf->ParentChildItem =
                            SP.mGraph.inf(SP.mGraph.source(theNodeInf->Parent->Edge))->Children.push(theNodeInf);
                }
            }

            theFailures++;

            #ifdef DEBUG_
                cout << "Failure: negative cycle found\n";
            #endif
            continue;
        }

        // Write output
        if ( theNewWeight < theOldWeight ) {        // Decrease
            O << theNode1ID << " " << theNode2ID << " " << theNewWeight;
            O << " D\n";
            theDecrCount++;
        }
        else if ( theNewWeight > theOldWeight ) {   // Increase
            O << theNode1ID << " " << theNode2ID << " " << theNewWeight;
            O << " I\n";
            theIncrCount++;
        }

        #ifdef DEBUG_
            cout << "OK #op " << theIncrCount + theDecrCount << endl;
        #endif

        #ifdef PRINT_PROGRESS_
            theProgressPercent = 100*(theIncrCount + theDecrCount) / theNumOp;
            if ( theProgressPercent / 10 != theOldProgressPercent / 10 ) {
                cout << theProgressPercent << "% done\n";
                theOldProgressPercent = theProgressPercent;
            }
        #endif

        // Break the loop if enough operations have been performed
        if ( theIncrCount + theDecrCount >= theNumOp )  break;
    }

    #ifdef DEBUG_
        cout << "\nFailures count: " << theFailures << "\n";
    #endif
}

