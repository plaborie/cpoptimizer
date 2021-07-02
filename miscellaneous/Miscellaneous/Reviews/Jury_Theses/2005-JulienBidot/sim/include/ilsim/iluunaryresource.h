// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains the IluUnaryResourceI class.

// --------------------------------------------------------------------------
// THIS  CLASS  ALLOWS  US  TO  BUILD UNARY RESOURCES THAT MAY BREAK DOWN.
// --------------------------------------------------------------------------




#ifndef __SIM_iluunaryresourcesimH
#define __SIM_iluunaryresourcesimH

#include <ilsolver/ilcerr.h>
#include <ilsched/iloscheduler.h>

#include "ilurandom.h"




class IluUnaryResourceI : public IloUnaryResource {
private:
	IluRandomVariableI* _interBreaksVarP, * _breaksDurationVarP;
		// _ interBreaksVarP represents the probability distribution of the duration between two consecutive breakdowns.
		// _ breaksDurationVarP represents the probability distribution of the breakdown duration.

	IloNum				_meanTimeBreak, _sigmaTimeBreak, _meanBreakDur, _sigmaBreakDur;
		// _meanTimeBreak is the mean time between 2 consecutive breakdowns.
		// _meanBreakDur is the mean duration of a breakdown.
		// _sigmaTimeBreak is the standard deviation of the time between 2 consecutive breakdowns.
		// _sigmaBreakDur is the standard deviation of the duration of a breakdown.
	IloInt				_index, _breakdownStartTime;	// _index is the index of the uncertain unary resource.
														// _breakdownStartTime is the date at which the resource broke down.
	IloBool				_brokenDown;					// _brokenDown indicates whether the resource is currently broken down or not.


public:
	IluUnaryResourceI(): _interBreaksVarP(0), _breaksDurationVarP(0), _breakdownStartTime(IloIntMax), _brokenDown(IloFalse)  {}
	//IluUnaryResourceI(const IluUnaryResourceI& resource);
	IluUnaryResourceI(const IloEnv env,
					  const IloNum meanTimeBreak,
					  const IloNum sigmaTimeBreak,
					  const IloNum timeBreakBound,
					  const IloNum meanBreakDur,
					  const IloNum sigmaBreakDur,
					  const IloNum breakDurBound,
					  IloRandom randGenerator,
					  IloInt index,
					  const char* name = 0);
	~IluUnaryResourceI(){}

	IloInt getIndex() const { return _index; }
	void setIndex(IloInt index) { _index = index; }

	IloNum getMeanTimeBreak() const { return _meanTimeBreak; }
	IloNum getSigmaTimeBreak() const { return _sigmaTimeBreak; }

	IloNum getMeanBreakDur() const { return _meanBreakDur; }
	IloNum getSigmaBreakDur() const { return _sigmaBreakDur; }

	void setInterBreaksVar(IluRandomVariableI* randVarP)
		{ _interBreaksVarP = randVarP; }
	void setBreaksDurationVar(IluRandomVariableI* randVarP)
		{ _breaksDurationVarP = randVarP; }
	IluRandomVariableI* getInterBreaksVar() { return _interBreaksVarP; }
	IluRandomVariableI* getBreaksDurationVar() { return _breaksDurationVarP; }

	void setBreakdownStartTime(IloInt breakdownStartTime) { _breakdownStartTime = breakdownStartTime; }
	IloInt getBreakdownStartTime() const { return _breakdownStartTime; }

	IloBool isBrokenDown() const { return _brokenDown; }
	void brokenDown() { _brokenDown = IloTrue; }
	void repaired() { _brokenDown = IloFalse; }
};



#endif