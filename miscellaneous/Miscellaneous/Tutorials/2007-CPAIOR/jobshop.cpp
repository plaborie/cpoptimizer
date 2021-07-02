#include <ilsched/iloscheduler.h>

IloInt NumberOfJobs      = 6;
IloInt NumberOfResources = 6;

IloInt ResourceNumbers [] = {2, 0, 1, 3, 5, 4,
			     1, 2, 4, 5, 0, 3,
			     2, 3, 5, 0, 1, 4,
			     1, 0, 2, 3, 4, 5,
			     2, 1, 4, 5, 0, 3,
			     1, 3, 5, 0, 4, 2};

IloInt Durations [] = { 1,  3,  6,  7,  3,  6,
			8,  5, 10, 10, 10,  4,
			5,  4,  8,  9,  1,  7,
			5,  5,  5,  3,  8,  9,
			9,  3,  5,  4,  3,  1,
			3,  3,  9, 10,  4,  1};

int main() {
  IloEnv env;
  IloModel model(env);
  IloIntVar makespan(env, 0, IloIntMax);
  IloInt i, j, k =0;
  IloUnaryResourceArray machines(env, NumberOfResources);
  for (j = 0; j < NumberOfResources; j++)
    machines[j] = IloUnaryResource(env);
  for (i = 0; i < NumberOfJobs; i++) {
    IloActivity prevAct;
    for (j = 0; j < NumberOfResources; j++) {
      IloActivity act(env, Durations[k]);
      model.add(act.requires(machines[ResourceNumbers[k]]));
      if (j != 0)
        model.add(act.startsAfterEnd(prevAct));
      prevAct = act;
      k++;
    }
    model.add(prevAct.endsBefore(makespan));
  }
  model.add(IloMinimize(env, makespan));
  IloSolver cp(model);
  cp.solve(IloRankForward(env, makespan));
  cout << "Optimal makespan = " << cp.getValue(makespan) << endl;
  env.end();
}


