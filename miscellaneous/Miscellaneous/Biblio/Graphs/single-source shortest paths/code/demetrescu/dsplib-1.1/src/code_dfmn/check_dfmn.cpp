//=========================================================================
//  check_dfmn.c
//=========================================================================

// Authors:         Camil Demetrescu
// Change log:      CD991007
// Description:     DFMN Algorithm Checker using Bellman-Ford-Moore algorithm


#include<LEDA/stream.h>
#include<LEDA/graph.h>
#include<LEDA/graph_alg.h>
#include<LEDA/h_array.h>

#include<stdio.h>
#include<stdlib.h>

#include"util/utility.h"
#include"class_dfmn.h"

//#define DEBUG_

const int infinite=MAXINT/2;

// FUNCTION PROTOTYPES

int Compare(class_dfmn &SP,
             GRAPH<int,int> &GP,
             node_array<int> &inDist,
             h_array<int,node> &inNodeLabel);


// FUNCTION DEFINITIONS

//=========================================================================
//  main
//=========================================================================

// argv[0]: this program name
// argv[1]: source node
// argv[2]: input graph file name
// argv[3]: input sequence file name

// La funzione calcola i cammini minimi a tutti i nodi del grafo da
// un nodo sorgente. In max, medio, totOp vengono restituiti i tempi mass.,
// medio e totale per le operazioni. argv[2] e' il nome del file su cui
// e` memorizzato il grafo su cui operare. argv[3] e' il nome del file
// su cui e` memorizzata la sequenza su cui operare.

int main(int argc, char *argv[]){

    if ( argc==1 ) {
        cout << argv[0] << ": no argument given.\n";
        return 1;
    }
    if ( argc!=4 ) {
        cout << argv[0] << ": wrong argument number.\n";
        return 1;
    }

    int theResult;
    int numNodi;
    int numArchi;
    int theSource=atoi(argv[1]);
    GRAPH<int,int> GP;

    // Load graph
    switch (GP.read(argv[2])) {
        case 1: cout <<"ERROR: file "<<argv[2]<<" not found\n"; return 0;
        case 2: cout <<"ERROR: graph type doesn't match\n"; return 0;
    }

    numNodi=GP.number_of_nodes();
    numArchi=GP.number_of_edges();

    #ifdef DEBUG_
        cerr << "\nGraph: " << argv[2]      << "\n";
        cerr << "# nodes: " << numNodi      << "\n";
        cerr << "# edges: " << numArchi     << "\n";
        cerr << "Source: "  << theSource    << "\n\n";
    #endif

    // Initialize cost, dist and parent arrays
    edge_array<int> &theCost=GP.edge_data();
    node_array<int> theDist(GP);
    node_array<edge> thePred(GP);

    // Create a mapping between node labels and nodes
    h_array<int,node> theNodeLabel;
    node v;
    forall_nodes(v,GP) {
        theNodeLabel[GP.inf(v)]=v;
        #ifdef DEBUG_
            cerr << "Node " << GP.inf(v) << endl;
        #endif
    }

    // Create a mapping between node pairs and edges
    node_matrix<edge> theAdjMatrix(GP);

    edge e;
    forall_edges(e,GP) {

        theAdjMatrix(GP.source(e),GP.target(e))=e;

        #ifdef DEBUG_
            cerr << "Edge " << GP.inf(GP.source(e)) << " "
                            << GP.inf(GP.target(e)) << " "
                            << GP.inf(e) << endl;
        #endif
    }

    // Compute initial solution through bfm
    if (!BELLMAN_FORD(GP, theNodeLabel[theSource], theCost, theDist, thePred)) {
        cout << "Cycle found: wrong input graph\n";
        exit(1);
    }

    // Create instance of FMN class
    bool theNegCycleFound;
    class_dfmn SP(GP,theNegCycleFound,theSource);

    // Initialize data structures
    if ( theNegCycleFound ){
        cout << "Cycle found: wrong input graph\n";
        exit(1);
    }

    // Check for initialization correctness
    if ( (theResult = Compare(SP, GP, theDist, theNodeLabel)) != 0) {
        cout << "Initialization error " << theResult << " in dfmn\n";
        exit(1);
    }
    else cout << "\nInitialization OK.\n";

    #ifdef DEBUG_
        forall_nodes(v,GP) { cerr << "Dist[" << GP.inf(v) << "]==" << theDist[v] << endl; }

        // Read sequence file header
        FILE *seq;
        seq=fopen(argv[3],"r");
        if (seq==0) {
            cerr <<"ERROR: file "<<argv[3]<<" not found";
            return 0;
        }

        char tipoOp[20];
        char tipoSeq[80];
        int numOp;
        fscanf(seq,"%s%s%i%i",tipoSeq,tipoOp,&numNodi,&numOp);
        fclose(seq);

        cerr << "\nSeq:"            << argv[3]      << "\n";
        cerr << "type of Seq.: "    << tipoSeq      << "\n";
        cerr << "type of op.: "     << tipoOp       << "\n";
        cerr << "# nodes: "         << numNodi      << "\n";
        cerr << "# operations: "    << numOp        << "\n";
        cerr << "Source: "          << theSource    << "\n";
    #endif

    // Algorithm execution on input sequence of operations

    file_istream I(argv[3]);
    string pos;
    int pos1, nodo1, nodo2, theWeight, numOp, theTotTrivial=0;
    char tipoModifica;

    I >> pos >> pos;
    I >> pos1;
    I >> numOp;
    I >> pos1 >> pos1;

     // Scan operations
    for (int i=0; i<numOp; i++) {

        // Read operation from file
        I >> nodo1 >> nodo2 >> theWeight >> tipoModifica;

        // Arc deletion is achived by setting its weight to infinite
        if (tipoModifica=='C') theWeight = infinite;

        // Perform operation over the graph
        node theXn=theNodeLabel[nodo1];
        node theYn=theNodeLabel[nodo2];
        edge e=theAdjMatrix(theXn, theYn);

        int theOldWeight=GP.inf(e);

        #ifdef DEBUG_
            cerr << "Changing the weight of " << GP.inf(GP.source(e)) << " " << GP.inf(GP.target(e))
                 << " from " << GP.inf(e) << " to " << theWeight << endl;
        #endif

        GP.assign(e,theWeight);

        if (theDist[theXn]+theWeight<theDist[theYn] ||          // effective decrease
            theDist[theXn]+theWeight>theDist[theYn] &&
            nodo2!=theSource &&
            theXn==GP.source(thePred[theYn])                    // increase of tree arc
            ) {

            // Compute new solution
            if (!BELLMAN_FORD(GP, theNodeLabel[theSource], theCost, theDist, thePred)) {
                cerr << "Cycle found: wrong input graph\n";
                exit(1);
            }
        }
        else {
            #ifdef DEBUG_
            cerr << "Trivial operation. Don't run Bellman-Ford-Moore\n" ;
            #endif
            theTotTrivial++;
        }

        // Perform dynamic operation over the graph
        if (SP.ChangeWeight(nodo1, nodo2, theWeight) != class_dfmn::_OK) {
            cout << "ERROR: invalid operation.\n";
            exit (1);
        }

        // Check for initialization correctness
        if ( ( theResult = Compare(SP, GP, theDist, theNodeLabel) ) != 0 ) {
            cout << "Op. # " << i+1 <<  ( (theWeight > theOldWeight) ? " Increase " : " Decrease " ) << "from "
                 <<  theOldWeight << " to " << theWeight << " on " << nodo1 << "-" << nodo2
                 << " - error " << theResult << " in dfmn\n";
            exit(1);
        }
        else cout << "Op. # " << i+1 << ( (theWeight > theOldWeight) ? " Increase " : " Decrease " ) << "from " <<
                     theOldWeight << " to " << theWeight << " on " << nodo1 << "-" << nodo2 << " OK.\n";

    }

    cout << "Times bfm was not called: " << theTotTrivial << "\n";

    I.close();
}


//=========================================================================
//  Compare
//=========================================================================

// Compare results of Bellman-Ford-Moore and Frigioni-Marchetti-Nanni

int Compare(class_dfmn &SP,
             GRAPH<int,int> &GP,
             node_array<int> &inDist,
             h_array<int,node> &inNodeLabel){

    node v;

    // Check for distance error
    forall_nodes(v,GP) if ( inDist[v] != SP.GetDistance(GP.inf(v)) ) return 1;

    // Check for parent error
    forall_nodes(v,SP.mGraph)
        if ( SP.mGraph.inf(v)->Parent != nil )
            if ( SP.mGraph.inf(SP.mGraph.source(SP.mGraph.inf(v)->Parent->Edge))->Dist +
                 SP.mGraph.inf(v)->Parent->Weight != SP.mGraph.inf(v)->Dist ) return 2;

    return 0;
}

