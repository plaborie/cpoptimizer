// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/sched_rcpsp.cpp
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/rcpsp_j120_1_2.data";
    IloInt failLimit = 10000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    IloInt nbTasks, nbResources, i, j;
    file >> nbTasks;
    file >> nbResources;
    IloCumulFunctionExprArray resources(env, nbResources);
    IloIntArray capacities(env, nbResources);
    for (j=0; j<nbResources; j++) {
      IloInt c;
      file >> c;
      capacities[j] = c;
      resources[j] = IloCumulFunctionExpr(env);
    }
    IloIntervalVarArray tasks(env, nbTasks);
    for (i=0; i<nbTasks; i++) {
      tasks[i] = IloIntervalVar(env);
    }
    IloIntExprArray ends(env);
    for (i=0; i<nbTasks; i++) {
      IloIntervalVar task = tasks[i];
      IloInt d, nbSucc;
      file >> d;
      task.setSizeMin(d);
      task.setSizeMax(d);
      ends.add(IloEndOf(task));
      for (j = 0; j < nbResources; j++) {
	IloInt q;
	file >> q;
	if (q > 0) {
	  resources[j] += IloPulse(task, q);
	}
      }
      file >> nbSucc;
      for (IloInt s=0; s<nbSucc; ++s) {
	IloInt succ;
	file >> succ;
	model.add(IloEndBeforeStart(env, task, tasks[succ-1]));
      }
    }
    
    for (j = 0; j < nbResources; j++) {
      model.add(resources[j] <= capacities[j]);
    }

    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);
    
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
      cp.out() << "Makespan \t: " << cp.getValue(objective) << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
    cp.printInformation();

  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}
