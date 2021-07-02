// --------------------------------------------------------------------
//
//  File:        dtc_italiano_nr.h
//  Date:        10/97
//  Last update: 03/99
//  Description: A non recursive version of Italiano's dynamic transitive 
//               closure algorithm.
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_ITALIANO_NR_H
#define _DTC_ITALIANO_NR_H

#include <LEP/dynamic_graphs/dtc_italiano.h>

const string ITALIANO_NR_NAME        = "Italiano NR";
const string ITALIANO_NR_DESCRIPTION = string("A non recursive version of a dynamic ")+
    "transitive closure algorithm developed by Italiano. The data structure suports "+
    "edge insertions in directed graphs and edge deletions in directed acyclic graphs.";
const string ITALIANO_NR_VERSION     = "2.0";
const string ITALIANO_NR_TIMESTAMP   = "October 1997";
const string ITALIANO_NR_AUTHORS     = "Tobias Miller";

/*{\Manpage {dtc_italiano_nr} {} {Non recursive version of Italiano's dynamic transitive closure algorithm} {DTC}}*/
class dtc_italiano_nr : public dtc_italiano
  {
/*{\Mdefinition
An instance |\Mvar| of type |\Mname| dynamically maintains the transitive closure of a directed graph $G$.
The data type is a non recursive version of a partially dynamic transitive closure algorithm developed by Italiano. It efficiently supports
sequences of edge insertions or edge deletions. But edges may be removed from acyclic graphs only.

The data structure supports [[insert_edge]], [[remove_edge]], [[path]] and [[desc]] operations. 
The [[remove_edge]] operation may be used on acyclic graphs only. Method [[remove_edge]] 
does not check whether the graph is acyclic. The data structure was augmented with a [[reset]] operation.
It allows to mix [[insert_edge]] and [[remove_edge]] operations. 
To ensure correctness, [[reset]] has to be called before each [[remove_edge]] operation, that is 
preceded by an [[insert_edge]] operation. Method [[reset]] is called automatically.

Note that |\Mname| is derived from class |dtc_italiano|. Only operations [[insert_edge]] 
and [[remove_edge]] are redefined in |\Mname|. All other operations are defined in class |dtc_italiano| or |dga_base|.}*/
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
      dtc_italiano_nr(msg_graph* G) : dtc_italiano(G) {}
/*{\Mcreate Creates an instance |\Mvar| of type |\Mname| and initializes it with message graph |G|.}*/
      dtc_italiano_nr(msg_graph& G) : dtc_italiano(G) {}
/*{\Mcreate Creates an instance |\Mvar| of type |\Mname| and initializes it with message graph |G|.}*/
      ~dtc_italiano_nr() {}
/*{\Moperations}*/
/*{\Mtext We distinguish between |inherited| operations (defined in class |dga_base|), |redefined| operations (defined in class |dga_base|, 
redefined in |\Mname|) and |stand-alone| operations (defined in class |\Mname|, but not in |dga_base|).

Due to inheritance, the default parameter |weight| is declared in some redefined operations. Note however, that it is not used in class |\Mname|.
Italiano's algorithm does not consider edge or vertex weights. 

\medskip \headerline{Inherited public update operations}}*/
/*{\Moptions warnings=no}*/
/*
      void init(msg_graph *G);
*/
/*{\Mop |Inherited|: Initializes the data structure with message graph |G|. |\Mvar|.[[init]] calls 
the protected method |\Mvar|.[[new_graph]].}*/
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
/*{\Mop |Inherited|: Calls |\Mvar|.[[update]] with message graph |G| as argument. 
It is called after the insertion of a new vertex into message graph |G|.}*/
/*
      void remove_node(node v);
*/
/*{\Mop |Inherited|: Does nothing. See Section `Protected update operations' for detailed information.
It is called before the deletion of a vertex from message graph |G|.}*/
/*{\Moptions warnings=yes}*/
/*{\Mtext \medskip \headerline{Redefined public update operations}}*/
      void insert_edge(edge e, double weight = 1.0);
/*{\Mop |Redefined|: Inserts a new edge |e| into |\Mvar|. It is called after the insertion of a new edge |e| into message graph |G|.}*/
      void remove_edge(edge e);
/*{\Mop |Redefined|: Removes edge |e| from |\Mvar|. It may be used on directed acyclic graphs only. 
It is called before the deletion of an edge |e| from message graph |G|.}*/
/*{\Moptions warnings=no}*/
/*
      void set_edge_weight(edge e, double d) {}  
*/
/*{\Mop |Redefined|: By default calls |\Mvar|.update with the current message graph |G| as argument. In class
|\Mname| it does nothing, since there is no need for edge weights.}*/
/*
      void set_node_weight(node v, double d) {}
*/
/*{\Mop |Redefined|: By default calls |\Mvar|.update with the current message graph |G| as argument. In class
|\Mname| it does nothing, since there is no need for vertex weights.}*/

    protected:
/*{\Mtext \medskip \headerline{Protected Update Operations}
The following operations may not be called by the user. We list them, to give a better understanding of the data structure.
 
Class |dga_base| supports four protected operations. They handle, in conjunction with the public operations [[remove_edge]], 
[[insert_edge]], [[remove_node]] and [[insert_node]], the insertion or deletion of vertices or edges. 
Each of these protected operations has a default definition in class |dga_base|. Additionaly there are two operations, [[update]] 
and [[new_graph]], that \underline{must} be redefined in all derived classes (e.~g. |\Mname|).}*/
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
/*
      void after_edge_removal(node v, node w) {}
*/
/*{\Mop |Redefined|: Is called after the deletion of an edge |e=(v,w)|. By default calls |\Mvar|.[[update]] 
with message graph |G| as argument. It is redefined in |\Mname| to do nothing.}*/
/*
      void update(msg_graph *G);
*/
/*{\Mop |Redefined|: Initializes |\Mvar| with message graph |G|. |\Mvar|.[[update]] is the default update operation, which 
is called after each update to the graph, unless the appropriate method is redefined (see Section `Redefined public update operations').}*/
/*
      void new_graph(msg_graph *G) { update(G); }
*/
/*{\Mop |Redefined|: Calls |\Mvar|.[[update]]. It is called by method |\Mvar|.[[init]], whenever the user or an 
application switches to a new underlying message graph.}*/

    public:
/*{\Mtext \medskip \headerline{Redefined public queries}}*/
/*
      bool query(node v, node w) { return path(v, w); }
*/
/*{\Mop |Redefined|: Like |\Mvar|.[[path(v, w)]].}*/
/*{\Mtext \medskip \headerline{Stand-alone public queries}}*/
/*
      bool query(node v, node w, list<node>& Path) { return path(v, w, Path); }
*/
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path)]].}*/
/*
      bool query(node v, node w, list<edge>& Path) { return path(v, w, Path); }
*/
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path)]].}*/
/*
      bool query(node v, node w, list<node>& Path, list<edge>& Cut) 
        { return path(v, w, Path, Cut); }
*/
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path, Cut)]].}*/
/*
      bool query(node v, node w, list<edge>& Path, list<edge>& Cut) 
        { return path(v, w, Path, Cut); }
*/
/*{\Mop |Stand-alone|: Like |\Mvar|.[[path(v, w, Path, Cut)]].}*/
/*
      bool path(node v, node w);
*/
/*{\Mop |Stand-alone|: Returns |true|, if there is a path from vertex |v| to |w| in |G|. Otherwise, returns |false|.}*/
/*
      bool path(node v, node w, list<node>& Path);
*/
/*{\Mop |Stand-alone| operation. Returns |true|, if there is a path from vertex |v| to |w| in |G| and assigns the 
vertices on that path to |Path|. Otherwise, returns |false| and makes |Path| the empty list.}*/
/*
      bool path(node v, node w, list<edge>& Path);
*/
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the 
edges on that path to |Path|. Otherwise, returns |false| and makes |Path| the empty list.}*/
/*
      bool path(node v, node w, list<node>& Path, list<edge>& Cut);
*/
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the
vertices on that path to |Path|. Otherwise, returns |false| and assigns to |Cut| the edges in the cut, that separates the descendant 
vertices of |v| from all others.}*/
/*
      bool path(node v, node w, list<edge>& Path, list<edge>& Cut);
*/
/*{\Mop |Stand-alone|: Returns |true| if there is a path from vertex |v| to |w| in |G| and assigns the
edges on that path to |Path|. Otherwise, returns |false| and assigns to |Cut| the edges in the cut, that separates the descendant 
vertices of |v| from all others.}*/
/*
      void desc(node v, list<node>& V);
*/
/*{\Mop |Stand-alone|: Assigns to |V| the descendant vertices of vertex |v| in |G|.}*/
/*
      void desc(node v, list<node>& V, list<edge>& E);
*/
/*{\Mop |Stand-alone|: Assigns to |V| and |E| the vertices and edges in the descendant tree rooted at vertex |v|.}*/
/*{\Mtext \medskip \headerline{Redefined public operations}}*/
/*
      void reset();
*/
/*{\Mop |Stand-alone|: Allows to mix [[insert_edge]] and [[remove_edge]] operations. It is called automatically before 
each [[remove_edge]] operation that is preceded by an [[insert_edge]] operations.}*/
      string name() const { return ITALIANO_NR_NAME; }
      string description() const { return ITALIANO_NR_DESCRIPTION; }
      string version() const { return ITALIANO_NR_VERSION; }
      string authors() const { return ITALIANO_NR_AUTHORS; }
      string timestamp() const { return ITALIANO_NR_TIMESTAMP; }
  };

/*{\Mimplementation
The data structure realizes a non recursive version of two partially dynamic transitive closure algorithms developed by Italiano. 
For further details see G.F.~Italiano, ``Amortized efficiency of a path retrieval data structure'', {\em Theoretical Computer Science}, 
48 (1986), 273-281 and G.F.~Italiano, ``Finding paths and deleting edges in directed acyclic graphs'', {\em Information Processing 
Letters} 28:1 (1988), 5-11. 

The data structure can be initialized in $O(n*(n+m))$ time and requires $O(n^2+m)$ space, where |n| is the number of vertices and |m| 
the number of edges in the graph. Any sequence of [[insert_edge]] or [[remove_edge]] operations requires $O(n*(n+m))$ time, where |m| 
is the maximum number of edges in the graph. A [[path(node, node)]] query requires $O(1)$, a [[desc]] or a [[path(node, node, list)]] 
query $O(n)$ and a [[path(node, node, list, list)]] query $O(m+n)$ time. A [[reset]] operation requires $O(n^2)$ time.}*/
#endif _DTC_ITALIANO_NR_H
