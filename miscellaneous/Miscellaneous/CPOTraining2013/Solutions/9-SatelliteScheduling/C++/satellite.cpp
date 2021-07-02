// -------------------------------------------------------------- -*- C++ -*-
// File: satellite.cpp
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

/* ------------------------------------------------------------

  Problem Description
  -------------------
  
  This scheduling problem is described in:

  L. Kramer, L. Barbulescu, and S. Smith. Understanding Performance Tradeoffs in Algorithms for Solving Oversubscribed Scheduling. Proc. AAAI-07, July, 2007. 
 
  Note that in this version of the model task priorities are not handled.
*/


#include <ilconcert/ilocsvreader.h>
#include <ilcp/cp.h>

ILOSTLBEGIN

int main(int argc, const char * argv[]) {
  const char* filename = "SatelliteData.csv";
  IloInt timeLimit = 30;
  if (argc > 1)
    filename = argv[1];
  
  IloEnv env;
  IloModel model(env);
  IloCsvReader reader(env, filename, IloTrue);
  
  // Reading stations
  IloCsvTableReader readS = reader.getTableByName("STATION");
  IloInt nbResources = readS.getNumberOfItems();
  std::cout << "Number of stations : " << nbResources << std::endl;
  IloCumulFunctionExprArray resources(env, nbResources);
  IloIntArray capacities(env, nbResources);
  IloInt i=0;
  for (IloCsvTableReader::LineIterator site(readS); site.ok(); ++site) {
    IloCsvLine sline = *site; 
    const char* name = sline.getStringByHeader("name");
    IloInt id  = sline.getIntByHeader("id");
    IloInt cap = sline.getIntByHeader("nbAntennas");
    if (i != (id-1)) {
      cerr << "Station index do not match in data file: " << filename << endl;
      exit(1);
    }
    IloCumulFunctionExpr res(env);
    res.setName(name);
    resources [i] = res;
    capacities[i] = cap; 
	i++;
  }
  
  // Reading alternatives
  IloCsvTableReader readA = reader.getTableByName("ALTERNATIVE");
  IloInt nbAlternatives = readA.getNumberOfItems();
  std::cout << "Number of alternatives : " << nbAlternatives << std::endl;
  IloIntervalVarArray allIntervals(env);
  for (IloCsvTableReader::LineIterator aite(readA); aite.ok(); ++aite) {
    IloCsvLine aline = *aite; 
    const char* name = aline.getStringByHeader("taskName");
    IloInt stationId = aline.getIntByHeader("stationId");
    IloInt startMin = aline.getIntByHeader("startMin");
    IloInt duration = aline.getIntByHeader("duration");
    IloInt endMax = aline.getIntByHeader("endMax");
    IloIntervalVar a(env, duration);
    a.setName(name);
    a.setOptional();
    a.setStartMin(startMin);
    a.setEndMax(endMax);
    allIntervals.add(a);
    resources[stationId-1] += IloPulse(a, 1);
  }

  IloInt n = allIntervals.getSize();
  IloIntExpr nbExecuted(env);
  IloIntervalVarArray alt(env);
  alt.add(allIntervals[0]);
  const char* nmcurr = allIntervals[0].getName();
  for (IloInt i=1; i<n; ++i) {
    const char* nm = allIntervals[i].getName();
    if (strcmp(nmcurr, nm)) {
      // Different names
      if (alt.getSize()==1) {
	      nbExecuted += IloPresenceOf(env, alt[0]);
      } else {
      IloIntervalVar a(env);
      a.setOptional();
      model.add(IloAlternative(env, a, alt));
      nbExecuted += IloPresenceOf(env, a);
      }
      nmcurr = nm;
      alt = IloIntervalVarArray(env);
    }
    alt.add(allIntervals[i]);
  }
  
  if (alt.getSize()==1) {
    nbExecuted += IloPresenceOf(env, alt[0]);
  } else {
    IloIntervalVar a(env);
    a.setOptional();
    model.add(IloAlternative(env, a, alt));
    nbExecuted += IloPresenceOf(env, a);
  }
  
  for (IloInt j=0; j<nbResources; ++j) {
    model.add(resources[j] <= capacities[j]);
  }
  
  model.add(IloMaximize(env, nbExecuted));

  IloCP cp(model);
  cp.setParameter(IloCP::TimeLimit, timeLimit);
  cp.solve();
  env.end();
  
  return 0;
}