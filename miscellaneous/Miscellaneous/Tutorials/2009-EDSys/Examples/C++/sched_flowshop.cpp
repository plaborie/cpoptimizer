// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/sched_flowshop.cpp
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
    const char* filename = "../../../examples/data/flowshop_tail20_5_3.data";
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
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    for (IloInt j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbJobs; i++) {
      IloIntervalVar prec;
      for (IloInt j = 0; j < nbMachines; j++) {
	IloInt d;
	file >> d;
	IloIntervalVar ti(env, d);
	machines[j].add(ti);
	if (0 != prec.getImpl())
	  model.add(IloEndBeforeStart(env, prec, ti));
	prec = ti;
      }
      ends.add(IloEndOf(prec));
    }
    for (IloInt j = 0; j < nbMachines; j++)
      model.add(IloNoOverlap(env, machines[j]));

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
