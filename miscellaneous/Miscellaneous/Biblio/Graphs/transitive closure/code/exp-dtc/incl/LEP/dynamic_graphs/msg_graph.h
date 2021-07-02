#ifndef LEDA_MSG_GRAPH_H
#define LEDA_MSG_GRAPH_H


//-------------------------------------------------------------------- //
// msg_graph, derived from graph, adds some extra features             //
//                                                                     //
// David Alberts (1997)                                                //
//                                                                     //
// Modified by G. Prasinos & C. Zaroliagis (2002) to run with LEDA-4.2 //
//-------------------------------------------------------------------- //

#include<LEDA/graph.h>

struct graph_msg
{
    enum msg_type
    {
      // updates
      pre_new_node=1, post_new_node=2,
      pre_new_edge=4, post_new_edge=8,
      pre_del_node=16, post_del_node=32,
      pre_del_edge=64, post_del_edge=128,
      pre_move_edge=(1<<8), post_move_edge=(1<<9),
      pre_clear_graph=(1<<10), post_clear_graph=(1<<11),

      // queries and the like (const methods)
      touch_node=(1<<12), touch_edge=(1<<13),
      global_query=(1<<14), nodes_query=(1<<15),
      edge_query=(1<<16), comment=(1<<17),

      // groups
      all=0xFFFFFFFF, updates=(1<<12)-1,
      queries=(1<<18)-(1<<12), none=0
    };

    msg_type type;      // type of the update
    node v;             // involved nodes or edge
    node w;
    edge e;
    string s;           // a string given in comment or touch

    graph_msg() { v=w=0; e=0; type=none; s=""; }

    graph_msg(const graph_msg& gu)
    : type(gu.type), v(gu.v), w(gu.w), s(gu.s),
      e(gu.e) {}

    graph_msg& operator=(const graph_msg& gu)
    { type=gu.type; v=gu.v; w=gu.w; s=gu.s;
      e=gu.e; return *this; }

    friend ostream& operator << (ostream&,const graph_msg&);

    LEDA_MEMORY(graph_msg)
};

class msg_graph;

class get_graph_msg
{
  protected:

    msg_graph* ggm_G;
    graph_msg::msg_type msg_types; // type of messages I want to hear
    list_item where;               // ... am I in the recipients list

    bool query_result;             // result of last query

  public:

    get_graph_msg(msg_graph* H, graph_msg::msg_type mask=graph_msg::all,
                  list_item in_front_of=0);
    get_graph_msg(msg_graph& H, graph_msg::msg_type mask=graph_msg::all,
                  list_item in_front_of=0);
    get_graph_msg() { ggm_G = 0; }
    virtual ~get_graph_msg();

    virtual void ggm_init(msg_graph* H,
                      graph_msg::msg_type mask=graph_msg::all);
    virtual void ggm_init(msg_graph& H,
                      graph_msg::msg_type mask=graph_msg::all);

    virtual graph_msg::msg_type disable()
    { graph_msg::msg_type current=msg_types;
      msg_types=graph_msg::none; return current; }

    virtual graph_msg::msg_type accept(graph_msg::msg_type mask)
    { graph_msg::msg_type current=msg_types; msg_types=mask; return current; }

    virtual graph_msg::msg_type accepted_types() { return msg_types; }

    virtual void get_msg(const graph_msg&) = 0; 

  friend class msg_graph;
};


/*{\Manpage {msg_graph} {} {Graphs sending Update Messages} }*/

class /* __exportC */ msg_graph : public graph {

/*{\Mdefinition
An instance $G$ of the data type $msg\_graph$ is a $graph$ with
the additional feature of sending messages about updates to
interested third parties. An example of a class using such
messages is the class $graph\_recorder$ which records, saves,
loads, and plays sequences of updates performed on a message
graph.

Since the class $msg\_graph$ is derived from the class $graph$,
all operations on $graphs$ can be performed on $msg\_graphs$
as well.
}*/

  list<get_graph_msg*> recipients;   // list of who wants to get messages

public:

  msg_graph()               : graph()  { the_msg = new graph_msg; }
  msg_graph(const graph& G) : graph(G) { the_msg = new graph_msg; }
  ~msg_graph()                         { delete the_msg; }

  // we redefine these methods to send messages
  void touch(node,string) const;
  void touch(edge,string) const;

  void comment(string) const;
  bool query() const;
  bool query(node,node) const;
  bool query(edge) const;

protected:

  void send() const
  {
    get_graph_msg* rec;
    forall(rec,recipients)
      if(rec->accepted_types() & the_msg->type) rec->get_msg(*the_msg);
  }

  void pre_new_node_handler();
  void post_new_node_handler(node v);
  void pre_del_node_handler(node v);
  void post_del_node_handler();
  void pre_new_edge_handler(node v, node w);
  void post_new_edge_handler(edge e);
  void pre_del_edge_handler(edge e);
  void post_del_edge_handler(node v, node w);
  void pre_move_edge_handler(edge e, node v, node w);
  void post_move_edge_handler(edge e, node v, node w);
  void pre_clear_handler();
  void post_clear_handler();

private:

  graph_msg *the_msg;

  friend class get_graph_msg;
};


// ------------------------------------------------------------------ //
// inline methods                                                     //
// ------------------------------------------------------------------ //


// ------------------------------------------------------------------ //
// class get_graph_msg                                                //
// ------------------------------------------------------------------ //

inline get_graph_msg::get_graph_msg(msg_graph* H,
                                    graph_msg::msg_type mask,
                                    list_item in_front_of)
{
  ggm_G=H;
  msg_types=mask;
  query_result = false;
// LEDA-3.7.1
// if(in_front_of) where = ggm_G->recipients.insert(this,in_front_of,before);
// LEDA-4.2
  if(in_front_of) where = ggm_G->recipients.insert(this,in_front_of,LEDA::before);
  else            where = ggm_G->recipients.append(this);
}

inline get_graph_msg::get_graph_msg(msg_graph& H,
                                    graph_msg::msg_type mask,
                                    list_item in_front_of)
{
  ggm_G=&H;
  msg_types=mask;
  query_result = false;
// LEDA-3.7.1
// if(in_front_of) where = ggm_G->recipients.insert(this,in_front_of,before);
// LEDA-4.2
  if(in_front_of) where = ggm_G->recipients.insert(this,in_front_of,LEDA::before);
  else            where = ggm_G->recipients.append(this);
}

inline get_graph_msg::~get_graph_msg() { ggm_G->recipients.del_item(where); }

inline void get_graph_msg::ggm_init(msg_graph* H,
                                    graph_msg::msg_type mask)
{
  if(ggm_G) ggm_G->recipients.del_item(where);
  ggm_G=H;
  msg_types=mask;
  query_result = false;
  where = ggm_G->recipients.append(this);
}

inline void get_graph_msg::ggm_init(msg_graph& H,
                                    graph_msg::msg_type mask)
{
  if(ggm_G) ggm_G->recipients.del_item(where);
  ggm_G=&H;
  msg_types=mask;
  query_result = false;
  where = ggm_G->recipients.append(this);
}



// ------------------------------------------------------------------ //
// class msg_graph                                                    //
// ------------------------------------------------------------------ //

// --- public methods --- //

inline void msg_graph::touch(node v, string s) const
{
  the_msg->type = graph_msg::touch_node;
  the_msg->v = v;
  the_msg->s = s;
  send();
}

inline void msg_graph::touch(edge e, string s) const
{
  the_msg->type = graph_msg::touch_edge;
  the_msg->e = e;
  the_msg->s = s;
  send();
}

inline bool msg_graph::query() const
{
  the_msg->type = graph_msg::global_query;
  send();
  return (recipients.tail())->query_result;
}

inline bool msg_graph::query(node v, node w) const
{
  the_msg->type = graph_msg::nodes_query;
  the_msg->v = v;
  the_msg->w = w;
  send();
  return (recipients.tail())->query_result;
}

inline bool msg_graph::query(edge e) const
{
  the_msg->type = graph_msg::edge_query;
  the_msg->e = e;
  send();
  return (recipients.tail())->query_result;
}

inline void msg_graph::comment(string s) const
{
  the_msg->type = graph_msg::comment;
  the_msg->s = s;
  send();
}



// --- protected methods --- //

inline void msg_graph::pre_new_node_handler()
{
  the_msg->type = graph_msg::pre_new_node;
  send();
}

inline void msg_graph::post_new_node_handler(node v)
{
  the_msg->type = graph_msg::post_new_node;
  the_msg->v = v;
  send();
}

inline void msg_graph::post_del_node_handler()
{
  the_msg->type = graph_msg::post_del_node;
  send();
}

inline void msg_graph::pre_new_edge_handler(node v, node w)
{
  the_msg->type = graph_msg::pre_new_edge;
  the_msg->v = v;
  the_msg->w = w;
  send();
}

inline void msg_graph::post_new_edge_handler(edge e)
{
  the_msg->type = graph_msg::post_new_edge;
  the_msg->e = e;
  send();
}

inline void msg_graph::pre_del_edge_handler(edge e)
{
  the_msg->type = graph_msg::pre_del_edge;
  the_msg->e = e;
  send();
}

inline void msg_graph::post_del_edge_handler(node v, node w)
{
  the_msg->type = graph_msg::post_del_edge;
  the_msg->v = v;
  the_msg->w = w;
  send();
}

inline void msg_graph::pre_move_edge_handler(edge e, node v, node w)
{
  the_msg->type = graph_msg::pre_move_edge;
  the_msg->e = e;
  the_msg->v = v;
  the_msg->w = w;
  send();
}

inline void msg_graph::post_move_edge_handler(edge e, node v, node w)
{
  the_msg->type = graph_msg::post_move_edge;
  the_msg->e = e;
  the_msg->v = v;
  the_msg->w = w;
  send();
}

inline void msg_graph::pre_clear_handler()
{
  the_msg->type = graph_msg::pre_clear_graph;
  send();
}

inline void msg_graph::post_clear_handler()
{
  the_msg->type = graph_msg::post_clear_graph;
  send();
}



#endif

