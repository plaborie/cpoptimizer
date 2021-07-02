#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step 1: Activities (cpert)
//
// Building a house comprises different tasks.
// Some tasks must be completed before others begin.
//
// We want to :
//   * know the minimal completion time of the whole process.
//   * have the time interval within which each task is to be processed.
//
///////////////////////////////////////////////////////////////////////////////

void
DefineProblem(IloModel& model, IloIntVar& makespan,
			  IloArray<IloActivity>& arrayActivities)
{
  // current environment
  IloEnv env = model.getEnv();

  // CREATE THE ACTIVITIES ...
  /********* to be filled in ... *********/
  IloActivity masonry(env,7);
  IloActivity carpentery(env,3);
  IloActivity roofing(env,1);
  IloActivity plumbing(env,8);
  IloActivity ceiling(env,3);
  IloActivity windows(env,1);
  IloActivity facade(env,2);
  IloActivity garden(env,1);
  IloActivity painting(env,2);
  IloActivity movingin(env,1);
	



  // ... AND STORE THEM IN AN ARRAY
  IloInt nbActivities = 10;
  arrayActivities =  IloArray<IloActivity>(env,nbActivities); /********* to be filled in ... *********/
   /********* to be filled in ... *********/
  arrayActivities[0] = masonry;
  arrayActivities[1] = carpentery;
  arrayActivities[2] = roofing;
  arrayActivities[3] = plumbing;
  arrayActivities[4] = ceiling;
  arrayActivities[5] = windows;
  arrayActivities[6] = facade;
  arrayActivities[7] = garden;
  arrayActivities[8] = painting;
  arrayActivities[9] = movingin;



  // POST THE PRECEDENCE CONSTRAINTS
  /********* to be filled in ... *********/
  model.add(carpentery.startsAfterEnd(masonry));
  model.add(roofing.startsAfterEnd(carpentery));
  model.add(plumbing.startsAfterEnd(masonry));
  model.add(ceiling.startsAfterEnd(masonry));
  model.add(windows.startsAfterEnd(roofing));
  model.add(facade.startsAfterEnd(roofing));
  model.add(facade.startsAfterEnd(plumbing));
  model.add(garden.startsAfterEnd(roofing));
  model.add(garden.startsAfterEnd(plumbing));
  model.add(painting.startsAfterEnd(ceiling));
  model.add(movingin.startsAfterEnd(windows));
  model.add(movingin.startsAfterEnd(facade));
  model.add(movingin.startsAfterEnd(garden));
  model.add(movingin.startsAfterEnd(painting));



  // set the makespan variable
  makespan = movingin.getEndVariable();  /********* to be filled in ... *********/
  IloObjective myObjective(IloMinimize(env,makespan));
  model.add(myObjective);

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
  /********* to be filled in ... *********/
  IloEnv env;
  IloModel model(env);


  IloIntVar makespan;
  IloArray<IloActivity> arrayActivities;

  // filling the model structure
  DefineProblem(model, makespan, arrayActivities);

  // extracting a resolution structure
  IloSolver solver(model);

  // (cf header) we don't want to completely solve the problem,
  // i.e. instantiate every date of activities,
  // but only propagate the effects of the precedence constraints
  // on the minimum value of the makespan
  solver.solve(  IloInstantiate(env,makespan)/********* to be filled in ... *********/);

  // edition
  // using customized structure
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


// end of file step01.cpp

