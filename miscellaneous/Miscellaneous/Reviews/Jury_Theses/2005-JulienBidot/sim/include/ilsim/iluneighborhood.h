// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the RelocateHighestAllocCostActivityNHoodI class.



#ifndef __SIM_iluneighborhoodsimH
#define __SIM_iluneighborhoodsimH



#include <ilsched/ilolnsgoals.h>
#include <ilsolver/iimmeta.h>
#include <ilsolver/iimls.h>




////////////////////////////////////////////////////////////////////
//
// CREATE LOCAL SEARCH SOLUTION
//
////////////////////////////////////////////////////////////////////

IloSchedulerSolution CreateLSSolution(IloEnv env, 
                                      IloSchedulerSolution globalSolution);




////////////////////////////////////////////////////////////////////
//
// PREDICATES
//
////////////////////////////////////////////////////////////////////



IloPredicate<IloResourceConstraint> IloRCFalsePredicate(IloSolver solver);
IloPredicate<IloResourceConstraint> IloRCFalsePredicate(IlcManager manager);
IloPredicate<IloResourceConstraint> IloRCFalsePredicate(IloEnv env);

IloPredicate<IloResourceConstraint> IloRCTrueIfNotSelectedPredicate(IloSolver solver);
IloPredicate<IloResourceConstraint> IloRCTrueIfNotSelectedPredicate(IlcManager manager);
IloPredicate<IloResourceConstraint> IloRCTrueIfNotSelectedPredicate(IloEnv env);

IloPredicate<IloActivity> IloActivityFalsePredicate(IloSolver solver);
IloPredicate<IloActivity> IloActivityFalsePredicate(IlcManager manager);
IloPredicate<IloActivity> IloActivityFalsePredicate(IloEnv env);
/*
IloPredicate<IloActivity> IsActivityBeforeSelected(IloSolver solver);
IloPredicate<IloActivity> IsActivityBeforeSelected(IlcManager manager);
IloPredicate<IloActivity> IsActivityBeforeSelected(IloEnv env);*/


////////////////////////////////////////////////////////////////////
//
// DEFINING A NEW NEIGHBORHOOD: RELOCATE A PROCESS PLAN
//
////////////////////////////////////////////////////////////////////

class MyRelocateJobNHoodI: public IloSchedulerLargeNHoodI {
protected:
  IloArray<IloActivityArray> _jobs;
  IloComparator<IloActivityArray> _jobComparator;
  IloArrayI** _tmp;
  IloArray<IloSchedulerSolution> _selectedActs;

public:
  MyRelocateJobNHoodI(IloEnv env,
					  IloArray<IloActivityArray> jobs,
					  IloComparator<IloActivityArray> jobComparator,
					  const char* name);
  ~MyRelocateJobNHoodI();
  // virtuals
  virtual IloInt getSize(IloSolver solver) const;
  virtual IloSolution defineSelected(IloSolver solver, IloInt index);
  virtual void start(IloSolver solver, IloSolution solution);
};




IloSchedulerLargeNHood 
MyRelocateJobNHood(IloEnv env,
				   IloArray<IloActivityArray> jobs,
				   IloComparator<IloActivityArray> jobComparator = 0,
				   const char* name = 0);





////////////////////////////////////////////////////////////////////
//
// DEFINING A NEW NEIGHBORHOOD: RELOCATE A SUBSET OF PROCESS PLANS
//
////////////////////////////////////////////////////////////////////

class MyRelocateSubsetNHoodI: public IloSchedulerLargeNHoodI {
protected:
	IloArray<IloActivityArray>		_procPlans;
	IloComparator<IloActivityArray>	_procPlanComparator;
	IloArrayI**						_tmp;
	IloNum							_probability;
	IloRandom						_rand;
	IloArray<IloSchedulerSolution>	_selectedActs;


public:
  MyRelocateSubsetNHoodI(IloEnv env,
						 IloArray<IloActivityArray> procPlans,
						 IloRandom rand,
						 IloNum probability,
						 IloComparator<IloActivityArray> procPlanComparator,
						 
						 const char* name);
  ~MyRelocateSubsetNHoodI();
  void seed(IloInt seed) { _rand.reSeed(seed); }
  // virtuals
  virtual IloInt getSize(IloSolver solver) const;
  virtual IloSolution defineSelected(IloSolver solver, IloInt index);
  virtual void start(IloSolver solver, IloSolution solution);
};




IloSchedulerLargeNHood 
MyRelocateSubsetNHood(IloEnv env,
					  IloArray<IloActivityArray> procPlans,
					  IloRandom rand,
					  IloNum probability,
					  IloComparator<IloActivityArray> procPlanComparator = 0,
					  const char* name = 0);






////////////////////////////////////////////////////////////////////
//
// DEFINING A NEW NEIGHBORHOOD: RELOCATE ACTIVITIES CHOSEN RANDOMLY
//
////////////////////////////////////////////////////////////////////

class RandomActivityNHoodI : public IloSchedulerLargeNHoodI {
protected:
  IloNum               _probability;
  IloRandom            _rand;
  IloSchedulerSolution _delta;

public:
	RandomActivityNHoodI(IloEnv env,
						 IloRandom rand,
						 IloNum probability,
						 const char* name);
	~RandomActivityNHoodI() {}
	void seed(IloInt seed) { _rand.reSeed(seed); }
	// virtuals
	virtual IloInt getSize(IloSolver solver) const { return 1; }
	virtual IloSolution defineSelected(IloSolver solver, IloInt index) {
		assert(_delta.getImpl() != 0);
		assert(_delta.getSolution().getImpl() != 0);
		return _delta.getSolution().makeClone(solver.getEnv());
	}
	virtual void start(IloSolver solver, IloSolution solution);
};



IloSchedulerLargeNHood
RandomActivityNHood(IloEnv env,
					IloRandom rand,
					IloNum probability,
					const char* name = 0);




/*
////////////////////////////////////////////////////////////////////
//
// DEFINING A NEW NEIGHBORHOOD: RELOCATE HIGHEST ALLOCATION COST
// ACTIVITY
//
////////////////////////////////////////////////////////////////////

class RelocateHighestAllocCostActivityNHoodI: public IloSchedulerLargeNHoodI {
protected:
  IloActivityArray _activities;

public:
  RelocateHighestAllocCostActivityNHoodI(IloEnv env,
										 IloArray<IloActivityArray> jobs,
										 const char* name);
  ~RelocateHighestAllocCostActivityNHoodI();
  // virtuals
  virtual IloInt getSize(IloSolver solver) const;
  virtual IloSolution defineSelected(IloSolver solver, IloInt index);
  virtual void start(IloSolver solver, IloSolution solution);
};


IloPredicate<IloActivity> IsActivityBeforeSelected(IloSolver solver);
IloPredicate<IloActivity> IsActivityBeforeSelected(IlcManager manager);
IloPredicate<IloActivity> IsActivityBeforeSelected(IloEnv env);

IloSchedulerLargeNHood 
RelocateHighestAllocCostActivityNHoodI(IloEnv env,
									   IloArray<IloActivityArray> jobs,
									   const char* name = 0);





*/




#endif