// --------------------------------------------------------------------
//
//  File:        dtc_italiano.c
//  Date:        10/97
//  Last update: 03/99
//  Description: Italiano's dynamic transitive closure algorithm
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#include <LEP/dynamic_graphs/dtc_italiano.h>

/******************** UPDATE ********************/

void dtc_italiano::update(msg_graph *G)
  { 
    Index.init(*G,NULL);
    Hook.init(*G,NULL);
    Desc.init(*G);
    Reset=0;
    node v;
    edge e;

    forall_nodes(v,*G) Index(v,v)=Desc[v].new_node(v);
    list<edge> L=G->all_edges();
    forall(e,L) G->hide_edge(e);
    forall(e,L) { G->restore_edge(e); insert_edge(e); }
  }

/******************** QUERIES ********************/

bool dtc_italiano::path(node v, node w)
  {
    return (Index(v,w) != NULL);
  }

bool dtc_italiano::path(node v, node w, list<node>& Path)
  {
    Path.clear();
    node u = Index(v,w);
    if (u == NULL) return false;
    while (u != Desc[v].first_node())
      {
        Path.push(Desc[v].inf(u));
        u = source(Desc[v].first_in_edge(u));
      }
    Path.push(v);
    return true;
  }

bool dtc_italiano::path(node v, node w, list<edge>& Path)
  {
    Path.clear();
    node u = Index(v,w);
    edge e;
    if (u == NULL) return false;
    while (u != Desc[v].first_node())
      {
        u = source(e = Desc[v].first_in_edge(u));
        Path.push(Desc[v].inf(e));
      }
    return true;
  }

bool dtc_italiano::path(node v, node w, list<node>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    edge e;
    forall_edges(e,*the_graph)
        if ((Index(v,source(e)) == NULL) != (Index(v,target(e)) == NULL))
            Cut.push(e);
    return false;      
  }

bool dtc_italiano::path(node v, node w, list<edge>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    edge e;
    forall_edges(e,*the_graph)
        if ((Index(v,source(e)) == NULL) != (Index(v,target(e)) == NULL))
            Cut.push(e);
    return false;      
  }

void dtc_italiano::desc(node v, list<node>& V)
  {
    V.clear();
    node w;
    forall_nodes(w,Desc[v]) V.push(Desc[v].inf(w));
  }

void dtc_italiano::desc(node v, list<node>& V, list<edge>& E)
  {
    V.clear();
    E.clear();
    node w;
    edge e;
    forall_nodes(w,Desc[v]) V.push(Desc[v].inf(w));
    forall_edges(e,Desc[v]) E.push(Desc[v].inf(e));
  }


/******************** INSERT EDGE ********************/

void dtc_italiano::insert_edge(edge e, double weight)
  { 
    Reset = 1;
    node v, s=source(e), t=target(e);
    if (Index(s,t) == NULL)
        forall_nodes(v,*the_graph)
            if ((Index(v,s) != NULL) && (Index(v,t) == NULL))
                meld(v,s,t,e);
  }

void dtc_italiano::meld(node v, node s, node t, edge e)
  {
    Index(v,t) = Desc[v].new_node(t);
    Desc[v].new_edge(Index(v,s), Index(v,t), e);
    forall_out_edges(e, t)
        if (Index(v,target(e)) == NULL)
            meld(v,t,target(e),e);
  }

/******************** REMOVE EDGE ********************/

void dtc_italiano::remove_edge(edge e)
  {
    if (Reset) reset();

    node v, t=target(e);
    edge f;
    forall_nodes(v,*the_graph)
        if ((f = Hook(v,t)) == e) Hook(v,t)=the_graph->in_succ(f);
    the_graph->hide_edge(e);
    forall_nodes(v,*the_graph)
        if ((v != t) && (Index(v,t) != NULL))
          { 
            f = Desc[v].first_in_edge(Index(v,t));
            if (Desc[v].inf(f) == e)
              { 
                Desc[v].del_edge(f);
                remove_hook(v,t);
              }
          }
    the_graph->restore_edge(e);
  }

void dtc_italiano::remove_hook(node v, node w)
  { 
    node u;
    edge e = Hook(v,w);
    while (e != NULL)
      { 
        if ((Index(v,source(e)) != NULL) && (source(e) != target(e)))
          { 
            Hook(v,w) = the_graph->in_succ(e);
            Desc[v].new_edge(Index(v,source(e)), Index(v,w), e);
            return;
          }
        e = Hook(v,w) = the_graph->in_succ(e);
      }
    list<node> Children;
    forall_out_edges(e,Index(v,w)) Children.push(Desc[v].inf(target(e)));
    Desc[v].del_node(Index(v,w));
    Index(v,w) = NULL;
    while (! Children.empty())
      {
        u = Children.pop();
        remove_hook(v,u);
      }
  }

/******************** RESET ********************/

void dtc_italiano::reset()
  {
    node u, v;
    forall_nodes(u,*the_graph)
        forall_nodes(v,*the_graph)
            Hook(u,v) = the_graph->first_in_edge(v);
    Reset = 0;
  }
