/* BFCH.C
   Bellmann-Ford algorithm.
   Howard's variant
*/
#define IN_TREE        2
#define ON_CYCLE       3

node* spc (long n, node *node_first, node *source)

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
int improved;
long reach_nodes, tree_size;

/* initialization */

FOR_ALL_NODES ( i )
   { 
      i -> parent   = NNULL;
      i -> dist     = VERY_FAR;
      i -> status   = OUT_OF_QUEUE;
   }

INIT_QUEUE ( source );
source -> dist   = 0;
reach_nodes = 0;

/* build initial tree, say BFS */
while ( NONEMPTY_QUEUE )
 {
   EXTRACT_FIRST(node_from);
   num_scans++;
   reach_nodes++;
   FOR_ALL_ARCS_FROM_NODE(node_from, arc_ij)  
     { 
       /* scanning arcs outgoing from node_from  */
       node_to  = arc_ij -> head;
       if ((node_to->parent == NNULL) && (node_to != source))
	 {
	   node_to->parent = node_from;
	   INSERT_TO_QUEUE(node_to);
	 }
     }
 }

/* main loop */
 do
   {
     /* compute tree distances */
     /* this can be faster if we have a parent arc pointer */
     tree_size = 0;
     FOR_ALL_NODES(node_from)
       {
	 node_from->status = OUT_OF_QUEUE;
       }
     INSERT_TO_QUEUE(source);
     while (NONEMPTY_QUEUE)
       {
	 EXTRACT_FIRST(node_from);
	 node_from->status = IN_TREE;
	 num_scans++;
	 tree_size++;
	 dist_from = node_from->dist;
	 FOR_ALL_ARCS_FROM_NODE (node_from, arc_ij)  
	   { 
	     node_to = arc_ij->head;
	     if (node_to->parent == node_from)
	       {
		 /* parallel arcs possible */

		 dist_new = dist_from + arc_ij->len;
		 if (dist_new < node_to->dist)
		   node_to->dist = dist_new;
		 if (node_to->status == OUT_OF_QUEUE)
		   {
		     INSERT_TO_QUEUE(node_to);
		   }
	       }
	   } 
       }

     if (tree_size < reach_nodes) /* we have a cycle */
       { 
	 /* only the source can have a null parent */
	 FOR_ALL_NODES(node_from)
	   {
	     if (node_from->status == IN_TREE) continue;

	     do
	       {
		 node_from->status = ON_CYCLE;
		 node_to = node_from;
		 node_from = node_to->parent;
	       } while (node_from->status == OUT_OF_QUEUE);

	     n_scans = num_scans;
	     return(node_from);
	   }
       }

     improved = 0;
     /* scan all arcs */
     FOR_ALL_NODES(node_from)
       {
	 num_scans++;
	 dist_from = node_from->dist;
	 FOR_ALL_ARCS_FROM_NODE (node_from, arc_ij)  
	   { 
	     /* scanning arcs outgoing from  node_from  */
	     node_to  = arc_ij -> head;

	     dist_new = dist_from + arc_ij->len;

	     if (dist_new < node_to->dist)
	       { 
		 improved = 1;
		 node_to->dist = dist_new;
		 node_to->parent = node_from;
	       }
	   } /* end of scanning  node_from */
       }
   }
 while (improved);

 n_scans = num_scans;
 return NNULL;
}

