//------------------------------------------------------------------------------
// BELLMAN FORD (with parent checking if PARENT_CHECKING_ is defined)
//                                                                              
// Change log: CD000430
//------------------------------------------------------------------------------

// if KEEP_DECR_DIST_ is defined, bfm does not overwrite distances in case of
// decrease

#include <LEDA/b_queue.h>

#include "_bellman_ford.h"

#ifdef REFCOUNT_
    #define INCn_           sRefCountN++;
    #define INCm_           sRefCountM++;

    static unsigned long    sRefCountN;
    static unsigned long    sRefCountM;
    unsigned long           gTotRefCountN = 0;
    unsigned long           gTotRefCountM = 0;
#else
    #define INCm_
    #define INCn_
#endif

bool BELLMAN_FORD(const graph& G, node s,
                  const edge_array<int>& cost,
                  node_array<int>& dist,
                  node_array<edge>& pred,
                  bool keepdist )

/* single source shortest paths from s using a queue (breadth first search) with parent checking
   if PARENT_CHECKING_ is defined
   computes for all nodes v:
   a) dist[v] = cost of shortest path from s to v
   b) pred[v] = predecessor edge of v in shortest paths tree
*/

{ 
  int max_num;
  Max_Value(max_num);

  #ifdef REFCOUNT_
  sRefCountN = sRefCountM = 0;
  #endif

  node_array<int> count(G,0);
  node_array<bool> inqueue(G,false);                                    // CD990906

  int n = G.number_of_nodes();

  node_list Q;

  node u,v;
  edge e;

  forall_nodes(v,G) 
  { pred[v] = 0;
    #ifdef KEEP_DECR_DIST_
    if (!keepdist) dist[v] = max_num;
    #else
    dist[v] = max_num;
    #endif
   }

  dist[s] = 0;
  Q.append(s);
  inqueue[s]=true;                                                                            // CD990906

  while(! Q.empty() )
  { u = Q.pop();
    inqueue[u]=false;                                                                         // CD990906

    INCn_ // count ***

    if (++count[u] > n) return false;   // negative cycle

    #ifdef PARENT_CHECKING_
        if ( pred[u]==0 ? false : inqueue[G.source(pred[u])] ) continue;                      // CD990906
    #endif

    int du = dist[u];

    forall_adj_edges(e,u) 
    {

      INCm_ // count ***

      v = G.opposite(u,e);              // makes it work for undirected graphs
      int c = du + cost[e];
      if (c < dist[v]) 
      { dist[v] = c; 
        pred[v] = e;
        if (!Q.member(v)) {
            Q.append(v);
            inqueue[v]=true;                                                                  // CD990906
        }
       }
     } 
   }

  #ifdef REFCOUNT_
    gTotRefCountN += ( sRefCountN + REFCNTPREC_/2 ) / REFCNTPREC_;
    gTotRefCountM += ( sRefCountM + REFCNTPREC_/2 ) / REFCNTPREC_;
  #endif

  return true;
}

