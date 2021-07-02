/* SIMP.C
   simplex for shortest paths
   with removing nodes from queue
   and detecting negative cycles */

/* simplex pivoting */

node* pivot ( i, j, decr )

node *i,     /* node_from */
     *j;     /* node_to */

long decr;   /* distanses decrease */

{
node *v;     /* node for walk along subtree */
long deg;    /* total degree */


  /* update distances */
  for (  v = j,  deg = 0;  deg >= 0;  v = v -> t_next )
    {
      if ( v == i )
	{
	  j -> parent = i;
	  return i;
	}

      v -> dist -= decr;

      /* mark a node as inactive */
      if ( v -> status == ACTIVE )
	v -> status = INACTIVE;

      deg += v -> degree;
    }

  /* update tree walk */
  v -> t_prev -> t_next = i -> t_next;
  i -> t_next -> t_prev = v -> t_prev;
  j -> t_prev -> t_next = v;
  v -> t_prev = j -> t_prev;
  i -> t_next = j;
  j -> t_prev = i;

  ( j -> parent -> degree ) --;

  return NNULL;
}

/* shortest paths simplex */

node* spc ( n, node_first, source )

long n;                         /* number of nodes */

node *node_first,               /* pointer to the first node */
     *source;                   /* pointer to the source     */

{

node *node_from,    /* scanning node */
     *node_to,      /* node on the head of the arc */
     *node_stop,    /* fiction: node after the last in the network */
     *neg_cycle,    /* current node for detecting negative cycle */
     *i;            /* current node */

arc  *arc_ij,       /* current arc */
     *arc_stop;     /* the next arc after the last arc going from current node */

long dist_new,      /* distance to node_to via node_from */
     dist_from;     /* distance of node_from */

long num_scans = 0; /* statistic - number of scans */
int  n_status;      /* status of node extracted from the queue */

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> degree   = -1;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
   }

source -> dist   = 0;
source -> parent = source;
source -> t_next = source;
source -> t_prev = source;

INIT_QUEUE ( source );

/* main loop */

while ( NONEMPTY_QUEUE )
 {
     EXTRACT_FIRST_WITH_STATUS ( node_from );

     n_status            = node_from -> status;
     node_from -> status = OUT_OF_QUEUE;

     if ( n_status == INACTIVE ) continue;

   num_scans ++;
   dist_from = node_from -> dist;
   
     FOR_ALL_ARCS_FROM_NODE ( node_from, arc_ij )   
     { 
       /* scanning arcs outgoing from  node_from  */
       node_to  = arc_ij -> head;

       dist_new = dist_from + ( arc_ij -> len );

       if ( dist_new <  node_to -> dist )
	 {
	   if ( node_to -> parent == NNULL )
	     { 
	       node_to -> dist   = dist_new;

	       /* add node_to to tree */
	       node_to -> t_next = node_from -> t_next;
	       node_to -> t_next -> t_prev = node_to;
	       node_from -> t_next = node_to;
	       node_to -> t_prev = node_from;
	     }
	   else
	     {
	       neg_cycle = pivot (node_from, node_to, node_to->dist - dist_new);
	       if ( neg_cycle != NNULL )
		 {
		   n_scans = num_scans;
		   return neg_cycle;
		 }
	     }
	   node_to -> parent = node_from;
	   node_from -> degree ++;
	 }

       if ( node_to -> parent == node_from )
	 MAKE_ACTIVE ( node_to )

     } /* end of scanning  node_from */

 } /* end of the main loop */

n_scans = num_scans;
return NNULL;
}
