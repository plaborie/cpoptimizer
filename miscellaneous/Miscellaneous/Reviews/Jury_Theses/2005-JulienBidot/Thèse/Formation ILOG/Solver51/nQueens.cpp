/* ------------------------------------------------------------

Problem nQueen
-------------------


Exercise
--------
   Search for a solution of this problem
------------------------------------------------------------ */



#include <ilsolver/ilosolver.h>
ILOSTLBEGIN

int main(){
   IloEnv env;
   try {
      IloModel model(env);
      IloInt nQueen;
      
      // declare the variables
      // ---------------------
      IloNumVarArray queens(env,nQueen,1,nQueen,ILOINT); 
      
      
      // post the constraints
      // --------------------
      
      model.add(IloAllDiff(env,queens));
      
	  for(IlcInt i=0;i<nQueens;i++) {
		  for(IlcInt j=1;j<nQueens;j++) {
			model.add(queens[j]-queens[i]!=j-i);
		  }
	  }



	  IloSolver solver(model);
	  solver.solve();
	  //while(solver.solve()) {
		  env.out() << "Solution :" << endl;
		  for(IlcInt i=0;i<5;i++) {
			env.out() << solver.getValue(queens[i]) << endl;
		  }
		  env.out() << endl;
	  //}
      
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