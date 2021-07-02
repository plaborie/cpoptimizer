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
      IloInt nQueen = 12;
      
      // declare the variables
      // ---------------------
      IloNumVarArray queens(env,nQueen,0,nQueen-1,ILOINT); 
      
      
      // post the constraints
      // --------------------
      
      model.add(IloAllDiff(env,queens));
      
	  for(IlcInt i=0;i<nQueen-1 ;i++) {
		  for(IlcInt j=i+1;j<nQueen;j++) {
			model.add(IloAbs(queens[j]-queens[i])!=IloAbs(j-i));
		  }
	  }



	  IloSolver solver(model);
	  if (solver.solve())
	  {
	  //while(solver.solve()) {
		  env.out() << "Solution :" << endl;
		  for(IlcInt k=0;k<nQueen;k++) {
			env.out() << solver.getValue(queens[k]) << endl;
		  }
		  env.out() << endl;
		  solver.printInformation();
	  } else {env.out() << "No solution" << endl;}
      
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