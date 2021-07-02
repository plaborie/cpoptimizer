#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step Opt03a: Alternative Resources:
//
// Three different workers that build a house (Solution)
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
  IloUnaryResource jim(env, "Jim");
  jim.setInitialOccupation(15, 17, 1);
  IloUnaryResource joe(env, "Joe");
  IloUnaryResource jack(env, "Jack");
  IloAltResSet theWorkers(env, 3, jim, joe, jack);

  // ... and require it for each activity
  IloArray<IloAltResConstraint> alternatives(env, nbActivities);
  for (IloInt i = 0; i < nbActivities; i++)
  {
    IloActivity activity = arrayActivities[i];
    IloAltResConstraint alternative = activity.requires(theWorkers);
	alternatives[i]=alternative;
    model.add(alternative);
  }

  // add the constraints about the alternatives
  // step a
  alternatives[0].setRejected(jim);
  alternatives[2].setSelected(jim);

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
    for (IlcAltResConstraintIterator altIte(currentActivityExtensions);
         altIte.ok(); ++altIte) {
      env.out() << (*altIte ).getSelected();
      break;
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

/* Step a
masonry    [0 -- 7 --> 7] is executed by Joe [1]
carpentry  [7..9 -- 3 --> 10..12] is executed by Jack [1]
plumbing   [7 -- 8 --> 15] is executed by Jim [1]
ceiling    [7..9 -- 3 --> 10..12] is executed by Joe [1]
roofing    [10..12 -- 1 --> 11..13] is executed by Joe [1]
painting   [11..13 -- 2 --> 13..15] is executed by Joe [1]
windows    [13..15 -- 1 --> 14..16] is executed by Joe [1]
facade     [15 -- 2 --> 17] is executed by Jack [1]
garden     [15..16 -- 1 --> 16..17] is executed by Joe [1]
moving     [17 -- 1 --> 18] is executed by Jim [1]
*/
