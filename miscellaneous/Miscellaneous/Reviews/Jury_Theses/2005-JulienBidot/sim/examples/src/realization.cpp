///////////////////////////////////////////////////////////////////////////////
//
// THIS PROGRAM PERMITS US TO GENERATE A REALIZATION OF AN INSTANCE OF A
// SCHEDULING PROBLEM WITH nbProcessPlans PROCESS PLANS, nbActivities
// ACTIVITIES BY PROCESS PLAN, nbResources POSSIBLE RESOURCES ASSOCIATED WITH
// EACH ACTIVITY, nbProcessPlans*nbActivities ACTIVITIES, nbTotalResources
// RESOURCES.
//
///////////////////////////////////////////////////////////////////////////////


#include <ilsched/iloscheduler.h>

//#if defined(ILOUSESTL)
//#include <fstream>
//#else
//#include <fstream.h>
//#endif
//#if defined(ILO_WIN32)
//#include <strstrea.h>
//#else
//#include <strstream.h>
//#endif


#include <ilsim/ilurandom.h>


ILOSTLBEGIN




///////////////////////////////////////////////////////////////////////////////
//
// DEFAULT PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
defaultParamPrint(IloInt seed,
				  IloNum durBound,
				  IloNum timeBreakBound,
				  IloNum breakDurBound,
				  IloInt thetaBreak) {
	printf("Parameters are set by default as follows:\n");
	printf("A file describing an instance followed by one of the following optional parameters:\n");
	printf("-seed %ld\n", seed);
	printf("-durBound %.1f\n", durBound);
	printf("-timeBreakBound %.1f\n", timeBreakBound);
	printf("-breakDurBound %.1f\n", breakDurBound);
	printf("-thetaBreak %ld\n", thetaBreak);
}





///////////////////////////////////////////////////////////////////////////////
//
// PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
paramPrint(IloInt seed,
		   IloNum durBound,
		   IloNum timeBreakBound,
		   IloNum breakDurBound,
		   IloInt thetaBreak) {
	printf("The seed used is %ld.\n", seed);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the activity durations is set to %.1f.\n", durBound);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the durations between two consecutive breakdowns is set to %.1f.\n", timeBreakBound);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the resource breakdown durations is set to %.1f.\n",
			breakDurBound);
	printf("The parameter permitting us to determine the date at which we start to generate breakdowns is set to %ld.\n", thetaBreak);
}






///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	try {
		IloInt i,j,k;
		IloBool init = IloTrue;

		// DEFAULT PARAMETER VALUES
		IloInt _seed = 1;
		IloNum _durBound = 5.0;
		IloNum _timeBreakBound = 5.0;
		IloNum _breakDurBound = 5.0;
		IloInt _thetaBreak = 2;

		// USED PARAMETER VALUES
		IloInt seed = _seed;
		IloNum durBound = _durBound;
		IloNum timeBreakBound = _timeBreakBound;
		IloNum breakDurBound = _breakDurBound;
		IloInt thetaBreak = _thetaBreak;


		// We retrieve the data of the scheduling problem instance in order to be able to generate a file of a realization.
		char* instanceName;
		if(argc > 1) {
			instanceName = argv[1];		
			ifstream stream(instanceName);
			int lineLimit = 4096;
			char buffer[4096];
			stream.getline(buffer, lineLimit);
		//	printf("The buffer is %ld long.\n", strlen(buffer));
			if(strlen(buffer) > 0) {
				istrstream line(buffer, strlen(buffer));
				IloInt nbProcessPlans, nbActivities, nbResources, nbTotalResources;
				IloNum kMax;
				line >> nbProcessPlans >> nbActivities >> nbResources >> nbTotalResources >> kMax;
				//printf("There are %ld process plans.\n", nbProcessPlans);
				//printf("There are %ld activities by process plan.\n", nbActivities);
				//printf("There are %ld possible resources associated with each activity.\n", nbResources);
				//printf("There are %ld resources.\n", nbTotalResources);
				//printf("The maximum cost is equal to %.14f.\n", kMax);
				if((nbProcessPlans < 0)||(nbActivities < 0)||(nbResources < 0)||(nbTotalResources < 0)||(kMax < 0.0))
					init = IloFalse;

				IloEnv env;

		// ACTIVITIES

				IloInt** mDur = new (env) IloInt*[nbProcessPlans];
				for(i = 0 ; i < nbProcessPlans ; i++)
					mDur[i] = new (env) IloInt[nbActivities];
		
				IloNum** sigmaDur = new (env) IloNum*[nbProcessPlans];
				for(i = 0 ; i < nbProcessPlans ; i++)
					sigmaDur[i] = new (env) IloNum[nbActivities];
				
				for(i = 0 ; i < nbProcessPlans ; i++) {
					stream.getline(buffer, lineLimit);
					istrstream line(buffer, strlen(buffer));
					for(j = 0 ; j < nbActivities ; j++) {
						for(k = 0 ; k < nbResources ; k++){
							IloNum temp;
							line >> temp >> temp;
						}
						line >> mDur[i][j] >> sigmaDur[i][j];
					}
				}

				//for(i = 0 ; i < nbProcessPlans ; i++) {
				//	for(j = 0 ; j < nbActivities ; j++) {
				//		printf("The activity a%ld%ld has a duration whose mean is equal to %ld and standard deviation is equal to %.14f.\n",i,j,mDur[i][j],sigmaDur[i][j]);
				//	}
				//}

		// RESOURCES

				IloInt* mTimeBreak = new (env) IloInt[nbTotalResources];
				IloNum* sigmaTimeBreak = new (env) IloNum[nbTotalResources];
				IloInt* mBreakDur = new (env) IloInt[nbTotalResources];
				IloNum* sigmaBreakDur = new (env) IloNum[nbTotalResources];

				for(i = 0 ; i < nbTotalResources ; i++) {
					stream.getline(buffer, lineLimit);
					istrstream line(buffer, strlen(buffer));
					line >> mTimeBreak[i] >> sigmaTimeBreak[i] >> mBreakDur[i] >> sigmaBreakDur[i];
				}

				//for(i = 0 ; i < nbTotalResources ; i++) {
				//	printf("The resource r%ld breaks down between two consecutive breaksdowns with a mean time equal to %ld and a standard deviation equal to %.14f.\n",i,mTimeBreak[i],sigmaTimeBreak[i]);
				//	printf("The resource r%ld breaks down during a mean time equal to %ld with a standard deviation equal to %.14f.\n",i,mBreakDur[i],sigmaBreakDur[i]);
				//}
			

				for(i = 2 ; (i < argc)&&(init) ; i+=2) {
					if(0 == strcmp(argv[i],"-seed")) {
						if(argc > i+1)
							seed = atol(argv[i+1]);
						else
							init = IloFalse;
					}
					else if(0 == strcmp(argv[i], "-durBound")) {
						if(argc > i+1)
							durBound = atof(argv[i+1]);
						else
							init = IloFalse;
					}
					else if(0 == strcmp(argv[i], "-timeBreakBound")) {
						if(argc > i+1)
							timeBreakBound = atof(argv[i+1]);
						else
							init = IloFalse;
					}
					else if(0 == strcmp(argv[i], "-breakDurBound")) {
						if(argc > i+1)
							breakDurBound = atof(argv[i+1]);
						else
							init = IloFalse;
					}
					else if(0 ==strcmp(argv[i], "-thetaBreak")) {
						if(argc > i+1)
							thetaBreak = atol(argv[i+1]);
						else
							init = IloFalse;
					}
					else if(0 == strcmp(argv[i], "-default"))
						defaultParamPrint(_seed,
										  _durBound,
										  _timeBreakBound,
										  _breakDurBound,
										  _thetaBreak);
					else
						init = IloFalse;
				}
		
				if(init) {
					paramPrint(seed,
							   durBound,
							   timeBreakBound,
							   breakDurBound,
							   thetaBreak);

		// RANDOM GENERATOR CREATION
					IloRandom randGenerator(env);
					randGenerator.reSeed(seed);
		

		// PICKING EFFECTIVE ACTIVITY DURATIONS

					IluNormalVariableI::Init(); // Only one normal probability distribution is initialized and is then transformed.

				//	IloNum delta = 0.0;
					IloInt maxDuration = 0;
					IloInt** durations = new (env) IloInt*[nbProcessPlans];
					for(i = 0 ; i < nbProcessPlans ; i++) {
						durations[i] = new (env) IloInt[nbActivities];
						for(j = 0 ; j < nbActivities ; j++) {
							IloNum min = IloMax(1.0,mDur[i][j] - durBound * sigmaDur[i][j]);
							IloNum max = mDur[i][j] + durBound * sigmaDur[i][j];
							//printf("The activity a%ld%ld has a minimum effective duration equal to %.14f and a maximum effective duration equal to %.14f.\n",i,j,min,max);
							IluNormalVariableI* varP = new (env) IluNormalVariableI(env, randGenerator, min, max, mDur[i][j], sigmaDur[i][j]);
							durations[i][j] = (IloInt) varP->getValue();
						//	delta += mDur[i][j];
							if(durations[i][j] > maxDuration)
								maxDuration = durations[i][j];

						}
					}
					//printf("The longest activity duration is equal to %ld.\n", maxDuration);
			//		delta /= nbProcessPlans*nbActivities;
					//printf("The mean duration over all actitvities is equal to %.14f.\n", delta);

					//for(i = 0 ; i < nbProcessPlans ; i++) {
					//	for(j = 0 ; j < nbActivities ; j++) {
					//		printf("The activity a%ld%ld has an effective duration equal to %ld.\n",i,j,durations[i][j]);
					//	}
					//}


		// PICKING EFFECTIVE BREAKDOWN START TIMES
					IloNum mPeriodMax = 0.0;
					IloNum sigmaMin = IloInfinity;
					for(i = 0 ; i < nbTotalResources ; i++) {
						IloNum temp = IloNum(mTimeBreak[i] + mBreakDur[i]);
						if(temp > mPeriodMax)
							mPeriodMax = temp;
						temp = sigmaTimeBreak[i] + sigmaBreakDur[i];
						if(temp < sigmaMin)
							sigmaMin = temp;
					}
					
					IloNum temp;
					if(0.0 == sigmaMin)
						temp = (IloNum)IloIntMin;
					else
						temp = -thetaBreak * mPeriodMax * mPeriodMax/sigmaMin;
					IloNum min = (IloNum)IloIntMin;
					if(temp < min) {
						init = IloFalse;
						printf("The date at which we start to run simulations is too small!\n");
					}
					IloInt tStartSim = (IloInt) temp;
				//	printf("The date at which we start to run simulations is equal to %ld.\n", tStartSim);
					

					IloInt* startBreak = new (env) IloInt[nbTotalResources];	// startBreak is the current breakdown start time.
					IloInt* endBreak = new (env) IloInt[nbTotalResources];		// endBreak is the current breakdown end time.
					IloIntervalList* breakList = new (env) IloIntervalList[nbTotalResources];

					IluNormalVariableI** startP = new (env) IluNormalVariableI*[nbTotalResources];
					IluNormalVariableI** durP = new (env) IluNormalVariableI*[nbTotalResources];
					IloNum* tempStart = new (env) IloNum[nbTotalResources];
					IloNum* tempEnd = new (env) IloNum[nbTotalResources];
					for(i = 0; i < nbTotalResources; i++) {
						IloNum minStart = IloMax(0.0, mTimeBreak[i] - timeBreakBound * sigmaTimeBreak[i]);
						IloNum maxStart = mTimeBreak[i] + timeBreakBound * sigmaTimeBreak[i];
						IloNum minDur = IloMax(1.0, mBreakDur[i] - breakDurBound * sigmaBreakDur[i]);
						IloNum maxDur = mBreakDur[i] + breakDurBound * sigmaBreakDur[i];
						startP[i] = new (env) IluNormalVariableI(env, randGenerator, minStart, maxStart, mTimeBreak[i], sigmaTimeBreak[i]);
						durP[i] = new (env) IluNormalVariableI(env, randGenerator, minDur, maxDur, mBreakDur[i], sigmaBreakDur[i]);
					}

					for(i = 0 ; i < nbTotalResources ; i++) {
						breakList[i] = IloIntervalList(env);
						endBreak[i] = tStartSim;
						do{
							startBreak[i] = endBreak[i] + (IloInt) startP[i]->getValue();
							IloInt dur = (IloInt) durP[i]->getValue();
							endBreak[i] = startBreak[i]+dur;
						}
						while(endBreak[i] <= 0); // We generate breakdowns until breakdowns finish after the date 0.
						if(startBreak[i] < 0) 
							startBreak[i] = 0;
						tempStart[i] = (IloNum) startBreak[i];
						tempEnd[i] = (IloNum) endBreak[i];
					}
					
				//	for(i = 0 ; i < nbTotalResources ; i++) {
				//		printf("The resource r%ld breaks down the first time at %ld.\n", i, startBreak[i]);
				//	}

					
					IloNum activityEnd = 0.0;					// activityEnd is the date at which the preceding virtual activity ends execution by taking
																// into account all current breakdowns.

					IloNum max = (IloNum)IloIntMax;
					temp = (IloNum)maxDuration;					// maxDuration is constant and is equal to the duration of one activity.
					for(j = nbProcessPlans*nbActivities; j > 0; j--) {
						IloInt activityEndMax = (IloInt)activityEnd + maxDuration;
						IloInt biggestActivityEnd = activityEndMax;
						for(i = 0; i < nbTotalResources; i++) {
							IloInt currentActivityEnd = activityEndMax;
							IloInt currentDuration = maxDuration;
							IloInt min = (IloInt)activityEnd;
							while((startBreak[i] < currentActivityEnd)&&(tempStart[i] <= max)&&(tempEnd[i] <= max)) {
								breakList[i].addInterval(startBreak[i], endBreak[i]);
								if((startBreak[i] < 0)||(endBreak[i] < startBreak[i]))
									init = IloFalse;
								if(startBreak[i] > min)
									min = startBreak[i];
								if((IloInt)activityEnd + endBreak[i] + currentDuration - min > currentActivityEnd) {
									currentActivityEnd = (IloInt)activityEnd + endBreak[i] + currentDuration - min;
									currentDuration = currentDuration + endBreak[i] - min;
								}
								tempStart[i] = IloNum(endBreak[i]) + startP[i]->getValue();
								startBreak[i] = (IloInt) tempStart[i];
								IloNum tempDur = durP[i]->getValue();
								IloInt dur = (IloInt) tempDur;
								tempEnd[i] = tempStart[i] + tempDur;
								endBreak[i] = startBreak[i] + dur;
								if((tempStart[i] > max)||(tempEnd[i] > max))
									printf("Warning: activity %ld: generation of breakdowns exceeds integer capacity!\n", i);
							}
							if(currentActivityEnd > biggestActivityEnd)
								biggestActivityEnd = currentActivityEnd;
						}
						activityEnd = (IloNum)biggestActivityEnd;
					}

					//for(i = 0 ; i < nbTotalResources ; i++){
					//	BreakI* current = breakStartTimes[i];
					//	printf("Concerning resource r%ld:\n", i);
					//	j = 0;
					//	while(current != 0) {
					//		printf("A break occurs at time %ld and lasts %ld time units.\n", current->getStartTime(), current->getDuration());
					//		current = current->getNext();
					//		j++;
					//	}
					//	printf("Number of breakdowns = %ld.\n", j);
					//}
					if(IloTrue == init) {
					// REALIZATION FILE CREATION

						char bufferRealization[128];
						sprintf(bufferRealization, "Realization%ldPP%ldAct%ldRes%ldTResSeed%ld.dat", nbProcessPlans, nbActivities, nbResources,
								nbTotalResources, seed);
						printf(bufferRealization);
						printf("\n");
						ofstream streamRealization(bufferRealization); // A file named buffer is created.
						streamRealization.precision(32);

						streamRealization << instanceName << endl;
		
						for(i = 0 ; i < nbProcessPlans ; i++) {
							for(j = 0 ; j < nbActivities ; j++)
								streamRealization << durations[i][j] << "\t";
							streamRealization << endl;
						}

						for(i = 0 ; i < nbTotalResources ; i++) {
							for(IloIntervalListCursor cursor(breakList[i]) ; cursor.ok() ; ++cursor)
								streamRealization << cursor.getStart() << "\t" << cursor.getEnd()-cursor.getStart() << "\t";
							streamRealization << endl;
						}

						streamRealization.close();
					}

					env.end();
	
					IluNormalVariableI::End(); // The normal distribution is here erased.

				}
				else
					init = IloFalse;
			}
			else
				init = IloFalse;
		}
		else
			init = IloFalse;

		if(IloFalse == init) {
			printf("PARAMETERS HAVE NOT BEEN CORRECTLY SET!\n");
			defaultParamPrint(_seed,
							  _durBound,
							  _timeBreakBound,
							  _breakDurBound,
							  _thetaBreak);
		}
	
	} catch(IloException& exc) {
  		cout << exc << endl; }

	return 0;
}