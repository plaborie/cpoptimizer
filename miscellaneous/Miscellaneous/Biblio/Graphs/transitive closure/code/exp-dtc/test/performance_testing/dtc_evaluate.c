// --------------------------------------------------------------------
//
//  File:        dtc_evaluate.c
//  Date:        10/97
//  Last update: 03/99
//  Description: Evaluation program for dynamic transitive closure algorithms
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#include <LEDA/window.h>
#include <LEDA/stream.h>

/******************** Variables ********************/

window	WTest("DTC Test");
const	int AlgAnz	= 13;
string	AlgName[AlgAnz]	= { "DFS", "DFS NR", "BFS", "DBFS", "Italiano", "Italiano NR", "Italiano Opt", "Italiano Gen", "HK 1", "HK 2", "Yellin", "CFNP", "CFNP Opt" };
string	AlgSuffix[AlgAnz] = { "dfs", "dfs_nr", "bfs", "dbfs", "ital", "ital_nr", "ital_opt", "ital_gen", "hk_1", "hk_2", "yellin", "cfnp", "cfnp_opt" };

#include "dtc_variables.h"
#include "dtc_functions.h"
#include "dtc_result.h"
#include "dtc_test_gen.h"
#include "dtc_test_exe.h"

dtc_result Result;
dtc_test_gen TestGen;
dtc_test_exe TestExe;

/******************** Generate ********************/

/********** Graph Panel **********/
int graph_panel()
  {
    panel P("Initial Graph");
    P.int_item("Number of vertices (low) ", NodeAnzLow);
    P.int_item("Number of vertices (high) ", NodeAnzHigh);
    P.int_item("Number of vertex sampling steps ", NodeStep);
    if (! Random) P.int_item("Number of vertices in subgraph ", SubNodeAnz);
    if (Random) P.int_item("Number of edges (low) ", EdgeAnzLow);
    if (Random) P.int_item("Number of edges (high) ", EdgeAnzHigh);
    if (Random) P.int_item("Number of edge sampling steps ", EdgeStep);
    P.choice_item("Acyclic ", Acyclic, "false", "true");
    if (Random) P.choice_item("Simple ", Simple, "false", "true");
    if (Random) P.choice_item("Bounded degree", DgrOn, "false", "true");
    if (Random) P.int_item("Maximum node degree ", Dgr);
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    to_range(NodeAnzLow, 10);
    to_range(NodeAnzHigh, NodeAnzLow);
    to_range(EdgeAnzLow, 0);
    to_range(EdgeAnzHigh, EdgeAnzLow);
    to_range(NodeStep, 1, 100);
    to_range(EdgeStep, 1, 100);
    to_range(SubNodeAnz, 1, NodeAnzHigh);
    to_range(Dgr, 0);
    return value;
  }

/********** Operations Panel **********/
int opseq_panel()
  {
    if (QryType != _mixed_) QryType -= 3;
    panel P("Sequence of Operations");
    P.int_item("Number of operations ", OpAnz);
    P.int_item("Percentage of queries ", OpPerc, 0, 100);
    P.choice_item("Type of queries ", QryType, "bool", "path", "mixed");
    P.int_item("Percentage of boolean queries (of all queries) ", QryPerc, 0, 100);
    P.choice_item("Type of updates ", UpdType, "insert", "remove", "mixed");
    if (Random) P.int_item("Percentage of insert operations (of all updates) ", UpdPerc, 0, 100);
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    if (QryType != _mixed_) QryType += 3;
    to_range(OpAnz, 0);
    return value;
  }

/********** Generate Load Panel **********/
int generate_load_panel()
  {
    list<string> Content;
    get_dir_content(SaveDir, Content);
    panel P("Generate");
    P.string_item("Load graph from file ", GraphFile);
    P.int_item("Number of different OpSeq ", OpSeqAnz);
    P.string_item("Name of test sequence ", Tmp, Content);
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    to_range(OpSeqAnz, 1, 100);
    return value;
  }

/********** Generate Gen Panel **********/
int generate_gen_panel()
  {
    list<string> Content;
    get_dir_content(SaveDir, Content);
    panel P("Generate");
    if (Random) P.int_item("Number of different graphs ", GraphAnz);
    if (Random) P.int_item("Number of different operation sequences ", OpSeqAnz);
    P.string_item("Name of test sequence ", Tmp, Content);
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    to_range(GraphAnz, 1, 100);
    to_range(OpSeqAnz, 1, 100);
    return value;
  }

void generate(int i)
  {
    int value, Status = 1;
    Tmp = SessionDir;
    while (Status < 6)
      {
        switch (Status)
          {
            case 0: if (GenOn) load_test_index(); return;
            case 1: value = simple_panel("Generate", _gen_random_); if (! Random) LoadGraph = 0; break;
            case 2: value = (Random) ? simple_panel("Generate", _gen_load_) : value; break;
            case 3: value = (LoadGraph) ? generate_load_panel() : generate_gen_panel(); break;
            case 4: value = (LoadGraph) ? value : graph_panel(); break;
            case 5: value = opseq_panel(); break;
          }
        if (value == 2) Status--;					/* Back */
        if (value == 1) Status++;					/* Ok */
        if (value == 0) { if (GenOn) load_test_index(); return; }	/* Cancel */
      }
    
    SessionDir = Tmp;
    graph Graph;
    if ((LoadGraph) && (! load_graph(Graph))) return;
    clean_test_index();
    RunOn = false;
    if (GenOn = TestGen.generate(Graph, WTest)) save_test_index();
    else system("rm -r -f "+SaveDir+"/"+SessionDir);
  }

/******************** Load ********************/

void load(int i)
  {
    list<string> Content;
    get_dir_content(SaveDir, Content);
    if (Content.empty()) { ok_panel("Error","Unable to load a session. There is no previous session."); return; }
    Tmp = Content.head();
    panel P("Load");
    P.string_item("Test sequence to load ", Tmp, Content);
    P.button("Back", 0);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    if (! P.open()) return;
    SessionDir = Tmp;
    if (GenOn = load_test_index()) RunOn = load_result_index();
    else RunOn = false;
  }

/******************** Run ********************/

int run_panel()
  {
    panel P("Run");
    P.int_item("Large step (in operations) ", LargeStep);
    P.int_item("Small step (in operations) ", SmallStep);
    P.int_item("Number of iterations per test ", IterAnz);
    P.choice_item("Correctness check ", CheckOn, "off", "on");
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    to_range(SmallStep, 10);
    to_range(LargeStep, SmallStep);
    to_range(IterAnz, 1, 100);
    return value;
  }

void run(int i)
  {
    if (! GenOn) { ok_panel("Error", "Please load or generate a test first."); return; }
    int value, Status = 1;
    while (Status<3)
      {
        switch (Status)
          {
            case 0: if (RunOn) load_result_index(); return;
            case 1: value = run_panel(); break;
            case 2: value = simple_panel("Run", _run_alg_); break;
          }
        if (value == 2) Status--;					/* Back */
        if (value == 1) Status++;					/* Ok */
        if (value == 0) { if (RunOn) load_result_index(); return; }	/* Cancel */
      }
    
    if (RunOn = TestExe.run(WTest)) save_result_index();
    else system("rm -r -f "+SaveDir+"/"+SessionDir+"/"+ResultDir);
  }

/******************** Print ********************/

int print_panel(list<string> &ResultList)	/* random */
  {
    string Patern;
    int n = 0, e = 0;
    list<string> NodeList, EdgeList;
    get_dir_content(SaveDir+"/"+SessionDir+"/"+ResultDir, ResultList);
    ResultList.sort(&compare_result);
    extract_result(ResultList, NodeList, EdgeList);

    panel P("Print");
    P.string_item("Output file name ", OutputFile);
    P.choice_item("Output format ", OutputFormat, "View", "LaTeX", "GnuPlot", "PostScript");
    if ((! Random) && (OutputAxes == _ops_)) P.choice_item("Initial number of nodes ", n, NodeList);
    if ((Random) && (OutputAxes != _nodes_)) P.choice_item("Initial number of nodes ", n, NodeList);
    if ((Random) && (OutputAxes != _edges_)) P.choice_item("Initial number of edges ", e, EdgeList);
    if (OutputType == _time_)
      {
        if (OutputAxes != _ops_) P.choice_item("Show time for ", OutputTime, "Ops+Init", "Ops", "Init");
        else { to_range(OutputTime, 0, 1); P.choice_item("Show time for ", OutputTime, "Ops+Init", "Ops"); }
      }
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    int value = P.open();
    
    switch (OutputAxes)
      {
        case _ops_:   if (Random) extract_result(ResultList, NodeList[NodeList[n]], EdgeList[EdgeList[e]]);
                      else extract_result(ResultList, NodeList[NodeList[n]], ""); break;
        case _nodes_: if (Random) extract_result(ResultList, "", EdgeList[EdgeList[e]]); break;
        case _edges_: if (Random) extract_result(ResultList, NodeList[NodeList[n]], ""); break;
      }
    
    return value;
  }

void print(int i)
  {
    if (! RunOn) { ok_panel("Error", "Please load or run a test first."); return; }
    list<string> ResultList;
    int value = 1, Status = 1;
    while (Status<6)
      {
        switch (Status)
          {
            case 0: load_result_index(); return;
            case 1: if (CheckOn) value = simple_panel("Print", _print_type_); else OutputType = _time_; break;
            case 2: value = simple_panel("Print", _print_axes_); break;
            case 3: value = print_panel(ResultList); break;
            case 4: value = simple_panel("Print", _print_alg_); break;
            case 5: value = ((OutputFormat == _gnuplot_) || (OutputFormat == _ps_)) ? simple_panel("Print", _print_frmt_) : 1; break;
          }
        if (value == 2) Status--;				/* Back */
        if (value == 1) Status++;				/* Ok */
        if (value == 0) { load_result_index(); return; }	/* Cancel */
      }
    switch (OutputFormat)
      {
        case _latex_:   Result.print_latex(ResultList, OutputFile); break;
        case _gnuplot_: Result.print_gnuplot(ResultList, OutputFile); break;
        case _ps_:      Result.print_ps(ResultList, OutputFile); break;
        case _view_:    Result.view(ResultList); break;
      }
    load_result_index();
  }

/******************** Main ********************/

main()
  {
    for (int a=0; a<AlgAnz; a++) { AlgOn[a] = 0; AlgLabel[a] = AlgName[a]+" "; while (AlgLabel[a].length()<25) AlgLabel[a]+=" "; }
    
    WTest.buttons_per_line(3);
    WTest.button("Generate", 0, generate);
    WTest.button("Run", 0, run);
    WTest.button("Load", 0, load);
    WTest.button("Show Result", 0, print);
    WTest.button("Quit", 1);
    WTest.set_redraw(message);
    
    WTest.open(window::center, 100);
    do message(&WTest);
    while (WTest.read_mouse() < 1);
    WTest.close();
  }
