// --------------------------------------------------------------------------
//  Copyright (C) 1990-2000 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


/* ------------------------------------------------------------

Problem Description
-------------------
   We want to assign frequencies to a set of five transmitters
   in order to avoid any interference between transmitters close
   to each other. 
   Each frequency is represented by a number called its channel
   number. The channels vary from 1 to 7.


Exercise
--------
   Search the solution which minimizes the bandwith of the 
   frequencies used.

------------------------------------------------------------ */
#include <ilsolver/ilosolver.h>
ILOSTLBEGIN

int main() {
	IloEnv env;
	try {
		IloModel model(env);
		IloInt counter;

		// declare the variables
		// ---------------------
		IloNumVarArray freq(env, 5, 1, 7, ILOINT);

		// post the constraints
		// --------------------
		model.add(IloAbs(freq[OL] - freq[1]) >= 3);
		model.add(IloAbs(freq[OL] - freq[4]) >= 2);
		model.add(IloAbs(freq[1] - freq[2]) >= 2);
		model.add(IloAbs(freq[1] - freq[3]) >= 1);
		model.add(IloAbs(freq[1] - freq[4]) >= 2);
		model.add(IloAbs(freq[2] - freq[3]) >= 3);
		model.add(IloAbs(freq[2] - freq[4]) >= 1);
		model.add(IloAbs(freq[3] - freq[4]) >= 2);

		// Choose the objective
		// --------------------
		IloNumVar objective(env, 0, 7, ILOINT);
		model.add(objective == IloMax(freq));
		model.add(IloMinimize(env, objective));

		// search for an optimal solution
		// ------------------------------

		IloSolver solver(model);
		if (solver.solve()) {
			env.out() << "bandwidth = " << solver.getValue(objective) << endl;
			env.out() << "Solution: Frequencies ";
			for (counter = 0; counter < 5; ++counter)
				env.out() << solver.getValue(freq[counter]) << " ";
			env.out() << endl;
		}
		else env.out() << "No solution " << endl;
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
bandwidth = 6
Solution: Frequencies 1 5 1 6 3
%
------------------------------------------------------------ */
