// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
// THIS  CLASS  ALLOWS  US  TO  BUILD ACTIVITIES WHOSE PROCESSING TIMES ARE
// IMPRECISE. WE ASSUME AN ACTIVITY IS SUSPENDED WHEN THE RESOURCE, WHICH IS
// ALLOCATED TO IT, BREAKS DOWN AND THIS ACTIVITY RESUMES EXECUTION WHEN
// THE ALLOCATED RESOURCE IS REPAIRED.
// --------------------------------------------------------------------------
#ifndef __SIM_iluactsimH
#define __SIM_iluactsimH

#include <ilsolver/ilcerr.h>
#include <ilsched/iloscheduler.h>


#include "ilurandom.h"


class IluActivityI : public IloActivity {
private:
	IluRandomVariableI* _randVarP;	// _randVarP represents the probability distribution of the activity duration
	IloInt				_index;		// _index is the index of the activity (_index equals 1 to the total number of activities)
	IloBool				_executed;	// _executed is true when the activity has finished execution.
	IloNum				_indicativeProcessingTime, _expIndicativeStartTime;
									//_indicativeProcessingTime = indicative duration (mean duration)
									//_expIndicativeStartTime = expected indicative start time (simulated indicative schedule) REACTIVE APPROACH
	IlcInt				_effectiveStartTime, _effectiveEndTime, _effectiveProcessingTime, _indicativeStartTime;
	IluHistogramI*		_histP;		// _histP permits us to store the realizations of the end times

public:
	IluActivityI(): _randVarP(0) {}
	//IluActivityI(const IluActivityI& activity);
	IluActivityI(IloEnv const env,
				 const IloNum meanDur,
				 const IloNum sigmaDur,
				 const IloNum durBound,
				 IloRandom randGen,
				 IloInt index,
				 const char* name = 0);
	~IluActivityI() {}

	void setRandomVar(IluRandomVariableI* randVarP) { _randVarP = randVarP; }
	IluRandomVariableI* getRandomVar() { return _randVarP; }

	IloInt getIndex() const { return _index; }
	void setIndex(IloInt index) { _index = index; }

	IloBool isExecuted() const { return _executed; }
	void executed() { _executed = IloTrue; }

	IloNum getIndicativeProcessingTime() const { return _indicativeProcessingTime; }
	void setIndicativeProcessingTime(IloNum indicativeProcessingTime) { _indicativeProcessingTime = indicativeProcessingTime; }

	IlcInt getEffecProcessingTime() const { return _effectiveProcessingTime; }
	void setEffecProcessingTime(IlcInt effectiveProcessingTime) { _effectiveProcessingTime = effectiveProcessingTime; }

	IlcInt getEffecStartTime() const { return _effectiveStartTime; }
	void setEffecStartTime(IlcInt t) { _effectiveStartTime = t; }

	IlcInt getEffecEndTime() const { return _effectiveEndTime; }
	void setEffecEndTime(IlcInt t) { _effectiveEndTime = t; }

	IlcInt getIndicativeStartTime() const { return _indicativeStartTime; }
	void setIndicativeStartTime(IlcInt t) { _indicativeStartTime = t; }

	IluHistogramI* getHist() { return _histP; }

	IloNum getExpIndicativeStartTime() const { return _expIndicativeStartTime; }
	void setExpIndicativeStartTime(IloNum expIndicativeStartTime) { _expIndicativeStartTime = expIndicativeStartTime; }

};



#endif
