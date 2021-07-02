/* Executor of SP codes with detecting negative cycles  */

/* Functions for constructing shortest path tree:
   bfcf.c   - Bellmann-Ford algorithm + walt down to the root
   bfcs.c   - Bellmann-Ford algorithm + peridic tree testing
   bfct.c   - Bellmann-Ford algorithm + traversing the upper brunch (Tarjan)
   bfcm.c   - Bellmann-Ford algorithm + lowest tree level selection
   bfctn.c  - bfctn.c + updating subtree distances
   simp.c   - simplex for shortest paths
   gorc.c   - topological sorting algorithm (Goldberg-Radzik)
   ghk[i].c - Goldfarb-Hao-Kao with i levels.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* statistical variables */
long n_scans;


#define DBG
#define RUN
#define XRUN
#define PRINT_CYCLE

/* types_### -  node & arc structures */
/* queue.h   - operation with queues  *
/* stack.h   - operations with stacks */

/* some useful definitions to simlify text */
#include "usef_def.h"

#ifdef ghk
#include "types_ghk.h"
#include "queue.h"
long    *levCnt;               /* count of nodes at levels */     
long x;
#include "ghk.c"
#endif

#ifdef ghk1
#define ghk 1
#include "types_ghk.h"
#include "queue.h"
long    *levCnt;               /* count of nodes at levels */     
long x;
#include "ghk1.c"
#endif

#ifdef ghk2
#define ghk 2
#include "types_ghk.h"
#include "queue.h"
long    *levCnt;               /* count of nodes at levels */     
long x;
#include "ghk2.c"
#endif

#ifdef ghk3

#define ghk 1
#include "types_ghk.h"
#include "queue.h"

long    *levCnt;               /* count of nodes at levels */     
long x;

#include "ghk3.c"

#endif

#ifdef bfcf
#include "types_bf.h"
#include "queue.h"
#include "bfcf.c"
#endif

#ifdef bfch
#include "types_bf.h"
#include "queue.h"
#include "bfch.c"
#endif

#ifdef bfcs
#include "types_bf.h"
#include "queue.h"
#include "bfcs.c"
#endif

#ifdef bfct
#include "types_simp.h"
#include "queue.h"
#include "bfct.c"
#endif

#ifdef bfcm
#include "types_bfm.h"
#include "globals.h"
#include "mqueue.h"
#include "bfcm.c"
#endif

#ifdef bfctn
#include "types_simp.h"
#include "queue.h"
#include "bfctn.c"
#endif

#ifdef simp
#include "types_simp.h"
#include "queue.h"
#include "simp.c"
#endif

#ifdef gorc
#include "types_gor.h"
#include "stack.h"
#include "gorc.c"
#endif

#ifdef palt
#include "types_simp.h"
#include "queue.h"
#include "palt.c"
#endif

/* parser for reading extended DIMACS format input and transforming the
   data to the internal representation */
#include "parser.c"

/* function 'timer()' for mesuring processor time */
#include "timer.c"

main ()

{

float   t;                     /* running time */

arc     *arc_first,            /* first arc */
        *arc_stop,             /* stop arc for scans */
        *ta;                   /* current arc */

node    *node_first,           /* first node */
        *node_stop,            /* after the last node */
        *source,               /* root of the tree */
        *k,                    /* current node */
        *i,                    /* current node */
        *point;                /* NULL if no negative cycle detected
                                  or a node on the negative cycle */

long    n,                     /* number of nodes */
        m,                     /* number of arcs  */
        nmin,                  /* minimal number of nodes */
        l_cycle = 0;           /* number of arcs in negative cycle */

double  sum_d = 0,             /* control sum for solution */
        len_c = 0,             /* length of negative cycle */
        l_min;                 /* current minimal length of arcs */

char    title[40];             /* title of running program */

node    *dummyNode;            /* for sentinel uses */

#ifdef RUN
printf("SPC version 1.1\n");
printf ( "\nBegin parsing...\n" );
#endif


 dummyNode = (node *) calloc(1, sizeof(node));
 parse( &n, &m, &node_first, &arc_first, &source, &nmin );

#ifdef ghk
levCnt = (long *) calloc(n+1, sizeof(long));
for (x = 0; x <= n; x++)
  levCnt[x] = 0;
#endif

#ifdef DBG

printf ( "Initial network:\nn= %ld, m= %ld, nmin= %ld, source = %ld\n",
          n, m, nmin, NODE_NUMBER(source) );
 
printf ("\nOrdered arcs:\n");
FOR_ALL_NODES ( k )
  {
    FOR_ALL_ARCS_FROM_NODE ( k, ta )
      printf ( " %ld %ld %ld\n",
               NODE_NUMBER ( k ), NODE_NUMBER ( ta -> head ), ta -> len
             );
  }
#endif


#ifdef RUN
printf ( "Parsing has finished. Begin calculating shortest paths...\n" );
#endif

t = timer();

point = spc ( n, node_first, source );

t = timer() - t;

if ( point == NNULL )
  {
    FOR_ALL_NODES ( k )
      {
	if ( k -> parent != NNULL )
	  sum_d += (double) (k -> dist);
      }
  }
#ifndef ghk
else
  {
    k = point;
    do {
      l_cycle ++ ;
      i = k -> parent;

      l_min = VERY_FAR;
      FOR_ALL_ARCS_FROM_NODE ( i, ta )
	{
	  if ( ( ta -> head ) == k && ( ta -> len ) < l_min )
	    l_min = ta -> len;
	}
      len_c += l_min;
      
      k = i;
    } while ( k != point );
    sum_d = len_c;
  }
#else
else {
  l_cycle = len_c = -1;
}
#endif

#ifdef ghk
strcpy ( title, "Goldfarb-Hao-Kai (cycle existance only)" );
#endif

#ifdef ghk1
strcpy ( title, "Goldfarb-Hao-Kai 1 level (cycle existance only)" );
#endif

#ifdef ghk2
strcpy ( title, "Goldfarb-Hao-Kai 2 levels (cycle existance only)" );
#endif

#ifdef ghk3
strcpy ( title, "Goldfarb-Hao-Kai 3 levels(cycle existance only)" );
#endif

#ifdef bfcf
strcpy ( title, "Bellman-Ford (down to the root)" );
#endif

#ifdef bfch
strcpy ( title, "Bellman-Ford, Howard's variant " );
#endif

#ifdef bfcs
strcpy ( title, "Bellman-Ford (periodic testing)" );
#endif

#ifdef bfct
strcpy ( title, "Bellman-Ford (Tarjan modification)" );
#endif

#ifdef bfcm
strcpy ( title, "Bellman-Ford (Lowest tree level)" );
#endif

#ifdef bfctn
strcpy ( title, "Bellman-Ford (Tarjan modification + updates)" );
#endif

#ifdef simp
strcpy ( title, "Shortest paths simplex" );
#endif

#ifdef gorc
strcpy ( title, "Topological sort" );
#endif

#ifdef palt
strcpy ( title, "Pallotino (Tarjan modification)" );
#endif

#ifdef XRUN
printf ("%8ld %8ld %15.0f %8ld %9ld %11.2f\n",
         n, m, sum_d, l_cycle, n_scans, t );
#endif

#ifdef RUN
printf ("\n%s\n\
Nodes: %9ld\n\
Arcs : %9d\n\n\
Number of scans:  %15ld\n\
Sum of distances: %15.0f\n\n\
Running time:     %15.2f\n",
         title, n, m, n_scans, sum_d, t ); 

if ( point != NNULL )
  {
    printf ("Negative cycle arcs:   %10ld\n", l_cycle );
    printf ("Negative cycle length: %10.0f\n\n", len_c );
#ifdef PRINT_CYCLE
    printf ("Negative cycle:\n");
    k = point;
    do {
	printf ("%7ld\n", NODE_NUMBER ( k ) );
		k = k -> parent;	      
    } while ( k != point );
#endif
  }
else
    printf ("No negative cycles detected\n\n");

#endif

#ifdef DBG
  printf ( "\nTree:\n" );
  FOR_ALL_NODES ( k )
   printf ("%ld %ld %ld\n", 
            NODE_NUMBER ( k ), NODE_NUMBER ( k -> parent ), k -> dist );
#endif

}
