//------------------------------------------------------------------------------
// BELLMAN FORD (with parent checking)
//
// change log: CD000430
//------------------------------------------------------------------------------

#include <LEDA/graph.h>

#define REFCOUNT_

#ifdef REFCOUNT_
    #define REFCNTPREC_     10
    extern unsigned long gTotRefCountN;
    extern unsigned long gTotRefCountM;
#endif

bool BELLMAN_FORD(const graph& G, node s, const edge_array<int>& cost,
                                          node_array<int>& dist,
                                          node_array<edge>& pred,
                                          bool keepdist = false );

