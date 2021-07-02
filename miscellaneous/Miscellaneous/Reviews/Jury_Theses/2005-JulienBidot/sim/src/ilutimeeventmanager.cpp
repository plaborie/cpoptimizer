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

#include <ilsim/ilutimeeventmanager.h>

ILOSTLBEGIN


IluTimeEventManagerI::IluTimeEventManagerI(IloEnv env,
										   IluDynamicSchedulerI* dynamicSchedP,
										   IloInt currTime,
										   IluRealizationI* realizationP,
										   IluExtendedModelI* modelP)
							:	_controlEStackP				(new IluEventStackI()),
								_contingEStackP				(new IluEventStackI()),
								_actStartRecyclingStackP	(new IluEventStackI()),
								_actEndRecyclingStackP		(new IluEventStackI()),
								_breakStartRecyclingStackP	(new IluEventStackI()),
								_breakEndRecyclingStackP	(new IluEventStackI()),
								_dynamicSchedP				(dynamicSchedP),
								_currTime					(currTime),
								_realizationP				(realizationP),
								_modelP						(modelP) {
	IluBreakdownStackI* stacks = realizationP->getBreakdownStacks();
	IluBreakdownI* breakdown;
	if(0 != stacks) {
		IloInt i;
		IloInt nbTotalResources = realizationP->getNbTotalResources();
		for(i = 0 ; i < nbTotalResources ; i++) {
			IluUnaryResourceI* unaryResP = _modelP->getUUnaryResource(i);
			breakdown = stacks[i].getFirst();
			IluResourceStartEventI* newResStartEventP = new (env) IluResourceStartEventI(breakdown->getStartTime(), unaryResP);
			addCtigEvent(newResStartEventP);
		//	printf("Resource %ld breaks down at %ld.\n", unaryResP->getIndex(), breakdown->getStartTime());
		}
	}
}



IluTimeEventManagerI::IluTimeEventManagerI(IloEnv env,
										   IluEventStackI* contingEventStackP,
										   IluEventStackI* controlEventStackP,
										   IloInt currTime,
										   IluDynamicSchedulerI* dynamicSchedP,
										   IluRealizationI* realizationP,
										   IluExtendedModelI* modelP)
										 : _actStartRecyclingStackP		(new IluEventStackI()),
										   _actEndRecyclingStackP		(new IluEventStackI()),
										   _breakStartRecyclingStackP	(new IluEventStackI()),
										   _breakEndRecyclingStackP		(new IluEventStackI()),
										   _dynamicSchedP				(dynamicSchedP),
										   _currTime					(currTime),
										   _realizationP				(realizationP),
										   _modelP						(modelP) {
	if (0 != contingEventStackP) {
		_contingEStackP = new IluEventStackI();
		_contingEStackP->setFirst(contingEventStackP->getFirst());
		_contingEStackP->setLast(contingEventStackP->getLast());
	}
	if (0 != controlEventStackP) {
		_controlEStackP = new IluEventStackI();
		_controlEStackP->setFirst(controlEventStackP->getFirst());
		_controlEStackP->setLast(controlEventStackP->getLast());
	}
}



void
IluTimeEventManagerI::addEvents(IluEventI** eventArray, IlcInt eventNumber) {
	IluEventI* newEventP;	
	for(IlcInt i = 0; i < eventNumber; i++) {
		newEventP = eventArray[i];
		_controlEStackP->insert(newEventP);
	}
}



void
IluTimeEventManagerI::updateTime() {
	IloInt cigTime = IloIntMax;
	_currTime = IloIntMax;
	if(0 != _controlEStackP->getFirst())
		_currTime = _controlEStackP->getFirst()->getTime();
	if(0 != _contingEStackP->getFirst())
		cigTime = _contingEStackP->getFirst()->getTime();
	if(cigTime < _currTime)
		_currTime = cigTime;
//	printf("The current time is %ld\n\n", _currTime);
}


IloBool
IluTimeEventManagerI::executableEvents() {
	IloBool temp = IloFalse;
	if(_contingEStackP->isEmpty() == IloFalse)
		temp = IloTrue;
	if(_controlEStackP->isEmpty() == IloFalse)
		temp = IloTrue;
		
	return temp;
}


IloInt
IluTimeEventManagerI::executeNext(IloInt now) {	
// Updating the current time _currTime i.e. the next event execution time 
	updateTime();
	while(_currTime == now) {
		IluEventI* firstEventP = 0;
		if(0 != _controlEStackP->getFirst())
			if(_controlEStackP->getFirst()->getTime() == _currTime) {
				firstEventP = _controlEStackP->getFirst();
				_controlEStackP->setFirst(_controlEStackP->getFirst()->getNext());
				if(0 == _controlEStackP->getFirst())
					_controlEStackP->setLast(0);
			}
		if((0 != _contingEStackP->getFirst()) && (0 == firstEventP))
			if(_contingEStackP->getFirst()->getTime() == _currTime) {
				firstEventP = _contingEStackP->getFirst();
				_contingEStackP->setFirst(_contingEStackP->getFirst()->getNext());
				if(0 == _contingEStackP->getFirst())
					_contingEStackP->setLast(0);

			}
		if(0 != firstEventP) {
//			printf("Execution of the event occurring at %ld\n", firstEventP->getTime());
			_dynamicSchedP->execute(firstEventP, this);
//			printf("Some events may be added to the lists now.\n");
			IluSimulatorI* allocSim = _dynamicSchedP->getSimulator();
			IluSimulatorI::NodeI* node = allocSim->getFirstNode();
			IloBool stop = IloFalse;
			while(IloFalse == stop) {
				if(0 == node)
					stop = IloTrue;
				else {
					if((IloFalse == node->isFrontier())&&(0 == node->getCurrentIndegree())) {
						if(0 != node->getResource()) {
							if(IloFalse == node->getResource()->isBrokenDown()) {
							//	printf("Node %ld start time: mean = %.14f and standard deviation = %.14f\n", node->getActivityId(), node->getHistogram()->getAverage(), node->getHistogram()->getStandardDev());
								IloInt nodeId = node->getActivityId();
					//			printf("Activity %ld is executable.\n", nodeId);
								IluActivityI*  uncAct = _modelP->getAltAllocSet(nodeId)->getUActivity();
								IluActStartEventI* eventP;
								if(0 == getActStartRecyclingStack()->getFirst())
									eventP = new (allocSim->getEnv()) IluActStartEventI((IlcInt)now, uncAct);
								else {
									eventP = (IluActStartEventI*)(getActStartRecyclingStack()->pop());
									eventP->setTime((IlcInt)now);
									eventP->setUncActivity(uncAct);
									eventP->setSuspended(IloFalse);
								}
								addCtrlEvent(eventP);
								stop = IloTrue;
							}
						}
						else {
				//			printf("Node %ld start time: mean = %.14f and standard deviation = %.14f\n", node->getActivityId(), node->getHistogram()->getAverage(), node->getHistogram()->getStandardDev());
							IloInt nodeId = node->getActivityId();
				//			printf("Activity %ld is executable.\n", nodeId);
							IluActivityI*  uncAct = _modelP->getAltAllocSet(nodeId)->getUActivity();
							IluActStartEventI* eventP;
							if(0 == getActStartRecyclingStack()->getFirst())
								eventP = new (allocSim->getEnv()) IluActStartEventI((IlcInt)now, uncAct);
							else {
								eventP = (IluActStartEventI*)(getActStartRecyclingStack()->pop());
								eventP->setTime((IlcInt)IloIntMin);
								eventP->setUncActivity(uncAct);
								eventP->setSuspended(IloFalse);
							}
							addCtrlEvent(eventP);
							stop = IloTrue;
						}						
					}
					node = node->getNext();
				}
			}
		}
		updateTime();
	}
	return _currTime;
}



IloBool
IluTimeEventManagerI::updateControlStack(IluSimulatorI* simP, IloNum time) {
// Some controllable events may be added to the list.
	IloBool updated = IloFalse;
	IluExtendedModelI* uncModel = _modelP;
	IluSimulatorI::NodeI* node = simP->getFirstNode();
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == node)
			stop = IloTrue;
		else {
			if((IloFalse == node->isFrontier())&&(0 == node->getCurrentIndegree())&&(0 != node->getHistogram())) {
			//	printf("Node %ld start time: mean = %.14f and standard deviation = %.14f\n", node->getActivityId(), node->getHistogram()->getAverage(), node->getHistogram()->getStandardDev());
				if(time == node->getHistogram()->getAverage()) {
					IloInt nodeId = node->getActivityId();
		//			printf("Activity %ld is executable.\n", nodeId);
					IluActivityI*  uncAct = uncModel->getAltAllocSet(nodeId)->getUActivity();
					IluActStartEventI* eventP;
					if(0 == getActStartRecyclingStack()->getFirst())
						eventP = new (simP->getEnv()) IluActStartEventI((IlcInt)time, uncAct);
					else {
						eventP = (IluActStartEventI*)(getActStartRecyclingStack()->pop());
						eventP->setTime((IlcInt)time);
						eventP->setUncActivity(uncAct);
						eventP->setSuspended(IloFalse);
					}
					addCtrlEvent(eventP);
					updated = IloTrue;
					stop = IloTrue;
				}
			}
			node = node->getNext();
		}
	}
	return updated;
}
