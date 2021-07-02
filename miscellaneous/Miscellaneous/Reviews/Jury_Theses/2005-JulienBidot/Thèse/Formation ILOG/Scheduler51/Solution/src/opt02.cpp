#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
//  Transition Costs
//
// A worker builds a single house.
// Each activity has a type (set when calling its constructor).
// This type is accessed through IloActivity::getTransitionType().
//
// Changing of type of task (for example from carpentry to
// windows) has a certain cost. Given such two types, the cost
// is returned by the function GetTransitionType( type1, type2 ).
//
// We want to minimize the overall cost, using three equivalent
// methods:
//
//  Method #1:
//
// Build a transition table using an IloTransitionParam.
// In the model, this parameter should be associated to the
// IloUnaryResource of the model.
//
//  Method #2:
//
// Build a transition table through an IlcTransitionTable (ILOG Sheduler
// Extensions object).
// Associate this object with the IlcUnaryResource using
// a special goal.
//
//  Method #3:
//
// Using ILOG Scheduler Extensions, subclass the object
// IlcTransitionCostObjectI into your own myTransitionCostObjectI,
// and encode the virtual getTransitionCost() function in order
// to return the value of the corresponding GetTransitionCost()
// call. This myTransitionCostObjectI should be a constant cost.
// Associate this object with the IlcUnaryResource using
// a special goal.
//
// Problem:
//
// Using either methods, associate a transition cost to the model's resource
// 'worker', and set the variable sum of costs as the one to minimize.
// For method #1, use the IloSequenceFoward goal this cost as criterion
// variable.
// For method #2 and #3, complete the functions MakeTransitionCostMethod2
// and MakeTransitionCostMethod3, called by the dedicated goal 'AddExtensions',
// in order to use ILOG Scheduler Extensions objects extracted from the
// model.
//
// Complementary exercise:
//
// Change the IloTransitionCost construction (Method #1) so that transition
// costs are used with 'Prev' variables instead of 'Next'. Check that
// solutions are the same, although propagation might be more efficient.
//
///////////////////////////////////////////////////////////////////////////////

/* duration of tasks */

IloInt durations [] = { 7, 3,  8, 3, 1, 2, 1, 2, 1, 1};

/* number of activities per house */

const IloInt nbActivities = 10;

void MakeHouse(const IloModel model,
               IloInt* durations,
               IloInt startMin,
               IloInt endMax,
               const IloUnaryResource worker)
{
  IloEnv env = model.getEnv();

  // CREATE THE ACTIVITIES ...
  IloActivity masonry   = IloActivity(env, durations[0], 1, "masonry   ");
  IloActivity carpentry = IloActivity(env, durations[1], 2, "carpentry ");
  IloActivity plumbing  = IloActivity(env, durations[2], 3, "plumbing  ");
  IloActivity ceiling   = IloActivity(env, durations[3], 4, "ceiling   ");
  IloActivity roofing   = IloActivity(env, durations[4], 5, "roofing   ");
  IloActivity painting  = IloActivity(env, durations[5], 6, "painting  ");
  IloActivity windows   = IloActivity(env, durations[6], 7, "windows   ");
  IloActivity facade    = IloActivity(env, durations[7], 8, "facade    ");
  IloActivity garden    = IloActivity(env, durations[8], 9, "garden    ");
  IloActivity moving    = IloActivity(env, durations[9], 0, "moving    ");

  // SET STARTMIN AND ENDMAX.
  model.add(masonry.startsAfter(startMin));
  model.add(moving.endsBefore(endMax));

  // POST THE PRECEDENCE CONSTRAINTS
  model.add(carpentry.startsAfterEnd(masonry));
  model.add(plumbing.startsAfterEnd(masonry));
  model.add(ceiling.startsAfterEnd(masonry));
  model.add(roofing.startsAfterEnd(carpentry));
  model.add(painting.startsAfterEnd(ceiling));
  model.add(windows.startsAfterEnd(roofing));
  model.add(facade.startsAfterEnd(roofing));
  model.add(facade.startsAfterEnd(plumbing));
  model.add(garden.startsAfterEnd(roofing));
  model.add(garden.startsAfterEnd(plumbing));
  model.add(moving.startsAfterEnd(windows));
  model.add(moving.startsAfterEnd(facade));
  model.add(moving.startsAfterEnd(garden));
  model.add(moving.startsAfterEnd(painting));

  // ADD THE RESOURCE CONSTRAINTS.
  model.add(carpentry.requires(worker));
  model.add(ceiling.requires(worker));
  model.add(roofing.requires(worker));
  model.add(windows.requires(worker));
  model.add(facade.requires(worker));
  model.add(masonry.requires(worker));
  model.add(plumbing.requires(worker));
  model.add(garden.requires(worker));
  model.add(painting.requires(worker));
  model.add(moving.requires(worker));
}

void DefineModel(IloModel model,
                 IloUnaryResource &worker)
{
  // current environment
  IloEnv env = model.getEnv();

  // CREATE A TIGHT HORIZON
  IloInt horizon = 0;
  for(IloInt i=0; i<nbActivities; ++i)
    horizon += durations[i];

  // CREATE THE UNARY RESOURCE
  worker = IloUnaryResource(env, "lonesome worker");
  worker.setSequenceEnforcement(IloHigh);

  // CREATE THE ACTIVITIES ...
  MakeHouse(model, durations, 0, horizon, worker);
}

///////////////////////////////////////////////////////////////////////////////
//
// COMMON FUNCTION DEFINING THE TRANSITION COST BETWEEN TWO TYPES
//
///////////////////////////////////////////////////////////////////////////////

IloInt GetTransitionCost(IloInt type1, IloInt type2) {
  if (!type1 || !type2 || (type1==type2))
    return 0;
  return (IloInt)IloMax(3, IloAbs((type1 - type2)/2L));
}

///////////////////////////////////////////////////////////////////////////////
//
// METHOD #1 (Using an IloTransitionParam on the unary resource)
//
///////////////////////////////////////////////////////////////////////////////

void MakeTransitionCostMethod1(const IloEnv env,
                               const IloUnaryResource worker,
                               IloIntVar& cost)
{
  // CREATE THE SYMMETRICAL TRANSITION PARAM
  IloTransitionParam transitionParam(env, nbActivities, IloTrue, "transitionParam");

  // FILL IN IT WITH THE TRANSITION COSTS
  IloInt i, j;
  for(i = OL; i<nbActivities; i++)
    for(j = OL; j<=i; ++j)
      transitionParam.setValue(i, j, GetTransitionCost(i, j));

  // ADD A "NEXT" TRANSITION COST TO worker
  IloTransitionCost transitionCost(worker, transitionParam, IloTrue);

  // BIND THE TOTAL COST TO MODEL VARIABLE cost
  cost = transitionCost.getCostSumVar();
}

///////////////////////////////////////////////////////////////////////////////
//
// METHOD #2 (Using IlcTransitionTable Scheduler Extension )
//
///////////////////////////////////////////////////////////////////////////////

void MakeTransitionCostMethod2(const IlcScheduler schedule,
                               IlcUnaryResource ilcWorker,
                               const IlcIntVar ilcCost)
{
  // FILL THE TRANSITION TABLE
  IlcTransitionTable table(schedule, nbActivities, IloTrue);
  IlcInt i, j;
  for(i = OL; i<nbActivities; i++)
    for(j = OL; j<=i; ++j)
      table.setValue(i, j, GetTransitionCost(i, j));

  // CREATE AN INSTANCE OF IlcTransitionCostObject
  IlcTransitionCostObject tcostObject = IlcMakeTransitionCost(table);

  // SET IT AS "NEXT" TRANSITION COST ON ilcWorker
  ilcWorker.addNextTransitionCost(tcostObject);

  // SET THE TOTAL COST VARIABLE TO BE ilcCost
  ilcWorker.setNextTransitionCostSum(tcostObject, ilcCost);
}

///////////////////////////////////////////////////////////////////////////////
//
// METHOD #3 (Using IlcTransitionCostObjectI Scheduler Extension )
//
///////////////////////////////////////////////////////////////////////////////

class myTransitionCostObjectI : public IlcTransitionCostObjectI
{
  public:
    myTransitionCostObjectI() : IlcTransitionCostObjectI(IlcFalse) {}

    virtual IlcInt getTransitionCost(const IlcResourceConstraint resCt1,
                                     const IlcResourceConstraint resCt2) const;
};

IlcInt
myTransitionCostObjectI::getTransitionCost(const IlcResourceConstraint resCt1,
                                           const IlcResourceConstraint resCt2) const
{
  return GetTransitionCost( resCt1.getActivity().getTransitionType(),
                            resCt2.getActivity().getTransitionType() );
}

class myTransitionCostObject : public IlcTransitionCostObject {
  public:
    myTransitionCostObject(IlcTransitionCostObjectI* impl) : IlcTransitionCostObject(impl) {}
};

void MakeTransitionCostMethod3(const IloSolver solver,
                               IlcUnaryResource ilcWorker,
                               const IlcIntVar ilcCost)
{
  // CREATE AN INSTANCE OF myTransitionCostObject
  myTransitionCostObject tcostObject(new (solver.getHeap()) myTransitionCostObjectI());

  // SET IT AS "NEXT" TRANSITION COST ON ilcWorker
  ilcWorker.addNextTransitionCost(tcostObject);

  // SET THE TOTAL COST VARIABLE TO BE ilcCost
  ilcWorker.setNextTransitionCostSum(tcostObject, ilcCost);
}

///////////////////////////////////////////////////////////////////////////////
//
// GOAL FOR USING AN IlcTransitionCostObject (Methods #2 or #3)
//
///////////////////////////////////////////////////////////////////////////////

ILOCPGOALWRAPPER3(AddExtensions, solver,
                  IloIntVar, cost,
                  IloUnaryResource, worker,
                  IloInt, method)
{
  // Get ILOG Scheduler Extension objects
  IlcScheduler schedule(solver);
  IlcIntVar ilcCost = solver.getIntVar(cost);
  IlcUnaryResource ilcWorker = schedule.getUnaryResource(worker);

  // Add transition cost
  if (method==2) MakeTransitionCostMethod2(schedule, ilcWorker, ilcCost);
  else MakeTransitionCostMethod3(solver, ilcWorker, ilcCost);

  // Return goal
  return IlcSequence(schedule, ilcCost);
}

///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void PrintSolution(const IloSolver solver,
                   const IloIntVar cost)
{
  IlcScheduler scheduler(solver);
  IloEnv env = solver.getEnv();

  env.out() << "solution with cost: " << solver.getIntVar(cost).getMin();
  env.out() << endl;

  for(IloIterator<IloActivity> act(env);
      act.ok();
      ++act) {
    // current iterated activity
    IloActivity currentActivity = *act;

    // corresponding instance in Extensions
    IlcActivity currentActivityExtensions =
      scheduler.getActivity(currentActivity);

    // edition
    env.out() << currentActivityExtensions << endl;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  IloInt method = 1;
  if (argc>1) {
    method = atoi(argv[1]);
    if (method>3 || method<1) {
      cout << "invalid argument: " << argv[1] << "." << endl;
      exit(1);
    }
  }

  try {

    // creating the environment and the model
    IloEnv env;
    IloModel model(env);

    IloIntVar cost;
    IloUnaryResource worker;
    IloGoal goal;

    // filling in the model structure
    DefineModel(model, worker);

    env.out() << "Using method #" << method << "." << endl;

    // using either method for using a transition cost
    if (method==1) {

      // method #1:
      // add transition cost in the model

      MakeTransitionCostMethod1(env, worker, cost);
      goal = IloSequenceForward(env, cost);

    }
    else {

      // method #2 or #3:
      // add the transition cost with Extensions,
      // using the dedicated AddExtensions goal.

      cost = IloIntVar(env, 0, IloIntMax);
      goal = AddExtensions(env, cost, worker, method);

    }

    // setting the objective
    model.add(IloMinimize(env, cost));

    // extracting a resolution structure
    IloSolver solver(model);

    // solve
    if ( solver.solve(goal) )
    {
      PrintSolution(solver, cost);
    }
    else {
      env.out() << "no solution!" << endl;
    }

    // detailed informations
    solver.printInformation();

    // cleaning the house ...
    solver.end();
    env.end();

  } catch(IloException &e) {
    cout << e << endl;
  }

  // ... before moving!
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// RESULTS
//
///////////////////////////////////////////////////////////////////////////////

/*
solution with cost: 24
masonry    [0 -- 7 --> 7]
carpentry  [7 -- 3 --> 10]
plumbing   [10 -- 8 --> 18]
ceiling    [19 -- 3 --> 22]
roofing    [18 -- 1 --> 19]
painting   [22 -- 2 --> 24]
windows    [27 -- 1 --> 28]
facade     [25 -- 2 --> 27]
garden     [24 -- 1 --> 25]
moving     [28 -- 1 --> 29]
*/

// end of file transitioncost.cpp
