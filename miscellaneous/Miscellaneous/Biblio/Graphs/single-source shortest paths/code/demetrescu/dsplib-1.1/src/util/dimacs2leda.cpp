//=========================================================================
//  dimacs2leda.cpp
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD990827
// Description:     converts a dimacs graph file from stdin to argv[1]


#include<LEDA/graph.h>
#include<LEDA/graph_alg.h>
#include<LEDA/node_array.h>
#include<LEDA/h_array.h>
#include<LEDA/stream.h>

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//#define DEBUG_

#define BUF_SIZE		256

#define Error_(str)	{				\
	fprintf(stderr,"\n"str"\n");	\
	exit(1);						\
}

#define CheckNode_(x)	{ if (x<1 || x>theNumNodes) Error_("Wrong file data"); }


//=========================================================================
//  main
//=========================================================================

// argv[0]: this program name
// argv[1]: input graph

int main(int argc, char* argv[]) {
	
    char	            theFileName[BUF_SIZE],
			            theLineBuf[BUF_SIZE],
			            theProblemName[BUF_SIZE];
    int                 theNumNodes,
                        theNumEdges,
                        theSource;
	GRAPH<int,int>      theGraph;
    h_array<int,node>   theNodeLabel;

    // check argument count
    if ( argc < 2 ) Error_("Wrong argument count");

    // read file line by line
    while ( fgets(theLineBuf, BUF_SIZE, stdin) != NULL ) {

        // print input line
        //fputs(theLineBuf,stdout);

        switch( theLineBuf[0] ) {

            // problem name tag
            case 'p':

				// read problem name, number of nodes and number of arcs
				sscanf(theLineBuf+1, "%s %d %d", theProblemName, &theNumNodes, &theNumEdges);

				// allocate nodes
				for (int i=0 ; i<theNumNodes ; i++) theNodeLabel[i] = theGraph.new_node(i);
					
				break;

			// graph source tag
			case 'n':

				// read source node id
				theSource=atoi(theLineBuf+1);

				CheckNode_(theSource);

				break;

			// arc description
			case 'a': {

				int  theTail,
					 theHead,
				     theLength;

			    node theTailNode,
			         theHeadNode;
					
				// check file data validity
				if (theNumNodes == 0 || theNumEdges ==0 )
					Error_("Wrong file data");

				// read arc tail, head and length
				sscanf(theLineBuf+1, "%d %d %d", &theTail, &theHead, &theLength);			

				CheckNode_(theTail);
				CheckNode_(theHead);

				theTailNode = theNodeLabel[theTail-1];
				theHeadNode = theNodeLabel[theHead-1];
				
				// allocate edge
				theGraph.new_edge(theTailNode, theHeadNode, theLength);

				break;
			}
		}
	}

	// write leda file
    theGraph.write(argv[1]);
}

