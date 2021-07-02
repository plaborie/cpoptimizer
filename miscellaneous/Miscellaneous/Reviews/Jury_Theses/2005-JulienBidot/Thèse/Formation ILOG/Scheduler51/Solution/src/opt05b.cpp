#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

//////////////////////////////////////////////////////////////////////////
//
// Solution for Opt05: writing search goals
//
// In this exercise you will build on the example discussed in the slides.
// However, rather than a job-shop problem, this exercise is to solve a job-
// shop with alternative resource problem.  Each activity is defined as it
// was in the job-shop except:
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

  /* CREATE THE ACTIVITIES. */
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

  /* RETURN THE MODEL. */
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
  return IlcAnd(IlcAssignAlternative(scheduler),
                IlcRank(scheduler,
                        solver.getIntVar(makespan)));
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

ILCGOAL2(MyTryAssignAltGoal, IlcAltResConstraint, altRC,
         IlcResource, res) {
  // Use the correct IlcAltResConstraint method to assign it to a resource
  altRC.setSelected(res);
  return 0;
}

ILCGOAL2(MyTryAssignNotAltGoal, IlcAltResConstraint, altRC,
         IlcResource, res) {
  // Use the correct IlcAltResConstraint to NOT assign it to a
  // resource
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
  IloSolver solver = getSolver();
  IlcResourceSelectorObject resSel(IlcUnaryResourcePredicate(solver) &&
                                     !IlcResRankedPredicate(solver),
                                   IlcResGlobalSlackEvaluator(solver));
  IlcUnaryResource minGSResource;
  (void)resSel.select(minGSResource, _scheduler);
  return minGSResource;
}

IlcResourceConstraint
MyRankGoalI::selectResourceConstraint(IlcUnaryResource resource) {

  IloSolver solver = getSolver();
  IlcRCSelectorObject rcSel (
      IlcRCPossibleFirstPredicate(solver),
      IlcLexicalComposition(IlcRCStartMinEvaluator(solver),
                            IlcRCStartMaxEvaluator(solver)));

  IlcResourceConstraint constraint;
  (void)rcSel.select(constraint, resource);

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
  return IlcAnd(IlcOr(MyTryAssignAltGoal(solver, altRC, res),
                      MyTryAssignNotAltGoal(solver, altRC, res)),
                this);
}

IlcAltResSet MyAssignAltGoalI::selectAltResSet() {
  // iterate through the IlcAltResSets and find the first one that has an
  // alternative resource constraint that has not been assigned.
  // Return it.

  for(IlcAltResSetIterator altResIter(_scheduler);
      altResIter.ok(); ++altResIter) {
    IlcAltResSet resSet = *altResIter;
    for(IlcAltResConstraintIterator altRCIter(resSet);
        altRCIter.ok(); ++altRCIter) {
      if (!(*altRCIter).isResourceSelected())
        return resSet;
    }
  }

  return 0; // empty handle
}

IlcAltResConstraint
MyAssignAltGoalI::selectAltResConstraint(IlcAltResSet resSet) {
  // iterate through the alternative resource constraints on the alt
  // res set and pick up the one with the greatest difference between the
  // durations on the possible resources
  IlcInt greatestDiff = -1;
  IlcAltResConstraint greatestDiffConstraint;

  for(IlcAltResConstraintIterator altRCIter(resSet);
      altRCIter.ok(); ++altRCIter) {
    IlcAltResConstraint altRC = *altRCIter;

    IlcInt overallMaxDur = -1;
    IlcInt overallMinDur = IlcIntMax;

    if (!altRC.isResourceSelected()) {
      for(IlcPossibleAltResIterator resIter(altRC); resIter.ok(); ++resIter) {
        IlcResource res = *resIter;
        IlcInt maxDur = altRC.getDurationMax(res);
        if (maxDur > overallMaxDur)
          overallMaxDur = maxDur;
        IlcInt minDur = altRC.getDurationMin(res);
        if (minDur < overallMinDur)
          overallMinDur = minDur;
      }

      IlcInt diff = overallMaxDur - overallMinDur;
      if (diff > greatestDiff) {
        greatestDiff = diff;
        greatestDiffConstraint = altRC;
      }
    }
  }

  return greatestDiffConstraint; // maybe empty handle
}

IlcResource MyAssignAltGoalI::selectResource(IlcAltResConstraint altRC) {
  // iterate through the possible resources for altRC and select the
  // one for which the activity has the smallest min duration
  IlcInt minDur = IlcIntMax;
  IlcResource minDurRes;
  for(IlcPossibleAltResIterator resIter(altRC); resIter.ok(); ++resIter) {
    IlcResource res = *resIter;
    IlcInt dur = altRC.getDurationMin(res);
    if (dur < minDur) {
      minDur = dur;
      minDurRes = res;
    }
  }

  return minDurRes; // maybe empty handle
}

// "Bridge" goals
IlcGoal MyRankGoal(IloSolver solver) {
  // allocate and return your custom goal
  return new (solver.getHeap()) MyRankGoalI(solver);
}

IlcGoal MyAssignAltGoal(IloSolver solver) {
  // allocate and return your custom goal
  return new (solver.getHeap()) MyAssignAltGoalI(solver);
}

ILOCPGOALWRAPPER1(MyCustomAssignAltAndRank, solver, IloIntVar, makespan) {
  // return goal by combining custom goals
  // Hint:
  // 1) assign alternative resources then
  // 2) rank resources
  // 3) instanciate makespan
  return IlcAnd(MyAssignAltGoal(solver),
                MyRankGoal(solver),
                IlcInstantiate(solver.getIntVar(makespan)));
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

    //IloGoal goal = IloAssignAlternative(env) &&
    //               IloRankForward(env, makespan);

    // Task 2) Use "bridge" to predefined Scheduler Extensions goal

    //IloGoal goal = MySolverAssignAltAndRank(env, makespan);

    // Task 3) Use custom built goal

    IloGoal goal = MyCustomAssignAltAndRank(env, makespan);

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

/*
Solution with makespan [48..48]
J0S0R2[0..1 -- 2 --> 2..3]: R3
J0S1R0[2..7 -- 3 --> 5..10]: R0
J0S2R1[8..22 -- 6 --> 14..28]: R1
J0S3R3[22..28 -- 7 --> 29..35]: R3
J0S4R5[34..35 -- 3 --> 37..38]: R5
J0S5R4[37..41 -- 7 --> 44..48]: R1
J1S0R1[0 -- 9 --> 9]: R2
J1S1R2[9 -- 5 --> 14]: R2
J1S2R4[14 -- 10 --> 24]: R4
J1S3R5[24 -- 10 --> 34]: R5
J1S4R0[34 -- 10 --> 44]: R0
J1S5R3[44 -- 4 --> 48]: R3
J2S0R2[6 -- 6 --> 12]: R3
J2S1R3[12 -- 4 --> 16]: R3
J2S2R5[16 -- 8 --> 24]: R5
J2S3R0[25 -- 9 --> 34]: R0
J2S4R1[34..40 -- 1 --> 35..41]: R1
J2S5R4[41 -- 7 --> 48]: R4
J3S0R1[3..5 -- 5 --> 8..10]: R1
J3S1R0[8..10 -- 5 --> 13..15]: R0
J3S2R2[14..15 -- 5 --> 19..20]: R2
J3S3R3[19..21 -- 3 --> 22..24]: R3
J3S4R4[24 -- 8 --> 32]: R4
J3S5R5[32..36 -- 11 --> 43..47]: R2
J4S0R2[19..20 -- 9 --> 28..29]: R2
J4S1R1[28..29 -- 3 --> 31..32]: R1
J4S2R4[32 -- 5 --> 37]: R4
J4S3R5[37..38 -- 4 --> 41..42]: R5
J4S4R0[41..42 -- 4 --> 45..46]: R5
J4S5R3[45..46 -- 2 --> 47..48]: R0
J5S0R1[0 -- 3 --> 3]: R1
J5S1R3[3 -- 3 --> 6]: R3
J5S2R5[6 -- 9 --> 15]: R5
J5S3R0[15 -- 10 --> 25]: R0
J5S4R4[37 -- 4 --> 41]: R4
J5S5R2[43..47 -- 1 --> 44..48]: R2
Number of fails               : 585
Number of choice points       : 615
Number of variables           : 218
Number of constraints         : 240
Reversible stack (bytes)      : 88464
Solver heap (bytes)           : 160824
Solver global heap (bytes)    : 100620
And stack (bytes)             : 4044
Or stack (bytes)              : 4044
Search Stack (bytes)          : 4044
Constraint queue (bytes)      : 11144
Total memory used (bytes)     : 373184
Elapsed time since creation   : 1.302
*/