// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilumodel.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------



#include <ilsched/iloscheduler.h>

#if defined(ILOUSESTL)
#include <fstream>
#else
#include <fstream.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif

#include <ilsim/ilumodel.h>
#include <ilsim/iluactivity.h>




IluModelI::IluModelI(const IloEnv& env,
					 char* fileName,
					 IloRandom randGenerator,
					 const IloNum alpha,
					 const IloInt technique,
					 ofstream streamScenario)
	:_model(env),
	_solution(env) {
	
	IloInt numberOfJobs;
	IloInt numberOfResources;
	
	ifstream file(fileName);
	int lineLimit = 1024;
	char buffer[1024];

	file.getline(buffer, lineLimit);
	istrstream line(buffer, strlen(buffer));
	line >> numberOfJobs >> numberOfResources;

//	For recording scenarios	
	streamScenario << numberOfJobs << "\t" << numberOfResources << endl;

	IloInt* resourceNumbers = new (env) IloInt[numberOfJobs * numberOfResources];
	IloInt* durations = new (env) IloInt[numberOfJobs * numberOfResources];

	IloInt k = 0;
	for (IloInt i = 0; i < numberOfJobs; i++) {
		file.getline(buffer, lineLimit);
		istrstream jobline(buffer, strlen(buffer));

		for (IloInt j = 0; j < numberOfResources; j++) {
			jobline >> resourceNumbers[k] >> durations[k];
			k++;
		}
	}

  // CREATE THE MAKESPAN VARIABLE.
	IloInt numberOfActivities = numberOfJobs * numberOfResources;
	IloInt horizon = 0;
	for (k = 0; k < numberOfActivities; k++)
	    horizon += durations[k];
	_makespan = IloNumVar(env, 0, horizon, ILOINT);

  // CREATE THE RESOURCES.
	IloSchedulerEnv schedEnv(env);
	schedEnv.getResourceParam().setCapacityEnforcement(IloExtended); // Edge-finder, propagation of resource constraints

	IloInt j;
	IloUnaryResource *resources = new (env) IloUnaryResource[numberOfResources];
	for (j = 0; j < numberOfResources; j++)
		resources[j] = IloUnaryResource(env);

  // CREATE THE UNCERTAIN ACTIVITIES AND ADD TEMPORAL AND RESOURCE CONSTRAINTS.
	char buffer2[128], buffer3[128];
	for (i = 0, k = 0; i < numberOfJobs; i++) {
		IluActivityI* previousUncActP;
	    for (j = 0; j < numberOfResources; j++, k++) {
			IluActivityI* uncActP = new (env) IluActivityI(env.getObject(), durations[k], alpha*durations[k], 5.0, randGenerator, k+1);
			_solution.add(uncActP);

			if(1 == technique) // Proactive technique
				uncActP->setIndicativeProcessingTime(uncActP->getRandomVar()->getCoverValue(0.75));

//	For recording scenarios
			streamScenario << resourceNumbers[k] << "\t" << uncActP->getEffecProcessingTime() << "\t";
//			printf("Resource number %ld\tEffective processing time %ld\n", resourceNumbers[k],
//					uncActP->getEffecProcessingTime());
			
			sprintf(buffer2, "J%ldS%ldR%ld", i, j, resourceNumbers[k]);
			sprintf(buffer3, "startJ%ldS%ldR%ld", i, j, resourceNumbers[k]);
			uncActP->getStartVariable().setName(buffer3);
			sprintf(buffer3, "endJ%ldS%ldR%ld", i, j, resourceNumbers[k]);
			uncActP->getEndVariable().setName(buffer3);
			sprintf(buffer3, "processTimeJ%ldS%ldR%ld", i, j, resourceNumbers[k]);
			uncActP->getProcessingTimeVariable().setName(buffer3);

			uncActP->setName(buffer2);
			
//			cout << uncActP->getName() << " processing time between ";
//			cout << uncActP->getProcessingTimeVariable().getLB() << " and " << uncActP->getProcessingTimeVariable().getUB();
//			cout << "." << endl;
			IloActivity act = uncActP;
			IloResourceConstraint rct = act.requires(resources[resourceNumbers[k]]);
//			NbUnrankedExt* nbOfUnranked = new (env) NbUnrankedExt();
//			rct.setObject(nbOfUnranked);

			_model.add(rct);
			_solution.add(rct);
			
			if (0 != j) {
				IloActivity prevAct = previousUncActP;
				IloPrecedenceConstraint tc = act.startsAfterEnd(prevAct);
				_model.add(tc);
			}
			_model.add(uncActP->endsBefore(_makespan));
			
			previousUncActP = uncActP;
		}
//	For recording scenarios
		streamScenario << endl;
	}
}

