// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluglobalcost.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains IloMyAssignAlternativeAndRank, the FindInitialSolutionGlobalCost function, the SolveModelGlobalCost function,
// and the DefineModelGlobalCost function.


#include <ilsim/iluglobalcost.h>
#include <ilsim/iluprocessplan.h>
#include <ilsim/iluconstraints.h>
#include <ilsim/iluneighborhood.h>
#include <ilsim/iluprintsolution.h>


ILOSTLBEGIN




void FindInitialSolutionGlobalCost(IloModel						model,
								   IloSchedulerSolution			globalSolution,
								   IloSchedulerSolution			lSSolution,
								   IloNumVar					globalCostVar,
								   IloNumVar					allocCostVar,
								   IloNumVar					tardiCostVar,
								   IloArray<IloActivityArray>	actArrays,
								   IloNum						globalSearchTimeLimit,
								   IloInt						globalSearchFailLimit,
								   IloInt						kAllocLowerBound,
								   IloNum						kTardiLowerBound
								  // IloInt nbResources
								   ) {
	IloEnv env = model.getEnv();
//	model.add(IloMinimize(env, globalCostVar));
	IloSolver solver0(model);
	solver0.solve(IloPrintLowerBound(env, globalCostVar)&&IloProcPlanEndMin(env, actArrays));
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
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(//-IlcMyAssignAltRCMinCostEvaluator(solver),
//																					-IlcMyAssignAltRCDispersionEvaluator(solver),
//																								IlcMyAltResConstraintMyRandomEvaluator(solver, randGenerator));

//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(-IlcMyAssignAltRCDispersionEvaluator(solver),
//																									IlcMyAltResConstraintAddressEvaluator(solver));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltResConstraintMyRandomEvaluator(solver, randGenerator).makeLessThanComparator();
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(solver).makeLessThanComparator();
	
//	IloComparator<IlcResource> assignResComparator = IloComposeLexical(IlcMyAssignAltResourceEvaluator(solver),
//																									IlcMyResourceRandomEvaluator(solver, randGenerator));
//	IloComparator<IlcResource> assignResComparator = IloComposeLexical(IlcMyAssignAltResourceEvaluator(solver),
//																									IlcMyResourceAddressEvaluator(solver));
//	IloComparator<IlcResource> assignResComparator = IlcMyResourceRandomEvaluator(solver, randGenerator).makeGreaterThanComparator();
	IloSelector<IlcResource, IlcAltResConstraint> assignAltRSelector = IloMyAllocCostResourceInAltResCtSelector(solver, randGenerator);

//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(solver);
	// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
	// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

//	IloComparator<IlcResource> rankResComparator = IlcMyResourceRandomEvaluator(solver, randGenerator).makeLessThanComparator();
/*	IloComparator<IlcResource> rankResComparator = IloComposeLexical(
																		-IlcMyResourceRefDateEvaluator(solver),
																		IlcMyResourceRandomEvaluator(solver, randGenerator)
																		);*/
//	IloComparator<IlcResource> rankResComparator = IlcMyResourceMyRandomEvaluator(solver, randGenerator).makeGreaterThanComparator();
//	IloComparator<IlcResource> rankResComparator = IloComposeLexical(IlcMyRankResourceEvaluator(solver), IlcMyResourceAddressEvaluator(solver));

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


	IluHeuristic heuristic;
	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
//	heuristic.setAssignAltRComparator(assignResComparator);
	heuristic.setAssignAltRSelector(assignAltRSelector);
//	heuristic.setRankResPredicate(rankResPredicate);
//	heuristic.setRankResComparator(rankResComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);
	
	IlcScheduler scheduler(solver);

	IloNum bestVal = IloInfinity;

	IloGoal g = 
//				IloPrintLowerBound(env, globalCostVar)
//				&&
//				IloProcPlanEndMin(env, actArrays)
//				IloAssignAlternative(env)
//				&&
//				IloMyMinimizeFloatVar(env, globalCostVar, bestVal)
//				&&
				IloMyAssignAlternativeAndRank(env, heuristic) 
//				IloRankForward(env)
//				&& IloSetTimesForward(env) 
//				&& IloDichotomize(env, costVar, IloTrue)
				&& IloMyInstantiate(env, globalCostVar)
				;

//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	g = IloApply(env, g, myNodeEvaluator);

//	g = IloLimitSearch(env, g, IloTimeLimit(env, globalSearchTimeLimit));
//	g = IloLimitSearch(env, g, IloTimeLimit(env, 40.0));
//	if(globalSearchFailLimit < IloIntMax)
//		g = IloLimitSearch(env, g, IloFailLimit(env, globalSearchFailLimit));

//	solver.setRelativeOptimizationStep(0.05);
//	solver.setOptimizationStep(1.0);

/*	solver.startNewSearch(g);
//	IloInt* allocatedResources = new(env) IloInt[nbResources];
//	for(IloInt i = 0; i < nbResources; i++)
//		allocatedResources[i] = 0;
	IloNum timeTemp = env.getTime();
	while(solver.next()) {
//		IloNum best = solver.getMin(globalCostVar);
		printf("*** ");
		printf("global cost = %.14f\t", solver.getFloatVar(globalCostVar).getMax());
		printf("allocation cost = %ld\t", solver.getIntVar(allocCostVar).getMax());
		printf("tardiness cost = %.14f\t", solver.getFloatVar(tardiCostVar).getMax());
		printf("beta = %.14f\t", solver.getIntVar(allocCostVar).getMax()/solver.getFloatVar(tardiCostVar).getMax());
		printf("at time %.3f", env.getTime() - timeTemp);
		printf("\n");
//		solver.out() << "*** Initial solution at cost: " <<  best << endl;
//		solver.printInformation();
		globalSolution.store(scheduler);
		lSSolution.store(scheduler);

//	IloInt nbAllocatedResources = nbResources;
//	IloInt nbActivities = 0;
//	IloInt nbSuccessors = 0;
//	for(IloSchedulerSolution::ResourceIterator rsIte(globalSolution); rsIte.ok(); ++rsIte)
//		nbResources++;

//	for(IloSchedulerSolution::ResourceConstraintIterator ite(globalSolution); ite.ok(); ++ite) {
//		IloResourceConstraint rc = *ite;
//		IloResource res = globalSolution.getSelected(rc);
//		IloInt resIndex = (IloInt)res.getObject();
	//	printf("Resource %ld has been selected.\n", resIndex);
//		allocatedResources[resIndex] = 1;
//		if(globalSolution.hasNextRC(rc))
//			nbSuccessors++;
//		nbActivities++;
//	}

//	for(IloInt i = 0; i < nbResources; i++) {
//		if(0 == allocatedResources[i])
//			nbAllocatedResources--;
//	}
//	printf("In this solution, there are:\n");
//	printf("\t%ld activities\n", nbActivities);
//	printf("\t%ld resources\n", nbResources);
//	printf("\t%ld allocated resources\n", nbAllocatedResources);
//	printf("\t%ld successors\n", nbSuccessors);
//	if(nbSuccessors != nbActivities - nbAllocatedResources)
//		printf("Warning: too few successors!\n");

	}
//	else
//		printf("No solution found.\n");
//	solver.printInformation();
	solver.endSearch();*/

	// FAST RESTARTS WITH A SMALL FAIL LIMIT
	IloNum timeTemp3 = 0.0;
	IloInt seedCounter = 1;
	while(timeTemp3 < globalSearchTimeLimit) {
		IloNum timeTemp = env.getTime();
//		printf("Seed = %ld\n", seedCounter);
//		randGenerator->init(seedCounter);
//		if(solver.solve(g)) {
//			printf("*** Init. sol.: ");
//			printf("global cost = %.14f\t", solver.getFloatVar(globalCostVar).getMax());
//			printf("allocation cost = %ld\t", solver.getIntVar(allocCostVar).getMax());
//			printf("tardiness cost = %.14f\t", solver.getFloatVar(tardiCostVar).getMax());
//			printf("beta = %.14f\t", solver.getIntVar(allocCostVar).getMax()/solver.getFloatVar(tardiCostVar).getMax());
//			printf("at time %.3f", env.getTime() - timeTemp + timeTemp3);
//			printf("\n");
//			bestVal = solver.getFloatVar(globalCostVar).getMax();
//		    globalSolution.store(scheduler);
//			lSSolution.store(scheduler);
//		}
		solver.solve(g);
		IloNum currentValue = solver.getFloatVar(globalCostVar).getMax();
		if(currentValue < bestVal) {
			printf("*** Init. sol.: ");
			printf("global cost = %.14f\t", currentValue);
			printf("alloc. cost = %ld\t", solver.getIntVar(allocCostVar).getMax());
			printf("tardi. cost = %.14f\t", solver.getFloatVar(tardiCostVar).getMax());
			printf("beta = %.14f\t", (solver.getIntVar(allocCostVar).getMax()-kAllocLowerBound)/(solver.getFloatVar(tardiCostVar).getMax()-kTardiLowerBound));
			printf("at time %.3f", env.getTime() - timeTemp + timeTemp3);
			printf("\n");
			bestVal = currentValue;
			globalSolution.store(scheduler);
			lSSolution.store(scheduler);
		}
		
//		else
//			printf("No solution found.\n");
//		solver.printInformation();
		timeTemp3 += env.getTime() - timeTemp;
//		seedCounter++;
	}

	solver.end();
}


IloNum
SolveModelGlobalCost(IloModel					model,
					 IloNumVar					globalCostVar,
					 IloNumVar					allocCostVar,
					 IloNumVar					tardiCostVar,
					 IloArray<IloActivityArray>	actArrays,
					 IloSchedulerSolution&		globalSolution,
					 IloNum						globalSearchTimeLimit,
					 IloInt						globalSearchFailLimit,
					 IloNum						localSearchTimeLimit,
					 IloInt						localSearchFailLimit,
					 IloInt						kAllocLowerBound,
					 IloNum						kTardiLowerBound,
					 IloNum						relaxActProb,
					 IloNum						minImprovePercentage,
					 IloNum						improvePercentageFactor
		//			 IloInt						nbTotalResources
		) {

	IloEnv env = model.getEnv();

	// CREATE LOCAL SEARCH SOLUTION
	IloSchedulerSolution lSSolution = CreateLSSolution(env, globalSolution);

	// GENERATE AN INITIAL SOLUTION.
	globalSolution.getSolution().add(globalCostVar);
	globalSolution.getSolution().add(allocCostVar);
	globalSolution.getSolution().add(tardiCostVar);
	IloNum timeTemp4 = env.getTime();
	FindInitialSolutionGlobalCost(model, globalSolution, lSSolution, globalCostVar, allocCostVar, tardiCostVar, actArrays, globalSearchTimeLimit,
		globalSearchFailLimit, kAllocLowerBound, kTardiLowerBound);//, nbResources);
	IloNum timeTemp5 = env.getTime();
	printf("%.3f CPU second(s) spent to look for a first solution globally.\n", timeTemp5 - timeTemp4);
	IloNum bestGlobalCost = globalSolution.getMax(globalCostVar);
	model.add(globalCostVar <= bestGlobalCost - 1.0);
//	env.out() << "Initial solution" << endl;
//	PrintSolution(env, globalSolution, globalCostVar);

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
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(//-IlcMyAssignAltRCMinCostEvaluator(solver),
																					IlcMyAssignAltRCDispersionEvaluator(lSSolver).makeGreaterThanComparator(),
																					IlcMyAltResConstraintRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator()
																					);
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(-IlcMyAssignAltRCDispersionEvaluator(lSSolver),
//																									IlcMyAltResConstraintAddressEvaluator(lSSolver));
	
//	IloComparator<IlcResource> assignResComparator = IloComposeLexical(IlcMyAssignAltResourceEvaluator(lSSolver),
//																									IlcMyResourceRandomEvaluator(lSSolver, randGenerator));
//	IloComparator<IlcResource> assignResComparator = IloComposeLexical(IlcMyAssignAltResourceEvaluator(lSSolver),
//																									IlcMyResourceAddressEvaluator(lSSolver));
//	IloComparator<IlcResource> assignResComparator = IlcMyResourceRandomEvaluator(lSSolver, randGenerator).makeGreaterThanComparator();
	IloSelector<IlcResource, IlcAltResConstraint> assignAltRSelector = IloMyAllocCostResourceInAltResCtSelector(lSSolver, randGenerator);

//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(lSSolver);
	// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
	// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

//	IloComparator<IlcResource> rankResComparator = IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
/*	IloComparator<IlcResource> rankResComparator = IloComposeLexical(
																		-IlcMyResourceRefDateEvaluator(lSSolver),
																		IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator)
																		);*/
//	IloComparator<IlcResource> rankResComparator = IloComposeLexical(IlcMyRankResourceEvaluator(lSSolver), IlcMyResourceAddressEvaluator(lSSolver));

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


	IluHeuristic heuristic;
	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
//	heuristic.setAssignAltRComparator(assignResComparator);
	heuristic.setAssignAltRSelector(assignAltRSelector);
//	heuristic.setRankResPredicate(rankResPredicate);
//	heuristic.setRankResComparator(rankResComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);

	// SET PARAMETERS FOR LOCAL SEARCH.
	IloObjective obj = IloMinimize(env, globalCostVar);
	lSSolution.getSolution().add(obj);


	// SUB-GOAL
	IloGoal subGoal =
//						IloAssignAlternative(env)
//						&&
						IloMyAssignAlternativeAndRank(env, heuristic)
//						IloRankForward(env) 
//						&& IloInstantiate(env, costVar)
//						&& IloDichotomize(env, costVar, IloTrue)
						&& IloMyInstantiate(env, globalCostVar)
						;

	if(localSearchFailLimit < IloIntMax)
		subGoal = IloLimitSearch(env, subGoal, IloFailLimit(env, localSearchFailLimit));
//	subGoal = IloLimitSearch(env, subGoal, IloTimeLimit(env, localSearchTimeLimit));

//	subGoal = IloSelectSearch(env, subGoal, IloMinimizeVar(env, globalCostVar, 1.0)); // The next solution has a cost equal to the previous cost minus 1.0.

//	IloPredicate<IloActivity> isBeforeSelected = IsActivityBeforeSelected(env); // This is used with time window neighborhood.

	// GLIDING TIME WINDOW SEARCH
//	 IloSchedulerLargeNHood timeWindowNHood = IloTimeWindowNHood(env, 20, 10);

	// relocate activity
//	IloSchedulerLargeNHood relocateActivities = IloRelocateActivityNHood(env);

	// relocate process plan
//	IloComparator<IloActivityArray> myPPComparator = IloMyGlobalDeltaCostPPEvaluator(lSSolver).makeGreaterThanComparator();
	IloComparator<IloActivityArray> myPPComparator = IloMyGainGlobalCostPPEvaluator(lSSolver).makeGreaterThanComparator();
//	IloComparator<IloActivityArray> myPPComparator = IloMyRandomPPEvaluator(lSSolver).makeGreaterThanComparator();

	IloRandom randGenerator3(env, 1);
	IloSchedulerLargeNHood relocatePPNHood = MyRelocateSubsetNHood(env, actArrays, randGenerator3, relaxActProb, myPPComparator);
//	IloSchedulerLargeNHood relocatePPNHood = MyRelocateJobNHood(env, actArrays, myPPComparator);

	IloNHood nhood = 
//						IloContinue(env, relocateActivities)		// IloContinue tries each neighbor and starts from the state in
//						+											// which the schedule was after the last move.
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

	IloNum tempImprovePercentage = minImprovePercentage/10;
	IloNum improveStep = 1.0;//IloMax(1.0, globalSolution.getMax(globalCostVar) * tempImprovePercentage / 100.0);
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
		if(lSSolver.solve(greedyMove)) {
		//	IloNum cost = lsSolution.getSolution().getObjectiveValue();
			IloNum globalCost = lSSolver.getFloatVar(globalCostVar).getMax();
			IloInt allocCost = lSSolver.getIntVar(allocCostVar).getMax();
			IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
			lSSolver.out() << "Move1: " << movesDone << ":\t";
			++movesDone;
			printf("global cost = %.14f\t", globalCost);
			printf("alloc. cost = %ld\t", allocCost);
			printf("tardi. cost = %.14f\t", tardiCost);
			printf("beta = %.14f\t", (allocCost-kAllocLowerBound)/(tardiCost-kTardiLowerBound));
			printf("at time %.3f\n", env.getTime() - timeTemp);
			if(globalCost < bestGlobalCost) {
				bestGlobalCost = globalCost;
				globalSolution.store(lSScheduler);
			}
		//	IloNumVar costVar = lsSolution.getSolution().getObjectiveVar();
		//	env.out() << "Solution" << endl;
		//	PrintSolution(globalSolution, costVar);
		}
		else while((env.getTime() - timeTemp < localSearchTimeLimit)&&(IloFalse == betterSolution)) {
		//	printf("Large Neighborhood Search with relaxing %.1f percent of activities with an improvement step of %.1f\n",
		//			relaxActProb * 100,
		//			improveStep);
			if(lSSolver.solve(greedyMove2)) {
				IloNum globalCost = lSSolver.getFloatVar(globalCostVar).getMax();
				IloInt allocCost = lSSolver.getIntVar(allocCostVar).getMax();
				IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
				betterSolution = IloTrue;
				if(globalCost > 0.0) {
					lSSolver.out() << "Move2: " << movesDone << ":\t";
					++movesDone;
					printf("global cost = %.14f\t", globalCost);
					printf("alloc. cost = %ld\t", allocCost);
					printf("tardi. cost = %.14f\t", tardiCost);
					printf("beta = %.14f\t", (allocCost-kAllocLowerBound)/(tardiCost-kTardiLowerBound));
					printf("at time %.3f\n", env.getTime() - timeTemp);
					if(globalCost < bestGlobalCost) {
						bestGlobalCost = globalCost;
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
		//	else
		//		printf("This relaxation failed!\n");
		}
		betterSolution = IloFalse;
	}
	
	
/*	while((improveStep >= 1.0)&&(env.getTime() - timeTemp < localSearchTimeLimit)) {
		if(lSSolver.solve(greedyMove)) {
		//	IloNum cost = lsSolution.getSolution().getObjectiveValue();
			IloNum globalCost = lSSolver.getFloatVar(globalCostVar).getMax();
			IloInt allocCost = lSSolver.getIntVar(allocCostVar).getMax();
			IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
			lSSolver.out() << "Move: " << movesDone << ":\t";
			++movesDone;
			printf("global cost = %.14f\t", globalCost);
			printf("alloc. cost = %ld\t", allocCost);
			printf("tardi. cost = %.14f\t", tardiCost);
			printf("beta = %.14f\t", (allocCost-kAllocLowerBound)/(tardiCost-kTardiLowerBound));
			printf("at time %.3f\n", env.getTime() - timeTemp);
			if(globalCost < bestGlobalCost) {
				bestGlobalCost = globalCost;
				globalSolution.store(lSScheduler);
			}
		//	IloNumVar costVar = lsSolution.getSolution().getObjectiveVar();
		//	env.out() << "Solution" << endl;
		//	PrintSolution(globalSolution, costVar);
		}
		else
			improveStep = 0.0;
		else {
			tempImprovePercentage *= improvePercentageFactor;
			improveStep = globalSolution.getMax(globalCostVar) * tempImprovePercentage / 100.0;
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
//		improveStep = IloMax(1.0, globalSolution.getMax(globalCostVar) * tempImprovePercentage / 100.0);
//		IloGoal greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, improveStep), IloFirstSolution(env), subGoal);
		printf("Large Neighborhood Search with relaxing %.1f percent of activities with an improvement step of %.1f\n",
				relaxActProb * 100,
				improveStep);
		while(env.getTime() - timeTemp < localSearchTimeLimit) {
			if(lSSolver.solve(greedyMove2)) {
				IloNum globalCost = lSSolver.getFloatVar(globalCostVar).getMax();
				IloInt allocCost = lSSolver.getIntVar(allocCostVar).getMax();
				IloNum tardiCost = lSSolver.getFloatVar(tardiCostVar).getMax();
				if(globalCost > 0.0) {
					lSSolver.out() << "Move: " << movesDone << ":\t";
					++movesDone;
					printf("global cost = %.14f\t", globalCost);
					printf("alloc. cost = %ld\t", allocCost);
					printf("tardi. cost = %.14f\t", tardiCost);
					printf("beta = %.14f\t", (allocCost-kAllocLowerBound)/(tardiCost-kTardiLowerBound));
					printf("at time %.3f\n", env.getTime() - timeTemp);
					if(globalCost < bestGlobalCost) {
						bestGlobalCost = globalCost;
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
				improveStep = IloMax(1.0, globalSolution.getMax(globalCostVar) * tempImprovePercentage / 100.0);
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

	printf("Final solution at global cost: %.14f\n\n", bestGlobalCost);
//	PrintSolution(globalSolution, costVar);
	
	return bestGlobalCost;
}



IloModel
DefineModelGlobalCost(IloEnv&						env,
					  IloInt						numberOfProcessPlans,
					  IloInt						numberOfActivitiesPerProcessPlan,
					  IloInt						numberOfResourcesPerActivity,
					  IloInt						numberOfResources,
					  IloInt***						resourceNumbers,
					  IloInt**						durations,
					  IloIntervalList*				breakList,
					  IloInt**						allocCosts,
					  IloInt*						dueDates,
					  IloNum*						phis,
					  IloNum						beta,
					  IloNum						gamma,
					  IloNum						omega,
					  IloRandom						randomGenerator,
					  IloSchedulerSolution			solution,
					  IloArray<IloActivityArray>&	actArrays,
					  IloNumVar&					globalCostVar,
					  IloNumVar&					allocCostVar,
					  IloNumVar&					tardiCostVar) {
	IloModel model(env);
	IloInt numberOfActivities = numberOfProcessPlans * numberOfActivitiesPerProcessPlan;
	IloInt i,j,k,l;

	// CREATE THE RESOURCES WITH BREAKS.
	IloSchedulerEnv schedEnv(env);
	IloResourceParam resParam = schedEnv.getResourceParam();
	resParam.setCapacityEnforcement(IloMediumLow);
//	resParam.setPrecedenceEnforcement(IloMediumHigh);		// Precedence graph
	
	char buffer[128];
	IloUnaryResource* resources = new(env) IloUnaryResource[numberOfResources];
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
	
	  // CREATE THE ACTIVITIES, POST INITIAL TEMPORAL CONSTRAINTS, AND POST RESOURCE CONSTRAINTS.
	printf("Creation of activities and initial precedence constraints added\n");
	IloActivity* activities = new(env) IloActivity[numberOfActivities];
	IloActivity previousActivity;
	actArrays = IloArray<IloActivityArray>(env);
	for(i = 0; i < numberOfProcessPlans; i++) {
		IloActivityArray actArray(env);
		actArrays.add(actArray);
		IloInt tempQueue = 0;
		IloNum betaPhi = beta * phis[i];
		IluProcPlan* procPlan = new(env) IluProcPlan(env, i, IlcIntMin, betaPhi, gamma, omega, phis[i], dueDates[i], IloIntMax, -IlcInfinity, actArray);
	    for(j = numberOfActivitiesPerProcessPlan - 1; j >= 0; j--) {
			k = i * numberOfActivitiesPerProcessPlan + j;
			activities[k] = IloActivity(env, durations[i][j]);
		//	printf("Activity %ld: duration %ld\n", k, durations[i][j]);
			IluInfoActivity* infoAct = new(env) IluInfoActivity(k, tempQueue, allocCosts[k], 0, procPlan);
		//	for(l = 0; l < numberOfResources; l++)
		//		printf("\tResource %ld costs: %ld\n", l, allocCosts[k][l]);
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
			sprintf(buffer, "PP%ldS%ld", i, j);
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

	// CREATE THE TARDINESS COST VARIABLE AND POST THE TARDINESS COST CONSTRAINT.
		printf("Creating a tardiness variable and posting the tardiness cost constraint\n");
		tardiCostVar = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT, "Tardiness");
		IloExpr expr;
		for(i = 1; i < numberOfProcessPlans + 1; i++) {
			IloExpr tardi = IloMax(activities[i * numberOfActivitiesPerProcessPlan - 1].getEndExpr() - dueDates[i - 1], 0.0);
			tardi = phis[i - 1] * tardi;
			if(0 == expr.getImpl())
				expr = tardi;
			else
				expr = expr + tardi;
		}
		model.add(tardiCostVar == expr);


	// CREATE THE ALLOCATION COST VARIABLE AND POST THE ALLOCATION COST CONSTRAINT.
		printf("Creating an allocation cost variable and posting the allocation cost constraint\n");
		IloNum lowerAllocCost = 0.0;
		IloNum upperAllocCost = 0.0;
		IloNum maxInt = (IloNum)IloIntMax;
		for(i = 0; i < numberOfProcessPlans; i++) {
			for(j = 0; j < numberOfActivitiesPerProcessPlan; j++) {
				IloInt lowerAllocCostTemp = IloIntMax;
				IloInt upperAllocCostTemp = 0;
				for(k = 0; k < numberOfResourcesPerActivity; k++) {
					if(allocCosts[i * numberOfActivitiesPerProcessPlan + j][resourceNumbers[i][j][k]] < lowerAllocCostTemp)
						lowerAllocCostTemp = allocCosts[i * numberOfActivitiesPerProcessPlan + j][resourceNumbers[i][j][k]];
					if(allocCosts[i * numberOfActivitiesPerProcessPlan + j][resourceNumbers[i][j][k]] > upperAllocCostTemp)
						upperAllocCostTemp = allocCosts[i * numberOfActivitiesPerProcessPlan + j][resourceNumbers[i][j][k]];
				}
				lowerAllocCost += (IloNum)lowerAllocCostTemp;
				upperAllocCost += (IloNum)upperAllocCostTemp;
			}
		}
		if((lowerAllocCost > maxInt)||(upperAllocCost > maxInt)) {
			printf("Bounds of the allocation cost variable: interger overflow! Allocation constraint and global cost constraint are not posted.\n");
		}
		else {
			allocCostVar = IloIntVar(env, (IloInt)lowerAllocCost, (IloInt)upperAllocCost, "Allocation");
			model.add(IloGlobalAllocationCostConstraint(env, allocCostVar, allocCosts, 0));


		// CREATE THE GLOBAL COST VARIABLE AND POST THE GLOBAL COST CONSTRAINT.
			printf("Creating a global cost variable and posting the global cost constraint\n");
			globalCostVar = IloNumVar(env, lowerAllocCost, IloInfinity, "Global cost");
			model.add(globalCostVar == allocCostVar + beta * tardiCostVar);
		}

  // RETURN THE MODEL
  return model;
}
