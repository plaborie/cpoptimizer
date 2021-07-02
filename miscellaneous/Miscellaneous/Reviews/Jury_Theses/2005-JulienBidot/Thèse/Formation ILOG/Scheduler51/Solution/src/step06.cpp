#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Solution of Step 6 : Transition times
//
// Activities have 3 types:
//   type A: masonry, garden, moving
//   type B: carpentry, plumbing
//   type C: ceiling, roofing, painting, facade, windows
//  Between activities of type A and activities of type B, we need 1 day
//  Between activities of type B and activities of type C, we need 2 days
//  Between activities of type A and activities of type C, we need 3 days
//  Check the triangular inequality
//
///////////////////////////////////////////////////////////////////////////////

void DefineProblem(IloModel model,
                   IloIntVar& makespan,
                   IloArray<IloActivity>& arrayActivities)
{
  // current environment
  IloEnv env = model.getEnv();

  // CREATE THE ACTIVITIES ...
  IloActivity masonry   (env, 7, "masonry   ");
  IloActivity carpentry (env, 3, "carpentry ");
  IloActivity plumbing  (env, 8, "plumbing  ");
  IloActivity ceiling   (env, 3, "ceiling   ");
  IloActivity roofing   (env, 1, "roofing   ");
  IloActivity painting  (env, 2, "painting  ");
  IloActivity windows   (env, 1, "windows   ");
  IloActivity facade    (env, 2, "facade    ");
  IloActivity garden    (env, 1, "garden    ");
  IloActivity moving    (env, 1, "moving    ");

  // ... AND STORE THEM IN AN ARRAY
  IloInt nbActivities = 10;
  arrayActivities = IloArray<IloActivity>(env, nbActivities);
  arrayActivities[0] = masonry;
  arrayActivities[1] = carpentry;
  arrayActivities[2] = plumbing;
  arrayActivities[3] = ceiling;
  arrayActivities[4] = roofing;
  arrayActivities[5] = painting;
  arrayActivities[6] = windows;
  arrayActivities[7] = facade;
  arrayActivities[8] = garden;
  arrayActivities[9] = moving;

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


  // ADD THE RESOURCE CONSTRAINTS
  // create the Unary Resource ...
  IloUnaryResource worker(env, "lonesome worker");

  // BREAKS
  // create the week profile
  IloIntervalList breakList(env,0,42);

  // adds the week-end breaks
  breakList.addPeriodicInterval(5,2,7,breakList.getDefinitionIntervalMax());
  // affect the break list to the resource
  worker.setBreaksParam(breakList);

  // set the concerned activities to be breakable
  IloActivityBasicParam basicParam(env,"basicParam");
  basicParam.setBreakable(IloTrue);
  masonry.setActivityBasicParam(basicParam);
  plumbing.setActivityBasicParam(basicParam);

  // OVERLAPPING :
  // set other activities to can overlap one day
  IloActivityOverlapParam overlapParam(env,"overlapParam");
  overlapParam.addEndBreakOverlapType(0);
  overlapParam.setEndBreakOverlapMax(1);

  // affect it to the non-breakable and long enough activities
  for (IloInt w = 0; w < nbActivities; w++) {
    IloActivity activity = arrayActivities[w];
    if (!activity.isBreakable() && activity.getProcessingTimeVariable().getMin() > 1) {
      activity.setActivityOverlapParam(overlapParam);
    }
  }

  // requires the worker for each activity
  for (IloInt i = 0; i < nbActivities; i++)
  {
    IloActivity activity = arrayActivities[i];
    model.add(activity.requires(worker));
  }

  // TRANSITION TIMES
  // let's assume that A = 0, B = 1, C = 2
  // affect the type for each activity
  masonry.setTransitionType(0);
  garden.setTransitionType(0);
  moving.setTransitionType(0);
  carpentry.setTransitionType(1);
  plumbing.setTransitionType(1);
  ceiling.setTransitionType(2);
  roofing.setTransitionType(2);
  painting.setTransitionType(2);
  facade.setTransitionType(2);
  windows.setTransitionType(2);

  // create the symmetric transition time table and fills it
  IloTransitionParam transitionParam(env,3,1,"transitionParam");
  transitionParam.setValue(0,1,1);
  transitionParam.setValue(1,2,2);
  transitionParam.setValue(0,2,3);

  // affect it to the resource
  IloTransitionTime transitionTime(worker,transitionParam,"transitionTime");



  // CREATE THE CONSUMPTION RESOURCE
  IloDiscreteResource budget(env, 29000);
  budget.setCapacityMax(0, 15, 13000);

  // ADD THE RESOURCE CONSUMPTION CONSTRAINTS
  for (i = 0; i < nbActivities; i++)
  {
    IloActivity activity = arrayActivities[i];

    // processing time :
    // in fact, the variable is instancied
    IloIntVar varProcessingTime = activity.getProcessingTimeVariable();
    IloNum processingTime = varProcessingTime.getMin();

    // cost = $1000 per day
    IloNum cost = 1000. * processingTime;

    // create the resource constraint instance
    IloResourceConstraint cstBudget = activity.consumes(budget, cost);

    // and add it in the model
    model.add(cstBudget);
  }


  // set the makespan variable
  makespan = moving.getEndVariable();
}

///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void
PrintSolutionByArray(IloSolver solver, IloArray<IloActivity> arrayActivities)
{
  IloEnv env = solver.getEnv();
  IlcScheduler scheduler(solver);

  // iteration by indexing the array
  IloInt nbActivities = arrayActivities.getSize();
  for (IloInt i = 0; i < nbActivities; i++)
  {
    // current activity indexed
    IloActivity currentActivity = arrayActivities[i];

    // corresponding instance in extensions
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

int main()
{
  // creating the environment and the model
  IloEnv env;
  IloModel model(env);

  IloIntVar makespan;

  // filling the model structure
  IloArray<IloActivity> arrayActivities;
  DefineProblem(model, makespan, arrayActivities);

  // defining the objective
  model.add(IloMinimize(env, makespan));

  // extracting a resolution structure
  IloSolver solver(model);

  // solve the problem,
  solver.solve();

  // edition
  PrintSolutionByArray(solver, arrayActivities);

  // cleaning the house ...
  solver.end();
  env.end();

  // ... before moving!
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// RESULTS
//
///////////////////////////////////////////////////////////////////////////////

/*
masonry    [0 -- (7) 9 --> 9]
carpentry  [10 -- 3 --> 13]
plumbing   [15 -- (8) 10 --> 25]
ceiling    [28 -- 3 --> 31]
roofing    [31 -- 1 --> 32]
painting   [32 -- 2 --> 34]
windows    [35 -- 1 --> 36]
facade     [36 -- 2 --> 38]
garden     [42 -- 1 --> 43]
moving     [43 -- 1 --> 44]
*/

// end of file step06.cpp

