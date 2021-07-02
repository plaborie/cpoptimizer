// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilumakespan.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the FindInitialSolution function
// the SolveModel function, the CreateLSSolution function, and the DefineModel function.

#include<ilsim/ilumakespan.h>
#include<ilsim/iluneighborhood.h>
#include<ilsim/iluheuristic.h>
#include<ilsim/iluprintsolution.h>
#include<ilsim/ilurandom.h>

ILOSTLBEGIN





void FindInitialSolution(IloModel				model,
                         IloSchedulerSolution	globalSolution,
						 IloSchedulerSolution	lSSolution,
                         IloNumVar				costVar,
						 IloNum					globalSearchTimeLimit,
						 IloInt					globalSearchFailLimit
						// IloInt nbResources
						 ) {
	IloEnv env = model.getEnv();
//	model.add(IloMinimize(env, costVar));

	IloSolver solver0(model);
	solver0.solve(IloPrintLowerBound(env, costVar));
	solver0.end();

	IloSolver solver(model);
	IloRandom randGenerator(env, 1);
//	MyRandom* randGenerator = new(env) MyRandom(solver, 1);

	IlcScheduler scheduler(solver);

	IloInt bestVal = IloIntMax;

		// SEARCH HEURISTIC
	IloPredicate<IlcAltResConstraint> assignAltRCPredicate = !IlcAltResConstraintResourceSelectedPredicate(solver);
	
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAssignAltRCNbPossibleEvaluator(solver).makeLessThanComparator();
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcMyAssignAltRCNbPossibleEvaluator(solver),
//																									IlcMyAltResConstraintAddressEvaluator(solver));
//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcMyAssignAltRCNbPossibleEvaluator(solver),
//																						IlcMyAltResConstraintMyRandomEvaluator(solver, randGenerator));
	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(solver).makeLessThanComparator();
	

//	IloComparator<IlcResource> resComparator = IlcResourceGlobalSlackEvaluator(solver).makeLessThanComparator();
//	IloComparator<IlcResource> resComparator = IloComposeLexical(IlcResourceGlobalSlackEvaluator(solver), IlcMyResourceAddressEvaluator(solver));
//	IloComparator<IlcResource> resComparator = IloComposeLexical(-IlcResourceGlobalSlackEvaluator(solver),
//																	IlcMyResourceMyRandomEvaluator(solver, randGenerator));
	IloComparator<IlcResource> resComparator = IlcMyResourceRandomEvaluator(solver, randGenerator).makeLessThanComparator();
	
//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(solver);
	

	IloPredicate<IlcResourceConstraint> rankRCPredicate = IlcResourceConstraintPossibleFirstPredicate(solver);
	
	IloComparator<IlcResourceConstraint> rankRCComparator = IloComposeLexical(
																				IlcMyResourceConstraintStartMinEvaluator(solver).makeLessThanComparator(),
																				IlcMyResourceConstraintEndMaxEvaluator(solver).makeLessThanComparator(),
																			//	IlcMyResourceConstraintDurationMinEvaluator(solver).makeLessThanComparator(),
																				IlcMyResourceConstraintRandomEvaluator(solver, randGenerator).makeLessThanComparator()
																				);
//	IloComparator<IlcResourceConstraint> rankRCComparator = IlcMyResourceConstraintRandomEvaluator(solver, randGenerator).makeLessThanComparator();

	IluHeuristic heuristic;
	heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
	heuristic.setAssignAltRCComparator(assignAltRCComparator);
	heuristic.setAssignAltRComparator(resComparator);
//	heuristic.setRankResPredicate(rankResPredicate);
//	heuristic.setRankResComparator(resComparator);
	heuristic.setRankRCPredicate(rankRCPredicate);
	heuristic.setRankRCComparator(rankRCComparator);

	IloGoal g = 
//				IloPrintLowerBound(env, costVar)
//				&&
//				IloAssignAlternative(env) 
//				&&
//				IloRankForward(env) 
//				&&
//				IloMyMinimizeIntVar(env, costVar, bestVal)
//				&&
				IloMyAssignAlternativeAndRank(env, heuristic)
//				&& IloSetTimesForward(env) 
//				&& IloDichotomize(env, costVar, IloTrue)
				&& IloMyInstantiate(env, costVar)
				;

//	IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
//	g = IloApply(env, g, myNodeEvaluator);

//	g = IloLimitSearch(env, g, IloTimeLimit(env, globalSearchTimeLimit));
//	if(globalSearchFailLimit < IloIntMax)
//		g = IloLimitSearch(env, g, IloFailLimit(env, globalSearchFailLimit));

//	solver.setRelativeOptimizationStep(0.05);
//	solver.setOptimizationStep(1.0);

/*	solver.startNewSearch(g);
//  IloInt* allocatedResources = new(env) IloInt[nbResources];
//  for(IloInt i = 0; i < nbResources; i++)
//	  allocatedResources[i] = 0;

	IloNum timeTemp = env.getTime();
	while(solver.next()) {
//		IloNum best = solver.getMin(costVar);
		printf("*** Initial solution at cost = ");
		printf("%ld\tat time %.3f\n", solver.getIntVar(costVar).getMax(), env.getTime() - timeTemp);
//		solver.out() << "*** Initial solution at cost: " <<  best << endl;
//		solver.printInformation();
	    globalSolution.store(scheduler);
		lSSolution.store(scheduler);

//		IloInt nbAllocatedResources = nbResources;
//		IloInt nbActivities = 0;
//		IloInt nbSuccessors = 0;
//		for(IloSchedulerSolution::ResourceIterator rsIte(globalSolution); rsIte.ok(); ++rsIte)
//			nbResources++;

//		for(IloSchedulerSolution::ResourceConstraintIterator ite(globalSolution); ite.ok(); ++ite) {
//			IloResourceConstraint rc = *ite;
//			IloResource res = globalSolution.getSelected(rc);
//			IloInt resIndex = (IloInt)res.getObject();
//			printf("Resource %ld has been selected.\n", resIndex);
//			allocatedResources[resIndex] = 1;
//			if(globalSolution.hasNextRC(rc))
//				nbSuccessors++;
//			nbActivities++;
//		}

//		for(IloInt i = 0; i < nbResources; i++) {
//			if(0 == allocatedResources[i])
//				nbAllocatedResources--;
//		}
//		printf("In this solution, there are:\n");
//		printf("\t%ld activities\n", nbActivities);
//		printf("\t%ld resources\n", nbResources);
//		printf("\t%ld allocated resources\n", nbAllocatedResources);
//		printf("\t%ld successors\n", nbSuccessors);
//		if(nbSuccessors != nbActivities - nbAllocatedResources)
//			printf("Warning: too few successors!\n");

	}

	solver.endSearch();*/

	// FAST RESTARTS WITH A SMALL FAIL LIMIT
	IloNum timeTemp3 = 0.0;
/*	while(timeTemp3 < globalSearchTimeLimit) {
		IloNum timeTemp = env.getTime();
		g = IloLimitSearch(env, g, IloFailLimit(env, globalSearchFailLimit));
		if(solver.solve(g)) {
			printf("*** Initial solution at cost = ");	
			printf("%ld\tat time %.3f\n", solver.getIntVar(costVar).getMax(), timeTemp3 + env.getTime() - timeTemp);
			bestVal = solver.getValue(costVar);
		    globalSolution.store(scheduler);
			lSSolution.store(scheduler);
		}
//		globalSearchFailLimit++;
//		else
//			printf("No solution found.\n");
		timeTemp3 += env.getTime() - timeTemp;
	}*/
	while(timeTemp3 < globalSearchTimeLimit) {
		IloNum timeTemp = env.getTime();
		solver.solve(g);
		IloInt currentValue = solver.getIntVar(costVar).getMax();
		if(currentValue < bestVal) {
			printf("*** Initial solution at cost = ");	
			printf("%ld\tat time %.3f\n", currentValue, timeTemp3 + env.getTime() - timeTemp);
			bestVal = currentValue;
		    globalSolution.store(scheduler);
			lSSolution.store(scheduler);
		//	solver.printInformation();
		//	exit(0);
		}
//		globalSearchFailLimit++;
//		else
//			printf("No solution found.\n");
		timeTemp3 += env.getTime() - timeTemp;
	}

	solver.end();
//	model.remove(IloMinimize(env, costVar));
}




IloNum
SolveModel(IloModel						model,
		   IloNumVar					costVar,
	//	   IloArray<IloActivityArray>	actArrays,
		   IloSchedulerSolution&		globalSolution,
		   IloNum						globalSearchTimeLimit,
		   IloInt						globalSearchFailLimit,
		   IloNum						localSearchTimeLimit,
		   IloInt						localSearchFailLimit,
		   IloNum						relaxActProb
	//	   IloInt						nbResources
		   ) {

	IloEnv env = model.getEnv();

	// CREATE LOCAL SEARCH SOLUTION
	IloSchedulerSolution lSSolution = CreateLSSolution(env, globalSolution);

	IloNum best = IloInfinity;
//	model.add(IloMinimize(env, costVar));
	globalSolution.getSolution().add(costVar);
//	IloNum timeTemp3;
//	IloBool endSearch = IloFalse;
//	while(IloFalse == endSearch) {
		// GENERATE AN INITIAL SOLUTION.
		IloNum timeTemp4 = env.getTime();
		FindInitialSolution(model, lSSolution, globalSolution, costVar, globalSearchTimeLimit, globalSearchFailLimit);//, nbResources);
		IloNum timeTemp5 = env.getTime();
		printf("%.3f CPU second(s) spent to look for a solution globally.\n", timeTemp5 - timeTemp4);
		IloNum bestTemp = globalSolution.getMax(costVar);
//		if(bestTemp < best) {
			best = bestTemp;
			model.add(costVar < best);
		//	env.out() << "Initial solution" << endl;
		//	PrintSolution(env, globalSolution, costVar);

			IloSolver lSSolver(model);
			IlcScheduler lSScheduler(lSSolver);
			IloRandom randGenerator(env, 1);
		//	IlcRandom randGenerator(lSSolver, 1);
		//	MyRandom* randGenerator = new(env) MyRandom(lSSolver, 1);
		//	lSSolver.solve(IloInitRandom(env, randGenerator));

			// SEARCH HEURISTIC
			IloPredicate<IlcAltResConstraint> assignAltRCPredicate = !IlcAltResConstraintResourceSelectedPredicate(lSSolver);
			// This function returns an alternative resource constraint predicate whose operator(const IlcAltResConstraint& altrc) returns IlcTrue if and only if
			// a single resource has been selected for the activity corresponding to altrc. If there are still multiple resources that can be selected, IlcFalse
			// is returned.
		//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAssignAltRCNbPossibleEvaluator(lSSolver).makeLessThanComparator();
		//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcMyAssignAltRCNbPossibleEvaluator(lSSolver),
		//																									IlcMyAltResConstraintAddressEvaluator(lSSolver));
		//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(IlcMyAssignAltRCNbPossibleEvaluator(lSSolver),
		//																						IlcMyAltResConstraintMyRandomEvaluator(lSSolver, randGenerator));
			// This function returns an IloEvaluator<IlcAltResConstraint> whose operator(const IlcAltResConstraint& a) method returns the number of resources
			// that are possible for a.
		//	IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltResConstraintMyRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
			IloComparator<IlcAltResConstraint> assignAltRCComparator = IlcMyAltRCConstantEvaluator(lSSolver).makeLessThanComparator();
		/*	IloComparator<IlcAltResConstraint> assignAltRCComparator = IloComposeLexical(
																					IlcAltResConstraintNbPossibleEvaluator(lSSolver),
																					IlcMyAltRCConstantEvaluator(lSSolver)
																			//		IlcMyAltResConstraintMyRandomEvaluator(lSSolver, randGenerator)
																					);*/


		//	IloComparator<IlcResource> resComparator = IlcResourceGlobalSlackEvaluator(lSSolver).makeLessThanComparator();
		//	IloComparator<IlcResource> resComparator = IloComposeLexical(IlcResourceGlobalSlackEvaluator(lSSolver), IlcMyResourceAddressEvaluator(lSSolver));
		//	IloComparator<IlcResource> resComparator = IloComposeLexical(-IlcResourceGlobalSlackEvaluator(lSSolver),
		//																	IlcMyResourceMyRandomEvaluator(lSSolver, randGenerator));
			// This function returns a resource evaluator whose operator(const IlcResource& resource) returns the global slack of the resource object to which it
			// is applied.
			IloComparator<IlcResource> resComparator = IlcMyResourceRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator();
			
		//	IloPredicate<IlcResource> rankResPredicate = !IlcResourceRankedPredicate(lSSolver);
			// This function returns a resource predicate whose operator(const IlcResource& resource) returns IlcTrue if and only if ranking is not supported on resource
			// (if IlcResource::hasRankInfo returns IlcFalse) or if ranking is supported and the resource constraints on the resource are completely ranked.

			IloPredicate<IlcResourceConstraint> rankRCPredicate = IlcResourceConstraintPossibleFirstPredicate(lSSolver);
			// This function returns a resource constraint predicate whose operator(const IlcResourceConstraint& rc) returns IlcTrue if and only if rc can be ranked
			// first among the non-ranked resource constraints. In particular, it returns IlcFalse if rc is already ranked or if rc represents a virtual source
			// or sink node (empty handle).
			IloComparator<IlcResourceConstraint> rankRCComparator = IloComposeLexical(
																						IlcMyResourceConstraintStartMinEvaluator(lSSolver).makeLessThanComparator(),
																						IlcMyResourceConstraintEndMaxEvaluator(lSSolver).makeLessThanComparator(),
																						IlcMyResourceConstraintRandomEvaluator(lSSolver, randGenerator).makeLessThanComparator()
																						);

			IluHeuristic heuristic;
			heuristic.setAssignAltRCPredicate(assignAltRCPredicate);
			heuristic.setAssignAltRCComparator(assignAltRCComparator);
			heuristic.setAssignAltRComparator(resComparator);
		//	heuristic.setRankResPredicate(rankResPredicate);
		//	heuristic.setRankResComparator(resComparator);
			heuristic.setRankRCPredicate(rankRCPredicate);
			heuristic.setRankRCComparator(rankRCComparator);

			// SET PARAMETERS FOR LOCAL SEARCH.
			IloObjective obj = IloMinimize(env, costVar);
			lSSolution.getSolution().add(obj);

			// SUB-GOAL
			IloGoal subGoal =
//								IloMyPrintStartSubGoal(env)
//								&&
		//						IloAssignAlternative(env)
		//						&&
		//						IloRankForward(env) 
		//						&&
								IloMyAssignAlternativeAndRank(env, heuristic)
		//						&& IloInstantiate(env, costVar)
		//						&& IloDichotomize(env, costVar, IloTrue)
								&&
								IloMyInstantiate(env, costVar)
						//		&& IloPrintSolution(env, costVar)
//								&&
//								IloMyPrintEndSubGoal(env)
								;

			if(localSearchFailLimit < IloIntMax)
				subGoal = IloLimitSearch(env, subGoal, IloFailLimit(env, localSearchFailLimit));
		//	subGoal = IloLimitSearch(env, subGoal, IloTimeLimit(env, localSearchTimeLimit));

		//	subGoal = IloSelectSearch(env, subGoal, IloMinimizeVar(env, costVar, 1.0));

		//	IloPredicate<IloActivity> isBeforeSelected = IsActivityBeforeSelected(env); // This is used with time window neighborhood.

			// GLIDING TIME WINDOW SEARCH
		//	 IloSchedulerLargeNHood timeWindowNHood = IloTimeWindowNHood(env, 20, 10);

			// relocate activity
		//	IloSchedulerLargeNHood relocateActivities = IloRelocateActivityNHood(env);

	/*		// relocate process plan
			IloSchedulerLargeNHood relocatePPNHood = MyRelocateJobNHood(env, actArrays);

			IloNHood nhood = 
		//						IloContinue(env, relocateActivities)
		//						+
								IloContinue(env, relocatePPNHood)*/
		//						+
		//						IloContinue(env, timeWindowNHood)
								;

			// SBS=LDS
	//		IloNodeEvaluator myNodeEvaluator = IloSBSEvaluator(env, 1);
	//		subGoal = IloApply(env, subGoal, myNodeEvaluator);

/*			IloGoal greedyMove;

			if(costVar.getType() == IloNumVar::Int)
				greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env), IloFirstSolution(env), subGoal);
			else
				greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);*/

			
		//	greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env), IloFirstSolution(env), subGoal);
		//	greedyMove = IloSingleMove(env, lSSolution, nhood, IloImprove(env, 1.0), subGoal);


			// relocate activities
			IloRandom randGenerator2(env, 1);
			IloSchedulerLargeNHood relocateActNHood = RandomActivityNHood(env, randGenerator2, relaxActProb);

		//	IloNHood nhood2 = IloContinue(env, relocateActNHood);		// It does not make sense because size equals 1.
			IloNHood nhood2 = relocateActNHood;

			IloGoal greedyMove2;

			if(costVar.getType() == IloNumVar::Int)
				greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env), IloFirstSolution(env), subGoal);		// improve step = 0.0001
			else
				greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);




			IloInt movesDone = 0;
		//	IloInt globalNbFails = 0;
		//	IloNum lastTime = 0.0;
		//	IloInt localRatio = 2;
			IloNum timeTemp = env.getTime();

		//	IloBool atLeastOneMove = IloFalse;
/*			printf("Large Neighborhood Search with relaxing a process plan\n");
			while((env.getTime() - timeTemp < localSearchTimeLimit)&&(lSSolver.solve(greedyMove))) {
							// IloSingleMove permits LNS to visit all neighbors and returns true when a
							// better solution is found; it returns false when no better solution is found after
							// visiting all neighbors.
				//	IloNum cost = lSSolution.getSolution().getObjectiveValue();
					IloInt cost = lSSolver.getIntVar(costVar).getMin();
					if(cost > 0) {
						lSSolver.out() << "Move: " << movesDone << ":\t solution at cost: ";
						++movesDone;
						printf("%ld\t at time: %.3f ** HC\n", cost, env.getTime() - timeTemp);
						best = cost;
						globalSolution.store(lSScheduler);
					//	atLeastOneMove = IloTrue;
					}
				//	if(0 == cost) {
				//		PrintSolution(env, lSScheduler);
				//		IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
				//		env.out() << "Solution" << endl;
				//		PrintSolution(globalSolution, costVar);
				//	}
			}*/

			if(env.getTime() - timeTemp < localSearchTimeLimit) {
				printf("Large Neighborhood Search with relaxing %.1f percent of activities\n", relaxActProb * 100);
				while(env.getTime() - timeTemp < localSearchTimeLimit) {
					if(lSSolver.solve(greedyMove2)) {
					//	IloNum cost = lSSolution.getSolution().getObjectiveValue();
						IloInt cost = lSSolver.getIntVar(costVar).getMin();
						if(cost > 0) {
							lSSolver.out() << "Move: " << movesDone << ":\t solution at cost: ";
							++movesDone;
						//	lastTime = env.getTime();
							printf("%ld\t at time: %.3f ** HC\n", cost, env.getTime() - timeTemp);
							best = cost;
							globalSolution.store(lSScheduler);
						//	globalNbFails = 0;
						//	atLeastOneMove = IloTrue;
						}
					//	if(0 == cost) {
					//		PrintSolution(env, lSScheduler);
					//		IloNumVar costVar = lSSolution.getSolution().getObjectiveVar();
					//		env.out() << "Solution" << endl;
					//		PrintSolution(globalSolution, costVar);
					//	}
					}
			/*		if(env.getTime() - lastTime > 60.0) {
						lastTime = env.getTime();
						subGoal = IloLimitSearch(env, subGoal, IloFailLimit(env, localSearchFailLimit * localRatio));
						relocateActNHood = RandomActivityNHood(env, randGenerator2, relaxActProb * localRatio);
						nhood2 = relocateActNHood;
						greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);
						printf("Large Neighborhood Search with relaxing %.1f percent of activities\n", relaxActProb * 100 * localRatio);
					//	if(localRatio * relaxActProb <= 0.5)
						localRatio *= 2;
					}*/
				/*	else {
				//		printf("LNS has failed (activities)!\n");
						globalNbFails++;
						if(300 == globalNbFails) {
							globalNbFails = 0;
							relaxActProb *= 2;
							if(relaxActProb < 100.0) {
								relocateActNHood = RandomActivityNHood(env, randGenerator2, relaxActProb);
								nhood2 = relocateActNHood;
								greedyMove2 = IloSingleMove(env, lSSolution, nhood2, IloImprove(env, 1.0), IloFirstSolution(env), subGoal);
								printf("Large Neighborhood Search with relaxing %.1f percent of activities\n", relaxActProb * 100);
							}
						}
					}*/
				}
			}

		//	PrintSolution(globalSolution, costVar);

			lSSolver.end();
//			if(IloTrue == atLeastOneMove)
//				model.add(costVar < best);
//		}
//		else
//			endSearch = IloTrue;
//	}

	printf("Final solution with a makespan = %ld\n\n", (IloInt)best);

	return best;
}



IloModel
DefineModel(IloEnv&						env,
            IloInt						numberOfProcPlans,
			IloInt						numberOfActivitiesPerProcPlan,
            IloInt						numberOfResourcesPerActivity,
			IloInt						numberOfResources,
            IloInt***					resourceNumbers,
            IloInt**					durations,
			IloIntervalList*			breakList,
            IloRandom					randomGenerator,
            IloSchedulerSolution		solution,
	//		IloArray<IloActivityArray>&	actArrays,
            IloNumVar&					makespan) {
	IloModel model(env);
	IloInt numberOfActivities = numberOfProcPlans * numberOfActivitiesPerProcPlan;
	IloInt i,j,k,l;

	IloInt horizon = 0;
	IloNum numHorizon = 0.0;
	IloNum intMax = (IloNum)IloIntMax;
	for(i = 0; i < numberOfProcPlans; i++) {
		for (j = 0; j < numberOfActivitiesPerProcPlan; j++) {
			numHorizon += (IloNum)durations[i][j];
			if(numHorizon < intMax)
				horizon += durations[i][j];
			else {
				printf("Horizon upper bound: Integer capacity is overflowed!\n");
				horizon = IloIntMax;
			}
		}
	}

	// CREATE THE RESOURCES WITH BREAKS
	IloSchedulerEnv schedEnv(env);
	IloResourceParam resParam = schedEnv.getResourceParam();
//	schedEnv.setPrecedenceEnforcement(IloExtended);
//	resParam.setCapacityEnforcement(IloLow);	// Time table constraint
	resParam.setCapacityEnforcement(IloMediumLow);			// Light precedence graph
//	resParam.setPrecedenceEnforcement(IloMediumHigh);		// Precedence graph
	
	char buffer[128];
	IloUnaryResource* resources = new(env) IloUnaryResource[numberOfResources];
	for(i = 0; i < numberOfResources; i++) {
		sprintf(buffer, "R%ld", i);
		resources[i] = IloUnaryResource(env, buffer);
		resources[i].setObject((void*)(i));
		solution.add(resources[i]);
		IloInt endTemp = 0;
		for(IloIntervalListCursor cursor(breakList[i]); cursor.ok(); ++cursor) {
			resources[i].addBreak(cursor.getStart(), cursor.getEnd());
			endTemp = cursor.getEnd();
		}
		if(endTemp > horizon)
			horizon = endTemp;
	}

	// CREATE THE MAKESPAN VARIABLE.
	makespan = IloNumVar(env, 0.0, (IloNum)horizon, ILOINT, "Makespan");

	// CREATE THE ALTERNATIVE RESOURCE SET.
	printf("Creating an alternative resource set\n");
	IloAltResSet altResSet(env);
	for(i = 0; i < numberOfResources; i++) {
		altResSet.add(resources[i]);
	}
	IloBool* rejectedResources = new(env) IloBool[numberOfResources];
	
	  // CREATE ACTIVITIES AND PROCESS PLANS, AND ADD INITIAL TEMPORAL CONSTRAINTS.
	printf("Creation of activities, process plans, and initial precedence constraints added\n");
	IloActivity* activities = new(env) IloActivity[numberOfActivities];
	IloActivity previousActivity;
//	actArrays = IloArray<IloActivityArray>(env);
	for(i = 0, k = 0; i < numberOfProcPlans; i++) {
	//	IloActivityArray actArray(env);
	//	actArrays.add(actArray);
		IluProcPlan* procPlan = new(env) IluProcPlan(env, i, IlcIntMin, 0.0, 0.0, 0.0, 0.0, 0, IloIntMax, -IlcInfinity, 0);
	    for (j = 0; j < numberOfActivitiesPerProcPlan; j++, k++) {
			activities[k] = IloActivity(env, durations[i][j]);
		//	printf("Activity %ld: duration %ld\n", k, durations[i][j]);
			IluInfoActivity* infoAct = new(env) IluInfoActivity(k, 0, 0, 0, procPlan);
			activities[k].setObject(infoAct);
			activities[k].setBreakable();
		//	model.add(activities[k]);
			if(j > 0) {
				IloPrecedenceConstraint tCt = activities[k].startsAfterEnd(previousActivity);
			//	printf("A precedence constraint is added to the model between activity %ld and activity %ld.\n", k - 1, k);
				model.add(tCt);
			}
			previousActivity = activities[k];
		//	actArray.add(activities[k]);
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

			model.add(rCt);
			solution.add(rCt, IloRestoreNothing);
		}
	//	printf("A precedence constraint is added to the model between activity %ld and makespan.\n", k - 1);
		model.add(previousActivity.endsBefore(makespan));
	}

  // RETURN THE MODEL
  return model;
}




IloInt
ComputeAllocCost(IloSchedulerSolution	solution,
				 IloInt**				allocCosts) {

	IloInt allocCost = 0;
	for(IloSchedulerSolution::ResourceConstraintIterator iter(solution); iter.ok(); ++iter) {
		IloResourceConstraint rCt = *iter;
		IloActivity act = rCt.getActivity();
		IluInfoActivity* infoAct = (IluInfoActivity*) act.getObject();
		IloInt actIndex = infoAct->getActIndex();
		IloUnaryResource res = (IloUnaryResource) solution.getSelected(rCt);
		IloInt resIndex = (IloInt) res.getObject();
//		printf("Activity %ld is allocated to resource %ld.\n", actIndex, resIndex);
		allocCost += allocCosts[actIndex][resIndex];	
	}
	printf("Allocation cost of this solution is equal to %ld.\n", allocCost);
	return allocCost;
}


void
RetrieveEffProcPlanEndTimes(IloSchedulerSolution	solution,
							IloInt*					effProcPlanEndTimes) {
	for(IloSchedulerSolution::ActivityIterator iter(solution); iter.ok(); ++iter) {
		IloActivity act = *iter;
		IluInfoActivity* infoAct = (IluInfoActivity*) act.getObject();
		IloInt procPlanIndex = infoAct->getProcPlan()->getProcPlanIndex();
		IloInt endTime = solution.getEndMin(act);
		if(endTime > effProcPlanEndTimes[procPlanIndex])
			effProcPlanEndTimes[procPlanIndex] = endTime;
	}
}