#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Solution of Step 9: Reservoirs.
//
// Five houses of different types has to be built. We add the fact that
// each activity costs an investment of $1,000 per day. We also say
// that we make a profit of $1,000 for each activity at the end of
// the activity. So if an activity  has a duration of D, it uses
// D x $1,000 when it is started and makes (D+1) x $1,000 when
// it is finished. We start with $11,000.
//
///////////////////////////////////////////////////////////////////////////////


/* Three types of houses each having different task durations */

IloInt durationsType1 [] = { 7, 3,  8, 3, 1, 2, 1, 2, 1, 1};
IloInt durationsType2 [] = {12, 5, 10, 5, 2, 5, 2, 3, 2, 1};
IloInt durationsType3 [] = {15, 3, 10, 6, 2, 3, 2, 3, 2, 1};

void makeHouse(IloModel model,
	       IloInt* dur,
	       IloInt startMin,
	       IloInt endMax,
	       IloArray<IloUnaryResource> workers,
	       IloReservoir budget,
	       IloIntVar makespan) 
{
  IloEnv env = model.getEnv();
  
  /* CREATE THE ACTIVITIES. */
  IloActivity masonry(env, dur[0], "masonry   ");
  IloActivity carpentry(env, dur[1], "carpentry ");
  IloActivity plumbing(env, dur[2], "plumbing  ");
  IloActivity ceiling(env, dur[3], "ceiling   ");
  IloActivity roofing(env, dur[4], "roofing   ");
  IloActivity painting(env, dur[5], "painting  ");
  IloActivity windows(env, dur[6], "windows   ");
  IloActivity facade(env, dur[7], "facade    ");
  IloActivity garden(env, dur[8], "garden    ");
  IloActivity moving(env, dur[9], "moving    ");
  
  /* SET STARTMIN AND ENDMAX. */
  model.add(masonry.startsAfter(startMin));
  model.add(moving.endsBefore(endMax));


  /* POST THE TEMPORAL CONSTRAINTS. */
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

  model.add(moving.endsBefore(makespan));

  /* POST THE RESOURCE CONSTRAINTS ON THE WORKERS. */ 
  model.add(carpentry.requires(workers[0]));
  model.add(ceiling.requires(workers[0]));
  model.add(roofing.requires(workers[0]));
  model.add(windows.requires(workers[0]));
  model.add(facade.requires(workers[0]));

  model.add(masonry.requires(workers[1]));

  model.add(plumbing.requires(workers[2]));

  model.add(garden.requires(workers[3]));
  model.add(painting.requires(workers[3]));
  model.add(moving.requires(workers[3]));

  /* POST THE RESOURCE CONSTRAINTS ON THE BUDGET. */ 
  model.add(masonry.consumes(budget, dur[0]));
  model.add(carpentry.consumes(budget, dur[1]));
  model.add(plumbing.consumes(budget, dur[2]));
  model.add(ceiling.consumes(budget, dur[3]));
  model.add(roofing.consumes(budget, dur[4]));
  model.add(painting.consumes(budget, dur[5]));
  model.add(windows.consumes(budget, dur[6]));
  model.add(facade.consumes(budget, dur[7]));
  model.add(garden.consumes(budget, dur[8]));
  model.add(moving.consumes(budget, dur[9]));

  model.add(masonry.produces(budget, (dur[0] + 1)));
  model.add(carpentry.produces(budget, (dur[1] + 1)));
  model.add(plumbing.produces(budget, (dur[2] + 1)));
  model.add(ceiling.produces(budget, (dur[3] + 1)));
  model.add(roofing.produces(budget, (dur[4] + 1)));
  model.add(painting.produces(budget, (dur[5] + 1)));
  model.add(windows.produces(budget, (dur[6] + 1)));
  model.add(facade.produces(budget, (dur[7] + 1)));
  model.add(garden.produces(budget, (dur[8] + 1)));
  model.add(moving.produces(budget, (dur[9] + 1)));
}

void DefineProblem(IloModel model, 
		   IloIntVar& makespan)
{
  /* CREATE THE SCHEDULE. */
  IloInt horizon = 150;
  IloEnv env = model.getEnv();
  IloSchedulerEnv schedEnv(env);
  schedEnv.setOrigin(0);
  schedEnv.setHorizon(horizon);

  /* CREATE THE MAKESPAN VARIABLE. */
  makespan = IloIntVar(env, 0, horizon);
  
  /* CREATE THE WORKERS. */
  IloInt nbOfWorkers = 4;
  IloArray<IloUnaryResource> workers(env, nbOfWorkers);
  schedEnv.getResourceParam().setCapacityEnforcement(IloMediumHigh);
  IloInt k;

  for (k = 0; k < nbOfWorkers; k++) {
    workers[k] = IloUnaryResource(env);
  }

  /* CREATE THE BUDGET RESOURCE. */
  IloReservoir budget(env, IloIntMax, 11);

  /* CREATE THE ACTIVITIES AND CONSTRAINTS FOR THE HOUSES. */
  makeHouse(model, durationsType1, 0, horizon, workers, budget, makespan);
  makeHouse(model, durationsType1, 0, horizon, workers, budget, makespan);
  makeHouse(model, durationsType2, 0, horizon, workers, budget, makespan);
  makeHouse(model, durationsType2, 0, horizon, workers, budget, makespan);
  makeHouse(model, durationsType3, 0, horizon, workers, budget, makespan);
  
}

///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void PrintSolution(const IloSolver solver, const IloIntVar makespan)
{
  IlcScheduler scheduler(solver);
  IloEnv env = solver.getEnv();
  env.out() << "Solution with makespan " 
            << solver.getIntVar(makespan).getMin() << endl;
  for(IloIterator<IloActivity> act(env);
      act.ok();
      ++act)
    env.out() << scheduler.getActivity(*act) << endl;
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
  DefineProblem(model, makespan);
  
  // defining the objective
  model.add(IloMinimize(env, makespan));

  // extracting a resolution structure
  IloSolver solver(model);

  // SETTING TIME GOAL
  IloGoal goal = IloSetTimesForward(env, makespan);
  if (solver.solve(goal))
    PrintSolution(solver, makespan);
  else
    solver.out() << "No Solution" << endl;
  solver.end();
  env.end();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// RESULTS
//
///////////////////////////////////////////////////////////////////////////////

/*
Solution with makespan 81
masonry    [7 -- 7 --> 14]
carpentry  [17 -- 3 --> 20]
plumbing   [31 -- 8 --> 39]
ceiling    [14 -- 3 --> 17]
roofing    [20 -- 1 --> 21]
painting   [19 -- 2 --> 21]
windows    [22 -- 1 --> 23]
facade     [42 -- 2 --> 44]
garden     [39 -- 1 --> 40]
moving     [45 -- 1 --> 46]
masonry    [0 -- 7 --> 7]
carpentry  [7 -- 3 --> 10]
plumbing   [21 -- 8 --> 29]
ceiling    [11 -- 3 --> 14]
roofing    [10 -- 1 --> 11]
painting   [17 -- 2 --> 19]
windows    [21 -- 1 --> 22]
facade     [31 -- 2 --> 33]
garden     [29 -- 1 --> 30]
moving     [38 -- 1 --> 39]
masonry    [44 -- 12 --> 56]
carpentry  [62 -- 5 --> 67]
plumbing   [59 -- 10 --> 69]
ceiling    [57 -- 5 --> 62]
roofing    [67 -- 2 --> 69]
painting   [62 -- 5 --> 67]
windows    [78 -- 2 --> 80]
facade     [75 -- 3 --> 78]
garden     [69 -- 2 --> 71]
moving     [80 -- 1 --> 81]
masonry    [14 -- 12 --> 26]
carpentry  [33 -- 5 --> 38]
plumbing   [39 -- 10 --> 49]
ceiling    [26 -- 5 --> 31]
roofing    [38 -- 2 --> 40]
painting   [40 -- 5 --> 45]
windows    [40 -- 2 --> 42]
facade     [72 -- 3 --> 75]
garden     [49 -- 2 --> 51]
moving     [75 -- 1 --> 76]
masonry    [29 -- 15 --> 44]
carpentry  [50 -- 3 --> 53]
plumbing   [49 -- 10 --> 59]
ceiling    [44 -- 6 --> 50]
roofing    [53 -- 2 --> 55]
painting   [51 -- 3 --> 54]
windows    [55 -- 2 --> 57]
facade     [69 -- 3 --> 72]
garden     [59 -- 2 --> 61]
moving     [72 -- 1 --> 73]
*/

// end of file step9.cpp