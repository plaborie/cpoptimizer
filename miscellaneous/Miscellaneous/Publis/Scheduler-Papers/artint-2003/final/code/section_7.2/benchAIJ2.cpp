// -------------------------------------------------------------- -*- C++ -*- 
// File: benchAIJ2.cpp
// --------------------------------------------------------------------------
// Copyright (C) 1999-2001 by ILOG.
// All Rights Reserved.
//
// This code implements the search procedure  described in section 7.2
// of  the paper "Algorithms for   Propagating Resource Constraints in
// A.I.   Planning   and   Scheduling: Existing  Approaches   and  New
// Results",  Artificial  Intelligence Journal.   It   runs with  ILOG
// Concert 1.1/Solver 5.1/Scheduler 5.1 and later releases.
//
// --------------------------------------------------------------------------

#include <ilsched/iloscheduler.h>
#ifndef WIN32
#include <strstream.h>
#else
#include <strstrea.h>
#endif

ILOSTLBEGIN

// --------------------------------------------------------------------------
// SECTION 1. PROBLEM DEFINITION
// --------------------------------------------------------------------------

class NbUnrankedExt {
private:
  IlcInt _nbOfUnranked;

public:
  NbUnrankedExt() :_nbOfUnranked(0) {};
  ~NbUnrankedExt(){};
  void setValue(IlcInt nbOfUnranked) {
    _nbOfUnranked = nbOfUnranked;
  }
  IlcInt getValue() const {
    return _nbOfUnranked;
  }
};

IloModel
DefineModel(IloEnv& env,
            IloInt numberOfJobs,
            IloInt numberOfResources,        
            IloInt* resourceNumbers,
            IloInt* durations,
	    IloInt levelCap,
	    IloInt levelPrec,
            IloNumVar& makespan)
{
  IloModel model(env);

  /* Create makespan variable. */
  IloInt numberOfActivities = numberOfJobs * numberOfResources;
  IloInt horizon = 0;
  IloInt k;

  for (k = 0; k < numberOfActivities; k++)
    horizon += durations[k];

  makespan = IloNumVar(env, 0, horizon, ILOINT);

  /* Crate resources. */
  IloSchedulerEnv schedEnv(env);

  if (levelCap == 1) {
    schedEnv.getResourceParam().setCapacityEnforcement(IloLow);
  } else if (levelCap == 2) {
    schedEnv.getResourceParam().setCapacityEnforcement(IloBasic);
  } else if (levelCap == 3) {
    schedEnv.getResourceParam().setCapacityEnforcement(IloMediumHigh);
  } else if (levelCap == 4) {
    schedEnv.getResourceParam().setCapacityEnforcement(IloHigh);
  }

  if (levelPrec == 0) {
    schedEnv.getResourceParam().setPrecedenceEnforcement(IloMediumHigh);
  } else  if (levelPrec == 1) {
    schedEnv.getResourceParam().setPrecedenceEnforcement(IloHigh);
  } else if (levelPrec == 2) {
    schedEnv.getResourceParam().setPrecedenceEnforcement(IloExtended);
  }

  IloInt j;
  IloUnaryResource *resources = 
    new (env) IloUnaryResource[numberOfResources];
  for (j = 0; j < numberOfResources; j++) {   
      resources[j] = IloUnaryResource(env);
      if (levelCap == 0) 
	resources[j].ignoreCapacityConstraints();
  }
  
  /* Create activities. */
  char buffer[128];
  k = 0;
  IloInt i;
  for (i = 0; i < numberOfJobs; i++) {
    IloActivity previousActivity;
    for (j = 0; j < numberOfResources; j++) {
      IloActivity activity(env, durations[k]);
      sprintf(buffer, "J%ldS%ldR%ld", i, j, resourceNumbers[k]);
      activity.setName(buffer);

      IloResourceConstraint rct = 
        activity.requires(resources[resourceNumbers[k]]);
      NbUnrankedExt* nbOfUnranked = 
        new (env) NbUnrankedExt();
      rct.setObject(nbOfUnranked);
      model.add(rct);

      if (j != 0)
        model.add(activity.startsAfterEnd(previousActivity));
      previousActivity = activity;
      k++;
    }
    model.add(previousActivity.endsBefore(makespan));
  }

	makespan.setMax(1362247);
	
  /* Return the model. */
  return model;
}

// --------------------------------------------------------------------------
// SECTION 2. PROBLEM SOLVING
// --------------------------------------------------------------------------

IlcInt GetNumberOfUnranked(const IlcResourceConstraint& rct) {
  /* Return number of unranked w.r.t. rct. */
  IlcInt nb = 0;
  for (IlcResourceConstraintIterator ite(rct, IlcUnranked);
       ite.ok(); ++ite)
    nb++;
  return nb;
}

IloNum Commit(IlcInt xmin, IlcInt xmax,
	      IlcInt ymin, IlcInt ymax) {
  IloNum result = (xmax-ymin+1)*(xmax-ymin+1);
  if (xmin > ymin)
    result -= (xmin-ymin)*(xmin-ymin);
  if (xmax > ymax)
    result -= (xmax-ymax)*(xmax-ymax);
  return result / (2*(ymax-ymin+1)*(xmax-xmin+1));
}

IloNum GetOpportunity(const IlcScheduler& scheduler,
                      const IlcResourceConstraint& srct1,
                      const IlcResourceConstraint& srct2) {
  IlcActivity act1 = srct1.getActivity();
  IlcActivity act2 = srct2.getActivity();
  
  /* Estimation of temporal commitment when rct1 is ranked before rct2. */
  IloNum delta12 = Commit(act1.getEndMin(), act1.getEndMax(),
			  act2.getStartMin(), act2.getStartMax());
  
  /* Estimation of temporal commitment when rct2 is ranked before rct1. */
  IloNum delta21 = Commit(act2.getEndMin(), act2.getEndMax(),
			  act1.getStartMin(), act1.getStartMax());
  
  /* Minimal number of unranked resource constraints. */
  IlcInt nbUrkd1   = ((NbUnrankedExt*)(scheduler.getExtractable(srct1).getObject()))->getValue();
  IlcInt nbUrkd2   = ((NbUnrankedExt*)(scheduler.getExtractable(srct2).getObject()))->getValue();
  IlcInt minNbUrkd = (nbUrkd1 <= nbUrkd2) ? nbUrkd1 : nbUrkd2;
  
  /* Return estimation of opportunity. */
  return (minNbUrkd * (delta12 - delta21));
}

IlcBool 
SelectMostOpportunisticConflict(IlcScheduler& schedule,
                                IlcResourceConstraint& selectedRct1,
                                IlcResourceConstraint& selectedRct2) {
  
  IlcBool existsConflict = IlcFalse;

  IloNum oppMaxAbs = -1;
  IloNum oppMax = 0;
  IloNum opp;
  
  for (IlcUnaryResourceIterator ires(schedule); ires.ok(); ++ires) {
    IlcUnaryResource resource = (*ires);
    if (resource.hasPrecedenceGraphConstraint() &&
        !resource.isRanked()) {

      /* For each resource constraint, compute and store the number of 
	 resource constraints unranked w.r.t. it . */
      for (IlcResourceConstraintIterator irct(resource); 
           irct.ok(); ++irct) {
        IlcResourceConstraint rct = (*irct);
        if (!rct.isRanked())
          ((NbUnrankedExt*)schedule.getExtractable(rct).getObject())->
            setValue(GetNumberOfUnranked(rct));
     }
      
      /* Select most opportunistic pair of resource constraints. */
      for (IlcResourceConstraintIterator isrct1(resource); 
           isrct1.ok(); ++isrct1) {
        IlcResourceConstraint srct1 = (*isrct1);
        if (!srct1.isRanked()) {
          for (IlcResourceConstraintIterator isrct2(srct1, IlcUnranked);
               isrct2.ok(); ++isrct2) {
            IlcResourceConstraint srct2 = (*isrct2);
      opp = GetOpportunity(schedule, srct1, srct2);
      if (oppMaxAbs < IloAbs(opp)) {
              existsConflict = IlcTrue;
              oppMaxAbs      = IlcAbs(opp);
              oppMax         = opp;
              selectedRct1   = srct1;
              selectedRct2   = srct2;
            }
          }
        }
      }
    }
  }
  
  /* Select which branch will be chosen first among rct1 << rct2 and
     rct2 << rct1. */
  if (existsConflict && (0 < oppMax)) {
    IlcResourceConstraint tmpRct = selectedRct1;
    selectedRct1 = selectedRct2;
    selectedRct2 = tmpRct;
  }
  
  return existsConflict;
}

ILCGOAL0(SolveIlc) {
  IloSolver s = getSolver();
  IlcScheduler scheduler = IlcScheduler(s);
  IlcResourceConstraint srct1;
  IlcResourceConstraint srct2;
  if (SelectMostOpportunisticConflict(scheduler, srct1, srct2))
    return IlcAnd(IlcTrySetSuccessor(srct1, srct2), this);

  return 0;
}

ILOCPGOALWRAPPER0(Solve, solver) {
  return SolveIlc(solver);
}

// --------------------------------------------------------------------------
// SECTION 3. MAIN FUNCTION
// --------------------------------------------------------------------------

void
InitParameters(int      argc,
	       char**   argv,
	       IlcInt&  numberOfJobs,
	       IlcInt&  numberOfResources,		    
	       IlcInt*& resourceNumbers,
	       IlcInt*& durations,
	       IlcInt*& releaseDates,
	       IlcInt*& dueDates,
	       IlcInt&  levelCap,
	       IlcInt&  levelPrec)
{
  char* filename = argv[1];
  cout << "INSTANCE: " << filename << "\t";
  ifstream file(filename);
  int lineLimit = 1024;
  char buffer[1024];
  file.getline(buffer, lineLimit);
  istrstream line(buffer, strlen(buffer));
  line >> numberOfJobs;
  line >> numberOfResources;
  resourceNumbers = new IlcInt[numberOfJobs * numberOfResources];
  durations = new IlcInt[numberOfJobs * numberOfResources];
  if (2 < argc)
    levelCap = atoi(argv[2]);
  if (3 < argc)
    levelPrec = atoi(argv[3]);
  IlcInt k = 0;
  for (IlcInt i = 0; i < numberOfJobs; i++) {
    file.getline(buffer, lineLimit);
    istrstream jobline(buffer, strlen(buffer));
    for (IlcInt j = 0; j < numberOfResources; j++) {
      jobline >> resourceNumbers[k];
      jobline >> durations[k];
      k++;
    }
    if (releaseDates) {
      jobline >> releaseDates[i];
      jobline >> dueDates[i];
    }
  }
}

int main(int argc, char** argv)
{
  IloInt numberOfJobs = 0;
  IloInt numberOfResources = 0;
  IloInt levelCap = 0;
  IloInt levelPrec = 2;
  IloInt* resourceNumbers = 0;
  IloInt* durations = 0;
  IloInt* releaseDates = 0;
  IloInt* dueDates = 0;
  InitParameters(argc,
		 argv,
		 numberOfJobs,
		 numberOfResources,
		 resourceNumbers,
		 durations,
		 releaseDates,
		 dueDates,
		 levelCap,
		 levelPrec);
  IloEnv env;
  IloNumVar makespan;
  IloModel model = DefineModel(env, 
			       numberOfJobs,
			       numberOfResources,
			       resourceNumbers,
			       durations,
			       levelCap,
			       levelPrec,
			       makespan);
  IloSolver solver(model);
  solver.solve(Solve(env));
  solver.out() << solver.getMin(makespan) << endl;
  delete [] resourceNumbers;
  delete [] durations;
  delete [] releaseDates;
  delete [] dueDates;
  env.end();
  return 0;
}
