#include <ilsched/iloscheduler.h>
#include <ilsched/cumlsrs.h>

ILOSTLBEGIN

#if defined(IL_STD)
#include <fstream>
#include <strstream>
#include <iostream>
#else
#if !defined(_WINDOWS)
#include <strstream.h>
#else
#include <strstrea.h>
#endif
#include <fstream.h>
#include <string.h>
#endif

//#define VERBOSE
//#define USE_Z_BIAS
#define MINIMIZE_MCS
#define USE_SHAVING
//#define FULL_SHAVING
//#define VERBOSE_SHAVING

#include "mcs_search.cpp"

IloInt NumberOfJobs;
IloInt NumberOfResources;
IloInt Factor;
IloInt UB;

// Copied from psched_main.cpp. Could be shared by multiple products
void getFileNameWithoutPath(const char* completeName, char* fileName, char separator) {
  int nNameLen = (completeName ? strlen (completeName) : 0), nLen(0);
  if (nNameLen == 0) {
    strcpy (fileName, "");
    return;
  }
  // get the instance name without path
  while (nNameLen > nLen && completeName[nNameLen-nLen-1] != separator)
    nLen++;
  memcpy (fileName, completeName + nNameLen - nLen, nLen);
  strcpy (fileName + nLen, "\0");
}

///////////////////////////////////////////////////////////////////////////////
// READ FILE
///////////////////////////////////////////////////////////////////////////////

IloInt NbJobs;
IloInt NbResources;

IloModel ReadOpenShop(char* instanceDir,
		      char* instanceName, IloIntVar& makespan,
		      IloSchedulerSolution& solution,
		      IloActivity**& acts) {
  char name[128];
  sprintf(name, "%s%s", instanceDir, instanceName);
#if defined(VERBOSE)
  cout << "READING " << name << endl;
#endif

  IloEnv env;
  IloSchedulerEnv schedEnv(env);
  solution = IloSchedulerSolution(env);
  schedEnv.getResourceParam().setCapacityEnforcement(IloExtended);
  schedEnv.getResourceParam().setPrecedenceEnforcement(IloExtended);

  IloModel model(env);
  IloUnaryResource* resources;
  IloUnaryResource* jobs;

  IloInt sumDur = 0;
  ifstream file(name);
  const int lineLimit  = 1024;
  char buffer[lineLimit];

  IloInt d, i,j, nbJobs, nbResources;
  IloBool readActivity = IloTrue;
  IloInt nbActs = 0;
  IloInt nbRess = 0;
  IloInt nbPrec = 0;
  file.getline(buffer, lineLimit);
  file.getline(buffer, lineLimit);
  istrstream line1(buffer, strlen(buffer));
  line1 >> nbJobs;
  jobs = new IloUnaryResource[nbJobs];
  for (i=0; i<nbJobs; ++i)
    jobs[i]=IloUnaryResource(env);

  file.getline(buffer, lineLimit);
  istrstream line2(buffer, strlen(buffer));
  line2 >> nbResources;
  resources = new IloUnaryResource[nbResources];
  for (i=0; i<nbResources; ++i) 
    resources[i]=IloUnaryResource(env);

  NbJobs = nbJobs;
  NbResources = nbResources;

  acts = new (env) IloActivity*[nbJobs];
  for (i=0; i<nbJobs; ++i) 
    acts[i] = new (env) IloActivity[nbResources];
  char buffer2[128];

  makespan = IloIntVar(env, 0, IloIntMax);
  for (i=0; i<nbJobs; ++i) {
    file.getline(buffer, lineLimit);
    istrstream line(buffer, strlen(buffer));
    for (j=0; j<nbResources; ++j) {
      line >> d;
      sumDur += d;
      IloActivity act(env, d);
      sprintf(buffer2, "J%d_%d", i+1, j+1);
      act.setName(buffer2);
      solution.add(act);
      acts[i][j]=act;
      model.add(act.requires(resources[j]));
      model.add(act.requires(jobs[i]));
      model.add(act.endsBefore(makespan));
    }
  }
  makespan.setUb(sumDur);
  solution.add(makespan);
  return model;
}

///////////////////////////////////////////////////////////////////////////////
// DEFINE MODEL
///////////////////////////////////////////////////////////////////////////////

#ifdef INCRORDERING
ILOCPCONSTRAINTWRAPPER1(IloLSRSConstraint, solver, IloDiscreteResource, res) {
  use(solver, res);
  IlcScheduler sched(solver);
  return IlcMakeCumulativeLSRSCt(solver, sched.getDiscreteResource(res));
}
#endif /* INCRORDERING */


void WriteSolution(const char* instanceFile, 
		   IloSchedulerSolution sol, 
		   IloIntVar makespan, 
		   IloActivity** acts) {
  char filename[128];
  sprintf(filename, "Solutions/%s.opt", instanceFile);
  ofstream stream(filename);

  stream << "Instance Name   :" << instanceFile << endl;
  stream << "Makespan        :" << sol.getMin(makespan) << endl;
  stream << "------------------------------------------------------------------" << endl;
  stream << "Solution" << endl;
  stream << "------------------------------------------------------------------" << endl;

  for (IloInt i=0; i<NbJobs; ++i) {
    for (IloInt j=0; j<NbResources; ++j) {
      IloActivity act = acts[i][j];
      stream << sol.getStartMin(act) << "\t";
    }
    stream << endl;
  }
  stream << "==================================================================" << endl;
}

///////////////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
///////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
#if defined(USE_SHAVING)
  InitShavingHistory();
#endif

  IloInt    factor = 1;
  IloInt    numberOfJobs, numberOfResources;
  IloInt*   numberOfActivities;
  IloInt*** resourceAssignments;
  IloInt**  durations;
  IloInt**  precedences;
  IloInt*   resourceCapacity;
  char*     instanceDir  = "data/ossp/";
  char*     instanceFile = ""; 
  IlcFloat  timeLimit = 1800.0;
  IloInt    lb = 0;
  IloInt    ub = IloIntMax;

  IlcInt argNr = 1;
  while (argNr < argc) {
    if (!strcmp(argv[argNr], "-instance")) {
      argNr++;
      if (argNr < argc) {
	instanceFile = argv[argNr];
      }
    } else if (!strcmp(argv[argNr], "-lb")) {
      argNr++;
      if (argNr < argc) {
	lb = atoi(argv[argNr]);
      }
    }    
     else if (!strcmp(argv[argNr], "-ub")) {
      argNr++;
      if (argNr < argc) {
	ub = atoi(argv[argNr]);
      }
    }    
    else if (!strcmp(argv[argNr], "-time")) {
      argNr++;
      if (argNr < argc) {
	timeLimit = atoi(argv[argNr]);
      }
    }
    argNr++;
  }

  IloIntVar makespan;
  IloSchedulerSolution sol;
  IloActivity** acts;
#if defined(VERBOSE)
  cout << "Before reading" << endl;
#endif
  IloModel model = ReadOpenShop(instanceDir, instanceFile, makespan,
				sol, acts);
  IloEnv env = model.getEnv();
#if defined(VERBOSE)
  cout << "After reading nbJobs=" <<  NbJobs << "\tnbResources=" << NbResources << endl;
#endif
  IlcBool solution;
  IlcFloat time;
  
  // FOR ACTUAL TIME
  IlcFloat t0 = env.getTime();
  
  IlcInt nbFails;
  IlcBool ok;  
#if defined(USE_SHAVING)
  ShavingThreshold    = 1;
#endif
  IlcInt v = lb;
  for (; ; ++v) {
    ok = IloFalse;
    if (v == ub)
      break;
    ok = SolveProblem(model, makespan, sol, v, timeLimit, solution, time, nbFails);
    if (solution)
      break;
  }
  cout << instanceFile << "\t" << v << "\t";
  if (!ok && (v == ub))
    cout << "-";
  else if (ok) {
    cout << "*";
    WriteSolution(instanceFile, sol, makespan, acts);
  }
  
  cout << "\t" << env.getTime() - t0 << endl;
  env.end();
  
}






