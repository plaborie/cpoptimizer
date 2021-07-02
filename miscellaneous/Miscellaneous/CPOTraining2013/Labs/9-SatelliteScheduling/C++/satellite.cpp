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
  // Define arrays of cumul functions and integer capacities
  // ...
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
    // Create cumul function for the current station
    // ...
    // Add cumul function and integer capacity in arrays
    // ...
	  i++;
  }
  
  // Reading alternatives
  IloCsvTableReader readA = reader.getTableByName("ALTERNATIVE");
  IloInt nbAlternatives = readA.getNumberOfItems();
  std::cout << "Number of alternatives : " << nbAlternatives << std::endl;
  // Create an array with all interval variables of alternatives
  // ...
  for (IloCsvTableReader::LineIterator aite(readA); aite.ok(); ++aite) {
    IloCsvLine aline = *aite; 
    const char* name = aline.getStringByHeader("taskName");
    IloInt stationId = aline.getIntByHeader("stationId");
    IloInt startMin = aline.getIntByHeader("startMin");
    IloInt duration = aline.getIntByHeader("duration");
    IloInt endMax = aline.getIntByHeader("endMax");
    // Create interval variable of alternative and add it in array
    // ...
    // Create contribution of alternative to the cumul function of its station
    // ...
  }

  IloInt n = allIntervals.getSize();
  // Create alternative constraints for alternative intervals of the same task
  // Create integer expression representing the number of executed tasks and maximize it in objective function
  // ...
  
  for (IloInt j=0; j<nbResources; ++j) {
    // Post capacity constraint for stations
  }
  
  IloCP cp(model);
  cp.setParameter(IloCP::TimeLimit, timeLimit);
  cp.solve();
  env.end();
  
  return 0;
}