// --------------------------------------------------------------------
//
//  File:        dtc_italiano_opt.h
//  Date:        04/98
//  Last update: 03/99
//  Description: An optimized version of Italiano's dynamic transitive
//               closure algorithm. DESC trees are implicitly
//               represented using a matrix; lazy update of the
//               data structures in case of mixed sequences of updates.
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#include <LEP/dynamic_graphs/dtc_italiano_opt.h>

/******************** UPDATE ********************/

void dtc_italiano_opt::update(msg_graph *G)
  {
    Parent.init(*G,NULL);
    Desc.init(*G,false);
    ResetTime.init(*G,0);
    Reset=Time=0;
    node v;
    edge e;

    forall_nodes(v,*G) Desc(v,v)=true;
    list<edge> L=G->all_edges();
    forall(e,L) G->hide_edge(e);
    forall(e,L) { G->restore_edge(e); insert_edge(e); }
  }

/******************** QUERIES ********************/

bool dtc_italiano_opt::path(node v, node w, list<node>& Path)
  {
    Path.clear();
    
    if (! Desc(v,w)) return false;
    edge e;
    while ((e=Parent(v,w)) != NULL)
      {
        Path.push(w);
        w=source(e);
      }
    Path.push(w);
    return true;
  }

bool dtc_italiano_opt::path(node v, node w, list<edge>& Path)
  {
    Path.clear();
    
    if (! Desc(v,w)) return false;
    edge e;
    while ((e=Parent(v,w)) != NULL)
      {
        Path.push(e);
        w=source(e);
      }
    return true;
  }

bool dtc_italiano_opt::path(node v, node w, list<node>& Path, list<edge>& Cut)
  {
    Cut.clear();
    
    if (path(v,w,Path)) return true;
    
    edge e;
    forall_edges(e,*the_graph)
        if (Desc(v,source(e)) != Desc(v,target(e))) Cut.push(e);
    return false;      
  }

bool dtc_italiano_opt::path(node v, node w, list<edge>& Path, list<edge>& Cut)
  {
    Cut.clear();
    
    if (path(v,w,Path)) return true;
    
    edge e;
    forall_edges(e,*the_graph)
        if (Desc(v,source(e)) != Desc(v,target(e))) Cut.push(e);
    return false;      
  }

void dtc_italiano_opt::desc(node v, list<node>& Nodes)
  {
    Nodes.clear();
    
    node w;
    forall_nodes(w,*the_graph)
        if (Desc(v,w)) Nodes.push(w);
  }

void dtc_italiano_opt::desc(node v, list<node>& Nodes, list<edge>& Edges)
  {
    Nodes.clear();
    Edges.clear();
    
    node w;
    forall_nodes(w,*the_graph)
        if (Desc(v,w)) 
          {
            Nodes.push(w);
            if (v != w) Edges.push(Parent(v,w));
          }
  }

/******************** INSERT EDGE ********************/

void dtc_italiano_opt::insert_edge(edge e, double weight)
  { 
    Reset = 1;
    node v, s=source(e), t=target(e);
    if (! Desc(s,t))
        forall_nodes(v,*the_graph)
            if (Desc(v,s) && (! Desc(v,t))) meld(v,e);
  }

void dtc_italiano_opt::meld(node v, edge e)
  {
    node t=target(e);
    Desc(v,t)=true;
    Parent(v,t)=e;
    forall_out_edges(e,t)
        if (! Desc(v,target(e))) meld(v,e);
  }

/******************** REMOVE EDGE ********************/

void dtc_italiano_opt::remove_edge(edge e)
  {
    if (Reset) reset();
    
    the_graph->hide_edge(e);
    node v;
    forall_nodes(v,*the_graph) hook(v,e);
    the_graph->restore_edge(e);
  }

void dtc_italiano_opt::hook(node v, edge e)
  {
    node w=target(e);
    if (Parent(v,w) != e) return;
    
    if (ResetTime(v,w) != Time)
      {
        e=the_graph->first_in_edge(w);
        ResetTime(v,w)=Time;
      }
    else e=the_graph->in_succ(e);
    
    while ((e != NULL) && (! Desc(v,source(e))))
        e=the_graph->in_succ(e);
    
    if ((Parent(v,w)=e) != NULL) return;
    Desc(v,w)=false;
    forall_out_edges(e,w) hook(v,e);
  }

/******************** RESET ********************/

void dtc_italiano_opt::reset()
  {
    Time++;
    if (Time > the_graph->number_of_nodes())
      {
        Time=1;
        node v, w;
        forall_nodes(v,*the_graph)
            forall_nodes(w,*the_graph) ResetTime(v,w)=0;
      }

    Reset=0;
  }
