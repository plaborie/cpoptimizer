/* BFCS.C
   searching negative cycle - DFS over the tree 
*/

node* neg_cycle ( n, node_first )

long n;             /* number of nodes */

node *node_first;   /* pointer to the first node */

{

#define NONE 0

node *node_stop,    /* fiction: node after the last in the network */
     *node_test,    /* current node for detecting negative cycle */
     *i;            /* current node */

long n_path = 0;    /* label for going down from current node */

 FOR_ALL_NODES ( i )
   i -> label = NONE;

 FOR_ALL_NODES ( i )
   {
     if ( i -> label == NONE )
       {
	 /* searching negative cycle */
	 n_path ++ ;

	 for ( node_test = i; 
	       node_test != NNULL; 
	       node_test = node_test -> parent
	      )
	   {
	     if ( node_test -> label == n_path )
	       return node_test;

	     if ( node_test -> label != NONE )
	       break;

	     node_test -> label = n_path;
	   }
       }
   }
 /* cycle is not found */
 return NNULL;

} /* end of function neg_cycle */

/* Bellmann-Ford algorithm.
   Periodcally testing negative cycles in the tree by DFS
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

long num_scans = 0, /* statistic - number of scans */
     bound,         /* number of scans between two negative cycle testing */
     time_to_check; /* number of scans after last negative cycle testing */

#define FREQ      0.2
#define MIN_BOUND 1

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
   }
source -> dist   = 0;
INIT_QUEUE ( source )

if (
    ( bound = (long) ( n * FREQ ) ) < MIN_BOUND 
   )
      bound = MIN_BOUND;

time_to_check = 0;

/* main loop */
while ( NONEMPTY_QUEUE )
 {
   EXTRACT_FIRST ( node_from )
   dist_from = node_from -> dist;

     FOR_ALL_ARCS_FROM_NODE ( node_from, arc_ij )  
     { 
       /* scanning arcs outgoing from  node_from  */
       node_to  = arc_ij -> head;

       dist_new = dist_from + ( arc_ij -> len );

       if ( dist_new <  node_to -> dist )
	   { 
	     node_to -> dist  = dist_new;
             node_to -> parent = node_from;

             if ( NODE_OUT_OF_QUEUE ( node_to ) )
	          INSERT_TO_QUEUE ( node_to )
	   }
     } /* end of scanning  node_from */
     num_scans ++;

     if ( ++ time_to_check >= bound )
       {
	 if ( ( node_test = neg_cycle ( n, node_first ) )
              != 
              NNULL
	    )
	   {
	     n_scans = num_scans;
	     return node_test;
	   }

	 time_to_check = 0;
       }    
 } /* end of the main loop */

n_scans = num_scans;
return neg_cycle ( n, node_first );

}
