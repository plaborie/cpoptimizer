// --------------------------------------------------------------------
//
//  File:        dtc_hk_2.c
//  Date:        10/97
//  Last update: 03/99
//  Description: Henzinger-King fully dynamic algorithm
//               for transitive closure
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#include <LEP/dynamic_graphs/dtc_hk_2.h>
          
/******************** Class DTC HK 2 ********************/

void dtc_hk_2::clear()
  {
    dtc_hk_1::clear();
    dist_node* dn;
    forall(dn,SpecVList) delete dn;
    SpecVList.clear();
  }

void dtc_hk_2::update(msg_graph *G)
  {
    clear();
    dtc_hk_1::update(G);
    SqrtN = sqrt(N);
    UpdCounter = 0;
  }

/********** Updates **********/

void dtc_hk_2::insert_edge(edge e, double weight)
  {
    if (UpdCounter++ < SqrtN)
      {
        ENum[e]=ENumMax++;
        dist_node* dn=new dist_node;
        dn->Node=source(e);
        dn->Out.init(this,source(e),N);
        dn->In.init(this,source(e),N);
        SpecVList.append(dn);
      }
    else update(the_graph);
  }

void dtc_hk_2::remove_edge(edge e)
  {
    if (UpdCounter++ < SqrtN)
      {
        dtc_hk_1::remove_edge(e);
        dist_node* dn;
        forall(dn,SpecVList)
          {
            dn->Out.init(this,dn->Node,N);
            dn->In.init(this,dn->Node,N);
          }
      }
    else update(the_graph);
  }

/********** Queries **********/

bool dtc_hk_2::path(node v, node w)
  {
    if (dtc_hk_1::path(v,w)) return true;
    dist_node* dn;
    forall(dn,SpecVList)
        if (dn->path(v,w)) return true;
    return false;
  }

bool dtc_hk_2::path(node v, node w, list<node>& Path)
  {
    if (dtc_hk_1::path(v,w,Path)) return true;
    dist_node* dn;
    forall(dn,SpecVList)
        if (dn->path(v,w,Path)) return true;
    return false;
  }

bool dtc_hk_2::path(node v, node w, list<edge>& Path)
  {
    if (dtc_hk_1::path(v,w,Path)) return true;
    dist_node* dn;
    forall(dn,SpecVList)
        if (dn->path(v,w,Path)) return true;
    return false;
  }

bool dtc_hk_2::path(node v, node w, list<node>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    find_cut(v,Cut);
    return false;
  }

bool dtc_hk_2::path(node v, node w, list<edge>& Path, list<edge>& Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    find_cut(v,Cut);
    return false;
  }

void dtc_hk_2::find_cut(node v, list<edge>& Cut)
  {
    edge e;
    node w;
    node_array<bool> Desc(*the_graph,false);
    forall_nodes(w,*the_graph)
        if (path(v,w)) Desc[w] = true;
    forall_edges(e,*the_graph)
        if (Desc[source(e)] != Desc[target(e)]) Cut.append(e);
  }

void dtc_hk_2::desc(node v, list<node>& V)
  {
    node w;
    V.clear();
    forall_nodes(w,*the_graph)
        if (path(v,w)) V.append(w);
  }

void dtc_hk_2::spec_nodes(list<node>& V)
  {
    dist_node* dn;
    V.clear();
    forall(dn,SpecVList)
        V.append(dn->Node);
  }
  
void dtc_hk_2::spec_node_out_tree(node v, list<node>& V, list<edge>& E)
  {
    dist_node* dn;
    forall(dn,SpecVList)
        if (dn->Node == v)
            dn->Out.tree(V, E);
   }

void dtc_hk_2::spec_node_in_tree(node v, list<node>& V, list<edge>& E)
  {
    dist_node* dn;
    forall(dn,SpecVList)
        if (dn->Node == v)
            dn->In.tree(V, E);
  }

