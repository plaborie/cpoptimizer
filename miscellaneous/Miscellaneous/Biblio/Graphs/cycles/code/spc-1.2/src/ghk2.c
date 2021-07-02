/* GHK.C
   Bellmann-Ford algorithm.
   Goldfarb-Hao-Kai level-based optimization, cycle detection
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

node *dummy_node;
node dummy;

arc  *arc_ij,       /* current arc */
     *arc_stop;     /* the next arc after the last arc going from current node */

long dist_new,      /* distance to node_to via node_from */
     dist_from;     /* distance of node_from */

long num_scans = 0; /* statistic - number of scans */
long passCnt;       /* number of queue passes */
long toLev;

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
      i -> tLevel = 0;
   }
source -> dist   = 0;
source -> tLevel = 1;
levCnt[1] = 1;
INIT_QUEUE (source);
dummy_node = &dummy;
INSERT_TO_QUEUE(dummy_node);
passCnt = 1;

/* main loop */
while (1)
 {

   EXTRACT_FIRST ( node_from );
   if (node_from == dummy_node) {
     if (NONEMPTY_QUEUE) {
       passCnt++;
       INSERT_TO_QUEUE(dummy_node);
       continue;
     }
     else {
       break;
     }
   }

   switch (node_from->tLevel) {
   case 1:
     break;
   case 2:
     if (node_from->parent->tLevel >= passCnt) {
       assert(node_from -> tLevel == passCnt + 1);
       INSERT_TO_QUEUE(node_from);
       continue;
     }
     break;
   default:
     if (node_from->parent->parent->tLevel >= passCnt - 1) {
       assert(node_from -> tLevel == passCnt + 1);
       INSERT_TO_QUEUE(node_from);
       continue;
     }
     break;
   }

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
	     
             node_to -> parent = node_from;

             if ( NODE_OUT_OF_QUEUE ( node_to ) )
	          INSERT_TO_QUEUE ( node_to );

	     /* update levels, look for cycles */
	     toLev = node_to -> tLevel;
	     if (toLev != passCnt + 1) {
	       levCnt[toLev]--;
	       if ((levCnt[toLev] == 0) && (toLev > 0) && (toLev < passCnt)) {
		 n_scans = num_scans;
		 return(node_to);
	       }

	       node_to -> tLevel = passCnt + 1;
	       levCnt[node_to -> tLevel]++;
	     }
	   }
     } /* end of scanning  node_from */
 } /* end of the main loop */

n_scans = num_scans;
return NNULL;
}
