#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
//
// Solution of step Opt01: Printing solutions with IloSchedulerSolution
//
//  Store and print the solution of Step 3
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

  // initial occupation
  IloNumToNumStepFunction weekProfile(env, 0,7,0,"weekProfile");
  weekProfile.setValue(5,7,1);
  IloNumToNumStepFunction yearProfile(env,0,14,0,"yearProfile");
  yearProfile.setPeriodic(weekProfile,0);
  worker.setInitialOccupationParam(yearProfile);

  // .. and requires it for each activity
  for (IloInt i = 0; i < nbActivities; i++)
  {
	  IloActivity activity = arrayActivities[i];
	  model.add(activity.requires(worker));
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
PrintSolution(IloSchedulerSolution solution)
{
  IloEnv env = solution.getEnv();

  // iteration with solution iterator
  for (IloSchedulerSolution::ActivityIterator ite(solution);
       ite.ok();
       ++ite)
  {
    IloActivity activity = *ite;
    env.out() << solution.getStartMin(activity)
      << ": " << activity.getName() << endl;
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

	// creating the scheduler solution
	IloSchedulerSolution solution(env);
	IloInt nbActivities = arrayActivities.getSize();
	for (IloInt i = 0; i < nbActivities; i++)
	{
  	  solution.add(arrayActivities[i]);
	}

	// extracting a resolution structure
	IloSolver solver(model);

	// now we want to solve the problem,
	// because introduction of the resource creates disjunctive constraints,
	// temporal propagation is not complete!
	solver.solve();

	// editing
	solution.store(IlcScheduler(solver));
	PrintSolution(solution);

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
14: masonry
21: carpentry
24: plumbing
32: ceiling
35: roofing
36: painting
38: windows
39: facade
41: garden
42: moving
*/

// end of file

