// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilureactivescheduler.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/iludynamicscheduler.h>
#include <ilconcert/ilomodel.h>
#include <ilconcert/ilosys.h>
#include <ctime>




void
IluDynamicSchedulerI::execute(IluEventI* eventP, IluTimeEventManagerI* tEMgrP) {
	eventP->execute(this, tEMgrP);
}


IloInt
IluDynamicSchedulerI::getEffectAllocation(IloInt actIndex) {
	IloInt resIndex = -1;
	for(IloInt i = 0; -1 == resIndex; i++) {
		Allocation* currentAllocation = _effectAllocations[i];
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == currentAllocation)
				stop = IloTrue;
			else {
				if(actIndex == currentAllocation->getId()) {
					resIndex = i;
					stop = IloTrue;
				}
				else				
					currentAllocation = currentAllocation->getNext();
			}
		}
	}
	return resIndex;
}


void
IluDynamicSchedulerI::setEffectAllocation(Allocation* newEffectAllocation, IloInt resIndex) {
	Allocation* currentAllocation = _effectAllocations[resIndex];
	Allocation* previousAllocation = 0;
	while(0 != currentAllocation) {
		previousAllocation = currentAllocation;
		currentAllocation = currentAllocation->getNext();
	}
	if(0 == previousAllocation)
		_effectAllocations[resIndex] = newEffectAllocation;
	else
		previousAllocation->setNext(newEffectAllocation);
}




//-------------------------------------------------------------------
//----------------  PRIORITY HEAP (based on a binary heap)  ---------
//-------------------------------------------------------------------


IluPriorityHeap::IluPriorityHeap(IloInt size)
  : _maxSize(size), _size(0) 
{
  if (size == 0)
    _maxSize = 16;
  _elems = new Node*[_maxSize + 1];
}

IluPriorityHeap::~IluPriorityHeap() {
  delete [] _elems;
  _elems = 0;
}


void IluPriorityHeap::resize() {
  if (_size == _maxSize-1) {
    _maxSize *= 2;
    Node** tmp = _elems;
    _elems = new Node*[_maxSize + 1];
    for(IloInt i=0 ; i < _size ; i++)
      _elems[i+1] = tmp[i+1];
    delete [] tmp;
  }
}


void IluPriorityHeap::insert(IluPriorityHeap::Node* node) {
  ((Node*)node)->setPosition(-1);
  resize();
  _size++;
  assert(_size < _maxSize); 
  _elems[_size] = (Node*) node;
  ((Node*)node)->setPosition(_size);
  upHeap(_size); 
}


void IluPriorityHeap::upHeap(IloInt k) {
  Node* elem = _elems[k]; 
  while (k != 1 && _elems[k>>1]->getKey() > elem->getKey()) {
    _elems[k] = _elems[k>>1]; 
    _elems[k]->setPosition(k);
    k >>= 1; 
  }
  _elems[k] = elem;
  elem->setPosition(k);
}


void IluPriorityHeap::downHeap(IloInt k) {
  Node* elem = _elems[k];
  IloInt halfSize = _size >> 1;
  while (k <= halfSize) {
    IloInt j = k+k;
    if (j < _size && _elems[j]->getKey() > _elems[j+1]->getKey()) j++;
    if (elem->getKey() <= _elems[j]->getKey()) break;
    _elems[k] = _elems[j]; 
    _elems[k]->setPosition(k);
    k = j;
  }
  _elems[k] = elem; 
  elem->setPosition(k);
}


IluPriorityHeap::Node* IluPriorityHeap::extractMin() 
{
  Node* elem = _elems[1];
  _elems[1] = _elems[_size];
  _elems[1]->setPosition(1);
  _size--;
  downHeap(1);
  elem->setPosition(-1);
  return elem;
}


IluProgressiveSchedulerI::IluProgressiveSchedulerI(IluLocalSearchSolverI*	lSSolver,
												   IloInt					deltaTProgressMin,
												   IloInt					deltaTProgressMax,
												   IloNum					sigmaTProgressMin,
												   IloNum					sigmaTProgressMax,
												   IluSimulatorI*			simP,
												   IloInt*					curAllocations,
												   Allocation**				effectAllocations)
		:	_deltaTProgressMin			(deltaTProgressMin),
		    _deltaTProgressMax			(deltaTProgressMax),
			_sigmaTProgressMin			(sigmaTProgressMin),
			_sigmaTProgressMax			(sigmaTProgressMax),
			_localSSolver				(lSSolver),
			IluDynamicSchedulerI		(simP, curAllocations, effectAllocations),
			_earliestEndTimeResource	(0),
			_earliestBreakdown			(0),
			_currentEndTimeResource		(0),
			_currentBreakdown			(0),
			_priorityHeap				(0),
			_recycling					(0),
			_eligible					(0),
			_pending					(0),
			_lastSelectedActivity		(0) {
	IluSimulatorI* sim = getSimulator();
	// Simulator: some random variables are associated with resources.
	IluExtendedModelI* uncModel = _localSSolver->getUncModel();
	IloInt nbTotalResources = uncModel->getNbTotalResources();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IloInt nbTotalActivities = uncModel->getNbTotalActivities();

	IloInt i, j, k;
	for(i = 0; i < nbTotalResources; i++) {
		IluUnaryResourceI* uncRes = uncModel->getUUnaryResource(i);
		sim->setBreakdownDurationVariable(i, uncRes->getBreaksDurationVar());
		sim->setInterBreakdownDurationVariable(i, uncRes->getInterBreaksVar());
	}

	_currentFrontier = sim->getStartFrontier();
	IluHistogramI** histResource = _currentFrontier->getLastHistogramResource();
	// Simulator: frontier nodes are initialized.
	IluSimulatorI::NodeI* node = sim->getFirstNode();
	for(i = 0; i < nbTotalResources; i++) {
		IluSimulatorI::ResourceI* res = sim->getResource(i);
		node->setActivityId(nbTotalActivities + i + 1);
		node->setResource(res);
		node->setHistogram(histResource[i]);
		node = node->getNext();
	}
	IluHistogramI** histProcessPlan = _currentFrontier->getLastHistogramProcessPlan();
	for(i = 0; i < nbProcessPlans; i++) {
		node->setProcessPlanId(i);
		node->setActivityId(nbTotalActivities + nbTotalResources + i + 1);
		node->setHistogram(histProcessPlan[i]);
		node = node->getNext();
	}

	IloRandom activityRand = uncModel->getUUnaryResource(0)->getInterBreaksVar()->getRandom();
	_allocationFrontier = sim->getEndFrontier();
// Creation of the simulator of one activity
	IluSimulatorOneActivityI* selecSim = new (activityRand.getEnv()) IluSimulatorOneActivityI(activityRand.getEnv(), _allocationFrontier);
	_selectionFrontier = selecSim->getSelectionFrontier();
	_selecSim = selecSim;

	IloInt nbActivities = nbTotalActivities/nbProcessPlans;
	IloInt nbResources = uncModel->getAltAllocSet(1)->getNbAltURes();
	IloInt* dueDates = uncModel->getDueDates();
	IloNum* phis = uncModel->getPhis();
	IloNum** alloc = uncModel->getAllocCosts();
	IloNum* allocExp = uncModel->getAllocExp();

	// Creation and initialisation of _priorities
	_allocCostInvSum = new (activityRand.getEnv()) IloNum[nbTotalActivities + 1];
	for(i = 1 ; i < nbTotalActivities + 1 ; i++) {
		_allocCostInvSum[i] = 0.0;
		for(j = 0 ; j < nbResources ; j++) {
			_allocCostInvSum[i] += 1/alloc[i][j];
		}
	//	printf("Activity %ld: AllocCostInvSum = %14f\n", i, _allocCostInvSum[i]);
	}
	_energyActivities = new (activityRand.getEnv()) IloNum*[nbTotalActivities + 1];
	for(i = 0 ; i < nbTotalActivities + 1 ; i++)
		_energyActivities[i] = new (activityRand.getEnv()) IloNum[nbResources];
	for(i = 1 ; i < nbTotalActivities + 1 ; i++) {
		for(j = 0 ; j < nbResources ; j++) {
			_energyActivities[i][j] = uncModel->getAltAllocSet(i)->getUActivity()->getIndicativeProcessingTime() / (_allocCostInvSum[i] * alloc[i][j]);
		//	printf("Energy activity %ld %ld = %.14f\n", i, j, _energyActivities[i][j]);
		}
	}
	_energyResources = new (activityRand.getEnv()) IloNum[nbTotalResources];
	for(i = 0 ; i < nbTotalResources ; i++) {
		_energyResources[i] = 0.0;
		for(j = 1 ; j < nbTotalActivities + 1 ; j++) {
			for(k = 0 ; k < nbResources ; k++) {
				if(uncModel->getAltAllocSet(j)->getUResource(k)->getIndex() == i) {
					_energyResources[i] += _energyActivities[j][k];
				}
			}
		}
	//	printf("Resource %ld: energy = %.14f\n", i, _energyResources[i]);
	}
	_criticities = new (activityRand.getEnv()) IloNum[nbTotalActivities + 1];
	_criticities[0] = 0.0;
	for(i = 1 ; i < nbTotalActivities + 1 ; i++) {
		_criticities[i] = 1.0;
		for(j = 0 ; j < nbResources ; j++) {
			_criticities[i] += 1 - _energyActivities[i][j] / _energyResources[uncModel->getAltAllocSet(i)->getUResource(j)->getIndex()];
		}
	//	printf("Activity %ld: criticity = %.14f\n", i, _criticities[i]);
	}
	_extendedMDur= new (activityRand.getEnv()) IloNum[nbTotalActivities + 1];
	_extendedMDur[0] = 0.0;
	for(i = 1 ; i < nbTotalActivities + 1 ; i++) {
		_extendedMDur[i] = _criticities[i] * uncModel->getAltAllocSet(i)->getUActivity()->getIndicativeProcessingTime();
	//	printf("Activity %ld: extended duration = %.14f\n", i, _extendedMDur[i]);
	}
	IluHistogramI** processPlanHist = _selectionFrontier->getLastHistogramProcessPlan();
	_pendingDur = new (activityRand.getEnv()) IloNum[nbProcessPlans];
	for(i = 0 ; i < nbProcessPlans ; i++) {
		_pendingDur[i] = 0.0;
		for(j = i * nbActivities + 1 ; j < (i + 1) * nbActivities + 1 ; j++) {
			_pendingDur[i] += _extendedMDur[j];
		}
	//	printf("Process plan %ld: pending duration = %.14f\n", i, _pendingDur[i]);
	}
	IloNum tempPending = 0.0;
	for(i = 0 ; i < nbProcessPlans ; i++)
		tempPending += _pendingDur[i];
	_meanPendingDur = new (activityRand.getEnv()) IloNum[nbProcessPlans];
	for(i = 0 ; i < nbProcessPlans ; i++) {
		_meanPendingDur[i] = (tempPending - _pendingDur[i]) / nbProcessPlans;
	//	printf("Process plan %ld: mean pending duration = %.14f\n", i, _meanPendingDur[i]);
	}
	_endPExp = new (activityRand.getEnv()) IloNum[nbProcessPlans];
	for(i = 0 ; i < nbProcessPlans ; i++) {
		_endPExp[i] = processPlanHist[i]->getAverage() + _pendingDur[i];
	//	printf("Process plan %ld: endPExp = %.14f\n", i, _endPExp[i]);
	}
	_tardiExp = new (activityRand.getEnv()) IloNum[nbProcessPlans];
	for(i = 0 ; i < nbProcessPlans ; i++) {
		_tardiExp[i] = phis[i] * IloMax(_endPExp[i] - (IloNum)dueDates[i], 0.0);
	//	printf("Process plan %ld: expected tardiness = %.14f\n", i, _tardiExp[i]);
	}
	_weights = new (activityRand.getEnv()) IloNum[nbProcessPlans];
	for(i = 0 ; i < nbProcessPlans ; i++) {
		_weights[i] = _tardiExp[i] + allocExp[i * nbActivities + 1];
	//	printf("Process plan %ld: weight = %.14f\n", i, _weights[i]);
	}

	_nodes = new (activityRand.getEnv()) IluPriorityHeap::Node*[nbTotalActivities + 1];
	for(i = 0; i < nbTotalActivities + 1; i++)
		_nodes[i] = 0;
	for(i = 0; i < nbProcessPlans; i++) {
		IluActivityI* uncAct = uncModel->getAltAllocSet(i * nbActivities + 1)->getUActivity();
		IloNum priority = _weights[i] * IloExponent(-IloMax(0.0, (IloNum)dueDates[i] - 0.0 - _pendingDur[i]) / (2 * _meanPendingDur[i])) / _pendingDur[i];
	//	printf("Activity %ld: priority = %.14f, due date = %ld\n", i * nbActivities + 1, priority, dueDates[i]);
		IluPriorityHeap::Node* node = new IluPriorityHeap::Node(uncAct, -priority);
		_priorityHeap.insert(node);
		_nodes[i * nbActivities + 1] = node;
	}

	// Creations and initialisations of the pending and eligible lists
	for(i = nbTotalActivities ; i > 0 ; i--) {
		ItemI* newItem = new (activityRand.getEnv()) ItemI(i);
		newItem->setNext(_pending);
		_pending = newItem;
	}
	for(i = 0 ; i < nbProcessPlans ; i++) {
		ItemI* newItem = new (activityRand.getEnv()) ItemI(i * nbActivities + 1);
		newItem->setNext(_eligible);
		_eligible = newItem;
	}
}


IloBool
IluProgressiveSchedulerI::timeForSelection(IloNum currentTime) {
	IloBool selection = IloFalse;
	if(0 != _pending) {
		IloNum minTime = _allocationFrontier->getEarliestProcessPlanEndTime(currentTime);
		if(minTime - currentTime <= _deltaTProgressMin)
			selection = IloTrue;
		IloNum maxStandardDev = _allocationFrontier->getMaxStandardDeviationProcessPlan(currentTime);
		if(maxStandardDev <= _sigmaTProgressMin)
			selection = IloTrue;
	}
	return selection;
}


void
IluProgressiveSchedulerI::updatePG(IloEnv env, IloNum currentTime) {
	IluExtendedModelI* uncModel = _localSSolver->getUncModel();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IloInt nbTotalActivities = uncModel->getNbTotalActivities();
	IloInt nbActivities = nbTotalActivities/nbProcessPlans;
	IloInt nbTotalResources = uncModel->getNbTotalResources();
	IloInt* processPlanIds = uncModel->getProcessPlanIds();

	IloInt i;

	IloBool graphChanged = IloFalse;

	// Recycling of the nodes and the outgoing arcs corresponding to executed activities.
	IluSimulatorI* allocSim = getSimulator();
	IluSimulatorI::NodeI* currentNode = allocSim->getFirstNode();
	while(0 != currentNode) {
		if(currentNode->getEndTime() < currentTime) {
			graphChanged = IloTrue;
			IluSimulatorI::NodeI* recyclingNode = currentNode;
			currentNode = currentNode->getNext();
			allocSim->recycleNode(recyclingNode->getActivityId());
		//	printf("The node %ld is recycled.\n", recyclingNode->getActivityId());
		}
		else	
			currentNode = currentNode->getNext();
	}
	
	IloInt nbSimulations = allocSim->getNbSimulations();
	// Some new nodes are created. They correspond to new selected activities. Some histograms are also created.
	for(IloSchedulerSolution::ActivityIterator ite(_solution); ite.ok(); ++ite) {
		graphChanged = IloTrue;
		IluActivityI* uncAct = (IluActivityI*)((*ite).getObject());
		IloInt actIndex = uncAct->getIndex();
		allocSim->addNode(actIndex, IloFalse);
		IluRandomVariableI* randVar = uncAct->getRandomVar();
		IloInt processPlanId = processPlanIds[actIndex];
		IluSimulatorI::NodeI* node = allocSim->getFirstNode();
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == node)
				stop = IloTrue;
			else {
				if(actIndex == node->getActivityId()) {
					IluHistogramI* h = 0;
					if(0 == allocSim->getFirstRecyclingHistogram())
						h = new (env) IluHistogramI(env, nbSimulations);
					else {
						h = allocSim->getFirstRecyclingHistogram();
						allocSim->setFirstRecyclingHistogram(h->getNext());
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
		if(0 == _pending) {
		//	printf("A dummy node is added.\n");
			IluExtendedModelI* uncModel = _localSSolver->getUncModel();
			IloRandom activityRand = uncModel->getUUnaryResource(0)->getInterBreaksVar()->getRandom();
			IluNormalVariableI*	randVar = new (activityRand.getEnv()) IluNormalVariableI(activityRand.getEnv(), activityRand, 0.0,   0.0,   0.0,  0.0);
		// This is a dummy random variable used for computing makespan.
			allocSim->addNode(0, IloFalse);
			IluSimulatorI::NodeI* node = allocSim->getFirstNode();
			IloBool stop = IloFalse;
			while(IloFalse == stop) {
				if(0 == node)
					stop = IloTrue;
				else {
					if(0 == node->getActivityId()) {
						IluHistogramI* h = 0;
						if(0 == allocSim->getFirstRecyclingHistogram())
							h = new (env) IluHistogramI(env, nbSimulations);
						else {
							h = allocSim->getFirstRecyclingHistogram();
							allocSim->setFirstRecyclingHistogram(h->getNext());
							h->setNext(0);
						}
						// Simulator: makespan node is initialized.
						node->setHistogram(h);
						node->setDurationVariable(randVar);
						stop = IloTrue;
					}
					node = node->getNext();
				}
			}

			for(i = 0; i < nbProcessPlans; i++) {
				IluSimulatorI::NodeI* firstNode = allocSim->getFirstNode();
				while(0 != firstNode) {
					IloInt firstIndex = firstNode->getActivityId();
					if(((i + 1) * nbActivities == firstIndex)&&(IloFalse == allocSim->hasArc(firstIndex, 0))) {
						allocSim->addArc(firstIndex, 0);						// This arc is added in order to compute makespan.
					//	printf("An arc is added between node %ld and node 0.\n", firstIndex);
						if(firstNode->getEndTime() < IloInfinity)
							allocSim->decrCurrentIndegree(0);
					}
					firstNode = firstNode->getNext();
				}
			}
		}


		// Initial precedence constraints are retrieved and arcs are thus created.
		for(i = 0; i < nbProcessPlans; i++) {
			IluSimulatorI::NodeI* firstNode = allocSim->getFirstNode();
			IloInt secondIndex = -1;
			IloInt firstIndex = -1;
			IloInt sIndex = -1;
			IloInt bIndex = -1;
			while(0 != firstNode) {
				if((IloFalse == firstNode->isFrontier())&&(i == firstNode->getProcessPlanId())) {
					firstIndex = firstNode->getActivityId();
					IluSimulatorI::NodeI* secondNode = allocSim->getFirstNode();
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
								if((IloFalse == allocSim->hasArc(sIndex, bIndex))&&(1 == bIndex - sIndex)) {
									allocSim->addArc(sIndex, bIndex);
								//	printf("An arc is added between node %ld and node %ld.\n", sIndex, bIndex);
								if((firstIndex == sIndex)&&(firstNode->getEndTime() < IloInfinity))
									allocSim->decrCurrentIndegree(bIndex);
								if((firstIndex == bIndex)&&(secondNode->getEndTime() < IloInfinity))
									allocSim->decrCurrentIndegree(bIndex);
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
			IluSimulatorI::ResourceI* res = allocSim->getResource(i);
			IloInt nodeId = res->getNodeId();
			if(_solution.contains(resource)) {
				if(_solution.hasSetupRC(resource)) {
					IloResourceConstraint rCt = _solution.getSetupRC(resource);
					if(nodeId < IloIntMax) {
						IloActivity act = rCt.getActivity();
						IluActivityI* uncAct = (IluActivityI*)(act.getObject());
						IloInt actIndex = uncAct->getIndex();
						if(IloFalse == allocSim->hasArc(nodeId, actIndex)) {
							allocSim->addArc(nodeId, actIndex);
						//	printf("An arc is added between node %ld and node %ld.\n", nodeId, actIndex);
							if(allocSim->getEndTime(nodeId) < IloInfinity)
								allocSim->decrCurrentIndegree(actIndex);
						}
					}
					IloResourceConstraint nextRCt = _solution.getSetupRC(resource);
					IloInt lastActIndex = -1;
					do {
						rCt = nextRCt;
						IloActivity act = rCt.getActivity();
						IluActivityI* uncAct = (IluActivityI*)(act.getObject());
						IloInt actIndex = uncAct->getIndex();
						IluSimulatorI::NodeI* node = allocSim->getFirstNode();
						IloBool stop = IloFalse;
						while(IloFalse == stop) {
							if(0 == node)
								stop = IloTrue;
							else {
								if(actIndex == node->getActivityId()) {
									node->setResource(allocSim->getResource(i));
									stop = IloTrue;
								}
								else
									node = node->getNext();
							}
						}
						if((-1 != lastActIndex)&&(IloFalse == allocSim->hasArc(lastActIndex, actIndex))) {
							allocSim->addArc(lastActIndex, actIndex);
						//	printf("An arc is added between node %ld and node %ld.\n", lastActIndex, actIndex);
							if(allocSim->getEndTime(lastActIndex) < IloInfinity)
								allocSim->decrCurrentIndegree(actIndex);

						}
						lastActIndex = actIndex;
						if(_solution.hasNextRC(rCt))
							nextRCt = _solution.getNextRC(rCt);
					} while(_solution.hasNextRC(rCt));
				}
			}
		}
		// Topological sort
		allocSim->topologicalSort();

		// Some arcs are added between frontier arcs and other arcs.
		allocSim->addFrontierArcs();

	//	IluSimulatorI::NodeI* node = allocSim->getFirstNode();
	//	while(0 != node) {
	//		printf("Node %ld has a current in degree = %ld.\n", node->getActivityId(), node->getCurrentInDegree());
	//		node = node->getNext();
	//	}
	}

	// The Scheduler solution _solution is emptied.
	_solution.end();
}


void
IluProgressiveSchedulerI::updateCurFrontier() {
	// The current frontier is updated.
	IluSimulatorI* allocSim = getSimulator();
	IluSimulatorI::NodeI* node = allocSim->getFirstTopologicalSort();
	IluFrontierI* startFrontier = allocSim->getStartFrontier();
	IluHistogramI** processPlanHist = startFrontier->getLastHistogramProcessPlan();
	IluHistogramI** resourceHist = startFrontier->getLastHistogramResource();
	while(0 != node) {
		if((node->getEndTime() < IloInfinity)&&(0 != node->getResource())) {
			IloNum endTime = node->getEndTime();
			IloInt processPlanIndex = node->getProcessPlanId();
			processPlanHist[processPlanIndex]->setValue(endTime, 1, 0);
			processPlanHist[processPlanIndex]->setDeterministic(IloTrue);
			IloInt resourceIndex = node->getResource()->getId();
			resourceHist[resourceIndex]->setValue(endTime, 1, 0);
			resourceHist[resourceIndex]->setDeterministic(IloTrue);
		}
		node = node->getTopologicalNext();
	}
}


void
IluProgressiveSchedulerI::updateAllocFrontier() {
	IloInt j;
	// The allocation frontier is updated.
	IloInt nbProcessPlans = _localSSolver->getUncModel()->getNbProcessPlans();
	IloInt nbTotalResources = _localSSolver->getUncModel()->getNbTotalResources();

	IluHistogramI** lastHistogramBreakdownAlloc = _allocationFrontier->getLastHistogramBreakdown();
	IluHistogramI** lastHistogramProcessPlanAlloc = _allocationFrontier->getLastHistogramProcessPlan();
	IluHistogramI** lastHistogramResourceAlloc = _allocationFrontier->getLastHistogramResource();

	IluSimulatorI* sim = getSimulator();
	IluFrontierI* simFrontier = sim->getEndFrontier();
	IluHistogramI** lastHistogramBreakdownSim = simFrontier->getLastHistogramBreakdown();
	IluHistogramI** lastHistogramProcessPlanSim = simFrontier->getLastHistogramProcessPlan();
	IluHistogramI** lastHistogramResourceSim = simFrontier->getLastHistogramResource();

	for(j = 0; j < nbProcessPlans; j++)
		lastHistogramProcessPlanAlloc[j] = lastHistogramProcessPlanSim[j];

	for(j = 0; j < nbTotalResources; j++) {
		lastHistogramBreakdownAlloc[j] = lastHistogramBreakdownSim[j];
		lastHistogramResourceAlloc[j] = lastHistogramResourceSim[j];
	}
}


IloInt
IluProgressiveSchedulerI::select(IloInt currentTime) {
	IloInt selectedActivityIndex = -1;
	IloBool selection = IloFalse;
	computeEligiblePriorities(currentTime);
	while(eligibleActivities()&&(IloFalse == selection)) {
		IluExtendedModelI* uncModel = _localSSolver->getUncModel();
		IloInt nbProcessPlans = uncModel->getNbProcessPlans();
		IloInt* processPlanIds = uncModel->getProcessPlanIds();
		IluHistogramI** processPlanHist = _selectionFrontier->getLastHistogramProcessPlan();
		IluPriorityHeap::Node* node = _priorityHeap.extractMin();
		IluActivityI* uncAct = (IluActivityI*)node->getObject();
		IloInt actIndex = uncAct->getIndex();
	//	printf("%s\n", uncAct->getName());
		_nodes[actIndex] = 0;
		IloInt currentProcessPlanId = processPlanIds[actIndex];
	//	printf("Process plan %ld: average = %.14f and standard deviation = %.14f\n", currentProcessPlanId, processPlanHist[currentProcessPlanId]->getAverage(),
	//		processPlanHist[currentProcessPlanId]->getStandardDev());
		if((processPlanHist[currentProcessPlanId]->getAverage() < currentTime + _deltaTProgressMax)
					|| (processPlanHist[currentProcessPlanId]->getStandardDev() < _sigmaTProgressMax)) {
			ItemI* currentEligible = _eligible;
			ItemI* previousEligible = 0;
			IloBool loopCondition = IloTrue;
			while(loopCondition) {
				if(0 == currentEligible) {
					loopCondition = IloFalse;
				}
				else {
					if(actIndex == currentEligible->getIndex()) {
						loopCondition = IloFalse;
					}
					else {
						previousEligible = currentEligible;
						currentEligible = currentEligible->getNext();
					}
				}
			}
			if(previousEligible != 0)
				previousEligible->setNext(currentEligible->getNext());
			else
				_eligible = _eligible->getNext();
			selection = IloTrue;
			selectedActivityIndex = actIndex;
		//	printf("Activity %ld is selected.\n", selectedActivityIndex);
			_lastSelectedActivity = currentEligible;
			ItemI* currentPending = _pending;
			ItemI* previousPending = 0;
			loopCondition = IloTrue;
			while(loopCondition) {
				if(0 == currentPending) {
					loopCondition = IloFalse;
				}
				else {
					if(actIndex == currentPending->getIndex()) {
						loopCondition = IloFalse;
					}
					else {
						previousPending = currentPending;
						currentPending = currentPending->getNext();
					}
				}
			}
			if(0 != previousPending)
				previousPending->setNext(currentPending->getNext());
			else
				_pending = _pending->getNext();
			currentPending->setNext(_recycling);
			_recycling = currentPending;
			if(0 != previousPending)
				currentPending = previousPending->getNext();
			else
				currentPending = _pending;
			loopCondition = IloTrue;
			while(loopCondition) {
				if(0 == currentPending) {
					loopCondition = IloFalse;
				}
				else {
					if(currentProcessPlanId == processPlanIds[currentPending->getIndex()]) {
						loopCondition = IloFalse;
					// A pending activity becomes eligible.
						ItemI* newEligible = _recycling;
						_recycling = _recycling->getNext();
						newEligible->setIndex(currentPending->getIndex());
						newEligible->setNext(_eligible);
						_eligible = newEligible;
					//	printf("Activity %ld becomes eligible.\n", newEligible->getIndex());
						uncAct = uncModel->getAltAllocSet(newEligible->getIndex())->getUActivity();
						IluPriorityHeap::Node* node = new IluPriorityHeap::Node(uncAct, 0.0);
						_priorityHeap.insert(node);
						_nodes[newEligible->getIndex()] = node;
					}
					else {
						currentPending = currentPending->getNext();
					}
				}
			}
		}
		else {
			ItemI* currentEligible = _eligible;
			ItemI* previousEligible = 0;
			while(currentEligible->getIndex() != actIndex) {
				previousEligible = currentEligible;
				currentEligible = currentEligible->getNext();
			}
			if(0 != previousEligible)
				previousEligible->setNext(currentEligible->getNext());
			else
				_eligible = _eligible->getNext();
			currentEligible->setNext(_recycling);
			_recycling = currentEligible;
		//	printf("Activity %ld is no more eligible.\n", currentEligible->getIndex());
		}
	}
	return selectedActivityIndex;
}


IloInt
IluProgressiveSchedulerI::selectActivities(IloNum time) {
	IloInt nbSelectedActivities = 0;
	IloInt nbSimulations = getSimulator()->getNbSimulations();
	IluExtendedModelI* uncModel = getLocalSearchSolver()->getUncModel();
	IloInt nbResources = uncModel->getNbResources();
	IloInt* processPlanIds = uncModel->getProcessPlanIds();
	IloInt* dueDates = uncModel->getDueDates();
	IloNum* phis = uncModel->getPhis();
	IloNum beta = uncModel->getBeta();
	IloNum** allocCosts = uncModel->getAllocCosts();
	do {
	//	printf("Selection frontier is updated.\n");
		_selecSim->updateSelectionFrontier();

	//	printf("Solution is updated.\n");
		updateSolution();
		IloInt activityIndex = select(time);
		if(-1 != activityIndex) {
			nbSelectedActivities++;
			AltAllocSetI* altAllocSet = uncModel->getAltAllocSet(activityIndex);
			for(IloInt i = 0; i < nbResources; i++) {
				IloInt processPlanId = processPlanIds[activityIndex];
				_selecSim->setAllocatedActivity(altAllocSet->getUActivity(),
												altAllocSet->getUResource(i),
												processPlanId);
				_selecSim->simulate(time, nbSimulations);
				_selecSim->updateBest(beta, allocCosts[activityIndex][_selecSim->getResourceId()], dueDates[processPlanId], phis[processPlanId],
																										_pendingDur[processPlanId],	_extendedMDur[activityIndex]);
			}
		//	printf("Activity %ld is selected and associated with resource %ld.\n", activityIndex, _selecSim->getBestResourceId());
			setCurAllocation(activityIndex, _selecSim->getBestResourceId());
		}
	} while(eligibleActivities());
	//	printf("Selection frontier is updated.\n");
	_selecSim->updateSelectionFrontier();
//	printf("Solution is updated.\n");
	updateSolution();
	return nbSelectedActivities;
}


void
IluProgressiveSchedulerI::computeEligiblePriorities(IloNum currentTime) {
	IluExtendedModelI* uncModel = _localSSolver->getUncModel();
	IloInt nbTotalActivities = uncModel->getNbTotalActivities();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IloInt nbActivities = nbTotalActivities/nbProcessPlans;
	IloInt nbTotalResources = uncModel->getNbTotalResources();
	IloInt nbResources = uncModel->getAltAllocSet(1)->getNbAltURes();
	IloInt* processPlanIds = uncModel->getProcessPlanIds();
	IloInt* dueDates = uncModel->getDueDates();
	IloNum* phis = uncModel->getPhis();
	IloNum** alloc = uncModel->getAllocCosts();
	IloNum* allocExp = uncModel->getAllocExp();
	IloNum beta = uncModel->getBeta();
	IloInt i, j;

	if(0 != _lastSelectedActivity) {
		for(i = 0; i < nbResources; i++) {
			j = _lastSelectedActivity->getIndex();
			_energyResources[uncModel->getAltAllocSet(j)->getUResource(i)->getIndex()] -= _energyActivities[j][i];
		}
		
		ItemI* currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			_criticities[i] = 1.0;
			for(j = 0 ; j < nbResources ; j++)
				_criticities[i] += 1 - _energyActivities[i][j] / _energyResources[uncModel->getAltAllocSet(i)->getUResource(j)->getIndex()];
			currentPending = currentPending->getNext();
		}
		
		currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			IloNum tempDur = uncModel->getAltAllocSet(i)->getUActivity()->getIndicativeProcessingTime();
			_extendedMDur[i] = _criticities[i] * tempDur;
			currentPending = currentPending->getNext();
		}

		for(i = 0; i < nbProcessPlans; i++)
			_pendingDur[i] = 0;
		currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			j = processPlanIds[i];
			_pendingDur[j] += _extendedMDur[i];
			currentPending = currentPending->getNext();
		}
		
		IloNum tempPending = 0.0;
		for(i = 0 ; i < nbProcessPlans ; i++)
			tempPending += _pendingDur[i];
		for(i = 0 ; i < nbProcessPlans ; i++)
			_meanPendingDur[i] = (tempPending - _pendingDur[i]) / nbProcessPlans;

		IluHistogramI** processPlanHist = _selectionFrontier->getLastHistogramProcessPlan();

		for(i = 0 ; i < nbProcessPlans ; i++) {
			_endPExp[i] = processPlanHist[i]->getAverage() + _pendingDur[i];
			_tardiExp[i] = phis[i] * IloMax(_endPExp[i] - (IloNum)dueDates[i], 0.0);
		}

		ItemI* currentEligible = _eligible;
		while(0 != currentEligible) {
			i = currentEligible->getIndex();
			j = processPlanIds[i];
			_weights[j] = allocExp[i] + beta * _tardiExp[j];
			currentEligible = currentEligible->getNext();
		}

		currentEligible = _eligible;
		while(0 != currentEligible) {
		//	printf("A new priority is computed.\n");
			i = currentEligible->getIndex();
			j = processPlanIds[i];
			IluActivityI* uncAct = uncModel->getAltAllocSet(i)->getUActivity();
			IloNum priority = _weights[j] * IloExponent(-IloMax(0.0, (IloNum)dueDates[j] - currentTime - _pendingDur[j]) / (2 * _meanPendingDur[j])) / _pendingDur[j];
		//	printf("Activity %ld: priority = %.14f, due date = %ld\n", i, priority, dueDates[j]);
			IluPriorityHeap::Node* node = _nodes[i];
			_priorityHeap.changeKey(node, -priority);
			currentEligible = currentEligible->getNext();
		}
		_lastSelectedActivity = 0;
	}
	if(0 == _eligible){
	//	printf("New priorities are computed.\n");
		for(i = 0 ; i < nbTotalResources ; i++) {
			_energyResources[i] = 0.0;
		}
		ItemI* currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			for(j = 0; j < nbResources; j++)
				_energyResources[uncModel->getAltAllocSet(i)->getUResource(j)->getIndex()] += _energyActivities[i][j];
			currentPending = currentPending->getNext();
		}
		
	//	for(i = 0; i < nbTotalResources; i++) {
	//		printf("Resource %ld: energy = %.14f\n", i, _energyResources[i]);
	//	}

		for(i = 1; i < nbTotalActivities + 1; i++) {
			_criticities[i] = 1.0;
		}
		currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			for(j = 0; j < nbResources; j++)
				_criticities[i] += 1 - _energyActivities[i][j] / _energyResources[uncModel->getAltAllocSet(i)->getUResource(j)->getIndex()];
		//	printf("Activity %ld: criticity = %.14f\n", i, _criticities[i]);
			currentPending = currentPending->getNext();
		}

		currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			_extendedMDur[i] = _criticities[i] * uncModel->getAltAllocSet(i)->getUActivity()->getIndicativeProcessingTime();
		//	printf("Activity %ld: extended duration = %.14f\n", i, _extendedMDur[i]);
			currentPending = currentPending->getNext();
		}

		IluHistogramI** processPlanHist = _selectionFrontier->getLastHistogramProcessPlan();
		for(i = 0 ; i < nbProcessPlans ; i++) {
			_pendingDur[i] = 0.0;
		}
		currentPending = _pending;
		while(0 != currentPending) {
			i = currentPending->getIndex();
			j = processPlanIds[i];
			_pendingDur[j] += _extendedMDur[i];
			currentPending = currentPending->getNext();
		}
	//	for(i = 0 ; i < nbProcessPlans ; i++) {
	//		printf("Process plan %ld: pending duration = %.14f\n", i, _pendingDur[i]);
	//	}

		IloNum tempPending = 0.0;
		for(i = 0; i < nbProcessPlans; i++)
			tempPending += _pendingDur[i];
		for(i = 0; i < nbProcessPlans; i++) {
			_meanPendingDur[i] = (tempPending - _pendingDur[i]) / nbProcessPlans;
		//	printf("Process plan %ld: mean pending duration = %.14f\n", i, _meanPendingDur[i]);
		}
		for(i = 0; i < nbProcessPlans; i++) {
			_endPExp[i] = processPlanHist[i]->getAverage() + _pendingDur[i];
		//	printf("Process plan %ld: endPExp = %.14f\n", i, _endPExp[i]);
		}
		for(i = 0; i < nbProcessPlans; i++) {
			_tardiExp[i] = phis[i] * IloMax(_endPExp[i] - (IloNum)dueDates[i], 0.0);
		//	printf("Process plan %ld: expected tardiness = %.14f\n", i, _tardiExp[i]);
		}
		
		for(i = 0; i < nbProcessPlans; i++) {
			ItemI* currentPending = _pending;
			IloBool eligible = IloFalse;
			while((IloFalse == eligible)&&(0 != currentPending)) {
				if(i == processPlanIds[currentPending->getIndex()]) {
					eligible = IloTrue;
					ItemI* newEligible;
					if(0 != _recycling) {
						newEligible = _recycling;
						_recycling = _recycling->getNext();
						newEligible->setIndex(currentPending->getIndex());
						newEligible->setNext(_eligible);
						_eligible = newEligible;
					}
					else {
						newEligible = new (_solution.getEnv()) ItemI(currentPending->getIndex());
						newEligible->setNext(_eligible);
						_eligible = newEligible;					
					}
				}
				currentPending = currentPending->getNext();
			}
		}

		ItemI* currentEligible = _eligible;
		while(0 != currentEligible) {
			i = currentEligible->getIndex();
			j = processPlanIds[i];
			_weights[j] = allocExp[i] + beta * _tardiExp[j];
		//	printf("Process plan %ld: weight = %.14f\n", j, _weights[j]);
			currentEligible = currentEligible->getNext();
		}

		currentEligible = _eligible;
		while(0 != currentEligible) {
			i = currentEligible->getIndex();
			j = processPlanIds[i];
			IluActivityI* uncAct = uncModel->getAltAllocSet(i)->getUActivity();
			IloNum priority = _weights[j] * IloExponent(-IloMax(0.0, (IloNum)dueDates[j] - 0.0 - _pendingDur[j]) / (2 * _meanPendingDur[j])) / _pendingDur[j];
		//	printf("Activity %ld: priority = %.14f, due date = %ld\n", i, priority, dueDates[j]);
			IluPriorityHeap::Node* node = new IluPriorityHeap::Node(uncAct, -priority);
			_priorityHeap.insert(node);
			_nodes[i] = node;
			currentEligible = currentEligible->getNext();
		}
	}
}


void
IluProgressiveSchedulerI::updateSolution() {
	IluActivityI* uncAct = _selecSim->getActivity();
	if(0 != uncAct) {
		IloInt actIndex = uncAct->getIndex();
	//	printf("Activity %ld is added to the solution.\n", actIndex);
		_solution.add(*uncAct, IloRestoreAll);				// An activity is added to the solution.
		IluExtendedModelI* uncModel = _localSSolver->getUncModel();
		IloInt nbResources = uncModel->getAltAllocSet(1)->getNbAltURes();
		IloInt resourceIndex = _selecSim->getBestResourceId();
		IloInt i;
		IluUnaryResourceI* uncRes = 0;
		for(i = 0; i < nbResources; i++) {
			IluUnaryResourceI* currentUncRes = uncModel->getAltAllocSet(actIndex)->getUResource(i);
			if(currentUncRes->getIndex() == resourceIndex)
				uncRes = currentUncRes;
		}
		if(IloFalse == _solution.contains(*uncRes)) {
		//	printf("Resource %ld is added to the solution.\n", resourceIndex);
			_solution.add(*uncRes);
		}
		IloUnaryResource res = (IloUnaryResource)(*uncRes);
		IloResourceConstraint rCt = (*uncAct).requires(res);
		_solution.add(rCt, IloRestoreAll); // A resource constraint is added to the solution.
		// Precendence constraints have to be added by using _solution.setSuccessor(IloResourceConstraint sRCt1, IloResourceConstraint sRCt2).
		// This member function makes sRCt2 a successor of sRCt1 in the invoking scheduler solution.
		if(_solution.hasTeardownRC(res)) {
			IloResourceConstraint lastRCt = _solution.getTeardownRC(res);
			_solution.setNext(lastRCt, rCt);
			_solution.unsetTeardown(lastRCt);
			_solution.setTeardown(rCt);
		}
		else {
			_solution.setTeardown(rCt);
			_solution.setSetup(rCt);
		}
		_selecSim->setActivity(0);
		_selecSim->setBestResourceId(-1);
		_selecSim->setProcessPlanId(-1);
	}
}


ItemI*
IluProgressiveSchedulerI::getItem(IloInt index, ItemI* itemList) {
	ItemI* currentItem = itemList;
	ItemI* previousItem = 0;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == currentItem)
			stop = IloTrue;
		else {
			if(index == currentItem->getIndex())
				stop = IloTrue;
			else {
				previousItem = currentItem;
				currentItem = currentItem->getNext();
			}
		}
	}
	if(0 == previousItem)
		itemList = currentItem->getNext();
	else
		previousItem->setNext(currentItem->getNext());
	currentItem->setNext(0);
	return currentItem;
}


IloNum
IluProgressiveSchedulerI::computeEffectTardiCost() {
	IloNum effectTardiCost = 0.0;

	IluLocalSearchSolverI* lSSolver = getLocalSearchSolver();
	IluExtendedModelI* uncModel = lSSolver->getUncModel();
	IloInt* dueDates = uncModel->getDueDates();
	IloNum* phis = uncModel->getPhis();
	IloInt nbProcessPlans = uncModel->getNbProcessPlans();
	IluHistogramI** lastHistProcessPlan = _allocationFrontier->getLastHistogramProcessPlan();

	for(IloInt i = 0; i < nbProcessPlans; i++) {
		printf("Process plan %ld:\n", i);
		printf("\tEffective end time = %.14f\n", lastHistProcessPlan[i]->getValue(0));
		printf("\tDue date = %ld\n", dueDates[i]);
		printf("\tPhi = %.14f\n", phis[i]);
		IloNum tempEffectTardiCost = 0.0;
		if(lastHistProcessPlan[i]->getValue(0) > dueDates[i]) {
			tempEffectTardiCost = (lastHistProcessPlan[i]->getValue(0) - dueDates[i]) * phis[i];
			printf("\tTardiness cost = %.14f\n", tempEffectTardiCost);
		}
		effectTardiCost += tempEffectTardiCost;
	}
	return effectTardiCost;
}



/*

IluReactiveSchedulerI::IluReactiveSchedulerI(IloEnv env, IlcScheduler sched, IloRandom randGen, IloNum sensitivity, IloInt criterion)
		:   _sched(sched),
			_histP(new (env) IluHistogramI(env)),
			_normVarP(new (env) IluNormalVariableI(env, randGen, 0.0, 0.0, 0.0, 0.0)),
			_threshold(0),
			_sensitivity(sensitivity),
			_criterion(criterion) {
	_nbActivities = sched.getNumberOfActivities();
	_processingTimeMean = 0;
	for (IlcActivityIterator actite6(sched); actite6.ok(); ++actite6) {
		IluActivityI* uncActP = (IluActivityI*)(*actite6).getObject();
		_processingTimeMean += uncActP->getRandomVar()->getAverage();
	}
	_processingTimeMean /= _nbActivities;
}




void
IluReactiveSchedulerI::initPG(IloEnv env, IlcScheduler sched, IloNum currentTime) {
	IlcInt nbNodes = sched.getNumberOfActivities() + 1;
	IluSimulatorI* simP = getSimulator();
	if (0 != simP)
		delete simP;
	simP = new IluSimulatorI(env, startFrontier, endFrontier, nbNodes);
	
	for (IlcActivityIterator actIte(sched); actIte.ok(); ++actIte) {
		IlcFollowingActivityIterator fAct(*actIte);
		if (!fAct.ok())
			simP->addArc(((IluActivityI*)(*actIte).getObject())->getIndex(), 0); // A precedence constraint between
																			  // the activity end time and the makespan.
		for ( ; fAct.ok(); ++fAct) 
			simP->addArc(((IluActivityI*)(*actIte).getObject())->getIndex(), ((IluActivityI*)(*fAct).getObject())->getIndex());
					// A precedence constraint between 2 consecutive activities of a
					 // given job is added to the precedence graph.
	}

	for (IlcUnaryResourceIterator ite(sched); ite.ok(); ++ite) { // Precedence constraints are added to the precedence
		IlcUnaryResource res = *ite;                                 // graph due to the resource constraints.
		IloInt prev = -1;
		IlcActivity prevActivity;
		for (IlcResource::ResourceConstraintIterator itef(res, IlcResource::RankedFirst); itef.ok(); ++itef) {
			if (0 <= prev) {
				simP->addArc(prev, ((IluActivityI*)(*itef).getActivity().getObject())->getIndex());
// 				cout << prevActivity.getName() << "\t" << prev << "\t";
// 				cout << (*itef).getActivity().getName() << "\t";
// 				cout << ((IluActivityI*)(*itef).getActivity().getObject())->getIndex() << endl;
			}
			prev = ((IluActivityI*)(*itef).getActivity().getObject())->getIndex();
			prevActivity = (*itef).getActivity();
		}
		if (0 != res.getLastRankedLastRC().getObject()) {
			for (IlcResource::ResourceConstraintIterator itel(res, IlcResource::RankedLast, res.getLastRankedLastRC()) ; itel.ok() ; --itel) {  
                if (0 <= prev) {
					simP->addArc(prev, ((IluActivityI*)(*itel).getActivity().getObject())->getIndex());
// 					cout << prevActivity.getName() << "\t" << prev << "\t";
// 					cout << (*itel).getActivity().getName() << "\t";
// 					cout << ((IluActivityI*)(*itel).getActivity().getObject())->getIndex() << endl;
				}
				prev = ((IluActivityI*)(*itel).getActivity().getObject())->getIndex();
				prevActivity = (*itel).getActivity();
	        }
		}
	}
	
	simP->topologicalSort();
	simP->setDurationVariable(0, _normVarP); // This associates a normal variable with the node 0 associated with the makespan variable.


	for (IlcActivityIterator actIte2(sched); actIte2.ok(); ++actIte2) {
		simP->setDurationVariable(((IluActivityI*)(*actIte2).getObject())->getIndex(), ((IluActivityI*)(*actIte2).getObject())->getRandomVar());
		if (IlcIntMax > ((IluActivityI*)(*actIte2).getObject())->getEffecStartTime()) {
			simP->setStartTime(((IluActivityI*)(*actIte2).getObject())->getIndex(), ((IluActivityI*)(*actIte2).getObject())->getEffecStartTime());
//			printf("Call to setStartTime \n");
//			cout << ((IluActivityI*)(*actIte2).getObject())->getIndex() << "\t";
//			cout << ((IluActivityI*)(*actIte2).getObject())->getEffecStartTime() << endl;
		}
		if (IlcIntMax > ((IluActivityI*)(*actIte2).getObject())->getEffecEndTime()) {
			simP->setEndTime(((IluActivityI*)(*actIte2).getObject())->getIndex(), ((IluActivityI*)(*actIte2).getObject())->getEffecEndTime());
//			printf("Call to setEndTime \n");
//			cout << ((IluActivityI*)(*actIte2).getObject())->getIndex() << "\t";
//			cout << ((IluActivityI*)(*actIte2).getObject())->getEffecEndTime() << endl;
		}
	}


// HISTOGRAMS ARE ASSOCIATED WITH THE ACTIVIY END TIME VARIABLES
	
	simP->setHistogram(0, _histP);  // _histP is associated with the makespan variable whose index equals 0.
	_histP->clean();
	if(3 == _criterion)
		for (IlcActivityIterator actite3(sched); actite3.ok(); ++actite3) {
			IluActivityI* uncActP = (IluActivityI*)(*actite3).getObject();
			IluHistogramI* histP = uncActP->getHist();
			simP->setHistogram(uncActP->getIndex(),histP);
			histP->clean();
		}

	simP->simulate(currentTime, 1000);	// We generate a sample for getting the probability distribution of the makespan and
										// the end time variables of the activities.
//	_histP->display("curve.dat", 250);

	_threshold = _histP->getAverage();	// _threshold is used for checking the criteria 1 and 2
	if(3 == _criterion)
		for (IlcActivityIterator actite5(sched); actite5.ok(); ++actite5) {
			IluActivityI* uncActP = (IluActivityI*)(*actite5).getObject();
			IluHistogramI* histP = uncActP->getHist();
			uncActP->setExpIndicativeEndTime(histP->getAverage());
//			printf("Mean end time is %f\n", histP->getAverage());
		}

//	printf("At time %f avg. = %f and st. dev. = %f\n", now, _histP->getAverage(), _histP->getStandardDev());
	setSimulator(simP);
}



IlcBool
IluReactiveSchedulerI::reschedule(IlcScheduler sched) {
	IlcBool resched = IlcFalse; // If the criterion equals 0 it also holds.

	if((1 == _criterion)&&(_threshold/_sensitivity < (_histP->getAverage()))) // first criterion REACTIVE APPROACH
		resched = IlcTrue;
	
	if(2 == _criterion) {			// second criterion REACTIVE APPROACH
		IloNum delta = 0.0;
		delta = fabs(_threshold - _histP->getAverage());
		if(delta > _processingTimeMean/_sensitivity)
			resched = IlcTrue;
	}

	if(3 == _criterion) {
		IloNum delta = 0.0; // third criterion REACTIVE APPROACH
		IlcInt nbActivities = 0;
		for (IlcActivityIterator actite7(sched); actite7.ok(); ++actite7) {
			IluActivityI* uncActP = (IluActivityI*)(*actite7).getObject();
			IluHistogramI* histP = uncActP->getHist();
			if(uncActP->getExpIndicativeEndTime() != histP->getAverage()){
				delta += fabs(uncActP->getExpIndicativeEndTime() - histP->getAverage()); // fabs requires to include math.h.
//				printf("Standard deviation is %f\n", _histP->getStandardDev());
				nbActivities++;
			}
		}
		if(nbActivities > 0)
			delta /= nbActivities;
//		printf("delta = %f and threshold = %f\n", delta, _processingTimeMean/_sensitivity);
		if(delta > _processingTimeMean/_sensitivity)
			resched = IlcTrue;
	}

	if(4 == _criterion) {
		IloNum delta = 0.0;	// fourth criterion: it only assesses the schedule part already executed. REACTIVE APPROACH
		IlcInt nbActivities = 0;
		for (IlcActivityIterator actite6(sched); actite6.ok(); ++actite6) {
			IluActivityI* uncActP = (IluActivityI*)(*actite6).getObject();
			if((uncActP->getEffecStartTime() != uncActP->getIndicativeStartTime())&&(uncActP->getEffecStartTime() < IlcIntMax)) {
				delta += (IloNum)fabs(uncActP->getEffecStartTime() - uncActP->getIndicativeStartTime());
//				printf("Effective start time is %ld\nIndicative start time is %ld\n", uncActP->getEffecStartTime(), uncActP->getIndicativeStartTime());
//				printf("Delta is updated.\n");
				nbActivities++;
			}
		}
		if(nbActivities > 0)
			delta /= nbActivities;
//		printf("delta = %f and threshold = %f\n", delta, _processingTimeMean/_sensitivity);
		if(delta > _processingTimeMean/_sensitivity)
			resched = IlcTrue;
	}*/

/*	if(5 == _criterion) { // REACTIVE APPROACH
		IloNum delta = 0.0; // fifth criterion: it takes into account the standard deviation of each event.
		IlcInt nbActivities = 0;
		for (IlcActivityIterator actite7(sched); actite7.ok(); ++actite7) {
			IluActivityI* uncActP = (IluActivityI*)(*actite7).getObject();
			IluHistogramI* histP = uncActP->getHist();
			if((uncActP->getExpIndicativeEndTime() != histP->getAverage())&&(histP->getStandardDev() < 1000.0)) {
				delta += fabs(uncActP->getExpIndicativeEndTime() - histP->getAverage()); // fabs requires to include math.h.
				nbActivities++;
			}
				
		}
		if(nbActivities > 0)
			delta /= nbActivities;
//		printf("delta = %f and threshold = %f\n", delta, _processingTimeMean/_sensitivity);
		if(delta > _processingTimeMean/_sensitivity)
			resched = IlcTrue;
	}*/
//	if(IlcFalse == resched)
//		printf("The criterion is not verified.\n");
/*	return resched;
}*/
