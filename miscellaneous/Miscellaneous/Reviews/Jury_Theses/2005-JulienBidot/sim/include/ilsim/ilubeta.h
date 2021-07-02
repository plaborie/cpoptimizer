// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the DefineModelBeta function.

#ifndef __SIM_ilubetasimH
#define __SIM_ilubetasimH


#include <ilsched/iloscheduler.h>
#include <ilsim/iluheuristic.h>



////////////////////////////////////////////////////////////////////
//
// FINDING A FIRST SOLUTION
//
////////////////////////////////////////////////////////////////////

void FindInitialSolutionBeta(IloModel				model,
							 IloSchedulerSolution	globalSolution,
							 IloSchedulerSolution	lsSolution,
							 IloNumVar				tardiCostVar,
							 IluHeuristic			heuristic,
							 IloNum					globalSearchTimeLimit,
							 IloInt					globalSearchFailLimit
							 );




////////////////////////////////////////////////////////////////////
//
// SOLVING THE MODEL BY USING A GREEDY DESCENT SEARCH
//
////////////////////////////////////////////////////////////////////

IloNum
SolveModelBeta(IloModel						model,
			   IloNumVar					tardiCostVar,
			   IloArray<IloActivityArray>	actArrays,
			   IloSchedulerSolution&		globalSolution,
			   IloNum						globalSearchTimeLimit,
			   IloInt						globalSearchFailLimit,
			   IloNum						localSearchTimeLimit,
			   IloInt						localSearchFailLimit
			   );


////////////////////////////////////////////////////////////////////
//
// DEFINING THE MODEL WITH ALTERNATIVE RESOURCES
//
////////////////////////////////////////////////////////////////////

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
				IloNumVar&					tardiCostVar);




#endif
