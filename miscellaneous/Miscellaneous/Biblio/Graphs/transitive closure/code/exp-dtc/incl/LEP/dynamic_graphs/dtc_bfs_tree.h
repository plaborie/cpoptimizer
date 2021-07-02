// --------------------------------------------------------------------
//
//  File:        dtc_bfs_tree.h
//  Date:        10/97
//  Last update: 10/97
//  Description: BFS tree; needed by older versions of dtc_hk_*.h;
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _BFS_TREE_H
#define _BFS_TREE_H

#include <LEDA/graph.h>
#include <LEDA/list.h>

class bfs_tree_fw
  {
    protected:
      struct bfs_node_fw
        {
          bfs_tree_fw*	Tree;
          node		Node;
          bool		Member;	/* necessary ? */
          int		Depth;
          list<edge>	HookList;

          bfs_node_fw() : Member(false) {}
          void init(bfs_tree_fw* T, node N, int D) { Tree = T; Node = N; Member = true; Depth = D; }
          void init_hooklist()
            {
              edge e;
              forall_in_edges(e, Node)
                  if ((Tree->NodeIndex[source(e)].Member) && (Tree->NodeIndex[source(e)].Depth < Depth))
                      Tree->EdgeIndex[e] = HookList.append(e);
            }
        };
        
      graph*	Graph;
      node	Root;
      int	MaxDepth;
      node_array<bfs_node_fw>	NodeIndex;
      edge_array<list_item>	EdgeIndex;

    public:
      bfs_tree_fw(graph*, node, int);
      bool path(node);
      bool path(node, list<node> &);
      bool path(node, list<edge> &);
      void desc(list<node> &L);
      void tree(list<node> &LNodes, list<edge> &LEdges);
      void remove_edge(edge);
    friend struct bfs_node_fw;
  };

class bfs_tree_bw
  {
    protected:
      struct bfs_node_bw
        {
          bfs_tree_bw*	Tree;
          node		Node;
          bool		Member;
          int		Depth;
          list<edge>	HookList;

          bfs_node_bw() : Member(false) {}
          void init(bfs_tree_bw* T, node N, int D) { Tree = T; Node = N; Member = true; Depth = D; }
          void init_hooklist()
            {
              edge e;
              forall_out_edges(e, Node)
                  if ((Tree->NodeIndex[target(e)].Member) && (Tree->NodeIndex[target(e)].Depth < Depth))
                      Tree->EdgeIndex[e] = HookList.append(e);
            }
        };
      
      graph*	Graph;
      node	Root;
      int	MaxDepth;
      node_array<bfs_node_bw>	NodeIndex;
      edge_array<list_item>	EdgeIndex;

    public:
      bfs_tree_bw(graph*, node, int);
      bool path(node);
      bool path(node, list<node> &);
      bool path(node, list<edge> &);
      void desc(list<node> &);
      void tree(list<node> &LNodes, list<edge> &LEdges);
      void remove_edge(edge);
   friend struct bfs_node_bw; 
  };
  
#endif _BFS_TREE_H
