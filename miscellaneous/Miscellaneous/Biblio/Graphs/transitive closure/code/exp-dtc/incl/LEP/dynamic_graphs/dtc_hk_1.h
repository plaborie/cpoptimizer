// --------------------------------------------------------------------
//
//  File:        dtc_hk_1.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Henzinger-King decremental algorithm
//               for transitive closure
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_HK_1_H
#define _DTC_HK_1_H

#include <LEP/dynamic_graphs/dga_base.h>
#include <LEDA/graph.h>
#include <LEDA/list.h>
#include <LEDA/edge_map.h>

const string HK_1_NAME		= "HK Dec";
const string HK_1_DESCRIPTION	= string("A randomized decremental dynamic transitive ")+
    "closure algorithm developed by Henzinger and King. The data structure supports "+
    "edge deletions in directed graphs.";
const string HK_1_VERSION	= "2.0";
const string HK_1_TIMESTAMP	= "October 1997";
const string HK_1_AUTHORS	= "Tobias Miller";
const string HK_1_NODES_YES	= "There is a path between these two nodes.";
const string HK_1_NODES_NO	= "There is no path between these two nodes.";

/*{\Manpage {dtc_hk_1} {} {Henzinger and King decremental dynamic transitive closure algorithm} {DTC}}*/
class dtc_hk_1 : public dga_base
  {
/*{\Mdefinition
An instance |\Mvar| of type |\Mname| dynamically maintains the transitive closure of a directed graph $G$.
The data type realizes a randomized decremental transitive closure algorithm developed by Henzinger and King. It 
efficiently supports sequences of edge deletions.

The data structure supports [[remove_edge]], [[path]] and [[desc]] operations on directed graphs.}*/
/*{\Mtext \medskip \headerline{Relation between |msg_graph|, |dga_base| and |\Mname|}
An instance of type |msg_graph|, is a graph, that sends messages about modifications (e.~g. edge insertions, 
node deletions, \ldots) to interested third parties. Data type |msg_graph| is derived from class |graph|. All operations 
valid on class |graph| are valid on |msg_graph| as well.  

Data type |\Mname| is derived from class |dga_base|. Class |dga_base| is the base class for all dynamic graph algorithms 
in the LEDA extension package for dynamic graph algorithms. It provides a common interface for clients and a message passing 
interface for class |msg_graph|. Operations, efficiently supported by the dynamic graph algorithm, have to be redefined in 
the derived class. 

An instance |\Mvar| of type |\Mname| is maintained with respect to a message graph |G|. 
Whenever |G| is modified, a message is sent to |\Mvar|, and the data structure updated respectively.
 
Note that several dynamic graph algorithms can be maintained with the same message graph.}*/
    public:
/*{\Mcreation}*/     
      dtc_hk_1(msg_graph &G) : dga_base(&G) { update(&G); }
/*{\Mcreate Creates an instance |\Mvar| of type |\Mname| and initializes it with message graph |G|.}*/
      dtc_hk_1(msg_graph *G) : dga_base(G) { update(G); }
/*{\Mcreate Creates an instance |\Mvar| of type |\Mname| and initializes it with message graph |G|.}*/
      ~dtc_hk_1() { clear(); }
/*{\Moperations}*/
/*{\Mtext We distinguish between |inherited| operations (defined in class |dga_base|), |redefined| operations (defined in class |dga_base|, 
redefined in |\Mname|) and |stand-alone| operations (defined in class |\Mname|, but not in |dga_base|).

Due to inheritance, the default parameter |weight| is declared in some redefined operations. Note however, that it is not used in class |\Mname|.
The Henzinger and King algorithm does not consider edge or vertex weights. 

\medskip \headerline{Inherited public update operations}}*/
/*{\Moptions warnings=no}*/
/*
      void init(msg_graph *G);
*/
/*{\Mop |Inherited|: Initializes the data structure with message graph |G|. |\Mvar|.[[init]] calls the protected method |\Mvar|.[[new_graph]].}*/
/*
      edge new_edge(node v, node w, double weight=1.0);
*/
/*{\Mop |Inherited|: Calls |\Mvar|.[[before_edge_insertion]], inserts a new edge from |v| to |w| into message graph |G|, and 
calls |\Mvar|.[[insert_edge]].}*/
/*
      void delete_edge(edge e);
*/
/*{\Mop |Inherited|: Calls |\Mvar|.[[remove_edge]], deletes edge |e| from message graph |G|, and calls |\Mvar|.[[after_edge_removal]].}*/
/*
      node new_node(double weight=1.0);
*/
/*{\Mop |Inherited|: Calls |\Mvar|.[[before_node_insertion]], inserts a new vertex |v| into message graph |G|, and 
calls |\Mvar|.[[insert_node]].}*/
/*
      void delete_node(node v);
*/
/*{\Mop |Inherited|: Calls |\Mvar|.[[remove_node]], deletes vertex |v| from message graph |G|, and calls 
|\Mvar|.[[after_node_removal]].}*/
/*
      void insert_node(node v, double weight = 1.0);
*/
/*{\Mop |Inherited|: Calls |\Mvar|.[[update]] with message graph |G| as argument. It is called after the insertion of a new vertex 
into message graph |G|.}*/
/*
      void remove_node(node v);
*/
/*{\Mop |Inherited|: Does nothing. See Section `Protected update operations' for detailed information.
It is called before the deletion of a vertex from message graph |G|.}*/
/*
      void insert_edge(edge e, double weight = 1.0);
*/
/*{\Mop |Inherited|: Recomputes the data structure |\Mvar|. It is called after the insertion of a new edge |e| into message graph |G|.}*/
/*{\Mtext \medskip \headerline{Redefined public update operations}}*/
/*{\Moptions warnings=yes}*/
      void remove_edge(edge e);
/*{\Mop |Redefined|: Removes edge |e| from |\Mvar|. It is called before the deletion of an edge |e| from message graph |G|.}*/
      void set_edge_weight(edge e, double d) {}  
/*{\Mop |Redefined|: By default calls |\Mvar|.update with the current message graph |G| as argument. In class
|\Mname| it does nothing, since there is no need for edge weights.}*/
      void set_node_weight(node v, double d) {}
/*{\Mop |Redefined|: By default calls |\Mvar|.update with the current message graph |G| as argument. In class
|\Mname| it does nothing, since there is no need for vertex weights.}*/

    protected:
/*{\Mtext \medskip \headerline{Protected Update Operations}
The following operations may not be called by the user. We list them, to give a better understanding of the data structure.
 
Class |dga_base| supports four protected operations. They handle, in conjunction with the public operations [[remove_edge]], 
[[insert_edge]], [[remove_node]] and [[insert_node]], the insertion or deletion of vertices or edges. 
Each of these protected operations has a default definition in class |dga_base|. Additionaly there are two operations, [[update]] 
and [[new_graph]], that \underline{must} be redefined in all derived classes (e.~g. |\Mname|).}*/
/*{\Moptions warnings=no}*/
/*
      void before_edge_insertion(node v, node w, double weight = 1.0)
*/
/*{\Mop |Inherited|: Is called before the insertion of edge |e=(v,w)|. By default it does nothing. It is not redefined in |\Mname|.}*/
/*
      void before_node_insertion(double weight = 1.0);     
*/
/*{\Mop |Inherited|: Is called before the insertion of a new vertex. By default it does nothing. It is not redefined in |\Mname|.}*/
/*
      void after_node_removal();
*/
/*{\Mop |Inherited|: Is called after the deletion of a vertex. By default calls |\Mvar|.[[update]] with message graph |G| as argument. 
It is not redefined in |\Mname|.}*/
/*{\Moptions warnings=yes}*/
      void after_edge_removal(node v, node w) {}
/*{\Mop |Redefined|: Is called after the deletion of an edge |e=(v,w)|. By default calls |\Mvar|.[[update]] 
with message graph |G| as argument. It is redefined in |\Mname| to do nothing.}*/
      void update(msg_graph *G);
/*{\Mop |Redefined|: Initializes |\Mvar| with message graph |G|. |\Mvar|.[[update]] is the default update operation, which 
is called after each update to the graph, unless the appropriate method is redefined (see Section `Redefined public update operations').}*/
      void new_graph(msg_graph *G) { update(G); }
/*{\Mop |Redefined|: Calls |\Mvar|.[[update]]. It is called by method |\Mvar|.[[init]], whenever the user or an 
application switches to a new underlying message graph.}*/

    public:
      bool query() { return false; }
      bool query(edge e) { return false; }
/*{\Mtext \medskip \headerline{Redefined public queries}}*/
      bool query(node v, node w) { return path(v, w); }
/*{\Mop |Redefined|: Like |\Mvar|.[[path(v, w)]].}*/
/*{\Mtext \medskip \headerline{Stand-alone public queries}}*/
      bool query(node v, node w, list<node>& Path) { return path(v, w, Path); }
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path)]].}*/
      bool query(node v, node w, list<edge>& Path) { return path(v, w, Path); }
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path)]].}*/
      bool query(node v, node w, list<node>& Path, list<edge>& Cut) 
        { return path(v, w, Path, Cut); }
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path, Cut)]].}*/
      bool query(node v, node w, list<edge>& Path, list<edge>& Cut) 
        { return path(v, w, Path, Cut); }
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path, Cut)]].}*/
      bool path(node v, node w);
/*{\Mop |Stand-alone|: Returns |true|, if there is a path from vertex |v| to |w| in |G|. Otherwise, returns |false|.}*/
      bool path(node v, node w, list<node>& Path);
/*{\Mop |Stand-alone| operation. Returns |true|, if there is a path from vertex |v| to |w| in |G| and assigns the 
vertices on that path to |Path|. Otherwise, returns |false| and makes |Path| the empty list.}*/
      bool path(node v, node w, list<edge>& Path);
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the 
edges on that path to |Path|. Otherwise, returns |false| and makes |Path| the empty list.}*/
      bool path(node v, node w, list<node>& Path, list<edge>& Cut);
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the
vertices on that path to |Path|. Otherwise, returns |false| and assigns to |Cut| the edges in the cut, that separates the descendant 
vertices of |v| from all others.}*/
      bool path(node v, node w, list<edge>& Path, list<edge>& Cut);
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the
edges on that path to |Path|. Otherwise, returns |false| and assigns to |Cut| the edges in the cut, that separates the descendant 
vertices of |v| from all others.}*/
      void desc(node v, list<node>& V);
/*{\Mop |Stand-alone|: Assigns to |V| the descendant vertices of vertex |v| in |G|.}*/
      void out_tree(node v, list<node>& V, list<edge>& E);
/*{\Mop |Stand-alone|: Assigns the vertices and edges in the BFS structure stored with vertex |v| to |V| and |E|.}*/
      void dist_nodes(list<node>& V);
/*{\Mop |Stand-alone|: Assigns all distinguished vertices in the graph to |V|.}*/
      void dist_node_out_tree(node v, list<node>& V, list<edge>& E);
/*{\Mop |Stand-alone|: Assigns the vertices and edges in the outgoing BFS structure stored with distinguished vertex |v| 
to |V| and |E|. Makes |V| and |E| empty, if |v| is not a distinguished vertex.}*/
      void dist_node_in_tree(node v, list<node>& V, list<edge>& E);
/*{\Mop |Stand-alone|: Assigns the vertices and edges in the ingoing BFS structure stored with distinguished vertex |v| 
to |V| and |E|. Makes |V| and |E| empty, if |v| is not a distinguished vertex.}*/
      string name() const { return HK_1_NAME; }
      string description() const { return HK_1_DESCRIPTION; }
      string version() const { return HK_1_VERSION; }
      string timestamp() const { return HK_1_TIMESTAMP; }
      string authors() const { return HK_1_AUTHORS; }
      string nodes_yes() const { return HK_1_NODES_YES; }
      string nodes_no()  const { return HK_1_NODES_NO; }  

    public:
      struct map
        {
          list_item*     Map;
          edge_map<int>* Num;
          int            Max;
          
          map() { Map=NULL; }
          ~map() { if (Map != NULL) delete Map; }
          void inline init(edge_map<int>* num, int max)
            { Num=num; Max=max; Map=new list_item[Max+1]; for (int i=0; i<Max; i++) Map[i]=NULL; }
          void inline set(edge e, list_item it)
            { int i=(*Num)[e]; if ((i>=0) && (i<Max)) Map[i]=it; }
          void inline unset(edge e) { int i=(*Num)[e]; if ((i>=0) && (i<Max)) Map[i]=NULL; }
          bool inline check(edge e) { int i=(*Num)[e]; return ((i>=0) && (i<Max)); }
          bool inline get(edge e, list_item& it)
            { int i=(*Num)[e]; it=NULL; if ((i>=0) && (i<Max)) it=Map[i]; return (it!=NULL); }
          
          LEDA_MEMORY(map)
        };
      
      class bfs_struct
        {
          protected:
            graph* Graph;          /* the underlying graph */
            node   Root;           /* root of BFS structure */
            int    MaxDepth;       /* maximum depth of BFS structure */
            map    Index;          /* index into the Up lists */
            node_array<int> Depth; /* depth of each vertex */
            node_array< list<edge> > Up;
      
          public:
            void init(dtc_hk_1*, node, int);
            void desc(list<node> &);
            void tree(list<node> &, list<edge> &);
         };

      class out : public bfs_struct
        {
          private:
            void searchup(node);
          
          public:
            void init(dtc_hk_1*, node, int);
            void remove_edge(edge);
            inline bool path(node v) { return (Depth[v] <= MaxDepth); }
            bool path(node, list<node> &);
            bool path(node, list<edge> &);
        };

      class in : public bfs_struct
        {
          private:
            void searchup(node);

          public:
            void init(dtc_hk_1*, node, int);
            void remove_edge(edge);
            inline bool path(node v) { return (Depth[v] <= MaxDepth); }
            bool path(node, list<node> &);
            bool path(node, list<edge> &);
        };
       
      struct dist_node
        {
          node Node;
          in   In;     /* in-coming BFS structure */
          out  Out;    /* out-going BFS structure */
          
          inline bool path(node v, node w) { return (In.path(v) && Out.path(w)); }
          inline bool path(node v, node w, list<node>& Path)
            {
              if (! path(v,w)) return false;
              In.path(v,Path);
              Out.path(w,Path);
              return true;
            }
          inline bool path(node v, node w, list<edge>& Path)
            {
              if (! path(v,w)) return false;
              In.path(v,Path);
              Out.path(w,Path);
              return true;
            }
        };

      int              N, ENumMax;
      float            R, LogN;
      list<dist_node*> DistVList; /* list of distinguished vertices */
      node_array<out>  Out;       /* out-going BFS structures */
      edge_map<int>    ENum;
       
      void clear();
      void find_cut(node, list<edge>&);
      
      friend class bfs_struct;
  };

/*{\Mimplementation
The data structure realizes a randomized decremental transitive closure algorithms developed by M.R.~Henzinger and V.~King. 
For further details see Henzinger and Rauch, ``Fully dynamic biconnectivity and transitive closure'', {\em IEEE}, (1995), 664-672. 

The data structure can be initialized in $O(n*m)$ time and requires $O(n*m)$ space, where |n| is the number of vertices and |m| 
the number of edges in the graph. Any sequence of [[remove_edge]] operations requires $O(m*n*\log^2 n)$ time, where |m| 
is the initial number of edges in the graph. A [[path(node, node)]] query requires $O(n/\log n)$, a [[path(node, node, list)]] 
query $O(n)$ and a [[desc]] query $O(n^2/\log n)$ time.}*/
#endif _DTC_HK_1_H

