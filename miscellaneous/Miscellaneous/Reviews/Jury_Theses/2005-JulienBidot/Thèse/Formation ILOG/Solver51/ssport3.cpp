// --------------------------------------------------------------------------
//  Copyright (C) 1990-2000 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

#include <ilsolver/ilosolver.h>
ILOSTLBEGIN

typedef IloArray<IloNumVarArray> NumVarMatrix; 

IloInt nbTeams;
IloInt nbWeeks;
IloInt nbPeriods;
IloInt nbSlots;

int main(int argc, char** argv){
	
	nbTeams = (argc>1? atol(argv[1] ) : 8 );
	// nbTeams must be an even number.
	if ( nbTeams % 2 ) ++nbTeams;

	nbWeeks = nbTeams-1;
	nbPeriods = nbTeams /2;
	nbSlots = nbWeeks * nbPeriods;

	IloEnv env;
	try {
		
		IloModel model(env);

		// declare the variables
		// ---------------------
		ostream& log = env.out();

		log << "* sports scheduling."
			<< "teams: " << nbTeams << ' '
			<< "weeks: " << nbWeeks << ' '
			<< "periods: " << nbPeriods 
			<< endl;

		// build matrix of slot variables
		NumVarMatrix slotVars = NumVarMatrix(env, nbPeriods);
		// ie : NumVarMatrix slotVars = IloArray<IloNumVarArray>(env, nbPeriods);
		for(IloInt i = 0; i < nbPeriods; i++)
			slotVars[i] = IloNumVarArray(env, nbWeeks, 0, nbSlots-1, ILOINT);

		// build copy vector of slot variables matrix 
		IloNumVarArray allSlotVars(env, nbSlots);
		IloInt s = 0;
		for (IloInt p = 0; p < nbPeriods; ++p) {
			for (IloInt w = 0; w < nbWeeks; ++w) {
				allSlotVars[s] = slotVars[p][w];
				++s;
			}
		}

		// matches
		IloNumArray firstTeam (env, nbSlots);
		IloNumArray secondTeam(env, nbSlots);
		IloInt m = 0;
		for (i=0; i < nbTeams; ++i) {
			for ( IloInt j=i+1; j < nbTeams; ++j) {
				firstTeam[m]  = i;
				secondTeam[m] = j;
				++m;
			}
		}
		assert( m == nbSlots );

		// build matrix of team variables
		NumVarMatrix firstTeamVars = NumVarMatrix(env, nbPeriods);
 		NumVarMatrix secondTeamVars = NumVarMatrix(env, nbPeriods);

		for (p = 0; p < nbPeriods; ++p) {
			firstTeamVars[p]  = IloNumVarArray(env, nbWeeks, 0, nbTeams-1, ILOINT);
			secondTeamVars[p] = IloNumVarArray(env, nbWeeks, 0, nbTeams-1, ILOINT);
		}

		// post the constraints
		// --------------------

		// ---------------------------------
		// Constraint 1: link team and match vars with elem cst
		// ---------------------------------
		IloInt z = 0;
		for (p = 0; p < nbPeriods; ++p) {
			for (IloInt w = 0; w < nbWeeks; ++w) {
				IloNumVar slotVar = allSlotVars[z];
				model.add( firstTeamVars[p][w]  == firstTeam ( slotVar ) );
				model.add( secondTeamVars[p][w] == secondTeam( slotVar ) );
				++z;
			}
		}

		// ---------------------------------
		// Constraint 2 : each team plays each other exactly once,
		// i.e. a match (T1, T2) occurs exactly once.
		// ---------------------------------
		model.add( IloAllDiff(env, allSlotVars));

		// ---------------------------------
		// Constraint 3 : each week, all teams do play.
		// remember 2*nbPeriods == nbTeams.
		// ---------------------------------
		for (IloInt w = 0; w < nbWeeks; ++w) {
			IloNumVarArray weekTeams(env, nbTeams);
			for (p = 0; p < nbPeriods; ++p) {
				weekTeams[2*p]   = firstTeamVars [p][w];
				weekTeams[2*p+1] = secondTeamVars[p][w];
			}
			model.add( IloAllDiff(env, weekTeams) );
		}

		// ---------------------------------
		// Adding a dummy column of size nbPeriods.
		// ---------------------------------
		IloNumVarArray dummyFirstTeamVars(env, nbPeriods, 0, nbTeams-1, ILOINT);
		IloNumVarArray dummySecondTeamVars(env, nbPeriods, 0, nbTeams-1, ILOINT);
		IloNumVarArray dummyTeamVars(env, nbTeams);

		for (p=0; p < nbPeriods; ++p) {
			model.add( dummyFirstTeamVars[p]  < dummySecondTeamVars[p] );
			dummyTeamVars[ 2*p ]  = dummyFirstTeamVars[p];
			dummyTeamVars[ 2*p+1] = dummySecondTeamVars[p];
		}
		model.add( IloAllDiff(env, dummyTeamVars) );
			
		// ---------------------------------
		// constraint 4 : in each period, each team plays no more than twice.
		// one can demonstrate that the number of occurences of each team in
		// one period is ate least, so we constrain it to be in [1..2]
		// ---------------------------------
		IloNumArray teams(env, nbTeams);
		for (m = 0; m < nbTeams; ++m) 
			teams[m] = m;

		for (p = 0; p < nbPeriods; ++p) {
			// the array grows...
			IloNumVarArray periodTeams(env, 2*nbWeeks+2);
			for ( w = 0; w < nbWeeks; ++w) {
				periodTeams[ 2*w ]   = firstTeamVars [p][w];
				periodTeams[ 2*w+1 ] = secondTeamVars[p][w];
			}
			// the dummy column
			periodTeams[ 2 * nbWeeks ] = dummyFirstTeamVars[p];
			periodTeams[ 2 * nbWeeks+1 ] = dummySecondTeamVars[p];

			// the cardinality value : exactly 2
			IloNumVarArray cardVars(env, nbTeams, 2, 2, ILOINT);

			model.add( IloDistribute(env, cardVars, teams, periodTeams) );
		}
		
		// search for a solution
		// ---------------------
		IloSolver solver(env);
		solver.extract(model);

		// experiment different level of propagation 
		solver.setDefaultFilterLevel(IlcAllDiffCt, IlcExtended);

		if (solver.solve()) {
			cout << endl << "SOLUTION" << endl;
			for (p=0; p < nbPeriods; ++p) {
				log << "period " << p << " : ";
				for (IloInt w=0; w < nbWeeks; ++w) {
					log << solver.getValue(firstTeamVars[p][w]) << " vs " << solver.getValue(secondTeamVars[p][w]) << " - " ;
				}
				log << endl;
			}

			solver.printInformation();
		}
		else
	      cout << "**** NO SOLUTION ****" << endl;
	}
	catch (IloException& ex) {
		cerr << "Error: " << ex << endl;
	} 
	
	env.end();
	return 0;

}

/* ------------------------------------------------------------
Output:

%
* sports scheduling.teams: 8 weeks: 7 periods: 4

SOLUTION
period 0 : 0 vs 1 - 0 vs 2 - 1 vs 2 - 3 vs 4 - 3 vs 5 - 4 vs 5 - 6 vs 7 -
period 1 : 2 vs 3 - 3 vs 6 - 4 vs 6 - 0 vs 7 - 0 vs 4 - 2 vs 7 - 1 vs 5 -
period 2 : 4 vs 7 - 5 vs 7 - 0 vs 5 - 1 vs 6 - 2 vs 6 - 1 vs 3 - 0 vs 3 -
period 3 : 5 vs 6 - 1 vs 4 - 3 vs 7 - 2 vs 5 - 1 vs 7 - 0 vs 6 - 2 vs 4 -

%
------------------------------------------------------------ */
