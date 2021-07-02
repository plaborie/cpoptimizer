// --------------------------------------------------------------------
//
//  File:        dtc_dbfs.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Smart Depth-Breadth first search
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#ifndef _DTC_DBFS_H
#define _DTC_DBFS_H

#include <LEP/dynamic_graphs/dga_base.h>

const string DBFS_Name		= "DBFS";
const string DBFS_Description	= string("The data structure maintains path information ")+
    "in directed graphs. It uses a smart DBFS to answer queries statically. A DBFS " 
    "is a DFS, only that whenever it reaches a vertex it examines all its neighbours " 
    "as well. Smart means that it stops as soon as the desired path information was found.";
const string DBFS_Version	= "1.1";
const string DBFS_Timestamp	= "September 1997";
const string DBFS_Authors	= "Tobias Miller";
const string DBFS_Nodes_Yes	= "There is a path between these two nodes.";
const string DBFS_Nodes_No	= "There is no path between these two nodes.";

class dtc_dbfs : public dga_base
  {
    public:
      dtc_dbfs(msg_graph &G) : dga_base(&G) { update(&G); }
      dtc_dbfs(msg_graph *G) : dga_base(G) { update(G); }
      ~dtc_dbfs() {}

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

      string name() const { return DBFS_Name; }
      string description() const { return DBFS_Description; }
      string version() const { return DBFS_Version; }
      string timestamp() const { return DBFS_Timestamp; }
      string authors() const { return DBFS_Authors; }
      string nodes_yes() const { return DBFS_Nodes_Yes; }
      string nodes_no()  const { return DBFS_Nodes_No; }  

    private:
      node_array<bool> Visit;
  };

/******************** QUERIES ********************/
          
bool dtc_dbfs::path(node v, node w)
  {
    if (v == w)
        return true;
    edge e;
    forall_out_edges(e,v)
        if (target(e) == w) return true;
    
    list<edge> L;
    Visit.init(*the_graph,false);
    Visit[v]=true;
    e=the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v=target(e)])
              {
                Visit[v]=true;
                L.push(e);
                forall_out_edges(e,v)
                    if (target(e) == w) return true;
                e=the_graph->first_adj_edge(v);
              }
            else
                e=the_graph->adj_succ(e);
          }
        if (L.empty()) return false;
        e=L.pop();
      }
    return false;
  }
  
bool dtc_dbfs::path(node v, node w, list<node> &Path)
  {
    Path.clear();
    Path.push(v);
    if (v == w) return true;
    edge e;
    forall_out_edges(e,v)
        if (target(e) == w)
          {
            Path.push(w);
            return true;
          }
    
    list<edge> L;
    Visit.init(*the_graph, false);
    Visit[v]=true;
    e=the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v=target(e)])
              {
                Visit[v]=true;
                Path.push(v);
                L.push(e);
                forall_out_edges(e,v)
                    if (target(e) == w)
                      {
                        Path.push(w);
                        return true;
                      }
                e=the_graph->first_adj_edge(v);
              }
            else
                e=the_graph->adj_succ(e);
          }
        Path.pop();
        if (L.empty()) return false;
        e=L.pop();
      }
    return false;
  }

bool dtc_dbfs::path(node v, node w, list<edge> &Path)
  {
    Path.clear();
    if (v == w) return true;
    edge e;
    forall_out_edges(e,v)
        if (target(e) == w)
          {
            Path.push(e);
            return true;
          }
    
    Visit.init(*the_graph,false);
    Visit[v]=true;
    e = the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v=target(e)])
              {
                Visit[v]=true;
                Path.push(e);
                forall_out_edges(e,v)
                    if (target(e) == w)
                      {
                        Path.push(e);
                        return true;
                      }
                e=the_graph->first_adj_edge(v);
              }
            else
                e=the_graph->adj_succ(e);
          }
        if (Path.empty()) return false;
        e=Path.pop();
      }
    return false;
  }

#endif _DTC_DBFS_H

