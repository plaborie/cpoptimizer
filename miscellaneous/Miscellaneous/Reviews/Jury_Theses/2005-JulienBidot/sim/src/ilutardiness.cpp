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


#include <ilsim/ilutardiness.h>
#include <ilsim/iluprocessplan.h>
#include <ilsim/iluneighborhood.h>
#include <ilsim/iluprintsolution.h>


ILOSTLBEGIN



void FindInitialSolutionTardiness(IloModel						model,
								  IloSchedulerSolution			globalSolution,
								  IloSchedulerSolution			lSSolution,
								  IloNumVar						tardiCostVar,
								  IloArray<IloActivityArray>&	actArrays,
								  IloNum						globalSearchTimeLimit,
								  IloInt						globalSearchFailLimit
							 ) {
	IloEnv env = model.getEnv();
//	model.add(IloMinimize(env, tardiCostVar));
	IloSolver solver0(model);
	solver0.solve(
		IloPrintLowerBound(env, tardiCostVar)
		&&
		IloProcPlanEndMin(env, actArrays)
		);
	solver0.end();
	
	IloSolver solver(model);
//	IluMyStamp* iluStamp = new(env) IluMyStamp();
//	solver.setObject((void*)iluStamp);

//	IlcRandom randGenerator(solver, 1);
//	MyRandom* randGenerator = new(env) MyRandom(solver, 1);
	IloRandom randGenerator(env, 1);


	// SEARCH HEURISTIC
	IloPredicate<IlcAltResConstraint> assignAltRCPredicate = !IlcAltResConstraintResourceSelectedPredicate(solver);
	// This function returns an alternative resource constraint predicate whose operator(const IlcAltResConstraint& altrc) returns IlcTrue if and only if
	// a single resource has been selected for the activity corresponding to altrc. If there are still multiple resources that can be selected, IlcFalse
	// is returned.
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcAltResConstraintNbPossibleEvaluator(solver).makeLessThanComparator();
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcAltResConstraintNbPossibleEvaluator(solver),
//																									IlcMyAltResConstraintMyRandomEvaluator(solver, randGenerator));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcAltResConstraintNbPossibleEvaluator(solver),
//																									IlcMyAltResConstraintAddressEvaluator(solver));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltResConstraintMyRandomEvaluator(solver, randGenerator).makeLessThanComparator();
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(solver).makeLessThanComparator();

	IloComparator<IlcResource> assignResComparator = IlcMyResourceRandomEvaluator(solver, randGenerator).makeLessThanComparator();
//	IloComparator<IlcResource> assignResComparator = IlcMyResourceAddressEvaluator(solver).makeLessThanComparator();
		
//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(solver);
	// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
	// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

	
	
//	IloComparator<IlcResource> rankResComparator = IlcMyResourceRandomEvaluator(solver, randGenerator).makeLessThanComparator();
/*	IloComparator<IlcResource> rankResComparator = IloComposeLexical(
																		IlcMyResourceRefDateEvaluator(solver).makeGreaterThanComparator(),
																		IlcMyResourceRandomEvaluator(solver, randGenerator).makeLessThanComparator()
																		);*/
//	IloComparator<IlcResource> rankResComparator = IloComposeLexical(-IlcMyRankResourceEvaluator(solver), IlcMyResourceAddressEvaluator(solver));
//	IloComparator<IlcResource> rankResComparator = IlcMyRankResourceEvaluator(solver).makeLessThanComparator();
//	IloComparator<IlcResource> rankResComparator = IlcMyResourceMyRandomEvaluator(solver, randGenerator).makeLessThanComparator();

	IloPredicate<IlcResourceConstraint> rankRCPredicate = IlcResourceConstraintPossibleFirstPredicate(solver);
	// This function returns a resource constraint predicate whose operator(const IlcResourceConstraint& rc) returns IlcTrue if and only if rc can be ranked
	// first among the non-ranked resource constraints. In particular, it returns IlcFalse if rc is already ranked or if rc represents a virtual source
	// or sink node (empty handle).
	
	IloEvaluator<IlcResourceConstraint> myRCtStartMinEvaluator = IlcMyResourceConstraintStartMinEvaluator(solver);
//	IloEvaluator<IlcResourceConstraint> myRCtEndMaxEvaluator = IlcMyResourceConstraintEndMaxEvaluator(solver);
//	IloEvaluator<IlcResourceConstraint> myRCtDurationMinEvaluator = IlcMyResourceConstraintDurationMinEvaluator(solver);
	IloEvaluator<IlcResourceConstraint> myRCtProcPlanCostRefDateEvaluator = IlcMyProcPlanCostRefDateEvaluator(solver);
	IloEvaluator<IlcResourceConstraint> myRCtRandomEvaluator = IlcMyResourceConstraintRandomEvaluator(solver, randGenerator);


	IloComparator<IlcResourceConstraint> rankRCComparator = IloComposeLexical(
																				myRCtStartMinEvaluator.makeLessThanComparator(),
																			//	myRCtEndMaxEvaluator.makeLessThanComparator(),
																			//	myRCtDurationMinEvaluator.makeLessThanComparator(),
																				myRCtProcPlanCostRefDateEvaluator.makeGreaterThanComparator(),
																				myRCtRandomEvaluator.makeLessThanComparator()
																				);

	IloPredicate<IlcActivity> setTimesActPredicate = !IlcActivityStartVarBoundPredicate(solver) && !IlcActivityPostponedPredicate(solver);

	IloTranslator<IlcResourceConstraint, IlcActivity> rCtActTranslator = IloMyRCtTranslator(solver);

	IloEvaluator<IlcActivity> myActStartMinEvaluator = myRCtStartMinEvaluator << rCtActTranslator;
	IloEvaluator<IlcActivity> myActProcPlanCostRefDateEvaluator = myRCtProcPlanCostRefDateEvaluator << rCtActTranslator;
	IloEvaluator<IlcActivity> myActRandomEvaluator = myRCtRandomEvaluator << rCtActTranslator;

	IloComparator<IlcActivity> setTimesActComparator = IloComposeLexical(
																			myActStartMinEvaluator.makeLessThanComparator(),
																			myActProcPlanCostRefDateEvaluator.makeGreaterThanComparator(),
																			myActRandomEvaluator.makeLessThanComparator()
																			);


	IluHeuristic heuristic;

	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
	heuristic.setAssignAltRComparator(assignResComparator);

//	heuristic.setRankResPredicate(rankResPredicate);
//	heuristic.setRankResComparator(rankResComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);

	heuristic.setSetTimesActPredicate(setTimesActPredicate);
	heuristic.setSetTimesActComparator(setTimesActComparator);

	IlcScheduler scheduler(solver);

	IloNum bestVal = IloInfinity;

	IloGoal g = 
//				IloPrintLowerBound(env, tardiCostVar)
//				&&
//				IloProcPlanEndMin(env, actArrays)
//				IloAssignAlternative(env)
//				&&
//				IloMyMinimizeFloatVar(env, tardiCostVar, bestVal)
//				&&
				IloMyAssignAlternativeAndRank(env, heuristic)
//				IloMyAssignAlternativeAndSetTimes(env, heuristic)
//				IloRankForward(env)
				&&
//				&& IloSetTimesForward(env) 
//				&& IloDichotomize(env, tardiCostVar, IloTrue)
				IloMyInstantiate(env, tardiCostVar)
				;

//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	g = IloApply(env, g, myNodeEvaluator);

//	g = IloLimitSearch(env, g, IloTimeLimit(env, globalSearchTimeLimit));
//	if(globalSearchFailLimit < IloIntMax)
//		g = IloLimitSearch(env, g, IloFailLimit(env, globalSearchFailLimit));

//	solver.setRelativeOptimizationStep(0.05);
//	solver.setOptimizationStep(1.0);	

/*	solver.startNewSearch(g);
	IloNum timeTemp = env.getTime();
	while(solver.next()) {
//		IloNum best = solver.getMin(tardiCostVar);
		printf("*** Initial solution at tardiness cost = ");
		printf("%.14f\tat time %.3f\n", solver.getFloatVar(tardiCostVar).getMax(), env.getTime() - timeTemp);
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
/*	while(timeTemp3 < globalSearchTimeLimit) {
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
	}*/
	while(timeTemp3 < globalSearchTimeLimit) {
		IloNum timeTemp = env.getTime();
		solver.solve(g);
		IloNum currentValue = solver.getFloatVar(tardiCostVar).getMax();
		if(currentValue < bestVal) {
			printf("*** Initial solution at tardiness cost = ");
			printf("%.14f\tat time %.3f\n", currentValue, timeTemp3 + env.getTime() - timeTemp);
			bestVal = currentValue;
		    globalSolution.store(scheduler);
			lSSolution.store(scheduler);
	//		PrintSolution(globalSolution, actArrays, tardiCostVar, IloTrue);
	//		exit(0);
		}
//		else
//			printf("No solution found.\n");
		timeTemp3 += env.getTime() - timeTemp;
	}
//	exit(0);

	solver.end();
}




IloNum
SolveModelTardiness(IloModel					model,
					IloNumVar					tardiCostVar,
					IloArray<IloActivityArray>	actArrays,
					IloSchedulerSolution&		globalSolution,
					IloNum						globalSearchTimeLimit,
					IloInt						globalSearchFailLimit,
					IloNum						localSearchTimeLimit,
					IloInt						localSearchFailLimit,
					IloNum						relaxActProb,
					IloNum						minImprovePercentage,
					IloNum						improvePercentageFactor) {


	IloEnv env = model.getEnv();

	// CREATE LOCAL SEARCH SOLUTION
	IloSchedulerSolution lSSolution = CreateLSSolution(env, globalSolution);

	// GENERATE AN INITIAL SOLUTION.
	globalSolution.getSolution().add(tardiCostVar);
	IloNum timeTemp4 = env.getTime();
	FindInitialSolutionTardiness(model, globalSolution, lSSolution, tardiCostVar, actArrays, globalSearchTimeLimit, globalSearchFailLimit);
	IloNum timeTemp5 = env.getTime();
	printf("%.3f CPU second(s) spent to look for a first solution globally.\n", timeTemp5 - timeTemp4);
	IloNum bestTardiCost = globalSolution.getMax(tardiCostVar);
	model.add(tardiCostVar <= bestTardiCost - 1.0);
//	env.out() << "Initial solution" << endl;
//	PrintSolution(env, globalSolution, tardiCostVar);

	IloSolver lSSolver(model);
//	IluMyStamp* iluStamp = new(env) IluMyStamp();
//	lSSolver.setObject((void*)iluStamp);

	IlcScheduler lSScheduler(lSSolver);
	IloRandom randGenerator(env, 1);
//	IlcRandom randGenerator(lSSolver, 1);
//	MyRandom* randGenerator = new(env) MyRandom(lSSolver, 1);

	// SEARCH HEURISTIC
	IloPredicate<IlcAltResConstraint> assignAltRCPredicate = !IlcAltResConstraintResourceSelectedPredicate(lSSolver);
	// This function returns an alternative resource constraint predicate whose operator(const IlcAltResConstraint& altrc) returns IlcTrue if and only if
	// a single resource has been selected for the activity corresponding to altrc. If there are still multiple resources that can be selected, IlcFalse
	// is returned.
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcAltResConstraintNbPossibleEvaluator(lSSolver).makeLessThanComparator();
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(lSSolver).makeLessThanComparator();
/*	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(
																					IlcAltResConstraintNbPossibleEvaluator(lSSolver),
																					IlcMyAltRCConstantEvaluator(lSSolver)
																			//		IlcMyAltResConstraintMyRandomEvaluator(lSSolver, randGenerator)
																					);*/
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcAltResConstraintNbPossibleEvaluator(lSSolver),
//																									IlcMyAltResConstraintAddressEvaluator(lSSolver));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltResConstraintMyRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(lSSolver).makeLessThanComparator();

	IloComparator<IlcResource> assignResComparator = IlcMyResourceRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
//	IloComparator<IlcResource> assignResComparator = IlcMyResourceAddressEvaluator(lSSolver).makeLessThanComparator();

		
//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(lSSolver);
	// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
	// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

//	IloComparator<IlcResource> rankResComparator = IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
/*	IloComparator<IlcResource> rankResComparator = IloComposeLexical(	
																//		IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator),
																		-IlcMyResourceRefDateEvaluator(lSSolver),
																		IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator)
																		);*/
//	IloComparator<IlcResource> rankResComparator = IloComposeLexical(IlcMyRankResourceEvaluator(lSSolver), IlcMyResourceAddressEvaluator(lSSolver));
//	IloComparator<IlcResource> rankResComparator = IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();

	IloPredicate<IlcResourceConstraint> rankRCPredicate = IlcResourceConstraintPossibleFirstPredicate(lSSolver);
	// This function returns a resource constraint predicate whose operator(const IlcResourceConstraint& rc) returns IlcTrue if and only if rc can be ranked
	// first among the non-ranked resource constraints. In particular, it returns IlcFalse if rc is already ranked or if rc represents a virtual source
	// or sink node (empty handle).
	
	IloEvaluator<IlcResourceConstraint> myRCtStartMinEvaluator = IlcMyResourceConstraintStartMinEvaluator(lSSolver);
//	IloEvaluator<IlcResourceConstraint> myRCtEndMaxEvaluator = IlcMyResourceConstraintEndMaxEvaluator(lSSolver);
//	IloEvaluator<IlcResourceConstraint> myRCtDurationMinEvaluator = IlcMyResourceConstraintDurationMinEvaluator(lSSolver);
	IloEvaluator<IlcResourceConstraint> myRCtProcPlanCostRefDateEvaluator = IlcMyProcPlanCostRefDateEvaluator(lSSolver);
	IloEvaluator<IlcResourceConstraint> myRCtRandomEvaluator = IlcMyResourceConstraintRandomEvaluator(lSSolver, randGenerator);


	IloComparator<IlcResourceConstraint> rankRCComparator = IloComposeLexical(
																				myRCtStartMinEvaluator.makeLessThanComparator(),
																			//	myRCtEndMaxEvaluator.makeLessThanComparator(),
																			//	myRCtDurationMinEvaluator.makeLessThanComparator(),
																				myRCtProcPlanCostRefDateEvaluator.makeGreaterThanComparator(),
																				myRCtRandomEvaluator.makeLessThanComparator()
																				);
	// We rank first the resource constraint that is the most critical with respect to the process plan due date.

	IluHeuristic heuristic;
	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
	heuristic.setAssignAltRComparator(assignResComparator);
//	heuristic.setRankResPredicate(rankResPredicate);
//	heuristic.setRankResComparator(rankResComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);


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
//	subGoal = IloLimitSearch(env, subGoal, IloTimeLimit(env, localSearchTimeLimit));

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
	IloRandom randGenerator3(env, 1);
	IloSchedulerLargeNHood relocatePPNHood = MyRelocateSubsetNHood(env, actArrays, randGenerator3, relaxActProb, myPPComparator);
//	IloSchedulerLargeNHood relocatePPNHood = MyRelocateJobNHood(env, actArrays, myPPComparator);

	IloNHood nhood = 
//						IloContinue(env, relocateActivities)		// IloContinue tries each neighbor and starts from the state in
//						+											// which the schedule was after the last move.
//						IloContinue(env, relocateJobNHood)
						IloContinue(env, relocatePPNHood)
//						+
//						IloContinue(env, timeWindowNHood)
						;

	// relocate activities
	IloRandom randGenerator2(env, 1);
	IloSchedulerLargeNHood relocateActNHood = RandomActivityNHood(env, randGenerator2, relaxActProb);

//	IloNHood nhood2 = IloContinue(env, relocateActNHood);		// It does not make sense because size equals 1.
	IloNHood nhood2 = relocateActNHood;

	// SBS=LDS
//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	subGoal = IloApply(env, subGoal, myNodeEvaluator);


	IloNum tempImprovePercentage = minImprovePercentage;
	IloNum improveStep = 1.0;//IloMax(1.0, globalSolution.getMax(tardiCostVar) * tempImprovePercentage / 100.0);
	IloGoal greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env, improveStep), IloFirstSolution(env), subGoal);	
	IloGoal greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);

	IloInt movesDone = 0;
	IloBool betterSolution = IloFalse;
//	printf("Large Neighborhood Search with relaxing a process plan with %.2f percent of improvement and an improvement step of %.1f\n",
//			tempImprovePercentage,
//			improveStep);
	IloNum timeTemp = env.getTime();

	printf("Large Neighborhood Search with relaxing %.1f percent of activities\n", relaxActProb * 100);
	while(env.getTime() - timeTemp < localSearchTimeLimit) {
	//	printf("Large Neighborhood Search with relaxing a process plan with %.2f percent of improvement and an improvement step of %.1f\n", tempImprovePercentage, improveStep);
		if(lSSolver.solve(greedyMove)) {
		//	IloNum cost = lsSolution.getSolution().getObjectiveValue();
			IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
			lSSolver.out() << "Move1: " << movesDone << ":\t";
			++movesDone;
			printf("solution at tardiness cost = %.14f\tat time %.3f\n", tardiCost, env.getTime() - timeTemp);
			if(tardiCost < bestTardiCost) {
				bestTardiCost = tardiCost;
				globalSolution.store(lSScheduler);
			}
		//	IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
		//	env.out() << "Solution" << endl;
		//	PrintSolution(globalSolution, costVar);
		}
		else while((env.getTime() - timeTemp < localSearchTimeLimit)&&(IloFalse == betterSolution)) {
		//	printf("Large Neighborhood Search with relaxing %.1f percent of activities with an improvement step of %.1f\n",
		//			relaxActProb * 100,
		//			improveStep);
			if(lSSolver.solve(greedyMove2)) {
				IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
				betterSolution = IloTrue;
				if(tardiCost > 0.0) {
					lSSolver.out() << "Move2: " << movesDone << ":\tsolution at tardiness cost = ";
					++movesDone;
					printf("%.14f\tat time %.3f\n", tardiCost, env.getTime() - timeTemp);
					if(tardiCost < bestTardiCost) {
						bestTardiCost = tardiCost;
						globalSolution.store(lSScheduler);
					}
				}
			}
		//	else
		//		printf("This relaxation failed!\n");
		}
		betterSolution = IloFalse;
	}

/*	while((improveStep >= 1.0)&&(env.getTime() - timeTemp < localSearchTimeLimit)) {
		if(lSSolver.solve(greedyMove)) {
		//	IloNum cost = lsSolution.getSolution().getObjectiveValue();
			IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
			lSSolver.out() << "Move: " << movesDone << ":\t";
			++movesDone;
			printf("solution at tardiness cost = %.14f\tat time %.3f\n", tardiCost, env.getTime() - timeTemp);
			if(tardiCost < bestTardiCost) {
				bestTardiCost = tardiCost;
				globalSolution.store(lSScheduler);
			}
		//	IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
		//	env.out() << "Solution" << endl;
		//	PrintSolution(globalSolution, costVar);
		}
		else
			improveStep = 0.0;
		else {
			tempImprovePercentage *= improvePercentageFactor;
			improveStep = globalSolution.getMax(tardiCostVar) * tempImprovePercentage / 100.0;
			greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env, improveStep), IloFirstSolution(env), subGoal);
			if(improveStep >= 1.0)
				printf("Percentage of improvement = %.2f with an improvement step of %.1f at time %.3f\n",
						tempImprovePercentage,
						improveStep,
						env.getTime() - timeTemp);
		}
	}
	improveStep = 1.0;

	if(env.getTime() - timeTemp < localSearchTimeLimit) {
//		tempImprovePercentage = minImprovePercentage;
//		improveStep = IloMax(1.0, globalSolution.getMax(tardiCostVar) * tempImprovePercentage / 100.0);
//		IloGoal greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, improveStep), IloFirstSolution(env), subGoal);
		printf("Large Neighborhood Search with relaxing %.1f percent of activities with an improvement step of %.1f\n",
				relaxActProb * 100,
				improveStep);
		while(env.getTime() - timeTemp < localSearchTimeLimit) {
			if(lSSolver.solve(greedyMove2)) {
				IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
				if(tardiCost > 0.0) {
					lSSolver.out() << "Move: " << movesDone << ":\tsolution at tardiness cost = ";
					++movesDone;
					printf("%.14f\tat time %.3f\n", tardiCost, env.getTime() - timeTemp);
					if(tardiCost < bestTardiCost) {
						bestTardiCost = tardiCost;
						globalSolution.store(lSScheduler);
					}
				//	atLeastOneMove = IloTrue;
				}
			//	if(0 == cost) {
			//		PrintSolution(env, lSScheduler);
			//		IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
			//		env.out() << "Solution" << endl;
			//		PrintSolution(globalSolution, costVar);
			//	}
			}
			else {
				tempImprovePercentage *= improvePercentageFactor;
				improveStep = IloMax(1.0, globalSolution.getMax(tardiCostVar) * tempImprovePercentage / 100.0);
				greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, improveStep), IloFirstSolution(env), subGoal);
				if(improveStep > 1.0)
					printf("Percentage of improvement = %.2f with an improvement step of %.1f at time %.3f\n",
							tempImprovePercentage,
							improveStep,
							env.getTime() - timeTemp);
		//		printf("LNS has failed (activities)!\n");
			}
		}
	}*/




	lSSolver.end();

	printf("Final solution at tardiness cost: %.14f\n\n", bestTardiCost);
//	PrintSolution(globalSolution, tardiCostVar);
	
	return bestTardiCost;
}


IloModel
DefineModelTardiness(IloEnv&						env,
					 IloInt							numberOfProcessPlans,
					 IloInt							numberOfActivitiesPerProcessPlan,
					 IloInt							numberOfResourcesPerActivity,
					 IloInt							numberOfResources,
					 IloInt***						resourceNumbers,
					 IloInt**						durations,
					 IloIntervalList*				breakList,
					 IloInt*						dueDates,
					 IloNum*						phis,
					 IloNum							beta,
					 IloNum							gamma,
					 IloRandom						randomGenerator,
					 IloSchedulerSolution			solution,
					 IloArray<IloActivityArray>&	actArrays,
					 IloNumVar&						tardiness) {
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
		solution.add(resources[i]);
	//	IloInt endTemp = 0;
		for(IloIntervalListCursor cursor(breakList[i]); cursor.ok(); ++cursor) {
			resources[i].addBreak(cursor.getStart(), cursor.getEnd());
	//		endTemp = cursor.getEnd();
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
		IluProcPlan* procPlan = new(env) IluProcPlan(env, i, IlcIntMin, betaPhi, gamma, 0.0, phis[i], dueDates[i], IloIntMax, -IlcInfinity, actArray);
	    for (j = numberOfActivitiesPerProcessPlan - 1; j >= 0; j--) {
			k = i * numberOfActivitiesPerProcessPlan + j;
			activities[k] = IloActivity(env, durations[i][j]);
		//	printf("Activity %ld: duration %.1f\n", k, activities[k].getProcessingTimeMin());
		//	cout << activities[k].getProcessingTimeMin() << endl;
			IluInfoActivity* infoAct = new(env) IluInfoActivity(k, tempQueue, 0, 0, procPlan);
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