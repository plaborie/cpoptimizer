// --------------------------------------------------------------------
//
//  File:        dtc_italiano_anim.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Animation of Italiano's dynamic transitive closure algorithm.
//               The class dtc_italiano_anim is derived from the class
//               dtc_italiano. It offers some additional functionality
//               for animation purposes.
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_ITALIANO_ANIM_H
#define _DTC_ITALIANO_ANIM_H

#include <LEP/dynamic_graphs/dtc_italiano.h>

class dtc_italiano_anim : public dtc_italiano
  {
    public:
      dtc_italiano_anim(msg_graph& G, GraphWin& w) : dtc_italiano(G), GW(&w), Animation(0) {}
      void insert_edge(edge, double);
      void anim_insert_edge(edge);
      void anim_remove_edge(edge);
      void set_animation(int i) { Animation = i; }

    private:
      bool text_panel(string, string);
      window* text_window(string, string);
      bool message(window*, string);
      void color_node(node n, color c) { GW->set_color(n, c); }
      void color_edge(edge e, color c) { GW->set_color(e, c); GW->set_width(e, 3); GW->set_style(e, solid_edge); }
      void reset_node(node n) { GW->set_color(n, yellow); }
      void reset_edge(edge e) { GW->set_color(e, black); GW->set_width(e, 1); GW->set_style(e, solid_edge); }
      void color_path(node, node, color, color, color);
      void color_desc(node, color, color);

      GraphWin*	GW;
      int	Animation;
  };

/******************** USEFUL FUNCTIONS ********************/

bool dtc_italiano_anim::text_panel(string s1, string s2)
  {
    panel P;
    P.text_item(string("\\bf\\blue ")+s1+" \\rm\\black\\c4 "+s2);
    P.button("Continue", 1);
    P.button("Quit", 0);
    if (P.open(GW->get_window(), window::center, 0)) return true;
    update(the_graph);
    GW->reset();
    GW->zoom(1.25);
    return false;
  }

window* dtc_italiano_anim::text_window(string s1, string s2)
  {
    window* W = new window(500, 150);
    W->text_item(string("\\bf\\blue ")+s1+" \\rm\\black\\c4 "+s2+" \\c2");
    W->button("Continue", 1);
    W->button("Quit", 0);
    W->display(GW->get_window(), window::center, 0);
    return W;
  }

bool dtc_italiano_anim::message(window* W, string s)
  {
    int i;
    W->del_messages();
    while ((i = s.pos("|")) > 0)
      {
        W->message(s.head(i-1));
        s = s.del(0, i);
      }
    W->message(s);
    if (W->read())
        return true;
    W->close();
    update(the_graph);
    GW->reset();
    GW->zoom(1.25);
    return false;
  }

void dtc_italiano_anim::color_path(node n, node m, color color_path, color color_n, color color_m)
  {
    GW->set_flush(false);
    GW->reset();
    node v = Index(n,m);
    edge e;
    while (v != Desc[n].first_node())
      {
        v = source(e = Desc[n].first_in_edge(v));
        color_edge(Desc[n].inf(e), color_path);
        color_node(Desc[n].inf(v), color_path);
      }
    color_node(n, color_n);
    color_node(m, color_m);
    GW->set_shape(n, square_node);
    GW->set_shape(m, square_node);
    GW->set_flush(true);
    GW->redraw();
  }

void dtc_italiano_anim::color_desc(node n, color color_desc, color color_n)
  {
    GW->set_flush(false);
    GW->reset();
    node v;
    edge e;
    forall_nodes(v, Desc[n]) color_node(Desc[n].inf(v), color_desc);
    forall_edges(e, Desc[n]) color_edge(Desc[n].inf(e), color_desc);
    color_node(n, color_n);
    GW->set_shape(n, square_node);
    GW->set_flush(true);
    GW->redraw();
  }

/******************** INSERT EDGE ********************/

void dtc_italiano_anim::insert_edge(edge e, double weight = 1.0)
  {
    if (! Animation)
        dtc_italiano::insert_edge(e, weight);
  }
    
void dtc_italiano_anim::anim_insert_edge(edge e)
  {
    GW->zoom(0.8);
    Reset = 1;

    node n=source(e), m=target(e), u, v, w;
    edge f, g;
    if (Index(n,m) == NULL)
      {
        color_edge(e, red);
        if (! text_panel("Insert edge ("+GW->get_label(n)+","+GW->get_label(m)+")", string("The insertion of\\red edge (")+GW->get_label(n)+
            ","+GW->get_label(m)+")\\black gives rise to new paths in the graph. All the descendant trees have to be updated."))
            return;
        
        forall_nodes(u, *the_graph)
          {
            color_desc(u, blue, cyan);
            if (! path(u, n)) color_node(n, red);
            if (! path(u, m)) color_node(m, red);
            color_edge(e, red);
            window* W = text_window("Update Desc("+GW->get_label(u)+")", "Updating the descendant tree\\blue Desc("+
                GW->get_label(u)+")\\black .\\blue Desc("+GW->get_label(u)+")\\black is colored blue.");
            
            if ((Index(u,n) != NULL) && (Index(u,m) == NULL))
              {
                if (! message(W, "The insertion of edge ("+GW->get_label(n)+","+GW->get_label(m)+
                    ") gives rise to a new path from node "+GW->get_label(u)+" to |node "+GW->get_label(m)+
                    ". Edge ("+GW->get_label(n)+","+GW->get_label(m)+") has to be inserted into Desc("+
                    GW->get_label(u)+"). |Performing a smart BFS rooted at node "+GW->get_label(m)+"."))
                    return;
                color_node(m, blue);
                color_edge(e, blue);
                            
                Index(u,m) = Desc[u].new_node(m);
                Desc[u].new_edge(Index(u,n),Index(u,m),e);
                list<node> L;
                L.push(m);
                while (! L.empty())
                  {
                    v = L.pop();
                    forall_out_edges(f, Index(m,v))
                      {
                        w = Desc[m].inf(target(f));
                        if (Index(u,w) == NULL)
                          {
                            g = Desc[m].inf(f);
                            color_node(w, red);
                            color_edge(g, red);
                            if (! message(W, "There is a path from node "+GW->get_label(u)+" to node "+GW->get_label(w)+
                                ". Node "+GW->get_label(w)+" is not yet in Desc("+GW->get_label(u)+"). |Therefore edge ("+
                                GW->get_label(source(g))+","+GW->get_label(w)+") has to be inserted into Desc("+
                                GW->get_label(u)+")."))
                                return; 
                            color_node(w, blue);
                            color_edge(g, blue);
                            L.push(w);
                            Index(u,w) = Desc[u].new_node(w);
                            Desc[u].new_edge(Index(u,v), Index(u,w), g);
                          }
                      }
                  }
                if (! message(W, "Desc("+GW->get_label(u)+") is up to date."))
                    return;
              }
            else
              {
                if (Index(u,n) == NULL)
                  {
                    if (! message(W, "Node "+GW->get_label(n)+" is not in Desc("+GW->get_label(u)+
                        "). Therefore the insertion of edge ("+GW->get_label(n)+","+GW->get_label(m)+
                        ") does |not give rise to any new paths in Desc("+GW->get_label(u)+"). |Desc("+GW->get_label(u)+
                        ") is up to date."))
                        return;
                  }
                else
                  {
                    if (! message(W, "Node "+GW->get_label(m)+" is already in Desc("+GW->get_label(u)+
                        "). Therefore the insertion of edge ("+GW->get_label(n)+","+GW->get_label(m)+
                        ") does |not give rise to any new paths in Desc("+GW->get_label(u)+"). |Desc("+GW->get_label(u)+
                        ") is up to date."))
                        return;
                  }
              }
            W->close();
          }
      }
    else
      {
        color_path(n, m, blue, cyan, cyan);
        color_edge(e, red);
        if (! text_panel("Insert edge ("+GW->get_label(n)+","+GW->get_label(m)+")", 
            string("There is already a path from\\cyan node ")+GW->get_label(n)+"\\black to\\cyan node "+
            GW->get_label(m)+"\\black . The path is colored blue. The insertion of\\red edge ("+GW->get_label(n)+","+
            GW->get_label(m)+")\\black does not give rise to any new paths. Therefore no descendant tree needs to be updated.\\c2"))
            return;
      }

    GW->reset();
    if (! text_panel("End", "The data structure is up to date. End of animation."))
        return;
    GW->zoom(1.25);
  }

/******************** REMOVE EDGE ********************/

void dtc_italiano_anim::anim_remove_edge(edge e)
  {
    GW->zoom(0.8);
    if (Reset) reset();

    node u, v, n=source(e), m=target(e);
    edge f, g;
    forall_nodes(u, *the_graph)
        if ((f = Hook(u,m)) == e) Hook(u,m) = the_graph->in_succ(f);
    
    color_edge(e, red);
    if (! text_panel("Remove edge ("+GW->get_label(n)+","+GW->get_label(m)+")",
        string("Removing\\red edge (")+GW->get_label(n)+","+GW->get_label(m)+
        ")\\black from the graph. All the descendant trees have to be updated."))
        return;
        
    forall_nodes(u, *the_graph)
      { 
        color_desc(u, blue, cyan);
        if (! path(u, n)) color_node(n, red);
        if (! path(u, m)) color_node(m, red);
        color_edge(e, red);
        window* W = text_window("Update Desc("+GW->get_label(u)+")", "Updating the descendant tree\\blue Desc("+
            GW->get_label(u)+")\\black .\\blue Desc("+GW->get_label(u)+")\\black is colored blue.");
        
        if ((u != m) && (Index(u,m) != NULL))
          { 
            f = Desc[u].first_in_edge(Index(u,m));
            if (Desc[u].inf(f) == e)
              { 
                color_node(m, red);
                if (! message(W, "Edge ("+GW->get_label(n)+","+GW->get_label(m)+
                    ") is in Desc("+GW->get_label(u)+") and has to be removed. |Trying to find a hook for node "+
                    GW->get_label(m)+"."))
                    return;
                reset_edge(e);
                
                Desc[u].del_edge(f);
                list<node> L;
                L.push(m);
                while (! L.empty())
                  {
                    v = L.pop();
                    color_node(v, red);
                    g = Hook(u,v);
                    bool hook = 0;
                    while ((g != NULL) && (! hook))
                      { 
                        if ((g != e) && (Index(u,source(g)) != NULL) && (source(g) != target(g)))
                          {
                            color_edge(g, green);
                            if (! message(W, "Edge ("+GW->get_label(source(g))+","+GW->get_label(v)+
                                ") is a (tentative) hook for node "+GW->get_label(v)+" in Desc("+GW->get_label(u)+")."))
                                return;
                            color_edge(g, blue);
                            color_node(v, blue);
                            
                            Desc[u].new_edge(Index(u,source(g)), Index(u,v), g);
                            hook = 1;
                          }
                        g = Hook(u,v) = the_graph->in_succ(g);
                      }
                    if (! hook)
                      {
                        forall_out_edges(g, Index(u,v))
                          {
                            color_node(Desc[u].inf(target(g)), cyan);
                            color_edge(Desc[u].inf(g), cyan);
                          }
                        if (! message(W, "There is no hook for node "+GW->get_label(v)+" in Desc("+GW->get_label(u)+
                            "). Node "+GW->get_label(v)+" has to be removed |from Desc("+GW->get_label(u)+
                            "). |Trying to find hooks for the children of node "+GW->get_label(v)+
                            " in Desc("+GW->get_label(u)+")."))
                            return;
                        reset_node(v);
                        forall_out_edges(g, Index(u,v))
                          {
                            L.push(Desc[u].inf(target(g)));
                            reset_edge(Desc[u].inf(g));
                          }
                        Desc[u].del_node(Index(u,v));
                        Index(u,v) = NULL;
                      }
                  }
                if (! message(W, "Desc("+GW->get_label(u)+") is up to date.")) return;
              }
            else
              {
                if (! message(W, "Edge ("+GW->get_label(n)+","+GW->get_label(m)+
                    ") is not in Desc("+GW->get_label(u)+"). |Desc("+GW->get_label(u)+") is up to date."))
                    return;
              }
          }
        else
          {
            if (! message(W, "Edge ("+GW->get_label(n)+","+GW->get_label(m)+
                ") is not in Desc("+GW->get_label(u)+"). |Desc("+GW->get_label(u)+") is up to date."))
                return;
          }
        W->close();
      }
    
    GW->reset();
    if (! text_panel("End", "The data structure is up to date. End of animation.")) return;
    GW->zoom(1.25);
  }

#endif _DTC_ITALIANO_ANIM_H
