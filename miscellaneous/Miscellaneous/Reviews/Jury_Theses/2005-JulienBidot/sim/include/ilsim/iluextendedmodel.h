// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains the AltAllocSetI class and the IluExtendedModelI class.


#ifndef __SIM_iluextendedmodelsimH
#define __SIM_iluextendedmodelsimH

#include <ilconcert/ilomodel.h>
#include <ilconcert/ilorandom.h>
#include <ilsched/iloscheduler.h>
#include <ilsched/ilosolution.h>
#include "iluunaryresource.h"
#include "iluactivity.h"
#include "iluconstraints.h"


// --------------------------------------------------------------------------
// THIS   CLASS PERMITS US TO MODEL A SET OF ALTERNATIVE UNCERTAIN 
// RESOURCES ASSOCIATED WITH AN UNCERTAIN ACTIVITY.
// --------------------------------------------------------------------------


class AltAllocSetI {
	private:
		IluActivityI*		_uActivity;
		IluUnaryResourceI**	_altUResSet;		// _altUResSet is an array of size nbResources containing the alternative uncertain resources.
		IloInt				_nbAltURes;			// _nbAltURes is the number of alternative resources per activity.
	public:
		AltAllocSetI()
			: _uActivity(0),
			  _altUResSet(0),
			  _nbAltURes(0) {}
		AltAllocSetI(IluActivityI* uActivity, IluUnaryResourceI** altUResSet, IloInt nbAltURes)
			: _uActivity(uActivity),
			  _altUResSet(altUResSet),
			  _nbAltURes(nbAltURes) {}

		IluActivityI* getUActivity() const { return _uActivity; }
		IloInt getNbAltURes() const { return _nbAltURes; }
		IluUnaryResourceI* getUResource(IloInt index) { if((0 != _altUResSet)&&(index >= 0)&&(index < _nbAltURes)) return _altUResSet[index]; else return 0; }
};




// --------------------------------------------------------------------------
// THIS   CLASS PERMITS US TO MODEL A NON-DETERMINISTIC PROBLEM:
// UNCERTAIN ACTIVITIES AND RESOURCES.
// --------------------------------------------------------------------------



class IluExtendedModelI {	
private:
	IloModel			_model;				// global model
	IloNumVar			_allocCost, _tardiCost, _makespan, _globalCost;
	IloNum				_kMax, _beta;
	// _allocCost represents the total allocation cost and _tardCost
	// is the total weighted tardiness cost of the scheduling problem.
	// _makespan is the overall duration of the schedule.
	AltAllocSetI**		_altAllocSets;
	// _altAllocSets is an array of size nbTotalActivities + 1. It contains uncertain activities allocated to alternative uncertain resources.
	IloNum**			_allocCosts;		// _allocCosts is a matrix of size (nbTotalActivities + 1) * nbResources and contains all allocation costs.
	IloNum*				_allocExp;			// _allocExp is an array of size nbTotalActivities + 1 and contains mean allocation costs.
	IloInt*				_processPlanIds;	// _processPlanIds is an array of size nbTotalActivities that gives the process plan index of each activity.
	IloInt				_nbTotalActivities; // _nbTotalActivities is the total number of activities of the scheduling problem.
	IloInt				_nbResources;		// _nbResources is the number of alternative resources per activity.
	IloInt				_nbTotalResources;	// _nbTotalResources is the total number of resources of the scheduling problem.
	IloInt				_nbProcessPlans;	// _nbProcessPlans is the number of process plans of the scheduling problem.
	IluUnaryResourceI**	_resources;			// _resources is an array of size _nbTotalResources and contains all uncertain resources.
	IloInt*				_dueDates;			// _dueDates is an array of size _nbProcessPlans and contains all the due dates.
	IloNum*				_phis;				// _phis is an array of size _nbProcessPlans and contains all the tardiness slope coefficients.
public:
	IluExtendedModelI()
		: _kMax				(0.0),
		  _beta				(0.0),
		  _altAllocSets		(0),
		  _allocCosts		(0),
		  _allocExp			(0),
		  _processPlanIds	(0),
		  _nbTotalActivities(0),
		  _nbResources		(0),
		  _nbTotalResources	(0),
		  _nbProcessPlans	(0),
		  _resources		(0),
		  _dueDates			(0),
		  _phis				(0)	{}
	IluExtendedModelI(IloEnv env,
					  char* instanceName,
					  const IloNum mDurBound,
					  const IloNum mTimeBreakBound,
					  const IloNum mBreakDurBound,
					  IloRandom randGenerator);

	~IluExtendedModelI() {}
	void setModel(IloModel model) { _model = model; }
	IloModel getModel() const { return _model; }

	void setAllocCostVar(IloNumVar allocCost) { _allocCost = allocCost; }
	void setTardCostVar(IloNumVar tardiCost) { _tardiCost = tardiCost; }
	void setMakespanVar(IloNumVar makespan) { _makespan = makespan; }

	AltAllocSetI* getAltAllocSet(IloInt index) const { if((0 != _altAllocSets)&&(index >= 0)&&(index < _nbTotalActivities + 1)) return _altAllocSets[index]; else return 0; }

	IloInt* getProcessPlanIds() const { return _processPlanIds; }
	IloNum** getAllocCosts() const { return _allocCosts; }
	IloNum* getAllocExp() const { return _allocExp; }
	IloInt getNbTotalActivities() const { return _nbTotalActivities; }
	IloInt getNbResources() const { return _nbResources; }
	IloInt getNbTotalResources() const { return _nbTotalResources; }
	IloInt getNbProcessPlans() const { return _nbProcessPlans; }
	IloInt* getDueDates() const { return _dueDates; }
	IloNum* getPhis() const { return _phis; }

	IluUnaryResourceI* getUUnaryResource(IloInt index) const {if((0 != _resources)&&(index >= 0)&&(index < _nbTotalResources)) return _resources[index]; else return 0;  }

	IloNumVar getAllocCostVar() const { return _allocCost; }
	IloNumVar getTardiCostVar() const { return _tardiCost; }
	IloNumVar getMakespanVar() const { return _makespan; }
	IloNumVar getGlobalCostVar() const { return _globalCost; }

	IloNum getKMAx() const { return _kMax; }
	IloNum getBeta() const { return _beta; }
};

#endif
