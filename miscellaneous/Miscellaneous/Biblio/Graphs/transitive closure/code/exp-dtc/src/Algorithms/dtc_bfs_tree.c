// --------------------------------------------------------------------
//
//  File:        dtc_bfs_tree.c
//  Date:        10/97
//  Last update: 10/97
//  Description: BFS tree; needed by older versions of dtc_hk_*.h;
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#include <LEP/dynamic_graphs/dtc_bfs_tree.h>

/******************** BFS TREE FORWARD ********************/

bfs_tree_fw :: bfs_tree_fw(graph* G, node R, int D) : Graph(G), Root(R), MaxDepth(D)
  {
    node n1, n2;
    edge e;
    int d;
    list<node> L;
    NodeIndex.init(*Graph);
    EdgeIndex.init(*Graph, NULL);
    NodeIndex[Root].init(this, Root, 0);
    L.append(Root);
    while (! L.empty())
      {
        n1 = L.pop();
        forall_out_edges(e, n1)
            if (! NodeIndex[n2 = target(e)].Member)
              {
                NodeIndex[n2].init(this, n2, d = NodeIndex[n1].Depth+1);
                if (d < MaxDepth)
                    L.append(n2);
              }
      }
    forall_nodes(n1, *Graph)
        if (NodeIndex[n1].Member)
            NodeIndex[n1].init_hooklist();
  }

bool bfs_tree_fw :: path(node n)
  {
    if (NodeIndex[n].Member)
        return true;
    return false;
  }

bool bfs_tree_fw :: path(node n, list<node> &LPath)
  {
    if (! NodeIndex[n].Member)
        return false;
    while (n != Root)
      {
        LPath.push(n);
        n = source(NodeIndex[n].HookList.front());
      }
    LPath.push(Root);
    return true;
  }

bool bfs_tree_fw :: path(node n, list<edge> &LPath)
  {
    edge e;
    if (! NodeIndex[n].Member)
        return false;
    while (n != Root)
      {
        LPath.push(e = NodeIndex[n].HookList.front());
        n = source(e);
      }
    return true;
  }

void bfs_tree_fw :: desc(list<node> &L)
  {
    node n;
    forall_nodes(n, *Graph)
        if (NodeIndex[n].Member)
            L.append(n);
  }

void bfs_tree_fw :: tree(list<node> &LNodes, list<edge> &LEdges)
  {
    node n;
    forall_nodes(n, *Graph)
        if (NodeIndex[n].Member)
          {
            LNodes.append(n);
            if (n != Root)
                LEdges.append(NodeIndex[n].HookList.front());
          }
  }

void bfs_tree_fw :: remove_edge(edge e1)
  {
    node n;
    edge e2;
    list<edge> L;
    L.append(e1);
    while (! L.empty())
      {
        e2 = L.pop();
        if (EdgeIndex[e2] != NULL)
          {
            NodeIndex[n = target(e2)].HookList.del(EdgeIndex[e2]);
            EdgeIndex[e2] = NULL;
            if (NodeIndex[n].HookList.empty())
              {
                while ((NodeIndex[n].Depth < MaxDepth) && (NodeIndex[n].HookList.empty()))
                  {
                    NodeIndex[n].Depth++;
                    NodeIndex[n].init_hooklist();
                  }
                if (NodeIndex[n].HookList.empty())
                    NodeIndex[n].Member = false;
                forall_out_edges(e2, n)
                    L.append(e2);
              }
          }
      }
  }

/******************** BFS TREE BACKWARD ********************/

bfs_tree_bw :: bfs_tree_bw(graph* G, node R, int D) : Graph(G), Root(R), MaxDepth(D)
  {
    node n1, n2;
    edge e;
    list<node> L;
    NodeIndex.init(*Graph);
    EdgeIndex.init(*Graph, NULL);
    NodeIndex[Root].init(this, Root, 0);
    L.append(Root);
    while (! L.empty())
      {
        n1 = L.pop();
        forall_in_edges(e, n1)
            if (! NodeIndex[n2 = source(e)].Member)
              {
                NodeIndex[n2].init(this, n2, NodeIndex[n1].Depth+1);
                if (NodeIndex[n1].Depth < MaxDepth)
                    L.append(n2);
              }
      }
    forall_nodes(n1, *Graph)
        if (NodeIndex[n1].Member)
            NodeIndex[n1].init_hooklist();
  }

bool bfs_tree_bw :: path(node n)
  {
    if (NodeIndex[n].Member)
        return true;
    return false;
  }

bool bfs_tree_bw :: path(node n1, list<node> &LPath)
  {
    if (! NodeIndex[n1].Member)
        return false;
    while (n1 != Root)
      {
        LPath.append(n1);
        n1 = target(NodeIndex[n1].HookList.front());
      }
    LPath.append(Root);
    return true;
  }

bool bfs_tree_bw :: path(node n1, list<edge> &LPath)
  {
    if (! NodeIndex[n1].Member)
        return false;
    edge e;
    while (n1 != Root)
      {
        LPath.append(e = NodeIndex[n1].HookList.front());
        n1 = target(e);
      }
    return true;
  }

void bfs_tree_bw :: desc(list<node> &L)
  {
    node n;
    forall_nodes(n, *Graph)
        if (NodeIndex[n].Member)
            L.append(n);
  }

void bfs_tree_bw :: tree(list<node> &LNodes, list<edge> &LEdges)
  {
    node n;
    forall_nodes(n, *Graph)
        if (NodeIndex[n].Member)
          {
            LNodes.append(n);
            if (n != Root)
                LEdges.append(NodeIndex[n].HookList.front());
          }
  }

void bfs_tree_bw :: remove_edge(edge e1)
  {
    node n;
    edge e2;
    list<edge> L;
    L.append(e1);
    while (! L.empty())
      {
        e2 = L.pop();
        if (EdgeIndex[e2] != NULL)
          {
            n = source(e2);
            NodeIndex[n = source(e2)].HookList.del(EdgeIndex[e2]);
            EdgeIndex[e2] = NULL;
            if (NodeIndex[n].HookList.empty())
              {
                while ((NodeIndex[n].Depth < MaxDepth) && (NodeIndex[n].HookList.empty()))
                  {
                    NodeIndex[n].Depth++;
                    NodeIndex[n].init_hooklist();
                  }
                if (NodeIndex[n].HookList.empty())
                    NodeIndex[n].Member = false;
                forall_in_edges(e2, n)
                    L.append(e2);
              }
          }
      }
  }
