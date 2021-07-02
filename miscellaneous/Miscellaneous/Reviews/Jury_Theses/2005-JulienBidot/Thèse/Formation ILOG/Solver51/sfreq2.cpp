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
      IloInt nbSolution = 0;

      // declare the variables
      // ---------------------
      IloNumVarArray freq(env, 5, 1, 7, ILOINT);
    
      // post the constraints
      // --------------------
      model.add(IloAbs(freq[0L] - freq[1]) >= 3);
      model.add(IloAbs(freq[0L] - freq[4]) >= 2);
      model.add(IloAbs(freq[1] - freq[2]) >= 2);
      model.add(IloAbs(freq[1] - freq[3]) >= 1);
      model.add(IloAbs(freq[1] - freq[4]) >= 2);
      model.add(IloAbs(freq[2] - freq[3]) >= 3);
      model.add(IloAbs(freq[2] - freq[4]) >= 1);
      model.add(IloAbs(freq[3] - freq[4]) >= 2);
    
      // search for a solution
      // ---------------------

      IloSolver solver(model);
      solver.startNewSearch();
      while (solver.next()) {
         nbSolution++;
         env.out() << "Solution: Frequencies ";
         for (IloInt counter = 0; counter < 5; ++counter)
            env.out() << solver.getValue(freq[counter]) << " ";
         env.out() << endl;
      }
      solver.endSearch();
      env.out() << nbSolution << " solutions" << endl;
      solver.printInformation();
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
Solution: Frequencies 1 4 7 1 6 ...
Solution: Frequencies 7 4 1 7 2
120 solutions
Number of fails               : 0
Number of choice points       : 119
Number of variables           : 5
Number of constraints         : 0
Reversible stack (bytes)      : 4044
Solver heap (bytes)           : 8064
Solver global heap (bytes)    : 4044
And stack (bytes)             : 4044
Or stack (bytes)              : 4044
Search Stack (bytes)          : 4044
Constraint queue (bytes)      : 11144
Total memory used (bytes)     : 39428
Elapsed time since creation   : 0.3
%
------------------------------------------------------------ */
