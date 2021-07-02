// --------------------------------------------------------------------
//
//  File:        dtc_dfs.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Smart Depth first search
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_DFS_H
#define _DTC_DFS_H

#include <LEP/dynamic_graphs/dga_base.h>

const string DFS_Name		= "DFS";
const string DFS_Description	= string("The data structure maintains path information ")+
    "in directed graphs. It uses a smart DFS to answer queries statically. Smart means "+
    "that it stops as soon as the desired path information was found.";
const string DFS_Version	= "1.1";
const string DFS_Timestamp	= "September 1997";
const string DFS_Authors	= "Tobias Miller";
const string DFS_Nodes_Yes	= "There is a path between these two nodes.";
const string DFS_Nodes_No	= "There is no path between these two nodes.";

class dtc_dfs : public dga_base
  {
    public:
      dtc_dfs(msg_graph &G) : dga_base(&G) { update(&G); }
      dtc_dfs(msg_graph *G) : dga_base(G) { update(G); }
      ~dtc_dfs() {}

      void insert_edge(edge e, double weight = 1.0) {}
      void remove_edge(edge e) {}
      
    protected:
      void after_edge_removal(node v, node w) {}
      void update(msg_graph *G) {}
      void new_graph(msg_graph *G) { update(G); }
      
    public:
      bool query() { return false; }
      bool query(edge e) { return false; }
      bool query(node v, node w) { return path(v, w); }
      bool query(node v, node w, list<node> &Path) { return path(v, w, Path); }
      bool query(node v, node w, list<edge> &Path) { return path(v, w, Path); }
      bool path(node v, node w);
      bool path(node v, node w, list<node> &Path);
      bool path(node v, node w, list<edge> &Path);

      string name() const { return DFS_Name; }
      string description() const { return DFS_Description; }
      string version() const { return DFS_Version; }
      string timestamp() const { return DFS_Timestamp; }
      string authors() const { return DFS_Authors; }
      string nodes_yes() const { return DFS_Nodes_Yes; }
      string nodes_no()  const { return DFS_Nodes_No; }  

    private:
      node_array<int> Visit;

      bool rec_path(node v, node w);
      bool rec_path(node v, node w, list<node> &Path);
      bool rec_path(node v, node w, list<edge> &Path);
  };

/******************** QUERIES ********************/
          
bool dtc_dfs::path(node v, node w)
  {
    Visit.init(*the_graph, 0);
    return rec_path(v, w);
  }
  
bool dtc_dfs::rec_path(node v, node w)
  {
    if (v == w)
        return true;
    edge e;
    Visit[v] = 1;
    forall_out_edges(e, v)
        if (Visit[target(e)] == 0)
            if (rec_path(target(e), w))
                return true;
    return false;
  }

bool dtc_dfs::path(node v, node w, list<node> &Path)
  {
    Visit.init(*the_graph, 0);
    Path.clear();
    return rec_path(v, w, Path);
  }

bool dtc_dfs::rec_path(node v, node w, list<node> &Path)
  {
    if (v == w)
      {
        Path.push(v);
        return true;
      }
    edge e;
    Visit[v] = 1;
    forall_out_edges(e, v)
        if (Visit[target(e)] == 0)
            if (rec_path(target(e), w, Path))
              {
                Path.push(v);
                return true;
              }
    return false;
  }

bool dtc_dfs::path(node v, node w, list<edge> &Path)
  {
    Visit.init(*the_graph, 0);
    Path.clear();
    return rec_path(v, w, Path);
  }

bool dtc_dfs::rec_path(node v, node w, list<edge> &Path)
  {
    if (v == w)
        return true;
    edge e;
    Visit[v] = 1;
    forall_out_edges(e, v)
        if (Visit[target(e)] == 0)
            if (rec_path(target(e), w, Path))
              {
                Path.push(e);
                return true;
              }
    return false;
  }

#endif _DTC_DFS_H

