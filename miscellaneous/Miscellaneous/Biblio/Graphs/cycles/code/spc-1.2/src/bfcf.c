/* BFCF.C
   Bellmann-Ford algorithm.
   Looks for negative cycle on each iteration
   going from the node untill the root
*/

node* spc ( n, node_first, source )

long n;             /* number of nodes */

node *node_first,   /* pointer to the first node */
     *source;       /* pointer to the source */

{

node *node_from,    /* scanning node */
     *node_to,      /* node on the head of the arc */
     *node_stop,    /* fiction: node after the last in the network */
     *node_test,    /* current node for detecting negative cycle */
     *i;            /* current node */

arc  *arc_ij,       /* current arc */
     *arc_stop;     /* the next arc after the last arc going from current node */

long dist_new,      /* distance to node_to via node_from */
     dist_from;     /* distance of node_from */

long num_scans = 0; /* statistic - number of scans */

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
   }
source -> dist   = 0;
INIT_QUEUE ( source )

/* main loop */
while ( NONEMPTY_QUEUE )
 {
   EXTRACT_FIRST ( node_from )
   num_scans ++;

   dist_from = node_from -> dist;

   FOR_ALL_ARCS_FROM_NODE ( node_from, arc_ij )  
     { 
       /* scanning arcs outgoing from  node_from  */
       node_to  = arc_ij -> head;

       dist_new = dist_from + ( arc_ij -> len );

       if ( dist_new <  node_to -> dist )
	   { 
	     node_to -> dist  = dist_new;
	     
	     /* searching negative cycle */
	     for ( node_test = node_from; 
		   node_test != NNULL; 
		   node_test = node_test -> parent
		 )
	       {
		 if ( node_test == node_to )
		   {
		     n_scans = num_scans;
		     node_to -> parent = node_from;
		     return node_to;
		   }
	       }
	     /* cycle is not found */
             node_to -> parent = node_from;

             if ( NODE_OUT_OF_QUEUE ( node_to ) )
	          INSERT_TO_QUEUE ( node_to )
	   }
     } /* end of scanning  node_from */
 } /* end of the main loop */

n_scans = num_scans;
return NNULL;
}
