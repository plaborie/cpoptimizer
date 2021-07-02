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

#include <ctime>
#include <ilsim/ilutimeeventmanager.h>
#include <ilsim/ilumodel.h>
#include <ilsim/ilusolver.h>
#include <ilsched/ilsched.h>
#include <ilsim/ilupgsim.h>
#include <ilsim/iluactivity.h>


ILCSTLBEGIN





///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS PrintSolution
//
///////////////////////////////////////////////////////////////////////////////

void
PrintSolution(const IlcScheduler& scheduler)
{
  
	IlcInt makespan = 0;
// iteration with solution iterator
  for (IlcActivityIterator ite(scheduler) ; ite.ok(); ++ite)
  {
    IlcActivity activity = *ite;
    cout << activity.getName() << " [ ";
	
	if(activity.getStartMin() < activity.getStartMax())
		cout << activity.getStartMin() << ".." << activity.getStartMax();
	else
		cout << activity.getStartMin();
	cout << " -- ";
	
	if(activity.getDurationMin() < activity.getDurationMax())
		cout << activity.getDurationMin() << ".." << activity.getDurationMax();
	else
		cout << activity.getDurationMin();
	cout << " --> ";

	if(activity.getEndMin() < activity.getEndMax())
		cout << activity.getEndMin() << ".." << activity.getEndMax();
	else
		cout << activity.getEndMin();
	cout << " ]" << endl;
	if (activity.getEndMin() > makespan)
		makespan = activity.getEndMin();
  }
	cout << endl << "The makespan is equal to " << makespan << endl;
}





///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF UNCERTAIN SOLUTIONS PrintUncSolution
//
///////////////////////////////////////////////////////////////////////////////

void
PrintUncSolution(const IlcScheduler& scheduler)
{
 // iteration with solution iterator
  for (IlcActivityIterator ite(scheduler) ; ite.ok() ; ++ite)
  {
    IlcActivity activity = *ite;
    cout << activity.getName() << " [ ";
	
	if(activity.getStartMin() < activity.getStartMax())
		cout << activity.getStartMin() << ".." << activity.getStartMax();
	else
		cout << activity.getStartMin();
	cout << " -- ";
	
	if(activity.getDurationMin() < activity.getDurationMax())
		cout << activity.getDurationMin() << ".." << activity.getDurationMax();
	else
		cout << activity.getDurationMin();
	cout << " --> ";

	if(activity.getEndMin() < activity.getEndMax())
		cout << activity.getEndMin() << ".." << activity.getEndMax();
	else
		cout << activity.getEndMin();
	cout << " ]";
	if (activity.getObject() != 0)
	{
		IluActivityI* uncAct = (IluActivityI*)activity.getObject();
		IluNormalVariableI* randVar= (IluNormalVariableI*)uncAct->getRandomVar();
		IloNum min, max, std;
		min = randVar->getMin();
		max = randVar->getMax();
	//	ave = randVar->getAverage();
		std = randVar->getStandardDev();

		cout << " : proc. time between " << min << " and " << max << " and stand. dev. of " << std;
	}
	cout << endl;
  }
}





///////////////////////////////////////////////////////////////////////////////
//
// EXECUTION LOOP MainLoop
//
///////////////////////////////////////////////////////////////////////////////

IlcInt
MainLoop(IloEnv env,
		 IloRandom randGen,
		 IluSolverI* solver,
		 IloNum sensitivity,
		 IluHistogramI* nbRescheduling,
		 IluHistogramI* effectiveMakespan,
		 IluHistogramI* firstEstimatedMakespan,
		 IluHistogramI* estimatedTime,
		 IluHistogramI* relativeError,
		 ofstream stream,
		 IloInt criterion,
		 IloInt optim,
		 IloNum upper,
		 IloNum* optimSol,
		 IloInt seed)
{
	IlcInt nbResched = 0;
//	IlcInt nbPossibleResched = 0;
	IloNum now = 0.0;
	IloNum computationTime = 0.0;

	IlcScheduler sched = solver->solve(now, optim, upper); // sched is our current indicative schedule.
	computationTime = solver->getSearchTime();
	
	// dynamic scheduler creation
	IluReactiveSchedulerI* reactiveSchedP = new (env) IluReactiveSchedulerI(env, sched, randGen, sensitivity, criterion);

	reactiveSchedP->initPG(env, sched, now); // Simulations are run.
	if(3 == criterion)
		computationTime += 0.085;
	else
		computationTime += 0.05;

	firstEstimatedMakespan->setValue(reactiveSchedP->getHist()->getAverage());

//	printf("First estimated makespan: \n\tMean = %f\n\tStandard deviation = %f\n",
//							reactiveSchedP->getHist()->getAverage(),
//							reactiveSchedP->getHist()->getStandardDev());

//	stream << now << reactiveSchedP->getHist()->getAverage() << reactiveSchedP->getHist()->getStandardDev() << endl;


// time event manager creation + controllable and contingent lists creation
//	printf("Time event manager creation\n\n");
	IluTimeEventManagerI* tEMgrP = new (env) IluTimeEventManagerI(reactiveSchedP, (IlcInt)now);

	IloNum increment, nextExecutionTime;
	increment = IloIntMax;
//	int timesimu = 0;
//	int counter = 0;

	do {
// schedule execution starts

// Is their at least one activity that can be executed now?
		nextExecutionTime = tEMgrP->executeNext((IlcInt)now);
		
		for (IlcActivityIterator actite3(sched); actite3.ok(); ++actite3) {	// Time spent in this loop is negligible.
			IlcActivity act = *actite3;
			if (reactiveSchedP->getPrecGraph()->isExecutable(((IluActivityI*)act.getObject())->getIndex())) {
//				printf("Activity ");
//				printf((*actite3).getName());
//				printf(" is executable at %f.\n", now);
				IluActStartEventI* eventP = new (env) IluActStartEventI((IlcInt)now, ((IluActivityI*)act.getObject()));
				tEMgrP->addCtrlEvent(eventP);
			}
		}

		nextExecutionTime = tEMgrP->executeNext((IlcInt)now);
//		stream3 << nextExecutionTime-1 << "\t" << 0 << endl;
//		stream3 << nextExecutionTime << "\t" << 1350000 << endl;
//		stream3 << nextExecutionTime+1 << "\t" << 0 << endl;
		nextExecutionTime = IloMax(0, nextExecutionTime); // Generation of the end time events.
	
		reactiveSchedP->getHist()->clean();

		if(3 == criterion)		// Time spent in this loop is negligible.
			for (IlcActivityIterator actite4(sched); actite4.ok(); ++actite4) {
				IluHistogramI* histP = ((IluActivityI*)(*actite4).getObject())->getHist();
				histP->clean();
			}

// We generate a sample for getting the probability distribution of the makespan.
// An example for measuring time spent in a member function
//		time_t t1,t2;
//		(void) time(&t1);
//		reactiveSchedP->getPrecGraph()->simulate(now, 100000);
//		(void) time(&t2);
//		printf("Time for running 100,000 simulations %d seconds\n", (int) t2-t1);
//		timesimu += (int) t2-t1;
//		counter++;
		reactiveSchedP->getPrecGraph()->simulate(now, 1000);
		computationTime += 0.032;
		
//		reactiveSchedP->getHist()->display("curve.dat", 250);
//		printf("Time %f\t... avg = %f;\t st. dev. = %f\n", now, reactiveSchedP->getHist()->getAverage(), reactiveSchedP->getHist()->getStandardDev());
//		nbPossibleResched++;
			
		if (reactiveSchedP->reschedule(sched)) { // We re-schedule if the rescheduling criterion is verified.
										// This test is insignificant in terms of time.
			nbResched++;
			sched = solver->solve(now, optim, upper); // sched is our current indicative schedule.
			computationTime += uncScheduler->getSearchTime();
//			stream4 << now-1 << "\t" << 0 << endl;
//			stream4 << now << "\t" << 1350000 << endl;
//			stream4 << now+1 << "\t" << 0 << endl;
		
// The execution policy is to execute the activities as early as possible.

// We start simulating this current indicative schedule.

// Uncertain activities caracteristics printing
//			PrintSolution(sched);
			#if defined(VERBOSE) // Add the flag VERBOSE to compile this line.
				env.out() << endl << " Precedence graph creation + execution as soon as possible " << endl << endl;
			#endif
		//	delete dynamicSchedP->getPrecGraph();
			reactiveSchedP->initPG(env, sched, now); // Simulations are run.
			if(3 == criterion)
				computationTime += 0.085;
			else
				computationTime += 0.05;
		}
//		stream << now << "\t" << reactiveSchedP->getHist()->getAverage() << "\t" << reactiveSchedP->getHist()->getStandardDev() << endl;
		now += increment;
		now = IloMin(now, nextExecutionTime);
	}
	while(tEMgrP->executableEvents());		// While there are some controllable events continue.

	nbRescheduling->setValue((IloNum)nbResched);
//	printf("Mean time for running simulations = %f\n", (float) timesimu/counter);
//	printf("Number of possible reschedulings = %ld\n", nbPossibleResched);
//	printf("Number of reschedulings = %ld\n", nbResched);
	effectiveMakespan->setValue(reactiveSchedP->getHist()->getAverage());
//	printf("Effective makespan = %f\n", dynamicSchedP->getHist()->getAverage());
	estimatedTime->setValue(computationTime);
	IloNum relatErr;
	relatErr = (reactiveSchedP->getHist()->getAverage() - optimSol[seed-1])/optimSol[seed-1];
//	printf("Optimal solution = %f\n", optimSol[seed-1]);
//	printf("Relative error = %f\n", relatErr);
	relativeError->setValue(relatErr);
	IloInt numberActivities = sched.getNumberOfActivities();
	sched.getSolver().end(); // All memory allocated by Solver on the Solver heap for _solver is freed.
	return numberActivities;
}





///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	try {
		char* pathName = argv[1];

		char* fileName = argv[2];

		char totalName [1024];
		sprintf(totalName, "%s%s", pathName, fileName);
		
		//IloInt precision = 1000;
		//if (argc > 3)
		//	precision = atol(argv[3]);
		//printf("The precision used is %ld.\n", precision);
		
		IloInt optim = 0;				// optim is set either to 0 or to 1.
		if(argc > 4)
			optim = atol(argv[4]);
		if(0 == optim)
			printf("Execution simulation with a reactive technique\n");
		if(1 == optim)
			printf("Execution simulation with a proactive technique\n");

		IloNum alpha = 0.2;
		if (argc > 5)
			alpha = atof(argv[5]);
//		printf("The alpha value is %f.\n", alpha);
		char* uncertDegree = argv[5];

		IluNormalVariableI::Init(); // Only one normal probability distribution is initialized and it is then transformed.



	// We simulate execution and reschedule if necessary (reactive technique).
		if(0 == optim){
			IloInt criterion = 1;
			if(argc > 6)
				criterion = atol(argv[6]);
//			printf( "The rescheduling criterion used is %ld.\n", criterion);

			char* critName = argv[6];
			char buffer[64];
			sprintf(buffer, "%scrit%suncert%s.dat", fileName, critName, uncertDegree);
			printf(buffer);
			printf("\n");
			ofstream stream(buffer); // A file named buffer is created.
			stream.precision(10);

	// We retrieve optimal solutions in order to be able to compute relative errors.
			char* optimSolFullName = argv[7];
			ifstream optimFile(optimSolFullName);
			int lineLimit = 1024;
			char buffer3[1024];
			IloNum optimSol[100];
			for (IloInt k = 0; k < 100; k++) {
				optimFile.getline(buffer3, lineLimit);
				istrstream line(buffer3, strlen(buffer3));
				IloInt seedNumber;
				line >> seedNumber >> optimSol[k];
			}
			
			IloNum upper;	// Upper bound of the makespan variable domain
			if(argc > 8)
				upper = atof(argv[8]);

			IluHistogramI* nbRescheduling, * firstEstimatedMakespan, * estimatedTime, * effectiveMakespan, * relativeError;

			IloBool endSim = IloFalse;
			IlcInt nbActivities = 0;

			IloNum lowerSensitivity, stepSensitivity;


			if(0.3 == alpha) {
				if(1 == criterion){
					lowerSensitivity = 0.99;
					stepSensitivity  = 0.001;
				}
				if(2 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  2.0;
				}
				if(3 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  0.4;
				}
				if(4 == criterion){
					lowerSensitivity =   0.26;
					stepSensitivity  =   0.07;
				}
			}

			if(0.5 == alpha) {
				if(1 == criterion){
					lowerSensitivity = 0.99;
					stepSensitivity  = 0.001;
				}
				if(2 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  2.0;
				}
				if(3 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  0.3;
				}
				if(4 == criterion){
					lowerSensitivity =   0.2;
					stepSensitivity  =   0.06;
				}
			}

			if(0.8 == alpha) {
				if(1 == criterion){
					lowerSensitivity = 0.99;
					stepSensitivity  = 0.001;
				}
				if(2 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  2.0;
				}
				if(3 == criterion){
					lowerSensitivity =  0.1;
					stepSensitivity  =  0.23;
				}
				if(4 == criterion){
					lowerSensitivity =   0.135;
					stepSensitivity  =   0.05;
				}
			}

		// sensitivity is the sensitivity level.
			for(IloNum sensitivity = lowerSensitivity ; IloFalse == endSim ; sensitivity += stepSensitivity) {
				IloEnv env2;
			//	printf("Line 1\n");
				nbRescheduling = new (env2) IluHistogramI(env2);
			//	printf("Line 2\n");
				effectiveMakespan = new (env2) IluHistogramI(env2);
			//	printf("Line 3\n");
				firstEstimatedMakespan = new (env2) IluHistogramI(env2);
			//	printf("Line 4\n");
				estimatedTime = new (env2) IluHistogramI(env2);
			//	printf("Line 5\n");
				relativeError = new (env2) IluHistogramI(env2);
			//	printf("Line 6\n");

				for(IloInt seed = 1 ; seed < 101 ; seed++) {
			//		printf("Line 7\n");
					printf("Execution with a seed of %ld and a sensitivity of %f.\n", seed, sensitivity);
			//		printf("Line 8\n");

					IloEnv env;  // Environment creation
			//		printf("Line 9\n");

			// Random generator creation
					IloRandom randomGenerator(env);
			//		printf("Line 10\n");

			//	env.setDeleter(IloSafeDeleterMode); // This mode is used to delete extractables.

					randomGenerator.reSeed(seed);
			//		printf("Line 11\n");


			// File creation for recording the scenario
					char buffer2[64];
			//		printf("Line 12\n");
					sprintf(buffer2, "%suncert%sseed%ld.dat", fileName, uncertDegree, seed);
			//		printf("Line 13\n");
					printf(buffer2);
			//		printf("Line 14\n");
					printf("\n");
			//		printf("Line 15\n");
					ofstream streamScenario(buffer2); // A file named buffer2 is created.
			//		printf("Line 16\n");
					streamScenario.precision(10);
			//		printf("Line 17\n");

			// Initial uncertain model creation
					IluModelI* uncModel = new (env) IluModelI(env, totalName, randomGenerator, alpha, optim,
																streamScenario);
			//		printf("Line 18\n");
					
					streamScenario.close();	// The file that contains the scenario is closed.
			//		printf("Line 19\n");

			// Our optimization criterion is the makespan and we want to minimize it.
					(uncModel->getModel()).add(IloMinimize(env,uncModel->getMakespan()));
			//		printf("Line 20\n");

					upper = (uncModel->getMakespan()).getUB();
			//		printf("Line 21\n");
	 
					IloGoal goal;
			//		printf("Line 22\n");

			// This goal picks a resource with minimum global slack	
			// and then, from that resource, picks a resource constraint	
			// with minimum start min that can execute first among the	
			// non-ranked resource constraints.
					IloGoal rankGoal =  IloRankForward(env, uncModel->getMakespan(), IloSelResMinGlobalSlack,
															IloSelFirstRCMinStartMax);
			//		printf("Line 23\n");

		//			goal = IloLimitSearch(env, rankGoal, IloFailLimit(env, 1000));

		
		
		//			goal =  SolveAIJ2(env) && IloRankForward(env);
		

					goal = IloLimitSearch(env, rankGoal, IloTimeLimit(env, 1.0));		// one second is spent for searching
			//		printf("Line 24\n");

// Solver creation
					IluTreeSearchSolverI* solver = new (env) IluTreeSearchSolverI(env, uncModel, goal);
			//		printf("Line 25\n");

		
// The execution policy is to execute all activities as early as possible.

// We start simulating this current indicative schedule.

// Uncertain activities caracteristics printing
//					PrintSolution(sched);

//					printf("Precedence graph creation + execution as soon as possible\n\n");

					nbActivities = MainLoop(env, randomGenerator, uncScheduler, sensitivity, nbRescheduling, effectiveMakespan,
								firstEstimatedMakespan, estimatedTime, relativeError, stream, criterion, optim,
								upper, optimSol, seed);
			//		printf("Line 26\n");
	

					env.out() << endl;
			//		printf("Line 27\n");

					env.end();
			//		printf("Line 28\n");
				}
				stream << sensitivity << "\t" << firstEstimatedMakespan->getAverage() << "\t" <<
							firstEstimatedMakespan->getStandardDev() << "\t" <<
							effectiveMakespan->getAverage() << "\t" <<
							effectiveMakespan->getStandardDev() << "\t" <<
							nbRescheduling->getAverage() << "\t" << nbRescheduling->getStandardDev() << "\t" <<
							estimatedTime->getAverage() << "\t" << estimatedTime->getStandardDev() << "\t" <<
							relativeError->getAverage() << "\t" << relativeError->getStandardDev() << endl;
			//	printf("Line 29\n");
			//	printf("Mean number of reschedulings = %f\n", nbRescheduling->getAverage());
				env2.end();
			//	printf("Line 30\n");
			//	printf("The number of activities of the schedule is: %ld\n", nbActivities);
				IloNum nbActThreshold = (IloNum) nbActivities/10;
			//	printf("Line 31\n");
				if(nbRescheduling->getAverage() > nbActThreshold) // 10% of the number of activities is the maximum number of reschedulings permitted.
					endSim = IloTrue;
			//	printf("Line 32\n");

			}
			stream.close(); // The file stream is closed.
		}

		


		// We simulate execution by using a robust indicative solution (proactive technique).
		if(1 == optim){
			IloInt criterion = 0; // No reschedulings will occur.
			IloNum sensitivity = 0; 
		
	// We retrieve optimal solutions in order to be able to compute relative errors.
			char* optimSolFullName = argv[6];
			ifstream optimFile(optimSolFullName);
			int lineLimit = 1024;
			char buffer3[1024];
			IloNum optimSol[100];
			for (IloInt k = 0; k < 100; k++) {
				optimFile.getline(buffer3, lineLimit);
				istrstream line(buffer3, strlen(buffer3));
				IloInt seedNumber;
				line >> seedNumber;
				line >> optimSol[k];
			}

			char* proactiveName = argv[7];
			char buffer[64];
			sprintf(buffer, "%sproactive%suncert%s.dat", fileName, proactiveName, uncertDegree);
			printf(buffer);
			printf("\n");
			ofstream stream(buffer); // A file named buffer is created.
			stream.precision(10);

			IloNum upper;	// Upper bound of the makespan variable domain
			if(argc > 8)
				upper = atof(argv[8]);

			IluHistogramI* nbRescheduling, * firstEstimatedMakespan, * estimatedTime, * effectiveMakespan, * relativeError;

			IlcInt nbActivities = 0;
			
			IloEnv env2;

			nbRescheduling = new (env2) IluHistogramI(env2);
			effectiveMakespan = new (env2) IluHistogramI(env2);
			firstEstimatedMakespan = new (env2) IluHistogramI(env2);
			estimatedTime = new (env2) IluHistogramI(env2);
			relativeError = new (env2) IluHistogramI(env2);

			for(IloInt seed = 1 ; seed < 101 ; seed++) {
//				printf("Execution with a seed of %ld.\n", seed);

				IloEnv env;  // Environment creation

			// Random generator creation
				IloRandom randomGenerator(env);
				randomGenerator.reSeed(seed);

			// File creation for recording the scenario
				char buffer2[64];
				sprintf(buffer2, "%suncert%sseed%ld.dat", fileName, uncertDegree, seed);
				printf(buffer2);
				printf("\n");
				ofstream streamScenario(buffer2); // A file named buffer2 is created.
				streamScenario.precision(10);

		// Initial uncertain model creation
				IluModelI* uncModel = new (env) IluModelI(env, totalName, randomGenerator, alpha, optim, streamScenario);
					
				streamScenario.close();	// The file that contains the scenario is closed.
			// Our optimization criterion is the makespan and we want to minimize it.
				(uncModel->getModel()).add(IloMinimize(env,uncModel->getMakespan()));

				upper = (uncModel->getMakespan()).getUB();			
	 
				IloGoal goal;	

			// This goal picks a resource with minimum global slack	
			// and then, from that resource, picks a resource constraint	
			// with minimum start min that can execute first among the	
			// non-ranked resource constraints.
				IloGoal rankGoal =  IloRankForward(env, uncModel->getMakespan(), IloSelResMinGlobalSlack,
															IloSelFirstRCMinStartMax);

				goal = IloLimitSearch(env, rankGoal, IloTimeLimit(env, 1.0));		// one second is spent for searching

// Solver creation
				IluTreeSearchSolverI* solver = new (env) IluTreeSearchSolverI(env, uncModel, goal);

// The execution policy is to execute all activities as early as possible.

// We start simulating this current indicative schedule.

// Uncertain activities caracteristics printing
//				PrintSolution(sched);

//				printf("Precedence graph creation + execution as soon as possible\n\n");

				nbActivities = MainLoop(env, randomGenerator, solver, sensitivity, nbRescheduling, effectiveMakespan,
								firstEstimatedMakespan, estimatedTime, relativeError, stream, criterion, optim,
								upper, optimSol, seed);

				env.out() << endl;

				env.end();
			}
			stream << firstEstimatedMakespan->getAverage() << "\t" <<
						firstEstimatedMakespan->getStandardDev() << "\t" <<
						effectiveMakespan->getAverage() << "\t" <<
						effectiveMakespan->getStandardDev() << "\t" << nbRescheduling->getAverage() << "\t" <<
						nbRescheduling->getStandardDev() << "\t" << estimatedTime->getAverage() << "\t" <<
						estimatedTime->getStandardDev() << "\t" << relativeError->getAverage() << "\t" <<
						relativeError->getStandardDev() << endl;
			stream.close();
			env2.end();
		}

		IluNormalVariableI::End(); // The normal distribution is here erased.
		
	} catch(IloException& exc) {
  		cout << exc << endl; }

	return 0;
}