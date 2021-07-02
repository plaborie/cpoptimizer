// --------------------------------------------------------------------
//
//  File:        dtc_demo.c
//  Date:        10/97
//  Last update: 03/99
//  Description: A demo program for dynamic transitive closure algorithms
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------
//
// REMARK: incorporates Yellin's algorithm
//


#include <LEDA/graphwin.h>
#include <LEP/dynamic_graphs/msg_graph.h>
#include <LEP/dynamic_graphs/dtc_italiano_anim.h>
#include <LEP/dynamic_graphs/dtc_italiano_nr.h>
#include <LEP/dynamic_graphs/dtc_italiano_opt.h>
#include <LEP/dynamic_graphs/dtc_italiano_gen.h>
#include <LEP/dynamic_graphs/dtc_yellin.h>
#include <LEP/dynamic_graphs/dtc_hk_1.h>
#include <LEP/dynamic_graphs/dtc_hk_2.h>
#include <LEP/dynamic_graphs/dtc_dfs.h>
#include <LEP/dynamic_graphs/dtc_dfs_nr.h>
#include <LEP/dynamic_graphs/dtc_bfs.h>
#include <LEP/dynamic_graphs/dtc_dbfs.h>

#define ALG0  dtc_italiano_anim
#define ALG1  dtc_italiano_nr
#define ALG2  dtc_italiano_opt
#define ALG3  dtc_italiano_gen
#define ALG4  dtc_hk_1
#define ALG5  dtc_hk_2
#define ALG6  dtc_dfs
#define ALG7  dtc_dfs_nr
#define ALG8  dtc_bfs
#define ALG9  dtc_dbfs
#define ALG10 dtc_yellin

msg_graph	G;					/* The current graph */
msg_graph	GEmpty;					/* Empty graph */

GraphWin	GW(G, 700, 700);			/* Main window */
panel*		GW_Menu = new panel();			/* Panel for algorithm specific buttons */
menu		Menu_1;					/* Distinguished nodes submenu */
menu		Menu_2;					/* Special nodes submenu */

const int	AlgsAnz		= 11;			/* Number of algorithms */
dga_base*	Algs[AlgsAnz]	= { new ALG0(GEmpty, GW), new ALG1(&GEmpty), new ALG2(&GEmpty), new ALG3(&GEmpty), new ALG4(&GEmpty),
				    new ALG5(&GEmpty), new ALG6(&GEmpty), new ALG7(&GEmpty), new ALG8(&GEmpty), new ALG9(&GEmpty), new ALG10(&GEmpty) };
list<string>	AlgsName;				/* Names of algorithms */

dga_base*	Alg;					/* The current algorithm */
int		AlgNum		= 0;			/* Number of current algorithm */
int		Warning		= 1;			/* Italiano warning: 1 = on, 0 = off */

/******************** USEFUL FUNCTIONS ********************/

void ok_panel(string s1, string s2)
  {
    panel P;
    P.text_item(string("\\bf\\blue ")+s1+"\\rm\\black\\c4 "+s2);
    P.button("Ok");
    P.open(GW.get_window(), window::center, 30);
    P.close();
  }

node get_node(string s1, string s2)
  {
    panel P;
    P.text_item(string("\\bf\\blue ")+s1+"\\rm\\black\\c4 "+s2);
    P.display(GW.get_window(), window::center, 37);
    node n = GW.read_node();
    P.close();
    return n;
  }

edge get_edge(string s1, string s2)
  {
    panel P;
    P.text_item(string("\\bf\\blue ")+s1+"\\rm\\black\\c4 "+s2);
    P.display(GW.get_window(), window::center, 37);
    edge e = GW.read_edge();
    P.close();
    return e;
  }

/******************** PATH ********************/

void path(GraphWin& GW)
  {
    node n1 = get_node("Path", "Please select a source node.");
    if (n1)
      {
        GW.set_color(n1, red);
        node n2 = get_node("Path", "Please select a target node.");
        GW.set_color(n2, blue);
        if (Alg->query(n1, n2))
            ok_panel("Path", string("There is a path from \\red node ")+GW.get_label(n1)+" \\black to \\blue node "+
                GW.get_label(n2)+"\\black .");
        else
            ok_panel("Path", string("There is no path from \\red node ")+GW.get_label(n1)+" \\black to \\blue node "+
                GW.get_label(n2)+"\\black .");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
    GW.redraw();
  }

/******************** SHOW_PATH ********************/

void get_path(list<node>& LNodes, list<edge>& LEdges)
  {
    node n1, n2;
    edge e;
    bool found;
    if (! LNodes.empty())
        n1 = LNodes.pop();
    forall (n2, LNodes)
      {
        found = false;
        forall_out_edges(e, n1)
          {
            if ((target(e) == n2) && (! found))
              {
                LEdges.append(e);
                found = true;
              }
          }
        n1 = n2;
      }
  }

void show_path(GraphWin& GW)
  {
    node n1 = get_node("Show Path", "Please select a source node.");
    if (n1)
      {
        list<edge> LEdges;
        list<node> LNodes;
        bool Result; 
        GW.set_color(n1, red);
        node n2 = get_node("Show Path", "Please select a target node.");
        switch(AlgNum)
          {
            case 0: if (Result = ((ALG0*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG0*) Alg)->query(n1, n2, LEdges);
                    else
                        ((ALG0*) Alg)->desc(n1, LNodes);
                    break;
            case 1: if (Result = ((ALG1*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG1*) Alg)->query(n1, n2, LEdges);
                    else
                        ((ALG1*) Alg)->desc(n1, LNodes);
                    break;
            case 2: if (Result = ((ALG2*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG2*) Alg)->query(n1, n2, LEdges);
                    else
                        ((ALG2*) Alg)->desc(n1, LNodes);
                    break;
            case 3: if (Result = ((ALG3*) Alg)->query(n1, n2, LNodes, LEdges))
                        get_path(LNodes, LEdges); 
                    else
                        ((ALG3*) Alg)->desc(n1, LNodes); break;
            case 4: if (Result = ((ALG4*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG4*) Alg)->query(n1, n2, LEdges);
                    else
                        ((ALG4*) Alg)->desc(n1, LNodes);
                    break;
            case 5: if (Result = ((ALG5*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG5*) Alg)->query(n1, n2, LEdges);
                    else
                        ((ALG5*) Alg)->desc(n1, LNodes);
                    break;
            case 6: if (Result = ((ALG6*) Alg)->query(n1, n2, LNodes))
                        ((ALG6*) Alg)->query(n1, n2, LEdges);
                    break;
            case 7: if (Result = ((ALG7*) Alg)->query(n1, n2, LNodes))
                        ((ALG7*) Alg)->query(n1, n2, LEdges);
                    break;
            case 8: if (Result = ((ALG8*) Alg)->query(n1, n2, LNodes))
                        ((ALG8*) Alg)->query(n1, n2, LEdges);
                    break;
            case 9: if (Result = ((ALG9*) Alg)->query(n1, n2, LNodes))
                        ((ALG9*) Alg)->query(n1, n2, LEdges);
                    break;
           case 10: if (Result = ((ALG10*) Alg)->query(n1, n2, LNodes, LEdges))
                        ((ALG10*) Alg)->query(n1, n2, LNodes, LEdges);
                    else
                        ((ALG10*) Alg)->desc(n1, LNodes);
                    break;
          }
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        if (Result)
          {
            GW.set_color(LNodes, green);
            GW.set_color(n1, red);
            GW.set_color(n2, blue);
            ok_panel("Show Path", string("There is a path from\\red node ")+GW.get_label(n1)+"\\black to\\blue node "+
                GW.get_label(n2)+"\\black . The nodes and edges on that path are drawn green.");
          }
        else
          {
            GW.set_color(LNodes, orange);
            GW.set_color(n1, red);
            GW.set_color(n2, blue);
            ok_panel("Show Path", string("There is no path from\\red node ")+GW.get_label(n1)+"\\black to\\blue node "+
                GW.get_label(n2)+"\\black . The descendant nodes of \\red node "+GW.get_label(n1)+" \\black are colored orange."+
                " The edges in the cut separating them from the non descendand nodes are drawn green.");
          }
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** DESC ********************/

void show_desc(int i)
  {
    node n = get_node("Desc", "Please select a node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        switch(AlgNum)
          {
            case 0: ((ALG0*) Alg)->desc(n, LNodes, LEdges); break;
            case 1: ((ALG1*) Alg)->desc(n, LNodes, LEdges); break;
            case 2: ((ALG2*) Alg)->desc(n, LNodes, LEdges); break;
            case 3: ((ALG3*) Alg)->desc(n, LNodes); break;
            case 4: ((ALG4*) Alg)->desc(n, LNodes); break;
            case 5: ((ALG5*) Alg)->desc(n, LNodes); break;
           case 10: ((ALG10*) Alg)->desc(n, LNodes); break;
          }
        GW.set_color(LNodes, green);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        GW.set_color(n, red);
        ok_panel("Desc", "The descendant nodes of\\red node "+GW.get_label(n)+"\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** SCC ********************/

void show_scc(int i)
  {
    node n = get_node("SCC", "Please select a node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        ((ALG3*) Alg)->scc_graph(n, LNodes, LEdges);
        GW.set_color(LNodes, green);
        GW.set_color(n, red);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        ok_panel("SCC", "The nodes in the same component as\\red node "+GW.get_label(n)+"\\black are colored green. The edges in the sparse graph are colored green as well.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** OUT TREE ********************/

void show_out(int i)
  {
    list<node> LNodes;
    list<edge> LEdges;
    node n = get_node("Out Tree", "Please select a node.");
    if (n)
      {
        switch (AlgNum)
          {
            case 4: ((ALG4*) Alg)->out_tree(n, LNodes, LEdges); break;
            case 5: ((ALG5*) Alg)->out_tree(n, LNodes, LEdges); break;
          }
        GW.set_color(LNodes, green);
        GW.set_color(n, red);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        ok_panel("Out Tree", "The nodes and edges in the tree rooted at\\red node "+GW.get_label(n)+
            "\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** DIST NODE ********************/

void show_dstng_nodes(int i)
  {
    list<node> LNodes;
    switch (AlgNum)
      {
        case 4: ((ALG4*) Alg)->dist_nodes(LNodes); break;
        case 5: ((ALG5*) Alg)->dist_nodes(LNodes); break;
      }
    GW.set_color(LNodes, red);
    if (LNodes.empty())
        ok_panel("Dstng Node - All", "There are no distinguished nodes.");
    else
        ok_panel("Dstng Node - All", "The\\red distinguished nodes\\black are colored red.");
    GW.reset();
  }

void show_dstng_node_out(int i)
  {
    node n =get_node("Dstng Node - Out Tree", "Please select a distinguished node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        switch (AlgNum)
          {
            case 4: ((ALG4*) Alg)->dist_node_out_tree(n, LNodes, LEdges); break;
            case 5: ((ALG5*) Alg)->dist_node_out_tree(n, LNodes, LEdges); break;
          }
        GW.set_color(LNodes, green);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        GW.set_color(n, red);
        ok_panel("Dstng Node - Out Tree", "The nodes and edges in the tree rooted at\\red node "+
            GW.get_label(n)+"\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

void show_dstng_node_in(int i)
  {
    node n = get_node("Dstng Node - In Tree", "Please select a distinguished node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        switch (AlgNum)
          {
            case 4: ((ALG4*) Alg)->dist_node_in_tree(n, LNodes, LEdges); break;
            case 5: ((ALG5*) Alg)->dist_node_in_tree(n, LNodes, LEdges); break;
          }
        GW.set_color(LNodes, green);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        GW.set_color(n, red);
        ok_panel("Dstng Node - In Tree", "The nodes and edges in the tree rooted at\\red node "+
            GW.get_label(n)+"\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** SPEC NODE ********************/

void show_spcl_nodes(int i)
  {
    list<node> LNodes;
    ((ALG5*) Alg)->spec_nodes(LNodes);
    GW.set_color(LNodes, red);
    if (LNodes.empty())
        ok_panel("Spcl Node - All", "There are no special nodes.");
    else
        ok_panel("Spcl Node - All", "The\\red special nodes\\black are colored red.");
    GW.reset();
  }

void show_spcl_node_out(int i)
  {
    node n = get_node("Spcl Node - Out Tree", "Please select a special node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        ((ALG5*) Alg)->spec_node_out_tree(n, LNodes, LEdges);
        GW.set_color(LNodes, green);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        GW.set_color(n, red);
        ok_panel("Spcl Node - Out Tree", "The nodes and edges in the tree rooted at\\red node "+
            GW.get_label(n)+"\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

void show_spcl_node_in(int i)
  {
    node n = get_node("Spcl Node - In Tree", "Please select a special node.");
    if (n)
      {
        list<node> LNodes;
        list<edge> LEdges;
        ((ALG5*) Alg)->spec_node_in_tree(n, LNodes, LEdges);
        GW.set_color(LNodes, green);
        GW.set_color(LEdges, green);
        GW.set_width(LEdges, 3);
        GW.set_color(n, red);
        ok_panel("Spcl Node - In Tree", "The nodes in the tree rooted at\\red node "+
            GW.get_label(n)+"\\black are colored green.");
        GW.reset();
      }
    else
        ok_panel("Error", "Please create a digraph first.");
  }

/******************** ANIMATION ********************/

void anim_ins(int i)
  {
    node n2, n1 = get_node("Animation Ins", "Please select a source node.");
    if (n1)
      {
        GW.set_color(n1, red);
        n2 = get_node("Animation Ins", "Please select a target node.");
        GW.set_color(n2, red);
        ((ALG0*) Alg)->set_animation(1);
        ((ALG0*) Alg)->anim_insert_edge(GW.new_edge(n1, n2));
        ((ALG0*) Alg)->set_animation(0);
      }
  }

void anim_rem(int i)
  {
    edge e = get_edge("Animation Rem", "Please select an edge.");
    if (e)
      {
        GW.set_color(e, red);
        ((ALG0*) Alg)->anim_remove_edge(e);
        GW.del_edge(e);
      }
  }

/******************** ALGORITHM ********************/

void show_gw_menu()
  {
    GW.set_frame_label(string("Demo: ")+Alg->name());
    GW.enable_call(74);		/* show path button */
    
    GW_Menu->close();
    delete GW_Menu;
    GW_Menu = new panel(700, 35);
    switch (AlgNum)
      {
        case 0:  GW_Menu->button("Desc", show_desc);		/* Italiano */
                 GW_Menu->button("Anim Insert", anim_ins);
                 GW_Menu->button("Anim Remove", anim_rem); break;
        case 1:  GW_Menu->button("Desc", show_desc); break;	/* Italiano NR */
        case 2:  GW_Menu->button("Desc", show_desc); break;	/* Optimized */
        case 3:  GW_Menu->button("Desc", show_desc);		/* General */
                 GW_Menu->button("SCC", show_scc); break;
        case 4:  GW_Menu->button("Desc", show_desc);		/* HK 1 */
                 GW_Menu->button("Out Tree", show_out);
                 GW_Menu->button("Dstng Node", 0, Menu_1); break;
        case 5:  GW_Menu->button("Desc", show_desc);		/* HK 2 */
                 GW_Menu->button("Out Tree", show_out);
                 GW_Menu->button("Dstng Node", 0, Menu_1);
                 GW_Menu->button("Spcl Node", 0, Menu_2); break;
        case 6:  break;	/* DFS */
        case 7:  break;	/* DFS NR */
        case 8:  break;	/* BFS */
        case 9:  break;	/* DBFS */
       case 10:  GW_Menu->button("Desc", show_desc); break; /* Yellin */
      }
    GW_Menu->display(GW.get_window(), 0, 60);
  }

void algorithm_select(GraphWin& GW)
  {
    panel P;
    P.text_item("\\bf\\blue Algorithm - Select \\rm\\black\\c4 Please select a dynamic transitive closure algorithm.");
    P.choice_item("", AlgNum, AlgsName);
    P.button("Ok");
    P.open(GW.get_window(), window::center, 30);
    P.close();

    Alg->init(&GEmpty);
    Alg = Algs[AlgNum];
    Alg->init(&G);
    if (AlgNum < 2)
        Warning = 1;
    
    show_gw_menu();
  }

void algorithm_description(GraphWin& GW)
  {
    ok_panel("Algorithm - Description", string("The current algorithm is\\red ")+Alg->name()+"\\black . "+
        Alg->description()+" Version "+Alg->version()+" from "+Alg->timestamp()+". Implemented by "+
        Alg->authors()+".");
  }

/******************** INIT ********************/

void initialize(GraphWin& GW)
  {
    Alg->init(&G);
    if (AlgNum < 2)
        Warning = 1;
    ok_panel("Init", "The data structure was reinitialized.");
  }
  
/******************** DEL EDGE HANDLER ********************/

bool del_edge_handler(GraphWin& GW, edge e)
  {
    if ((AlgNum < 2) && (Warning) && (! Is_Acyclic(G)))
      {
        GW.redraw();
        panel P;
        P.text_item("\\bf\\blue Warning \\rm\\black\\c4 The graph is not acyclic. The data structure may not be updated correctly. Do you really want to remove this edge?");
        P.choice_item("Repeat warning ", Warning, "Off", "On");
        P.button("Remove", 1);
        P.button("Cancel", 0);
        bool b = (bool) P.open(GW.get_window(), window::center, 30);
        P.close();
        return b;
      }
    return true;
  }

/******************** MAIN ********************/

main()
  {
    AlgsName.append("Ital");
    AlgsName.append("Ital NR");
    AlgsName.append("Ital Opt");
    AlgsName.append("Ital Gen");
    AlgsName.append("HK 1");
    AlgsName.append("HK 2");
    AlgsName.append("DFS");
    AlgsName.append("DFS NR");
    AlgsName.append("BFS");
    AlgsName.append("DBFS");
    AlgsName.append("Yellin");
    Alg = Algs[AlgNum];
    Alg->init(&G);

    GW.set_node_shape(circle_node);
    GW.set_node_color(yellow);
    GW.set_node_border_color(black);
    GW.set_node_width(18);
    GW.set_node_height(18);
    GW.set_edge_style(solid_edge);
    GW.set_edge_color(black);
    GW.set_edge_width(1);
    GW.set_directed(true);
    GW.set_show_status(true);
    GW.set_del_edge_handler(del_edge_handler);
    GW.reset();
    
    Menu_1.button("All", show_dstng_nodes);
    Menu_1.button("Out Tree", show_dstng_node_out);
    Menu_1.button("In Tree", show_dstng_node_in);
    Menu_2.button("All", show_spcl_nodes);
    Menu_2.button("Out Tree", show_spcl_node_out);
    Menu_2.button("In Tree", show_spcl_node_in);

    gw_add_simple_call(GW, path, "Path", 0);
    gw_add_simple_call(GW, show_path, "Show Path", 0);
    gw_add_simple_call(GW, initialize, "Init", 0);
    int AlgorithmMenu = gw_add_menu(GW, "Algorithm");
    gw_add_simple_call(GW, algorithm_select, "Select", AlgorithmMenu);
    gw_add_simple_call(GW, algorithm_description, "Description", AlgorithmMenu);
    
    show_gw_menu();
    GW.open(50, 50);
    GW.close();
  }
