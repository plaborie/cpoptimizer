// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains the IluBreakDownI class, the IluBreakdownStackI class,
// and the IluRealizationI class.

#ifndef __SIM_ilurealizationsimH
#define __SIM_ilurealizationsimH

#include <ilconcert/ilomodel.h>

// --------------------------------------------------------------------------
// THIS   CLASS ALLOWS US TO STORE THE START TIME AND THE DURATION OF A
// BREAKDOWN.
// --------------------------------------------------------------------------

class IluBreakdownI {
private:
	IloInt			_breakdownStartTime, _breakdownDuration;
	IluBreakdownI*	_nextBreakdown;


public:
	IluBreakdownI(): _nextBreakdown(0) {}
	IluBreakdownI(IloInt breakdownStartTime, IloInt breakdownDuration): _breakdownStartTime(breakdownStartTime),
																		_breakdownDuration(breakdownDuration),
																		_nextBreakdown(0) {}
	~IluBreakdownI() {}

	void setStartTime(IloInt breakdownStartTime) { _breakdownStartTime = breakdownStartTime; }
	IloInt getStartTime() const { return _breakdownStartTime; }

	void setDuration(IloInt breakdownDuration) { _breakdownDuration = breakdownDuration; }
	IloInt getDuration() const { return _breakdownDuration; }

	void setNext(IluBreakdownI* nextBreakdown) { _nextBreakdown = nextBreakdown; }
	IluBreakdownI* getNext() const { return _nextBreakdown; }
};



// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO MANAGE A STACK OF BREAKDOWNS.
// --------------------------------------------------------------------------

class IluBreakdownStackI {
private:
	IluBreakdownI* _first;

public:
	IluBreakdownStackI(): _first(0) {}
	IluBreakdownStackI(IluBreakdownI* breakdownP): _first(breakdownP) {}

	~IluBreakdownStackI() {}

	void addBreakdown(IluBreakdownI* breakdownP);

	void setFirst(IluBreakdownI* first) { _first = first; }
	IluBreakdownI* getFirst() const { return _first; }

	IloInt getDuration(IloInt breakdownStartTime) const;
	IloInt getNextStartTime(IloInt breakdownStartTime) const;
};



// --------------------------------------------------------------------------
// THIS   CLASS ALLOWS US TO STORE THE EFFECTIVE VALUES OF THE RANDOM
// VARIABLES, I.E. ACTIVITY DURATIONS, BREAKDOWN START TIMES, AND BREAKDOWN
// DURATIONS.
// --------------------------------------------------------------------------

class IluRealizationI {
private:
	IloInt*				_effectActDurations;			// An array of size nbTotalActivities + 1 containing all effective activity durations
	IluBreakdownStackI* _breakdownStacks;
	IloInt				_nbTotalResources, _nbTotalActivities;


public:
	IluRealizationI(): _effectActDurations(0), _breakdownStacks(0) {}
	IluRealizationI(IloEnv env, IloInt* effectActDurations, IloInt nbTotalResources, IloInt nbTotalActivities);
	
	~IluRealizationI() {}

	void setBreakdownStacks(IluBreakdownStackI* breakdownStacks) { _breakdownStacks = breakdownStacks; }
	IluBreakdownStackI* getBreakdownStacks() const { return _breakdownStacks; }

	void addBreakdown(IloInt index, IluBreakdownI* breakdownP);

	void setNbTotalResources(IloInt nbTotalResources) { _nbTotalResources = nbTotalResources; }
	IloInt getNbTotalResources() const { return _nbTotalResources; }

	void setNbTotalActivities(IloInt nbTotalActivities) { _nbTotalActivities = nbTotalActivities; }
	IloInt getNbTotalActivities() const { return _nbTotalActivities; }

	IloInt getEffecActDuration(IloInt i) const { if(0 != _effectActDurations) return _effectActDurations[i]; else return IloIntMax; }
};




#endif