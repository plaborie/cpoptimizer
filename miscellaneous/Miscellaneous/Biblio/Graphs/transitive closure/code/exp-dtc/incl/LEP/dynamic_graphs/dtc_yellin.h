// -----------------------------------------------------------------------------
//
//  File:        dtc_yellin.h
//  Date:        07/97
//  Description: Realization of Yellin's algorithm to maintain the transitive 
//               closure of a digraph dynamically using the class `dga_base'.
//               The data structure is parameterized to support either
//               find-path queries (enable_SG_update = true; default) or
//               boolean queries only (enable_SG_update = false).
//
//  (C) 1997 Guido Schaefer and Christos Zaroliagis
//
// -----------------------------------------------------------------------------


#ifndef _DTC_YELLIN_H
#define _DTC_YELLIN_H


#include <LEP/dynamic_graphs/dga_base.h>
#include <LEDA/array.h>
#include <LEDA/list.h>
#include <LEDA/string.h>
#include <LEDA/node_array.h>
#include <LEDA/node_matrix.h>
#include <LEDA/graph.h>

#define DEBUG 0  

// constants that contain information about the algorithm
const string YELLIN_NAME        = "Yellin";
const string YELLIN_DESCRIPTION = string("The selected data structure realizes Yellin's algorithm ") + 
                                  string("to maintain dynamically the transitive closure of a digraph. ") +
                                  string("The data structure allows edge insertions and query operations ") +
                                  string("for any digraph, and edge deletions only for DAG's");
const string YELLIN_NODES_YES   = "There exists a path between these two nodes.";
const string YELLIN_NODES_NO    = "There does not exist a path between these two nodes.";
const string YELLIN_VERSION     = "1.0";
const string YELLIN_TIMESTAMP   = "July 1997";
const string YELLIN_AUTHORS     = "Guido Schaefer";


/*{\Manpage {dtc_yellin} {} {Dynamic maintenance of the transitive closure of a digraph based on Yellin's algorithm} {DTC} }*/

class dtc_yellin : public dga_base 
{
/*{\Mdefinition
An instance |\Mvar| of the data type |\Mname| can be used to maintain the transitive closure of a simple digraph |G| dynamically. 

The data type |\Mname| is derived from the class |dga_base|, a base class for all dynamic graph algorithms in the LEDA Extension 
Package (LEP) for Dynamic Graph Algorithms. 
}*/ 
/*{\Mtext
\medskip
\headerline{Relation between |msg_graph|, |dga_base| and |\Mname|}
A message graph |G| of the data type |msg_graph| is a |graph| that sends messages about modifications (e.~g.~edge insertion, 
node deletion, \ldots) to interested third parties. The class |msg_graph| is derived from |graph|. So all operations on |graphs| 
can be performed on |msg_graphs| as well.  

The class |dga_base| defines basic operations for all dynamic graph algorithms in the LEP Dynamic Graph Algorithms. The purpose 
of this class is to provide a consistent interface across all different algorithms. The class uses a message graph |G|. Whenever 
|G| is modified a message is send to the class |dga_base|. By redefining the basic operations that are supported by a specific 
dynamic graph algorithm (using inheritance) it is possible to react on update operations on |G|. Furthermore it is facile to 
maintain several dynamic graph algorithms that use |dga_base| as a base class. 
}*/

  private:

    node_array< list< node > > Adjacent;
    node_array< list< node > > Reaches;

    bool enable_SG_update;

    struct index_info {
      list_item edgeTarget, closureSource;     
      node      SGnode;                        
      int       refcount;                      
    };

    node_matrix< index_info > Index;             

    void MakeEdge(const node &v, const node &w);      
    void RemoveEdge(const node &v, const node &w);    
    void MakeClosure(const node &v, const node &w);  
    void RemoveClosure(const node &v, const node &w); 

    bool find_path_rec(const node &x, const node &z, list<node> &path) const;

    struct SGnode_info {
      bool        isJoin;
      array< node > label;

      friend istream& operator >>(istream& is, SGnode_info info) { return is; }
      friend ostream& operator <<(ostream& os, SGnode_info info) { return os; }
    };

    GRAPH< SGnode_info, int > SG;               

    void CreateClosureSGnode(const node &a, const node &b);              
    void CreateJoinSGnode(const node &a, const node &b, const node &c);  
    void DeleteClosureSGnode(const node &a, const node &b);             


  public:

/*{\Mcreation}*/     

    dtc_yellin(msg_graph *G) : dga_base(G) { enable_SG_update=true; new_graph(G); }
/*{\Mcreate creates an instance |\Mvar| of type |\Mname| and initializes it with the message graph |G|.}*/

    dtc_yellin(msg_graph &G) : dga_base(&G) { enable_SG_update=true; new_graph(&G); }
/*{\Mcreate creates an instance |\Mvar| of type |\Mname| and initializes 
it with the message graph |G|.}*/
 
   ~dtc_yellin() {}

/*{\Moperations}*/
/*{\Mtext
From now on we distinguish among |inherited| operations (defined in class |dga_base|), |redefined| operations (primarily defined in 
|dga_base|, redefined in |\Mname|) and |stand-alone| operations (additional operations that are not defined in |dga_base|).

Because of inheritance and redefinition, in some operations the default parameter |weight| is declared. Note however, that we don't use this 
parameter, since |\Mname| does not require weighted nodes or edges. 

\medskip
\headerline{Inherited Public Update Operations (defined in class |dga_base|)}
}*/
/*{\Moptions warnings=no}*/

    /* inherited
    void init(msg_graph *G);
    */
/*{\Mop |inherited| operation. Initializes the dynamic data structure 
to the graph pointed to by |G|. |\Mvar|.init calls 
the protected method |\Mvar|.new\_graph.}*/

    /* inherited
    edge new_edge(node v, node w, double weight=1.0);
    */
/*{\Mop |inherited| operation. First, it calls |\Mvar|.before\_edge\_insertion, then inserts a new edge |e=(v,w)| in the underlying graph and 
finally calls |\Mvar|.insert\_edge.}*/

    /* inherited
    void delete_edge(edge e);
    */
/*{\Mop |inherited| operation. First, it calls |\Mvar|.remove\_edge, then deletes the edge |e=(v,w)| in the underlying graph, and finally calls 
|\Mvar|.after\_edge\_removal. }*/

    /* inherited
    node new_node(double weight=1.0);
    */
/*{\Mop |inherited| operation. First, it calls |\Mvar|.before\_node\_insertion, then inserts a new node |v| in the underlying graph, and finally 
calls |\Mvar|.insert\_node.}*/

    /* inherited
    void delete_node(node v);
    */
/*{\Mop |inherited| operation. First, it calls |\Mvar|.remove\_node, then deletes the node |v| in the underlying graph, and finally calls 
|\Mvar|.after\_node\_removal. }*/

    /* inherited
    void insert_node(node v, double weight = 1.0);
    */
/*{\Mop |inherited| operation. Calls |\Mvar|.update with the current message graph |G| as argument. \\ 
(NOTE: This operation is called after the insertion of a new node |v| into the message graph |G|.)}*/

    /* inherited
    void remove_node(node v);
    */
/*{\Mop |inherited| operation. This operation does nothing. See Section 
`Inherited and Redefined Protected Update Operations' for detailed information.\\ 
(NOTE: This operation is called before the deletion of a node |v| from the message graph |G|.)}*/

/*{\Moptions warnings=yes}*/

/*{\Mtext
\medskip
\headerline{Efficiently Supported Public Update Operations (redefined operations)}
}*/

    void insert_edge(edge e, double weight = 1.0); 
/*{\Mop |redefined| operation. Inserts a new edge |e| into |\Mvar|. \\ 
(NOTE: This operation is called after the insertion of a new edge |e| into the message graph |G|.)}*/

    void remove_edge(edge e);
/*{\Mop |redefined| operation. Deletes the edge |e| from |\Mvar|. This operation should be performed only if the related 
message graph |G| is acyclic. \\ (NOTE: This operation is called before the deletion of an edge |e| from the message graph |G|.)}*/

    void set_edge_weight(edge e, double d) {}  
/*{\Mop |redefined| operation. By default this operation calls |\Mvar|.update with the current message graph |G| as argument. In class
|\Mname| it does nothing, because there is no need for weighted edges.}*/

    void set_node_weight(node v, double d) {}
/*{\Mop |redefined| operation. By default this operation calls |\Mvar|.update with the current message graph |G| as argument.In class
|\Mname| it does nothing, because there is no need for weighted nodes.}*/


  protected:

/*{\Mtext
\medskip
\headerline{Inherited and Redefined Protected Update Operations}
The following operations should not be called by the user. 
Thus, they are protected methods. We list them here to give 
a better understanding of how the data structure functions.
 
The base class |dga_base| supports four protected operations that allow (in conjunction with the public operations |remove_edge|, 
|insert_edge|, |remove_node| and |insert_node|) to handle node and edge insertions (resp.~deletions) very precisely. 
Each of these operations has a default definition in the base class |dga_base|. Because of this it is probably necessary to redefine these 
operations. Furthermore, |dga_base| defines two operations |update| 
and |new_graph| that {\bf must} be redefined in all derivated 
classes (consequently in |\Mname|, too). 

A description of the default definition and their redefinition (when necessary) of the methods is given below.}*/
/*{\Moptions warnings=no}*/

      /*
      void before_edge_insertion(node v, node w, double weight = 1.0)
      */
/*{\Mop |inherited| operation. It is called before the insertion of 
the edge |(v,w)| with the weight |weight| into the graph. By default ,
this does nothing. The operation is not redefined in |\Mname|.}*/

      /* inherited
      void before_node_insertion(double weight = 1.0);     
      */
/*{\Mop |inherited| operation. It is called before the insertion of a new node with weight |weight| into the graph. By default this does 
nothing. The operation is not redefined in |\Mname|.}*/

      /* inherited
      void after_node_removal();
      */
/*{\Mop |inherited| operation. It is called after the deletion of a node from the graph. By default, |\Mvar|.update is called with the current 
graph. The operation is not redefined in |\Mname|.}*/

/*{\Moptions warnings=yes}*/

      void after_edge_removal(node v, node w) {}
/*{\Mop |redefined| operation. It is called after the deletion of an edge |(v,w)| from the graph. By default, |\Mvar|.update is called with the 
current graph. The operation is redefined in |\Mname| to do nothing.}*/

      void update(msg_graph *G);
/*{\Mop |redefined| operation. This operation initializes |\Mvar| with the message graph |G|. It is the default update operation which is 
called after each update of the graph, unless the appropriate method is redefined (see Section `Efficiently Supported Public Update 
Operations').}*/

      void new_graph(msg_graph *G) { update(G); }
/*{\Mop |redefined| operation. This operation calls the |\Mvar|.update operation. This operation is called by the |\Mvar|.init method, when the 
user or application switched to a new underlying graph.}*/


  public:
    bool query() { return false; }
    bool query(edge e) { return false; }
/*{\Mtext
\medskip
\headerline{Redefined Public Queries (primary defined in |dga_base|)}
}*/

    bool query(node v, node w) { return path(v, w); }
/*{\Mop |redefined| operation. Returns |\Mvar|.path$(v, w)$.}*/

/*{\Mtext
\medskip
\headerline{Additional Public Operations (|stand-alone| operations)}
}*/

    bool query(node v, node w, list<node> &path, list<edge> &cut) {
      return find_path(v, w, path, cut);
    }
/*{\Mop |stand-alone| operation. Returns |\Mvar|.find\_path|(v, w, path, cut)|.}*/

    bool path(const node &v, const node &w) const;   
/*{\Mop |stand-alone| operation. Returns |true| if there exists a path between nodes |v| and |w|, and |false| otherwise.}*/ 

    void desc(const node &x, list<node> &reach) const;
/*{\Mop |stand-alone| operation. Returns in |reach| the list of nodes that are reachable from node |x|.}*/

    bool find_path(const node &x, const node &z, list<node> &path) const;
/*{\Mop |stand-alone| operation. Returns |true| if there exists a path from 
node |x| to node |z| in the related message graph |G|, and assigns to 
|path| the list of nodes in the $x$-$z$ path. Otherwise, it returns
|false| and sets |path| to the empty list.}*/

    bool find_path(const node &x, const node &z, list<node> &path, list<edge> &cut) const;
/*{\Mop |stand-alone| operation. Returns |true| if there exists a path from node |x| to node |z| in the related message graph |G|, 
assigns |cut| to the empty list, and assigns to |path| the list of nodes in the $x$-$z$ path. Otherwise, it returns |false|, 
assigns |path| to the empty list, and assigns to |cut| the list of edges in the cut defined by the nodes in $G$ that are 
reachable by $x$.}*/

    void set_SG(bool b);
/*{\Mop |stand-alone| operation. It enables or disables the
support of |\Mvar|.find\_path operations. If |b| is |true|, then the 
|\Mvar|.find\_path operation is supported; otherwise, not. 
Initially, |b=true|.}*/

    string nodes_yes() const { return YELLIN_NODES_YES; }
    string nodes_no()  const { return YELLIN_NODES_NO; }

    string name() const { return YELLIN_NAME; }
    string description() const { return YELLIN_DESCRIPTION; }

    string version() const { return YELLIN_VERSION; }
    string authors() const { return YELLIN_AUTHORS; }
    string timestamp() const { return YELLIN_TIMESTAMP; }         
};


/*{\Mimplementation 
The data structure realizes Yellin's partially dynamic transitive 
closure algorithm (see D.~M. Yellin: ``Speeding up dynamic transitive 
closure for bounded degree graphs'', {\em Acta Informatica} 30 (1993), 369-384). 

The initialization of the data structure with a digraph $G_0=(V_0,E_0)$ 
takes $O(n_0^2 + d_0\cdot m_0^*)$ time and space, 
where $n_0=\Lvert V_0\Lvert$, $m_0=\Lvert E_0\Lvert$, $d_0$ 
is the maximum outdegree
of $G_0$, and $m_0^*$ is the number of edges in the transitive closure of $G_0$. 
(If the |find_path(a,b,path)| operation is not supported,
then the space required by the data structure is $O(n_0^2)$.)
Any sequence of edge insertions (resp.~deletions) intermixed 
with $p$ |path(a,b)| operations requires time $O(d \cdot m^*+p)$, where |d| 
is the maximum outdegree of the resulting (resp.~initial) digraph 
|G| and $m^*$ is the number of edges in the transitive closure of |G|. 
The operation |path(a,b)| takes $O(1)$ time. 
The operation |find_path(a,b,path)| requires, if it returns |true|,
time $O(\ell)$, where $\ell$ is the number of edges of the reported path;
otherwise, it  requires $O(1)$ time.
The operation |find_path(a,b,path,cut)| requires, if it returns |true|,
time $O(\ell)$, where $\ell$ is the number of edges of the reported path; 
otherwise, it  requires at most $O(n + m)$ time,
where $n$ (resp.~$m$) is the number of
vertices (resp.~edges) in the current graph. Note that edge insertions and
deletions can be intermixed subject to the condition that
edge insertions do not create cycles. In this case, however,
the above claimed time bounds may not hold, since Yellin's
algorithm is not fully dynamic.}*/


// -----------------------------------------------------------------------------
//  private Methods
// -----------------------------------------------------------------------------


inline void dtc_yellin :: MakeEdge(const node &v, const node &w) 
{
  Index(v,w).edgeTarget = Adjacent[v].push(w);
}


inline void dtc_yellin :: RemoveEdge(const node &v, const node &w) 
{ 
  if (Index(v,w).edgeTarget) {
    Adjacent[v].del_item(Index(v,w).edgeTarget);
    Index(v,w).edgeTarget = nil;
  }
  else if(DEBUG)
         cerr << "void dtc_yellin :: RemoveEdge( const node &v, const node &w ): \n"   
              << "Index(" << index(v) << "," << index(w) << ").edgeTarget is set to nil \n"; 
}


inline void dtc_yellin :: MakeClosure(const node &v, const node &w) 
{
  Index(v,w).closureSource = Reaches[w].push(v);
}


inline void dtc_yellin :: RemoveClosure(const node &v, const node &w) 
{  
  if (Index(v,w).closureSource) {
    Reaches[w].del_item(Index(v,w).closureSource);
    Index(v,w).closureSource = nil;
  }
  else if(DEBUG)
         cerr << "void dtc_yellin :: RemoveClosure( const node &v, const node &w ): \n"   
              << "Index(" << index(v) << "," << index(w) << ").closureSource is set to nil \n"; 
}


inline void dtc_yellin :: CreateClosureSGnode(const node &a, const node &b) 
{
  Index(a,b).SGnode = SG.new_node();    

  SG[Index(a,b).SGnode].isJoin = false;
  SG[Index(a,b).SGnode].label  = array<node> (0, a, b);
}


inline void dtc_yellin :: CreateJoinSGnode(const node &a, const node &b, const node &c) 
{
  node key = SG.new_node();
   
  SG.new_edge(Index(a,b).SGnode, key);
  SG.new_edge(Index(b,c).SGnode, key);
  SG.new_edge(key, Index(a,c).SGnode);

  SG[key].isJoin = true;
  SG[key].label = array<node> (0, a, b, c);
}


inline void dtc_yellin :: DeleteClosureSGnode(const node &a, const node &b) 
{
  if (Index(a,b).SGnode) { 
    edge e;

    forall_out_edges(e, Index(a,b).SGnode) {
      if(SG.in_succ(SG.first_in_edge(SG.target(e))) == e) {
        // Index(a,b).SGnode comes from the right side into the join vertex
        SG.del_node(SG.target(e));
      }
      else {
        // Index(a,b).SGnode comes from the left side into the join vertex
	if(SG.indeg(Index(a,b).SGnode) == 0) {
          SG.del_node(SG.target(e));
        }
      }
    }
    if(SG.indeg(Index(a,b).SGnode) == 0) {
      SG.del_node(Index(a,b).SGnode);
      Index(a,b).SGnode = nil;
    }
  }
  else if(DEBUG)
         cerr << "void dtc_yellin :: DeleteClosureSGnode( const node &a, const node &b ): \n"   
              << "Index(" << index(a) << "," << index(b) << ").SGnode is set to nil \n";    
}


inline bool dtc_yellin :: find_path_rec(const node &x, const node &z, list<node> &path) const 
{
  if(Index(x,z).edgeTarget) {      
    path.clear();
    path.push(z);
    path.push(x);
    return true;    
  }
  else {      
    node joinVertex = source(SG.first_in_edge(Index(x,z).SGnode));     
    array<node> NodeLabel = SG[joinVertex].label;
    bool erg = find_path_rec(NodeLabel[0], NodeLabel[1], path);
    path.append(NodeLabel[2]);
    return erg;
  }
}


// -----------------------------------------------------------------------------
//  public methods
// -----------------------------------------------------------------------------


void dtc_yellin :: update(msg_graph *G)
{
  Adjacent.init(*G);
  Reaches.init(*G);

  index_info obj;
  obj.refcount = 0;
  obj.edgeTarget = nil;
  obj.closureSource = nil;
  obj.SGnode = nil;
  Index.init(*G, obj);

  SG.clear();

  edge e;
 
  forall_edges(e, *G) {
    insert_edge(e);
  }
}


void dtc_yellin :: insert_edge(edge e, double weight) 
{
  list< array< node > >  worklist;
  node                   x, s, t, a, b;
  array<node>            obj, f;
  
  a = source(e);
  b = target(e);
  if (!Index(a,b).refcount) {
    MakeClosure(a,b);
    if(enable_SG_update) CreateClosureSGnode(a,b);
    obj = array<node> (0, a, b);
    worklist.push(obj);
  }
  MakeEdge(a,b);
  Index(a,b).refcount++;
  
  forall(x, Reaches[a]) {
    if (!Index(x,b).refcount) {
      MakeClosure(x,b);
      if(enable_SG_update) CreateClosureSGnode(x,b);
      obj = array<node> (0, x, b);
      worklist.push(obj);
    }
    Index(x,b).refcount++;
    if(enable_SG_update) CreateJoinSGnode(x,a,b);
  }
  
  while (!worklist.empty()) {
    f = worklist.Pop();
    s = f[0];
    t = f[1];

    forall(x, Adjacent[t]) {
      if (!Index(s,x).refcount) {
        MakeClosure(s,x);
        if(enable_SG_update) CreateClosureSGnode(s,x);
        obj = array<node> (0, s, x); 
        worklist.push(obj);
      }

      Index(s,x).refcount++;
      if(enable_SG_update) CreateJoinSGnode(s,t,x);
    }
  }
}


void dtc_yellin :: remove_edge(edge e)
{
  node a, b;
    
  a = source(e);
  b = target(e);

  if (Index(a,b).edgeTarget) {
    list< array< node > > worklist;
    node                  x, s, t;
    array< node >         obj, f;
  
    RemoveEdge(a,b);
    if(enable_SG_update) DeleteClosureSGnode(a,b);
    Index(a,b).refcount--;
    if (!Index(a,b).refcount) {
      RemoveClosure(a,b);
      obj = array<node> (0, a, b);
      worklist.push(obj);
    }
  
    forall(x, Reaches[a]) {
      Index(x,b).refcount--;
      if (!Index(x,b).refcount) {
        RemoveClosure(x,b);
        if(enable_SG_update) DeleteClosureSGnode(x,b);
        obj = array<node> (0, x, b);
        worklist.push(obj);
      }
    }
  
    while (!worklist.empty()) {
      f = worklist.Pop();
      s = f[0];
      t = f[1];

      forall(x, Adjacent[t]) {
        Index(s,x).refcount--;
        if (!Index(s,x).refcount) {
          RemoveClosure(s,x);
          if(enable_SG_update) DeleteClosureSGnode(s,x);
          obj = array<node> (0, s, x);
          worklist.push(obj);
        }
      }
    }
  }
  else if(DEBUG)
         cerr << "The edge (" << index(a) << "," << index(b) 
	      << "> which should be deleted is not in the corresponding graph -> EXIT" << endl;
}

     
bool dtc_yellin :: path(const node &v, const node &w) const
{
  return Index(v,w).refcount; 
}


void dtc_yellin :: desc(const node &x, list<node> &reach) const
{
    reach.clear();
    reach.append(x);
    
    node y;

    forall_nodes(y, *the_graph) {
      if(Index(x, y).refcount) reach.append(y);
    }
}

bool dtc_yellin :: find_path(const node &x, const node &z, list<node> &path) const
{
  if(enable_SG_update) {
     path.clear();

     if(Index(x, z).refcount) 
            return find_path_rec(x, z, path);
     else
            return Index(x,z).refcount;
  }
  else {
    cerr << "find_path operation has been called, but data structure is set" << endl
         << "not to support this operation." << endl
         << "Call set_SG(true) if you whish to enable find_path operation." << endl;
    return false;
  }
}

bool dtc_yellin :: find_path(const node &x, const node &z, list<node> &path, list<edge> &cut) const 
{
  if(enable_SG_update) {
    path.clear();
    cut.clear();  

    if(Index(x, z).refcount) return find_path_rec(x, z, path);
    else {
      graph &G = *the_graph;
      node_array<bool> marked(G, false);
      list<node> reachable;
      edge e;
      node y;

      desc(x, reachable);

      forall(y, reachable) {
        marked[y] = true;
      }        

      forall_nodes(y, G)
        if(marked[y]) 
          forall_in_edges(e, y) 
            if(!marked[source(e)]) cut.append(e);

      return false;
    }
  }
  else {
    cerr << "find_path operation has been called, but data structure is set" << endl
         << "not to support this operation." << endl
         << "Call set_SG(true) if you whish to enable find_path operation." << endl;
    return false;
  }
}


void dtc_yellin :: set_SG(bool b)
{
  if(b==enable_SG_update) return;
  else {
    enable_SG_update=b;
    if(enable_SG_update==1) init(the_graph);
    else SG.clear();
  }
}

#endif _DTC_YELLIN_H

