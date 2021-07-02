// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilubeta.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the FindInitialSolutionBeta function, the DefineModelBeta function.


#include <ilsim/ilubeta.h>
#include <ilsim/iluprocessplan.h>
#include <ilsim/iluneighborhood.h>
#include <ilsim/iluprintsolution.h>


ILOSTLBEGIN



void FindInitialSolutionBeta(IloModel						model,
							 IloSchedulerSolution			globalSolution,
							 IloSchedulerSolution			lSSolution,
							 IloNumVar						tardiCostVar,
							 IluHeuristic					heuristic,
							 IloArray<IloActivityArray>&	actArrays,
							 IloNum							globalSearchTimeLimit,
							 IloInt							globalSearchFailLimit
							 ) {
	IloEnv env = model.getEnv();
//	model.add(IloMinimize(env, tardiCostVar));
	IloSolver solver(model);
	solver.solve(IloPrintLowerBound(env, tardiCostVar)&&IloProcPlanEndMin(env, actArrays));

	IlcScheduler scheduler(solver);

	IloNum bestVal = IloInfinity;

	IloGoal g = 
//				IloAssignAlternative(env)
//				&&
				IloMyMinimizeFloatVar(env, tardiCostVar, bestVal)
				&&
				IloMyAssignAlternativeAndRank(env, heuristic) 
//				IloRankForward(env)
				&&
//				&& IloSetTimesForward(env) 
//				&& IloDichotomize(env, tardiCostVar, IloTrue)
				IloMyInstantiate(env, tardiCostVar)
				;

//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	g = IloApply(env, g, myNodeEvaluator);

//	g = IloLimitSearch(env, g, IloTimeLimit(env, globalSearchTimeLimit));
	if(globalSearchFailLimit < IloIntMax)
		g = IloLimitSearch(env, g, IloFailLimit(env, globalSearchFailLimit));

//	solver.setRelativeOptimizationStep(0.05);
	solver.setOptimizationStep(1.0);	

/*	solver.startNewSearch(g);

	while(solver.next()) {
//		IloNum best = solver.getMin(tardiCostVar);
		printf("*** Initial solution at tardiness cost = ");
		if(tardiCostVar.getType() == IloNumVar::Int)
			printf("%ld", solver.getIntVar(tardiCostVar).getMax());
		else
			printf("%.14f", solver.getFloatVar(tardiCostVar).getMax());
		printf("\n");
//		solver.out() << "*** Initial solution at tardiness cost: " <<  best << endl;
//		solver.printInformation();
		globalSolution.store(scheduler);
		lSSolution.store(scheduler);
	}
//	else
//		printf("No solution found.\n");
	solver.endSearch();*/
	// FAST RESTARTS WITH A SMALL FAIL LIMIT
	IloNum timeTemp3 = 0.0;
	while(timeTemp3 < globalSearchTimeLimit) {
		IloNum timeTemp = env.getTime();
		if(solver.solve(g)) {
			printf("*** Initial solution at cost = ");
			printf("%.14f\tat time %.3f\n", solver.getFloatVar(tardiCostVar).getMax(), timeTemp3 + env.getTime() - timeTemp);
			bestVal = solver.getFloatVar(tardiCostVar).getMax();
		    globalSolution.store(scheduler);
			lSSolution.store(scheduler);
		}
//		else
//			printf("No solution found.\n");
		timeTemp3 += env.getTime() - timeTemp;
	}
	solver.end();
}




IloNum
SolveModelBeta(IloModel						model,
			   IloNumVar					tardiCostVar,
			   IloArray<IloActivityArray>	actArrays,
			   IloSchedulerSolution&		globalSolution,
			   IloNum						globalSearchTimeLimit,
			   IloInt						globalSearchFailLimit,
			   IloNum						localSearchTimeLimit,
			   IloInt						localSearchFailLimit) {


	IloEnv env = model.getEnv();
	IloSolver lSSolver(model);
	IlcScheduler lSScheduler(lSSolver);
	IloRandom randGenerator(env, 1);

	// CREATE LOCAL SEARCH SOLUTION
	IloSchedulerSolution lSSolution = CreateLSSolution(env, globalSolution);


	// SEARCH HEURISTIC
	IloPredicate<IlcAltResConstraint> assignAltRCPredicate = !IlcAltResConstraintResourceSelectedPredicate(lSSolver);
	// This function returns an alternative resource constraint predicate whose operator(const IlcAltResConstraint& altrc) returns IlcTrue if and only if
	// a single resource has been selected for the activity corresponding to altrc. If there are still multiple resources that can be selected, IlcFalse
	// is returned.
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcAltResConstraintNbPossibleEvaluator(lSSolver).makeLessThanComparator();
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcAltResConstraintNbPossibleEvaluator(lSSolver),
																									IlcMyAltResConstraintRandomEvaluator(lSSolver, randGenerator));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcAltResConstraintNbPossibleEvaluator(lSSolver),
//																									IlcMyAltResConstraintAddressEvaluator(lSSolver));

	IloComparator<IlcResource> assignResComparator = IlcMyResourceRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
//	IloComparator<IlcResource> assignResComparator = IlcMyResourceAddressEvaluator(lSSolver).makeLessThanComparator();
		
	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(lSSolver);
	// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
	// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

	IloComparator<IlcResource> rankResComparator = IloComposeLexical(IlcMyRankResourceEvaluator(lSSolver), IlcMyResourceRandomEvaluator(lSSolver, randGenerator));
//	IloComparator<IlcResource> rankResComparator = IloComposeLexical(IlcMyRankResourceEvaluator(lSSolver), IlcMyResourceAddressEvaluator(lSSolver));
//	IloComparator<IlcResource> rankResComparator = IlcMyRankResourceEvaluator(lSSolver).makeLessThanComparator();

	IloPredicate<IlcResourceConstraint> rankRCPredicate = IlcResourceConstraintPossibleFirstPredicate(lSSolver);
	// This function returns a resource constraint predicate whose operator(const IlcResourceConstraint& rc) returns IlcTrue if and only if rc can be ranked
	// first among the non-ranked resource constraints. In particular, it returns IlcFalse if rc is already ranked or if rc represents a virtual source
	// or sink node (empty handle).
	
	IloComparator<IlcResourceConstraint> rankRCComparator = IloComposeLexical(-IlcMyRankProcPlanCostEvaluator(lSSolver),
																					IlcMyResourceConstraintStartMinEvaluator(lSSolver),
																					IlcMyResourceConstraintRandomEvaluator(lSSolver, randGenerator));
	// We rank first the resource constraint that is the most critical with respect to the process plan due date.

	IluHeuristic heuristic;
	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
	heuristic.setAssignAltRComparator(assignResComparator);
	heuristic.setRankResPredicate(rankResPredicate);
	heuristic.setRankResComparator(rankResComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);

	// GENERATE AN INITIAL SOLUTION.
	globalSolution.getSolution().add(tardiCostVar);
	IloNum timeTemp4 = env.getTime();
	FindInitialSolutionBeta(model, globalSolution, lSSolution, tardiCostVar, heuristic, actArrays, globalSearchTimeLimit, globalSearchFailLimit);
	IloNum timeTemp5 = env.getTime();
	printf("%.3f CPU second(s) spent to look for a first solution.\n", timeTemp5 - timeTemp4);
	IloNum bestTardiCost = globalSolution.getMax(tardiCostVar);
	model.add(tardiCostVar <= bestTardiCost - 1.0);
//	env.out() << "Initial solution" << endl;
//	PrintSolution(env, globalSolution, tardiCostVar);

	// SET PARAMETERS FOR LOCAL SEARCH.
	IloObjective obj = IloMinimize(env, tardiCostVar);
	lSSolution.getSolution().add(obj);
	

	// SUB-GOAL
	IloGoal subGoal =
//						IloAssignAlternative(env)
//						&&
						IloMyAssignAlternativeAndRank(env, heuristic)
//						&& IloRankForward(env) 
//						&& IloInstantiate(env, tardiCostVar)
//						&& IloDichotomize(env, tardiCostVar, IloTrue)
						&& IloMyInstantiate(env, tardiCostVar)
						;

	if(localSearchFailLimit < IloIntMax)
		subGoal = IloLimitSearch(env, subGoal, IloFailLimit(env, localSearchFailLimit));
	subGoal = IloLimitSearch(env, subGoal, IloTimeLimit(env, localSearchTimeLimit));

//	subGoal = IloSelectSearch(env, subGoal, IloMinimizeVar(env, tardiCostVar, 1.0)); // The next solution has a cost equal to the previous cost minus 1.0.

//	IloPredicate<IloActivity> isBeforeSelected = IsActivityBeforeSelected(env); // This is used with time window neighborhood.

	// GLIDING TIME WINDOW SEARCH
//	 IloSchedulerLargeNHood timeWindowNHood = IloTimeWindowNHood(env, 20, 10);

	// relocate activity
//	IloSchedulerLargeNHood relocateActivities = IloRelocateActivityNHood(env);

	// relocate process plan
//	IloComparator<IloActivityArray> myPPComparator = IloMyRandomPPComparator(env);
//	IloComparator<IloActivityArray> myPPComparator = IloMyTardiCostPPEvaluator(lSSolver).makeGreaterThanComparator();
	IloComparator<IloActivityArray> myPPComparator = IloMyGainTardiCostPPEvaluator(lSSolver).makeGreaterThanComparator();
	IloSchedulerLargeNHood relocatePPNHood = MyRelocateJobNHood(env, actArrays, myPPComparator);

	IloNHood nhood = 
//						IloContinue(env, relocateActivities)		// IloContinue tries each neighbor and starts from the state in
//						+											// which the schedule was after the last move.
//						IloContinue(env, relocateJobNHood)
						IloContinue(env, relocatePPNHood)
//						+
//						IloContinue(env, timeWindowNHood)
						;
  
//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	subGoal = IloApply(env, subGoal, myNodeEvaluator);

	IloGoal greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);
	IloInt movesDone = 0;
	IloNum timeTemp3 = 0.0;
	IloNum timeTemp = env.getTime();
	while((timeTemp3 < localSearchTimeLimit)&&(lSSolver.solve(greedyMove))) {
		IloNum timeTemp2 = env.getTime();
		timeTemp3 += timeTemp2 - timeTemp;
	//	IloNum cost = lsSolution.getSolution().getObjectiveValue();
		IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
	//	lSSolver.out() << "Move: " << movesDone << ":\t";
	//	++movesDone;
	//	printf("solution:  ** HC\t");
	//	printf("at tardiness cost = %.14f", tardiCost);
	//	printf("\n");
		if(tardiCost < bestTardiCost) {
			bestTardiCost = tardiCost;
			globalSolution.store(lSScheduler);
		}
		timeTemp = env.getTime();
	//	IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
	//	env.out() << "Solution" << endl;
	//	PrintSolution(globalSolution, costVar);
	}

	printf("Final solution at tardiness cost: %.14f found after %.3f CPU second(s).\n\n", bestTardiCost, timeTemp3);
//	PrintSolution(globalSolution, tardiCostVar);

	lSSolver.end();
	return bestTardiCost;
}


IloModel
DefineModelBeta(IloEnv&						env,
				IloInt						numberOfProcessPlans,
				IloInt						numberOfActivitiesPerProcessPlan,
				IloInt						numberOfResourcesPerActivity,
				IloInt						numberOfResources,
				IloInt***					resourceNumbers,
				IloInt**					durations,
				IloIntervalList*			breakList,
				IloInt*						dueDates,
				IloNum*						phis,
				IloNum						beta,
				IloNum						gamma,
				IloRandom					randomGenerator,
				IloSchedulerSolution		solution,
				IloArray<IloActivityArray>& actArrays,
				IloNumVar&					tardiness) {
	IloModel model(env);
	IloInt numberOfActivities = numberOfProcessPlans * numberOfActivitiesPerProcessPlan;
	IloInt i,j,k,l;

	// CREATE THE RESOURCES WITH BREAKS.
	IloSchedulerEnv schedEnv(env);
	IloResourceParam resParam = schedEnv.getResourceParam();
//	schedEnv.setPrecedenceEnforcement(IloExtended);
	resParam.setCapacityEnforcement(IloMediumLow);
//	resParam.setPrecedenceEnforcement(IloMediumHigh);		// Precedence graph
	
	char buffer[128];
	IloUnaryResource* resources = new (env) IloUnaryResource[numberOfResources];
	for(i = 0; i < numberOfResources; i++) {
		sprintf(buffer, "R%ld", i);
		resources[i] = IloUnaryResource(env, buffer);
		resources[i].setObject((void*)(i));
	//	solution.add(resources[i]);
		IloInt endTemp = 0;
		for(IloIntervalListCursor cursor(breakList[i]); cursor.ok(); ++cursor) {
		//	resources[i].addBreak(cursor.getStart(), cursor.getEnd());
			endTemp = cursor.getEnd();
		}
	}

	// CREATE THE ALTERNATIVE RESOURCE SET.
	printf("Creating an alternative resource set\n");
	IloAltResSet altResSet(env);
	for(i = 0; i < numberOfResources; i++) {
		altResSet.add(resources[i]);
	}
	IloBool* rejectedResources = new(env) IloBool[numberOfResources];
	
	  // CREATE THE ACTIVITIES AND ADD INITIAL TEMPORAL CONSTRAINTS.
	printf("Creation of activities and initial precedence constraints added\n");
	IloActivity* activities = new (env) IloActivity[numberOfActivities];
	IloActivity previousActivity;
	actArrays = IloArray<IloActivityArray>(env);
	for(i = 0; i < numberOfProcessPlans; i++) {
		IloActivityArray actArray(env);
		actArrays.add(actArray);
		IloInt tempQueue = 0;
		IloNum betaPhi = beta * phis[i];
		IluProcPlan* procPlan = new(env) IluProcPlan(env, i, IlcIntMin, betaPhi, gamma, phis[i], dueDates[i], IloIntMax, 0.0, actArray);
	    for (j = numberOfActivitiesPerProcessPlan - 1; j >= 0; j--) {
			k = i * numberOfActivitiesPerProcessPlan + j;
			activities[k] = IloActivity(env, durations[i][j]);
		//	printf("Activity %ld: duration %ld\n", k, durations[i][j]);
			IluInfoActivity* infoAct = new(env) IluInfoActivity(k, tempQueue, 0, 0, 0, procPlan);
			tempQueue += durations[i][j];
			activities[k].setObject(infoAct);
			activities[k].setBreakable();
		//	model.add(activities[k]);
			if(j < numberOfActivitiesPerProcessPlan - 1) {
				IloPrecedenceConstraint tCt = previousActivity.startsAfterEnd(activities[k]);
			//	printf("A precedence constraint is added to the model between the end of activity %ld and the start of activity %ld.\n", k, k + 1);
				model.add(tCt);
			}
			previousActivity = activities[k];
			actArray.add(activities[k]);
			sprintf(buffer, "J%ldS%ld", i, j);
			activities[k].setName(buffer);
			solution.add(activities[k], IloRestoreNothing);
			IloResourceConstraint rCt = activities[k].requires(altResSet);
			for(l = 0; l < numberOfResources; l++) {
				rejectedResources[l] = IloTrue;
			}
			for(l = 0; l < numberOfResourcesPerActivity; l++) {
				rejectedResources[resourceNumbers[i][j][l]] = IloFalse;
			}
			for(l = 0; l < numberOfResources; l++) {
				if(IloTrue == rejectedResources[l])
					rCt.setRejected(resources[l]);
			}
			infoAct->setResCt(rCt);
			model.add(rCt);
			solution.add(rCt, IloRestoreNothing);
		}
	}
//	for(i = 0; i < numberOfProcessPlans; i++) {
//		IloActivityArray job = jobs[i];
//		for (j = 0; j < numberOfActivitiesPerProcessPlan; j++) {
//			IloActivity act = job[j];
//			IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
//			printf("Activity %ld\n", infoAct->getActIndex());
//		}
//	}

	// CREATE THE TARDINESS COST VARIABLE.
	IloExpr expr;
	for(i = 1; i < numberOfProcessPlans + 1; i++) {
		IloExpr tardi = IloMax(activities[i * numberOfActivitiesPerProcessPlan - 1].getEndExpr() - dueDates[i - 1], 0.0);
		tardi = phis[i - 1] * tardi;
		if(0 == expr.getImpl())
			expr = tardi;
		else
			expr = expr + tardi;
	}

	tardiness = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT, "Tardiness");

	model.add(tardiness == expr);

  // RETURN THE MODEL
  return model;
}