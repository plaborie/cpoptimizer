// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------



// The file contains the IluInfoActivity class and the IluProcPlan class.


#ifndef __SIM_iluprocessplansimH
#define __SIM_iluprocessplansimH

#include <ilsched/iloscheduler.h>

////////////////////////////////////////////////////////////////////
//
// ASSOCIATING INFORMATION TO EACH ACTIVITY AND EACH PROCESS PLAN
//
////////////////////////////////////////////////////////////////////

class IluProcPlan {
private:
	IloInt				_procPlanIndex;					// _procPlanIndex is equal to the index of the process plan
	IlcStamp			_stamp;							// _stamp is used to decide when to update _cost in the evaluators for ranking resource constraints.
	IloNum				_betaPhi, _gamma, _omega, _phi;	// _betaPhi is equal to beta times the process plan _phi and _gamma is used to heuristally rank 
														// resource constraints.
	IloInt				_dueDate;
	IlcInt				_endMin;						// _endMin is the earliest end time of the process plan; it is computed after a first propagation.
	IlcFloat			_cost;
	IloActivityArray	_activities;
	char*				_name;

public:
	IluProcPlan()
		: _procPlanIndex(IloIntMax), _stamp(IlcIntMin), _betaPhi(IloInfinity), _gamma(IloInfinity), _omega(IloInfinity), _phi(IloInfinity), _dueDate(IloIntMax),
			_endMin(IloIntMax), _cost(-IlcInfinity), _activities(0), _name(0) {}
	IluProcPlan(IloEnv env,
				IloInt procPlanIndex,
				IlcStamp stamp,
				IloNum betaPhi,
				IloNum gamma,
				IloNum omega,
				IloNum phi,
				IloInt dueDate,
				IlcInt endMin,
				IlcFloat cost,
				IloActivityArray activities,
				char* name = 0)
		: _procPlanIndex(procPlanIndex), _stamp(stamp), _betaPhi(betaPhi), _gamma(gamma), _omega(omega), _phi(phi), _dueDate(dueDate), _endMin(endMin),
			_cost(cost), _activities(activities), _name(name) {
		char buffer[128];
		sprintf(buffer, "PP%ld", procPlanIndex);
		IloInt len = strlen(buffer);
		_name =  new(env) char[len+1];
		strcpy(_name, buffer);
	}
	~IluProcPlan(){}

	IloInt getProcPlanIndex() const { return _procPlanIndex; }
	IlcStamp getStamp() const { return _stamp; }
	IloNum getBetaPhi() const { return _betaPhi; }
	IloNum getGamma() const { return _gamma; }
	IloNum getOmega() const { return _omega; }
	IloNum getPhi() const { return _phi; }
	IloInt getDueDate() const { return _dueDate; }
	IlcInt getEndMin() const { return _endMin; }
	IlcFloat getCost() const { return _cost; }
	IloActivityArray getActivities() const { return _activities; }
	const char* getName() const { return _name;}

	void setProcPlanIndex(IloInt procPlanIndex) { _procPlanIndex = procPlanIndex; }
	void setStamp(IlcStamp stamp) { _stamp = stamp; }
	void setBetaPhi(IloNum betaPhi) { _betaPhi = betaPhi; }	
	void setGamma(IloNum gamma) { _gamma = gamma; }
	void setOmega(IloNum omega) { _omega = omega; }
	void setPhi(IloNum phi) { _phi = phi; }
	void setDueDate(IloInt dueDate) { _dueDate = dueDate; }
	void setEndMin(IlcInt endMin) { _endMin = endMin; }
	void setCost(IlcFloat cost) { _cost = cost; }
	void setActivities(IloActivityArray activities) { _activities = activities; }
};


class IluInfoActivity {
private:
	IloInt					_actIndex, _queue;			// _queue is equal to the process plan queue, i.e., the sum of the activity durations.
	IloInt*					_allocCosts;				// _allocCosts is an array containing allocation costs.
	IloResourceConstraint	_resCt;
	IluProcPlan*			_procPlan;
	
public:
	IluInfoActivity()
		: _actIndex(IloIntMax), _queue(IloIntMax), _allocCosts(0), _resCt(0), _procPlan(0) {}
	IluInfoActivity(IloInt actIndex, IloInt queue, IloInt* allocCosts, IloResourceConstraint resCt, IluProcPlan* procPlan)
		: _actIndex(actIndex), _queue(queue), _allocCosts(allocCosts), _resCt(resCt), _procPlan(procPlan) {}
	~IluInfoActivity() {}
	
	IloInt getActIndex() const { return _actIndex; }
	IloInt getQueue() const { return _queue; }
	IluProcPlan* getProcPlan() const { return _procPlan; }
	IloInt* getAllocCosts() const { return _allocCosts; }
	IloResourceConstraint getResCt() const { return _resCt; }
	
	void setActIndex(IloInt actIndex) { _actIndex = actIndex; }
	void setQueue(IloInt queue) { _queue = queue; }
	void setProcPlan(IluProcPlan* procPlan) { _procPlan = procPlan; }
	void setAllocCosts(IloInt* allocCosts) { _allocCosts = allocCosts; }
	void setResCt(IloResourceConstraint resCt) { _resCt = resCt; }
};


class IluResourceSet {
private:
	IlcStamp		_stamp;
	IlcFloatArray	_energies;

public:
	IluResourceSet()
		:_energies(0) {}

	~IluResourceSet(){}

	IlcStamp getStamp() const { return _stamp; }







};

#endif