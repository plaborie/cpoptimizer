///////////////////////////////////////////////////////////////////////////////
//
// Opt05a: a Job-shop problem
//
///////////////////////////////////////////////////////////////////////////////

#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

IloInt ResourceNumbers06 [] = {2, 0, 1, 3, 5, 4,
                               1, 2, 4, 5, 0, 3,
                               2, 3, 5, 0, 1, 4,
                               1, 0, 2, 3, 4, 5,
                               2, 1, 4, 5, 0, 3,
                               1, 3, 5, 0, 4, 2};

IloInt Durations06 [] = { 1,  3,  6,  7,  3,  6,
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

IloInt Durations10 [] = {29, 78,  9, 36, 49, 11, 62, 56, 44, 21,
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

IloInt Durations20 [] = {29,  9, 49, 62, 44,
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
            IloInt* durations,
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

  IloInt j;
  IloUnaryResource *resources =
    new (env) IloUnaryResource[numberOfResources];
  for (j = 0; j < numberOfResources; j++)
    resources[j] = IloUnaryResource(env);

  /* CREATE THE ACTIVITIES. */
  char buffer[128];
  k = 0;
  IloInt i;
  for (i = 0; i < numberOfJobs; i++) {
    IloActivity previousActivity;
    for (j = 0; j < numberOfResources; j++) {
      IloActivity activity(env, durations[k]);
      sprintf(buffer, "J%dS%dR%d", i, j, resourceNumbers[k]);
      activity.setName(buffer);

      IloResourceConstraint rct =
        activity.requires(resources[resourceNumbers[k]]);
      model.add(rct);

      if (j != 0)
        model.add(activity.startsAfterEnd(previousActivity));
      previousActivity = activity;
      k++;
    }
    model.add(previousActivity.endsBefore(makespan));
  }

  model.add(IloMinimize(env, makespan));

  /* RETURN THE MODEL. */
  return model;
}


///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void
PrintSolution(const IloSolver& solver, IloIntVar makespan)
{
  IlcScheduler scheduler(solver);
  IloEnv env = solver.getEnv();
  solver.out() << "Solution with makespan: "
         << solver.getIntVar(makespan).getValue() << endl;
  for(IloIterator<IloActivity> ite(env); ite.ok(); ++ite)
    solver.out() << scheduler.getActivity(*ite) << endl;
}

///////////////////////////////////////////////////////////////////////////////
//
// PROBLEM SOLVING
//
///////////////////////////////////////////////////////////////////////////////

ILOCPGOALWRAPPER1(MySolverRank, solver, IloIntVar, makespan) {
  IlcScheduler scheduler(solver);

  return IlcRank(scheduler,
                 solver.getIntVar(makespan),
                 IlcSelResMinGlobalSlack,
                 IlcSelFirstRCMinStartMax);
}

// Custom building the Solver goals

ILCGOAL1(MyTryRankFirstGoal, IlcResourceConstraint, rc) {
  rc.rankFirst();
  return 0;
}

ILCGOAL1(MyTryRankNotFirstGoal, IlcResourceConstraint, rc) {
  rc.rankNotFirst();
  return 0;
}

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

IlcGoal MyRankGoal(IloSolver solver) {
  IloEnv env = solver.getEnv();
  return new (env) MyRankGoalI(solver);
}

ILOCPGOALWRAPPER1(MyCustomRank, solver,
                  IloIntVar, makespan) {
  return IlcAnd(MyRankGoal(solver),
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
               IloInt*& durations)
{
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
  try {
    IloEnv env;

    IloInt numberOfJobs = 6;
    IloInt numberOfResources = 6;
    IloInt* resourceNumbers = ResourceNumbers06;
    IloInt* durations = Durations06;
    InitParameters(argc,
                   argv,
                   numberOfJobs,
                   numberOfResources,
                   resourceNumbers,
                   durations);
    IloIntVar makespan;
    IloModel model = DefineModel(env,
                                 numberOfJobs,
                                 numberOfResources,
                                 resourceNumbers,
                                 durations,
                                 makespan);
    IloSolver solver(model);

    // Concert solve
//    IloGoal goal = IloRankForward(env, makespan,
//                                  IloSelResMinGlobalSlack,
//                                  IloSelFirstRCMinStartMax);

//    IloGoal goal = MySolverRank(env, makespan);

    IloGoal goal = MyCustomRank(env, makespan);

    solver.solve(goal);
    PrintSolution(solver, makespan);
    solver.printInformation();
    env.end();
  }
  catch (IloException& exc) {
    cout << exc << endl;
  }

  return 0;
}

