//------------------------------------------------------------------- //
// LEDA Extension Package Dynamic Graph Algorithms                    //
//                                                                    //
// msg_graph_copy: copy of a msg_graph, maintains a mapping           //
//                                                                    //
// David Alberts (1997)                                               //
//------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/impl/msg_graph_copy.h>

void msg_graph_copy::mgc_init(msg_graph* G)
{
  orig = G;         // remember original graph
  ggm_init(G);      // get messages about updates from G from now on...
  mgc_init_maps(G); // reinitialize mappings
}

void msg_graph_copy::get_msg(const graph_msg& gm)
{
  node v,w;
  edge e,f;

  switch(gm.type)
  {
    case graph_msg::post_new_edge:
    {
      v = copy_of(source(gm.e));
      w = copy_of(target(gm.e));
      if(v && w)                 // create new edge only, if copies of source
      {                          // and target exist
        e = new_edge(v,w);
        original_edge[e] = gm.e;
        copy_edge[gm.e] = e;
      }
      break;
    }

    case graph_msg::post_new_node:
    {
      v = new_node();
      original_node[v] = gm.v;
      copy_node[gm.v] = v;
      break;
    }

    case graph_msg::pre_del_edge:
    {
      e = copy_of(gm.e);
      if(e) del_edge(e); // if there is a copy of the edge, delete it
      break;
    }

    case graph_msg::pre_del_node:
    {
      v = copy_of(gm.v);
      if(v) del_node(v);
      break;
    }

    case graph_msg::post_move_edge:
    {
      e = copy_of(gm.e);
      if(e)                        // if there is a copy of the edge
      {
        v = copy_of(source(gm.e)); // if there is a copy of the edge
        w = copy_of(target(gm.e)); // then copies of src & tg exist, too
        move_edge(e,v,w);          // thus, move the edge
        // N.B.: no maps have to be changed!
      }        
      break;
    }

    case graph_msg::post_clear_graph:
    { mgc_init(orig); break; }   // reinitialize for empty original graph

    default: break;
  }
}

void msg_graph_copy::mgc_init_maps(msg_graph* G)
{
  // reset graph and maps
  clear();                        // kill current nodes and edges
  original_node.init(*this,nil);
  original_edge.init(*this,nil);
  copy_node.init(*G,nil);
  copy_edge.init(*G,nil);

  node v,w;
  edge e,f;

  // calculate new mappings
  forall_nodes(v,*G)
  {
    w = new_node();
    original_node[w] = v;
    copy_node[v] = w;
  }

  forall_edges(e,*G)
  {
    v = copy_of(source(e));
    w = copy_of(target(e));
    f = new_edge(v,w);
    original_edge[f] = e;
    copy_edge[e] = f;
  }
}
