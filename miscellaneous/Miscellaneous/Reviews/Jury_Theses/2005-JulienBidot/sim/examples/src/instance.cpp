///////////////////////////////////////////////////////////////////////////////
//
// THIS PROGRAM PERMITS US TO GENERATE AN INSTANCE OF A SCHEDULING PROBLEM WITH
// nbProcessPlans PROCESS PLANS, nbActivities ACTIVITIES BY PROCESS PLAN,
// nbResources POSSIBLE RESOURCES ASSOCIATED WITH EACH ACTIVITY,
// nbProcessPlans*nbActivities ACTIVITIES, nbTotalResources RESOURCES.
//
///////////////////////////////////////////////////////////////////////////////


#include <ilsched/iloscheduler.h>
#include <ilsim/ilurandom.h>
#include <ilsim/ilumakespan.h>
#include <ilsim/ilutardiness.h>
#include <ilsim/iluglobalcost.h>
#include <ilsim/iluprintsolution.h>



ILOSTLBEGIN




///////////////////////////////////////////////////////////////////////////////
//
// DEFAULT PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
defaultParamPrint(IloInt nbProcessPlans,
				  IloInt nbActivities,
				  IloInt nbResources,
				  IloInt nbTotalResources,
				  IloNum alphaDurMin,
				  IloNum alphaDurMax,
				  IloNum alphaTimeBreakMin,
				  IloNum alphaTimeBreakMax,
				  IloNum alphaBreakDurMin,
				  IloNum alphaBreakDurMax,
				  IloNum mAllocAlloc,
				  IloNum deltaAllocAlloc,
				  IloNum alphaAlloc,
				  IloInt mDurVarMin,
				  IloInt mDurVarMax,
				  IloInt chiDur,
				  IloNum thetaTimeBreakMin,
				  IloNum thetaTimeBreakMax,
				  IloInt chiTimeBreak,
				  IloNum thetaBreakDurMin,
				  IloNum thetaBreakDurMax,
				  IloInt chiBreakDur,
				  IloNum phiMin,
				  IloNum phiMax,
				  IloNum beta,
				  IloNum gamma,
				  IloNum omega,
				  IloInt precision,
				  IloInt seed,
				  IloNum durBound,
				  IloNum timeBreakBound,
				  IloNum breakDurBound,
				  IloInt thetaBreak,
				  IloNum globalSearchTimeLimit,
				  IloInt globalSearchFailLimit,
				  IloNum localSearchTimeLimit,
				  IloInt localSearchFailLimit,
				  IloInt sampleSize,
				  IloNum relaxActProb) {
	printf("Parameters are set by default as follows:\n");
	printf("-nbProcessPlans %ld\n", nbProcessPlans);
	printf("-nbActivities %ld\n", nbActivities);
	printf("-nbResources %ld\n", nbResources);
	printf("-nbTotalResources %ld\n", nbTotalResources);
	printf("-alphaDurMin %.1f\n", alphaDurMin);
	printf("-alphaDurMax %.1f\n", alphaDurMax);
	printf("-alphaTimeBreakMin %.1f\n", alphaTimeBreakMin);
	printf("-alphaTimeBreakMax %.1f\n", alphaTimeBreakMax);
	printf("-alphaBreakDurMin %.1f\n", alphaBreakDurMin);
	printf("-alphaBreakDurMax %.1f\n", alphaBreakDurMax);
	printf("-mAllocAlloc %.1f\n", mAllocAlloc);
	printf("-deltaAllocAlloc %.1f\n", deltaAllocAlloc);
	printf("-alphaAlloc %.1f\n", alphaAlloc);
	printf("-mDurVarMin %ld\n", mDurVarMin);
	printf("-mDurVarMax %ld\n", mDurVarMax);
	printf("-chiDur %ld\n", chiDur);
	printf("-thetaTimeBreakMin %.1f\n", thetaTimeBreakMin);
	printf("-thetaTimeBreakMax %.1f\n", thetaTimeBreakMax);
	printf("-chiTimeBreak %ld\n", chiTimeBreak);
	printf("-thetaBreakDurMin %.1f\n", thetaBreakDurMin);
	printf("-thetaBreakDurMax %.1f\n", thetaBreakDurMax);
	printf("-chiBreakDur %ld\n", chiBreakDur);
	printf("-phiMin %.5f\n", phiMin);
	printf("-phiMax %.5f\n", phiMax);
	printf("-beta %.1f\n", beta);
	printf("-gamma %.1f\n", gamma);
	printf("-omega %.1f\n", omega);
	printf("-precision %ld\n", precision);
	printf("-seed %ld\n", seed);
	printf("-durBound %.1f\n", durBound);
	printf("-timeBreakBound %.1f\n", timeBreakBound);
	printf("-breakDurBound %.1f\n", breakDurBound);
	printf("-thetaBreak %ld\n", thetaBreak);
	printf("-globalSearchTimeLimit %.3f\n", globalSearchTimeLimit);
	printf("-globalSearchFailLimit %ld\n", globalSearchFailLimit);
	printf("-localSearchTimeLimit %.3f\n", localSearchTimeLimit);
	printf("-localSearchFailLimit %ld\n", localSearchFailLimit);
	printf("-sampleSize %ld\n", sampleSize);
	printf("-relaxActProb %.2f\n", relaxActProb);
	printf("\n");
}



///////////////////////////////////////////////////////////////////////////////
//
// PARAMETER PRINT FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

void
paramPrint(IloInt nbProcessPlans,
		   IloInt nbActivities,
		   IloInt nbResources,
		   IloInt nbTotalResources,
		   IloNum alphaDurMin,
		   IloNum alphaDurMax,
		   IloNum alphaTimeBreakMin,
		   IloNum alphaTimeBreakMax,
		   IloNum alphaBreakDurMin,
		   IloNum alphaBreakDurMax,
		   IloNum mAllocAlloc,
		   IloNum deltaAllocAlloc,
		   IloNum alphaAlloc,
		   IloInt mDurVarMin,
		   IloInt mDurVarMax,
		   IloInt chiDur,
		   IloNum thetaTimeBreakMin,
		   IloNum thetaTimeBreakMax,
		   IloInt chiTimeBreak,
		   IloNum thetaBreakDurMin,
		   IloNum thetaBreakDurMax,
		   IloInt chiBreakDur,
		   IloNum phiMin,
		   IloNum phiMax,
		   IloNum beta,
		   IloNum gamma,
		   IloNum omega,
		   IloInt precision,
		   IloInt seed,
		   IloNum durBound,
		   IloNum timeBreakBound,
		   IloNum breakDurBound,
		   IloInt thetaBreak,
		   IloNum globalSearchTimeLimit,
		   IloInt globalSearchFailLimit,
		   IloNum localSearchTimeLimit,
		   IloInt localSearchFailLimit,
		   IloInt sampleSize,
		   IloNum relaxActProb) {
	printf("There are %ld process plans.\n", nbProcessPlans);
	printf("There are %ld activities by process plan.\n", nbActivities);
	printf("There are %ld possible resources associated with each activity.\n", nbResources);
	printf("There are %ld resources.\n", nbTotalResources);
	printf("The uncertainty degree concerning each activity duration is in [%.1f;%.1f].\n", alphaDurMin, alphaDurMax);
	printf("The uncertainty degree concerning each time between two consecutive breakdowns is in [%.1f;%.1f].\n", alphaTimeBreakMin, alphaTimeBreakMax);
	printf("The uncertainty degree concerning each breakdown duration is in [%.1f;%.1f].\n", alphaBreakDurMin, alphaBreakDurMax);
	printf("The mean allocation cost is equal to %.1f.\n", mAllocAlloc);
	printf("The delta allocation parameter is set to %.1f.\n", deltaAllocAlloc);
	printf("The alpha allocation parameter permitting us to balance allocation costs is set to %.1f.\n", alphaAlloc);
	printf("The minimum mean variable activity duration is equal to %ld.\n", mDurVarMin);
	printf("The maximum mean variable activity duration is equal to %ld.\n", mDurVarMax);
	printf("The parameter permitting us to shift horizontally distribution of each activity duration is equal to %ld.\n", chiDur);
	printf("The parameter permitting us to compute the minimum bounds on the mean times between two consecutive breakdowns is set to %.1f.\n", thetaTimeBreakMin);
	printf("The parameter permitting us to compute the maximum bounds on the mean times between two consecutive breakdowns is set to %.1f.\n", thetaTimeBreakMax);
	printf("The parameter permitting us to shift horizontally distribution of each resource inter breakdown is equal to %ld.\n", chiTimeBreak);
	printf("The parameter permitting us to compute the minimum bounds on the mean breadown durations is set to %.1f.\n", thetaBreakDurMin);
	printf("The parameter permitting us to compute the maximum bounds on the mean breadown durations is set to %.1f.\n", thetaBreakDurMax);
	printf("The parameter permitting us to shift horizontally distribution of each resource breakdown duration is equal to %ld.\n", chiBreakDur);
	printf("The parameter defining the minimum weight for the tardiness costs is set to %.5f.\n", phiMin);
	printf("The parameter defining the maximum weight for the tardiness costs is set to %.5f.\n", phiMax);
	printf("The parameter defining the heuristic to balance allocation costs and tardiness costs is set to %.1f.\n", beta);
	printf("The parameter defining the heuristic to rank resource constraints and minimizing the global cost is set to %.1f.\n", gamma);
	printf("The parameter defining the heuristic to assign resources and minimize the global cost is set to %.1f.\n", omega);
	printf("The data precision used is %ld.\n", precision);
	printf("The seed used is %ld.\n", seed);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the activity durations is set to %.1f.\n", durBound);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the durations between two consecutive breakdowns is set to %.1f.\n", timeBreakBound);
	printf("The parameter permitting us to set the bounds used to truncate the distributions of the resource breakdown durations is set to %.1f.\n", breakDurBound);
	printf("The parameter permitting us to determine the date at which we start to generate breakdowns is set to %ld.\n", thetaBreak);
	printf("The parameter permitting us to determine the time spent to look for a better solution during whole global search is set to %.3f CPU second(s).\n",
																																			globalSearchTimeLimit);
	printf("The parameter permitting us to determine the number of fails during global search for a better solution is set to %ld.\n",
																																		globalSearchFailLimit);
	printf("The parameter permitting us to determine the time spent to look for a better solution during whole local search is set to %.3f CPU second(s).\n",
																																			localSearchTimeLimit);
	printf("The parameter permitting us to determine the number of fails during local search in a neighborhood for a better solution is set to %ld.\n",
																																		localSearchFailLimit);
	printf("%ld realization(s) will constitute the sample.\n", sampleSize);
	printf("The probability for choosing activities to relax during LNS is set to %.2f.\n", relaxActProb);
	printf("\n");
}



///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	try {
		printf("Generation of a scheduling problem with uncertain activity durations, uncertain machine breakdowns, and alternative resources.\n\n");

		// PARAMETERS

		// DEFAULT PARAMETER VALUES
		IloInt _nbProcessPlans = 10;
		IloInt _nbActivities = 10;
		IloInt _nbResources = 3;
		IloInt _nbTotalResources = 15;
		IloNum _alphaDurMin = 0.2;
		IloNum _alphaDurMax = 0.4;
		IloNum _alphaTimeBreakMin = 0.2;
		IloNum _alphaTimeBreakMax = 0.4;
		IloNum _alphaBreakDurMin = 0.2;
		IloNum _alphaBreakDurMax = 0.4;
		IloInt _mAllocAlloc = 15000;
		IloInt _deltaAllocAlloc = 10000;
		IloNum _alphaAlloc = 2.0;
		IloInt _mDurVarMin = 33;
		IloInt _mDurVarMax = 99;
		IloInt _chiDur = 66;
		IloNum _thetaTimeBreakMin = 5.0;
		IloNum _thetaTimeBreakMax = 15.0;
		IloInt _chiTimeBreak = 10*_chiDur;
		IloNum _thetaBreakDurMin = 1.0;
		IloNum _thetaBreakDurMax = 3.0;
		IloInt _chiBreakDur = 2*_chiDur;
		IloNum _phiMin = 0.01;
		IloNum _phiMax = 0.05;
		IloNum _beta = 1.0;
		IloNum _gamma = 0.6;
		IloNum _omega = 2.0;
		IloInt _precision = 1000;
		IloInt _seed = 1;
		IloNum _durBound = 5.0;
		IloNum _timeBreakBound = 5.0;
		IloNum _breakDurBound = 5.0;
		IloInt _thetaBreak = 2;
		IloNum _globalSearchTimeLimit = 1200.0;
		IloInt _globalSearchFailLimit = 10000;
		IloNum _localSearchTimeLimit = 600.0;
		IloInt _localSearchFailLimit = 100;
		IloInt _sampleSize = 10;
		IloNum _relaxActProb = 0.05;

		// USED PARAMETER VALUES
		IloInt nbProcessPlans = _nbProcessPlans;
		IloInt nbActivities = _nbActivities;
		IloInt nbResources = _nbResources;
		IloInt nbTotalResources = _nbTotalResources;
		IloNum alphaDurMin = _alphaDurMin;
		IloNum alphaDurMax = _alphaDurMax;
		IloNum alphaTimeBreakMin = _alphaTimeBreakMin;
		IloNum alphaTimeBreakMax = _alphaTimeBreakMax;
		IloNum alphaBreakDurMin = _alphaBreakDurMin;
		IloNum alphaBreakDurMax = _alphaBreakDurMax;
		IloInt mAllocAlloc = _mAllocAlloc;
		IloInt deltaAllocAlloc = _deltaAllocAlloc;
		IloNum alphaAlloc = _alphaAlloc;
		IloInt mDurVarMin = _mDurVarMin;
		IloInt mDurVarMax = _mDurVarMax;
		IloInt chiDur = _chiDur;
		IloNum thetaTimeBreakMin = _thetaTimeBreakMin;
		IloNum thetaTimeBreakMax = _thetaTimeBreakMax;
		IloInt chiTimeBreak = _chiTimeBreak;
		IloNum thetaBreakDurMin = _thetaBreakDurMin;
		IloNum thetaBreakDurMax = _thetaBreakDurMax;
		IloInt chiBreakDur = _chiBreakDur;
		IloNum phiMin = _phiMin;
		IloNum phiMax = _phiMax;
		IloNum beta = _beta;
		IloNum gamma = _gamma;
		IloNum omega = _omega;
		IloInt precision = _precision;
		IloInt seed = _seed;
		IloNum durBound = _durBound;
		IloNum timeBreakBound = _timeBreakBound;
		IloNum breakDurBound = _breakDurBound;
		IloInt thetaBreak = _thetaBreak;
		IloNum globalSearchTimeLimit = _globalSearchTimeLimit;
		IloInt globalSearchFailLimit = _globalSearchFailLimit;
		IloNum localSearchTimeLimit = _localSearchTimeLimit;
		IloInt localSearchFailLimit = _localSearchFailLimit;
		IloInt sampleSize = _sampleSize;
		IloNum relaxActProb = _relaxActProb;

		IloInt i;
		IloBool init = IloTrue;
	//	IloNum beta = IloInfinity;
		IloNum kMax = IloInfinity;

		for(i = 1 ; (i < argc)&&(init) ; i+=2) {
			if(strcmp(argv[i],"-nbProcessPlans") == 0) {
				if(argc > i+1)
					nbProcessPlans = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-nbActivities") == 0) {
				if(argc > i+1)
					nbActivities = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-nbResources") == 0) {
				if(argc > i+1)
					nbResources = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-nbTotalResources") == 0) {
				if(argc > i+1)
					nbTotalResources = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaDurMin") == 0) {
				if(argc > i+1)
					alphaDurMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaDurMax") == 0) {
				if(argc > i+1)
					alphaDurMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaTimeBreakMin") == 0) {
				if(argc > i+1)
					alphaTimeBreakMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaTimeBreakMax") == 0) {
				if(argc > i+1)
					alphaTimeBreakMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaBreakDurMin") == 0) {
				if(argc > i+1)
					alphaBreakDurMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaBreakDurMax") == 0) {
				if(argc > i+1)
					alphaBreakDurMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-mAllocAlloc") == 0) {
				if(argc > i+1)
					mAllocAlloc = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-deltaAllocAlloc") == 0) {
				if(argc > i+1)
					deltaAllocAlloc = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-alphaAlloc") == 0) {
				if(argc > i+1)
					alphaAlloc = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-mDurVarMin") == 0) {
				if(argc > i+1)
					mDurVarMin = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-mDurVarMax") == 0) {
				if(argc > i+1)
					mDurVarMax = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-chiDur") == 0) {
				if(argc > i+1)
					chiDur = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-thetaTimeBreakMin") == 0) {
				if(argc > i+1)
					thetaTimeBreakMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-thetaTimeBreakMax") == 0) {
				if(argc > i+1)
					thetaTimeBreakMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-chiTimeBreak") == 0) {
				if(argc > i+1)
					chiTimeBreak = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-thetaBreakDurMin") == 0) {
				if(argc > i+1)
					thetaBreakDurMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-thetaBreakDurMax") == 0) {
				if(argc > i+1)
					thetaBreakDurMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-chiBreakDur") == 0) {
				if(argc > i+1)
					chiBreakDur = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-phiMin") == 0) {
				if(argc > i+1)
					phiMin = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-phiMax") == 0) {
				if(argc > i+1)
					phiMax = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-beta") == 0) {
				if(argc > i+1)
					beta = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-gamma") == 0) {
				if(argc > i+1)
					gamma = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-omega") == 0) {
				if(argc > i+1)
					omega = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-precision") == 0) {
				if(argc > i+1)
					precision = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(strcmp(argv[i], "-seed") == 0) {
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
			else if(0 == strcmp(argv[i], "-globalSearchTimeLimit")) {
				if(argc > i+1)
					globalSearchTimeLimit = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 ==strcmp(argv[i], "-globalSearchFailLimit")) {
				if(argc > i+1)
					globalSearchFailLimit = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 == strcmp(argv[i], "-localSearchTimeLimit")) {
				if(argc > i+1)
					localSearchTimeLimit = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 ==strcmp(argv[i], "-localSearchFailLimit")) {
				if(argc > i+1)
					localSearchFailLimit = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 ==strcmp(argv[i], "-sampleSize")) {
				if(argc > i+1)
					sampleSize = atol(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 == strcmp(argv[i], "-relaxActProb")) {
				if(argc > i+1)
					relaxActProb = atof(argv[i+1]);
				else
					init = IloFalse;
			}
			else if(0 == strcmp(argv[i], "-default"))
				init = IloFalse;
			else
				init = IloFalse;
		}

		if((init == IloFalse)||(nbResources > nbTotalResources)||(alphaDurMin > alphaDurMax)||(alphaTimeBreakMin > alphaTimeBreakMax)||(beta <= 0.0)||(gamma < -1.0)||
			(omega <= 1.0)||(alphaBreakDurMin > alphaBreakDurMax)||(alphaDurMin < 0.0)||(alphaTimeBreakMin < 0.0)||(alphaBreakDurMin < 0.0)||(sampleSize < 1)||
			(mAllocAlloc < deltaAllocAlloc)||(mAllocAlloc < 0)||(deltaAllocAlloc < 0)||(alphaAlloc < 0.0)||(chiDur < 0)||(chiTimeBreak < 0)||(chiBreakDur < 0)||
			(durBound < 0.0)||(timeBreakBound < 0.0)||(breakDurBound < 0.0)||(mDurVarMin < 0)||(mDurVarMax < 0)||(relaxActProb < 0.0)) {
			printf("PARAMETERS HAVE NOT BEEN CORRECTLY SET!\n");
			defaultParamPrint(_nbProcessPlans,
							  _nbActivities,
							  _nbResources,
							  _nbTotalResources,
							  _alphaDurMin,
							  _alphaDurMax,
							  _alphaTimeBreakMin,
							  _alphaTimeBreakMax,
							  _alphaBreakDurMin,
							  _alphaBreakDurMax,
							  _mAllocAlloc,
							  _deltaAllocAlloc,
							  _alphaAlloc,
							  _mDurVarMin,
							  _mDurVarMax,
							  _chiDur,
							  _thetaTimeBreakMin,
							  _thetaTimeBreakMax,
							  _chiTimeBreak,
							  _thetaBreakDurMin,
							  _thetaBreakDurMax,
							  _chiBreakDur,
							  _phiMin,
							  _phiMax,
							  _beta,
							  _gamma,
							  _omega,
							  _precision,
							  _seed,
							  _durBound,
							  _timeBreakBound,
							  _breakDurBound,
							  _thetaBreak,
							  _globalSearchTimeLimit,
							  _globalSearchFailLimit,
							  _localSearchTimeLimit,
							  _localSearchFailLimit,
							  _sampleSize,
							  _relaxActProb);
		}
		else {
		/*	paramPrint(nbProcessPlans,
					   nbActivities,
					   nbResources,
					   nbTotalResources,
					   alphaDurMin,
					   alphaDurMax,
					   alphaTimeBreakMin,
					   alphaTimeBreakMax,
					   alphaBreakDurMin,
					   alphaBreakDurMax,
					   mAllocAlloc,
					   deltaAllocAlloc,
					   alphaAlloc,
					   mDurVarMin,
					   mDurVarMax,
					   chiDur,
					   thetaTimeBreakMin,
					   thetaTimeBreakMax,
					   chiTimeBreak,
					   thetaBreakDurMin,
					   thetaBreakDurMax,
					   chiBreakDur,
					   phiMin,
					   phiMax,
					   beta,
					   gamma,
					   omega,
					   precision,
					   seed,
					   durBound,
					   timeBreakBound,
					   breakDurBound,
					   thetaBreak,
					   globalSearchTimeLimit,
					   globalSearchFailLimit,
					   localSearchTimeLimit,
					   localSearchFailLimit,
					   sampleSize,
					   relaxActProb);*/


			
		// ENVIRONMENT CREATION AND RANDOM GENERATOR CREATION
			IloEnv env;
			IloRandom randGenerator(env);
			randGenerator.reSeed(seed);

			IloBool problem = IloFalse;

		// PROCESS PLANS

			IloInt j,k;
			IloInt nbTotalActivities = nbProcessPlans * nbActivities;
			IloInt* randTab = new (env) IloInt[nbTotalResources];
			for(i = 0 ; i < nbTotalResources ; i++) {
				randTab[i] = i;
			}

			IloInt*** resourceNumbers = new (env) IloInt**[nbProcessPlans];
			for(i = 0; i < nbProcessPlans; i++) {
				resourceNumbers[i] = new (env) IloInt*[nbActivities];
				for(j = 0; j < nbActivities; j++) {
					resourceNumbers[i][j] = new (env) IloInt[nbResources];
					for(k = 0; k < nbResources; k++) {
						IloInt indice, res;
						indice = k + randGenerator.getInt(nbTotalResources - k);
						res = randTab[k];
						randTab[k] = randTab[indice];
						randTab[indice] = res;
					}
				//	printf("The activity a%ld%ld is associated to:\n", i, j);
					for(k = 0; k < nbResources; k++) {
						resourceNumbers[i][j][k] = randTab[k];
				//		printf("\tthe resource r%ld.\n", resourceNumbers[i][j][k]);
					}
				//	printf("\n");
				}
			}

			IloInt temp = 0;

			IloInt* allocRes = new(env) IloInt[nbTotalResources];
			for(i = 0; i < nbTotalResources; i++) {
				allocRes[i] = alphaAlloc*mAllocAlloc - alphaAlloc*deltaAllocAlloc + randGenerator.getInt(alphaAlloc*2*deltaAllocAlloc+1);
				printf("Resource %ld costs %ld.\n", i, allocRes[i]);
			}
			printf("\n");

			IloInt** allocCosts = new(env) IloInt*[nbTotalActivities];
			for(i = 0; i < nbTotalActivities; i++) {
				allocCosts[i] = new(env) IloInt[nbTotalResources];
				for(j = 0; j < nbTotalResources; j++) {
					allocCosts[i][j] = IloIntMax;
				}
			}
			for(i = 0; i < nbProcessPlans; i++) {
				for(j = 0; j < nbActivities; j++) {
				//	printf("The activity a%ld%ld associated to:\n", i, j);
					for(k = 0; k < nbResources; k++) {
						allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]] = mAllocAlloc - deltaAllocAlloc +
																						randGenerator.getInt(2*deltaAllocAlloc+1) +
																						allocRes[resourceNumbers[i][j][k]];
				//		printf("\tresource r%ld costs %ld.\n",resourceNumbers[i][j][k], allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]]);
					}
				}
			}
			printf("\n");

			IloInt** mDur = new (env) IloInt*[nbProcessPlans];
			IloNum** sigmaDur = new (env) IloNum*[nbProcessPlans];
			IloNum delta = 0.0;
			for(i = 0; i < nbProcessPlans; i++) {
				mDur[i] = new (env) IloInt[nbActivities];
				sigmaDur[i] = new (env) IloNum[nbActivities];
				for(j = 0; j < nbActivities; j++) {
					IloInt mDurVarIJ = mDurVarMin * precision + randGenerator.getInt((mDurVarMax - mDurVarMin) * precision + 1);
					mDur[i][j] = chiDur * precision + mDurVarIJ;
					if(mDur[i][j] < 0)
						init = IloFalse;
				//	printf("The activity a%ld%ld has a mean duration equal to %ld.\n",i,j,mDur[i][j]);
					delta += mDurVarIJ;
					IloNum alphaDur = alphaDurMin + randGenerator.getFloat() * (alphaDurMax - alphaDurMin);
					IloInt temp = IloInt(precision * alphaDur * mDurVarIJ);
					sigmaDur[i][j] = (IloNum) temp/precision;
				//	printf("The activity a%ld%ld has a duration whose standard deviation is equal to %.10f.\n",i,j,sigmaDur[i][j]);
				}
			}
			delta /= nbActivities*nbProcessPlans;
		//	printf("The mean activity duration of this problem is equal to %.10f.\n",delta);
			
			IloNum* phis = new (env) IloNum[nbProcessPlans];
			for(i = 0 ; i < nbProcessPlans ; i++) {
				IloInt temp = IloInt (precision*(phiMin+randGenerator.getFloat()*(phiMax-phiMin)));
			//	printf("Temp = %ld\n", temp);
				phis[i] = (IloNum) temp/precision;
			//	printf("The process plan %ld has a tardiness slope equal to %.15f.\n",i,phis[i]);
			}


		// RESOURCE BREAKDOWNS

			IloInt* mTimeBreak = new (env) IloInt[nbTotalResources];
			IloNum* sigmaTimeBreak = new (env) IloNum[nbTotalResources];
			for(i = 0 ; i < nbTotalResources ; i++) {
				IloNum thetaTimeBreakI = thetaTimeBreakMin + randGenerator.getFloat()*(thetaTimeBreakMax-thetaTimeBreakMin);
				//printf("The resource r%ld has a thetaTimeBreak equal to %.10f.\n",i,thetaTimeBreakI);
				IloInt mTimeBreakVarI = IloInt (thetaTimeBreakI * delta);
				mTimeBreak[i] = chiTimeBreak * precision + mTimeBreakVarI;
			//	printf("The resource r%ld has a mean time between two consecutive breakdowns equal to %ld.\n",i,mTimeBreak[i]);
				if(mTimeBreak[i] < 0)
					init = IloFalse;
				IloNum alphaTimeBreak = alphaTimeBreakMin + randGenerator.getFloat() * (alphaTimeBreakMax - alphaTimeBreakMin);
				IloInt temp = IloInt(precision * alphaTimeBreak * mTimeBreakVarI);
				sigmaTimeBreak[i] = (IloNum) temp/precision;
			//	printf("The resource r%ld has a time between two consecutive breakdowns whose standard deviation is equal to %.10f.\n",i,sigmaTimeBreak[i]);
			}

			IloInt* mBreakDur = new (env) IloInt[nbTotalResources];
			IloNum* sigmaBreakDur = new (env) IloNum[nbTotalResources];
			for(i = 0 ; i < nbTotalResources ; i++) {
				IloNum thetaBreakDurI = thetaBreakDurMin+randGenerator.getFloat()*(thetaBreakDurMax-thetaBreakDurMin);
				//printf("The resource r%ld has a thetaBreakDur equal to %.10f.\n",i,thetaBreakDurI);
				IloInt mBreakDurVarI = IloInt (thetaBreakDurI * delta);
				mBreakDur[i] = chiBreakDur * precision + mBreakDurVarI;
			//	printf("The resource r%ld has a mean breakdown duration equal to %ld.\n",i,mBreakDur[i]);
				IloNum alphaBreakDur = alphaBreakDurMin + randGenerator.getFloat() * (alphaBreakDurMax - alphaBreakDurMin);
				IloInt temp = IloInt(precision * alphaBreakDur * mBreakDurVarI);
				sigmaBreakDur[i] = (IloNum) temp/precision;
			//	printf("The resource r%ld has a breakdown duration whose standard deviation is equal to %.10f.\n",i,sigmaBreakDur[i]);
			}

			IluNormalVariableI::Init(); // Only one normal probability distribution is initialized and is then transformed.
			IloInt** durations = new(env) IloInt*[nbProcessPlans];
			IluNormalVariableI** durationVars = new(env) IluNormalVariableI*[nbTotalActivities];

			for(i = 0, k = 0; i < nbProcessPlans; i++) {
				durations[i] = new (env) IloInt[nbActivities];
				for(j = 0; j < nbActivities; j++, k++) {
					IloNum min = IloMax(1.0, mDur[i][j] - durBound * sigmaDur[i][j]);
					IloNum max = mDur[i][j] + durBound * sigmaDur[i][j];
					//printf("The activity a%ld%ld has a minimum effective duration equal to %.14f and a maximum effective duration equal to %.14f.\n",i,j,min,max);
					durationVars[k] = new (env) IluNormalVariableI(env, randGenerator, min, max, mDur[i][j], sigmaDur[i][j]);
				}
			}

			IloInt* startBreak = new (env) IloInt[nbTotalResources];	// startBreak is the current breakdown start time.
			IloInt* endBreak = new (env) IloInt[nbTotalResources];		// endBreak is the current breakdown end time.

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

			IloNum mPeriodMax = 0.0;
			IloNum sigmaMin = IloInfinity;
			for(i = 0 ; i < nbTotalResources ; i++) {
				IloNum temp = IloNum(mTimeBreak[i]+mBreakDur[i]);
				if(temp > mPeriodMax)
					mPeriodMax = temp;
				temp = sigmaTimeBreak[i] + sigmaBreakDur[i];
				if(temp < sigmaMin)
					sigmaMin = temp;
			}
			
			IloNum numTemp;
			if(0.0 == sigmaMin)
				numTemp = (IloNum)IloIntMin;
			else
				numTemp = -thetaBreak*mPeriodMax*mPeriodMax/sigmaMin;
			IloNum min = (IloNum)IloIntMin;
			if(numTemp < min) {
				init = IloFalse;
				printf("The date at which we start to run simulations is too small!\n");
			}
			IloInt tStartSim = (IloInt) numTemp;
		//	printf("The date at which we start to run simulations is equal to %ld.\n", tStartSim);
			
			IloNum intUpperBound = (IloNum) IloIntMax;

			// DUE DATES ARE DETERNINED AS FOLLOWS.
			IloInt* dueDates = new(env) IloInt[nbProcessPlans];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			printf("Mean makespan is now determined by overlooking tardiness and allocation costs.\n\n");
			IloNum meanMakespan = 0.0;
			IloInt seedTemp;
			IloInt makespanAllocCost = 0;
			IloInt* effProcPlanEndTimes = new(env) IloInt[nbProcessPlans];
			for(i = 0; i < nbProcessPlans; i++) {
				dueDates[i] = 0;
				effProcPlanEndTimes[i] = 0;
			}
			// REALIZATIONS ARE GENERATED AND SOLVED.
			for(seedTemp = 1; seedTemp < sampleSize + 1; seedTemp++) {
				printf("A realization is generated with seed = %ld\n", seedTemp);
				randGenerator.reSeed(seedTemp);

				// PICKING EFFECTIVE ACTIVITY DURATIONS
				IloInt maxDuration = 0;
				for(i = 0, k = 0; i < nbProcessPlans; i++) {
					for(j = 0; j < nbActivities; j++, k++) {
						durations[i][j] = (IloInt) durationVars[k]->getValue();
						if(durations[i][j] > maxDuration)
							maxDuration = durations[i][j];
					}
				}
				//printf("The longest activity duration is equal to %ld.\n", maxDuration);
				

				//for(i = 0 ; i < nbProcessPlans ; i++) {
				//	for(j = 0 ; j < nbActivities ; j++) {
				//		printf("The activity a%ld%ld has an effective duration equal to %ld.\n",i,j,durations[i][j]);
				//	}
				//}


			// PICKING EFFECTIVE BREAKDOWN START TIMES				
				IloEnv envTemp;
				IloIntervalList* breakList = new (envTemp) IloIntervalList[nbTotalResources];
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
							breakList[i].addInterval((IloNum)startBreak[i], (IloNum)endBreak[i]);
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

			//	for(i = 0 ; i < nbTotalResources ; i++) {
			//		printf("Concerning resource r%ld:\n", i);
			//		j = 0;
			//		for(IloIntervalListCursor cursor(breakList[i]); cursor.ok(); ++cursor) {
			//			printf("A breakdown starts at time %.1f and ends at time %.1f.\n", cursor.getStart(), cursor.getEnd());
			//			j++;
			//		}
			//		printf("Number of breakdowns = %ld\n", j);
			//	}


				IloNumVar makespan;
			//	IloArray<IloActivityArray> actArrays;
				IloSchedulerSolution solution(envTemp);
				IloModel model = DefineModel(envTemp,
											 nbProcessPlans,
											 nbActivities,
											 nbResources,
											 nbTotalResources,
											 resourceNumbers,
											 durations,
											 breakList,
											 randGenerator,
											 solution,
										//	 actArrays,
											 makespan);
				IloNum makespanTemp = SolveModel(model,
												 makespan,
											//	 actArrays,
												 solution,
												 globalSearchTimeLimit,
												 globalSearchFailLimit,
												 localSearchTimeLimit,
												 localSearchFailLimit,
												 relaxActProb);//, nbTotalResources);
				makespanTemp /= sampleSize;
				meanMakespan += makespanTemp;
			//	PrintSolution(solution, makespan);
				makespanAllocCost = ComputeAllocCost(solution, allocCosts);
				RetrieveEffProcPlanEndTimes(solution, effProcPlanEndTimes);
				envTemp.end();
			}
		//	meanMakespan = 3245081.0;// 20*10 activities with breakdowns
		//	meanMakespan = 2728646.0;// 20*10 activities without breakdowns
			printf("For this instance, mean makespan = %.14f\n", meanMakespan);
			if(IloCeil(meanMakespan) > intUpperBound) {
				printf("Interger capacity is overflowed!\n");
				problem = IloTrue;
			}
	//		printf("\n");

			IloInt makespanIntTemp = (IloInt) IloCeil(meanMakespan);
			for(i = 0; i < nbProcessPlans; i++) {
				IloInt minProcPlanEndTime = 0;
				for(j = 0; j < nbActivities; j++) {
					minProcPlanEndTime += mDur[i][j];
				}
				dueDates[i] = minProcPlanEndTime + randGenerator.getInt(makespanIntTemp - minProcPlanEndTime);
			//	printf("Process plan %ld: due date = %ld\tminimum end time = %ld\n", i, dueDates[i], minProcPlanEndTime);
			}

			IloNum makespanTardiCost = 0.0;
			for(i = 0; i < nbProcessPlans; i++) {
			//	printf("Effective end time of process plan %ld is equal to %ld.\n", i, effProcPlanEndTimes[i]);
				IloNum procPlanTardi = IloMax(0.0, effProcPlanEndTimes[i] - dueDates[i]);
				makespanTardiCost += procPlanTardi * phis[i];
			}
			printf("Tardiness cost of this solution is equal to %.14f.\n", makespanTardiCost);
			IloNum makespanGlobalCost = makespanAllocCost + beta * makespanTardiCost;
			printf("Global cost of this solution is equal to %.14f.\n", makespanGlobalCost);
			printf("\n");

			// BETA IS DETERNINED AS FOLLOWS.
	//		printf("Beta is now determined.\n\n");
	//		IloNum beta = 4.36 * 2 * 2 * nbTotalResources * deltaAlloc / ((phiMax + phiMin) * delta * nbActivities * nbProcessPlans);
	//		printf("For this instance, beta = %.14f\n\n", beta);

			if(gamma > 1)
				printf("gamma should be less than 1.0.\n");

			IloInt kAllocLowerBound = 0;
			IloInt kAllocUpperBound = 0;
				for(i = 0; i < nbProcessPlans; i++) {
					for(j = 0; j < nbActivities; j++) {
						IloInt minTemp = IloIntMax;
						IloInt maxTemp = 0;
						for(k = 0; k < nbResources; k++) {
							if(allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]] < minTemp)
								minTemp = allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]];
							if(allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]] > maxTemp)
								maxTemp = allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]];
						}
						kAllocLowerBound += minTemp;
						kAllocUpperBound += maxTemp;
					}
				}
			printf("Allocation cost:\tlower bound = %ld\tupper bound = %ld\n\n", kAllocLowerBound, kAllocUpperBound);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			IloNum meanTardiness = 0.0;
			IloNum meanGlobalCost = 0.0;
			IloInt maxDuration = 0;
			IloInt tardinessAllocCost = 0;
			for(seedTemp = 1; seedTemp < sampleSize + 1; seedTemp++) {
//			for(seedTemp = sampleSize + 1; seedTemp < 2 * sampleSize + 1; seedTemp++) {
				
				// MEAN TARDINESS COST IS DETERMINED AS FOLLOWS.
				printf("Mean tardiness cost is now determined.\n\n");
				printf("A realization is generated with seed = %ld\n", seedTemp);
				randGenerator.reSeed(seedTemp);

			// PICKING EFFECTIVE ACTIVITY DURATIONS
				for(i = 0, k = 0; i < nbProcessPlans; i++) {
					for(j = 0; j < nbActivities; j++, k++) {
						durations[i][j] = (IloInt) durationVars[k]->getValue();
					//	printf("Process plan %ld: activity %ld lasts %ld time units.\n", i, j, durations[i][j]);
						if(durations[i][j] > maxDuration)
							maxDuration = durations[i][j];
					}
				}
				//printf("The longest activity duration is equal to %ld.\n", maxDuration);
				

			//	for(i = 0 ; i < nbProcessPlans ; i++) {
			//		for(j = 0 ; j < nbActivities ; j++) {
			//			printf("The activity a%ld%ld has an effective duration equal to %ld.\n",i,j,durations[i][j]);
			//		}
			//	}


			// PICKING EFFECTIVE BREAKDOWN START TIMES

				IloEnv envTemp;
				IloIntervalList* breakList = new (envTemp) IloIntervalList[nbTotalResources];
				for(i = 0 ; i < nbTotalResources ; i++) {
					breakList[i] = IloIntervalList(env);
					endBreak[i] = tStartSim;
					do{
						startBreak[i] = endBreak[i] + (IloInt) startP[i]->getValue();
						IloInt dur = (IloInt) durP[i]->getValue();
						endBreak[i] = startBreak[i] + dur;
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

				IloInt max = (IloNum)IloIntMax;
				temp = (IloNum)maxDuration;					// maxDuration is constant and is equal to the duration of one activity.
				for(j = nbProcessPlans*nbActivities; j > 0; j--) {
					IloInt activityEndMax = (IloInt)activityEnd + maxDuration;
					IloInt biggestActivityEnd = activityEndMax;
					for(i = 0; i < nbTotalResources; i++) {
						IloInt currentActivityEnd = activityEndMax;
						IloInt currentDuration = maxDuration;
						IloInt min = (IloInt)activityEnd;
						while((startBreak[i] < currentActivityEnd)&&(tempStart[i] <= max)&&(tempEnd[i] <= max)) {
							breakList[i].addInterval((IloNum)startBreak[i], (IloNum)endBreak[i]);
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

			//	for(i = 0 ; i < nbTotalResources ; i++) {
			//		printf("Concerning resource r%ld:\n", i);
			//		j = 0;
			//		for(IloIntervalListCursor cursor(breakList[i]); cursor.ok(); ++cursor) {
			//			printf("A breakdown starts at time %.1f and ends at time %.1f.\n", cursor.getStart(), cursor.getEnd());
			//			j++;
			//		}
			//		printf("Number of breakdowns = %ld\n", j);
			//	}

				IloNumVar tardiCost;
				IloArray<IloActivityArray> actArrays;
				IloSchedulerSolution solution(envTemp);

				IloModel model = DefineModelTardiness(envTemp,
													  nbProcessPlans,
													  nbActivities,
													  nbResources,
													  nbTotalResources,
													  resourceNumbers,
													  durations,
													  breakList,
													  dueDates,
													  phis,
													  beta,
													  gamma,
													  randGenerator,
													  solution,
													  actArrays,
													  tardiCost);
				IloNum tardiTemp = SolveModelTardiness(model,
													   tardiCost,
													   actArrays,
													   solution,
													   globalSearchTimeLimit,
													   globalSearchFailLimit,
													   localSearchTimeLimit,
													   localSearchFailLimit,
													   relaxActProb);
				IloNum kTardiLowerBound = tardiTemp;
				tardiTemp /= sampleSize;
				meanTardiness += tardiTemp;
				tardinessAllocCost = ComputeAllocCost(solution, allocCosts);
				envTemp.end();
				IloNum tardinessGlobalCost = tardinessAllocCost + beta * kTardiLowerBound;
				printf("Global cost of this solution is equal to %.14f.\n\n", tardinessGlobalCost);





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				IloNum kMaxLowerBound = kAllocLowerBound + beta * kTardiLowerBound;
				printf("Lower bound of global cost for this realization = %.14f\n", kMaxLowerBound);

				// MEAN GLOBAL COST IS DETERMINED AS FOLLOWS.
				printf("Mean global cost is now determined.\n\n");

				printf("A realization is generated with seed = %ld\n", seedTemp);
				randGenerator.reSeed(seedTemp);

				// PICKING EFFECTIVE ACTIVITY DURATIONS
		//		IloInt maxDuration = 0;
				for(i = 0, k = 0; i < nbProcessPlans; i++) {
					for(j = 0; j < nbActivities; j++, k++) {
						durations[i][j] = (IloInt) durationVars[k]->getValue();
						if(durations[i][j] > maxDuration)
							maxDuration = durations[i][j];
					}
				}
				//printf("The longest activity duration is equal to %ld.\n", maxDuration);
				

			//	for(i = 0 ; i < nbProcessPlans ; i++) {
			//		for(j = 0 ; j < nbActivities ; j++) {
			//			printf("The activity a%ld%ld has an effective duration equal to %ld.\n",i,j,durations[i][j]);
			//		}
			//	}


			// PICKING EFFECTIVE BREAKDOWN START TIMES
				
				IloEnv envTemp2;
				IloIntervalList* breakList2 = new (envTemp2) IloIntervalList[nbTotalResources];
				for(i = 0 ; i < nbTotalResources ; i++) {
					breakList2[i] = IloIntervalList(env);
					endBreak[i] = tStartSim;
					do{
						startBreak[i] = endBreak[i] + (IloInt) startP[i]->getValue();
						IloInt dur = (IloInt) durP[i]->getValue();
						endBreak[i] = startBreak[i] + dur;
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

				
				activityEnd = 0.0;					// activityEnd is the date at which the preceding virtual activity ends execution by taking
															// into account all current breakdowns.

				max = (IloNum)IloIntMax;
				temp = (IloNum)maxDuration;					// maxDuration is constant and is equal to the duration of one activity.
				for(j = nbProcessPlans*nbActivities; j > 0; j--) {
					IloInt activityEndMax = (IloInt)activityEnd + maxDuration;
					IloInt biggestActivityEnd = activityEndMax;
					for(i = 0; i < nbTotalResources; i++) {
						IloInt currentActivityEnd = activityEndMax;
						IloInt currentDuration = maxDuration;
						IloInt min = (IloInt)activityEnd;
						while((startBreak[i] < currentActivityEnd)&&(tempStart[i] <= max)&&(tempEnd[i] <= max)) {
							breakList2[i].addInterval((IloNum)startBreak[i], (IloNum)endBreak[i]);
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

		//		for(i = 0 ; i < nbTotalResources ; i++) {
		//			printf("Concerning resource r%ld:\n", i);
		//			j = 0;
		//			for(IloIntervalListCursor cursor(breakList2[i]); cursor.ok(); ++cursor) {
		//				printf("A breakdown starts at time %.1f and ends at time %.1f.\n", cursor.getStart(), cursor.getEnd());
		//				j++;
		//			}
		//			printf("Number of breakdowns = %ld\n", j);
		//		}

				IloNumVar globalCost, allocCost;
				IloArray<IloActivityArray> actArrays2;
				IloSchedulerSolution solution2(envTemp2);

				IloModel model2 = DefineModelGlobalCost(envTemp2,
														nbProcessPlans,
														nbActivities,
														nbResources,
														nbTotalResources,
														resourceNumbers,
														durations,
														breakList2,
														allocCosts,
														dueDates,
														phis,
														beta,
														gamma,
														omega,
														randGenerator,
														solution2,
														actArrays2,
														globalCost,
														allocCost,
														tardiCost);
				IloNum globalCostTemp = SolveModelGlobalCost(model2,
															 globalCost,
															 allocCost,
															 tardiCost,
															 actArrays2,
															 solution2,
															 globalSearchTimeLimit,
															 globalSearchFailLimit,
															 localSearchTimeLimit,
															 localSearchFailLimit,
															 kAllocLowerBound,
															 kTardiLowerBound,
															 relaxActProb);//, nbTotalResources);
				globalCostTemp /= sampleSize;
				meanGlobalCost += globalCostTemp;
				PrintSolution(solution2, actArrays2, globalCost, IloTrue);
				envTemp2.end();
			}
			
			printf("For this instance, mean tardiness = %.14f\n", meanTardiness);
			if(meanTardiness > intUpperBound) {
				printf("Interger capacity is overflowed!\n");
				problem = IloTrue;
			}

			printf("For this instance, mean global cost = %.14f\n", meanGlobalCost);
			if(meanGlobalCost > intUpperBound) {
				printf("Integer capacity is overflowed!\n");
				problem = IloTrue;
			}

			kMax = meanGlobalCost;




			IluNormalVariableI::End(); // The normal distribution is here erased.

		// INSTANCE FILE CREATION
			if(IloFalse == problem) {
				char buffer[128];
				sprintf(buffer, "Instance%ldPP%ldAct%ldRes%ldTResMinUnc%.1fMaxUnc%.1fSeed%ld.dat", nbProcessPlans, nbActivities, nbResources, nbTotalResources,
					alphaDurMin, alphaDurMax, seed);
				printf(buffer);
				printf("\n");
				ofstream stream(buffer); // A file named buffer is created.
				stream.precision(32);

				stream << nbProcessPlans << "\t" << nbActivities << "\t" << nbResources << "\t" << nbTotalResources << "\t" << kMax << "\t"	<< endl;
		
				for(i = 0 ; i < nbProcessPlans ; i++) {
					for(j = 0 ; j < nbActivities ; j++) {
						for(k = 0 ; k < nbResources ; k++) {
							stream << resourceNumbers[i][j][k] << "\t"<< allocCosts[i * nbActivities + j][resourceNumbers[i][j][k]] << "\t";
						}
						stream << mDur[i][j] << "\t" << sigmaDur[i][j] << "\t";
					}
					IloInt temp = IloInt (phis[i] * beta * precision);
					phis[i] = (IloNum) temp/precision;
					stream << dueDates[i] << "\t" << phis[i] << endl;
				}
				for(i = 0 ; i < nbTotalResources ; i++) {
					stream << mTimeBreak[i] << "\t" << sigmaTimeBreak[i] << "\t" << mBreakDur[i] << "\t" <<	sigmaBreakDur[i] << endl;
				}
				stream.close();
			}


			env.end();
		}
		
	} catch(IloException& exc) { cout << exc << endl; }

	return 0;
}
