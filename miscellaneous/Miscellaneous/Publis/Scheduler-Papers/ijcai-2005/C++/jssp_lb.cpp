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
#define USE_Z_BIAS
#define MINIMIZE_MCS
#define USE_SHAVING
//#define FULL_SHAVING
//#define VERBOSE_SHAVING

#include "mcs_search.cpp"

IloInt NumberOfJobs;
IloInt NumberOfResources;
IloInt Factor;
IloInt UB;

///////////////////////////////////////////////////////////////////////////////
// READ FILE
///////////////////////////////////////////////////////////////////////////////

IloInt NbJobs;
IloInt NbResources;

void
ReadJSSP(char* instanceDir,
	 char* instanceName,
	 IloInt& numberOfJobs,
	 IloInt& numberOfResources,
	 IloInt*& numberOfActivities,    // per job
	 IloInt***& resourceAssignments, // per job / activity / resource
	 IloInt**& durations,            // per job / activity
	 IloInt**& precedences,          // per job / job
	 IloInt*& resourceCapacity       // per resource
	 )
{
  char name[128];
  sprintf(name, "%s%s", instanceDir, instanceName);
#if defined(VERBOSE)
  cout << "READING " << name << endl;
#endif
  IloInt a, j, r;
  ifstream file(name);
  const int lineLimit = 1024;
  char buffer[lineLimit];
  file.getline(buffer, lineLimit);
  istrstream line(buffer, strlen(buffer));
  line >> numberOfJobs;
  line >> numberOfResources;
  NbJobs = numberOfJobs;
  NbResources = numberOfResources;
  numberOfActivities = new IloInt[numberOfJobs];
  for (j=0; j<numberOfJobs; ++j) {
    numberOfActivities[j] = numberOfResources;
  }
  resourceAssignments = new IloInt**[numberOfJobs];
  durations = new IloInt*[numberOfJobs];
  for (j=0; j<numberOfJobs; ++j) {
    IloInt nbActivities = numberOfActivities[j];
    assert (nbActivities > 0);
    resourceAssignments[j] = new IloInt*[nbActivities];
    durations[j] = new IloInt[nbActivities];
    for (a=0; a<nbActivities; ++a) {
      resourceAssignments[j][a] = new IloInt[numberOfResources];
      durations[j][a] = 0;
      for (r=0; r<numberOfResources; ++r)
	resourceAssignments[j][a][r] = 0;
    }
  }

  for (j=0; j<numberOfJobs; ++j) {
    file.getline(buffer, lineLimit);
    istrstream jobline(buffer, strlen(buffer));
    IloInt nbActivities = numberOfActivities[j];
    for (a=0; a<nbActivities; ++a) {
      IloInt resource, duration;
      jobline >> resource;
      jobline >> duration;
      resourceAssignments[j][a][resource] = 1;
      durations[j][a] = duration;
    }
  }
  
  resourceCapacity = new IloInt[numberOfResources];
  for (r=0; r<numberOfResources; ++r) 
    resourceCapacity[r] = 1;

  IloInt j1=0, j2=0;
  precedences = new IloInt*[numberOfJobs];
  for (j1=0; j1<numberOfJobs; ++j1) {
    precedences[j1] = new IloInt[numberOfJobs];
    for (j2=0; j2<numberOfJobs; ++j2)
      precedences[j1][j2] = IloFalse;
  }
}


///////////////////////////////////////////////////////////////////////////////
// READ FILE
///////////////////////////////////////////////////////////////////////////////

void
InitParameters(char* instanceName,
	       IloInt*& resourceNumbers,
	       IloInt*& durations) {
  cout << "---------------------------------------------------------" << endl;
  cout << "Problem " << instanceName << endl;
  ifstream file(instanceName);
  const int lineLimit = 1024;
  char buffer[lineLimit];
  file.getline(buffer, lineLimit);
  istrstream line(buffer, strlen(buffer));
  line >> NumberOfJobs;
  line >> NumberOfResources;
  resourceNumbers = new IloInt[NumberOfJobs * NumberOfResources];
  durations = new IloInt[NumberOfJobs * NumberOfResources];
  IloInt k = 0;
  for (IloInt i = 0; i < NumberOfJobs; i++) {
    file.getline(buffer, lineLimit);
    istrstream jobline(buffer, strlen(buffer));
    for (IloInt j = 0; j < NumberOfResources; j++) {
      jobline >> resourceNumbers[k];
      jobline >> durations[k];
      k++;
    }
  }
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

IloModel StateModel(IloInt numberOfJobs,
		    IloInt numberOfResources,
		    IloInt* numberOfActivities,    // per job
		    IloInt*** resourceAssignments, // per job / activity / resource
		    IloInt** durations,            // per job / activity
		    IloInt** precedences,        // per job / job
		    IloInt* resourceCapacity,      // per resource
		    IloInt factor,
		    IloIntVar& makespan,
		    IloActivity***& acts, IloSchedulerSolution& sol) {
  IloEnv env;
  IloModel model(env);
  sol = IloSchedulerSolution(env);

  /* CREATE THE MAKESPAN VARIABLE. */
  IloInt horizon = 0;
  IloInt j=0, a=0, k=0, r=0;
  for (j=0; j<numberOfJobs; ++j) {
    IloInt nbActivities = numberOfActivities[j];
    for (a=0; a<nbActivities; ++a) {
      horizon += durations[j][a];
    }
  }

  makespan = IloIntVar(env, 0, horizon);

  acts = new (env) IloActivity**[factor];
  for (k=0; k<factor; ++k) {
    acts[k] = new (env) IloActivity*[numberOfJobs];
    for (j=0; j<numberOfJobs; ++j) {
      acts[k][j] = new (env) IloActivity[numberOfResources];
    }
  }

  IloSchedulerEnv schedEnv(env);
  schedEnv.setHorizon(horizon);
  schedEnv.getResourceParam().setCapacityEnforcement(IloExtended);
  schedEnv.getResourceParam().setPrecedenceEnforcement(IloExtended);
  IloArray<IloDiscreteResource> resources(env, numberOfResources);
  char buffer[128];

  for (r = 0; r < numberOfResources; r++) {
    resources[r] = IloDiscreteResource(env, factor*resourceCapacity[r]);
    sprintf(buffer, "Resource %d", r);
    resources[r].setName(buffer);
  }

  IloArray<IloActivityArray> jobs(env, numberOfJobs*factor);
  for (j=0; j<jobs.getSize(); ++j)
    jobs[j] = IloActivityArray(env, numberOfResources);

  IloActivity* previousActs =
    new (env) IloActivity[factor+1];
  
  IloInt z;
  for (j = 0; j < numberOfJobs; j++) {
    IloInt nbActivities = numberOfActivities[j];
    for (a = 0; a < nbActivities; a++) {
      IloActivity actNrMinus1;
      for (z = 1; z <= factor; z++) {
	IloActivityArray job = jobs[j+(z-1)*numberOfJobs];
	IloActivity activity(env, durations[j][a]);
	strcpy(buffer, "");
	for (r=0; r<numberOfResources; r++) {
	  IloInt capacity = resourceAssignments[j][a][r];
	  if (capacity > 0) {
	    sprintf(buffer, "J%dS%dR%dnr%d", j, a, r, z);
	    break;
	  }
	}
	activity.setName(buffer);
 	activity.setObject((IloAny)(z-1));
	acts[z-1][j][a]=activity;
	sol.add(activity);
	job[a] = activity;
	for (r=0; r<numberOfResources; r++) {
	  IloResourceConstraint rct;
	  IloInt capacity = resourceAssignments[j][a][r];
	  if (capacity > 0) {
	    rct = activity.requires(resources[r], capacity);
	    model.add(rct);
	  }
	}
	if (a != 0)
	  model.add(activity.startsAfterEnd(previousActs[z]));
	previousActs[z] = activity;
	// Symmetry breaking constraint
	if (z != 1) {
  	  model.add(activity.startsAfterStart(actNrMinus1));
  	  model.add(activity.endsAfterEnd(actNrMinus1));
	}
	actNrMinus1 = activity;
      }
    }
    for (z = 1; z <= factor; z++) {
      model.add(previousActs[z].endsBefore(makespan));
    }
  }
  
  IloInt j1 = 0, j2 = 0;
  
  for (j1=0; j1<numberOfJobs; ++j1) {
    IloActivity lastAct = jobs[j1][numberOfActivities[j1]-1];	
    for (j2=0; j2<numberOfJobs; ++j2) {
      if (j2 == j1) continue;
      IloActivity firstAct = jobs[j2][0];
      if (precedences[j1][j2] == IloTrue) {
	model.add(firstAct.startsAfterEnd(lastAct));
      }
    }
  }
  
#ifdef INCRORDERING  
  for (j = 0; j < numberOfResources; j++)
    model.add(IloLSRSConstraint(env, resources[j]));
#endif /* INCRORDERING */
  sol.add(makespan);
  return model;
}

void WriteSolution(const char* instanceFile, 
		   IloSchedulerSolution sol, 
		   IloIntVar makespan, 
		   IloActivity*** acts) {
  char filename[128];
  sprintf(filename, "Solutions/%s_%d.opt", instanceFile, Factor);
  ofstream stream(filename);

  stream << "Instance Name   :" << instanceFile << endl;
  stream << "Factor          :" << Factor << endl;
  stream << "Makespan        :" << sol.getMin(makespan) << endl;
  stream << "------------------------------------------------------------------" << endl;
  stream << "Solution" << endl;
  stream << "------------------------------------------------------------------" << endl;

  for (IloInt k=0; k<Factor; ++k) {
    for (IloInt i=0; i<NbJobs; ++i) {
      for (IloInt j=0; j<NbResources; ++j) {
	IloActivity act = acts[k][i][j];
	stream << sol.getStartMin(act) << "\t";
      }
      stream << endl;
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
  char*     instanceDir  = "data/jssp/";
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
    else if (!strcmp(argv[argNr], "-factor")) {
      argNr++;
      if (argNr < argc) {
	factor = atoi(argv[argNr]);
      }
    }
    argNr++;
  }
  Factor = factor;

  IloIntVar makespan;
  IloSchedulerSolution sol;
#if defined(VERBOSE)
  cout << "Before reading" << endl;
#endif
  
  ReadJSSP(instanceDir, instanceFile,
	   numberOfJobs,
	   numberOfResources,
	   numberOfActivities,
	   resourceAssignments,
	   durations,
	   precedences,
	   resourceCapacity);

  IloActivity*** acts;
  IloModel model = StateModel(numberOfJobs, 
			      numberOfResources, 
			      numberOfActivities, 
			      resourceAssignments, 
			      durations, 
			      precedences, 
			      resourceCapacity,
			      factor,
			      makespan, acts, sol);
  
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






