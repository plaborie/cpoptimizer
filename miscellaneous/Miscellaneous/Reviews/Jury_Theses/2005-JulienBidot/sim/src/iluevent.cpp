// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluevent.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/iluevent.h>
#include <ilsim/ilutimeeventmanager.h>


//void IluFailEvent::execute(IluDynamicScheduler*, IluTimeEventManager*) {}


void IluActStartEventI::execute(IluDynamicSchedulerI* dynamicSchedP, IluTimeEventManagerI* tEMgrP) {
	IluRealizationI* realizationP = tEMgrP->getRealization();
//	cout << getUncActivity()->getName() << " starts at time " << getTime() << "." << endl << endl;
	IloInt actIndex = getUncActivity()->getIndex();
	IlcInt duration = (IlcInt)(realizationP->getEffecActDuration(actIndex));
	getUncActivity()->setEffecProcessingTime(duration);

	IloEnv env = getUncActivity()->getEnv();

//	cout << "effective processing time: " << duration << endl;
	IluActEndEventI* newEventP;
	if(0 == tEMgrP->getActEndRecyclingStack()->getFirst())
		newEventP = new (env) IluActEndEventI(getTime() + duration, getUncActivity());
	else {
		newEventP = (IluActEndEventI*)(tEMgrP->getActEndRecyclingStack()->pop());
		newEventP->setTime(getTime() + duration);
		newEventP->setUncActivity(getUncActivity());
		newEventP->setSuspended(IloFalse);
	}
	tEMgrP->addCtigEvent(newEventP);

// The precedence graph is updated.
	dynamicSchedP->getSimulator()->setStartTime(actIndex, getTime());

	getUncActivity()->setEffecStartTime(getTime());
	printf("Activity %ld: effective start time is %d\n", actIndex, getUncActivity()->getEffecStartTime());
	
// This activity start time event is recycled.
	tEMgrP->getActStartRecyclingStack()->push(this);

// Effective allocation and allocation cost are updated.
	IloInt curResIndex = dynamicSchedP->getCurAllocation(actIndex);
	printf("This activity is associated with resource %ld.\n", curResIndex);
	Allocation* newEffectAllocation = new (env) Allocation(actIndex);
	dynamicSchedP->setEffectAllocation(newEffectAllocation, curResIndex);
	IloNum** allocCosts = tEMgrP->getUncModel()->getAllocCosts();
	dynamicSchedP->addEffectAllocCost(allocCosts[actIndex][curResIndex]);
}


void IluActEndEventI::execute(IluDynamicSchedulerI* dynamicSchedP, IluTimeEventManagerI* tEMgrP) {
	IluRealizationI* realizationP = tEMgrP->getRealization();
//	cout << "\t\t\t\t\t\t\t" << getUncActivity()->getName();
//	cout << " ends at time " << getTime() << endl << endl;
//	cout << ". The effective duration is ";
//	cout << getTime() - getUncActivity()->getEffecStartTime() << "." << endl << endl;

	IloInt actIndex = getUncActivity()->getIndex();

// The precedence graph is updated.
	dynamicSchedP->getSimulator()->setEndTime(actIndex, getTime());

	getUncActivity()->setEffecEndTime(getTime());
	getUncActivity()->executed();
	printf("\t\t\t\t\t\t\tActivity %ld: effective end time is %d\n", actIndex, getUncActivity()->getEffecEndTime());
	

// This activity end time event is recycled.
	tEMgrP->getActEndRecyclingStack()->push(this);
}


void IluResourceStartEventI::execute(IluDynamicSchedulerI* dynamicSchedP, IluTimeEventManagerI* tEMgrP) {
	IluRealizationI* realizationP = tEMgrP->getRealization();
	IluUnaryResourceI* uncRes = getUncUnaryResource();
	IloInt resIndex = uncRes->getIndex();
	printf("Machine %ld is broken down at time %ld.\n", resIndex, getTime());
	uncRes->setBreakdownStartTime(getTime());
	uncRes->brokenDown();
// If an activity is currently executed, then its execution is suspended.
	IluEventStackI* cigStack = tEMgrP->getCigStack();
	IloInt actIndex = dynamicSchedP->getSimulator()->getResource(resIndex)->getExecActId();
	if(actIndex < IloIntMax) {
		IluEventI* event = cigStack->getFirst();
		IluEventI* previousEvent = 0;
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == event)
				stop = IloTrue;
			else {
				IluActEndEventI* actEndEvent = (IluActEndEventI*)(event);
				IluActivityI* uncAct = actEndEvent->getUncActivity();
				if(uncAct != 0) {
					if(actIndex == uncAct->getIndex()) {
						event->setTime(IloIntMax);
						event->setSuspended(IloTrue);
						if(0 == previousEvent)
							cigStack->pop();
						else {
							previousEvent->setNext(event->getNext());
							event->setNext(0);
							if(event == cigStack->getLast())
								cigStack->setLast(previousEvent);
						}
						cigStack->append(event);
						stop = IloTrue;
					}
				}
				previousEvent = event;
				event = event->getNext();
			}
		}
	}
// The precedence graph is updated.
	dynamicSchedP->getSimulator()->setBreakdownStartTime(resIndex, getTime());
// A breakdown end time event is created.
	IluBreakdownStackI* stacks = realizationP->getBreakdownStacks();
	IlcInt breakdownEndTime = (IlcInt)(getTime() + stacks[resIndex].getDuration(getTime()));
	if(breakdownEndTime < IlcIntMax) {
		IluResourceEndEventI* newEndEventP;
		if(0 == tEMgrP->getBreakEndRecyclingStack()->getFirst())
			newEndEventP = new (uncRes->getEnv()) IluResourceEndEventI(breakdownEndTime, uncRes);
		else {
			newEndEventP = (IluResourceEndEventI*)(tEMgrP->getBreakEndRecyclingStack()->pop());
			newEndEventP->setTime(breakdownEndTime);
			newEndEventP->setUncUnaryResource(uncRes);
			newEndEventP->setSuspended(IloFalse);
		}
		tEMgrP->addCtigEvent(newEndEventP);
	}
// This breakdown start time event is recycled.
	tEMgrP->getBreakStartRecyclingStack()->push(this);
// A breakdown start time event is created.
	IlcInt breakdownStartTime = (IlcInt)(stacks[resIndex].getNextStartTime(getTime()));
	if(breakdownStartTime < IlcIntMax) {
		IluResourceStartEventI* newStartEventP = (IluResourceStartEventI*)(tEMgrP->getBreakStartRecyclingStack()->pop());
		newStartEventP->setTime(breakdownStartTime);
		newStartEventP->setUncUnaryResource(uncRes);
		newStartEventP->setSuspended(IloFalse);
		tEMgrP->addCtigEvent(newStartEventP);
	}
}


void IluResourceEndEventI::execute(IluDynamicSchedulerI* dynamicSchedP, IluTimeEventManagerI*tEMgrP) {
	IluRealizationI* realizationP = tEMgrP->getRealization();
	IluUnaryResourceI* uncRes = getUncUnaryResource();
	IloInt resIndex = uncRes->getIndex();
	printf("\t\t\t\t\t\t\tMachine %ld is repaired at time %ld.\n", resIndex, getTime());
	uncRes->addBreak(uncRes->getBreakdownStartTime(), getTime());
	uncRes->repaired();
	
// If an activity allocated to this resource was suspended, then it now resumes execution.
	IluEventStackI* cigStack = tEMgrP->getCigStack();
	IloInt actIndex = dynamicSchedP->getSimulator()->getResource(resIndex)->getExecActId();
	if(actIndex < IloIntMax) {
		IluEventI* event = cigStack->getFirst();
		IluEventI* previousEvent = 0;
		IloBool stop = IloFalse;
		while(IloFalse == stop) {
			if(0 == event)
				stop = IloTrue;
			else {
				IluActEndEventI* actEndEvent = (IluActEndEventI*)(event);
				IluActivityI* uncAct = actEndEvent->getUncActivity();
				if(uncAct != 0) {
					if((actIndex == uncAct->getIndex())&&(actEndEvent->isSuspended())) {
						event->setSuspended(IloFalse);
						uncAct->setEffecProcessingTime(uncAct->getEffecProcessingTime() + IlcInt(getTime()) - (IlcInt)(uncRes->getBreakdownStartTime()));
						uncAct->setEffecEndTime(uncAct->getEffecStartTime() + uncAct->getEffecProcessingTime());
						event->setTime((IloInt)(uncAct->getEffecEndTime()));
						if(0 == previousEvent)
							cigStack->pop();
						else {
							previousEvent->setNext(event->getNext());
							event->setNext(0);
							if(event == cigStack->getLast())
								cigStack->setLast(previousEvent);
						}
						cigStack->insert(event);
					}
				}
				previousEvent = event;
				event = event->getNext();
			}
		}
	}
// The precedence graph is updated.
	dynamicSchedP->getSimulator()->setBreakdownEndTime(resIndex, getTime());
	uncRes->setBreakdownStartTime(IloIntMax);
// This breakdown end time event is recycled.
	tEMgrP->getBreakEndRecyclingStack()->push(this);
}