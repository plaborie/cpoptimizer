//=========================================================================
//  test_dfmn.cpp
//=========================================================================

// Author:         Camil Demetrescu
// Change log:     CD991007
// Description:    test app for dfmn

#include "class_dfmn.h"
#include <LEDA/stream.h>

#include <stdlib.h>
#include <stdio.h>

void main(){

    class_dfmn*     theSP = nil;
    GRAPH<int,int>  theGraph;

    while(true) {

        char c[3];
        puts("\nChoose option\n");
        puts("l. load graph");
        puts("s. show graph");
        puts("c. change arc weight");
        puts("r. run sequence");
        puts("p. print distances");
        puts("q. quit");

        fgets(c,3,stdin);

        switch (c[0]) {

            // Load graph
            case 'l': {

                char theFileName[256];
                bool theNegCycleFound;

                printf("Enter file name: ");
                scanf("%s",theFileName);

                // Load graph
                switch (theGraph.read(theFileName)) {

                    case 1:
                        printf("ERROR: file %s not found\n",theFileName);
                        break;

                    case 2:
                        printf("ERROR: graph type doesn't match\n");
                        break;

                    default:
                        if ( theSP != nil ) delete theSP;
                        theSP = new class_dfmn(theGraph, theNegCycleFound);
                        if ( theSP == nil ) printf("ERROR: out of memory\n");
                        if ( theNegCycleFound ) {
                            printf("ERROR: negative cycle found\n");
                            delete theSP;
                            theSP = nil;
                        }
                }

                break;
            }

            // Show graph
            case 's': {
                node theNode;
                edge theEdge;
                if ( theSP == nil ) break;
                forall_nodes(theNode,theSP->mGraph) {
                    printf("| %-2d| ",theSP->mGraph.inf(theNode)->ID);
                    forall_out_edges(theEdge,theNode)
                        printf(" -> [%2d | %-3d]",theSP->mGraph.inf(theSP->mGraph.target(theEdge))->ID,theSP->mGraph.inf(theEdge)->Weight);
                    printf("\n");
                }
                break;
            }

            // Change arc weight
            case 'c': {
                int theX,theY,theW;
                if ( theSP == nil ) break;
                printf("Enter arc-weight x-y-w: ");
                scanf("%d %d %d",&theX,&theY,&theW);
                switch (theSP->ChangeWeight(theX,theY,theW)) {
                    case class_dfmn::_OK:             puts("OK");                                           break;
                    case class_dfmn::_InvalidEdge:    puts("Invalid edge");                                 break;
                    case class_dfmn::_NegativeCycle:  puts("Negative cycle"); delete theSP; theSP = nil;    break;
                }
                break;
            }

            // Print distances
            case 'p': {
                if ( theSP == nil ) break;
                for (int i=0 ; i < theGraph.number_of_nodes() ; i++)
                    printf("d[%d]=%d  /  p[%d]=%d\n", i, theSP->GetDistance(i), i, theSP->GetParent(i));
                break;
            }

            // Run the first many operations of a sequence
            case 'r': {

                if ( theSP == nil ) break;

                char theFileName[256];
                int  theMaxNumOp;

                printf("Enter sequence file name: ");
                scanf("%s",theFileName);
                printf("Enter maximum number of operations to perform: ");
                scanf("%d",&theMaxNumOp);

                // Sequence-related declarations
                file_istream    I(theFileName);
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
                for (int i=1; i<=theNumOp && i<=theMaxNumOp; i++) {

                    // Read operation from file
                    I >> theNodeXId >> theNodeYId >> theNewWeight >> theOpType;

                    printf("Op.# %d: setting weight of arc (%d-%d) to %d (%c)\n",i,theNodeXId,theNodeYId,theNewWeight,theOpType);

                    // Perform weight update
                    if ( theSP->ChangeWeight(theNodeXId, theNodeYId, theNewWeight ) != class_dfmn::_OK ) {
                        printf("ERROR: invalid operation.\n");
                        delete theSP;
                        theSP = nil;
                    }
                }

                // Close the sequence file
                I.close();

                break;
            }

            // Quit
            case 'q':
                exit(0);
                break;
        }
    }
}

