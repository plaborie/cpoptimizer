#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step Opt03a: Alternative Resources
//
// Three different workers build a house (a)
//
///////////////////////////////////////////////////////////////////////////////

void
DefineProblem(IloModel& model, IloIntVar& makespan,
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
  // create the resource set ...
  /*  to be filled in ... */
  IloUnaryResource jim(env,"Jim");
  jim.setInitialOccupation(15,17,1);
  IloUnaryResource joe(env,"Joe");
  IloUnaryResource jack(env,"Jack");

  IloAltResSet workers(env,3,jim,joe,jack);

  // ... and require it for each activity
  /*  to be filled in ... */
  // add the constraints about the alternatives
  // step a
  /*  to be filled in ... */
  for(IloInt i = 0 ; i < nbActivities ; i++) {
	  IloAltResConstraint contrainte = arrayActivities[i].requires(workers,1);
	  if (i == 0)
	    contrainte.setRejected(jim);
      if (i == 2)
	    contrainte.setSejected(jim);

	  model.add(contrainte);
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
PrintSolutionByArray(IloSolver& solver, IloArray<IloActivity>& arrayActivities)
{
  IloEnv env = solver.getEnv();
  IlcScheduler scheduler(solver);

  // iteration by indexing the array
  IloInt nbActivities = arrayActivities.getSize();
  for (IloInt i = 0; i < nbActivities; i++)
  {
    // current activity indexed
    IloActivity currentActivity = arrayActivities[i];
    IlcActivity currentActivityExtensions =
      scheduler.getActivity(currentActivity);

    // edition
    env.out() << currentActivityExtensions << " is executed by ";

    // search for the resource used by the current activity
    /*  to be fill in ... */
	for (IlcAltResConstraintIterator it(currentActivityExtensions); it.ok(); ++it) {
		if ( (*it).isResourceSelected() ) {
			IlcResource ilcRes = (*it).getSelected();
			env.out() << ilcRes.getName() << endl;
		}
	}



    env.out() << endl;
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

	// adding the objective
	model.add(IloMinimize(env, makespan));

	// extracting a resolution structure
	IloSolver solver(model);

	// defining the goal and solving
    /*  to be fill in ... */
	IloGoal goal = IloAssignAlternative(env) && IloRankForward(env, makespan);
	solver.solve(goal);
    
	// editions
	PrintSolutionByArray(solver, arrayActivities);
	solver.printInformation();

	// end
	solver.end();
	env.end();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// RESULTS
//
///////////////////////////////////////////////////////////////////////////////

/*  to be filled in ... */
