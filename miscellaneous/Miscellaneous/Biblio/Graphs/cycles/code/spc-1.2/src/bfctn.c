/* BFCTN.C
   Bellmann-Ford algorithm.
   Modified Tarjan strategy - scanning successor list,
   removing nodes from queue, updating distances
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
     *after,        /* next node in the successor list */
     *before,       /* previous node in the successor list */
     *i;            /* current node */

arc  *arc_ij,       /* current arc */
     *arc_stop;     /* the next arc after the last arc going from current node */

long dist_new,      /* distance to node_to via node_from */
     dist_from,     /* distance of node_from */
     decr;          /* decrease of distances on the branch */

long total_degree;  /* sum of degrees of scanned nodes */

long num_scans = 0; /* statistic - number of scans */
int  n_status;      /* status of node extracted from the queue */

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent = NNULL;
      i -> t_prev = NNULL;
      i -> dist   = VERY_FAR;
      i -> status = OUT_OF_QUEUE;
   }

source -> dist   = 0;
source -> degree = -1;
source -> t_next = source;
source -> t_prev = source;
source -> parent = source;
INIT_QUEUE ( source )

/* main loop */
while ( NONEMPTY_QUEUE )
 {
   EXTRACT_FIRST_WITH_STATUS ( node_from );

   n_status            = node_from -> status;
   node_from -> status = OUT_OF_QUEUE;

   if ( n_status == INACTIVE ) continue;

/* printf ( "\nscan %ld\n", node_from-node_first+1 );*/
   num_scans ++;
   dist_from = node_from -> dist;

   FOR_ALL_ARCS_FROM_NODE ( node_from, arc_ij )  
     { 
       /* scanning arcs outgoing from  node_from  */
       node_to  = arc_ij -> head;

       dist_new = dist_from + ( arc_ij -> len );

       if (  dist_new <  node_to -> dist  )
	   { 
	     if ( node_to -> t_prev != NNULL )
	       {	     
		 /* searching negative cycle and changing the tree */
		 before = node_to -> t_prev;
		 decr   = node_to -> dist - dist_new;

		 for ( node_test = node_to, total_degree = 0; 
		      total_degree >= 0;
		      node_test = node_test -> t_next
		     )
		   {
		     if ( node_test == node_from )
		       {
			 n_scans = num_scans;
			 node_to -> parent = node_from;
			 return node_from;
		       }
		     
		     node_test -> dist -= decr;
		     total_degree += node_test -> degree;

		     node_test -> t_prev = NNULL;

		     if ( node_test -> status == ACTIVE )
		       node_test -> status = INACTIVE;
		   }
		 /* branch is scanned - negative cycle is not found */
	     
	    	 (node_to -> parent) -> degree -- ;
		 before     -> t_next = node_test;
		 node_test  -> t_prev = before;
	       }

	     node_to -> dist   = dist_new;
             node_to -> parent = node_from;
	   }

       if ( node_to -> parent == node_from 
             &&
            node_to -> t_prev == NNULL  )
	 {
	   node_from -> degree ++ ;
	   node_to   -> degree = -1;
       
	   after = node_from -> t_next;
	     
	   node_from -> t_next = node_to;
	   node_to   -> t_prev = node_from;

	   node_to   -> t_next = after;
	   after     -> t_prev = node_to;

	   MAKE_ACTIVE ( node_to )
	 }
     } /* end of scanning  node_from */
 } /* end of the main loop */

n_scans = num_scans;
return NNULL;
}
