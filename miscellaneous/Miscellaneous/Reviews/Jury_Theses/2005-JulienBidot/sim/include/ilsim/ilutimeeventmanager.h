// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains the IluTimeEventManagerI class.

#ifndef __SIM_ilueventmanagersimH
#define __SIM_ilueventmanagersimH


#include "iludynamicscheduler.h"
#include "ilueventstack.h"
#include "ilurealization.h"
#include "iluextendedmodel.h"


// --------------------------------------------------------------------------
// THIS   CLASS ALLOWS US TO MANAGE THE CONTROLLABLE AND CONTINGENT EVENT
// STACKS. IT INDICATES THE DYNAMIC SCHEDULER dynamicSchedP WHAT EVENTS OCCUR
// AT currTime.
// --------------------------------------------------------------------------


class IluTimeEventManagerI {
private:
	IluEventStackI*			_controlEStackP, * _contingEStackP;
	IluEventStackI*			_actStartRecyclingStackP, * _actEndRecyclingStackP, * _breakStartRecyclingStackP, * _breakEndRecyclingStackP;
	// _controlEStackP is a stack of controllable events (typically start times),
	// _contingEStackP is a stack of contingent events (typically breakdown start times), and
	// _actStartRecyclingStackP, _actEndRecyclingStackP, _breakStartRecyclingStackP, _breakEndRecyclingStackP are stacks of recycled events.
	IloInt					_currTime;						// _currTime is the current time.
	IluDynamicSchedulerI*	_dynamicSchedP;
	IluRealizationI*		_realizationP;
	IluExtendedModelI*		_modelP;

public:
	IluTimeEventManagerI()
		: _controlEStackP				(0),
		  _contingEStackP				(0),
		  _actStartRecyclingStackP		(0),
  		  _actEndRecyclingStackP		(0),
		  _breakStartRecyclingStackP	(0),
		  _breakEndRecyclingStackP		(0),
		  _dynamicSchedP				(0),
		  _currTime						(IloIntMax),
		  _realizationP					(0),
		  _modelP						(0) {}
	IluTimeEventManagerI(IloEnv env, IluDynamicSchedulerI* dynamicSchedP, IloInt currTime, IluRealizationI* realizationP, IluExtendedModelI* modelP);
	IluTimeEventManagerI(IloEnv env,
						 IluEventStackI* contingEventStackP,
						 IluEventStackI* controlEventStackP,
						 IloInt currTime,
						 IluDynamicSchedulerI* dynamicSchedP,
						 IluRealizationI* realizationP,
						 IluExtendedModelI* modelP);

	~IluTimeEventManagerI() {}

	void addEvents(IluEventI**, IlcInt);
	void addCtrlEvent(IluEventI* newEventP) { _controlEStackP->insert(newEventP); }
	void addCtigEvent(IluEventI* newEventP) { _contingEStackP->insert(newEventP); }

	IloInt executeNext(IloInt);

	void updateTime();

	IloBool executableEvents();
	
	IluEventStackI* getCtlStack() const { return _controlEStackP; }
	IluEventStackI* getCigStack() const { return _contingEStackP; }
	IluEventStackI* getActStartRecyclingStack() const { return _actStartRecyclingStackP; }
	IluEventStackI* getActEndRecyclingStack() const { return _actEndRecyclingStackP; }
	IluEventStackI* getBreakStartRecyclingStack() const { return _breakStartRecyclingStackP; }
	IluEventStackI* getBreakEndRecyclingStack() const { return _breakEndRecyclingStackP; }

	void setTime(IloInt currTime) { _currTime = currTime; }
	IloInt getTime() const { return _currTime; }

	void setDynamicScheduler(IluDynamicSchedulerI* dynamicSchedP) { _dynamicSchedP = dynamicSchedP; }
	IluDynamicSchedulerI* getDynamicScheduler() const { return _dynamicSchedP; }

	void setRealization(IluRealizationI* realizationP) { _realizationP = realizationP; }
	IluRealizationI* getRealization() const { return _realizationP;}

	void setUncModel(IluExtendedModelI* modelP) { _modelP = modelP; }
	IluExtendedModelI* getUncModel() const { return _modelP;}

	IloBool updateControlStack(IluSimulatorI* simP, IloNum time);
};

#endif
