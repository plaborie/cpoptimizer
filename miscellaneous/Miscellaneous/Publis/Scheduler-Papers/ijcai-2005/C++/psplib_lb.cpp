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

static void skip(ifstream &entree, char* text, int n) {
  for (int i=0 ; i<n ; ++i)
    entree >> text;
}

void ReadPSPLib(char* instanceDir,
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
  char text[2048];
  int  a, j, r, cpt; 
  char name[128];
  sprintf(name, "%s%s", instanceDir, instanceName);
#if defined(VERBOSE)
  cout << "READING " << name << endl;
#endif
  ifstream stream(name, ios::in);
  skip(stream, text, 19);
  // number of jobs (one task per job in PSPLib)
  stream >> numberOfJobs;
  skip(stream, text, 7);
  // number of resources
  stream >> numberOfResources;
  // number of activities per job (one here)
  numberOfActivities = new IloInt[numberOfJobs];
  for (j=0; j<numberOfJobs; ++j) {
    numberOfActivities[j] = 1;
  }
  skip(stream, text, 36);
  // precedences
  precedences = new IlcInt*[numberOfJobs];
  IloInt j1=0, j2=0;
  precedences = new IloInt*[numberOfJobs];
  for (j1=0; j1<numberOfJobs; ++j1) {
    precedences[j1] = new IloInt[numberOfJobs];
    for (j2=0; j2<numberOfJobs; ++j2)
      precedences[j1][j2] = IloFalse;
  }
  cpt = 0;
  for (j1=0 ; j1<numberOfJobs ; ++j1) {
    int nbSucc;
    stream >> nbSucc;
    for (j=0 ; j<nbSucc ; ++j) {
      stream >> j2;
      --j2;
      precedences[j1][j2] = IloTrue;
    }
    skip(stream, text, 2);
  }
  skip(stream, text, 14);
  // activity durations & resource assignments
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
  cpt = 0;
  for (j=0 ; j<numberOfJobs; ++j) {
    IloInt nbActivities = numberOfActivities[j];
    for (a=0; a<nbActivities; ++a) {
      int duration;
      stream >> duration;
      durations[j][a] = duration;
      for (r=0 ; r<numberOfResources ; ++r) {
	int capacity;
	stream >> capacity;
	resourceAssignments[j][a][r] = capacity;
      }
      skip(stream, text, 2);
    }
  }
  skip(stream, text, 8);
  // resource capacity 
  resourceCapacity = new IloInt[numberOfResources];
  for (r=0; r<numberOfResources; ++r) 
    resourceCapacity[r] = 1;
  for (r=0 ; r<numberOfResources; ++r) {
    int capacity;
    stream >> capacity;
    resourceCapacity[r] = capacity;
  }
  stream.close();
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
		    IloSchedulerSolution& solution) {
  IloEnv env;
  IloModel model(env);
 
  /* CREATE THE MAKESPAN VARIABLE. */
  IloInt horizon = 0;
  IloInt j=0, a=0;
  for (j=0; j<numberOfJobs; ++j) {
    IloInt nbActivities = numberOfActivities[j];
    for (a=0; a<nbActivities; ++a) {
      horizon += durations[j][a];
    }
  }

  makespan = IloIntVar(env, 0, horizon);
  solution = IloSchedulerSolution(env);

  IloSchedulerEnv schedEnv(env);
  schedEnv.setHorizon(horizon);
  schedEnv.getResourceParam().setCapacityEnforcement(IloExtended);
  schedEnv.getResourceParam().setPrecedenceEnforcement(IloExtended);
  IloArray<IloDiscreteResource> resources(env, numberOfResources);
  char buffer[128];
  IloInt r = 0;
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
	solution.add(activity);
	sprintf(buffer, "%d", j+1);
	activity.setName(buffer);
       	activity.setObject(job.getImpl());
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

  solution.add(makespan);

  return model;
}

void WriteSolution(IloInt instanceSet,
		   IloInt paramNumber,
		   IloInt instanceNumber,
		   IloSchedulerSolution sol,
		   IloIntVar makespan) {
  char filename[128];
  sprintf(filename, "Solutions/j%d%d_%d.sm.opt", instanceSet, paramNumber, instanceNumber);
  ofstream stream(filename);

  stream << "Instance Set    :j" << instanceSet << endl;
  stream << "Type            :sm" << endl;
  stream << "Parameter Number:" << paramNumber << endl;
  stream << "Instance Number :" << instanceNumber << endl;
  stream << "Makespan        :" << sol.getMin(makespan) << endl;
  stream << "------------------------------------------------------------------" << endl;
  stream << "Solution" << endl;
  stream << "Job \tMode \tStart Time" << endl;
  stream << "------------------------------------------------------------------" << endl;

  for (IloSchedulerSolution::ActivityIterator ite(sol); ite.ok(); ++ite) {
    IloActivity act = *ite;
    stream << act.getName() << "\t1\t" << sol.getStartMin(act) << endl;
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
  char*     instanceDir = "data/rcpsp/";
  IlcFloat  timeLimit = 1800.0;
  IloInt    lb = 0;
  IloInt    ub = IloIntMax;

  IloInt instanceSet;
  IloInt paramNumber;
  IloInt instanceNumber;

  IlcInt argNr = 1;
  while (argNr < argc) {
    if (!strcmp(argv[argNr], "-set")) {
      argNr++;
      if (argNr < argc) {
	instanceSet = atoi(argv[argNr]);
      }
    }
    else if (!strcmp(argv[argNr], "-param")) {
      argNr++;
      if (argNr < argc) {
	paramNumber = atoi(argv[argNr]);
      }
    }   
    else if (!strcmp(argv[argNr], "-instance")) {
      argNr++;
      if (argNr < argc) {
	instanceNumber = atoi(argv[argNr]);
      }
    }
    else if (!strcmp(argv[argNr], "-lb")) {
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
  
  char instanceFile[128];

  sprintf(instanceFile, "j%d%d_%d.sm", instanceSet, paramNumber, instanceNumber);

  ReadPSPLib(instanceDir, 
	     instanceFile,
	     numberOfJobs,
	     numberOfResources,
	     numberOfActivities,
	     resourceAssignments,
	     durations,
	     precedences,
	     resourceCapacity);

  IloIntVar makespan;
  IloSchedulerSolution sol;
  IloModel model = StateModel(numberOfJobs, 
			      numberOfResources, 
			      numberOfActivities, 
			      resourceAssignments, 
			      durations, 
			      precedences, 
			      resourceCapacity,
			      factor,
			      makespan,
			      sol);
  IloEnv env = model.getEnv();
  
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
    WriteSolution(instanceSet, paramNumber, instanceNumber, sol, makespan);
  }
  
  cout << "\t" << env.getTime() - t0 << endl;
  env.end();
  
}

