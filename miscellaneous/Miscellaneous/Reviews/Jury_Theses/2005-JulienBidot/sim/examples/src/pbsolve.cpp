#include <ilsched/iloscheduler.h>
/*
#if defined(ILOUSESTL)
#include <fstream>
#else
#include <fstream.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif*/

#include <ctime>
#include <ilsim/ilutimeeventmanager.h>
#include <ilsim/iluextendedmodel.h>
#include <ilsched/ilsched.h>
#include <ilsim/ilusimulator.h>
#include <ilsim/iludynamicscheduler.h>
#include <ilsim/ilurealization.h>


ILOSTLBEGIN




///////////////////////////////////////////////////////////////////////////////
//
// DEFINE THE MODEL FOR CHECKING SOLUTION WITH SCHEDULER FOR A DETERMINISTIC
// PROBLEM
//
///////////////////////////////////////////////////////////////////////////////

IloModel DefineModel(IloEnv				env,
					 char*				instanceName,
					 char*				realizationName,
					 Allocation**		allocations,
					 IloNumVar&			makespan) {
	IloModel model(env);

	IloInt numberOfJobs;
	IloInt numberOfActivitiesPerJob;
	IloInt numberOfResourcesPerActivity;
	IloInt numberOfResources;

	ifstream instanceFile(instanceName);
	ifstream realizationFile(realizationName);

	int lineLimit = 65536;
	char buffer[65536];

	instanceFile.getline(buffer, lineLimit);
	istrstream line(buffer, strlen(buffer));
	line >> numberOfJobs >> numberOfActivitiesPerJob >> numberOfResourcesPerActivity >> numberOfResources;

	IloInt numberOfActivities = numberOfJobs * numberOfActivitiesPerJob;
	
	IloInt* durations = new (env) IloInt[numberOfActivities];

	IloInt i, j, k;
	realizationFile.getline(buffer, lineLimit);
	istrstream firstLine(buffer, strlen(buffer));
	for (i = 0, k = 0; i < numberOfJobs; i++) {
		realizationFile.getline(buffer, lineLimit);
		istrstream jobLine(buffer, strlen(buffer));
		for (j = 0; j < numberOfActivitiesPerJob; j++, k++) {
			jobLine >> durations[k];
		}
	}
  
  // CREATE THE RESOURCES WITH BREAKS.
	IloNum horizon = 0.0;
	IloSchedulerEnv schedEnv(env);
	schedEnv.getResourceParam().setCapacityEnforcement(IloHigh); // Edge-finder, propagation of resource constraints

	IloUnaryResource* resources = new (env) IloUnaryResource[numberOfResources];
	for(i = 0; i < numberOfResources; i++) {
		resources[i] = IloUnaryResource(env);
		resources[i].setObject((void*)(i));

		realizationFile.getline(buffer, lineLimit);
		if(strlen(buffer) > 0) {
			istrstream resourceLine(buffer, strlen(buffer));
			IloInt start, duration;
			IloInt lastStart = -1;
			do {										
				resourceLine >> start >> duration;
				if(start > lastStart) {
					resources[i].addBreak(start, start + duration);
					IloNum temp = IloNum(start + duration);
					if(temp > horizon)
						horizon = temp;
					lastStart = start;
				}
			} while(0 == resourceLine.eof());
		}
	//	printf("Resource %ld: first breakdown starts at %ld, lasts %ld time units, and has a breakdown period of %ld time units.\n", i,
	//		breakdownStarts[i], breakdownDurations[i], breakdownPeriods[i]);
	}

  // CREATE THE MAKESPAN VARIABLE.
	printf("The horizon is equal to %ld.\n", (IloInt)horizon);
	makespan = IloNumVar(env, 0.0, horizon, ILOINT);


  // CREATE THE ACTIVITIES AND ADD INITIAL TEMPORAL CONSTRAINTS.
	printf("Initial precedence constraints are added.\n");
	IloActivity* activities = new (env) IloActivity[numberOfActivities];
	IloActivity previousActivity;
	for(i = 0, k = 0; i < numberOfJobs; i++) {
	    for (j = 0; j < numberOfActivitiesPerJob; j++, k++) {
			activities[k] = IloActivity(env, durations[k]);
		//	printf("Activity %ld: duration %ld\n", k, durations[k]);
			activities[k].setObject((void*)(i));
			activities[k].setBreakable();
		//	model.add(activities[k]);
			if(j > 0) {
				IloPrecedenceConstraint tCt = activities[k].startsAfterEnd(previousActivity);
			//	printf("A precedence constraint is added to the model between activity %ld and activity %ld.\n", k - 1, k);
				model.add(tCt);
			}
			previousActivity = activities[k];
		}
	//	printf("A precedence constraint is added to the model between activity %ld and makespan.\n", k - 1);
		model.add(previousActivity.endsBefore(makespan));
	}

  // ADD TEMPORAL CONSTRAINTS AND RESOURCE CONSTRAINTS FOUND DURING SCHEDULE EXECUTION SIMULATION.
	printf("Temporal constraints and resource constraints are added.\n");
	for(i = 0; i < numberOfResources; i++) {
		Allocation* alloc = allocations[i];
		IloInt previousActIndex = -1;
	//	IloResourceConstraint previousRCt;
		while(0 != alloc) {
			IloInt actIndex = alloc->getId();
			IloInt jobIndex = (IloInt)(activities[actIndex - 1].getObject());
			sprintf(buffer, "J%ldS%ldR%ld", jobIndex, actIndex - jobIndex * numberOfActivitiesPerJob - 1, i);
			activities[actIndex - 1].setName(buffer);
			activities[actIndex - 1].setObject((void*)(actIndex - 1));
			if(previousActIndex > -1) {
				IloPrecedenceConstraint tCt = activities[actIndex - 1].startsAfterEnd(activities[previousActIndex - 1]);
			//	printf("A temporal constraint is added to the model between activity %ld and activity %ld.\n", previousActIndex - 1, actIndex - 1);
				model.add(tCt);
			}
			IloResourceConstraint rCt = activities[actIndex - 1].requires(resources[i]);
		//	printf("A resource constraint is added to the model between activity %ld and resource %ld.\n", actIndex - 1, i);
		//	if(-1 == previousActIndex) {
		//		rCt.setSetup();
		//		printf("This is a setup resource constraint.\n");
		//	}
		//	else
				//	previousRCt.setNext(rCt);
		//	if(0 == alloc->getNext()) {
		//		rCt.setTeardown();
		//		printf("This is a teardown resource constraint.\n");
		//	}
			model.add(rCt);
		//	previousRCt = rCt;
			previousActIndex = actIndex;
			alloc = alloc->getNext();
		}	
	}
	return model;
}


///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS (DETERMINISTIC PROBLEMS)
//
///////////////////////////////////////////////////////////////////////////////


void
PrintSolution(const IloSolver& solver) {
	IlcScheduler scheduler(solver);
	IloEnv env = solver.getEnv();
	for(IloIterator<IloActivity> ite(env); ite.ok(); ++ite) {
	//	solver.out() << *ite << endl;							// Before extraction
		solver.out() << scheduler.getActivity(*ite) << endl;	// After extraction
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// DEFAULT PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
defaultParamPrint(IloInt technique,
				  IloInt seed,
				  IloNum mDurBound,
				  IloNum mTimeBreakBound,
				  IloNum mBreakDurBound,
				  IloInt thetaBreak,
				  IloInt nbSimulations,
				  IloNum execTimeStep,
				  IloBool schedulerTest,
				  IloInt criterion,
				  IloInt deltaTMinProgress,
				  IloInt deltaTMaxProgress,
				  IloNum sigmaTMinProgress,
				  IloNum sigmaTMaxProgress,
				  IloNum reactiveTimeLimit,
				  IloNum progressiveTimeLimit) {
	printf("Parameters are set by default as follows:\n");
	printf("A file describing an instance followed by a file describing a realization followed by one of the following optional parameters:\n");
	printf("-technique %ld\n", technique);
	printf("-seed %ld\n", seed);
	printf("-mDurBound %.1f\n", mDurBound);
	printf("-mTimeBreakBound %.1f\n", mTimeBreakBound);
	printf("-mBreakDurBound %.1f\n", mBreakDurBound);
	printf("-thetaBreak %ld\n", thetaBreak);
	printf("-nbSimulations %ld\n", nbSimulations);
	printf("-execTimeStep %.1f\n", execTimeStep);
	printf("-schedulerTest %ld\n", schedulerTest);
	printf("\n");

	printf("If the technique used is the reactive one (technique = 1):\n");
	printf("-criterion %ld\n", criterion);
	printf("-reactiveTimeLimit %.1f\n", reactiveTimeLimit);
	printf("\n");

	printf("If the technique used is the proactive one (techique = 2):\n");
	printf("\n");

	printf("If the technique used is the progressive one (technique = 3):\n");
	printf("-deltaTMinProgress %ld\n", deltaTMinProgress);
	printf("-deltaTMaxProgress %ld\n", deltaTMaxProgress);
	printf("-sigmaTMinProgress %.1f\n", sigmaTMinProgress);
	printf("-sigmaTMaxProgress %.1f\n", sigmaTMaxProgress);
	printf("-progressiveTimeLimit %.1f\n", progressiveTimeLimit);
	printf("\n");
}





///////////////////////////////////////////////////////////////////////////////
//
// PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
paramPrint(IloInt technique,
		   IloInt seed,
		   IloNum mDurBound,
		   IloNum mTimeBreakBound,
		   IloNum mBreakDurBound,
		   IloInt thetaBreak,
		   IloInt nbSimulations,
		   IloNum execTimeStep,
		   IloBool schedulerTest,
		   IloInt criterion,
		   IloInt deltaTMinProgress,
		   IloInt deltaTMaxProgress,
		   IloNum sigmaTMinProgress,
		   IloNum sigmaTMaxProgress,
		   IloNum reactiveTimeLimit,
		   IloNum progressiveTimeLimit) {
	printf("The technique used is %ld.\n", technique);
	printf("The seed used is %ld.\n", seed);
	printf("The parameter for truncating the probability distributions associated with activity durations is set to %.1f\n", mDurBound);
	printf("The parameter for truncating the probability distributions associated with durations between two consecutive resource breakdowns is set to %.1f\n", mBreakDurBound);
	printf("The parameter for truncating the probability distributions associated with resource breakdown durations is set to %.1f\n", mBreakDurBound);
	printf("The parameter permitting us to determine the date at which we start to generate breakdowns is set to %ld.\n", thetaBreak);
	printf("The number of simulations run at each step is equal to %ld\n", nbSimulations);
	printf("The execution time step is %.1f.\n", execTimeStep);
	if(IloTrue == schedulerTest)
		printf("A test is going to be performed by using ILOG Scheduler to check if the effective makespan is correct.\n");
	if(1 == technique) {
		printf("The rescheduling criterion used is %ld.\n", criterion);
		printf("The time limit for searching a solution is %.1f second(s)\n", reactiveTimeLimit);
	}
	if(3 == technique) {
		printf("The temporal domain for deciding when to select a new subset of activities and stop the selection is set to [%ld,%ld].\n", deltaTMinProgress, deltaTMaxProgress);
		printf("The uncertainty domain for deciding when to select a new subset of activities and stop the selection is set to [%.1f,%.1f].\n", sigmaTMinProgress, sigmaTMaxProgress);
		printf("The time limit for searching a solution is %.1f second(s)\n", progressiveTimeLimit);
	}
}





///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	try {
		IloInt i, j;
		IloBool init = IloTrue;
	  
		// DEFAULT PARAMETER VALUES
		IloInt _technique = 3;					// 3: progressive approach
		IloInt _seed = 1;
		IloNum _mDurBound = 5.0;
		IloNum _mTimeBreakBound = 5.0;
		IloNum _mBreakDurBound = 5.0;
		IloInt _thetaBreak = 2;
		IloInt _nbSimulations = 500;
		IloNum _execTimeStep = IloInfinity;
		IloBool _schedulerTest = IloFalse;
	  
		// REACTIVE TECHNIQUE PARAMETER (technique = 1)
		IloInt _criterion = 1;
		IloNum _reactiveTimeLimit = 1.0;				// _reactiveTimeLimit in seconds. This is the time spent for reoptimizing a partial solution.

		// PROACTIVE TECHNIQUE PARAMETER (technique = 2)
	  
		// PROGRESSIVE TECHNIQUE PARAMETERS (technique = 3) BY DEFAULT
		IloInt _deltaTMinProgress = 100000;
		IloInt _deltaTMaxProgress = 200000;
		IloNum _sigmaTMinProgress =  10000.0;
		IloNum _sigmaTMaxProgress = 100000.0;
		IloNum _progressiveTimeLimit = 1.0;				// _progressiveTimeLimit in seconds. This is the time spent for finding a partial solution.
		
		// USED PARAMETER VALUES
		IloInt technique = _technique;
		IloInt seed = _seed;
		IloNum mDurBound = _mDurBound;
		IloNum mTimeBreakBound = _mTimeBreakBound;
		IloNum mBreakDurBound = _mBreakDurBound;
		IloInt thetaBreak = _thetaBreak;
		IloInt nbSimulations = _nbSimulations;
		IloNum execTimeStep = _execTimeStep;
		IloBool schedulerTest = _schedulerTest;
		IloInt criterion = _criterion;
		IloInt deltaTMinProgress = _deltaTMinProgress;
		IloInt deltaTMaxProgress = _deltaTMaxProgress;
		IloNum sigmaTMinProgress = _sigmaTMinProgress;
		IloNum sigmaTMaxProgress = _sigmaTMaxProgress;
		IloNum reactiveTimeLimit = _reactiveTimeLimit;
		IloNum progressiveTimeLimit = _progressiveTimeLimit;

		// We retrieve the data of the scheduling problem instance.
		char* instanceName, * realizationName;
		if(argc > 1) {
			instanceName = argv[1];
			ifstream instanceStream(instanceName);
			int lineLimit = 65536;
			char buffer[65536];
			instanceStream.getline(buffer, lineLimit);
		//	printf("The buffer is %ld long.\n", strlen(buffer));
			if(strlen(buffer) > 0) {
				//printf("There are %ld process plans.\n", nbProcessPlans);
				//printf("There are %ld activities by process plan.\n", nbActivities);
				//printf("There are %ld possible resources associated with each activity.\n", nbResources);
				//printf("There are %ld resources.\n", nbTotalResources);
				//printf("The maximum tardiness cost is equal to %.14f.\n", Ktardiness);
				//printf("The maximum allocation cost is equal to %.14f.\n", Kallocation);
				if(argc > 2)
					realizationName = argv[2];
				ifstream realizationStream(realizationName);
				realizationStream.getline(buffer, lineLimit);
			//	printf("The buffer is %ld long.\n", strlen(buffer));
				if(0 == strcmp(buffer, instanceName)) {
					//printf("There are %ld process plans.\n", nbProcessPlans);
					//printf("There are %ld activities by process plan.\n", nbActivities);
					//printf("There are %ld possible resources associated with each activity.\n", nbResources);
					//printf("There are %ld resources.\n", nbTotalResources);
					//printf("The maximum tardiness cost is equal to %.14f.\n", Ktardiness);
					//printf("The maximum allocation cost is equal to %.14f.\n", Kallocation);

					printf("Parameter initialization\n");
					for(i = 3; (i < argc)&&(init); i += 2) {
						if(0 == strcmp(argv[i],"-technique")) {
							if(argc > i+1)
								technique = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-nbSimulations")) {
							if(argc > i+1)
								nbSimulations = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-execTimeStep")) {
							if(argc > i+1)
								execTimeStep = atof(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-schedulerTest")) {
							if(argc > i+1)
								schedulerTest = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-criterion")) {
							if(argc > i+1)
								criterion = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i],"-seed")) {
							if(argc > i+1)
								seed = atol(argv[i+1]);
							else
								init = IloFalse;
						} 
						else if(0 == strcmp(argv[i], "-deltaTMinProgress")) {
							if(argc > i+1)
								deltaTMinProgress = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-deltaTMaxProgress")) {
							if(argc > i+1)
								deltaTMaxProgress = atol(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-sigmaTMinProgress")) {
							if(argc > i+1)
								sigmaTMinProgress = atof(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-sigmaTMaxProgress")) {
							if(argc > i+1)
								sigmaTMaxProgress = atof(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-reactiveTimeLimit")) {
							if(argc > i+1)
								reactiveTimeLimit = atof(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-progressiveTimeLimit")) {
							if(argc > i+1)
								progressiveTimeLimit = atof(argv[i+1]);
							else
								init = IloFalse;
						}
						else if(0 == strcmp(argv[i], "-default")) {
							defaultParamPrint(_technique,
											  _seed,
											  _mDurBound,
											  _mTimeBreakBound,
											  _mBreakDurBound,
											  _thetaBreak,
											  _nbSimulations,
											  _execTimeStep,
											  _schedulerTest,
											  _criterion,
											  _deltaTMinProgress,
											  _deltaTMaxProgress,
											  _sigmaTMinProgress,
											  _sigmaTMaxProgress,
											  _reactiveTimeLimit,
											  _progressiveTimeLimit);
						}
						else {
							init = IloFalse;
						}
					}

					if(init) {
						paramPrint(technique,
								   seed,
								   mDurBound,
								   mTimeBreakBound,
								   mBreakDurBound,
								   thetaBreak,
								   nbSimulations,
								   execTimeStep,
								   schedulerTest,
								   criterion,
								   deltaTMinProgress,
								   deltaTMaxProgress,
								   sigmaTMinProgress,
								   sigmaTMaxProgress,
								   reactiveTimeLimit,
								   progressiveTimeLimit);

						IluNormalVariableI::Init(); // Only one normal probability distribution is initialized and it is then transformed.
						
						// Environment creation
						IloEnv env;

						// RANDOM GENERATOR CREATION
						IloRandom randGenerator(env);
						randGenerator.reSeed(seed);

						// Uncertain model creation
						IluExtendedModelI* uncModel = new (env) IluExtendedModelI(env,
																				  instanceName,
																				  mDurBound,
																				  mTimeBreakBound,
																				  mBreakDurBound,
																				  randGenerator);

						ifstream instanceStream(instanceName);
						instanceStream.getline(buffer, lineLimit);
						istrstream instanceLine(buffer, strlen(buffer));
						IloInt nbProcessPlans, nbActivities, nbResources, nbTotalResources;
						IloNum kMax, beta;
						instanceLine >> nbProcessPlans >> nbActivities >> nbResources >> nbTotalResources >> kMax >> beta;
						IloInt nbTotalActivities = nbProcessPlans * nbActivities;

					//	ifstream realizationStream(realizationName);
					//	realizationStream.getline(buffer, lineLimit);
					//	istrstream realizationLine(buffer, strlen(buffer));
						IloInt* effectActDurations = new (env) IloInt[nbTotalActivities + 1];
						effectActDurations[0] = 0; // Duration of the last dummy activity for computing the makespan
						for(i = 0; i < nbProcessPlans; i++) {
							realizationStream.getline(buffer, lineLimit);
							istrstream realizationLine(buffer, strlen(buffer));
							for(j = 0; j < nbActivities; j++) {		
								realizationLine >> effectActDurations[i*nbActivities+j+1];
							//	printf("The activitiy a%ld%ld has an effective duration equal to %ld.\n", i, j, effectActDurations[i*nbActivities+j+1]);
							}
						}
						IluRealizationI* realizationP = new (env) IluRealizationI(env, effectActDurations, nbTotalResources, nbTotalActivities);
					//	printf("All effective activity durations have been successfully retrieved.\n");
						
						for(i = 0; i < nbTotalResources; i++) {
							realizationStream.getline(buffer, lineLimit);
							if(strlen(buffer) > 0) {
								istrstream realizationLine(buffer, strlen(buffer));
								IloInt start, duration;
								IloInt lastStart = -1;
								do {										
									realizationLine >> start >> duration;
									if(start > lastStart) {
										IluBreakdownI* breakdown = new (env) IluBreakdownI(start, duration);
										realizationP->addBreakdown(i, breakdown);
									//	printf("Resource %ld -> a break is added: start = %ld and duration = %ld\n", i, start, duration);
										lastStart = start;
									}
								} while(0 == realizationLine.eof());
							}
						}
					//	printf("All effective breakdown start times and durations have been successfully retrieved.\n");
						IloNum effectMakespan, effectAllocCost, effectTardiCost, effectGlobalCost;
						IloInt* curAllocations = new (env) IloInt[nbTotalActivities + 1];
						for(i = 0; i < nbTotalActivities + 1; i++)
							curAllocations[i] = -1;

						// For checking the solution with Scheduler
						Allocation** effectAllocations = new (env) Allocation*[nbTotalResources];
						for(i = 0; i < nbTotalResources; i++)
							effectAllocations[i] = 0;

						char bufferTest[256];
						
						if(3 == technique) { // progressive approach

						/*	printf("First scheduler solution\n");
							IloSchedulerSolution solution(env);
							
							
							printf("Cost function creation\n");
							IloNumVar globalCost(env, 0.0, IloInfinity, ILOINT);
							IloObjective objective = IloMinimize(env, globalCost);
							solution.getSolution().add(objective);*/

							sprintf(bufferTest, "Progressive%s%sdeltaTMin%lddeltaTMax%ldsigmaTMin%.1fsigmaTMax%.1f", instanceName, realizationName,
							deltaTMinProgress, deltaTMaxProgress, sigmaTMinProgress, sigmaTMaxProgress);
							printf(bufferTest);
							printf("\n");
							ofstream streamTest(bufferTest); // A file named buffer is created.
							streamTest.precision(64);

							printf("Goal creation\n");
							IloGoal goal;

							printf("Histogram creations\n");
							IluHistogramI** currentLastHistogramBreakdown = new (env) IluHistogramI*[nbTotalResources];
							IluHistogramI** currentLastHistogramProcessPlan = new (env) IluHistogramI*[nbProcessPlans];
							IluHistogramI** currentLastHistogramResource = new (env) IluHistogramI*[nbTotalResources];

							IluHistogramI** allocLastHistogramBreakdown = new (env) IluHistogramI*[nbTotalResources];
							IluHistogramI** allocLastHistogramProcessPlan = new (env) IluHistogramI*[nbProcessPlans];
							IluHistogramI** allocLastHistogramResource = new (env) IluHistogramI*[nbTotalResources];

							IluHistogramI** selecLastHistogramBreakdown = new (env) IluHistogramI*[nbTotalResources];
							IluHistogramI** selecLastHistogramProcessPlan = new (env) IluHistogramI*[nbProcessPlans];
							IluHistogramI** selecLastHistogramResource = new (env) IluHistogramI*[nbTotalResources];

							IluHistogramI** lNSearchSelecLastHistogramBreakdown = new (env) IluHistogramI*[nbTotalResources];
							IluHistogramI** lNSearchSelecLastHistogramProcessPlan = new (env) IluHistogramI*[nbProcessPlans];
							IluHistogramI** lNSearchSelecLastHistogramResource = new (env) IluHistogramI*[nbTotalResources];


							for(i = 0 ; i < nbTotalResources ; i++) {
								currentLastHistogramBreakdown[i] = new (env) IluHistogramI(env, nbSimulations);
								currentLastHistogramResource[i] = new (env) IluHistogramI(env, nbSimulations);
								allocLastHistogramBreakdown[i] = new (env) IluHistogramI(env, nbSimulations);
								allocLastHistogramResource[i] = new (env) IluHistogramI(env, nbSimulations);
								selecLastHistogramBreakdown[i] = new (env) IluHistogramI(env, nbSimulations);
								selecLastHistogramResource[i] = new (env) IluHistogramI(env, nbSimulations);
								lNSearchSelecLastHistogramResource[i] = new (env) IluHistogramI(env, nbSimulations);
								lNSearchSelecLastHistogramBreakdown[i] = new (env) IluHistogramI(env, nbSimulations);
							}
		
							for(i = 0 ; i < nbProcessPlans ; i++) {
								currentLastHistogramProcessPlan[i] = new (env) IluHistogramI(env, nbSimulations);
								allocLastHistogramProcessPlan[i] = new (env) IluHistogramI(env, nbSimulations);
								selecLastHistogramProcessPlan[i] = new (env) IluHistogramI(env, nbSimulations);
								lNSearchSelecLastHistogramProcessPlan[i] = new (env) IluHistogramI(env, nbSimulations);
							}

							IloNum now = 0.0;
							IloNum lastTime = 0.0;

							for(j = 0; j < nbSimulations; j++) {
								for(i = 0; i < nbTotalResources; i++) {
									currentLastHistogramResource[i]->setValue(now, 1, j);
									allocLastHistogramResource[i]->setValue(now, 1, j);
									selecLastHistogramResource[i]->setValue(now, 1, j);
									lNSearchSelecLastHistogramResource[i]->setValue(now, 1, j);
								}
								for(i = 0; i < nbProcessPlans; i++) {
									currentLastHistogramProcessPlan[i]->setValue(now, 1, j);
									allocLastHistogramProcessPlan[i]->setValue(now, 1, j);
									selecLastHistogramProcessPlan[i]->setValue(now, 1, j);
									lNSearchSelecLastHistogramProcessPlan[i]->setValue(now, 1, j);
								}
							}

							//Computation of the start time for simulating resource breakdowns.
							IloNum mPeriodMax = 0.0;
							IloNum sigmaMin = IloInfinity;
							for(i = 0; i < nbTotalResources; i++) {
								IluUnaryResourceI* res = uncModel->getUUnaryResource(i);
								IloNum mTimeBreak = res->getMeanTimeBreak();
								IloNum mBreakDur = res->getMeanBreakDur();
								IloNum temp = mTimeBreak + mBreakDur;
								if(temp > mPeriodMax)
									mPeriodMax = temp;
								IloNum sTimeBreak = res->getSigmaTimeBreak();
								IloNum sBreakDur = res->getSigmaBreakDur();
								temp = sTimeBreak + sBreakDur;
								if(temp < sigmaMin)
									sigmaMin = temp;
							}

							IloNum tempTime;
							if(sigmaMin > 0.0)
								tempTime = -thetaBreak * mPeriodMax * mPeriodMax / sigmaMin;
						//	printf("Resource breakdown generation starts at %.14f.\n", tempTime);

							
							for(j = 0; j < nbTotalResources; j++) {
								IluUnaryResourceI* res = uncModel->getUUnaryResource(j);
								IluRandomVariableI* timeBreakVar = res->getInterBreaksVar();
								IluRandomVariableI* breakDurVar = res->getBreaksDurationVar();
								for(i = 0; i < nbSimulations; i++) {
									IloNum endTime, lastEndTime;
									if(0.0 == sigmaMin) {
										IluBreakdownStackI* tempStacks = realizationP->getBreakdownStacks();
										IluBreakdownI* tempFirst = tempStacks[j].getFirst();
										IloNum tempEnd = IloNum(tempFirst->getStartTime() + tempFirst->getDuration());
										if(tempEnd > now)
											tempTime = IloNum(tempEnd) - timeBreakVar->getValue() - breakDurVar->getValue();
										else
											tempTime = (IloNum)tempEnd;
									}

									endTime = tempTime;
									do{	
										lastEndTime = endTime;
										IloNum startTime =  lastEndTime + timeBreakVar->getValue();
										endTime = startTime + breakDurVar->getValue();
									//	printf("Resource %ld: a breakdown starts at %.14f and finishes at %.14f.\n", j, startTime, endTime);
									}
									while(endTime <= now); // We generate breakdowns until 0.
								//	printf("Resource %ld: last breakdown end time = %.14f\n", j, lastEndTime);
									currentLastHistogramBreakdown[j]->setValue(lastEndTime, 1, i);
									allocLastHistogramBreakdown[j]->setValue(lastEndTime, 1, i);
									selecLastHistogramBreakdown[j]->setValue(lastEndTime, 1, i);
									lNSearchSelecLastHistogramBreakdown[j]->setValue(lastEndTime, 1, i);
								}
							}

							printf("Current frontier, allocation frontier, and selection frontier creations\n");
							IluFrontierI* currentFrontier = new (env) IluFrontierI(nbProcessPlans,
																				   nbTotalResources,
																				   currentLastHistogramBreakdown,
																				   currentLastHistogramProcessPlan,
																				   currentLastHistogramResource);
							IluFrontierI* allocationFrontier = new (env) IluFrontierI(nbProcessPlans,
																					  nbTotalResources,
																					  allocLastHistogramBreakdown,
																					  allocLastHistogramProcessPlan,
																					  allocLastHistogramResource);

							IluFrontierI* selectionFrontier = new (env) IluFrontierI(nbProcessPlans,
																					 nbTotalResources,
																					 selecLastHistogramBreakdown,
																					 selecLastHistogramProcessPlan,
																					 selecLastHistogramResource);

							IluFrontierI* lNSearchSelecFrontier = new (env) IluFrontierI(nbProcessPlans,
																						 nbTotalResources,
																						 lNSearchSelecLastHistogramBreakdown,
																						 lNSearchSelecLastHistogramProcessPlan,
																						 lNSearchSelecLastHistogramResource);


							printf("Simulator (for simulating non executed, just allocated activities during large neighborhood search) creation.\n");
							IluSimulatorI* lNSearchSimP = new (env) IluSimulatorI(env, allocationFrontier, selectionFrontier, 0);
							
							printf("Local search solver creation\n");
							// 10 CPU seconds are given to search for solutions.
							IluLocalSearchSolverI* lSSolver = new (env) IluLocalSearchSolverI(uncModel, goal, 10.0, lNSearchSimP, lNSearchSelecFrontier);



							printf("Simulator (for simulating non executed, allocated schedule execution) creation.\n");
							IluSimulatorI* simP = new (env) IluSimulatorI(env, currentFrontier, allocationFrontier, 0);

							printf("Progressive scheduler creation\n");
							IluProgressiveSchedulerI* progressiveSchedP = new (env) IluProgressiveSchedulerI(lSSolver,
																											 deltaTMinProgress,
																											 deltaTMaxProgress,
																											 sigmaTMinProgress,
																											 sigmaTMaxProgress,
																											 simP,
																											 curAllocations,
																											 effectAllocations);

							printf("Time event manager creation; breakdown events are also created.\n");
							IluTimeEventManagerI* tEMgrP = new (env) IluTimeEventManagerI(env, progressiveSchedP, IloIntMax, realizationP, uncModel);

							IloNum nextExecutionTime;
							IloInt activityIndex = IloIntMax;
							IloInt* processPlanIds = uncModel->getProcessPlanIds();

							IluSimulatorOneActivityI* simulatorOneAct = progressiveSchedP->getSimulatorOneActivity();
							printf("Schedule execution starts.\n");
							IluSimulatorI* allocSim = progressiveSchedP->getSimulator();

							IloNum lastMakespan = 0.0;

							do {
						// schedule execution starts

								if(progressiveSchedP->timeForSelection(lastTime)) {
									printf("\nA selection process starts at time %.14f.\n", lastTime);
									progressiveSchedP->updateCurFrontier();
									progressiveSchedP->initializeSolution();
									IloInt nbSelectedActivities = progressiveSchedP->selectActivities(lastTime);								
									printf("%ld activities have been selected.\n", nbSelectedActivities);

									sprintf(bufferTest, "A selection process occurs at time %.1f: %ld activities have been selected.", lastTime, nbSelectedActivities);
									streamTest << bufferTest << endl;
									printf("The optimization of the solution of the subproblem starts.\n");
									lSSolver->solve(progressiveSchedP->getSolution(), lastTime, nbSimulations, progressiveSchedP->getFirstPendingItem());






									printf("The precedence graph is updated.\n");
									progressiveSchedP->updatePG(env, lastTime);

									allocSim->clean();									// All histograms associated with non frontier nodes are cleaned.
									printf("Simulation starts.\n");
									IloNum tempTime = env.getTime();
									allocSim->simulate(lastTime, nbSimulations);
									IloNum tempTime2 = env.getTime();
									printf("Time spent = %.14f second(s)\n", tempTime2 - tempTime);
									if(0 != allocSim->getHistogram(0))
										printf("Time: %ld; makespan: mean = %.14f and standard deviation = %.14f\n", (IloInt)lastTime, allocSim->getHistogram(0)->getAverage(), allocSim->getHistogram(0)->getStandardDev());

								// Some controllable events may be added to the list.
									if(tEMgrP->updateControlStack(allocSim, lastTime)) {
										tEMgrP->executeNext((IlcInt)lastTime);
										allocSim->recycleExecutedNodes(lastTime);				// The precedence graph is updated
										allocSim->clean();									// All histograms associated with non frontier nodes are cleaned.
										printf("Simulation starts.\n");
										IloNum tempTime = env.getTime();
										allocSim->simulate(lastTime, nbSimulations);
										IloNum tempTime2 = env.getTime();
										printf("Time spent = %.14f second(s)\n", tempTime2 - tempTime);

										if(0 != allocSim->getHistogram(0)) {
									//		if((0 != lastMakespan)&&(allocSim->getHistogram(0)->getAverage() != lastMakespan)) {
									//			sprintf(bufferTest, "Warning: makespan changes.");
									//			printf("Warning: makespan changes.\n");
									//			streamTest << bufferTest << endl;
									//		}
											printf("Time: %ld; makespan: mean = %.14f and standard deviation = %.14f\n", (IloInt)lastTime, allocSim->getHistogram(0)->getAverage(), allocSim->getHistogram(0)->getStandardDev());
									//		lastMakespan = allocSim->getHistogram(0)->getAverage();
										}
									}
									printf("\nSchedule execution resumes.\n");
								}

							// Some controllable events may be added to the list.
								tEMgrP->updateControlStack(allocSim, now);

							// Is their at least one activity that can be executed now? Some controllable and contingent events are added to the time event manager lists.
								nextExecutionTime = tEMgrP->executeNext((IlcInt)now);

								allocSim->recycleExecutedNodes(now);				// The precedence graph is updated
								allocSim->clean();									// All histograms associated with non frontier nodes are cleaned.
								printf("Simulation starts.\n");
								IloNum tempTime = env.getTime();
								allocSim->simulate(now, nbSimulations);
								IloNum tempTime2 = env.getTime();
								printf("Time spent = %.14f second(s)\n", tempTime2 - tempTime);

								if(0 != allocSim->getHistogram(0)) {
							//		if((0 != lastMakespan)&&(allocSim->getHistogram(0)->getAverage() != lastMakespan)) {
							//			sprintf(bufferTest, "Warning: makespan changes.");
							//			printf("Warning: makespan changes.\n");
							//			streamTest << bufferTest << endl;
							//		}
									printf("Time: %ld; makespan: mean = %.14f and standard deviation = %.14f\n", (IloInt)now, allocSim->getHistogram(0)->getAverage(), allocSim->getHistogram(0)->getStandardDev());
							//		lastMakespan = allocSim->getHistogram(0)->getAverage();
								}

								lastTime = now;
								now += execTimeStep;
								now = IloMin(now, nextExecutionTime);
							} while((tEMgrP->executableEvents())&&(now >= 0));		// While there are some events in stacks and now is positive (no dummy activity).
							printf("This is the end of schedule execution simulation.\n");
							effectTardiCost = progressiveSchedP->computeEffectTardiCost();
							effectAllocCost = progressiveSchedP->getEffectAllocCost();
							effectMakespan = allocSim->getHistogram(0)->getAverage();
							streamTest.close();
						}
						printf("The effective allocation cost is equal to %.14f.\n", effectAllocCost);
						printf("The effective tardiness cost is equal to %.14f.\n", effectTardiCost);
						effectGlobalCost = effectAllocCost + beta * effectTardiCost;
						printf("The effective global cost is equal to %.14f.\n", effectGlobalCost);

						if(IloTrue == schedulerTest) {
							printf("The deterministic problem is now solved by Scheduler.\n");
							// For checking solution with Scheduler
							
							sprintf(bufferTest, "SchedTest%s%sdeltaTMin%lddeltaTMax%ldsigmaTMin%.1fsigmaTMax%.1f", instanceName, realizationName,
																						deltaTMinProgress, deltaTMaxProgress, sigmaTMinProgress, sigmaTMaxProgress);
							printf(bufferTest);
							printf("\n");
							ofstream streamSchedTest(bufferTest); // A file named buffer is created.
							streamSchedTest.precision(64);
						
							IloNumVar detMakespan;
							IloEnv env2;
							IloModel checkModel = DefineModel(env2,
															  instanceName,
															  realizationName,
															  effectAllocations,
															  detMakespan);
							checkModel.add(IloMinimize(env2, detMakespan));
							IloSolver solver(checkModel);
							IloGoal detGoal = IloRankForward(env,
															 detMakespan,
															 IloSelResMinGlobalSlack,
															 IloSelFirstRCMinStartMax);
							solver.startNewSearch(detGoal);
							if(solver.next()) {
								printf("A solution has been found.\n");
								IloNum bestMakespan = solver.getMin(detMakespan);
								printf("Best solution found with a makespan = %f\n", bestMakespan);
								PrintSolution(solver);
								if(bestMakespan == effectMakespan) {
									printf("Test is ok.\n");
									sprintf(bufferTest, "Test is ok.");
									streamSchedTest << bufferTest << endl;
									sprintf(bufferTest, "Effective makespan = %f", effectMakespan);
									streamSchedTest << bufferTest << endl;
								}
								else {
									printf("Test has failed!\n");
									sprintf(bufferTest, "Test has failed!");
									streamSchedTest << bufferTest << endl;
								}
							}
							else {
								printf("No solution has been found!\n");
								sprintf(bufferTest, "Test has failed!");
								streamSchedTest << bufferTest << endl;
							}
							env2.end();
						}
						
						env.end(); // All the objects dynamically created are destroyed, i.e. some memory is dumped.
						
						IluNormalVariableI::End(); // The normal distribution is here erased.

					}
				}
				else
					init = IloFalse;
			}
			else
				init = IloFalse;
		}
		else
			init = IloFalse;
		if(init == IloFalse) {
			printf("Paramaters have not been correctly set!\n");
			defaultParamPrint(_technique,
							  _seed,
							  _mDurBound,
							  _mTimeBreakBound,
							  _mBreakDurBound,
							  _thetaBreak,
							  _nbSimulations,
							  _execTimeStep,
							  _schedulerTest,
							  _criterion,
							  _deltaTMinProgress,
							  _deltaTMaxProgress,
							  _sigmaTMinProgress,
							  _sigmaTMaxProgress,
							  _reactiveTimeLimit,
							  _progressiveTimeLimit);
		}

	} catch(IloException& exc) { cout << exc << endl; }
	return 0;
}