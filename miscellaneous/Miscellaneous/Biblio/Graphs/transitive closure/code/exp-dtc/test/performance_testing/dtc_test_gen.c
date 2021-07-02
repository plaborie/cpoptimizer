// --------------------------------------------------------------------
//
//  File:        dtc_test_gen.c
//  Date:        02/98
//  Last update: 03/99
//  Description: Generator class for tests (graphs and opseqs)
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#include "dtc_test_gen.h"

/******************** Class DTC_Test_Gen ********************/

void dtc_test_gen::save()
  {
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+TestDir+string("/test-%i_%i", NodeAnz, EdgeAnz), "a");
    save_graph(File);
    save_opseq(File);
    fclose(File);
  }

bool dtc_test_gen::gen_error_panel()
  {
    panel P("Error");
    P.text_item(string("Unable to find an appropriate edge to insert or remove. Maybe trying to insert too many")+
        "edges in a simple or bounded degree graph or trying to remove an edge from an empty graph.");
    P.button("Retry", 1);
    P.button("Cancel", 0);
    if (P.open()) return true;
    delete DTC;
    clear();
    return false;
  }

/********** Generate **********/

bool dtc_test_gen::generate(graph& G, window& WTest)
  {
    system("rm -r -f "+SaveDir+"/"+SessionDir);
    system("mkdir -p "+SaveDir+"/"+SessionDir+"/"+TestDir);

    if (QryType == _bool_qry_) QryPerc = 100;
    if (QryType == _path_qry_) QryPerc = 0;
    if (UpdType == _insert_) UpdPerc = 100;
    if (UpdType == _remove_) UpdPerc = 0;

    QryAnz = OpAnz*OpPerc/100;
    UpdAnz = OpAnz-QryAnz;
    BoolQryAnz = QryAnz*QryPerc/100;
    PathQryAnz = QryAnz-BoolQryAnz;
    InsUpdAnz = UpdAnz*UpdPerc/100;
    RemUpdAnz = UpdAnz-InsUpdAnz;

    window W(400, 350);
    W.text_item("\\bf\\blue Generate \\rm\\black\\c4");
    W.display(WTest, window::center, window::center);
    cout<<"\nGenerating test sequence:\n";

    if (LoadGraph)
      {
        cout<<"    - Initial graph with "<<NodeAnz<<" nodes and "<<EdgeAnz<<" edges.\n";
        W.message("Generate random test (load initial graph):");        
        W.message(string("    - Initial graph with %i nodes and %i edges.", NodeAnz, EdgeAnz));
        copy_graph(G);
        if (! gen_random(1)) return false;
      }
    else
      {
        if (Random)
          {
            W.message("Generate random test:");
            for (int i=0; i<NodeStep; i++)
                for (int j=0; j<EdgeStep; j++)
                  {
                    NodeAnz = (NodeStep==1) ? NodeAnzLow : NodeAnzLow+(NodeAnzHigh-NodeAnzLow)*i/(NodeStep-1);
                    EdgeAnz = (EdgeStep==1) ? EdgeAnzLow : EdgeAnzLow+(EdgeAnzHigh-EdgeAnzLow)*j/(EdgeStep-1);
                    cout<<"    - Initial graph with "<<NodeAnz<<" nodes and "<<EdgeAnz<<" edges.\n";
                    W.message(string("    - Initial graph with %i nodes and %i edges.", NodeAnz, EdgeAnz));
                    for (int k=0; k<GraphAnz; k++)
                      {
                        if (! gen_random(0)) return false;
                        for (int l=1; l<OpSeqAnz; l++) if (! gen_random(1)) return false;
                      }
                  }
          }
        else
          {
            W.message("Generate non-random test:");
            for (int i=0; i<NodeStep; i++)
              {
                NodeAnz = (NodeStep==1) ? NodeAnzLow : NodeAnzLow+(NodeAnzHigh-NodeAnzLow)*i/(NodeStep-1);
                cout<<"    - Initial graph (non random) with "<<NodeAnz<<" nodes.\n";
                W.message(string("    - Initial graph with %i nodes.", NodeAnz));
                if (! gen_non_random()) return false;
              }
          }
      }
    
    cout<<"Done.\n";
    W.close();
    return true;
  }


void dtc_test_gen::insert_queries(int Anz, int Type)
  {
    int Random, Max = OpSeq.length();
    while (Anz-- > 0)
        if ((Random = RandomSource(0, Max++)) == 0) 
            OpSeq.push(new op_cls(Type, RandomSource(0, NodeAnz-1), RandomSource(0, NodeAnz-1)));
        else OpSeq.insert(new op_cls(Type, RandomSource(0, NodeAnz-1), RandomSource(0, NodeAnz-1)), OpSeq[Random-1]);
  }

/********** Generate Non Random **********/
bool dtc_test_gen::gen_non_random()
  {
    clear();
    NoEdges = NoOpSeq = false;
    list<int> L1, L2;
    
    for (int Step=0; Step<NodeAnz; Step+=SubNodeAnz)	/* generate initial edges */
      {
        if (Step != 0)
          {
            L1.append(Step);
            if (UpdType == _remove_) Edges.append(new edge_cls(Step-1, Step));
          }
        
        int Anz = (SubNodeAnz < NodeAnz-Step) ? SubNodeAnz : NodeAnz-Step;
        for (int Node1=0; Node1<Anz; Node1++)
          {
            int Start = (Acyclic) ? Node1+1 : 0;
            for (int Node2=Start; Node2<Anz; Node2++)
                if (Node1 != Node2) Edges.append(new edge_cls(Node1+Step, Node2+Step));
          }
      }
    
    EdgeAnz = Edges.length();		/* set parameter */
    if ((EdgeAnzLow>EdgeAnz) || (EdgeAnzLow==0)) EdgeAnzLow = EdgeAnz;
    if (EdgeAnzHigh<EdgeAnz) EdgeAnzHigh = EdgeAnz;
    
    bool TooLong = ((UpdType != _mixed_) && (UpdAnz > L1.length())) ? true : false;
    int TmpUpdAnz = (TooLong) ? L1.length() : UpdAnz;
    int TmpQryAnz = (TooLong) ? TmpUpdAnz*OpPerc/(100-OpPerc) : QryAnz;
    BoolQryAnz = TmpQryAnz*QryPerc/100;
    PathQryAnz = TmpQryAnz-BoolQryAnz;

    while (! L1.empty())		/* generate opseq */
      {
        L2.append(L1.pop());
        if (! L1.empty()) L2.append(L1.pop_back());
      }
    
    list_item it = L2.first();
    int Type = (UpdType == _mixed_) ? _insert_ : UpdType;
    for (int Max=TmpUpdAnz-1; Max>=0; Max--)
      {
        OpSeq.append(new op_cls(Type, L2[it]-1, L2[it]));
        if (it == L2.last()) Type = (Type == _insert_) ? _remove_ : _insert_;
        it = L2.cyclic_succ(it);
      }
    
    insert_queries(BoolQryAnz, _bool_qry_);
    insert_queries(PathQryAnz, _path_qry_);
    
    save();
    return true;
  }

/********** Generate Random **********/
bool dtc_test_gen::gen_random(int Type) /* 0: graph+opseq, 1: opseq only */
  {
    if (Acyclic) DTC = new DTC_ALGORITHM(Graph);
    if (Type == 0)
        while (! gen_random_graph())
            if (! gen_error_panel()) return false;
    while (! gen_random_opseq())
        if (! gen_error_panel()) return false;
    if (Acyclic) delete DTC;
    
    save();
    return true;
  }

bool dtc_test_gen::gen_random_graph()
  {
    clear();
    NoEdges = false;
    init_graph();
    
    node s, t;
    for (int i = 0; i < EdgeAnz; i++)
      {
        bool NotFound = true;
        for (int i=0; (i<NodeAnz) && (NotFound); i++)
            if (gen_random_source(s = Graph.choose_node()))
                for (int j=0; (j<NodeAnz) && (NotFound); j++)
                    if (gen_random_target(s, t = Graph.choose_node())) NotFound = false;
        if (NotFound) return false;
        Edges.append(new edge_cls(NtoI[s], NtoI[t]));
        Graph.new_edge(s, t);
      }
    return true;
  }

bool dtc_test_gen::gen_random_opseq()
  {
    clear_opseq();
    NoOpSeq = false;
    if (! init_graph()) return false;
    
    int Random;
    node s, t;
    edge e;
    int TmpInsUpdAnz = InsUpdAnz;
    for (int Max=UpdAnz-1; Max>=0; Max--)
      {
        Random = RandomSource(0, Max);
        if (Random < TmpInsUpdAnz)	/* Insert operation */
          {
            bool NotFound = true;
            for (int i=0; (i<NodeAnz) && (NotFound); i++)
                if (gen_random_source(s = Graph.choose_node()))
                    for (int j=0; (j<NodeAnz) && (NotFound); j++)
                        if (gen_random_target(s, t = Graph.choose_node())) NotFound = false;
            if (NotFound) return false;
            OpSeq.append(new op_cls(_insert_, NtoI[s], NtoI[t]));
            Graph.new_edge(s, t);
            TmpInsUpdAnz--;
          }
        else				/* Remove operation */
          {
            if ((e = Graph.choose_edge()) == NULL) return false;
            OpSeq.append(new op_cls(_remove_, NtoI[source(e)], NtoI[target(e)]));
            Graph.del_edge(e);
          }
      }

    insert_queries(BoolQryAnz, _bool_qry_);
    insert_queries(PathQryAnz, _path_qry_);
    
    return true;
  }

bool dtc_test_gen::gen_random_source(node &Source)
  {
    if ((DgrOn) && (Graph.degree(Source) >= Dgr)) return false;
    return true;
  }

bool dtc_test_gen::gen_random_target(node &Source, node &Target)
  {
    if ((DgrOn) && (Graph.degree(Target) >= Dgr)) return false;
    if ((DgrOn) && (Source == Target) && (Graph.degree(Target) >= Dgr-1)) return false;
    if ((Acyclic) && (Source == Target)) return false;
    if ((Acyclic) && (DTC->query(Target, Source))) { node tmp = Source; Source = Target; Target = tmp; }
    if ((Simple) && (find_edge(Source, Target) != NULL)) return false;
    return true;
  }
