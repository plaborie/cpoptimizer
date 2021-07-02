//=========================================================================
//  gen_graph_rnd_nnc.c
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD000424
// Description:     random graph generator - no negative cycles


#include <LEDA/graph.h>
#include <LEDA/h_array.h>
#include <LEDA/node_matrix.h>
#include <LEDA/string.h>
#include <LEDA/stream.h>

#include <LEDA/queue.h>
#include <LEDA/stack.h>

//#define DEBUG_

// define this macro if you want the generation progress percent to be shown
#define PRINT_PROGRESS_


//=========================================================================
//  gen_graph
//=========================================================================

// Creates a graph with "inNumNodes" nodes and "inNumArcs" arcs with zero weight.
// All nodes are reachable from the source that has id 0; this is achieved
// by initially creating a connecting path having inNumNodes-1 arcs.

void gen_graph(GRAPH<int, int> &outGraph, h_array<int, node> &outNodeLabel,
               int inNumNodes, int inNumArcs, int inSeed) {

    node_matrix<edge> theAdjmatrix;

    int i, theProgressPercent, theOldProgressPercent = 0;
    int theNode1, theNode2;
    int theArcCount=0;

    random_source theRndSource(0, inNumNodes-1);
    theRndSource.set_seed(inSeed);

    #ifdef DEBUG_
        cout << "seed=" << inSeed << endl ;
    #endif

    // create nodes and initialize the edge map
    for (i=0; i<inNumNodes; i++) outNodeLabel[i]=outGraph.new_node(i);

    theAdjmatrix.init(outGraph, 0);

    // add inNumArcs arcs to the graph to ensure all nodes are
    // reachable from the source

    for (i = 1 ; i < inNumNodes ; i++) {
        theAdjmatrix(outNodeLabel[i-1], outNodeLabel[i]) = outGraph.new_edge( outNodeLabel[i-1], outNodeLabel[i], 0);
        theArcCount++;
    }

    while ( theArcCount < inNumArcs ) {

        // randomly choose a pair of nodes
        theNode1 = theRndSource();
        theNode2 = theRndSource();

        #ifdef DEBUG_
            cout << "Arcs inserted: " << theArcCount << ";  ";
            cout << "Trying arc: " << theNode1 << "-" << theNode2 << endl;
        #endif

        // nodes must be different
        if (theNode1 == theNode2) continue;

        // arc must be new
        if ( theAdjmatrix(outNodeLabel[theNode1], outNodeLabel[theNode2]) ) continue;

        // insert edge
        theAdjmatrix(outNodeLabel[theNode1], outNodeLabel[theNode2]) =
            outGraph.new_edge(outNodeLabel[theNode1], outNodeLabel[theNode2], 0);
        theArcCount++;

        // print progress
        #ifdef PRINT_PROGRESS_
            theProgressPercent = 100*theArcCount/inNumArcs;
            if ( theProgressPercent / 10 != theOldProgressPercent / 10 ) {
                cout << theProgressPercent << "% done\n";
                theOldProgressPercent = theProgressPercent;
            }
        #endif
    }
}


//=========================================================================
//  set_dfs_weights
//=========================================================================
// set weights according to a deep solution structure

void set_dfs_weights(GRAPH<int, int> &thruGraph,
                     h_array<int, node> &inNodeLabel,
                     node_array<int> &outPotential,
                     int inMinWeight, int inMaxWeight,
                     int inSeed,
                     bool inZero) {

    node_array<bool>    theFlag(thruGraph,false);
    stack<node>         theStack;
    node                theNode,
                        theChildNode;
    edge                theEdge;
    random_source       theRndSource;
    int                 theWeight, 
    			theSurplus,
    			theNumNodes=thruGraph.number_of_nodes();

    // initialize potentials array
    outPotential.init(thruGraph);

    // initialize random generator seed
    theRndSource.set_seed(inSeed);

    // perform a breadth-first visit of the graph
    theStack.push(inNodeLabel[0]);
    theFlag[inNodeLabel[0]] = true;
    outPotential[inNodeLabel[0]] = 0;

    do {
        theNode = theStack.pop();
        forall_out_edges( theEdge, theNode) {

            theChildNode = thruGraph.target(theEdge);

            // the edge is not in the tree
            if (theFlag[theChildNode]) {
                if (!inZero) theWeight = (inMaxWeight-inMinWeight)*theNumNodes;
                else theWeight = outPotential[theChildNode]-outPotential[theNode];
                thruGraph.assign(theEdge,theWeight);
            }

            // the edge is in the tree =>
            else {
                theWeight = theRndSource(inMinWeight,inMaxWeight);
                theSurplus = inZero ? 0 : theRndSource(theWeight,inMaxWeight);
                thruGraph.assign(theEdge, theWeight + theSurplus);
                theStack.push(theChildNode);
                theFlag[theChildNode]=true;
                outPotential[theChildNode] = outPotential[theNode] + theWeight;
            }
        }

    } while (!theStack.empty());
}


//=========================================================================
//  set_bfs_weights
//=========================================================================
// set weights according to a shallow solution structure

void set_bfs_weights(GRAPH<int, int> &thruGraph,
                     h_array<int, node> &inNodeLabel,
                     node_array<int> &outPotential,
                     int inMinWeight, int inMaxWeight,
                     int inSeed,
                     bool inZero) {

    node_array<bool>    theFlag(thruGraph,false);
    queue<node>         theQueue;
    node                theNode,
                        theChildNode;
    edge                theEdge;
    random_source       theRndSource;
    int                 theWeight,
    			theSurplus,
    			theNumNodes=thruGraph.number_of_nodes();

    // initialize potentials array
    outPotential.init(thruGraph);

    // initialize random generator seed
    theRndSource.set_seed(inSeed);

    // perform a breadth-first visit of the graph
    theQueue.append(inNodeLabel[0]);
    theFlag[inNodeLabel[0]] = true;
    outPotential[inNodeLabel[0]] = 0;

    do {
        theNode = theQueue.pop();
        forall_out_edges( theEdge, theNode) {

            theChildNode = thruGraph.target(theEdge);

            // the edge is not in the tree
            if (theFlag[theChildNode]) {
                if (!inZero) theWeight = 2*(inMaxWeight-inMinWeight); //*theNumNodes;
                else theWeight = outPotential[theChildNode]-outPotential[theNode];
                thruGraph.assign(theEdge,theWeight);
            }

            // the edge is in the tree =>
            else {
                theWeight = theRndSource(inMinWeight,inMaxWeight);
                theSurplus = inZero ? 0 : theRndSource(theWeight,inMaxWeight);
                thruGraph.assign(theEdge, theWeight + theSurplus);
                theQueue.append(theChildNode);
                theFlag[theChildNode]=true;
                outPotential[theChildNode] = outPotential[theNode] + theWeight;
            }
        }

    } while (!theQueue.empty());
}


//=========================================================================
//  set_rnd_weights
//=========================================================================
// set random weights (uses potentials to avoid negative cycles)
// precondition: inMinWeight<=0 and inMaxWeight>=0
// if inZero==false then zero cycles are avoided

void set_rnd_weights(GRAPH<int, int> &thruGraph,
                     node_array<int> &outPotential,
                     int inMinWeight, int inMaxWeight,
                     int inSeed,
                     bool inZero) {

    int             theMinDiam = -inMinWeight < inMaxWeight ? -inMinWeight : inMaxWeight;
    node            theNode;
    edge            theEdge;
    random_source   thePotRndSource(0,theMinDiam);
    random_source   theWeightRndSource;

    // initialize potentials array
    outPotential.init(thruGraph);

    thePotRndSource.set_seed(inSeed);
    theWeightRndSource.set_seed(inSeed);

    // create node potentials
    forall_nodes(theNode,thruGraph) outPotential[theNode] = thePotRndSource();

    // set arc weight
    forall_edges(theEdge,thruGraph) {
        int thePotDelta =   outPotential[thruGraph.target(theEdge)] -
                            outPotential[thruGraph.source(theEdge)];
        if (!inZero) thePotDelta += theWeightRndSource(1,inMaxWeight-thePotDelta);
        thruGraph.assign(theEdge, thePotDelta);
    }
}


//=========================================================================
//  main
//=========================================================================

// argv[0]: this program name
// argv[1]: output file path
// argv[2]: number of nodes
// argv[3]: number of arcs
// argv[4]: min weight
// argv[5]: max weight
// argv[6]: rnd generator seed
// argv[7]: potentials file name
// argv[8]: options
//              z - initial all zero length cycles
//              s - shallow solution tree
//              p - deep solution tree

int main(int argc, char* argv[]) {

    if (argc==1) {
        cerr<<"ERROR: no arguments given\n";
        exit(1);
    }

    if (argc<8) {
        cerr<<"ERROR: wrong argument count\n";
        exit(1);
    }

    GRAPH<int,int>      theGraph;
    node_array<int>     thePotential;
    node                theNode;
    h_array<int, node>  theNodeLabel;

    int   theNumNodes   = atoi(argv[2]),
          theNumArcs    = atoi(argv[3]);
    int   theMinWeight  = atoi(argv[4]),
          theMaxWeight  = atoi(argv[5]),
          theSeed       = atoi(argv[6]);
    char* theOptions    = argc>8 ? argv[8] : "";
    bool  isZeroLength  = strchr(theOptions,'z') != NULL;

    if ( theNumNodes < 1 || theNumArcs < 0 ||
         theNumArcs > theNumNodes * ( theNumNodes - 1 ) ||
         theNumArcs < theNumNodes - 1 ||
         theMinWeight>0 || theMaxWeight<0) {

        cerr<<"ERROR: wrong input parameters\n";
        exit(1);
    }

    // generate a random graph
    gen_graph(theGraph, theNodeLabel, theNumNodes, theNumArcs, theSeed);

    // generate random potentials and bfs solution structure (shallow tree)
    if (strchr(theOptions,'s') != NULL)
         set_bfs_weights(theGraph, theNodeLabel, thePotential, theMinWeight, theMaxWeight, theSeed, isZeroLength);

    // generate random potentials and dfs solution structure (deep tree)
    if (strchr(theOptions,'d') != NULL)
         set_dfs_weights(theGraph, theNodeLabel, thePotential, theMinWeight, theMaxWeight, theSeed, isZeroLength);

    // generate random potentials and random weights
    else set_rnd_weights(theGraph, thePotential, theMinWeight, theMaxWeight, theSeed, isZeroLength);

    // save the graph
    theGraph.write(argv[1]);

    // save potentials file
    FILE* thePotFilePtr=fopen(argv[7],"w");
    if (thePotFilePtr==NULL) { cerr << "ERROR: can't create potentials file\n"; exit(1); }

    forall_nodes(theNode,theGraph) fprintf(thePotFilePtr,"%d %d\n",theGraph.inf(theNode),thePotential[theNode]);

    fclose(thePotFilePtr);

    return 0;
}
