#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

//////////////////////////////////////////////////////////////////////////
//
// Step Opt05: writing search goals
// 
// In this exercise you will build on the example discussed in the slides.
// However, rather than a job-shop problem, this exercise is to solve a job
// shop with alternative resources problem.  Each activity is defined as it
// was in the job shop except:
// 1) each activity has 2 alternative resources
// 2) activities have different durations depending on which resource they
// are assigned to
// 
// The goal is to assign all activities to a resource and rank each
// resource so as to minimize the total makespan of the schedule.
// 
// Your task has three parts mirroring the ways that goals can be created
// in Concert/Scheduler and Solver/Scheduler Extensions.
// 
// Task 1. Using Scheduler predefined goals, write a goal that will solve
// this problem.  Hint: assign all the resources first then rank each
// resource.  Note this task should require a single command.
// 
// Task 2. Using Scheduler Extensions goals, write a goal that will solve
// this problem. Use the same hint as in Task 1.
// 
// Task 3. Write your own custom goals to solve this problem.  Use the same
// hint as in Task 1.
// 
// These three tasks are separate: you should comment out the code you
// wrote for Task k before doing Task k+1.
// 
///////////////////////////////////////////////////////////////////////////

IloInt ResourceNumbers06 [] = {2, 0, 1, 3, 5, 4,
                               1, 2, 4, 5, 0, 3,
                               2, 3, 5, 0, 1, 4,
                               1, 0, 2, 3, 4, 5,
                               2, 1, 4, 5, 0, 3,
                               1, 3, 5, 0, 4, 2};

IloNum Durations06 [] = { 1,  3,  6,  7,  3,  6,
                          8,  5, 10, 10, 10,  4,
                          5,  4,  8,  9,  1,  7,
                          5,  5,  5,  3,  8,  9,
                          9,  3,  5,  4,  3,  1,
                          3,  3,  9, 10,  4,  1};

IloInt ResourceNumbers10 [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 2, 4, 9, 3, 1, 6, 5, 7, 8,
                               1, 0, 3, 2, 8, 5, 7, 6, 9, 4,
                               1, 2, 0, 4, 6, 8, 7, 3, 9, 5,
                               2, 0, 1, 5, 3, 4, 8, 7, 9, 6,
                               2, 1, 5, 3, 8, 9, 0, 6, 4, 7,
                               1, 0, 3, 2, 6, 5, 9, 8, 7, 4,
                               2, 0, 1, 5, 4, 6, 8, 9, 7, 3,
                               0, 1, 3, 5, 2, 9, 6, 7, 4, 8,
                               1, 0, 2, 6, 8, 9, 5, 3, 4, 7};

IloNum Durations10 [] = {29, 78,  9, 36, 49, 11, 62, 56, 44, 21,
                         43, 90, 75, 11, 69, 28, 46, 46, 72, 30,
                         91, 85, 39, 74, 90, 10, 12, 89, 45, 33,
                         81, 95, 71, 99,  9, 52, 85, 98, 22, 43,
                         14,  6, 22, 61, 26, 69, 21, 49, 72, 53,
                         84,  2, 52, 95, 48, 72, 47, 65,  6, 25,
                         46, 37, 61, 13, 32, 21, 32, 89, 30, 55,
                         31, 86, 46, 74, 32, 88, 19, 48, 36, 79,
                         76, 69, 76, 51, 85, 11, 40, 89, 26, 74,
                         85, 13, 61,  7, 64, 76, 47, 52, 90, 45};

IloInt ResourceNumbers20 [] = {0, 1, 2, 3, 4,
                               0, 1, 3, 2, 4,
                               1, 0, 2, 4, 3,
                               1, 0, 4, 2, 3,
                               2, 1, 0, 3, 4,
                               2, 1, 4, 0, 3,
                               1, 0, 2, 3, 4,
                               2, 1, 0, 3, 4,
                               0, 3, 2, 1, 4,
                               1, 2, 0, 3, 4,
                               1, 3, 0, 4, 2,
                               2, 0, 1, 3, 4,
                               0, 2, 1, 3, 4,
                               2, 0, 1, 3, 4,
                               0, 1, 4, 2, 3,
                               1, 0, 3, 4, 2,
                               0, 2, 1, 3, 4,
                               0, 1, 4, 2, 3,
                               1, 2, 0, 3, 4,
                               0, 1, 2, 3, 4};

IloNum Durations20 [] = {29,  9, 49, 62, 44,
                         43, 75, 69, 46, 72,
                         91, 39, 90, 12, 45,
                         81, 71,  9, 85, 22,
                         14, 22, 26, 21, 72,
                         84, 52, 48, 47,  6,
                         46, 61, 32, 32, 30,
                         31, 46, 32, 19, 36,
                         76, 76, 85, 40, 26,
                         85, 61, 64, 47, 90,
                         78, 36, 11, 56, 21,
                         90, 11, 28, 46, 30,
                         85, 74, 10, 89, 33,
                         95, 99, 52, 98, 43,
                          6, 61, 69, 49, 53,
                          2, 95, 72, 65, 25,
                         37, 13, 21, 89, 55,
                         86, 74, 88, 48, 79,
                         69, 51, 11, 89, 74,
                         13,  7, 76, 52, 45};


///////////////////////////////////////////////////////////////////////////////
//
// PROBLEM DEFINITION
//
///////////////////////////////////////////////////////////////////////////////

IloModel
DefineModel(IloEnv& env,
            IloInt numberOfJobs,
            IloInt numberOfResources,        
            IloInt* resourceNumbers,
            IloNum* durations,
      IloRandom randomGenerator,
      IloSchedulerSolution solution,
            IloIntVar& makespan)
{
  IloModel model(env);

  /* CREATE THE MAKESPAN VARIABLE. */
  IloInt numberOfActivities = numberOfJobs * numberOfResources;
  IloInt horizon = 0;
  IloInt k;

  for (k = 0; k < numberOfActivities; k++)
    horizon += durations[k];

  makespan = IloIntVar(env, 0, horizon);

  /* CREATE THE RESOURCES. */
  IloSchedulerEnv schedEnv(env);
  schedEnv.getResourceParam().setCapacityEnforcement(IloMediumHigh);
  schedEnv.getResourceParam().setPrecedenceEnforcement(IloMediumHigh);

  char buffer[128];
  IloInt j;
  IloUnaryResource *resources = 
    new (env) IloUnaryResource[numberOfResources];
  for (j = 0; j < numberOfResources; j++) {
    sprintf(buffer, "R%d", j);
    resources[j] = IloUnaryResource(env, buffer);
  }

  /* CREATE THE ALTERNATIVE RESOURCE SETS */
  env.out() << "Creating resource sets" << endl;
  IloInt *altResourceNumbers = new (env) IloInt[numberOfResources];
  IloAltResSet *altResSets = 
    new (env) IloAltResSet[numberOfResources];
  for (j = 0; j < numberOfResources; j++) {
    altResSets[j] = IloAltResSet(env);
    altResSets[j].add(resources[j]);

    // RANDOMLY PICK ANOTHER RESOURCE TO BE IN THE SET
    assert(numberOfResources > 1);
    IloInt index = randomGenerator.getInt(numberOfResources);
    while(index == j) {
      index = randomGenerator.getInt(numberOfResources);
    }

    altResSets[j].add(resources[index]);
    altResourceNumbers[j] = index;
    env.out() << "Set #" << j << ":\t" << resources[j].getName()
        << " " << resources[index].getName() << endl;
  }

  /* CREATE THE ALTERNATIVE DURATIONS */
  IloNum *altDurations = new (env) IloNum[numberOfActivities];
  for(k = 0; k < numberOfActivities; k++) {
    IloNum multiplier = 1.0 + (randomGenerator.getFloat() / 2.0);
    altDurations[k] = IloCeil(multiplier * durations[k]);
  }

  /* CREATE THE ACTIVITIES */
  env.out() << "Setting alternative processing times" << endl;
  k = 0;
  IloInt i;
  for (i = 0; i < numberOfJobs; i++) {
    IloActivity previousActivity;
    for (j = 0; j < numberOfResources; j++) {
      IloInt ptMin = IloMin(durations[k], altDurations[k]);
      IloInt ptMax = IloMax(durations[k], altDurations[k]);
      IloIntVar ptVar(env, ptMin, ptMax);

      IloActivity activity(env, ptVar);
      sprintf(buffer, "J%dS%dR%d", i, j, resourceNumbers[k]);
      activity.setName(buffer);
      solution.add(activity);

      IloAltResConstraint altrc =
         activity.requires(altResSets[resourceNumbers[k]]);
      
      /* SET THE DIFFERENT DURATIONS DEPENDING ON RESOURCE SELECTION */
      altrc.setDurationMax(resources[resourceNumbers[k]], durations[k]);
      altrc.setDurationMin(resources[resourceNumbers[k]], durations[k]);

      altrc.setDurationMax(resources[altResourceNumbers[resourceNumbers[k]]],
         altDurations[k]);
      altrc.setDurationMin(resources[altResourceNumbers[resourceNumbers[k]]],
         altDurations[k]);

      env.out() << activity.getName() 
                << ":\tProcessing Time(" 
                << resources[resourceNumbers[k]].getName()
                << "): " << durations[k]
                << "\n\tProcessing Time(" 
                << resources[altResourceNumbers[resourceNumbers[k]]].getName()
                << "): " << altDurations[k]
                << endl;
  
      model.add(altrc);
      solution.add(altrc);

      if (j != 0)
        model.add(activity.startsAfterEnd(previousActivity));
      previousActivity = activity;
      k++;
    }
    model.add(previousActivity.endsBefore(makespan));
  }

  model.add(IloMinimize(env, makespan));
  solution.add(makespan);

  /* RETURN THE MODEL */
  return model;
}

///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void PrintRange(IloSolver solver, IloNum min, IloNum max) {
  if (min == max)
    solver.out() << (IlcInt)min;
  else
    solver.out() << (IlcInt)min << ".." << (IlcInt)max;
}

void PrintSolution(IloSolver solver, 
                   const IloSchedulerSolution solution,
                   const IloIntVar makespan)
{
  solver.out() << "Solution with makespan ["
         << solution.getMin(makespan) << ".." 
         << solution.getMax(makespan) << "]" << endl;

  for (IloSchedulerSolution::AltResConstraintIterator iter(solution);
       iter.ok();
       ++iter) 
  {
    IloAltResConstraint altrc = *iter;
    if (!solution.isResourceSelected(altrc))
      IloSchedulerException("No resource assigned!");

    IloActivity activity = altrc.getActivity();
    solver.out() << activity.getName() << "[";
    PrintRange(solver, 
               solution.getStartMin(activity),  
               solution.getStartMax(activity));
    solver.out() << " -- ";
    PrintRange(solver, 
               solution.getDurationMin(activity),
               solution.getDurationMax(activity));
    solver.out() << " --> ";
    PrintRange(solver, 
               solution.getEndMin(activity),
               solution.getEndMax(activity));
    solver.out() << "]: " << solution.getSelected(altrc).getName() << endl;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// PROBLEM SOLVING
//
///////////////////////////////////////////////////////////////////////////////

// Using predefined Solver goals.

ILOCPGOALWRAPPER1(MySolverAssignAltAndRank, solver, IloIntVar, makespan) {
  IlcScheduler scheduler(solver);

  // WRITE THE GOAL USING PREDEFINED SOLVER GOALS

  /***************** to be filled in ... ********/

  return IlcAnd(IlcAssignAlternative(scheduler), IlcRank(scheduler), IlcInstantiate(solver.getIntVar(makespan)));
  
}

// Custom building the Solver goals

// "Helper" goals

ILCGOAL1(MyTryRankFirstGoal, IlcResourceConstraint, rc) {
  rc.rankFirst();
  return 0;
}

ILCGOAL1(MyTryRankNotFirstGoal, IlcResourceConstraint, rc) {
  rc.rankNotFirst();
  return 0;
}

ILCGOAL2(MyTryAssignAltGoal, IlcAltResConstraint, altRC, IlcResource, res) {
  // Use the correct IlcAltResConstraint method to assign it to a resource 

  /***************** to be filled in ... (one line) ********/
	altRC.setSelected(res);
  
  return 0;
}

ILCGOAL2(MyTryAssignNotAltGoal, IlcAltResConstraint, altRC, IlcResource, res) {
  // Use the correct IlcAltResConstraint method to NOT assign it to a
  // resource 

  /***************** to be filled in ... (one line) ********/

	altRC.setNotPossible(res);

  return 0;
}

// Custom Rank Goal
class MyRankGoalI : public IlcGoalI {
private:
  IlcScheduler _scheduler;

public:
  MyRankGoalI(IloSolver s) : IlcGoalI(s), _scheduler(s) {}
  virtual IlcGoal execute();

  IlcUnaryResource selectResource();
  IlcResourceConstraint selectResourceConstraint(IlcUnaryResource);
};

IlcGoal MyRankGoalI::execute() {
  // select a resource
  IlcUnaryResource resource = selectResource();  
  if (0 == resource.getImpl())
    return 0;

  // select a resource constraint
  IlcResourceConstraint rc = selectResourceConstraint(resource);
  assert(rc.getImpl() != 0);
    
  // return a choice point
  IloSolver solver = getSolver();
  return IlcAnd(IlcOr(MyTryRankFirstGoal(solver, rc), 
          MyTryRankNotFirstGoal(solver, rc)),
    this);          
}

IlcUnaryResource MyRankGoalI::selectResource() {
  IlcInt minGS = IlcIntMax;
  IlcUnaryResource minGSResource;

  for(IlcUnaryResourceIterator iter(_scheduler); iter.ok(); ++iter) {
    IlcUnaryResource res = *iter;
    if (!res.isRanked()) {
      IlcInt globalSlack = res.getGlobalSlack();
      if (globalSlack < minGS) {
  minGS = globalSlack;
  minGSResource = res;
      }
    }
  }

  return minGSResource;
}

IlcResourceConstraint
MyRankGoalI::selectResourceConstraint(IlcUnaryResource resource) {
  IlcInt minStartMin = IlcIntMax;
  IlcInt minStartMax = minStartMin;
  IlcResourceConstraint constraint;
  for (IlcResourceConstraintIterator ite(resource); ite.ok(); ++ite) {
    IlcResourceConstraint ct = *ite;
    if (ct.isPossibleFirst()) {
      IlcActivity act = ct.getActivity();
      IlcInt smin = act.getStartMin();
      IlcInt smax = act.getStartMax();
      if ((smin < minStartMin) || 
          ((smin == minStartMin) && (smax < minStartMax))) {
        constraint = ct;
        minStartMin = smin;
        minStartMax = smax;
      }
    }
  }

  return constraint;
}

// Custom Assign Alternative goal
class MyAssignAltGoalI : public IlcGoalI {
private:
  IlcScheduler _scheduler;

public:
  MyAssignAltGoalI(IloSolver s) : IlcGoalI(s), _scheduler(s) {}
  virtual IlcGoal execute();

  IlcAltResSet selectAltResSet();
  IlcAltResConstraint selectAltResConstraint(IlcAltResSet);
  IlcResource selectResource(IlcAltResConstraint);
};

IlcGoal MyAssignAltGoalI::execute() {
  // select an alternative resource set
  IlcAltResSet resSet = selectAltResSet();
  if (resSet.getImpl() == 0)
    return 0; 

  // select an alternative resource constraint
  IlcAltResConstraint altRC = selectAltResConstraint(resSet);
  assert(altRC.getImpl() != 0);

  // select a resource
  IlcResource res = selectResource(altRC);
  assert(res.getImpl() != 0);
    
  // return a choice point
  IloSolver solver = getSolver();
  
  // Write the recursive goal (with a choice point) to be returned

  /***************** to be filled in ... ********/
  
  return IlcAnd(IlcOr(MyTryAssignAltGoal(solver,altRC,res),MyTryAssignNotAltGoal(solver,altRC,res)),this);
}

IlcAltResSet MyAssignAltGoalI::selectAltResSet() {
  // iterate through the IlcAltResSets and find the first one that has an 
  // alternative resource constraint that has not been assigned.
  // Return it.

  /***************** to be filled in ... ********/
	for (IlcAltResSetIterator altIte(_scheduler); altIte.ok(); ++altIte) 
		for(IlcAltResConstraintIterator altResConsIte(*altIte); altResConsIte.ok(); ++altResConsIte)
      if (!(*altResConsIte).isResourceSelected())
		  return (*altIte);

  return 0; // empty handle
}

IlcAltResConstraint
MyAssignAltGoalI::selectAltResConstraint(IlcAltResSet resSet) {
  // iterate through the alternative resource constraints on the alt
  // res set and pick up the one with the greatest difference between the 
  // durations on the possible resources
  IlcInt greatestDiff = -1;
  IlcAltResConstraint greatestDiffConstraint;
    /***************** to be filled in ... ********/
  for (IlcAltResSetIterator altIte(resSet); altIte.ok(); ++altIte) 
	for ()
	  if ((*altIte).getDurationMax()-(*altIte).getDurationMin() > greatestDiff) {
		  greatestDiff = (*altIte).getDurationMax()-(*altIte).getDurationMin();
		  greatestDiffConstraint = *altIte;
  
	  }
  }

  return greatestDiffConstraint; // maybe empty handle
}

IlcResource MyAssignAltGoalI::selectResource(IlcAltResConstraint altRC) {
  // iterate through the possible resources for altRC and select the
  // one for which the activity has the smallest min duration
  IlcInt minDur = IlcIntMax;
  IlcResource minDurRes;

  /***************** to be filled in ... ********/
  for (IlcAltResConstraintIterator altIte(altRC); altIte.ok(); ++altIte) {
	  if ((*altIte).getDurationMin() < minDur) {
		  minDur = (*altIte).getDurationMin();
		  minDurRes = *altIte;
  	  }
  }
  
  return minDurRes; // maybe empty handle
}

// "Bridge" goals
IlcGoal MyRankGoal(IloSolver solver) {
  // allocate and return your custom goal

  /***************** to be filled in ... ********/




  return 
}

IlcGoal MyAssignAltGoal(IloSolver solver) {
  // allocate and return your custom goal

  /***************** to be filled in ... ********/




  return 
}

ILOCPGOALWRAPPER1(MyCustomAssignAltAndRank, solver, IloIntVar, makespan) {
  // return goal by combining custom goals
  // Hint: 
  // 1) assign alternative resources then
  // 2) rank resources
  // 3) instanciate makespan

  /***************** to be filled in ... ********/




   return 
}

///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
InitParameters(int argc,
               char** argv,
               IloInt& numberOfJobs,
               IloInt& numberOfResources,                    
               IloInt*& resourceNumbers,
               IloNum*& durations)
{
  numberOfJobs = 6;
  numberOfResources = 6;
  resourceNumbers = ResourceNumbers06;
  durations = Durations06;

  if (argc > 1) {
    IloInt number = atol(argv[1]);
    if (number == 10) {
      numberOfJobs = 10;
      numberOfResources = 10;
      resourceNumbers = ResourceNumbers10;
      durations = Durations10;
    }
    else if (number == 20) {
      numberOfJobs = 20;
      numberOfResources = 5;
      resourceNumbers = ResourceNumbers20;
      durations = Durations20;
    }
  }
}

int main(int argc, char** argv)
{
  IloInt numberOfJobs;
  IloInt numberOfResources;
  IloInt* resourceNumbers;
  IloNum* durations;

  InitParameters(argc,
                 argv,
                 numberOfJobs,
                 numberOfResources,
                 resourceNumbers,
                 durations);
  try {
    IloEnv env;
    IloIntVar makespan;
    IloRandom randGen(env, 8975324);
    IloSchedulerSolution solution(env);
    IloModel model = DefineModel(env,
                                 numberOfJobs,
                                 numberOfResources,
                                 resourceNumbers,
                                 durations,
                                 randGen,
                                 solution,
                                 makespan);

    IloSolver solver(model);

    // NOTE:  Exercise contains 3 tasks.
    // 1) Solve the problem using the predefined Scheduler goal.
    // 2) Solve the problem using the predefined Scheduler Extension
    //    goals and write your own "bridge" goals (comment out the
    //    code from #1)
    // 3) Solve the problem using custom goals (comment out the code
    //    of tasks #1 and #2)

    // Task 1) Use predefined Scheduler goal
    // Hint: 
    // 1. assign alternative resources then
    // 2. rank resources and instanciate makespan

    /***************** to be filled in ... ********/
//IloGoal goal = MySolverAssignAltAndRank(env,makespan);


    // Task 2) Use "bridge" to predefined Scheduler Extensions goal

    /***************** to be filled in ... ********/
	IloGoal goal = 





    // Task 3) Use custom built goal

    /***************** to be filled in ... ********/

    solver.solve(goal);
    IlcScheduler scheduler(solver);
    solution.store(scheduler);
    PrintSolution(solver, solution, makespan);
    solver.printInformation();
    env.end();
  }
  catch (IloException& exc) {
    cout << exc << endl;
  }

  return 0;
}

