// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluneighborhood.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the CreateLSSolution function.


#include <ilsim/iluneighborhood.h>




IloSchedulerSolution CreateLSSolution(IloEnv env, 
                                      IloSchedulerSolution globalSolution) {

  // CREATE LOCAL SEARCH SOLUTION
  IloSchedulerSolution lsSolution = globalSolution.makeClone(env);

  for (IloIterator <IloResourceConstraint> iter(env); iter.ok(); ++iter)
    lsSolution.setRestorable(*iter, IloRestoreRCNext | IloRestoreRCSelected);

  return lsSolution;
}



ILOPREDICATE0(IloMyRCFalsePredicate, IloResourceConstraint, rc) {
	return IloFalse;
}


ILOCTXPREDICATE0(IloMyRCTrueIfNotSelectedPredicate,
				 IloResourceConstraint, rc,
				 IloSchedulerLargeNHood, nhood) {
	if (nhood.isSelected(rc))
		return IloFalse;
	else 
		return IloTrue;
}


ILOPREDICATE0(IloMyActivityFalsePredicate, IloActivity, activity) {
	return IloFalse;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MyRelocateJobNHoodI::MyRelocateJobNHoodI(IloEnv env,
										 IloArray<IloActivityArray> jobs,
										 IloComparator<IloActivityArray> jobComparator,
										 const char* name)
  : IloSchedulerLargeNHoodI(env, name),
    _jobs(env),
	_jobComparator(jobComparator),
	_tmp(0),
	_selectedActs(env, jobs.getSize()) {
	// set default restore policy
	IloPredicate<IloResourceConstraint> rcFalsePredicate = IloMyRCFalsePredicate(env);
	setRestoreRCNextPredicate(rcFalsePredicate);
	setRestoreRCPrevPredicate(rcFalsePredicate);
	setRestoreRCSetupPredicate(rcFalsePredicate);
	setRestoreRCTeardownPredicate(rcFalsePredicate);

	IloPredicate<IloResourceConstraint> rcTrueIfNotSelectedPredicate = IloMyRCTrueIfNotSelectedPredicate(env);
	setRestoreRCDirectSuccessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCDirectPredecessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCCapacityPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCSelectedPredicate(rcTrueIfNotSelectedPredicate);

	IloPredicate<IloActivity> activityFalsePredicate = IloMyActivityFalsePredicate(env);
	setRestoreActivityStartPredicate(activityFalsePredicate);
	setRestoreActivityEndPredicate(activityFalsePredicate);
	setRestoreActivityDurationPredicate(activityFalsePredicate);
	setRestoreActivityProcessingTimePredicate(activityFalsePredicate);
	setRestoreActivityExternalPredicate(activityFalsePredicate);

	for(IloInt j = 0; j < jobs.getSize(); ++j) {
		IloInt nbActivities = jobs[j].getSize();
		IloActivityArray job(env);
		for(IloInt a = 0; a < nbActivities; ++a) {
			IloActivity activity = jobs[j][a];
			job.add(activity);
		}
		_jobs.add(job);
	}
	_tmp = new(env) IloArrayI*[_jobs.getSize()];
//	_tmp = new IloArrayI*[_jobs.getSize()];

	for(IloInt i = 0; i < _selectedActs.getSize(); i++) {
		_selectedActs[i] = IloSchedulerSolution();
	}
}


MyRelocateJobNHoodI::~MyRelocateJobNHoodI() {
	_jobs.end();

	for(IloInt i = 0; i < _selectedActs.getSize(); i++) {
		if(0 != _selectedActs[i].getImpl()) {
			IloSchedulerSolution solution = _selectedActs[i];
			solution.end();
			_selectedActs[i] = IloSchedulerSolution();
		}
	}
	_selectedActs.end();


	// we should also release _tmp:
	// delete [] _tmp;
}

static IloSchedulerLargeNHood TheNeighborHood = 0;
static IloComparator<IloActivityArray> TheJobComparator = 0;

static int CompareJob(const void* j1, const void* j2) {
	IloArrayI* a1 = (*(IloArrayI**)j1);
	IloArrayI* a2 = (*(IloArrayI**)j2);
	IloActivityArray job1(a1);
	IloActivityArray job2(a2);
	return TheJobComparator(job1, job2, &TheNeighborHood);
}


void 
MyRelocateJobNHoodI::start(IloSolver solver, IloSolution solution) {
	IloSchedulerLargeNHoodI::start(solver, solution);

	IloEnv env = getEnv();

//	IloSchedulerSolution currentSolution = getCurrentSolution();

	IloInt j;

	IloInt nbJobs = _jobs.getSize();
/*	for(j = 0; j < nbJobs; ++j) {
		IloActivityArray job = _jobs[j];
		IloInt nbActivities = job.getSize();
		for(IloInt a = 0; a < nbActivities; ++a) {
			IloActivity activity = job[a];
			if(!currentSolution.contains(activity)) {
				throw IloSchedulerException("Error: activity not in solution!");
			}
		}
	}
*/
	for(j = 0; j < _selectedActs.getSize(); j++) {
		if(0 != _selectedActs[j].getImpl()) {
			IloSchedulerSolution tempSolution = _selectedActs[j];
			tempSolution.end();
			_selectedActs[j] = IloSchedulerSolution();
		}
	}

	// qsort using comparator (if any)
	if(0 != _jobComparator.getImpl()) {		
		// initialize _tmp
		for(j = 0; j < nbJobs; j++) {
			_tmp[j] = _jobs[j].getImpl();
		}
		// sort
		TheNeighborHood = this;
		TheJobComparator = _jobComparator;
		qsort((void *)_tmp, (size_t) nbJobs, sizeof(IloArrayI*), CompareJob);
		// update _jobs
		for (j = 0; j < nbJobs; j++) {
			_jobs[j] = _tmp[j];
		}
	}
}


IloInt 
MyRelocateJobNHoodI::getSize(IloSolver solver) const {
	IloInt size = _jobs.getSize();
	return size;
}


IloSolution
MyRelocateJobNHoodI::defineSelected(IloSolver solver, IloInt index) {
	IloEnv env = solver.getEnv();

	IloActivityArray job;
	job = _jobs[index];
//	printf("Job %ld/%ld is selected.\n", index, _jobs.getSize());

//	IloSchedulerSolution selected(env);
	IloSchedulerSolution selected = _selectedActs[index];
	if(0 == selected.getImpl()) {
		selected = IloSchedulerSolution(env);
		IloSchedulerSolution currentSolution = getCurrentSolution();

		// add in selected all activities of the selected job and their
		// resource constraints
		IloInt nbActivities = job.getSize();
		for (IloInt a = 0; a < nbActivities; a++) {
			IloActivity activity = job[a];
			add(selected, activity);
			IloSchedulerSolution::ResourceConstraintIterator rcIter(currentSolution, activity);
			for (; rcIter.ok(); ++rcIter) {
				IloResourceConstraint rc = *rcIter;
				selected.add(rc);
			}
		}
	}
	return selected;
}


/*ILOCTXPREDICATE0(IsActivityBeforeSelected,
				 IloActivity, activity,
				 IloTimeWindowNHood, nhood) {
	if(nhood.isBeforeSelected(activity))
		return IloTrue;
	return IloFalse;
}*/


IloSchedulerLargeNHood 
MyRelocateJobNHood(IloEnv env, 
				   IloArray<IloActivityArray> jobs,
				   IloComparator<IloActivityArray> jobComparator,
				   const char* name) {
	return new(env) MyRelocateJobNHoodI(env, jobs, jobComparator, name);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MyRelocateSubsetNHoodI::MyRelocateSubsetNHoodI(IloEnv env,
											   IloArray<IloActivityArray> procPlans,
											   IloRandom rand,
											   IloNum probability,
											   IloComparator<IloActivityArray> procPlanComparator,
											   const char* name)
  : IloSchedulerLargeNHoodI(env, name),
	_procPlans(env),
	_procPlanComparator(procPlanComparator),
	_tmp(0),
	_rand(rand),
	_probability(probability),
	_selectedActs(env, procPlans.getSize()) {
	// set default restore policy
	IloPredicate<IloResourceConstraint> rcFalsePredicate = IloMyRCFalsePredicate(env);
	setRestoreRCNextPredicate(rcFalsePredicate);
	setRestoreRCPrevPredicate(rcFalsePredicate);
	setRestoreRCSetupPredicate(rcFalsePredicate);
	setRestoreRCTeardownPredicate(rcFalsePredicate);

	IloPredicate<IloResourceConstraint> rcTrueIfNotSelectedPredicate = IloMyRCTrueIfNotSelectedPredicate(env);
	setRestoreRCDirectSuccessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCDirectPredecessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCCapacityPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCSelectedPredicate(rcTrueIfNotSelectedPredicate);

	IloPredicate<IloActivity> activityFalsePredicate = IloMyActivityFalsePredicate(env);
	setRestoreActivityStartPredicate(activityFalsePredicate);
	setRestoreActivityEndPredicate(activityFalsePredicate);
	setRestoreActivityDurationPredicate(activityFalsePredicate);
	setRestoreActivityProcessingTimePredicate(activityFalsePredicate);
	setRestoreActivityExternalPredicate(activityFalsePredicate);

	for(IloInt j = 0; j < procPlans.getSize(); ++j) {
		IloInt nbActivities = procPlans[j].getSize();
		IloActivityArray procPlan(env);
		for(IloInt a = 0; a < nbActivities; ++a) {
			IloActivity activity = procPlans[j][a];
			procPlan.add(activity);
		}
		_procPlans.add(procPlan);
	}
	_tmp = new(env) IloArrayI*[_procPlans.getSize()];
//	_tmp = new IloArrayI*[_jobs.getSize()];

	for(IloInt i = 0; i < _selectedActs.getSize(); i++) {
		_selectedActs[i] = IloSchedulerSolution();
	}
}


MyRelocateSubsetNHoodI::~MyRelocateSubsetNHoodI() {
	_procPlans.end();

	for(IloInt i = 0; i < _selectedActs.getSize(); i++) {
		if(0 != _selectedActs[i].getImpl()) {
			IloSchedulerSolution solution = _selectedActs[i];
			solution.end();
			_selectedActs[i] = IloSchedulerSolution();
		}
	}
	_selectedActs.end();


	// we should also release _tmp:
	// delete [] _tmp;
}


void 
MyRelocateSubsetNHoodI::start(IloSolver solver, IloSolution solution) {
	IloSchedulerLargeNHoodI::start(solver, solution);

	IloEnv env = getEnv();

	IloInt j;

	for(j = 0; j < _selectedActs.getSize(); j++) {
		if(0 != _selectedActs[j].getImpl()) {
			IloSchedulerSolution tempSolution = _selectedActs[j];
			tempSolution.end();
			_selectedActs[j] = IloSchedulerSolution();
		}
	}

	IloInt nbProcPlans = _procPlans.getSize();
	// qsort using comparator (if any)
	if(0 != _procPlanComparator.getImpl()) {		
		// initialize _tmp
		for(j = 0; j < nbProcPlans; j++) {
			_tmp[j] = _procPlans[j].getImpl();
		}
		// sort
		TheNeighborHood = this;
		TheJobComparator = _procPlanComparator;
		qsort((void *)_tmp, (size_t) nbProcPlans, sizeof(IloArrayI*), CompareJob);
		// update _jobs
		for (j = 0; j < nbProcPlans; j++) {
			_procPlans[j] = _tmp[j];
		}
	}
}


IloInt 
MyRelocateSubsetNHoodI::getSize(IloSolver solver) const {
	IloInt size = _procPlans.getSize();
	return size;
}


IloSolution
MyRelocateSubsetNHoodI::defineSelected(IloSolver solver, IloInt index) {
//	printf("Selection starts.\n");

	IloSchedulerSolution selected = _selectedActs[index];
	if(0 == selected.getImpl()) {
		IloEnv env = solver.getEnv();
		selected = IloSchedulerSolution(env);
		IloSchedulerSolution currentSolution = getCurrentSolution();

		// Select process plans
		IloInt nbProcPlans = _procPlans.getSize();
		for(IloInt i = 0; i < nbProcPlans; i++) {
			if((_rand.getFloat() <= _probability)||(i == index)) {
				IloActivityArray procPlan;
				procPlan = _procPlans[i];
			//	printf("Process plan %ld/%ld is selected.\n", i, _procPlans.getSize());

			//	Add in selected all activities of the selected process plan and their resource constraints
				IloInt nbActivities = procPlan.getSize();
				for (IloInt a = 0; a < nbActivities; a++) {
					IloActivity activity = procPlan[a];
					add(selected, activity);
					IloSchedulerSolution::ResourceConstraintIterator rcIter(currentSolution, activity);
					for (; rcIter.ok(); ++rcIter) {
						IloResourceConstraint rc = *rcIter;
						selected.add(rc);
					}
				}

			}
		}

	}

	return selected;
}



IloSchedulerLargeNHood 
MyRelocateSubsetNHood(IloEnv env,
					  IloArray<IloActivityArray> procPlans,
					  IloRandom rand,
					  IloNum probability,
					  IloComparator<IloActivityArray> procPlanComparator,
					  const char* name) {
	return new(env) MyRelocateSubsetNHoodI(env, procPlans, rand, probability, procPlanComparator, name);
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Random activity neighborhood
//

RandomActivityNHoodI::RandomActivityNHoodI(IloEnv env,
										   IloRandom rand,
										   IloNum probability,
										   const char* name)
	: IloSchedulerLargeNHoodI(env, name),
    _rand(rand),
    _probability(probability){
	// set default restore policy
	IloPredicate<IloResourceConstraint> rcFalsePredicate = IloMyRCFalsePredicate(env);
	setRestoreRCNextPredicate(rcFalsePredicate);
	setRestoreRCPrevPredicate(rcFalsePredicate);
	setRestoreRCSetupPredicate(rcFalsePredicate);
	setRestoreRCTeardownPredicate(rcFalsePredicate);

	IloPredicate<IloResourceConstraint> rcTrueIfNotSelectedPredicate = IloMyRCTrueIfNotSelectedPredicate(env);
	setRestoreRCDirectSuccessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCDirectPredecessorPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCSelectedPredicate(rcTrueIfNotSelectedPredicate);
	setRestoreRCCapacityPredicate(rcTrueIfNotSelectedPredicate);

	IloPredicate<IloActivity> activityFalsePredicate = IloMyActivityFalsePredicate(env);
	setRestoreActivityStartPredicate(activityFalsePredicate);
	setRestoreActivityEndPredicate(activityFalsePredicate);
	setRestoreActivityDurationPredicate(activityFalsePredicate);
	setRestoreActivityProcessingTimePredicate(activityFalsePredicate);
	setRestoreActivityExternalPredicate(activityFalsePredicate);
}


void 
RandomActivityNHoodI::start(IloSolver solver, IloSolution solution) {
	IloEnv env = getEnv();
#ifdef _TRACE
	env.out() << "*** Executing RandomActivityNHoodI::start..." << endl;
#endif

	IloSchedulerLargeNHoodI::start(solver, solution);

	if (_delta.getImpl() != 0)
		_delta.end();
  
	_delta = IloSchedulerSolution(env);
  
	IloSchedulerSolution currentSolution = getCurrentSolution();
  
	assert (currentSolution.getImpl());

	// select activities randomly
	IloSchedulerSolution::ActivityIterator actIter(currentSolution);
	for(; actIter.ok(); ++actIter) {
		IloActivity activity = *actIter;
		if(_rand.getFloat() <= _probability) {
#ifdef _TRACE
			PrintActivity(currentSolution, activity);
			env.out() << ";";
#endif
			_delta.add(activity);
		}
	}

	// select resource constraints of activities selected
	IloSchedulerSolution::ResourceConstraintIterator rcIter(currentSolution);
	for(; rcIter.ok(); ++rcIter) {
		IloResourceConstraint rc = *rcIter;
		IloActivity activity = rc.getActivity();
		if(_delta.contains(activity))
			_delta.add(rc);
	}
}


IloSchedulerLargeNHood 
RandomActivityNHood(IloEnv env,
				    IloRandom rand,
					IloNum probability,
					const char* name){
	return new(env) RandomActivityNHoodI(env,
										 rand,
										 probability,
										 name);
}