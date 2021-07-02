// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluconstraints.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IloGlobalAllocationCostConstraint class.


#include <ilsim/iluconstraints.h>
#include <ilsched/iloscheduler.h>
#include <ilsim/iluprocessplan.h>



ILOCPCONSTRAINTWRAPPER2(IloGlobalAllocationCostConstraint, solver, IloIntVar, allocCostVar, IloInt**, allocCosts) {
	use(solver, allocCostVar);
	return IlcGlobalAllocationCostConstraint(solver.getIntVar(allocCostVar), allocCosts);
}



IlcGlobalAllocationCostConstraintI::IlcGlobalAllocationCostConstraintI(IloSolver	solver,
																	   IlcIntVar	allocCostVar,
																	   IlcInt**		allocCosts)
																	   : IlcConstraintI(solver),
																	   _allocCostVar(allocCostVar),
																	   _allocCosts(allocCosts) {}


void
IlcGlobalAllocationCostConstraintI::post() {
	IloSolver solver = _allocCostVar.getSolver();
	IlcScheduler sched(solver);
	IlcIntExp cost;

	for (IlcActivityIterator actIte(sched); actIte.ok(); ++actIte) {
		IlcActivity act = *actIte;
		for (IlcAltResConstraintIterator altResCtIte(act); altResCtIte.ok(); ++altResCtIte) {
			IlcAltResConstraint altResCt = *altResCtIte;
			IlcAltResSet altResSet = altResCt.getAltResSet();
			IlcInt numberOfResources = altResSet.getSize();
			IlcIntVar indexVar = altResCt.getIndexVariable();
			IlcActivity act = altResCt.getActivity();
			IloActivity activity = sched.getExtractable(act);
			IluInfoActivity* infoAct = (IluInfoActivity*)activity.getObject(); 
			IlcInt actIndex = infoAct->getActIndex();
		//	altResCt.setObject((void*)_allocCosts[actIndex]);
			IlcConstIntArray allocCostTemp(solver, numberOfResources, _allocCosts[actIndex]);
			IlcIntArray intArray(solver, numberOfResources, _allocCosts[actIndex]);
			IlcIntVar localAllocCostVar(solver, intArray);
			solver.add(IlcTableConstraint(localAllocCostVar, allocCostTemp, indexVar));
			if(0 == cost.getImpl())
				cost = localAllocCostVar;
			else
				cost = cost + localAllocCostVar;
		}
	}
	solver.add(_allocCostVar == cost);
}

void
IlcGlobalAllocationCostConstraintI::propagate() {}


IlcConstraint
IlcGlobalAllocationCostConstraint(IlcIntVar	allocCostVar,
												IlcInt**	allocCosts) {
	IloSolver solver = allocCostVar.getSolver();
    return new(solver.getHeap()) IlcGlobalAllocationCostConstraintI(solver, allocCostVar, allocCosts);
}