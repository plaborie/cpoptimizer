// -------------------------------------------------------------- -*- C++ -*-
// File: tdtsp.cpp
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2013
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------


#include <../team/TDTSP/src/ttime.cpp>

#include <../team/TDTSP/src/tdnooverlapct.cpp>

//lancer les tests de la nouvelle contrainte

int main() {

  IloInt n=5; // Nb visits
  IloInt m=5;  // Nb time steps
  IloInt w=10; //  Size of time step
  IloEnv env;
  try {
  IloModel model(env);
  IloIntervalVarArray visits(env, n);
  IloIntArray positions(env, n);
  IloInt i,j,k;
  IloIntArray3 td = IloIntArray3(env, n); // Time-dependent transition times
  IloIntVar distance(env, 0, 1000);
  char name[32];
  srand((int) time(NULL));
  for (i=0; i<n; ++i) {
    td[i] = IloIntArray2(env, n);
    sprintf(name, "V%ld", i);
    visits[i] = IloIntervalVar(env, 1);
    visits[i].setName(name);
    positions[i] = i;
    for (j=0; j<n; ++j) {
      td[i][j] = IloIntArray(env, m);
      for (k=0; k<m; ++k) {
        if(i==j) td[i][j][k] = 100;
        else td[i][j][k] = 10 + rand()%6;
      }
    }
  }
  IloIntervalSequenceVar route(env, visits, positions);
  model.add(IloTDNoOverlap(env, route, distance, td, w, 0));
  IloCP cp(model);
  cp.setParameter(IloCP::Workers, 1);
  IloSearchPhaseArray phases(env);
  IloIntervalSequenceVarArray routes(env);
  routes.add(route);
  phases.add(IloSearchPhase(env, routes));
  cp.solve(phases);
  std::cout << cp.getValue(distance) << std::endl;
  } catch (IloException& exc) {
    std::cout << exc << std::endl;
  }
  env.end();
  return 0;
}