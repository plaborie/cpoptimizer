// --------------------------------------------------------------------
//
//  File:        dtc_bfs.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Smart Breadth first search
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_BFS_H
#define _DTC_BFS_H

#include <LEP/dynamic_graphs/dga_base.h>

const string BFS_Name	= "BFS";
const string BFS_Description	= string("The data structure maintains path information ")+
    "in directed graphs. It uses a smart BFS to answer queries statically. Smart "+
    "means that it stops as soon as the desired path information was found.";
const string BFS_Version	= "1.1";
const string BFS_Timestamp	= "September 1997";
const string BFS_Authors	= "Tobias Miller";
const string BFS_Nodes_Yes	= "There is a path between these two nodes.";
const string BFS_Nodes_No	= "There is no path between these two nodes.";

class dtc_bfs : public dga_base
  {
    public:
      dtc_bfs(msg_graph &G) : dga_base(&G) { update(&G); }
      dtc_bfs(msg_graph *G) : dga_base(G) { update(G); }
      ~dtc_bfs() {}

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

      string name() const { return BFS_Name; }
      string description() const { return BFS_Description; }
      string version() const { return BFS_Version; }
      string timestamp() const { return BFS_Timestamp; }
      string authors() const { return BFS_Authors; }
      string nodes_yes() const { return BFS_Nodes_Yes; }
      string nodes_no()  const { return BFS_Nodes_No; }  

    private:
       node_array<bool> Visit;
       node_array<edge> Hook;
  };

/******************** QUERIES ********************/
          
bool dtc_bfs::path(node v, node w)
  {
    if (v == w)
        return true;
        
    node n3, n4;
    list<node> L;
    Visit.init(*the_graph, false);
    Visit[v] = true;
    L.append(v);
    
    while (! L.empty())
      {
        n3 = L.pop();
        forall_adj_nodes(n4, n3)
            if (! Visit[n4])
              {
                if (w == n4)
                    return true;
                Visit[n4] = true;
                L.append(n4);
              }
      }
    return false;
  }
  
bool dtc_bfs::path(node v, node w, list<node> &Path)
  {
    Path.clear();
    if (v == w)
      {
        Path.push(v);
        return true;
      }
        
    node n3, n4;
    edge e;
    list<node> L;
    Visit.init(*the_graph, false);
    Hook.init(*the_graph);
    Visit[v] = true;
    L.append(v);
    
    while (! L.empty())
      {
        n3 = L.pop();
        forall_adj_edges(e, n3)
            if (! Visit[n4 = target(e)])
              {
                if (w == n4)
                  {
                    Path.push(w);
                    while (v != n3)
                      {
                        Path.push(n3);
                        n3 = source(Hook[n3]);
                      }
                    Path.push(v);
                    return true;
                  }
                Visit[n4] = true;
                Hook[n4] = e;
                L.append(n4);
              }
      }
    return false;
  }

bool dtc_bfs::path(node v, node w, list<edge> &Path)
  {
    Path.clear();
    if (v == w)
        return true;
        
    node n3, n4;
    edge e;
    list<node> L;
    Visit.init(*the_graph, false);
    Hook.init(*the_graph);
    Visit[v] = true;
    L.append(v);
    
    while (! L.empty())
      {
        n3 = L.pop();
        forall_adj_edges(e, n3)
            if (! Visit[n4 = target(e)])
              {
                if (w == n4)
                  {
                    while (v != n3)
                      {
                        Path.push(e);
                        n3 = source(e = Hook[n3]);
                      }
                    Path.push(e);
                    return true;
                  }
                Visit[n4] = true;
                Hook[n4] = e;
                L.append(n4);
              }
      }
    return false;
  }

#endif _DTC_BFS_H

