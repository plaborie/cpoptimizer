// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilutimeeventmanager.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/ilusolver.h>
#include <ilsim/iluactivity.h>



IluLocalSearchSolverI::IluLocalSearchSolverI(IluExtendedModelI* uncModel, const IloGoal& goal, IloNum searchTime, IluSimulatorI* lSSimP, IluFrontierI* selecFrontier)
				: IluSolverI(lSSimP->getEnv(), uncModel, goal, searchTime), _simulator(lSSimP), _partialFrontier(0), _selectionFrontier(selecFrontier) {

// Simulator: some random variables are associated with resources.
	IloInt nbTotalResources = uncModel->getNbTotalResources();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IloInt nbTotalActivities = uncModel->getNbTotalActivities();

	IloInt i;
	for(i = 0; i < nbTotalResources; i++) {
		IluUnaryResourceI* uncRes = uncModel->getUUnaryResource(i);
		_simulator->setBreakdownDurationVariable(i, uncRes->getBreaksDurationVar());
		_simulator->setInterBreakdownDurationVariable(i, uncRes->getInterBreaksVar());
	}

	IluHistogramI** histResource = _simulator->getStartFrontier()->getLastHistogramResource();
	// Simulator: frontier nodes are initialized.
	IluSimulatorI::NodeI* node = _simulator->getFirstNode();
	for(i = 0; i < nbTotalResources; i++) {
		IluSimulatorI::ResourceI* res = _simulator->getResource(i);
		node->setActivityId(nbTotalActivities + i + 1);
		node->setResource(res);
		node->setHistogram(histResource[i]);
		node = node->getNext();
	}
	IluHistogramI** histProcessPlan = _simulator->getStartFrontier()->getLastHistogramProcessPlan();
	for(i = 0; i < nbProcessPlans; i++) {
		node->setProcessPlanId(i);
		node->setActivityId(nbTotalActivities + nbTotalResources + i + 1);
		node->setHistogram(histProcessPlan[i]);
		node = node->getNext();
	}


}


void
IluLocalSearchSolverI::solve(IloSchedulerSolution& solution, IloNum time, IloInt nbSimulations, ItemI* pending) {
	// solution is changed by using local search.
	_pending = pending;
	updatePG(getEnv(), solution, time);
	_simulator->clean();
	_simulator->simulate(time, nbSimulations);

	_partialFrontier = _simulator->getEndFrontier();
	IluHistogramI** lastHistsBreakdown = _partialFrontier->getLastHistogramBreakdown();
	IluHistogramI** lastHistsProcessPlan = _partialFrontier->getLastHistogramProcessPlan();
	IluHistogramI** lastHistsResource = _partialFrontier->getLastHistogramResource();









}


void
IluLocalSearchSolverI::updatePG(IloEnv env, IloSchedulerSolution& solution, IloNum currentTime) {




	IluExtendedModelI* uncModel = getUncModel();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IloInt nbTotalActivities = uncModel->getNbTotalActivities();
	IloInt nbActivities = nbTotalActivities/nbProcessPlans;
	IloInt nbTotalResources = uncModel->getNbTotalResources();
	IloInt* processPlanIds = uncModel->getProcessPlanIds();

	IloInt i;

	IloBool graphChanged = IloFalse;

	IloInt nbSimulations = _simulator->getNbSimulations();
	// Some new nodes are created. They correspond to new selected activities. Some histograms are also created.
	for(IloSchedulerSolution::ActivityIterator ite(solution); ite.ok(); ++ite) {
		graphChanged = IloTrue;
		IluActivityI* uncAct = (IluActivityI*)((*ite).getObject());
		IloInt actIndex = uncAct->getIndex();
		_simulator->addNode(actIndex, IloFalse);
		IluRandomVariableI* randVar = uncAct->getRandomVar();
		IloInt processPlanId = processPlanIds[actIndex];
		IluSimulatorI::NodeI* node = _simulator->getFirstNode();
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(actIndex == node->getActivityId()) {
					IluHistogramI* h = 0;
					if(0 == _simulator->getFirstRecyclingHistogram())
						h = new (env) IluHistogramI(env, nbSimulations);
					else {
						h = _simulator->getFirstRecyclingHistogram();
						_simulator->setFirstRecyclingHistogram(h->getNext());
						h->setNext(0);
					}
					node->setHistogram(h);
					node->setDurationVariable(randVar);
					node->setProcessPlanId(processPlanId);
					stop = IloTrue;
				}
				node = node->getNext();
			}
		}
	}
	
	if(IloTrue == graphChanged) {

		// Initial precedence constraints are retrieved and arcs are thus created.
		for(i = 0; i < nbProcessPlans; i++) {
			IluSimulatorI::NodeI* firstNode = _simulator->getFirstNode();
			IloInt secondIndex = -1;
			IloInt firstIndex = -1;
			IloInt sIndex = -1;
			IloInt bIndex = -1;
			while(0 != firstNode) {
				if((IloFalse == firstNode->isFrontier())&&(i == firstNode->getProcessPlanId())) {
					firstIndex = firstNode->getActivityId();
					IluSimulatorI::NodeI* secondNode = _simulator->getFirstNode();
					IloBool stop = IloFalse;
					while(IloFalse == stop) {
						if(0 == secondNode)
							stop = IloTrue;
						else {
							if((IloFalse == secondNode->isFrontier())&&(i == secondNode->getProcessPlanId())&&(firstNode != secondNode)) {
								secondIndex = secondNode->getActivityId();
								sIndex = firstIndex;
								bIndex = secondIndex;
								if(sIndex > bIndex) {
									sIndex = secondIndex;
									bIndex = firstIndex;
								}
								if((IloFalse == _simulator->hasArc(sIndex, bIndex))&&(1 == bIndex - sIndex)) {
									_simulator->addArc(sIndex, bIndex);
								//	printf("An arc is added between node %ld and node %ld.\n", sIndex, bIndex);
								if((firstIndex == sIndex)&&(firstNode->getEndTime() < IloInfinity))
									_simulator->decrCurrentIndegree(bIndex);
								if((firstIndex == bIndex)&&(secondNode->getEndTime() < IloInfinity))
									_simulator->decrCurrentIndegree(bIndex);
									stop = IloTrue;
								}
							}
							secondNode = secondNode->getNext();
						}
					}
				}
				firstNode = firstNode->getNext();
			}
		}

		// Activity indices and resource items are associated with nodes. Some arcs are also added.
		for(i = 0; i < nbTotalResources; i++) {
			IluUnaryResourceI* uncResource = uncModel->getUUnaryResource(i);
			IloUnaryResource resource = (IloUnaryResource) (*uncResource);
			IluSimulatorI::ResourceI* res = _simulator->getResource(i);
			IloInt nodeId = res->getNodeId();
			if(solution.contains(resource)) {
				if(solution.hasSetupRC(resource)) {
					IloResourceConstraint rCt = solution.getSetupRC(resource);
					if(nodeId < IloIntMax) {
						IloActivity act = rCt.getActivity();
						IluActivityI* uncAct = (IluActivityI*)(act.getObject());
						IloInt actIndex = uncAct->getIndex();
						if(IloFalse == _simulator->hasArc(nodeId, actIndex)) {
							_simulator->addArc(nodeId, actIndex);
						//	printf("An arc is added between node %ld and node %ld.\n", nodeId, actIndex);
							if(_simulator->getEndTime(nodeId) < IloInfinity)
								_simulator->decrCurrentIndegree(actIndex);
						}
					}
					IloResourceConstraint nextRCt = solution.getSetupRC(resource);
					IloInt lastActIndex = -1;
					do {
						rCt = nextRCt;
						IloActivity act = rCt.getActivity();
						IluActivityI* uncAct = (IluActivityI*)(act.getObject());
						IloInt actIndex = uncAct->getIndex();
						IluSimulatorI::NodeI* node = _simulator->getFirstNode();
						IloBool stop = IloFalse;
						while(IloFalse == stop) {
							if(0 == node)
								stop = IloTrue;
							else {
								if(actIndex == node->getActivityId()) {
									node->setResource(_simulator->getResource(i));
									stop = IloTrue;
								}
								else
									node = node->getNext();
							}
						}
						if((-1 != lastActIndex)&&(IloFalse == _simulator->hasArc(lastActIndex, actIndex))) {
							_simulator->addArc(lastActIndex, actIndex);
						//	printf("An arc is added between node %ld and node %ld.\n", lastActIndex, actIndex);
							if(_simulator->getEndTime(lastActIndex) < IloInfinity)
								_simulator->decrCurrentIndegree(actIndex);

						}
						lastActIndex = actIndex;
						if(solution.hasNextRC(rCt))
							nextRCt = solution.getNextRC(rCt);
					} while(solution.hasNextRC(rCt));
				}
			}
		}
		// Topological sort
		_simulator->topologicalSort();

		// Some arcs are added between frontier arcs and other arcs.
		_simulator->addFrontierArcs();

	//	IluSimulatorI::NodeI* node = _simulator->getFirstNode();
	//	while(0 != node) {
	//		printf("Node %ld has a current in degree = %ld.\n", node->getActivityId(), node->getCurrentInDegree());
	//		node = node->getNext();
	//	}
	}




















}


void
IluLocalSearchSolverI::computeQueues() {




}




/*



IlcScheduler
IluTreeSearchSolverI::solve(IloNum currentTime, IloInt optim, IloNum upperBound) {
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


//	printf("Initial problem:\ninitPb\n\n");
//	printf("Global problem:"\nproblem\n\n");

	IloEnv env = _env;
	
	_solver = IloSolver(problem); // Model extraction

//	printf("Extraction is done.\n");

	_solver.startNewSearch(_goal);

	IlcScheduler sched(_solver);

	IloSchedulerSolution solution = _uncModel->getSolution();*/
	/*IloSchedulerSolution solution[17];
	for(IloInt i = 0; i < 17; i++)
		solution[i] = _uncModel->getSolution();*/
/*
//	IloNum searchTime = 10.0;
	IloNum bestMakespan = 0.0;

	//IloInt k = 0;*/
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
/*	
	while(_solver.next()) {
		bestMakespan = _solver.getMin(_uncModel->getMakespan());
		solution.store(sched);
//		printf("Best solution found so far with a makespan = %f\n", bestMakespan);
//		solution[k].store(sched);		
//		k++;
	}



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
	}*/
	
	/*IloInt best = 0;
	for(IloInt i = 0; i < 17; i++){*/
/*	_solver.endSearch();
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
}*/
