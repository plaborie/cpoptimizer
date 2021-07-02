// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluextendedmodel.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------




/*#if defined(ILOUSESTL)
#include <fstream>
#else
#include <fstream.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif*/

#include <ilsim/iluextendedmodel.h>


ILOSTLBEGIN


IluExtendedModelI::IluExtendedModelI(IloEnv env,
									 char* instanceName,
									 const IloNum mDurBound,
									 const IloNum mTimeBreakBound,
									 const IloNum mBreakDurBound,
									 IloRandom randGenerator)
	: _model(env) {	
	IloInt nbActivities;
	
	int lineLimit = 2048;
	char buffer[2048];

	ifstream instanceStream(instanceName);
	instanceStream.getline(buffer, lineLimit);
//	printf("The buffer is %ld long.\n", strlen(buffer));
	if(strlen(buffer) > 0) {
		istrstream instanceLine(buffer, strlen(buffer));
		instanceLine >> _nbProcessPlans >> nbActivities >> _nbResources >> _nbTotalResources >> _kMax >> _beta;
		if((_nbProcessPlans < 0)||(nbActivities < 0)||(_nbResources < 0)||(_nbTotalResources < 0)||(_kMax < 0.0)||(_beta < 0.0))
			printf("Paramaters have not been correctly set!\n");

		_nbTotalActivities = _nbProcessPlans * nbActivities;

		IloInt*** resourceNumbers = new(env) IloInt**[_nbProcessPlans];
		IloInt** meanDurations = new(env) IloInt*[_nbProcessPlans];
		IloNum** sigmaDurations = new(env) IloNum*[_nbProcessPlans];
		_dueDates = new(env) IloInt[_nbProcessPlans];
		_phis = new(env) IloNum[_nbProcessPlans];

		_allocCosts = new (env) IloNum*[_nbTotalActivities + 1];
		_processPlanIds = new (env) IloInt[_nbProcessPlans * nbActivities + 1];
		_allocExp = new(env) IloNum[_nbTotalActivities + 1];
		
		IloInt i, j, k;

		for(i = 0; i < _nbTotalActivities + 1; i++)
			_allocCosts[i] = new(env) IloNum[_nbResources];

		_processPlanIds[0] = -1;							// Activity 0 is a fictive activity used for computing makespan.
		for(i = 0 ; i < _nbResources ; i++)
			_allocCosts[0][i] = 0.0;
	
		for(i = 0; i < _nbProcessPlans; i++) {
			instanceStream.getline(buffer, lineLimit);
		//	printf("The buffer is %ld long.\n", strlen(buffer));
			if(strlen(buffer) > 0) {
				istrstream instanceLine(buffer, strlen(buffer));
	
				resourceNumbers[i] = new(env) IloInt*[nbActivities];
				meanDurations[i] = new(env) IloInt[nbActivities];
				sigmaDurations[i] = new(env) IloNum[nbActivities];
				for(j = 0; j < nbActivities; j++) {
					_processPlanIds[i * nbActivities + j + 1] = i;
					resourceNumbers[i][j] = new(env) IloInt[_nbResources];
					IloNum temp = 0.0;
					for(k = 0; k < _nbResources; k++) {
						instanceLine >> resourceNumbers[i][j][k] >> _allocCosts[i * nbActivities + j + 1][k];
						temp += _allocCosts[i * nbActivities + j + 1][k]; 
					//	printf("The activity a%ld%ld is associated to the resource r%ld.\n",i,j, resourceNumbers[i][j][k]);
					//	printf("This allocation costs %.14f.\n", _allocCosts[i * nbActivities + j + 1][k]);
					}
					_allocExp[i * nbActivities + j + 1] = temp / _nbResources;
					instanceLine >> meanDurations[i][j] >> sigmaDurations[i][j];
				//	printf("The activitiy a%ld%ld has a mean duration equal to %ld and a standard deviation equal to %.14f.\n", i, j, meanDurations[i][j], sigmaDurations[i][j]);
				}
				instanceLine >> _dueDates[i] >> _phis[i];
			//	printf("The process plan p%ld has a due date equal to %ld and a tardiness slope equal to %.14f.\n", i, _dueDates[i], _phis[i]);
			}
		}

		IloInt* mTimeBreak = new(env) IloInt[_nbTotalResources];
		IloNum* sigmaTimeBreak = new (env) IloNum[_nbTotalResources];
		IloInt* mBreakDur = new(env) IloInt[_nbTotalResources];
		IloNum* sigmaBreakDur = new(env) IloNum[_nbTotalResources];

		for(i = 0 ; i < _nbTotalResources ; i++) {
			instanceStream.getline(buffer, lineLimit);
			istrstream instanceLine(buffer, strlen(buffer));
			instanceLine >> mTimeBreak[i] >> sigmaTimeBreak[i] >> mBreakDur[i] >> sigmaBreakDur[i];
		}

		// CREATE THE MAKESPAN VARIABLE.
		printf("Creating a makespan variable\n");
		
		IloInt maxDuration = 0;
		for(i = 0; i < _nbProcessPlans; i++)
			for(j=0 ; j < nbActivities ; j++)
				if(meanDurations[i][j] > maxDuration)
					maxDuration = meanDurations[i][j];

		IloInt* currentBreakStart = new(env) IloInt[_nbTotalResources];
		IloInt* currentBreakEnd = new(env) IloInt[_nbTotalResources];
		for(i = 0; i < _nbTotalResources; i++) {
			currentBreakStart[i] = 0;
			currentBreakEnd[i] = mBreakDur[i] + mTimeBreak[i];
		}

		IloNum activityEnd = 0.0;		// activityEnd is the date at which the preceding virtual activity ends execution by taking into account all current breakdowns.
		IloNum max = (IloNum)IloIntMax;
		IloNum temp = (IloNum)maxDuration;		// maxDuration is constant and is equal to the duration of one activity.
		for(j = _nbTotalActivities ; j > 0 ; j--) {
			IloInt activityEndMax = (IloInt)activityEnd + maxDuration;
			IloNum temp1 = (IloNum)activityEnd + temp;
				if(temp1 > max)							// We assess if the integer capacity is not exceeded.
					printf("The horizon is too big!\n");
			for(i = 0; i < _nbTotalResources; i++){
				IloInt currentDuration = maxDuration;
				IloInt min = (IloInt)activityEnd;
				while(currentBreakStart[i] < activityEndMax) {
					if(currentBreakEnd[i] < currentBreakStart[i])
						printf("The process for computing horizon does not work correctly.\n");
					if(currentBreakStart[i] > min)
						min = currentBreakStart[i];
					if((IloInt)activityEnd + currentBreakEnd[i] + currentDuration - min > activityEndMax)
						activityEndMax = (IloInt)activityEnd + currentBreakEnd[i] + currentDuration - min;
					currentDuration = currentDuration + currentBreakEnd[i] - min;
					currentBreakStart[i] += mBreakDur[i] + mTimeBreak[i];
					currentBreakEnd[i] = currentBreakStart[i] + mBreakDur[i];
				}
			}
			activityEnd = (IloNum)activityEndMax;
		}

		IloNum horizon = activityEnd;

		_makespan = IloNumVar(env, 0.0, horizon, ILOINT, "Makespan");	// This is equivalent to _makespan = IloIntVar(env, 0, (IloInt)horizon, ILOINT, "Makespan");


		// CREATE THE UNCERTAIN ALTERNATIVE RESOURCES.
		printf("Creating uncertain alternative resources\n");
		IloSchedulerEnv schedEnv(env);
		schedEnv.getResourceParam().setCapacityEnforcement(IloExtended); // Edge-finder, propagation of resource constraints

		_resources = new (env) IluUnaryResourceI*[_nbTotalResources];
		for (i = 0; i < _nbTotalResources; i++)
			_resources[i] = new (env) IluUnaryResourceI(env,
														mTimeBreak[i],
														sigmaTimeBreak[i],
														mTimeBreakBound,
														mBreakDur[i],
														sigmaBreakDur[i],
														mBreakDurBound,
														randGenerator,
														i);

		// CREATE THE UNCERTAIN ACTIVITIES AND ADD TEMPORAL AND RESOURCE CONSTRAINTS.
		printf("Creating uncertain unary activities and adding temporal and resource constraints\n");
		
		IluActivityI** uncActP = new(env) IluActivityI*[_nbTotalActivities + 1];
		uncActP[0] = new(env) IluActivityI(env, 0.0, 0.0, 0.0, randGenerator, 0);
		_altAllocSets = new(env) AltAllocSetI*[_nbTotalActivities + 1];
		AltAllocSetI* altAllocSet0 = new(env) AltAllocSetI(uncActP[0], 0, 0);
		_altAllocSets[0] = altAllocSet0;
		for(i = 0; i < _nbProcessPlans; i++) {
			IluActivityI* previousUncActP;
			for(j = 0; j < nbActivities; j++) {
				sprintf(buffer, "J%ldS%ld", i, j);
				uncActP[i * nbActivities + j + 1] = new(env) IluActivityI(env,
																		  meanDurations[i][j],
																		  sigmaDurations[i][j],
																		  mDurBound,
																		  randGenerator,
																		  i * nbActivities + j + 1,
																		  buffer);
				sprintf(buffer, "startJ%ldS%ld", i, j);
				uncActP[i * nbActivities + j + 1]->getStartExpr().setName(buffer);
				sprintf(buffer, "endJ%ldS%ld", i, j);
				uncActP[i * nbActivities + j + 1]->getEndExpr().setName(buffer);
				sprintf(buffer, "processTimeJ%ldS%ld", i, j);
//				printf("%s processing time between %ld and %ld.\n", uncActP->getName(),
//																	uncActP->getProcessingTimeVariable().getLB(),
//																	uncActP->getProcessingTimeVariable().getUB());
				uncActP[i * nbActivities + j + 1]->getProcessingTimeVariable().setName(buffer);
				IloAltResSet altResSet(env);
				IluUnaryResourceI** altUResSet = new(env) IluUnaryResourceI*[_nbResources];
				for(k = 0; k < _nbResources; k++) {
					altResSet.add(*_resources[resourceNumbers[i][j][k]]);
					altUResSet[k] = _resources[resourceNumbers[i][j][k]];
				}
				IloResourceConstraint rCt = uncActP[i * nbActivities + j + 1]->requires(altResSet);
				AltAllocSetI* altAllocSet = new(env) AltAllocSetI(uncActP[i * nbActivities + j + 1], altUResSet, _nbResources);
				_altAllocSets[i * nbActivities + j + 1] = altAllocSet;
				_model.add(rCt);

				if(0 != j) {
					IloPrecedenceConstraint tC = uncActP[i * nbActivities + j + 1]->startsAfterEnd(*previousUncActP);
					_model.add(tC);
				}
				_model.add(uncActP[i * nbActivities + j + 1]->endsBefore(_makespan));
				previousUncActP = uncActP[i * nbActivities + j + 1];
			}
		}


		// CREATE THE TARDINESS COST VARIABLE AND POST THE TARDINESS COST CONSTRAINT.
		printf("Creating a tardiness variable and posting the tardiness cost constraint\n");
		_tardiCost = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT, "Tardiness");
		IloExpr expr;
		for(i = 1; i < _nbProcessPlans + 1; i++) {
			IloExpr tardi = IloMax(uncActP[i * nbActivities - 1]->getEndExpr() - _dueDates[i - 1], 0.0);
			tardi = _phis[i - 1] * tardi;
			if(0 == expr.getImpl())
				expr = tardi;
			else
				expr = expr + tardi;
		}
		_model.add(_tardiCost == expr);


		// CREATE THE ALLOCATION COST VARIABLE AND POST THE ALLOCATION COST CONSTRAINT.
		printf("Creating an allocation cost variable and posting the allocation cost constraint\n");
		IloNum lowerAllocCost = 0.0;
		IloNum upperAllocCost = 0.0;
		for(i = 1; i < _nbTotalActivities + 1; i++) {
			IloNum lowerAllocCostTemp = IloInfinity;
			IloNum upperAllocCostTemp = 0.0;
			for(j = 0; j < _nbResources; j++) {
				if(_allocCosts[i][j] < lowerAllocCostTemp)
					lowerAllocCostTemp = _allocCosts[i][j];
				if(_allocCosts[i][j] > upperAllocCostTemp)
					upperAllocCostTemp = _allocCosts[i][j];
			}
			lowerAllocCost += lowerAllocCostTemp;
			upperAllocCost += upperAllocCostTemp;
		}
		_allocCost = IloNumVar(env, lowerAllocCost, upperAllocCost, "Allocation");
		_model.add(IloGlobalAllocationCostConstraint(env, _allocCost, _allocCosts, 0));


		// CREATE THE GLOBAL COST VARIABLE AND POST THE GLOBAL COST CONSTRAINT.
		printf("Creating a global cost variable and posting the global cost constraint\n");
		_globalCost = IloNumVar(env, lowerAllocCost, IloInfinity, "Global");
		_model.add(_globalCost == _allocCost + _beta * _tardiCost);
	}
}