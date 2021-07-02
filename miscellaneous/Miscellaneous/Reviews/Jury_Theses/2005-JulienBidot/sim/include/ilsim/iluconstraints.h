// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IloGlobalAllocationCostConstraint class.



#ifndef __SIM_iluconstraintssimH
#define __SIM_iluconstraintssimH


#include <ilsolver/ilosolver.h>




////////////////////////////////////////////////////////////////////
//
// DEFINING A NEW GLOBAL CONSTRAINT FOR COMPUTING GLOBAL ALLOCATION COST.
//
////////////////////////////////////////////////////////////////////

class IlcGlobalAllocationCostConstraintI: public IlcConstraintI {
private:
    IlcIntVar	_allocCostVar;
	IlcInt**	_allocCosts;
public:
    IlcGlobalAllocationCostConstraintI(IloSolver solver, IlcIntVar allocCostVar, IlcInt** allocCosts);
    ~IlcGlobalAllocationCostConstraintI() {} // empty destructor

    virtual void propagate();
    virtual void post();
//    virtual IlcBool isViolated() const;
};


IlcConstraint IlcGlobalAllocationCostConstraint(IlcIntVar allocCostVar, IlcInt** allocCosts);

IloConstraint IloGlobalAllocationCostConstraint(IloEnv env, IloIntVar allocCostVar, IloInt** allocCosts, const char* name);


#endif