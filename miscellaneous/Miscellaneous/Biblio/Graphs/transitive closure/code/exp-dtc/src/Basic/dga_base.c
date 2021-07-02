//------------------------------------------------------------------- //
// LEDA Extension Package Dynamic Graph Algorithms                    //
//                                                                    //
// dga_base: base class for dynamic graph algorithms                  //
//                                                                    //
// David Alberts (1997)                                               //
//------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/dga_base.h>

void dga_base::get_msg(const graph_msg& gm)
// handles messages about operations on the graph
{
  switch(gm.type)
  {
    case graph_msg::pre_new_edge:
    { before_edge_insertion(gm.v,gm.w,the_weight); break; }

    case graph_msg::post_new_edge:
    { insert_edge(gm.e,the_weight); break; }

    case graph_msg::pre_new_node:
    { before_node_insertion(the_weight); break; }

    case graph_msg::post_new_node:
    { insert_node(gm.v,the_weight); break; }

    case graph_msg::pre_del_edge:
    { remove_edge(gm.e); break; }

    case graph_msg::post_del_edge:
    { after_edge_removal(gm.v,gm.w); break; }

    case graph_msg::pre_del_node:
    { remove_node(gm.v); break; }

    case graph_msg::post_del_node:
    { after_node_removal(); break; }

    case graph_msg::pre_move_edge:
    { before_move_edge(gm.e,gm.v,gm.w); break; }

    case graph_msg::post_move_edge:
    { after_move_edge(gm.e,gm.v,gm.w); break; }

    case graph_msg::pre_clear_graph:
    { before_clear_graph(); break; }

    case graph_msg::post_clear_graph:
    { after_clear_graph(); break; }

    case graph_msg::global_query:
    { query_result = query(); break; }

    case graph_msg::nodes_query:
    { query_result = query(gm.v,gm.w); break; }

    case graph_msg::edge_query:
    { query_result = query(gm.e); break; }

    default: break;
  }
}
