// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluscheduler.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004	 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/iluscheduler.h>
#include <ilsim/iluactivity.h>


void
PrintSolution(const IlcScheduler& scheduler);


IlcScheduler
IluSchedulerI::solve(IloNum currentTime, IloInt optim, IloNum upperBound) {
	_solver.end();	// All memory allocated by Solver on the Solver heap for _solver is freed.

	IloModel initPb = _uncModel->getModel(); // Initial problem
	
// The problem represents both the initial model and the constraints posted on a part of its variables.
	IloModel problem(_env);
	problem.add(initPb);

	for (IloIterator<IloActivity> it(_env); it.ok(); ++it) {
		IloActivity act = *it;
		IluActivityI* uncActP = (IluActivityI*)act.getObject();
		if (!(uncActP->isExecuted())) {
			if (uncActP->getEffecStartTime() < IlcIntMax) {
				IloNumVar procTimeVar = uncActP->getProcessingTimeVariable();
				IloInt pt = (IloInt)uncActP->getRandomVar()->getAverage(currentTime - uncActP->getEffecStartTime());
				problem.add(procTimeVar == pt);
				IloNumVar startVar = uncActP->getStartVariable();
				IloInt start = uncActP->getEffecStartTime();
				problem.add(startVar  == start);
//				printf(uncActP->getName());
//				printf(" lasts %ld time units. ", pt);
//				printf("This activity is being executed.\n");
			}
			else {
				IloNumVar procTimeVar = uncActP->getProcessingTimeVariable();
				IloNum procTime = uncActP->getIndicativeProcessingTime();
				problem.add(procTimeVar == procTime);
				problem.add((IloInt)currentTime <= uncActP->getStartVariable());
//				printf(uncActP->getName());
//				printf(" lasts " %ld time units. ", procTime);
//				printf("This activity has not yet been executed.\n");
			}
		}	else {
			IloNumVar endVar = uncActP->getEndVariable();
			IloInt end = uncActP->getEffecEndTime();
			problem.add(endVar  == end);
		}
	}
	
	if(1 == optim){
		problem.add(_uncModel->getMakespan() <= upperBound);
		printf( "A constraint is added: makespan <= %f\n", upperBound);
	}


//	printf("Initial problem:\ninitPb\n\n");
//	printf("Global problem:"\nproblem\n\n");

	IloEnv env = _env;
	
	_solver = IloSolver(problem); // Model extraction

//	printf("Extraction is done.\n");

	_solver.startNewSearch(_goal);

	IlcScheduler sched(_solver);

	IloSchedulerSolution solution = _uncModel->getSolution();
	/*IloSchedulerSolution solution[17];
	for(IloInt i = 0; i < 17; i++)
		solution[i] = _uncModel->getSolution();*/

//	IloNum searchTime = 10.0;
	IloNum bestMakespan = 0.0;

	//IloInt k = 0;
/*	while ((_solver.next())||(_solver.getTime() > searchTime)){
		if(_solver.getTime() > searchTime){
			_solver.endSearch();
			searchTime += 10.0;
			problem.add(_uncModel->getMakespan() <= bestMakespan);
			printf("A constraint is added: makespan < %f\n", bestMakespan);
			_solver.startNewSearch(_goal);
		}
		else {
			bestMakespan = _solver.getMin(_uncModel->getMakespan());
			solution.store(sched);
			printf("Best solution found so far with a makespan = %f\n", bestMakespan);
//			solution[k].store(sched);		
//			k++;
		}
	}*/
	
	while(_solver.next()) {
		bestMakespan = _solver.getMin(_uncModel->getMakespan());
		solution.store(sched);
//		printf("Best solution found so far with a makespan = %f\n", bestMakespan);
//		solution[k].store(sched);		
//		k++;
	}


/*	if(1 == optim){		// Optimization
		if((bestMakespan > 0.0)&&(_solver.getTime() <= 60.0)){
			printf("The optimal makespan is equal to %f\n", bestMakespan);
		}
		else
			if((bestMakespan > 0.0)&&(_solver.getTime() > 60.0)){
				printf("%f\tThis is not proven this is the optimal makespan.\n", bestMakespan);
				"\t" << bestMakespan << endl;
			}
		else
			if((0.0 == bestMakespan)&&(_solver.getTime() <= 60.0)){	
				printf("This problem is overconstrained.\n");
			}
		else {
			printf("No solution found.\n");
		}
	}*/

	if(1 == optim)
		printf("The optimal makespan is equal to %f\n", bestMakespan);


//	if(_solver.getTime() > 1.0)
//		printf("Best solution = %f\n", bestMakespan);
//	else
//		printf("Optimal solution = %f\n", bestMakespan);

	if(0 == optim){		// Simulation
		if(_solver.getTime() > 1.0) {
			_searchTime = 1.0;
//			printf("Best solution found = %f in %f second(s).\n", bestMakespan, _solver.getTime());
		}
		else {
			_searchTime = _solver.getTime();
//			printf("Optimal solution = %f\n, found in %f second(s).", bestMakespan, _solver.getTime());
		}
	}
	
	/*IloInt best = 0;
	for(IloInt i = 0; i < 17; i++){*/
	_solver.endSearch();
		//_solver.startNewSearch(IloRestoreSolution(_env, solution[i]) && IloRankForward(_env));
	_solver.startNewSearch(IloRestoreSolution(_env, solution) && IloRankForward(_env));
	if(bestMakespan > 0.0)
		_solver.next();

//	}

//	printf("Solve is done.\n");

//	We update indicative activity start times.
	for (IloIterator<IloActivity> it2(_env); it2.ok(); ++it2) {
		IloActivity act = *it2;
		IluActivityI* uncActP = (IluActivityI*)act.getObject();
		uncActP->setIndicativeStartTime((sched.getActivity(act)).getStartMin());
//		printf("Activity %s starts at %ld and lasts %f.\n", uncActP->getName(), uncActP->getIndicativeStartTime(), uncActP->getProcessingTime());
	}
	return sched;
}
