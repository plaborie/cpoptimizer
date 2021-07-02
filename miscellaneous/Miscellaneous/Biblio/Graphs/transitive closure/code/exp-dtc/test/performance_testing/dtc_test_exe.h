// --------------------------------------------------------------------
//
//  File:        dtc_test_exe.h
//  Date:        02/98
//  Last update: 03/99
//  Description: Execute class for tests (graphs and opseqs)
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------


#ifndef _DTC_TEST_EXE_H
#define _DTC_TEST_EXE_H

#include <LEP/dynamic_graphs/dtc_italiano.h>
#include <LEP/dynamic_graphs/dtc_italiano_nr.h>
#include <LEP/dynamic_graphs/dtc_italiano_opt.h>
#include <LEP/dynamic_graphs/dtc_italiano_gen.h>
#include <LEP/dynamic_graphs/dtc_hk_1.h>
#include <LEP/dynamic_graphs/dtc_hk_2.h>
#include <LEP/dynamic_graphs/dtc_yellin.h>
#include <LEP/dynamic_graphs/dtc_dfs.h>
#include <LEP/dynamic_graphs/dtc_dfs_nr.h>
#include <LEP/dynamic_graphs/dtc_bfs.h>
#include <LEP/dynamic_graphs/dtc_dbfs.h>
#include <LEP/dynamic_graphs/dtc_cfnp.h>
#include <LEP/dynamic_graphs/dtc_cfnp_turbo.h>

#include "dtc_test.h"
#include "dtc_result.h"
#include <LEDA/graph_alg.h>

extern dtc_result Result;

/******************** TIME FUNCTIONS ********************/

#include<sys/resource.h>

#define TIME_FKT used_time	/* use LEDA functions to meassure elapsed time */
//#define TIME_FKT get_time	/* use functions defined below (using getrusage) to meassure time */

//float get_time()
//  {
//    rusage tm1;
//    getrusage(RUSAGE_SELF, &tm1);
//    return (tm1.ru_utime.tv_sec+tm1.ru_utime.tv_usec/1E6);
//  }

//float get_time(float &t2)
//  {
//    float t1 = t2; 
//    rusage tm1;
//    getrusage(RUSAGE_SELF, &tm1);
//    t2 = tm1.ru_utime.tv_sec+tm1.ru_utime.tv_usec/1E6;
//    return (t2-t1);
//  }

/******************** Class DTC_Test_Exe ********************/

class dtc_test_exe : public dtc_test
  {
    private:
      dga_base* Alg;

      bool run_opseq(int);

    public:
      bool run(window&);
  };

bool dtc_test_exe::run(window& WTest)
  {
    system("rm -r -f "+SaveDir+"/"+SessionDir+"/"+ResultDir);
    system("mkdir -p "+SaveDir+"/"+SessionDir+"/"+ResultDir);

    window W(400, 350);
    W.text_item("\\bf\\blue Run \\rm\\black\\c4");
    W.display(WTest, window::center, window::center);
    cout<<"\nRuning test sequence:\n";
    
    string Test, Path;
    FILE* File;
    list<string> TestList;
    get_dir_content(SaveDir+"/"+SessionDir+"/"+TestDir, TestList);
    TestList.sort(&compare_result);
    forall (Test, TestList)
      {
        Path = SaveDir+"/"+SessionDir+"/"+TestDir+"/"+Test;
        File = fopen(Path, "r");
        load(File);
        fclose(File);
        cout<<"    - Initial graph with "<<NodeAnz<<" nodes and "<<EdgeAnz<<" edges.\n";
        W.message(string("Initial graph with %i nodes and %i edges.", NodeAnz, EdgeAnz));
        Result.clear();
        int Counter = 0;
        for (int a=0; a<AlgAnz; a++)
            if (AlgOn[a])
              {
                W.message("    - Executing algorithm "+AlgName[a]+".");
                for (int i=0; i<IterAnz; i++)
                  {
                    File = fopen(Path, "r");
                    while (load(File)) { run_opseq(a); Counter++; }
                    fclose(File);
                  }
              }
        Result.average(Counter);
        Result.save();
        W.del_messages();
      }
      
    cout<<"Done.\n";
    W.close();
    return true;
  }

bool dtc_test_exe::run_opseq(int AlgNum)
  {
    if (! init_graph()) return false;
    
    msg_graph EmptyGraph;      /* required for initializing Yellin's algorithm */
    list<node> Path;           /* required for search-path queries */
    int OpCounter = 0, StepCounter = 0, NextStep = 0;
    float Time, TimeStart = TIME_FKT();

    switch(AlgNum)
      {
        case 0:  Alg = new dtc_dfs(&Graph); break;
        case 1:  Alg = new dtc_dfs_nr(&Graph); break;
        case 2:  Alg = new dtc_bfs(&Graph); break;
        case 3:  Alg = new dtc_dbfs(&Graph); break;
        case 4:  Alg = new dtc_italiano(&Graph); break;
        case 5:  Alg = new dtc_italiano_nr(&Graph); break;
        case 6:  Alg = new dtc_italiano_opt(&Graph); break;
        case 7:  Alg = new dtc_italiano_gen(&Graph); break;
        case 8:  Alg = new dtc_hk_1(&Graph); break;
        case 9:  Alg = new dtc_hk_2(&Graph); break;
        case 10: Alg = new dtc_yellin(EmptyGraph);
                 ((dtc_yellin*) Alg)->set_SG((QryType == _bool_qry_) ? false : true);
                 ((dtc_yellin*) Alg)->init(&Graph); break;
                 /* Yellin's algorithm can be set to handle boolean queries only */
        case 11: Alg = new dtc_cfnp(&Graph); break;
        case 12: Alg = new dtc_cfnp_turbo(&Graph); break;
      }
    
    op_cls* o;
    forall(o, OpSeq)
      {
        if (OpCounter >= NextStep)
          {
            Time = TimeStart;
            Result.set(AlgNum, StepCounter, TIME_FKT(Time), OpCounter);
            if (NextStep >= LargeStep) NextStep += LargeStep;
            else NextStep = (NextStep+SmallStep > LargeStep) ? LargeStep : NextStep+SmallStep;
            StepCounter++;
          }
        OpCounter++;
        switch (o->Type)
          {
            case _insert_:   Graph.new_edge(ItoN[o->Source], ItoN[o->Target]); break;
            case _remove_:   Graph.del_edge(find_edge(ItoN[o->Source], ItoN[o->Target])); break;
            case _bool_qry_: Alg->query(ItoN[o->Source], ItoN[o->Target]); break;
            case _path_qry_: switch(AlgNum)
                {
                  case 0:  ((dtc_dfs*) 		Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 1:  ((dtc_dfs_nr*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 2:  ((dtc_bfs*) 		Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 3:  ((dtc_dbfs*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 4:  ((dtc_italiano*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 5:  ((dtc_italiano_nr*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 6:  ((dtc_italiano_opt*) Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 7:  ((dtc_italiano_gen*) Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 8:  ((dtc_hk_1*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 9:  ((dtc_hk_2*) 	Alg)->path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 10: ((dtc_yellin*) 	Alg)->find_path(ItoN[o->Source], ItoN[o->Target], Path); break;
                  case 11: Alg->query(ItoN[o->Source], ItoN[o->Target]); break;		/* CFNP supports boolean queries only */
                  case 12: Alg->query(ItoN[o->Source], ItoN[o->Target]); break;		/* CFNP supports boolean queries only */
                }
              break;
          }
      }
    Result.set(AlgNum, StepCounter, TIME_FKT(TimeStart), OpCounter);
    
    if (CheckOn) 
      {
        node n1, n2;
        int Error = 0;
        forall_nodes (n1, Graph)
          {
            node_array<bool> Reached(Graph, false);
            DFS(Graph, n1, Reached);
            forall_nodes(n2, Graph)
                if (Alg->query(n1, n2) != Reached[n2]) Error++;
          }
        Result.set(AlgNum, (float) (Error*100)/(Graph.number_of_nodes()*Graph.number_of_nodes()));
      }
    
    delete Alg;
    return true;
  }

#endif _DTC_TEST_EXE_H
