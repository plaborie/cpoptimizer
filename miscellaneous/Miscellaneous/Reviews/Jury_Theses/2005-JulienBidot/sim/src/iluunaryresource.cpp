// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluunaryresource.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/iluunaryresource.h>
#include <ilsim/ilurandom.h>




IluUnaryResourceI::IluUnaryResourceI(const IloEnv env,
									 const IloNum meanTimeBreak,
									 const IloNum sigmaTimeBreak,
									 const IloNum timeBreakBound,
									 const IloNum meanBreakDur,
									 const IloNum sigmaBreakDur,
									 const IloNum breakDurBound,
									 IloRandom randGenerator,
									 IloInt index,
									 const char* name)
		:	IloUnaryResource(env, name),
			_meanTimeBreak(meanTimeBreak),
			_sigmaTimeBreak(sigmaTimeBreak),
			_meanBreakDur(meanBreakDur),
			_sigmaBreakDur(sigmaBreakDur),
			_index(index),
			_breakdownStartTime(IloIntMax),
			_brokenDown(IloFalse) {
	IloNum average = meanTimeBreak;
	IloNum stdDev = sigmaTimeBreak;
	IloNum bound = timeBreakBound;
	IloNum min = IloMax(1.0, average - bound*stdDev);
	IloNum max = average + bound*stdDev;
	IluNormalVariableI* varP = new (env) IluNormalVariableI(env, randGenerator, min, max, average, stdDev);
	_interBreaksVarP = varP;

	average = meanBreakDur;
	stdDev = sigmaBreakDur;
	bound = breakDurBound;
	min = IloMax(1.0, average - bound*stdDev);
	max = average + bound*stdDev;
	IluNormalVariableI* varP2 = new (env) IluNormalVariableI(env, randGenerator, min, max, average, stdDev);
	_breaksDurationVarP = varP2;
	setObject(this);	// Pointer to an IluUnaryResourceI object from an IloUnaryResource object
}