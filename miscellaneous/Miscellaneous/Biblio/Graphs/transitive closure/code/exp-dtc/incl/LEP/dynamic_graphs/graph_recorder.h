#ifndef LEDA_GRAPH_RECORDER_H
#define LEDA_GRAPH_RECORDER_H

// ------------------------------------------------------------------- //
// class graph_recorder                                                //
//                                                                     //
// record and replay updates of a msg_graph                            //
// new version supports parallel edges, too                            //
//                                                                     //
// David Alberts (1997/98)                                             //
//                                                                     //
// Modified by G. Prasinos & C. Zaroliagis (2002) to run with LEDA-4.2 //
// ------------------------------------------------------------------- //

#include<LEP/dynamic_graphs/msg_graph.h>
#include<LEDA/map2.h>
#include<LEDA/map.h>

/*{\Manpage {graph_recorder} {}
   {Recording and Playing Sequences of Operations on Graphs} }*/

class  graph_recorder;

class graph_update
{
    enum update_type
    {
      // updates
      new_node=0, new_edge=1,
      del_node=2, del_edge=3,
      move_edge=4, clear_graph=5,

      // const methods
      touch_node=6, touch_edge=7,
      comment=8, global_query=9,
      nodes_query=10, edge_query=11
    };

    update_type type;  // type of the update
    int v;             // node indices
    int w;
    int x;
    int y;
    string s;          // a string given to touch or comment

    bool ierr;         // last input was unsuccessful

  public:

    graph_update() { ierr = false; }

    graph_update(const graph_update& gu)
    : type(gu.type), v(gu.v), w(gu.w), x(gu.x), y(gu.y),
      s(gu.s), ierr(gu.ierr) {}

    graph_update(const graph_msg&);

    graph_update& operator=(const graph_update& gu)
    { type=gu.type; v=gu.v; w=gu.w; x=gu.x; y=gu.y; s=gu.s;
      ierr=gu.ierr; return *this; }

    bool fail() { return ierr; }
    bool is_update() { return (int(type) < 6); }
    bool is_query()  { return (int(type) > 8); }
    bool is_touch()  { return !(is_query() || is_update()); }

    friend class graph_recorder;
    friend ostream& operator << (ostream&,const graph_update&);
    friend istream& operator >> (istream&,graph_update&);

    LEDA_MEMORY(graph_update)
};


class int_pair
{
    int s;
    int t;

  public:

    int_pair() { s=t=-1; }
    int_pair(int ess, int teh) { s=ess; t=teh; }
    int_pair(const int_pair& ip) : s(ip.s), t(ip.t) {}

    int_pair& operator=(const int_pair& ip) { s=ip.s; t=ip.t; return *this; }

    friend ostream& operator << (ostream&,const int_pair&);
    friend istream& operator >> (istream&,int_pair&);

    friend class graph_recorder;

    LEDA_MEMORY(int_pair)
};

class __exportC graph_recorder : public get_graph_msg
{
  public:

/*{\Mdefinition

An instance $rec$ of the data type $graph\_recorder$ receives
update messages from an instance $G$ of the data type $msg\_graph$.
These updates can be recorded, saved to and loaded from disk, and
played either on $G$ or on an arbitrary instance of type $graph$.

\textbf{Important:} |graph_recorder| does not work for graphs with
parallel edges.
}*/

/*{\Mcreation rec }*/

    graph_recorder(msg_graph& G);
/*{\Mcreate creates an object |\Mvar| of type |\Mname| and
            initializes it for receiving updates from $G$. Initially,
            no updates are recorded. }*/

    virtual ~graph_recorder();

/*{\Moperations 1 4.8 }*/

    void record();
/*{\Mop starts recording updates. }*/

    void stop() { state = waiting; }
/*{\Mop stops recording updates. }*/

    void play();
/*{\Mop replays the recorded update sequence on the graph given
        in the constructor or in the last $init$ call. }*/

    void play(graph& H);
/*{\Mop replays the recorded update sequence on the graph $H$. }*/

    void step();
/*{\Mop replays the next (or first) step in the recorded update sequence. }*/

    void step_back();
/*{\Mop reverses the last step, if possible ($G.clear$ cannot be
        reversed). }*/

    void search_index(string name);
/*{\Mop plays the recorded sequence, until the comment ``index($<name>$)''
        (where $<name>$ denotes the contents of the $string$ $name$)
        is found. }*/

    void save(string filename);
/*{\Mop saves the current recording to $filename$. }*/

    void load(string filename, bool rewind=true);
/*{\Mop loads a recording from the file $filename$ and $rewinds$, if
        $rewind$ is $true$. }*/

    void undo();
/*{\Mop undoes the last recorded operation (except $G.touch$ and $G.clear$),
        if updates are currently recorded. }*/

    void rewind();
/*{\Mop $stops$ and reconstructs the graph at the beginning of the last
        recording, if it exists. }*/

    void scratch();
/*{\Mop $stops$ and deletes current recording. }*/

    void show(ostream& out = cout);
/*{\Mop prints current recording to $out$. }*/

    void init(msg_graph& H);
/*{\Mop reinitializes |\Mvar| for receiving updates from $H$. }*/

    bool is_recording() { return (state == recording); }
/*{\Mop returns $true$ iff the recorder is currently recording. }*/

    bool is_waiting()   { return (state == waiting); }
/*{\Mop returns $true$ iff the recorder is currently waiting. }*/

    bool is_single_stepping()   { return (state == single_stepping); }
/*{\Mop returns $true$ iff the recorder is currently single stepping. }*/

    int number_of_operations() { return updates.size(); }
/*{\Mop returns the number of recorded operations. }*/



  private:

    void get_msg(const graph_msg&);

    enum states { waiting, recording, playing, single_stepping };

    states state;

    msg_graph* G;       // pointer to graph given in the constructor
// LEDA-3.7.1
// graph *P;            // where to play the recording
// LEDA-4.2 
    msg_graph* P;       // where to play the recording

    list<int>      initial_nodes; // nodes in graph at beginning or recording
    list<int_pair> initial_edges; // edges ---"---

    list<graph_update> updates;   // sequence of updates
    list_item          pos;       // current position during single stepping

    bool dirty;

    map<int,node>              nodes;
    map<node,int>              node_ind;
    map2<int,int, list<edge> > edges;
    map<edge,list_item>        edge_pos;  // pos of edge in edges list
    map<edge,int_pair>         edge_ind;
    // keep track of nodes and edges by means of indices
    // this is needed for replaying

    void play_update(const graph_update& gu);
    void undo_update(bool change_updates=true);

    void index_correction(const graph_msg& gm, graph_update& gu);

    int next_ind;

    // stuff for handling deletion of nodes with undo
    node del_nd;    // node to be deleted
    int  del_ind;   // its index
    int  del_ae;    // its #adj. edges


/*{\Mimplementation

$graph\_recorders$ are implemented using $get\_graph\_msg$, $maps$,
and $lists$.

}*/

};

#endif
