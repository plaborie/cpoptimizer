// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


// This file contains IluModelI class.


#ifndef __SIM_ilumodelsimH
#define __SIM_ilumodelsimH

#include <ilconcert/ilomodel.h>
#include <ilconcert/ilorandom.h>
#include <ilsched/iloscheduler.h>
#include <ilsched/ilosolution.h>

// --------------------------------------------------------------------------
// THIS   CLASS PERMITS US TO GENERQTE THE NON-DETERMINISTIC PROBLEM:
// UNCERTAIN ACTIVITIES AND RESOURCES.
// --------------------------------------------------------------------------



class IluModelI {
private:
	IloModel  _model;
	IloNumVar _makespan;	// _makespan is the makespan of the scheduling
							// problem and has to be minimized.
	IloSchedulerSolution _solution;
	
public:
	IluModelI() {}
	IluModelI(const IloEnv&, char*, IloRandom, const IloNum, const IloInt, ofstream);

	~IluModelI() {}
	void setModel(IloModel model) { _model = model; }
	IloModel getModel() const { return _model; }

	void setMakespan(IloNumVar makespan) { _makespan = makespan; }
	IloNumVar getMakespan() const { return _makespan; }

	IloSchedulerSolution getSolution() const { return _solution; }
};

#endif
