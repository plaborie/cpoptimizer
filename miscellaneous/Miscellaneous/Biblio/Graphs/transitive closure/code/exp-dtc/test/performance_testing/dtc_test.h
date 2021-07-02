// --------------------------------------------------------------------
//
//  File:        dtc_test.h
//  Date:        02/98
//  Last update: 03/99
//  Description: Test class consisting of graph and opseq
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_TEST_H
#define _DTC_TEST_H

#include <LEDA/stream.h>
#include <LEP/dynamic_graphs/msg_graph.h>

/******************** Variables ********************/

#ifndef _OP_TYPE_
#define _OP_TYPE_
enum op_type { _insert_, _remove_, _mixed_, _bool_qry_, _path_qry_ };
#endif  _OP_TYPE_

extern bool GenOn, RunOn;
extern int NodeAnz, NodeAnzLow, NodeAnzHigh, NodeStep;
extern int EdgeAnz, EdgeAnzLow, EdgeAnzHigh, EdgeStep;
extern int Acyclic, Simple, DgrOn, Dgr, SubNodeAnz;
extern int OpAnz, OpPerc, UpdType, UpdPerc, QryType, QryPerc;
extern int LoadGraph, Random, GraphAnz, OpSeqAnz;
extern int SmallStep, LargeStep;
extern const int AlgAnz;
extern int AlgOn[];
extern string GraphFile, SaveDir, SessionDir, TestDir, ResultDir;

/******************** Class DTC_Test ********************/

class dtc_test
  {
    protected:
      struct edge_cls
        {
          int	Source, Target;
          edge_cls(int source, int target) : Source(source), Target(target) {}
          edge_cls(FILE* File) { fscanf(File, "%i %i\n", &Source, &Target); }
          void save(FILE* File) { fprintf(File, "%i %i\n", Source, Target); }
        };
      
      struct op_cls
        {
          int	Type, Source, Target;
          op_cls(int type, int source, int target) : Type(type), Source(source), Target(target) {}
          op_cls(FILE* File) { fscanf(File, "%i: %i %i\n", &Type, &Source, &Target); }
          void save(FILE* File) { fprintf(File, "%i: %i %i\n", Type, Source, Target); }
        };

      list<edge_cls*>	Edges;
      list<op_cls*>	OpSeq;

      msg_graph		Graph;
      node*		ItoN;
      node_array<int>	NtoI;
      bool		NoEdges, NoOpSeq, NoGraph;

      bool init_graph();
      bool copy_graph(graph&);
      void clear_edges() { if (NoEdges) return; edge_cls *e; forall(e, Edges) delete e; Edges.clear(); NoEdges = true; }
      void clear_graph() { if (NoGraph) return; Graph.clear(); delete ItoN; NoGraph = true; }
      void clear_opseq() { if (NoOpSeq) return; op_cls *o; forall(o, OpSeq) delete o; OpSeq.clear(); NoOpSeq = true; }
      bool load_graph(FILE*);
      bool load_opseq(FILE*);
      void save_graph(FILE*);
      void save_opseq(FILE*);
      edge find_edge(node, node);
      
    public:
      dtc_test() : NoEdges(true), NoGraph(true), NoOpSeq(true) {}
      ~dtc_test() { clear(); }
      
      bool load(FILE* File) { clear(); return (load_graph(File) && load_opseq(File)); }
      void save(FILE* File) { save_graph(File); save_opseq(File); }
      void clear() { clear_edges(); clear_graph(); clear_opseq(); }
  };

#endif _DTC_TEST_H
