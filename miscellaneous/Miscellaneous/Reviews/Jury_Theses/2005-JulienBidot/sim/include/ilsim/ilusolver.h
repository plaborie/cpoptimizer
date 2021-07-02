// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains the IluSolverI class, the IluLocalSearchSolverI class, and the IluTreeSearchSolverI class.


#ifndef __SIM_ilusolversimH
#define __SIM_ilusolversimH

#include <ilconcert/ilorandom.h>
#include <ilsched/iloscheduler.h>
#include <ilsched/ilosolution.h>
#include "iluextendedmodel.h"
#include "ilusimulator.h"



// --------------------------------------------------------------------------
// THIS CLASS PERMITS US TO SOLVE A NON-DETERMINISTIC PROBLEM:
// UNCERTAIN ACTIVITIES AND RESOURCES.
// --------------------------------------------------------------------------

class IluSolverI {
private:
	IloEnv				_env;
	IluExtendedModelI*	_uncModel;	// Initial model
	IloGoal				_goal;		// Heuristic for determining how to take allocation and ordering decisions during search.
	IloSolver			_solver;	
	IloNum				_searchTime;// Time spent for looking for a solution


public:
	IluSolverI(): _uncModel(0) {}
	IluSolverI(IloEnv env, IluExtendedModelI* uncModel, const IloGoal& goal, IloNum searchTime = 0.0): _env(env),
																									  _uncModel(uncModel),
																									  _goal(goal),
																									  _searchTime(searchTime) {}
	~IluSolverI() {}

	void setEnv(IloEnv env) { _env = env; }
	void setSolver(IloSolver solver) { _solver = solver; }
	void setGoal(IloGoal goal) { _goal = goal; }
	void setUncModel(IluExtendedModelI* uncModel) { _uncModel = uncModel; }
	
	IloEnv getEnv() const { return _env; }
	IloSolver getSolver() const { return _solver; }
	IlcScheduler getScheduler() const { return IlcScheduler(_solver); }
	IloGoal getGoal() const { return _goal; }
	IluExtendedModelI* getUncModel() const { return _uncModel; }
	IloNum getSearchTime() const { return _searchTime; }
};






// --------------------------------------------------------------------------
// THIS CLASS PERMITS US TO SOLVE A NON-DETERMINISTIC PROBLEM:
// UNCERTAIN ACTIVITIES AND RESOURCES BY USING LOCAL SEARCH.
// --------------------------------------------------------------------------

class IluLocalSearchSolverI: public IluSolverI {
private:
	IluSimulatorI*				_simulator;	// _simulator permits us to assess solution with respect to costs during local search.
	IluFrontierI*				_partialFrontier, * _selectionFrontier;
	IluSimulatorOneActivityI*	_oneActSim;	// _oneActSim permits us to generate a first complete solution by scheduling one activity at a time.
	IloNum*						_queues, * _costQueues;
	ItemI*						_recycling, * _eligible, * _pending, * _lastSelectedActivity;

public:
	IluLocalSearchSolverI():	_simulator(0), _partialFrontier(0), _selectionFrontier(0), _oneActSim(0), _recycling(0), _eligible(0), _pending(0), _lastSelectedActivity(0) {}
	IluLocalSearchSolverI(IluExtendedModelI* uncModel, const IloGoal& goal, IloNum searchTime, IluSimulatorI* lSSimP, IluFrontierI* selecFrontier);

	~IluLocalSearchSolverI() {}

	void solve(IloSchedulerSolution& solution, IloNum time, IloInt nbSimulations, ItemI* pendingItem);
	// Optimization of solution by using local search; solution is changed: different precedence and allocation constraints are posted.

	void updatePG(IloEnv env, IloSchedulerSolution& solution, IloNum currentTime);

	void computeQueues();
};




/*

// --------------------------------------------------------------------------
// THIS CLASS PERMITS US TO SOLVE A NON-DETERMINISTIC PROBLEM:
// UNCERTAIN ACTIVITIES AND RESOURCES BY USING TREE SEARCH.
// --------------------------------------------------------------------------

class IluTreeSearchSolverI: public IluSolverI {
private:

public:
	IluTreeSearchSolverI() {}
	IluTreeSearchSolverI(IloEnv env, IluExtendedModelI* uncModel, const IloGoal& goal, IloNum searchTime): IluSolverI(env,
																													  uncModel,
																													  goal,
																													  searchTime) {}

	~IluTreeSearchSolverI() {}

	IlcScheduler solve(IloNum currentTime, IloInt optim, IloNum upperBound); // Solving of the CSP by searching in a tree
	

};*/



#endif