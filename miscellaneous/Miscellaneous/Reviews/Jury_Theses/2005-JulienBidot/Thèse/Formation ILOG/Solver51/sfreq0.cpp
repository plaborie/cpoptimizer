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
   Search for a solution of this problem
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
		model.add(IloAbs(freq[0] - freq[1]) >= 3);
		model.add(IloAbs(freq[0] - freq[4]) >= 2);
		model.add(IloAbs(freq[1] - freq[2]) >= 2);
		model.add(IloAbs(freq[1] - freq[3]) >= 1);
		model.add(IloAbs(freq[1] - freq[4]) >= 2);
		model.add(IloAbs(freq[2] - freq[3]) >= 3);
		model.add(IloAbs(freq[2] - freq[4]) >= 1);
		model.add(IloAbs(freq[3] - freq[4]) >= 2);
    
	  }
	  catch (IloException& ex) {
		cerr << "Error: " << ex << endl;
	  } 
	env.end();
	return 0;
}

/* ------------------------------------------------------------
No Output.
------------------------------------------------------------ */







