// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilurandom.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/ilurealization.h>
/*
#if defined(ILOUSESTL)
#include <fstream>
#include <stdio>
#else
#include <fstream.h>
#include <stdio.h>
#endif
*/

ILOSTLBEGIN

void
IluBreakdownStackI::addBreakdown(IluBreakdownI* breakdownP) {
	if(0 == _first)
		_first = breakdownP;
	else {
		if(breakdownP->getStartTime() < _first->getStartTime()) {
			breakdownP->setNext(_first);
			_first = breakdownP;
		}
		else {
			IluBreakdownI* currentP = _first;
			IluBreakdownI* nextP = _first->getNext();
			IloBool added = IloFalse;
			while((0 != nextP)&&(IloFalse == added)) {
				if(breakdownP->getStartTime() > currentP->getStartTime()) {
					currentP = currentP->getNext();
					nextP = nextP->getNext();
				}
				else
					added = IloTrue;
			}
			breakdownP->setNext(nextP);
			currentP->setNext(breakdownP);
		}
	}
}


IloInt
IluBreakdownStackI::getDuration(IloInt breakdownStartTime) const {
	IloInt duration = IloIntMax;
	IluBreakdownI* breakdown = _first;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == breakdown)
			stop = IloTrue;
		else {
			if(breakdownStartTime == breakdown->getStartTime()) {
				duration = breakdown->getDuration();
				stop = IloTrue;
			}
			else
				breakdown = breakdown->getNext();
		}
	}
	return duration;
}


IloInt
IluBreakdownStackI::getNextStartTime(IloInt breakdownStartTime) const {
	IloInt start = IloIntMax;
	IluBreakdownI* breakdown = _first;
	IloBool stop = IloFalse;
	while(IloFalse == stop) {
		if(0 == breakdown)
			stop = IloTrue;
		else {
			if(breakdownStartTime == breakdown->getStartTime()) {
				if(0 != breakdown->getNext())
					start = breakdown->getNext()->getStartTime();
				stop = IloTrue;
			}
			else
				breakdown = breakdown->getNext();
		}
	}
	return start;
}


IluRealizationI::IluRealizationI(IloEnv env, IloInt* effectActDurations, IloInt nbTotalResources, IloInt nbTotalActivities)
	: _effectActDurations(effectActDurations),
	  _nbTotalResources(nbTotalResources),
	  _nbTotalActivities(nbTotalActivities) {
	_breakdownStacks = new (env) IluBreakdownStackI[nbTotalResources];
//	IloInt i;
//	for(i = 0; i < _nbTotalActivities + 1; i++)
//		printf("Activity %ld: duration = %ld\n", i, effectActDurations[i]);
}


 
void
IluRealizationI::addBreakdown(IloInt index, IluBreakdownI* breakdownP) {
	if(0 != _breakdownStacks)
		_breakdownStacks[index].addBreakdown(breakdownP);
}
