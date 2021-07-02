// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the FindInitialSolutionGlobalCost function, the SolveModelGlobalCost function, and the DefineModelGlobalCost function.


#ifndef __SIM_iluglobalcostsimH
#define __SIM_iluglobalcostsimH

#include <ilsched/ilolnsgoals.h>
#include <ilsolver/iimmeta.h>
#include <ilsolver/iimls.h>
#include <ilsim/iluheuristic.h>




////////////////////////////////////////////////////////////////////
//
// FINDING A FIRST SOLUTION
//
////////////////////////////////////////////////////////////////////
void
FindInitialSolutionGlobalCost(IloModel						model,
							  IloSchedulerSolution			globalSolution,
							  IloSchedulerSolution			lsSolution,
							  IloNumVar						costVar,
							  IloArray<IloActivityArray>	actArrays,
							  IloNum						globalSearchTimeLimit,
							  IloInt						globalSearchFailLimit,
							  IloInt						kAllocLowerBound,
							  IloNum						kTardiLowerBound
						//	  IloInt nbResources
							  );


////////////////////////////////////////////////////////////////////
//
// SOLVING THE MODEL BY USING A GREEDY DESCENT SEARCH
//
////////////////////////////////////////////////////////////////////
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
				//	 IloInt						nbTotalResources
					 );




////////////////////////////////////////////////////////////////////
//
// DEFINING THE MODEL WITH ALTERNATIVE RESOURCES
//
////////////////////////////////////////////////////////////////////
IloModel
DefineModelGlobalCost(IloEnv&						env,
					  IloInt						numberOfJobs,
					  IloInt						numberOfActivitiesPerJob,
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
					  IloNumVar&					tardiCostVar
					  );




#endif