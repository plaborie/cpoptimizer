#ifndef LEPDGA_MSG_GRAPH_COPY_H
#define LEPDGA_MSG_GRAPH_COPY_H

//------------------------------------------------------------------- //
// LEDA Extension Package Dynamic Graph Algorithms                    //
//                                                                    //
// msg_graph_copy: copy of a msg_graph, maintains a mapping           //
//                                                                    //
// David Alberts (1997)                                               //
//------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/msg_graph.h>


/*{\Manpage {msg_graph_copy} {} {Dynamic Copy of a Message Graph} }*/

class msg_graph_copy : public msg_graph, public get_graph_msg
{

/*{\Mdefinition
}*/

  public:

/*{\Mcreation gc }*/

    msg_graph_copy(msg_graph* G) : orig(G), get_graph_msg(G)
    { mgc_init_maps(G); }

/*{\Mcreate creates an object |\Mvar| of type |\Mname| which is a dynamic
            copy of the graph pointed to by |G|. }*/

    msg_graph_copy(msg_graph& G) : orig(&G), get_graph_msg(&G)
    { mgc_init_maps(&G); }

/*{\Mcreate creates an object |\Mvar| of type |\Mname| which is a dynamic
            copy of the graph |G|. }*/


    virtual ~msg_graph_copy() {}

/*{\Moperations}*/

/*{\Mtext

  The class |\Mvar| is derived from the class |msg_graph|, thus all
  operations on |msg_graph| are supported. Moreover, there are operations
  for getting the copy of a node or an edge of the original graph and
  for getting the original node or edge given a node or edge in the copy,
  respectively.

  Since it is possible to delete or insert nodes in the copy without
  changing the source, it is possible that after some updates for example
  there is no longer a copy of a certain node in the original graph or
  that there are edges in the copy which do not correspond to an edge in
  the original. The functions for maintaining the mapping between the
  original graph and the copy return |nil| in these cases.

}*/
    void mgc_init(msg_graph* G);
/*{\Mop reinitializes |\Mname| to be a copy of |G|. }*/

    node original(node v) { return original_node[v]; }
/*{\Mop returns for |v| being a node of |\Mname| the node in the original
        graph that |v| is a copy of or |nil|.

        \precond |v| is a node of |\Mname|.}*/

    edge original(edge e) { return original_edge[e]; }
/*{\Mop returns for |e| being an edge of |\Mname| the edge in the original
        graph that |e| is a copy of or |nil|.

        \precond |e| is an edge of |\Mname|.}*/

    node copy_of(node v) { return copy_node[v]; }
/*{\Mop returns for a node |v| in the original graph its copy in |\Mname|
        or |nil|.

        \precond |v| is a node in the original graph.}*/

    edge copy_of(edge e) { return copy_edge[e]; }
/*{\Mop returns for an edge |e| in the original graph its copy in
        |\Mname| or |nil|.

        \precond |e| is an edge in the original graph.}*/

  protected:

    void pre_del_node_handler(node v)
    {
      node w = original(v);
      if(w) copy_node[w] = nil;
      msg_graph::pre_del_node_handler(v);
    }

    void pre_del_edge_handler(edge e)
    {
      edge f = original(e);
      if(f) copy_edge[f] = nil;
      msg_graph::pre_del_edge_handler(e);
    }

    void get_msg(const graph_msg&);

  private:

    void mgc_init_maps(msg_graph* G);

    msg_graph* orig;

    node_map<node> original_node;
    edge_map<edge> original_edge;

    node_map<node> copy_node;
    edge_map<edge> copy_edge;

/*{\Mimplementation

|\Mvar| is derived from |msg_graph| and implemented using the classes
|node_map| and |edge_map|.

}*/

};

#endif
