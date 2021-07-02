#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step 9: Reservoirs
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

  // To be filled in.
  model.add(masonry.consumes(budget,masonry.getProcessingTimeVariable().getMax()));
  model.add(carpentry.consumes(budget,carpentry.getProcessingTimeVariable().getMax()));
  model.add(plumbing.consumes(budget,plumbing.getProcessingTimeVariable().getMax()));
  model.add(ceiling.consumes(budget,ceiling.getProcessingTimeVariable().getMax()));
  model.add(painting.consumes(budget,painting.getProcessingTimeVariable().getMax()));
  model.add(windows.consumes(budget,windows.getProcessingTimeVariable().getMax()));
  model.add(facade.consumes(budget,facade.getProcessingTimeVariable().getMax()));
  model.add(garden.consumes(budget,garden.getProcessingTimeVariable().getMax()));
  model.add(roofing.consumes(budget,roofing.getProcessingTimeVariable().getMax()));
  model.add(moving.consumes(budget,moving.getProcessingTimeVariable().getMax()));

  model.add(masonry.produces(budget,masonry.getProcessingTimeVariable().getMax()+1));
  model.add(carpentry.produces(budget,carpentry.getProcessingTimeVariable().getMax()+1));
  model.add(plumbing.produces(budget,plumbing.getProcessingTimeVariable().getMax()+1));
  model.add(ceiling.produces(budget,ceiling.getProcessingTimeVariable().getMax()+1));
  model.add(painting.produces(budget,painting.getProcessingTimeVariable().getMax()+1));
  model.add(windows.produces(budget,windows.getProcessingTimeVariable().getMax()+1));
  model.add(facade.produces(budget,facade.getProcessingTimeVariable().getMax()+1));
  model.add(garden.produces(budget,garden.getProcessingTimeVariable().getMax()+1));
  model.add(roofing.produces(budget,roofing.getProcessingTimeVariable().getMax()+1));
  model.add(moving.produces(budget,moving.getProcessingTimeVariable().getMax()+1));
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
  // note that the resources are, by default, closed at extraction
  // in Solver
  // To be filled in.

  IloReservoir budget(env,IloInfinity,11);
  

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
  
  // Ranking is efficient only if unary resources are used.
  // Because of the reservoir, the solution must assign the
  // time to each activity.
  // To optimize the makespan, use a forward goal
  IloGoal goal = IloSetTimesForward(env, makespan);/***** To be filled in *****/ 

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
To be filled in
*/

// end of file step09.cpp