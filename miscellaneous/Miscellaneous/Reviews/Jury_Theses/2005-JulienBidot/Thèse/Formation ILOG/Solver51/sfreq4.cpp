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
   Search a solution for this problem.

------------------------------------------------------------ */

#include <ilsolver/ilosolver.h>
ILOSTLBEGIN

class IlcAreNotMultipleI : public IlcConstraintI {
private:
  IlcIntExp _x;
  IlcIntExp _y;
public:
  IlcAreNotMultipleI(IloSolver s, IlcIntExp x, IlcIntExp y);
  void post();
  void propagateAux(IlcIntExp x, IlcIntExp y);
  void propagate();
};

IlcAreNotMultipleI::IlcAreNotMultipleI(IloSolver s,
				       IlcIntExp x,
				       IlcIntExp y):
  IlcConstraintI(s), _x(x), _y(y){}

void IlcAreNotMultipleI::post() {
  _x.whenValue(this);
  _y.whenValue(this);
}

void IlcAreNotMultipleI::propagateAux(IlcIntExp x, IlcIntExp y){
  if (x.isBound()) {
    IlcInt value = x.getValue();
    for (IlcIntExpIterator iter(y); iter.ok(); ++iter) {
      if ((*iter) % value == 0) {
	y.removeValue(*iter);
      }
    }
  }
}

void IlcAreNotMultipleI::propagate(){
  propagateAux(_x,_y);
  propagateAux(_y,_x);
}

IlcConstraint IlcAreNotMultiple(IloSolver s, IlcIntExp x, IlcIntExp y) {
  return new (s.getHeap()) IlcAreNotMultipleI(s, x, y);
}

ILOCPCONSTRAINTWRAPPER2(IloAreNotMultiple, solver, IloNumVar, x, IloNumVar, y) {
  use(solver, x);
  use(solver, y);
  return IlcAreNotMultiple(solver, solver.getIntVar(x), solver.getIntVar(y));
}

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
    model.add(IloAreNotMultiple(env, freq[OL], freq[1]));
    model.add(IloAbs(freq[OL] - freq[4]) >= 2);
    model.add(IloAreNotMultiple(env, freq[OL], freq[4]));
    model.add(IloAbs(freq[1] - freq[2]) >= 2);
    model.add(IloAreNotMultiple(env, freq[1], freq[2]));
    model.add(IloAbs(freq[1] - freq[3]) >= 1);
    model.add(IloAreNotMultiple(env, freq[1], freq[3]));
    model.add(IloAbs(freq[1] - freq[4]) >= 2);
    model.add(IloAreNotMultiple(env, freq[1], freq[4]));
    model.add(IloAbs(freq[2] - freq[3]) >= 3);
    model.add(IloAreNotMultiple(env, freq[2], freq[3]));
    model.add(IloAbs(freq[2] - freq[4]) >= 1);
    model.add(IloAreNotMultiple(env, freq[2], freq[4]));
    model.add(IloAbs(freq[3] - freq[4]) >= 2);
    model.add(IloAreNotMultiple(env, freq[3], freq[4]));
    
		// search for a solution
		// ---------------------

		IloSolver solver(model);
		if (solver.solve()) {
			env.out() << "Solution: Frequencies ";
			for (counter = 0; counter < 5; ++counter)
				env.out() << solver.getValue(freq[counter]) << " ";
			env.out() << endl;
		}
		else 
			env.out() << "No solution " << endl;
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
Solution: Frequencies 7 2 7 3 5
%
------------------------------------------------------------ */







