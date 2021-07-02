#include <ilsched/iloscheduler.h>


ILOSTLBEGIN


// Scheduler does not find a solution to this problem whatever the resource constraint propagation level because there is a problem with the precedence graph.


////////////////////////////////////////////////////////////////////////////////////////
/////
/////		MAIN
/////
/////
////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	try {
		IloEnv env;
		IloNumVar makespan(env, 0.0, 100.0, ILOINT);
		IloModel model(env);
		IloSchedulerEnv schedEnv(env);
		schedEnv.getResourceParam().setCapacityEnforcement(IloMediumLow); // Edge-finder, propagation of resource constraints

		IloUnaryResource resource(env);
		IloActivity activityA(env, 10);
		IloActivity activityB(env, 20);
		IloActivity activityC(env, 30);

		IloPrecedenceConstraint tCt1 = activityB.startsAfterEnd(activityA);
		model.add(tCt1);
		IloPrecedenceConstraint tCt2 = activityC.startsAfterEnd(activityB);
		model.add(tCt2);
		
		IloResourceConstraint rCt1 = activityA.requires(resource);
		model.add(rCt1);

		IloResourceConstraint rCt2 = activityB.requires(resource);
		rCt1.setSuccessor(rCt2);
		model.add(rCt2);

		IloResourceConstraint rCt3 = activityC.requires(resource);
		rCt2.setSuccessor(rCt3);
		model.add(rCt3);

		IloSolver solver(model);
		IloGoal goal = IloRankForward(env,
									  IloSelResMinGlobalSlack,
									  IloSelFirstRCMinStartMax);
		if(solver.solve(goal))
			printf("A solution has been found.\n");
	//	IloNum bestMakespan;
	//	IlcScheduler sched(solver);
	//	if(solver.next())
		env.end();
			

	} catch (IloException& e) {
		cout << e << endl;
	}
	
	return 0;
}