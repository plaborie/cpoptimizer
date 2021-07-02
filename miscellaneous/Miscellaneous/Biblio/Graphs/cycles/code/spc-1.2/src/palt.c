/* PALT.C
   Pallotino algorithm.
   Tarjan strategy - scanning successor list and removing nodes from queue
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
     *father,       /* parent of the node */
     *i;            /* current node */

arc  *arc_ij,       /* current arc */
     *arc_stop;     /* the next arc after the last arc going from current node */

long dist_new,      /* distance to node_to via node_from */
     dist_from;     /* distance of node_from */

long total_degree;  /* sum of degrees of scanned nodes */

long num_scans = 0; /* statistic - number of scans */
int  n_status;      /* status of node extracted from the queue */

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
      i -> t_prev = NNULL;
   }

INIT_DQUEUE (source)

source -> parent = source;
source -> dist   = 0;
source -> t_next = source;
source -> t_prev = source;
source -> degree = -1;

#define nd(i) (i-node_first+1)


/* main loop */

while ( NONEMPTY_QUEUE )
 {
   DEXTRACT_FIRST ( node_from )
/*
printf ("from %ld st %d\n", nd(node_from), node_from -> status );

i = source; total_degree=0;
while (1)
  {

printf (" %ld s %ld n %ld p %ld d %ld\n", nd(i), i -> status, 
nd(i->t_next), nd(i->t_prev), i->degree );

total_degree+=i->degree;
i = i -> t_next;
if (i == source) break;
  }
if ( total_degree != -1 ) 
{
printf ("herovoe derevo\n");exit(9);
}
*/
   n_status            = node_from -> status;

   if ( n_status == NOT_ACTIVE )
     {
       node_from -> status = SCANNED_OUT;
       continue;
     }

   if ( n_status == IN_QUEUE )
     {
       /* insert to tree walk */
       node_from -> degree = -1;
       father = node_from -> parent;
       after  = father    -> t_next;
       
       father -> degree ++;
       father -> t_next    = node_from;
       node_from -> t_prev = father;
       node_from -> t_next = after;
       after  -> t_prev    = node_from;
     }
   
   num_scans ++;
   node_from -> status = SCANNED;
   dist_from = node_from -> dist;
   
   FOR_ALL_ARCS_FROM_NODE ( node_from, arc_ij )
     { 
       /* scanning aecs outgoing from  node_from  */
       node_to  = arc_ij -> head;

       dist_new = dist_from  + ( arc_ij -> len );

       if (  dist_new <  node_to -> dist  )
	   { 
/*
printf ("to %ld st %d\n", nd(node_to), node_to -> status );
*/
	     node_to -> dist = dist_new;

	     if ( node_to -> status >= IN_QUEUE_AGAIN )
	       {	     
		 /* searching negative cycle and changing the tree */
		 before = node_to -> t_prev;

		 for ( node_test = node_to, total_degree = 0; 
		      total_degree >= 0;
		      node_test = node_test -> t_next
		     )
		   {

/*
printf ("test %ld st %d d %ld td %ld\n", nd(node_test), node_test -> status,
node_test -> degree, total_degree );
*/
		     if ( node_test == node_from )
		       {
			 n_scans = num_scans;
			 node_to -> parent = node_from;
			 return node_from;
		       }
		     
		     total_degree += node_test -> degree;
		     if ( node_test -> status == IN_QUEUE_AGAIN )
		       node_test -> status = NOT_ACTIVE;
		     else
		       node_test -> status = SCANNED_OUT;
		   }
		 /* branch is scanned - negative cycle is not found */
	     
	    	 (node_to -> parent) -> degree -- ;
		 before     -> t_next = node_test;
		 node_test  -> t_prev = before;
	       }

	     node_to -> parent = node_from;
	     
	     if ( node_to -> status >= NOT_ACTIVE )
	       {
		 node_to -> degree = -1;
		 ( node_from -> degree ) ++ ;

		 after = node_from -> t_next;
	     
		 node_from -> t_next = node_to;
		 node_to   -> t_prev = node_from;

		 node_to   -> t_next = after;
		 after     -> t_prev = node_to;

		 if ( node_to -> status == SCANNED
                      ||
		      node_to -> status == SCANNED_OUT
		    )
		   INSERT_TO_ENTRY ( node_to )

		 node_to -> status = IN_QUEUE_AGAIN;    
	       }
	     else
	       {
		 if ( NODE_OUT_OF_QUEUE ( node_to ) )
		   /* wasn't scanned before */
		   INSERT_TO_QUEUE ( node_to )
	       }
/*
i = source; total_degree=0;
while (1)
  {
printf (" %ld s %ld n %ld p %ld d %ld\n", nd(i), i -> status, 
nd(i->t_next), nd(i->t_prev), i->degree );
total_degree+=i->degree;
i = i -> t_next;
if (i == source) break;
  }
if ( total_degree != -1 ) 
{
printf ("herovatoe derevo\n");exit(9);
}
*/
	   }
     } /* end of scanning  node_from */
 } /* end of the main loop */

n_scans = num_scans;
return NNULL;
}
