// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

#ifndef __SIM_iluprintsimH
#define __SIM_iluprintsimH


#include <ilsched/iloscheduler.h>


////////////////////////////////////////////////////////////////////
//
// PRINTING A SOLUTION
//
////////////////////////////////////////////////////////////////////


void PrintSolution(const IloSchedulerSolution solution,
                   const IloNumVar costVar);

void PrintSolution(const IloSchedulerSolution	solution,
				   IloArray<IloActivityArray>	actArrays,
				   const IloNumVar				costVar,
				   bool							sdxlOutput=IloFalse);

void PrintSolution(IloEnv& env,
				   const IlcScheduler& scheduler);

#endif