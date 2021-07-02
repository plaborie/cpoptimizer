// --------------------------------------------------------------------
//
//  File:        dtc_italiano_nr.h
//  Date:        10/97
//  Last update: 03/99
//  Description: A non recursive version of Italiano's dynamic transitive
//               closure algorithm.
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#include <LEP/dynamic_graphs/dtc_italiano_nr.h>

/******************** INSERT EDGE ********************/
    
void dtc_italiano_nr::insert_edge(edge e, double weight)
  {
    Reset = 1;
    node s, t, v;
    edge f;
    list<edge> L;
    if (Index(source(e),target(e)) != NULL) return;
    forall_nodes(v,*the_graph)
      {
        if (Index(v,source(e)) == NULL) continue;
        L.append(e);
        while(! L.empty())
          {
            f=L.pop();
            s=source(f);
            t=target(f);
            if (Index(v,t) == NULL)
              {
                Index(v,t)=Desc[v].new_node(t);
                Desc[v].new_edge(Index(v,s),Index(v,t),f);
                forall_out_edges(f,t) L.append(f);
              }
          }
      }
  }

/******************** REMOVE EDGE ********************/

void dtc_italiano_nr::remove_edge(edge e)
  {
    if (Reset) reset();

    node u, v, n=source(e), m=target(e);
    edge f, g;
    forall_nodes(u, *the_graph)
        if ((f = Hook(u,m)) == e) Hook(u,m) = the_graph->in_succ(f);
    the_graph->hide_edge(e);
    forall_nodes(u, *the_graph)
      { 
        if ((u != m) && (Index(u,m) != NULL))
          { 
            f = Desc[u].first_in_edge(Index(u,m));
            if (Desc[u].inf(f) == e)
              { 
                Desc[u].del_edge(f);
                list<node> L;
                L.push(m);
                while (! L.empty())
                  {
                    v = L.pop();
                    g = Hook(u,v);
                    bool hook = 0;
                    while ((g != NULL) && (! hook))
                      { 
                        if ((Index(u,source(g)) != NULL) &&  (source(g) != target(g)))
                          { 
                            Desc[u].new_edge(Index(u,source(g)), Index(u,v), g);
                            hook = 1;
                          }
                        g = Hook(u,v)=the_graph->in_succ(g);
                      }
                    if (! hook)
                      {
                        forall_out_edges(g, Index(u,v))
                            L.push(Desc[u].inf(target(g)));
                        Desc[u].del_node(Index(u,v));
                        Index(u,v) = NULL;
                      }
                  }
              }
          }
      }
    the_graph->restore_edge(e);
  }

