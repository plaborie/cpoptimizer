// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// This file contains the IluSchedulerI class.

#ifndef __SIM_iluschedulersimH
#define __SIM_iluschedulersimH


#include <ilsched/iloscheduler.h>

#include "ilumodel.h"
#include "ilupgsim.h"





class IluSchedulerI { // tree search
private:
	IloEnv				_env;
	IluExtendedModelI*	_uncModel; // Initial model
	IloGoal				_goal;		// heuristic for choosing how to order activities.
	IloSolver			_solver;
	IloNum				_searchTime;	// Time spent for searching

public:
	IluSchedulerI(): _uncModel(0) {}
	IluSchedulerI(IloEnv env, IluModelI* uncModel, const IloGoal& goal): _env(env),
																		 _uncModel(uncModel),
																		 _goal(goal),
																		 _solver(env),
																		 _searchTime(0.0){}

	~IluSchedulerI() {}

	IlcScheduler solve(IloNum currentTime, IloInt optim, IloNum upperBound); // Solving of the CSP by searching in a tree
	
	void setEnv(IloEnv env) { _env = env; }
	void setSolver(IloSolver solver) { _solver = solver; }
	void setGoal(IloGoal goal) { _goal = goal; }
	void setUncModel(IluExtendedModelI* uncModel) { _uncModel = uncModel; }
	
	
	IloEnv getEnv() { return _env; }
	IloSolver getSolver() { return _solver; }
	IlcScheduler getScheduler() { return IlcScheduler(_solver); }
	IloGoal getGoal() { return _goal; }
	IluModelI* getUncModel() { return _uncModel; }
	IloNum getSearchTime() { return _searchTime; }
};

#endif
