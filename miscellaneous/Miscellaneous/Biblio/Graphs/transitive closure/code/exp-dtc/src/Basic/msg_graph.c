//------------------------------------------------------------------- //
// msg_graph, derived from graph, adds some extra features            //
//                                                                    //
// David Alberts (1997)                                               //
//------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/msg_graph.h>

// ------------------------------------------------------------------ //
// class graph_msg                                                    //
// ------------------------------------------------------------------ //

ostream& operator << (ostream& out, const graph_msg& gu)
{
  switch(gu.type)
  {
    // updates
    case graph_msg::pre_new_node:
    { out << "pre_new_node()\n" << flush; break; }
    case graph_msg::post_new_node:
    { out << "post_new_node(" << index(gu.v) << ")\n" << flush; break; }
    case graph_msg::pre_new_edge:
    { out << "pre_new_edge(" << index(gu.v) << "," << index(gu.w);
      out << ")\n" << flush; break; }
    case graph_msg::post_new_edge:
    { out << "post_new_edge(" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << ")\n" << flush; break; }
    case graph_msg::pre_del_node:
    { out << "pre_del_node(" << index(gu.v) << ")\n" << flush; break; }
    case graph_msg::post_del_node:
    { out << "post_del_node()\n" << flush; break; }
    case graph_msg::pre_del_edge:
    { out << "pre_del_edge(" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << ")\n" << flush; break; }
    case graph_msg::post_del_edge:
    { out << "post_del_edge(" << index(gu.v) << ",";
      out << index(gu.w) << ")\n" << flush; break; }
    case graph_msg::pre_move_edge:
    { out << "pre_move_edge((" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << "),";
      out << index(gu.v) << "," << index(gu.w) << ")\n" << flush; break; }
    case graph_msg::post_move_edge:
    { out << "post_move_edge((" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << "),";
      out << index(gu.v) << "," << index(gu.w) << ")\n" << flush; break; }
    case graph_msg::pre_clear_graph: { out << "pre_clear()\n" << flush; break; }
    case graph_msg::post_clear_graph: { out << "post_clear()\n" << flush; break; }

    // const methods
    case graph_msg::touch_node:
    { out << "touch_node(" << index(gu.v) << "," << '"' << gu.s;
      out << '"' << ")\n" << flush; break; }
    case graph_msg::touch_edge:
    { out << "touch_edge(" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << "," << '"';
      out << gu.s << '"' << ")\n" << flush; break; }
    case graph_msg::global_query:
    { out << "global_query()\n" << flush; break; }
    case graph_msg::nodes_query:
    { out << "nodes_query(" << index(gu.v) << "," << index(gu.w);
      out << ")\n" << flush; break; }
    case graph_msg::edge_query:
    { out << "edge_query(" << index(source(gu.e)) << ",";
      out << index(target(gu.e)) << ")\n" << flush; break; }
    case graph_msg::comment:
    { out << "comment(" << '"' << gu.s << '"' << ")\n" << flush; break; }

    default:
    { out << "warning: unrecognized operation\n" << flush; break; }
  }
  return out;
}

void msg_graph::pre_del_node_handler(node v)
{

#if __LEDA__ == 350
  // BEGIN WORKAROUND
  edge e;
  while ((e=first_adj_edge(v)) != nil) 
    del_edge(e);
  while ((e=first_in_edge(v)) != nil) 
    del_edge(e);
  // END WORKAROUND
  // This workaround is necessary in LEDA 3.5, because the pre_del_node_handler
  // is called before deleting adjacent edges of the node, which leads to
  // different kinds of problems.
  // This is fixed in LEDA 3.5.1.
#endif

  the_msg->type = graph_msg::pre_del_node;
  the_msg->v = v;
  send();
}
