#ifndef LEPDGA_DGA_BASE_H
#define LEPDGA_DGA_BASE_H

//------------------------------------------------------------------- //
// LEDA Extension Package Dynamic Graph Algorithms                    //
//                                                                    //
// dga_base: base class for dynamic graph algorithms                  //
//                                                                    //
// David Alberts (1997/98)                                            //
//------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/msg_graph.h>

/*{\Manpage {dga_base} {} {Base Class for Dynamic Graph Algorithms} }*/

class __exportC dga_base : public get_graph_msg
{

/*{\Mdefinition

  The class |dga_base| is the base for all dynamic graph algorithms
  in the LEP Dynamic Graph Algorithms. Use inheritance for ``real''
  dynamic graph algorithms.

  The purpose of this class is to provide a consistent interface
  across all different algorithms, and thus to provide a means
  for highly reusable code. Moreover, this class handles the synchronization
  with the underlying graph, e.g., if you insert an edge into
  some graph |G| the corresponding |insert_edge| method is automatically
  called for all dynamic graph algorithms working on |G|.

  There is no default way of storing edge or node weights, although the
  corresponding methods are already there. If you want to use node or
  edge weights you have to supply some mechanism (e.g., some |map|) in your
  derived class. Otherwise you cannot retrieve weights formerly stored
  with $set\_\ldots\_weight$ by $get\_\ldots\_weight$. Instead you will
  always get back 1 indepently of what you set before.

  Most dynamic graph data structures support only a subset of all
  possible updates, e.g., insertion and deletion of edges. Thus, typically
  there are some updates which the data structure cannot handle
  efficiently. By default the |update| procedure is called after each
  update, i.e., the data structure is recomputed from scratch.

  In your derived class you have to redefine |update| such that it
  recomputes your data structure from scratch for a given graph.
  Moreover, redefine the methods which are supported efficiently by
  your data structure, such that they do not call |update|, but your
  specific update procedure.

  \textbf{Important:} Your data structure is not allowed to change the
  underlying graph (e.g. by inserting dummy edges which help in maintaining
  some property). It should only react on changes in the graph.
  This has technical reasons on the one hand, and on the other hand
  it makes several concurrent dynamic graph data structures on the
  same graph impossible, because of lack of transparency.

}*/

  public:

/*{\Mcreation dgab }*/

    dga_base(msg_graph& G)
    : the_graph(&G), the_weight(1.0), get_graph_msg(&G) {}
/*{\Mcreate creates an object |\Mvar| of type |\Mname| for the graph
            |G|. }*/

    dga_base(msg_graph* G)
    : the_graph(G), the_weight(1.0), get_graph_msg(G) {}
/*{\Mcreate creates an object |\Mvar| of type |\Mname| for the graph
            pointed to by |G|. }*/

    virtual ~dga_base() {}


/*{\Moperations 1.5 5.5 }*/

/*{\Mtext {\bf Fixed Public Update Operations}

  Do not redefine the following methods which are not virtual in your
  derived classes, but the methods below.

}*/

    void init(msg_graph* G) { the_graph = G; ggm_init(G); new_graph(G); }
/*{\Mop changes the underlying graph of the dynamic data structure to
        the graph pointed to by |G|. |init| calls the protected
        method |new_graph| which can be redefined in derived classes. }*/


/*{\Mtext {\bf Changeable Public Update Operations}

  If you define a method, e.g., for inserting an edge into the dynamic
  graph data structure, then instead of creating the edge by yourself
  redefine the method |insert_edge| which takes an already created new edge.
  This way it is possible to react on updates of the graph which are not
  created by the $dga\_base::new\_\ldots$ or $dga\_base::delete\_\ldots$ 
  methods. For example, |dga_base::new_edge| creates an edge and then
  calls |dga_base::insert_edge|. If a user creates a new edge in the
  underlying graph using |msg_graph::new_edge|, then |dga_base::insert_edge|
  is called, too, but |dga_base::new_edge| is not called.

  |insert_edge| and |insert_node| call |update| for the new graph by
  default. The same holds for removing nodes or edges, but since the
  corresponding node or edge is still present in the graph when
  |remove_edge| or |remove_node| are called, they do nothing by
  default, and |update| is called in |after_edge_removal| or
  |after_node_removal|, respectively (see below).
  If the whole graph is deleted by |msg_graph::clear|, |update| 
  is called, too.

  Currently, it is not possible to synchronize edge or node weight
  changes. This means that it is not possible just to change, e.g., an
  edge weight in the underlying graph |G|, and all dynamic graph data
  structures working on |G| are updated accordingly. Instead, you have
  to update the data structures explicitly by calling $set\_\ldots\_weight$
  after a weight change. By default this results in calling |update|, too.
  If your data structure does not care about weight changes, you should
  redefine $change\_\ldots\_weight$ to do nothing.

}*/

    virtual void insert_edge(edge e, double weight=1.0)
    { update(the_graph); }
/*{\Mop is called after the insertion of a new edge |e| with weight |weight|
        into the graph. }*/

    virtual void remove_edge(edge e) {}
/*{\Mop is called, if edge |e| is going to be deleted from the graph. }*/

    virtual void insert_node(node v, double weight=1.0)
    { update(the_graph); }
/*{\Mop is called after the insertion of a new node |v| with weight |weight|
        into the graph. }*/

    virtual void remove_node(node v) {}
/*{\Mop is called, if node |v| is going to be deleted from the graph. }*/

    virtual void set_edge_weight(edge e, double d) { update(the_graph); }
/*{\Mop sets the weight of edge |e| to the double |d|. }*/

    virtual void set_node_weight(node v, double d) { update(the_graph); }
/*{\Mop sets the weight of edge |e| to the double |d|. }*/



/*{\Mtext {\bf Changeable Protected Update Operations}

  The following update methods should not be called directly by the
  users. Thus, they are protected methods.

  You should at least redefine |update| and |new_graph|, such that the
  former reinitializes your data after an update which is not efficiently
  supported, whereas the latter reinitializes your data structure for
  a totally new graphsupplied by the user via |init|.

  Some data structures cannot be cleanly updated after some change
  in the graph has happened. Therefore,  a set of methods is provided
  which are invoked before an update in the graph happens. By redefining
  them you can react appropriately.

}*/

  protected:


    virtual void before_edge_insertion(node v, node w, double weight=1.0) {}
/*{\Mop is called before the insertion of edge |(v,w)| with weight |weight|
        into the graph. This does nothing by default. }*/

    virtual void after_edge_removal(node v, node w) { update(the_graph); }
/*{\Mop is called after the deletion of edge |(v,w)| from the graph. 
        By default, |init| is called with the current graph. }*/

    virtual void before_node_insertion(double weight=1.0) {}
/*{\Mop is called before the insertion of a new node with weight |weight|
        into the graph. This does nothing by default. }*/

    virtual void after_node_removal() { update(the_graph); }
/*{\Mop is called after the deletion of a node from the graph.
        By default, |update| is called with the current graph. }*/

    virtual void before_move_edge(edge e, node v, node w) { remove_edge(e); }
/*{\Mop is called before edge |e| is moved to |(v,w)|. By default,
        |remove_edge(e)| is called. }*/

    virtual void after_move_edge(edge e, node v, node w) { insert_edge(e); }
/*{\Mop is called after edge |e| was moved, where |v| and |w| are the
        the old source and target of |e|, respectively. By default, 
        |insert_edge(e)| is called. }*/

    virtual void before_clear_graph() {}
/*{\Mop is called before the current graph is cleared by |graph::clear_graph|.
        By default, this does nothing. }*/

    virtual void after_clear_graph() { update(the_graph); }
/*{\Mop is called after a graph is cleared by |graph::clear_graph|.
        By default, |update| is called. }*/

    virtual void update(msg_graph* G) {}
/*{\Mop is the default update operation which is called after each
        update of the graph, unless you redefine the appropriate method,
        if there exists a more efficient procedure for this kind of
        update in your derived class. }*/

    virtual void new_graph(msg_graph* G) {}
/*{\Mop is called by the |init| method, when the user or application
        switched to a new underlying graph. }*/



  public:

/*{\Mtext {\bf Queries}

  By default all queries return |false|. Redefine the
  supported queries in your derived class.

  If you want to have a more descriptive name for your
  specific query (e.g., |connected(v,w)| instead of
  |query(v,w)|) you should nevertheless redefine the
  appropiate query and add a wrapper (e.g., you define
  |query(v,w)| somewhere to do the real work and put
  \mbox{\tt bool} \mbox{\tt connected(v,w)}
  \mbox{\tt \{ return query(v,w); \}} into the
  header file). This way it is possible to write generic
  programs which work with several data structures calling
  their different queries using always the same interface.

  If you are using a |msg_graph| with several other
  data structures receiving messages from it, then
  |msg_graph::query| returns just one of the several
  results which might be supplied. If you want to be
  sure to get the right result, you can use 
  |dga_base::get_last_result| for the appropriate data
  structure.

  Often, a certain subgraph is maintained. The method
  |subgraph| should be defined to return the edges in
  this subgraph, if possible.

}*/

    virtual bool query() { return false; }
/*{\Mop is a global query (e.g., planarity). }*/

    virtual bool query(node v, node w) { return false; }
/*{\Mop is a query conecrning two nodes (e.g., ``same connected
        component?''). You can use this for queries on just one
        node, too. }*/

    virtual bool query(edge e) { return false; }
/*{\Mop is a query concerning an edge (e.g., ``Does |e| belong to
        the current matching?''). }*/

    virtual double get_edge_weight(edge e) { return 1.0; }
/*{\Mop returns the weight of |e| (1.0 by default). }*/

    virtual double get_node_weight(node v) { return 1.0; }
/*{\Mop returns the weight of |v| (1.0 by default). }*/

    virtual bool get_last_result() { return query_result; }
/*{\Mop returns the result of the last query (or |false| if no query
        has been executed so far). }*/

    virtual list<edge> subgraph() { list<edge> el; return el; }
/*{\Mop returns the subgraph maintained by this data structure, if
        appropriate. }*/


/*{\Mtext {\bf Information for a User Interface}

  The following methods are a means of providing specific
  information on a dynamic graph algorithm in programs
  which are nevertheless generic.

  Redefine the following methods to provide information on
  your dynamic graph data structure. Define |description|
  such that it describes the purpose of the data structure,
  gives some hints on the implementation if possible, contains
  a reference for the theoretical background if appropriate,
  and lists the operations which are supported efficiently.

}*/   

    virtual string name() const { return string("dga_base"); }
/*{\Mop returns the name of this dynamic graph algorithm. }*/

    virtual string description() const
    { return string("This is the base class for all ")
           + string("algorithms in the LEP Dynamic Graph Algorithms."); }
/*{\Mop returns a description of this dynamic graph algorithm. }*/

    virtual string global_yes() const
    { return string("The graph is foobared."); }
/*{\Mop returns a string to be used in a user interface in case
        of a global query returning |true|. }*/

    virtual string global_no() const
    { return string("The graph is not foobared."); }
/*{\Mop returns a string for a global query returning |false|. }*/

    virtual string nodes_yes() const
    { return string("These nodes are foobared."); }
/*{\Mop returns a string to be used in a user interface in case
        of a nodes query returning |true|. }*/

    virtual string nodes_no() const
    { return string("These nodes are not foobared."); }
/*{\Mop returns a string for a nodes query returning |false|. }*/

    virtual string edge_yes() const
    { return string("This edge is foobared."); }
/*{\Mop returns a string to be used in a user interface in case
        of an edge query returning |true|. }*/

    virtual string edge_no() const
    { return string("This edge is not foobared."); }
/*{\Mop returns a string for an edge query returning |false|. }*/



/*{\Mtext {\bf Information on the Implementation} 

          Redefine the following methods to provide information on
          the implementation of your dynamic graph data structure.
}*/

    virtual string version() const { return string("0.3"); }
/*{\Mop returns the version of the implementation. }*/

    virtual string timestamp() const { return string("27 June 1997"); }
/*{\Mop returns the time of the last modification of the implementation. }*/

    virtual string authors() const { return string("D. Alberts"); }
/*{\Mop returns the name(s) of the author(s) of this implementation. }*/


  protected:

    msg_graph* the_graph;     // the underlying graph

    virtual void get_msg(const graph_msg& msg);
    // handles synchronization of the data structure with
    // the underlying graph

  private:

    double the_weight;        // used for temporarily storing the weight
                              // of an inserted edge/node

};
#endif
