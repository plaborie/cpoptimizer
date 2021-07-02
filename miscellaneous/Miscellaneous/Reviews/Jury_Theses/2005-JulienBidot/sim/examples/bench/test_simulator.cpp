#include <ilsim/ilusimulator.h>
#include <ilsim/ilupgsim.h>
#include <stdlib.h>
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
#endif


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



int main(int argc, char** argv) {
	try {
		const char* fileName = "default.graph";
		if (argc > 1)
			fileName = argv[1];

		IloNum alpha = 0.1;		// uncertainty degree
		if (argc > 2)
			alpha = atof(argv[2]);

		IloInt n = 1000;		// number of simulations
		if (argc > 3)
			n = atol(argv[3]);

		printf("Uncertainty degree = %.14f\n", alpha);
		printf("%ld simulations will be run.\n", n);

		IloEnv env;
		IloInt i;

		// Random number generator creations
		IloRandom randGenerator(env);

		IloInt currentSeed = 1;
		randGenerator.reSeed(currentSeed);
		
		
		// We suppose we run n simulations
		IluNormalVariableI::Init();

		
		// 5 activity duration variables; these activities are supposed to form the frontier.
		IluNormalVariableI* d0 = new (env) IluNormalVariableI(env, randGenerator, 0.0,   0.0,   0.0,  0.0);		// This is a dummy variable used for computing makespan.
		IluNormalVariableI* d1 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 101.0,  51.0, 20.0);
		IluNormalVariableI* d2 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 101.0,  51.0, 20.0);
		IluNormalVariableI* d3 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 201.0, 101.0, 40.0);
		IluNormalVariableI* d4 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 161.0,  81.0, 32.0);
		
		


		// Resource breakdown variables; these activities are supposed to form the frontier.

		// Suppose that the 3 resources never break down
		printf("All resources never break down.\n");
		IluNormalVariableI* inter0 = new (env) IluNormalVariableI(env, randGenerator, IloInfinity, IloInfinity, IloInfinity, 0.0);
		IluNormalVariableI* inter1 = new (env) IluNormalVariableI(env, randGenerator, IloInfinity, IloInfinity, IloInfinity, 0.0);
		IluNormalVariableI* inter2 = new (env) IluNormalVariableI(env, randGenerator, IloInfinity, IloInfinity, IloInfinity, 0.0);

		IluNormalVariableI* breakDur0 = new (env) IluNormalVariableI(env, randGenerator, 0.0, 0.0, 0.0, 0.0);
		IluNormalVariableI* breakDur1 = new (env) IluNormalVariableI(env, randGenerator, 0.0, 0.0, 0.0, 0.0);
		IluNormalVariableI* breakDur2 = new (env) IluNormalVariableI(env, randGenerator, 0.0, 0.0, 0.0, 0.0);


	/*	IluNormalVariableI* inter0 = new (env) IluNormalVariableI(env, randGenerator, 40.0, 200.0, 120.0, 40.0);
		IluNormalVariableI* inter1 = new (env) IluNormalVariableI(env, randGenerator, 80.0, 240.0, 160.0, 30.0);
		IluNormalVariableI* inter2 = new (env) IluNormalVariableI(env, randGenerator, 100.0, 300.0, 200.0, 50.0);

		IluNormalVariableI* breakDur0 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 21.0, 11.0, 3.0);
		IluNormalVariableI* breakDur1 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 31.0, 16.0, 2.0);
		IluNormalVariableI* breakDur2 = new (env) IluNormalVariableI(env, randGenerator, 1.0, 41.0, 21.0, 4.0);*/

			
		// Histogram arrays for the start frontier
		IluHistogramI** lastBreakdownEndTime = new (env) IluHistogramI*[3];
		IluHistogramI** lastEndTimeProcessPlan = new (env) IluHistogramI*[2];
		IluHistogramI** lastEndTimeResource = new (env) IluHistogramI*[3];
		
		lastBreakdownEndTime[0] = new (env) IluHistogramI(env, n);
		lastBreakdownEndTime[1] = new (env) IluHistogramI(env, n);
		lastBreakdownEndTime[2] = new (env) IluHistogramI(env, n);

		lastEndTimeProcessPlan[0] = new (env) IluHistogramI(env, n);
		lastEndTimeProcessPlan[1] = new (env) IluHistogramI(env, n);
		
		lastEndTimeResource[0] = new (env) IluHistogramI(env, n);
		lastEndTimeResource[1] = new (env) IluHistogramI(env, n);
		lastEndTimeResource[2] = new (env) IluHistogramI(env, n);	
		
		IloInt tempSeed;

		for(i = 0 ; i < n ; i++) {
			IloNum endTime, lastEndTime;
			endTime = -100000.0;
			do{	
				lastEndTime = endTime;
				IloNum startTime = inter0->getValue() + lastEndTime;
				endTime = startTime + breakDur0->getValue();
			//	printf("Resource 0: a breakdown starts at %.14f and finishes at %.14f.\n", startTime, endTime);
			}
			while(endTime <= 0.0); // We generate breakdowns until 0.
		//	printf("Resource 0: last breakdown end time = %.14f\n", lastEndTime);
			tempSeed = randGenerator.getInt(IloIntMax) + 1;
			lastBreakdownEndTime[0]->setValue(lastEndTime, tempSeed, i);
			
			endTime = -100000.0;
			do{
				lastEndTime = endTime;
				IloNum startTime = inter1->getValue() + lastEndTime;
				endTime = startTime + breakDur1->getValue();
			//	printf("Resource 1: a breakdown starts at %.14f and finishes at %.14f.\n", startTime, endTime);
			}
			while(endTime <= 0.0); // We generate breakdowns until 0.
		//	printf("Resource 1: last breakdown end time = %.14f\n", lastEndTime);
			tempSeed = randGenerator.getInt(IloIntMax) + 1;
			lastBreakdownEndTime[1]->setValue(lastEndTime, tempSeed, i);

			endTime = -100000.0;
			do{
				lastEndTime = endTime;
				IloNum startTime = inter2->getValue() + lastEndTime;
				endTime = startTime + breakDur2->getValue();
			//	printf("Resource 2: a breakdown starts at %.14f and finishes at %.14f.\n", startTime, endTime);
			}
			while(endTime <= 0.0); // We generate breakdowns until 0.
		//	printf("Resource 2: last breakdown end time = %.14f\n", lastEndTime);
			tempSeed = randGenerator.getInt(IloIntMax) + 1;
			lastBreakdownEndTime[2]->setValue(lastEndTime, tempSeed, i);
		}


/*		for(i = 0 ; i < n ; i++) {
			printf("Resource 0: last breakdown end time = %.14f\n", lastBreakdownEndTime[0]->getValue(i));
			printf("Resource 1: last breakdown end time = %.14f\n", lastBreakdownEndTime[1]->getValue(i));
			printf("Resource 2: last breakdown end time = %.14f\n", lastBreakdownEndTime[2]->getValue(i));
		}*/

		IluFrontierI* startFrontier = new (env) IluFrontierI(2,
															 3,
															 lastBreakdownEndTime,
															 lastEndTimeProcessPlan,
															 lastEndTimeResource);



		// Histogram arrays for the end frontier
		IluHistogramI** lastBreakdownEndTimeEnd = new (env) IluHistogramI*[3];
		IluHistogramI** lastEndTimeProcessPlanEnd = new (env) IluHistogramI*[2];
		IluHistogramI** lastEndTimeResourceEnd = new (env) IluHistogramI*[3];

		lastBreakdownEndTimeEnd[0] = new (env) IluHistogramI(env, n);
		lastBreakdownEndTimeEnd[1] = new (env) IluHistogramI(env, n);
		lastBreakdownEndTimeEnd[2] = new (env) IluHistogramI(env, n);

		lastEndTimeProcessPlanEnd[0] = new (env) IluHistogramI(env, n);
		lastEndTimeProcessPlanEnd[1] = new (env) IluHistogramI(env, n);
		
		lastEndTimeResourceEnd[0] = new (env) IluHistogramI(env, n);
		lastEndTimeResourceEnd[1] = new (env) IluHistogramI(env, n);
		lastEndTimeResourceEnd[2] = new (env) IluHistogramI(env, n);


		IluFrontierI* endFrontier = new (env) IluFrontierI(2,
														   3,
														   lastBreakdownEndTimeEnd,
														   lastEndTimeProcessPlanEnd,
														   lastEndTimeResourceEnd);

	

/*		printf("\n");
		for(i = 0 ; i < n ; i++) {
			IluHistogramI** hist = startFrontier->getLastHistogramBreakdown();
			printf("Resource 0: last breakdown end time = %.14f\n", hist[0]->getValue(i));
			printf("Resource 1: last breakdown end time = %.14f\n", hist[1]->getValue(i));
			printf("Resource 2: last breakdown end time = %.14f\n", hist[2]->getValue(i));
		}*/


/*		// Only one node
		IluSimulatorI* sim = new (env) IluSimulatorI(env, startFrontier, endFrontier, 2);

		sim->addArc(3,1);
		sim->addArc(6,1);

		sim->addArc(1,0);

		sim->setActivityDurationVariable(0, d0);
		sim->setActivityDurationVariable(1, d3);

		sim->setProcessPlanId(1, 1);
		sim->setProcessPlanId(5, 0);
		sim->setProcessPlanId(6, 1);

		sim->setBreakdownDurationVariable(0, breakDur0);
		sim->setInterBreakdownDurationVariable(0, inter0);
		sim->setBreakdownDurationVariable(1, breakDur1);
		sim->setInterBreakdownDurationVariable(1, inter1);
		sim->setBreakdownDurationVariable(2, breakDur2);
		sim->setInterBreakdownDurationVariable(2, inter2);

		sim->requires(1,1);
		sim->requires(2,0);
		sim->requires(3,1);
		sim->requires(4,2);*/


		printf("\n\n\n\nTHE FIRST SIMULATOR CREATION AND USE (IluSimulatorI).\n");
		IluSimulatorI* sim = new (env) IluSimulatorI(env, startFrontier, endFrontier, 5);

		sim->addArc(5,3);
		sim->addArc(6,1);
		sim->addArc(7,4);
		sim->addArc(8,3);
		sim->addArc(9,1);

		sim->addArc(1,2);
		sim->addArc(3,2);
		sim->addArc(2,0);
		sim->addArc(3,4);
		sim->addArc(4,0);

		sim->setActivityDurationVariable(0, d0);
		sim->setActivityDurationVariable(1, d3);
		sim->setActivityDurationVariable(2, d4);
		sim->setActivityDurationVariable(3, d1);
		sim->setActivityDurationVariable(4, d2);

		sim->setProcessPlanId(1, 1);
		sim->setProcessPlanId(2, 1);
		sim->setProcessPlanId(3, 0);
		sim->setProcessPlanId(4, 0);
		sim->setProcessPlanId(8, 0);
		sim->setProcessPlanId(9, 1);

		sim->setBreakdownDurationVariable(0, breakDur0);
		sim->setInterBreakdownDurationVariable(0, inter0);
		sim->setBreakdownDurationVariable(1, breakDur1);
		sim->setInterBreakdownDurationVariable(1, inter1);
		sim->setBreakdownDurationVariable(2, breakDur2);
		sim->setInterBreakdownDurationVariable(2, inter2);

		sim->requires(1,1);
		sim->requires(2,0);
		sim->requires(3,0);
		sim->requires(4,2);
		sim->requires(5,0);
		sim->requires(6,1);
		sim->requires(7,2);

	//	sim->graphDisplay();

	
	
	//	const char* fileName = "makespan.dat";
		//ofstream out(filename);

		
	
		IluHistogramI* h0 = new (env) IluHistogramI(env, n);		// This histogram permits us to get the mean and standard devaition of the makespan.
		sim->setHistogram(0, h0);
	
		sim->topologicalSort();

		
	/*	h0->clean();
		IloNum now = 0.0;
	//	IloNum temp = env.getTime();
		sim->simulate(now, n);
	//	IloNum temp1 = env.getTime();
	//	printf("Time spent for running %ld simulations: %.14f\n", n, temp1-temp);
		h0->display(fileName, n);
		printf("Makespan: mean = %.14f and standard deviation = %.14f\n", h0->getAverage(), h0->getStandardDev());

		const char* fileName2 = "startFrontier";
		startFrontier->display(fileName2, n);

		const char* fileName3 = "endFrontier";
		endFrontier->display(fileName3, n);*/


	/*	printf("\n\n\n\nTHE SECOND SIMULATOR CREATION AND USE.\n");

		IluSimulatorI* sim2 = new (env) IluSimulatorI(env, startFrontier, endFrontier, 5);

		sim2->addArc(5,3);
		sim2->addArc(6,1);
		sim2->addArc(7,4);
		sim2->addArc(8,3);
		sim2->addArc(9,1);

		sim2->addArc(1,2);
		sim2->addArc(3,2);
		sim2->addArc(2,0);
		sim2->addArc(3,4);
		sim2->addArc(4,0);

		sim2->setActivityDurationVariable(0, d0);
		sim2->setActivityDurationVariable(1, d3);
		sim2->setActivityDurationVariable(2, d4);
		sim2->setActivityDurationVariable(3, d1);
		sim2->setActivityDurationVariable(4, d2);

		sim2->setProcessPlanId(1, 1);
		sim2->setProcessPlanId(2, 1);
		sim2->setProcessPlanId(3, 0);
		sim2->setProcessPlanId(4, 0);
		sim2->setProcessPlanId(8, 0);
		sim2->setProcessPlanId(9, 1);

		sim2->setBreakdownDurationVariable(0, breakDur0);
		sim2->setInterBreakdownDurationVariable(0, inter0);
		sim2->setBreakdownDurationVariable(1, breakDur1);
		sim2->setInterBreakdownDurationVariable(1, inter1);
		sim2->setBreakdownDurationVariable(2, breakDur2);
		sim2->setInterBreakdownDurationVariable(2, inter2);

		sim2->requires(1,1);
		sim2->requires(2,0);
		sim2->requires(3,0);
		sim2->requires(4,2);
		sim2->requires(5,0);
		sim2->requires(6,1);
		sim2->requires(7,2);

		sim2->setHistogram(0, h0);
		
		sim2->topologicalSort();

		h0->clean();
		sim2->simulate(now, n);
		const char* fileName3 = "makespan2.dat";
		h0->display(fileName3, n);
		endFrontier->display(fileName3, n);*/

		
		IloNum* starts = new (env) IloNum[5];		// These are controled by the agent.
		IloNum* ends   = new (env) IloNum[5];		// These are observed by the agent.
		
	/*	// Resource breakdown event creations

		IloNum* breakStartTimes = new (env) IloNum[3];
		breakStartTimes[0] = inter0->getValue();
		breakStartTimes[1] = inter1->getValue();
		breakStartTimes[2] = inter2->getValue();

		IloNum* breakDurs = new (env) IloNum[3];
		breakDurs[0] = breakDur0->getValue();
		breakDurs[1] = breakDur1->getValue();
		breakDurs[2] = breakDur2->getValue();

		for(i = 0 ; i < 3 ; i++)
			printf("Resource %ld breaks down at %.14f during %.14f time units.\n", i, breakStartTimes[i], breakDurs[i]);

		starts[5] = breakStartTimes[0];
		starts[6] = breakStartTimes[1] - 200.0;
		starts[7] = breakStartTimes[2];
		ends[5] = starts[5] + breakDurs[0];
		ends[6] = starts[6] + breakDurs[1];
		ends[7] = starts[7] + breakDurs[2];*/

		
		d0->getRandom().reSeed(10);
		// Activity event creations
		IloNum* durs = new (env) IloNum[5];
		durs[0] = d0->getValue();
		durs[1] = d1->getValue();
		durs[2] = d4->getValue();
		durs[3] = d3->getValue();
		durs[4] = d2->getValue();

		for(i = 0 ; i < 5 ; i++)
			printf("Activity %ld has a duration equal to %.14f\n", i, durs[i]);

		starts[1] = 0;
		starts[3] = 0;
		ends[1] = starts[1] + durs[1];
		ends[3] = starts[3] + durs[3];// + breakDurs[1];
		starts[2] = ends[1] + 1e-3;
		starts[4] = IloMax(ends[1], ends[3]) + 1e-3;
		ends[2] = starts[2] + durs[2];
		ends[4] = starts[4] + durs[4];// + breakDurs[0];
		starts[0] = IloMax(ends[2], ends[4]) + 1e-3;
	//	starts[0] = ends[1] + 1e-3;
		ends[0] = starts[0] + durs[0];
		
		


		EventI** events = new (env) EventI*[10];
		for(i = 0 ; i < 5 ; ++i) {
			EventI* event = new (env) EventI(i, starts[i], 1);
			events[i] = event;
			printf("Activity %ld starts at %.14f.\n", i, events[i]->_t);
		}
		for(i = 0 ; i < 5 ; ++i) {
			EventI* event = new (env) EventI(i, ends[i], 2);
			events[5 + i] = event;
			printf("Activity %ld ends at %.14f.\n", i, events[5 + i]->_t);
		}
	/*	EventI** events = new (env) EventI*[4];
		for(i = 0 ; i < 2 ; ++i) {
			EventI* event = new (env) EventI(i, starts[i], 1);
			events[i] = event;
			printf("Activity %ld starts at %.14f.\n", i, events[i]->_t);
		}
		for(i = 0 ; i < 2 ; ++i) {
			EventI* event = new (env) EventI(i, ends[i], 2);
			events[2 + i] = event;
			printf("Activity %ld ends at %.14f.\n", i, events[2 + i]->_t);
		}*/
/*		for(i = 0 ; i < 3 ; ++i) {
			EventI* event = new (env) EventI(i, starts[5 + i], 3);
			events[10 + i] = event;
			printf("Resource %ld breakdowns at %.14f.\n", i, events[10 + i]->_t);
		}
		for(i = 0 ; i < 3 ; ++i) {
			EventI* event = new (env) EventI(i, ends[5 + i],  4);
			events[13 + i] = event;
			printf("Resource %ld resumes execution at %.14f.\n", i, events[13 + i]->_t);
		}*/

		qsort(events, 10, sizeof(EventI*), compareTime);
		//	void  qsort ( void * base, size_t num, size_t width, int (*fncompare)(const void *, const void *) );
		//	Sort using quicksort algorithm.
		//	This function uses an implementation of the quicksort algorithm to sort the num elements of an array pointed by base,
		//	each element has the specified width in bytes.
		//	The method used to compare each pair of elements is provided by the caller to this function with fncompare parameter,
		//	that is a function called one or more times during the sort process. 
		
		IloNum now = 0.0;
		printf("\nExecution simulation starts.\n");
		for(i = 0 ; i < 10 ;) {
			for(; now < events[i]->_t ; now++) {
				h0->clean();
				sim->simulate(now, n);
				printf("%.1f\t... avg = %.14f;\t st. dev. = %.14f\n", now, h0->getAverage(), h0->getStandardDev());
			//	out << now << "\t" <<  h0->getAverage() << "\t" <<  h0->getStandardDev() << endl;
			}
			while((i < 10) && (events[i]->_t <= now)) {
				IloInt id = events[i]->_id;
				IloNum t  = events[i]->_t;
				IloInt state = events[i]->_state;
				if(1 == state) {
					cout << t << "\t ACTIVITY START TIME \t\t" << id << endl;
					sim->setStartTime(id, t);
				}
				if(2 == state) {
					cout << t << "\t ACTIVITY END TIME \t\t" << id << endl;
					sim->setEndTime(id, t);
				}
				if(3 == state) {
					cout << t << "\t RESOURCE BREAKDOWN START TIME \t\t\t" << id << endl;
					sim->setBreakdownStartTime(id, t);
				}
				if(4 == state) {
					cout << t << "\t RESOURCE BREAKDOWN END TIME \t\t\t" << id << endl;
					sim->setBreakdownEndTime(id, t);
				}

				i++;
			}
		}



		printf("\n\n\n\nTHE SECOND SIMULATOR CREATION AND USE (IluPrecGraphI).\n");
		IluPrecGraphI* sim2 = new (env) IluPrecGraphI(5);

		sim2->addArc(1,2);
		sim2->addArc(3,2);
		sim2->addArc(2,0);
		sim2->addArc(3,4);
		sim2->addArc(4,0);

		sim2->setDurationVariable(0, d0);
		sim2->setDurationVariable(1, d3);
		sim2->setDurationVariable(2, d4);
		sim2->setDurationVariable(3, d1);
		sim2->setDurationVariable(4, d2);


/*		printf("\n\n\n\nTHE SECOND SIMULATOR CREATION AND USE (IluPrecGraphI).\n");
		IluPrecGraphI* sim2 = new (env) IluPrecGraphI(2);

		sim2->addArc(1,0);
		
		sim2->setDurationVariable(0, d0);
		sim2->setDurationVariable(1, d3);*/
		


	
		IluHistogramI* h02 = new (env) IluHistogramI(env, n);		// This histogram permits us to get the mean and standard deviation of the makespan.
		sim2->setHistogram(0, h02);
	
		sim2->topologicalSort();
		
		now = 0.0;
		printf("\nExecution simulation starts.\n");
		for(i = 0 ; i < 10 ;) {
			for(; now < events[i]->_t ; now++) {
				h02->clean();
				d0->getRandom().reSeed(1);
				sim2->simulate(now, n);
				printf("%.1f\t... avg = %.14f;\t st. dev. = %.14f\n", now, h02->getAverage(), h02->getStandardDev());
			}
			while((i < 10) && (events[i]->_t <= now)) {
				IloInt id = events[i]->_id;
				IloNum t  = events[i]->_t;
				IloInt state = events[i]->_state;
				if(1 == state) {
					cout << t << "\t ACTIVITY START TIME \t\t" << id << endl;
					sim2->setStartTime(id, t);
				}
				if(2 == state) {
					cout << t << "\t ACTIVITY END TIME \t\t" << id << endl;
					sim2->setEndTime(id, t);
				}
				i++;
			}
		}

		//out.close();


		IluNormalVariableI::End();
		env.end();

			

	} catch (IloException& e) {
		cout << e << endl;
	}
	
	return 0;
}