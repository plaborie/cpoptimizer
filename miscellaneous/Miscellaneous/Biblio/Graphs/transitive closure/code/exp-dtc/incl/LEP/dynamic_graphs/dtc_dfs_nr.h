// --------------------------------------------------------------------
//
//  File:        dtc_dfs_nr.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Smart non recursive depth first search
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_DFS_NR_H
#define _DTC_DFS_NR_H

#include <LEP/dynamic_graphs/dga_base.h>

const string DFS_NR_Name	= "DFS NR";
const string DFS_NR_Description	= string("The data structure maintains path information ")+
    "in directed graphs. It uses a smart non recursive DFS to answer queries statically. "+
    "Smart means that it stops as soon as the desired path information was found.";
const string DFS_NR_Version	= "1.1";
const string DFS_NR_Timestamp	= "September 1997";
const string DFS_NR_Authors	= "Tobias Miller";
const string DFS_NR_Nodes_Yes	= "There is a path between these two nodes.";
const string DFS_NR_Nodes_No	= "There is no path between these two nodes.";

class dtc_dfs_nr : public dga_base
  {
    public:
      dtc_dfs_nr(msg_graph &G) : dga_base(&G) { update(&G); }
      dtc_dfs_nr(msg_graph *G) : dga_base(G) { update(G); }
      ~dtc_dfs_nr() {}

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

      string name() const { return DFS_NR_Name; }
      string description() const { return DFS_NR_Description; }
      string version() const { return DFS_NR_Version; }
      string timestamp() const { return DFS_NR_Timestamp; }
      string authors() const { return DFS_NR_Authors; }
      string nodes_yes() const { return DFS_NR_Nodes_Yes; }
      string nodes_no()  const { return DFS_NR_Nodes_No; }  

    private:
       node_array<bool> Visit;
  };

/******************** QUERIES ********************/
          
bool dtc_dfs_nr::path(node v, node w)
  {
    if (v == w)
        return true;
    
    list<edge> L;
    Visit.init(*the_graph, false);
    Visit[v]=true;
    edge e = the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v = target(e)])
              {
                Visit[v] = true;
                L.push(e);
                if (v == w)
                    return true;
                e = the_graph->first_adj_edge(v);
              }
            else
                e = the_graph->adj_succ(e);
          }
        if (L.empty())
            return false;
        e = L.pop();
      }
    return false;
  }
  
bool dtc_dfs_nr::path(node v, node w, list<node> &Path)
  {
    Path.clear();
    Path.push(v);
    if (v == w)
        return true;
    
    list<edge> L;
    Visit.init(*the_graph, false);
    Visit[v]=true;
    edge e = the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v = target(e)])
              {
                Visit[v] = true;
                Path.push(v);
                L.push(e);
                if (v == w)
                    return true;
                e = the_graph->first_adj_edge(v);
              }
            else
                e = the_graph->adj_succ(e);
          }
        Path.pop();
        if (L.empty())
            return false;
        e = L.pop();
      }
    return false;
  }

bool dtc_dfs_nr::path(node v, node w, list<edge> &Path)
  {
    Path.clear();
    if (v == w)
        return true;
    
    Visit.init(*the_graph, false);
    Visit[v]=true;
    edge e = the_graph->first_adj_edge(v);
    
    while (true)
      {
        while (e != NULL)
          {
            if (! Visit[v = target(e)])
              {
                Visit[v] = true;
                Path.push(e);
                if (v == w)
                    return true;
                e = the_graph->first_adj_edge(v);
              }
            else
                e = the_graph->adj_succ(e);
          }
        if (Path.empty())
            return false;
        e = Path.pop();
      }
    return false;
  }

#endif _DTC_DFS_NR_H

