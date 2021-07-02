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
  IloInt capMax  = 306;

  IloEnv env;
  IloIntVar peak(env, 0, capMax);
  IloCumulFunctionExpr level(env);
  IloIntExpr nbPresent(env);
  for(IloInt i = 0; i < n; ++i) {
    IloIntervalVar a(env, i);
    a.setOptional();
    a.setEndMax(horizon);
    level += IloPulse(a, n-i);
    nbPresent += IloPresenceOf(env, a);
  }
  IloModel model(env);
  model.add(IloMaximize(env, nbPresent));
  model.add(level <= capMax);
  IloCP cp(model);
  cp.setParameter(IloCP::TimeLimit, 15);
  cp.solve();
  env.end();
  return 0;
}

