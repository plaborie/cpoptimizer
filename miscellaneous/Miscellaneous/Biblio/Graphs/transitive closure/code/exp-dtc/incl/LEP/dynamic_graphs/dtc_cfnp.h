//-----------------------------------------------------
// dtc_cfnp.h : header of the dtc_cfnp class
// The class implements the Dynamic Transitive Closure
// by Cicerone,Frigioni,Nanni and Pugliese on directed graphs.
//
// Giulio Pasqualone 1997
//-----------------------------------------------------


#ifndef _DTC_CFNP_H
#define _DTC_CFNP_H 

#include<LEDA/graph.h>
#include<LEDA/list.h>
#include<LEDA/node_array.h>
#include<LEDA/basic.h>
#include<LEDA/node_map.h>
#include<LEDA/graph_alg.h>

#include<LEP/dynamic_graphs/dga_base.h>

//---------------------------------------------------------------------------
#ifndef _DYN_NODE_MATRIX
#define _DYN_NODE_MATRIX

/* class similar to the node_matrix, but working on a dynamic number of the
   nodes of the graph */
  template<class type>
  class dyn_node_matrix
  {
    private:
      node_map<node_map<type>*> M;
      graph* G;//the dyn_node_matrix in on this graph 
      bool initialized;
    public:
      dyn_node_matrix() { initialized=false; }
      ~dyn_node_matrix() { reset(); }
      void reset() //remove the node_matrix and release memory 
           { node v;  
             forall_nodes(v,*G)
                delete M[v];
           }
      void set(type dato_iniz)//set all data to dato_iniz
           { node v,w;
             forall_nodes(v,*G)
               forall_nodes(w,*G)
                  M[v]->operator[](w)=dato_iniz;
           }
      void init(graph& grafo, type dato_iniz)
           { if (initialized) reset();
             initialized=true;
             G=&grafo;
             M.init(grafo);
             node v;
             forall_nodes(v,grafo)
               { M[v]=new node_map<type>;
                 M[v]->init(grafo,dato_iniz);
               }
          }
    /* new_node MUST be executed AFTER the new_node on the pointed graph:
       in this way one row and one column are added for the node v */ 
      void new_node(node v,type dato_iniz)
            { node w;
              M[v]=new node_map<type>;
              M[v]->init(*G,dato_iniz);
              forall_nodes(w,*G)
                if (v!=w)
                   M[w]->operator[](v)=dato_iniz;
            }
    //del_node MUST be executed BEFORE del_node on the pointed graph
      void del_node(node v)
            { delete M[v];}
      type& operator()(node v,node w)
            { return M[v]->operator[](w);}
  }; //end of class dyn_node_matrix

#endif DYN_NODE_MATRIX
//----------------------------------------------------------------------------

/*{\Manpage {dtc_cfnp} {} {Dynamic maintenance of the Transitive Closure of a digraph based on Cicerone, Frigioni, Nanni and Pugliese's 
algorithm} {DTC}}*/

class dtc_cfnp : public dga_base
{
/*{\Mdefinition
An instance |\Mvar| of the data type |\Mname| dynamically maintains the
Transitive Closure on a directed graph $G=(V, E)$.  

The data type |\Mname| is derived from the class |dga_base|, a base class for all dynamic graph algorithms in the LEDA 
Extension Package (LEP) for Dynamic Graph Algorithms. 

The data structure allows [[new_edge]], [[delete_edge]],
[[new_node]], [[delete_node]], [[query]] on a couple of nodes
(it returns whether a path exists or not) and a global [[query]] 
(returns true if the graph is acyclic).
The [[delete_edge]] operation must 
be used on directed acyclic graphs only. The method [[delete_edge]] 
does not check whether the graph is acyclic or not. 
}*/

 msg_graph* MGP; //pointer to the message graph
 dyn_node_matrix<int> TC;
 void closure(node,node);
 void transitive_closure();        
 void initialize_new_g(msg_graph&);
 void initialize_same_g(msg_graph&);
 void tc_edge_insertion(node,node);/* NOTE: function updates ONLY the data
                                          structure and MUST follow the
                                          insertion on the graph */
 void tc_edge_deletion(node,node);/* NOTE: function updates ONLY the data
                                          structure and MUST precede the
                                          deletion on the graph */
 void tc_node_insertion(node); /* NOTE: function updates only the data
                                         structure and MUST follow the 
                                         insertion on the graph */
 void tc_node_deletion(node);/* NOTE: node MUST be isolated,
                                     function must precede the
                                     deletion on the graph */
 bool path(node,node);

 public:
/*{\Mcreation}*/     

 dtc_cfnp(msg_graph* G) : dga_base(G) { initialize_new_g(*G); }
/*{\Mcreate}*/

 dtc_cfnp(msg_graph& G) : dga_base(&G) { initialize_new_g(G); }
/*{\Mcreate creates an instance |\Mvar| of type |\Mname| and initializes it with the underlying message graph |G|.}*/

 ~dtc_cfnp() { }

/*{\Moperations}*/
/*{\Mtext
The following methods are all usable for the user.
The dynamic ones are: [[new_node]], [[delete_node]], [[new_edge]]
and [[delete_edge]].
 
Note that a [[delete_node]] involves as many [[delete_edge]] as the number
of incident edges on the node.
\headerline{Update Operations}
}*/

/*{\Moptions warnings=no}*/

  void insert_edge(edge e,double weight=1.0)
                    { tc_edge_insertion(MGP->source(e),MGP->target(e));
                    }

 /*
  edge new_edge(node v,node w)
 */
/*{\Mop inserts a new edge |(v,w)| into |\Mvar|.}*/
 
 /*
  void delete_edge(edge e); 
 */
/*{\Mop deletes the edge |e| from |\Mvar|.}*/

  void after_edge_removal(node v,node w) { tc_edge_deletion(v,w); }
  void insert_node(node v,double weight=1.0) { tc_node_insertion(v); }

 /*
  node new_node(); 
 */
/*{\Mop inserts a new node into |\Mvar|.}*/

  void remove_node(node v) 
              { 
               tc_node_deletion(v);
              }

 /* 
  void delete_node(node v)
 */
/*{\Mop deletes a node |v| from |\Mvar|.}*/

/*{\Moptions warnings=yes}*/
                    
  void after_node_removal() { /*update not necessary*/ }         
  void set_edge_weight(edge,double weight=1.0) {}
  void set_node_weight(node,double weight=1.0) {}
  void update(msg_graph* G) { initialize_same_g(*G); }
  void new_graph(msg_graph* G) { initialize_new_g(*G); }
  void before_clear_graph() { TC.reset(); }
  void after_clear_graph() { }                               
  
//QUERIES
/*{\Mtext
\headerline{Query Operations}
}*/

  bool query() { node_array<int> dummy(*MGP);
                 return TOPSORT(*MGP,dummy);
               }
/*{\Mop returns true if the underlying graph is cyclic, false otherwise.}*/

  bool query(node v,node w) { return path(v,w); }
/*{\Mop returns true if there exist a path from node |u| to node |v|, false otherwise.}*/

  bool query(edge e) { return dga_base::query(e); }
  /* dummy for compatibility */

//MESSAGES
/*{\Mtext
\headerline{Message Operations}
}*/

  string global_yes() const { return string("Graph is acyclic"); }
/*{\Mop returns the message for the case |true| of the [[query()]].}*/

  string global_no() const { return string("Graph is CYCLIC: deletions could ")
                                  + string("lead to WRONG results !");
                           }
/*{\Mop returns the message for the case |false| of the [[query()]].}*/

  string nodes_yes() const { return string("A path exists"); }
/*{\Mop returns the message for the case |true| of the [[query(v,w)]].}*/

  string nodes_no() const { return string("No existing path"); }
/*{\Mop returns the message for the case |false| of the [[query(v,w)]].}*/

//DESCRIPTION
/*{\Mtext
\headerline{Description Operations}
}*/

  string name() const { return string("CFNP"); }
/*{\Mop returns the name of the authors of the paper the algorithm is derived
from.}*/

  string description() const 
                     { return 
                           string("The data structure dtc_cfnp implements the ")
                           + string("Dynamic Transitive Closure by ") 
                           + string("Cicerone, Frigioni, Nanni and Pugliese. ") 
                           + string("Insertions and deletions of edges and ") 
                           + string("nodes, global and nodes ")
                           + string("queries are supported.");
                     }
/*{\Mop returns a brief description about the class.}*/

  string version() const { return string("1.0"); }
/*{\Mop returns the current version of the class.}*/
 
  string authors() const { return string("Giulio Pasqualone"); }
/*{\Mop returns the name of the authors of the implementation.}*/

  string timestamp() const { return string("28 May 1997"); }
/*{\Mop returns the date of the current implementation.}*/

   };
#endif _DTC_CFNP_H
