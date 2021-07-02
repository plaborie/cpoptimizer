// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluactivity.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/iluactivity.h>
#include <ilsim/ilurandom.h>


IluActivityI::IluActivityI(const IloEnv env, 
						   const IloNum meanDur, 
						   const IloNum sigmaDur, 
						   const IloNum durBound,
						   IloRandom randGen, 
						   IloInt index,
						   const char* name)
	:   IloActivity(env,
					IloNumVar(env, IloMax(0.0, meanDur - durBound * sigmaDur),	meanDur + durBound * sigmaDur, ILOINT), 
					0, 
					name),
		_indicativeProcessingTime(meanDur),
		_index(index),
		_executed(IloFalse),
		_effectiveStartTime(IlcIntMax),
		_effectiveEndTime(IlcIntMax),
		_effectiveProcessingTime(IlcIntMax),
		_indicativeStartTime(IlcIntMax),
		_histP(new (env) IluHistogramI(env)),
		_expIndicativeStartTime(IloInfinity) {
	IloNum average = meanDur;
	IloNum stdDev = sigmaDur;
	IloNum min, max;
	min = IloMax(1.0 ,average - durBound*sigmaDur);
	max = average + durBound*sigmaDur;

// The environment dynamic allocation permits us not to take care of the memory management (objects destructions).
	IluNormalVariableI* varP = new (env) IluNormalVariableI(env, randGen, min, max, average, stdDev);
	_randVarP = varP;
	setObject(this); // Pointer to an IluActivityI object from an IloActivity object
}
