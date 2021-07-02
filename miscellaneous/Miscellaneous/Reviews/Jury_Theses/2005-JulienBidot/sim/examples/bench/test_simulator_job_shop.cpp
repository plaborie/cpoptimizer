#include <ilsched/iloscheduler.h>
#include <ilsim/ilusimulator.h>
#include <ilsim/ilupgsim.h>
/*#include <stdlib.h>
#include <stdio.h>

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

ILOSTLBEGIN



class EventI {
public:
	IloInt  _id;
	IloNum  _t;
	IloInt _state;								// 1: activity start time
												// 2: activity end time
												// 3: resource breakdown start time
												// 4: resource breakdown end time
	EventI(IloInt id, IloNum t, IloBool state)
		:_id (id), _t(t), _state(state){}
	~EventI(){}
};


static int compareTime(const void* fir, const void* sec) {
  IloNum firVal = (*((EventI**) fir))->_t;
  IloNum secVal = (*((EventI**) sec))->_t;
  return (firVal < secVal)?-1:((firVal > secVal)?1:0);
}





////////////////////////////////////////////////////////////////////////////////////////
/////
/////		PROBLEM DEFINITION
/////
/////
////////////////////////////////////////////////////////////////////////////////////////

IloModel
DefineModel(const IloEnv& env,
			const char* fileName,
			IloNumVar& makespan,
			IloInt* breakStarts,
			IloInt* breakDurations,
			IloInt* breakPeriods) {
	IloModel model(env);

	IloInt numberOfJobs;
	IloInt numberOfResources;

	ifstream file(fileName);
	int lineLimit = 1024;
	char buffer[1024];
	file.getline(buffer, lineLimit);
	istrstream line(buffer, strlen(buffer));
	line >> numberOfJobs >> numberOfResources;

	IloInt* resourceNumbers = new (env) IloInt[numberOfJobs * numberOfResources];
	IloInt* durations = new (env) IloInt[numberOfJobs * numberOfResources];

	IloInt i, j, k;
	for (i = 0, k = 0; i < numberOfJobs; i++) {
		file.getline(buffer, lineLimit);
		istrstream jobLine(buffer, strlen(buffer));
		for (j = 0; j < numberOfResources; j++) {
			jobLine >> resourceNumbers[k] >> durations[k];
			k++;
		}
	}	
  
	IloInt numberOfActivities = numberOfJobs * numberOfResources;
  // CREATE THE MAKESPAN VARIABLE.
	IloInt maxDuration = 0;
	for (k = 0; k < numberOfActivities; k++) {
		if(durations[k] > maxDuration)
			maxDuration = durations[k];
	}

	IloInt* currentBreakStart = new (env) IloInt[numberOfResources];	// startBreak is the current breakdown start time.
	IloInt* currentBreakEnd = new (env) IloInt[numberOfResources];
	for(i = 0 ; i < numberOfResources ; i++) {
		currentBreakStart[i] = breakStarts[i];
		currentBreakEnd[i] = currentBreakStart[i] + breakDurations[i];
	}

	IloNum activityEnd = 0.0;		// activityEnd is the date at which the preceding virtual activity ends execution by taking into account all current breakdowns.
	IloNum max = (IloNum)IloIntMax;
	IloNum temp = (IloNum)maxDuration;		// maxDuration is constant and is equal to the duration of one activity.
	for(j = numberOfActivities; j > 0; j--) {
		IloInt activityEndMax = (IloInt)activityEnd + maxDuration;
		IloNum temp1 = (IloNum)activityEnd + temp;
		if(temp1 > max)							// We assess if the integer capacity is not exceeded.
			printf("The horizon is too big!\n");
		IloInt biggestActivityEnd = activityEndMax;
		for(i = 0 ; i < numberOfResources ; i++){
			IloInt currentActivityEnd = activityEndMax;
			IloInt currentDuration = maxDuration;
			IloInt min = (IloInt)activityEnd;
			while(currentBreakStart[i] < currentActivityEnd) {
				if((currentBreakStart[i] < 0)||(currentBreakEnd[i] < currentBreakStart[i]))
					printf("The process for computing horizon does not work correctly.\n");
				if(currentBreakStart[i] > min)
					min = currentBreakStart[i];
				if((IloInt)activityEnd + currentBreakEnd[i] + currentDuration - min > currentActivityEnd) {
					currentActivityEnd = (IloInt)activityEnd + currentBreakEnd[i] + currentDuration - min;
					currentDuration = currentDuration + currentBreakEnd[i] - min;
				}
				currentBreakStart[i] += breakPeriods[i];
				currentBreakEnd[i] = currentBreakStart[i] + breakDurations[i];
			}
			if(currentActivityEnd > biggestActivityEnd)
				biggestActivityEnd = currentActivityEnd;
		}
		activityEnd = (IloNum)biggestActivityEnd;
	}

	IloNum horizon = activityEnd;

	printf("The horizon is equal to %ld.\n", (IloInt)horizon);
	makespan = IloNumVar(env, 0.0, horizon, ILOINT);

  // CREATE THE RESOURCES WITH BREAKS.
	IloSchedulerEnv schedEnv(env);
	schedEnv.getResourceParam().setCapacityEnforcement(IloHigh); // Edge-finder, propagation of resource constraints

	
	IloUnaryResource *resources = new (env) IloUnaryResource[numberOfResources];
	for (i = 0 ; i < numberOfResources ; i++) {
		resources[i] = IloUnaryResource(env);
		resources[i].setObject((void*)(i));
		resources[i].addPeriodicBreak(breakStarts[i], breakDurations[i], breakPeriods[i], horizon);
	}

  // CREATE THE ACTIVITIES AND ADD TEMPORAL AND RESOURCE CONSTRAINTS.
	for (i = 0, k = 0; i < numberOfJobs; i++) {
		IloActivity previousActivity;
	    for (j = 0; j < numberOfResources; j++, k++) {
			IloActivity activity(env, durations[k]);
			sprintf(buffer, "J%ldS%ldR%ld", i, j, resourceNumbers[k]);
			activity.setName(buffer);
			activity.setObject((void*)(numberOfResources * i + j + 1));
//			cout << activity.getName() << " processing time between ";
//			cout << activity.getProcessingTimeVariable().getLB() << " and " << activity.getProcessingTimeVariable().getUB();
//			cout << "." << endl;
			activity.setBreakable();					// The activity is breakable.
			IloResourceConstraint rct = activity.requires(resources[resourceNumbers[k]]);
			model.add(rct);
			if (0 != j) {
				IloPrecedenceConstraint tc = activity.startsAfterEnd(previousActivity);
				model.add(tc);
			}
			previousActivity = activity;
		}
		model.add(previousActivity.endsBefore(makespan));
	}
	// RETURN THE MODEL.
	return model;
}


////////////////////////////////////////////////////////////////////////////////////////
/////
/////		PRINTING OF SOLUTIONS
/////
/////
////////////////////////////////////////////////////////////////////////////////////////

void
PrintSolution(IloEnv env, IlcScheduler sched) {
	for(IloIterator<IloActivity> ite(env) ; ite.ok() ; ++ite)
		env.out() << sched.getActivity(*ite) << endl;
}






////////////////////////////////////////////////////////////////////////////////////////
/////
/////		MAIN
/////
/////
////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	try {
		const char* fileName = "default.graph";
		if (argc > 1)
			fileName = argv[1];
		printf("We solve a job-shop problem called %s.\n", fileName);

		IloNum alpha = 0.1;		// uncertainty degree
		if (argc > 2)
			alpha = atof(argv[2]);

		IloInt n = 1000;		// number of simulations
		if (argc > 3)
			n = atol(argv[3]);

		IloInt currentSeed = 1;		// random generator initial seed
		if (argc > 4)
			currentSeed = atol(argv[4]);

		IloInt activeEvents = 1;
		if (argc > 5)
			activeEvents = atol(argv[5]);

		printf("Uncertainty degree = %.4f\n", alpha);
		printf("%ld simulations will be run.\n", n);
		printf("Seed = %ld\n", currentSeed);

		IloEnv env;

		IloNumVar makespan;

		IloInt i, j, nbJobs, nbResources;

		// Random number generator creations
		IloRandom randGenerator(env);

		randGenerator.reSeed(currentSeed);

		// Breakdown starts, durations, and periods are randomly generated.
		ifstream file(fileName);
		int lineLimit = 1024;
		char buffer[1024];
		file.getline(buffer, lineLimit);
		istrstream line(buffer, strlen(buffer));
		line >> nbJobs >> nbResources;
		IloInt nbTotalActivities = nbJobs * nbResources;

		IloInt* breakStarts = new (env) IloInt[nbResources];
		IloInt* breakDurations = new (env) IloInt[nbResources];
		IloInt* breakPeriods = new (env) IloInt[nbResources];

		for (i = 0 ; i < nbResources; i++) {
			breakDurations[i] = 1 + randGenerator.getInt(100);
			breakPeriods[i] = breakDurations[i] + 1 + randGenerator.getInt(400);
			breakStarts[i] = randGenerator.getInt(breakPeriods[i]) - breakPeriods[i];
	/*	// No breakdown occurs.	
			breakDurations[i] = IloIntMax;
			breakPeriods[i] = IloIntMax;
			breakStarts[i] = IloIntMax;*/
		//	printf("Resource %ld breaks down the first time at %ld; each breakdown lasts %ld time units;", i, breakStarts[i], breakDurations[i]);
		//	printf(" there are %ld time units between two consecutive breakdowns.\n", breakPeriods[i] - breakDurations[i]);
		}

		IloModel model = DefineModel(env, fileName, makespan, breakStarts, breakDurations, breakPeriods);
		model.add(IloMinimize(env, makespan));

		IloSolver solver(model);
	//	solver.useHeap(IlcFalse);		// C++ memory allocator is used.
		IloGoal goal = IloRankForward(env,
									  makespan,
									  IloSelResMinGlobalSlack,
									  IloSelFirstRCMinStartMax);

		solver.startNewSearch(goal);

		IloNum bestMakespan;

		IlcScheduler sched(solver);
		
		solver.next();
		bestMakespan = solver.getMin(makespan);
		printf("Best solution found with a makespan = %f\n", bestMakespan);

	// OUTPUT FILE CREATION
		sprintf(buffer, "%sSeed%ldAlpha%.2fSim%ld.dat", fileName, currentSeed, alpha, n);
		printf(buffer);
		printf("\n");
		ofstream stream(buffer); // A file named buffer is created.
		stream.precision(64);

		stream << "Best solution found with a makespan = " << bestMakespan << endl;
		
	//	PrintSolution(env, sched);

		randGenerator.reSeed(1);
		
		// Creation of a digraph representing a job-shop scheduling solution

		// We suppose we run n simulations.

		IluNormalVariableI::Init();

		IluNormalVariableI** inter = new (env) IluNormalVariableI*[nbResources];
		IluNormalVariableI** breakDur = new (env) IluNormalVariableI*[nbResources];
		IloNum* breakdownStarts = new (env) IloNum[nbResources];
		for(i = 0 ; i < nbResources ; i++) {
			IloNum min, max, avg, stdev;
			min = 1.0;
			max = 2 * (breakPeriods[i] - breakDurations[i]);
			avg = breakPeriods[i] - breakDurations[i];
			stdev = avg * alpha;
		//	printf("Resource %ld: mean duration between two consecutive breakdowns = %.14f\n", i, avg);
			inter[i] = new (env) IluNormalVariableI(env, randGenerator, min, max, avg, stdev);
			min = 1.0;
			max = 2 * breakDurations[i];
			avg = breakDurations[i];
			stdev = avg * alpha;
			breakDur[i] = new (env) IluNormalVariableI(env, randGenerator, min, max, avg, stdev);
		//	printf("Resource %ld: mean breakdown duration = %.14f\n", i, avg);
		}

	/*	for(i = 0 ; i < nbResources ; i++) {
			IloNum min, max, avg, stdev;
			min = 100.0 + randGenerator.getFloat() * 199;
			max = 300.0 + randGenerator.getFloat() * 199;
			avg = min + (max - min)/2;
			stdev = avg * alpha;*/
		/*	min = IloInfinity;						// without breakdowns
			max = IloInfinity;
			avg = IloInfinity;
			stdev = 0.0;*/
		//	printf("Resource %ld: mean duration between two consecutive breakdowns = %.14f\n", i, avg);
	/*		inter[i] = new (env) IluNormalVariableI(env, randGenerator, min, max, avg, stdev);
		}*/

	/*	IluNormalVariableI** breakDur = new (env) IluNormalVariableI*[nbResources];
		for(i = 0 ; i < nbResources ; i++) {
			IloNum min, max, avg, stdev;
			min = 1.0 + randGenerator.getFloat() * 99;
			max = 100.0 + randGenerator.getFloat() * 99;
			avg = min + (max - min)/2;
			stdev = avg * alpha;*/
		/*	min = 0.0;								// without breakdowns
			max = 0.0;
			avg = 0.0;
			stdev = 0.0;*/
		//	printf("Resource %ld: mean breakdown duration = %.14f\n", i, avg);
	/*		breakDur[i] = new (env) IluNormalVariableI(env, randGenerator, min, max, avg, stdev);
		}*/


		// Histogram arrays for the start frontier
		IluHistogramI** lastBreakdownEndTime = new (env) IluHistogramI*[nbResources];
		IluHistogramI** lastEndTimeProcessPlan = new (env) IluHistogramI*[nbJobs];
		IluHistogramI** lastEndTimeResource = new (env) IluHistogramI*[nbResources];

		for(i = 0; i < nbResources; i++)
			lastBreakdownEndTime[i] = new (env) IluHistogramI(env, n);
		
		for(i = 0; i < nbJobs; i++) {
			lastEndTimeProcessPlan[i] = new (env) IluHistogramI(env, n);
			for(j = 0; j < n; j++)
				lastEndTimeProcessPlan[i]->setValue(0.0, 1, j);
		}
		
		for(i = 0; i < nbResources; i++) {
			lastEndTimeResource[i] = new (env) IluHistogramI(env, n);
			for(j = 0; j < n; j++)
				lastEndTimeResource[i]->setValue(0.0, 1, j);

		}
		
	/*	IloNum endTime, lastEndTime, startTime;
		IloInt tempSeed;
		for(i = 0 ; i < n ; i++) {
			for(j = 0 ; j < nbResources ; j++) {
				endTime = -100000.0;
				do{	
					lastEndTime = endTime;
					startTime = inter[j]->getValue() + lastEndTime;
					endTime = startTime + breakDur[j]->getValue();
				//	printf("Resource %ld: a breakdown starts at %.14f and finishes at %.14f.\n", j, startTime, endTime);
				}
				while(endTime <= 0.0); // We generate breakdowns until date 0.
			//	printf("Resource %ld: last breakdown end time = %.14f\n", j, lastEndTime);
				tempSeed = randGenerator.getInt(IloIntMax) + 1;
				lastBreakdownEndTime[j]->setValue(lastEndTime, tempSeed, i);
			}
		}*/
		for(i = 0 ; i < n ; i++)
			for(j = 0 ; j < nbResources ; j++)
				lastBreakdownEndTime[j]->setValue(breakStarts[j] - breakPeriods[j] + breakDurations[j], 1, i);

		IluFrontierI* startFrontier = new (env) IluFrontierI(nbJobs,
															 nbResources,
															 lastBreakdownEndTime,
															 lastEndTimeProcessPlan,
															 lastEndTimeResource);

		// Histogram arrays for the end frontier
		IluHistogramI** lastBreakdownEndTimeEnd = new (env) IluHistogramI*[nbResources];
		IluHistogramI** lastEndTimeProcessPlanEnd = new (env) IluHistogramI*[nbJobs];
		IluHistogramI** lastEndTimeResourceEnd = new (env) IluHistogramI*[nbResources];
		
		for(i = 0 ; i < nbResources ; i++)
			lastBreakdownEndTimeEnd[i] = new (env) IluHistogramI(env, n);
		
		for(i = 0 ; i < nbJobs ; i++)
			lastEndTimeProcessPlanEnd[i] = new (env) IluHistogramI(env, n);
		
		for(i = 0 ; i < nbResources ; i++)
			lastEndTimeResourceEnd[i] = new (env) IluHistogramI(env, n);

		IluFrontierI* endFrontier = new (env) IluFrontierI(nbJobs,
														   nbResources,
														   lastBreakdownEndTimeEnd,
														   lastEndTimeProcessPlanEnd,
														   lastEndTimeResourceEnd);

	/*	IluPrecGraphI* g = new (env) IluPrecGraphI();
		g->readJobShop(randGenerator, fileName, alpha);

		// Precedences of the schedule solution
		for (IlcUnaryResourceIterator resIte(sched); resIte.ok(); ++resIte) {
			IlcUnaryResource res = *resIte;
		//	printf("Resource with index %ld\n", res.getObject());
			IlcResourceConstraint rct = res.getSetupRC();
		//	printf("Activity %s with node index %ld\n", rct.getActivity().getName(), rct.getActivity().getObject());
			while(rct.hasNextRC()) {
				IlcResourceConstraint prevRct = rct;
				rct = rct.getNextRC();
		//		printf("Previous activity %s with node index %ld\n", prevRct.getActivity().getName(), prevRct.getActivity().getObject());
		//		printf("Activity %s with node index %ld\n", rct.getActivity().getName(), rct.getActivity().getObject());
				IloInt index = (IloInt)(prevRct.getActivity().getObject());
				IloInt next = (IloInt)(rct.getActivity().getObject());
				g->addArc(index, next);
			//	printf("An arc is added between node %ld and node %ld.\n", index, next);
			}
		}

		IluHistogramI* h0 = new (env) IluHistogramI(env, n);
		g->setHistogram(0,h0);
	
		g->topologicalSort();

		h0->clean();

		IloNum temp = env.getTime();
		g->simulate(0.0, n);
		IloNum temp1 = env.getTime();
		printf("Time spent for running %ld simulations: %.14f\n", n, temp1 - temp);
		
		printf("Makespan: mean = %.14f and standard deviation = %.14f\n", h0->getAverage(), h0->getStandardDev());*/




		IluSimulatorI* sim = new (env) IluSimulatorI(env, startFrontier, endFrontier, nbTotalActivities + 1);
		for(i = 0 ; i < nbResources ; i++) {
			sim->setBreakdownDurationVariable(i, breakDur[i]);
			sim->setInterBreakdownDurationVariable(i, inter[i]);
		}
		sim->readJobShop(randGenerator, fileName, alpha);

		// Precedences of the schedule solution
		for (IlcUnaryResourceIterator resIte(sched); resIte.ok(); ++resIte) {
			IlcUnaryResource res = *resIte;
		//	printf("Resource with index %ld\n", res.getObject());
			IlcResourceConstraint rct = res.getSetupRC();
		//	printf("Activity %s with node index %ld\n", rct.getActivity().getName(), rct.getActivity().getObject());
			while(rct.hasNextRC()) {
				IlcResourceConstraint prevRct = rct;
				rct = rct.getNextRC();
		//		printf("Previous activity %s with node index %ld\n", prevRct.getActivity().getName(), prevRct.getActivity().getObject());
		//		printf("Activity %s with node index %ld\n", rct.getActivity().getName(), rct.getActivity().getObject());
				IloInt index = (IloInt)(prevRct.getActivity().getObject());
				IloInt next = (IloInt)(rct.getActivity().getObject());
				sim->addArc(index, next);
			//	printf("An arc is added between %ld and %ld.\n", index, next);
			}
		}

		IluHistogramI* h0 = new (env) IluHistogramI(env, n);		// This histogram permits us to get the mean and standard deviation of the makespan.
		sim->setHistogram(0, h0);

		IluHistogramI** h = new (env) IluHistogramI*[nbTotalActivities];
		for(i = 1; i < nbTotalActivities + 1; i++) {
			h[i-1] = new (env) IluHistogramI(env, n);
			sim->setHistogram(i, h[i-1]);
		}

		sim->topologicalSort();
		sim->addFrontierArcs();

		IloInt nbBreakdowns = 0;
		IloInt* nbResourceBreakdowns = new (env) IloInt[nbResources];
		for(i = 0 ; i < nbResources ; i++) {
			IloInt temp = breakStarts[i];
			IloInt nb = 0;
			while(temp < bestMakespan) {
				nb++;
				temp += breakPeriods[i];
			}
			nbResourceBreakdowns[i] = nb;
			nbBreakdowns += nb;
		}

		IloInt nbEvents = 2 * nbTotalActivities + 2 + 2 * nbBreakdowns;
		
		EventI** events = new (env) EventI*[nbEvents];

		EventI* startMakespanEvent = new (env) EventI(0, bestMakespan, 1);
		events[0] = startMakespanEvent;
		EventI* endMakespanEvent = new (env) EventI(0, bestMakespan, 2);
		events[1] = endMakespanEvent;
		i = 2;
		for(IlcActivityIterator actIte(sched); actIte.ok(); ++actIte) {
			IlcActivity act = *actIte;
			IloNum start = (IloNum)(act.getStartMin());
			EventI* startEvent = new (env) EventI((IloInt)(act.getObject()), start, 1);
			events[i] = startEvent;
		//	printf("Activity %ld starts at %.14f.\n", (IloInt)(act.getObject()), events[i]->_t);
			IloNum end = (IloNum)(act.getEndMin());
			EventI* endEvent = new (env) EventI((IloInt)(act.getObject()), end, 2);
			events[i + 1] = endEvent;
		//	printf("Activity %ld ends at %.14f.\n", (IloInt)(act.getObject()), events[i + 1]->_t);
			i += 2;
		}

		IloInt index = 2 * nbTotalActivities + 2;
		for(i = 0 ; i < nbResources ; ++i) {
			for(j = 0 ; j < nbResourceBreakdowns[i] ; j++) {
				EventI* startBreakEvent = new (env) EventI(i, breakStarts[i] + j * breakPeriods[i], 3);
				events[index] = startBreakEvent;
			//	printf("Resource %ld breaks down at %.14f.\n", i, events[index]->_t);
				index++;
				EventI* endBreakEvent = new (env) EventI(i, breakStarts[i] + breakDurations[i] + j * breakPeriods[i], 4);
				events[index] = endBreakEvent;
			//	printf("Resource %ld is repaired at %.14f.\n", i, events[index]->_t);
				index++;
			}
		}

		qsort(events, nbEvents, sizeof(EventI*), compareTime);
		//	void  qsort ( void * base, size_t num, size_t width, int (*fncompare)(const void *, const void *) );
		//	Sort using quicksort algorithm.
		//	This function uses an implementation of the quicksort algorithm to sort the num elements of an array pointed by base,
		//	each element has the specified width in bytes.
		//	The method used to compare each pair of elements is provided by the caller to this function with fncompare parameter,
		//	that is a function called one or more times during the sort process. 

		IloNum now = 0.0;
		printf("\nSchedule execution simulation starts.\n");
		for(i = 0 ; i < nbEvents ;) {
			for(; now < events[i]->_t ; now++) {
				h0->clean();
				for(j = 0; j < nbTotalActivities; j++)
					h[j]->clean();
			//	IloNum temp = env.getTime();
				sim->simulate(now, n);
			//	IloNum temp1 = env.getTime();
			//	printf("%.1f\t... avg. = %.14f;\tst. dev. = %.14f\ttime spent = %.14f second(s)\n", now, h0->getAverage(), h0->getStandardDev(), temp1 - temp);
			//	for(j = 1; j < nbTotalActivities + 1; j++)
			//		printf("Activity %ld start time: mean = %.14f strandard deviation = %.14f\n", j, h[j-1]->getAverage(), h[j-1]->getStandardDev());
				if(h0->getAverage() != bestMakespan) {
					printf("There is a problem at time %.14f: average = %.14f and best makespan = %.14f\n", now, h0->getAverage(), bestMakespan);
					stream << "There is a problem at time " << now << endl;
				}
			}
			while((i < nbEvents) && (events[i]->_t <= now)) {
				IloInt id = events[i]->_id;
				IloNum t  = events[i]->_t;
				IloInt state = events[i]->_state;
				if(1 == state) {
				//	cout << t << "\t ACTIVITY START TIME \t" << id << endl;
					sim->setStartTime(id, t);
				}
				if(2 == state) {
				//	cout << t << "\t\t\t\t\t\t ACTIVITY END TIME \t" << id << endl;
					sim->setEndTime(id, t);
				}
				if(3 == state) {
				//	cout << t << "\t RESOURCE BREAKDOWN START TIME \t" << id << endl;
					sim->setBreakdownStartTime(id, t);
				}
				if((4 == state)&&(events[i]->_t <= bestMakespan)) {
				//	cout << t << "\t\t\t\t\t\t RESOURCE BREAKDOWN END TIME \t" << id << endl;
					sim->setBreakdownEndTime(id, t);
				}
				i++;
			}
		}






	/*	if(1 == activeEvents) {
			printf("Unexpected events occur.\n");
			sim->setBreakdownStartTime(3, -289);
			sim->setBreakdownEndTime(3, -282);
			sim->setBreakdownStartTime(9, -223);
			sim->setBreakdownStartTime(2, -164);
			sim->setBreakdownEndTime(9, -162);
			sim->setBreakdownEndTime(2, -97);
			sim->setBreakdownStartTime(0, -96);
			sim->setBreakdownStartTime(6, -95);
			sim->setBreakdownEndTime(6, -94);
			sim->setBreakdownStartTime(4, -77);
			sim->setBreakdownEndTime(0, -74);
			sim->setBreakdownStartTime(5, -60);
			sim->setBreakdownStartTime(7, -33);
			sim->setBreakdownStartTime(8, -13);
			sim->setBreakdownStartTime(1, -2);
		}*/


	//	sim->setBreakdownEndTime(id, t);

	/*	sim->setStartTime(121, 0.0);
		sim->setStartTime(131, 0.0);
		sim->setStartTime(141, 0.0);
		sim->setStartTime(151, 0.0);*/


	/*	IloNum now = 0.0;
		h0->clean();
		for(j = 0; j < nbTotalActivities; j++)
			h[j]->clean();
		IloNum temp = env.getTime();
		sim->simulate(now, n);
		IloNum temp1 = env.getTime();
		printf("Time spent for running %ld simulations: %.14f\n", n, temp1 - temp);
		
		printf("Makespan: mean = %.14f and standard deviation = %.14f\n", h0->getAverage(), h0->getStandardDev());*/
	/*	const char* fileName2 = "endFrontier";
		endFrontier->display(fileName2, n);
		const char* fileName3 = "startFrontier";
		startFrontier->display(fileName3, n);*/


		// End of a digraph representing a job-shop scheduling solution


		stream.close();
		
		IluNormalVariableI::End();

		env.end();

	} catch (IloException& e) {
		cout << e << endl;
	}
	
	return 0;
}