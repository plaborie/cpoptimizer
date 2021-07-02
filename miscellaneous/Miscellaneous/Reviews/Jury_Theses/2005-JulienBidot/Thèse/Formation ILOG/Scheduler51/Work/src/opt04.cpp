#include <ilsched/iloscheduler.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// Step Opt04: State Resources.
//
// Introduction of meteorological conditions:
// Temperature can be COLD, HOT or  MILD
// Sunlight can be SUNNY, CLOUDY or RAINY
//
// Meteorological forecast :
//
// Day 0         -> MILD and RAINY
// Day 1 to 10   -> MILD and CLOUDY
// Days 11 to 17 -> COLD and CLOUDY
// Day 18        -> MILD and CLOUDY
// Days 19 to 21 -> MILD and RAINY
// Days 22 to 26 -> MILD and CLOUDY
// Days 27 to 30 -> MILD and SUNNY
// Day 31        -> HOT  and SUNNY
// Days 32 to 36 -> MILD and SUNNY
//
//
// Activities requirements:
//
//                   temperature          sunlight
//  ---------------------------------------------------
//  masonry          MILD                 not RAINY
//  carpentry        MILD                 not RAINY
//  roofing          MILD                 not RAINY
//  plumbing         not COLD             indifferent
//  ceiling          indifferent          indifferent
//  windows          indifferent          indifferent
//  facade           not HOT              SUNNY
//  garden           indifferent          not RAINY
//  painting         not HOT              not RAINY
//  moving           not HOT              not RAINY
//
///////////////////////////////////////////////////////////////////////////////

/* DEFINE THE POSSIBLE WEATHER CONDITIONS */
/* FOR TEMPERATURE ... */
#define COLD   (IloAny)1
#define MILD   (IloAny)2
#define HOT    (IloAny)3

/* ... AND FOR FOR SUNLIGHT */
#define SUNNY  (IloAny)1
#define CLOUDY (IloAny)2
#define RAINY  (IloAny)3

void
DefineProblem(IloModel model, IloIntVar& makespan,
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
  for (IloInt i = 0; i < nbActivities; i++) {
    IloActivity activity = arrayActivities[i];
    model.add(activity.requires(worker));
  }

  // CREATE THE WEATHER STATE RESOURCES
  /* To be filled in */
  IloAnySet temp(env);
  temp.add(COLD);
  temp.add(MILD);
  temp.add(HOT);
  IloStateResource temperature(env,temp);

  IloAnySet sun(env);
  sun.add(SUNNY);
  sun.add(CLOUDY);
  sun.add(RAINY);
  IloStateResource sunlight(env,sun);


  // DEFINE WEATHER FORECAST
  /* To be filled in */
  temperature.removePossibleStates(0,37,temp);
  sunlight.removePossibleStates(0,37,sun);

  temperature.addPossibleState(0,11,MILD);
  temperature.addPossibleState(11,18,COLD);
  temperature.addPossibleState(18,31,MILD);
  temperature.addPossibleState(31,32,HOT);
  temperature.addPossibleState(32,37,MILD);

  sunlight.addPossibleState(0,1,RAINY);
  sunlight.addPossibleState(1,19,CLOUDY);
  sunlight.addPossibleState(19,22,RAINY);
  sunlight.addPossibleState(22,27,CLOUDY);
  sunlight.addPossibleState(27,37,SUNNY);




  // ADD THE WEATHER CONSTRAINTS
  /* To be filled in */
  model.add(masonry.requires(temperature,MILD));
  model.add(masonry.requiresNot(sunlight,RAINY));
  
  model.add(carpentry.requires(temperature,MILD));
  model.add(carpentry.requiresNot(sunlight,RAINY));
  
  model.add(roofing.requires(temperature,MILD));
  model.add(roofing.requiresNot(sunlight,RAINY));
  
  model.add(plumbing.requiresNot(temperature,COLD));
   
  model.add(facade.requiresNot(temperature,HOT));
  model.add(facade.requires(sunlight,SUNNY));
  
  model.add(garden.requiresNot(sunlight,RAINY));
  
  model.add(painting.requiresNot(temperature,HOT));
  model.add(painting.requiresNot(sunlight,RAINY));
  
  model.add(moving.requiresNot(temperature,HOT));
  model.add(moving.requiresNot(sunlight,RAINY));



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
  for (IloInt i = 0; i < nbActivities; i++) {
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

  // filling in the model structure
  IloArray<IloActivity> arrayActivities;
  DefineProblem(model, makespan, arrayActivities);

  // defining the objective
  model.add(IloMinimize(env, makespan));

  // extracting a resolution structure
  IloSolver solver(model);

  // now we solve the problem,
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
  // To be filled in
*/

// end of file opt04.cpp

