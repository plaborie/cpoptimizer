// --------------------------------------------------------------------
//
//  File:        dtc_test.c
//  Date:        02/98
//  Last update: 03/99
//  Description: Test class consisting of graph and opseq
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#include "dtc_test.h"

/******************** Class DTC_Test ********************/

bool dtc_test::init_graph()
  {
    int i;
    if (NoEdges) return false;
    clear_graph();
    ItoN = new node[NodeAnz];
    for (i=0; i<NodeAnz; i++) ItoN[i] = Graph.new_node();
    NtoI.init(Graph);
    for (i=0; i<NodeAnz; i++) NtoI[ItoN[i]] = i;
    edge_cls* e;
    forall (e, Edges) Graph.new_edge(ItoN[e->Source], ItoN[e->Target]);
    NoGraph = false;
    return true;
  }

bool dtc_test::copy_graph(graph& GraphNew)
  {
    clear();
    NodeAnz = GraphNew.number_of_nodes();
    EdgeAnz = GraphNew.number_of_edges();
    Simple = (int) Is_Simple(GraphNew);
    Acyclic = (int) Is_Acyclic(GraphNew);
    DgrOn = 0;
    NtoI.init(GraphNew);
    int i = 0;
    node n;
    edge e;
    forall_nodes (n, GraphNew) NtoI[n] = i++;
    forall_edges (e, GraphNew) Edges.append(new edge_cls(NtoI[source(e)], NtoI[target(e)]));
    NoEdges = false;
    return true;
  }

void dtc_test::save_graph(FILE* File)
  {
    edge_cls* e;
    fprintf(File, "Graph\nNodeAnz: %i\nEdgeAnz: %i\nAcyclic: %i\nSimple: %i\nDgrOn: %i\nDgr: %i\n", 
        NodeAnz, Edges.length(), Acyclic, Simple, DgrOn, Dgr);
    forall (e, Edges) e->save(File);
  }

void dtc_test::save_opseq(FILE *File)
  {
    op_cls *o;
    fprintf(File, "Operations\nOpAnz: %i\nOpPerc: %i\nQryType: %i\nQryPerc: %i\nUpdType: %i\nUpdPerc: %i\n", 
        OpSeq.length(), OpPerc, QryType, QryPerc, UpdType, UpdPerc);
    forall (o, OpSeq) o->save(File);
  }

bool dtc_test::load_graph(FILE* File)
  {
    clear();
    NoEdges = false;
    if (fscanf(File, "Graph\nNodeAnz: %i\nEdgeAnz: %i\nAcyclic: %i\nSimple: %i\nDgrOn: %i\nDgr: %i\n", 
        &NodeAnz, &EdgeAnz, &Acyclic, &Simple, &DgrOn, &Dgr) != 6) return false;
    for (int i=0; i<EdgeAnz; i++) Edges.append(new edge_cls(File));
    return true;
  }

bool dtc_test::load_opseq(FILE* File)
  {
    clear_opseq();
    NoOpSeq = false;
    if (fscanf(File, "Operations\nOpAnz: %i\nOpPerc: %i\nQryType: %i\nQryPerc: %i\nUpdType: %i\nUpdPerc: %i\n", 
        &OpAnz, &OpPerc, &QryType, &QryPerc, &UpdType, &UpdPerc) != 6) return false;
    for (int i=0; i<OpAnz; i++) OpSeq.append(new op_cls(File));
    return true;
  }

edge dtc_test::find_edge(node n1, node n2)
  {
    edge e;
    forall_out_edges(e, n1)
        if (target(e) == n2) return e;
    return NULL;
  }
