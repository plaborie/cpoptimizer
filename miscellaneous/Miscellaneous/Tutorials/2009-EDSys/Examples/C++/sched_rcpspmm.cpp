// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/sched_rcpspmm.cpp
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
    const char* filename = "../../../examples/data/rcpspmm_j30_12_8.data";
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

    IloInt i, j, k;
    IloModel model(env);
    IloInt nbTasks, nbRenewable, nbNonRenewable;
    file >> nbTasks >> nbRenewable >> nbNonRenewable;
    IloCumulFunctionExprArray renewables(env, nbRenewable);
    IloIntExprArray nonRenewables(env, nbNonRenewable);
    IloIntArray capRenewables   (env, nbRenewable);
    IloIntArray capNonRenewables(env, nbNonRenewable);
    for (j=0; j<nbRenewable; j++) {
      renewables[j] = IloCumulFunctionExpr(env);
      file >> capRenewables[j];
    }
    for (j=0; j<nbNonRenewable; j++){
      nonRenewables[j] = IloIntExpr(env);
      file >> capNonRenewables[j];
    }
  
    IloIntervalVarArray  tasks(env, nbTasks);
    IloIntervalVarArray2 modes(env, nbTasks);
    for (i=0; i<nbTasks; i++) {
      tasks[i] = IloIntervalVar(env);
      modes[i] = IloIntervalVarArray(env);
    }
    IloIntExprArray ends(env);
    for (i=0; i<nbTasks; i++) {
      IloIntervalVar task = tasks[i];
      IloInt d, nbModes, nbSucc;
      file >> d >> nbModes >> nbSucc;
      for (k=0; k<nbModes; ++k) {
	IloIntervalVar alt(env);
	alt.setOptional();
	modes[i].add(alt);
      }
      model.add(IloAlternative(env, task, modes[i]));
      ends.add(IloEndOf(task));
      for (IloInt s=0; s<nbSucc; ++s) {
	IloInt succ;
	file >> succ;
	model.add(IloEndBeforeStart(env, task, tasks[succ]));
      }
    }
    for (i=0; i<nbTasks; i++) {
      IloIntervalVar task = tasks[i];
      IloIntervalVarArray imodes = modes[i];
      for (k=0; k<imodes.getSize(); ++k) {
	IloInt taskId, modeId, d;
	file >> taskId >> modeId >> d;
	imodes[k].setSizeMin(d);
	imodes[k].setSizeMax(d);
	IloInt q;
	for (j = 0; j < nbRenewable; j++) {
	  file >> q;
	  if (0 < q) {
	    renewables[j] += IloPulse(imodes[k], q);
	  }
	}
	for (j = 0; j < nbNonRenewable; j++) {
	  file >> q;
	  if (0 < q) {
	    nonRenewables[j] += q * IloPresenceOf(env, imodes[k]);
	  }
	}
      }
    }
    
    for (j = 0; j < nbRenewable; j++) {
      model.add(renewables[j] <= capRenewables[j]);
    }

    for (j = 0; j < nbNonRenewable; j++) {
      model.add(nonRenewables[j] <= capNonRenewables[j]);
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
