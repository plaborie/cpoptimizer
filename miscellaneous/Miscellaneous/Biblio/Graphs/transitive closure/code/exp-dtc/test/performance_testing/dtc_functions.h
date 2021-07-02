// --------------------------------------------------------------------
//
//  File:        dtc_functions.h
//  Date:        10/97
//  Last update: 03/99
//  Description: Global functions used by dtc_evaluate.c
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#ifndef _DTC_FUNCTIONS_H
#define _DTC_FUNCTIONS_H

#include <LEDA/stream.h>
#include <LEDA/window.h>
#include <LEDA/graph.h>

/******************** Check Variable Range ********************/

bool in_range(int value, int low, int high)
  {
    return ((value < low) || (value > high)) ? false : true;
  }

bool to_range(int &value, int low, int high = 0)
  {
    if (value < low) { value = low; return false; }
    if ((high != 0) && (value > high)) { value = high; return false; }
    return true;
  }

/******************** Panel ********************/

void ok_panel(string Title, string Text)
  {
    panel P(Title);
    P.text_item(Text);
    P.button("Ok");
    P.open();
  }

int simple_panel(string Title, int Type)
  {
    int a;
    panel P(Title);
    switch (Type)
      {
        case _print_type_: P.choice_item("Information type ", OutputType, "  Time  ", "Error"); break;
        case _print_axes_: if (OutputType == _time_) P.choice_item("X-Axes label ", OutputAxes, "Nodes", "Edges", " Operations ");
                           else { to_range(OutputAxes, 0, 1); P.choice_item("X-Axes label ", OutputAxes, "  Nodes  ", "Edges"); } break;
        case _print_alg_:  for (a=0; a<AlgAnz; a++) if (AlgOn[a]) P.choice_item(AlgLabel[a], AlgOn[a], "  off  ", "on"); break;
        case _print_frmt_: P.int_item("Font size ", OutputFSize);
                           P.choice_item("Format ", OutputLandscp, "Portrait", "Landscape");
                           P.choice_item("Line Style   ", OutputColor, " Black & White ", "Color");
                           P.choice_item("Line Style ", OutputSolid, "Dashed", "Solid"); break;
        case _run_alg_:    for (a=0; a<AlgAnz; a++) P.choice_item(AlgLabel[a], AlgOn[a], "  off  ", "on"); break;
        case _gen_random_: P.choice_item("", Random, " Non Random ", "Random"); break;
        case _gen_load_:   P.choice_item("", LoadGraph, " Generate Graph ", "Load Graph"); break;
      }
    P.button("Back", 2);
    P.button("Ok", 1);
    P.button("Cancel", 0);
    return P.open();
  }

/******************** Index Functions ********************/

bool save_test_index()
  {
    system("mkdir -p "+SaveDir+"/"+SessionDir+"/"+TestDir);
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+TestDir+"/index", "w");
    fprintf(File, "Random = %i\nLoadGraph = %i\n", Random, LoadGraph);
    fprintf(File, "GraphAnz = %i\nOpSeqAnz = %i\n\n", GraphAnz, OpSeqAnz);
    fprintf(File, "NodeStep = %i [%i, %i]\n", NodeStep, NodeAnzLow, NodeAnzHigh);
    fprintf(File, "EdgeStep = %i [%i, %i]\n", EdgeStep, EdgeAnzLow, EdgeAnzHigh);
    fprintf(File, "Acyclic = %i\nSimple = %i\nDgrOn = %i [%i]\n\n", Acyclic, Simple, DgrOn, Dgr);
    fprintf(File, "OpAnz = %i\nOpPerc = %i\nUpdType = %i\n", OpAnz, OpPerc, UpdType);
    fprintf(File, "UpdPerc = %i\nQryType = %i\nQryPerc = %i\n", UpdPerc, QryType, QryPerc);
    fclose(File);
    return true;
  }

bool load_test_index()
  {
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+TestDir+"/index", "r");
    if (File == 0)
      {
        ok_panel("Error", "Can not load test index. No such file in session "+SessionDir+".");
        return false;
      }
    fscanf(File, "Random = %i\nLoadGraph = %i\n", &Random, &LoadGraph);
    fscanf(File, "GraphAnz = %i\nOpSeqAnz = %i\n\n", &GraphAnz, &OpSeqAnz);
    fscanf(File, "NodeStep = %i [%i, %i]\n", &NodeStep, &NodeAnzLow, &NodeAnzHigh);
    fscanf(File, "EdgeStep = %i [%i, %i]\n", &EdgeStep, &EdgeAnzLow, &EdgeAnzHigh);
    fscanf(File, "Acyclic = %i\nSimple = %i\nDgrOn = %i [%i]\n\n", &Acyclic, &Simple, &DgrOn, &Dgr);
    fscanf(File, "OpAnz = %i\nOpPerc = %i\nUpdType = %i\n", &OpAnz, &OpPerc, &UpdType);
    fscanf(File, "UpdPerc = %i\nQryType = %i\nQryPerc = %i\n", &UpdPerc, &QryType, &QryPerc);
    fclose(File);
    return true;
  }

void clean_test_index()
  {
    if (QryType == _bool_qry_) QryPerc = 100;
    if (QryType == _path_qry_) QryPerc = 0;
    if (UpdType == _insert_) UpdPerc = 100;
    if (UpdType == _remove_) UpdPerc = 0;
    if (! Random)
      {
        EdgeAnzLow = EdgeAnzHigh = LoadGraph = DgrOn = 0;
        GraphAnz = OpSeqAnz = Simple = EdgeStep = 1;
        if (UpdType == _mixed_) UpdPerc = 50;
      }
  }

bool save_result_index()
  {
    system("mkdir -p "+SaveDir+"/"+SessionDir+"/"+ResultDir);
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+ResultDir+"/index", "w");
    fprintf(File, "Active algorithms = ");
    for (int a=0; a<AlgAnz; a++) if (AlgOn[a]) fprintf(File, "%i, ", a);
    fprintf(File, "\nSmallStep = %i\nLargeStep = %i\nIterAnz = %i\nCheckOn = %i\n", SmallStep, LargeStep, IterAnz, CheckOn);
    fclose(File);
    return true;
  }

bool load_result_index()
  {
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+ResultDir+"/index", "r");
    if (File == 0)
      {
        ok_panel("Error", "Can not load result index. No such file in session "+SessionDir+".");
        return false;
      }
    fscanf(File, "Active algorithms = ");
    int a; for (a=0; a<AlgAnz; a++) AlgOn[a] = 0; while (fscanf(File, "%i, ", &a)) if (a<AlgAnz) AlgOn[a] = 1;
    fscanf(File, "\nSmallStep = %i\nLargeStep = %i\nIterAnz = %i\nCheckOn = %i\n", &SmallStep, &LargeStep, &IterAnz, &CheckOn);
    fclose(File);
    return true;
  }

/******************** Test or result file functions ********************/

/* Extract node and edge numbers from file names */
void extract_result(list<string> ResultList, list<string> &List1, list<string> &List2)
  {
    List1.clear();
    List2.clear();
    string s, s1, s2;
    forall (s, ResultList)
      {
        s1 = s(s.pos("-")+1, s.pos("_")-1);
        s2 = s(s.pos("_")+1, s.length());
        if (List1.search(s1) == NULL) List1.append(s1);
        if (List2.search(s2) == NULL) List2.append(s2);
      }
  }

/* Remove all file names but those with appropriate node and edge number */
void extract_result(list<string> &List, string s1, string s2)
  {
    string s;
    list<string> TmpList;
    forall (s, List)
        if ((s(s.pos("-")+1, s.pos("_")-1) == s1) || (s1 == "")) TmpList.append(s);
    List.clear();
    forall (s, TmpList)
        if ((s(s.pos("_")+1, s.length()) == s2) || (s2 == "")) List.append(s);
  }

/* Compare file names according to node and edge numbers */
int compare_result(const string &s1, const string &s2)
  {
    int i1 = atoi(s1(s1.pos("-")+1, s1.pos("_")-1));
    int i2 = atoi(s2(s2.pos("-")+1, s2.pos("_")-1));
    if (i1 != i2) return i1-i2;
    i1 = atoi(s1(s1.pos("_")+1, s1.length()));
    i2 = atoi(s2(s2.pos("_")+1, s2.length()));
    if (i1 != i2) return i1-i2;
    return 0;
  }

/******************** Graph Functions ********************/

bool load_graph(graph& Graph)
  {
    switch (Graph.read(GraphFile))
      {
        case 1: ok_panel("Error", "Can not load graph. File "+GraphFile+" does not exist."); return false;
        case 2: ok_panel("Error", "Can not load graph. Graph in file "+GraphFile+" is not of correct format."); return false;
        case 3: ok_panel("Error", "Can not load graph. File "+GraphFile+" does not contain a graph."); return false;
      }
    Random = GraphAnz = NodeStep = EdgeStep = 1;
    NodeAnz = NodeAnzHigh = NodeAnzLow = Graph.number_of_nodes();
    EdgeAnz = EdgeAnzHigh = EdgeAnzLow = Graph.number_of_edges();
    Acyclic = (int) Is_Acyclic(Graph);
    Simple = (int) Is_Simple(Graph);
    DgrOn = 0;
    return true;
  }

/******************** Get Directory Content ********************/

void get_dir_content(string Dir, list<string>& Content, string Patern = "")
  {
    Content.clear();
    system("mkdir -p "+Dir);
    system("ls "+Dir+"/"+Patern+" > "+Dir+"/dir_index");
    file_istream I(Dir+"/dir_index");
    Content.read(I);
    Content.remove("dir_index");
    Content.remove("index");
    system("rm -f "+Dir+"/dir_index");
  }

/******************** Message ********************/

void message(window* W)
  {
    W->del_messages();
    if (! GenOn) W->message("No test sequence.");
    else 
      {
        if (Random) W->message("Random test sequence:");
        else W->message("Non-random test sequence:");
        W->message("    - Name of test sequence: "+SessionDir);
        W->message(string("    - %i different data sets (%i graphs, %i operation sequences)",GraphAnz*OpSeqAnz, GraphAnz, OpSeqAnz));
        W->message("    - Graphs:");
        W->message(string("        - %i to %i vertices (%i vertex sampling steps)", NodeAnzLow, NodeAnzHigh, NodeStep));
        if (Random) W->message(string("        - %i to %i initial edges (%i edge sampling steps)", EdgeAnzLow, EdgeAnzHigh, EdgeStep));
        if (Acyclic) W->message("        - Acyclic");
        if (Simple) W->message("        - Simple");
        if (DgrOn) W->message(string("        - Bounded Degree %i", Dgr));
        W->message("    - Sequences of operations:");
        W->message(string("        - %i operations (%i percent queries)", OpAnz, OpPerc));
        if (UpdType == _mixed_) W->message(string("        - Update operations are mixed (%i percent insert)", UpdPerc));
        else W->message(string("        - Update operations are ")+((UpdType == _insert_) ? "insert" : "remove"));
        if (QryType == _mixed_) W->message(string("        - Queries are mixed (%i percent bool)", QryPerc));
        else W->message(string("        - Queries are ")+((QryType == _bool_qry_) ? "bool" : "path"));
      }
    if (! RunOn) W->message("No result.");
    else
      {
        W->message("Result:");
        int Active = 0;
        for (int a=0; a<AlgAnz; a++) if (AlgOn[a]) Active++;
        if (Active>0)
          {
            W->message("    - Active algorithms:");
            for (int i =0; i < AlgAnz; i++)
                if (AlgOn[i]) W->message("        - "+AlgName[i]);
          }
        else W->message("    - No active algorithms.");
        if (CheckOn) W->message("    - Correctness check is on");
        W->message(string("    - Number of iterations is %i", IterAnz));
      }
  }

#endif _DTC_FUNCTIONS_H

