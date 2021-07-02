// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilusimulator.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/ilusimulator.h>

/*#if defined(ILOUSESTL)
#include <fstream>
#else
#include <fstream.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif

#include <stdio.h>*/

ILOSTLBEGIN


IluFrontierI::IluFrontierI(IloInt nbProcessPlans,
						   IloInt nbTotalResources,
						   IluHistogramI** lastHistogramBreakdown,
						   IluHistogramI** lastHistogramProcessPlan,
						   IluHistogramI** lastHistogramResource)
			:	_nbProcessPlans(nbProcessPlans),
				_nbTotalResources(nbTotalResources),
				_lastHistogramBreakdown(lastHistogramBreakdown),
				_lastHistogramProcessPlan(lastHistogramProcessPlan),
				_lastHistogramResource(lastHistogramResource) {
	if(_lastHistogramBreakdown[0] > 0)
		_nbSimulations = _lastHistogramBreakdown[0]->getNbSimulations();
	if(_lastHistogramProcessPlan[0] > 0)
		if(_lastHistogramProcessPlan[0]->getNbSimulations() < _nbSimulations)
			_nbSimulations = _lastHistogramProcessPlan[0]->getNbSimulations();
	if(_lastHistogramResource[0] > 0)
		if(_lastHistogramResource[0]->getNbSimulations() < _nbSimulations)
			_nbSimulations = _lastHistogramResource[0]->getNbSimulations();
}


IluFrontierI::IluFrontierI(IloEnv env,
						   IloInt nbProcessPlans,
						   IloInt nbTotalResources,
						   IloInt nbSimulations)
			:	_nbProcessPlans(nbProcessPlans),
				_nbTotalResources(nbTotalResources),
				_nbSimulations(nbSimulations) {
	_lastHistogramBreakdown = new (env) IluHistogramI*[nbTotalResources];
	_lastHistogramProcessPlan = new (env) IluHistogramI*[nbProcessPlans];
	_lastHistogramResource = new (env) IluHistogramI*[nbTotalResources];

	IloInt i;
	for(i = 0; i < nbTotalResources; i++) {
		_lastHistogramBreakdown[i] = 0;
		_lastHistogramResource[i] = 0;
	}
	for(i = 0; i < nbProcessPlans; i++)
		_lastHistogramProcessPlan[i] = 0;
}


IloNum
IluFrontierI::getEarliestProcessPlanEndTime(IloNum currentTime) {
	IloNum minTime = IloInfinity;
	IloInt processId = -1;
	for(IloInt i = 0; i < _nbProcessPlans; i++) {
		IloNum average = 0.0;
		if(_lastHistogramProcessPlan[i]->getAverage() < currentTime)
			average = _lastHistogramProcessPlan[i]->getValue(0);
		else
			average = _lastHistogramProcessPlan[i]->getAverage();
		if((average < minTime)&&(average >= currentTime)) {
			minTime = average;
			processId = i;
		}
	}
//	printf("Selection process: process plan %ld and minimum Time = %.10f\n", processId, minTime);
	return minTime;
}


IloNum
IluFrontierI::getMaxStandardDeviationProcessPlan(IloNum currentTime) {
	IloNum maxStDev = 0.0;
	for(IloInt i = 0; i < _nbProcessPlans; i++) {
		if((_lastHistogramProcessPlan[i]->getStandardDev() > maxStDev)&&(_lastHistogramProcessPlan[i]->getAverage() > currentTime))
			maxStDev = _lastHistogramProcessPlan[i]->getStandardDev();
	}
	return maxStDev;
}


void
IluFrontierI::display(const char* name, IloInt nbPoints) {
	IloInt i;
	char fName[64];
	for(i = 0; i < _nbProcessPlans; i++) {
		sprintf(fName, "%sProcessPlanActivityEndTime%ld.dat", name, i);
		_lastHistogramProcessPlan[i]->display(fName, nbPoints);
	}
	for(i = 0; i < _nbTotalResources; i++) {
		sprintf(fName, "%sResourceBreakdownEndTime%ld.dat", name, i);
		_lastHistogramBreakdown[i]->display(fName, nbPoints);
		sprintf(fName, "%sResourceActivityEndTime%ld.dat", name, i);
		_lastHistogramResource[i]->display(fName, nbPoints);
	}
}


IloNum
IluSimulatorI::NodeI::getDuration(IloNum t) const {
	IloNum dur;
	if (_start <= t )
			dur = _dur->getValue(_executedPart);
	else
		dur = _dur->getValue();
	return dur;
}


IluSimulatorI::NodeI::~NodeI() {
	ArcI* it = _out;
	ArcI* del;
	while (it != 0) {
		del = it;
		it = it->getNext();
		delete del;
	}
	if(0 != _res)
		delete _res;
}


IloNum
IluSimulatorI::NodeI::computeEnd(IloNum t) {
	IloNum start = _val;
	IloNum end = start + getDuration(t); // -executedPart;
	IloNum lastBreakdownEndTime = _res->getLastEnd();
	IloNum currentBreakdownStartTime = _res->getCurrentStart();
	IloNum currentBreakdownEndTime = _res->getCurrentEnd();
/*	if(lastBreakdownEndTime > end)
		cout << "Error: resource " << _res->getId() << " breaks down too late." << endl;*/
	if((IloFalse == _res->isBrokenDown())&&(_res->getEndTime() == _res->getLastEnd())) { // This resource has been repaired.
		if(lastBreakdownEndTime > start)
			end += _res->getEndTime() - _res->getStartTime();
	}
	while(currentBreakdownStartTime < end) {
		if(currentBreakdownStartTime > start)
			end += currentBreakdownEndTime - currentBreakdownStartTime;
		else if(currentBreakdownEndTime > start) {
			IloNum duration = end - start;
			start = currentBreakdownEndTime;
			if(_res->isBrokenDown())
				_val = start;
			end = start + duration;
		}
	/*	printf("Resource %ld, associated with node %ld, starts to break down at %.14f and ends to break down at %.14f.\n",
			_res->getId(),
			_res->getNodeId(),
			currentBreakdownStartTime,
			currentBreakdownEndTime);*/
	//	IloInt tempSeed = _res->getSeed();
	//	interVar->getRandom().reSeed(tempSeed);
	//	_res->setSeed(_res->getInterBreakdownDurationVariable()->getRandom().getInt(IloIntMax) + 1);
		lastBreakdownEndTime = currentBreakdownEndTime;
		currentBreakdownStartTime = currentBreakdownEndTime + _res->getInterBreakdownDurationVariable()->getValue();
		currentBreakdownEndTime = currentBreakdownStartTime + _res->getBreakdownDurationVariable()->getValue();
	}
	_res->setLastEnd(lastBreakdownEndTime);
	_res->setCurrentStart(currentBreakdownStartTime);
	_res->setCurrentEnd(currentBreakdownEndTime);

//	printf("Activity associated with node %ld starts execution at %.14f and ends execution at %.14f\n", _id, start, end);
	return end;
}


IluSimulatorI::IluSimulatorI(IloEnv env, IluFrontierI* startFrontier, IluFrontierI* endFrontier, IloInt nbNodes)
	:_env						(env),
	 _firstRecyclingHistogram	(0),
	 _work						(0),
	 _sortFirst					(0),
	 _sortLast					(0),
	 _startFrontier				(startFrontier),
	 _endFrontier				(endFrontier),
	 _maxNbSimulations			(startFrontier->getNbSimulations()),
	 _nbResources				(startFrontier->getNbTotalResources()),
	 _nbProcessPlans			(startFrontier->getNbProcessPlans()),
	 _firstNode					(0),
	 _firstRecyclingNode		(0),
	 _firstRecyclingArc			(0),
	 _lastSimulationTime		(IloInfinity) {
	IloInt i;
	_lastNodeProcessPlan = new (env) IloInt[_nbProcessPlans];
	for(i = 0; i < _nbProcessPlans; i++)
		_lastNodeProcessPlan[i] = IloIntMax;
	_resources = new (env) ResourceI*[_nbResources];
	for(i = 0; i < _nbResources; i++)
		_resources[i] = new (env) ResourceI(i);
	_nbNodes = nbNodes + _nbProcessPlans + _nbResources;
	for(i = 0; i < nbNodes; ++i) {
		NodeI* newNode = new NodeI(IloFalse);
		newNode->setNext(_firstNode);
		_firstNode = newNode;
	}
	for(i = nbNodes; i < _nbNodes; i++) {
		NodeI* newNode = new NodeI(IloTrue);		// These are the nodes representing the current frontier.
		newNode->setNext(_firstNode);
		_firstNode = newNode;
	}
}


IluSimulatorI::~IluSimulatorI() {
	assert (0 == _work);
	NodeI* node = _firstNode;
	NodeI* nodeDel;
	while(node != 0) {
		nodeDel = node;
		node = node->getNext();
		delete nodeDel;	
	}
	node = _firstRecyclingNode;
	while(node != 0) {
		nodeDel = node;
		node = node->getNext();
		delete nodeDel;	
	}
	ArcI* arc = _firstRecyclingArc;
	ArcI* arcDel = 0;
	while(arc != 0) {
		arcDel = arc;
		arc = arc->getNext();
		delete arcDel;	
	}
}


void
IluSimulatorI::stackWork(NodeI* node) {
	node->setTopologicalNext(_work);
	_work = node;
/*	node = _work;
	printf("Current nodes in _work:\n");
	while(0 != node) {
		printf("Node %ld\n", node->getActivityId());
		node = node->getTopologicalNext();
	}
	printf("\n");*/
}


void
IluSimulatorI::stackSort(NodeI* node) {
	if (0 != _sortFirst) {
		_sortLast->setTopologicalNext(node);
		node->setTopologicalNext(0);
		_sortLast = node;
	} else {
		_sortFirst = node;
		_sortLast  = node;
	}
}


IluSimulatorI::NodeI*
IluSimulatorI::unstackWork() {
/*	printf("Unstack _work\n");
	NodeI* node = _work;
	printf("Current nodes in _work:\n");
	while(0 != node) {
		printf("Node %ld\n", node->getActivityId());
		node = node->getTopologicalNext();
	}
	printf("\n");*/

	NodeI* node = _work;
	NodeI* previousNode = 0;
	NodeI* selected = _work;
	NodeI* previousSelected = 0;
	void* address = 0;
	void* currentAddress = 0;
	while(0!= node) {
		currentAddress = node->getDurationVariable();	// This permits us to use local search because we need to get the same estimation value
														// if allocation and ordering decisions made are the same.
	//	cout << "Node "<< node->getActivityId() << " has the duration variable address: " << currentAddress << endl;
		if((node == _work)||(currentAddress < address)) {
			address = currentAddress;
			previousSelected = previousNode;
			selected = node;
		}
		previousNode = node;
		node = node->getTopologicalNext();
	}
	if(_work == selected)
		_work = _work->getTopologicalNext();
	else
		previousSelected->setTopologicalNext(selected->getTopologicalNext());
/*	node = _work;
	printf("Current nodes in _work:\n");
	while(0 != node) {
		printf("Node %ld\n", node->getActivityId());
		node = node->getTopologicalNext();
	}
	printf("\n");*/
	return selected;
}


void
IluSimulatorI::addArc(IloInt source, IloInt target) {
	NodeI* sourceNode = _firstNode;
	IloBool sourceStop = IloFalse;
//	printf("An arc is added between %ld and %ld.\n", source, target);
	while(IloFalse == sourceStop) {
		if(0 == sourceNode)
			sourceStop = IloTrue;
		else {
			if(source == sourceNode->getActivityId()) {
				NodeI* targetNode = _firstNode;
				IloBool targetStop = IloFalse;
				while(IloFalse == targetStop) {
					if(0 == targetNode)
						targetStop = IloTrue;
					else {
						if(target == targetNode->getActivityId()) {
							targetNode->incrIndegree();
							targetNode->incrCurrentIndegree();
							targetStop = IloTrue;
							if(0 == _firstRecyclingArc)
								sourceNode->addArc(targetNode);
							else {
								ArcI* arc = _firstRecyclingArc;
								_firstRecyclingArc = _firstRecyclingArc->getNext();
								arc->setTarget(targetNode);
								sourceNode->addArc(arc);
							}
							sourceStop = IloTrue;
						}
						else
							targetNode = targetNode->getNext();
					}
				}
			}
			else
				sourceNode = sourceNode->getNext();
		}
	}
}


void
IluSimulatorI::setActivityDurationVariable(IloInt i, IluRandomVariableI* dur) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				node->setDurationVariable(dur);
				stop = IloTrue;
			}	
			node = node->getNext();
		}
	}
}


void
IluSimulatorI::setHistogram(IloInt i, IluHistogramI* hist) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				if(0 == node->getHistogram())
					node->setHistogram(hist);
				stop = IloTrue;
			}
			node = node->getNext();
		}
	}
}


void
IluSimulatorI::setProcessPlanId(IloInt i, IloInt processPlanIndex) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				node->setProcessPlanId(processPlanIndex);
				stop = IloTrue;
			}	
			node = node->getNext();
		}
	}
}



void
IluSimulatorI::topologicalSort() {	// If the number of nodes in the list _work is big, then it would be more efficient to use a heap that
									// would be incrementally sorted.
//	printf("Topological sort.\n");
//	printf("Before sorting topologically:\n");
	NodeI* node = _firstNode;
//	while(0 != node) {
//		printf("Node %ld: indegree = %ld\n", node->getActivityId(), node->getIndegree());
//		printf("Node %ld: current indegree = %ld\n", node->getActivityId(), node->getCurrentIndegree());
//		node = node->getNext();
//	}
//	assert(0 == _sortFirst);
	_sortFirst = 0;
	node = _firstNode;
	while(0 != node) {						// Frontier nodes are systematically sorted first.
		if(node->isFrontier())
			stackSort(node);
		node = node->getNext();
	}
	IloInt i = 0;
	node = _firstNode;
	while(node != 0) {
		if(IloFalse == node->isFrontier()) {
			node->setTopoIndegree(node->getIndegree());
		//	printf("Node %ld: indegree = %ld\n", node->getActivityId(), node->getIndegree());
			if (0 == node->getTopoIndegree())
				stackWork(node);
			i++;
		}
		node = node->getNext();
	}
	while(0 != _work) {
		NodeI* unstackNode = unstackWork();
		stackSort(unstackNode);
	//	printf("Node %ld\n", unstackNode->getActivityId());
		for(ArcI* out = unstackNode->getArcs(); 0 != out; out = out->getNext()) {
			node = out->getTarget();
			node->decrTopoIndegree();
			if(0 == node->getTopoIndegree())
				stackWork(node);
		}
	}
//	node = _sortFirst;
//	printf("After the topological sort:\n");
//	while(0 != node) {
//		printf("Node %ld; indegree = %ld and current indegree = %ld\n", node->getActivityId(), node->getIndegree(), node->getCurrentIndegree());
//		node = node->getTopologicalNext();
//	}
	for(NodeI* firstTopoNode = _sortFirst; firstTopoNode != 0; firstTopoNode = firstTopoNode->getTopologicalNext()) {
		IloInt nodeId = firstTopoNode->getActivityId();
		node = firstTopoNode;
		if((IloFalse == node->isFrontier())&&(0 != node->getResource())) {
			IloInt processPlanId = node->getProcessPlanId();
			_lastNodeProcessPlan[processPlanId] = nodeId;
			ResourceI* resP = node->getResource();
			resP->setNodeId(nodeId);
		}
	}
	for(i = 0; i < _nbProcessPlans; i++) {
		node = _firstNode;
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(_lastNodeProcessPlan[i] == node->getActivityId()) {
					node->setLastProcessPlan(IloTrue);
					stop = IloTrue;
				}
				node = node->getNext();
			}
		}
	}
	for(i = 0; i < _nbResources; i++) {
		node = _firstNode;
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(_resources[i]->getNodeId() == node->getActivityId()) {
					node->setLastResource(IloTrue);
					stop = IloTrue;
				}
				node = node->getNext();
			}
		}
	}
}


void
IluSimulatorI::addFrontierArcs() {
//	printf("Some frontier arcs are added.\n");
	IloInt i;
	NodeI* frontierNode = _sortFirst;
	for(i = 0; i < _nbResources; i++) {
		NodeI* node = 0;
		IloBool frontierStop = IloFalse;
		while(IloFalse == frontierStop) {
			if(0 == frontierNode)
				frontierStop = IloTrue;
			else {
				if(0 != frontierNode->getResource()) {
					if(i == frontierNode->getResource()->getId()) {
						frontierStop = IloTrue;
						node = frontierNode->getTopologicalNext();
						IloBool stop = IloFalse;
						while(IloFalse == stop) {
							if(0 == node)
								stop = IloTrue;
							else {
								if((IloFalse == node->isFrontier())&&(0 != node->getResource())) {
									if((i == node->getResource()->getId())&&(IloFalse == hasArc(frontierNode->getActivityId(), node->getActivityId()))) {
										addArc(frontierNode->getActivityId(), node->getActivityId());
										node->decrCurrentIndegree();
										node->decrIndegree();
									//	printf("An arc is added between %ld and %ld.\n", frontierNode->getActivityId(), node->getActivityId());
									//	printf("Node %ld: current indegree = %ld\n", node->getActivityId(), node->getCurrentIndegree());
										stop = IloTrue;
									}
								}
								node = node->getTopologicalNext();
							}
						}
					}
				}
				frontierNode = frontierNode->getTopologicalNext();
			}
		}
	}
	for(i = 0; i < _nbProcessPlans; i++) {
		NodeI* node = 0;
		IloBool frontierStop = IloFalse;
		while(IloFalse == frontierStop) {
			if(0 == frontierNode)
				frontierStop = IloTrue;
			else {
				if(i == frontierNode->getProcessPlanId()) {
					frontierStop = IloTrue;
					node = frontierNode->getTopologicalNext();
					IloBool stop = IloFalse;
					while(IloFalse == stop) {
						if(0 == node)
							stop = IloTrue;
						else {
							if((i == node->getProcessPlanId())&&(IloFalse == hasArc(frontierNode->getActivityId(), node->getActivityId()))) {
								addArc(frontierNode->getActivityId(), node->getActivityId());
								node->decrCurrentIndegree();
								node->decrIndegree();
							//	printf("An arc is added between %ld and %ld.\n", frontierNode->getActivityId(), node->getActivityId());
							//	printf("Node %ld: current indegree = %ld\n", node->getActivityId(), node->getCurrentIndegree());
								stop = IloTrue;
							}
							node = node->getTopologicalNext();
						}
					}
				}
				frontierNode = frontierNode->getTopologicalNext();
			}
		}
	}
}


void
IluSimulatorI::setStartTime(IloInt i, IloNum t) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				node->setStartTime(t);
				node->setCurrentIndegree(-1); // MARK EXECUTED
				if(0 != node->getResource())
					node->getResource()->setExecActId(i);
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}

}


void
IluSimulatorI::setEndTime(IloInt i, IloNum t) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				stop = IloTrue;
				node->setEndTime(t);
				if(0 != node->getResource())
					node->getResource()->setExecActId(IloIntMax);
			}
			else
				node = node->getNext();
		}
	}
	if(node->getResource() != 0) {
		for (ArcI* out = node->getArcs(); out != 0; out = out->getNext()) {
			NodeI* targetNode = out->getTarget();
			targetNode->decrCurrentIndegree();
		}
	}
}


void
IluSimulatorI::setBreakdownEndTime(IloInt i,
								   IloNum t) {
	_resources[i]->setEndTime(t);
	IluHistogramI** breakdownHist = _startFrontier->getLastHistogramBreakdown();
	breakdownHist[i]->setValue(t, 1, 0);
	breakdownHist[i]->setDeterministic(IloTrue);
}


IloBool
IluSimulatorI::isExecutable(IloInt i) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return((0 == node->getCurrentIndegree())&&(IloFalse == node->isFrontier()));
}


IloBool
IluSimulatorI::hasArc(IloInt source, IloInt target) {
	IloBool hasArc = IloFalse;
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(source == node->getActivityId()) {
				for (ArcI* out = node->getArcs(); out != 0; out = out->getNext()) {
					NodeI* targetNode = out->getTarget();
					if(target == targetNode->getActivityId())
						hasArc = IloTrue;
				}
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
	return hasArc;
}


IloBool
IluSimulatorI::nodeExists(IloInt nodeId) {
	IloBool exists = IloFalse;
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(nodeId == node->getActivityId()) {
				exists = IloTrue;
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
	return exists;
}


IloBool
IluSimulatorI::arcCanBeAdded(IloInt source, IloInt target) {
	IloBool canBeAdded = IloTrue;
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node) {
			stop = IloTrue;
			canBeAdded = IloFalse;
		}
		else {
			if(source == node->getActivityId()) {
				for (ArcI* out = node->getArcs(); out != 0; out = out->getNext()) {
					NodeI* targetNode = out->getTarget();
					if(target == targetNode->getActivityId())
						canBeAdded = IloFalse;
				}
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
	return canBeAdded;
}


void
IluSimulatorI::simulate(IloNum t, IloInt nb) {
	if(_maxNbSimulations >= nb) {
		NodeI* node = _firstNode;
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(0 != node->getDurationVariable()) {
					node->getDurationVariable()->getRandom().reSeed(1);
					stop = IloTrue;
				}
				else
					node = node->getNext();
			}
		}
		IloInt i, j;
		if(IloInfinity == _lastSimulationTime)
			_lastSimulationTime = t;
		node = _firstNode;
		stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(0 != node->getDurationVariable()&&(0 != node->getResource())) {
					ResourceI* resource = node->getResource();
					if((node->getEndTime() > t)&&(node->getStartTime() < t)) {
						if(((resource->isBrokenDown() == IloFalse)&&(resource->getEndTime() != t))||(resource->getStartTime() == t)) { // _executedPart is updated.
							IloNum temp = node->getExecutedPart();
							node->setExecutedPart(temp + t - _lastSimulationTime);
						}
					}
				}
				node = node->getNext();
			}
		}
		IluHistogramI** breakdownHist = _startFrontier->getLastHistogramBreakdown();
		for(j = 0 ; j < nb ; j++) {
		//	printf("\n");
			for(i = 0 ; i < _nbResources ; i++) {		// Resources are initialized.
				if(breakdownHist[i]->isDeterministic())
					_resources[i]->setLastEnd(breakdownHist[i]->getValue(0));
				else
					_resources[i]->setLastEnd(breakdownHist[i]->getValue(j));
			//	printf("Resource %ld: last breakdown occured at %.14f\n", i, _resources[i]->getLastEnd());
			//	IloInt tempSeed = hist[i]->getSeed(j);
			//	_resources[i]->getInterBreakdownDurationVariable()->getRandom().reSeed(tempSeed);
				if(_resources[i]->isBrokenDown()) {			// This resource is broken down.
					IloNum start = _resources[i]->getStartTime();
					_resources[i]->setCurrentStart(start);
					IloNum fictiveStart = _resources[i]->getLastEnd() + _resources[i]->getInterBreakdownDurationVariable()->getValue();
					IloNum end = fictiveStart + _resources[i]->getBreakdownDurationVariable()->getValue(start - fictiveStart);
					_resources[i]->setCurrentEnd(end);
				}
				else {
					IloNum start = _resources[i]->getLastEnd() + _resources[i]->getInterBreakdownDurationVariable()->getValue();
					_resources[i]->setCurrentStart(start);
					IloNum end = _resources[i]->getCurrentStart() + _resources[i]->getBreakdownDurationVariable()->getValue();
					_resources[i]->setCurrentEnd(end);
				}
			//	_resources[i]->setSeed(tempSeed);
			}
			node = _firstNode;
			while(0 != node) {
				if(IloFalse == node->isFrontier())
					node->cleanVal(t);
				node = node->getNext();
			}
			for(NodeI* sortFirst = _sortFirst; sortFirst != 0; sortFirst = sortFirst->getTopologicalNext()) {
				IloInt nodeId = sortFirst->getActivityId();
			//	printf("Topological sort: node %ld\n", nodeId);
				node = sortFirst;
				IluHistogramI* h = node->getHistogram();
				IloNum end;
				if(IloFalse == node->isFrontier()) {
					end = node->getEndTime();
					if((end > t)&&(node->getResource() != 0))
						end = node->computeEnd(t);
					if(0 != h)
						h->setValue(node->getVal(), 1);		// A histogram is updated.
				} else
					end = h->getValue(j);
				for (ArcI* out = node->getArcs(); out != 0; out = out->getNext()) {
					NodeI* targetNode = out->getTarget();
					targetNode->setMinVal(end);
				}
			//	IluHistogramI** hist;
				if(node->isLastProcessPlan()) {
					_endFrontier->setLastProcessPlanEndTime(node->getProcessPlanId(), end, 1, j);
				//	hist = _endFrontier->getLastHistogramProcessPlan();
				/*	printf("The last activity of process plan %ld finishes at %.14f.\n", node->getProcessPlanId(),
																						 hist[node->getProcessPlanId()]->getValue(j));*/
				}
				if(node->isLastResource()) {
					IloInt index = node->getResource()->getId();
					_endFrontier->setLastResourceEndTime(index, end, 1, j);
				//	hist = _endFrontier->getLastHistogramResource();
				//	printf("The last activity of resource %ld finishes at %.14f.\n", index, hist[index]->getValue(j));
					_endFrontier->setLastBreakdownEndTime(index,
														  _resources[index]->getLastEnd(),
														  _resources[index]->getSeed(),
														  j);
				//	hist = _endFrontier->getLastHistogramBreakdown();
				//	printf("The last breakdown of resource %ld finishes at %.14f.\n", index, hist[index]->getValue(j));
				}
			}
		}
	}
	else
		cout << "The number of simulations you want to run is too big!" << endl;
	_lastSimulationTime = t;
}


IloNum
IluSimulatorI::getStartTime(IloInt i) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getStartTime();
}


IloNum
IluSimulatorI::getVal(IloInt i) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getVal();
}


IloNum
IluSimulatorI::getEndTime(IloInt i) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getEndTime();
}


IluRandomVariableI*
IluSimulatorI::getDurationVariable(IloInt i) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getDurationVariable();
}


IloNum
IluSimulatorI::getDuration(IloInt i, IloNum t = 0) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getDuration(t);
}


IloNum
IluSimulatorI::getDuration(IloInt i, IloInt d) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	return node->getDuration(d);
}


void
IluSimulatorI::decrCurrentIndegree(IloInt nodeId) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(nodeId == node->getActivityId()) {
				node->decrCurrentIndegree();
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
}


IloInt
IluSimulatorI::getCurrentIndegree(IloInt nodeId) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	IloInt curIndegree;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(nodeId == node->getActivityId()) {
				curIndegree = node->getCurrentIndegree();
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
	return curIndegree;
}


IluHistogramI*
IluSimulatorI::getHistogram(IloInt i) const {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId())
				stop = IloTrue;
			else
				node = node->getNext();
		}
	}
	if(0 != node)
		return node->getHistogram();
	else
		return 0;
}


void
IluSimulatorI::requires(IloInt i, IloInt resIndex) {
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				node->setResource(_resources[resIndex]);
				stop = IloTrue;
			}
			else
				node = node->getNext();
		}
	}
}


void
IluSimulatorI::graphDisplay() {
	printf("\n");
	NodeI* node = _firstNode;
	IloBool stop = IloFalse;
	while(0 != node) {
		if(0 != node->getResource()) {
			printf("Node %ld is associated with the resource %ld.\n", node->getActivityId(), node->getResource()->getId());
		}
		if(node->getProcessPlanId() < IloIntMax) {
			printf("Node %ld is associated with the process plan %ld.\n", node->getActivityId(), node->getProcessPlanId());
		}
		node = node->getNext();
	}
}


void
IluSimulatorI::readJobShop(IluRandomEnv activityRand,
						   const char* filename,
						   IloNum alpha) {
	// FORMAT OF THE INPUT FILE:
	// nbJobs	nbResources
	// nodeId(1)						AvgDuration(1)						...	nodeId(nbResources)			AvgDuration(nbResources)
	// ...	
	// nodeId(nbResources*(nbJobs-1))	AvgDuration(nbResources*(nbJobs-1))	...	nodeId(nbResources*nbJobs)	AvgDuration(nbResources*nbJobs)

	IloInt nbJobs, nbResources, i, j, resourceId; // nbJobs is equal to the number of jobs and nbResources is equal to the number of resources
	ifstream file(filename);
	int lineLimit = 1024;
	char buffer[1024];
	file.getline(buffer, lineLimit);
	istrstream line(buffer, strlen(buffer));
	line >> nbJobs >> nbResources;
	IloInt nbTotalActivities = nbJobs * nbResources;
	
	// READ nbTotalActivities + 1 + nbJobs + nbResources NODES
	// Frontier nodes, associated with histograms, are initialized.
	NodeI* node = _firstNode;
	IluHistogramI** resourceHists = _startFrontier->getLastHistogramResource();
	i = 0;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(node->isFrontier()) {
				node->setResource(_resources[i]);
				node->setActivityId(i + nbTotalActivities + 1);
				IluHistogramI* hist = new (activityRand.getEnv()) IluHistogramI(activityRand.getEnv(), _maxNbSimulations);
				hist->clean();
				for(j = 0; j < _maxNbSimulations; j++)
					hist->setValue(resourceHists[i]->getValue(j), 1, j);
				node->setHistogram(hist);
				i++;
				if(nbResources == i)
					stop = IloTrue;
			}
			node = node->getNext();
		}
	}
	IluHistogramI** processPlanHists = _startFrontier->getLastHistogramProcessPlan();
	i = 0;
	stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(node->isFrontier()) {
				node->setProcessPlanId(i);
				node->setActivityId(i + nbTotalActivities + nbResources + 1);
				IluHistogramI* hist = new (activityRand.getEnv()) IluHistogramI(activityRand.getEnv(), _maxNbSimulations);
				for(j = 0; j < _maxNbSimulations; j++)
					hist->setValue(processPlanHists[i]->getValue(j), 1, j);
				node->setHistogram(hist);
				i++;
				if(nbJobs == i)
					stop = IloTrue;
			}
			node = node->getNext();
		}
	}
	
	// The nodes associated with activities are now initialized and some arcs are added (precedence constraints).
	IluRandomVariableI* randomVar = new (activityRand.getEnv()) IluNormalVariableI(activityRand.getEnv(), activityRand, 0.0,   0.0,   0.0,  0.0);		// This is a dummy variable used for computing makespan.
	node->setActivityId(0);
	node->setDurationVariable(randomVar);
	node = node->getNext();

	IloNum avg, min, max, sigma;
	NodeI* previousNode = 0;
	for (i = 0 ; i < nbJobs ; i++) {
		file.getline(buffer, lineLimit);
		istrstream line(buffer, strlen(buffer));
		for(j = 0 ; j < nbResources ; j++) {
			if(IloFalse == node->isFrontier()) {
				node->setActivityId(i * nbResources + j + 1);
				node->setProcessPlanId(i);
				line >> resourceId >> avg;
				node->setResource(_resources[resourceId]);
				if(j > 0) {
					addArc(previousNode->getActivityId(), node->getActivityId());
				//	printf("An arc is added between %ld and %ld.\n", previousNode->getActivityId(), node->getActivityId());
				}
				sigma = alpha * avg;
				min = IloMax(1.0, avg - 5 * sigma);
				max = avg + 5 * sigma;
				randomVar = new (activityRand.getEnv()) IluNormalVariableI(activityRand.getEnv(), activityRand, min, max, avg, sigma);
				node->setDurationVariable(randomVar);
			//	cout << "Node "<< node->getActivityId() << " has the duration variable address: " << randomVar << endl;
				if(nbResources - 1 == j) {
					addArc(node->getActivityId(), 0);
				//	printf("An arc is added between %ld and 0.\n", node->getActivityId());
				}
			}
			previousNode = node;
			node = node->getNext();
		}
	}
/*	node = _firstNode;
	while(0 != node) {
		printf("Node %ld: indegree = %ld\n", node->getActivityId(), node->getInDegree());
		node = node->getNext();
	}*/
}


void
IluSimulatorI::addNode(IloInt i, IloBool frontier) {
	NodeI* node = _firstNode;
	NodeI* previousNode = 0;
	NodeI* newNode = 0;
	while(0 != node) {
		previousNode = node;
		node = node->getNext();
	}
	if(0 == _firstRecyclingNode)
		newNode = new NodeI(frontier);
	else {
		newNode = _firstRecyclingNode;
		_firstRecyclingNode = _firstRecyclingNode->getNext();
		newNode->setNext(0);
		newNode->setTopologicalNext(0);
		newNode->setEndTime(IloInfinity);
		newNode->setStartTime(IloInfinity);
		newNode->setVal(0.0);
		newNode->setExecutedPart(0.0);
		newNode->setIndegree(0);
		newNode->setTopoIndegree(0);
		newNode->setCurrentIndegree(0);
		newNode->setFrontier(frontier);
		newNode->setProcessPlanId(IloIntMax);	
		newNode->setLastProcessPlan(IloFalse);
		newNode->setLastResource(IloFalse);
	}
	newNode->setActivityId(i);
	if(_firstNode == node)
		_firstNode = newNode;
	else
		previousNode->setNext(newNode);
}


void
IluSimulatorI::recycleNode(IloInt i) {
	NodeI* node = _firstNode;
	NodeI* previousNode = 0;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if(i == node->getActivityId()) {
				// The indegrees of the target nodes of node are decreased.
				ArcI* arc = node->getArcs();
				while(0 != arc) {
					NodeI* targetNode = arc->getTarget();
					targetNode->decrIndegree();
					arc = arc->getNext();
				}

				// Outgoing arcs of this node are recycled.
				arc = node->getArcs();
				node->removeArcs();
				ArcI* currentArc = arc;
				ArcI* previousArc = 0;
				while(0 != currentArc) {
					currentArc->setTarget(0);
					previousArc = currentArc;
					currentArc = currentArc->getNext();
				}
				if(0 != previousArc) {
					previousArc->setNext(_firstRecyclingArc);
					_firstRecyclingArc = arc;
				}

				// The histogram of this node is recycled.
				if(0 != node->getHistogram()) {
					IluHistogramI* hist = node->getHistogram();
					hist->setNext(_firstRecyclingHistogram);
					hist->setDeterministic(IloFalse);
					hist->clean();
					_firstRecyclingHistogram = hist;
					node->setHistogram(0);
				}

				// Ingoing arcs of this node are recycled.
				NodeI* tempNode = _firstNode;
				while(0 != tempNode) {
					ArcI* tempArc = tempNode->getArcs();
					previousArc = 0;
					IloBool tempArcStop = IloFalse;
					while(0 == tempArcStop) {
						if(0 == tempArc)
							tempArcStop = IloTrue;
						else {
							if(node == tempArc->getTarget()) {
								if(0 == previousArc)
									tempNode->setArc(tempArc->getNext());
								else
									previousArc->setNext(tempArc->getNext());
								// This arc is recycled.
								tempArc->setNext(_firstRecyclingArc);
								_firstRecyclingArc = tempArc;
								tempArcStop = IloTrue;
							}
							else {
								previousArc = tempArc;
								tempArc = tempArc->getNext();
							}
						}
					}
					tempNode = tempNode->getNext();
				}

				IloInt resId = node->getResource()->getId();
				if(i == _resources[resId]->getNodeId())
					_resources[resId]->setNodeId(IloIntMax);
				
				// This node is recycled.
				if(0 == previousNode)
					_firstNode = node->getNext();
				else
					previousNode->setNext(node->getNext());
				node->setNext(_firstRecyclingNode);
				node->setResource(0);
				node->setTopologicalNext(0);
				node->setDurationVariable(0);
				_firstRecyclingNode = node;

				stop = IloTrue;
			}
			else {
				previousNode = node;
				node = node->getNext();
			}
		}
	}
//	printf("Node %ld is recycled.\n", i);
}


void
IluSimulatorI::clean() {
	NodeI* node = _firstNode;
	while(0 != node) {
		if((IloFalse == node->isFrontier())&&(0 != node->getHistogram())) {
			IluHistogramI* hist = node->getHistogram();
			hist->clean();
		}
		node = node->getNext();
	}
}


void
IluSimulatorI::recycleExecutedNodes(IloNum currentTime) {
	IloBool graphChanged = IloFalse;
	// Recycling of the nodes and the outgoing arcs corresponding to executed activities.
	NodeI* currentNode = _firstNode;
	while(0 != currentNode) {
		if(currentNode->getEndTime() < currentTime) {
			graphChanged = IloTrue;
			NodeI* recyclingNode = currentNode;
			currentNode = currentNode->getNext();
			recycleNode(recyclingNode->getActivityId());
		//	printf("The node %ld is recycled.\n", recyclingNode->getActivityId());
		}
		else	
			currentNode = currentNode->getNext();
	}
	if(IloTrue == graphChanged) {
		// Topological sort
		topologicalSort();
		// Some arcs are added between frontier arcs and other arcs.
		addFrontierArcs();
	}
}


IluSimulatorOneActivityI::IluSimulatorOneActivityI(IloEnv env, IluFrontierI* frontier)
	: _env(env),
	  _processPlanId(-1),
	  _bestResourceId(-1),
	  _activity(0),
	  _bestGlobalCost(IloInfinity) {
	IloInt nbSimulations = frontier->getNbSimulations();
	IloInt nbProcessPlans = frontier->getNbProcessPlans();
	IloInt nbTotalResources = frontier->getNbTotalResources();
	_maxNbSimulations = nbSimulations;
	_lastEndTimeBreakdown = new (env) IluHistogramI(env, _maxNbSimulations);
	_lastEndTimeProcessPlan = new (env) IluHistogramI(env, _maxNbSimulations);
	_bestLastEndTimeBreakdown = new (env) IluHistogramI(env, _maxNbSimulations);
	_bestLastEndTimeProcessPlan = new (env) IluHistogramI(env, _maxNbSimulations);
	IloInt i, j;
	for(i = 0; i < _maxNbSimulations; i++) {
		_lastEndTimeBreakdown->setValue(0.0, 1);
		_bestLastEndTimeBreakdown->setValue(IloInfinity, 1);
		_lastEndTimeProcessPlan->setValue(0.0, 1);
		_bestLastEndTimeProcessPlan->setValue(IloInfinity, 1);	
	}
	IluHistogramI** newBreakdownHists = new (env) IluHistogramI*[nbTotalResources];
	IluHistogramI** newProcessPlanHists = new (env) IluHistogramI*[nbProcessPlans];
	IluHistogramI** newResourceHists = new (env) IluHistogramI*[nbTotalResources];
	IluHistogramI** frontierBreakdownHists = frontier->getLastHistogramBreakdown();
	IluHistogramI** frontierProcessPlanHists = frontier->getLastHistogramProcessPlan();
	IluHistogramI** frontierResourceHists = frontier->getLastHistogramResource();
	for(i = 0 ; i < nbTotalResources ; i++) {
		newBreakdownHists[i] = new (env) IluHistogramI(env, _maxNbSimulations);
		newResourceHists[i] = new (env) IluHistogramI(env, _maxNbSimulations);
		for(j = 0; j < _maxNbSimulations; j++) {
			newBreakdownHists[i]->setValue(frontierBreakdownHists[i]->getValue(j), 1, j);
			newResourceHists[i]->setValue(frontierResourceHists[i]->getValue(j), 1, j);
		}
	}
	for(i = 0; i < nbProcessPlans; i++) {
		newProcessPlanHists[i] = new (env) IluHistogramI(env, _maxNbSimulations);
		for(j = 0; j < _maxNbSimulations; j++)
			newProcessPlanHists[i]->setValue(frontierProcessPlanHists[i]->getValue(j), 1, j);
	}
	_selectionFrontier = new (env) IluFrontierI(nbProcessPlans, nbTotalResources, newBreakdownHists, newProcessPlanHists, newResourceHists);
	_resource = new (env) ResourceI(-1);
}



void
IluSimulatorOneActivityI::setAllocatedActivity(IluActivityI* act, IluUnaryResourceI* res, IloInt processPlanId) {
//	printf("Assessment: activity %ld is allocated to resource %ld.\n", act->getIndex(), res->getIndex());
	_activity = act;
	_resource->setId(res->getIndex());
	_resource->setBreakdownDurationVariable(res->getBreaksDurationVar());
	_resource->setInterBreakdownDurationVariable(res->getInterBreaksVar());
	_resource->setBroken(res->isBrokenDown());
	if(_resource->isBrokenDown())
		_resource->setStartTime(res->getBreakdownStartTime());
	_processPlanId = processPlanId;
}



void
IluSimulatorOneActivityI::simulate(IloNum t, IloInt nb) {
	if(_maxNbSimulations >= nb) {
		_resource->getBreakdownDurationVariable()->getRandom().reSeed(1);
		IloInt j, resourceIndex;
		resourceIndex = _resource->getId();
		IluHistogramI** breakdownHists = _selectionFrontier->getLastHistogramBreakdown();
		IluHistogramI** processPlanHists = _selectionFrontier->getLastHistogramProcessPlan();
		IluHistogramI** resourceHists = _selectionFrontier->getLastHistogramResource();
		IloNum lastBreakdownEndTime, currentBreakdownStartTime, currentBreakdownEndTime;
		
		for(j = 0; j < nb; j++) {
		//	printf("\n");
		// The allocated resource is initialized.
			lastBreakdownEndTime = breakdownHists[resourceIndex]->getValue(j);
		//	printf("Resource %ld: last breakdown occured at %.14f\n", resourceIndex, lastBreakdownEndTime);
			if(_resource->isBrokenDown()) {
				currentBreakdownStartTime = _resource->getStartTime();
				currentBreakdownEndTime = currentBreakdownStartTime + _resource->getBreakdownDurationVariable()->getValue(t - currentBreakdownStartTime);
			}
			else {
				currentBreakdownStartTime = lastBreakdownEndTime + _resource->getInterBreakdownDurationVariable()->getValue();
				currentBreakdownEndTime = currentBreakdownStartTime + _resource->getBreakdownDurationVariable()->getValue();
			}
		
			IloInt activityId = _activity->getIndex();
		//	printf("%ld\n", activityId);			

			IloNum lastProcessPlan, lastResource;
			lastProcessPlan = processPlanHists[_processPlanId]->getValue(j);

			lastResource = resourceHists[resourceIndex]->getValue(j);

			IloNum start = IloMax(lastProcessPlan, lastResource);
			IloNum end = start + _activity->getRandomVar()->getValue();

		/*	if(lastBreakdownEndTime > end)
				cout << "Error: resource " << _resource->getId() << " breaks down too late." << endl;*/

			while(currentBreakdownStartTime < end) {
				if(currentBreakdownStartTime > start)
					end += currentBreakdownEndTime - IloMax(currentBreakdownStartTime, t);
				else if(currentBreakdownEndTime > start) {
					IloNum duration = end - start;
					start = currentBreakdownEndTime;
					end = start + duration;
				}
			/*	printf("Resource %ld starts to break down at %.14f and ends to break down at %.14f.\n",
					_resource->getId(),
					currentBreakdownStartTime,
					currentBreakdownEndTime);*/
				lastBreakdownEndTime = currentBreakdownEndTime;
				currentBreakdownStartTime = lastBreakdownEndTime + _resource->getInterBreakdownDurationVariable()->getValue();
				currentBreakdownEndTime = currentBreakdownStartTime + _resource->getBreakdownDurationVariable()->getValue();
			}
		//	printf("Activity %ld starts execution at %.14f and ends execution at %.14f\n", activityId, start, end);

		//	The two histograms are updated.
			_lastEndTimeBreakdown->setValue(lastBreakdownEndTime, 1, j);
			_lastEndTimeProcessPlan->setValue(end, 1, j);	
		}
	}
	else
		cout << "The number of simulations you want to run is too big!" << endl;
}



void
IluSimulatorOneActivityI::updateBest(IloNum beta, IloNum allocCost, IloInt dueDate, IloNum phi, IloNum pendingDur, IloNum extendedMDur) {
	IloNum expEndTime = _lastEndTimeProcessPlan->getAverage() + pendingDur - extendedMDur;
	IloNum tardiCost = 	phi * IloMax(expEndTime - dueDate, 0.0);	
	IloNum globalCost = allocCost + beta * tardiCost;
	if(globalCost < _bestGlobalCost) {
		_bestGlobalCost = globalCost;
		_bestLastEndTimeProcessPlan->clean();
		_bestLastEndTimeBreakdown->clean();
		for(IloInt i = 0; i < _maxNbSimulations; i++) {
			_bestLastEndTimeProcessPlan->setValue(_lastEndTimeProcessPlan->getValue(i), 1, i);
			_bestLastEndTimeBreakdown->setValue(_lastEndTimeBreakdown->getValue(i), 1, i);
		}
		_bestResourceId = _resource->getId();
	}
//	printf("The best resource so far is %ld.\n", _bestResourceId);
}



void
IluSimulatorOneActivityI::updateSelectionFrontier() {
	IluHistogramI** breakdownHists = _selectionFrontier->getLastHistogramBreakdown();
	IluHistogramI** processPlanHists = _selectionFrontier->getLastHistogramProcessPlan();
	IluHistogramI** resourceHists = _selectionFrontier->getLastHistogramResource();

	IluHistogramI* tempBreakdownHist = breakdownHists[_bestResourceId];
	IluHistogramI* tempProcessPlanHist = processPlanHists[_processPlanId];

	if(_bestResourceId >= 0) {	
		breakdownHists[_bestResourceId] = _bestLastEndTimeBreakdown;
		processPlanHists[_processPlanId] = _bestLastEndTimeProcessPlan;
		for(IloInt i = 0; i < _maxNbSimulations; i++)
			resourceHists[_bestResourceId]->setValue(_bestLastEndTimeProcessPlan->getValue(i), 1, i);

		_bestLastEndTimeBreakdown = tempBreakdownHist;
		_bestLastEndTimeProcessPlan = tempProcessPlanHist;
		// Best histograms are cleaned.
		_bestLastEndTimeBreakdown->clean();
		_bestLastEndTimeProcessPlan->clean();
		for(IloInt j = 0; j < _maxNbSimulations; j++) {
			_bestLastEndTimeBreakdown->setValue(IloInfinity, 1);
			_bestLastEndTimeProcessPlan->setValue(IloInfinity, 1);
		}
		_bestGlobalCost = IloInfinity;
	}
}