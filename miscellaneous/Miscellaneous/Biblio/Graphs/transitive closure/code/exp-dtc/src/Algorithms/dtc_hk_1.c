// --------------------------------------------------------------------
//
//  File:        dtc_hk_1.c
//  Date:        10/97
//  Last update: 03/99
//  Description: Henzinger-King decremental algorithm
//               for transitive closure
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#include <LEP/dynamic_graphs/dtc_hk_1.h>
#include <math.h>
          
/******************** Class DTC HK 1 ********************/

void dtc_hk_1::clear()
  {
    dist_node* dn;
    forall(dn, DistVList) delete dn;
    DistVList.clear();
  }

void dtc_hk_1::update(msg_graph *G)
  {
    clear();
    
    N = G->number_of_nodes();
    LogN = log(N); if (LogN < 1) LogN = 1;
    R = (float) N/(LogN*LogN); if (R < 2) R = 2;
    
    Out.init(*G);
    ENum.init(*G);
    
    node v;
    edge e;
    dist_node* dn;
    
    ENumMax=0;
    forall_edges(e,*the_graph) ENum[e]=ENumMax++;
    forall_nodes(v,*the_graph) Out[v].init(this,v,N/R);
    
    list<node> V=the_graph->all_nodes();
    V.permute();
    list_item it = V.first();
    for (int i=2; i<=R; i=i*2)
        for (int j=1; (j<=i*LogN) && (it!=NULL); j++)
          {
            dn=new dist_node;
            dn->Node=V.inf(it);
            dn->Out.init(this, V.inf(it), N/i);
            dn->In.init(this, V.inf(it), N/i);
            DistVList.append(dn);
            it = V.succ(it);
          }
  }

/********** Updates **********/

void dtc_hk_1::remove_edge(edge e)
  {
    node v;
    dist_node* dn;
    
    the_graph->hide_edge(e);
    forall_nodes(v,*the_graph) Out[v].remove_edge(e);
    forall(dn,DistVList) { dn->Out.remove_edge(e); dn->In.remove_edge(e); }
    the_graph->restore_edge(e);
  }

/********** Queries **********/

bool dtc_hk_1::path(node v, node w)
  {
    dist_node* dn;
    if (Out[v].path(w)) return true;
    forall(dn,DistVList)
        if (dn->path(v,w)) return true;
    return false;
  }

bool dtc_hk_1::path(node v, node w, list<node>& Path)
  {
    Path.clear();
    dist_node* dn;
    if (Out[v].path(w,Path)) return true;
    forall(dn,DistVList)
        if (dn->path(v,w,Path)) return true;
    return false;
  }

bool dtc_hk_1::path(node v, node w, list<edge>& Path)
  {
    Path.clear();
    dist_node* dn;
    if (Out[v].path(w,Path)) return true;
    forall(dn,DistVList)
        if (dn->path(v,w,Path)) return true;
    return false;
  }

bool dtc_hk_1::path(node v, node w, list<node>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    find_cut(v,Cut);
    return false;
  }

bool dtc_hk_1::path(node v, node w, list<edge>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    find_cut(v,Cut);
    return false;
  }

void dtc_hk_1::find_cut(node v, list<edge>& Cut)
  {
    edge e;
    node w;
    node_array<bool> Desc(*the_graph,false);
    forall_nodes(w,*the_graph)
        if (path(v,w)) Desc[w]=true;
    forall_edges(e,*the_graph)
        if (Desc[source(e)] != Desc[target(e)]) Cut.append(e);
  }

void dtc_hk_1::desc(node v, list<node>& V)
  {
    node w;
    V.clear();
    forall_nodes(w,*the_graph)
        if (path(v,w)) V.append(w);
  }

void dtc_hk_1::out_tree(node v, list<node>& V, list<edge>& E)
  {
    V.clear();
    E.clear();
    Out[v].tree(V,E);
  }

void dtc_hk_1::dist_nodes(list<node>& L)
  {
    dist_node* dn;
    L.clear();
    forall(dn,DistVList) L.append(dn->Node);
  }

void dtc_hk_1::dist_node_out_tree(node v, list<node>& V, list<edge>& E)
  {
    dist_node* dn;
    V.clear();
    E.clear();
    forall(dn,DistVList)
        if (dn->Node == v) dn->Out.tree(V,E);
   }

void dtc_hk_1::dist_node_in_tree(node v, list<node>& V, list<edge>& E)
  {
    dist_node* dn;
    V.clear();
    E.clear();
    forall(dn,DistVList)
        if (dn->Node == v) dn->In.tree(V,E);
  }

/******************** Class Map ********************/

/*void dtc_hk_1::map::init(edge_map<int>* num, int max)
  {
    Num = num;
    Max = max;
    Map = new list_item[Max+1];
    for (int i=0; i<Max; i++) Map[i] = NULL;
  }

void dtc_hk_1::map::set(edge e, list_item it)
  {
    int i = (*Num)[e];
    if ((i>=0) && (i<Max)) Map[i] = it;
  }

void dtc_hk_1::map::unset(edge e)
  {
    int i = (*Num)[e];
    if ((i>=0) && (i<Max)) Map[i] = NULL;
  }

bool dtc_hk_1::map::check(edge e)
  {
    int i = (*Num)[e];
    return ((i>=0) && (i<Max));
  }

bool dtc_hk_1::map::get(edge e, list_item& it)
  {
    int i = (*Num)[e];
    it = NULL;
    if ((i>=0) && (i<Max)) it = Map[i];
    return (it!=NULL);
  }*/

/******************** Class BFS Tree ********************/

void dtc_hk_1::bfs_struct::init(dtc_hk_1* hk, node r, int d)
  {
    Root=r; 
    MaxDepth=d;
    Graph = hk->the_graph;
    Index.init(&(hk->ENum), hk->ENumMax);
    Depth.init(*Graph, MaxDepth+1);
    Up.init(*Graph);
    Depth[Root] = 0;
  }

void dtc_hk_1::bfs_struct::desc(list<node> &L)
  {
    node v;
    forall_nodes(v,*Graph)
        if (Depth[v] <= MaxDepth) L.append(v);
  }

void dtc_hk_1::bfs_struct::tree(list<node> &V, list<edge> &E)
  {
    node v;
    forall_nodes(v,*Graph)
        if (Depth[v] <= MaxDepth)
          {
            V.append(v);
            if (v != Root) E.append(Up[v].head());
          }
  }

/******************** Class Out ********************/

void dtc_hk_1::out::init(dtc_hk_1* hk, node r, int d)
  {
    bfs_struct::init(hk,r,d);
    node s, t;
    edge e;
    list<node> L;
    
    if (MaxDepth > 0) L.append(Root);
    while (! L.empty())
      {
        s=L.pop();
        forall_out_edges(e,s)
            if (Depth[t=target(e)] > MaxDepth)	/* not yet in tree */
              {
                if ((Depth[t]=Depth[s]+1) < MaxDepth) L.append(t);
                searchup(t);
              }
      }
  }

void dtc_hk_1::out::searchup(node v)
  {
    edge e;
    forall_in_edges(e,v)
        if ((Depth[source(e)] < Depth[v]) && (Index.check(e)))
            Index.set(e,Up[v].append(e));
  }

void dtc_hk_1::out::remove_edge(edge e)
  {
    node t;
    list_item it;
    if (Index.get(e,it))
      {
        Up[t=target(e)].del(it);
        Index.unset(e);
        if (Up[t].empty())
          {
            while ((Up[t].empty()) && (++Depth[t] <= MaxDepth))
                searchup(t);
            forall_out_edges(e,t)
                if (Depth[target(e)] <= Depth[t]) remove_edge(e);
          }
      }
  }

bool dtc_hk_1::out::path(node v, list<node> &Path)
  {
    if (Depth[v] > MaxDepth) return false;
    Path.push(v);
    for (int i=Depth[v]; i>0; i--) Path.push(v=source(Up[v].head()));
    return true;
  }

bool dtc_hk_1::out::path(node v, list<edge> &LPath)
  {
    if (Depth[v] > MaxDepth) return false;
    edge e;
    for (int i=Depth[v]; i>0; i--) { LPath.push(e=Up[v].head()); v=source(e); }
    return true;
  }

/******************** Class In ********************/

void dtc_hk_1::in::init(dtc_hk_1* hk, node r, int d)
  {
    bfs_struct::init(hk,r,d);
    node s, t;
    edge e;
    list<node> L;
    
    if (MaxDepth > 0) L.append(Root);
    while (! L.empty())
      {
        t=L.pop();
        forall_in_edges(e,t)
            if (Depth[s=source(e)] > MaxDepth)	/* not yet in tree */
              {
                if ((Depth[s]=Depth[t]+1) < MaxDepth) L.append(s);
                searchup(s);
              }
      }
  }

void dtc_hk_1::in::searchup(node v)
  {
    edge e;
    forall_out_edges(e,v)
        if ((Depth[target(e)] < Depth[v]) && (Index.check(e)))
            Index.set(e,Up[v].append(e));
  }

void dtc_hk_1::in::remove_edge(edge e)
  {
    node s;
    list_item it;
    if (Index.get(e,it))
      {
        Up[s=source(e)].del(it);
        Index.unset(e);
        if (Up[s].empty())
          {
            while ((Up[s].empty()) && (++Depth[s] <= MaxDepth))
                searchup(s);
            forall_in_edges(e,s)
                if (Depth[source(e)] <= Depth[s]) remove_edge(e);
          }
      }
  }

bool dtc_hk_1::in::path(node v, list<node> &Path)
  {
    if (Depth[v] > MaxDepth) return false;
    Path.push(v);
    for (int i=Depth[v]; i>0; i--) Path.push(v=target(Up[v].head()));
    return true;
  }

bool dtc_hk_1::in::path(node v, list<edge> &LPath)
  {
    if (Depth[v] > MaxDepth) return false;
    edge e;
    for (int i=Depth[v]; i>0; i--) { LPath.push(e=Up[v].head()); v=target(e); }
    return true;
  }



