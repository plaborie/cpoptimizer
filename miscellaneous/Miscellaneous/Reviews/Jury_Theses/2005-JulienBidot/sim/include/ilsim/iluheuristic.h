// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IluHeuristic class.


#ifndef __SIM_iluheuristicsimH
#define __SIM_iluheuristicsimH



#include <ilsched/ilolnsgoals.h>
#include <ilsolver/iimmeta.h>
#include <ilsolver/iimls.h>
#include<ilsim/ilurandom.h>




IloEvaluator<IlcAltResConstraint> IlcMyAssignAltRCNbPossibleEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAssignAltRCMinCostEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAssignAltRCMaxCostEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAssignAltRCDeltaCostEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAssignAltRCDispersionEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAltResConstraintRandomEvaluator(IloMemoryManager memManager, IloRandom randGenerator);
IloEvaluator<IlcAltResConstraint> IlcMyAltResConstraintMyRandomEvaluator(IloMemoryManager memManager, MyRandom* randGenerator);
IloEvaluator<IlcAltResConstraint> IlcMyAltResConstraintAddressEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcAltResConstraint> IlcMyAltRCConstantEvaluator(IloMemoryManager memManager);

IloEvaluator<IlcResource> IlcMyAssignAltResourceEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResource> IlcMyResourceDueDateEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResource> IlcMyResourceRefDateEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResource> IlcMyResourceRandomEvaluator(IloMemoryManager memManager, IloRandom randGenerator);
IloEvaluator<IlcResource> IlcMyResourceMyRandomEvaluator(IloMemoryManager memManager, MyRandom* randGenerator);
IloEvaluator<IlcResource> IlcMyResourceAddressEvaluator(IloMemoryManager memManager);

IloEvaluator<IlcResourceConstraint> IlcMyProcPlanCostDueDateEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyProcPlanCostRefDateEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyNothingResourceConstraintEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintStartMinEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintEndMaxEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintDurationMinEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintAddressEvaluator(IloMemoryManager memManager);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintRandomEvaluator(IloMemoryManager memManager, IloRandom randGenerator);
IloEvaluator<IlcResourceConstraint> IlcMyResourceConstraintMyRandomEvaluator(IloMemoryManager memManager, MyRandom* randGenerator);

IloEvaluator<IloActivityArray> IloMyGlobalCostPPEvaluator(IloMemoryManager memManager);
IloEvaluator<IloActivityArray> IloMyGlobalDeltaCostPPEvaluator(IloMemoryManager memManager);
IloEvaluator<IloActivityArray> IloMyGainGlobalCostPPEvaluator(IloMemoryManager memManager);
IloEvaluator<IloActivityArray> IloMyTardiCostPPEvaluator(IloMemoryManager memManager);
IloEvaluator<IloActivityArray> IloMyRandomPPEvaluator(IloMemoryManager memManager);
IloEvaluator<IloActivityArray> IloMyGainTardiCostPPEvaluator(IloMemoryManager memManager);

IloTranslator<IlcResourceConstraint, IlcActivity> IloMyRCtTranslator(IloMemoryManager memManager);

IloSelector<IlcResource, IlcAltResConstraint> IloMyAllocCostResourceInAltResCtSelector(IloMemoryManager memManager, IloRandom randGenerator);
IloSelector<IlcResource, IlcAltResConstraint> IloMyAllocCostResourceInAltResCtMySelector(IloMemoryManager memManager, MyRandom* randGenerator);

//IloSelector<IlcResource, IlcAltResConstraint> IloMyLoadResourceInAltResCtMySelector(IloMemoryManager memManager, MyRandom* randGenerator);




class IluHeuristic {
private:
	IloPredicate<IlcAltResConstraint>				_assignAltRCPredicate;
	IloPredicate<IlcResource>						_assignAltRPredicate, _rankResPredicate;
	IloPredicate<IlcResourceConstraint>				_rankRCPredicate;
	IloPredicate<IlcActivity>						_setTimesActPredicate;
	IloComparator<IlcAltResConstraint>				_assignAltRCComparator;
	IloComparator<IlcResource>						_assignAltRComparator, _rankResComparator;
	IloComparator<IlcResourceConstraint>			_rankRCComparator;
	IloComparator<IlcActivity>						_setTimesActComparator;
	IloSelector<IlcAltResConstraint, IlcAltResSet>	_assignAltRCSelector;
	IloSelector<IlcResource, IlcAltResConstraint>	_assignAltRSelector;
	IloSelector<IlcResource, IlcSchedule>			_rankResSelector;
	IloSelector<IlcResourceConstraint, IlcResource>	_rankRCSelector;
	IloSelector<IlcActivity, IlcSchedule>			_setTimesActSelector;

public:
	IluHeuristic()
				 :_assignAltRComparator(0),
				  _assignAltRPredicate(0),
				  _assignAltRCPredicate(0),
				  _setTimesActPredicate(0),
				  _assignAltRCComparator(0),
				  _rankResPredicate(0),
				  _rankResComparator(0),
				  _rankRCPredicate(0),
				  _rankRCComparator(0),
				  _assignAltRCSelector(0),
				  _assignAltRSelector(0),
				  _rankResSelector(0),
				  _rankRCSelector(0),
				  _setTimesActSelector(0) {}
					
	~IluHeuristic() {}

	void setAssignAltRCPredicate(IloPredicate<IlcAltResConstraint> assignAltRCPredicate) { _assignAltRCPredicate = assignAltRCPredicate; }
	void setAssignAltRPredicate(IloPredicate<IlcResource> assignAltRPredicate) { _assignAltRPredicate = assignAltRPredicate; }
	void setRankResPredicate(IloPredicate<IlcResource> rankResPredicate) { _rankResPredicate = rankResPredicate; }
	void setRankRCPredicate(IloPredicate<IlcResourceConstraint> rankRCPredicate) { _rankRCPredicate = rankRCPredicate; }
	void setSetTimesActPredicate(IloPredicate<IlcActivity> setTimesActPredicate) { _setTimesActPredicate = setTimesActPredicate; }
	
	void setAssignAltRCComparator(IloComparator<IlcAltResConstraint> assignAltRCComparator) { _assignAltRCComparator = assignAltRCComparator; }
	void setAssignAltRComparator(IloComparator<IlcResource> assignAltRComparator) { _assignAltRComparator = assignAltRComparator; }
	void setRankResComparator(IloComparator<IlcResource> rankResComparator) { _rankResComparator = rankResComparator; }
	void setRankRCComparator(IloComparator<IlcResourceConstraint> rankRCComparator) { _rankRCComparator = rankRCComparator; }
	void setSetTimesActComparator(IloComparator<IlcActivity> setTimesActComparator) { _setTimesActComparator = setTimesActComparator; }

	void setAssignAltRCSelector(IloSelector<IlcAltResConstraint, IlcAltResSet> assignAltRCSelector) { _assignAltRCSelector = assignAltRCSelector; }
	void setAssignAltRSelector(IloSelector<IlcResource, IlcAltResConstraint> assignAltRSelector) { _assignAltRSelector = assignAltRSelector; }
	void setRankResSelector(IloSelector<IlcResource, IlcSchedule> rankResSelector) { _rankResSelector = rankResSelector; }
	void setRankRCSelector(IloSelector<IlcResourceConstraint, IlcResource> rankRCSelector) { _rankRCSelector = rankRCSelector; }
	void setSetTimesActSelector(IloSelector<IlcActivity, IlcSchedule> setTimesActSelector) { _setTimesActSelector = setTimesActSelector; }
	
	IloPredicate<IlcAltResConstraint> getAssignAltRCPredicate() const { return _assignAltRCPredicate; }
	IloPredicate<IlcResource> getAssignAltRPredicate() const { return _assignAltRPredicate; }
	IloPredicate<IlcResource> getRankResPredicate() const { return _rankResPredicate; }
	IloPredicate<IlcResourceConstraint> getRankRCPredicate() const { return _rankRCPredicate; }
	IloPredicate<IlcActivity> getSetTimesActPredicate() const { return _setTimesActPredicate; }

	IloComparator<IlcAltResConstraint> getAssignAltRCComparator() const { return _assignAltRCComparator; }
	IloComparator<IlcResource> getAssignAltRComparator() const { return _assignAltRComparator; }
	IloComparator<IlcResource> getRankResComparator() const { return _rankResComparator; }
	IloComparator<IlcResourceConstraint> getRankRCComparator() const { return _rankRCComparator; }
	IloComparator<IlcActivity> getSetTimesActComparator() const { return _setTimesActComparator; }

	IloSelector<IlcAltResConstraint, IlcAltResSet> getAssignAltRCSelector() const { return _assignAltRCSelector; }
	IloSelector<IlcResource, IlcAltResConstraint> getAssignAltRSelector() const { return _assignAltRSelector; }
	IloSelector<IlcResource, IlcSchedule> getRankResSelector() const { return _rankResSelector; }
	IloSelector<IlcResourceConstraint, IlcResource> getRankRCSelector() const { return _rankRCSelector; }
	IloSelector<IlcActivity, IlcSchedule> getSetTimesActSelector() const { return _setTimesActSelector; }
};


IlcGoal IlcMyInstantiateIntVar(IlcManager manager, IlcIntVar var);
IlcGoal IlcMyInstantiateFloatVar(IlcManager manager, IlcFloatVar var);
IloGoal IloMyInstantiate(IloEnv env, IloNumVar var);

IloGoal IloMyMinimizeIntVar(IloEnv env, IloNumVar var, IloInt& val);
IloGoal IloMyMinimizeFloatVar(IloEnv env, IloNumVar var, IloNum& val);

IloGoal IloMyAssignAlternativeAndRank(IloEnv env, IluHeuristic heuristic);

IloGoal IloMyAssignAlternativeAndSetTimes(IloEnv env, IluHeuristic heuristic);

IlcGoal IlcPrintIntLowerBound(IlcManager manager, IlcIntVar var);
IlcGoal IlcPrintFloatLowerBound(IlcManager manager, IlcFloatVar var);
IloGoal IloPrintLowerBound(IloEnv env, IloNumVar var);

IloGoal IloProcPlanEndMin(IloEnv env, IloArray<IloActivityArray> actArrays);

IlcGoal IlcPrintIntSolution(IlcManager manager, IlcIntVar var);
IlcGoal IlcPrintFloatSolution(IlcManager manager, IlcFloatVar var);
IloGoal IloPrintSolution(IloEnv env, IloNumVar var);

IloGoal IloMyPrintStartSubGoal(IloEnv env);
IloGoal IloMyPrintEndSubGoal(IloEnv env);

IlcGoal IlcMyRank(IlcSchedule schedule, IloComparator<IlcResourceConstraint> rankRCComparator);

/*
class IluMyStamp {
private:
	IlcStamp _stamp;

public:
	IluMyStamp(): _stamp(0) {}
	~IluMyStamp() {}
	void setStamp(IlcStamp stamp) { _stamp = stamp; }
	IlcStamp getStamp() const { return _stamp; }
};*/


#endif