// ------------------------------------------------------------------- //
// class graph_recorder                                                //
//                                                                     //
// record and replay updates of a msg_graph                            //
// David Alberts (1997/98)                                             //
//                                                                     //
// Modified by G. Prasinos & C. Zaroliagis (2002) to run with LEDA-4.2 //
// ------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/graph_recorder.h>
#include<LEDA/stream.h>
#include<ctype.h>
#include<LEDA/dictionary.h>


// ------------------------------------------------------------ //
// class graph_update                                           //
// ------------------------------------------------------------ //

graph_update::graph_update(const graph_msg& gm)
{
  switch(gm.type)
  {
    // updates
    case graph_msg::pre_new_node:
    {
      type = new_node;
      break;
    }
    case graph_msg::post_new_node:
    {
      type = new_node;
      v = index(gm.v);
      break;
    }
    case graph_msg::pre_new_edge:
    {
      type = new_edge;
      v = index(gm.v);
      w = index(gm.w);
      break;
    }
    case graph_msg::post_new_edge:
    {
      type = new_edge;
      v = index(source(gm.e));
      w = index(target(gm.e));
      break;
    }
    case graph_msg::pre_del_node:
    {
      type = del_node;
      v = index(gm.v);
      break;
    }
    case graph_msg::post_del_node:
    {
      type = del_node;
      break;
    }
    case graph_msg::pre_del_edge:
    {
      type = del_edge;
      v = index(source(gm.e));
      w = index(target(gm.e));
      break;
    }
    case graph_msg::post_del_edge:
    {
      type = del_edge;
      v = index(gm.v);
      w = index(gm.w);
      break;
    }
    case graph_msg::pre_move_edge:
    {
      type = move_edge;
      v = index(source(gm.e));
      w = index(target(gm.e));
      x = index(gm.v);
      y = index(gm.w);
      break;
    }
    case graph_msg::post_move_edge:
    {
      type = move_edge;
      v = index(source(gm.e));
      w = index(target(gm.e));
      x = index(gm.v);
      y = index(gm.w);
      break;
    }
    case graph_msg::pre_clear_graph:
    {
      type = clear_graph;
      break;
    }

    // const methods
    case graph_msg::touch_node:
    {
      type = touch_node;
      v = index(gm.v);
      s = gm.s;
      break;
    }
    case graph_msg::touch_edge:
    {
      type = touch_edge;
      v = index(source(gm.e));
      w = index(target(gm.e));
      s = gm.s;
      break;
    }
    case graph_msg::global_query:
    {
      type = global_query;
      break;
    }
    case graph_msg::nodes_query:
    {
      type = nodes_query;
      v = index(gm.v);
      w = index(gm.w);
      break;
    }
    case graph_msg::edge_query:
    {
      type = edge_query;
      v = index(source(gm.e));
      w = index(target(gm.e));
      break;
    }
    case graph_msg::comment:
    {
      type = comment;
      s = gm.s;
      break;
    }
    default: break;
  }
}

ostream& operator << (ostream& out, const graph_update& gu)
{
  switch(gu.type)
  {
    case graph_update::new_node:
    { out << "new_node(" << gu.v << ")\n" << flush; break; }
    case graph_update::new_edge:
    { out << "new_edge(" << gu.v << "," << gu.w << ")\n" << flush; break; }
    case graph_update::del_node:
    { out << "del_node(" << gu.v << ")\n" << flush; break; }
    case graph_update::del_edge:
    { out << "del_edge(" << gu.v << "," << gu.w << ")\n" << flush; break; }
    case graph_update::move_edge:
    { out << "move_edge((" << gu.v << "," << gu.w << "),";
      out << gu.x << "," << gu.y << ")\n" << flush; break; }
    case graph_update::clear_graph: { out << "clear()\n" << flush; break; }
    case graph_update::touch_node:
    { out << "touch_node(" << gu.v << "," << '"' << gu.s << '"';
      out << ")\n" << flush; break; }
    case graph_update::touch_edge:
    { out << "touch_edge(" << gu.v << "," << gu.w << ",";
      out << '"' << gu.s << '"' << ")\n" << flush; break; }
    case graph_update::global_query:
    { out << "global_query()\n" << flush; break; }
    case graph_update::nodes_query:
    { out << "nodes_query(" << gu.v << "," << gu.w << ")\n" << flush; break; }
    case graph_update::edge_query:
    { out << "edge_query(" << gu.v << "," << gu.w << ")\n" << flush; break; }
    case graph_update::comment:
    { out << "comment(" << '"' << gu.s << '"' << ")\n" << flush; break; }
  }
  return out;
}

istream& operator >> (istream& in, graph_update& gu)
{
  dictionary<string,graph_update::update_type> types;
  types.insert("new_node",graph_update::new_node);
  types.insert("new_edge",graph_update::new_edge);
  types.insert("del_node",graph_update::del_node);
  types.insert("del_edge",graph_update::del_edge);
  types.insert("move_edge",graph_update::move_edge);
  types.insert("clear_graph",graph_update::clear_graph);
  types.insert("touch_node",graph_update::touch_node);
  types.insert("touch_edge",graph_update::touch_edge);
  types.insert("global_query",graph_update::global_query);
  types.insert("nodes_query",graph_update::nodes_query);
  types.insert("edge_query",graph_update::edge_query);
  types.insert("comment",graph_update::comment);

  string token;
  in >> token;              // read key word for type of operation

  if(token == string("]"))  // no more operations
  {
    gu.ierr = true;
    return in;
  }

  gu.type = types.access(token);     // retrieve type of operation
  switch(gu.type)                    // get additional parameters
  {
    case graph_update::new_node:
    case graph_update::del_node:
    {
      in >> token;  // "["
      in >> token;  // "id"
      in >> token;  // the index
      gu.v = atoi(token.cstring());
      in >> token;  // "]"
      break;
    }
    case graph_update::new_edge:
    case graph_update::del_edge:
    case graph_update::edge_query:
    case graph_update::nodes_query:
    {
      in >> token;  // "["
      in >> token;  // "source" or "node1"
      in >> token;  // the source index
      gu.v = atoi(token.cstring());
      in >> token;  // "target" or "node2"
      in >> token;  // the target index
      gu.w = atoi(token.cstring());
      in >> token;  // "]"
      break;
    }
    case graph_update::move_edge:
    {
      in >> token;  // "["
      in >> token;  // "source"
      in >> token;  // the source index
      gu.v = atoi(token.cstring());
      in >> token;  // "target"
      in >> token;  // the target index
      gu.w = atoi(token.cstring());
      in >> token;  // "new_source"
      in >> token;  // the new source index
      gu.x = atoi(token.cstring());
      in >> token;  // "new_target"
      in >> token;  // the new target index
      gu.y = atoi(token.cstring());
      in >> token;  // "]"
      break;
    }
    case graph_update::clear_graph:
    case graph_update::global_query:
    {
      in >> token;  // "1"
      break;
    }
    case graph_update::touch_node:
    {
      in >> token;  // "["
      in >> token;  // "id"
      in >> token;  // the index
      gu.v = atoi(token.cstring());
      in >> token;  // "value"
      while('"' != in.get());  // proceed past first "
      gu.s.read(in,'"');       // get the string
      in.get();                // skip terminating "
      in >> token;  // "]"
      break;
    }
    case graph_update::touch_edge:
    {
      in >> token;  // "["
      in >> token;  // "source"
      in >> token;  // the source index
      gu.v = atoi(token.cstring());
      in >> token;  // "target"
      in >> token;  // the target index
      gu.w = atoi(token.cstring());
      in >> token;  // "value"
      while('"' != in.get());  // proceed past first "
      gu.s.read(in,'"');       // get the string
      in.get();                // skip terminating "
      in >> token;  // "]"
      break;
    }
    case graph_update::comment:
    {
      in >> token;  // the string with leading and ending "
      gu.s = token(1,token.length()-2);  // strip "
      break;
    }
  }
  gu.ierr = false;
  return in;
}


// ------------------------------------------------------------ //
// class int_pair                                               //
// ------------------------------------------------------------ //

ostream& operator << (ostream& out, const int_pair& ip)
{
  out << "(" << ip.s << "," << ip.t << ")";
  return out;
}

istream& operator >> (istream& in, int_pair& ip)
{
  char c;
  in >> c;
  while(isspace(c)) in >> c;     // skip white space
  // now c == '('
  in >> ip.s >> c >> ip.t >> c;  // s,t)
  return in;
}



// ------------------------------------------------------------ //
// class graph_recorder                                         //
// ------------------------------------------------------------ //

graph_recorder::graph_recorder(msg_graph& H)
: get_graph_msg(&H)
{
  G = &H;
  P = G;
  del_nd = 0;
  del_ind = -1;
  del_ae = 0;
  dirty = false;
  state = waiting;
  pos = 0;
  next_ind = 0;
}

graph_recorder::~graph_recorder()
{
  scratch();
}

void graph_recorder::show(ostream& out)
{
  out << "initial nodes:\n " << initial_nodes << "\n";
  out << "initial edges:\n " << initial_edges << "\n";
  out << "updates:\n " << updates << "dirty: " << (dirty ? 1 : 0) << "\n\n";
}
  
void graph_recorder::scratch()
{
  // stop possible recording
  state = waiting;

  // delete old recording
  initial_nodes.clear();
  initial_edges.clear();
  updates.clear();

  // clear dirty bit
  dirty = false;
}

void graph_recorder::init(msg_graph& H)
{
  scratch();

  ggm_init(&H);
}

void graph_recorder::record()
{
  // delete possible old recording
  scratch();

  // store initial graph
  node v;
  next_ind = 0;
  forall_nodes(v,*G)
  {
    int i = index(v);
    nodes[i] = v;
    node_ind[v] = i;
    if(i >= next_ind) next_ind = i + 1;
    initial_nodes.append(i);
  }
  edge e;
  forall_edges(e,*G)
  {
    int s = index(source(e));
    int t = index(target(e));
    int_pair st(s,t);
    edge_pos[e] = edges(s,t).push(e);
    edge_ind[e] = st;
    initial_edges.append(st);
  }

  // set dirty bit
  dirty = false;

  // start recording
  state = recording;
}

void graph_recorder::rewind()
// reconstruct initial graph
{
  stop();

  if(dirty)
  {
    int ind;
    P->clear();
    next_ind = 0;
    forall(ind,initial_nodes)
    {
      node v = P->new_node();
      nodes[ind] = v;
      node_ind[v] = ind;
      if(ind >= next_ind) next_ind = ind + 1;
    }
    int_pair ip;
    forall(ip,initial_edges)
    {
      edge e = P->new_edge(nodes[ip.s],nodes[ip.t]);
      edge_pos[e] = edges(ip.s,ip.t).push(e);
      edge_ind[e] = ip;
    }
  }

  dirty = false;
  pos = 0;
}

void graph_recorder::play(graph& H)
{
// LEDA-3.7.1
//  P = &H;
// LEDA-4.2
  P = (msg_graph *) &H;
  dirty = true;
  play();
  P = G;
}

void graph_recorder::play()
{
  if(state != single_stepping)
  {
    // rebuild initial graph
    rewind();

    state = playing;

    // do the updates
    graph_update gu;
    forall(gu,updates) play_update(gu);
  }
  else  // single_stepping ...
  {
    state = playing;

    if(!pos) rewind();  // otherwise, some steps have been made

    // do remaining updates
    for(pos = updates.succ(pos); pos; pos = updates.succ(pos))
      play_update(updates.contents(pos));
  }

  state = waiting;
}

void graph_recorder::play_update(const graph_update& gu)
{
  node v,w;
  edge e;

  switch(gu.type)
  {
    case graph_update::new_node:
    {
      v = P->new_node();
      nodes[gu.v] = v;
      node_ind[v] = gu.v;
      break;
    }
    case graph_update::new_edge:
    {
      e = P->new_edge(nodes[gu.v],nodes[gu.w]);
      edge_pos[e] = edges(gu.v,gu.w).push(e);
      edge_ind[e] = int_pair(gu.v,gu.w);
      break;
    }
    case graph_update::del_node:
    {
      v = nodes[gu.v];
      forall_inout_edges(e,v)
      {
        int s = edge_ind[e].s;
        int t = edge_ind[e].t;
        edges(s,t).del_item(edge_pos[e]);
        edge_pos[e] = nil;
        edge_ind[e] = int_pair(-1,-1);
      }
      nodes[gu.v] = nil;
      node_ind[v] = -1;
      P->del_node(v);
      break;
    }
    case graph_update::del_edge:
    {
      e = edges(gu.v,gu.w).pop();   // get (last parallel) edge
      edge_pos[e] = nil;
      edge_ind[e] = int_pair(-1,-1);
      P->del_edge(e);
      break;
    }
    case graph_update::move_edge:
    {
      e = edges(gu.v,gu.w).pop();   // get (last parallel) edge
      edge_pos[e] = edges(gu.x,gu.y).push(e);
      edge_ind[e] = int_pair(gu.x,gu.y);
      v = nodes[gu.x];
      w = nodes[gu.y];
      P->move_edge(e,v,w);
      break;
    }

    case graph_update::clear_graph:
    {
      nodes.clear();
      node_ind.clear();
      edges.clear();
      edge_ind.clear();
      edge_pos.clear();
      P->clear();
      break;
    }
    case graph_update::touch_node:
    {
      P->touch(nodes[gu.v],gu.s);
      break;
    }
    case graph_update::touch_edge:
    {
      P->touch(edges(gu.v,gu.w).head(),gu.s);
      break;
    }
    case graph_update::global_query:
    {
      P->query();
      break;
    }
    case graph_update::nodes_query:
    {
      v = nodes[gu.v];
      w = nodes[gu.w];
      P->query(v,w);
      break;
    }
    case graph_update::edge_query:
    {
      P->query(edges(gu.v,gu.w).head());
      break;
    }
    case graph_update::comment:
    {
      P->comment(gu.s);
      break;
    }
  }
}

void graph_recorder::step()
{
  if(state == recording) return;

  if(state != single_stepping) state = single_stepping;

  if(!pos)                        // first step
  {
    if(!updates.empty())
    {
      rewind();
      pos = updates.first();
    }
    else return;                  // no recorded sequence
  }
  else pos = updates.succ(pos);   // not the first step, get next update

  if(pos)                         // there is still one step
  {
    graph_update curr = updates.contents(pos);
    play_update(curr);
    if(curr.is_update()) dirty = true;
  }
  else    state = waiting;        // end of recording reached
}

void graph_recorder::step_back()
{
  if(state == recording) return;

  if(state != single_stepping) state = single_stepping;

  if(!pos)
  {
    // (re)start at the end of the recorded sequence
    pos = updates.last();
  }

  if(pos)
  {
    undo_update(false);
    dirty = true;
  }
  // reverse current update without changing the recorded sequence,
  // this updates pos, too
}

void graph_recorder::search_index(string name)
{
  string ind = string("index(%s)",name);

  do { step(); } while(pos && (updates.contents(pos).s != ind));

  state = waiting;
}


void graph_recorder::save(string s)
{
  file_ostream out(s);
  if(out.fail()) return;

  out << "Creator " << '"' << "graph_recorder::save" << '"' << "\n\n";


  out << "graph [\n";

  int i;
  forall(i,initial_nodes) out << "  node [ id " << i << " ]\n";
  out << "\n";

  int_pair ip;
  forall(ip,initial_edges)
    out << "  edge [ source " << ip.s << " target " << ip.t << " ]\n";

  out << "]\n\n";


  out << "updates [\n";

  graph_update gu;
  forall(gu,updates)
  {
    switch(gu.type)
    {
      // updates
      case graph_update::new_node:
      {
        out << "  new_node [ id " << gu.v << " ]\n";
        break;
      }
      case graph_update::new_edge:
      {
        out << "  new_edge [ source " << gu.v << " target " << gu.w << " ]\n";
        break;
      }
      case graph_update::del_node:
      {
        out << "  del_node [ id " << gu.v << " ]\n";
        break;
      }
      case graph_update::del_edge:
      {
        out << "  del_edge [ source " << gu.v << " target " << gu.w << " ]\n";
        break;
      }
      case graph_update::move_edge:
      {
        out << "  move_edge [ source " << gu.v << " target " << gu.w << "\n";
        out << "              new_source " << gu.x << " new_target " << gu.y;
        out << "\n            ]\n";
        break;
      }
      case graph_update::clear_graph:
      {
        out << "  clear_graph 1\n";
        break;
      }

      // const methods
      case graph_update::touch_node:
      {
        out << "  touch_node [ id " << gu.v << " value " << '"';
        out << gu.s << '"' << " ]\n";
        break;
      }
      case graph_update::touch_edge:
      {
        out << "  touch_edge [ source " << gu.v << " target " << gu.w;
        out << " value " << '"' << gu.s << '"' << " ]\n";
        break;
      }
      case graph_update::global_query:
      {
        out << "  global_query 1\n";
        break;
      }
      case graph_update::nodes_query:
      {
        out << "  nodes_query [ node1 " << gu.v << " node2 " << gu.w;
        out << " ]\n";
        break;
      }
      case graph_update::edge_query:
      {
        out << "  edge_query [ source " << gu.v << " target " << gu.w;
        out << " ]\n";
        break;
      }
      case graph_update::comment:
      {
        out << "  comment " << '"' << gu.s << '"' << "\n";
        break;
      }
      default: break;
    }
  }
  out << "]\n";
}

void graph_recorder::load(string s, bool rew)
{
  scratch();

  file_istream in(s);
  if(in.fail()) return;

  string token;
  
  // skip preamble
  in >> token;      // "Creator"
  in >> token;      // `"graph_recorder::save"'
  in >> token;      // "graph"
  in >> token;      // "["

  in >> token;
  while(token != string("]"))
  {
    if(token == string("node"))
    {
      in >> token;  // "["
      in >> token;  // "id"
      in >> token;  // the node id
      initial_nodes.append(atoi(token.cstring()));
      in >> token;  // "]"
    }
    if(token == string("edge"))
    {
      int_pair ip;
      in >> token;  // "["
      in >> token;  // "source"
      in >> token;  // the source id
      ip.s = atoi(token.cstring());
      in >> token;  // "target"
      in >> token;  // the target id
      ip.t = atoi(token.cstring());
      initial_edges.append(ip);
      in >> token;  // "]"
    }
    in >> token;
  }

  in >> token;      // "updates"
  in >> token;      // "["

  graph_update gu;
  while(!gu.fail())
  {
    in >> gu;
    if(!gu.fail()) updates.append(gu);
  }

  dirty = true;

  if(rew) rewind();
}


void graph_recorder::undo()
{
  if(state!=recording) return;
  if(updates.empty()) return;

  state = waiting;

  undo_update();

  state = recording;
}
      
void graph_recorder::undo_update(bool change_updates)
{
  list_item old_pos = pos;

  graph_update gu;
  if(change_updates) gu = updates.Pop();
  else
  {
    gu = updates.contents(pos);
    pos = updates.pred(pos);
  }

  node v,w;
  edge e;

  switch(gu.type)
  {
    case graph_update::new_node:
    {
      v = nodes[gu.v];
      node_ind[v] = -1;
      nodes[gu.v] = nil;
      P->del_node(v);
      break;
    }
    case graph_update::new_edge:
    {
      e = edges(gu.v,gu.w).pop();
      edge_pos[e] = nil;
      edge_ind[e] = int_pair(-1,-1);
      P->del_edge(e);
      break;
    }
    case graph_update::del_node:
    {
      v = P->new_node();
      nodes[gu.v] = v;
      node_ind[v] = gu.v;
      for(int i=0; i<atoi(gu.s.cstring()); i++) undo_update(change_updates);
      // reinsert adjacent edges
      break;
    }
    case graph_update::del_edge:
    {
      e = P->new_edge(nodes[gu.v],nodes[gu.w]);
      edge_pos[e] = edges(gu.v,gu.w).push(e);
      edge_ind[e] = int_pair(gu.v,gu.w);
      break;
    }
    case graph_update::move_edge:
    {
      e = edges(gu.x,gu.y).pop();
      v = nodes[gu.v];
      w = nodes[gu.w];
      P->move_edge(e,v,w);
      edge_pos[e] = edges(gu.v,gu.w).push(e);
      edge_ind[e] = int_pair(gu.v,gu.w);
      break;
    }
    case graph_update::clear_graph:
    {
      // not supported
      break;
    }
    case graph_update::touch_node:
    {
      v = nodes[gu.v];
      P->touch(v,"undo()");
      break;
    }
    case graph_update::touch_edge:
    {
      e = edges(gu.v,gu.w).head();
      P->touch(e,"undo()");
      break;
    }      
    case graph_update::global_query:
    case graph_update::nodes_query:
    case graph_update::edge_query:
    {
      // do nothing
      break;
    }
    case graph_update::comment:
    {
      if(!change_updates && (gu.s == string("index(start)")) )
      {
        pos = old_pos;
        break;
      }
      P->comment("");
      break;
    }
  }
}

void graph_recorder::index_correction(const graph_msg& gm, graph_update& gu)
{
  switch(gm.type)
  {
    case graph_msg::post_new_node:
    {
      gu.v = next_ind++;
      break;
    }
    case graph_msg::pre_new_edge:
    case graph_msg::nodes_query:
    {
      gu.v = node_ind[gm.v];
      gu.w = node_ind[gm.w];
      break;
    }
    case graph_msg::pre_del_node:
    case graph_msg::touch_node:
    {
      gu.v = node_ind[gm.v];
      break;
    }
    case graph_msg::post_new_edge:
    case graph_msg::pre_del_edge:
    case graph_msg::post_del_edge:
    case graph_msg::touch_edge:
    case graph_msg::edge_query:
    {
      gu.v = node_ind[source(gm.e)];
      gu.w = node_ind[target(gm.e)];
      break;
    }
    case graph_msg::pre_move_edge:
    case graph_msg::post_move_edge:
    {
      gu.v = node_ind[source(gm.e)];
      gu.w = node_ind[target(gm.e)];
      gu.x = node_ind[gm.v];
      gu.y = node_ind[gm.w];
      break;
    }

    default: break;
  }
}

void graph_recorder::get_msg(const graph_msg& gm)
{
  if(state != recording)
  {
    if(gm.type & graph_msg::updates)
    {
      // graph is changing...
      dirty = true;
    }
    return;
  }

  graph_update gu(gm);
  index_correction(gm,gu);

  node v,w;
  edge e;
  int  val;
 
  switch(gm.type)
  {
    case graph_msg::post_new_node:
    {
      v = gm.v;
      nodes[gu.v] = v;
      node_ind[v] = gu.v;
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::post_new_edge:
    {
      e = gm.e;
      edge_pos[e] = edges(gu.v,gu.w).push(e);
      edge_ind[e] = int_pair(gu.v,gu.w);
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::pre_del_node:
    {
      v = gm.v;
      del_nd = v;
      del_ind = node_ind[v];
      del_ae = G->degree(v);
      break;
    }
    case graph_msg::post_del_node:
    {
      gu.type = graph_update::del_node;
      gu.v = del_ind;
      gu.s = string("%d",del_ae);
      nodes[gu.v] = nil;
      node_ind[del_nd] = -1;
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::pre_del_edge:
    {
      e = gm.e;
      edges(gu.v,gu.w).del_item(edge_pos[e]);
      edge_ind[e] = int_pair(-1,-1);
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::pre_move_edge:
    {
      e = gm.e;
      edges(gu.v,gu.w).del_item(edge_pos[e]);
      edge_pos[e] = edges(gu.x,gu.y).push(e);
      edge_ind[e] = int_pair(gu.x,gu.y);
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::pre_clear_graph:
    {
      node v;
      nodes.clear();
      node_ind.clear();
      edges.clear();
      edge_pos.clear();
      edge_ind.clear();
      updates.append(gu);
      dirty = true;
      break;
    }
    case graph_msg::touch_node:
    case graph_msg::touch_edge:
    case graph_msg::global_query:
    case graph_msg::nodes_query:
    case graph_msg::edge_query:
    case graph_msg::comment:
    {
      updates.append(gu);
      break;
    }
    default: break;
  }
  pos = updates.last();
}
