//=========================================================================
//  run_dfmn.cpp
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD000430
// Description:     class_dfmn algorithm driver

#include<LEDA/graph.h>
#include<stdio.h>
#include<stdlib.h>

#include"class_dfmn.h"
#include"util/utility.h"


#define MORE_SCREEN_DATA_


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

int main(int argc, char *argv[]){

    int             theSource,
                    theMinWeight,
                    theMaxWeight,
                    theSeed;
    float           theTotInitTime,
                    theTotDecrTime,
                    theTotIncrTime,
                    theTotTime,
                    theAverDecrOpTime,
                    theAverIncrOpTime,
                    theMaxDecrOpTime,
                    theMaxIncrOpTime,
                    theTotRefTime,
                    theOpRefTime,
                    theOpTime;
    GRAPH<int,int>  theGraph;
    bool            theNegCycleFound;

    // Check argument count
    if ( argc==1 || argc!=8 ) {
        cout << argv[0] << ": no arguments or wrong argument number.\n";
        return 1;
    }

    // Load graph
    switch (theGraph.read(argv[2])) {
        case 1: cerr <<"ERROR: file " << argv[2] << " not found\n"; exit(1);
        case 2: cerr <<"ERROR: graph type doesn't match\n"; exit(1);
    }

    // Variables initialization
    theSource        = atoi(argv[1]);
    theMinWeight     = atoi(argv[5]);
    theMaxWeight     = atoi(argv[6]);
    theSeed          = atoi(argv[7]);
    theMaxDecrOpTime =
    theMaxIncrOpTime =
    theTotDecrTime   =
    theTotIncrTime   = 0.0;

    // Base time start
    theTotRefTime = stop_watch();

    // Build class_dfmn instance
    class_dfmn SP(theGraph, theNegCycleFound, theSource);

    // Initialize data structures
    if ( theNegCycleFound ){
        cout << "ERROR: negative cycle detected\n";
        exit(1);
    }

    // Initialization time stop
    theTotInitTime = stop_watch(theTotRefTime);

    // Sequence-related declarations
    file_istream    I(argv[3]);
    string          theDummyStr;
    int             theDummyInt,
                    theNodeXId,
                    theNodeYId,
                    theNewWeight,
                    theNumOp;
    char            theOpType;

    // Read sequence file header
    I >> theDummyStr >> theDummyStr;
    I >> theDummyInt;
    I >> theNumOp;
    I >> theDummyInt >> theDummyInt;

    // Main updates loop
    for (int i=0; i<theNumOp; i++) {

        // Read operation from file
        I >> theNodeXId >> theNodeYId >> theNewWeight >> theOpType;

        // Operation time start
        theOpRefTime = stop_watch();

        // Perform weight update
        if ( SP.ChangeWeight(theNodeXId, theNodeYId, theNewWeight ) != class_dfmn::_OK ) {
            cout << "ERROR: invalid operation.\n";
            exit (1);
        }

        // Operation time stop
        theOpTime = stop_watch(theOpRefTime);

        // Update time counters
        switch(theOpType) {
            case 'I':   theMaxIncrOpTime = Max(theMaxIncrOpTime,theOpTime);
                        theTotIncrTime  += theOpTime;
                        break;
            case 'D':   theMaxDecrOpTime = Max(theMaxDecrOpTime,theOpTime);
                        theTotDecrTime  += theOpTime;
                        break;
        }
    }

    // Close the sequence file
    I.close();

    // Overall computation time stop
    theTotTime = stop_watch(theTotRefTime);

    // Compute average operation time
    theAverDecrOpTime = (SP.mOpCountD ? (float)theTotDecrTime/SP.mOpCountD : 0.0);
    theAverIncrOpTime = (SP.mOpCountI ? (float)theTotIncrTime/SP.mOpCountI : 0.0);

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
    cout << "Mean decr node extraction: "   << (float)REFCNTPREC_ * (SP.mOpCountD ? (float)SP.mTotRefCountDN/SP.mOpCountD : 0.0) << "\n";
    cout << "Mean decr edge references: "   << (float)REFCNTPREC_ * (SP.mOpCountD ? (float)SP.mTotRefCountDM/SP.mOpCountD : 0.0) << "\n";
    cout << "Mean incr node extraction: "   << (float)REFCNTPREC_ * (SP.mOpCountI ? (float)SP.mTotRefCountIN/SP.mOpCountI : 0.0) << "\n";
    cout << "Mean incr edge references: "   << (float)REFCNTPREC_ * (SP.mOpCountI ? (float)SP.mTotRefCountIM/SP.mOpCountI : 0.0) << "\n";
    #endif
    cout << "Total processing time for "    << argv[3]         << " = " << theTotTime << "\n";
    #endif

    // Create experimentation results table
    AppendToTable(argv[4],
                  theGraph.number_of_nodes(),
                  theGraph.number_of_edges(),
                  theMinWeight,
                  theMaxWeight,
                  theSeed,
                  SP.mOpCountD,
                  SP.mOpCountI,
                  theTotInitTime,
                  theTotDecrTime,
                  theTotIncrTime,
                  theMaxDecrOpTime,
                  theAverDecrOpTime,
                  theMaxIncrOpTime,
                  theAverIncrOpTime
                  #ifdef REFCOUNT_
                  ,(float)REFCNTPREC_ * (SP.mOpCountD ? (float)SP.mTotRefCountDN/SP.mOpCountD : 0.0)
                  ,(float)REFCNTPREC_ * (SP.mOpCountD ? (float)SP.mTotRefCountDM/SP.mOpCountD : 0.0)
                  ,(float)REFCNTPREC_ * (SP.mOpCountI ? (float)SP.mTotRefCountIN/SP.mOpCountI : 0.0)
                  ,(float)REFCNTPREC_ * (SP.mOpCountI ? (float)SP.mTotRefCountIM/SP.mOpCountI : 0.0)
                  #endif
                 );
}

