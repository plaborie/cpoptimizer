// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/sched_openshop.cpp
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
    const char* filename = "../../../examples/data/openshop_j8-per10-2.data";
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
    IloInt i, j;
    IloIntervalVarArray2 jobs(env, nbJobs);
    IloIntervalVarArray2 machines(env, nbMachines);
    for (i = 0; i < nbJobs; i++)
      jobs[i] = IloIntervalVarArray(env);
    for (j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (i = 0; i < nbJobs; i++) {
      for (j = 0; j < nbMachines; j++) {
	IloInt pt;
	file >> pt;
	IloIntervalVar ti(env, pt);
	jobs[i].add(ti);
	machines[j].add(ti);
	ends.add(IloEndOf(ti));
      }
    }
    for (i = 0; i < nbJobs; i++)
      model.add(IloNoOverlap(env, jobs[i]));
    for (j = 0; j < nbMachines; j++)
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
