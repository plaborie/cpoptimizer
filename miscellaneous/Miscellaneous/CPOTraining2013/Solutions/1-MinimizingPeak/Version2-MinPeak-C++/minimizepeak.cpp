// -------------------------------------------------------------- -*- C++ -*-
// File: minimizepeak.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2012. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

#include <ilcp/cp.h>

ILOSTLBEGIN

int main(int, const char *[]) {
  IloInt n       = 100;
  IloInt horizon = 500;
  IloInt capMax  = 500;

  IloEnv env;
  IloIntVar peak(env, 0, capMax);
  // IloIntVarArray phase(env);
  // phase.add(peak);
  IloCumulFunctionExpr level(env);
  for(IloInt i = 0; i < n; ++i) {
    IloIntervalVar a(env, i);
    a.setEndMax(horizon);
    level += IloPulse(a, n-i);
  }
  IloModel model(env);
  model.add(IloMinimize(env, peak));
  model.add(level <= peak);
  IloCP cp(model);
  cp.setParameter(IloCP::TimeLimit, 15);
  // cp.setSearchPhases(IloSearchPhase(env, phase));
  cp.solve();
  env.end();
  return 0;
}

