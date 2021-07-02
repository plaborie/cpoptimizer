#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step 4: Resource Constraint
//
// Besides a worker, the building of the house requires an amount of money
// for each activity (rate $1,000 per day).
// For the first 15 days we only have a limited budget ($13,000).
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

  // .. and require it for each activity
  for (IloInt i = 0; i < nbActivities; i++)
  {
	  IloActivity activity = arrayActivities[i];
	  model.add(activity.requires(worker));
  }

  // CREATE THE CONSUMPTION RESOURCE
  /********* to be filled in ... *********/
  IloDiscreteResource budget(env,29000);
  budget.setCapacityMax(0,15,13000);

  // ADD THE RESOURCE CONSUMPTION CONSTRAINTS
  for (i = 0; i < nbActivities; i++)
  {
    IloActivity activity = arrayActivities[i];

    // compute the cost of the activity: cost = $1000 per day
    /********* to be filled in ... *********/
	IloIntVar processingTime = activity.getProcessingTimeVariable();
	IloNum cost = 1000. * processingTime.getMax();/********* to be filled in ... *********/
	

    // create the resource constraint instance
    
	// and add it to the model
    /********* to be filled in ... *********/
	
	model.add(activity.consumes(budget, cost));
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

	// now whe want to solve the problem,
	// because introduction of the resource creates disjunctive constraints,
	// temporal propagation is not complete!
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

  /********* to be filled in ... *********/

// end of file step04.cpp

