// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the FindInitialSolution function, the IloRCFalsePredicate
// predicate, the IloRCTrueIfNotSelectedPredicate predicate, the IloActivityFalsePredicate
// predicate, the RelocateJobNHoodI class, the IsActivityBeforeSelected
// predicate, the SolveModel function, and the DefineModel function.

#ifndef __SIM_ilumakespansimH
#define __SIM_ilumakespansimH

#include<ilsched/ilolnsgoals.h>
#include<ilsolver/iimmeta.h>
#include<ilsolver/iimls.h>
#include<ilsim/iluheuristic.h>
#include<ilsim/iluprocessplan.h>









////////////////////////////////////////////////////////////////////
//
// FINDING A FIRST SOLUTION
//
////////////////////////////////////////////////////////////////////

void FindInitialSolution(IloModel				model,
                         IloSchedulerSolution	globalSolution,
						 IloSchedulerSolution	lSSolution,
                         IloNumVar				costVar,
						 IloNum					globalSearchTimeLimit,
						 IloInt					globalSearchFailLimit
					//	 IloInt nbResources
						 );





////////////////////////////////////////////////////////////////////
//
// SOLVING THE MODEL BY USING A GREEDY DESCENT SEARCH
//
////////////////////////////////////////////////////////////////////

IloNum SolveModel(IloModel						model,
				  IloNumVar						costVar,
			//	  IloArray<IloActivityArray>	actArrays,
				  IloSchedulerSolution&			globalSolution,
				  IloNum						globalSearchTimeLimit,
				  IloInt						globalSearchFailLimit,
				  IloNum						localSearchTimeLimit,
				  IloInt						localSearchFailLimit,
				  IloNum						relaxActProb
			//	  IloInt						nbResources
				  );



////////////////////////////////////////////////////////////////////
//
// DEFINING THE MODEL WITH ALTERNATIVE RESOURCES
//
////////////////////////////////////////////////////////////////////

IloModel
DefineModel(IloEnv&						env,
            IloInt						numberOfJobs,
			IloInt						numberOfActivitiesPerJob,
            IloInt						numberOfResourcesPerActivity,
			IloInt						numberOfResources,
            IloInt***					resourceNumbers,
            IloInt**					durations,
			IloIntervalList*			breakList,
            IloRandom					randomGenerator,
            IloSchedulerSolution		solution,
		//	IloArray<IloActivityArray>&	actArrays,
            IloNumVar&					makespan
			);


////////////////////////////////////////////////////////////////////
//
// COMPUTING ALLOCATION COST OF A SOLUTION
//
////////////////////////////////////////////////////////////////////

IloInt
ComputeAllocCost(IloSchedulerSolution	solution,
				 IloInt**				allocCosts);

void
RetrieveEffProcPlanEndTimes(IloSchedulerSolution	solution,
							IloInt*					effProcPlanEndTimes);



#endif
